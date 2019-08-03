/*
 * Copyright (c) 2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 *
 * The purpose of this file is to be a container for all decompression
 * functions.
 */

#include <stdlib.h>
#include <math.h>
#include "icemeshinternal.h"

#ifndef __SPU__
#include "icebitextract.h"
#endif

using namespace Ice;
using namespace Ice::MeshProc;

/**
 * Transfers a block of data into a newly allocated chunk of memory in
 * the work buffer.
 *
 * @param xferSrcOffset Offset of source data within input buffer.
 * @param xferSize Number of bytes to allocate and copy.
 * @param pXferDestPtr The address of the newly allocated block in the work
 *                     buffer will be stored here.
 */
static void TransferData(U32 *pStaticMem, U16 const srcOffset, U16 const size, U32 *pDestPtr)
{
	U32 const alignedSize = (size + 0xF) & ~0xF;
	U8 *pInputBuffer = (U8 *)pStaticMem[kStaticInputBufferPtr];
	*pDestPtr = (U32)MeshMemAllocate(pStaticMem, alignedSize);
	CopyBytes((U8 *)(*pDestPtr), pInputBuffer + srcOffset, size);
}

static void TransferQuads(U32 *pStaticMem, U16 const srcOffset, U16 const size, U32 *pDestPtr)
{
	U32 const alignedSize = (size + 0xF) & ~0xF;
	U8 *pInputBuffer = (U8 *)pStaticMem[kStaticInputBufferPtr];
	*pDestPtr = (U32)MeshMemAllocate(pStaticMem, alignedSize);
	CopyQWords((U8*)(*pDestPtr), pInputBuffer + srcOffset, alignedSize);
}

void DecompressIndexes4Byte(U32 *in, U16 *out, U32 numTris)
{
	for(U32 i = 0; i < numTris; i+=8)
	{
		for(U32 ii = i; ii < i + 8; ++ii)
		{
			out[ii*4+0] = (in[ii] & 0x000003FF);
			out[ii*4+1] = (in[ii] & 0x000FFC00) >> 10;
			out[ii*4+2] = (in[ii] & 0x3FF00000) >> 20;
			out[ii*4+3] = (in[ii] & 0x000003FF);
		}
	}
}

#ifndef __SPU__
void DecompressIndexes6Byte(U16 *in, U16 *out, U32 numTris)
{
	for(U32 i = 0; i < numTris; i+=8)
	{
		for(U32 ii = i; ii < i + 8; ++ii)
		{
			out[ii*4+0] = in[ii*3+0];
			out[ii*4+1] = in[ii*3+1];
			out[ii*4+2] = in[ii*3+2];
			out[ii*4+3] = in[ii*3+0];
		}
	}
}
#endif

#ifndef __SPU__
static inline F32 LoadUnalignedF32(U8 const * const pSrc)
{
	IntFloat dstTemp;
	dstTemp.m_u32 = (pSrc[0] << 24) | (pSrc[1] << 16) | (pSrc[2] << 8) | pSrc[3];
	return dstTemp.m_f32;
}

void DecompressF32(F32 const * const pSrc, U32 const srcStride, F32 * __restrict pDst, U32 const /*numComponents*/,
	U32 const numVerts)
{
	U8 *pSrcTemp = (U8 *)pSrc;

	for (U32 i = 0; i < numVerts; ++i)
	{
		pDst[0] = LoadUnalignedF32(pSrcTemp + 0);
		pDst[1] = LoadUnalignedF32(pSrcTemp + 4);
		pDst[2] = LoadUnalignedF32(pSrcTemp + 8);
		pDst[3] = LoadUnalignedF32(pSrcTemp + 12);

		pSrcTemp += srcStride;
		pDst += 4;
	}
}
#endif

#ifndef __SPU__
static F32 F16toF32(U16 i)
{
	// Extract the sign
	U32 s = ((U32)i) << 16 & 0x80000000;
	// Extract the exponent
	U32 e = (i >> 10) & 0x0000001f;
	// Extract the mantissa
	U32 m =  i        & 0x000003ff;

	IntFloat retval;

	if (e == 0)
	{
		if (m == 0)
		{
			retval.m_u32 = s;
			return retval.m_f32;
		}
		else
		{
			// Denormalized number -- renormalize it
			while (!(m & 0x00000400))
			{
				m <<= 1;
				e -=  1;
			}

			e += 1;
			m &= ~0x00000400;
		}
	}
	else if (e == 31)
	{
		if (m == 0)
		{
			// Positive or negative infinity
			retval.m_u32 = (s | 0x7f800000);
			return retval.m_f32;
		}
		else
		{
			// NAN -- preserve sign and mantissa bits
			retval.m_u32 = s | 0x7f800000 | (m << 13);
			return retval.m_f32;
		}
	}

	// Normalized number
	e = e + (127 - 15);
	m = m << 13;

	// Assemble s, e and m.
	retval.m_u32 = s | (e << 23) | m;
   	return retval.m_f32;
}

static inline U16 LoadUnalignedF16(U8 const * const pSrc)
{
	U16 dstTemp;
	dstTemp = (pSrc[0] << 8) | pSrc[1];
	return dstTemp;
}

void DecompressF16(U16 const * const pSrc, U32 const srcStride, F32 * __restrict pDst, U32 const /*numComponents*/,
	U32 const numVerts, const IceQuadWord scale, const IceQuadWord bias)
{
	U8 const *pSrcTemp = (U8 *)pSrc;

	for (U32 i = 0; i < numVerts; ++i)
	{
		pDst[0] = F16toF32(LoadUnalignedF16(pSrcTemp + 0)) * scale.m_data.f32.A + bias.m_data.f32.A;
		pDst[1] = F16toF32(LoadUnalignedF16(pSrcTemp + 2)) * scale.m_data.f32.B + bias.m_data.f32.B;
		pDst[2] = F16toF32(LoadUnalignedF16(pSrcTemp + 4)) * scale.m_data.f32.C + bias.m_data.f32.C;
		pDst[3] = F16toF32(LoadUnalignedF16(pSrcTemp + 6)) * scale.m_data.f32.D + bias.m_data.f32.D;

		pSrcTemp += srcStride;
		pDst += 4;
	}
}
#endif

#ifndef __SPU__
static inline I16 LoadUnalignedI16(U8 const * const pSrc)
{
	I16 dstTemp;
	dstTemp = (pSrc[0] << 8) | pSrc[1];
	return dstTemp;
}

void DecompressI16(I16 const * const pSrc, U32 const srcStride, F32 * __restrict pDst, U32 const /*numComponents*/,
	U32 const numVerts, const IceQuadWord scale, const IceQuadWord bias)
{
	U8 const *pSrcTemp = (U8 *)pSrc;

	for (U32 i = 0; i < numVerts; ++i)
	{
		pDst[0] = static_cast<F32>(LoadUnalignedI16(pSrcTemp + 0)) * scale.m_data.f32.A + bias.m_data.f32.A;
		pDst[1] = static_cast<F32>(LoadUnalignedI16(pSrcTemp + 2)) * scale.m_data.f32.B + bias.m_data.f32.B;
		pDst[2] = static_cast<F32>(LoadUnalignedI16(pSrcTemp + 4)) * scale.m_data.f32.C + bias.m_data.f32.C;
		pDst[3] = static_cast<F32>(LoadUnalignedI16(pSrcTemp + 6)) * scale.m_data.f32.D + bias.m_data.f32.D;

		pSrcTemp += srcStride;
		pDst += 4;
	}
}
#endif

#ifndef __SPU__
void DecompressU8(U8 const * const pSrc, U32 const srcStride, F32 * __restrict pDst, U32 const /*numComponents*/,
	U32 const numVerts, const IceQuadWord scale, const IceQuadWord bias)
{
	U8 *pSrcTemp = (U8 *)pSrc;

	for (U32 i = 0; i < numVerts; ++i)
	{
		pDst[0] = static_cast<F32>(pSrcTemp[0]) * scale.m_data.f32.A + bias.m_data.f32.A;
		pDst[1] = static_cast<F32>(pSrcTemp[1]) * scale.m_data.f32.B + bias.m_data.f32.B;
		pDst[2] = static_cast<F32>(pSrcTemp[2]) * scale.m_data.f32.C + bias.m_data.f32.C;
		pDst[3] = static_cast<F32>(pSrcTemp[3]) * scale.m_data.f32.D + bias.m_data.f32.D;

		pSrcTemp += srcStride;
		pDst += 4;
	}
}
#endif

#ifndef __SPU__
static inline U32 LoadUnalignedU32(U8 const * const pSrc)
{
	U32 dstTemp;
	dstTemp = (pSrc[0] << 24) | (pSrc[1] << 16) | (pSrc[2] << 8) | pSrc[3];
	return dstTemp;
}

void DecompressX11Y11Z10(U32 const * const pSrc, U32 const srcStride, F32 * __restrict pDst, U32 const /*numComponents*/,
	U32 const numVerts, const IceQuadWord scale, const IceQuadWord bias)
{
	U8 *pSrcTemp = (U8 *)pSrc;

	for (U32 i = 0; i < numVerts; ++i)
	{
		U32 pSrcTempVal = LoadUnalignedU32(pSrcTemp);
		FROM_X11Y11Z10N(pSrcTempVal, pDst[0], pDst[1], pDst[2])
		pDst[0] = pDst[0] * scale.m_data.f32.A + bias.m_data.f32.A;
		pDst[1] = pDst[1] * scale.m_data.f32.B + bias.m_data.f32.B;
		pDst[2] = pDst[2] * scale.m_data.f32.C + bias.m_data.f32.C;
		pDst[3] = 1.0f;

		pSrcTemp += srcStride;
		pDst += 4;
	}
}
#endif

#ifndef __SPU__

#define M00 (0*4+0)
#define M01 (0*4+1)
#define M02 (0*4+2)
#define M03 (0*4+3)
#define M10 (1*4+0)
#define M11 (1*4+1)
#define M12 (1*4+2)
#define M13 (1*4+3)
#define M20 (2*4+0)
#define M21 (2*4+1)
#define M22 (2*4+2)
#define M23 (2*4+3)
#define M30 (3*4+0)
#define M31 (3*4+1)
#define M32 (3*4+2)
#define M33 (3*4+3)

#define IN_MATRIX_SIZE 4*4
#define OUT_MATRIX_SIZE 3*4

void DecompressMatrices(F32 *out, F32 *in, U32 numMats)
{
	numMats = (numMats + 1) & ~0x1;
	for(U32 i = 0; i < numMats; ++i)
	{
		F32 mat[16];
		mat[M00] = in[M00];
		mat[M01] = in[M01];
		mat[M02] = in[M02];
		mat[M03] = in[M03];

		mat[M10] = in[M10];
		mat[M11] = in[M11];
		mat[M12] = in[M12];
		mat[M13] = in[M13];

		mat[M20] = in[M20];
		mat[M21] = in[M21];
		mat[M22] = in[M22];
		mat[M23] = in[M23];

		mat[M30] = in[M30];
		mat[M31] = in[M31];
		mat[M32] = in[M32];
		mat[M33] = in[M33];

		out[M00] = mat[M00];
		out[M01] = mat[M10];
		out[M02] = mat[M20];
		out[M03] = mat[M30];

		out[M10] = mat[M01];
		out[M11] = mat[M11];
		out[M12] = mat[M21];
		out[M13] = mat[M31];

		out[M20] = mat[M02];
		out[M21] = mat[M12];
		out[M22] = mat[M22];
		out[M23] = mat[M32];

		out += OUT_MATRIX_SIZE;
		in += IN_MATRIX_SIZE;
	}
}
#endif

#ifndef __SPU__
//! Decompresses a Software Format (variable bit width)
void DecompressSwFormat(
	U8  *in,
	F32 *out,
	U32 numVerts,
	U32 bitsX,
	U32 bitsY,
	U32 bitsZ,
	U32 bitsW,
	IceQuadWord intoff,
	IceQuadWord scale,
	IceQuadWord bias)
{
	U32 intoffX = intoff.m_data.u32.A;
	U32 intoffY = intoff.m_data.u32.B;
	U32 intoffZ = intoff.m_data.u32.C;
	U32 intoffW = intoff.m_data.u32.D;
	F32 scaleX = scale.m_data.f32.A;
	F32 scaleY = scale.m_data.f32.B;
	F32 scaleZ = scale.m_data.f32.C;
	F32 scaleW = scale.m_data.f32.D;
	F32 biasX = bias.m_data.f32.A;
	F32 biasY = bias.m_data.f32.B;
	F32 biasZ = bias.m_data.f32.C;
	F32 biasW = bias.m_data.f32.D;

	U32 stride = bitsX + bitsY + bitsZ + bitsW;
	U32 xoff = 0;
	U32 yoff = bitsX;
	U32 zoff = bitsX + bitsY;
	U32 woff = bitsX + bitsY + bitsZ;
	U32 xmask = (~(0xFFFFFFFF << bitsX)) << 2;
	U32 ymask = (~(0xFFFFFFFF << bitsY)) << 2;
	U32 zmask = (~(0xFFFFFFFF << bitsZ)) << 2;
	U32 wmask = (~(0xFFFFFFFF << bitsW)) << 2;
	for(U32 i = 0; i < numVerts; i += 8)
	{
		for(U32 ii = 0; ii < 8; ++ii)
		{
			U32 xo = xoff >> 3;
			U32 yo = yoff >> 3;
			U32 zo = zoff >> 3;
			U32 wo = woff >> 3;
			U32 xn = xoff & 7;
			U32 yn = yoff & 7;
			U32 zn = zoff & 7;
			U32 wn = woff & 7;

			IntFloat xval, yval, zval, wval;
			U8 *pXval = (U8 *)&xval;
			U8 *pYval = (U8 *)&yval;
			U8 *pZval = (U8 *)&zval;
			U8 *pWval = (U8 *)&wval;
			pXval[0] = *(in + xo + 0);
			pXval[1] = *(in + xo + 1);
			pXval[2] = *(in + xo + 2);
			pXval[3] = *(in + xo + 3);
			pYval[0] = *(in + yo + 0);
			pYval[1] = *(in + yo + 1);
			pYval[2] = *(in + yo + 2);
			pYval[3] = *(in + yo + 3);
			pZval[0] = *(in + zo + 0);
			pZval[1] = *(in + zo + 1);
			pZval[2] = *(in + zo + 2);
			pZval[3] = *(in + zo + 3);
			pWval[0] = *(in + wo + 0);
			pWval[1] = *(in + wo + 1);
			pWval[2] = *(in + wo + 2);
			pWval[3] = *(in + wo + 3);

			xval.m_u32 >>= (30 - xn - bitsX);
			yval.m_u32 >>= (30 - yn - bitsY);
			zval.m_u32 >>= (30 - zn - bitsZ);
			wval.m_u32 >>= (30 - wn - bitsW);

			xval.m_u32 &= xmask;
			yval.m_u32 &= ymask;
			zval.m_u32 &= zmask;
			wval.m_u32 &= wmask;

			xval.m_u32 += intoffX;
			yval.m_u32 += intoffY;
			zval.m_u32 += intoffZ;
			wval.m_u32 += intoffW;

			xval.m_u32 |= 0x3f800000;
			yval.m_u32 |= 0x3f800000;
			zval.m_u32 |= 0x3f800000;
			wval.m_u32 |= 0x3f800000;

			out[0] = xval.m_f32 * scaleX + biasX;
			out[1] = yval.m_f32 * scaleY + biasY;
			out[2] = zval.m_f32 * scaleZ + biasZ;
			out[3] = wval.m_f32 * scaleW + biasW;
			out += 4;

			xoff += stride;
			yoff += stride;
			zoff += stride;
			woff += stride;
		}
	}
}
#endif

void DecompressSwFormatHL(U32 *pStaticMem, U8 *pFormat, U8 *pStream)
{
	U16 const numVerts = (pStaticMem[kStaticVertexCount] + 3) & ~3;
	U32 const uniformSize = 0x10 * (numVerts + 4);

	U8 const id = pFormat[0];
	U8 const scaleAndBiasFlags = pFormat[1];
	U8 const componentCount = pFormat[1] & 0x0F;

	U32 const uniformIdx = pStaticMem[kStaticUniformCount]++;
	ICE_ASSERT(uniformIdx < 16);

	F32 *out = (F32*)MeshMemAllocate(pStaticMem, uniformSize );
	switch(id)
	{
		case kPositionId:
			pStaticMem[kStaticUniformPosPtr] = (U32)out;
			break;
		case kNormalId:
			pStaticMem[kStaticUniformNormPtr] = (U32)out;
			break;
		case kTangentId:
			pStaticMem[kStaticUniformTanPtr] = (U32)out;
			break;
		case kDispNormalId:
			pStaticMem[kStaticUniformDnormPtr] = (U32)out;
			break;
	}

	U8 *attributeIds = (U8*)&pStaticMem[kStaticAttributeId];

	pStaticMem[kStaticUniformPtr + uniformIdx] = (U32)out;
	attributeIds[uniformIdx] = id;

	U32 scaleAndBiasBase = pStaticMem[kStaticFixedFormatPtr];
	if (scaleAndBiasFlags & kScaleAndBiasVariable)
		scaleAndBiasBase = pStaticMem[kStaticVariableFormatPtr];
	F32 *pScales = (F32 *)(scaleAndBiasBase + *((U16 *)(&pFormat[2])));
	F32 *pBiases = pScales + componentCount;
	U32 *pIntoffs = (U32 *)pBiases + componentCount;

	bool const hasIntOff = scaleAndBiasFlags & kHasIntoff;

	IceQuadWord intoff;
	IceQuadWord scale;
	IceQuadWord bias;
	intoff.m_data.u32.A = hasIntOff ? pIntoffs[0] : 0;
	intoff.m_data.u32.B = hasIntOff ? pIntoffs[1] : 0;
	intoff.m_data.u32.C = hasIntOff ? pIntoffs[2] : 0;
	intoff.m_data.u32.D = hasIntOff ? pIntoffs[3] : 0;
	scale.m_data.f32.A = pScales[0];
	scale.m_data.f32.B = pScales[1];
	scale.m_data.f32.C = pScales[2];
	scale.m_data.f32.D = pScales[3];
	bias.m_data.f32.A = pBiases[0];
	bias.m_data.f32.B = pBiases[1];
	bias.m_data.f32.C = pBiases[2];
	bias.m_data.f32.D = pBiases[3];

	DecompressSwFormat(pStream, out, numVerts, pFormat[4], pFormat[5], pFormat[6], pFormat[7], intoff, scale, bias);
}

void Ice::MeshProc::CmdSetupNvControlInfo(U32 *pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);
	TransferData(pStaticMem, cmdQuad1.m_data.u16.B, kSizeOfNvControl,
		&pStaticMem[kStaticNvControlPtr]);

	// If using a ring buffer, start the transfer of the ring buffer allocator for this SPU.
	U32 semaphorePbHolePtr = *(U32 *)(pStaticMem[kStaticNvControlPtr] + 0x28);
	if (semaphorePbHolePtr != 0) {
#ifdef __SPU__
		U32 *pRingBufferAllocator = &pStaticMem[kStaticRingBufferStartPtr];
		U32 ringBufferAllocatorPtr = *(U32 *)(pStaticMem[kStaticNvControlPtr] + 0x10) + WwsJob_JobApiGetSpuNum() * 0x10;
		ICE_ASSERT((ringBufferAllocatorPtr & 0xF) == 0);

		StartDma(pRingBufferAllocator, ringBufferAllocatorPtr, 16, pStaticMem[kStaticWorkBufferTagId], kDmaGet);
#else
		VU8 *pRingBufferAllocatorLs = (VU8 *)&pStaticMem[kStaticRingBufferStartPtr];
		VU8 *pRingBufferAllocatorEa = (VU8 *)(*(U32 *)(pStaticMem[kStaticNvControlPtr] + 0x10) + g_spuNum * 0x10);
		ICE_ASSERT(((U32)pRingBufferAllocatorEa & 0xF) == 0);

		*pRingBufferAllocatorLs = *pRingBufferAllocatorEa;
#endif
	}
}

void Ice::MeshProc::CmdSetupObjectInfo(U32 *pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);
	TransferData(pStaticMem, cmdQuad1.m_data.u16.B, kSizeOfObjectInfo,
		&pStaticMem[kStaticObjInfoPtr]);
}

void Ice::MeshProc::CmdSetupFormatInfo(U32 *pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);
	U8 *pInputBuffer = (U8 *)pStaticMem[kStaticInputBufferPtr];

	if (cmdQuad1.m_data.u16.B != 0) {
		TransferQuads(pStaticMem, cmdQuad1.m_data.u16.B, *(U16*)(pInputBuffer + cmdQuad1.m_data.u16.B),
				&pStaticMem[kStaticFixedFormatPtr]);
	}

	if (cmdQuad1.m_data.u16.C != 0) {
		TransferQuads(pStaticMem, cmdQuad1.m_data.u16.C, *(U16*)(pInputBuffer + cmdQuad1.m_data.u16.C),
				&pStaticMem[kStaticVariableFormatPtr]);
	}
}

void Ice::MeshProc::CmdSetupDeltaFormatInfo(U32 *pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);
	U8 *pInputBuffer = (U8 *)pStaticMem[kStaticInputBufferPtr];

	TransferQuads(pStaticMem, cmdQuad1.m_data.u16.B, *(U16*)(pInputBuffer + cmdQuad1.m_data.u16.B),
			&pStaticMem[kStaticDeltaFormatPtr]);
}

void Ice::MeshProc::CmdSetupVertexInfo(U32 *pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);
	pStaticMem[kStaticVertexCount]       = cmdQuad1.m_data.u16.B;
	pStaticMem[kStaticHaloVertexCount]   = cmdQuad1.m_data.u16.C;
	pStaticMem[kStaticOutputVertexCount] = pStaticMem[kStaticVertexCount] - pStaticMem[kStaticHaloVertexCount];
}

void Ice::MeshProc::CmdSetupPmVertexInfo(U32 *pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);
	pStaticMem[kStaticPmHighestLod] = cmdQuad1.m_data.u16.C;
	U16 *pLodInfo    = (U16 *)&pStaticMem[kStaticPmVertexLodInfo];
	U8 *pInputBuffer = (U8 *)pStaticMem[kStaticInputBufferPtr];
	CopyBytes(pLodInfo, pInputBuffer + cmdQuad1.m_data.u16.B, kSizeOfLodInfo);
	U16 lodCount = pLodInfo[3];
	ICE_ASSERT(lodCount <= kMaxLodCount);
	pStaticMem[kStaticVertexCount]     = 0;
	pStaticMem[kStaticHaloVertexCount] = 0;
	for(U8 i = pStaticMem[kStaticPmHighestLod]; i < lodCount; ++i)
	{
		pStaticMem[kStaticVertexCount]     += pLodInfo[4*i+1];
		pStaticMem[kStaticHaloVertexCount] += pLodInfo[4*i+2];
	}

	// With multiple LODs, the halo vertexes are interleaved with the
	// base vertexes, so we can only ignore the very last set of halo
	// verts when outputing vertex data.
	pStaticMem[kStaticOutputVertexCount] = pStaticMem[kStaticVertexCount] - pLodInfo[4*pStaticMem[kStaticPmHighestLod]+2];
}

#ifndef __SPU__
typedef void (*CustomDecompressFunc)(U32 numVerts, U32 pSrc, U32 constDataPtr,
	F32 *pUniform0, IceQuadWord scale0, IceQuadWord bias0, F32 *pUniform1, IceQuadWord scale1, IceQuadWord bias1,
	F32 *pUniform2, IceQuadWord scale2, IceQuadWord bias2, F32 *pUniform3, IceQuadWord scale3, IceQuadWord bias3,
	F32 *pUniform4, IceQuadWord scale4, IceQuadWord bias4, F32 *pUniform5, IceQuadWord scale5, IceQuadWord bias5,
	F32 *pUniform6, IceQuadWord scale6, IceQuadWord bias6, F32 *pUniform7, IceQuadWord scale7, IceQuadWord bias7,
	F32 *pUniform8, IceQuadWord scale8, IceQuadWord bias8, F32 *pUniform9, IceQuadWord scale9, IceQuadWord bias9,
	F32 *pUniform10, IceQuadWord scale10, IceQuadWord bias10, F32 *pUniform11, IceQuadWord scale11, IceQuadWord bias11,
	F32 *pUniform12, IceQuadWord scale12, IceQuadWord bias12, F32 *pUniform13, IceQuadWord scale13, IceQuadWord bias13,
	F32 *pUniform14, IceQuadWord scale14, IceQuadWord bias14, F32 *pUniform15, IceQuadWord scale15, IceQuadWord bias15);

void DecompressNvFormat(U32 *pStaticMem, U16 *pFormat, U8 *pStream, U16 flags, U32 customDecompFuncPtr)
{
	static const F32 kScales[8] = {1.0f, 1.0f, 2.0f / 65535.0f, 1.0f, 1.0f, 1.0f / 255.0f, 1.0f, 1.0f};
	static const F32 kBiases[8] = {0.0f, 0.0f, 1.0f / 65535.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

	U16 numVerts = pStaticMem[kStaticVertexCount];
	U32 attribCount = pFormat[0];
	U32 stride = pFormat[1];
	ICE_ASSERT(stride <= 256); // a larger stride usually indicates a misaligned pointer somewhere
	U8 *attrib = (U8*)&pFormat[2];
	numVerts = (numVerts + 3) & ~3;
	U32 uniformSize = 0x10 * (numVerts + 4);
	U16 *pScaleAndBiasLocs = pFormat + 2 + attribCount * sizeof(U32) / sizeof(U16);
	U32 fixedFormatPtr = pStaticMem[kStaticFixedFormatPtr];
	U32 variableFormatPtr = pStaticMem[kStaticVariableFormatPtr];
	F32 *pFirstUniform = NULL;

	U8 *attributeIds = (U8*)&pStaticMem[kStaticAttributeId];

	// The scale and bias for each attribute needs to be saved in case a custom decompression routine is called.
	IceQuadWord customSab[32];
	U32 numCustomSab = 0;

	while (flags != 0) {
		// Get the attribute number.
#ifdef __SPU__
		U32 attribNum = 31 - si_clz(flags);
#else
		U32 attribNum = 31 - Cntlzw(flags);
#endif

		// Clear this attribute from the flags.
		flags = flags & ~(1 << attribNum);
		U8 id = attrib[attribNum * 4 + 0];
		ICE_ASSERT(id > 0);
		U8 componentCount = attrib[attribNum * 4 + 1] & 0xF;
		U8 type = (attrib[attribNum * 4 + 1] >> 4) & 0xF;
		U8 scaleAndBiasFlags = attrib[attribNum * 4 + 2];
		U8 offset = attrib[attribNum * 4 + 3];

		F32 *pUniform = (F32*)MeshMemAllocate(pStaticMem, uniformSize );
		if (pFirstUniform == NULL)
			pFirstUniform = pUniform;
		if (id == kPositionId)
			pStaticMem[kStaticUniformPosPtr] = (U32)pUniform;
		else if (id == kNormalId)
			pStaticMem[kStaticUniformNormPtr] = (U32)pUniform;
		else if (id == kTangentId)
			pStaticMem[kStaticUniformTanPtr] = (U32)pUniform;
		else if (id == kDispNormalId)
			pStaticMem[kStaticUniformDnormPtr] = (U32)pUniform;

		U32 uniformIndex = pStaticMem[kStaticUniformCount]++;
		ICE_ASSERT(uniformIndex < 16);
		pStaticMem[kStaticUniformPtr + uniformIndex] = (U32)pUniform;
		attributeIds[uniformIndex] = id;

		// Get the scales and biases, if they exist.
		IceQuadWord scale;
		IceQuadWord bias;
		if (scaleAndBiasFlags & kScaleAndBiasOn) {
			U32 scaleAndBiasBase = fixedFormatPtr;
			if (scaleAndBiasFlags & kScaleAndBiasVariable)
				scaleAndBiasBase = variableFormatPtr;
			F32 *pScales = (F32 *)(scaleAndBiasBase + pScaleAndBiasLocs[scaleAndBiasFlags & 0xF]);
			F32 *pBiases = pScales + componentCount;
			scale.m_data.f32.A = pScales[0];
			scale.m_data.f32.B = pScales[1];
			scale.m_data.f32.C = pScales[2];
			scale.m_data.f32.D = pScales[3];
			bias.m_data.f32.A = pBiases[0];
			bias.m_data.f32.B = pBiases[1];
			bias.m_data.f32.C = pBiases[2];
			bias.m_data.f32.D = pBiases[3];
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

		// Adjust the scale and bias depening upon the attribute type;
		F32 attribScale = kScales[type];
		F32 attribBias = kBiases[type];
		bias.m_data.f32.A += scale.m_data.f32.A * attribBias;
		bias.m_data.f32.B += scale.m_data.f32.B * attribBias;
		bias.m_data.f32.C += scale.m_data.f32.C * attribBias;
		bias.m_data.f32.D += scale.m_data.f32.D * attribBias;
		scale.m_data.f32.A *= attribScale;
		scale.m_data.f32.B *= attribScale;
		scale.m_data.f32.C *= attribScale;
		scale.m_data.f32.D *= attribScale;

		// Save the scale and bias for the custom decompression routine.
		customSab[numCustomSab] = scale;
		customSab[numCustomSab + 1] = bias;
		numCustomSab += 2;

		// Call the correct decompression routine if a custom decompress routine does not exist.
		if (customDecompFuncPtr == 0)
		{
			switch(type)
			{
				case kF32:
					DecompressF32((F32*)(pStream + offset), stride, pUniform, componentCount, numVerts);
					break;
				case kF16:
					DecompressF16((U16*)(pStream + offset), stride, pUniform, componentCount, numVerts, scale, bias);
					break;
				case kI16n:
					DecompressI16((I16*)(pStream + offset), stride, pUniform, componentCount, numVerts, scale, bias);
					break;
				case kI16:
					DecompressI16((I16*)(pStream + offset), stride, pUniform, componentCount, numVerts, scale, bias);
					break;
				case kX11Y11Z10n:
					DecompressX11Y11Z10((U32*)(pStream + offset), stride, pUniform, componentCount, numVerts, scale, bias);
					break;
				case kU8n:
					DecompressU8((U8*)(pStream + offset), stride, pUniform, componentCount, numVerts, scale, bias);
					break;
				case kU8:
					DecompressU8((U8*)(pStream + offset), stride, pUniform, componentCount, numVerts, scale, bias);
					break;
				default:
					ICE_ASSERT(0); // unsupported vertex attribute type
			}
		}
	}

	// Call the custom decompress routine if it exists.
	if (customDecompFuncPtr != 0)
	{
		U32 uniSize = uniformSize / sizeof(F32);
		CustomDecompressFunc pCustomDecompFunc = (CustomDecompressFunc)customDecompFuncPtr;
		pCustomDecompFunc(numVerts, U32(pStream), U32(&g_meshGlobalConstData),
			pFirstUniform, customSab[0], customSab[1], pFirstUniform + uniSize, customSab[2], customSab[3],
			pFirstUniform + uniSize * 2, customSab[4], customSab[5], pFirstUniform + uniSize * 3, customSab[6], customSab[7],
			pFirstUniform + uniSize * 4, customSab[8], customSab[9], pFirstUniform + uniSize * 5, customSab[10], customSab[11],
			pFirstUniform + uniSize * 6, customSab[12], customSab[13], pFirstUniform + uniSize * 7, customSab[14], customSab[15],
			pFirstUniform + uniSize * 8, customSab[16], customSab[17], pFirstUniform + uniSize * 9, customSab[18], customSab[19],
			pFirstUniform + uniSize * 10, customSab[20], customSab[21], pFirstUniform + uniSize * 11, customSab[22], customSab[23],
			pFirstUniform + uniSize * 12, customSab[24], customSab[25], pFirstUniform + uniSize * 13, customSab[26], customSab[27],
			pFirstUniform + uniSize * 14, customSab[28], customSab[29], pFirstUniform + uniSize * 15, customSab[30], customSab[31]);
	}
}

void CmdSetupNvStream(U32 *pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);
	U8 const streamName      = (cmdQuad1.m_data.u8.B & 0x70) >> 4;
	U8 *pInputBuffer         = (U8 *)pStaticMem[kStaticInputBufferPtr];
	U8 *pStream = pInputBuffer + cmdQuad1.m_data.u16.B;
	U32 customDecompFuncPtr = NULL;
#ifdef __SPU___
	if (cmdQuad1.m_data.u16.D != 0)
		customDecompFuncPtr = U32(pInputBuffer) + cmdQuad1.m_data.u16.D;
#endif
	U16 const fixedOff = *(U16 *)(pStaticMem[kStaticFixedFormatPtr] + streamName * 2 + 2);
	U16 *pFormat = (U16 *)(pStaticMem[kStaticFixedFormatPtr] + fixedOff);
	DecompressNvFormat(pStaticMem, pFormat, pStream, cmdQuad1.m_data.u16.C, customDecompFuncPtr);

	// Copy the stream to the work buffer if the copy bit is set
	if (cmdQuad1.m_data.u8.B & 0x01)
	{
		U16 const numVerts = (pStaticMem[kStaticVertexCount]+3) & ~3;
		U32 const vertexStride = pFormat[1];
		U32 const streamSize = (numVerts*vertexStride + 0xF) & ~0xF;
		pStaticMem[kStaticStreamWorkPtr + streamName] = (U32)MeshMemAllocate(pStaticMem, streamSize);
		CopyQWords((void *)pStaticMem[kStaticStreamWorkPtr + streamName], pStream, streamSize);
	}
}
#endif

void Ice::MeshProc::CmdSetupSwStream(U32 *pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);
	U8 const streamName      = (cmdQuad1.m_data.u8.B & 0xF0) >> 4;
	U8 *pInputBuffer         = (U8 *)pStaticMem[kStaticInputBufferPtr];
	U8 *pStream = pInputBuffer + cmdQuad1.m_data.u16.B;
	U8 *pFormat = (U8 *)(pStaticMem[kStaticVariableFormatPtr] + (streamName + 1) * 8);
	DecompressSwFormatHL(pStaticMem, pFormat, pStream);
}

void Ice::MeshProc::CmdSetupCustomCompress(U32 *pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);
	U8 const streamName = (cmdQuad1.m_data.u8.B & 0x70) >> 4;
	TransferQuads(pStaticMem, cmdQuad1.m_data.u16.B, cmdQuad1.m_data.u16.C,
		&pStaticMem[kStaticCustomCompressionFuncPtr + streamName]);
}

void Ice::MeshProc::CmdSetupIndexes(U32 *pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);
	// Allocate index table rounded up top nearest multiple of 8 tris
	pStaticMem[kStaticIndexPtr] = (U32)MeshMemAllocate(pStaticMem,
			((cmdQuad1.m_data.u16.C + 0x7) & ~0x7) * 4*sizeof(U16)); // 8 bytes per triangle
	U8 *pInputBuffer = (U8 *)pStaticMem[kStaticInputBufferPtr];
	const U8 miscFlags        = (cmdQuad1.m_data.u8.B & 0x0F);
	switch(miscFlags)
	{
	case kFormatIndex2U16:
		DecompressIndexes4Byte((U32 *)(pInputBuffer + cmdQuad1.m_data.u16.B),
			(U16 *)pStaticMem[kStaticIndexPtr], cmdQuad1.m_data.u16.C); // note order of parameters is in, out, count!
		break;
	case kFormatIndex3U16:
		DecompressIndexes6Byte((U16 *)(pInputBuffer + cmdQuad1.m_data.u16.B),
			(U16 *)pStaticMem[kStaticIndexPtr], cmdQuad1.m_data.u16.C); // note order of parameters is in, out, count!
		break;
	default:
		ICE_ASSERT(0); // unsupported index format
	}
	pStaticMem[kStaticIndexCount] = cmdQuad1.m_data.u16.C * 3;
	pStaticMem[kStaticHaloIndexCount] = cmdQuad1.m_data.u16.D * 3;
}

void Ice::MeshProc::CmdSetupPixelShader(U32 *pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);
	TransferData(pStaticMem,  cmdQuad1.m_data.u16.B, cmdQuad1.m_data.u16.C * 16, // 16 bytes per instruction
		&pStaticMem[kStaticPixelShaderPtr]);
	pStaticMem[kStaticPixelShaderSize] = cmdQuad1.m_data.u16.C * 16;
	TransferData(pStaticMem,  cmdQuad1.m_data.u16.D, cmdQuad1.m_data.u16.E,
		&pStaticMem[kStaticPixelShaderPatchPtr]);
	TransferData(pStaticMem,  cmdQuad1.m_data.u16.F, cmdQuad1.m_data.u16.G * 16, // 16 bytes per constant
		&pStaticMem[kStaticPixelShaderConstPtr]);
	// the pixel shader is patched in CmdCleanupAndExit
}

void Ice::MeshProc::CmdSetupViewportInfo(U32 *pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);
	TransferData(pStaticMem, cmdQuad1.m_data.u16.B, kSizeOfViewportInfo,
		&pStaticMem[kStaticViewportInfoPtr]);
}

void Ice::MeshProc::CmdSetupRootTransformInfo(U32 *pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);
	TransferData(pStaticMem, cmdQuad1.m_data.u16.B, kSizeOfRigidXformInfo,
		&pStaticMem[kStaticRigidObjectXformPtr]);
}

#if ICE_MESH_STENCIL_SHADOWS_ON
void Ice::MeshProc::CmdSetupShadowInfo(U32 *pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);
	TransferData(pStaticMem, cmdQuad1.m_data.u16.B, kSizeOfShadowInfo,
		&pStaticMem[kStaticShadowInfoPtr]);
}

void Ice::MeshProc::CmdSetupEdges(U32 *pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);
	U32 edgesSize = ((cmdQuad1.m_data.u16.C + 0x3) & ~0x3) * sizeof(U32);
	pStaticMem[kStaticEdgePtr] = (U32)MeshMemAllocate(pStaticMem, edgesSize);
	const U8 miscFlags        = (cmdQuad1.m_data.u8.B & 0x0F);
	U8 *pInputBuffer = (U8 *)pStaticMem[kStaticInputBufferPtr];
	switch(miscFlags)
	{
	case kFormatEdgeU32:
		CopyQWords((void *)pStaticMem[kStaticEdgePtr], pInputBuffer + cmdQuad1.m_data.u16.B, edgesSize); // out, in, count
		break;
	default:
		ICE_ASSERT(0); // unsupported edge format
	}
	pStaticMem[kStaticEdgeCount] = cmdQuad1.m_data.u16.C;
}
#endif // ICE_MESH_STENCIL_SHADOWS_ON

void Ice::MeshProc::CmdSetupContinuousPmInfo(U32 *pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);
	TransferData(pStaticMem, cmdQuad1.m_data.u16.B, kSizeOfContinuousPmInfo,
		&pStaticMem[kStaticContinuousPmInfoPtr]);
}

void Ice::MeshProc::CmdSetupDiscretePmInfo(U32 *pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);
	TransferData(pStaticMem, cmdQuad1.m_data.u16.B, kSizeOfDiscretePmInfo,
		&pStaticMem[kStaticDiscretePmInfoPtr]);
}

void Ice::MeshProc::CmdSetupPmParent(U32 *pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);
	U32 parentIdx = (cmdQuad1.m_data.u8.B & 0xF0) >> 4;

	// If there are no parents, then don't bother to setup the parent table.
	U32 parentSize = ((cmdQuad1.m_data.u16.C + 0x07) & ~0x07) * sizeof(U16);
	if (parentSize == 0)
		return;
	
	// If using continuous PM, then make sure the parent table is actually in the range that will undergo PM.
	// If not, then don't bother to setup the parent table.
	U32 *pContinuousPmInfo = (U32 *)pStaticMem[kStaticContinuousPmInfoPtr];
	if (pContinuousPmInfo != NULL) {
		if (parentIdx > pContinuousPmInfo[0])
			return;
	}

	// If using discrete PM, then check to see if blending will even be performed.
	// If not, then don't bother to setup the parent table.
//      This check can't be performed as it interferes with DM which always sets up a parent table, but doesn't have
//      a valid discretePmInfo structure, so we'll setup parent tables for PM all the time until real DM.
//	F32 *pDiscretePmInfo = (F32 *)pStaticMem[kStaticDiscretePmInfoPtr];
//	if (pDiscretePmInfo != NULL) {
//		if (pDiscretePmInfo[0] >= 1.0f)
//			return;
//	}

	pStaticMem[kStaticPmParentPtr + parentIdx] = (U32)MeshMemAllocate(pStaticMem, parentSize);
	const U8 miscFlags = (cmdQuad1.m_data.u8.B & 0x0F);
	U8 *pInputBuffer = (U8 *)pStaticMem[kStaticInputBufferPtr];
	switch(miscFlags)
	{
	case kFormatPmParentU16:
		CopyQWords((void *)pStaticMem[kStaticPmParentPtr + parentIdx],
			pInputBuffer + cmdQuad1.m_data.u16.B, parentSize); // out, in, count
		break;
	default:
		ICE_ASSERT(0); // unsupported parent format
	}
}

#ifndef __SPU__
void CmdSetupSkinning(U32 *pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);
	TransferData(pStaticMem, cmdQuad1.m_data.u16.B, cmdQuad1.m_data.u16.C + 1,		// 1 byte/control + 1 byte
		&pStaticMem[kStaticSkinControlPtr]);
	TransferData(pStaticMem, cmdQuad1.m_data.u16.D, (cmdQuad1.m_data.u16.E + 0xF) & ~0xF,	// 2 bytes/same
		&pStaticMem[kStaticSkinSamePtr]);
	TransferData(pStaticMem, cmdQuad1.m_data.u16.F, (cmdQuad1.m_data.u16.G + 0xF) & ~0xF,	// 2 bytes/diff
		&pStaticMem[kStaticSkinDiffPtr]);

	// Patch control bytes to end properly.
	U8 *pSkinControlBytes = (U8 *)pStaticMem[kStaticSkinControlPtr];
	pSkinControlBytes[cmdQuad1.m_data.u16.C] = 8;   // CONTROL_END

	const U8 miscFlags = (cmdQuad1.m_data.u8.B & 0x0F);
	U8 *pInput         = (U8 *)(pStaticMem[kStaticInputBufferPtr] + cmdQuad1.m_data.u16.H);
	switch(miscFlags)
	{
	case kFormatWeightU8:
		{
			U16 numWeights = (cmdQuad2.m_data.u16.B + 0xF) & ~0xF;
			U32 weightSize = numWeights * sizeof(F32);
			U32 numWeightsPerDecomp = numWeights >> 2;
			F32 *pSkinWeights = (F32*)MeshMemAllocate(pStaticMem, weightSize);
			pStaticMem[kStaticSkinWeightPtr] = (U32)pSkinWeights;

			// Treat the weights as 1-byte vertexes
			DecompressU8(pInput, 4, (F32 *)pStaticMem[kStaticSkinWeightPtr], 4, numWeightsPerDecomp, 1 / 255.0f, 0.0f);

			// Handle the aux weight table, if it's present
			if (cmdQuad2.m_data.u16.A != 0)
			{
				F32 blendFactor = 0;
				F32 *pPmInfo = (F32 *)pStaticMem[kStaticDiscretePmInfoPtr];
				if(pPmInfo)
				{
					 blendFactor = *pPmInfo;
				}
				else
				{
					F32* discreteDmInfo = (F32 *)pStaticMem[kStaticDmInfoPtr];
					if(discreteDmInfo)
					{
						blendFactor = *discreteDmInfo;
					}
				}
				F32 *pAuxWeights = (F32 *)MeshMemAllocate(pStaticMem, weightSize);
				U8 *pAuxInput = (U8 *)(pStaticMem[kStaticInputBufferPtr] + cmdQuad2.m_data.u16.A);
				// Treat the weights as 1-byte vertexes
				DecompressU8(pAuxInput, 4, pAuxWeights, 4, numWeightsPerDecomp, 1 / 255.0f, 0.0f);
				BlendUniform(pSkinWeights, pAuxWeights, pSkinWeights, blendFactor, 1.0f - blendFactor, numWeightsPerDecomp);
				MeshMemFree(pStaticMem, (U32)pAuxWeights);
			}
		}
		break;
	case kFormatWeightF32:
		{
			U32 weightSize = (cmdQuad2.m_data.u16.B + 0x3F) & ~0x3F;
			U16 numWeights = weightSize / sizeof(F32);
			F32 *pSkinWeights = (F32*)MeshMemAllocate(pStaticMem, weightSize);
			pStaticMem[kStaticSkinWeightPtr] = (U32)pSkinWeights;

			CopyQWords((void *)pStaticMem[kStaticSkinWeightPtr], pInput, weightSize); // out, in, count

			// Handle the aux weight table, if it's present
			if (cmdQuad2.m_data.u16.A != 0)
			{
				F32 blendFactor = 0;
				F32 *pPmInfo = (F32 *)pStaticMem[kStaticDiscretePmInfoPtr];
				if(pPmInfo)
				{
					 blendFactor = *pPmInfo;
				}
				else
				{
					F32* discreteDmInfo = (F32 *)pStaticMem[kStaticDmInfoPtr];
					if(discreteDmInfo)
					{
						blendFactor = *discreteDmInfo;
					}
				}
				F32 *pAuxWeights = (F32 *)MeshMemAllocate(pStaticMem, weightSize);
				U8 *pAuxInput = (U8 *)(pStaticMem[kStaticInputBufferPtr] + cmdQuad2.m_data.u16.A);
				CopyQWords((void *)pAuxWeights, pAuxInput, weightSize); // out, in, count
				BlendUniform(pSkinWeights, pAuxWeights, pSkinWeights, blendFactor, 1.0f - blendFactor, numWeights >> 2);
				MeshMemFree(pStaticMem, (U32)pAuxWeights);
			}
		}
		break;
	default:
		ICE_ASSERT(0); // unsupported weight format
	}
}
#endif

#ifndef __SPU__
void CmdSetupMatrices(U32 *pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);
	U32 matrixSize = ((cmdQuad1.m_data.u16.C + 1) & ~0x1) * 12 * sizeof(F32); // 4x3 matrix
	pStaticMem[kStaticMatrixPtr] = (U32)MeshMemAllocate(pStaticMem, matrixSize);
	const U8 miscFlags        = (cmdQuad1.m_data.u8.B & 0x0F);
	U8 *pInputBuffer = (U8 *)pStaticMem[kStaticInputBufferPtr];
	switch(miscFlags)
	{
	case kFormatMatrix44:
		DecompressMatrices((F32 *)pStaticMem[kStaticMatrixPtr],
			(F32 *)(pInputBuffer + cmdQuad1.m_data.u16.B), cmdQuad1.m_data.u16.C);
		break;
	case kFormatMatrix43:
		CopyQWords((void *)pStaticMem[kStaticMatrixPtr], pInputBuffer + cmdQuad1.m_data.u16.B, matrixSize);
		break;
	default:
		ICE_ASSERT(0); // unsupported matrix format
	}
}
#endif

void Ice::MeshProc::CmdSetupDmInfo(U32 *pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);
	TransferData( pStaticMem, cmdQuad1.m_data.u16.B, kSizeOfDmInfo,
	    &pStaticMem[kStaticDmInfoPtr]);
}

void Ice::MeshProc::CmdSetupDmDisplacements(U32 *pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);
	/*
	  const U8 miscFlags = (cmdQuad1.m_data.u8.B & 0x0F);
	  U32 dispElemSize = 0;
	  switch(miscFlags)
	  {
	  case kFormatDmDispF32:
	  dispElemSize = 4;
	  break;
	  default:
	  break;
	  }
	*/
	TransferData( pStaticMem, cmdQuad1.m_data.u16.B, cmdQuad1.m_data.u16.C,
	    &pStaticMem[kStaticDmDisplacementPtr]);
}

