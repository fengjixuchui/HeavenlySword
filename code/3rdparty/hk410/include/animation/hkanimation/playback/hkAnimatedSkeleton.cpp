/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkanimation/hkAnimation.h>
#include <hkanimation/rig/hkSkeleton.h>
#include <hkanimation/rig/hkSkeletonUtils.h>
#include <hkanimation/rig/hkPose.h>
#include <hkanimation/playback/hkAnimatedSkeleton.h>

#include <hkanimation/animation/hkSkeletalAnimation.h>
#include <hkanimation/animation/hkAnimationBinding.h>
#include <hkanimation/motion/hkAnimatedReferenceFrame.h>

#include <hkanimation/playback/control/hkAnimationControl.h>

#include <hkbase/memory/hkLocalArray.h>
#include <hkbase/monitor/hkMonitorStream.h>

#include <hkanimation/playback/utilities/hkSampleAndCombineUtils.h>

hkAnimatedSkeleton::hkAnimatedSkeleton( const hkSkeleton* skeleton )
{
	m_skeleton = skeleton;
	m_referencePoseWeightThreshold = 0.1f;
}

hkAnimatedSkeleton::~hkAnimatedSkeleton()
{
	for (int i = 0; i < m_animationControls.getSize(); ++i )
	{
		m_animationControls[i]->removeAnimationControlListener(this);
		m_animationControls[i]->removeReference();
	}
}

void hkAnimatedSkeleton::stepDeltaTime( hkReal deltaTime )
{
	HK_TIMER_BEGIN("StepDelta", HK_NULL);
	for (int i = 0; i < m_animationControls.getSize(); ++i )
	{
		m_animationControls[i]->update( deltaTime );
	}
	HK_TIMER_END();
}

// This method is the core of the blending and binding.
// It works by iterating over the skeleton extracting bound animation
// track info and then accumulates or blends this across tracks.
void hkAnimatedSkeleton::sampleAndCombineAnimations( hkQsTransform* poseOut, hkChunkCache* cache ) const
{
	sampleAndCombineInternal(poseOut, m_skeleton->m_numBones, cache, false);
}

void hkAnimatedSkeleton::sampleAndCombinePartialAnimations( hkQsTransform* poseOut, hkUint32 numBones, hkChunkCache* cache ) const
{
	sampleAndCombineInternal(poseOut, numBones, cache, true);
}

void hkAnimatedSkeleton::sampleAndCombineInternal( hkQsTransform* poseOut, hkUint32 numBones, hkChunkCache* cache, bool partial ) const
{
	HK_TIMER_BEGIN_LIST("SampComb", "Init");

	// Grab temporary scratch space for the t-pose and animation pose.
	hkLocalArray<hkQsTransform> animPose( numBones );
	animPose.setSize(numBones);

	hkLocalArray<hkReal> perBoneWeight( numBones  );
	perBoneWeight.setSize(numBones);

	// Initialize
	for (unsigned  i=0; i < numBones; i++)
	{
		// Prepare for blending
		poseOut[i].setZero();
		perBoneWeight[i] = 0.0f;
	}

	for (int blendType = hkAnimationBinding::NORMAL; blendType <= hkAnimationBinding::ADDITIVE; blendType ++)
	{

		// Blend in each animation in sequence
		int ac;
		for (ac=0; ac < m_animationControls.getSize(); ac++)
		{
			const hkAnimationControl* control = m_animationControls[ac];

			// Grab the state of the current control
			hkReal weight = control->getWeight();

		    // Early out if animation has no weight.
		    if (weight < HK_REAL_EPSILON)
		    {
			    continue;
		    }

			const hkAnimationBinding* binding = control->getAnimationBinding();

			if (binding == HK_NULL)
			{
				continue;
			}

			// If the animation is the wrong type, don't blend it now
			if (binding->m_blendHint != blendType)
			{
				continue;
			}

			const hkSkeletalAnimation* animation = binding->m_animation;


			HK_ASSERT2(0x57a6b7c3, animation->m_numberOfTracks < 256, "Too many tracks in animation");

			HK_TIMER_SPLIT_LIST("Sample");

			// We know the number of bones to sample, but this doesn't necessarily correspond to the maximum
			// track to be sampled. While we're searching for the max track, we can also disregard bones
			// with zero weight, and hopefully reduce the size of the animation to be sampled.

			hkUint32 maxTrack = hkSampleAndCombineUtils::getMaxTrack(binding->m_animationTrackToBoneIndices, control->getTracksWeights().begin(), numBones, animation->m_numberOfTracks);

			// Sample the actual pose. 
			if (partial)
			{
				// The number of tracks to sample is maxTrack + 1
				animation->samplePartialPose( control->getLocalTime(), maxTrack+1, animPose.begin(), cache );
			}
			else
			{
				animation->samplePose( control->getLocalTime(), animPose.begin(), cache  );
			}

			HK_TIMER_SPLIT_LIST("Combine");

			hkSampleAndCombineUtils::BlendParameters params;
			params.m_animationTransformsIn = animPose.begin();
			params.m_masterWeight =  control->getWeight();
			params.m_numAnimationTransforms = maxTrack;
			params.m_numBones = numBones;
			params.m_perBoneWeights = control->getTracksWeights().begin();
			params.m_trackToBoneMapping = binding->m_animationTrackToBoneIndices;

			if (blendType == hkAnimationBinding::NORMAL)
			{
				hkSampleAndCombineUtils::blendNormal(params, poseOut, perBoneWeight.begin());
			}
			else
			{
				hkSampleAndCombineUtils::blendAdditive(params, poseOut);
			}
		}

		// We need to blend in the reference pose and renormalize once we're
		// done with the NORMAL animations
		if (blendType == hkAnimationBinding::NORMAL) 
		{
			HK_TIMER_SPLIT_LIST("ReferencePose");

			// Fill in Bones below the reference pose threshold
			if (m_referencePoseWeightThreshold > 0.0f)
			{
				for (unsigned i=0; i < numBones; i++)
				{
					if (perBoneWeight[i] <= m_referencePoseWeightThreshold)
					{
						// Fill in tPose to avoid snapping
						const hkReal weight = m_referencePoseWeightThreshold - perBoneWeight[i];
						poseOut[i].blendAddMul(m_skeleton->m_referencePose[i], weight);
						perBoneWeight[i] += weight;
					}
				}
			}

			HK_TIMER_SPLIT_LIST("Normalize");

			// Renormalize the orientation and the weights
			hkQsTransform::fastRenormalizeBatch(poseOut, perBoneWeight.begin(), numBones);
			
		}
	}
	HK_TIMER_END_LIST();
}


void hkAnimatedSkeleton::getDeltaReferenceFrame(hkReal deltaTime, hkQsTransform& deltaMotionOut) const
{
	HK_TIMER_BEGIN("MotionExt", HK_NULL);

	hkReal weightNormalization = 0.0f;

	// Prepare for blending
	deltaMotionOut.setZero();

	int numAnims = m_animationControls.getSize();

	hkQsTransform delta;
	for (int ac=0; ac < numAnims; ac++)
	{
		hkAnimationControl* animControl = m_animationControls[ac];

		hkReal weight = animControl->getWeight();
		
		if (weight > 0)
		{
			const hkAnimationBinding* binding = animControl->getAnimationBinding();
			if (binding && binding->m_animation->m_extractedMotion)
			{
				hkReal futureLocalTime; int loops;

				animControl->getFutureTime( deltaTime, futureLocalTime, loops );
				binding->m_animation->m_extractedMotion->getDeltaReferenceFrame( animControl->getLocalTime(), futureLocalTime, loops, delta );
			}
			else
			{
				delta.setIdentity();
			}

			deltaMotionOut.blendAddMul(delta, weight);
	
			weightNormalization = weightNormalization + weight; // simd real does not have +=
		}
	}


	if (weightNormalization > HK_REAL_EPSILON)
	{
		if (weightNormalization > m_referencePoseWeightThreshold)
		{
			deltaMotionOut.blendNormalize(weightNormalization);
		}
		else
		{
			deltaMotionOut.blendNormalize(m_referencePoseWeightThreshold);

			if (deltaMotionOut.m_rotation.m_vec.length3() > HK_REAL_EPSILON)
			{
				// Reduce the rotation as the weight goes to zero
				hkVector4 axis;
				deltaMotionOut.m_rotation.getAxis(axis); 
				hkReal angle = deltaMotionOut.m_rotation.getAngle();
				angle = angle * hkReal(weightNormalization) / m_referencePoseWeightThreshold;
				deltaMotionOut.m_rotation.setAxisAngle(axis, angle);
			}

			// Scale should always be identity at the end
			deltaMotionOut.m_scale.setAll(1.0f);
		}
	}
	else
	{
		deltaMotionOut.setIdentity();
	}

	HK_TIMER_END();
}

void hkAnimatedSkeleton::addAnimationControl( hkAnimationControl* control )
{
	m_animationControls.pushBack( control );
	control->addReference();
	control->addAnimationControlListener(this);
}

void hkAnimatedSkeleton::removeAnimationControl( hkAnimationControl* control )
{
	int index = m_animationControls.indexOf( control );

	if (index != -1)
	{
		m_animationControls.removeAt( index );
		control->removeAnimationControlListener(this);
		control->removeReference();
	}
}

void hkAnimatedSkeleton::controlDeletedCallback(hkAnimationControl* control)
{
	int index = m_animationControls.indexOf( control );

	if (index != -1)
	{
		m_animationControls.removeAt( index );
		control->removeAnimationControlListener(this);
	}	
}

hkUint32 hkAnimatedSkeleton::getNumAnnotations( hkReal deltaTime ) const
{
	int count = 0;
	for (int ac=0; ac < m_animationControls.getSize(); ac++)
	{
		const hkAnimationControl& control = *m_animationControls[ ac ];
		const hkSkeletalAnimation* anim = control.getAnimationBinding()->m_animation;
		const hkReal startTime = control.getLocalTime();
		const hkReal endTime = startTime + deltaTime;

		// Iterate through the tracks
		for (int t=0; t < anim->m_numAnnotationTracks; t++)
		{
			const hkAnnotationTrack* track = anim->m_annotationTracks[ t ];

			// Iterate through the annotations
			for (int a=0; a < track->m_numAnnotations ;a++)
			{
				const hkAnnotationTrack::Annotation& note = track->m_annotations[ a ];

				if (( note.m_time >= startTime ) && ( note.m_time <= endTime ))
				{
					count++;
				}
			}
		}
	}
	return count;
}

void hkAnimatedSkeleton::getAnnotations(hkReal deltaTime, BoneAnnotation* annotations) const
{
	int count = 0;
	for (int ac=0; ac < m_animationControls.getSize(); ac++)
	{
		const hkAnimationControl& control = *m_animationControls[ ac ];
		const hkAnimationBinding* binding = control.getAnimationBinding();
		const hkSkeletalAnimation* anim = binding->m_animation;
		const hkReal startTime = control.getLocalTime();

		//TODO: This doesn't handle wrapping looping.
		const hkReal endTime = startTime + deltaTime;

		// Iterate through the tracks
		for (int t=0; t < anim->m_numAnnotationTracks; t++)
		{
			const hkAnnotationTrack* track = anim->m_annotationTracks[ t ];

			// Iterate through the annotations
			for (int a=0; a < track->m_numAnnotations ;a++)
			{
				const hkAnnotationTrack::Annotation& note = track->m_annotations[ a ];

				if (( note.m_time >= startTime ) && ( note.m_time <= endTime ))
				{
					annotations[ count ].m_boneID = binding->m_animationTrackToBoneIndices[ t ];
					annotations[ count ].m_annotation = &track->m_annotations[ a ];
					count++;
				}
			}
		}
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
