/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkragdoll/hkRagdoll.h>
#include <hkragdoll/posematching/hkPoseMatchingUtility.h>

// Rig
#include <hkanimation/rig/hkSkeleton.h>

// Removes any rotation around a given axis from 
static void hkRemoveTwist(const hkVector4& direction, hkQuaternion& rotationInOut)
{
	// Pick any orthogonal axis
	hkVector4 oAxis; oAxis.setZero4();
	oAxis( ( direction.getMajorAxis() + 1 ) % 3 ) = 1.0f;

	// Rotate it into the frame and construct a basis
	oAxis.setRotatedDir(rotationInOut,oAxis);

	// Construct another orthogonal axis
	hkVector4 aoAxis;
	aoAxis.setCross(oAxis, direction);
	aoAxis.normalize3();
	oAxis.setCross(direction, aoAxis);

	hkRotation r; r.setCols(oAxis, direction, aoAxis);

	// Proj represents the rotation about the twist axis
	hkQuaternion proj(r);

	hkQuaternion qInv;
	qInv.setInverse(proj);

	hkQuaternion result;
	result.setMul(qInv, rotationInOut);

	rotationInOut = result;
}

hkPoseMatchingUtility::hkPoseMatchingUtility(hkInt16 rootBoneIdx, hkInt16 otherBoneIdx, hkInt16 anotherBoneIdx, const hkVector4& rotAxis)
{
	m_rootBoneIdx = rootBoneIdx;
	m_otherBoneIdx = otherBoneIdx;
	m_anotherBoneIdx = anotherBoneIdx;
	m_rotationAxis = rotAxis;
}


void hkPoseMatchingUtility::computeOrientation( const hkQsTransform* poseModelSpace, hkQuaternion& frameOut) const
{
	const hkVector4 rootRagdoll = poseModelSpace[m_rootBoneIdx].m_translation;
	const hkVector4 otherRagdoll = poseModelSpace[m_otherBoneIdx].m_translation;
	const hkVector4 anotherRagdoll = poseModelSpace[m_anotherBoneIdx].m_translation;

	hkVector4 bodyAxis; 
	bodyAxis.setSub4( otherRagdoll, rootRagdoll);
	bodyAxis.normalize3();

	hkVector4 otherAxis; 
	otherAxis.setSub4( anotherRagdoll, rootRagdoll);
	otherAxis.normalize3();

	hkVector4 up;
	up.setCross(otherAxis, bodyAxis);
	up.normalize3();

	hkVector4 right; right.setCross(up, bodyAxis);
	right.normalize3();
	up.setCross(bodyAxis, right);

	hkRotation r;
	r.setCols(right, up, bodyAxis);
	frameOut = hkQuaternion( r );
}

int hkPoseMatchingUtility::findBestCandidatePoseIndex( const hkQsTransform* poseModelSpace , hkReal& error ) const
{
	hkQuaternion ragdollFrame;
	computeOrientation( poseModelSpace, ragdollFrame );
	hkRemoveTwist( m_rotationAxis, ragdollFrame );

	// Search for one with lowest error (largest dotp)
	int bestIndex = -1;
	error = HK_REAL_MAX;

	for (int i=0; i < m_candidatePoses.getSize(); i++)
	{
		//Compute transformation from one frame to the next
		hkReal err = 1.0f - hkMath::fabs(ragdollFrame.m_vec.dot4( m_candidatePoses[i].m_reference.m_vec ));
		if (err < error)
		{
			bestIndex = i;
			error = err;
		}
	}

	return bestIndex;
}

int hkPoseMatchingUtility::addCandidatePose(const hkQsTransform* poseModelSpace, const class hkAnimationBinding* binding, hkReal time)
{
	hkPoseMatchingUtility::CandidatePoseInfo candidate;
	candidate.m_time = time;
	candidate.m_binding = binding;

	computeOrientation( poseModelSpace, candidate.m_reference );
	hkRemoveTwist( m_rotationAxis, candidate.m_reference );

	m_candidatePoses.pushBack( candidate );

	return m_candidatePoses.getSize() - 1;
}

void hkPoseMatchingUtility::computeReferenceFrame(const hkQsTransform* animPoseModelSpace, const hkQsTransform* ragdollPoseWorldSpace, hkQsTransform& animWorldFromModel, hkQsTransform& ragdollWorldFromModel ) const
{

	hkQuaternion refFrame;
	computeOrientation( animPoseModelSpace, refFrame );

	hkQuaternion ragdollFrame;
	computeOrientation( ragdollPoseWorldSpace, ragdollFrame );

	// Construct the model space to align the ragdoll with the pose
	ragdollWorldFromModel.setIdentity();
	{
		const hkVector4& rootRagdoll = ragdollPoseWorldSpace[ m_rootBoneIdx ].m_translation;
		const hkVector4& rootRef = animPoseModelSpace[ m_rootBoneIdx ].m_translation;

		ragdollWorldFromModel.m_translation = rootRagdoll;
		ragdollWorldFromModel.m_rotation.setMulInverse( ragdollFrame, refFrame );

		hkVector4 offset;
		offset.setRotatedDir( ragdollWorldFromModel.m_rotation, rootRef);
		ragdollWorldFromModel.m_translation.sub4(offset);
	}

	// compute the new reference frame for the animation
	animWorldFromModel.setIdentity();
	{
		hkQuaternion qOut; hkReal angOut;
		animWorldFromModel = ragdollWorldFromModel;
		animWorldFromModel.m_rotation.decomposeRestAxis(m_rotationAxis, qOut, angOut);
		animWorldFromModel.m_rotation.setAxisAngle(m_rotationAxis, angOut);
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
