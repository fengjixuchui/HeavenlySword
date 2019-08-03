/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkanimation/hkAnimation.h>
#include <hkanimation/animation/deltacompressed/hkDeltaCompressedSkeletalAnimation.h>
#include <hkanimation/animation/interleaved/hkInterleavedSkeletalAnimation.h>
#include <hkcompression/hkCompression.h>
#include <hkbase/memory/hkLocalArray.h>
#include <hkanimation/animation/util/hkTrackAnalysis.h>
#include <hkanimation/playback/cache/hkChunkCache.h>
#include <hkbase/monitor/hkMonitorStream.h>
#include <hkbase/class/hkTypeInfo.h>

#include <hkbase/thread/hkSpuStack.h>

#if defined (HK_SIMULATE_SPU_DMA_ON_CPU)
#	include <hkbase/thread/util/hkSpuSimulator.h>
#endif

HK_REFLECTION_DEFINE_VIRTUAL(hkDeltaCompressedSkeletalAnimation);

hkDeltaCompressedSkeletalAnimation::CompressionParams::CompressionParams()
{
	m_quantizationBits = 8;
	m_blockSize = hkUint16(-1);
	m_absolutePositionTolerance = 0.0f;
	m_relativePositionTolerance = 0.01f;
	m_rotationTolerance = 0.001f;
	m_scaleTolerance = 0.01f;
}

hkDeltaCompressedSkeletalAnimation::~hkDeltaCompressedSkeletalAnimation()
{
	// If this class hasn't been serialized
	if ( m_memSizeAndFlags != 0 )
	{
		hkDeallocateChunk<hkUint8>( m_dataBuffer, m_numDataBuffer, HK_MEMORY_CLASS_ANIM_COMPRESSED );
	}
}

struct hkDeltaDecompressionParameters
{
	/// Number of dynamic degrees of freedom
	hkUint32 m_numD;

	/// Block Size
	int m_blockSize; 
	int m_numValues;

	hkUint8 m_preserved;

	hkUint8 m_maxBitWidth;

	const hkReal* m_offset;
	const hkReal* m_scale;
	const hkUint8* m_bitWidth;

	const hkUint8* m_quantizedData;

};

static void HK_FORCE_INLINE getFrameAndDelta( hkReal time, hkReal duration, hkUint32 numFrames, int& frame, hkReal& delta )
{
	const hkReal timeSlice = duration / (numFrames-1);

	// Compute the index / frame number for this pose
	frame = static_cast<int>( time / timeSlice );
	const int secondLast = int(numFrames-2);
	frame = (frame > secondLast) ? secondLast : frame;

	// Compute the interpolation delta within the frame
	delta = ( time / timeSlice ) - frame;
	delta = (delta > 1.0f) ? 1.0f : delta;
}

int hkDeltaCompressedSkeletalAnimation::getNumOriginalFrames() const
{
	return m_numberOfPoses;
}

static HK_FORCE_INLINE hkUint32 getFullCacheKey( const hkDeltaCompressedSkeletalAnimation* animation, hkUint32 poseIdx/*, hkUint32 numD*/)
{
	return (hkUint32)(hkUlong)animation + (poseIdx / animation->m_blockSize)*(animation->m_numberOfTracks+1);
}


int hkDeltaCompressedSkeletalAnimation::getNumDataChunks(hkReal time) const
{
	int frame; hkReal delta;
	getFrameAndDelta(time, m_duration, m_numberOfPoses, frame, delta);

	int pIdx = frame % m_blockSize;
	if (pIdx + 1 != m_blockSize)
		return 3;
	else
		return 4; // straddling two blocks
}

void hkDeltaCompressedSkeletalAnimation::getBlockDataAndSize(int blockNum, int numBlocks, DataChunk& dataChunkOut) const
{
	hkUint32 actualStart = (hkUint32)(m_dataBuffer) + m_quantizedDataIdx + blockNum*m_totalBlockSize;
	dataChunkOut.m_offset = actualStart & 0xF;
	actualStart &= 0xFFFFFFF0;
	dataChunkOut.m_data = (const void*) actualStart;

	dataChunkOut.m_size = (blockNum == numBlocks-1) ? m_lastBlockSize : m_totalBlockSize;
	dataChunkOut.m_size += dataChunkOut.m_offset;
	dataChunkOut.m_size = HK_NEXT_MULTIPLE_OF(16, dataChunkOut.m_size);
}

/// Return the compressed data required to sample a pose at time t
void hkDeltaCompressedSkeletalAnimation::getDataChunks(hkReal time, DataChunk* dataChunks, int numDataChunks) const
{
	HK_ASSERT2(0x12343243, numDataChunks>=3, "Has memory been allocated for the data chunks");

	int frame; hkReal delta;
	getFrameAndDelta(time, m_duration, m_numberOfPoses, frame, delta);

	// 1st data chunk points to the base animation
	dataChunks[0].m_data = this;
	dataChunks[0].m_size = HK_NEXT_MULTIPLE_OF(16, sizeof(hkDeltaCompressedSkeletalAnimation));

	// 2nd chunk is all of the static data, e.g. quantization format, static track masks, static track values
	dataChunks[1].m_data = m_dataBuffer;
	dataChunks[1].m_size = HK_NEXT_MULTIPLE_OF(16, m_quantizedDataIdx);

	int blockNumber = frame / m_blockSize;
	hkUint32 numBlocks = (m_numberOfPoses + m_blockSize-1) / m_blockSize; // Number of blocks

	getBlockDataAndSize(blockNumber, numBlocks, dataChunks[2]);
	if (numDataChunks == 4)
	{
		getBlockDataAndSize((blockNumber+1) % numBlocks, numBlocks, dataChunks[3]);
	}
}


static void _decompress(hkDeltaDecompressionParameters params, hkReal* outputData)
{
	// Init the quantize header
	// NB - the bitwidth here will be overwritten; we set it to the max here for allocating scratch space.
	hkQuantizeDesc desc;
	desc.m_preserved = params.m_preserved;

	const hkUint8* quantData = params.m_quantizedData;

	hkUint32 index = 0;

	for (hkUint32 idx=0; idx < params.m_numD; idx++)
	{
		desc.m_offset = params.m_offset[idx];
		desc.m_scale = params.m_scale[idx];
		// Backwards compatibility: if we've got an old animation without the array of bitwidths,
		// then the max is the correct width
		desc.m_bitWidth = params.m_bitWidth ? params.m_bitWidth[idx] : params.m_maxBitWidth;
		
		// Expand the entire block 
		hkExpandReal( &quantData[index], desc, outputData, params.m_numValues  );	

		// Delta transform
		hkInverseDeltaTransform( outputData, params.m_numValues );

		// skip over decompressed data
		outputData += params.m_blockSize;

		index += hkCalcQuantizedSize( params.m_numValues , desc );
	}
}

void hkDeltaCompressedSkeletalAnimation::samplePartialWithDataChunks(hkReal time, hkUint32 maxTrack, hkQsTransform* poseLocalSpaceOut, const DataChunk* dataChunks, int numDataChunks)
{
	HK_ASSERT2(0x12343243, numDataChunks>=2, "Has memory been allocated for the data chunks");

	hkFinishLoadedObjectFlag f;
	const hkDeltaCompressedSkeletalAnimation* anim = new((void*)dataChunks[0].m_data) hkDeltaCompressedSkeletalAnimation( f );

	HK_TIMER_BEGIN_LIST("SampleDeltaChunk", "StaticD");

	const hkUint8* staticDataPtr = (const hkUint8*)dataChunks[1].m_data;

	HK_ASSERT2(0x32e345ea, anim->m_staticMaskIdx < dataChunks[1].m_size, "Trying to read past end of static data");
	const hkUint16* staticMaskPtr = reinterpret_cast<const hkUint16*>(staticDataPtr + anim->m_staticMaskIdx);

	HK_ASSERT2(0x123ae453, anim->m_staticDOFsIdx < dataChunks[1].m_size, "Trying to read past end of static data");
	const hkReal* staticDOFPtr = reinterpret_cast<const hkReal*>(staticDataPtr + anim->m_staticDOFsIdx);

	const hkUint32 numT = maxTrack;
	const hkUint32 numDfull = anim->m_qFormat.m_numD;
	hkUint32 numD = numDfull;

	//if (maxTrack != (hkUint32)anim->m_numberOfTracks)
	{
		hkTrackAnalysis::AnalysisStats stats;
		hkTrackAnalysis::calcDynamicStats( staticMaskPtr, maxTrack, stats );
		numD = stats.m_dynamicPosDOF + stats.m_dynamicRotDOF + stats.m_dynamicSclDOF;
	}

	HK_TIMER_SPLIT_LIST("DecompressDChunk");

	int poseIdx;
	hkReal delta;
	getFrameAndDelta(time, anim->m_duration, anim->m_numberOfPoses, poseIdx, delta);
	const hkReal oneMinusDelta = 1.0f - delta;

	hkReal* HK_RESTRICT tmpCachePtr;
	hkReal* HK_RESTRICT poseDataPtr;

	// local array to decompress into
	if (HK_PLATFORM_IS_SPU)
	{
		tmpCachePtr = static_cast<hkReal*>(hkSpuStack::getInstance().allocateStack( HK_NEXT_MULTIPLE_OF(16, sizeof(hkReal) * numD * anim->m_blockSize), "Temp Cache" ));
		poseDataPtr = static_cast<hkReal*>(hkSpuStack::getInstance().allocateStack( HK_NEXT_MULTIPLE_OF(16, sizeof(hkReal) * numD                    ), "Pose Data" ));
	}
	else
	{
		tmpCachePtr = hkAllocateStack<hkReal>(numD * anim->m_blockSize);
		poseDataPtr = hkAllocateStack<hkReal>(numD);
	}

	hkDeltaDecompressionParameters params;
	int numValues = anim->m_blockSize;
	if (anim->m_blockSize > anim->m_numberOfPoses)
	{
		numValues = anim->m_numberOfPoses;
	}

	params.m_numD = numD;
	params.m_numValues = numValues;
	// We need to use a different numVales when we're on the last block
	// Fortunately, we can just check the size to find out if that's the case
	if (dataChunks[2].m_size < anim->m_totalBlockSize)
	{
		params.m_numValues = anim->m_numberOfPoses % anim->m_blockSize;
	}

	params.m_blockSize = anim->m_blockSize;
	params.m_preserved = anim->m_qFormat.m_preserved;
	params.m_maxBitWidth = anim->m_qFormat.m_maxBitWidth;

	HK_ASSERT2(0x54654e34, anim->m_qFormat.m_offsetIdx    < dataChunks[1].m_size, "Trying to read past end of static data");
	HK_ASSERT2(0x7656543e, anim->m_qFormat.m_scaleIdx     < dataChunks[1].m_size, "Trying to read past end of static data");
	HK_ASSERT2(0x12435e43, anim->m_qFormat.m_bitWidthIdx  < dataChunks[1].m_size, "Trying to read past end of static data");
	params.m_offset = reinterpret_cast<const hkReal*>(staticDataPtr + anim->m_qFormat.m_offsetIdx);
	params.m_scale = reinterpret_cast<const hkReal*>(staticDataPtr + anim->m_qFormat.m_scaleIdx);
	params.m_bitWidth = staticDataPtr + anim->m_qFormat.m_bitWidthIdx;

	if (numDataChunks == 3) // only need to sample one block
	{
		params.m_quantizedData = (const hkUint8*)dataChunks[2].m_data;
		params.m_quantizedData += dataChunks[2].m_offset;
		_decompress(params, tmpCachePtr);

		hkUint32 idx;
		int pIdx = poseIdx % anim->m_blockSize;
		// We're interpolating between two poses in the same block
		// So we can do the interpolation in one pass
		for( idx=0; idx < numD; idx++ )
		{
			poseDataPtr[idx] = oneMinusDelta * tmpCachePtr[pIdx] + delta * tmpCachePtr[pIdx+1];
			pIdx += anim->m_blockSize;
		}
	}
	else
	{
		params.m_quantizedData = (const hkUint8*)dataChunks[2].m_data;
		params.m_quantizedData += dataChunks[2].m_offset;
		_decompress(params, tmpCachePtr);

		hkUint32 idx;
		int pIdx = poseIdx % anim->m_blockSize;
		for( idx=0; idx < numD; idx++ )
		{
			poseDataPtr[idx] = oneMinusDelta * tmpCachePtr[pIdx];
			pIdx += anim->m_blockSize;
		}

		params.m_quantizedData = (const hkUint8*)dataChunks[3].m_data;
		params.m_quantizedData += dataChunks[3].m_offset;
		// Check that this chunk isn't the last block (see note above)
		if (dataChunks[3].m_size < anim->m_totalBlockSize)
		{
			params.m_numValues = anim->m_numberOfPoses % anim->m_blockSize;
		}
		_decompress(params, tmpCachePtr);

		pIdx = 0;
		for( idx=0; idx < numD; idx++ )
		{
			poseDataPtr[idx] += delta * tmpCachePtr[pIdx];
			pIdx += anim->m_blockSize;
		}
	}

	HK_TIMER_SPLIT_LIST("RecomposeDChunk");

	hkTrackAnalysis::AnalysisCompose combiner;
	combiner.m_numberOfTracks = numT;
	combiner.m_staticDOFs = staticDOFPtr;
	combiner.m_staticMask = staticMaskPtr;

	hkTrackAnalysis::recompose(combiner, poseDataPtr, poseLocalSpaceOut);

	if (HK_PLATFORM_IS_SPU)
	{
		hkSpuStack::getInstance().deallocateStack(poseDataPtr);
		hkSpuStack::getInstance().deallocateStack(tmpCachePtr);
	}
	else
	{
		hkDeallocateStack( poseDataPtr );
		hkDeallocateStack( tmpCachePtr );
	}

	HK_TIMER_END_LIST();
}

void hkDeltaCompressedSkeletalAnimation::samplePose( hkReal time, hkQsTransform* pose, hkChunkCache* cache ) const
{
	samplePartialPose(time, m_numberOfTracks, pose, cache);
}

#ifndef HK_PLATFORM_PS3SPU

static void decompressBlockToCache( const hkDeltaCompressedSkeletalAnimation* animation, hkUint32 pose, hkReal *data, hkUint32 numD)
{
	HK_TIMER_BEGIN_LIST("decompressBlockCacheD", "InitD"); 

	// Decompress the pose and pass on the data
	int block = pose / animation->m_blockSize;
	int numValues = animation->m_blockSize;


	if (animation->m_blockSize > animation->m_numberOfPoses)
	{
		numValues = animation->m_numberOfPoses;
		block = 0;
	}

	// Check for last partially filled block
	if ((block*animation->m_blockSize + numValues) > animation->m_numberOfPoses)
	{
		numValues = animation->m_numberOfPoses - block * animation->m_blockSize;
	}

	const hkUint8* dataBufferPtr = animation->m_dataBuffer;

	hkDeltaDecompressionParameters params;
	params.m_numD = numD;
	params.m_blockSize = animation->m_blockSize;
	params.m_numValues = numValues;
	params.m_preserved = animation->m_qFormat.m_preserved;
	params.m_maxBitWidth = animation->m_qFormat.m_maxBitWidth;
	params.m_offset = reinterpret_cast<const hkReal*>(dataBufferPtr + animation->m_qFormat.m_offsetIdx);
	params.m_scale = reinterpret_cast<const hkReal*>(dataBufferPtr + animation->m_qFormat.m_scaleIdx);
	params.m_bitWidth = dataBufferPtr + animation->m_qFormat.m_bitWidthIdx;
	params.m_quantizedData = dataBufferPtr + animation->m_quantizedDataIdx + block*animation->m_totalBlockSize;

	_decompress(params, data);


	HK_TIMER_END_LIST();
}

static const hkReal* retrieveFromCache( const hkDeltaCompressedSkeletalAnimation* animation, hkUint32 poseIdx, hkUint32 numD, hkChunkCache** cache )
{
	HK_TIMER_BEGIN("cacheD", HK_NULL);

	// pointer to memory chunk where decompressed data lies
	const hkReal* chunkPointer = HK_NULL;

	// make a unique key for this block and animation from the 'this' address and the block index
	const hkUint32 key = getFullCacheKey(animation, poseIdx);

	hkUint32 chunkSize = animation->m_blockSize * sizeof(hkReal) * numD;

	// query the cache for this key
	chunkPointer = (const hkReal*)(*cache)->retrieveChunk( key, chunkSize );

	if( chunkPointer == HK_NULL )
	{
		// key has not been cached - ask for memory chunk
		chunkPointer = (hkReal*)(*cache)->allocateChunk( key, chunkSize );

		if( chunkPointer == HK_NULL )
		{
			HK_WARN_ONCE( 0x270111fc, "Cache chunk allocation failure! " );
			return HK_NULL;
		}

		// we have a valid chunk pointer so decompress the data block to this location
		decompressBlockToCache( animation, poseIdx, const_cast<hkReal*>(chunkPointer), numD );

	}

	HK_TIMER_END();

	// cache has valid memory chunk with the data we require
	return chunkPointer;

}

#endif 




void hkDeltaCompressedSkeletalAnimation::samplePartialPose( hkReal time, hkUint32 maxTrack, hkQsTransform* pose, hkChunkCache* cache ) const
{
#ifndef HK_PLATFORM_PS3SPU
	HK_ASSERT(0x3c447417,  0 <= time );
	HK_ASSERT(0x21f74f44,  HK_REAL_EPSILON < m_duration );
	HK_ASSERT(0x65fe43ef, m_numberOfPoses > 1);

	HK_TIMER_BEGIN_LIST("SampleDelta", "TrackAnalysis");

	const hkUint32 numT = maxTrack;
	const hkUint32 numDfull = m_qFormat.m_numD;
	hkUint32 numD = numDfull;

	const hkUint16* staticMaskPtr = reinterpret_cast<const hkUint16*>(m_dataBuffer + m_staticMaskIdx);

	if (maxTrack != (hkUint32)m_numberOfTracks)
	{
		hkTrackAnalysis::AnalysisStats stats;
		hkTrackAnalysis::calcDynamicStats( staticMaskPtr, maxTrack, stats );
		numD = stats.m_dynamicPosDOF + stats.m_dynamicRotDOF + stats.m_dynamicSclDOF;
	}

	int poseIdx; hkReal delta;
	getFrameAndDelta(time, m_duration, m_numberOfPoses, poseIdx, delta);
	const hkReal oneMinusDelta = 1.0f - delta;

	// local array to store interpolated dynamic DOFs
	hkLocalArray<hkReal> poseData( numD );
	poseData.setSize( numD );

	// local array to store decompress into if cache is unavailable
	hkLocalArray<hkReal> tmpCache( numD * m_blockSize );
	tmpCache.setSize( numD * m_blockSize );

	// Pointer to decompressed chunk
	const hkReal* chunkPointer = HK_NULL;

	// Retrieve from cache if possible
	if (cache != HK_NULL)
	{
		// ask the cache for a pointer to the decompressed data for 'pose'
		chunkPointer = retrieveFromCache( this, poseIdx, numDfull, &cache );
	}

	// check for cache failure ( either no cache, cache bucket full or unable to allocate chunk )
	if( chunkPointer == HK_NULL )
	{
		// Decompress to temporary cache line
		// If there's no cache, then only decompress the partial pose
		decompressBlockToCache( this, poseIdx, tmpCache.begin(), numD );
		chunkPointer = tmpCache.begin();
	}

	// Interpolate into dynamicDOF array
	int pIdx = poseIdx % m_blockSize;
	hkUint32 idx;

	if (pIdx + 1 != m_blockSize)
	{
		// We're interpolating between two poses in the same block
		// So can do the interpolation in one pass
		for( idx=0; idx < numD; idx++ )
		{
			poseData[idx] = oneMinusDelta * chunkPointer[pIdx] + delta * chunkPointer[pIdx+1];
			pIdx += m_blockSize;
		}
	}

	else
	{
		// The next pose in is the next block, so we have to do two interpolation sweeps
		// And also try to decompress the next block
		for( idx=0; idx < numD; idx++ )
		{
				poseData[idx] = oneMinusDelta * chunkPointer[pIdx];
				pIdx += m_blockSize;
		}

		// Grab next pose
		poseIdx++;
		pIdx = 0;

		// If its in the next block we have to decompress that block
		chunkPointer = HK_NULL;

		// Retrieve from cache
		if (cache != HK_NULL)
		{
			// ask the cache for a pointer to the decompressed data for 'pose'
			chunkPointer = retrieveFromCache( this, poseIdx, numDfull, &cache );
		}

		// check for cache failure ( either no cache, cache bucket full or unable to allocate chunk )
		if( chunkPointer == HK_NULL )
		{
			// Decompress to temporary cache line
			decompressBlockToCache( this, poseIdx, tmpCache.begin(), numD );
			chunkPointer = tmpCache.begin();
		}

		// Interpolate into dynamicDOF array
		for( idx=0; idx < numD; idx++ )
		{
				poseData[idx] += delta * chunkPointer[pIdx];
				pIdx += m_blockSize;
		}
	}

	HK_TIMER_SPLIT_LIST("RecomposeD");
	const hkReal* staticDOFPtr = reinterpret_cast<const hkReal*>(m_dataBuffer + m_staticDOFsIdx);

	hkTrackAnalysis::AnalysisCompose combiner;
	combiner.m_numberOfTracks = numT;
	combiner.m_staticDOFs = staticDOFPtr;
	combiner.m_staticMask = staticMaskPtr;

	hkTrackAnalysis::recompose(combiner, poseData.begin(), pose);

	HK_TIMER_END_LIST();
#endif
}

static inline hkFloat32 hkConvertLittleEndianF32( hkFloat32 n )
{
#if HK_ENDIAN_LITTLE
	return n;
#else
	union fmt
	{
		hkUint8 b[4];
		hkFloat32 v;
	} fDataIn, fDataOut;

	fDataIn.v = n;
	fDataOut.b[0] = fDataIn.b[3];
	fDataOut.b[1] = fDataIn.b[2];
	fDataOut.b[2] = fDataIn.b[1];
	fDataOut.b[3] = fDataIn.b[0];
	return fDataOut.v;	

#endif
}

static inline hkUint32 hkConvertLittleEndianU32( hkUint32 n )
{
#if HK_ENDIAN_LITTLE
	return n;
#else
	union fmt
	{
		hkUint8 b[4];
		hkUint32 v;
	} fDataIn, fDataOut;

	fDataIn.v = n;
	fDataOut.b[0] = fDataIn.b[3];
	fDataOut.b[1] = fDataIn.b[2];
	fDataOut.b[2] = fDataIn.b[1];
	fDataOut.b[3] = fDataIn.b[0];
	return fDataOut.v;	

#endif
}
static inline hkUint16 hkConvertLittleEndianU16( hkUint16 n )
{
#if HK_ENDIAN_LITTLE
	return n;
#else
	union fmt
	{
		hkUint8 b[2];
		hkUint16 v;
	} fDataIn, fDataOut;

	fDataIn.v = n;
	fDataOut.b[0] = fDataIn.b[1];
	fDataOut.b[1] = fDataIn.b[0];
	return fDataOut.v;	

#endif
}

void hkDeltaCompressedSkeletalAnimation::handleEndian( )
{
#ifndef HK_PLATFORM_PS3SPU
#if HK_ENDIAN_BIG

	// Convert everything to the correct endian
	{
		// Convert static mask
		hkUint16* staticMask;
		{
			staticMask = reinterpret_cast<hkUint16*> (m_dataBuffer + m_staticMaskIdx);
			for (int t=0; t < m_numberOfTracks ; t++)
			{
				staticMask[t] = hkConvertLittleEndianU16( staticMask[t] );
			}
		}

		hkTrackAnalysis::AnalysisStats stats;
		hkTrackAnalysis::calcStats( staticMask, m_numberOfTracks, stats );
		// Convert the static DOFs
		{
			int numStatic = stats.m_staticPosDOF + stats.m_staticRotDOF + stats.m_staticSclDOF;
			hkReal* staticDOFs = reinterpret_cast<hkReal*> (m_dataBuffer + m_staticDOFsIdx);
			for (int i=0 ; i< numStatic; i++)
			{
				*staticDOFs = hkConvertLittleEndianF32( *staticDOFs );
				staticDOFs++;
			}

		}

		// Convert the dynamic DOFs
		{
			hkUint32 numD = stats.m_dynamicPosDOF + stats.m_dynamicRotDOF + stats.m_dynamicSclDOF;
			hkReal* offsetBuffer = reinterpret_cast<hkReal*>( m_dataBuffer + m_qFormat.m_offsetIdx );
			hkReal* scaleBuffer = reinterpret_cast<hkReal*>( m_dataBuffer + m_qFormat.m_scaleIdx );
			for (unsigned i=0; i < numD ; i++)
			{
				*offsetBuffer = hkConvertLittleEndianF32( *offsetBuffer );
				*scaleBuffer = hkConvertLittleEndianF32( *scaleBuffer );
				offsetBuffer++;
				scaleBuffer++;
			}
		}
	}
#endif
#endif
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
