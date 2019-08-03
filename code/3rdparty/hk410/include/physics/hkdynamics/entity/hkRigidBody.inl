/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

inline hkRigidBody* hkGetRigidBody( const hkCollidable* collidable )
{
	if ( collidable->getType() == hkWorldObject::BROAD_PHASE_ENTITY )
	{
		return static_cast<hkRigidBody*>( hkGetWorldObject(collidable) );
	}
	return HK_NULL;
}

inline hkMotion* hkRigidBody::getRigidMotion() const
{
	return const_cast<hkMaxSizeMotion*>(&m_motion);
}

// Get the mass of the rigid body.
inline hkReal hkRigidBody::getMass() const
{
	return getRigidMotion()->getMass();
}

inline hkReal hkRigidBody::getMassInv() const
{
	return getRigidMotion()->getMassInv();
}


// Get the inertia tensor in local space
inline void hkRigidBody::getInertiaLocal(hkMatrix3& inertia) const
{
	getRigidMotion()->getInertiaLocal(inertia);
}

// Get the inertia tensor in world space
inline void hkRigidBody::getInertiaWorld(hkMatrix3& inertia) const
{
	getRigidMotion()->getInertiaWorld(inertia);
}


// Get the inverse inertia tensor in local space
void hkRigidBody::getInertiaInvLocal(hkMatrix3& inertiaInv) const 
{
	getRigidMotion()->getInertiaInvLocal(inertiaInv);
}

// Get the inverse inertia tensor in World space
void hkRigidBody::getInertiaInvWorld(hkMatrix3& inertiaInv) const 
{
	getRigidMotion()->getInertiaInvWorld(inertiaInv);
}

// Explicit center of mass in local space
const hkVector4& hkRigidBody::getCenterOfMassLocal() const
{
	return getRigidMotion()->getCenterOfMassLocal();
}

		
const hkVector4& hkRigidBody::getCenterOfMassInWorld() const
{
	return getRigidMotion()->getCenterOfMassInWorld();
}

/*
** UTILITIY
*/



// Return the position (Local Space origin) for this rigid body, in World space.
// Note: The center of mass is no longer necessarily the local space origin
inline const hkVector4& hkRigidBody::getPosition() const
{
	return getRigidMotion()->getPosition();
}

// Returns the rotation from Local to World space for this rigid body.
inline const hkQuaternion& hkRigidBody::getRotation() const
{
	return getRigidMotion()->getRotation();
}

// Returns the rigid body (local) to world transformation.
inline const hkTransform& hkRigidBody::getTransform() const
{
	return getRigidMotion()->getTransform();
}

// Used by the graphics engine.
// It only uses a simple lerp quaternion interpolation while 
// hkSweptTransformUtil::lerp2() used for collision detection
// uses the 'dobule-linear' interpolation 
void hkRigidBody::approxTransformAt( hkTime time, hkTransform& transformOut ) const
{
	getRigidMotion()->approxTransformAt( time, transformOut );
}

void hkRigidBody::approxCurrentTransform( hkTransform& transformOut ) const
{
	HK_ASSERT2(0x3dd87047, m_world, "This function requires the body to be in an hkWorld, in order to retrieve the current time.");
	getRigidMotion()->approxTransformAt( m_world->getCurrentTime(), transformOut );
}

/*
** VELOCITY ACCESS
*/

// Return the linear velocity of the center of mass of the rigid body, in World space.
inline const hkVector4& hkRigidBody::getLinearVelocity() const
{
	return getRigidMotion()->getLinearVelocity();
}


// Returns the angular velocity around the center of mass, in World space.
inline const hkVector4& hkRigidBody::getAngularVelocity() const
{
	return getRigidMotion()->getAngularVelocity();
}


// Velocity of point p on the rigid body, in World space.
void hkRigidBody::getPointVelocity(const hkVector4& p, hkVector4& vecOut) const
{
	getRigidMotion()->getPointVelocity(p, vecOut);
}


/*
** IMPULSE APPLICATION
*/

// Apply an impulse to the center of mass.
inline void hkRigidBody::applyLinearImpulse(const hkVector4& imp)
{
	activate();
	getRigidMotion()->applyLinearImpulse(imp);
}


// Apply an impulse at the point p in world space.
inline void hkRigidBody::applyPointImpulse(const hkVector4& imp, const hkVector4& p)
{
	activate();
	getRigidMotion()->applyPointImpulse(imp, p);
}


// Apply an instantaneous change in angular velocity around center of mass.
inline void hkRigidBody::applyAngularImpulse(const hkVector4& imp)
{
	activate();
	getRigidMotion()->applyAngularImpulse(imp);
}


/*
** FORCE AND TORQUE APPLICATION
*/

// Applies a force to the rigid body. The force is applied to the center of mass.
inline void hkRigidBody::applyForce(const hkReal deltaTime, const hkVector4& force)
{
	activate();
	getRigidMotion()->applyForce(deltaTime, force);
}

// Applies a force to the rigid body at the point p in world space.
inline void hkRigidBody::applyForce(const hkReal deltaTime, const hkVector4& force, const hkVector4& p)
{
	activate();
	getRigidMotion()->applyForce(deltaTime, force, p);
}


// Applies the specified torque to the rigid body.
inline void hkRigidBody::applyTorque(const hkReal deltaTime, const hkVector4& torque)
{
	activate();
	getRigidMotion()->applyTorque(deltaTime, torque);
}



/*
** DAMPING
*/

// Naive momentum damping
inline hkReal hkRigidBody::getLinearDamping() const
{
	return getRigidMotion()->getLinearDamping();
}

inline hkReal hkRigidBody::getAngularDamping() const
{
	return getRigidMotion()->getAngularDamping();
}



inline void hkRigidBody::setLinearDamping( hkReal d )
{
	getRigidMotion()->setLinearDamping( d );
}



inline void hkRigidBody::setAngularDamping( hkReal d )
{
	getRigidMotion()->setAngularDamping( d );
}


inline hkReal hkRigidBody::getMaxLinearVelocity() const
{
	return getRigidMotion()->getMotionState()->m_maxLinearVelocity;
}

inline hkReal hkRigidBody::getMaxAngularVelocity() const
{
	return getRigidMotion()->getMotionState()->m_maxAngularVelocity;
}

inline void hkRigidBody::setMaxLinearVelocity( hkReal maxVel )
{
	getRigidMotion()->getMotionState()->m_maxLinearVelocity = maxVel;
}

inline void hkRigidBody::setMaxAngularVelocity( hkReal maxVel )
{
	getRigidMotion()->getMotionState()->m_maxAngularVelocity = maxVel;
}

inline hkRigidBodyDeactivator* hkRigidBody::getDeactivator()
{
	return reinterpret_cast<hkRigidBodyDeactivator*>( hkEntity::getDeactivator() );
}

inline const hkRigidBodyDeactivator* hkRigidBody::getDeactivator() const
{
	return reinterpret_cast<const hkRigidBodyDeactivator*>( hkEntity::getDeactivator() );
}

inline enum hkMotion::MotionType hkRigidBody::getMotionType() const
{
	return getRigidMotion()->getType();
}

// Sets the linear velocity at the center of mass, in World space.
void hkRigidBody::setLinearVelocity(const hkVector4& newVel)
{
	activate();
	getRigidMotion()->setLinearVelocity(newVel);
}

// Sets the angular velocity around the center of mass, in World space.
void hkRigidBody::setAngularVelocity(const hkVector4& newVel)
{
	activate();
	getRigidMotion()->setAngularVelocity(newVel);
}

hkUint32 hkRigidBody::getCollisionFilterInfo() const
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RO );
	return getCollidable()->getBroadPhaseHandle()->getCollisionFilterInfo();
}

void hkRigidBody::setCollisionFilterInfo( hkUint32 info )
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
	getCollidableRw()->getBroadPhaseHandle()->setCollisionFilterInfo(info);
}

HK_FORCE_INLINE hkCollidableQualityType hkRigidBody::getQualityType() const
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RO );
	return hkCollidableQualityType( getCollidable()->getBroadPhaseHandle()->m_objectQualityType );
}

HK_FORCE_INLINE void hkRigidBody::setQualityType(hkCollidableQualityType type)
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
	getCollidableRw()->getBroadPhaseHandle()->m_objectQualityType = hkUint16(type);
}

HK_FORCE_INLINE hkReal hkRigidBody::getAllowedPenetrationDepth() const
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RO );
	return getCollidable()->m_allowedPenetrationDepth;
}

HK_FORCE_INLINE void hkRigidBody::setAllowedPenetrationDepth( hkReal val )
{
	HK_ACCESS_CHECK_WITH_PARENT( m_world, HK_ACCESS_IGNORE, this, HK_ACCESS_RW );
	HK_ASSERT2(0xad45bd3d, val > HK_REAL_EPSILON, "Must use a non-zero ( > epsilon ) value when setting allowed penetration depth of bodies.");
	getCollidableRw()->m_allowedPenetrationDepth = val;
}

/*
* Havok SDK - PUBLIC RELEASE, BUILD(#20060902)
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
