/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

hkMotionState* hkMotion::getMotionState() 
{
	return &m_motionState;
}

const hkMotionState* hkMotion::getMotionState() const
{
	return &m_motionState;
}

// Get the mass inv of the rigid body.
hkSimdReal hkMotion::getMassInv() const
{
	return m_inertiaAndMassInv.getSimdAt(3);
}


const hkVector4& hkMotion::getCenterOfMassLocal() const
{
	return m_motionState.getSweptTransform().m_centerOfMassLocal;
}

const hkVector4& hkMotion::getCenterOfMassInWorld() const
{
	return m_motionState.getSweptTransform().m_centerOfMass1;
}


// Return the position (Local Space origin) for this rigid body, in World space.
// Note: The center of mass is no longer necessarily the local space origin
const hkVector4& hkMotion::getPosition() const
{
	return m_motionState.getTransform().getTranslation();
}

// Returns the rotation from Local to World space for this rigid body.
const hkQuaternion& hkMotion::getRotation() const
{
	return m_motionState.getSweptTransform().m_rotation1;
}

// Returns the rigid body (local) to world transformation.
const hkTransform& hkMotion::getTransform() const
{
	return m_motionState.getTransform();
}




/*
** VELOCITY ACCESS
*/

const hkVector4& hkMotion::getLinearVelocity() const
{
	return m_linearVelocity;
}

const hkVector4& hkMotion::getAngularVelocity() const
{
	return m_angularVelocity;
}


void hkMotion::getPointVelocity(const hkVector4& p, hkVector4& pointVelOut) const
{
	// CODE IS getLinearVelocity() + getAngularVelocity().cross(p - centerOfMassWorld);
	hkVector4 relPos; relPos.setSub4( p, getCenterOfMassInWorld() );
	pointVelOut.setCross( m_angularVelocity, relPos);
	pointVelOut.add4( m_linearVelocity );
}



/*
** DAMPING
*/

hkReal hkMotion::getLinearDamping()
{
	return m_motionState.m_linearDamping;
}

hkReal hkMotion::getAngularDamping()
{
	return m_motionState.m_angularDamping;
}

void hkMotion::setLinearDamping( hkReal d )
{
	m_motionState.m_linearDamping = d;
}


void hkMotion::setAngularDamping( hkReal d )
{
	m_motionState.m_angularDamping = d;
}

/*
*	SOLVER LEVEL DEACTIVATION
*/

int hkMotion::getDeactivationClass()
{
	return m_motionState.m_deactivationClass;
}

int hkMotion::getDeactivationCounter()
{
	return m_motionState.m_deactivationCounter;
}

void hkMotion::setDeactivationCounter( int newCounter )
{
	m_motionState.m_deactivationCounter = hkUint16(newCounter);
}

void hkMotion::enableDeactivation( bool value, int randomNumber)
{
	if ( value )
	{
		m_deactivationIntegrateCounter = hkUint8(0xf & randomNumber);
	}
	else
	{
		m_deactivationIntegrateCounter = 0xff;
		m_deactivationNumInactiveFrames[0] = 0;
		m_deactivationNumInactiveFrames[1] = 0;
	}
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
