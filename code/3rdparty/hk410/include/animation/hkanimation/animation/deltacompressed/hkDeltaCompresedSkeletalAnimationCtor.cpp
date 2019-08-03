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

// Data comes in in interleaved format
// It is split into interleaved block format.
hkDeltaCompressedSkeletalAnimation::hkDeltaCompressedSkeletalAnimation(const hkInterleavedSkeletalAnimation& raw, const CompressionParams& params )
{
	PerTrackCompressionParams ptparams;
	ptparams.m_trackIndexToPaletteIndex.setSize(0);
	ptparams.m_parameterPalette.pushBack(params);
	initialize(raw, ptparams);
}

hkDeltaCompressedSkeletalAnimation::hkDeltaCompressedSkeletalAnimation( const hkInterleavedSkeletalAnimation& raw, const PerTrackCompressionParams& params )
{
	initialize(raw, params);
}

void hkDeltaCompressedSkeletalAnimation::initialize( const hkInterleavedSkeletalAnimation& raw, const PerTrackCompressionParams& params )
{
	this->m_type = hkSkeletalAnimation::HK_DELTA_COMPRESSED_ANIMATION;

	this->m_duration = raw.m_duration;
	this->m_numberOfTracks = raw.m_numberOfTracks;
	this->m_extractedMotion = raw.m_extractedMotion;
	this->m_annotationTracks = raw.m_annotationTracks;
	this->m_numAnnotationTracks = raw.m_numAnnotationTracks;

	const hkUint32 numTracks = raw.m_numberOfTracks;	// Number of tracks
	const hkUint32 numPoses = raw.m_numTransforms / numTracks;

	m_numberOfPoses = hkUint16(numPoses);
	m_blockSize = hkMath::min2(params.m_parameterPalette[0].m_blockSize, hkUint16(numPoses));

	// check that the block size are identical for all the palettes
	for (int i=1; i<params.m_parameterPalette.getSize(); i++)
	{
		HK_ASSERT2(0x545654e4, m_blockSize == params.m_parameterPalette[i].m_blockSize, "Variable blocksize not supported in hkWaveletSkeletalAnimation");
	}

	// We pack all compression data into this data buffer
	hkArray<hkUint8> dataBuffer;
	int dataBufferOffset = 0;

	// Allocate space for the static mask
	hkUint16* staticMask;
	{
		m_staticMaskIdx = dataBufferOffset;
		int deltaSize = HK_NEXT_MULTIPLE_OF(4, numTracks * sizeof(hkUint16));
		dataBuffer.expandBy( deltaSize );
		staticMask = reinterpret_cast<hkUint16*> (dataBuffer.begin() + m_staticMaskIdx);
		dataBufferOffset += deltaSize;
	}

	// Do track analysis
	hkTrackAnalysis::AnalysisStats stats;
	{
		// Worst Case
		hkLocalArray<hkReal> staticDOFs( numTracks * 10);
		staticDOFs.setSize( numTracks * 10);

		const int numTransforms = raw.m_numTransforms;
		hkTrackAnalysis::PerBoneAnalysisInput input;
		input.m_boneData = raw.m_transforms;
		input.m_numberOfTracks = numTracks;
		input.m_numberOfPoses = numTransforms / numTracks;

		input.m_absolutePositionTolerance.setSize(numTracks);
		input.m_relativePositionTolerance.setSize(numTracks);
		input.m_rotationTolerance.setSize(numTracks);
		input.m_scaleTolerance.setSize(numTracks);

		for (hkUint32 i=0; i < numTracks; i++)
		{
			const int paletteIdx = (params.m_trackIndexToPaletteIndex.getSize() > 0) ? params.m_trackIndexToPaletteIndex[i] : 0;
			const CompressionParams& currentBoneParams = params.m_parameterPalette[ paletteIdx ];

			input.m_absolutePositionTolerance[i] = currentBoneParams.m_absolutePositionTolerance;
			input.m_relativePositionTolerance[i] = currentBoneParams.m_relativePositionTolerance;
			input.m_rotationTolerance[i] = currentBoneParams.m_rotationTolerance;
			input.m_scaleTolerance[i] = currentBoneParams.m_scaleTolerance;
		}

		hkTrackAnalysis::analyze( input, staticMask, staticDOFs.begin() );
		hkTrackAnalysis::calcStats( staticMask, numTracks, stats );

		// Copy static DOFs
		const int numStatic = stats.m_staticPosDOF + stats.m_staticRotDOF + stats.m_staticSclDOF;
		m_staticDOFsIdx = dataBufferOffset;
		const int deltaSize = numStatic * sizeof(hkReal);
		dataBuffer.expandBy( deltaSize );
		hkString::memCpy( dataBuffer.begin() + m_staticDOFsIdx, staticDOFs.begin(), deltaSize);
		dataBufferOffset += deltaSize;

	}

	// The toal number of dynamic degrees of freedom in the animation.
	hkUint32 numD = stats.m_dynamicPosDOF + stats.m_dynamicRotDOF + stats.m_dynamicSclDOF;
	m_qFormat.m_numD = numD;

	if ( numD == 0)
	{
		HK_WARN_ALWAYS(0xabba554e, "Nothing to compress : All tracks are static or clear");
	}
	
	// Split interleaved data into channels
	// The interleaved data is laid out as
	// Bone1.Px(frame 1..n), Bone1.Py(frame1..n)...BoneN.Qz(frame1..n)
	hkLocalArray<hkReal> realData( numD * numPoses );
	realData.setSize( numD * numPoses );

	hkTrackAnalysis::AnalysisSplit splitter;
	splitter.m_data = raw.m_transforms;
	splitter.m_numberOfTracks = numTracks;
	splitter.m_numberOfPoses = numPoses;
	splitter.m_channelAlign = 0;
	splitter.m_staticMask = reinterpret_cast<hkUint16*> (dataBuffer.begin() + m_staticMaskIdx);
	hkTrackAnalysis::channelSplit( splitter, realData.begin() );


	// Examine the data and work out the quantization header
	HK_ASSERT(0x65654334, params.m_parameterPalette[0].m_quantizationBits < 255);
	//m_qFormat.m_bitWidth = hkUint8(params.m_parameterPalette[0].m_quantizationBits);
	m_qFormat.m_preserved = 1;	// preserve the first real 
	m_qFormat.m_maxBitWidth = 0;

	// Allocate space in the buffer for the offset, scale and bitwidth arrays
	{
		int alignedBitWidthSize = HK_NEXT_MULTIPLE_OF(4, sizeof(hkUint8) * numD );
		dataBuffer.expandBy( numD * 2 * sizeof(hkReal) + alignedBitWidthSize );

		m_qFormat.m_offsetIdx = dataBufferOffset;
		dataBufferOffset += numD * sizeof(hkReal);

		m_qFormat.m_scaleIdx = dataBufferOffset;
		dataBufferOffset += numD * sizeof(hkReal);

		m_qFormat.m_bitWidthIdx = dataBufferOffset;
		dataBufferOffset += alignedBitWidthSize;
	}

	hkReal* offsetBuffer = reinterpret_cast<hkReal*>( dataBuffer.begin() + m_qFormat.m_offsetIdx );
	hkReal* scaleBuffer = reinterpret_cast<hkReal*>( dataBuffer.begin() + m_qFormat.m_scaleIdx );
	hkUint8* bitWidthBuffer = reinterpret_cast<hkUint8*>( dataBuffer.begin() + m_qFormat.m_bitWidthIdx );

	// Init scale and offset arrays
	for (hkUint32 i=0 ; i < numD; i++)
	{
		offsetBuffer[i] =  HK_REAL_MAX;
		scaleBuffer[i]  = -HK_REAL_MAX;
	}


	// Delta compress in blocks
	// Compute the quantization min and max as we go along
	{
		hkUint32 lastBlockSize = numPoses % m_blockSize;
		hkUint32 fullBlocks = numPoses / m_blockSize;
		hkQuantizeDesc tmpDesc;

		// Full blocks
		for (hkUint32 b=0; b <= fullBlocks; b++)
		{
			hkReal*  rBase = realData.begin() + b * m_blockSize;

			hkUint32 idx = 0; // Nth dynamic degree of freedom 
			for (hkUint32 boneIdx = 0; boneIdx < numTracks; boneIdx++)
			{
				hkUint16* staticMask = reinterpret_cast<hkUint16*> (dataBuffer.begin() + m_staticMaskIdx);

				// shift over by 6 so we can access the dynamic masks
				hkUint32 detail = (staticMask[boneIdx] >> 6);

				for (int dofIdx = 0; dofIdx < 10; dofIdx++)
				{
					// make sure this dof is dynamic
					if (! ((detail >> dofIdx) & 0x1) )
					{
						continue;
					}

					hkUint32 numVals = (b==fullBlocks) ? lastBlockSize : m_blockSize;

					if (numVals > 0)
					{
						const int paletteIdx = (params.m_trackIndexToPaletteIndex.getSize() > 0) ? params.m_trackIndexToPaletteIndex[boneIdx] : 0;
						CompressionParams currentBoneParams = params.m_parameterPalette[paletteIdx];
						hkDeltaTransform( rBase + (idx * numPoses), numVals);		

						// Calc quantisation params
						// The number of bits doesn't matter here, only the preserve value.
						hkQuantizeCalcDesc( rBase + (idx * numPoses), numVals, tmpDesc, 0, m_qFormat.m_preserved);

						offsetBuffer[idx] = hkMath::min2( tmpDesc.m_offset, offsetBuffer[idx] );
						scaleBuffer[idx] = hkMath::max2( tmpDesc.m_offset + tmpDesc.m_scale, scaleBuffer[idx] );
						bitWidthBuffer[idx] = (hkUint8) currentBoneParams.m_quantizationBits;
						m_qFormat.m_maxBitWidth = hkMath::max2( m_qFormat.m_maxBitWidth, bitWidthBuffer[idx] );
					}
					idx++;
				}
			}
		}
	}

	// Readjust min max to offset scale
	{
		for (hkUint32 idx=0; idx < numD; idx++)
		{
			scaleBuffer[idx] -= offsetBuffer[idx];
		}
	}

	// Init the quantize header
	hkQuantizeDesc desc;
	//desc.m_bitWidth = m_qFormat.m_maxBitWidth;
	desc.m_preserved = m_qFormat.m_preserved;

	// compute the total space needed for a full block
	m_totalBlockSize = 0;
	for (hkUint32 idx=0; idx < numD; idx++)
	{
		desc.m_bitWidth = bitWidthBuffer[idx];
		m_totalBlockSize += hkCalcQuantizedSize( m_blockSize, desc );
	}

	m_quantizedDataIdx = dataBufferOffset;

	// Qunatize the data
	// The quantized data is rearranged as interleaved blocks
	//
	// B1.pX(frame1..blockSize-1), B2.pY(frame1..blockSize-1).........BN.qZ(frame1..blockSize-1) = blockSize * numBones * numC elems
	// B1.pX(frameBlockSize..frameBlockSize*2-1), B1.pY(frameBlockSize..frameBlockSize*2-1)....  = blockSize * numBones * numC elems
	// B1.pX(frameBlockSize*numBlocks-1...frameNumFrames)....                                   <= blockSize * numBones * numC elems 
	{
		hkUint32 lastBlockSize = numPoses % m_blockSize;
		hkUint32 fullBlocks = numPoses / m_blockSize;
		hkUint32 totalQSize = 0;
		
		for (hkUint32 b=0; b <= fullBlocks; b++)
		{
			hkReal*  rBase = realData.begin() + b * m_blockSize;

			for (hkUint32 idx=0; idx < numD; idx++)
			{
				hkReal* offsetBuffer = reinterpret_cast<hkReal*>( dataBuffer.begin() + m_qFormat.m_offsetIdx );
				hkReal* scaleBuffer = reinterpret_cast<hkReal*>( dataBuffer.begin() + m_qFormat.m_scaleIdx );
				hkUint8* bitWidthBuffer = reinterpret_cast<hkUint8*>( dataBuffer.begin() + m_qFormat.m_bitWidthIdx );

				desc.m_bitWidth = bitWidthBuffer[idx];
				hkUint32 fullQSize = hkCalcQuantizedSize( m_blockSize, desc );
				hkUint32 lastQSize = (lastBlockSize > 0) ? hkCalcQuantizedSize( lastBlockSize, desc ) : 0;

				hkUint32 numVals = (b==fullBlocks) ? lastBlockSize : m_blockSize;
				hkUint32 qSize   = (b==fullBlocks) ? lastQSize : fullQSize;

				desc.m_offset = offsetBuffer[idx];
				desc.m_scale  = scaleBuffer[idx];

				dataBuffer.expandBy( qSize );
				hkUint8* quantizeBase = dataBuffer.begin() + m_quantizedDataIdx + totalQSize;

				if (numVals > 0)
				{
					hkQuantizeReal( rBase + (idx * numPoses), numVals, desc, quantizeBase );
				}
				totalQSize += qSize;
			}
		}
		m_quantizedDataSize = totalQSize;
	}

	// Compute m_lastBlockSize
	m_lastBlockSize = 0;
	{
		int lastBlockFrames = m_numberOfPoses % m_blockSize;
		hkQuantizeDesc desc;
		desc.m_preserved = m_qFormat.m_preserved;
		hkUint8* bitWidthBuffer = reinterpret_cast<hkUint8*>( dataBuffer.begin() + m_qFormat.m_bitWidthIdx );
		for (unsigned int i=0; i<this->m_qFormat.m_numD; i++)
		{
			desc.m_bitWidth = bitWidthBuffer[i];
			m_lastBlockSize += hkCalcQuantizedSize( lastBlockFrames, desc );
		}
	}

	// Allocate and copy data buffer
	m_numDataBuffer = dataBuffer.getSize();
	m_dataBuffer = hkAllocateChunk<hkUint8>( m_numDataBuffer, HK_MEMORY_CLASS_ANIM_COMPRESSED);
	hkString::memCpy(m_dataBuffer, dataBuffer.begin(), m_numDataBuffer);

	HK_REPORT_SECTION_BEGIN(0x432434a5, "Quantization");
	// Size from toolchain
	hkUint32 tcSize = raw.m_numTransforms * sizeof(hkReal) * 16;

	hkUint32 staticDataSize =	sizeof(hkDeltaCompressedSkeletalAnimation) +
								m_quantizedDataIdx;	

	hkUint32 dynamicDataSize =  m_quantizedDataSize;

	HK_REPORT("Original Size " << tcSize << " Compressed Size " << staticDataSize + dynamicDataSize << " [" << staticDataSize << " static/incompressible]");
	HK_REPORT("Compression Ratio: " << hkReal(tcSize)/ hkReal(staticDataSize + dynamicDataSize ) << ":1" );
	HK_REPORT_SECTION_END();


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
