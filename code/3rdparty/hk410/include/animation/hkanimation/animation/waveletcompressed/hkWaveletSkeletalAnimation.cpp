/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkanimation/hkAnimation.h>
#include <hkanimation/animation/waveletcompressed/hkWaveletSkeletalAnimation.h>
#include <hkcompression/hkCompression.h>
#include <hkbase/memory/hkLocalBuffer.h>
#include <hkanimation/animation/util/hkTrackAnalysis.h>
#include <hkanimation/playback/cache/hkChunkCache.h>
#include <hkbase/monitor/hkMonitorStream.h>
#include <hkbase/class/hkTypeInfo.h>

#include <hkbase/thread/hkSpuStack.h>

#if defined (HK_SIMULATE_SPU_DMA_ON_CPU)
#	include <hkbase/thread/util/hkSpuSimulator.h>
#endif

HK_REFLECTION_DEFINE_VIRTUAL(hkWaveletSkeletalAnimation);

//#define USE_DETAILED_WAVELET_TIMERS

#ifdef USE_DETAILED_WAVELET_TIMERS
#	include <hkbase/monitor/hkMonitorStream.h>
#else
#	undef HK_INTERNAL_TIMER_BEGIN_LIST
#	define HK_INTERNAL_TIMER_BEGIN_LIST(a,b)
#	undef HK_INTERNAL_TIMER_SPLIT_LIST
#	define HK_INTERNAL_TIMER_SPLIT_LIST(a)
#	undef HK_INTERNAL_TIMER_END_LIST
#	define HK_INTERNAL_TIMER_END_LIST()
#	undef HK_INTERNAL_TIMER_BEGIN
#	define HK_INTERNAL_TIMER_BEGIN(a,b)
#	undef HK_INTERNAL_TIMER_END
#	define HK_INTERNAL_TIMER_END()
#endif

/// Contains all the information necessary to decompress one block of wavelet data
struct hkWaveletDecompressionParameters
{
	/// Number of dynamic degrees of freedom
	hkUint32 m_numD;

	/// Block Size
	int m_blockSize; 

	hkUint8 m_preserved;

	hkUint8 m_maxBitWidth;

	const hkReal* m_offset;
	const hkReal* m_scale;
	const hkUint8* m_bitWidth;

	const hkUint8* m_encodedData;

};

hkWaveletSkeletalAnimation::CompressionParams::CompressionParams()
{
	m_quantizationBits = 8;
	m_blockSize = hkUint16(-1);
	m_preserve = 0;
	m_truncProp = 0.1f;
	m_absolutePositionTolerance = 0.0f;
	m_relativePositionTolerance = 0.01f;
	m_rotationTolerance = 0.001f;
	m_scaleTolerance = 0.01f;
}

hkWaveletSkeletalAnimation::~hkWaveletSkeletalAnimation()
{
	// If this class hasn't been serialized
	if ( m_memSizeAndFlags != 0 )
	{
		hkDeallocateChunk<hkUint8>( m_dataBuffer, m_numDataBuffer, HK_MEMORY_CLASS_ANIM_COMPRESSED );
	}
}


int hkWaveletSkeletalAnimation::getNumOriginalFrames() const
{
	return m_numberOfPoses;
}

void HK_FORCE_INLINE getFrameAndDelta( hkReal time, hkReal duration, hkUint32 numFrames, int& frame, hkReal& delta )
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

static HK_FORCE_INLINE hkUint32 getFullCacheKey( const hkWaveletSkeletalAnimation* animation, hkUint32 poseIdx/*, hkUint32 numD*/)
{
	return (hkUint32)(hkUlong)animation + (poseIdx / animation->m_blockSize)*(animation->m_numberOfTracks+1);
}

int hkWaveletSkeletalAnimation::getNumDataChunks(hkReal time) const
{
	int frame; hkReal delta;
	getFrameAndDelta(time, m_duration, m_numberOfPoses, frame, delta);

	int pIdx = frame % m_blockSize;
	if (pIdx + 1 != m_blockSize)
		return 3;
	else
		return 4; // straddling two blocks
}

void hkWaveletSkeletalAnimation::getBlockDataAndSize(int blockNum, DataChunk& dataChunkOut) const
{
	const hkUint32* blockIndexPtr = reinterpret_cast<const hkUint32*>(m_dataBuffer + m_blockIndexIdx);

	hkUlong actualStart = (hkUlong)(m_dataBuffer) + m_quantizedDataIdx + blockIndexPtr[blockNum];
	dataChunkOut.m_offset = actualStart & 0xF;
	actualStart &= 0xFFFFFFF0;
	dataChunkOut.m_data = (const void*) actualStart;

	if (blockNum != int(m_blockIndexSize-1) )
	{
		dataChunkOut.m_size = blockIndexPtr[blockNum+1] - blockIndexPtr[blockNum];
	}
	else
	{
		dataChunkOut.m_size = m_quantizedDataSize - blockIndexPtr[blockNum];
	}

	dataChunkOut.m_size += dataChunkOut.m_offset;
	dataChunkOut.m_size = HK_NEXT_MULTIPLE_OF(16, dataChunkOut.m_size);

}


/// Return the compressed data required to sample a pose at time t
void hkWaveletSkeletalAnimation::getDataChunks(hkReal time, DataChunk* dataChunks, int numDataChunks) const
{
	HK_ASSERT2(0x12343243, numDataChunks>=3, "Has memory been allocated for the data chunks");

	int frame; hkReal delta;
	getFrameAndDelta(time, m_duration, m_numberOfPoses, frame, delta);

	// 1st data chunk points to the base animation
	dataChunks[0].m_data = this;
	dataChunks[0].m_size = HK_NEXT_MULTIPLE_OF(16, sizeof(hkWaveletSkeletalAnimation));

	// 2nd chunk is all of the static data, e.g. quantization format, static track masks, static track values
	dataChunks[1].m_data = m_dataBuffer;
	dataChunks[1].m_size = HK_NEXT_MULTIPLE_OF(16, m_quantizedDataIdx);

	int blockNumber = frame / m_blockSize;

	getBlockDataAndSize(blockNumber, dataChunks[2]);
	if (numDataChunks == 4)
	{
		getBlockDataAndSize((blockNumber+1) % m_blockIndexSize,  dataChunks[3]);
	}

}


static void _decompress(hkWaveletDecompressionParameters params, hkReal* outputData)
{
	HK_INTERNAL_TIMER_BEGIN_LIST("_decompress", "_setup");
	// Init the quantize header
	// NB - the bitwidth here will be overwritten; we set it to the max here for allocating scratch space.
	hkQuantizeDesc qDesc;
	qDesc.m_bitWidth = params.m_maxBitWidth;
	qDesc.m_preserved = params.m_preserved;

	// Init the decoding header
	hkBlockDesc bDesc;
	bDesc.m_preserved = params.m_preserved;

	hkUint32 qDataSize = hkCalcQuantizedSize( params.m_blockSize, qDesc );

	// Alloc scratch space for quantization
	hkUint8* HK_RESTRICT quantizedData;

	if (HK_PLATFORM_IS_SPU)
		quantizedData = static_cast<hkUint8*>(hkSpuStack::getInstance().allocateStack( HK_NEXT_MULTIPLE_OF(16, sizeof(hkUint8) * qDataSize ), "Quantized Data" ));
	else
		quantizedData = hkAllocateStack<hkUint8>( qDataSize );


	const hkUint8* encodedDataPtr = params.m_encodedData;

	for (hkUint32 idx=0; idx < params.m_numD; idx++)
	{

		qDesc.m_offset = params.m_offset[idx];
		qDesc.m_scale = params.m_scale[idx];
		// Backwards compatibility: if we've got an old animation without the array of bitwidths,
		// then the max is the correct width
		qDesc.m_bitWidth = params.m_bitWidth ? params.m_bitWidth[idx] : params.m_maxBitWidth;
		// Make sure we compute the size correctly for this track
		qDataSize = hkCalcQuantizedSize( params.m_blockSize, qDesc );

		bDesc.m_bitWidth = qDesc.m_bitWidth;

		// Work out the zero sym - if quantisation changes we need to change this
		bDesc.m_runSymbol = hkUint32 ( (1<<qDesc.m_bitWidth) * (- qDesc.m_offset) / qDesc.m_scale );
		bDesc.m_runSymbol = (bDesc.m_runSymbol == hkUint32(1 << qDesc.m_bitWidth)) ? bDesc.m_runSymbol-1 : bDesc.m_runSymbol;

		HK_INTERNAL_TIMER_SPLIT_LIST("decode");

		// Expand the encoded data
		hkUint32 size = hkBlockDecode( encodedDataPtr, bDesc, quantizedData/*.begin()*/, qDataSize );
		encodedDataPtr += size;

		HK_INTERNAL_TIMER_SPLIT_LIST("expand");

		// Expand the quantized data
		hkExpandReal( quantizedData/*.begin()*/, qDesc, outputData, params.m_blockSize );	

		HK_INTERNAL_TIMER_SPLIT_LIST("transform");

		// Delta transform
		hkInverseWaveletTransform( outputData, params.m_blockSize );

		// Copy value
		outputData += params.m_blockSize;
	}

	if (HK_PLATFORM_IS_SPU)
		hkSpuStack::getInstance().deallocateStack( quantizedData );
	else
		hkDeallocateStack( quantizedData );

	HK_INTERNAL_TIMER_END_LIST();
}


/// Get a subset of the pose at a given time using block data
void hkWaveletSkeletalAnimation::samplePartialWithDataChunks(hkReal time, hkUint32 maxTrack, hkQsTransform* poseLocalSpaceOut, const DataChunk* dataChunks, int numDataChunks)
{
	HK_ASSERT2(0x12343243, numDataChunks>=2, "Has memory been allocated for the data chunks");

	hkFinishLoadedObjectFlag f;
	const hkWaveletSkeletalAnimation* anim = new((void*)dataChunks[0].m_data) hkWaveletSkeletalAnimation( f );

	HK_TIMER_BEGIN_LIST("SampleWaveChunk", "StaticW");
	
	const hkUint8* staticDataPtr = (const hkUint8*)dataChunks[1].m_data;

	HK_ASSERT2(0x34e76e43, anim->m_staticMaskIdx < dataChunks[1].m_size, "Trying to read past end of static data");
	const hkUint16* staticMaskPtr = reinterpret_cast<const hkUint16*>(staticDataPtr + anim->m_staticMaskIdx);
	
	HK_ASSERT2(0x87e4e3a3, anim->m_staticDOFsIdx < dataChunks[1].m_size, "Trying to read past end of static data");
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

	HK_TIMER_SPLIT_LIST("DecompressWChunk");

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

	hkWaveletDecompressionParameters params;
	params.m_numD = numD;
	params.m_blockSize = anim->m_blockSize;
	params.m_preserved = anim->m_qFormat.m_preserved;
	params.m_maxBitWidth = anim->m_qFormat.m_maxBitWidth;

	HK_ASSERT2(0x434e4565, anim->m_qFormat.m_offsetIdx    < dataChunks[1].m_size, "Trying to read past end of static data");
	HK_ASSERT2(0x4e454e45, anim->m_qFormat.m_scaleIdx     < dataChunks[1].m_size, "Trying to read past end of static data");
	HK_ASSERT2(0x12e3e321, anim->m_qFormat.m_bitWidthIdx  < dataChunks[1].m_size, "Trying to read past end of static data");
	params.m_offset = reinterpret_cast<const hkReal*>(staticDataPtr + anim->m_qFormat.m_offsetIdx);
	params.m_scale = reinterpret_cast<const hkReal*>(staticDataPtr + anim->m_qFormat.m_scaleIdx);
	params.m_bitWidth = staticDataPtr + anim->m_qFormat.m_bitWidthIdx;

	if (numDataChunks == 3) // only need to sample one block
	{
		params.m_encodedData = (const hkUint8*)dataChunks[2].m_data;
		params.m_encodedData += dataChunks[2].m_offset;
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
		params.m_encodedData = (const hkUint8*)dataChunks[2].m_data;
		params.m_encodedData += dataChunks[2].m_offset;
		_decompress(params, tmpCachePtr);

		hkUint32 idx;
		int pIdx = poseIdx % anim->m_blockSize;
		for( idx=0; idx < numD; idx++ )
		{
			poseDataPtr[idx] = oneMinusDelta * tmpCachePtr[pIdx];
			pIdx += anim->m_blockSize;
		}

		params.m_encodedData = (const hkUint8*)dataChunks[3].m_data;
		params.m_encodedData += dataChunks[3].m_offset;
		_decompress(params, tmpCachePtr);

		pIdx = 0;
		for( idx=0; idx < numD; idx++ )
		{
			poseDataPtr[idx] += delta * tmpCachePtr[pIdx];
			pIdx += anim->m_blockSize;
		}
	}

	HK_TIMER_SPLIT_LIST("RecomposeWChunk");

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



void hkWaveletSkeletalAnimation::samplePose( hkReal time, hkQsTransform* pose, hkChunkCache* cache ) const
{
	samplePartialPose(time, m_numberOfTracks, pose, cache);
}

#ifndef HK_PLATFORM_PS3SPU

static void decompressBlockToCache(const hkWaveletSkeletalAnimation* animation, hkUint32 pose, hkReal *data, hkUint32 numD)
{
	HK_TIMER_BEGIN_LIST("decompressBlockCacheW", "Init");

	// Decompress the pose and pass on the data
	hkUint32 block = pose / animation->m_blockSize;

	const hkUint8* dataBufferPtr = animation->m_dataBuffer;
	const hkUint32* indexArray = reinterpret_cast<const hkUint32*>(dataBufferPtr + animation->m_blockIndexIdx);
	//	hkUint32 quantizedIdx = animation->m_quantizedDataIdx + indexArray[ block ];

	hkWaveletDecompressionParameters params;

	params.m_numD = numD;
	params.m_blockSize = animation->m_blockSize;
	params.m_preserved = animation->m_qFormat.m_preserved;
	params.m_maxBitWidth = animation->m_qFormat.m_maxBitWidth;
	params.m_offset = reinterpret_cast<const hkReal*>(dataBufferPtr + animation->m_qFormat.m_offsetIdx);
	params.m_scale = reinterpret_cast<const hkReal*>(dataBufferPtr + animation->m_qFormat.m_scaleIdx);
	params.m_bitWidth =dataBufferPtr + animation->m_qFormat.m_bitWidthIdx;
	params.m_encodedData = dataBufferPtr + animation->m_quantizedDataIdx + indexArray[ block ];

	_decompress(params, data);
}



static hkReal* retrieveFromCache( const hkWaveletSkeletalAnimation* animation, hkUint32 poseIdx, hkUint32 numD, hkChunkCache** cache )
{
	HK_TIMER_BEGIN("cacheW", HK_NULL);

	// pointer to memory chunk where decompressed data lies
	hkReal* chunkPointer = HK_NULL;

	// make a unique key for this block and animation from the 'this' address and the block index
	const hkUint32 key = getFullCacheKey(animation, poseIdx);

	hkUint32 chunkSize = animation->m_blockSize * sizeof(hkReal) * numD;

	// query the cache for this key
	chunkPointer = (hkReal*)const_cast<hkUint8*>( (*cache)->retrieveChunk( key, chunkSize ) );

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
		decompressBlockToCache( animation, poseIdx, chunkPointer, numD );

	}

	HK_TIMER_END();

	// cache has valid memory chunk with the data we require
	return chunkPointer;

}

#endif  

void hkWaveletSkeletalAnimation::samplePartialPose( hkReal time, hkUint32 maxTrack, hkQsTransform* pose, hkChunkCache* cache ) const
{
#ifndef HK_PLATFORM_PS3SPU
	HK_ASSERT(0x3c447417,  0 <= time );
	HK_ASSERT(0x21f74f44,  HK_REAL_EPSILON < m_duration );
	HK_ASSERT(0x65fe43ef, m_numberOfPoses > 1);

	HK_TIMER_BEGIN_LIST("SampleWave", "StaticW");

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

	HK_TIMER_SPLIT_LIST("DecompressW");

	// Compute the index / frame number for this pose
	int poseIdx;
	hkReal delta;
	getFrameAndDelta(time, m_duration, m_numberOfPoses, poseIdx, delta);
	const hkReal oneMinusDelta = 1.0f - delta;

	// local array to store decompress into if cache is unavailable
	hkLocalBuffer<hkReal> tmpCache( numD * m_blockSize );

	// local array to store interpolated dynamic DOFs
	hkLocalBuffer<hkReal> poseData( numD );

	// Pointer to decompressed chunk
	hkReal* chunkPointer = HK_NULL;

	// Retrieve from cache if possible
	if (cache != HK_NULL)
	{
		// Ask the cache for a pointer to the decompressed data for 'pose'
		// If we have a cache, try to use the whole pose
		chunkPointer = retrieveFromCache( this, poseIdx, numDfull, &cache );
	}

	// Check for cache failure ( either no cache, cache bucket full or unable to allocate chunk )
	if( chunkPointer == HK_NULL )
	{
		// Decompress to temporary cache line
		// If there's no cache, then only decompress the partial pose
		decompressBlockToCache( this, poseIdx, tmpCache.begin(), numD );
		chunkPointer = tmpCache.begin();
	}

	// Interpolate into dynamicDOF array
	HK_TIMER_SPLIT_LIST("Interp");
	int pIdx = poseIdx % m_blockSize;

	hkUint32 idx;
	if (pIdx + 1 != m_blockSize)
	{
		// We're interpolating between two poses in the same block
		// So we can do the interpolation in one pass
		for( idx=0; idx < numD; idx++ )
		{
			poseData[idx] = oneMinusDelta * chunkPointer[pIdx] + delta * chunkPointer[pIdx+1];
			pIdx += m_blockSize;
		}
	}
	else
	{
		// The second pose in is the next block, so we have to do two interpolation sweeps
		// And also try to decompress the next block
		for( idx=0; idx < numD; idx++ )
		{
			poseData[idx] = oneMinusDelta * chunkPointer[pIdx];
			pIdx += m_blockSize;
		}

		HK_TIMER_SPLIT_LIST("DecompressW");

		// Grab next pose
		poseIdx++;
		pIdx = 0;

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
		HK_TIMER_SPLIT_LIST("Interp");
		for( idx=0; idx < numD; idx++ )
		{
			poseData[idx] += delta * chunkPointer[pIdx];
			pIdx += m_blockSize;
		}
	}

	HK_TIMER_SPLIT_LIST("RecomposeW");

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


void hkWaveletSkeletalAnimation::handleEndian( )
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

		// Convert the block indices
		{
			hkUint32* indexArray = reinterpret_cast<hkUint32*>(m_dataBuffer + m_blockIndexIdx);
			for (unsigned i=0; i < m_blockIndexSize; i++)
			{
				*indexArray = hkConvertLittleEndianU32( *indexArray );
				indexArray++;
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
