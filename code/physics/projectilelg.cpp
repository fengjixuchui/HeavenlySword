//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/staticlg.cpp
//!	
//!	DYNAMICS COMPONENT:	
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.08.01
//!
//---------------------------------------------------------------------------------------------------------

#include "config.h"

#include "projectilelg.h"

#include "element.h"
#include "behavior.h"
#include "rigidbody.h"
#include "maths_tools.h"
#include "physics/world.h"
#include "physics/advancedcharactercontroller.h"
#include "physics/system.h"
#include "physics/physicstools.h"
#include "physics/physicsloader.h"


#include "camera/camutils.h"
#include "anim/hierarchy.h"

#include "game/entity.h"
#include "game/entity.inl"
#include "game/randmanager.h"
#include "game/luaattrtable.h"
#include "game/luaglobal.h"
#include "game/query.h"
#include "game/messagehandler.h"
#include "game/renderablecomponent.h"
#include "game/interactioncomponent.h"
#include "game/entityprojectile.h"

#include "objectdatabase/dataobject.h"
#include "objectdatabase/objectdatabase.h"

#include "physics/havokincludes.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include <hkdynamics/phantom/hkSimpleShapePhantom.h>
#include <hkdynamics/world/hkWorld.h>
#include <hkcollide/shape/capsule/hkCapsuleShape.h>
#include <hkcollide/collector/pointcollector/hkClosestCdPointCollector.h>
#include <hkdynamics/phantom/hkAabbPhantom.h>

#endif

#include "game/entityprojectile.h"
#include "core/visualdebugger.h"

START_STD_INTERFACE	(ProjectileProperties)
	IENUM		(ProjectileProperties, MotionType, PROJECTILE_MOTION_TYPE )
	PUBLISH_VAR_AS	(m_fInitialSpeed, InitialSpeed)
	PUBLISH_VAR_AS	(m_fAcceleration, Acceleration)
	PUBLISH_VAR_AS	(m_fAccelerationTime, AccelerationTime)
	PUBLISH_VAR_AS	(m_fRicochetProbability, RicochetProbability)
	PUBLISH_VAR_AS	(m_fRicochetEnergyLoss, RicochetEnergyLoss)
	PUBLISH_VAR_AS	(m_fMass, Mass)
	PUBLISH_VAR_AS	(m_fSplineInterval, SplineInterval)
	PUBLISH_VAR_AS	(m_fSplineInitialRadius, SplineInitialRadius)
	PUBLISH_VAR_AS	(m_fSplineMaxRadius, SplineMaxRadius)
	PUBLISH_VAR_AS	(m_fSplineRadiusIncreaseRate, SplineRadiusIncreaseRate)
	PUBLISH_VAR_AS	(m_fGravity, Gravity)
	PUBLISH_VAR_AS	(m_bNoCollide, NoCollide)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_CollideWithCharacters, ProjectileProperties::COLLIDE_INEXACT, CollideWithCharacters)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_bCollideWithStatic, true, CollideWithStatic)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_bLinearSpin,		   false, LinearSpin)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_obLocalAxisToFace, CDirection( 0.0f,0.0f,1.0f ), LocalAxisToFace)

	PUBLISH_VAR_WITH_DEFAULT_AS (m_obParentRelativeStartDirection, CDirection(0.0f, 0.0f, 1.0f), ParentRelativeStartDirection)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_fPercentageDistanceToLerpOver, 0.9f, PercentageDistanceToLerpOver)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_fMaxHomingSpeed, 1.0f, MaxHomingSpeed)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_fSpinSpeed, 1000.0f, SpinSpeed)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_fPenetrability, 1.0f, Penetrability)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_fLinearDamping, 0.0f, LinearDamping)
	PUBLISH_VAR_AS	(m_fAftertouchEnterSpeedClamp, AftertouchEnterSpeedClamp)
END_STD_INTERFACE

START_STD_INTERFACE (BazookaProjectileProperties)
	DEFINE_INTERFACE_INHERITANCE(ProjectileProperties)
	COPY_INTERFACE_FROM(ProjectileProperties)

	PUBLISH_CONTAINER_AS( m_DropOffTimeList, DropOffTimes )
	PUBLISH_VAR_AS( m_fDropOffAcceleration, DropOffAcceleration )
	PUBLISH_VAR_AS( m_fAftertouchDropThreshold, AftertouchDropThreshold )
	PUBLISH_VAR_AS( m_bUseLODEffects, UseLODRocketEffects )
END_STD_INTERFACE


namespace Physics {

	ProjectileManager::~ProjectileManager ()
	{
		m_obStaticProjectileList.clear();
	}

	static const float fPROJECTILE_LIMIT = 200;
	void ProjectileManager::AddStaticProjectile (CEntity* pobEntity)
	{
		ntAssert(pobEntity);

		if (m_obStaticProjectileList.size()>=fPROJECTILE_LIMIT)
		{
			CEntity* pobFirstEntity=m_obStaticProjectileList.front();

			CMessageSender::SendEmptyMessage("msg_removefromworld",pobFirstEntity->GetMessageHandler());
					
			m_obStaticProjectileList.pop_front();
		}

		m_obStaticProjectileList.push_back(pobEntity);
	}

	void ProjectileManager::RemoveChildProjectiles (CEntity* pobParent)
	{
		ntAssert(pobParent);

		for(ntstd::List<CEntity*>::iterator obIt=m_obStaticProjectileList.begin(); obIt!=m_obStaticProjectileList.end();)
		{
			if ((*obIt)->GetParentEntity()==pobParent)
			{
				// Send a message to the projectile
				CMessageSender::SendEmptyMessage("msg_removefromworld",(*obIt)->GetMessageHandler());

				// Hide the entire renderable
				(*obIt)->GetRenderableComponent()->AddRemoveAll_Game(false);

				// Reparent this projectile back to the world
				CMatrix obWorldMatrix = (*obIt)->GetMatrix();
				(*obIt)->GetHierarchy()->GetRootTransform()->RemoveFromParent();
				(*obIt)->GetHierarchy()->GetRootTransform()->SetLocalMatrix(obWorldMatrix);	
				CHierarchy::GetWorld()->GetRootTransform()->AddChild((*obIt)->GetHierarchy()->GetRootTransform());
		
				// Unassign parent entity
				(*obIt)->SetParentEntity(0); 
						
				// Remove this from our list
				obIt = m_obStaticProjectileList.erase( obIt );
			}
			else
			{
				++obIt;
			}
		}
	}

	SplineBuilder::SplineBuilder ()
	{
		m_iIndex=0;
		m_fT=0.0f;
		m_fTStep=0.1f;
		m_fOffset=0.0f;
		m_obPosition.Clear();
		m_obVelocity.Clear();
		m_obNextPos.Clear();

		for(int iIndex=0; iIndex<4; iIndex++)
			m_aobPoint[iIndex].Clear();
	}

	SplineBuilder::~SplineBuilder ()
	{
	}


	void SplineBuilder::Update ()
	{
		m_fT += m_fTStep; // Move along curve

		if (m_obVelocity.LengthSquared()>EPSILON) // Curve is moving!
		{
			for(int iCount=0; iCount<4; iCount++) // Move each point
				m_aobPoint[iCount]+=m_obVelocity;
		}
		
		while(m_fT >= 1.0f) // Next waypoint has been reached
		{
			m_fT -= 1.0f; // Reset position
			m_iIndex++; // Move to next point
			m_iIndex &= 0x3; // Loop index 0->4
			m_aobPoint[m_iIndex]=m_obNextPos; // Fetch new point

			if (m_fOffset>EPSILON) // Add an offset if required
			{
				CDirection obOffset(CONSTRUCT_CLEAR);
				obOffset.X()=grandf(2.0f)-1.0f;
				obOffset.Y()=grandf(2.0f)-1.0f;
				obOffset.Z()=grandf(2.0f)-1.0f;
				obOffset.Normalise();
				obOffset*=grandf(m_fOffset);
				m_aobPoint[m_iIndex]+=obOffset;
			}	
		}

		// NOTE: These coefficient probably don't need to be re-calculated each frame!
		float fBi[4],fT,fT2,fT3,fMt,fD6;
				
		// Calculate co-efficients
		fT = m_fT;
		fT2 = fT*fT;
		fT3 = fT2*fT;
		fMt = 1.0f-fT;
		fD6 = 1.0f/6.0f;

		fBi[3] = fMt*fMt*fMt*fD6;
		fBi[2] = ((3.0f*fT3) - (6.0f*fT2) + 4.0f) * fD6;
		fBi[1] = ((-3.0f*fT3) + (3.0f*fT2) + (3.0f*fT) + 1.0f) * fD6;
		fBi[0] = fT3*fD6;
		
		// Get pi-3, pi-2, pi-1 & pi
		CPoint *pobPim [4];
		CPoint *pobPointer = m_aobPoint;
		int	iIndex = m_iIndex;

		pobPim[3] = pobPointer+((iIndex-3)&0x3);
		pobPim[2] = pobPointer+((iIndex-2)&0x3);
		pobPim[1] = pobPointer+((iIndex-1)&0x3);
		pobPim[0] = pobPointer+((iIndex)&0x3);
		
		// Calculate position on curve
		m_obPosition.X()=(fBi[3] * (*pobPim[3]).X()) +
						(fBi[2] * (*pobPim[2]).X()) +
						(fBi[1] * (*pobPim[1]).X()) +
						(fBi[0] * (*pobPim[0]).X());

		m_obPosition.Y()=(fBi[3] * (*pobPim[3]).Y()) + 
						(fBi[2] * (*pobPim[2]).Y()) + 
						(fBi[1] * (*pobPim[1]).Y()) + 
						(fBi[0] * (*pobPim[0]).Y());

		m_obPosition.Z()=(fBi[3] * (*pobPim[3]).Z()) + 
						(fBi[2] * (*pobPim[2]).Z()) + 
						(fBi[1] * (*pobPim[1]).Z()) + 
						(fBi[0] * (*pobPim[0]).Z());
	}

	// -  ProjectileLG ------------------------------------------------------------------------------------------------------------------
	ProjectileLG::ProjectileLG( const ntstd::String& p_name, CEntity* p_entity ) :
		LogicGroup( p_name, p_entity )
	{
		m_pobProperties = 0;
		m_pobRoot = 0;
		m_bMoving = false;
		m_bHasRicocheted=false;
		m_bStopMovingOnCollision=true;
		m_pobData = 0;
		m_fSpinOffset = 0.0f;
		m_bFreeze = false;
		m_AddToInitialSpeed = 0.0f;
		m_bInAftertouch = false;
		m_bForceStraightVectorLerp = false;
	}

	ProjectileLG::~ProjectileLG( )
	{ }

	const LogicGroup::GROUP_TYPE ProjectileLG::GetType( ) const
	{
		return PROJECTILE_LG;
	}
	
	bool ProjectileLG::HasMissedTarget(float fMissAngle)
	{
		CDirection obToTarget( m_pobData->pTarget->GetPosition() - m_entity->GetMatrix().GetTranslation() );
		obToTarget.Normalise();
		CDirection obVelocity( m_obVelocity );
		obVelocity.Normalise();

		CQuat obQuat( obToTarget, obVelocity );
		float fAngle = 0.0f;
		CDirection obAxis( CONSTRUCT_CLEAR );
		obQuat.GetAxisAndAngle( obAxis, fAngle );

		if (fabs(fAngle) > fMissAngle * DEG_TO_RAD_VALUE && m_fTime > m_fTimeToLerp )
		{
			// Yes we have missed
			return true;
		}

		return false;
	}

	class ProjectileRaycastFilter : public CastRayFilter
	{
	public:
		ProjectileRaycastFilter(CEntity& ent, ProjectileProperties * pobProp) 
			: m_entity(ent), m_filter(pobProp)  {};

		bool operator() (CEntity *pobEntity) const
		{			
			if (!m_entity.GetInteractionComponent()->CanCollideWith(pobEntity))
				return false;
			
			return m_filter(pobEntity);
		}
	protected:
		CEntity& m_entity;
		ProjectilePropertiesRaycastFilter m_filter; 	
	};

	void ProjectileLG::Update( const float fTimeStep )
	{ 
		if (!m_bMoving)
		{
			CDirection obHeadingVector( m_obVelocity );
			obHeadingVector.Normalise();

			CMatrix obNewLocal( CONSTRUCT_IDENTITY );
			CCamUtil::CreateFromPoints(
				obNewLocal,
				m_pobRoot->GetWorldMatrix().GetTranslation(),
				m_pobRoot->GetWorldMatrix().GetTranslation() + obHeadingVector,
				CVecMath::GetYAxis() );

			CDirection obWorldAxisToFace = m_pobProperties->m_obLocalAxisToFace * obNewLocal;
			obWorldAxisToFace.Normalise();

			CQuat obRotationDelta( obNewLocal.GetZAxis(), obWorldAxisToFace );
			CDirection obAxis;
			float fAngle;
			obRotationDelta.GetAxisAndAngle(obAxis,fAngle);

			CMatrix obRot( CONSTRUCT_IDENTITY );
			obRot.SetFromAxisAndAngle( obAxis, fAngle );
			obNewLocal *= obRot;

			obNewLocal.SetTranslation(m_pobRoot->GetWorldMatrix().GetTranslation());

			m_pobRoot->SetLocalMatrix(obNewLocal);	
			
			return;
		}

		ntAssert(m_pobProperties);

		CPoint obOldPosition(m_pobRoot->GetWorldMatrix().GetTranslation());
		CPoint obNewPosition(obOldPosition);

		// ----- Projectile motion control -----

		switch(m_pobProperties->m_eMotionType)
		{
			case MOTION_VECTORLERP:
				{
					ntError(m_pobData);

					CDirection obToTarget( (m_pobData->pTarget->GetPosition() + m_pobData->vTrackedEntityOffset) - m_entity->GetMatrix().GetTranslation() );
					float fToTargetDistance = obToTarget.Length();
					obToTarget.Normalise();
					
					/*#ifdef _DEBUG
					g_VisualDebug->RenderLine(m_entity->GetMatrix().GetTranslation(), m_entity->GetMatrix().GetTranslation() + obToTarget, DC_GREEN);
					#endif*/

					if (m_bFirstFrame)
					{
						m_fTimeToLerp = (m_pobProperties->m_fPercentageDistanceToLerpOver * fToTargetDistance) / (m_pobProperties->m_fInitialSpeed + m_pobProperties->m_fAcceleration);
						if (m_bForceStraightVectorLerp)
							m_obStartDirection = obToTarget;
						else
						m_obStartDirection = (m_pobProperties->m_obParentRelativeStartDirection * m_pobData->pAttacker->GetMatrix()) * m_entity->GetMatrix().GetAffineInverse();
						m_obStartDirection.Normalise();
						m_obSpinAxis = CDirection(0.0f,1.0f,0.0f);
					}

					CDirection obStartDirection( m_obStartDirection * m_entity->GetMatrix() );					
					obStartDirection.Normalise();

					if (m_bFirstFrame)
					{
						m_obLastToTarget = obToTarget;
					}

					float fVal = CMaths::SmoothStep( ntstd::Clamp( m_fTime / m_fTimeToLerp, 0.0f, 1.0f ) );
					float fSpeed = m_pobProperties->m_fInitialSpeed + ( m_pobProperties->m_fAcceleration * ( ntstd::Clamp( m_fTime / m_pobProperties->m_fAccelerationTime, 0.0f, 1.0f ) ) );
					if (m_bFreeze)
						fSpeed = 0.0f;

					CQuat obQuat( m_obLastToTarget, obToTarget );
					float fAngle = 0.0f;
					CDirection obAxis( CONSTRUCT_CLEAR );
					obQuat.GetAxisAndAngle( obAxis, fAngle );
										
					//g_VisualDebug->RenderLine(m_entity->GetMatrix().GetTranslation(), m_entity->GetMatrix().GetTranslation() + m_obSpinAxis, DC_YELLOW);

					if (fVal == 1.0f)
					{						
						if ( fabs(fAngle) > EPSILON )
						{
							// Check to see if we're not returning to our thrower
							if ( m_pobData->pTarget != m_pobData->pOriginator && fabs(fAngle) > DEG_TO_RAD_VALUE * m_pobProperties->m_fMaxHomingSpeed * fTimeStep )
							{
								CMatrix obRot( CONSTRUCT_IDENTITY );
								if (fAngle < 0)
									obRot.SetFromAxisAndAngle(m_obSpinAxis, DEG_TO_RAD_VALUE * m_pobProperties->m_fMaxHomingSpeed * fTimeStep * -1);
								else
									obRot.SetFromAxisAndAngle(m_obSpinAxis, DEG_TO_RAD_VALUE * m_pobProperties->m_fMaxHomingSpeed * fTimeStep);
								obToTarget = m_obLastToTarget * obRot;
							}
						}
					}
					else
					{
						// Hacky, but stop the axis flipping out on tiny changes
						if ( fabs(fAngle) > 0.1f )
							m_obSpinAxis = obAxis;
					}

					m_obVelocity = CDirection::Lerp( obStartDirection, obToTarget, fVal );
					m_obVelocity.Normalise();

					// Rotate our start direction so it stays in relatively the same orientation
					CQuat obVelQuat( m_obLastVelocity, m_obVelocity );
					float fVelAngle = 0.0f;
					CDirection obVelAxis( CONSTRUCT_CLEAR );
					obQuat.GetAxisAndAngle( obVelAxis, fVelAngle );
					if (m_fTime > 0.0f && fabs(fVelAngle) > 0.0f)
					{
						CMatrix obRot( CONSTRUCT_IDENTITY );
						obRot.SetFromAxisAndAngle(obVelAxis, fVelAngle);
						m_obStartDirection = m_obStartDirection * obRot;
					}

					/*#ifdef _DEBUG
					g_VisualDebug->RenderLine(m_entity->GetMatrix().GetTranslation(), m_entity->GetMatrix().GetTranslation() + obStartDirection, DC_RED);
					g_VisualDebug->RenderLine(m_entity->GetMatrix().GetTranslation(), m_entity->GetMatrix().GetTranslation() + m_obVelocity, DC_YELLOW);
					#endif*/

					m_obVelocity *= fSpeed;
					obNewPosition += m_obVelocity * fTimeStep;

					m_obLastToTarget = obToTarget;
					m_obLastVelocity = m_obVelocity;
					m_obLastVelocity.Normalise();

					break;
				}
			case MOTION_LINEAR:
			{
				if (m_bFirstFrame)
				{
					// Set the initial velocity of the projectile
					m_obVelocity = m_obThrustDirection;
					m_obVelocity *= m_pobProperties->m_fInitialSpeed + m_AddToInitialSpeed;
				}

				// To Projectile
				if( GetEntity() && GetEntity()->IsProjectile() )
				{
					Object_Projectile* pProj = GetEntity()->ToProjectile();
					
					if( pProj->m_bInAftertouch && !m_bInAftertouch )
					{
						// Get the current speed
						float fVel = m_obVelocity.Length();

						// Normalise the speed
						m_obVelocity /= fVel;

						// Clamp the speed
						fVel = clamp( fVel, 0.0f, m_pobProperties->m_fAftertouchEnterSpeedClamp );

						// Adjust the velocity to the new speed
						m_obVelocity *= fVel;

						// Mark that we've processed the aftertouch state change. 
						m_bInAftertouch = true;
					}
				}

				// Apply thrust
				
				if (m_fTime < m_pobProperties->m_fAccelerationTime)
				{
					CDirection obThrust(m_obThrustDirection);

					obThrust*=m_pobProperties->m_fAcceleration * fTimeStep;

					m_obVelocity+=obThrust;
				}

				// Apply Linear Damping
				if (m_pobProperties->m_fLinearDamping > 0.0f )
				{
					CDirection obDamping = m_obVelocity;

					obDamping *= m_pobProperties->m_fLinearDamping * fTimeStep;

					m_obVelocity -= obDamping;
				}
				
				// Apply gravity
				if (m_fFallAcceleration!=0.0f && m_fTime > m_fFallTime)
				{
					m_obVelocity.Y()+=m_fFallAcceleration * fTimeStep;
				}

				// Process pad input

				ntstd::List<Behavior*>::iterator itb = m_behaviorList.begin();
				while (	itb != m_behaviorList.end() )
				{
					Behavior* event = (*itb);
					++itb;
					bool remove = event->Update( this );
					if( remove )
					{
						m_behaviorList.remove( event );
						NT_DELETE( event );
					}
				}

				// Calculate the new and old position

				CDirection obChange(m_obVelocity);
				obChange*=fTimeStep;

				obNewPosition+=obChange;

				break;
			}

			case MOTION_SPLINE:
			{
				if (m_bFirstFrame)
				{
					// Set the initial velocity of the projectile
					m_obVelocity=m_obThrustDirection;
					m_obVelocity*=m_pobProperties->m_fInitialSpeed;
					m_fSplineOffsetRadius=m_pobProperties->m_fSplineInitialRadius;
				}

				// Apply thrust
				
				float fThisAcceleration;

				if (m_fTime < m_pobProperties->m_fAccelerationTime)
				{
					CDirection obThrust(m_obThrustDirection);

					obThrust*=m_pobProperties->m_fAcceleration * fTimeStep;

					m_obVelocity+=obThrust;

					fThisAcceleration=m_pobProperties->m_fAcceleration;// * fTimeStep;
				}
				else
				{
					fThisAcceleration=0.0f;
				}
				
				// Apply gravity

				if (m_fFallAcceleration!=0.0f && m_fTime > m_fFallTime)
				{
					m_obVelocity.Y()+=m_fFallAcceleration * fTimeStep;
				}
				else
				{
					ntstd::List<Behavior*>::iterator itb = m_behaviorList.begin();
					while (	itb != m_behaviorList.end() )
					{
						Behavior* event = (*itb);
						++itb;
						bool remove = event->Update( this );
						if( remove )
						{
							m_behaviorList.remove( event );
							NT_DELETE( event );
						}
					}
				}

				// Update our spline
					
				if (m_pobProperties->m_fSplineInterval > EPSILON)
				{
					//CDirection obChange(m_obVelocity);
					//obChange*=m_pobProperties->m_fSplineInterval;
					//obNewPosition+=obChange;

					CDirection obChange(m_obVelocity);
					obChange.Normalise();

					float fSpeed=m_obVelocity.Length();
					float fT=m_pobProperties->m_fSplineInterval;

					float fDistance=fSpeed*fT + (0.5f*fThisAcceleration*fT*fT);
					
					obChange*=fDistance;// * 2.0f;

					obNewPosition+=obChange;


					
					if (m_bFirstFrame)
					{
						m_obSplineBuilder.SetPoints(1,obOldPosition,obNewPosition,obOldPosition,obOldPosition);
					}
					
					// Spline radius increase
					if (m_pobProperties->m_fSplineRadiusIncreaseRate>EPSILON)
					{
						m_fSplineOffsetRadius+=fTimeStep * m_pobProperties->m_fSplineRadiusIncreaseRate;

						if (m_fSplineOffsetRadius>m_pobProperties->m_fSplineMaxRadius)
						{
							m_fSplineOffsetRadius=m_pobProperties->m_fSplineMaxRadius;
						}

						m_obSplineBuilder.SetOffset(m_fSplineOffsetRadius);
					}

					m_obSplineBuilder.AddPoint(obNewPosition);

					float fSplineStep=fTimeStep / m_pobProperties->m_fSplineInterval;
					fSplineStep*=2.0f;

					m_obSplineBuilder.SetStep(fSplineStep);


					m_obSplineBuilder.Update();

					obNewPosition=m_obSplineBuilder.GetPosition();
				}
				else
				{
					ntError_p( ( obNewPosition - obOldPosition ).Length() < 0.001f, ("obNewPosition is the same as obOldPosition - will not play well with CPhysicsWorld::Get().TraceLine.") );
				}

				break;
			}

			default:
			{
				ntAssert(0); // We should never get here!

				break;
			}
		}

		if (!m_pobProperties->m_bNoCollide)
		{

			ProjectileRaycastFilter filter(*m_entity, m_pobProperties); 

			psPhysicsMaterial * pobPhysicsMaterial = NULL;
			TRACE_LINE_QUERY stQuery;
			stQuery.ppobPhysicsMaterial = &pobPhysicsMaterial;

			bool bCollisionFound; 
			switch (m_pobProperties->m_CollideWithCharacters)
			{
			case ProjectileProperties::COLLIDE_EXACT :
				{
					Physics::RaycastCollisionFlag obFlag; obFlag.base = 0;
					obFlag.flags.i_am = Physics::PROJECTILE_RAYCAST_BIT; 
					obFlag.flags.i_collide_with = (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	|
						Physics::CHARACTER_CONTROLLER_ENEMY_BIT		|
						Physics::RAGDOLL_BIT						|
						Physics::SMALL_INTERACTABLE_BIT				|
						Physics::LARGE_INTERACTABLE_BIT				);

					bCollisionFound = CPhysicsWorld::Get().TraceLineExactCharacters(obOldPosition,obNewPosition,0.5f, stQuery,obFlag, &filter);
					break;
				}
			case ProjectileProperties::COLLIDE_INEXACT :
				{
					Physics::RaycastCollisionFlag obFlag; obFlag.base = 0;
					obFlag.flags.i_am = Physics::PROJECTILE_RAYCAST_BIT; 
					obFlag.flags.i_collide_with = (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
						Physics::CHARACTER_CONTROLLER_ENEMY_BIT		|
						Physics::RAGDOLL_BIT						|
						Physics::SMALL_INTERACTABLE_BIT				|
						Physics::LARGE_INTERACTABLE_BIT				);

					bCollisionFound = CPhysicsWorld::Get().TraceLine(obOldPosition,obNewPosition,NULL, stQuery,obFlag, &filter);

					break;
				}
			default:
				{
					Physics::RaycastCollisionFlag obFlag; obFlag.base = 0;
					obFlag.flags.i_am = Physics::PROJECTILE_RAYCAST_BIT; 
					obFlag.flags.i_collide_with = (	Physics::RAGDOLL_BIT						|
						Physics::SMALL_INTERACTABLE_BIT				|
						Physics::LARGE_INTERACTABLE_BIT				);

					bCollisionFound = CPhysicsWorld::Get().TraceLine(obOldPosition,obNewPosition,NULL, stQuery,obFlag, &filter);
				}
			}


			if (bCollisionFound)
			{
				obNewPosition=stQuery.obIntersect;

				// If we have a ragdoll body, we can read which part I have collided with
				if(stQuery.obCollidedFlag.flags.i_am & Physics::RAGDOLL_BIT ) 
				{
					// Send a collision message
					Message obMessage(msg_collision);
					obMessage.SetEnt("Sender",(CEntity*)0);
					obMessage.SetEnt("Collidee",stQuery.pobEntity);

					obMessage.SetFloat("ProjVel",stQuery.obNormal.Dot(m_obVelocity));
					//obMessage.SetFloat("Angle",0.0f);
					obMessage.SetFloat("pX",stQuery.obIntersect.X());
					obMessage.SetFloat("pY",stQuery.obIntersect.Y());
					obMessage.SetFloat("pZ",stQuery.obIntersect.Z());

					Physics::RagdollCollisionFlag ragFlag;
					ragFlag.base = stQuery.obCollidedFlag.base;
					obMessage.SetInt("RagdollMaterial", (int) ragFlag.flags.ragdoll_material);

					if (pobPhysicsMaterial)
						obMessage.SetInt("PhysicsMaterial",pobPhysicsMaterial->GetMaterialId());
					else
						obMessage.SetInt("PhysicsMaterial",INVALID_MATERIAL);

					obMessage.SetBool("HasRicocheted", false);

					m_entity->GetMessageHandler()->QueueMessage( obMessage );					
				}
				else if (stQuery.obCollidedFlag.flags.i_am & (Physics::CHARACTER_CONTROLLER_PLAYER_BIT | Physics::CHARACTER_CONTROLLER_ENEMY_BIT))
				{
					// Send a collision message
					Message obMessage(msg_collision);
					obMessage.SetEnt("Sender",(CEntity*)0);
					obMessage.SetEnt("Collidee",stQuery.pobEntity);

					obMessage.SetFloat("ProjVel",stQuery.obNormal.Dot(m_obVelocity));
					//obMessage.SetFloat("Angle",0.0f);
					obMessage.SetFloat("pX",stQuery.obIntersect.X());
					obMessage.SetFloat("pY",stQuery.obIntersect.Y());
					obMessage.SetFloat("pZ",stQuery.obIntersect.Z());					
					obMessage.SetInt("RagdollMaterial", (int) -1);
					obMessage.SetInt("PhysicsMaterial",INVALID_MATERIAL);
					obMessage.SetBool("HasRicocheted", false);

					m_entity->GetMessageHandler()->QueueMessage( obMessage );					
				}
				else //if (m_pobProperties->m_bCollideWithStatic && (stQuery.pobEntity->GetEntType() & CEntity::EntType_Static || stQuery.pobEntity->GetEntType() & CEntity::EntType_Interactable) )
				{
					// Send a collision message
					Message obMessage(msg_collision);
					obMessage.SetEnt("Sender",(CEntity*)0);
					obMessage.SetEnt("Collidee",stQuery.pobEntity);
					obMessage.SetFloat("ProjVel",stQuery.obNormal.Dot(m_obVelocity));

					if (!m_bHasRicocheted && m_pobProperties->m_fRicochetProbability>0.0f && m_pobProperties->m_fRicochetProbability>=grandf(1.0f)) // Ricochet
					{
						const CDirection& obNormal=stQuery.obNormal;
						const CPoint& obIntersect=stQuery.obIntersect;

						CDirection obDirection(obIntersect - obOldPosition); // previous position -> intersect

						const float fDistanceFromSurface=obDirection.Length();

						obDirection /= fDistanceFromSurface;

						// Calculate the reflected vector
						//const float fDotP=obNormal.X()*obDirection.X()+obNormal.Y()*obDirection.Y()+obNormal.Z()*obDirection.Z();
						const float fDotP = obNormal.Dot(obDirection);

						const float fCOSINE_45_DEGREES = -0.7071068f;

						if (fDotP > fCOSINE_45_DEGREES) // Reject deflections greater than 45 degrees
						{
							float fF=2.0f * fDotP;

							CDirection obReflection(
								obDirection.X() - fF * obNormal.X(),
								obDirection.Y() - fF * obNormal.Y(),
								obDirection.Z() - fF * obNormal.Z());

							// Calculate a new velocity vector
							float fSpeed = m_obVelocity.Length();

							m_obThrustDirection=obReflection;
							m_obVelocity=obReflection;

							// Adjust the speed 
							fSpeed *= (1.0f - m_pobProperties->m_fRicochetEnergyLoss);

							// Set the new speed
							m_obVelocity *= fSpeed;

							// send word of the ricochet and the new veloicty
							Message msg(msg_ricochet);
							msg.AddParam( fSpeed );
							m_entity->GetMessageHandler()->QueueMessage( msg );

							obOldPosition=obIntersect;

							obNewPosition=obIntersect;
							obNewPosition+=obReflection*fDistanceFromSurface;

							if (m_pobProperties->m_eMotionType==MOTION_SPLINE)
							{
								// If its a spline controlled projectile, we need to completely reset the curve

								m_fSplineOffsetRadius=0.0f; // Reset the spline radius

								CPoint obProjectedPosition(obNewPosition);

								obProjectedPosition+=m_obVelocity * m_pobProperties->m_fSplineInterval;

								m_obSplineBuilder.SetPoints(1,obNewPosition,obProjectedPosition,obNewPosition,obNewPosition);

								m_obSplineBuilder.SetOffset(m_fSplineOffsetRadius);
							}

							m_bHasRicocheted = m_bStopMovingOnCollision ? true : m_bHasRicocheted;
						}
						else
						{
							m_bMoving = m_bStopMovingOnCollision ? false : m_bMoving;
						}
					}
					else // Stop the projectile from moving
					{
						m_bMoving = m_bStopMovingOnCollision ? false : m_bMoving;
					}

					
					//obMessage.SetFloat("Angle",0.0f);
					obMessage.SetFloat("pX",stQuery.obIntersect.X());
					obMessage.SetFloat("pY",stQuery.obIntersect.Y());
					obMessage.SetFloat("pZ",stQuery.obIntersect.Z());

					obMessage.SetInt("RagdollMaterial", -1);

					if (pobPhysicsMaterial)
						obMessage.SetInt("PhysicsMaterial",pobPhysicsMaterial->GetMaterialId());
					else
						obMessage.SetInt("PhysicsMaterial",INVALID_MATERIAL);

					obMessage.SetBool("HasRicocheted", m_bHasRicocheted);
					m_entity->GetMessageHandler()->QueueMessage( obMessage );
				}
			}
		}
		
		// ----- Recalculate the local matrix for the parent transform -----

		CMatrix obNewLocal;

		if (fTimeStep<EPSILON) // The projectile is barely moving, so use the previous orientation
		{
			obNewLocal=m_pobRoot->GetLocalMatrix();
		}
		else
		{
			CCamUtil::CreateFromPoints(
				obNewLocal,
				obOldPosition,
				obNewPosition,
				CVecMath::GetYAxis() );
		}

		CDirection obHeadingVector( m_obVelocity );
		obHeadingVector.Normalise();
		CDirection obWorldAxisToFace = m_pobProperties->m_obLocalAxisToFace * obNewLocal;
		obWorldAxisToFace.Normalise();
		CQuat obRotationDelta( obNewLocal.GetZAxis(), obWorldAxisToFace );
		CDirection obAxis;
		float fAngle;
		obRotationDelta.GetAxisAndAngle(obAxis,fAngle);
		CMatrix obRot( CONSTRUCT_IDENTITY );
		obRot.SetFromAxisAndAngle( obAxis, fAngle );
		obNewLocal *= obRot;

		if (m_pobProperties->m_eMotionType == MOTION_VECTORLERP)
		{
			CDirection obBlah(obNewLocal.GetZAxis());
			m_fSpinOffset += m_pobProperties->m_fSpinSpeed * DEG_TO_RAD_VALUE * fTimeStep;
			m_fSpinOffset > 360.0f * DEG_TO_RAD_VALUE ? m_fSpinOffset = 0.0f : m_fSpinOffset = m_fSpinOffset;

			CMatrix obTrans( CONSTRUCT_IDENTITY );
			obTrans.SetTranslation( CPoint(obBlah) * -2 );
			CMatrix obTransB( CONSTRUCT_IDENTITY );
			obTransB.SetTranslation( CPoint(obBlah) * 2 );			
			obRot.SetFromAxisAndAngle(m_obSpinAxis, m_fSpinOffset);

			obNewLocal *= obTrans * obRot * obTransB;
			/*#ifdef _DEBUG
			g_VisualDebug->RenderAxis(m_pobRoot->GetWorldMatrix(), 1.0f);
			g_VisualDebug->RenderLine(obNewPosition, obNewPosition + m_obSpinAxis, DC_PURPLE);
			#endif*/
		}

		// 
		if (m_pobProperties->m_eMotionType == MOTION_LINEAR && m_pobProperties->m_bLinearSpin && m_bMoving )
		{
			m_obSpinAxis	= CDirection( 0.0f, 0.0f, 1.0f );
			m_fSpinOffset	+= m_pobProperties->m_fSpinSpeed * DEG_TO_RAD_VALUE * fTimeStep;
			//m_fSpinOffset   = fmodf( m_fSpinOffset, TWO_PI );
			
			//obRot.SetFromAxisAndAngle( m_obSpinAxis, fsinf( m_fSpinOffset ));
			obRot.SetFromAxisAndAngle( m_obSpinAxis, m_fSpinOffset );
			obNewLocal = obRot * obNewLocal;
		}


		obNewLocal.SetTranslation(obNewPosition);
		m_pobRoot->SetLocalMatrix(obNewLocal);

		// ----- Update time -----

		m_fTime+=fTimeStep;

		m_bFirstFrame=false;
	}
		
	void ProjectileLG::SetMoving(bool bMoving)
	{
		m_bMoving = bMoving;
	}

	void ProjectileLG::Activate( bool activateInHavok )
	{
		m_bMoving = true;
		m_bFirstFrame=true;
		m_fTime=0.0f;
		m_bHasRicocheted=false;
		LogicGroup::Activate( activateInHavok );
	}

	void ProjectileLG::Deactivate( )
	{
		m_bMoving=false;
		LogicGroup::Deactivate();
	}

	RigidBody* ProjectileLG::AddRigidBody( const BodyCInfo* /*p_info*/ )	
	{
		ntAssert( 0  ) ;
		return 0;
	}

	ProjectileLG* ProjectileLG::Construct( CEntity* p_entity, ProjectileProperties* pobProperties, CPoint& obPosition, CDirection& obDirection, float fFallTime, float fFallAcceleration, Projectile_Data* pobData )
	{
		ProjectileLG* lg = NT_NEW ProjectileLG( p_entity->GetName(), p_entity );
		lg->Initialise( pobProperties, obPosition, obDirection, fFallTime, fFallAcceleration, pobData );
		lg->Activate();
		return lg;
	}

	void ProjectileLG::Initialise( ProjectileProperties* pobProperties, const CPoint& obPosition, const CDirection& obDirection, float fFallTime, float fFallAcceleration, Projectile_Data* pobData )
	{
		ntAssert(pobProperties);

		m_pobData = pobData;

		m_pobProperties = pobProperties;

		CMatrix obMatrix;

		CCamUtil::CreateFromPoints(
			obMatrix,
			obPosition,
			obPosition+obDirection,
			CVecMath::GetYAxis() );

		obMatrix.SetTranslation(obPosition);

		m_pobRoot = m_entity->GetHierarchy()->GetRootTransform();
		m_pobRoot->SetLocalMatrix(obMatrix);

		// Set velocity and thrust direction
		
		m_obVelocity.Clear();
		m_obThrustDirection=obDirection;
		m_obThrustDirection.Normalise();

		m_fFallTime = fFallTime;
		m_fFallAcceleration = fFallAcceleration + pobProperties->m_fGravity;

		m_fSplineOffsetRadius=0.0f;
	}

	void ProjectileLG::Reset(Projectile_Data* pobData)
	{
		//m_bFirstFrame = true;
		m_fTime = 0.0f;
		m_pobData = pobData;
		m_obStartDirection = ((m_pobProperties->m_obParentRelativeStartDirection * m_entity->GetMatrix()) * m_entity->GetMatrix().GetAffineInverse()) * -1;
	}

	void ProjectileLG::ApplyLocalisedLinearImpulse (const CDirection& obImpulse,const CVector& /*obPoint*/)
	{
		// Do I need to divide the applied velocity by the mass?

		CDirection obDirection=obImpulse * m_pobRoot->GetLocalMatrix();

		CDirection obNewVelocity(obDirection - m_obVelocity);

		m_obVelocity+=obNewVelocity;
	}
}
