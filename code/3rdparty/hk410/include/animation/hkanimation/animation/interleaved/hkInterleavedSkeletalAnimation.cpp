/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkanimation/hkAnimation.h>
#include <hkanimation/animation/interleaved/hkInterleavedSkeletalAnimation.h>
#include <hkanimation/motion/hkAnimatedReferenceFrame.h>
#include <hkbase/monitor/hkMonitorStream.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkInterleavedSkeletalAnimation);

int hkInterleavedSkeletalAnimation::getNumOriginalFrames() const
{
	return m_numTransforms / m_numberOfTracks;
}

void hkInterleavedSkeletalAnimation::samplePose( hkReal time, hkQsTransform* pose, hkChunkCache* cache ) const
{
	samplePartialPose(time, m_numberOfTracks, pose, cache);
}

inline void  getFrameAndDelta( hkReal time, hkReal duration, hkUint32 numFrames, int& frame, hkReal& delta )
{
	if ( time > duration )
	{
		frame = numFrames - 2;
		delta = 1.0f;
	}

	const hkReal timeSlice = duration / (numFrames  - 1);
	const hkReal frameFloat = time / timeSlice;
	frame	= static_cast<int>( frameFloat );
	delta = frameFloat - frame;
}


void hkInterleavedSkeletalAnimation::samplePartialPose( hkReal time, hkUint32 maxTracks, hkQsTransform* pose, hkChunkCache* cache ) const
{
	HK_ASSERT(0x3c447416,  0 <= time );
	HK_ASSERT(0x21f74f45,  HK_REAL_EPSILON < m_duration );
	HK_ASSERT(0x4bb72ed1,  0 < m_numTransforms );

	HK_TIMER_BEGIN("samplePose", this);

	if ( time >= m_duration )
	{
		for ( hkUint32 i = 0; i < maxTracks; ++i )
		{
			pose[i] = m_transforms[ m_numTransforms - m_numberOfTracks + i];
		}
		return;
	}

	int frame;
	hkReal delta;
	getFrameAndDelta( time, m_duration, (m_numTransforms/m_numberOfTracks), frame, delta );

	DataChunk data[2];
	getDataChunks( time, data, 2 );
	
	samplePartialWithDataChunks( time, maxTracks, pose, data, 2 );

	HK_TIMER_END();
}

int hkInterleavedSkeletalAnimation::getNumDataChunks(hkReal time) const
{
	return 2;
}

/// Return the compressed data required to sample a pose at time t
void hkInterleavedSkeletalAnimation::getDataChunks(hkReal time, DataChunk* dataChunks, int m_numDataChunks) const
{
	HK_ASSERT2(0x12343243, m_numDataChunks>=2, "Has memory been allocated for the data chunks");

	// 1st data chunk points to the base animation
	dataChunks[0].m_data = this;
	dataChunks[0].m_size = HK_NEXT_MULTIPLE_OF(16, sizeof(hkInterleavedSkeletalAnimation));

	int frame;
	hkReal delta;
	getFrameAndDelta( time, m_duration, (m_numTransforms / m_numberOfTracks), frame, delta );

	dataChunks[1].m_data = reinterpret_cast< hkUint8* >(m_transforms + frame * m_numberOfTracks);
	dataChunks[1].m_size = sizeof(hkQsTransform) * m_numberOfTracks * 2;
}

/// Get a subset of the pose at a given time using block data
void hkInterleavedSkeletalAnimation::samplePartialWithDataChunks(hkReal time, hkUint32 maxTrack, hkQsTransform* poseLocalSpaceOut, const DataChunk* dataChunks, int m_numDataChunks)
{
	HK_ASSERT2(0x12343243, m_numDataChunks>=2, "Has memory been allocated for the data chunks");

	hkFinishLoadedObjectFlag f;
	const hkInterleavedSkeletalAnimation* anim = new((void*)dataChunks[0].m_data) hkInterleavedSkeletalAnimation( f );

	int frame;
	hkReal delta;
	getFrameAndDelta( time, anim->m_duration, (anim->m_numTransforms / anim->m_numberOfTracks), frame, delta );

	const hkQsTransform* HK_RESTRICT prevFrame = reinterpret_cast< const hkQsTransform* >( dataChunks[1].m_data );
	const hkQsTransform* HK_RESTRICT nextFrame = prevFrame + anim->m_numberOfTracks;

	for( hkUint32 i = 0; i < maxTrack; i++ )
	{
		poseLocalSpaceOut[i].setInterpolate4(*prevFrame++, *nextFrame++, delta);
	}
}

void hkInterleavedSkeletalAnimation::transformTrack(int track, const hkQsTransform& transform)
{
	// Remap the animation
	for (int i=0; i < (m_numTransforms/m_numberOfTracks); i++)
	{
		// Grab the root bone
		const hkQsTransform curTransform = m_transforms[i * m_numberOfTracks + track];

		m_transforms[i*m_numberOfTracks + track].setMul(transform, curTransform);

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
