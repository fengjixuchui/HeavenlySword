/*
 * Copyright (c) 2005 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#include "icemeshinternal.h"

using namespace Ice;
using namespace Ice::MeshProc;

#ifndef __SPU__
void BlendUniform(F32 *in1, F32 *in2, F32 *out, F32 blendFactor, F32 blendFactor2, U32 numElems)
{
	for(U32 i = 0; i < numElems; i+=4)
	{
		*(out++) = *(in1++) * blendFactor + *(in2++) * blendFactor2; // X
		*(out++) = *(in1++) * blendFactor + *(in2++) * blendFactor2; // Y
		*(out++) = *(in1++) * blendFactor + *(in2++) * blendFactor2; // Z
		*(out++) = *(in1++) * blendFactor + *(in2++) * blendFactor2; // W

		*(out++) = *(in1++) * blendFactor + *(in2++) * blendFactor2; // X
		*(out++) = *(in1++) * blendFactor + *(in2++) * blendFactor2; // Y
		*(out++) = *(in1++) * blendFactor + *(in2++) * blendFactor2; // Z
		*(out++) = *(in1++) * blendFactor + *(in2++) * blendFactor2; // W

		*(out++) = *(in1++) * blendFactor + *(in2++) * blendFactor2; // X
		*(out++) = *(in1++) * blendFactor + *(in2++) * blendFactor2; // Y
		*(out++) = *(in1++) * blendFactor + *(in2++) * blendFactor2; // Z
		*(out++) = *(in1++) * blendFactor + *(in2++) * blendFactor2; // W

		*(out++) = *(in1++) * blendFactor + *(in2++) * blendFactor2; // X
		*(out++) = *(in1++) * blendFactor + *(in2++) * blendFactor2; // Y
		*(out++) = *(in1++) * blendFactor + *(in2++) * blendFactor2; // Z
		*(out++) = *(in1++) * blendFactor + *(in2++) * blendFactor2; // W
	}
}
#endif

void Ice::MeshProc::CmdSetupRunTable(U32 *pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);
	U8 *pInputBuffer = (U8*)pStaticMem[kStaticInputBufferPtr];

	// If this is not the first blend shape, then a run table and a delta format info structure exist in the
	// work buffer.  These need to be freed and then reallocated for the next blend shape.
	U32 indexTablePtr = pStaticMem[kStaticIndexTablePtr];
	if (indexTablePtr != 0)
		MeshMemFree(pStaticMem, indexTablePtr);

	// Get the parameters to this command.
	U16 *pRunList = (U16 *)(pInputBuffer + cmdQuad1.m_data.u16.B);
	U16 numRuns = cmdQuad1.m_data.u16.C;

	// Reserve space for a two-byte index for the maximum number of vertexes, rounded up to next quadword.
	U16 *pIndexTable = (U16 *)MeshMemAllocate(pStaticMem, (pStaticMem[kStaticVertexCount] * sizeof(U16) + 0xF) & ~0xF);
	pStaticMem[kStaticIndexTablePtr] = (U32)pIndexTable;

	// Generate the indexes from the run list.
	U32 numIndexes = 0;
	while (numRuns > 0) {
		U32 runLength = *pRunList & 0x3F;
		numIndexes+=runLength;
		U32 startIndex = (*pRunList >> 2) & 0x3FF0;
		while (runLength > 0) {
			*pIndexTable = startIndex;
			pIndexTable++;
			startIndex += 0x10;
			runLength--;
		}
		pRunList++;
		numRuns--;
	}

	// Save the pointer to the index table as well as the number of entries in the table.
	pStaticMem[kStaticNumIndexesInTable] = numIndexes;
}

static void BlendUniformUsingIndexTable(U32 *pStaticMem, F32 *pUniform, F32 *pDeltaStream, F32 blendShapeFactor, U32 numComponents)
{
	// Get the index table info.
	U16 *pIndexTable = (U16 *)pStaticMem[kStaticIndexTablePtr];
	U32 numIndexes = pStaticMem[kStaticNumIndexesInTable];

	while (numIndexes > 0) {
		// Get the current index from the index table and increment the index table pointer.
		U32 index = *pIndexTable / sizeof(F32);
		pIndexTable++;
		
		// Blend each component that is active.
		for (U32 ii = 0; ii < numComponents; ii++)
			pUniform[index + ii] += pDeltaStream[ii] * blendShapeFactor;

		// Increment the delta stream to the next vertex.
		pDeltaStream += 4;

		numIndexes--;
	}
}

void Ice::MeshProc::CmdBlendDeltaStream(U32 *pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);
	U8 *pInputBuffer = (U8*)pStaticMem[kStaticInputBufferPtr];

	// Unpack blend factor from 1.3.12 fixed-point to F32. If the
	// blending wouldn't have any effect, early out of the function.
	F32 blendShapeFactor = float(cmdQuad1.m_data.i16.C) / 4096.0f;
	if (blendShapeFactor == 0.0f)
		return;

	U8 const streamName = cmdQuad1.m_data.u8.B;
	U8 *pStream = pInputBuffer + cmdQuad1.m_data.u16.B;
	U8 *pFormat = (U8 *)(pStaticMem[kStaticDeltaFormatPtr] + (streamName + 1) * 8);

	// Get the number of vertexes to decompress and allocate a uniform table of the correct size.
	U16 const numVerts = (pStaticMem[kStaticNumIndexesInTable] + 3) & ~3;
	U32 const uniformSize = 0x10 * (numVerts + 4);
	F32 *pUniformBlend = (F32*)MeshMemAllocate(pStaticMem, uniformSize );

	// Get attribute data about the delta stream.
	U8 const id = pFormat[0];
	ICE_ASSERT(id > 0);
	U8 const scaleAndBiasFlags = pFormat[1];
	U8 const componentCount = scaleAndBiasFlags & 0xF;

	// Get scale, bias, and intoff data about this delta stream.
	U32 scaleAndBiasBase = pStaticMem[kStaticFixedFormatPtr];
	if (scaleAndBiasFlags & kScaleAndBiasVariable)
		scaleAndBiasBase = pStaticMem[kStaticDeltaFormatPtr];
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
		
	// Decompress the delta stream.
	DecompressSwFormat(pStream, pUniformBlend, numVerts, pFormat[4], pFormat[5], pFormat[6], pFormat[7], intoff, scale, bias);

	// Find the index of the uniform table that matches the id of this delta stream.
	// In SPU code, this can be reduced to shufb, ceqb, gbb, clz
	U8 *pAttribIds  = (U8 *)&pStaticMem[kStaticAttributeId];
	U8 uniformIndex = 0;
	for(; uniformIndex<16; ++uniformIndex) {
		if (pAttribIds[uniformIndex] == id)
			break;
	}
	ICE_ASSERT(uniformIndex < 16);
	F32 *pUniformDest = (F32 *)(pStaticMem[kStaticUniformPtr + uniformIndex]);

	BlendUniformUsingIndexTable(pStaticMem, pUniformDest, pUniformBlend, blendShapeFactor, componentCount);

	// Free the uniform table that was allocated for unpacking the delta stream.
	MeshMemFree(pStaticMem, (U32)pUniformBlend);
}

