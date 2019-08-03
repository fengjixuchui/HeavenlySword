/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_ANIMATION_JOBS_H
#define HK_ANIMATION_JOBS_H

#include <hkbase/hkBase.h>
#include <hkmath/hkMath.h>

#include <hkbase/thread/hkSemaphoreBusyWait.h>
#include <hkanimation/animation/hkAnimationBinding.h>
#include <hkanimation/animation/hkSkeletalAnimation.h>

class hkAnimatedSkeleton;

enum hkAnimationType
{
	INTERLEAVED_SKELETAL_ANIMATION,
	WAVELET_COMPRESSED_SEKELTAL_ANIMATION,
	DELTA_COMPRESSED_SKELETAL_ANIMATION
};

enum hkAnimationJobType
{
	ANIMATION_JOB_SAMPLE_ANIMATION,
	ANIMATION_JOB_SAMPLE_AND_COMBINE
};

	/// The base class for all animation jobs
class hkAnimationJob
{
	public:
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_DYNAMICS, hkAnimationJob );

		HK_FORCE_INLINE hkAnimationJob( hkAnimationJobType type );

	public:

		hkEnum<hkAnimationJobType,hkObjectIndex>	m_jobType;
};

/// A job which samples a set of animations.
/// If more than one animation is specified then DMA cost is hidden through double buffering.
class hkSampleAnimationJob : public hkAnimationJob
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_DYNAMICS, hkSampleAnimationJob);

		HK_FORCE_INLINE hkSampleAnimationJob();

		HK_FORCE_INLINE ~hkSampleAnimationJob() {}

	public:

		struct AnimationData
		{
			/* Sample inputs */
			HK_ALIGN16(hkReal				m_localTime);	/// The local time to sample at
			hkUint32						m_maxTrack;		/// The highest track to sample to - used for LOD sampling

			/* Animation Data */
			hkSkeletalAnimation::Type		m_animationType;/// The type of animation data stored in the chunks
			hkSkeletalAnimation::DataChunk	m_chunks[4];	/// The static animation data required to sample at 'localTime'
			int								m_numChunks;	/// The number of chunks that contain valid data.

			/* Output Data */
			hkQsTransform*					m_poseOut;		/// The results are placed in this buffer in main memory
		};

		/* Animation Data */
		AnimationData*					m_animData;		// The array of animations to sample
		int								m_numAnims;		// The number of animations to sample

		hkSemaphoreBusyWait*			m_jobDone;		/// This semaphore is released when the work is finished.
};

/// A job which does all the decompression and blending for an animated skeleton
class hkAnimationSampleAndCombineJob : public hkAnimationJob
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_DYNAMICS, hkAnimationSampleAndCombineJob);

		HK_FORCE_INLINE hkAnimationSampleAndCombineJob();

		HK_FORCE_INLINE ~hkAnimationSampleAndCombineJob() {}

	public:

		struct ControlData
		{
			HK_ALIGN16(hkReal				m_localTime);	/// The local time to sample at
			hkReal							m_weight;		/// The master weight for blending this animation
			hkAnimationBinding				m_binding;		/// The binding for this animation
			hkSkeletalAnimation::Type		m_animationType;/// The type of animation data stored in the chunks
			hkSkeletalAnimation::DataChunk	m_chunks[4];	/// The static animation data required to sample at 'localTime'
			hkInt32							m_numChunks;	/// The number of chunks that contain valid data.
			const hkUint8*					m_trackWeights; /// The per track weights for this control
			hkInt32							m_numTrackWeights; /// Set to 0 if no per track weight are required.
		};

		/* Sample inputs */
		class hkQsTransform*	m_referencePose;				/// Point to the reference pose or HK_NULL for no filling
		hkReal					m_referencePoseWeightThreshold; /// The fill threshold (see hkAnimatedSkeleton)
		hkUint32				m_maxBone;						/// The highest number of bones to sample

		/* Animation Control Data */
		/// Note: The control data here is expected to be order by blend hint. Normal blending comes first followed by additive.
		ControlData*			m_animationControls;
		hkUint32				m_numAnimationControls;

		/* Output Data */
		hkQsTransform*			m_poseOut;	/// The results are placed in this buffer in main memory
		hkSemaphoreBusyWait*	m_jobDone;	/// This semaphore is released when the work is finished.
};

#include <hkanimation/playback/multithreaded/hkAnimationJobs.inl>

#endif // HK_ANIMATION_JOBS_H

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
