/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkanimation/hkAnimation.h>

#include <hkanimation/ik/lookat/hkLookAtIkSolver.h>

hkBool hkLookAtIkSolver::solve ( const Setup& setup, const hkVector4& targetMS, hkReal gain, hkQsTransform& boneModelSpaceInOut )
{
	const hkVector4& bonePosMS = boneModelSpaceInOut.getTranslation();

	// Calc vector to target in model space
	hkVector4 vecToTargetMS;
	vecToTargetMS.setSub4( targetMS, bonePosMS );
	vecToTargetMS.normalize3();

	hkBool insideLimits = true;

	// Limit the vecToTargetMS
	hkReal cosLimitAngle = hkMath::cos( setup.m_limitAngle );
	hkReal cosTargetAngle = vecToTargetMS.dot3( setup.m_limitAxisMS );
	if ( cosTargetAngle < cosLimitAngle )
	{
		hkVector4 cross;
		cross.setCross(  setup.m_limitAxisMS, vecToTargetMS );
		cross.normalize3();

		hkQuaternion q;
		q.setAxisAngle( cross, setup.m_limitAngle );

//		hkVector4 output;
		vecToTargetMS.setRotatedDir( q,  setup.m_limitAxisMS );

		insideLimits = false;
	}

	// Transform the model space of the bone
	{
		hkVector4 fwdMS;
		fwdMS.setRotatedDir( boneModelSpaceInOut.getRotation(), setup.m_fwdLS);

		hkVector4 cross;
		cross.setCross( fwdMS, vecToTargetMS );
		cross.normalize3();

		hkReal cosAngle = vecToTargetMS.dot3( fwdMS );

		const hkReal angle = hkMath::acos(cosAngle) * gain;

		hkQuaternion q;
		q.setAxisAngle( cross, angle );

		hkQuaternion newRotation; newRotation. setMul( q, boneModelSpaceInOut.getRotation() );

		boneModelSpaceInOut.m_rotation = newRotation;
	}

	return insideLimits;
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
