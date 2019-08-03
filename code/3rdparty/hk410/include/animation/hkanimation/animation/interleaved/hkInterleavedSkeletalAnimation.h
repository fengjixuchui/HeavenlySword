/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKANIMATION_ANIMATION_INTERLEAVED_HKINTERLEAVEDANIMATION_HKCLASS_H
#define HKANIMATION_ANIMATION_INTERLEAVED_HKINTERLEAVEDANIMATION_HKCLASS_H

#include <hkanimation/animation/hkSkeletalAnimation.h>

/// hkInterleavedSkeletalAnimation meta information
extern const class hkClass hkInterleavedSkeletalAnimationClass;

/// The information needed to construct an hkSimpleAnimation (An interleaved
/// uncompressed stream of Bone Transforms)
class hkInterleavedSkeletalAnimation : public hkSkeletalAnimation
{
	public:
	
		HK_DECLARE_CLASS_ALLOCATOR( HK_MEMORY_CLASS_ANIM_UNCOMPRESSED );
		HK_DECLARE_REFLECTION();

			/// Default constructor
		inline hkInterleavedSkeletalAnimation() { m_type = hkSkeletalAnimation::HK_INTERLEAVED_ANIMATION; }
		
			/// Get the pose at a given time
		virtual void samplePose(hkReal time, hkQsTransform* poseLocalSpaceOut, hkChunkCache* cache) const;

		virtual void samplePartialPose(hkReal time, hkUint32 maxTrack, hkQsTransform* poseLocalSpaceOut, hkChunkCache* cache) const;
	
			/// Transform for a given track by the bone transform
		void transformTrack(int track, const hkQsTransform& transform);
		
			/// Returns the number of original samples / frames of animation
		virtual int getNumOriginalFrames() const;

			/// Return the number of chunks of data required to sample a pose at time t
		virtual int getNumDataChunks(hkReal time) const;

			/// Return the chunks of data required to sample a pose at time t
		virtual void getDataChunks(hkReal time, DataChunk* dataChunks, int m_numDataChunks) const;

			/// Get a subset of the pose at a given time using data chunks
		static void HK_CALL samplePartialWithDataChunks(hkReal time, hkUint32 maxTrack, hkQsTransform* poseLocalSpaceOut, const DataChunk* dataChunks, int m_numDataChunks);

	public:
		
		hkQsTransform* m_transforms;
		int			   m_numTransforms;
		
	public:
	
			// Constructor for initialisation of vtable fixup
		HK_FORCE_INLINE hkInterleavedSkeletalAnimation( hkFinishLoadedObjectFlag flag ) : hkSkeletalAnimation(flag) {}
};

#endif // HKANIMATION_ANIMATION_INTERLEAVED_HKINTERLEAVEDANIMATION_HKCLASS_H

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
