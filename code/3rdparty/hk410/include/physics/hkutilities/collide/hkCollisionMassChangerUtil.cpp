/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkutilities/collide/hkCollisionMassChangerUtil.h>
#include <hkdynamics/collide/hkResponseModifier.h>
#include <hkdynamics/entity/hkRigidBodyCinfo.h>
#include <hkdynamics/entity/hkRigidBody.h>

	/// Will call setRelativeMasses() in hkResponseModifier
	/// this allows an object to interact differently with one object than with another
	/// (ie it may appear to be fixed to one object, and not to another)
	/// This is an entity listener, with 2 rigidbodies used to decide when to 
	/// change the mass and what to change it to. You may implement a world listener
	/// and a look-up table to change the response of any object in contact with any
	/// other object.
hkCollisionMassChangerUtil::hkCollisionMassChangerUtil( hkRigidBody* bodyA, hkRigidBody* bodyB, float inverseMassA, float inverseMassB )
{
	m_bodyA = bodyA;
	m_bodyB = bodyB;

	m_inverseMassA = inverseMassA;
	m_inverseMassB = inverseMassB;

	m_bodyA->addCollisionListener( this );
	m_bodyA->addEntityListener( this );

	this->addReference();
}

hkCollisionMassChangerUtil::~hkCollisionMassChangerUtil()
{
	if(m_bodyA)
	{
		m_bodyA->removeCollisionListener( this );
		m_bodyA->removeEntityListener( this );
	}
}


void hkCollisionMassChangerUtil::contactPointAddedCallback( hkContactPointAddedEvent& event )
{
	if ( static_cast<hkEntity*>(event.m_bodyA.getRootCollidable()->getOwner())->getMaterial().getResponseType() != hkMaterial::RESPONSE_SIMPLE_CONTACT )
	{
		return;
	}
	if ( static_cast<hkEntity*>(event.m_bodyB.getRootCollidable()->getOwner())->getMaterial().getResponseType() != hkMaterial::RESPONSE_SIMPLE_CONTACT )
	{
		return;
	}

	hkRigidBody* bodyA = static_cast<hkRigidBody*>(event.m_bodyA.getRootCollidable()->getOwner());
	hkRigidBody* bodyB = static_cast<hkRigidBody*>(event.m_bodyB.getRootCollidable()->getOwner());

	// The bodies could be in either order so we have to check both cases
	if ( ( ( bodyA == m_bodyA ) && (bodyB == m_bodyB ) ) ||
		( ( bodyB == m_bodyA) && (bodyA == m_bodyB ) ) )
	{
		hkResponseModifier::setInvMassScalingForContact( &event.m_internalContactMgr, m_bodyA, m_bodyB, *event.m_collisionOutput.m_constraintOwner, m_inverseMassA, m_inverseMassB );
	}
}

// The hkCollisionListener interface implementation
void hkCollisionMassChangerUtil::contactProcessCallback( hkContactProcessEvent& event)
{
}

void hkCollisionMassChangerUtil::entityDeletedCallback( hkEntity* entity )
{
	HK_ASSERT2(0x76abe9fb, entity == m_bodyA, "hkCollisionMassChangerUtil received an unexpected entity deleted callback");
	entity->removeCollisionListener( this );
	entity->removeEntityListener( this );
	m_bodyA = HK_NULL;
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
