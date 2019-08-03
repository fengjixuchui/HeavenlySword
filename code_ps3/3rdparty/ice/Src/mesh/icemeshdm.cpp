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

static void DmLoft( F32* pSrc, F32* pDst, F32* pNormalTable, F32* pDisplacementTable, F32 alpha, U16 vertCount )
{
	//PRINT_U32(vertCount);
	for( U32 i = 0; i < vertCount; ++i )
	{
		//PRINT_U32(i);
		F32 disp = pDisplacementTable[i] * alpha;
		//PRINT_F32(disp);
		pDst[i*4+X] = pSrc[i*4+X] + pNormalTable[i*4+X] * disp;
		pDst[i*4+Y] = pSrc[i*4+Y] + pNormalTable[i*4+Y] * disp;
		pDst[i*4+Z] = pSrc[i*4+Z] + pNormalTable[i*4+Z] * disp;
	}
}

void Ice::MeshProc::CmdPerformDm(U32 *pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);
	
	//PRINTF("CmdPerformDm\n");

	F32* discreteDmInfo = (F32 *)pStaticMem[kStaticDmInfoPtr];

	// Based on the value of alpha (the LOD distance), we want to
	// either slide (0.0-0.5) or extrude (0.5-1.0), after rescaling
	// the alpha to 0.0-1.0
	F32 dmAlpha = *discreteDmInfo;
	ICE_ASSERT(dmAlpha >= 0.0f && dmAlpha <= 1.0f);
	
	// An alpha of 0.0 to 0.5 means we're in the sliding phase of DM,
	// which is currently handled by the PM code.
	if (dmAlpha <= 0.5f)
	{
		// re-route to PM after updating the alpha value in the
		// PM info struct
		F32 *pPmInfo = (F32 *)pStaticMem[kStaticDiscretePmInfoPtr];
		pPmInfo[0] = dmAlpha * 2.0f;
		CmdPerformPm(pStaticMem, cmdQuad1, cmdQuad2);
	}
	else
	{
		// Otherwise we're in the DM extrusion phase.  Rescale alpha to
		// 0.0 to 1.0 before continuing.
		dmAlpha = (dmAlpha - 0.5f) * 2.0f;

		ICE_ASSERT(discreteDmInfo != NULL);
		ICE_ASSERT(pStaticMem[kStaticUniformPosPtr] != NULL);
		ICE_ASSERT(pStaticMem[kStaticUniformDnormPtr] != NULL);
		ICE_ASSERT(pStaticMem[kStaticDmDisplacementPtr] != NULL);

		DmLoft((F32 *)(pStaticMem[kStaticUniformPosPtr]),
				(F32 *)pStaticMem[kStaticUniformPosPtr],
				(F32 *)pStaticMem[kStaticUniformDnormPtr],
				(F32 *)pStaticMem[kStaticDmDisplacementPtr],
				dmAlpha,
				pStaticMem[kStaticVertexCount]);
	}

	// Free the DM data, but only if skinning is not performed.
	// NOTE: When DM is changed from fake to real, remember to change the free is CmdPerformSkinning()
	if (pStaticMem[kStaticSkinControlPtr] == 0)
		MeshMemFree(pStaticMem, pStaticMem[kStaticDiscretePmInfoPtr]);
}

