#include <hkbase\config\hkConfigVersion.h>
#include "config.h"
#include "havokincludes.h"

#include "collisionbitfield.h"
#include "world.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include <hkmath/hkmath.h>
#include <hkanimation/rig/hkPose.h>
#include <hkanimation/rig/hkSkeleton.h>
#include <hkanimation/ik/lookat/hkLookAtIkSolver.h>
#include <hkvisualize/hkDebugDisplay.h>
#include <hkcollide\castutil\hkWorldRayCastInput.h>
#include <hkcollide\collector\raycollector\hkClosestRayHitCollector.h>
#include <hkdynamics/world/hkWorld.h>

#include <hkdynamics/phantom/hkShapePhantom.h>
#include <hkcollide/shape/sphere/hkSphereShape.h>
#include <hkcollide/castutil/hkLinearCastInput.h>
#include <hkcollide/collector/pointcollector/hkSimpleClosestContactCollector.h>
#include <hkdynamics/phantom/hkSimpleShapePhantom.h>

#endif

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

#include <hkmath/hkMath.h>
#include <hkdynamics/entity/hkRigidBody.h>
#include <hkbase/debugutil/hkSimpleStatisticsCollector.h>
#include <hkconstraintsolver/simplex/hkSimplexSolver.h>
#include "hsCharacterProxy.h"
#include <hkutilities/charactercontrol/characterproxy/hkCharacterProxyCinfo.h>
#include <hkdynamics/phantom/hkShapePhantom.h>
#include <hkcollide/collector/pointcollector/hkAllCdPointCollector.h>
#include <hkcollide/castutil/hkLinearCastInput.h> 
#include <hkcollide/agent/hkProcessCollisionInput.h>
#include <hkconstraintsolver/simplex/hkSimplexSolver.h>
#include <hkvisualize/hkDebugDisplay.h>
#include <hkbase/memory/hkLocalArray.h>
#include <hkmath/linear/hkSweptTransformUtil.h>

// Enable this to see manifold planes. Beware it really slows down integration (by 30%)
#ifndef _RELEASE 
#define DEBUG_CHARACTER_CONTROLLER
#endif

#include "physics/havokthreadutils.h"
#include "collisionbitfield.h"

static float LOW_WALL_ANGLE_LIMT = -0.4f; 

/// Initialize members from the construction info
/// Add references to each of the shapes
hsCharacterProxy::hsCharacterProxy(const hkCharacterProxyCinfo& info, float fUpdateEverySeconds)
	: hkCharacterProxy(info)
{
	// Initialise all our control vars for position extrapolation while not integrating
	m_obTimeSinceLastUpdate.m_startTime = 0.0f;
	m_obTimeSinceLastUpdate.m_endTime = 0.0f;
	m_obTimeSinceLastUpdate.m_deltaTime = 0.0f;
	m_obTimeSinceLastUpdate.m_invDeltaTime = 0.0f;

	m_fUpdateEverySeconds = fUpdateEverySeconds; // A proper integration will occur every these seconds, and a linear extrapolation in between

	m_obMyPsuedoPosition = m_shapePhantom->getCollidable()->getTransform().getTranslation();
	m_aobMyLastRealPosition[0] = m_shapePhantom->getCollidable()->getTransform().getTranslation();
	m_aobMyLastRealPosition[1] = m_shapePhantom->getCollidable()->getTransform().getTranslation();

	m_bHasBeenUpdatedOnce = false;
	m_bForceUpdate = false;
	m_oldVelocity.setZero4();
}

void hsCharacterProxy::updateFromCinfo( const hkCharacterProxyCinfo& info )
{
	HK_ASSERT2(0x1e81f814, info.m_shapePhantom != HK_NULL, "Shape phantom can not be NULL");

	info.m_shapePhantom->addReference();
	if (m_shapePhantom)
	{
		m_shapePhantom->removeReference();
	}

	m_velocity = info.m_velocity;
	m_dynamicFriction = info.m_dynamicFriction;
	m_staticFriction = info.m_staticFriction;
	m_keepContactTolerance = info.m_keepContactTolerance;
	m_extraUpStaticFriction = info.m_extraUpStaticFriction;
	m_extraDownStaticFriction = info.m_extraDownStaticFriction;
	m_keepDistance = info.m_keepDistance;
	m_shapePhantom = info.m_shapePhantom;
	m_contactAngleSensitivity = info.m_contactAngleSensitivity;
	m_maxCastIterations = info.m_maxCastIterations;
	m_refreshManifoldInCheckSupport = info.m_refreshManifoldInCheckSupport;
	m_userPlanes = info.m_userPlanes;
	m_maxCharacterSpeedForSolver = info.m_maxCharacterSpeedForSolver;
	m_characterStrength = info.m_characterStrength;
	m_characterMass = info.m_characterMass;
	m_maxSlopeCosine = hkMath::cos( info.m_maxSlope );
	m_penetrationRecoverySpeed = info.m_penetrationRecoverySpeed;

	m_up = info.m_up;
	m_up.normalize3();

	// On construction we cannot guarantee that the phantom
	// has been added to the world already.
	// This often depends on the users specific toolchain
	if (m_shapePhantom->getWorld() != HK_NULL)
	{
		setPosition(info.m_position);
	}
	else
	{
		HK_WARN(0x6cee9071, "Shape phantom has not yet been added to the world. Initial position has been ignored");
	}

	m_oldVelocity.setZero4();

	{
		for (int i=0; i< m_bodies.getSize(); i++)
		{
			m_bodies[i]->removeEntityListener(this);
		}
		m_bodies.clear();

		for (int j=0; j< m_phantoms.getSize(); j++)
		{
			m_phantoms[j]->removePhantomListener(this);
		}
		m_phantoms.clear();
	}

	m_manifold.clear();

	// Add a property that allows us to identify this as a character proxy later
	if ( !m_shapePhantom->hasProperty(HK_PROPERTY_CHARACTER_PROXY) )
	{
		m_shapePhantom->addProperty(HK_PROPERTY_CHARACTER_PROXY, this);
	}
}

/// Remove references to the phantom
/// Remove the phantom if it has been added to the world
hsCharacterProxy::~hsCharacterProxy()
{
}

// Planes in this colour are passed directly to the simplex solver
// These planes actually restrict character movement
const hkInt32 HK_DEBUG_CLEAN_SIMPLEX_MANIFOLD_COLOR = hkColor::GREEN;

// Planes in this color are additional planes the user had added 
// Typically these are added in the processConstraintsCallback
// These planes actually restrict character movement
const hkInt32 HK_DEBUG_USER_SIMPLEX_MANIFOLD_COLOR = 0xffffdf00;

// Planes in this color are additional vertical planes added to restrict
// character movement up steep slopes
const hkInt32 HK_DEBUG_VERTICAL_SIMPLEX_MANIFOLD_COLOR = hkColor::CYAN;


// Planes in this color show the results of the start point collector
// These planes are filtered by the character proxy
const hkInt32 HK_DEBUG_STARTPOINT_HIT_POSITION_COLOR = hkColor::WHITE;

// Planes in this color show the results of the cast collector
// These planes are filtered by the character proxy
const hkInt32 HK_DEBUG_CASTPOINT_HIT_POSITION_COLOR = hkColor::WHITE;

// Planes in this color show the +ve distance returned by the start point collector
// These planes are filtered by the character proxy
const hkInt32 HK_DEBUG_NONPENETRATING_STARTPOINT_DIST_COLOR = hkColor::BLUE;

// Planes in this color show the -ve distance returned by the start point collector
// These planes are filtered by the character proxy
const hkInt32 HK_DEBUG_PENETRATING_STARTPOINT_DIST_COLOR = hkColor::RED;

// Planes in this color show the distance returned by the cast collector
// These planes are filtered by the character proxy
const hkInt32 HK_DEBUG_CASTPOINT_DIST_COLOR = hkColor::MAGENTA;

#ifdef DEBUG_CHARACTER_CONTROLLER
static void debugCast(const hkAllCdPointCollector& startPointCollector, const hkAllCdPointCollector& castCollector)
{
	{
		for (int h=0; h < startPointCollector.getHits().getSize(); h++)
		{
			const hkRootCdPoint& hit = startPointCollector.getHits()[h];
			hkVector4 plane = hit.m_contact.getNormal();
			plane(3) = 0.0f;
			hkVector4 pos = hit.m_contact.getPosition();
			HK_DISPLAY_PLANE(plane, pos, 0.5f, HK_DEBUG_STARTPOINT_HIT_POSITION_COLOR);
			pos.addMul4(hit.m_contact.getDistance(), hit.m_contact.getNormal());

				if (hit.m_contact.getDistance() < 0.0f)
				{
					HK_DISPLAY_PLANE(plane, pos, 0.5f, HK_DEBUG_PENETRATING_STARTPOINT_DIST_COLOR);
				}
				else
				{
					HK_DISPLAY_PLANE(plane, pos, 0.5f, HK_DEBUG_NONPENETRATING_STARTPOINT_DIST_COLOR);
				}
			}
		}

	// Add castCollector plane
	{
		for (int h=0; h < castCollector.getHits().getSize(); h++)
			{
			const hkRootCdPoint& hit = castCollector.getHits()[h];
			hkVector4 plane = hit.m_contact.getNormal();
			plane(3) = 0.0f;
			hkVector4 pos = hit.m_contact.getPosition();
			HK_DISPLAY_PLANE(plane, pos, 0.5f, HK_DEBUG_CASTPOINT_HIT_POSITION_COLOR);
			pos.addMul4(hit.m_contact.getDistance(), hit.m_contact.getNormal());
			HK_DISPLAY_PLANE(plane, pos, 0.5f, HK_DEBUG_CASTPOINT_DIST_COLOR);
		}
	}
}
#endif

void hsCharacterProxy::convertFractionToDistance( const hkRootCdPoint* cHit, int numHits, const hkVector4& displacement ) const
{
	hkRootCdPoint* hit = const_cast<hkRootCdPoint*>(cHit);
	for (int i = numHits-1; i>=0; i--)
	{
		hkReal fraction = hit->m_contact.getDistance();
		hit->m_contact.getPosition()(3) = fraction;
		hkReal projLength = displacement.dot3(hit->m_contact.getNormal());
		hit->m_contact.setDistance( fraction * -projLength );
		hit++;
	}
}


void hsCharacterProxy::integrate( const hkStepInfo& stepInfo, const hkVector4& worldGravity )
{
	hkAllCdPointCollector castCollector;
	hkAllCdPointCollector startPointCollector;
	integrateWithCollectors(stepInfo, worldGravity, castCollector, startPointCollector);
}

static bool addMaxSlopePlane( hkReal maxSlopeCos, const hkVector4& up, int index, hkArray<hkSurfaceConstraintInfo>& surfaceConstraintsInOut)
{
	const hkReal surfaceVerticalComponent = surfaceConstraintsInOut[index].m_plane.dot3(up);

	if ( surfaceVerticalComponent > 0.01f && surfaceVerticalComponent < maxSlopeCos )
	{
		// Add an additional vertical plane at the end of the constraints array
		hkSurfaceConstraintInfo& newConstraint = surfaceConstraintsInOut.expandOne();

		// Copy original info
		newConstraint = surfaceConstraintsInOut[index];

		// Reorient this new plane so it is vertical
		newConstraint.m_plane.addMul4( -/*2.0f **/ surfaceVerticalComponent, up );
		newConstraint.m_plane.normalize3();
		return true;
	}

	// No plane added
	return false;
}

/// Main character update loop:
///	Loop until timestep complete
///		Collect information about my current position
///		update the manifold
///		solve the simplex to find new position and velocity
///		cast to new desired position
///		if (hit)
///			jump to hit position
///		else
///			jump to new desired position
///	grab velocity from simplex
void hsCharacterProxy::integrateWithCollectors(const hkStepInfo& stepInfoParam, const hkVector4& worldGravityVelocity, hkAllCdPointCollector& castCollector, hkAllCdPointCollector& startPointCollector)
{
	// If we've got nothing to do, do nothing - FIXME need to check for collisions as well
	/*if (m_velocity.lengthSquared3() == 0.0f)
		return;*/

	// If this the frame we want to update on (or we haven't yet updated at all), do a full integration step (if m_fUpdateEveryFrames is set to 0, this'll happen every frame)
	if (!m_bHasBeenUpdatedOnce || m_bForceUpdate || m_obTimeSinceLastUpdate.m_deltaTime >= m_fUpdateEverySeconds)
	{
		HK_TIMER_BEGIN_LIST("updateCharacter", "Cast");

		Physics::WriteAccess mutex;

		// Use values we've accumulated
		m_obTimeSinceLastUpdate.m_deltaTime += stepInfoParam.m_deltaTime;
		m_obTimeSinceLastUpdate.m_invDeltaTime = 1.0f/m_obTimeSinceLastUpdate.m_deltaTime;
		hkStepInfo stepInfo;
		stepInfo.m_deltaTime = m_obTimeSinceLastUpdate.m_deltaTime;
		stepInfo.m_invDeltaTime = m_obTimeSinceLastUpdate.m_invDeltaTime;
		// Reset the control vars
		m_obTimeSinceLastUpdate.m_deltaTime = 0.0f;
		m_obTimeSinceLastUpdate.m_invDeltaTime = 0.0f;
		m_bForceUpdate = false;

		// Setup initial variables
		hkVector4 position(m_aobMyLastRealPosition[1]); // Need to use the last properly calculated value as a base
		hkReal remainingTime = stepInfo.m_deltaTime;
		hkLinearCastInput castInput;
		{
			castInput.m_startPointTolerance = m_keepDistance + m_keepContactTolerance; 
			castInput.m_maxExtraPenetration = 0.01f;
		}
		hkSimplexSolverOutput output;
#define MAX_CHARACTER_ITERS 3 // keep this number low to save some performance.
		int iter = 0;
#if HAVOK_SDK_VERSION_MAJOR >= 4 && HAVOK_SDK_VERSION_MINOR >= 1		
		for (  ;(remainingTime > HK_REAL_EPSILON) && (iter < MAX_CHARACTER_ITERS) ; iter++ )
#else
		for (  ;(remainingTime > hkMath::HK_REAL_EPSILON) && (iter < MAX_CHARACTER_ITERS) ; iter++ )
#endif 
		{
			HK_TIMER_SPLIT_LIST("InitialCast");
			//
			//	Cast in a direction, hopefully the right one.
			//  We actually only really need the start point collector information.
			//  That means we are free to use any direction as a cast direction
			//  For optimizations, we cast into the direction of the old displacement
			//  (just a guess). That means we might be able to avoid a second cast;
			//  see below.
			//
			hkVector4 displacement;
			{								
				if (m_bCurrentVelocityPrediction)
				{
					displacement.setMul4(remainingTime, m_velocity);				
				}
				else
				{
					displacement.setMul4(remainingTime, m_oldVelocity);					
				}
				castInput.m_to.setAdd4( position, displacement);
	
				castCollector.reset();
				startPointCollector.reset();
				//	Call the caster				
				m_shapePhantom->setPositionAndLinearCast( position, castInput, castCollector, &startPointCollector );				

				fireContactsFound(castCollector, startPointCollector);

				// Change the hit point so it reflects the a real distance rather than a fraction
				if (castCollector.hasHit())
				{
					castCollector.sortHits();
					convertFractionToDistance( castCollector.getHits().begin(), castCollector.getHits().getSize(), displacement);
				}
			}

			//
			// Maintain the internal manifold of contact points
			// See the method for detailed rules
			//
			HK_TIMER_SPLIT_LIST("UpdateManifold");
			updateManifold(startPointCollector, castCollector);

	#if defined DEBUG_CHARACTER_CONTROLLER 
			debugCast(startPointCollector, castCollector);
	#endif 

			//
			// Convert manifold points to plane equations
			//

			HK_TIMER_SPLIT_LIST("ExtractSurfaceConstraint");

			// set penetration speed so, that it will get out from penetraton in one frame
			m_penetrationRecoverySpeed = 1.0f / remainingTime;
			hkLocalArray<hkSurfaceConstraintInfo> surfaceConstraints(m_manifold.getSize() + m_userPlanes + 10);
			surfaceConstraints.setSizeUnchecked(m_manifold.getSize()); 			
			{
				for (int i=0; i < m_manifold.getSize() ; i++)
				{
					extractSurfaceConstraintInfo(m_manifold[i], surfaceConstraints[i], stepInfo.m_deltaTime - remainingTime );
					const hkReal surfaceVerticalComponent = surfaceConstraints[i].m_plane.dot3(m_up);						
					if ( surfaceVerticalComponent > 0.01f && surfaceVerticalComponent < m_maxSlopeCosine )
					{
						hkRigidBody* bd = hkGetRigidBody( m_manifold[i].m_rootCollidableA );					
						if (!bd)
							bd = hkGetRigidBody( m_manifold[i].m_rootCollidableB );

						if( bd && bd->isFixedOrKeyframed())						
						{
							hkVector4 a = m_manifold[i].m_contact.getPosition(); 
							hkVector4 b = position; 

							float diffY = a(1) - b(1);

							if( diffY > LOW_WALL_ANGLE_LIMT )
							{
								// contact is too high, probably person can not step over it
								addMaxSlopePlane( m_maxSlopeCosine, m_up, i, surfaceConstraints);
							} else {
#if 1
								HK_TIMER_BEGIN("StairsCheckRayCast", HK_NULL);
								// the following code, verifies if object has geometry like stairs if yes do not limit movement
								Physics::EntityCollisionFlag info;
								info.base = bd->getCollidable()->getCollisionFilterInfo();
								if ((info.flags.i_am & Physics::SMALL_INTERACTABLE_BIT) == 0 && (info.flags.i_am & Physics::LARGE_INTERACTABLE_BIT) == 0)
								{
									HK_TIMER_END();
									continue;
								}

								// perform raycast only on the shape from colliding body (it is faster than on the world)
								const hkShape * shape = bd->getCollidable()->getShape();

								Physics::RaycastCollisionFlag obFlag; obFlag.base = 0;
								obFlag.flags.i_am = Physics::COLLISION_ENVIRONMENT_BIT;
								obFlag.flags.i_collide_with = (	Physics::SMALL_INTERACTABLE_BIT				|
									Physics::LARGE_INTERACTABLE_BIT				);

								hkShapeRayCastInput input;
								input.m_from	= position;
								input.m_to		= m_manifold[i].m_contact.getPosition();


								hkVector4 to = m_manifold[i].m_contact.getPosition(); 
								to.sub4(position);
								input.m_to.add4( to );						

								// all position must be in shape local coord 													
								input.m_from.setTransformedInversePos(bd->getTransform(), input.m_from);
								input.m_to.setTransformedInversePos(bd->getTransform(), input.m_to);
								input.m_filterInfo = obFlag.base;

								hkShapeRayCastOutput  collector;
								{
									Physics::CastRayAccess mutex;
									shape->castRay(input, collector);
								}

								if( collector.hasHit() )
								{
									collector.m_normal.setRotatedDir(bd->getTransform().getRotation(), collector.m_normal);

									// probably check if it stairs and if not limit movement if needed by adding extra constraint 
									if( ( collector.m_normal(1) > 0.1f ) && ( collector.m_normal(1) <  0.9f ) )
										addMaxSlopePlane( m_maxSlopeCosine, m_up, i, surfaceConstraints);
								}	
								HK_TIMER_END();
#else                            
								Physics::RaycastCollisionFlag obFlag; obFlag.base = 0;
								obFlag.flags.i_am = Physics::COLLISION_ENVIRONMENT_BIT;
								obFlag.flags.i_collide_with = (	Physics::SMALL_INTERACTABLE_BIT				|
									Physics::LARGE_INTERACTABLE_BIT				);

								hkWorldRayCastInput input;

								input.m_from	= position;
								input.m_to		= m_manifold[i].m_contact.getPosition();


								hkVector4 to = m_manifold[i].m_contact.getPosition(); 
								to.sub4(position);
								input.m_to.add4( to );
								input.m_filterInfo = obFlag.base;

								hkClosestRayHitCollector collector;

								{
									Physics::CastRayAccess mutex;
									Physics::CPhysicsWorld::Get().GetHavokWorldP()->castRay(input, collector);
								}

								if( collector.hasHit() )
								{
									// probably check if it stairs and if not limit movement if needed by adding extra constraint 
									if( ( collector.getHit().m_normal(1) > 0.1f ) && ( collector.getHit().m_normal(1) <  0.9f ) )
										addMaxSlopePlane( m_maxSlopeCosine, m_up, i, surfaceConstraints);
								} 

#endif
							}
						}
					}
				}

				#if defined DEBUG_CHARACTER_CONTROLLER
				for (int j=0; j < surfaceConstraints.getSize() ; j++)
				{
					HK_DISPLAY_PLANE(surfaceConstraints[j].m_plane, position , 0.5f, (j < m_manifold.getSize()) ? HK_DEBUG_CLEAN_SIMPLEX_MANIFOLD_COLOR : HK_DEBUG_VERTICAL_SIMPLEX_MANIFOLD_COLOR);
				}
				for(int j=0; j < m_manifold.getSize() ; j++)
				{
					hkVector4 a = m_manifold[j].m_contact.getPosition(); 
					hkVector4 b = a; 
					b.add4(m_manifold[j].m_contact.getNormal()); 
					HK_DISPLAY_LINE( a, b, hkColor::CYAN );
				}

				#endif
			}

			// Resize array if it is now too small to accomodate the user planes.
			if (surfaceConstraints.getCapacity() - surfaceConstraints.getSize() < m_userPlanes )
			{
				surfaceConstraints.reserve(surfaceConstraints.getSize() + m_userPlanes);
			}			

			//
			// Solve the simplex
			//
			HK_TIMER_SPLIT_LIST("SlexMove");
			hkSimplexSolverInput input;

			hkLocalArray<hkSurfaceConstraintInteraction> surfaceInteractions(surfaceConstraints.getSize() + m_userPlanes);
			surfaceInteractions.setSizeUnchecked(surfaceConstraints.getSize() + m_userPlanes);
			{
				input.m_constraints = surfaceConstraints.begin();
				input.m_numConstraints = surfaceConstraints.getSize();
				input.m_velocity = m_velocity;
				input.m_deltaTime = remainingTime;
				if( m_velocity.lengthSquared3() == 0.0f )
				{
					input.m_minDeltaTime = 0.0f;
				}
				else 
				{
					input.m_minDeltaTime = 0.5f * (m_keepDistance * hkReal(m_velocity.lengthInverse3()));
				}
				input.m_position.setZero4();
				input.m_upVector = m_up;
				input.m_maxSurfaceVelocity.setAll3( m_maxCharacterSpeedForSolver );

				//
				// Allow the user to do whatever they wish with the surface constraints
				//
				fireConstraintsProcessed( m_manifold, input );

				output.m_planeInteractions = surfaceInteractions.begin();

				// Solve Simplex
				hkSimplexSolverSolve( input, output );
#if 1           
				// We separated movement to the "gravitation" and "animation". 
				// For "animation" movement the velocity should be projected to constraint.
				// - Already done by hkSimplexSolverSolve.
				// For "gravitation" movement move in gravitation direction till surface is reached.
				// - will be done by following code.

				HK_TIMER_SPLIT_LIST("GravityCalc");
				// apply longest shift in gravity dir... 
				float toDoShift = static_cast<hkReal>(worldGravityVelocity.length3()) * output.m_deltaTime;
				if (toDoShift > 0)
				{
					hkVector4 worldGravityDir = worldGravityVelocity;
					worldGravityDir.normalize3();
					
					float doneShift = output.m_position.dot3(worldGravityDir);					
					float extraShift = toDoShift - ((doneShift > 0) ? doneShift : 0);						
					for(int i = 0; extraShift > 0 && i < input.m_numConstraints; i++)
					{
						hkSurfaceConstraintInfo * surface = &(surfaceConstraints[i]);
						float planeGrav = surface->m_plane.dot3(worldGravityDir);
						if (planeGrav < -0.01f)
						{
							float d = static_cast<hkReal>(surface->m_plane.dot3(surface->m_velocity)) * output.m_deltaTime;
							d -= surface->m_plane.dot3(output.m_position);	
							d /= planeGrav;

							if (d < extraShift)
								extraShift = d;
						}
					}

					if (extraShift > 0)
					{
						output.m_position.addMul4(extraShift, worldGravityDir);
						output.m_velocity.addMul4(extraShift / output.m_deltaTime, worldGravityDir);
					}
					
				}
#endif
			}

	#if defined DEBUG_CHARACTER_CONTROLLER 
			{
				int extraConstraints = input.m_numConstraints - surfaceConstraints.getSize();
				// Display user planes at the centre of the character shape
				for (int i = 0; i < extraConstraints; i++)
				{
					HK_DISPLAY_PLANE(input.m_constraints[m_manifold.getSize()+i].m_plane, position , 0.5f, HK_DEBUG_USER_SIMPLEX_MANIFOLD_COLOR);
				}
			}
	#endif

			//
			// Apply forces - e.g. Character hits dynamic objects 
			//
			HK_TIMER_SPLIT_LIST("ApplySurf");
			applySurfaceInteractions( stepInfo, worldGravityVelocity );

			//
			// Check whether we can walk to the new position the simplex has suggested
			// 

			HK_TIMER_SPLIT_LIST("CastMove");
			{
				bool simplexResultOk = displacement.equals3(output.m_position);

				// If the simplex has given an output direction different from the cast guess
				// we re-cast to check we can move there. There is no need to get the start points again.				
				if (!simplexResultOk)
				{					
					if (m_bCurrentVelocityPrediction)
					{
						// we uses velocity prediction, but it failed
						m_bCurrentVelocityPrediction = false; 						
					}
					else
					{
						// if velocity predition would be correct use it
						hkVector4 teorDisplacement; 
						teorDisplacement.setMul4(remainingTime, m_velocity);	
						if (teorDisplacement.equals3(output.m_position))
							m_bCurrentVelocityPrediction = true; 						
					}
										
					castInput.m_to.setAdd4( position, output.m_position);
					HK_ON_DEBUG( HK_DISPLAY_LINE(position, castInput.m_to, hkColor::CYAN) );

					castCollector.reset();
					m_shapePhantom->setPositionAndLinearCast( position, castInput, castCollector, HK_NULL );

					if ( castCollector.hasHit() )
					{
						castCollector.sortHits();
						convertFractionToDistance( castCollector.getHits().begin(), castCollector.getHits().getSize(), output.m_position );

						// Add new point to manifold
						const hkRootCdPoint& surface = castCollector.getHits()[0];
						int index = findSurface(surface);

						//
						// If the new cast hit something we have to check if this is a new
						// surface, or one we have already passed to the simplex
						//

						// Did we already have this plane?
						while (findSurface(castCollector.getHits()[0]) != -1)
						{
							// We found it, so the simplex already took it into account
							// We ignore this point and test the others
							castCollector.getHits().removeAtAndCopy(0);
							if ( !castCollector.hasHit() )
							{
								// There are no more hits left so we can walk to where the simplex wants
								simplexResultOk = true;
								break;
							}
						}

						// Now add that surface
						if (index == -1)
						{
							fireContactAdded( surface );
							m_manifold.pushBack( surface );
						}
					}
					else
					{
						simplexResultOk = true;
					}
				}
				if (simplexResultOk)
				{
					position.add4(output.m_position);
					remainingTime -= output.m_deltaTime; 
				}
				else
				{    					
					remainingTime -= moveToLinearCastHitPosition(output, castCollector, castInput, position);					
				}	
				m_oldVelocity.setMul4(1.0f / output.m_deltaTime, output.m_position);
			}
		}

		// Update with the output velocity from the simplex.
		m_velocity = output.m_velocity;

		// Update the phantom with the new position
		m_shapePhantom->setPosition(position, castInput.m_startPointTolerance);
		// Update history
		m_aobMyLastRealPosition[0] = m_aobMyLastRealPosition[1];
		m_aobMyLastRealPosition[1] = position;
		m_obMyPsuedoPosition = position;

		// If this is our first ever update, give our history values something meaningful to work from
		if (!m_bHasBeenUpdatedOnce)
		{
			m_aobMyLastRealPosition[0] = position;
			m_aobMyLastRealPosition[1] = position;
			m_obMyPsuedoPosition = position;

			static float s_fLastFramesSinceLastUpdate = 0; // Used to initialise m_iFramesSinceLastUpdate so as to stagger updates across frames, rather than do them all at once, thus preventing spikes in performance
			m_obTimeSinceLastUpdate.m_deltaTime = s_fLastFramesSinceLastUpdate;
			s_fLastFramesSinceLastUpdate += 1.0f/30.0f;
			if (s_fLastFramesSinceLastUpdate > m_fUpdateEverySeconds) 
				s_fLastFramesSinceLastUpdate = 0;

			m_bHasBeenUpdatedOnce = true;
		}

		HK_TIMER_END_LIST();
	}
	else // Otherwise, smooth our position out using our last fully calulated position and velocity as a base
	{
		m_obTimeSinceLastUpdate.m_deltaTime += stepInfoParam.m_deltaTime; // Accumulate time
		// We can only smooth out successfully if we have meaningful history values
		if (m_bHasBeenUpdatedOnce)
		{
            hkVector4 obVelocityThisFrame(m_velocity); // Use current valid velocity value
			obVelocityThisFrame.mul4(m_obTimeSinceLastUpdate.m_deltaTime); // Scale it according to how much time has passed
			m_obMyPsuedoPosition.setAdd4(m_aobMyLastRealPosition[1],obVelocityThisFrame); // Use it to move us somewhere
			m_shapePhantom->setPosition(m_obMyPsuedoPosition, m_keepDistance + m_keepContactTolerance); // Set our physical representation
		}
	}
}

// Extract velocity information from the contact point.
void hsCharacterProxy::extractSurfaceVelocity(const hkRootCdPoint& hit, hkVector4& velocityOut) const
{
	velocityOut.setZero4();

	// Grab body information // Body A is always the shapePhantom
	hkRigidBody* body = hkGetRigidBody(hit.m_rootCollidableB);

	if (body)
	{
		// Extract velocity at point on surface
		body->getPointVelocity(hit.m_contact.getPosition(), velocityOut);
	}
}

void hsCharacterProxy::refreshManifold( hkAllCdPointCollector& startPointCollector )
{
	// Update the start point collector
	{
		Physics::WriteAccess mutex;

		startPointCollector.reset();
		hkWorld* world = m_shapePhantom->getWorld();
		HK_ASSERT2(0x54e32e12, world, "Character proxy must be added to world before calling refreshManifold");

		hkProcessCollisionInput* input = world->getCollisionInput();
		hkReal oldTolerance = input->m_tolerance;
		input->m_tolerance += (m_keepDistance + m_keepContactTolerance);
		if( m_shapePhantom )
		{
			if( m_shapePhantom->getWorld() )
			{
				m_shapePhantom->getClosestPoints(startPointCollector);
			}
		}
		input->m_tolerance = oldTolerance;
	}

	// Update the manifold so that it is correct, so that checkSupport() returns the correct answer.
	hkAllCdPointCollector castCollector;
	updateManifold(startPointCollector, castCollector );
}

void hsCharacterProxy::checkSupport(const hkVector4& direction, hkSurfaceInfo& ground)
{
	hkAllCdPointCollector startPointCollector;
	checkSupportWithCollector(direction, ground, startPointCollector);
}

void hsCharacterProxy::checkSupportWithCollector(const hkVector4& direction, hkSurfaceInfo& ground, hkAllCdPointCollector& startPointCollector)
{
	HK_ASSERT2(0x79d57ec9,  hkMath::equal(direction.length3(), 1.0f), "checkSupport Direction should be normalized");
	HK_TIMER_BEGIN("checkSupport", HK_NULL);
	if (m_refreshManifoldInCheckSupport)
	{
		refreshManifold( startPointCollector );
	}

	hkLocalArray<hkSurfaceConstraintInfo> constraints(m_manifold.getSize() + m_userPlanes + 10);
	constraints.setSizeUnchecked(m_manifold.getSize());
	m_penetrationRecoverySpeed = 20;
	{
		for (int i=0; i < m_manifold.getSize() ; i++)
		{	
			extractSurfaceConstraintInfo(m_manifold[i], constraints[i], 0);

			const hkReal surfaceVerticalComponent = constraints[i].m_plane.dot3(m_up);
			if ( surfaceVerticalComponent > 0.01f && surfaceVerticalComponent < m_maxSlopeCosine )
			{
				hkVector4 a = m_manifold[i].m_contact.getPosition(); 
				hkVector4 b = this->getPosition(); 

				float diffY = a(1) - b(1);

				if( diffY > LOW_WALL_ANGLE_LIMT )
				{
					addMaxSlopePlane( m_maxSlopeCosine, m_up, i, constraints);
				} else {
#if 1
					hkRigidBody* bd = hkGetRigidBody( m_manifold[i].m_rootCollidableA );					
					if (!bd)
						bd = hkGetRigidBody( m_manifold[i].m_rootCollidableB );

					if(! bd)// && bd->getMotionType() == hkMotion::MOTION_FIXED )						
						continue;

					// the following code, verifies if object has geometry like stairs if yes do not limit movement
					Physics::EntityCollisionFlag info;
					info.base = bd->getCollidable()->getCollisionFilterInfo();
					if ((info.flags.i_am & Physics::SMALL_INTERACTABLE_BIT) == 0 && (info.flags.i_am & Physics::LARGE_INTERACTABLE_BIT) == 0)
						continue;

					// perform raycast only on the shape from colliding body (it is faster than on the world)
					const hkShape * shape = bd->getCollidable()->getShape();

					Physics::RaycastCollisionFlag obFlag; obFlag.base = 0;
					obFlag.flags.i_am = Physics::COLLISION_ENVIRONMENT_BIT;
					obFlag.flags.i_collide_with = (	Physics::SMALL_INTERACTABLE_BIT				|
						Physics::LARGE_INTERACTABLE_BIT				);

					hkShapeRayCastInput input;
					input.m_from	= this->getPosition();
					input.m_to		= m_manifold[i].m_contact.getPosition();


					hkVector4 to = m_manifold[i].m_contact.getPosition(); 
					to.sub4(this->getPosition());
					input.m_to.add4( to );						

					// all position must be in shape local coord 													
					input.m_from.setTransformedInversePos(bd->getTransform(), input.m_from);
					input.m_to.setTransformedInversePos(bd->getTransform(), input.m_to);
					input.m_filterInfo = obFlag.base;

					hkShapeRayCastOutput  collector;
					{
						Physics::CastRayAccess mutex;
						shape->castRay(input, collector);
					}

					if( collector.hasHit() )
					{
						collector.m_normal.setRotatedDir(bd->getTransform().getRotation(), collector.m_normal);

						// probably check if it stairs and if not limit movement if needed by adding extra constraint 
						if( ( collector.m_normal(1) > 0.1f ) && ( collector.m_normal(1) <  0.9f ) )
							addMaxSlopePlane( m_maxSlopeCosine, m_up, i, constraints);
					}							
#else                   
					Physics::RaycastCollisionFlag obFlag; obFlag.base = 0;
					obFlag.flags.i_am = Physics::COLLISION_ENVIRONMENT_BIT;
					obFlag.flags.i_collide_with = (	Physics::SMALL_INTERACTABLE_BIT				|
						Physics::LARGE_INTERACTABLE_BIT				);

					hkWorldRayCastInput input;

					input.m_from	= this->getPosition();
					input.m_to		= m_manifold[i].m_contact.getPosition();


					hkVector4 to = m_manifold[i].m_contact.getPosition(); 
					to.sub4(this->getPosition());
					input.m_to.add4( to );
					input.m_filterInfo = obFlag.base;

					hkClosestRayHitCollector collector;

					{
						Physics::CastRayAccess mutex;
						Physics::CPhysicsWorld::Get().GetHavokWorldP()->castRay(input, collector);
					}

					if( collector.hasHit() )
					{
						if( ( collector.getHit().m_normal(1) > 0.1f ) && ( collector.getHit().m_normal(1) <  0.9f ) )
							addMaxSlopePlane( m_maxSlopeCosine, m_up, i, constraints);
					} 
#endif
				}
			}
		}
	}
	// Resize array if it is now too small to accomodate the user planes.
	if (constraints.getCapacity() - constraints.getSize() < m_userPlanes )
	{
		constraints.reserve(constraints.getSize() + m_userPlanes);
	}

	// Interactions array - this is the output of the simplex solver
	hkLocalArray<hkSurfaceConstraintInteraction> interactions(constraints.getSize() + m_userPlanes);

	// Stored velocities - used to remember surface velocities to give the correct output surface velocity
	hkLocalArray<hkVector4> storedVelocities( constraints.getSize() + m_userPlanes );

	//
	//	Test Direction
	//
	hkSimplexSolverInput input;
	hkSimplexSolverOutput output;
	{
		input.m_position.setZero4();
		input.m_constraints = constraints.begin();
		input.m_numConstraints = constraints.getSize();
		input.m_velocity = direction;
		input.m_deltaTime = 1.0f / 60.0f; 
		input.m_minDeltaTime = 1.0f / 60.0f;
		input.m_upVector = m_up;

		input.m_maxSurfaceVelocity.setAll( m_maxCharacterSpeedForSolver );
		output.m_planeInteractions = interactions.begin();

		//
		// Allow the user to do whatever they wish with the surface constraints
		//
		fireConstraintsProcessed( m_manifold, input );

		// Set the sizes of the arrays to be correct
		storedVelocities.setSize(input.m_numConstraints);
		interactions.setSize(input.m_numConstraints);
		constraints.setSize(input.m_numConstraints);

		// Remove velocities and friction to make this a query of the static geometry
		for (int i = 0; i < input.m_numConstraints; ++i )
		{
			storedVelocities[i] = constraints[i].m_velocity;
			constraints[i].m_velocity.setZero4();
		}

		hkSimplexSolverSolve( input, output );
	}

	ground.m_surfaceVelocity.setZero4();
	ground.m_surfaceNormal.setZero4();

	if ( output.m_velocity.equals3( direction ) )
	{
		ground.m_supportedState = hkSurfaceInfo::UNSUPPORTED;
	}
	else 
	{
		if ( output.m_velocity.lengthSquared3() < .001f )
		{
			ground.m_supportedState = hkSurfaceInfo::SUPPORTED;
		}
		else
		{
			output.m_velocity.normalize3();
			hkReal angleSin = output.m_velocity.dot3(direction);
			
			hkReal cosSqr = 1 - angleSin * angleSin;

			if (cosSqr < m_maxSlopeCosine * m_maxSlopeCosine )
			{
				ground.m_supportedState = hkSurfaceInfo::SLIDING;
			}
			else
			{
				ground.m_supportedState = hkSurfaceInfo::SUPPORTED;
			}
		}
	}

	if ( ground.m_supportedState != hkSurfaceInfo::UNSUPPORTED )
	{
		int numTouching = 0;

		for (int i=0; i < input.m_numConstraints; i++)
		{
			// If we touched this plane and it supports our direction
			if ((interactions[i].m_touched) && hkReal(constraints[i].m_plane.dot3(direction)) < -0.01f)
			{
				ground.m_surfaceNormal.add4( constraints[i].m_plane );
				ground.m_surfaceVelocity.add4( storedVelocities[i] );
				numTouching++;
			}
		}

		#ifdef DEBUG_CHARACTER_CONTROLLER
			HK_DISPLAY_ARROW(getPosition(),ground.m_surfaceNormal, 0xffffffff);
		#endif

		if (numTouching > 0)
		{
			ground.m_surfaceNormal.normalize3();
			ground.m_surfaceVelocity.mul4(1.f / numTouching);
		}
		else
		{
			ground.m_supportedState = hkSurfaceInfo::UNSUPPORTED;
		}
	}

	// if we are standing only on charcters, always slide down... 
	if (ground.m_supportedState == hkSurfaceInfo::SUPPORTED)
	{
		for (int i=0; i < input.m_numConstraints; i++)
		{
			// If we touched this plane and its not phantom return..
			if (interactions[i].m_touched)
			{
				Physics::EntityCollisionFlag info;
				info.base = m_manifold[i].m_rootCollidableA->getCollisionFilterInfo();
				if (!(info.flags.i_am & (Physics::CHARACTER_CONTROLLER_PLAYER_BIT | Physics::CHARACTER_CONTROLLER_ENEMY_BIT)))
				{
					HK_TIMER_END();
					return; 
				}

				info.base = m_manifold[i].m_rootCollidableB->getCollisionFilterInfo();
				if (!(info.flags.i_am & (Physics::CHARACTER_CONTROLLER_PLAYER_BIT | Physics::CHARACTER_CONTROLLER_ENEMY_BIT)))
				{
					HK_TIMER_END();
					return; 
				}
			}
		}

		ground.m_supportedState = hkSurfaceInfo::SLIDING;
	}

	HK_TIMER_END();
}


// Check and see if the character is supported in the give direction
void hsCharacterProxy::checkSupportDeprecated(const hkVector4& direction, hkSurfaceInfoDeprecated& ground) const
{
	HK_ASSERT2(0x79d57ec9,  hkMath::equal(direction.length3(), 1.0f), "checkSupport Direction should be normalized");
	HK_TIMER_BEGIN("checkSupport", HK_NULL);

	// We zero the velocity of all the planes, making this call a static geometric query
	hkLocalArray<hkSurfaceConstraintInfo> constraints(m_manifold.getSize() + m_userPlanes + 10);
	constraints.setSizeUnchecked(m_manifold.getSize());
	{
		for (int i=0; i < m_manifold.getSize() ; i++)
		{
			extractSurfaceConstraintInfo(m_manifold[i], constraints[i], 0);
			hkVector4 a = m_manifold[i].m_contact.getPosition(); 
			hkVector4 b = this->getPosition(); 
			
			float diffY = a(1) - b(1);
	
			if( diffY > LOW_WALL_ANGLE_LIMT )
			{
				addMaxSlopePlane( m_maxSlopeCosine, m_up, i, constraints);
			} else {
#if 1
				hkRigidBody* bd = hkGetRigidBody( m_manifold[i].m_rootCollidableA );					
			    if (!bd)
					bd = hkGetRigidBody( m_manifold[i].m_rootCollidableB );

				if(! bd)// && bd->getMotionType() == hkMotion::MOTION_FIXED )						
					continue;
					
				// the following code, verifies if object has geometry like stairs if yes do not limit movement
				Physics::EntityCollisionFlag info;
				info.base = bd->getCollidable()->getCollisionFilterInfo();
				if ((info.flags.i_am & Physics::SMALL_INTERACTABLE_BIT) == 0 && (info.flags.i_am & Physics::LARGE_INTERACTABLE_BIT) == 0)
					continue;

				// perform raycast only on the shape from colliding body (it is faster than on the world)
				const hkShape * shape = bd->getCollidable()->getShape();

				Physics::RaycastCollisionFlag obFlag; obFlag.base = 0;
				obFlag.flags.i_am = Physics::COLLISION_ENVIRONMENT_BIT;
				obFlag.flags.i_collide_with = (	Physics::SMALL_INTERACTABLE_BIT				|
					Physics::LARGE_INTERACTABLE_BIT				);

				hkShapeRayCastInput input;
				input.m_from	= this->getPosition();
				input.m_to		= m_manifold[i].m_contact.getPosition();


				hkVector4 to = m_manifold[i].m_contact.getPosition(); 
				to.sub4(this->getPosition());
				input.m_to.add4( to );						

				// all position must be in shape local coord 													
				input.m_from.setTransformedInversePos(bd->getTransform(), input.m_from);
				input.m_to.setTransformedInversePos(bd->getTransform(), input.m_to);
				input.m_filterInfo = obFlag.base;

				hkShapeRayCastOutput  collector;
				{
					Physics::CastRayAccess mutex;
					shape->castRay(input, collector);
				}

				if( collector.hasHit() )
				{
					collector.m_normal.setRotatedDir(bd->getTransform().getRotation(), collector.m_normal);

					// probably check if it stairs and if not limit movement if needed by adding extra constraint 
					if( ( collector.m_normal(1) > 0.1f ) && ( collector.m_normal(1) <  0.9f ) )
						addMaxSlopePlane( m_maxSlopeCosine, m_up, i, constraints);
				}							
#else 
				Physics::RaycastCollisionFlag obFlag; obFlag.base = 0;
				obFlag.flags.i_am = Physics::COLLISION_ENVIRONMENT_BIT;
				obFlag.flags.i_collide_with = (	Physics::SMALL_INTERACTABLE_BIT				|
												Physics::LARGE_INTERACTABLE_BIT				);

				hkWorldRayCastInput input;

				input.m_from	= this->getPosition();
				input.m_to		= m_manifold[i].m_contact.getPosition();
				

				hkVector4 to = m_manifold[i].m_contact.getPosition(); 
				to.sub4(this->getPosition());
				input.m_to.add4( to );
				input.m_filterInfo = obFlag.base;

				hkClosestRayHitCollector collector;

				{
					Physics::CastRayAccess mutex;
				Physics::CPhysicsWorld::Get().GetHavokWorldP()->castRay(input, collector);
				}

				if( collector.hasHit() )
				{
					if( ( collector.getHit().m_normal(1) > 0.1f ) && ( collector.getHit().m_normal(1) <  0.9f ) )
						addMaxSlopePlane( m_maxSlopeCosine, m_up, i, constraints);
				} 
#endif
			}
		}
	}

	// Resize array if it is now too small to accomodate the user planes.
	if (constraints.getCapacity() - constraints.getSize() < m_userPlanes )
	{
		constraints.reserve(constraints.getSize() + m_userPlanes);
	}

	// Magnitude of our velocity in the current direction
	// ToDo: Work with our current velocity
	hkLocalArray<hkSurfaceConstraintInteraction> interactions(constraints.getSize() + m_userPlanes);
	interactions.setSize(constraints.getSize() + m_userPlanes);


	//
	//	Test Direction
	//
	hkSimplexSolverInput input;
	hkSimplexSolverOutput output;
	{
		input.m_position.setZero4();
		input.m_constraints = constraints.begin();
		input.m_numConstraints = m_manifold.getSize();
		input.m_velocity.setAdd4(m_velocity,direction);
		input.m_deltaTime = 1.0f / 60.0f; 
		input.m_minDeltaTime = 1.0f / 60.0f;
		input.m_upVector = m_up;

		input.m_maxSurfaceVelocity.setAll( m_maxCharacterSpeedForSolver );
		output.m_planeInteractions = interactions.begin();

		//
		// Allow the user to do whatever they wish with the surface constraints
		//
		fireConstraintsProcessed( m_manifold, input );

		hkSimplexSolverSolve( input, output );
	}

	output.m_velocity.sub4(m_velocity);

	hkReal projectedVelocity = output.m_velocity.dot3(direction);

	// If our velocity was reduced then we hit something 
	// HVK-2402
	ground.m_isSupported = projectedVelocity < .99f;

	hkVector4 resultant;
	resultant.setAddMul4(output.m_velocity, direction, -projectedVelocity);
	ground.m_isSliding = resultant.lengthSquared3() > 0.01f;

	ground.m_surfaceVelocity.setZero4();
	ground.m_surfaceNormal.setZero4();

	int numTouching = 0;

	for (int i=0; i < input.m_numConstraints; i++)
	{
		// If we touched this plane and it supports our direction
		if ((interactions[i].m_touched) && hkReal(constraints[i].m_plane.dot3(direction)) < -0.08f)
		{
			ground.m_surfaceNormal.add4( constraints[i].m_plane );
			ground.m_surfaceVelocity.add4( constraints[i].m_velocity );
			numTouching++;
		}
	}

	if (numTouching > 0)
	{
		ground.m_surfaceNormal.normalize3();
		ground.m_surfaceVelocity.mul4(1.f / numTouching);
	}
	else
	{
		ground.m_isSupported = false;
	}

	HK_TIMER_END();
}

/// Extract the information from a contact point returned by the linear caster
/// This information is translated into surface constraint information.
/// This is the bridge between the collision detector and the character controller.
void hsCharacterProxy::extractSurfaceConstraintInfo(const hkRootCdPoint& hit, hkSurfaceConstraintInfo& surface, hkReal timeTravelled) const
{
	surface.m_plane = hit.m_contact.getSeparatingNormal();

	// Contract the planes by the keep distance
	surface.m_plane(3) -= m_keepDistance; 

	surface.m_staticFriction = m_staticFriction;
	surface.m_dynamicFriction = m_dynamicFriction;
	surface.m_extraUpStaticFriction = m_extraUpStaticFriction;
	surface.m_extraDownStaticFriction = m_extraDownStaticFriction; 

	// Assume the velocity of this surface is 0
	surface.m_velocity.setZero4();

	// Assume this is a low priority surface
	surface.m_priority = 0;

	// Grab body information
	hkRigidBody* body = hkGetRigidBody(hit.m_rootCollidableB);
	if (body)
	{
		/* We are intergrating characters after the all rigid bodies have been moved. 
		   So surfaces are in new positions and there is no need to move them according theirs
		   velocities. We need to place characters exactly on position where they are now. 
		   peterFe 06.09.2006
		   
		// Extract point velocity

		// HVK-1871. This code gets the point velocity at the collision, based on how far
		// the object actually travelled, rather than the velocity result of the constraint solver.
		// (i.e. the value got from getPointVelocity)
		// When a heavy force pushes a rigid body into a fixed rigid body these values can diverge,
		// which can cause the character controller to penetrate the moving rigid body, as it sees
		// an incorrectly moving plane.

		// Note, this means that velocities will be one frame behind, so for accelerating platforms
		// (HVK-1477) (i.e. For keyframed or fixed objects) we just take the velocity, to make sure the
		// character does not sink in.
		if (body->isFixedOrKeyframed())
		{
			body->getPointVelocity(hit.m_contact.getPosition(), surface.m_velocity);		
		}
		else
		{
			hkVector4 linVel;
			hkVector4 angVel;
			hkSweptTransformUtil::getVelocity(*body->getRigidMotion()->getMotionState(), linVel, angVel);
			hkVector4 relPos; relPos.setSub4( hit.m_contact.getPosition(), body->getCenterOfMassInWorld() );
			surface.m_velocity.setCross( angVel, relPos);
			surface.m_velocity.add4( linVel );
		}


		// Move the plane by the velocity, based on the timeTravelled HVK-1477
		surface.m_plane(3) -= static_cast<hkReal>(surface.m_velocity.dot3(surface.m_plane)) * timeTravelled;
        */   

		// Extract priority
		// - Static objects have highest priority
		// - Then keyframed
		// - Then dynamic
		if (body->getMotionType() == hkMotion::MOTION_FIXED)
		{
			// Increase the priority
			surface.m_priority = 2;
		}

		if (body->getMotionType() == hkMotion::MOTION_KEYFRAMED)
		{
			// Increase the priority
			surface.m_priority = 1;
		}
	}

	// Set player to zero... 
	if ( surface.m_plane(3) < m_keepContactTolerance)//-hkMath::HK_REAL_EPSILON)
	{
#if HAVOK_SDK_VERSION_MAJOR >= 4 && HAVOK_SDK_VERSION_MINOR >= 1
		if (surface.m_plane(3) < -HK_REAL_EPSILON)
#else
		if (surface.m_plane(3) < -hkMath::HK_REAL_EPSILON)
#endif 
		{
			HK_ON_DEBUG( HK_DISPLAY_ARROW(hit.m_contact.getPosition(),  hit.m_contact.getNormal(), hkColor::RED) );
		}

		surface.m_velocity.addMul4(-surface.m_plane(3) * m_penetrationRecoverySpeed, hit.m_contact.getNormal());	
		surface.m_plane(3) = 0.f;
	}

}


// Computes a metric to allow us to compare points for similarity.
//	Metric is based on:
//    - Angle between normals
//    - Contact Point Distances
//	  - Velocity
inline hkReal hsCharacterProxy::surfaceDistance(const hkRootCdPoint& p1, const hkRootCdPoint& p2) const
{
	// Small angle approx (changed from cross product HVK-2184)
	hkReal angleSquared = 1 - static_cast<hkReal>(p1.m_contact.getNormal().dot3(p2.m_contact.getNormal())); 
	angleSquared *= m_contactAngleSensitivity * m_contactAngleSensitivity;

	hkReal planeDistanceSqrd = p1.m_contact.getDistance() - p2.m_contact.getDistance();
	planeDistanceSqrd *= planeDistanceSqrd;

	hkVector4 p1Vel, p2Vel;
	extractSurfaceVelocity(p1, p1Vel);
	extractSurfaceVelocity(p2, p2Vel);

	hkVector4 velDiff; velDiff.setSub4( p1Vel, p2Vel );
	hkReal velocityDiffSqrd = velDiff.lengthSquared3();

	return angleSquared * 10.0f + velocityDiffSqrd * 0.1f + planeDistanceSqrd;
}

// Maintain the manifold of plane equations we pass on to the simple solver
// This forms the bridge between the collision detector and the simplex solver
//
// Manifold Rules:
//	- All moving planes found at start are kept.
// - All penetrating planes found at start are kept.
// - Cast Plane is always kept.
// - All other currently held planes must verify they are present in the start collector.
void hsCharacterProxy::updateManifold(const hkAllCdPointCollector& startPointCollector, const hkAllCdPointCollector& castCollector )
{
	Physics::WriteAccess mutex;	

	// Remove listener from all bodies and phantoms
	{
		for (int i=0 ; i < m_bodies.getSize(); i++)
		{
			m_bodies[i]->removeEntityListener(this);
		}
		m_bodies.clear();

		for (int j=0; j< m_phantoms.getSize(); j++)
		{
			m_phantoms[j]->removePhantomListener(this);
		}
		m_phantoms.clear();
	}


	// This is used to add the closest point always to the manifold.
	//hkReal minimumPenetration = hkMath::HK_REAL_MAX;
#if 1
	// simplified version take all contacts from startCollector. 
	// reporting of contacts fireContactRemoved & fireContactAdded take the most of time
	// of startPointCollector handling
	for (int i=0; i< m_manifold.getSize(); i++) 
	{
		fireContactRemoved(m_manifold[i]);
	}

	for (int i=0; i< startPointCollector.getHits().getSize(); i++) 
	{
		fireContactAdded(startPointCollector.getHits()[i]);
	}
	
	m_manifold = startPointCollector.getHits();

#else
	//
	// Copy hits from start point collector
	//
	hkLocalArray<hkRootCdPoint> startPointHits(startPointCollector.getHits().getSize());
	startPointHits.setSizeUnchecked(startPointCollector.getHits().getSize());

	for (int i=0; i< startPointHits.getSize(); i++) 
	{
		startPointHits[i] = startPointCollector.getHits()[i];

		// Before: only the most penetrating point from startPointHits was added into the manifold.
		// But it leads to bug where thown barrels (and other items) did not collide with not moving characters 
		// (because the collision points in startPointCollector was ignored)
		// Now: Add all points from startPointCollector	into manifold. 

		//if (startPointHits[i].m_contact.getDistance() < minimumPenetration && hkGetRigidBody(startPointHits[i].m_rootCollidableB) && hkGetRigidBody(startPointHits[i].m_rootCollidableB)->isFixedOrKeyframed())
		//{
		//	minimumPenetration = startPointHits[i].m_contact.getDistance();
		//}
	}

	//
	//	For each existing point in the manifold - 
	//		find it in the start point collector
	//		if found, copy the start point collector point over the manifold point
	//		otherwise drop the manifold point
	//
	{
		for (int s = m_manifold.getSize() - 1; s >= 0; s--)
		{
			int bestIndex = -1;
			hkReal minDistance = 1.1f;
			hkRootCdPoint& current = m_manifold[s];

			// Find the best match for this plane
			for (int h=0; h < startPointHits.getSize(); h++)
			{
				hkRootCdPoint& surface = startPointHits[h];

				hkReal distance = surfaceDistance( surface, current ); 
				if ( distance < minDistance )
				{
					minDistance = distance;
					bestIndex = h;
				}
			}

			// Plane already exists in manifold so we update and remove from the collector
			if ( bestIndex >= 0 )
			{
				hkRootCdPoint& surface = startPointHits[bestIndex];
				if (surface.m_rootCollidableB != current.m_rootCollidableB)
				{
					fireContactRemoved(current);
					fireContactAdded(surface);
				}

				current = surface;
				startPointHits.removeAt( bestIndex );
			}
			else
			{
				//
				//	No matching plan in start point collector - we remove this from the manifold
				//
				fireContactRemoved( m_manifold[s] );
				m_manifold.removeAt(s);
			}
		}
	}

	// Before:
	// Add the most penetrating point from the start point collector if it is still in the
	// collector (i.e. if it is not already in the manifold). This is safe, as the closest
	// point can never be an unwanted edge.
	// Now:
	// But we want all points in manifold. If we let there only the most penetrating one. Some collision are ignored 
	// (for example collision with the thrown barrels for standing characters).

	{
		for (int h=0; h < startPointHits.getSize(); h++)
		{			
			//if ( (startPointHits[h].m_contact.getDistance() ) == minimumPenetration )
			{
				//
				// Find existing plane
				//
				int index = findSurface( startPointHits[h] );
				if ( index < 0 )
				{
					fireContactAdded( startPointHits[h] );
					m_manifold.pushBack( startPointHits[h] );
				}
			}
		}
	}
#endif
	//
	// Add castCollector plane
	//
	if (castCollector.hasHit())
	{
		const hkRootCdPoint& surface = castCollector.getHits()[0];
		int index = findSurface(surface);
		if (index == -1)
		{
			fireContactAdded( surface );
			m_manifold.pushBack( surface );
		}

		// NOTE: We used to update the manifold with the new point from the cast collector here, but in fact this
		// is unnecessary and sometimes wrong. All points in the manifold have been correctly updated at this stage
		// by the start point collector so they do not need to be replaced here. If the points are penetrating, then
		// the cast collector will have a distance of 0, which is incorrect, and the negative distance picked up by
		// the start collector is the one that we want.
	}

	//
	// Cross check the manifold for clean planes
	// The simplex does not handle parallel planes
	//
	{
		for (int p1 = m_manifold.getSize()-1; p1 > 0; p1--)
		{
			hkBool remove = false;
			for (int p2 = p1-1; p2 >= 0; p2--)
			{
				// If p1 and p2 are the same then we should remove p1
				const hkReal minDistance = 0.1f;
				hkReal distance = surfaceDistance( m_manifold[p1], m_manifold[p2] );
				if ( distance < minDistance )
				{
					remove = true;
					break;
				}
			}
			if ( remove )
			{
				fireContactRemoved( m_manifold[p1] );
				m_manifold.removeAt( p1 );
			}
		}
	}

	// add entity listener to bodies in the manifold
	{
		for (int i=0 ; i < m_manifold.getSize(); i++)
		{
			hkRigidBody* body =  hkGetRigidBody(m_manifold[i].m_rootCollidableB);
			if ( body && ( m_bodies.indexOf(body) == -1 ) )
			{
				body->addEntityListener(this);
				m_bodies.pushBack(body);
			}

			hkPhantom* phantom = hkGetPhantom( m_manifold[i].m_rootCollidableB );
			if (phantom && (m_phantoms.indexOf(phantom) == -1) )
			{
				phantom->addPhantomListener( this );
				m_phantoms.pushBack(phantom);
			}
		}
	}
}


void hsCharacterProxy::applySurfaceInteractions( const hkStepInfo& stepInfo, const hkVector4& worldGravityVelocity )
{
	hkCharacterObjectInteractionEvent input;
	input.m_timestep = stepInfo.m_deltaTime;

	for (int s=0; s < m_manifold.getSize(); s++)
	{
		hkWorldObject* object = hkGetWorldObject(m_manifold[s].m_rootCollidableB);

		//
		// Check if we have hit another character
		//
		if (object->hasProperty(HK_PROPERTY_CHARACTER_PROXY))
		{
			hkPropertyValue val = object->getProperty(HK_PROPERTY_CHARACTER_PROXY);

			hsCharacterProxy* otherChar = reinterpret_cast<hsCharacterProxy*>( val.getPtr()) ;

			//
			// Callback to indicate characters have collided
			//
			{
				for ( int i = m_listeners.getSize()-1; i >= 0; i-- )
				{
					if (m_listeners[i] != HK_NULL)
					{
						m_listeners[i]->characterInteractionCallback( this, otherChar, m_manifold[s].m_contact );
					}
				}
			}
		}

		hkRigidBody* body = hkGetRigidBody(m_manifold[s].m_rootCollidableB);

		//
		// Check if we have hit a rigid body
		//
		if ( ( body != HK_NULL ) && ( !body->isFixedOrKeyframed() ) )
		{

			//
			//	Some constants used
			//
			const hkReal recoveryTau = 0.4f;
			const hkReal dampFactor = 0.9f;
			input.m_position = m_manifold[s].m_contact.getPosition();
			input.m_position(3) = m_manifold[s].m_contact.getDistance();
			input.m_normal  = m_manifold[s].m_contact.getNormal();
			input.m_body = body;


			//
			//	Calculate required velocity change
			//
			hkReal deltaVelocity;
			{
				hkVector4 pointVel; body->getPointVelocity( input.m_position, pointVel );
				pointVel.sub4( m_velocity );

				input.m_projectedVelocity = pointVel.dot3( input.m_normal );

				deltaVelocity = - input.m_projectedVelocity * dampFactor;

				// Only apply an extra impulse if the collision is actually penetrating. HVK-1903
				if ( input.m_position(3) < 0)
				{
					deltaVelocity += input.m_position(3) * stepInfo.m_invDeltaTime * recoveryTau;
				}
			}

			//
			// Initialize the output result
			//
			hkCharacterObjectInteractionResult output;
			output.m_impulsePosition = input.m_position;
			output.m_objectImpulse.setZero4();

			if (deltaVelocity < 0.0f)
			{
				//
				//	Calculate the impulse required
				//
				{
					hkMatrix3 inertiaInv;
					body->getInertiaInvWorld(inertiaInv);

					hkVector4 r; r.setSub4( input.m_position, body->getCenterOfMassInWorld() );
					hkVector4 jacAng; jacAng.setCross( r, input.m_normal );
					hkVector4 rc; rc.setMul3( inertiaInv, jacAng );

					input.m_objectMassInv = rc.dot3( jacAng );
					input.m_objectMassInv += body->getMassInv();
					input.m_objectImpulse = deltaVelocity / input.m_objectMassInv;
				}

				hkReal maxPushImpulse = - m_characterStrength * stepInfo.m_deltaTime;
				if (input.m_objectImpulse < maxPushImpulse)
				{
					input.m_objectImpulse = maxPushImpulse;
				}

				output.m_objectImpulse.setMul4( input.m_objectImpulse, input.m_normal );
			}
			else
			{
				input.m_objectImpulse = 0.0f;
				input.m_objectMassInv = body->getMassInv();
			}


			// Add gravity
			{
				hkVector4 charVelDown = worldGravityVelocity; //charVelDown.setMul4(stepInfo.m_deltaTime, worldGravity);

				// Normal points from object to character
				hkReal relVel = charVelDown.dot3(input.m_normal);

				if (input.m_projectedVelocity < 0 ) // if objects are separating
				{
					relVel -= input.m_projectedVelocity;
				}
#if HAVOK_SDK_VERSION_MAJOR >= 4 && HAVOK_SDK_VERSION_MINOR >= 1
				if (relVel < -HK_REAL_EPSILON)
#else
				if (relVel < -hkMath::HK_REAL_EPSILON)
#endif 
				{
					output.m_objectImpulse.addMul4(relVel * m_characterMass, input.m_normal);
				}
			}




			//
			// Callback to allow user to change impulse + use the info / play sounds
			//
			{
				for ( int i = m_listeners.getSize()-1; i >= 0; i-- )
				{
					if (m_listeners[i] != HK_NULL)
					{
						m_listeners[i]->objectInteractionCallback( this, input, output );
					}
				}
			}

			//
			//	Apply impulse based on callback result
			//
			{
				if( body->isAddedToWorld() )
					body->applyPointImpulse( output.m_objectImpulse, output.m_impulsePosition );

				HK_ON_DEBUG( HK_DISPLAY_ARROW(input.m_position, output.m_objectImpulse, 0xffffffff) );
			}
		}
	}
}

const hkVector4& hsCharacterProxy::getPosition() const
{
	Physics::ReadAccess mutex;
	return m_shapePhantom->getCollidable()->getTransform().getTranslation();
}


void hsCharacterProxy::setPosition(const hkVector4& position, bool bResetInterpolation)
{
	// Tolerance should be the same as the castInput.m_startPointTolerance used in integrateWithCollectors
	Physics::WriteAccess mutex;
	m_shapePhantom->setPosition(position, m_keepDistance + m_keepContactTolerance);
	
	if (!m_bHasBeenUpdatedOnce || bResetInterpolation)
	{
		m_aobMyLastRealPosition[0] = position;
		m_aobMyLastRealPosition[1] = position;
		m_obMyPsuedoPosition = position;
		m_velocity.setZero4();
	}
}

void hsCharacterProxy::addCharacterProxyContactListener(hsCharacterProxyContactListener* cpl)
{
	HK_ASSERT2(0x5efeeea3, m_contactListeners.indexOf( cpl ) < 0, "You tried to add  a character proxy listener listener twice" );
	m_contactListeners.pushBack( cpl );
}

void hsCharacterProxy::removeCharacterProxyContactListener(hsCharacterProxyContactListener* cpl)
{
	int i = m_contactListeners.indexOf( cpl );
	HK_ASSERT2(0x2c6b3925, i >= 0, "You tried to remove a character proxy listener, which was never added" );
	m_contactListeners.removeAt(i);
}

void hsCharacterProxy::fireContactsFound(hkAllCdPointCollector& castCollector, hkAllCdPointCollector& startPointCollector) const
{
	for ( int i = m_contactListeners.getSize()-1; i >= 0; i-- )
	{
		m_contactListeners[i]->contactsFoundCallback( castCollector, startPointCollector );
	}
}



#endif
