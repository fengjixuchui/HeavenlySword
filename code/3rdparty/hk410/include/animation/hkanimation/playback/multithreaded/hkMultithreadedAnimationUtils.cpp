/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkanimation/hkAnimation.h>
#include <hkanimation/rig/hkSkeleton.h>
#include <hkanimation/playback/hkAnimatedSkeleton.h>
#include <hkanimation/playback/control/hkAnimationControl.h>
#include <hkanimation/playback/multithreaded/hkMultithreadedAnimationUtils.h>

void hkMultithreadedAnimationUtils::allocateSampleAndCombineJob(const hkAnimatedSkeleton* animSkel, hkAnimationSampleAndCombineJob& jobOut)
{
	// Control data
	int activeControls = 0;
	for (int i=0 ; i < animSkel->getNumAnimationControls(); i++)
	{
		hkAnimationControl* control = animSkel->getAnimationControl(i);

		// Only use controls that have a non zero weight
		if (control->getWeight() > 0.0f )
		{
			activeControls++;	
		}
	}

	jobOut.m_animationControls = static_cast<hkAnimationSampleAndCombineJob::ControlData*> ( hkThreadMemory::getInstance().allocate( sizeof(hkAnimationSampleAndCombineJob::ControlData) * activeControls, HK_MEMORY_CLASS_ANIM_RUNTIME ) );
}

void hkMultithreadedAnimationUtils::deallocateSampleAndCombineJob( hkAnimationSampleAndCombineJob& jobOut)
{
	hkThreadMemory::getInstance().deallocate( jobOut.m_animationControls );
}

void hkMultithreadedAnimationUtils::createSampleAndCombineJob(const hkAnimatedSkeleton* animSkel, hkUint32 maxBone, hkQsTransform* poseOut, hkAnimationSampleAndCombineJob& jobOut)
{
	// Inputs
	{
		jobOut.m_referencePose = animSkel->getSkeleton()->m_referencePose;
		jobOut.m_referencePoseWeightThreshold = animSkel->getReferencePoseWeightThreshold();
		jobOut.m_maxBone = maxBone;
	}

	// Control data
	{
		HK_ASSERT2( 0x54e342e3,jobOut.m_animationControls, "Animation control data is not allocated" );

		hkUint32& controlIndex = jobOut.m_numAnimationControls = 0;

		// Sort controls normal first and additive later
		for (int blendType = hkAnimationBinding::NORMAL; blendType <= hkAnimationBinding::ADDITIVE; blendType ++)
		{
			for (int i=0 ; i < animSkel->getNumAnimationControls(); i++)
			{
				hkAnimationControl* control = animSkel->getAnimationControl(i);

				// Only use controls that have a non zero weight
				if ((control->getAnimationBinding()->m_blendHint == blendType) && (control->getWeight() > 0.0f ))
				{
					struct hkAnimationSampleAndCombineJob::ControlData& controlData = jobOut.m_animationControls[controlIndex];
					controlData.m_localTime = animSkel->getAnimationControl(i)->getLocalTime();
					controlData.m_weight = animSkel->getAnimationControl(i)->getWeight();
					controlData.m_binding = *animSkel->getAnimationControl(i)->getAnimationBinding();
					controlData.m_trackWeights = animSkel->getAnimationControl(i)->getTracksWeights().begin();
					controlData.m_numTrackWeights = animSkel->getAnimationControl(i)->getTracksWeights().getSize();

					hkSkeletalAnimation* animation = controlData.m_binding.m_animation;
					controlData.m_animationType = animation->getType();
					controlData.m_numChunks = animation->getNumDataChunks( controlData.m_localTime );
					animation->getDataChunks( controlData.m_localTime, controlData.m_chunks, controlData.m_numChunks);
					controlIndex++;
				}
			}
		}
	}

	// Outputs
	{
		jobOut.m_poseOut = poseOut;
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
