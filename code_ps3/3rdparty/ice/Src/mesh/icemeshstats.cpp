/*
 * Copyright (c) 2005 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 * 
 */

#include "icemesh.h"
#include "icemeshinternal.h"

// This is a place to collect some basic stats about mesh processing across more than one call to Ice::MeshProcessing().
// Currently only stats on outputs to the big buffer are tracked, but it wouldn't be hard to add more.
// Also, since no values can be kept across calls to Mesh Processing on the SPU, this feature only works if run on the PPU.
#ifndef __SPU__
using namespace Ice;
using namespace Ice::MeshProc;

namespace Ice
{
	namespace MeshProc
	{
		// One global structure to collect the stats.
		MeshProcessingStats g_meshProcessingStats =
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	}
}

/// Resets all of the Mesh Processing stats.
void Ice::MeshProc::ClearMeshProcessingStats()
{
	memset(&g_meshProcessingStats, 0, sizeof(MeshProcessingStats));
}

/// Prints out the Mesh Processing stats in a nice format.
void Ice::MeshProc::PrintMeshProcessingStats()
{
	printf("---------------------------------------\n");
	printf("Profile Edges: %7d (%8X bytes)\n", g_meshProcessingStats.m_profileEdgeNum, g_meshProcessingStats.m_profileEdgeSize);
	printf("Cap Indexes:   %7d (%8X bytes)\n", g_meshProcessingStats.m_capIndexesNum, g_meshProcessingStats.m_capIndexesSize);
	printf("Indexes:       %7d (%8X bytes)\n", g_meshProcessingStats.m_indexesNum, g_meshProcessingStats.m_indexesSize);
	printf("Vertexes:      %7d (%8X bytes)\n", g_meshProcessingStats.m_vertexesNum, g_meshProcessingStats.m_vertexesSize);
	printf("Caps:          %7d (%8X bytes)\n", g_meshProcessingStats.m_capsNum, g_meshProcessingStats.m_capsSize);
	printf("Hole size:             (%8X bytes)\n", g_meshProcessingStats.m_holeSize);
}

#endif

