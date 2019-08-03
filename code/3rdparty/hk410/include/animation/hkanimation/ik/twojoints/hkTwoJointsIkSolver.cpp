/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkanimation/hkAnimation.h>

#include <hkanimation/ik/twojoints/hkTwoJointsIkSolver.h>

#include <hkanimation/rig/hkSkeleton.h>
#include <hkanimation/rig/hkSkeletonUtils.h>
#include <hkanimation/rig/hkPose.h>

#include <hkbase/memory/hkLocalArray.h>
#include <hkbase/monitor/hkMonitorStream.h>
#include <hkbase/class/hkTypeInfo.h>

#define IN_RANGE(x,a,b) (((x)>=(a))&&((x)<=(b)))

hkBool hkTwoJointsIkSolver::solve( const hkTwoJointsIkSolver::Setup& setup, hkPose& poseInOut )
{

	HK_TIMER_BEGIN_LIST("TWOJOINTSOLVER","INIT");

#ifdef HK_DEBUG
	// In debug, check the validity of the setup
	{
		const hkSkeleton* skeleton = poseInOut.getSkeleton();

		HK_ASSERT2 (0x27360497, IN_RANGE(setup.m_firstJointIdx, 0 , skeleton->m_numBones-1), "Invalid setup for two joints solver.");
		HK_ASSERT2 (0x27360497, IN_RANGE(setup.m_secondJointIdx, 0 , skeleton->m_numBones-1), "Invalid setup for two joints solver.");
		HK_ASSERT2 (0x27360497, IN_RANGE(setup.m_endBoneIdx, 0 , skeleton->m_numBones-1), "Invalid setup for two joints solver.");
		HK_ASSERT2 (0x27360497, setup.m_hingeAxisLS.isNormalized3(), "Invalid setup for two joints solver.");
		HK_ASSERT2 (0x27360497, IN_RANGE(setup.m_cosineMaxHingeAngle, -1.0f, 1.0f), "Invalid setup for two joints solver.");
		HK_ASSERT2 (0x27360497, IN_RANGE(setup.m_cosineMinHingeAngle, -1.0f, 1.0f), "Invalid setup for two joints solver.");
		HK_ASSERT2 (0x27360497, IN_RANGE(setup.m_firstJointIkGain, 0.0f, 1.0f), "Invalid setup for two joints solver.");
		HK_ASSERT2 (0x27360497, IN_RANGE(setup.m_secondJointIkGain, 0.0f, 1.0f), "Invalid setup for two joints solver.");

		// Check that knee parents the end and the thigh parents the knee
		int secondDepth = -1;
		int firstDepth = -1;
		int depth=0;
		hkInt16 currentParent = setup.m_endBoneIdx;
		while ( currentParent!=-1 )
		{
			if (currentParent == setup.m_secondJointIdx)
			{
				secondDepth = depth;
			}
			if (currentParent == setup.m_firstJointIdx)
			{
				firstDepth = depth;
			}

			currentParent = skeleton->m_parentIndices[currentParent];
			depth++;
		}
		HK_ASSERT2(0x7c0bcab5, (secondDepth>0) && (firstDepth>0) && (secondDepth<firstDepth), "Inconsistent setup (order of joints/bones incorrect)");
	}
#endif


	HK_TIMER_SPLIT_LIST("FIXLENGTH");

	// Try to reach the proper distance G by extending the hinge joint (knee and hip)
	{

		const hkVector4& firstJointPosMS = poseInOut.getBoneModelSpace(setup.m_firstJointIdx).getTranslation();
		const hkVector4& secondJointPosMS = poseInOut.getBoneModelSpace(setup.m_secondJointIdx).getTranslation();
		const hkVector4& endPosMS = poseInOut.getBoneModelSpace(setup.m_endBoneIdx).getTranslation();

		hkVector4 v1; v1.setSub4(secondJointPosMS, firstJointPosMS);
		hkVector4 v2; v2.setSub4(endPosMS, secondJointPosMS);
		hkVector4 iToG; iToG.setSub4(setup.m_endTargetMS, firstJointPosMS);

		const hkReal D_sqr = iToG.lengthSquared3();

		const hkReal l1_sqr = v1.lengthSquared3();
		const hkReal l2_sqr = v2.lengthSquared3();
		const hkReal l1 = hkMath::sqrt(l1_sqr);
		const hkReal l2 = hkMath::sqrt(l2_sqr);

		HK_ASSERT2(0x3af21fc6, setup.m_cosineMaxHingeAngle < setup.m_cosineMinHingeAngle, "Limit angles for secondJoint are inconsistent");
		HK_ASSERT2(0x3af21fc6, setup.m_cosineMaxHingeAngle >= -1.0f, "Limit angles for secondJoint are inconsistent");
		HK_ASSERT2(0x3af21fc6, setup.m_cosineMinHingeAngle >= -1.0f, "Limit angles for secondJoint are inconsistent");
		HK_ASSERT2(0x3af21fc6, setup.m_cosineMaxHingeAngle <= 1.0f, "Limit angles for secondJoint are inconsistent");
		HK_ASSERT2(0x3af21fc6, setup.m_cosineMinHingeAngle <= 1.0f, "Limit angles for secondJoint are inconsistent");

		const hkReal cosAngle = hkMath::max2(setup.m_cosineMaxHingeAngle, hkMath::min2(setup.m_cosineMinHingeAngle,(D_sqr - l1_sqr - l2_sqr) / (-2.0f*l1*l2)));
		const hkReal desiredHingeAngle = hkMath::acos(cosAngle);

		v1.normalize3(); v1.setNeg3( v1 );
		v2.normalize3();

		const hkReal currentHingeAngle = hkMath::acos(v1.dot3(v2));

		const hkReal diffAngle = desiredHingeAngle - currentHingeAngle;

		// Add the desired extra hinge rotation to secondJoint
		// Rotate secondJoint
		{
			hkQsTransform& refHingeMS = poseInOut.accessBoneModelSpace(setup.m_secondJointIdx, hkPose::PROPAGATE);
			const hkQuaternion currentRot = refHingeMS.getRotation(); // we copy to avoid aliasing
			hkVector4 axis; axis.setRotatedDir(currentRot, setup.m_hingeAxisLS);

			hkQuaternion extra (axis, diffAngle * setup.m_secondJointIkGain);
			refHingeMS.m_rotation.setMul(extra, currentRot);
			refHingeMS.m_rotation.normalize();
		}


	}

	HK_TIMER_SPLIT_LIST("FIXDIRECTION");

	// Rotate the whole leg so ItoE is aligned with ItoG
	{
		const hkVector4& initMS = poseInOut.getBoneModelSpace(setup.m_firstJointIdx).getTranslation();  // I
		const hkVector4& endMS = poseInOut.getBoneModelSpace(setup.m_endBoneIdx).getTranslation();  // E
		const hkVector4& goalMS = setup.m_endTargetMS; // G

		hkVector4 iToE_n; iToE_n.setSub4(endMS, initMS); iToE_n.normalize3();
		hkVector4 iToG_n; iToG_n.setSub4(goalMS, initMS); iToG_n.normalize3();

		// Now, calculate the rotation required to reach that direction
		hkQuaternion extra;
		extra.setShortestRotationDamped( setup.m_firstJointIkGain, iToE_n, iToG_n);

		// Rotate firstJoint
		{
			hkQsTransform& refFirstJointMS = poseInOut.accessBoneModelSpace(setup.m_firstJointIdx, hkPose::PROPAGATE);
			const hkQuaternion currentRot = refFirstJointMS.getRotation(); // we copy to avoid aliasing
			refFirstJointMS.m_rotation.setMul(extra, currentRot);
			refFirstJointMS.m_rotation.normalize();
		}
	}

	HK_TIMER_END_LIST()

	return true;
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
