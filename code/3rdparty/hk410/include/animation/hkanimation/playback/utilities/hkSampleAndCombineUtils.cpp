/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkanimation/hkAnimation.h>
#include <hkanimation/playback/utilities/hkSampleAndCombineUtils.h>

void HK_CALL hkSampleAndCombineUtils::blendNormal(BlendParameters& paramsIn, hkQsTransform* accumulationPoseInOut, hkReal* perBoneWeights)
{
	int maxTrackIdx = paramsIn.m_numAnimationTransforms;

	if (paramsIn.m_trackToBoneMapping)
	{
		for (int trackIdx=0 ; trackIdx <= maxTrackIdx; trackIdx++)
		{
			// Grab the animation track this bone is bound to
			hkInt16 boneIdx = paramsIn.m_trackToBoneMapping[ trackIdx ];

			// If we have an invalid index then skip this track
			if ((boneIdx < 0) || ((hkUint16)boneIdx >= paramsIn.m_numBones))
			{
				continue;
			}

			// Grab the track and blend
			hkReal maskedWeight = paramsIn.m_masterWeight;
			maskedWeight *= paramsIn.m_perBoneWeights ? paramsIn.m_perBoneWeights[trackIdx] * (1.0f/255.0f): 1.0f;

			accumulationPoseInOut[ boneIdx ].blendAddMul( paramsIn.m_animationTransformsIn[ trackIdx ], maskedWeight);
			// Accumulate weight for this bone
			perBoneWeights[ boneIdx ]+= maskedWeight;
		}
	}
	else
	{
		// If there's no mapping, we assume that trackIdx = boneIdx
		for (int trackIdx=0 ; trackIdx <= maxTrackIdx; trackIdx++)
		{
			hkReal maskedWeight = paramsIn.m_masterWeight;
			maskedWeight *= paramsIn.m_perBoneWeights ? paramsIn.m_perBoneWeights[trackIdx] : 1.0f;

			accumulationPoseInOut[ trackIdx ].blendAddMul( paramsIn.m_animationTransformsIn[ trackIdx ], maskedWeight);
			// Accumulate weight for this bone
			if(perBoneWeights)
			{
				perBoneWeights[ trackIdx ]+= maskedWeight;
			}
		}
	}
}

void HK_CALL hkSampleAndCombineUtils::blendAdditive(BlendParameters& paramsIn, hkQsTransform* accumulationPoseInOut)
{
	int numTracks = paramsIn.m_numAnimationTransforms;

	for (int trackIdx=0 ; trackIdx <= numTracks; trackIdx++)
	{
		// Grab the animation track this bone is bound to
		hkInt16 boneIdx = paramsIn.m_trackToBoneMapping[ trackIdx ];

		// If we have an invalid index then skip this track
		if ((boneIdx < 0) || ((hkUint16)boneIdx >= paramsIn.m_numBones))
		{
			continue;
		}

		// Grab the track and blend
		hkReal maskedWeight = paramsIn.m_masterWeight;
		maskedWeight *= paramsIn.m_perBoneWeights ? paramsIn.m_perBoneWeights[trackIdx] * (1.0f/255.0f): 1.0f;

		hkQsTransform additiveResult;
		additiveResult.setMul( paramsIn.m_animationTransformsIn[ trackIdx ], accumulationPoseInOut[ boneIdx ] );

		// Blend
		accumulationPoseInOut[ boneIdx ].setInterpolate4( accumulationPoseInOut[ boneIdx ], additiveResult, maskedWeight );
	}
}

// Find the track index we need to sample to to guarantee we cover numBones.
hkUint32 hkSampleAndCombineUtils::getMaxTrack(const hkInt16* trackToBoneMapping, const hkUint8* perBoneWeights, hkUint32 numBones, hkUint32 numTracks)
{
	if (!trackToBoneMapping)
	{
		return numBones -1 ;
	}

	hkUint32 maxTrack = hkUint32(-1);

	// In reverse
	for (int maxTrack = numTracks-1; maxTrack >=0 ; --maxTrack)
	{
		// Grab the animation track this bone is bound to
		hkInt16 boneIdx = trackToBoneMapping[ maxTrack ];

		// Find the first non-zero weight track that we need to sample
		if ( (boneIdx >= 0) && ( hkUint32(boneIdx) < numBones ) && ( (!perBoneWeights) || (perBoneWeights[ maxTrack ] > HK_REAL_EPSILON) ) )
			return maxTrack;

	}

	return maxTrack;
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
