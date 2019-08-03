/*
 * Copyright (c) 2005 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#include <stdlib.h>
#include <math.h>
#include "icemeshinternal.h"

using namespace Ice;
using namespace Ice::MeshProc;

#define X 0
#define Y 1
#define Z 2
#define W 3

#ifndef __SPU__
void PmDiscreteTower(U16 *pParent, U32 numPtrs, U32 *pIn, U32 *pOut, F32 blendFactor, U32 startVert, U32 vertCount)
{
	F32 invBlendFactor = 1.f - blendFactor;

	ICE_ASSERT(((U32)pParent & 0xF) == 0);
	ICE_ASSERT(((U32)pIn & 0xF) == 0);
	ICE_ASSERT(((U32)pOut & 0xF) == 0);
	vertCount = (vertCount+3)&~3;

	ICE_ASSERT(blendFactor >= 0.0f);
	ICE_ASSERT(blendFactor <= 1.0f);

	//PRINTF("*PmDiscrete*\n");
	//PRINTF("\tPmDiscrete : vertCount	= %d  (%f)\n", vertCount, blendFactor);

	const U32 start = startVert*4;
	const U32 end = (startVert+vertCount)*4;
	for(U32 i = start; i < end; i+=4, ++pParent)
	{
		const U32 parent = *pParent * 4;

		F32 *pDst;
		const F32 *pSrc;
		switch(numPtrs)
		{
		default:
			for(U32 ii = 8; ii < numPtrs; ++ii)
			{
				pDst = (F32 *)pOut[ii];
				pSrc = (const F32 *)pIn[ii];
				const F32 inX = pSrc[i+X];
				const F32 inY = pSrc[i+Y];
				const F32 inZ = pSrc[i+Z];
				const F32 inW = pSrc[i+W];
				pDst[i+X] = inX + (pSrc[parent+X] - inX) * invBlendFactor;
				pDst[i+Y] = inY + (pSrc[parent+Y] - inY) * invBlendFactor;
				pDst[i+Z] = inZ + (pSrc[parent+Z] - inZ) * invBlendFactor;
				pDst[i+W] = inW + (pSrc[parent+W] - inW) * invBlendFactor;
			}
		case 8:
		{
			pDst = (F32 *)pOut[7];
			pSrc = (const F32 *)pIn[7];
			const F32 inX = pSrc[i+X];
			const F32 inY = pSrc[i+Y];
			const F32 inZ = pSrc[i+Z];
			const F32 inW = pSrc[i+W];
			pDst[i+X] = inX + (pSrc[parent+X] - inX) * invBlendFactor;
			pDst[i+Y] = inY + (pSrc[parent+Y] - inY) * invBlendFactor;
			pDst[i+Z] = inZ + (pSrc[parent+Z] - inZ) * invBlendFactor;
			pDst[i+W] = inW + (pSrc[parent+W] - inW) * invBlendFactor;
		}
		case 7:
		{
			pDst = (F32 *)pOut[6];
			pSrc = (const F32 *)pIn[6];
			const F32 inX = pSrc[i+X];
			const F32 inY = pSrc[i+Y];
			const F32 inZ = pSrc[i+Z];
			const F32 inW = pSrc[i+W];
			pDst[i+X] = inX + (pSrc[parent+X] - inX) * invBlendFactor;
			pDst[i+Y] = inY + (pSrc[parent+Y] - inY) * invBlendFactor;
			pDst[i+Z] = inZ + (pSrc[parent+Z] - inZ) * invBlendFactor;
			pDst[i+W] = inW + (pSrc[parent+W] - inW) * invBlendFactor;
		}
		case 6:
		{
			pDst = (F32 *)pOut[5];
			pSrc = (const F32 *)pIn[5];
			const F32 inX = pSrc[i+X];
			const F32 inY = pSrc[i+Y];
			const F32 inZ = pSrc[i+Z];
			const F32 inW = pSrc[i+W];
			pDst[i+X] = inX + (pSrc[parent+X] - inX) * invBlendFactor;
			pDst[i+Y] = inY + (pSrc[parent+Y] - inY) * invBlendFactor;
			pDst[i+Z] = inZ + (pSrc[parent+Z] - inZ) * invBlendFactor;
			pDst[i+W] = inW + (pSrc[parent+W] - inW) * invBlendFactor;
		}
		case 5:
		{
			pDst = (F32 *)pOut[4];
			pSrc = (const F32 *)pIn[4];
			const F32 inX = pSrc[i+X];
			const F32 inY = pSrc[i+Y];
			const F32 inZ = pSrc[i+Z];
			const F32 inW = pSrc[i+W];
			pDst[i+X] = inX + (pSrc[parent+X] - inX) * invBlendFactor;
			pDst[i+Y] = inY + (pSrc[parent+Y] - inY) * invBlendFactor;
			pDst[i+Z] = inZ + (pSrc[parent+Z] - inZ) * invBlendFactor;
			pDst[i+W] = inW + (pSrc[parent+W] - inW) * invBlendFactor;
		}
		case 4:
		{
			pDst = (F32 *)pOut[3];
			pSrc = (const F32 *)pIn[3];
			const F32 inX = pSrc[i+X];
			const F32 inY = pSrc[i+Y];
			const F32 inZ = pSrc[i+Z];
			const F32 inW = pSrc[i+W];
			pDst[i+X] = inX + (pSrc[parent+X] - inX) * invBlendFactor;
			pDst[i+Y] = inY + (pSrc[parent+Y] - inY) * invBlendFactor;
			pDst[i+Z] = inZ + (pSrc[parent+Z] - inZ) * invBlendFactor;
			pDst[i+W] = inW + (pSrc[parent+W] - inW) * invBlendFactor;
		}
		case 3:
		{
			pDst = (F32 *)pOut[2];
			pSrc = (const F32 *)pIn[2];
			const F32 inX = pSrc[i+X];
			const F32 inY = pSrc[i+Y];
			const F32 inZ = pSrc[i+Z];
			const F32 inW = pSrc[i+W];
			pDst[i+X] = inX + (pSrc[parent+X] - inX) * invBlendFactor;
			pDst[i+Y] = inY + (pSrc[parent+Y] - inY) * invBlendFactor;
			pDst[i+Z] = inZ + (pSrc[parent+Z] - inZ) * invBlendFactor;
			pDst[i+W] = inW + (pSrc[parent+W] - inW) * invBlendFactor;
		}
		case 2:
		{
			pDst = (F32 *)pOut[1];
			pSrc = (const F32 *)pIn[1];
			const F32 inX = pSrc[i+X];
			const F32 inY = pSrc[i+Y];
			const F32 inZ = pSrc[i+Z];
			const F32 inW = pSrc[i+W];
			pDst[i+X] = inX + (pSrc[parent+X] - inX) * invBlendFactor;
			pDst[i+Y] = inY + (pSrc[parent+Y] - inY) * invBlendFactor;
			pDst[i+Z] = inZ + (pSrc[parent+Z] - inZ) * invBlendFactor;
			pDst[i+W] = inW + (pSrc[parent+W] - inW) * invBlendFactor;
		}
		case 1:
		{
			pDst = (F32 *)pOut[0];
			pSrc = (const F32 *)pIn[0];
			const F32 inX = pSrc[i+X];
			const F32 inY = pSrc[i+Y];
			const F32 inZ = pSrc[i+Z];
			const F32 inW = pSrc[i+W];
			pDst[i+X] = inX + (pSrc[parent+X] - inX) * invBlendFactor;
			pDst[i+Y] = inY + (pSrc[parent+Y] - inY) * invBlendFactor;
			pDst[i+Z] = inZ + (pSrc[parent+Z] - inZ) * invBlendFactor;
			pDst[i+W] = inW + (pSrc[parent+W] - inW) * invBlendFactor;
			break;
		}
		}
	}
}
#endif//__SPU__

#ifndef __SPU__
void PmContinuousTower(const U16 *pParent, const U32 numPtrs, const F32 *pPos, const U32 *pIn, U32 *pOut, const F32 *blendParms, const F32 *cameraLoc, U32 startVert, U32 vertCount)
{
	//PRINTF("*PmContinuousTower*\n");
	ICE_ASSERT(((U32)pParent & 0xF) == 0);
	ICE_ASSERT(((U32)pPos & 0xF) == 0);
	ICE_ASSERT(((U32)pIn & 0xF) == 0);
	ICE_ASSERT(((U32)pOut & 0xF) == 0);
	ICE_ASSERT(((U32)blendParms & 0xF) == 0);
	ICE_ASSERT(((U32)cameraLoc & 0xF) == 0);
	vertCount = (vertCount+3)&~3;
	const U32 start = startVert*4;
	const U32 end = (startVert+vertCount)*4;
	const F32 near = blendParms[0];
	const F32 neard = 1.f / (blendParms[1]-blendParms[0]);
	const F32 cameraX = cameraLoc[X];
	const F32 cameraY = cameraLoc[Y];
	const F32 cameraZ = cameraLoc[Z];
	for(U32 i = start; i < end; i+=4, ++pParent)
	{
		const F32 x = pPos[i+X] - cameraX;
		const F32 y = pPos[i+Y] - cameraY;
		const F32 z = pPos[i+Z] - cameraZ;
		const F32 dist = sqrtf(x*x + y*y + z*z);
		const F32 blendFactor = Min(Max((dist - near) * neard, 0.f), 1.f);
		const U32 parent = *pParent * 4;

		F32 *pDst;
		const F32 *pSrc;
		switch(numPtrs)
		{
		default:
		{
			for(U32 ii = 8; ii < numPtrs; ++ii)
			{
				pDst = (F32 *)pOut[ii];
				pSrc = (const F32 *)pIn[ii];
				const F32 inX = pSrc[i+X];
				const F32 inY = pSrc[i+Y];
				const F32 inZ = pSrc[i+Z];
				const F32 inW = pSrc[i+W];
				pDst[i+X] = inX + (pSrc[parent+X] - inX) * blendFactor;
				pDst[i+Y] = inY + (pSrc[parent+Y] - inY) * blendFactor;
				pDst[i+Z] = inZ + (pSrc[parent+Z] - inZ) * blendFactor;
				pDst[i+W] = inW + (pSrc[parent+W] - inW) * blendFactor;
			}
		}
		case 8:
		{
			pDst = (F32 *)pOut[7];
			pSrc = (const F32 *)pIn[7];
			const F32 inX = pSrc[i+X];
			const F32 inY = pSrc[i+Y];
			const F32 inZ = pSrc[i+Z];
			const F32 inW = pSrc[i+W];
			pDst[i+X] = inX + (pSrc[parent+X] - inX) * blendFactor;
			pDst[i+Y] = inY + (pSrc[parent+Y] - inY) * blendFactor;
			pDst[i+Z] = inZ + (pSrc[parent+Z] - inZ) * blendFactor;
			pDst[i+W] = inW + (pSrc[parent+W] - inW) * blendFactor;
		}
		case 7:
		{
			pDst = (F32 *)pOut[6];
			pSrc = (const F32 *)pIn[6];
			const F32 inX = pSrc[i+X];
			const F32 inY = pSrc[i+Y];
			const F32 inZ = pSrc[i+Z];
			const F32 inW = pSrc[i+W];
			pDst[i+X] = inX + (pSrc[parent+X] - inX) * blendFactor;
			pDst[i+Y] = inY + (pSrc[parent+Y] - inY) * blendFactor;
			pDst[i+Z] = inZ + (pSrc[parent+Z] - inZ) * blendFactor;
			pDst[i+W] = inW + (pSrc[parent+W] - inW) * blendFactor;
		}
		case 6:
		{
			pDst = (F32 *)pOut[5];
			pSrc = (const F32 *)pIn[5];
			const F32 inX = pSrc[i+X];
			const F32 inY = pSrc[i+Y];
			const F32 inZ = pSrc[i+Z];
			const F32 inW = pSrc[i+W];
			pDst[i+X] = inX + (pSrc[parent+X] - inX) * blendFactor;
			pDst[i+Y] = inY + (pSrc[parent+Y] - inY) * blendFactor;
			pDst[i+Z] = inZ + (pSrc[parent+Z] - inZ) * blendFactor;
			pDst[i+W] = inW + (pSrc[parent+W] - inW) * blendFactor;
		}
		case 5:
		{
			pDst = (F32 *)pOut[4];
			pSrc = (const F32 *)pIn[4];
			const F32 inX = pSrc[i+X];
			const F32 inY = pSrc[i+Y];
			const F32 inZ = pSrc[i+Z];
			const F32 inW = pSrc[i+W];
			pDst[i+X] = inX + (pSrc[parent+X] - inX) * blendFactor;
			pDst[i+Y] = inY + (pSrc[parent+Y] - inY) * blendFactor;
			pDst[i+Z] = inZ + (pSrc[parent+Z] - inZ) * blendFactor;
			pDst[i+W] = inW + (pSrc[parent+W] - inW) * blendFactor;
		}
		case 4:
		{
			pDst = (F32 *)pOut[3];
			pSrc = (const F32 *)pIn[3];
			const F32 inX = pSrc[i+X];
			const F32 inY = pSrc[i+Y];
			const F32 inZ = pSrc[i+Z];
			const F32 inW = pSrc[i+W];
			pDst[i+X] = inX + (pSrc[parent+X] - inX) * blendFactor;
			pDst[i+Y] = inY + (pSrc[parent+Y] - inY) * blendFactor;
			pDst[i+Z] = inZ + (pSrc[parent+Z] - inZ) * blendFactor;
			pDst[i+W] = inW + (pSrc[parent+W] - inW) * blendFactor;
		}
		case 3:
		{
			pDst = (F32 *)pOut[2];
			pSrc = (const F32 *)pIn[2];
			const F32 inX = pSrc[i+X];
			const F32 inY = pSrc[i+Y];
			const F32 inZ = pSrc[i+Z];
			const F32 inW = pSrc[i+W];
			pDst[i+X] = inX + (pSrc[parent+X] - inX) * blendFactor;
			pDst[i+Y] = inY + (pSrc[parent+Y] - inY) * blendFactor;
			pDst[i+Z] = inZ + (pSrc[parent+Z] - inZ) * blendFactor;
			pDst[i+W] = inW + (pSrc[parent+W] - inW) * blendFactor;
		}
		case 2:
		{
			pDst = (F32 *)pOut[1];
			pSrc = (const F32 *)pIn[1];
			const F32 inX = pSrc[i+X];
			const F32 inY = pSrc[i+Y];
			const F32 inZ = pSrc[i+Z];
			const F32 inW = pSrc[i+W];
			pDst[i+X] = inX + (pSrc[parent+X] - inX) * blendFactor;
			pDst[i+Y] = inY + (pSrc[parent+Y] - inY) * blendFactor;
			pDst[i+Z] = inZ + (pSrc[parent+Z] - inZ) * blendFactor;
			pDst[i+W] = inW + (pSrc[parent+W] - inW) * blendFactor;
		}
		case 1:
		{
			pDst = (F32 *)pOut[0];
			pSrc = (const F32 *)pIn[0];
			const F32 inX = pSrc[i+X];
			const F32 inY = pSrc[i+Y];
			const F32 inZ = pSrc[i+Z];
			const F32 inW = pSrc[i+W];
			pDst[i+X] = inX + (pSrc[parent+X] - inX) * blendFactor;
			pDst[i+Y] = inY + (pSrc[parent+Y] - inY) * blendFactor;
			pDst[i+Z] = inZ + (pSrc[parent+Z] - inZ) * blendFactor;
			pDst[i+W] = inW + (pSrc[parent+W] - inW) * blendFactor;
		}
		break;
		}
	}
}
#endif//__SPU__

void Ice::MeshProc::CmdPerformPm(U32 *pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);

	// If you got here with no uniform tables, then something is wrong.
	ICE_ASSERT(pStaticMem[kStaticUniformCount]);

	U16 *pLodInfo = (U16*)&pStaticMem[kStaticPmVertexLodInfo];
	I32 pmHighestLod = pStaticMem[kStaticPmHighestLod];
	F32 *pContinuousPmInfo = (F32 *)pStaticMem[kStaticContinuousPmInfoPtr];
	F32 *pDiscretePmInfo = (F32 *)pStaticMem[kStaticDiscretePmInfoPtr];
	if (pContinuousPmInfo != NULL)
	{
		I32 pmLowestLod = ((I32 *)pContinuousPmInfo)[0];
		ICE_ALIGN(16) F32 pTransformToDistance[4] = 
			{ pContinuousPmInfo[1],
			  pContinuousPmInfo[2],
			  pContinuousPmInfo[3],
			  0
			};
		ICE_ASSERT(pmLowestLod >= pmHighestLod);
		pContinuousPmInfo += 4 + 2 * pmLowestLod;
		pLodInfo += 4 * pmLowestLod;
		for (I32 lod = pmLowestLod; lod >= pmHighestLod; --lod)
		{
			if(!(U32)pLodInfo[1] || !pStaticMem[kStaticPmParentPtr + lod])
			{
				// Move pointers back to next (higher) LOD.
				pContinuousPmInfo -= 2;
				pLodInfo -= 4;
				continue;
			}

			ICE_ALIGN(16) F32 pLodParams[2] = { pContinuousPmInfo[0], pContinuousPmInfo[1] };

			PmContinuousTower(
				(U16 *)pStaticMem[kStaticPmParentPtr + lod], 
				pStaticMem[kStaticUniformCount],
				(const F32 *)pStaticMem[kStaticUniformPosPtr],
				(const U32 *)&pStaticMem[kStaticUniformPtr],
				(U32 *)&pStaticMem[kStaticUniformPtr],
				&pLodParams[0],
				pTransformToDistance,
				(U32)pLodInfo[0], 
				(U32)pLodInfo[1]);

			// Move pointers back to next (higher) LOD.
			pContinuousPmInfo -= 2;
			pLodInfo -= 4;
		}
	}
	else
	{
		// Discrete PM.  blend between highest LOD and highest+1
		F32 blendFactor = pDiscretePmInfo[0];

		pLodInfo += 4 * pmHighestLod;

		if (blendFactor < 0.0f)
			blendFactor = 0.0f;

		U16 *pParents = (U16 *)pStaticMem[kStaticPmParentPtr];
		if ((pParents != 0) && (blendFactor < 1.0f)) {
			PmDiscreteTower(pParents, pStaticMem[kStaticUniformCount], (U32 *)&pStaticMem[kStaticUniformPtr],
					(U32 *)&pStaticMem[kStaticUniformPtr], blendFactor, (U32)pLodInfo[0], (U32)pLodInfo[1]);
		}
	}
	
	// We may have called PM from the DM routine, so we are not allowed to free the data if this is true.
	if (pStaticMem[kStaticDmInfoPtr] == 0)
		MeshMemFree(pStaticMem, (pDiscretePmInfo == NULL) ? pStaticMem[kStaticContinuousPmInfoPtr] : (U32)pDiscretePmInfo);
}

