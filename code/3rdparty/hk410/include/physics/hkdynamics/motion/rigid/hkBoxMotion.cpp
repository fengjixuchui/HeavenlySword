/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>
#include <hkdynamics/motion/rigid/hkBoxMotion.h>
#include <hkconstraintsolver/accumulator/hkVelocityAccumulator.h>
#include <hkmath/linear/hkSweptTransformUtil.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkBoxMotion);

HK_COMPILE_TIME_ASSERT( sizeof(hkBoxMotion) == sizeof(hkMotion) );
#if ( HK_POINTER_SIZE == 4 )
HK_COMPILE_TIME_ASSERT( sizeof( hkMotion) == 0x120 );
#endif


hkBoxMotion::hkBoxMotion(const hkVector4& position, const hkQuaternion& rotation)
	:hkMotion( position, rotation )
{
	m_inertiaAndMassInv.set(1,1,1,1);
	m_type = MOTION_BOX_INERTIA;
}

void hkBoxMotion::getInertiaLocal(hkMatrix3& inertia) const
{
	hkReal a = 1.0f / m_inertiaAndMassInv(0);
	hkReal b = 1.0f / m_inertiaAndMassInv(1);
	hkReal c = 1.0f / m_inertiaAndMassInv(2);
	inertia.setDiagonal(a, b, c);
}


void hkBoxMotion::setInertiaLocal(const hkMatrix3& inertia)
{

	HK_ASSERT(0x7708edc8,  inertia(0,0) >0 );
	HK_ASSERT(0x1ff66a9d,  inertia(1,1) >0 );
	HK_ASSERT(0x51ff9422,  inertia(2,2) >0 );
	hkReal a = 1.0f / inertia(0,0);
	hkReal b = 1.0f / inertia(1,1);
	hkReal c = 1.0f / inertia(2,2);

	m_inertiaAndMassInv.set( a,b,c, m_inertiaAndMassInv(3) );
}

void hkBoxMotion::getInertiaInvLocal(hkMatrix3& inertiaInv) const
{
	hkReal a = m_inertiaAndMassInv(0);
	hkReal b = m_inertiaAndMassInv(1);
	hkReal c = m_inertiaAndMassInv(2);
	inertiaInv.setDiagonal(a, b, c);
}

void hkBoxMotion::setInertiaInvLocal(const hkMatrix3& inertiaInv)
{
	m_inertiaAndMassInv.set( inertiaInv(0,0),
				 inertiaInv(1,1),
				 inertiaInv(2,2), 
				 m_inertiaAndMassInv(3) );
}

void hkBoxMotion::getInertiaInvWorld(hkMatrix3& inertiaInv) const
{
	hkMatrix3 in;

	hkVector4 a; a.setBroadcast3clobberW( m_inertiaAndMassInv, 0);
	hkVector4 b; b.setBroadcast3clobberW( m_inertiaAndMassInv, 1);
	hkVector4 c; c.setBroadcast3clobberW( m_inertiaAndMassInv, 2);
	in.getColumn(0).setMul4( a, getTransform().getColumn(0) );
	in.getColumn(1).setMul4( b, getTransform().getColumn(1) );
	in.getColumn(2).setMul4( c, getTransform().getColumn(2) );

	inertiaInv.setMulInverse( in, getTransform().getRotation() );
}

void hkBoxMotion::getInertiaWorld(hkMatrix3& inertia) const
{
	hkReal a = 1.0f / m_inertiaAndMassInv(0);
	hkReal b = 1.0f / m_inertiaAndMassInv(1);
	hkReal c = 1.0f / m_inertiaAndMassInv(2);
	inertia.setDiagonal(a, b, c);
	inertia.changeBasis( getTransform().getRotation() );
}


	// Set the mass of the rigid body.
void hkBoxMotion::setMass(hkReal mass)
{
	HK_ASSERT2(0x16462fdb,  mass > 0.0f, "If you want to set the mass to zero, use a fixed rigid body" );
	hkReal massInv = 1.f / mass;
	setMassInv( massInv );
}


void hkBoxMotion::applyPointImpulse(const hkVector4& imp, const hkVector4& p)
{
	// PSEUDOCODE IS m_linearVelocity += m_massInv * imp;
	// PSEUDOCODE IS m_angularVelocity += getWorldInertiaInv() * (p - centerOfMassWorld).cross(imp);
	m_linearVelocity.addMul4(getMassInv(), imp);

	// Can calc inertiaWorld * v, but it's faster to calc r * m_inertiaAndMassInv * r^-1 * v
	// where r is m_localToWorld.getRotation()
	hkVector4 relMassCenter; relMassCenter.setSub4( p, m_motionState.getSweptTransform().m_centerOfMass1 );
	hkVector4 crossWs; crossWs.setCross( relMassCenter, imp );

	hkVector4 crossLs; crossLs._setRotatedInverseDir( getTransform().getRotation(), crossWs);
	hkVector4 deltaVelLs; deltaVelLs.setMul4( m_inertiaAndMassInv, crossLs);
	hkVector4 deltaVelWs; deltaVelWs._setRotatedDir(getTransform().getRotation(), deltaVelLs);
	m_angularVelocity.add4(deltaVelWs);
}


void hkBoxMotion::applyAngularImpulse(const hkVector4& imp)
{
	// PSEUDOCODE IS m_angularVelocity += m_worldInertiaInv * imp;
	hkVector4 impLocal; impLocal._setRotatedInverseDir( getTransform().getRotation(), imp );
	hkVector4 dangVelLocal; dangVelLocal.setMul4( m_inertiaAndMassInv, impLocal );
	hkVector4 dangVel; dangVel._setRotatedDir( getTransform().getRotation(), dangVelLocal );
	m_angularVelocity.add4(dangVel);
}


void hkBoxMotion::applyForce( const hkReal deltaTime, const hkVector4& force)
{
	hkVector4 impulse; impulse.setMul4( deltaTime, force );
	m_linearVelocity.addMul4(getMassInv(), impulse);
}

void hkBoxMotion::applyForce( const hkReal deltaTime, const hkVector4& force, const hkVector4& p)
{
	hkVector4 impulse; impulse.setMul4( deltaTime, force );
	applyPointImpulse( impulse, p );
}


void hkBoxMotion::applyTorque( const hkReal deltaTime, const hkVector4& torque)
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
