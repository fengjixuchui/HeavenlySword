/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>

#include <hkdynamics/entity/hkRigidBody.h>
#include <hkdynamics/motion/rigid/hkStabilizedSphereMotion.h>
#include <hkdynamics/motion/rigid/hkStabilizedBoxMotion.h>
#include <hkdynamics/motion/rigid/hkFixedRigidMotion.h>
#include <hkdynamics/motion/rigid/hkKeyframedRigidMotion.h>
#include <hkdynamics/motion/rigid/ThinBoxMotion/hkThinBoxMotion.h>

#include <hkdynamics/entity/hkSpatialRigidBodyDeactivator.h>
#include <hkdynamics/entity/hkFakeRigidBodyDeactivator.h>

#include <hkmath/basetypes/hkAabb.h>
#include <hkcollide/shape/hkShape.h>
#include <hkdynamics/world/hkWorld.h>
#include <hkbase/memory/hkLocalArray.h>

#include <hkdynamics/world/util/hkWorldOperationQueue.h>
#include <hkdynamics/world/util/hkWorldOperationUtil.h>
#include <hkdynamics/world/util/hkWorldAgentUtil.h>

#include <hkdynamics/world/util/hkWorldCallbackUtil.h>
#include <hkdynamics/world/simulation/hkSimulation.h>

#include <hkdynamics/entity/util/hkEntityCallbackUtil.h>

#include <hkcollide/dispatch/hkCollisionDispatcher.h>
#include <hkcollide/shape/collection/hkShapeCollection.h>
#include <hkcollide/shape/list/hkListShape.h>
#include <hkcollide/shape/bvtree/hkBvTreeShape.h>
#include <hkdynamics/world/hkSimulationIsland.h>
#include <hkmath/linear/hkSweptTransformUtil.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkRigidBody);


HK_COMPILE_TIME_ASSERT( sizeof(hkRigidBody) == sizeof(hkEntity) );

static void estimateAllowedPenetrationDepth(hkCollidable* collidable, const hkReal allowedPenetrationDepth, const hkVector4& extent)
{
	hkReal minObjectDiameter = hkMath::min2( hkMath::min2( extent(0), extent(1)), extent(2));

	HK_ASSERT2(0xad65bbe4, minObjectDiameter > 0.0f, "Minimum object diameter cannot equal zero. The problem may be caused by the convex hull generator. E.g. in case of objects smaller than 1cm in size, all vertices may get welded together.");

	const hkReal radiusToAllowedPenetrationRatio = 5.0f; 
	if ( minObjectDiameter < 0.5f )
	{
		collidable->m_allowedPenetrationDepth = minObjectDiameter * (1.0f / radiusToAllowedPenetrationRatio);
	}
	else
	{
		collidable->m_allowedPenetrationDepth = 0.1f; 
	}

	HK_ASSERT2(0xad65bbe4, collidable->m_allowedPenetrationDepth > 0.00001f, "Allowed penetration depth cannot be zero. (This assert fires when it is less than 0.00001 (0.01mm), so if you know what you're doing you may ignore it.)");
	if (collidable->m_allowedPenetrationDepth < 0.001f)
	{
		HK_WARN(0xad65bbe4, "Allowed penetration depth cannot be zero. (This warning fires when it is less than 0.001 (1mm), so if you know what you're doing you may ignore it.)");
	}
}

void hkRigidBody::updateCachedShapeInfo(const hkShape* shape, hkVector4& extentOut )
{
	HK_ASSERT2(0x7cdcd3a0, shape, "The rigid body must have a shape.");
	// get the radius
	hkAabb aabb;
	shape->getAabb( hkTransform::getIdentity(), 0.0f, aabb );

	extentOut.setSub4( aabb.m_max, aabb.m_min ); 
	hkReal radius = 0.5f * hkReal(extentOut.length3());

	hkVector4 sphere; sphere.setAdd4( aabb.m_min, aabb.m_max );
	sphere.mul4( 0.5f );

	hkVector4 massCenterOffset; massCenterOffset.setSub4( sphere, getCenterOfMassLocal() );
	radius = radius + hkReal( massCenterOffset.length3() );

	hkMotionState& ms = getRigidMotion()->m_motionState;
	ms.m_objectRadius = radius;
}




void HK_CALL hkRigidBody::createDynamicRigidMotion( hkMotion::MotionType motionType, const hkVector4& position, const hkQuaternion& rotation, 
													hkReal mass, const hkMatrix3& inertiaLocal, const hkVector4& centreOfMassLocal, hkReal maxLinearVelocity, hkReal maxAngularVelocity,
													hkMaxSizeMotion* motionBufferOut )
{
	if(motionType != hkMotion::MOTION_KEYFRAMED)
	{
		HK_ASSERT2(0x305ac731,  mass > 0, "Mass not valid (mass > 0)" );
	}

	hkMotion* motion;
	switch(motionType)
	{
		case hkMotion::MOTION_SPHERE_INERTIA:
		{
			motion = new (motionBufferOut) hkSphereMotion(position, rotation );
			break;
		}
		
		case hkMotion::MOTION_BOX_INERTIA:
		{
			motion = new (motionBufferOut) hkBoxMotion(position, rotation );
			break;
		}
					
		case hkMotion::MOTION_THIN_BOX_INERTIA:
		{
			if ( maxAngularVelocity > 10.0f )
			{
				HK_WARN( 0xf03243df, "To get a stable thin box working, it is adviced to limit the angular velocity of the rigid body to be ~ 4.0f rad/sec" );
			}
			motion = new (motionBufferOut) hkThinBoxMotion(position, rotation );
			break;
		}

		case hkMotion::MOTION_STABILIZED_SPHERE_INERTIA:
		{
			motion = new (motionBufferOut) hkStabilizedSphereMotion(position, rotation );
			break;
		}
		
		case hkMotion::MOTION_STABILIZED_BOX_INERTIA:
		{
			motion = new (motionBufferOut) hkStabilizedBoxMotion(position, rotation );
			break;
		}
		case hkMotion::MOTION_DYNAMIC:
		{
			hkReal a = inertiaLocal(0,0);
			hkReal b = inertiaLocal(1,1);
			hkReal c = inertiaLocal(2,2);

			hkReal ma = hkMath::max2( a, hkMath::max2( b, c ));
			hkReal mi = hkMath::min2( a, hkMath::min2( b, c ));
			if ( mi > ma * 0.8f)
			{
				motion = new (motionBufferOut) hkSphereMotion(position, rotation );
			}
			else
			{
				if ( mi > ma* 0.1f )
				{
					motion = new (motionBufferOut) hkBoxMotion(position, rotation );
				}
				else
				{
					if ( maxAngularVelocity > 10.0f )
					{
						HK_WARN( 0xf03243df, "To get a stable thin box working, it is adviced to limit the angular velocity of the rigid body to be ~ 4.0f rad/sec" );
					}
					motion = new (motionBufferOut) hkThinBoxMotion(position, rotation );
				}
			}
			break;
		}
		
		case hkMotion::MOTION_KEYFRAMED:
		{
			maxLinearVelocity    = 1e6f;
			maxAngularVelocity	 = 1e6f;
			motion = new (motionBufferOut) hkKeyframedRigidMotion(position, rotation );
			break;
		}
		
		default:
		{
			motion = new (motionBufferOut) hkFixedRigidMotion( position, rotation );
			HK_ASSERT2(0x6fd67199, 0,"Motiontype invalid in RigidBody constuction info. Cannot construct body.");
		}
	}

	// If not a keyframed body, set the mass properties...
	if(motionType != hkMotion::MOTION_KEYFRAMED)
	{
		// Default value initialised to Diag(-1,-1,-1). Confirm that user has overwritten this
		// if we're about to use it for mass properties
		HK_ASSERT2(0x11a9ad41, (inertiaLocal(0,0) > 0.f)	|| (inertiaLocal(1,1) > 0.f)	|| (inertiaLocal(2,2) > 0.f), 
					"You have to specify a valid inertia tensor for rigid bodies in the hkRigidBodyCinfo.");	

		// Default value initialised to -1. Confirm that user has overwritten this
		// if we're about to use it for mass properties
		HK_ASSERT2(0x245a804f,  mass > 0, "You have to specify a valid mass for rigid bodies in the hkRigidBodyCinfo.");	

		motion->setInertiaLocal(inertiaLocal);
		motion->setCenterOfMassInLocal( centreOfMassLocal ); 
		motion->setMass(mass);
	}

	motion->setDeactivationCounter( 20 );
	motion->getMotionState()->m_maxLinearVelocity = maxLinearVelocity;
	motion->getMotionState()->m_maxAngularVelocity = maxAngularVelocity;
	
	// Info: this doesn't set motion->getMotionState()->m_objectRadius. 
	// This function is only meant to be used in hkRigidBody constructor and in void hkWorldOperationUtil::replaceMotionObject()
	// where it is followed by a call to either setMotionDeltaAngleMultiplier() or hkMotion::getPositionAndVelocities( newMotion );
}

hkRigidBody::hkRigidBody( const hkRigidBodyCinfo& info )
:	hkEntity( info.m_shape )
{
	//
	// Set hkEntity and hkWorld object properties
	//
	m_material.setResponseType( info.m_collisionResponse );
	m_processContactCallbackDelay = info.m_processContactCallbackDelay;
	m_collidable.setCollisionFilterInfo( info.m_collisionFilterInfo );

	HK_ASSERT2(0x492fb4e0,
		info.m_motionType > hkMotion::MOTION_INVALID && info.m_motionType < hkMotion::MOTION_MAX_ID,
		"Motiontype invalid (not yet specified) in RigidBody constuction info. Cannot construct body.");

	HK_ASSERT2( 0xf0010434, info.m_solverDeactivation > hkRigidBodyCinfo::SOLVER_DEACTIVATION_INVALID, "m_solverDeactivation not set");
	
	if (info.m_motionType == hkMotion::MOTION_FIXED)
	{
		hkFixedRigidMotion* p = new (&m_motion) hkFixedRigidMotion(info.m_position, info.m_rotation);

		p->getMotionState()->m_maxLinearVelocity  = info.m_maxLinearVelocity;
		p->getMotionState()->m_maxAngularVelocity = info.m_maxAngularVelocity;

		p->setDeactivationClass( hkRigidBodyCinfo::SOLVER_DEACTIVATION_OFF );

		hkMotionState* ms = getRigidMotion()->getMotionState();
		m_collidable.setMotionState( ms);

		if ( info.m_allowedPenetrationDepth <= 0.0f )
		{
			m_collidable.m_allowedPenetrationDepth = HK_REAL_MAX;
		}
		else
		{
			m_collidable.m_allowedPenetrationDepth = info.m_allowedPenetrationDepth;
		}
	}
	else
	{
		hkRigidBody::createDynamicRigidMotion( info.m_motionType, info.m_position, info.m_rotation, info.m_mass, info.m_inertiaTensor, info.m_centerOfMass, info.m_maxLinearVelocity, info.m_maxAngularVelocity, &m_motion );
		hkMotion* rigidMotion  = &m_motion;
		rigidMotion->setDeactivationClass( info.m_solverDeactivation );

		setLinearVelocity(info.m_linearVelocity);
		setAngularVelocity(info.m_angularVelocity);

		m_collidable.setMotionState( getRigidMotion()->getMotionState()); 

		// Set the deactivator type for this rigid body		
		setDeactivator(static_cast<enum hkRigidBodyDeactivator::DeactivatorType>(info.m_rigidBodyDeactivatorType));

		// User is doing 'lazy' construction.
		getCollidableRw()->m_allowedPenetrationDepth = info.m_allowedPenetrationDepth;
	}

	setLinearDamping( info.m_linearDamping );
	setAngularDamping( info.m_angularDamping );

	if( getCollidable()->getShape() )
	{	
		hkVector4 extent;
		updateCachedShapeInfo(getCollidable()->getShape(), extent);
		if ( getCollidable()->m_allowedPenetrationDepth <= 0.0f)
		{
			estimateAllowedPenetrationDepth(getCollidableRw(), getCollidable()->m_allowedPenetrationDepth, extent);
		}
#ifdef HK_DEBUG
		checkPerformance();
#endif
	}
	else
	{
		// only fixed body in the world has no shape
		HK_ASSERT2(0x7cdcd39f, info.m_motionType == hkMotion::MOTION_FIXED, "Cannot create an entity without a shape.");
	}
	
	if ( info.m_qualityType != HK_COLLIDABLE_QUALITY_INVALID )
	{
		setQualityType( info.m_qualityType );
		HK_ASSERT2( 0xf0ed54f7, !isFixed() || (HK_COLLIDABLE_QUALITY_FIXED == info.m_qualityType), "Only fixed objects can have the HK_COLLIDABLE_QUALITY_FIXED type" );
	}
	else
	{
		if ( isFixed() )
		{
			setQualityType( HK_COLLIDABLE_QUALITY_FIXED );
		}
		else if ( info.m_motionType == hkMotion::MOTION_KEYFRAMED)
		{
			setQualityType( HK_COLLIDABLE_QUALITY_KEYFRAMED );
		}
		else
		{
			setQualityType( HK_COLLIDABLE_QUALITY_DEBRIS );
		}
	}

	m_autoRemoveLevel = info.m_autoRemoveLevel;	

	getMaterial().setFriction( info.m_friction );
	getMaterial().setRestitution( info.m_restitution );
}

hkRigidBody::~hkRigidBody()
{
	HK_ACCESS_CHECK_WITH_PARENT( getWorld(), HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
}

void hkRigidBody::getCinfo(hkRigidBodyCinfo& info) const
{
	HK_ACCESS_CHECK_WITH_PARENT( getWorld(), HK_ACCESS_IGNORE, this, HK_ACCESS_RO );

	// Entity attributes
	{
		info.m_autoRemoveLevel = m_autoRemoveLevel;
		info.m_processContactCallbackDelay = m_processContactCallbackDelay;
		
		info.m_rigidBodyDeactivatorType = getDeactivator() ? getDeactivator()->getRigidBodyDeactivatorType() : hkRigidBodyDeactivator::DEACTIVATOR_NEVER;

		// Material attributes
		{
			info.m_friction = getMaterial().getFriction();	
			info.m_collisionResponse = m_material.getResponseType();	
			info.m_restitution = getMaterial().getRestitution();
		}

		// Motion attributes
		{
			info.m_linearDamping = getLinearDamping();		
			info.m_angularDamping = getAngularDamping();

			info.m_linearVelocity = getLinearVelocity();
			info.m_angularVelocity = getAngularVelocity();
	
			info.m_mass = getMass();			
			getInertiaLocal( info.m_inertiaTensor );

			info.m_motionType = getMotionType();

			// Motion state attributes
			{
				
				info.m_solverDeactivation =  static_cast<hkRigidBodyCinfo::SolverDeactivation>(getRigidMotion()->getDeactivationClass());		

				info.m_maxLinearVelocity  = getMaxLinearVelocity();
				info.m_maxAngularVelocity = getMaxAngularVelocity();

				info.m_position = getPosition();
				info.m_rotation = getRotation();
				
				info.m_centerOfMass = getCenterOfMassLocal();
			}
		}	
	}

	// World Object attributes
	{
		// Collidable attributes
		{
			info.m_shape = m_collidable.getShape();
			info.m_collisionFilterInfo = m_collidable.getCollisionFilterInfo();
			info.m_allowedPenetrationDepth = getCollidable()->m_allowedPenetrationDepth;
			info.m_qualityType = getCollidable()->getQualityType();	
		}
	}			
}

hkMotionState* hkRigidBody::getMotionState()
{
	HK_ACCESS_CHECK_WITH_PARENT( getWorld(), HK_ACCESS_RW, this, HK_ACCESS_RW );
	return getRigidMotion()->getMotionState();
}

void hkRigidBody::setDeactivator( hkRigidBodyDeactivator* deactivator )
{
	hkEntity::setDeactivator( deactivator );
}

hkRigidBody* hkRigidBody::clone() const 
{
	HK_ACCESS_CHECK_WITH_PARENT( getWorld(), HK_ACCESS_IGNORE, this, HK_ACCESS_RO );

	// Gather static shape info as well as dynamics motion state info
    hkRigidBodyCinfo currentData;
	{
		getCinfo( currentData );
	}

	// Initialize clone
	hkRigidBody* rbClone = new hkRigidBody( currentData );

	// Overwrite hkMotion of the body with a true clone
	{
			// we need memcpy, so that the vtable pointer is copied as well
		hkString::memCpy16( &rbClone->m_motion, &m_motion, sizeof( hkMaxSizeMotion)>>4);
		if ( m_motion.m_savedMotion )
		{
			rbClone->m_motion.m_savedMotion = new hkMaxSizeMotion();
			hkString::memCpy16( rbClone->m_motion.m_savedMotion, m_motion.m_savedMotion, sizeof( hkMaxSizeMotion)>>4);
		}
		// The CdBody has ptr back to the motion too, so we need to fix that up
		rbClone->getCollidableRw()->setMotionState( rbClone->getMotionState());
	}

	// Add user information to clone (properties)
	rbClone->copyProperties( this );
	{
		rbClone->setName( getName() );
		rbClone->setUserData( getUserData() );	
	}

	return rbClone;
}


void hkRigidBody::setMotionType( hkMotion::MotionType newState, hkEntityActivation preferredActivationState, hkUpdateCollisionFilterOnEntityMode collisionFilterUpdateMode )
{ 
	if (getWorld() && getWorld()->areCriticalOperationsLocked())
	{
		hkWorldOperation::SetRigidBodyMotionType op;
		op.m_rigidBody = this;
		op.m_motionType = newState;
		op.m_activation = preferredActivationState;
		op.m_collisionFilterUpdateMode = collisionFilterUpdateMode;

		getWorld()->queueOperation(op);
		return;
	}

	HK_ACCESS_CHECK_WITH_PARENT( getWorld(), HK_ACCESS_RW, this, HK_ACCESS_RW );
	hkWorldOperationUtil::setRigidBodyMotionType(this, newState, preferredActivationState, collisionFilterUpdateMode);

#ifdef HK_DEBUG
	checkPerformance();
#endif
}


hkWorldOperation::Result hkRigidBody::setShape(hkShape* shape )
{
	HK_ASSERT2(0x2005c7ff, shape, "Cannot setShape to NULL.");

	if (m_world && m_world->areCriticalOperationsLocked())
	{
		hkWorldOperation::SetWorldObjectShape op;
		op.m_worldObject = this;
		op.m_shape = shape;

		m_world->queueOperation(op);
		return hkWorldOperation::POSTPONED;
	}

	HK_ACCESS_CHECK_WITH_PARENT( getWorld(), HK_ACCESS_RW, this, HK_ACCESS_RW );

	if (m_world)
	{
		m_world->lockCriticalOperations();

		hkWorldOperationUtil::removeEntityBP(m_world, this);
	}


	{
		hkBool previousShapePresent = getCollidable()->getShape() != HK_NULL;

		// Handle reference counting here.
		if (getCollidable()->getShape())
		{
			getCollidable()->getShape()->removeReference();
		}
		getCollidableRw()->setShape(shape);
		shape->addReference();

#ifdef HK_DEBUG
		checkPerformance();
#endif

		// Perform additional necessary computations
		hkVector4 extent;
		updateCachedShapeInfo(shape, extent);

		if (previousShapePresent && getCollidable()->m_allowedPenetrationDepth != HK_REAL_MAX)
		{
			// Force automatic recalculation of allowed penetration depth if a shape was already present in that collidable
			getCollidableRw()->m_allowedPenetrationDepth = -1.0f; 
		}
		if ( getCollidableRw()->m_allowedPenetrationDepth <= 0.0f)
		{
			estimateAllowedPenetrationDepth(getCollidableRw(), getCollidable()->m_allowedPenetrationDepth, extent);
		}
	}

	// Callbacks -- done after all the other radii, allowedPenetrationDepths are recalculated.
	// and before the body is added back to broadphase
	if (m_world)
	{
		// Moreover it's better do it after 
		hkWorldCallbackUtil::fireEntityShapeSet( m_world, this );
	}
	hkEntityCallbackUtil::fireEntityShapeSet( this );


	if (m_world)
	{
		hkWorldOperationUtil::addEntityBP(m_world, this);

		m_world->unlockAndAttemptToExecutePendingOperations();
	}

	return hkWorldOperation::DONE;
}

	// Explicit center of mass in local space
void hkRigidBody::setCenterOfMassLocal(const hkVector4& centerOfMass)
{
	HK_ACCESS_CHECK_WITH_PARENT( getWorld(), HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
	getRigidMotion()->setCenterOfMassInLocal(centerOfMass);
	hkVector4 dummyExtent;
	updateCachedShapeInfo(m_collidable.getShape(), dummyExtent);
}

void hkRigidBody::setDeactivator( enum hkRigidBodyDeactivator::DeactivatorType rigidBodyDeactivatorType )
{
	HK_ACCESS_CHECK_WITH_PARENT( getWorld(), HK_ACCESS_RO, this, HK_ACCESS_RW );

	if( getDeactivator() != HK_NULL 
		&& getDeactivator()->getRigidBodyDeactivatorType() == rigidBodyDeactivatorType )
	{
		HK_WARN(0x14dccf5e, "You are attempting to replace the existing deactivator with a new one of the same type - Old deactivator remains in effect.");
	}
	else
	{
		switch( rigidBodyDeactivatorType )
		{
			case hkRigidBodyDeactivator::DEACTIVATOR_NEVER:
			{
				// Set the deactivator to HK_NO_RIGIDBODY_DEACTIVATOR type. The body will NEVER deactivate.
				setDeactivator( hkFakeRigidBodyDeactivator::getFakeRigidBodyDeactivator() );
				m_motion.enableDeactivation(false);
				break;
			}

			case hkRigidBodyDeactivator::DEACTIVATOR_SPATIAL:
			{
				// Set the deactivator to HK_SPATIAL_RIGIDBODY_DEACTIVATOR type. The body will use the hkSpatialRigidBodyDeactivator to determine activation state.
				hkSpatialRigidBodyDeactivator* deactivator = new hkSpatialRigidBodyDeactivator();
				setDeactivator( deactivator );
				m_motion.enableDeactivation(true, m_uid);
				deactivator->removeReference();
				break;
			}
		
			default:
			{
				HK_ASSERT2(0x482e56fd, 0, "Invalid hkDeactivatorType specified - unable to change/set deactivator.");
			}
		}
	}
}

hkRigidBodyDeactivator::DeactivatorType hkRigidBody::getDeactivatorType()
{
	return getDeactivator()->getRigidBodyDeactivatorType();
}


hkMotion* hkRigidBody::getStoredDynamicMotion()
{
	if ( m_motion.getType() == hkMotion::MOTION_KEYFRAMED || m_motion.getType() == hkMotion::MOTION_FIXED )
	{
		return static_cast<hkKeyframedRigidMotion*>( &m_motion )->m_savedMotion;
	}
	else
	{
		return HK_NULL;
	}
}


const hkMotion* hkRigidBody::getStoredDynamicMotion() const
{
	if ( m_motion.getType() == hkMotion::MOTION_KEYFRAMED || m_motion.getType() == hkMotion::MOTION_FIXED )
	{
		return static_cast<const hkKeyframedRigidMotion*>( &m_motion )->m_savedMotion;
	}
	else
	{
		return HK_NULL;
	}
}



/*
** UTILITIY
*/

//#define USE_OPERATION_DELAY_MGR



void HK_CALL hkRigidBody::updateBroadphaseAndResetCollisionInformationOfWarpedBody( hkEntity* entity )
{
	hkWorld* world = entity->getWorld();
	if (world)
	{
		if ( world->areCriticalOperationsLocked() )
		{
			hkWorldOperation::UpdateMovedBodyInfo op;
			op.m_entity = entity;
			world->queueOperation(op);
			return;
		}

		HK_ACCESS_CHECK_WITH_PARENT( world, HK_ACCESS_RW, entity, HK_ACCESS_RW );

		world->lockCriticalOperations();

		hkEntity* entities[1] = { entity };
		// Note: it is intended to invalidate manifolds of the agents in this function.
		// As we have no interface to do it, for now, we call update broadphase, so that
		// agents, and therefore contact points, are removed when a body is moved over a large distance.
		// (No mid phase is done though, so that doesn't work, when you're overlapping with a mopp.)

		// TOI events removed in resetCollisionInformationForEntities
		world->m_simulation->resetCollisionInformationForEntities( entities, 1, world );

		if(entity->getCollidable()->getShape() != HK_NULL)
		{
			hkSimulation::collideEntitiesBroadPhaseDiscrete(entities, 1, world);
		}

			// fixed bodies are also inactive, so we simplify the following check
		if (!entity->isActive())
		{
			if (world->m_shouldActivateOnRigidBodyTransformChange && !entity->isFixed())
			{
				entity->activate();
			}
			else
			{
				// We want to update aabbs's so that inactive bodies don't get waken up without no reason after 
				// This may fire immediate contactPointRemoved callbacks 

				// Temporarily BroadPhase Collision Detection executed for active bodies too (see above).
				// hkSimulation::collideEntitiesBroadPhaseDiscrete(entities, 1, world);
			}

			// update graphics, or sth.
			hkWorldCallbackUtil::fireInactiveEntityMoved(world, entity);
		}	

		world->unlockAndAttemptToExecutePendingOperations();
	}
}


	// Set the position (Local Space origin) of this rigid body, in World space.
void hkRigidBody::setPosition(const hkVector4& position)
{
	HK_ACCESS_CHECK_WITH_PARENT( getWorld(), HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
	getRigidMotion()->setPosition(position);
	updateBroadphaseAndResetCollisionInformationOfWarpedBody(this);
}

	// Set the rotation from Local to World Space for this rigid body.
void hkRigidBody::setRotation(const hkQuaternion& rotation)
{
	HK_ACCESS_CHECK_WITH_PARENT( getWorld(), HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
	HK_ASSERT2(0x275ec1fd, rotation.isOk(), "hkQuaternion used in hkRigidBody::setRotation() is not normalized/invalid!");

	getRigidMotion()->setRotation(rotation);
	updateBroadphaseAndResetCollisionInformationOfWarpedBody(this);
}

	// Set the position and rotation of the rigid body, in World space.
void hkRigidBody::setPositionAndRotation(const hkVector4& position, const hkQuaternion& rotation)
{
	HK_ACCESS_CHECK_WITH_PARENT( getWorld(), HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
	HK_ASSERT2(0x4669e4a0, rotation.isOk(), "hkQuaternion used in hkRigidBody::setPositionAndRotation() is not normalized/invalid!");
	
	getRigidMotion()->setPositionAndRotation(position, rotation);
	updateBroadphaseAndResetCollisionInformationOfWarpedBody(this);
}

	// Sets the rigid body (local) to world transformation
void hkRigidBody::setTransform(const hkTransform& transform)
{
	HK_ACCESS_CHECK_WITH_PARENT( getWorld(), HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
	getRigidMotion()->setTransform(transform);
	updateBroadphaseAndResetCollisionInformationOfWarpedBody(this);
}

	// Set the mass of the rigid body.
void hkRigidBody::setMass(hkReal m)
{
	HK_ACCESS_CHECK_WITH_PARENT( getWorld(), HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
	getRigidMotion()->setMass(m);
}

void hkRigidBody::setMassInv(hkReal mInv)
{
	HK_ACCESS_CHECK_WITH_PARENT( getWorld(), HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
	getRigidMotion()->setMassInv(mInv);
}

	// Sets the inertia tensor of the rigid body. Advanced use only.
void hkRigidBody::setInertiaLocal(const hkMatrix3& inertia)
{
	HK_ACCESS_CHECK_WITH_PARENT( getWorld(), HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
	getRigidMotion()->setInertiaLocal(inertia);
}


	// Sets the inertia tensor of the rigid body by supplying its inverse. Advanced use only.
void hkRigidBody::setInertiaInvLocal(const hkMatrix3& inertiaInv)
{
	HK_ACCESS_CHECK_WITH_PARENT( getWorld(), HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
	getRigidMotion()->setInertiaInvLocal(inertiaInv);
}

void hkRigidBody::setPositionAndRotationAsCriticalOperation(const hkVector4& position, const hkQuaternion& rotation)
{
	if (getWorld() && getWorld()->areCriticalOperationsLocked())
	{
		hkWorldOperation::SetRigidBodyPositionAndRotation op;
		op.m_rigidBody = this;
		op.m_positionAndRotation = hkAllocateChunk<hkVector4>( 2, HK_MEMORY_CLASS_DYNAMICS );
		op.m_positionAndRotation[0] = position;
		op.m_positionAndRotation[1] = rotation.m_vec;
		getWorld()->queueOperation(op);
		return;
	}

	HK_ACCESS_CHECK_WITH_PARENT( getWorld(), HK_ACCESS_RW, this, HK_ACCESS_RW );
	setPositionAndRotation(position, rotation);
}

void hkRigidBody::setLinearVelocityAsCriticalOperation(const hkVector4& newVel)
{
	if (getWorld() && getWorld()->areCriticalOperationsLocked())
	{
		hkWorldOperation::SetRigidBodyLinearVelocity op;
		op.m_rigidBody = this;
		op.m_linearVelocity[0] = newVel(0);
		op.m_linearVelocity[1] = newVel(1);
		op.m_linearVelocity[2] = newVel(2);
		getWorld()->queueOperation(op);
		return;
	}

	HK_ACCESS_CHECK_WITH_PARENT( getWorld(), HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
	setLinearVelocity(newVel);
}

void hkRigidBody::setAngularVelocityAsCriticalOperation(const hkVector4& newVel)
{
	if (getWorld() && getWorld()->areCriticalOperationsLocked())
	{
		hkWorldOperation::SetRigidBodyAngularVelocity op;
		op.m_rigidBody = this;
		op.m_angularVelocity[0] = newVel(0);
		op.m_angularVelocity[1] = newVel(1);
		op.m_angularVelocity[2] = newVel(2);
		getWorld()->queueOperation(op);
		return;
	}

	HK_ACCESS_CHECK_WITH_PARENT( getWorld(), HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
	setAngularVelocity(newVel);
}

void hkRigidBody::applyLinearImpulseAsCriticalOperation(const hkVector4& imp)
{
	if (getWorld() && getWorld()->areCriticalOperationsLocked())
	{
		hkWorldOperation::ApplyRigidBodyLinearImpulse op;
		op.m_rigidBody = this;
		op.m_linearImpulse[0] = imp(0);
		op.m_linearImpulse[1] = imp(1);
		op.m_linearImpulse[2] = imp(2);
		getWorld()->queueOperation(op);
		return;
	}

	HK_ACCESS_CHECK_WITH_PARENT( getWorld(), HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
	applyLinearImpulse(imp);
}

void hkRigidBody::applyPointImpulseAsCriticalOperation(const hkVector4& imp, const hkVector4& p)
{
	if (getWorld() && getWorld()->areCriticalOperationsLocked())
	{
		hkWorldOperation::ApplyRigidBodyPointImpulse op;
		op.m_rigidBody = this;
		op.m_pointAndImpulse = hkAllocateChunk<hkVector4>( 2, HK_MEMORY_CLASS_DYNAMICS ); 
		op.m_pointAndImpulse[0] = p;
		op.m_pointAndImpulse[1] = imp;
		getWorld()->queueOperation(op);
		return;
	}

	HK_ACCESS_CHECK_WITH_PARENT( getWorld(), HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
	applyPointImpulse(imp, p);
}

void hkRigidBody::applyAngularImpulseAsCriticalOperation(const hkVector4& imp)
{
	if (getWorld() && getWorld()->areCriticalOperationsLocked())
	{
		hkWorldOperation::ApplyRigidBodyAngularImpulse op;
		op.m_rigidBody = this;
		op.m_angularImpulse[0] = imp(0);
		op.m_angularImpulse[1] = imp(1);
		op.m_angularImpulse[2] = imp(2);
		getWorld()->queueOperation(op);
		return;
	}

	HK_ACCESS_CHECK_WITH_PARENT( getWorld(), HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
	applyAngularImpulse(imp);
}

void hkRigidBody::checkPerformance() const
{
	const hkShape* shape = getCollidable()->getShape();

	if( !shape )
	{
		return;
	}

	hkShapeType shapeType = shape->getType();

	if( shapeType == HK_SHAPE_LIST 
		&& static_cast<const hkListShape*>(shape)->getNumChildShapes() > 5 ) 
	{
		HK_WARN(0x2ff8c16c, "Collidable at address " << this << " is an hkListShape.\n" \
			"This can cause a significant performance loss when the shape is collided e.g. with another complex hkListShape.\n" \
			"When using hkListShape of more than 5 elements consider using hkConvexListShape or buliding an hkMoppBvTreeShape around it.\n");
	}

	if( getMotionType() != hkMotion::MOTION_FIXED &&
		( shapeType == HK_SHAPE_MOPP || shapeType == HK_SHAPE_BV_TREE )
		&& static_cast<const hkBvTreeShape*>(shape)->getShapeCollection()->getNumChildShapes() > 10 ) 
	{
		HK_WARN(0x2ff8c16d, "Collidable at address " << this << " is a complex mopp/bvTree shape (more than 10 elements) \n" \
			"and has motion type other than fixed.\n" \
			"Note that this can cause a significant performance loss. \n" \
			"Consider simpilifying the shape representation." );
	}

	if( shapeType == HK_SHAPE_TRIANGLE_COLLECTION )
	{
		HK_WARN(0x2ff8c16e, "Collidable at address " << this << " is a mesh shape.\n" \
			"This can cause a significant performance loss.\n" \
			"The collection of triangle shapes (hkMeshShape) should not be used for dynamic motions.\n" \
			"You may consider building an hkMoppBvTreeShape around the mesh.\n");
	}

	if( shapeType == HK_SHAPE_TRANSFORM)
	{
		HK_WARN(0x2ff8c16f, "Collidable at address " << this << " has a transform shape as the root shape.\n" \
			"This can cause a significant performance loss. To avoid getting this message\n" \
			"compose the transform into the collidable and remove the transform shape.\n" \
			"Please see the 'hkTransformShape' documentation in the User Guide for more information.\n");
	}

	if( shapeType == HK_SHAPE_COLLECTION
		&& static_cast<const hkShapeCollection*>(getCollidable()->getShape())->getNumChildShapes() > 10 )
	{
		HK_WARN(0x578cef50, "Collidable at address " << this << " has a shape collecion without a bvshape.\n" \
			"This can cause performance loss. To avoid getting this message\n" \
			"add a hkBvShape above this shape.");
	}
}

/*
* Havok SDK - CLIENT RELEASE, BUILD(#20060902)
*
* Confidential Information of Havok.  (C) Copyright 1999-2006 
* Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
* Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
* rights, and intellectual property rights in the Havok software remain in
* Havok and/or its suppliers.
*
* Use of this software for evaluation purposes is subject to and indicates 
* acceptance of the End User licence Agreement for this product. A copy of 
* the license is included with this software and is also available from salesteam@havok.com.
*
*/
