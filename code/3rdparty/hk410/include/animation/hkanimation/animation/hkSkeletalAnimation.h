/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKANIMATION_ANIMATION_HKSKELETALANIMATION_XML_H
#define HKANIMATION_ANIMATION_HKSKELETALANIMATION_XML_H

#include <hkanimation/animation/hkAnnotationTrack.h>
class hkAnimatedReferenceFrame;
class hkChunkCache;

/// hkSkeletalAnimation meta information
extern const class hkClass hkSkeletalAnimationClass;

/// The base class for animation storage.
/// All skeletal animation have a finite period specified in seconds.
/// The interface implies that each implementation can be sampled continuously over the period.
class hkSkeletalAnimation : public hkReferencedObject
{
	public:
	
		HK_DECLARE_CLASS_ALLOCATOR( HK_MEMORY_CLASS_ANIM_UNCOMPRESSED );
		HK_DECLARE_REFLECTION();

			/// Default constructor
		inline hkSkeletalAnimation();

			// Type information 
		enum Type {

			HK_UNKNOWN_ANIMATION = 0,

			HK_INTERLEAVED_ANIMATION,

			HK_DELTA_COMPRESSED_ANIMATION,

			HK_WAVELET_COMPRESSED_ANIMATION
		};

		
		HK_FORCE_INLINE hkSkeletalAnimation::Type getType() const;

			/// Get the pose at a given time
		virtual void samplePose(hkReal time, hkQsTransform* poseLocalSpaceOut, hkChunkCache* cache) const = 0;
		
			/// Get a subset of the pose at a given time
		virtual void samplePartialPose(hkReal time, hkUint32 maxTrack, hkQsTransform* poseLocalSpaceOut, hkChunkCache* cache) const;

			/// Returns the number of original samples / frames of animation
		virtual int getNumOriginalFrames() const = 0;

			/*
			* Block decompression
			*/

		struct DataChunk
		{
			const void* m_data;
			hkUint32	m_size;
			char        m_offset; // the amount we need to add to the data to bring it to the "real" data
			DataChunk() : m_offset(0) {}
		};

			/// Return the number of chunks of data required to sample a pose at time t
		virtual int getNumDataChunks(hkReal time) const;

			/// Return the chunks of data required to sample a pose at time t
		virtual void getDataChunks(hkReal time, DataChunk* dataChunks, int m_numDataChunks) const;

	protected:
		
		hkEnum<hkSkeletalAnimation::Type, hkInt32> m_type;

	public:

			/// The length of the animation cycle in seconds
		hkReal m_duration;
		
			/// The number of bone tracks to be animated.
		int m_numberOfTracks;
		
			/// An hkAnimatedReferenceFrame instance containing extracted motion.
		const hkAnimatedReferenceFrame* m_extractedMotion;
		
			/// The animation tracks associated with this skeletal animation.
		hkAnnotationTrack** m_annotationTracks;
		hkInt32 m_numAnnotationTracks;
	
			// Constructor for initialisation of vtable fixup
		HK_FORCE_INLINE hkSkeletalAnimation( hkFinishLoadedObjectFlag flag ) : hkReferencedObject(flag) {}
};

#include <hkanimation/animation/hkSkeletalAnimation.inl>

#endif // HKANIMATION_ANIMATION_HKSKELETALANIMATION_XML_H

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
