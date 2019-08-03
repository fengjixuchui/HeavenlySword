/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>
#include <hkdynamics/motion/rigid/hkSphereMotion.h>
#include <hkconstraintsolver/accumulator/hkVelocityAccumulator.h>
#include <hkmath/linear/hkSweptTransformUtil.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkSphereMotion);

HK_COMPILE_TIME_ASSERT( sizeof( hkSphereMotion) == sizeof( hkMotion) );

//
// INERTIA
//

	// Get the inertia tensor in local space
void hkSphereMotion::getInertiaLocal(hkMatrix3& inertia) const
{
	hkReal hkInvDiag = 1.0f / m_inertiaAndMassInv(0);
	inertia.setDiagonal(hkInvDiag, hkInvDiag, hkInvDiag);
}

	// Get the inertia tensor in local space
void hkSphereMotion::getInertiaWorld(hkMatrix3& inertia) const
{
	hkReal hkInvDiag = 1.0f/ m_inertiaAndMassInv(0);
	inertia.setDiagonal(hkInvDiag, hkInvDiag, hkInvDiag);
}


	// Sets the inertia tensor of the rigid body. Advanced use only.
void hkSphereMotion::setInertiaLocal(const hkMatrix3& inertia)
{
		// Take max value
	hkReal temp = hkMath::max2( hkMath::max2( inertia(0,0), inertia(1,1) ), inertia(2,2) );

	hkReal invInertia;
	if ( temp > 0.0f )
	{
		invInertia = 1.0f / temp;
	}
	else
	{
		HK_ASSERT2(0x7163b90d, 0, "Cannot set inertia of Oriented Particle: Diagonal of inertia tensor must have at least one positive element!");
		invInertia = 0.0f;
	}
	m_inertiaAndMassInv(0) = invInertia;
	m_inertiaAndMassInv(1) = invInertia;
	m_inertiaAndMassInv(2) = invInertia;
}


	// Sets the inertia tensor of the rigid body by supplying its inverse. Advanced use only.
void hkSphereMotion::setInertiaInvLocal(const hkMatrix3& inertiaInv)
{
	hkReal invInertia = hkMath::min2( hkMath::min2( inertiaInv(0,0), inertiaInv(1,1) ), inertiaInv(2,2) );
	m_inertiaAndMassInv(0) = invInertia;
	m_inertiaAndMassInv(1) = invInertia;
	m_inertiaAndMassInv(2) = invInertia;
}


	// Get the inverse inertia tensor in local space
void hkSphereMotion::getInertiaInvLocal(hkMatrix3& inertiaInv) const
{
	hkReal d = m_inertiaAndMassInv(0);
	inertiaInv.setDiagonal(d, d, d);
}

	// Get the inverse inertia tensor in world space
void hkSphereMotion::getInertiaInvWorld(hkMatrix3& inertiaInv) const
{
	hkReal d = m_inertiaAndMassInv(0);
	inertiaInv.setDiagonal(d, d, d);
}


//
// IMPULSE APPLICATION
//

void hkSphereMotion::applyPointImpulse(const hkVector4& imp, const hkVector4& p)
{
	// PSEUDOCODE IS m_linearVelocity += m_massInv * imp;
	// PSEUDOCODE IS m_angularVelocity += getWorldInertiaInv() * (p - centerOfMassWorld).cross(imp);
	m_linearVelocity.addMul4(getMassInv(), imp);


	// Can calc inertiaWorld * v, but it's faster to calc r * m_inertiaInv * r^-1 * v
	// where r is m_localToWorld.getRotation()
	hkVector4 relMassCenter; relMassCenter.setSub4( p, this->getCenterOfMassInWorld() );
	hkVector4 crossWs; crossWs.setCross( relMassCenter, imp );
	m_angularVelocity.addMul4( m_inertiaAndMassInv, crossWs);
}


void hkSphereMotion::applyAngularImpulse(const hkVector4& imp)
{
	// PSEUDOCODE IS m_angularVelocity += m_worldInertiaInv * imp;
	m_angularVelocity.addMul4( m_inertiaAndMassInv, imp);
}


//
// FORCE APPLICATION
//


void hkSphereMotion::applyForce( const hkReal deltaTime, const hkVector4& force)
{
	hkVector4 impulse; impulse.setMul4( deltaTime, force );
	m_linearVelocity.addMul4(getMassInv(), impulse);
}

void hkSphereMotion::applyForce( const hkReal deltaTime, const hkVector4& force, const hkVector4& p)
{
	hkVector4 impulse; impulse.setMul4( deltaTime, force );
	applyPointImpulse( impulse, p );
}


void hkSphereMotion::applyTorque( const hkReal deltaTime, const hkVector4& torque)
{
	hkVector4 impulse; impulse.setMul4( deltaTime, torque );
	applyAngularImpulse( impulse );
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
