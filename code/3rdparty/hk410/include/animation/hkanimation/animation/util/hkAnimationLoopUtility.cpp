/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkanimation/hkAnimation.h>
#include <hkanimation/animation/util/hkAnimationLoopUtility.h>
#include <hkanimation/animation/interleaved/hkInterleavedSkeletalAnimation.h>
#include <hkanimation/rig/hkSkeletonUtils.h>
#include <hkbase/memory/hkLocalBuffer.h>

// We blend the start and end animation 
void hkAnimationLoopUtility::crossFadeLoop( int numFrames, hkInterleavedSkeletalAnimation* animInOut )
{
	HK_ASSERT2(0x54e454a2, numFrames < (animInOut->getNumOriginalFrames() / 2), "Too many frames for cross fade. Number should be less than half the total animation length");
	
	const hkReal blendDelta = 0.5f / numFrames;

	hkLocalBuffer<hkQsTransform> startPose( animInOut->m_numberOfTracks );
	hkLocalBuffer<hkQsTransform> endPose( animInOut->m_numberOfTracks );

	for (int frame=0; frame < numFrames; frame++)
	{
		hkReal crossfade = blendDelta * frame + 0.5f; // range [0.5..1.0)
		
		// Grab copies of the tracks to blend
		int startIndex = animInOut->m_numberOfTracks * frame;
		int endIndex   = animInOut->m_numTransforms - animInOut->m_numberOfTracks * (frame + 1);
		hkString::memCpy( startPose.begin(), &animInOut->m_transforms[startIndex],  animInOut->m_numberOfTracks * sizeof( hkQsTransform ));
		hkString::memCpy( endPose.begin(), &animInOut->m_transforms[endIndex], animInOut->m_numberOfTracks * sizeof( hkQsTransform ));
		
		//	Blend
		hkSkeletonUtils::blendPoses( animInOut->m_numberOfTracks, endPose.begin(), startPose.begin(), crossfade, &animInOut->m_transforms[startIndex]);
		hkSkeletonUtils::blendPoses( animInOut->m_numberOfTracks, startPose.begin(), endPose.begin(), crossfade, &animInOut->m_transforms[endIndex]);
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
