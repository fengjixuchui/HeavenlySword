/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkutilities/collide/softcontact/hkSoftContactUtil.h>
#include <hkdynamics/collide/hkResponseModifier.h>
#include <hkdynamics/entity/hkRigidBodyCinfo.h>
#include <hkdynamics/entity/hkRigidBody.h>

hkSoftContactUtil::hkSoftContactUtil( hkRigidBody* bodyA, hkRigidBody* optionalBodyB, hkReal forceScale, hkReal maxAccel )
{
	m_bodyA = bodyA;
	m_bodyB = optionalBodyB;

	m_bodyA->addCollisionListener( this );
	m_bodyA->addEntityListener( this );

	m_forceScale = forceScale;
	m_maxAcceleration = maxAccel;

	this->addReference();
}

hkSoftContactUtil::~hkSoftContactUtil()
{
	if(m_bodyA)
	{
		m_bodyA->removeCollisionListener( this );
		m_bodyA->removeEntityListener( this );
	}
}



void hkSoftContactUtil::contactPointAddedCallback( hkContactPointAddedEvent& event )
{
	hkConstraintInstance* instance = event.m_internalContactMgr.getConstraintInstance();
	if ( !instance )
	{
		return;
	}


	hkRigidBody* bodyA = static_cast<hkRigidBody*>(event.m_bodyA.getRootCollidable()->getOwner());
	hkRigidBody* bodyB = static_cast<hkRigidBody*>(event.m_bodyB.getRootCollidable()->getOwner());

	hkRigidBody* b = (m_bodyB)? m_bodyB : hkSelectOther( m_bodyA, bodyA, bodyB );

	// The bodies could be in either order so we have to check both cases
	if ( ( (bodyA == m_bodyA) && (bodyB == b) ) ||
		 ( (bodyB == m_bodyA) && (bodyA == b) ) )
	{
		hkResponseModifier::setImpulseScalingForContact( &event.m_internalContactMgr, bodyA, bodyB, *event.m_collisionOutput.m_constraintOwner, m_forceScale, m_maxAcceleration );
	}
}

// The hkCollisionListener interface implementation
void hkSoftContactUtil::contactProcessCallback( hkContactProcessEvent& event)
{
}

void hkSoftContactUtil::entityDeletedCallback( hkEntity* entity )
{
	HK_ASSERT2(0x76abe9fb, entity == m_bodyA, "hkSoftContactUtil received an unexpected entity deleted callback");
	entity->removeCollisionListener( this );
	entity->removeEntityListener( this );
	m_bodyA = HK_NULL;
	m_bodyB = HK_NULL;

	this->removeReference();
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
