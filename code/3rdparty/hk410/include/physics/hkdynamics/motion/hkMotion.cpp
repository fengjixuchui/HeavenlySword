/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>
#include <hkdynamics/motion/hkMotion.h>
#include <hkconstraintsolver/solve/hkSolverInfo.h>
#include <hkmath/linear/hkSweptTransformUtil.h>


hkMotion::hkMotion(const hkVector4& position, const hkQuaternion& rotation, bool wantDeactivation)
{
	m_linearVelocity.setZero4();
	m_angularVelocity.setZero4();

	m_motionState.initMotionState( position, rotation );

	m_motionState.m_linearDamping = 0.0f;
	m_motionState.m_angularDamping = 0.0f;
	m_type = MOTION_INVALID;

	// deactivation data
	{
		if ( wantDeactivation)
		{
			m_deactivationIntegrateCounter = hkInt8(0xf & int(position(0)));
		}
		else
		{
			m_deactivationIntegrateCounter = 0xff;
		}
		m_deactivationNumInactiveFrames[0] = 0;
		m_deactivationNumInactiveFrames[1] = 0;
		m_deactivationRefPosition[0].setZero4();
		m_deactivationRefPosition[1].setZero4();
		m_motionState.m_deactivationRefOrientation[0] = 0;
		m_motionState.m_deactivationRefOrientation[1] = 0;
	}
}

// Set the mass of the rigid body.
void hkMotion::setMass(hkReal mass)
{
	hkReal massInv;
	if (mass == 0.0f)
	{
		massInv = 0.0f;
	}
	else
	{
		massInv = 1.0f / mass;
	}
	setMassInv( massInv );
}

	// Get the mass of the rigid body.
hkReal hkMotion::getMass() const
{
	hkReal massInv = getMassInv();
	if (massInv == 0)
	{
		return 0;
	}
	else
	{
		return 1 / massInv;
	}
}

// Set the mass of the rigid body.
void hkMotion::setMassInv(hkReal massInv)
{
	m_inertiaAndMassInv(3) = massInv;
}

// Explicit center of mass in local space.
void hkMotion::setCenterOfMassInLocal(const hkVector4& centerOfMass)
{	
	hkSweptTransformUtil::setCentreOfRotationLocal( centerOfMass, m_motionState );
}


void hkMotion::setPosition(const hkVector4& position)
{
	hkSweptTransformUtil::warpToPosition( position, m_motionState );
}

void hkMotion::setRotation(const hkQuaternion& rotation)
{
	hkSweptTransformUtil::warpToRotation( rotation, m_motionState);
}

void hkMotion::setPositionAndRotation(const hkVector4& position, const hkQuaternion& rotation)
{
	hkSweptTransformUtil::warpTo( position, rotation, m_motionState );
}

void hkMotion::setTransform(const hkTransform& transform)
{
	hkSweptTransformUtil::warpTo( transform, m_motionState );
}

void hkMotion::approxTransformAt( hkTime time, hkTransform& transformOut )
{
	getMotionState()->getSweptTransform().approxTransformAt( time, transformOut );
}

void hkMotion::setLinearVelocity(const hkVector4& newVel)
{
	HK_ASSERT2(0xf093fe57, newVel.isOk3(), "Invalid Linear Velocity");
	m_linearVelocity = newVel;
}

void hkMotion::setAngularVelocity(const hkVector4& newVel)
{
	HK_ASSERT2(0xf093fe56, newVel.isOk3(), "Invalid Angular Velocity");
	m_angularVelocity = newVel;
}

void hkMotion::applyLinearImpulse(const hkVector4& imp)
{
	// PSEUDOCODE IS m_linearVelocity += m_massInv * imp;
	m_linearVelocity.addMul4( getMassInv(), imp);
}

void hkMotion::getMotionStateAndVelocitiesAndDeactivationType(hkMotion* motionOut)
{
	motionOut->m_motionState = m_motionState;
	motionOut->m_linearVelocity = m_linearVelocity;	// Copy over linear velocity
	motionOut->m_angularVelocity = m_angularVelocity;	// Copy over angular velocity
	motionOut->m_deactivationIntegrateCounter = m_deactivationIntegrateCounter;
}

void hkMotion::setDeactivationClass(int deactivationClass)
{
	HK_ASSERT2( 0xf0230234, deactivationClass > 0 && deactivationClass < hkSolverInfo::DEACTIVATION_CLASSES_END, "Your deactivation class is out of range");
	m_motionState.m_deactivationClass = hkUint16(deactivationClass);
}


//HK_COMPILE_TIME_ASSERT( sizeof(hkMotion) == 0xd0 );

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
