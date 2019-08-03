/*
 * Copyright (c) 2005 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 * 
 */

#include <stdlib.h>
#include <math.h>
#include "icemeshinternal.h"

using namespace Ice;
using namespace Ice::MeshProc;

#ifndef __SPU__
void CompressIndexes6Byte(U16 *pSrc, U16 *pDst, U32 numTris)
{
	// 8 at a time, 2 per quad, 4 quads
	for(U32 i = 0; i < numTris; i+=8)
	{
		for(U32 ii = i; ii < i+8; ++ii)
		{
			pDst[ii * 3 + 0] = pSrc[ii * 4 + 0];
			pDst[ii * 3 + 1] = pSrc[ii * 4 + 1];
			pDst[ii * 3 + 2] = pSrc[ii * 4 + 2];
		}
	}
}
#endif

#ifndef __SPU__
static inline void StoreUnalignedF32(U8 * __restrict const pDst, F32 const src)
{
	IntFloat srcTemp;
	srcTemp.m_f32 = src;

	pDst[0] = srcTemp.m_u32 >> 24;
	pDst[1] = srcTemp.m_u32 >> 16;
	pDst[2] = srcTemp.m_u32 >> 8;
	pDst[3] = srcTemp.m_u32;
}

void InsertF32(F32 const * const pSrc, F32 * __restrict const pDst, U32 const dstStride, U32 const numComponents,
	U16 const *pReindex, U32 const numVerts)
{
	U8 * __restrict pDstTemp = (U8 * __restrict)pDst;

	for (U32 ii = 0; ii < numVerts; ii++) 
	{
		U16 index = pReindex[ii] >> 2;
		for (U32 jj = 0; jj < numComponents; jj++) 
		{
			StoreUnalignedF32(pDstTemp + jj * 4, pSrc[index + jj]);
		}
		pDstTemp += dstStride;
	}
}
#endif//__SPU__

#define SIMPLE_FP_CONVERT 1
#define HANDLE_FP_SPECIALS 1
#define HANDLE_EXP_OVERFLOW 1
#define DO_FP_ROUNDING 1
// Smallest positive normalized half
#define HALF_NRM_MIN			(6.10351562e-05)

#ifndef __SPU__
static inline void StoreUnalignedF16(U8 * __restrict const pDst, U16 const src)
{
	pDst[0] = src >> 8;
	pDst[1] = src;
}

void InsertF16(F32 const * const pSrc, U16 * __restrict const pDst, U32 const dstStride, U32 const numComponents,
	U16 const *pReindex, U32 const numVerts, IceQuadWord const scale, IceQuadWord const bias)
{
	F32 const *pScale = (F32 *)&scale;
	F32 const *pBias = (F32 *)&bias;
	U8 * __restrict pDstTemp = (U8 * __restrict)pDst;

	for (U32 ii = 0; ii < numVerts; ii++) 
	{
		U16 index = pReindex[ii] >> 2;
		for (U32 jj = 0; jj < numComponents; jj++) 
		{
			// Remember, the pReindex table is scaled up by 16!
			IntFloat srcTemp;
			srcTemp.m_f32 = pSrc[index + jj] * pScale[jj] + pBias[jj];
			U32 f = srcTemp.m_u32;
			U32 s = (f & 0x80000000) >> 16;
			I32 e = ((f & 0x7F800000) >> 23) - (127 - 15);
			U32 m = (f & 0x007FFFFF);
			U16 dstTemp;

#if SIMPLE_FP_CONVERT
			if (e <= 0)
				dstTemp = 0;	      // Small normalized float, a denormalized float, or zero
			else if (e > 30)
				dstTemp = s | 0x7BFF; // Too big -- store largest representable half with the same sign.
			else
				dstTemp = s | (e << 10) | (m >> 13);
#else
#if HANDLE_FP_SPECIALS
			if (e <= 0) {
				// Small normalized float, a denormalized float, or zero
				if (e < -10)
					dstTemp = 0;      // convert to 0.0f
				else
				{
					// e is between -10 and 0. f is a normalized float whose 
					// magnitude is less than HALF_NRM_MIN.
					// Convert to a denormalized half.
					m = (m | 0x00800000) >> (1 - e);

					// Round to nearest
					if (m & 0x00001000)
						m += 0x00002000;

					dstTemp = s | (m >> 13);
				}
			}
			else if (e == 0xFF - (127 - 15))
			{
				if (m == 0)
					dstTemp = s | 0x7C00; // +-Inf
				else
					dstTemp = 0x7DFF;     // NAN 
			}
			else
#endif
			{
				// a normalized float.

#if DO_FP_ROUNDING
				// Round to nearest, round "0.5" up
				if (m & 0x00001000) {
					m += 0x00002000;

					if (m & 0x00800000) {
						m = 0;
						e += 1;
					}
				}
#endif
#if HANDLE_EXP_OVERFLOW
				// Handle exponent overflow
				if (e > 30)
					dstTemp = s | 0x7C00;
				else
#endif
				{
					dstTemp = s | (e << 10) | (m >> 13);
				}
			}
#endif
			StoreUnalignedF16(pDstTemp + jj * 2, dstTemp);
		}
		pDstTemp += dstStride;
	}
}
#endif//__SPU__

#ifndef __SPU__
static inline void StoreUnalignedI16(U8 * __restrict const pDst, I16 const src)
{
	pDst[0] = src >> 8;
	pDst[1] = src;
}

void InsertI16(F32 const * const pSrc, I16 * __restrict const pDst, U32 const dstStride, U32 const numComponents,
	U16 const *pReindex, U32 const numVerts, IceQuadWord const scale, IceQuadWord const bias)
{
	F32 const *pScale = (F32 *)&scale;
	F32 const *pBias = (F32 *)&bias;
	U8 * __restrict pDstTemp = (U8 * __restrict)pDst;

	for (U32 ii = 0; ii < numVerts; ii++) 
	{
		U16 index = pReindex[ii] >> 2;
		for (U32 jj = 0; jj < numComponents; jj++) 
		{
			StoreUnalignedI16(pDstTemp + jj * 2, I16(pSrc[index + jj] * pScale[jj] + pBias[jj]));
		}
		pDstTemp += dstStride;
	}
}
#endif

#ifndef __SPU__
void InsertU8(F32 const * const pSrc, U8 * __restrict const pDst, U32 const dstStride, U32 const numComponents,
	U16 const *pReindex, U32 const numVerts, IceQuadWord const scale, IceQuadWord const bias)
{
	F32 const *pScale = (F32 *)&scale;
	F32 const *pBias = (F32 *)&bias;
	U8 * __restrict pDstTemp = pDst;

	for (U32 ii = 0; ii < numVerts; ii++) 
	{
		U16 index = pReindex[ii] >> 2;
		for (U32 jj = 0; jj < numComponents; jj++) 
		{
			pDstTemp[jj] = U8(pSrc[index + jj] * pScale[jj] + pBias[jj]);
		}
		pDstTemp += dstStride;
	}
}
#endif

#ifndef __SPU__
static inline void StoreUnalignedU32(U8 * __restrict const pDst, U32 const src)
{
	pDst[0] = src >> 24;
	pDst[1] = src >> 16;
	pDst[2] = src >> 8;
	pDst[3] = src;
}

void InsertX11Y11Z10(F32 const * const pSrc, U32 * __restrict const pDst, U32 const dstStride, U32 const /*numComponents*/,
	U16 const *pReindex, U32 const numVerts, IceQuadWord const scale, IceQuadWord const bias)
{
	F32 const *pScale = (F32 *)&scale;
	F32 const *pBias = (F32 *)&bias;
	U8 * __restrict pDstTemp = (U8 * __restrict)pDst;

	for (U32 ii = 0; ii < numVerts; ii++) 
	{
		U16 index = pReindex[ii] >> 2;
		StoreUnalignedU32(pDstTemp, MAKE_X11Y11Z10N(pSrc[index + 0] * pScale[0] + pBias[0],
			pSrc[index + 1] * pScale[1] + pBias[1], pSrc[index + 2] * pScale[2] + pBias[2]));
		pDstTemp += dstStride;
	}
}
#endif

void Ice::MeshProc::CmdOutputIndexes(U32 *pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);

	// If we're generating draw calls, transfer new index buffer to
	// output buffer (and compress it).
	U32 indexCount = pStaticMem[kStaticIndexCount] - pStaticMem[kStaticHaloIndexCount];
	U32 triCount = indexCount / 3;
	if (pStaticMem[kStaticOutputGenerateDrawCalls] != 0)
	{
		// Allocate up to the next multiple of 3 quadwords.
		U16 *pOutputIndexes = (U16*)MeshOutputAllocate(pStaticMem,
			((triCount + 0x7) >> 3) * 0x30, 128, &pStaticMem[kStaticIndexOutputPtr]);
#ifndef __SPU__
		// Collect some stats on what is being output.
		g_meshProcessingStats.m_indexesNum += triCount;
		g_meshProcessingStats.m_indexesSize += ((triCount + 0x7) >> 3) * 0x30;
#endif
		CompressIndexes6Byte( (U16*)pStaticMem[kStaticIndexPtr], pOutputIndexes,
			triCount );
	}
}

#ifndef __SPU__
void SelectVertsGeneric(const U8 *pIn, U8 *pOut, const U16 *pReindex, const U32 reindexCount, const U32 vertSize)
{
	for(U32 i = 0; i < reindexCount; ++i)
	{
		// Remember, the reindex table is scaled up by 16!
		CopyBytes( pOut + i*vertSize, pIn+(pReindex[i]/16)*vertSize, vertSize );
	}
}
#endif // __SPU__

// Unit test for SelectVerts() functions 
#ifdef __SPU__

#define TEST_SELECTVERTS 0

#if TEST_SELECTVERTS

static void SelectVerts(const U8 *pIn, U8 *pOut, const U16 *pReindex, const U32 reindexCount, const U32 vertSize)
{
	for(U32 i = 0; i < reindexCount; ++i)
	{
		// Remember, the reindex table is scaled up by 16!
		CopyBytes( pOut + i*vertSize, pIn+(pReindex[i]/16)*vertSize, vertSize );
	}
}

static U32 myrand(void)
{
	static U32 seed = 13;
	U32 ret = seed;
	seed = (seed * 19 + 37);;
	seed &= 0xFFFF0;
	seed >>= 4;
	return ret;
}

#define SV_BUF_SIZE 2048
#define SV_REINDEX_SIZE 128
static void TestSelectVerts(U32 *pStaticMem)
{
	PRINTF("Testing Select Verts\n");

	U32 * pIn      = (U32*)MeshMemAllocate(pStaticMem, SV_BUF_SIZE*sizeof(U32));
	U32 * pOut1    = (U32*)MeshMemAllocate(pStaticMem, SV_BUF_SIZE*sizeof(U32));
	U32 * pOut2    = (U32*)MeshMemAllocate(pStaticMem, SV_BUF_SIZE*sizeof(U32));
	U16 * pReindex = (U16*)MeshMemAllocate(pStaticMem, SV_REINDEX_SIZE*sizeof(U16));

	U32 testCount = 0;
	while(1)
	{
		// For 16N
		//U32 numVerts = ((myrand() % 4) + 1) * 8;
		//U32 stride   = ((myrand() % 4) + 1) * 16;
		// For 16N+k
		U32 numVerts = ((myrand() % 32) + 1);
		U32 stride   = ((myrand() % 10) + 1) * 4;

		U32 numInputVerts = 30;
		
		//PRINTF("Attempting size of %i * %i = %i\n", numVerts, stride, numVerts * stride);

		if (numVerts*stride > SV_BUF_SIZE*sizeof(U32))
			continue;

#if 0
		for(U32 i=0; i < numInputVerts; ++i)
		{
			U32 numWords = stride / 4;
			for(U32 ii = 0; ii < numWords; ++ii)
			{
				pIn[i*numWords + ii] = (ii << 16) | i;
			}
		}
#endif

		for(U32 i=0; i<SV_BUF_SIZE; ++i)
		{
			pIn[i]   = myrand();
			pOut1[i] = pOut2[i] = 0xdeadc0de;
		}

		for(U32 i=0; i<SV_REINDEX_SIZE; ++i)
		{
			pReindex[i] = (myrand() % numInputVerts) * 16;
		}

		SelectVerts((U8*)pIn, (U8*)pOut1, pReindex, numVerts, stride);
		SelectVertsGeneric((U8*)pIn, (U8*)pOut2, pReindex, numVerts, stride);
		bool passed = (memcmp(pOut1, pOut2, numVerts*stride) == 0);

		if (passed)
			continue;

		PRINTF("TEST %5d (stride=%3d, numVerts=%3d): %s\n", testCount++, stride, numVerts,
			passed ? "OK" : "FAILED!!!");
			
		U32 numQuads = (numVerts*stride + 15) / 16;
		PRINTF("There should be %d pairs of quads below\n", numQuads);
		for(U32 i=0; i<numQuads; ++i)
		{
			PRINTF("out1[%3d]: 0x%08X\t0x%08X\t0x%08X\t0x%08X\n",   i, pOut1[4*i+0], pOut1[4*i+1], pOut1[4*i+2], pOut1[4*i+3]);
			PRINTF("out2[%3d]: 0x%08X\t0x%08X\t0x%08X\t0x%08X\n\n", i, pOut2[4*i+0], pOut2[4*i+1], pOut2[4*i+2], pOut2[4*i+3]);
		}
	}
	MeshMemFree(pStaticMem, (U32)pIn);
}
#endif // 0

#endif // __SPU__

void Ice::MeshProc::CmdOutputCopiedNvStream(U32* pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);
	const U8 streamName       = (cmdQuad1.m_data.u8.B & 0x70) >> 4;

#ifdef __SPU__
	//TestSelectVerts(pStaticMem);
#endif

	CreateReindexTable(pStaticMem, true);

	U16* pReindex = (U16 *)pStaticMem[kStaticReindexPtr];
	U32 numVerts = pStaticMem[kStaticReindexCount];

	U16 *pFixedFormatPtr = (U16 *)pStaticMem[kStaticFixedFormatPtr];
	U16 *pStreamFormat = (U16 *)((U32)pFixedFormatPtr + pFixedFormatPtr[streamName + 1]);

	U32 stride = pStreamFormat[1];
	U32 size = (stride * ((numVerts + 15) & ~0xF) + 0xF) & ~0xF;      // Output is multiple of 16 vertexes
	U8* pDst = MeshOutputAllocate(pStaticMem, size, 16, &pStaticMem[kStaticStreamOutputPtr + streamName]);
#ifndef __SPU__
	// Collect some stats on what is being output.
	g_meshProcessingStats.m_vertexesNum += numVerts;
	g_meshProcessingStats.m_vertexesSize += size;
#endif
	U8* pSrc = (U8 *)pStaticMem[kStaticStreamWorkPtr + streamName];

	if (pStaticMem[kStaticReindexIsDummy] != 0)
	{
		CopyQWords(pDst, pSrc, size);
	}
	else
	{
#ifdef __SPU__
		switch (stride) {
		case 4:
			SelectVerts4Byte(pSrc, pDst, pReindex, numVerts);
			break;
		case 8:
			SelectVerts8Byte(pSrc, pDst, pReindex, numVerts);
			break;
		case 12:
			SelectVerts12Byte(pSrc, pDst, pReindex, numVerts);
			break;
		case 16:
			SelectVerts16N(pSrc, pDst, pReindex, numVerts);
			break;
		case 24:
			SelectVerts24Byte(pSrc, pDst, pReindex, numVerts);
			break;
		case 40:
			SelectVerts40Byte(pSrc, pDst, pReindex, numVerts);
			break;
		default:
			SelectVertsGeneric(pSrc, pDst, pReindex, numVerts, stride);
			break;
		}
#else
		SelectVertsGeneric(pSrc, pDst, pReindex, numVerts, stride);
#endif
	}
}

// Given an attribute ID, finds the corresponding uniform index.
// If no uniform table exists for the given attribute ID, then a uniform index of 16 is returned.
static inline U8 GetUniformIndex(U8 const * const pAttribIds, U8 const attribId)
{
	// In SPU code, this can be reduced to shufb, ceqb, gbb, clz
	U8 uniformIndex = 0;
	for(; uniformIndex < 16; ++uniformIndex) {
		if (pAttribIds[uniformIndex] == attribId)
			return uniformIndex;
	}

	return uniformIndex;
}

void Ice::MeshProc::InsertAttributeIntoNvStream(F32 const * const pSrcUniform, U8 * __restrict pDst, U32 const vertexStride,
	U32 const attribComponentCount, U8 const attribType, IceQuadWord scale, IceQuadWord bias,
	U16 const * const pReindex, U32 const numVerts, IceQuadWord * __restrict const pSaveSrcAndSab)
{
	static const F32 kScales[8] = {1.0f, 1.0f, 65535.0f / 2.0f, 1.0f, 1.0f, 255.0f, 1.0f, 1.0f};
	static const F32 kBiases[8] = {0.0f, 0.0f, -0.5f, 0.0f, 0.0f, 0.5f, 0.5f, 0.0f};

	// Adjust the scale and bias depening upon the attribute type.
	F32 attribScale = kScales[attribType];
	F32 attribBias = kBiases[attribType];
	scale.m_data.f32.A *= attribScale;
	scale.m_data.f32.B *= attribScale;
	scale.m_data.f32.C *= attribScale;
	scale.m_data.f32.D *= attribScale;
	bias.m_data.f32.A = bias.m_data.f32.A * attribScale + attribBias;
	bias.m_data.f32.B = bias.m_data.f32.B * attribScale + attribBias;
	bias.m_data.f32.C = bias.m_data.f32.C * attribScale + attribBias;
	bias.m_data.f32.D = bias.m_data.f32.D * attribScale + attribBias;

	if (pSaveSrcAndSab != NULL) {
		IceQuadWord srcUniformPtr;
		srcUniformPtr.m_data.u32.A = U32(pSrcUniform);
		pSaveSrcAndSab[0] = srcUniformPtr;
		pSaveSrcAndSab[1] = scale;
		pSaveSrcAndSab[2] = bias;
	}
	else {
		switch (attribType) {
			case kF32:
				InsertF32(pSrcUniform, (F32 *)pDst, vertexStride, attribComponentCount, pReindex, numVerts);
				break;
			case kF16:
				InsertF16(pSrcUniform, (U16 *)pDst, vertexStride, attribComponentCount, pReindex, numVerts, scale, bias);
				break;
			case kI16n:
				InsertI16(pSrcUniform, (I16 *)pDst, vertexStride, attribComponentCount, pReindex, numVerts, scale, bias);
				break;
			case kI16:
				InsertI16(pSrcUniform, (I16 *)pDst, vertexStride, attribComponentCount, pReindex, numVerts, scale, bias);
				break;
			case kX11Y11Z10n:
				InsertX11Y11Z10(pSrcUniform, (U32 *)pDst, vertexStride, attribComponentCount, pReindex, numVerts, scale, bias);
				break;
			case kU8n:
				InsertU8(pSrcUniform, (U8 *)pDst, vertexStride, attribComponentCount, pReindex, numVerts, scale, bias);
				break;
			case kU8:
				InsertU8(pSrcUniform, (U8 *)pDst, vertexStride, attribComponentCount, pReindex, numVerts, scale, bias);
				break;
			default:
				PRINTF("invalid attribute type: %d\n", attribType);
				ICE_ASSERT(attribType < kNumTypes);
				break;
		}
	}
}

static void InsertAttributeIntoNvStreamWithScaleAndBias(U8 const * const pAttribInfo, U8 const * const pAttribIds,
	U32 const fixedFormatPtr, U32 const variableFormatPtr, U16 const * const pScaleAndBiasLocs,
	F32 const * const * const pUniformPtrs, U8 * __restrict const pDst, U32 const vertexStride,
	U16 const * const pReindex, U32 const numVerts, IceQuadWord * __restrict const pSaveSrcAndSab)
{
	// Get information about the attribute.
	U8 attribId = pAttribInfo[0];
	ICE_ASSERT(attribId > 0);
	U8 attribComponentCount = pAttribInfo[1] & 0xF;
	U8 attribType = pAttribInfo[1] >> 4;
	U8 scaleAndBiasFlags = pAttribInfo[2];
	U8 attribOffset = pAttribInfo[3];

	U8 uniformIndex = GetUniformIndex(pAttribIds, attribId);
	if(uniformIndex < 16)
	{
		// Get the scales and biases, if they exist.
		IceQuadWord scale;
		IceQuadWord bias;
		if (scaleAndBiasFlags & kScaleAndBiasOn) {
			U32 scaleAndBiasBase = fixedFormatPtr;
			if (scaleAndBiasFlags & kScaleAndBiasVariable)
				scaleAndBiasBase = variableFormatPtr;
			F32 const *pScales = (F32 *)(scaleAndBiasBase + pScaleAndBiasLocs[scaleAndBiasFlags & 0xF]);
			F32 const *pBiases = pScales + attribComponentCount;
			scale.m_data.f32.A = 1.0f / pScales[0];
			scale.m_data.f32.B = 1.0f / pScales[1];
			scale.m_data.f32.C = 1.0f / pScales[2];
			scale.m_data.f32.D = 1.0f / pScales[3];
			bias.m_data.f32.A = -pBiases[0] * scale.m_data.f32.A;
			bias.m_data.f32.B = -pBiases[1] * scale.m_data.f32.B;
			bias.m_data.f32.C = -pBiases[2] * scale.m_data.f32.C;
			bias.m_data.f32.D = -pBiases[3] * scale.m_data.f32.D;
		}
		else {
			// Default scale and bias.
			scale.m_data.f32.A = 1.0f;
			scale.m_data.f32.B = 1.0f;
			scale.m_data.f32.C = 1.0f;
			scale.m_data.f32.D = 1.0f;
			bias.m_data.f32.A = 0.0f;
			bias.m_data.f32.B = 0.0f;
			bias.m_data.f32.C = 0.0f;
			bias.m_data.f32.D = 0.0f;
		}

		F32 const *pSrcUniform = pUniformPtrs[uniformIndex];
		InsertAttributeIntoNvStream(pSrcUniform, pDst + attribOffset, vertexStride, attribComponentCount, attribType,
			scale, bias, pReindex, numVerts, pSaveSrcAndSab);
	}
}

static inline U8 const *GetAttributeInfoPtr(U16 const * const pFormat, U16 const attributeId)
{
	U32 attribCount = pFormat[0];
	U8 const *pAttribInfo = (U8 *)&pFormat[2];

	// Find the correct format word and fill in type and component count
	for (U32 iAttrib = 0; iAttrib < attribCount; ++iAttrib) {
		if (attributeId == pAttribInfo[4 * iAttrib])
			return pAttribInfo + 4 * iAttrib;
	}

	// Could not find attribute!
	ICE_ASSERT(0);
	return NULL;
}

void Ice::MeshProc::CmdInsertAttributeIntoNvStream(U32 *pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);

	U8 const streamName = (cmdQuad1.m_data.u8.B & 0x70) >> 4;

	CreateReindexTable(pStaticMem, true);

	U16 const *pFixedFormatPtr = (U16 *)pStaticMem[kStaticFixedFormatPtr];
	U16 const *pStreamFormat = (U16 *)((U32)pFixedFormatPtr + pFixedFormatPtr[streamName + 1]);
	U32 attribCount = pStreamFormat[0];

	U8 const *pAttribInfo = GetAttributeInfoPtr(pStreamFormat, cmdQuad1.m_data.u16.B);
	U8 const *pAttribIds  = (U8 *)&pStaticMem[kStaticAttributeId];
	U32 fixedFormatPtr = pStaticMem[kStaticFixedFormatPtr];
	U32 variableFormatPtr = pStaticMem[kStaticVariableFormatPtr];
	U16 const *pScaleAndBiasLocs = pStreamFormat + 2 + attribCount * sizeof(U32) / sizeof(U16);
	F32 const * const *pUniformPtrs = (F32 **)&pStaticMem[kStaticUniformPtr];
	U8 * __restrict pDst = (U8 *)pStaticMem[kStaticStreamWorkPtr + streamName];
	U32 vertexStride = pStreamFormat[1];
	U16 const *pReindex = (U16 *)pStaticMem[kStaticReindexPtr];
	ICE_ASSERT(pReindex != 0);
	U32 numVerts = (pStaticMem[kStaticReindexCount] + 3) & ~0x3;

	InsertAttributeIntoNvStreamWithScaleAndBias(pAttribInfo, pAttribIds, fixedFormatPtr, variableFormatPtr,
		pScaleAndBiasLocs, pUniformPtrs, pDst, vertexStride, pReindex, numVerts, NULL);
}

typedef void (*CustomCompressFunc)(U32 numVerts, U32 pDst, U32 constDataPtr,
	F32 *pUniform0, IceQuadWord scale0, IceQuadWord bias0, F32 *pUniform1, IceQuadWord scale1, IceQuadWord bias1,
	F32 *pUniform2, IceQuadWord scale2, IceQuadWord bias2, F32 *pUniform3, IceQuadWord scale3, IceQuadWord bias3,
	F32 *pUniform4, IceQuadWord scale4, IceQuadWord bias4, F32 *pUniform5, IceQuadWord scale5, IceQuadWord bias5,
	F32 *pUniform6, IceQuadWord scale6, IceQuadWord bias6, F32 *pUniform7, IceQuadWord scale7, IceQuadWord bias7,
	F32 *pUniform8, IceQuadWord scale8, IceQuadWord bias8, F32 *pUniform9, IceQuadWord scale9, IceQuadWord bias9,
	F32 *pUniform10, IceQuadWord scale10, IceQuadWord bias10, F32 *pUniform11, IceQuadWord scale11, IceQuadWord bias11,
	F32 *pUniform12, IceQuadWord scale12, IceQuadWord bias12, F32 *pUniform13, IceQuadWord scale13, IceQuadWord bias13,
	F32 *pUniform14, IceQuadWord scale14, IceQuadWord bias14, F32 *pUniform15, IceQuadWord scale15, IceQuadWord bias15);

void Ice::MeshProc::CmdOutputConvertedUniformTable(U32* pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);
	const U8 streamName       = (cmdQuad1.m_data.u8.B & 0x70) >> 4;

	CreateReindexTable(pStaticMem, true);
	
	U16 const *pFixedFormatPtr = (U16 *)pStaticMem[kStaticFixedFormatPtr];
	U16 const *pStreamFormat = (U16 *)((U32)pFixedFormatPtr + pFixedFormatPtr[streamName + 1]);
	U32 attribCount = pStreamFormat[0];
	U32 numVertsOrg = pStaticMem[kStaticReindexCount];
	U32 numVerts = (numVertsOrg + 3) & ~0x3;
	U32 vertexStride = pStreamFormat[1];
	U32 size = vertexStride * numVerts;

#ifndef __SPU__
	// Collect some stats on what is being output.
	g_meshProcessingStats.m_vertexesNum += numVertsOrg;
	g_meshProcessingStats.m_vertexesSize += size;
#endif

	U8 const *pAttribInfo = (U8 *)&pStreamFormat[2];
	U8 const *pAttribIds  = (U8 *)&pStaticMem[kStaticAttributeId];
	U32 fixedFormatPtr = pStaticMem[kStaticFixedFormatPtr];
	U32 variableFormatPtr = pStaticMem[kStaticVariableFormatPtr];
	U16 const *pScaleAndBiasLocs = pStreamFormat + 2 + attribCount * sizeof(U32) / sizeof(U16);
	F32 const * const *pUniformPtrs = (F32 **)&pStaticMem[kStaticUniformPtr];
	U8 * __restrict pDst = MeshOutputAllocate(pStaticMem, size, 16, &pStaticMem[kStaticStreamOutputPtr + streamName]);
	U16 const *pReindex = (U16 *)pStaticMem[kStaticReindexPtr];
	ICE_ASSERT(pReindex != 0);

	// Sapce to save pointer to the uniform tables, scales, and biases for each possible attribute.
	IceQuadWord customSrcAndSab[48];

	// Get the pointer to the custom compressuion code, if it exists.
#ifdef __SPU__
	U32 customCompFuncPtr = pStaticMem[kStaticCustomCompressionFuncPtr + streamName];
#else
	// Custom compression code does not work on the PPU, so don't use it.
	U32 customCompFuncPtr = 0;
#endif

	for (U32 iAttrib = 0; iAttrib < attribCount; ++iAttrib )
	{
		InsertAttributeIntoNvStreamWithScaleAndBias(pAttribInfo + 4 * iAttrib, pAttribIds, fixedFormatPtr,
			variableFormatPtr, pScaleAndBiasLocs, pUniformPtrs, pDst, vertexStride, pReindex, numVerts,
			(customCompFuncPtr == 0) ? NULL : customSrcAndSab + 3 * iAttrib);
	}

	// Call the custom compression routine, if it exists.
	if (customCompFuncPtr != 0) {
		CustomCompressFunc pCustomCompFunc = (CustomCompressFunc)customCompFuncPtr;
		pCustomCompFunc(numVerts, U32(pDst), U32(&g_meshGlobalConstData),
			(F32 *)customSrcAndSab[0].m_data.u32.A, customSrcAndSab[1], customSrcAndSab[2],
			(F32 *)customSrcAndSab[3].m_data.u32.A, customSrcAndSab[4], customSrcAndSab[5],
			(F32 *)customSrcAndSab[6].m_data.u32.A, customSrcAndSab[7], customSrcAndSab[8],
			(F32 *)customSrcAndSab[9].m_data.u32.A, customSrcAndSab[10], customSrcAndSab[11],
			(F32 *)customSrcAndSab[12].m_data.u32.A, customSrcAndSab[13], customSrcAndSab[14],
			(F32 *)customSrcAndSab[15].m_data.u32.A, customSrcAndSab[16], customSrcAndSab[17],
			(F32 *)customSrcAndSab[18].m_data.u32.A, customSrcAndSab[19], customSrcAndSab[20],
			(F32 *)customSrcAndSab[21].m_data.u32.A, customSrcAndSab[22], customSrcAndSab[23],
			(F32 *)customSrcAndSab[24].m_data.u32.A, customSrcAndSab[25], customSrcAndSab[26],
			(F32 *)customSrcAndSab[27].m_data.u32.A, customSrcAndSab[28], customSrcAndSab[29],
			(F32 *)customSrcAndSab[30].m_data.u32.A, customSrcAndSab[31], customSrcAndSab[32],
			(F32 *)customSrcAndSab[33].m_data.u32.A, customSrcAndSab[34], customSrcAndSab[35],
			(F32 *)customSrcAndSab[36].m_data.u32.A, customSrcAndSab[37], customSrcAndSab[38],
			(F32 *)customSrcAndSab[39].m_data.u32.A, customSrcAndSab[40], customSrcAndSab[41],
			(F32 *)customSrcAndSab[42].m_data.u32.A, customSrcAndSab[43], customSrcAndSab[44],
			(F32 *)customSrcAndSab[45].m_data.u32.A, customSrcAndSab[46], customSrcAndSab[47]);
	}
}

