/*
 * Copyright (c) 2005 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#include "icemeshinternal.h"

#include <float.h>
#include <string.h>


using namespace Ice;
using namespace Ice::MeshProc;

#ifndef __SPU__

#include "rgmstable.cpp" // auto-generated RGMS lookup table.

/**
 * Concatenate two mat4x4s.
 */
static void MultiplyMat44(F32 out[16], const F32 mat1[16], const F32 mat2[16])
{
	out[0*4+0] = mat1[0*4+0]*mat2[0*4+0] + mat1[0*4+1]*mat2[1*4+0] + mat1[0*4+2]*mat2[2*4+0] + mat1[0*4+3]*mat2[3*4+0];
	out[0*4+1] = mat1[0*4+0]*mat2[0*4+1] + mat1[0*4+1]*mat2[1*4+1] + mat1[0*4+2]*mat2[2*4+1] + mat1[0*4+3]*mat2[3*4+1];
	out[0*4+2] = mat1[0*4+0]*mat2[0*4+2] + mat1[0*4+1]*mat2[1*4+2] + mat1[0*4+2]*mat2[2*4+2] + mat1[0*4+3]*mat2[3*4+2];
	out[0*4+3] = mat1[0*4+0]*mat2[0*4+3] + mat1[0*4+1]*mat2[1*4+3] + mat1[0*4+2]*mat2[2*4+3] + mat1[0*4+3]*mat2[3*4+3];

	out[1*4+0] = mat1[1*4+0]*mat2[0*4+0] + mat1[1*4+1]*mat2[1*4+0] + mat1[1*4+2]*mat2[2*4+0] + mat1[1*4+3]*mat2[3*4+0];
	out[1*4+1] = mat1[1*4+0]*mat2[0*4+1] + mat1[1*4+1]*mat2[1*4+1] + mat1[1*4+2]*mat2[2*4+1] + mat1[1*4+3]*mat2[3*4+1];
	out[1*4+2] = mat1[1*4+0]*mat2[0*4+2] + mat1[1*4+1]*mat2[1*4+2] + mat1[1*4+2]*mat2[2*4+2] + mat1[1*4+3]*mat2[3*4+2];
	out[1*4+3] = mat1[1*4+0]*mat2[0*4+3] + mat1[1*4+1]*mat2[1*4+3] + mat1[1*4+2]*mat2[2*4+3] + mat1[1*4+3]*mat2[3*4+3];

	out[2*4+0] = mat1[2*4+0]*mat2[0*4+0] + mat1[2*4+1]*mat2[1*4+0] + mat1[2*4+2]*mat2[2*4+0] + mat1[2*4+3]*mat2[3*4+0];
	out[2*4+1] = mat1[2*4+0]*mat2[0*4+1] + mat1[2*4+1]*mat2[1*4+1] + mat1[2*4+2]*mat2[2*4+1] + mat1[2*4+3]*mat2[3*4+1];
	out[2*4+2] = mat1[2*4+0]*mat2[0*4+2] + mat1[2*4+1]*mat2[1*4+2] + mat1[2*4+2]*mat2[2*4+2] + mat1[2*4+3]*mat2[3*4+2];
	out[2*4+3] = mat1[2*4+0]*mat2[0*4+3] + mat1[2*4+1]*mat2[1*4+3] + mat1[2*4+2]*mat2[2*4+3] + mat1[2*4+3]*mat2[3*4+3];

	out[3*4+0] = mat1[3*4+0]*mat2[0*4+0] + mat1[3*4+1]*mat2[1*4+0] + mat1[3*4+2]*mat2[2*4+0] + mat1[3*4+3]*mat2[3*4+0];
	out[3*4+1] = mat1[3*4+0]*mat2[0*4+1] + mat1[3*4+1]*mat2[1*4+1] + mat1[3*4+2]*mat2[2*4+1] + mat1[3*4+3]*mat2[3*4+1];
	out[3*4+2] = mat1[3*4+0]*mat2[0*4+2] + mat1[3*4+1]*mat2[1*4+2] + mat1[3*4+2]*mat2[2*4+2] + mat1[3*4+3]*mat2[3*4+2];
	out[3*4+3] = mat1[3*4+0]*mat2[0*4+3] + mat1[3*4+1]*mat2[1*4+3] + mat1[3*4+2]*mat2[2*4+3] + mat1[3*4+3]*mat2[3*4+3];
}

/**
 * Multiply a vec4 by a mat4x4, in-place.
 */
static void TransformVec4(const F32 mat[16], F32 vec[4])
{
	F32 prod[4];
	// The contents of the fourth component of the vector could be
	// garbage, so we assume it's 1.0
	prod[0] = mat[0*4+0]*vec[0] + mat[0*4+1]*vec[1] + mat[0*4+2]*vec[2] + mat[0*4+3]*1.0f;
	prod[1] = mat[1*4+0]*vec[0] + mat[1*4+1]*vec[1] + mat[1*4+2]*vec[2] + mat[1*4+3]*1.0f;
	prod[2] = mat[2*4+0]*vec[0] + mat[2*4+1]*vec[1] + mat[2*4+2]*vec[2] + mat[2*4+3]*1.0f;
	prod[3] = mat[3*4+0]*vec[0] + mat[3*4+1]*vec[1] + mat[3*4+2]*vec[2] + mat[3*4+3]*1.0f;

	vec[0] = prod[0];
	vec[1] = prod[1];
	vec[2] = prod[2];
	vec[3] = prod[3];
}

/**
 * Construct an object-to-screen transformation matrix from
 * information available in static memory.
 */
static void GetObjectToScreenMatrix(const ViewportInfo *pViewportInfo,
	const F32 *pRigidXformInfo, F32 subPixelOffset, F32 outMatrix[16])
{
	F32 objectToWorld[16] = {1,0,0,0,  0,1,0,0, 0,0,1,0, 0,0,0,1};
	if (pRigidXformInfo != 0)
		CopyQWords(objectToWorld, pRigidXformInfo, 16*sizeof(F32));

	F32 objectToHdc[16];
	MultiplyMat44(objectToHdc, pViewportInfo->m_worldToHdcMatrix, objectToWorld);

	F32 hdcToScreen[16] = {1,0,0,0,  0,1,0,0, 0,0,1,0, 0,0,0,1};
	hdcToScreen[0*4+0] = pViewportInfo->m_hdcToScreen[0];
	hdcToScreen[0*4+3] = pViewportInfo->m_hdcToScreen[1] + subPixelOffset;
	hdcToScreen[1*4+1] = pViewportInfo->m_hdcToScreen[2];
	hdcToScreen[1*4+3] = pViewportInfo->m_hdcToScreen[3] + subPixelOffset;

	MultiplyMat44(outMatrix, hdcToScreen, objectToHdc);
}

static U8 AreTrianglesTrimmable(IceQuadWord pPos[12], const F32 bounds[4],
	const U8 enableBackFaceCull, const U8 enableRgms,
	const U8 reverseBackFaceTest, const F32 depthBounds[2])
{
	U32 i;

	// We keep a byte which records whether each triangle can be trimmed.
	// The results are shifted up 4, because we'll eventually be using this
	// value as an index into a quadword table:
	// bit 4: set if triangle #1 should be trimmed
	// bit 5: set if triangle #2 should be trimmed
	// bit 6: set if triangle #3 should be trimmed
	// bit 7: set if triangle #4 should be trimmed
	U8 trimFlags = 0;

	// Test for back-facing triangles (if the caller asked us to).  If
	// the reverse backface flag is set, the winding order of the
	// triangle is effectively reversed (reverseBackFaceTest must be 0
	// or 1).
	F32 temp[4], cross, error;
	U8 isBackFacing = 0;
	for(i=0; i<4; ++i)
	{
		temp[0 + reverseBackFaceTest] = pPos[3*i+1].m_data.f32.A - pPos[3*i+0].m_data.f32.A; // h1-h0
		temp[1 - reverseBackFaceTest] = pPos[3*i+2].m_data.f32.A - pPos[3*i+0].m_data.f32.A; // h2-h0
		// V axis is inverted, so we negate the next two differences
		// by reversing the order of their operands
		temp[2 + reverseBackFaceTest] = pPos[3*i+0].m_data.f32.B - pPos[3*i+1].m_data.f32.B; // v1-v0
		temp[3 - reverseBackFaceTest] = pPos[3*i+0].m_data.f32.B - pPos[3*i+2].m_data.f32.B; // v2-v0
		cross                         = temp[0]*temp[3] - temp[1]*temp[2];
		error                         = -2*FLT_EPSILON * (temp[0] + temp[1] + temp[2] + temp[3] + 4*FLT_EPSILON);
		isBackFacing                 |= (cross < error) ? (0x10 << i) : 0x0;
	}
	trimFlags |= (isBackFacing & enableBackFaceCull);

	// Calculate triangle's screen-space bounding box
	F32 hMin[4], hMax[4], vMin[4], vMax[4];
	for(i=0; i<4; ++i)
	{
		hMin[i] = (pPos[3*i+0].m_data.f32.A < pPos[3*i+1].m_data.f32.A) ? pPos[3*i+0].m_data.f32.A : pPos[3*i+1].m_data.f32.A;
		vMin[i] = (pPos[3*i+0].m_data.f32.B < pPos[3*i+1].m_data.f32.B) ? pPos[3*i+0].m_data.f32.B : pPos[3*i+1].m_data.f32.B;
		hMax[i] = (pPos[3*i+0].m_data.f32.A > pPos[3*i+1].m_data.f32.A) ? pPos[3*i+0].m_data.f32.A : pPos[3*i+1].m_data.f32.A;
		vMax[i] = (pPos[3*i+0].m_data.f32.B > pPos[3*i+1].m_data.f32.B) ? pPos[3*i+0].m_data.f32.B : pPos[3*i+1].m_data.f32.B;

		hMin[i] = (pPos[3*i+2].m_data.f32.A < hMin[i])                  ? pPos[3*i+2].m_data.f32.A : hMin[i];
		vMin[i] = (pPos[3*i+2].m_data.f32.B < vMin[i])                  ? pPos[3*i+2].m_data.f32.B : vMin[i];
		hMax[i] = (pPos[3*i+2].m_data.f32.A > hMax[i])                  ? pPos[3*i+2].m_data.f32.A : hMax[i];
		vMax[i] = (pPos[3*i+2].m_data.f32.B > vMax[i])                  ? pPos[3*i+2].m_data.f32.B : vMax[i];
	}

	// Test for off-screen triangles.
	for(i=0; i<4; ++i)
	{
		trimFlags |= ((hMax[i] < bounds[0])
			|| (hMin[i] > bounds[1])
			|| (vMax[i] < bounds[2])
			|| (vMin[i] > bounds[3])) ? (0x10 << i) : 0;
	}

	// Test for triangles that don't hit any pixel samples.  We
	// perform two different tests (4xRGMS and normal), and use the
	// rgmsFlag to select from the two results.
	U8 missesPixelCenterNormal = 0;
	U8 missesPixelCenterRgms   = 0;
	for(i=0; i<4; ++i)
	{
		// Test against the center of the pixel.
		missesPixelCenterNormal |= (((I32)(hMin[i]-FLT_EPSILON) == (I32)(hMax[i]+FLT_EPSILON))
			|| ((I32)(vMin[i]-FLT_EPSILON) == (I32)(vMax[i]+FLT_EPSILON))) ? (0x10 << i) : 0;

		// Test against the four sub-pixel sample points in the
		// rotated grid.
		I32 originX  = (I32)(hMin[i] * 8.0f);
		I32 originY  = (I32)(vMin[i] * 8.0f);
		U32 width    = (U32)( (I32)(hMax[i] * 8.0f) - originX);
		U32 height   = (U32)( (I32)(vMax[i] * 8.0f) - originY);
		originX     &= 0x7;
		originY     &= 0x7;
		height       = (height > 7) ? 7 : height;
		U8 minWidth  = s_minWidthsForRgms[(height*64) + (originY*8) + originX];
		missesPixelCenterRgms |= (minWidth > width) ? (0x10 << i) : 0;
	}
	trimFlags |= enableRgms ? missesPixelCenterRgms : missesPixelCenterNormal;

	// Test if the triangle is partially behind the near clip
	// plane.  If so, the previous tests will be invalid, so we
	// can't trim.
	for(i=0; i<4; ++i)
	{
		bool partiallyBehind =
			(pPos[3*i+0].m_data.f32.C < depthBounds[0]) ||
			(pPos[3*i+1].m_data.f32.C < depthBounds[0]) ||
			(pPos[3*i+2].m_data.f32.C < depthBounds[0]);
		trimFlags &= partiallyBehind ? ~(0x10 << i) : 0xFF;
	}

	return trimFlags;
}

void TrimTriangles(U16 *pTris, U32 *trimCounts,
	const U8 *pViewportInfo_in, const F32 *pRigidXformInfo,
	const U8 *pObjectInfo, U32 *pVertTemp_in, U16 *pReindex,
    const F32 *pVertPositions_in)
{
	const ViewportInfo *pViewportInfo = (const ViewportInfo*)pViewportInfo_in;
	IceQuadWord *pVertTemp            = (IceQuadWord*)pVertTemp_in;
	const IceQuadWord *pVertPositions = (const IceQuadWord*)pVertPositions_in;

	// Extract trimming flag bits.
	U8 enableRgms          = (pViewportInfo->m_4xRgmsFlag)          ? 0xFF : 0x00;
	U8 reverseBackFaceTest = (pViewportInfo->m_reverseBackFaceFlag) ? 0x01 : 0x00;
	// The first byte of the object info structure (if it exists) is a
	// "disable backface culling" flag.
	U8 enableBackFaceCull  = (pObjectInfo && pObjectInfo[0])    ? 0x00 : 0xFF;

	// Construct object-to-screen matrix
	F32 objectToScreen[16];
	F32 subPixelOffset = enableRgms ? 0.0625f : 0.5f;
	GetObjectToScreenMatrix(pViewportInfo, pRigidXformInfo, subPixelOffset, objectToScreen);

	U32 loopCount = 0;

	// Loop #1: Transform vertex positions into screen-space.  Store
	// the transformed x, y and w (we don't care about z) in the
	// verttemp table.
	loopCount = trimCounts[1];
	for(U32 vert = 0; vert < loopCount; ++vert)
	{
		IceQuadWord vertPos = pVertPositions[vert];
		vertPos = pVertPositions[vert];
		TransformVec4(objectToScreen, (F32*)(&vertPos));
		F32 wInverse = 1.0f / vertPos.m_data.f32.D;
		pVertTemp[vert].m_data.f32.A = vertPos.m_data.f32.A * wInverse;
		pVertTemp[vert].m_data.f32.B = vertPos.m_data.f32.B * wInverse;
		pVertTemp[vert].m_data.f32.C = vertPos.m_data.f32.D;
	}
	
	// convert scissor area to floats
	F32 scissor[4];
	scissor[0] = (F32)(pViewportInfo->m_scissorArea)[0];
	scissor[1] = (F32)(pViewportInfo->m_scissorArea)[1];
	scissor[2] = (F32)(pViewportInfo->m_scissorArea)[2];
	scissor[3] = (F32)(pViewportInfo->m_scissorArea)[3];

	// Loop #2: Perform the trimming tests, writing the untrimmed
	// triangles back into the input index buffer (with empty spaces
	// removed).
	loopCount = (trimCounts[0]+3)/4;
	const U32 ignoreAtEnd = 4*loopCount - trimCounts[0]; // 0-3 triangles must be ignored at the end of the list
	const U8 lastIterationMask = ~(0xF0 >> ignoreAtEnd);
	U32 outTriCount = 0;
	const U16 *pInIndexes  = (const U16*)pTris;
	U16 *pOutIndexes = pTris;
	for(U32 loop=0; loop<loopCount; ++loop)
	{
		bool isLastIteration = (loop == loopCount-1);
#ifndef __SPU__
		// On the PPU, we have to be more careful about reading random
		// memory.  The last ignoreAtEnd triangles will be filled with
		// uninitialized data, and even though we know they'll be
		// forcibly trimmed later, we're about to use them as array
		// indexes.  To prevent operating on bad data, we initialize
		// those last triangles to all zeroes.
		if (isLastIteration)
		{
			U16 *pTempInIndexes = const_cast<U16*>(pInIndexes);
			for(U8 i=0; i<ignoreAtEnd; ++i)
			{
				pTempInIndexes[4*(4-i-1) + 0] = 0;
				pTempInIndexes[4*(4-i-1) + 1] = 0;
				pTempInIndexes[4*(4-i-1) + 2] = 0;
			}
		}
#endif
		

		// Load transformed vertex position data for four triangles
		IceQuadWord pPos[12];
		for(U8 i=0; i<4; ++i)
		{
			pPos[3*i+0] = pVertTemp[pInIndexes[4*i+0]];
			pPos[3*i+1] = pVertTemp[pInIndexes[4*i+1]];
			pPos[3*i+2] = pVertTemp[pInIndexes[4*i+2]];
		}

		// The high four bits of trimFlags indicate whether each
		// triangle of the four triangles should be trimmed.
		U8 trimFlags = AreTrianglesTrimmable(pPos, scissor, enableBackFaceCull,
			enableRgms, reverseBackFaceTest, pViewportInfo->m_depthBounds);

		// If this is the last loop iteration, we have to forcibly
		// trim away the last ignoreAtEnd triangles
		if (isLastIteration)
			trimFlags |= lastIterationMask;

		// Index trimming
		// The high four bits of trimFlags indicate whether each
		// triangle of the four triangles should be trimmed.
		for(U8 i=0; i<4; ++i)
		{
			U16 trimmed = (trimFlags & (0x10 << i)) ? 0xFFFF : 0;
			if (!trimmed)
			{
				pOutIndexes[0] = pInIndexes[i*4 + 0];
				pOutIndexes[1] = pInIndexes[i*4 + 1];
				pOutIndexes[2] = pInIndexes[i*4 + 2];
				pOutIndexes += 4;
				outTriCount += 1;
			}
		}
		pInIndexes += 16;
	}
	trimCounts[2] = outTriCount;

	// If we're not vertex trimming (or if there aren't any triangles
	// left), we can stop here
	if (pReindex == NULL || outTriCount == 0)
		return;

	// Loop #3: Clear the verttemp table to zero (it's about to become the rename table)
	loopCount = trimCounts[1];
	for(U32 vert=0; vert<loopCount; ++vert)
	{
		pVertTemp[vert].m_data.u32.A = 0;
		//pVertTemp[vert].m_data.u32.A = 0xFFFFFFFF; // HACK: uncomment this line to mark every vertex as used, effectively disabling vertex trimming.
	}

	// Loop #4: Iterate over the untrimmed triangles.  For each vertex that appears,
	// set its rename table entry to -1
	loopCount = trimCounts[2];
	for(U32 tri=0; tri<loopCount; ++tri)
	{
		ICE_ASSERT(pTris[tri*4+0] < trimCounts[1]);
		ICE_ASSERT(pTris[tri*4+1] < trimCounts[1]);
		ICE_ASSERT(pTris[tri*4+2] < trimCounts[1]);
		pVertTemp[ pTris[tri*4+0] ].m_data.u32.A = 0xFFFFFFFF;
		pVertTemp[ pTris[tri*4+1] ].m_data.u32.A = 0xFFFFFFFF;
		pVertTemp[ pTris[tri*4+2] ].m_data.u32.A = 0xFFFFFFFF;
	}

	// Loop #5: Build rename and reindex tables
	loopCount = trimCounts[1];
	U32 reindexCount = 0;
	for(U32 vert=0; vert<loopCount; ++vert)
	{
		if (pVertTemp[vert].m_data.u32.A == 0xFFFFFFFF)
		{
			pVertTemp[vert].m_data.u32.A = reindexCount;
			ICE_ASSERT(reindexCount < trimCounts[1]);
			pReindex[reindexCount++] = vert*16; // the reindex table is scaled up by 16, since it's used to index quads.
		}
	}
	
	// A little mini-loop to fill the remainder of the last reindex
	// quad with zeros; if we don't, terrible things can happen later
	// on.
	U32 tempReindexCount = reindexCount;
	while(tempReindexCount & 8)
	{
		pReindex[tempReindexCount++] = 0;
	}
	trimCounts[3] = reindexCount;

	// Loop #6: Apply rename table to output indexes.
	loopCount = trimCounts[2];
	for(U32 tri=0; tri<loopCount; ++tri)
	{
		pTris[tri*4+0] = pVertTemp[ pTris[tri*4+0] ].m_data.u32.A;
		pTris[tri*4+1] = pVertTemp[ pTris[tri*4+1] ].m_data.u32.A;
		pTris[tri*4+2] = pVertTemp[ pTris[tri*4+2] ].m_data.u32.A;
		ICE_ASSERT(pTris[tri*4+0] < trimCounts[3]);
		ICE_ASSERT(pTris[tri*4+1] < trimCounts[3]);
		ICE_ASSERT(pTris[tri*4+2] < trimCounts[3]);
	}
}
#endif // __SPU__

void TrimTrianglesWrapper(U32 *pStaticMem, bool enableVertexTrimming)
{
  	const U32 inTriangleCount = (pStaticMem[kStaticIndexCount]-pStaticMem[kStaticHaloIndexCount]) / 3;
	//const U32 inTriangleCount = 4; // HACK
	const U32 inVertexCount = pStaticMem[kStaticOutputVertexCount];

	if (enableVertexTrimming)
	{
		// Allocate reindex table, if it isn't there already
		CreateReindexTable(pStaticMem, false); // do NOT fill with dummy values!
	}

	// Allocate and initialize verttemp table (16 bytes per vertex, rounded up to 4 verts)
	IceQuadWord *pVertTemp  = (IceQuadWord *)MeshMemAllocate(pStaticMem, ((inVertexCount + 3) & ~3) * sizeof(IceQuadWord));

	U16 *pTris                    = (U16 *)pStaticMem[kStaticIndexPtr];
	IceQuadWord *pVertexPositions = (IceQuadWord *)pStaticMem[kStaticUniformPosPtr];
   	ViewportInfo *pViewportInfo   = (ViewportInfo *)pStaticMem[kStaticViewportInfoPtr];
	F32 *pRigidXformInfo          = (F32 *)pStaticMem[kStaticRigidObjectXformPtr];
	U8 *pObjectInfo               = (U8 *)pStaticMem[kStaticObjInfoPtr];
	U16 *pReindex                 = enableVertexTrimming ? (U16 *)pStaticMem[kStaticReindexPtr] : NULL;
	ICE_ASSERT(pTris != NULL);
	ICE_ASSERT(pVertexPositions != NULL);
	ICE_ASSERT(pViewportInfo != NULL);

	// This array contains four counters: 0 = inTriCount, 1 = inVertCount, 2 = outTriCount, 3 = reindexCount
	// The last two are filled in by TrimTriangles().
	ICE_ALIGN(16) U32 trimCounts[4] = { inTriangleCount, inVertexCount, 0, 0 };

	TrimTriangles(pTris, trimCounts, (U8*)pViewportInfo, pRigidXformInfo, pObjectInfo,
		(U32*)pVertTemp, pReindex, (F32*)pVertexPositions);

	//if (trimCounts[2] < trimCounts[0])
	//	PRINTF("trimmed from %3d to %3d triangles\n", trimCounts[0], trimCounts[2]);
	//if (enableVertexTrimming && trimCounts[3] < trimCounts[1])
	//	PRINTF("    and from %3d to %3d vertexes\n",  trimCounts[1], trimCounts[3]);

	//F32 cycles =  6.0f*trimCounts[1] + (pViewportInfo->m_4xRgmsFlag ? 21.0f : 17.75f)*trimCounts[0]; // start with the index trim count
	//if (enableVertexTrimming)
	//	cycles += 1.25f*trimCounts[1] + 5.25f*trimCounts[2] + 4.75f*trimCounts[1] + 7.5f*trimCounts[2];
	//PRINTF("%sex trimming performance: %.2f cycles/triangle\n", enableVertexTrimming ? "vert" : "ind",
	//	cycles / ((F32)trimCounts[0] + 0.00000001f));

		
	pStaticMem[kStaticIndexCount]              = trimCounts[2] * 3;
	pStaticMem[kStaticHaloIndexCount]          = 0;
	pStaticMem[kStaticOutputGenerateDrawCalls] = 1; // We're trimming, so we need to generate draw calls.
	// MeshMemFree(pStaticMem, pStaticMem[kStaticViewportInfoPtr]); do NOT do this! the new reindex buffer would be lost!
	MeshMemFree(pStaticMem, (U32)pVertTemp);
	if (enableVertexTrimming)
	{
		pStaticMem[kStaticReindexCount] = trimCounts[3];
		// pStaticMem[kStaticOutputVertexCount] = trimCounts[3]; // do NOT do this; we need the original vertex count for shadow cap output!
		pStaticMem[kStaticReindexIsDummy] = 0;
	}
}


/**
 * Perform index trimming on a triangle list.  A number of tests are
 * performed on each input triangle to determine whether it is
 * actually visible. A new index list is constructed containing only
 * the visible triangles.
 */
void Ice::MeshProc::CmdTrimIndexes(U32 *pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);

	TrimTrianglesWrapper(pStaticMem, false);
}


/**
 * Perform vertex trimming on a triangle list.  A number of tests are
 * performed on each input triangle to determine whether it is
 * actually visible. A new index list is constructed containing only
 * the visible triangles. In addition, a reindex table is generated which
 * can be passed to the compression routines to remove all unused vertexes
 * from the output NV streams.
 */
void Ice::MeshProc::CmdTrimVertexes(U32 *pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);

	TrimTrianglesWrapper(pStaticMem, true);
}

