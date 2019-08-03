/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkanimation/hkAnimation.h>

#include <hkanimation/ik/ccd/hkCcdIkSolver.h>

#include <hkanimation/rig/hkSkeleton.h>
#include <hkanimation/rig/hkSkeletonUtils.h>
#include <hkanimation/rig/hkPose.h>

#include <hkbase/memory/hkLocalArray.h>
#include <hkbase/monitor/hkMonitorStream.h>
#include <hkbase/class/hkTypeInfo.h>

hkCcdIkSolver::hkCcdIkSolver( hkInt32 iterations, hkReal gain )
: m_iterations( iterations ), m_gain( gain )
{
	
}

hkBool hkCcdIkSolver::solve( const hkArray<IkConstraint>& constraints, hkPose& poseInOut )
{
	HK_TIMER_BEGIN("CCD_IK", HK_NULL);

	const hkSkeleton* skeleton = poseInOut.getSkeleton();

	// indicates whether a chain set failed to be solved correctly
	hkBool allSolved = true;

	// how many constraint chain sets to solve for?
	const int numConstraintSets = constraints.getSize();

	// solve for each constraint chain set
	for( int i = 0; i < numConstraintSets; i++ )
	{
		const hkCcdIkSolver::IkConstraint& constraint = constraints[i];
		const hkInt16& nubBone = constraint.m_endBone;
		const hkInt16& baseBone = constraint.m_startBone;

		// find the set of bones within this chain
		hkArray<hkInt16> boneChain;
		hkSkeletonUtils::getBoneChain( *skeleton, baseBone, nubBone, boneChain );

		const int numBones = boneChain.getSize();

		if( numBones )
		{
			// remember the original position of the end effector
			hkVector4 nubBoneBegin = poseInOut.getBoneModelSpace(nubBone).getTranslation();

			for( hkInt32 iteration = 0; iteration < m_iterations; iteration++ )
			{
				// skip the nub bone
				for( hkInt16 curBone = (hkInt16)( (const hkInt16)(numBones) - 2 ) ; curBone >= 0; curBone-- )
				{
					hkInt16 bi = boneChain[curBone];

					const hkVector4& nubBoneModelSpace = poseInOut.getBoneModelSpace(nubBone).getTranslation();
					const hkVector4& curBoneModelSpace = poseInOut.getBoneModelSpace(bi).getTranslation();

					// current bone direction
					hkVector4 curDirection;
					curDirection.setSub4( nubBoneModelSpace, curBoneModelSpace );
					curDirection.normalize3();

					// target direction
					hkVector4 targetDirection;
					targetDirection.setSub4( constraint.m_targetMS, curBoneModelSpace);
					targetDirection.normalize3();

					hkQuaternion rotationQuaternion;
					rotationQuaternion.setShortestRotationDamped( m_gain, curDirection, targetDirection );

					// required transform
					const hkQuaternion& curRotation = poseInOut.getBoneModelSpace(bi).getRotation();
					hkQuaternion temp;
					temp.setMul( rotationQuaternion, curRotation );
					temp.normalize();
					poseInOut.accessBoneModelSpace(bi, hkPose::PROPAGATE).setRotation( temp ); // rotates all children

				}

			}
		}

		else
		{
			// invalid chain
			allSolved = false;
		}

	}

	HK_TIMER_END();
	// solved correctly?
	return allSolved;
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
