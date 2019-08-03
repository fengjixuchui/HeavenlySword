#ifndef _SPEEDTREEUTIL_H_
#define _SPEEDTREEUTIL_H_

//--------------------------------------------------
//!
//!	\file SpeedTreeUtil_ps3.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------
//#define SPEEDTREE_USE_32BIT_INDICES

#define SPEEDTREE_MEMORY_CHUNK	Mem::MC_PROCEDURAL
#define SPEEDGRASS_MEMORY_CHUNK Mem::MC_PROCEDURAL

namespace SpeedTree
{
	const int g_MaxNumLODs = 7;
	const int g_MaxNumStrips = 5;
}


enum SpeedTreeRenderableEnum
{
	SPEEDTREE_BRANCH, 
	SPEEDTREE_FROND,
	SPEEDTREE_LEAF,
	SPEEDTREE_BILLBOARD,
	SPEEDTREE_BILLBOARD2,
	SPEEDTREE_RENDERABLES_END
};

enum SpeedTreeTextureTypeEnum
{
	SPPEDTREE_TEXTURE_DIFFUSE,
	SPEEDTREE_TEXTURE_DETAIL,
	SPPEDTREE_TEXTURE_NORMAL,
	SPEEDTREE_TEXTURE_END
};

enum SpeedTreeShaderPass
{
	SPEEDTREE_PASS_MATERIAL,
	SPEEDTREE_PASS_DEPTH,
	SPEEDTREE_PASS_END
};

typedef enum
{
	Speedtree_RenderLeaves = (1 << SPEEDTREE_LEAF),
	Speedtree_RenderFronds = (1 << SPEEDTREE_FROND),
	Speedtree_RenderBranches = (1 << SPEEDTREE_BRANCH),
	Speedtree_RenderBillboards = (1 << SPEEDTREE_BILLBOARD),
	Speedtree_RenderAll = ((1 << 4) - 1), 
	Speedtree_LodDone = (1 << 5),
} SpeedTreeRendererMask;

///////////////////////////////////////////////////////////////////////  
//  Macros



class CSpeedTreeRT;

class SpeedTreeStat
{
public:
	uint32_t m_leafTriangleCount;
	uint32_t m_BranchTriangleCount;
	uint32_t m_FrondTriangleCount;
	uint32_t m_BillboardTriangleCount;
public:
	void operator += (const SpeedTreeStat& stat);
	//SpeedTreeStat(CSpeedTreeRT* pTree);
	SpeedTreeStat(uint32_t leafTriangleCount,
		uint32_t m_BranchTriangleCount,
		uint32_t FrondTriangleCount,
		uint32_t BillboardTriangleCount);
	SpeedTreeStat();
	uint32_t GetTotal();
}; // end of class SpeedTreeStat

inline unsigned int ComputeCellDimensions(const float (&totalDimensions)[2], unsigned int numObjects, unsigned int numObjectsPerCell, unsigned int (&numCells)[2])
{
	//unsigned int maxObjectsPerCell = numObjects < numObjectsPerCell ? numObjects : numObjectsPerCell;
	unsigned int maxObjectsPerCell = numObjectsPerCell;

	/*
	unsigned int numCellsTotal = numObjects / maxObjectsPerCell;

	numCells[1] = (unsigned int)fsqrtf(((float)maxObjectsPerCell / (float)numObjects) * float(int(totalDimensions[1] / totalDimensions[0])));
	if (0 == numCells[1]) numCells[1] = 1;
	numCells[0]  = (numObjects / maxObjectsPerCell) / numCells[1];


	ntAssert(numCells[0] * numCells[1] == numCellsTotal); */

	numCells[0] = (unsigned int)fsqrtf((float)numObjects / (float)maxObjectsPerCell ) + 1;
	numCells[1] = numCells[0];
	unsigned int numCellsTotal = numCells[0] * numCells[1];

	return numCellsTotal;

}

inline float DegToRad(float fDegrees)
{
    return fDegrees / 57.2957795f;
}

#endif // end of _SPEEDTREEUTIL_H_
