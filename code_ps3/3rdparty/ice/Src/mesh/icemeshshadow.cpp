/*
 * Copyright (c) 2005 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#include <stdlib.h>
#include <math.h>
#include "icemeshinternal.h"

#if ICE_MESH_STENCIL_SHADOWS_ON
using namespace Ice;
using namespace Ice::MeshProc;

#define X 0
#define Y 1
#define Z 2
#define W 3

// returns # of triangles facing the light
#ifndef __SPU__
U32 GenerateTriangleFacingTable(U16* indexes, F32 *pos, U32 *facing, F32 lightPos[3], U32 isPointLight, U32 numTris)
{
	U32 facingCount = 0;
	F32 vertScale = isPointLight ? 1.f : 0.f;

	for(U32 i = 0; i < numTris; i+=4)
	{
		for(U32 ii = i; ii < i+4; ++ii)
		{
			F32 vert0[3] = 
				{
					pos[indexes[ii*4+0]*4+X],
					pos[indexes[ii*4+0]*4+Y],
					pos[indexes[ii*4+0]*4+Z]
				};
			F32 vert1[3] = 
				{
					pos[indexes[ii*4+1]*4+X],
					pos[indexes[ii*4+1]*4+Y],
					pos[indexes[ii*4+1]*4+Z]
				};
			F32 vert2[3] = 
				{
					pos[indexes[ii*4+2]*4+X],
					pos[indexes[ii*4+2]*4+Y],
					pos[indexes[ii*4+2]*4+Z]
				};
			F32 toLight[3] = 
				{
					lightPos[X] - vert0[X] * vertScale,
					lightPos[Y] - vert0[Y] * vertScale,
					lightPos[Z] - vert0[Z] * vertScale
				};
			F32 legA[3] =
				{
					vert1[X] - vert0[X],
					vert1[Y] - vert0[Y],
					vert1[Z] - vert0[Z]
				};
			F32 legB[3] = 
				{
					vert2[X] - vert0[X],
					vert2[Y] - vert0[Y],
					vert2[Z] - vert0[Z]
				};
			F32 normal[3] =
				{
					legA[Y]*legB[Z] - legA[Z]*legB[Y],
					legA[Z]*legB[X] - legA[X]*legB[Z],
					legA[X]*legB[Y] - legA[Y]*legB[X]
				};
			F32 dot = normal[X]*toLight[X] + normal[Y]*toLight[Y] + normal[Z]*toLight[Z];
			facing[ii] = (dot > 0) ? ~0 : 0;
			facingCount += facing[ii] & 1;
		}
	}

	return facingCount;
}

// returns number of profile edges
U32 GenerateProfileEdgeList(U32 *edges, U16 *profileEdges, U32 numEdges, U32 *facing)
{
	U32 numProfileEdges = 0;
	for(U32 i = 0; i < numEdges; ++i)
	{
		U32 edge = edges[i];
		ICE_ASSERT((edge & 0x3) < 3); // can't start an edge on vertex #3
		U16 triOffset1 = U16((edge >> 16)    & ~7) >> 1;
		U16 triOffset2 = U16((edge & 0xFFFF) & ~7) >> 1;
		profileEdges[numProfileEdges] = (edge >> (facing[triOffset2/sizeof(*facing)] & 0x10)) & 0xFFFF;
		numProfileEdges += (facing[triOffset1/sizeof(*facing)] ^ facing[triOffset2/sizeof(*facing)]) & 1;
	}
	
	// Write out an extra quadword worth of junk data, as the actual algorithm 
	// will trash it.
	memset(&profileEdges[numProfileEdges], 0xCD, 16);
	return numProfileEdges;
}

void ExtrudeProfileEdges(U16 *indexes, F32 *pos, U16 *profileEdges, F32 *out, U32 numProfileEdges, U32 isPointLight)
{
	U16 *indexes2 = indexes+1;
	U32 bytesPerEdge = isPointLight ? 0x40 : 0x30;
	// Generate a batch of extruded edges
	for(U32 i=0; i<numProfileEdges; ++i)
	{
		U16 edge = profileEdges[i];
		U16 index = ((edge & ~0x7)>>1) + (edge & 0x3);
		
		// NOTE: Assumes 4th component of pos is a copy of the first. 
		out[0*4+X] = pos[indexes2[index]*4+X];
		out[0*4+Y] = pos[indexes2[index]*4+Y];
		out[0*4+Z] = pos[indexes2[index]*4+Z];
		out[0*4+W] = 1.f;
		
		out[1*4+X] = pos[indexes[index]*4+X];
		out[1*4+Y] = pos[indexes[index]*4+Y];
		out[1*4+Z] = pos[indexes[index]*4+Z];
		out[1*4+W] = 1.f;
		
		out[2*4+X] = out[1*4+X];
		out[2*4+Y] = out[1*4+Y];
		out[2*4+Z] = out[1*4+Z]; 
		out[2*4+W] = 0.f; 
		
		// Note: for directional lights, this will write an extra
		// quadword past the end of the array.
		out[3*4+X] = out[0*4+X];
		out[3*4+Y] = out[0*4+Y];
		out[3*4+Z] = out[0*4+Z]; 
		out[3*4+W] = 0.f; 
		
		out += (bytesPerEdge/sizeof(*out));
	}
}
#endif

/**
 * @return The number of indexes in the cap.
 */
U32 GenerateCapIndexes(U16 *indexes, U32 *facing, U16* out, U32 numTris)
{
	U32 capIndexCount = 0;

	// Note: numTris is NOT guarenteed to be a multiple of 4, since
	// this count doesn't include halo triangles at the end of the
	// index buffer.
	for(U32 i = 0; i < numTris; ++i)
	{
		out[0] = indexes[i*4+0];
		out[1] = indexes[i*4+1];
		out[2] = indexes[i*4+2];
		out           += facing[i] & 3;
		capIndexCount += facing[i] & 3;
	}

	// Write out an extra quadword worth of junk data, as the actual algorithm 
	// will trash it.
	memset(out, 0xCD, 16);
	return capIndexCount;
}

void Ice::MeshProc::CmdExtrudeShadows(U32 *pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);

	const U8 dataTypeSizes[kNumTypes] =
		{
			4, // kF32,
			2, // kF16,
			2, // kI16n,
			2, // kI16,
			4, // kX11Y11Z10n,
			1, // kU8n,
			1, // kU8,
		};

	// Allocate facing table up to next qword size (each tri gets 4 bytes)
	U32 numTriangles = pStaticMem[kStaticIndexCount] / 3;
	pStaticMem[kStaticTriFacingPtr] =
		(U32)MeshMemAllocate(pStaticMem, (numTriangles*sizeof(U32) + 0xF) & ~0xF);

	// Edge table is allocated to nearest qword, with one extra qword added for hword stream issues
	pStaticMem[kStaticProfileEdgePtr] = (U32)MeshMemAllocate(pStaticMem, ((pStaticMem[kStaticEdgeCount] + 0xF) & ~0x7) * 2);
	U32 numLights = *(U32 *)(pStaticMem[kStaticNvControlPtr] + 0x2C);
	ICE_ASSERT(numLights <= kMaxLightCount);
	U32 *pShadowInfo     = (U32 *)pStaticMem[kStaticShadowInfoPtr];
	U32 numHaloTriangles = pStaticMem[kStaticHaloIndexCount] / 3;
	U32 numEdges         = pStaticMem[kStaticEdgeCount];

	U8 outputVertexFormat = cmdQuad1.m_data.u8.B & 0x0F;
	pStaticMem[kStaticShadowCapVertexFormat] = outputVertexFormat;

	U16 *indexes      = (U16 *)pStaticMem[kStaticIndexPtr];
	F32 *pos          = (F32 *)pStaticMem[kStaticUniformPosPtr];
	U32 *facing       = (U32 *)pStaticMem[kStaticTriFacingPtr];
	U32 *edges        = (U32 *)pStaticMem[kStaticEdgePtr];
	U16 *profileEdges = (U16 *)pStaticMem[kStaticProfileEdgePtr];

	// Shadow generation uses a small dummy reindex table with only
	// 256 entries.  Edges are written 64 at a time using this table
	ICE_ASSERT(pStaticMem[kStaticReindexPtr] == 0); // in the current design, nothing can create a reindex table before this.
	U16 *pReindex = (U16*)MeshMemAllocate(pStaticMem, 256*sizeof(U16));
	F32 *pOutEdgeRaw = (F32*)MeshMemAllocate(pStaticMem, 4096);
	for(U16 i=0; i<256; ++i)
		pReindex[i] = i*16;
	pStaticMem[kStaticReindexPtr]   = (U32)pReindex;
	pStaticMem[kStaticReindexCount] = 256;

	// Scale and biases for output shadow positions.
	// These may not be correct, but the data is not readily available to do this properly.
	IceQuadWord scale;
	IceQuadWord bias;
	scale.m_data.f32.A = 1.0f;
	scale.m_data.f32.B = 1.0f;
	scale.m_data.f32.C = 1.0f;
	scale.m_data.f32.D = 1.0f;
	bias.m_data.f32.A = 0.0f;
	bias.m_data.f32.B = 0.0f;
	bias.m_data.f32.C = 0.0f;
	bias.m_data.f32.D = 0.0f;

	for(U16 i=0; i<numLights; ++i)
	{
		F32 *lightPos = (F32*)(pShadowInfo) + 4*i;
		U32 isPointLight = ((U8*)pShadowInfo)[16*i + 12];

		U8 outputVertexComponentCount = 4;
		U8 outputVertexStride = outputVertexComponentCount*dataTypeSizes[outputVertexFormat];

		// Generate the triangle facing table and profile edge list
		// for this light.
		const U8 vertsPerEdge = isPointLight ? 4 : 3;
		U32 numFacingLight    = GenerateTriangleFacingTable(indexes, pos, facing, lightPos, isPointLight, numTriangles);
		U32 numProfileEdges   = GenerateProfileEdgeList(edges, profileEdges, numEdges, facing);
		ICE_ASSERT(numProfileEdges < 0x3FFF); // otherwise, the next line will overflow
		pStaticMem[kStaticShadowProfileEdgeVertexCount + i] = (numProfileEdges * vertsPerEdge) | (isPointLight << 16);

		// Allocate main-memory space for extruded profile edge
		// geometry and shadow cap indexes.  Though this variable is a
		// pointer, we never use it as such, and it makes more sense
		// to declare it as U32.
		U32 shadowOutputSize   = ( ((numProfileEdges * vertsPerEdge + 0x3) & ~0x3) * outputVertexStride + 0x7F ) & ~0x7F;
		shadowOutputSize      += ( ((numFacingLight*3 + 0x17) & ~0x7) * sizeof(U16)  + 0x7F ) & ~0x7F;
		U32 pShadowOutput      = (U32)MeshMutexAllocate(pStaticMem, shadowOutputSize);

		// Extrude profile edges for shadow sides.  We write the
		// uncompressed geometry into the work buffer, and then
		// compress it into the output buffer.
		const U32 edgesPerBatch       = 64; 
		const U32 compressedBatchSize = edgesPerBatch*vertsPerEdge*outputVertexStride;
		U32 *edgePatchPtr             = &pStaticMem[kStaticShadowProfileEdgeOutputPtr + i];
		const U32 batchCount          = numProfileEdges / edgesPerBatch;
		for(U16 b = 0; b<batchCount; ++b)
		{
			ExtrudeProfileEdges(indexes, pos, profileEdges + edgesPerBatch*b, pOutEdgeRaw, edgesPerBatch, isPointLight);
			U8 *pOutEdgeCompressed = (U8*)MeshOutputAllocate(pStaticMem, compressedBatchSize, 16, edgePatchPtr, &pShadowOutput);

#ifndef __SPU__
			// Collect some stats on what is being output.
			g_meshProcessingStats.m_profileEdgeNum += 64;
			g_meshProcessingStats.m_profileEdgeSize += compressedBatchSize;
#endif

			edgePatchPtr = NULL; // should only be non-NULL for the first batch of edges in each light.
			U32 numVerts = edgesPerBatch * 4;
			pStaticMem[kStaticReindexCount] = numVerts;
			InsertAttributeIntoNvStream(pOutEdgeRaw, pOutEdgeCompressed, outputVertexStride,
				outputVertexComponentCount, outputVertexFormat, scale, bias, pReindex, numVerts);
		}
		const U32 lastCount = numProfileEdges % edgesPerBatch;
		if (lastCount > 0)
		{
			ExtrudeProfileEdges(indexes, pos, profileEdges + edgesPerBatch*batchCount, pOutEdgeRaw, lastCount, isPointLight);
			U8 *pOutEdgeCompressed = (U8*)MeshOutputAllocate(pStaticMem,
				((lastCount * vertsPerEdge + 0x3) & ~0x3) * outputVertexStride, 16, edgePatchPtr, &pShadowOutput);

#ifndef __SPU__
			// Collect some stats on what is being output.
			g_meshProcessingStats.m_profileEdgeNum += lastCount;
			g_meshProcessingStats.m_profileEdgeSize += ((lastCount * vertsPerEdge + 0x3) & ~0x3) * outputVertexStride;
#endif

			U32 numVerts = lastCount * 4;
			pStaticMem[kStaticReindexCount] = numVerts;
			InsertAttributeIntoNvStream(pOutEdgeRaw, pOutEdgeCompressed, outputVertexStride,
				outputVertexComponentCount, outputVertexFormat, scale, bias, pReindex, numVerts);
		}

		// Generate caps for this light
		U8 generateCapsForLight = ((U8*)pShadowInfo)[16 * i + 13];
		if (generateCapsForLight)
		{
			// At this stage of processing, we use the cap vertex
  			// output ptr as a flag to indicate that cap vertexes
			// should be output later, in CmdCleanupAndExit.
			pStaticMem[kStaticShadowCapVertexOutputPtr] = 1;
			// Allocate to next qword boundry and add two extra qwords (for hword stream issues)
			U16 *pOutCapIndex = (U16*)MeshOutputAllocate(pStaticMem, ((numFacingLight * 3 + 0x17) & ~0x7) * sizeof(U16),
				128, &pStaticMem[kStaticShadowCapIndexOutputPtr + i], &pShadowOutput);

#ifndef __SPU__
			// Collect some stats on what is being output.
			g_meshProcessingStats.m_capIndexesNum += numFacingLight * 3;
			g_meshProcessingStats.m_capIndexesSize += ((numFacingLight * 3 + 0x17) & ~0x7) * 2;
#endif

			pStaticMem[kStaticShadowCapIndexCount+i] = GenerateCapIndexes(indexes, facing,
				pOutCapIndex, numTriangles - numHaloTriangles);
		}

		// Kick output buffer between lights!
		MeshKickOutputBuffer(pStaticMem, &pShadowOutput);
	}

	MeshMemFree(pStaticMem, pStaticMem[kStaticShadowInfoPtr]);
	// clear reindex buffer info, so it's created fresh the next time it's needed.
	pStaticMem[kStaticReindexCount] = 0;
	pStaticMem[kStaticReindexPtr]   = 0;
}
#endif // ICE_MESH_STENCIL_SHADOWS_ON

