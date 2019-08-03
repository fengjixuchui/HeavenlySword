/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>

#include <hkdynamics/entity/hkSpatialRigidBodyDeactivator.h>
#include <hkdynamics/entity/hkRigidBody.h>

#include <hkcollide/shape/hkShape.h>
#include <hkmath/basetypes/hkAabb.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkSpatialRigidBodyDeactivator);

hkSpatialRigidBodyDeactivator::hkSpatialRigidBodyDeactivator()
{
	m_radiusSqrd = -1.0f; // this is not used
	m_minHighFrequencyTranslation = 0.01f;
	m_minHighFrequencyRotation = 0.005f;
	m_minLowFrequencyTranslation = 0.1f;
	m_minLowFrequencyRotation = 0.2f;

	m_highFrequencySample.m_refPosition.setAll( HK_REAL_MAX );
	m_lowFrequencySample.m_refPosition.setAll( HK_REAL_MAX );
	m_highFrequencySample.m_refRotation.setIdentity();
	m_lowFrequencySample.m_refRotation.setIdentity();
}

//hkReal hkSpatialRigidBodyDeactivator::getApproxRadiusSqrdOfShape(const hkShape* shape)
//{
//	hkReal radSqrd;
//	if ( shape != HK_NULL )
//	{
//		hkAabb aabb;
//		shape->getAabb( hkTransform::getIdentity(), 0, aabb );
//
//		hkVector4 diag;
//		diag.setSub4( aabb.m_max, aabb.m_min );
//		diag.mul4( 0.5f );
//		radSqrd = diag.lengthSquared3();
//	}
//	else
//	{
//		radSqrd = -1;
//	}
//	return radSqrd;
//}


enum hkRigidBodyDeactivator::DeactivatorType hkSpatialRigidBodyDeactivator::getRigidBodyDeactivatorType() const 
{ 
	return DEACTIVATOR_SPATIAL;
}

static inline hkReal HK_CALL rotationLengthSquared(const hkQuaternion& a, const hkQuaternion& b)
{
	// We do a subtract  and dot product. just as we would for 3D vectors
	hkVector4 imagDiff;
	imagDiff.setSub4( a.getImag(), b.getImag() );
	hkReal ls3 = imagDiff.lengthSquared4();
	return ls3;
}

hkBool hkSpatialRigidBodyDeactivator::shouldDeactivateHighFrequency( const hkEntity* entity )
{
	const hkMotionState* motionState = static_cast<const hkRigidBody*>( entity )->getRigidMotion()->getMotionState();
	const hkSweptTransform& sweptTransform = motionState->getSweptTransform();

	hkReal     radius          = motionState->m_objectRadius;
	const hkVector4& currentPosition = sweptTransform.m_centerOfMass1;
	const hkQuaternion& currentRotation = sweptTransform.m_rotation1;

	HK_ASSERT2(0x5c4d7c12,  radius > 0, "Radius was not set correctly for entity " << entity);

	hkVector4 transDist;
	transDist.setSub4( m_highFrequencySample.m_refPosition, currentPosition );
	const hkReal distSqrd = transDist.lengthSquared3();

	const hkReal rotSqrd = rotationLengthSquared( m_highFrequencySample.m_refRotation, currentRotation ) * radius * radius;

	if ( ( distSqrd < m_minHighFrequencyTranslation * m_minHighFrequencyTranslation) &&
	 	 ( rotSqrd  < m_minHighFrequencyRotation * m_minHighFrequencyRotation      ) )
	{
		return true;
	}
	else
	{
		// Not deactivating - Take a sample
		m_highFrequencySample.m_refPosition = currentPosition;
		m_highFrequencySample.m_refRotation = currentRotation;

		return false;
	}
}

hkBool hkSpatialRigidBodyDeactivator::shouldDeactivateLowFrequency( const hkEntity* entity )
{
	const hkMotionState* motionState = static_cast<const hkRigidBody*>( entity )->getRigidMotion()->getMotionState();
	const hkSweptTransform& sweptTransform = motionState->getSweptTransform();

	hkReal     radius          = motionState->m_objectRadius;
	const hkVector4& currentPosition = sweptTransform.m_centerOfMass1;
	const hkQuaternion& currentRotation = sweptTransform.m_rotation1;

	HK_ASSERT2(0x32d7e69c,  radius > 0, "Radius was not set correctly for entity " << entity);

	hkVector4 transDist;
	transDist.setSub4( m_lowFrequencySample.m_refPosition, currentPosition );
	const hkReal distSqrd = transDist.lengthSquared3();
	const hkReal rotSqrd = rotationLengthSquared( m_lowFrequencySample.m_refRotation, currentRotation ) * radius * radius;

	if ( ( distSqrd < m_minLowFrequencyTranslation * m_minLowFrequencyTranslation) &&
		 ( rotSqrd  < m_minLowFrequencyRotation * m_minLowFrequencyRotation ) )
	{
		return true;
	}
	else
	{
		// Not deactivating - Take a sample
		m_lowFrequencySample.m_refPosition = currentPosition;
		m_lowFrequencySample.m_refRotation = currentRotation;

		return false;
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
