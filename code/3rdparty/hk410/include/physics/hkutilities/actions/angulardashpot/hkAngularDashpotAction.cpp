/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkutilities/actions/angulardashpot/hkAngularDashpotAction.h>
#include <hkdynamics/entity/hkRigidBody.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkAngularDashpotAction);

hkAngularDashpotAction::hkAngularDashpotAction(hkRigidBody* entityA, hkRigidBody* entityB, hkUint32 userData) 
: hkBinaryAction( entityA, entityB, userData ),
  m_strength( 0.1f),
  m_damping(0.01f)
{
	m_rotation.setIdentity();
}

void hkAngularDashpotAction::applyAction(const hkStepInfo& stepInfo)
{
	const hkReal dtscale = 200; // to keep constants sensible around 1
	const hkReal dt = dtscale * (stepInfo.m_deltaTime);

	hkRigidBody* rbA = static_cast<hkRigidBody*>( m_entityA );
	hkRigidBody* rbB = static_cast<hkRigidBody*>( m_entityB );
	HK_ASSERT2(0xf668efca, rbA && rbB, "Bodies not set in angular dashpot.");

	hkQuaternion q;
	q.setMul(rbB->getRotation(),m_rotation);

	hkQuaternion inv;
	inv.setInverse(q);

	hkQuaternion rrot;
	rrot.setMul(rbA->getRotation(),inv);

	hkVector4 avel;
	avel.setSub4(rbA->getAngularVelocity(),rbB->getAngularVelocity());

 	const hkReal angle = rrot.getAngle();
	hkVector4 axis; axis.setZero4();


	if( hkMath::fabs(angle) > 0.001f)
	{
		axis.setMul4(angle, rrot.getImag());
	}

	axis.mul4(dt * m_strength);
	avel.mul4(dt * m_damping);

	hkVector4 impulse;
	impulse.setAdd4(axis,avel);

	rbB->applyAngularImpulse( impulse);
	impulse.setNeg4( impulse );
	rbA->applyAngularImpulse(impulse);

}

// hkAction clone interface
hkAction* hkAngularDashpotAction::clone( const hkArray<hkEntity*>& newEntities, const hkArray<hkPhantom*>& newPhantoms ) const
{
	HK_ASSERT2(0xf568efca, newEntities.getSize() == 2, "Wrong clone parameters given to a spring action (needs 2 bodies).");
	// should have two entities as we are a binary action.
	if (newEntities.getSize() != 2) return HK_NULL;

	HK_ASSERT2(0x392f03ac, newPhantoms.getSize() == 0, "Wrong clone parameters given to a spring action (needs 0 phantoms).");
	// should have no phantoms.
	if (newPhantoms.getSize() != 0) return HK_NULL;

	hkAngularDashpotAction* ada = new hkAngularDashpotAction( (hkRigidBody*)newEntities[0], (hkRigidBody*)newEntities[1], m_userData );
	ada->m_rotation = m_rotation;
	ada->m_strength = m_strength;
	ada->m_damping = m_damping;

	return ada;
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
