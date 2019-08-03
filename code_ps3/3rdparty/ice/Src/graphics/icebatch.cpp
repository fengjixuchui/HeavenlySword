/*
 * Copyright (c) 2005 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 * 
 * Revision History:
 *  - Created 8/23/05 
 */
 
#include "icegraphics.h"
#include "icemesh.h"
#include "icelights.h"
#include "iceobjectbatch.h"
#include "icetextures.h"
#include "icedebugdraw.h"
#include "icequicktimer.h"
#include "icebatch.h"
 
namespace Ice
{
	namespace Bucketer
	{
		ObjectBatchCollection *g_firstCollection = NULL;
	}
}

using namespace Ice;
using namespace Ice::Bucketer;
using namespace Ice::Graphics;
using namespace Ice::Mesh;
using namespace Ice::MeshProc;
using namespace Ice::Render;
using namespace SMath;

// Render any batched collections
void Ice::Bucketer::RenderBatches(const U64 variation)
{
	g_bucketedViewportInfo = (ViewportInfo *)(g_meshDataBuf[g_meshDataDblBuf] + g_meshDataBufSize);
	g_meshDataBufSize += (sizeof(*g_bucketedViewportInfo) + 0xF) & ~0xF;
	*g_bucketedViewportInfo = g_viewportInfo;

	// Render the main geometry.
	const ObjectBatchCollection *collection = g_firstCollection;
	for(; collection; collection=collection->m_next)
	{
		g_dmInfo = (DmInfo *)(g_meshDataBuf[g_meshDataDblBuf] + g_meshDataBufSize);
		g_meshDataBufSize += (sizeof(*g_dmInfo) + 0xF) & ~0xF;
		g_dmInfo->m_alpha = 1.0f - collection->m_lodFraction;
		g_dmInfo->m_phase = 0;
		g_dmInfo->m_IterationCount = 0;
		
		RenderCollections(collection, variation);
	}
}

// Render only the batches that exist inside the specified sphere
void Ice::Bucketer::RenderBatchesInSphere(const U64 variation, const Vec4 &sphere)
{
	g_bucketedViewportInfo = (ViewportInfo *)(g_meshDataBuf[g_meshDataDblBuf] + g_meshDataBufSize);
	g_meshDataBufSize += (sizeof(*g_bucketedViewportInfo) + 0xF) & ~0xF;
	*g_bucketedViewportInfo = g_viewportInfo;

	// Render the main geometry.
	const ObjectBatchCollection *collection = g_firstCollection;
	for(; collection; collection=collection->m_next)
	{
		g_dmInfo = (DmInfo *)(g_meshDataBuf[g_meshDataDblBuf] + g_meshDataBufSize);
		g_meshDataBufSize += (sizeof(*g_dmInfo) + 0xF) & ~0xF;
		g_dmInfo->m_alpha = 1.0f - collection->m_lodFraction;
		g_dmInfo->m_phase = 0;
		g_dmInfo->m_IterationCount = 0;

		RenderCollectionsInSphere(collection, variation, sphere);
	}
}

// Clear any batched collections
void Ice::Bucketer::ClearBatches()
{
	ObjectBatchMgr::FreeAll();
	g_firstCollection = NULL;
}

// Batch A collection of vertex sets that belong to this renderable
static void BatchVertexSets(
	DiscretePmRenderable const *__restrict renderable, 
	bool const isPm, 
	bool const isDm, 
	U32 const objectLod, 
	F32 const distance, 
	U32 const groupLod, 
	F32 const fraction,
	SMath::Transform *__restrict transforms, 
	bool const isRigid)
{
	// Grab the next available collection
	ObjectBatchCollection *__restrict collection = ObjectBatchMgr::Alloc();
	MaterialInstance *__restrict matInst = renderable->m_materialInstance;
	collection->m_materialInstance = matInst;
	collection->m_firstBatch = NULL;
	collection->m_transforms = transforms;
	collection->m_isRigid = isRigid;
	collection->m_lodFraction = fraction; // Ignored if not PMing
	collection->m_isCpm = false;
	collection->m_techniqueTableRow = (matInst->m_flags & kMaterialInstanceShaderLod) ? objectLod : (isDm ? 1 : 0);
	collection->m_next = g_firstCollection;
	g_firstCollection = collection;

	// Calculate the values of any fade factors
	const FadeFactorCoeffs * __restrict__ coeffs = renderable->m_fadeFactorCoeffs;
	F32 * __restrict__ fadeFactors = collection->m_fadeFactors;
	const U64 fadeFactorCount = renderable->m_fadeFactorCount;
	switch(fadeFactorCount)
	{
	case 4: fadeFactors[3] = Min(Max(coeffs[3].m_coeffs[0] * distance - coeffs[3].m_coeffs[1], 0.f), 1.f);
	case 3: fadeFactors[2] = Min(Max(coeffs[2].m_coeffs[0] * distance - coeffs[2].m_coeffs[1], 0.f), 1.f);
	case 2: fadeFactors[1] = Min(Max(coeffs[1].m_coeffs[0] * distance - coeffs[1].m_coeffs[1], 0.f), 1.f);
	case 1: fadeFactors[0] = Min(Max(coeffs[0].m_coeffs[0] * distance - coeffs[0].m_coeffs[1], 0.f), 1.f);
	}
	switch(fadeFactorCount)
	{
	case 0: fadeFactors[0] = 0;
	case 1: fadeFactors[1] = 0;
	case 2: fadeFactors[2] = 0;
	case 3: fadeFactors[3] = 0;
	}

	// Batch the vertex sets
	U64 const count = renderable->m_vertexSetCount;
	for (U64 i = 0; i < count; ++i)
	{
		PmVertexSet const *vertexSet = renderable->m_vertexSets[i];
		VertexSetBlendShapes const *vertexSetBlendShapes = renderable->m_vertexSetBlendShapes ? renderable->m_vertexSetBlendShapes[i] : NULL;
		IndexTable const *indexTable = &vertexSet->m_lodIndexTables[groupLod];
		if(indexTable->m_triangleCount > 0)
		{
			ObjectBatch *batch = ObjectBatchMgr::AllocBatch();

			// Fill it in
			batch->m_isTransitionTechnique = isDm ? 0 : isPm; // Never use transition technique if DM
			batch->m_vertexSet = vertexSet;
			batch->m_vertexSetBlendShapes = vertexSetBlendShapes;
			batch->m_lodIndex = groupLod;

			// Link it in.
			batch->m_next = collection->m_firstBatch;
			collection->m_firstBatch = batch;
		}
	}
}

static U64 FindLodGroupIndex(DiscretePmObject const *__restrict object, F32 const distance)
{
	U64 const numLods = object->m_lodGroupCount - 1;
	DiscretePmLodGroup const * const __restrict * const __restrict groups = object->m_lodGroups;
	LodDistances const *__restrict distances = object->m_lodDistances;
	for (U64 i = 0; i < numLods; ++i)
	{
		U64 const lod = groups[i]->m_endLod - 1;
		if (distance < distances[lod].m_far)
			return i;
	}
	return numLods;
}

// Find the LOD within the LOD group
static U64 FindLodInGroup(DiscretePmObject const *__restrict object, F32 const distance, DiscretePmLodGroup const *__restrict group)
{
	// lastLodInGroup is the last _blending_ LOD in the LOD group. It does not include the
	// last LOD in the object - if this is the last LOD group then lastLodInGroup will point
	// to the LOD before the last (since the last LOD is not involved in PM blending).
	LodDistances const *__restrict distances = object->m_lodDistances + group->m_startLod;
	U64 const numLods = group->m_endLod - 1 - group->m_startLod;
	U64 i = 0;
	while ((i < numLods) && (distance >= distances->m_far))
	{
		i++;
		distances++;
	}
	return i;
}

static F32 FindLodFraction(DiscretePmObject const *__restrict object, F32 const distance, DiscretePmLodGroup const *__restrict group, U64 const groupLod)
{
	// At this point groupLod points to the _blending_ LOD that the object belongs to. If it
	// is actually in the last LOD groupLod will still point to the previous LOD, this means
	// that it is "beyond the end" of the LOD and when we compute the LOD fraction then we will
	// get a value >= 1.0 (this should be the only case where this happens).
	LodDistances const *__restrict distances = object->m_lodDistances + group->m_startLod + groupLod;
	F32 fraction = (distance - distances->m_near) / (distances->m_far - distances->m_near);
	return fraction;
}

// If the group is a displacement mapping (DM) LOD group. The LOD fraction should never be >= 1.0
// (since that can only happen in the last LOD and should not be a DM LOD group).  It could be < 0.0
// if we are in a LOD 'flat spot' (for DM that just means that we are at maximum extrusion, all we need
// to do in this case is to clamp to 0.0).  If the group is a regular DPM (discrete progressive mesh)
// LOD group, the LOD fraction is >= 1.0 when we are actually in the final LOD (which doesn't have blending),
// and we need to bump the LOD index up by one.  It is < 0.0 when we are in a LOD 'flat spot'.  In either
// case we don't use a Pm technique, otherwise we do.
void Ice::Bucketer::BatchDiscretePmObjects(
	DiscretePmObject const *__restrict object,
	U64 const numObjects,
	SMath::Transform **__restrict transformsList,
	F32 const *__restrict distances, 
	U64 *__restrict partEnableBits)
{
	bool const isRigid = (object->m_transformCount == 1);

	if(object->m_lodCount > 1)
	{
		// Organize the objects
		U64 static const kMaxGroups = 8;
		U64 static const kMaxObjects = 128;
		U64 static sortedOId[kMaxGroups][kMaxObjects];
		U64 static sortedGLod[kMaxGroups][kMaxObjects];
		F32 static sortedFracs[kMaxGroups][kMaxObjects];
		U64 static numSorted[kMaxGroups];
		U64 const numGroups = object->m_lodGroupCount;
		ICE_ASSERT(numGroups < kMaxGroups);
		memset(numSorted, 0, sizeof(numSorted));
		for(U64 objectNum = 0; objectNum < numObjects; ++objectNum)
		{
			F32 const distance = distances[objectNum];
			U64 groupIdx = FindLodGroupIndex(object, distance);
			DiscretePmLodGroup const *__restrict group = object->m_lodGroups[groupIdx];
			U64 groupLod = FindLodInGroup(object, distance, group);
			F32 fraction = FindLodFraction(object, distance, group, groupLod);
			bool const isDm = group->m_flags & kDiscretePmLodGroupDisplacementMapping;
			ICE_ASSERT(!isDm || (fraction < 1.f));
			fraction = isDm ? Max(fraction, 0.f) : fraction;
			groupLod += (fraction >= 1.f) ? 1 : 0;

			U64 idx = numSorted[groupIdx]++;
			sortedOId[groupIdx][idx] = objectNum;
			sortedGLod[groupIdx][idx] = groupLod;
			sortedFracs[groupIdx][idx] = fraction;
		}
		
		// Batch em
		for(U64 groupIdx = 0; groupIdx < numGroups; ++groupIdx)
		{
			U64 const numSort = numSorted[groupIdx];
			if(numSort > 0)
			{
				DiscretePmLodGroup const *__restrict group = object->m_lodGroups[groupIdx];
				bool const isDm = group->m_flags & kDiscretePmLodGroupDisplacementMapping;
				U64 const groupStartLod = group->m_startLod;
				
				if (group->m_flags & kDiscretePmLodGroupHasParts) 
				{
					U64 const partCount = group->m_count;
					
					U64 partEnableMask = 0x1;
					for (U64 i = 0; i < partCount; ++i)
					{
						DiscretePmPart *__restrict part = group->m_parts[i];
						if (part && (*partEnableBits & partEnableMask)) 
						{
							U64 const count = part->m_renderableCount;
							for (U64 j = 0; j < count; ++j)
							{
								for(U64 sortIdx = 0; sortIdx < numSort; ++sortIdx)
								{
									U64 const objectNum = sortedOId[groupIdx][sortIdx];
									U64 const groupLod = sortedGLod[groupIdx][sortIdx];
									F32 const fraction = sortedFracs[groupIdx][sortIdx];
									F32 const distance = distances[objectNum];
									U64 const objectLod = groupStartLod + groupLod;
									bool const isPm = (fraction >= 1.f) ? false : (fraction >= 0.f);
									SMath::Transform *__restrict transforms = transformsList[objectNum];
									BatchVertexSets(part->m_renderables[j], isPm, isDm, objectLod, distance, groupLod, fraction, transforms, isRigid);
								}
							}
						}
						partEnableMask <<= 1;
						if (partEnableMask == 0) 
						{
							++partEnableBits;
							partEnableMask = 0x1;
						}
					} 
				}
				else 
				{
					U64 const renderableCount = group->m_count;
					for (U64 i = 0; i < renderableCount; ++i)
					{
						for(U64 sortIdx = 0; sortIdx < numSort; ++sortIdx)
						{
							U64 const objectNum = sortedOId[groupIdx][sortIdx];
							U64 const groupLod = sortedGLod[groupIdx][sortIdx];
							F32 const fraction = sortedFracs[groupIdx][sortIdx];
							F32 const distance = distances[objectNum];
							U64 const objectLod = groupStartLod + groupLod;
							bool const isPm = (fraction >= 1.f) ? false : (fraction >= 0.f);
							SMath::Transform *__restrict transforms = transformsList[objectNum];
							BatchVertexSets(group->m_renderables[i], isPm, isDm, objectLod, distance, groupLod, fraction, transforms, isRigid);
						}
					}
				}
			}
		}
	}
	else
	{
		DiscretePmLodGroup const * __restrict__ group = object->m_lodGroups[0];
		if (group->m_flags & kDiscretePmLodGroupHasParts) 
		{
			U64 const partCount = group->m_count;
			U64 partEnableMask = 0x1;
			for (U64 i = 0; i < partCount; ++i)
			{
				DiscretePmPart * __restrict__ part = group->m_parts[i];
				if (part && (*partEnableBits & partEnableMask)) 
				{
					U64 const count = part->m_renderableCount;
					for (U64 j = 0; j < count; ++j)
					{
						for(U64 objectNum = 0; objectNum < numObjects; ++objectNum)
						{
							SMath::Transform *__restrict transforms = transformsList[objectNum];
							BatchVertexSets(part->m_renderables[j], false, false, 0, 0, 0, 0, transforms, isRigid);
						}
					}
				}
				partEnableMask <<= 1;
				if (partEnableMask == 0) 
				{
					++partEnableBits;
					partEnableMask = 0x1;
				}
			}
		} 
		else 
		{
			U64 const renderableCount = group->m_count;
			for (U64 i = 0; i < renderableCount; ++i) 
			{
				for(U64 objectNum = 0; objectNum < numObjects; ++objectNum)
				{
					SMath::Transform *__restrict transforms = transformsList[objectNum];
					BatchVertexSets(group->m_renderables[i], false, false, 0, 0, 0, 0, transforms, isRigid);
				}
			}
		}
	}
}

// If the group is a displacement mapping (DM) LOD group. The LOD fraction should never be >= 1.0
// (since that can only happen in the last LOD and should not be a DM LOD group).  It could be < 0.0
// if we are in a LOD 'flat spot' (for DM that just means that we are at maximum extrusion, all we need
// to do in this case is to clamp to 0.0).  If the group is a regular DPM (discrete progressive mesh)
// LOD group, the LOD fraction is >= 1.0 when we are actually in the final LOD (which doesn't have blending),
// and we need to bump the LOD index up by one.  It is < 0.0 when we are in a LOD 'flat spot'.  In either
// case we don't use a Pm technique, otherwise we do.
void Ice::Bucketer::BatchDiscretePmObjects(
	DiscretePmObject const *__restrict object,
	U64 const numObjects,
	SMath::Transform **__restrict transformsList,
	F32 const *distances)
{
	bool const isRigid = (object->m_transformCount == 1);

	if(object->m_lodCount > 1)
	{
		// Organize the objects
		U64 static const kMaxGroups = 8;
		U64 static const kMaxObjects = 128;
		U64 static sortedOId[kMaxGroups][kMaxObjects];
		U64 static sortedGLod[kMaxGroups][kMaxObjects];
		F32 static sortedFracs[kMaxGroups][kMaxObjects];
		U64 static numSorted[kMaxGroups];
		U64 const numGroups = object->m_lodGroupCount;
		ICE_ASSERT(numGroups < kMaxGroups);
		memset(numSorted, 0, sizeof(numSorted));
		for(U64 objectNum = 0; objectNum < numObjects; ++objectNum)
		{
			F32 const distance = distances[objectNum];
			U64 groupIdx = FindLodGroupIndex(object, distance);
			DiscretePmLodGroup const *__restrict group = object->m_lodGroups[groupIdx];
			U64 groupLod = FindLodInGroup(object, distance, group);
			F32 fraction = FindLodFraction(object, distance, group, groupLod);
			bool const isDm = group->m_flags & kDiscretePmLodGroupDisplacementMapping;
			ICE_ASSERT(!isDm || (fraction < 1.f));
			fraction = isDm ? Max(fraction, 0.f) : fraction;
			groupLod += (fraction >= 1.f) ? 1 : 0;

			U64 idx = numSorted[groupIdx]++;
			sortedOId[groupIdx][idx] = objectNum;
			sortedGLod[groupIdx][idx] = groupLod;
			sortedFracs[groupIdx][idx] = fraction;
		}
		
		// Batch em
		for(U64 groupIdx = 0; groupIdx < numGroups; ++groupIdx)
		{
			U64 const numSort = numSorted[groupIdx];
			if(numSort > 0)
			{
				DiscretePmLodGroup const *__restrict group = object->m_lodGroups[groupIdx];
				bool const isDm = group->m_flags & kDiscretePmLodGroupDisplacementMapping;
				U64 const groupStartLod = group->m_startLod;
				
				if (group->m_flags & kDiscretePmLodGroupHasParts) 
				{
					U64 const partCount = group->m_count;
					for (U64 i = 0; i < partCount; ++i)
					{
						DiscretePmPart *__restrict part = group->m_parts[i];
						if (part == NULL) 
							continue;
							
						U64 const count = part->m_renderableCount;
						for (U64 j = 0; j < count; ++j)
						{
							for(U64 sortIdx = 0; sortIdx < numSort; ++sortIdx)
							{
								U64 const objectNum = sortedOId[groupIdx][sortIdx];
								U64 const groupLod = sortedGLod[groupIdx][sortIdx];
								F32 const fraction = sortedFracs[groupIdx][sortIdx];
								F32 const distance = distances[objectNum];
								U64 const objectLod = groupStartLod + groupLod;
								bool const isPm = (fraction >= 1.f) ? false : (fraction >= 0.f);
								SMath::Transform *__restrict transforms = transformsList[objectNum];
								BatchVertexSets(part->m_renderables[j], isPm, isDm, objectLod, distance, groupLod, fraction, transforms, isRigid);
							}
						}
					} 
				}
				else 
				{
					U64 const renderableCount = group->m_count;
					for (U64 i = 0; i < renderableCount; ++i)
					{
						for(U64 sortIdx = 0; sortIdx < numSort; ++sortIdx)
						{
							U64 const objectNum = sortedOId[groupIdx][sortIdx];
							U64 const groupLod = sortedGLod[groupIdx][sortIdx];
							F32 const fraction = sortedFracs[groupIdx][sortIdx];
							F32 const distance = distances[objectNum];
							U64 const objectLod = groupStartLod + groupLod;
							bool const isPm = (fraction >= 1.f) ? false : (fraction >= 0.f);
							SMath::Transform *__restrict transforms = transformsList[objectNum];
							BatchVertexSets(group->m_renderables[i], isPm, isDm, objectLod, distance, groupLod, fraction, transforms, isRigid);
						}
					}
				}
			}
		}
	}
	else
	{
		DiscretePmLodGroup const * __restrict__ group = object->m_lodGroups[0];
		if (group->m_flags & kDiscretePmLodGroupHasParts) 
		{
			U64 const partCount = group->m_count;
			for (U64 i = 0; i < partCount; ++i)
			{
				DiscretePmPart * __restrict__ part = group->m_parts[i];
				if (part == NULL) 
					continue;
					
				U64 const count = part->m_renderableCount;
				for (U64 j = 0; j < count; ++j)
				{
					for(U64 objectNum = 0; objectNum < numObjects; ++objectNum)
					{
						SMath::Transform *__restrict transforms = transformsList[objectNum];
						BatchVertexSets(part->m_renderables[j], false, false, 0, 0, 0, 0, transforms, isRigid);
					}
				}
			}
		} 
		else 
		{
			U64 const renderableCount = group->m_count;
			for (U64 i = 0; i < renderableCount; ++i) 
			{
				for(U64 objectNum = 0; objectNum < numObjects; ++objectNum)
				{
					SMath::Transform *__restrict transforms = transformsList[objectNum];
					BatchVertexSets(group->m_renderables[i], false, false, 0, 0, 0, 0, transforms, isRigid);
				}
			}
		}
	}
}

// If the group is a displacement mapping (DM) LOD group. The LOD fraction should never be >= 1.0
// (since that can only happen in the last LOD and should not be a DM LOD group).  It could be < 0.0
// if we are in a LOD 'flat spot' (for DM that just means that we are at maximum extrusion, all we need
// to do in this case is to clamp to 0.0).  If the group is a regular DPM (discrete progressive mesh)
// LOD group, the LOD fraction is >= 1.0 when we are actually in the final LOD (which doesn't have blending),
// and we need to bump the LOD index up by one.  It is < 0.0 when we are in a LOD 'flat spot'.  In either
// case we don't use a Pm technique, otherwise we do.
void Ice::Bucketer::BatchDiscretePmObject(
	DiscretePmObject const *__restrict object,
	SMath::Transform *__restrict transforms,
	F32 const distance)
{
	bool const isRigid = (object->m_transformCount == 1);

	if(object->m_lodCount > 1)
	{
		U64 groupIdx = FindLodGroupIndex(object, distance);
		DiscretePmLodGroup const *__restrict group = object->m_lodGroups[groupIdx];
		U64 const groupStartLod = group->m_startLod;
		U64 groupLod = FindLodInGroup(object, distance, group);
		F32 fraction = FindLodFraction(object, distance, group, groupLod);
		bool const isDm = group->m_flags & kDiscretePmLodGroupDisplacementMapping;
		ICE_ASSERT(!isDm || (fraction < 1.f));
		fraction = isDm ? Max(fraction, 0.f) : fraction;
		groupLod += (fraction >= 1.f) ? 1 : 0;
		U64 const objectLod = groupStartLod + groupLod;
		bool const isPm = (fraction >= 1.f) ? false : (fraction >= 0.f);

		// Batch em
		if (group->m_flags & kDiscretePmLodGroupHasParts) 
		{
			U64 const partCount = group->m_count;
			for (U64 i = 0; i < partCount; ++i)
			{
				DiscretePmPart *__restrict part = group->m_parts[i];
				if (part == NULL) 
					continue;
					
				U64 const count = part->m_renderableCount;
				for (U64 j = 0; j < count; ++j)
				{
					BatchVertexSets(part->m_renderables[j], isPm, isDm, objectLod, distance, groupLod, fraction, transforms, isRigid);
				}
			} 
		}
		else 
		{
			U64 const renderableCount = group->m_count;
			for (U64 i = 0; i < renderableCount; ++i)
			{
				BatchVertexSets(group->m_renderables[i], isPm, isDm, objectLod, distance, groupLod, fraction, transforms, isRigid);
			}
		}
	}
	else
	{
		DiscretePmLodGroup const * __restrict__ group = object->m_lodGroups[0];
		if (group->m_flags & kDiscretePmLodGroupHasParts) 
		{
			U64 const partCount = group->m_count;
			for (U64 i = 0; i < partCount; ++i)
			{
				DiscretePmPart * __restrict__ part = group->m_parts[i];
				if (part == NULL) 
					continue;
					
				U64 const count = part->m_renderableCount;
				for (U64 j = 0; j < count; ++j)
				{
					BatchVertexSets(part->m_renderables[j], false, false, 0, 0, 0, 0, transforms, isRigid);
				}
			}
		} 
		else 
		{
			U64 const renderableCount = group->m_count;
			for (U64 i = 0; i < renderableCount; ++i) 
			{
				BatchVertexSets(group->m_renderables[i], false, false, 0, 0, 0, 0, transforms, isRigid);
			}
		}
	}
}

// The size in elements of the Continuous Progressive Mesh rendering LIFO.
#define CPM_LIFO_SIZE 1024

// Batch the vertex sets that belong to a Continuous Progressive Mesh
static void BatchContinuousPmTree(
	const ContinuousPmObject * __restrict__ object, 
	const ContinuousPmRenderable * __restrict__ renderable)
{
	MaterialInstance * __restrict__ materialInst = renderable->m_materialInstance;

	// Grab the next available collection
	ObjectBatchCollection * __restrict__ collection = ObjectBatchMgr::Alloc();
	collection->m_materialInstance = materialInst;
	collection->m_firstBatch = NULL;
	collection->m_transforms = NULL;
	collection->m_isCpm = true;

	// CPM can never have shader LOD, it always uses row 0 and no fade factors
	collection->m_techniqueTableRow = 0;
	collection->m_fadeFactors[0] = collection->m_fadeFactors[1] = collection->m_fadeFactors[2] = collection->m_fadeFactors[3] = 0.0f;

	// Initialize the LIFO
	static struct 
	{
		const ContinuousPmNode *node;
		U32 lodGroup;
	} nodeLifo[CPM_LIFO_SIZE] = {NULL, 0};
	U64 numNodes = renderable->m_continuousPmTreeCount;
	const U32 numLodGroups = object->m_lodGroupCount;
	const U32 lastLodGroup = numLodGroups-1;
	const U64 treeCount = renderable->m_continuousPmTreeCount;
	
	// Fill the initial elements of the LIFO with the root
	// nodes.
	for(U64 i = 0; i < treeCount; ++i)
	{
		nodeLifo[i].node = renderable->m_continuousPmTrees[i];
		nodeLifo[i].lodGroup = lastLodGroup;
	}
	
	const Point &eyeVec = g_cameraWorldPosition;
	while(numNodes > 0)
	{
		--numNodes;
		const ContinuousPmNode * __restrict__ node = nodeLifo[numNodes].node;
		const Vec4 & __restrict__ sphere = node->m_boundingSphere;
		const F32 sphereW = (F32)sphere.W();
		const F32 dist = Dist(Point(sphere), eyeVec);
		const F32 farDist = dist + sphereW;

		// Cull nodes that are behind the camera
		if(farDist < 0)
			continue;
		
		// Frustum check
		Scalar const leftDist = Dot3(sphere, g_frustumPlaneLeft) + g_frustumPlaneLeft.W();
		if(leftDist < -sphereW)
			continue;
		
		Scalar const rightDist = Dot3(sphere, g_frustumPlaneRight) + g_frustumPlaneRight.W();
		if(rightDist < -sphereW)
			continue;

		Scalar const topDist = Dot3(sphere, g_frustumPlaneTop) + g_frustumPlaneTop.W();
		if(topDist < -sphereW)
			continue;
			
		Scalar const bottomDist = Dot3(sphere, g_frustumPlaneBottom) + g_frustumPlaneBottom.W();
		if(bottomDist < -sphereW)
			continue;
			
		const U64 numChildren = node->m_childCount;
		const U32 lodGroup = nodeLifo[numNodes].lodGroup;
		const U32 lodGroupStartLod = object->m_lodGroupStartLods[lodGroup];
		const F32 nearDist = dist - sphereW;
		ICE_ASSERT(lodGroup < numLodGroups);
		ICE_ASSERT(lodGroupStartLod < object->m_lodCount);

		// The order of evaluation matters in this if statement, since if there are no children
		// then lodGroupStartLod - 1 will be negative.
		if((numChildren > 0) && (nearDist < object->m_lodDistances[lodGroupStartLod - 1].m_far))
		{
			// Add this node's children to the LIFO.
			const U32 nextGroup = lodGroup-1;
			ContinuousPmNode ** __restrict__ children = node->m_childNodes;
			const U64 numLoop = numNodes + numChildren;
			for(U64 ii = numNodes; ii < numLoop; ++ii, ++children)
			{
				nodeLifo[ii].node = *children;
				nodeLifo[ii].lodGroup = nextGroup;
			}
			numNodes += numChildren;
			ICE_ASSERT(numNodes < CPM_LIFO_SIZE);
		}
		else
		{
			// Compute nearLodInGroup and farLodInGroup such that only the LODs which obey
			// nearLodInGroup <= lod < farLodInGroup are needed for PM.
			LodDistances * __restrict__ lodDistances = object->m_lodDistances + lodGroupStartLod;
			const U64 lastLodInGroup = (object->m_lodCount - 1) - lodGroupStartLod;
			U64 nearLodInGroup = 0;
			for (; nearLodInGroup < lastLodInGroup; ++nearLodInGroup)
				if (nearDist < lodDistances[nearLodInGroup].m_far) 
					break;
			U64 farLodInGroup = nearLodInGroup;
			for (; farLodInGroup < lastLodInGroup; ++farLodInGroup) 
				if (farDist < lodDistances[farLodInGroup].m_near) 
					break;

			// Batch the vertex set
			ObjectBatch * __restrict__ batch = ObjectBatchMgr::AllocBatch();

			// Fill it in
			batch->m_vertexSet = node->m_vertexSet;
			batch->m_vertexSetBlendShapes = NULL;
			batch->m_lodIndex = nearLodInGroup;

			// If nearLodInGroup == farLodInGroup, no PM blending is performed 
			// as it would have no effect if it was infact performed anyway.
			if (nearLodInGroup == farLodInGroup) 
			{
				batch->m_isTransitionTechnique = 0;
			}
			else 
			{
				batch->m_isTransitionTechnique = 1;
				batch->m_highestLodToBlend = farLodInGroup - 1;
				batch->m_lodDistances = lodDistances;
			}

			// Link it in.
			batch->m_next = collection->m_firstBatch;
			collection->m_firstBatch = batch;
		}
	}

	// Link it in
	collection->m_next = g_firstCollection;
	g_firstCollection = collection;
}

// Batch a Continuous Progressive Meshed Object
void Ice::Bucketer::BatchContinuousPmObject(const Ice::Graphics::ContinuousPmObject *__restrict object)
{
	const U64 numRenderables = object->m_renderableCount;
	for(U64 i = 0; i < numRenderables; ++i)
	{
		BatchContinuousPmTree(object, object->m_renderables[i]);
	}
}
