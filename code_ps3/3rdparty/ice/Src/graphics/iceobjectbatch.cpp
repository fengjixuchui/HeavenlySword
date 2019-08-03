/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 * 
 * Revision History:
 *  - Created 7/28/05
 */
 
#include "jobapi/joblist.h"
#include "jobapi/spumodule.h"
#include "icegraphics.h"
#include "icefx.h"
#include "icemesh.h"
#include "icelights.h"
#include "iceobjectbatch.h"
#include "iceeffects.h"
#include "icetextures.h"
#include "icedebugdraw.h"
#include "icequicktimer.h"
#include "icetimingbar.h"
#include "icemeshstructs.h"

using namespace Ice;
using namespace Ice::Bucketer;
using namespace Ice::Graphics;
using namespace Ice::Render;
using namespace Ice::Mesh;
using namespace Ice::MeshProc;
using namespace SMath;

namespace Ice
{
	namespace Bucketer
	{
		namespace ObjectBatchMgr
		{
			U32 g_numCollections = 0;
			U32 g_numBatches = 0;
			ObjectBatchCollection g_collections[kMaxCollections];
			ObjectBatch g_batches[kMaxBatches];
		}
		
		I16 g_blendShapeFactor[kNumBlendShapes];

		// Needed to setup vertex program constants.
		float const *g_fadeFactors = NULL;
		FixedStreamFormatInfo const *g_fixedStreamFormatInfo;
		VariableStreamFormatInfo const *g_firstVertexSetsVariableStreamFormatInfo = NULL;

		// Small table data pointers needed by the mesh front end.
		static ShadowInfo *s_shadowInfos;
		ViewportInfo *g_bucketedViewportInfo;
		DmInfo *g_dmInfo;

		// Small table data are stored here.
		U64 g_meshDataBufSize = 0;             // in bytes
		U64 g_meshDataDblBuf = 0;
		ICE_ALIGN(128) U8 g_meshDataBuf[2][MESH_DATA_BUF_SIZE];

		// Stores most of the mesh processing job related data.
		static ICE_ALIGN(128) U8 s_jobDataBuf[2][JOB_INFO_BUF_SIZE];
		static JobDataAllocator s_jobDataAllocator;
		static U32 s_jobDataFailedAllocationSize;

		// Structure to track the output locations required for the mesh processing for an entire frame.
		static OutputLocations s_outputLocs;

		// Colors needed by the ShowVertexSets routine.
		static U32 s_fragmentColorArray[] = 
		{
		#define MAKE_RGB(r,g,b) (0xFF000000 | ((r) << 16) | ((g) << 8) | (b))
		#include "icehexcolors.h"
		#undef MAKE_RGB
		};

		bool g_showVertexSets = false;

		Mat44 g_transposeMvpMatrix;

		Transform const *g_rigidObjectTransform; // For rigid instanced objects (NULL for worldspace or skinned objects)
		Mat44 *g_transposeRigidObjectTransform;
	}
}



static inline void ShowVertexSets(void const * const pVertexSet)
{
	if (!g_showVertexSets) return;

	static FragmentProgram *shader = NULL;
	if (!shader) shader = Graphics::LoadFragmentProgram("shaders/white.fbin");

	Render::SetFragmentProgram(shader);

	Render::EnableRenderState(kRenderBlend);
	Render::SetBlendEquation(kBlendEquationAdd);
	Render::SetBlendFuncSeparate(kBlendConstantColor, kBlendZero, kBlendOne, kBlendZero);
	Render::SetBlendColor(s_fragmentColorArray[((U32)pVertexSet >> 7) % ARRAY_COUNT(s_fragmentColorArray)]);
}

static bool WarnJobDataOutOfMemory(U8 **jobData, U8 **jobDataEnd, U32 size)
{
	// Keep track of the total size of all failed allocations.
	s_jobDataFailedAllocationSize += size;

	// Print warning about a failed allocation.
	U32 left = U32(jobDataEnd) - U32(jobData);
	printf("Warning: Failed allocation.  Job data allocation requestied %d bytes, but only %d bytes remain.\n", size, left);
	printf("         Total failed job data allocations thus far are %d bytes.\n", s_jobDataFailedAllocationSize);

	return false;
}


// Reset the job data
void Ice::Bucketer::ResetJobData(SpuModuleHandle *pSpuModule, SingleThreadJobList *pJobList, U32 meshOutputBufferType,
	void *meshOutputBufferAllocator, GpuSyncMutex *gpuSyncMutex, U32 meshOutputBufferOffset)
{
	g_meshDataBufSize = 0;
	g_meshDataDblBuf ^= 1;
	s_jobDataFailedAllocationSize = 0;

	// Setup the job data allocator for this frame.
	s_jobDataAllocator.m_pCurrent = s_jobDataBuf[g_meshDataDblBuf];
	s_jobDataAllocator.m_pEnd = s_jobDataAllocator.m_pCurrent + JOB_INFO_BUF_SIZE;
	s_jobDataAllocator.m_outOfMemoryCallback = &WarnJobDataOutOfMemory;

	// Setup the mesh frontend output locations for this frame.
	s_outputLocs.m_spuModule = pSpuModule;
	s_outputLocs.m_jobList = pJobList;
	s_outputLocs.m_jobDataAllocator = &s_jobDataAllocator;
	s_outputLocs.m_meshOutputBufferType = meshOutputBufferType;
	s_outputLocs.m_meshOutputBufferAllocator = meshOutputBufferAllocator;
	s_outputLocs.m_meshOutputBufferOffset = meshOutputBufferOffset;
	s_outputLocs.m_cmdBufferOffset = 0;
	s_outputLocs.m_gpuSyncMutex = gpuSyncMutex;
}

// Sets up the shadow information for a light source.
static void SetShadowInfoForLight(
	ShadowInfo * __restrict const si, 
	ShadowVolumeCastingPointLight const * const light,
	Transform const * const trans)
{
	// Transform the world space light position into object space?
	if (trans != NULL)
	{
		Point pt(light->m_position.x, light->m_position.y, light->m_position.z);
		pt = pt * Inverse(*trans);
		si->m_lightPosition[0] = F32(pt.X());
		si->m_lightPosition[1] = F32(pt.Y());
		si->m_lightPosition[2] = F32(pt.Z());
	}
	else
	{
		si->m_lightPosition[0] = light->m_position.x;
		si->m_lightPosition[1] = light->m_position.y;
		si->m_lightPosition[2] = light->m_position.z;
	}
	si->m_lightType = kPoint;
	// FIXME/TODO: Optimization. Always output shadow caps? Safe as it will 
	// always work, but they are not always necessary.
	si->m_outputShadowCaps = 1;
}

// Sets up the shadow information for a light source.
static void SetShadowInfoForLight(
	ShadowInfo * __restrict const si,
	ShadowVolumeCastingDirectionalLight const * const light,
	Transform const * const trans)
{
	// Transform the world space light position into object space?
	if (trans != NULL)
	{
		Point pt(light->m_direction.x, light->m_direction.y, light->m_direction.z);
		pt = pt * Inverse(*trans);
		si->m_lightPosition[0] = F32(pt.X());
		si->m_lightPosition[1] = F32(pt.Y());
		si->m_lightPosition[2] = F32(pt.Z());
	}
	else
	{
		si->m_lightPosition[0] = light->m_direction.x;
		si->m_lightPosition[1] = light->m_direction.y;
		si->m_lightPosition[2] = light->m_direction.z;
	}
	si->m_lightType = kDirectional;
	// FIXME/TODO: Optimization. Always output shadow caps? Safe as it will 
	// always work, but they are not always necessary.
	si->m_outputShadowCaps = 1;
}

void Ice::Bucketer::OnRigidObjectTransformChange()
{
	Mat44 mvpMatrix;
	F32 mvpDet;

	// Allocate & Setup 
	g_transposeRigidObjectTransform = (SMath::Mat44 *)(g_meshDataBuf[g_meshDataDblBuf] + g_meshDataBufSize);
	g_meshDataBufSize += (sizeof(*g_transposeRigidObjectTransform) + 0xF) & ~0xF;

	if (g_rigidObjectTransform)
	{
		mvpMatrix =  g_rigidObjectTransform->GetMat44() * g_viewProjectionMatrix;
		mvpDet = (F32)Determinant4(mvpMatrix);
		g_transposeMvpMatrix = Transpose(mvpMatrix);
		*g_transposeRigidObjectTransform = Transpose(Mat44(g_rigidObjectTransform->GetMat44()));
	}
	else
	{
		mvpMatrix = g_viewProjectionMatrix;
		mvpDet = (F32)Determinant4(mvpMatrix);
		g_transposeMvpMatrix = g_transposeViewProjectionMatrix;
		*g_transposeRigidObjectTransform = Mat44(kIdentity);
	}

	s_shadowInfos = (ShadowInfo *)(g_meshDataBuf[g_meshDataDblBuf] + g_meshDataBufSize);
	g_meshDataBufSize += (sizeof(*s_shadowInfos)*4 + 0xF) & ~0xF;
	ShadowInfo * __restrict si = s_shadowInfos;
	for(U32F i = 0; i < g_numPointLights; ++i, ++si)
		SetShadowInfoForLight(si, g_pPointLights[i], g_rigidObjectTransform); 
	for(U32F i = 0; i < g_numDirectionalLights; ++i, ++si)
		SetShadowInfoForLight(si, g_pDirectionalLights[i], g_rigidObjectTransform); 

	// TODO: get the value of this from somewhere special
	bool trimReverseBackfaceCull = false;
	g_viewportInfo.m_reverseBackFaceFlag = trimReverseBackfaceCull ? 0 : 1;
	g_viewportInfo.m_reverseBackFaceFlag ^= (mvpDet < 0) ? 1 : 0;

	// World space to clip space transform
	*((Mat44 *)(&g_viewportInfo.m_worldToHdcMatrix[0])) = g_transposeViewProjectionMatrix;
}

// Render a batch
static void RenderObjectBatch(ObjectBatch const * const batch,
	ObjectBatchCollection const * const collection,
	MaterialTechnique const * const technique)
{
	PmVertexSet const *vertexSet = batch->m_vertexSet;
	VertexSetBlendShapes const *vertexSetBlendShapes = batch->m_vertexSetBlendShapes;
	MaterialInstance const *matInst = collection->m_materialInstance;
	MaterialDescriptor const *matDesc = matInst->m_materialDescriptor;
	
	// Get the current command context.
	s_outputLocs.m_commandContext = g_currentCommandContext;
	
	U32F lodIndex = batch->m_lodIndex;
	InputInfo ii;
	ii.m_procFlags = technique->m_meshProcessingFlags;
	ii.m_pFixedFormatInfo = &matDesc->m_fixedFormatInfo;
	ii.m_pVertexInputInfo = &technique->m_vertexInputInfo;
	ii.m_pMeshProgramInfo = technique->m_meshProcInfo;
	ii.m_pMatrices = (float *)collection->m_transforms;
	ii.m_pViewport = g_bucketedViewportInfo;
	ii.m_pRootTransform = (RigidObjectTransform const *)g_transposeRigidObjectTransform;
	ii.m_pDmInfo = g_dmInfo;
	ii.m_pContinuousPmInfo = NULL;
	ii.m_pShadowInfos = s_shadowInfos;
	ii.m_pBlendShapeFactors = g_blendShapeFactor;
	ii.m_numPointLights = 0;
	ii.m_numDirectionalLights = 0;
	ii.m_lodLevel = lodIndex;
	ii.m_lodLevelFractional = batch->m_isTransitionTechnique ? collection->m_lodFraction : 0.0f;
	ii.m_lowestLodLevel = batch->m_highestLodToBlend;
	ii.m_deltaStreamUsageBits = technique->m_deltaStreamUsageBits;
	ii.m_gpuSkinningConstantIndex = technique->m_gpuSkinningConstantIndex;
	
	// Setup the continuous PM info structure if this batch is in a continuous PM collection.
	if (collection->m_isCpm && batch->m_isTransitionTechnique)
	{
		// Allocate memory for a continuous PM info structure.
		ContinuousPmInfo * __restrict info = (ContinuousPmInfo *)(g_meshDataBuf[g_meshDataDblBuf] + g_meshDataBufSize);
		g_meshDataBufSize += (sizeof(ContinuousPmInfo) + 0xF) & ~0xF;

		// Set the lowest LOD level.
		info->m_lowestLodToBlend = batch->m_highestLodToBlend;

		// Set the eye position for performing distance calculations.
		RenderCamera const *camera = GetRenderCamera();
		Point location = camera->m_transform.GetTranslation();
		info->m_eyeVec[0] = F32(location.X());
		info->m_eyeVec[1] = F32(location.Y());
		info->m_eyeVec[2] = F32(location.Z());

		// Copy the distances into the continuous PM info structure.
		ICE_ASSERT(batch->m_lodDistances != NULL);
		U64 const *pSrc = (U64 *)batch->m_lodDistances;
		U64 * __restrict pDst = (U64 *)info->m_distances;
		pDst[0] = pSrc[0];
		pDst[1] = pSrc[1];
		pDst[2] = pSrc[2];
		pDst[3] = pSrc[3];
		pDst[4] = pSrc[4];
		pDst[5] = pSrc[5];
		pDst[6] = pSrc[6];
		pDst[7] = pSrc[7];
	
		ii.m_pContinuousPmInfo = info;
	}

	switch (technique->m_meshProcessingFrontEndFunction)
	{
	case kRenderGeneralPmVertexSet:
		{
			Mesh::ShadowOutputLocations shadowOutputLocs;

			if(UNLIKELY(technique->m_meshProcessingFlags & kDoShadowVolumeExtrusion))
			{
				ii.m_numPointLights = g_numPointLights;
				ii.m_numDirectionalLights = g_numDirectionalLights;

				U64 light = 0;
				for(U32F i = 0; i < ii.m_numPointLights; ++i, ++light)
				{
					ShadowVolumeCastingPointLight const *bl = g_pPointLights[i];
					shadowOutputLocs.m_profileCommandContext[light] = bl->m_profileContext;
					shadowOutputLocs.m_frontCapCommandContext[light] = bl->m_frontCapContext;
					shadowOutputLocs.m_backCapCommandContext[light] = bl->m_backCapContext;
				}

				for(U32F i = 0; i < ii.m_numDirectionalLights; ++i, ++light)
				{
					ShadowVolumeCastingDirectionalLight const *bl = g_pDirectionalLights[i];
					shadowOutputLocs.m_profileCommandContext[light] = bl->m_profileContext;
					shadowOutputLocs.m_frontCapCommandContext[light] = bl->m_frontCapContext;
				}
			}
			
			Mesh::RenderGeneralPmVertexSet(vertexSet, vertexSetBlendShapes, &ii, &s_outputLocs, &shadowOutputLocs);
		}
		break;
	case kRenderGpuVertexSet:
		{
			// Convert a PmVertexSet into a GpuVertexSet
			GpuVertexSet gvs;
			gvs.m_streamCount = vertexSet->m_streamCount;
			gvs.m_triangleCount = vertexSet->m_lodIndexTables[lodIndex].m_triangleCount;
			gvs.m_matrixRunsCount = vertexSet->m_lodSkinningTables[lodIndex].m_matrixRunsCount;
			gvs.m_padding = 0;
			gvs.m_streams = vertexSet->m_streams;
			gvs.m_indexes = vertexSet->m_lodIndexTables[lodIndex].m_indexes;
			gvs.m_matrixRuns = vertexSet->m_lodSkinningTables[lodIndex].m_matrixRuns;

			// Render the GPU vertex set.
			RenderGpuVertexSet(&gvs, ii.m_pVertexInputInfo, ii.m_pMatrices, ii.m_pFixedFormatInfo->m_matrixFormat,
					ii.m_gpuSkinningConstantIndex, s_outputLocs.m_commandContext);
		}
		break;
	case kRenderBasicVertexSet:
		{
			// Convert a PmVertexSet into a BasicVertexSet

			// Count vertexes.
			U16 vertexCount = 0;
			U16 rowCount = vertexSet->m_lodVertexTable[0].m_rowCount;
			for (U16 iLod = lodIndex; iLod < rowCount; iLod++) 
				vertexCount += vertexSet->m_lodVertexTable[iLod].m_vertexCount;

			// Setup BasicVertexSet.
			BasicVertexSet bvs;
			bvs.m_streamCount = vertexSet->m_streamCount;
			bvs.m_nvStreamCount = vertexSet->m_nvStreamCount;
			bvs.m_vertexCount = vertexCount;
			bvs.m_triangleCount = vertexSet->m_lodIndexTables[lodIndex].m_triangleCount;
			bvs.m_streamFormatInfo = vertexSet->m_streamFormatInfo;
			bvs.m_streams = vertexSet->m_streams;
			bvs.m_indexes = vertexSet->m_lodIndexTables[lodIndex].m_indexes;
			bvs.m_skinningTables = &vertexSet->m_lodSkinningTables[lodIndex];
			bvs.m_stencilInfo = NULL;

			StencilShadowInfo ssi;
			if (vertexSet->m_lodEdgeLists != NULL) 
			{
				// Count halo vertexes.
				U16 haloVertexCount = 0;
				for (U16 iLod = lodIndex; iLod < rowCount; iLod++) 
					haloVertexCount += vertexSet->m_lodVertexTable[iLod].m_haloVertexCount;

				// Setup stencil shadow info.
				ssi.m_edgeTable = vertexSet->m_lodEdgeLists[lodIndex].m_edgeTable;
				ssi.m_haloVertexCount = haloVertexCount;
				ssi.m_haloTriangleCount = vertexSet->m_lodIndexTables[lodIndex].m_haloTriangleCount;
				ssi.m_edgeCount = vertexSet->m_lodEdgeLists[lodIndex].m_edgeCount;
				bvs.m_stencilInfo = &ssi;
			}

			Mesh::RenderBasicVertexSet(&bvs, NULL, &ii, &s_outputLocs, NULL);
		}
		break;
	case kRenderDiscretePmVertexSet:
		Mesh::RenderDiscretePmVertexSet(vertexSet, NULL, &ii, &s_outputLocs, NULL);
		break;
	case kRenderContinuousPmVertexSet:
		Mesh::RenderContinuousPmVertexSet(vertexSet, &ii, &s_outputLocs, NULL);
		break;
	case kRenderDmVertexSet:
		Mesh::RenderDmVertexSet(vertexSet, NULL, &ii, &s_outputLocs, NULL);
		break;
	default:
		printf("ERROR: Unrecognized mesh processing front end function in technique: %d\n", technique->m_meshProcessingFrontEndFunction);
		ICE_ASSERT(0);
	}

	ICE_ASSERT(U32(s_jobDataAllocator.m_pCurrent) - U32(s_jobDataBuf[g_meshDataDblBuf]) <= JOB_INFO_BUF_SIZE);
}

void Ice::Bucketer::RenderCollections(ObjectBatchCollection const * const collection, U64 const var)
{
	MaterialInstance const *materialInst = collection->m_materialInstance;
	MaterialDescriptor const *materialDesc = materialInst->m_materialDescriptor;

	// Find the entry in the technique table (same over all batches)
	U8 column = materialDesc->m_variationMappingTable[var]; 

	// If the material does not have this variation, don't render it. This is a completely valid
	// state which is required e.g. for unlit materials to skip lighting passes etc.
	if (column == kVariationNotPresent) 
		return;

	g_fadeFactors = collection->m_fadeFactors;
	g_fixedStreamFormatInfo = materialDesc->m_fixedFormatInfo.m_streamFormatInfo;
	g_firstVertexSetsVariableStreamFormatInfo = collection->m_firstBatch ? collection->m_firstBatch->m_vertexSet->m_streamFormatInfo : NULL;
	
	MaterialTechnique const *lastTechnique = NULL;

	Transform const *transforms = collection->m_isRigid ? collection->m_transforms : NULL;
	if (g_rigidObjectTransform != transforms)
	{
		g_rigidObjectTransform = transforms;
		OnRigidObjectTransformChange();
	}

	U32 columnCount = materialDesc->m_variationCount; 
	LodTechniqueIndexes const *tableEntry = materialInst->m_techniqueTable + (columnCount * collection->m_techniqueTableRow) + column;

	for (ObjectBatch const *batch = collection->m_firstBatch; batch != NULL; batch = batch->m_next)
	{
		U32 techIdx = batch->m_isTransitionTechnique ? tableEntry->m_transitionLodTechniqueIndex :
				tableEntry->m_flatLodTechniqueIndex;
		ICE_ASSERT(techIdx != kInvalidTechnique);
		MaterialTechnique const *technique = &materialDesc->m_techniques[techIdx];

		// Setupo a new material if the technique has changed since the last batch.
		if (UNLIKELY(lastTechnique != technique))
		{
			Graphics::SetupMaterial(materialInst, technique);
			SetupArrayFormats(technique->m_attributeArrayFormatTable, technique->m_attributeArrayFormatCount,
					g_currentCommandContext);
			lastTechnique = technique;
		}

		// If this flag is set then each vertex set is shown in a solid, but different color.
		if (g_showVertexSets)
		{
			ShowVertexSets(batch->m_vertexSet);
		}

		// Count the number of indices and place them in the proper counter.
		IndexTable const *indexTable = &batch->m_vertexSet->m_lodIndexTables[batch->m_lodIndex];
		U32 indexCount = (indexTable->m_triangleCount - indexTable->m_haloTriangleCount) * 3;
		//ICE_ASSERT(indexCount > 0);
		if (indexCount == 0)
			continue;
		if (technique->m_meshProcCommandList == NULL)
			g_toGpuIndexCount += indexCount;
		else if (technique->m_meshProcessingFlags & kDoTrimming)
			g_toSpuIndexCount += indexCount;
		else
			g_throughSpuIndexCount += indexCount;

		RenderObjectBatch(batch, collection, technique);
	}
}

void Ice::Bucketer::RenderCollectionsInSphere(ObjectBatchCollection const * const /*collection*/, U64 const /*var*/, Vec4 const &/*sphere*/)
{
	// This function is the same as RenderCollections except that the bounding sphere for each rendered vertex set
	// is supposed to be checked against the sphere parameter.  If the sphere intersect, then the vertex set is
	// rendered, otherwise it is skipped.
	// With the new data types, no vertex set bounding sphere is available, so this routine won't work.
	// The contents of the old routine were deleted as they were too out of date to be of any use.

	// Please don't call this function until it is brought back from the dead.
	ICE_ASSERT(0);
}

