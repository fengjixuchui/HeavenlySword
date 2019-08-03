/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkanimation/hkAnimation.h>
#include <hkanimation/animation/waveletcompressed/hkWaveletSkeletalAnimation.h>
#include <hkanimation/animation/interleaved/hkInterleavedSkeletalAnimation.h>
#include <hkcompression/hkCompression.h>
#include <hkbase/memory/hkLocalArray.h>
#include <hkanimation/animation/util/hkTrackAnalysis.h>
#include <hkanimation/playback/cache/hkChunkCache.h>

static inline hkUint32 hkRoundUpPow2(hkUint32 n)
{
	n--;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	n++;
	return n;
}

hkWaveletSkeletalAnimation::hkWaveletSkeletalAnimation(const hkInterleavedSkeletalAnimation& raw, const CompressionParams& params)
{
	PerTrackCompressionParams ptparams;
	ptparams.m_trackIndexToPaletteIndex.setSize(0); // 0 size implies use the first palette entry
	ptparams.m_parameterPalette.pushBack(params);
	initialize(raw, ptparams);
}

hkWaveletSkeletalAnimation::hkWaveletSkeletalAnimation(const hkInterleavedSkeletalAnimation& raw, const PerTrackCompressionParams& params)
{
	initialize(raw, params);
}

void hkWaveletSkeletalAnimation::initialize(const hkInterleavedSkeletalAnimation& raw, const PerTrackCompressionParams& params)
{	
	this->m_type = hkSkeletalAnimation::HK_WAVELET_COMPRESSED_ANIMATION;

	this->m_duration = raw.m_duration;
	this->m_numberOfTracks = raw.m_numberOfTracks;
	this->m_extractedMotion = raw.m_extractedMotion;
	this->m_annotationTracks = raw.m_annotationTracks;
	this->m_numAnnotationTracks = raw.m_numAnnotationTracks;

	const int numTracks = raw.m_numberOfTracks;				// Number of tracks
	const int numPoses = raw.m_numTransforms / numTracks;	// Number of original poses
	m_numberOfPoses = numPoses;

	// We can't have variable block size or preserve values, so just use the first CompressionParam
	m_blockSize = hkMath::min2(params.m_parameterPalette[0].m_blockSize, hkUint16(numPoses));
	m_blockSize = (hkUint16)hkRoundUpPow2( m_blockSize );

	// check that the block size and preserve values are identical for all the palette
	{
		HK_ON_DEBUG( int preserve  = params.m_parameterPalette[0].m_preserve );
		for (int i=1; i<params.m_parameterPalette.getSize(); i++)
		{
			HK_ASSERT2(0x3232345e, m_blockSize == params.m_parameterPalette[i].m_blockSize, "Variable blocksize not supported in hkWaveletSkeletalAnimation");
			HK_ASSERT2(0x5789876e, preserve == params.m_parameterPalette[i].m_preserve, "Variable preserve not supported in hkWaveletSkeletalAnimation");
		}
	}

	hkUint32 numBlocks = (numPoses + m_blockSize-1) / m_blockSize; // Number of blocks

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
		// Worst Case (all dynamic tracks)
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

		for (int i=0; i < numTracks; i++)
		{
			int paletteIdx = (params.m_trackIndexToPaletteIndex.getSize() > 0) ? params.m_trackIndexToPaletteIndex[i] : 0;
			const CompressionParams& currentBoneParams = params.m_parameterPalette[paletteIdx];

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

	// The total number of dynamic degrees of freedom in the animation.
	hkUint32 numD = stats.m_dynamicPosDOF + stats.m_dynamicRotDOF + stats.m_dynamicSclDOF;
	m_qFormat.m_numD = numD;

	if ( numD == 0)
	{
		HK_WARN_ALWAYS(0xabba554e, "Nothing to compress : All tracks are static or clear");
	}

	// Round the number of frames to match the blocksize
	hkUint32 numPosesRound = ((numPoses + m_blockSize-1) / m_blockSize) * m_blockSize;

	// The interleaved data is laid out as
	// Bone1.Px(frame 1..n), Bone1.Py(frame1..n)...BoneN.Qz(frame1..n)
	hkLocalArray<hkReal> realData( numD * numPosesRound );
	realData.setSize( numD * numPosesRound );

	hkTrackAnalysis::AnalysisSplit splitter;
	splitter.m_data = raw.m_transforms;
	splitter.m_numberOfTracks = numTracks;
	splitter.m_numberOfPoses = numPoses;
	splitter.m_channelAlign = numPosesRound;
	splitter.m_staticMask = reinterpret_cast<hkUint16*> (dataBuffer.begin() + m_staticMaskIdx);
	hkTrackAnalysis::channelSplit( splitter, realData.begin() );

	// Examine the data and work out the quantization header
	// Later : split this into a couple of ranges - for when some tracks are more dominant.
	HK_ASSERT(0x65654334, params.m_parameterPalette[0].m_preserve < 255);
	m_qFormat.m_preserved = hkUint8(params.m_parameterPalette[0].m_preserve); // keep the first real?

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

	// Compress in blocks
	// Compute the quantization min and max as we go along
	{
		hkQuantizeDesc tmpDesc;
		hkUint32 trunc = 0;

		// Full blocks
		for (hkUint32 b=0; b < numBlocks; b++)
		{
			hkReal*  rBase = realData.begin() + b * m_blockSize;

			hkUint32 idx = 0; // Nth dynamic degree of freedom 
			// We need to iterate through the bones here so that we can access the palette correctly
			for (int boneIdx = 0; boneIdx < numTracks; boneIdx++)
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

					const hkUint32 numVals = m_blockSize;
					if (numVals > 0)
					{
						const int paletteIdx = (params.m_trackIndexToPaletteIndex.getSize() > 0) ? params.m_trackIndexToPaletteIndex[boneIdx] : 0;
						CompressionParams currentBoneParams = params.m_parameterPalette[ paletteIdx ];

						hkWaveletTransform( rBase + (idx * numPosesRound), numVals );
						hkWaveletCoefficientThreshold(rBase + (idx * numPosesRound), numVals, currentBoneParams.m_truncProp );

						hkReal* val = rBase + (idx * numPosesRound);
						for (hkUint32 v=0; v < numVals; v++)
						{
							if (val[v]==0.0f) trunc++;
						}		

						// Calc quantisation params
						// The number of bits doesn't matter here, only the preserve value.
						hkQuantizeCalcDesc( rBase + (idx * numPosesRound), numVals, tmpDesc, 0, m_qFormat.m_preserved);

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
	hkQuantizeDesc qDesc;
	qDesc.m_preserved = m_qFormat.m_preserved;

	// Init the encoding header
	hkBlockDesc bDesc;
	bDesc.m_preserved = m_qFormat.m_preserved;

	// Qunatize & encode the data
	// The quantized data is rearranged as interleaved blocks
	//
	// B1.pX(frame1..blockSize-1), B2.pY(frame1..blockSize-1).........BN.qZ(frame1..blockSize-1) = blockSize * numBones * numC elems
	// B1.pX(frameBlockSize..frameBlockSize*2-1), B1.pY(frameBlockSize..frameBlockSize*2-1)....  = blockSize * numBones * numC elems
	// B1.pX(frameBlockSize*numBlocks-1...frameNumFrames)....                                   <= blockSize * numBones * numC elems 
	{
		hkUint32 encodedSize = 0;
		// NB - The qSisze and qDesc.m_bitWidth will get overwritten in the main loop.
		//      Using them here to figure out how much scratch space to reserve
		qDesc.m_bitWidth = m_qFormat.m_maxBitWidth;
		hkUint32 qSize = hkCalcQuantizedSize( m_blockSize, qDesc );
		hkLocalArray<hkUint8> quantized( qSize );
		quantized.setSize( qSize );

		// Allocate block lookup index
		int deltaSize = numBlocks * sizeof( hkUint32 );
		m_blockIndexIdx = dataBufferOffset;
		dataBuffer.expandBy(deltaSize);
		dataBufferOffset += deltaSize;
		m_blockIndexSize = numBlocks;

		m_quantizedDataIdx = dataBufferOffset;

		// Full blocks
		for (hkUint32 b=0; b < numBlocks; b++)
		{
			hkReal*  rBase = realData.begin() + b * m_blockSize;

			// Set up the index
			hkUint32* blockIndex = reinterpret_cast<hkUint32*>( dataBuffer.begin() + m_blockIndexIdx );
			blockIndex[b] = encodedSize;

			// All dynamic tracks
			for (hkUint32 idx=0; idx < numD; idx++)
			{
				hkUint32 numVals = m_blockSize;
				const hkReal* data = rBase + (idx * numPosesRound);

				hkReal* offsetBuffer = reinterpret_cast<hkReal*>( dataBuffer.begin() + m_qFormat.m_offsetIdx );
				hkReal* scaleBuffer = reinterpret_cast<hkReal*>( dataBuffer.begin() + m_qFormat.m_scaleIdx );
				hkUint8* bitWidthBuffer = reinterpret_cast<hkUint8*>( dataBuffer.begin() + m_qFormat.m_bitWidthIdx );

				qDesc.m_bitWidth = bitWidthBuffer[idx];
				qDesc.m_offset = offsetBuffer[idx];
				qDesc.m_scale  = scaleBuffer[idx];

				bDesc.m_bitWidth = qDesc.m_bitWidth;

				hkUint32 qSize = hkCalcQuantizedSize( m_blockSize, qDesc );
				
				if (numVals > 0)
				{
					// Quantize
					hkQuantizeReal( data, numVals, qDesc, quantized.begin() );

					// Work out the zero sym - if quantisation changes we need to change this	
					bDesc.m_runSymbol = hkUint32 ( (1 << qDesc.m_bitWidth) * (-qDesc.m_offset) / qDesc.m_scale );
					bDesc.m_runSymbol = (bDesc.m_runSymbol == hkUint32(1 << qDesc.m_bitWidth)) ? bDesc.m_runSymbol-1 : bDesc.m_runSymbol;

					//Calc the encoded size
					hkUint32 size = hkCalcBlockEncodedSize( quantized.begin(), qSize, bDesc );
					dataBuffer.expandBy(size);
					
					// Encode the data
					hkUint8* encodeBase = dataBuffer.begin() + m_quantizedDataIdx + encodedSize;
					hkBlockEncode( quantized.begin(), qSize, bDesc, encodeBase );

					encodedSize += size;

				}
			}
		}

		m_quantizedDataSize = encodedSize;
	}

	// Allocate and copy data buffer
	m_numDataBuffer = dataBuffer.getSize();
	m_dataBuffer = hkAllocateChunk<hkUint8>( m_numDataBuffer, HK_MEMORY_CLASS_ANIM_COMPRESSED);
	hkString::memCpy(m_dataBuffer, dataBuffer.begin(), m_numDataBuffer);

	// Log Results
	HK_REPORT_SECTION_BEGIN(0x432434a5, "Compression");
	// Size from toolchain
	hkUint32 tcSize = raw.m_numTransforms * sizeof(hkReal) * 16;

	hkUint32 staticDataSize = 	sizeof(hkWaveletSkeletalAnimation) +
								m_quantizedDataIdx;

	hkUint32 dynamicDataSize = m_quantizedDataSize;

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
