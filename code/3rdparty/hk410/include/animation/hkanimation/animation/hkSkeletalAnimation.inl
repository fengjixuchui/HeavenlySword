/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

inline hkSkeletalAnimation::hkSkeletalAnimation() : m_type(HK_UNKNOWN_ANIMATION)
{
	m_duration = 0.0f;
	m_numberOfTracks = 0;
	m_extractedMotion = HK_NULL;
	m_annotationTracks = HK_NULL;
	m_numAnnotationTracks = 0;
}

inline hkSkeletalAnimation::Type hkSkeletalAnimation::getType() const
{
	return m_type;
}

inline void hkSkeletalAnimation::samplePartialPose(hkReal time, hkUint32 maxTrack, hkQsTransform* poseLocalSpaceOut, hkChunkCache* cache) const
{
#ifndef HK_PLATFORM_IS_SPU
	HK_ERROR(0x54e32123, "samplePartialPose not implemented for this type of animation");
#endif
}

inline int hkSkeletalAnimation::getNumDataChunks(hkReal time) const
{
	return 0;
}

inline void hkSkeletalAnimation::getDataChunks(hkReal time, DataChunk* dataChunks, int m_numDataChunks) const
{
#ifndef HK_PLATFORM_IS_SPU
	HK_ERROR(0x54e32124, "getDataChunks not implemented for this type of animation");
#endif
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
