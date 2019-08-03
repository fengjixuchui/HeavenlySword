/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 * 
 * Revision History:
 *  - Created 7/28/05
 */

#ifndef ICEOBJECTBATCH_H
#define ICEOBJECTBATCH_H

#include "jobapi/joblist.h"
#include "jobapi/spumodule.h"
#include "icemeshfrontend.h"
#include "icematerial.h"

namespace Ice
{

	// Forward declarations
	namespace Graphics
	{
		struct LodDistances;
	}

	namespace Bucketer
	{
		#define JOB_INFO_BUF_SIZE (4096 * 1024)       // Must be a multiple of 128 bytes.
		#define MESH_DATA_BUF_SIZE (2048 * 1024)      // Must be a multiple of 128 bytes.

		// A ObjectBatch describes a small renderable part of a whole object.
		struct ObjectBatch
		{
			// The next object batch in this chain.
			ObjectBatch *m_next;
			// 0 if flat technique is to be used, 1 if transition technique
			U32 m_isTransitionTechnique;
			// The vertex set used to render
			const struct Ice::Mesh::PmVertexSet *m_vertexSet;
			const struct Ice::Mesh::VertexSetBlendShapes *m_vertexSetBlendShapes;
			// Highest LOD to blend for CPM.
			U32 m_highestLodToBlend; 
			// Array of LOD switching distances.
			Graphics::LodDistances *m_lodDistances; 
			// LOD to use for DPM (Lowest to blend for CPM).
			I32 m_lodIndex;
		};
	
		// ObjectBatchCollection is a collection of ObjectBatches.
		struct ObjectBatchCollection
		{
			// The next ObjectBatchCollection in this chain.
			ObjectBatchCollection *m_next;
			// The bits of renderable pieces.
			ObjectBatch *m_firstBatch;
			// The row into the technique table for the technique used.
			U32 m_techniqueTableRow;
			// The material instance
			struct Ice::Graphics::MaterialInstance *m_materialInstance;
			// The object's model to world transform
			const SMath::Transform *m_transforms;
			// Is this object rigid, or skinned?
			bool m_isRigid;
			// The values of the shader LOD fade factors
			F32 m_fadeFactors[4];
			// LOD fraction for DPM.
			F32 m_lodFraction;
			// Is this object using CPM?
			bool m_isCpm;
		};

		// The Object Batch manager is responsible for managing the internal list of
		// object batches.
		namespace ObjectBatchMgr
		{
			static const unsigned int kMaxCollections = 1024;
			static const unsigned int kMaxBatches = 8192;

			extern U32 g_numCollections;
			extern U32 g_numBatches;
			extern ObjectBatchCollection g_collections[kMaxCollections];
			extern ObjectBatch g_batches[kMaxBatches];

			// Allocate the next available collection
			inline ObjectBatchCollection *Alloc()
			{
				// Grab the next available collection
				assert(g_numCollections+1 < kMaxCollections);
				return &g_collections[g_numCollections++];
			}
			
			// Allocate the next available Object Batch
			inline ObjectBatch *AllocBatch()
			{
				// Grab the next available batch
				assert(g_numBatches+1 < kMaxBatches);
				return &g_batches[g_numBatches++];
			}
			
			// Free everything
			inline void FreeAll()
			{
				// Setting the counts to zero effectively frees everything.
				g_numCollections = 0;
				g_numBatches = 0;
			}
		};

	
		void ResetJobData(SpuModuleHandle *pSpuModule, SingleThreadJobList *pJobList, U32 meshOutputBufferType,
			void *meshOutputBufferAllocator, Ice::MeshProc::GpuSyncMutex *gpuSyncMutex, U32 meshOutputBufferOffset);
		void RenderCollections(const ObjectBatchCollection *collection, const U64 var);
		void RenderCollectionsInSphere(const ObjectBatchCollection *collection, const U64 var, const SMath::Vec4 &sphere);
		void OnRigidObjectTransformChange();

		U64 static const kNumBlendShapes = 8;
		extern I16 g_blendShapeFactor[kNumBlendShapes];

		// Needed to setup vertex program constants.
		extern const float *g_fadeFactors;
		extern MeshProc::FixedStreamFormatInfo const *g_fixedStreamFormatInfo;
		extern MeshProc::VariableStreamFormatInfo const *g_firstVertexSetsVariableStreamFormatInfo;

		// Small data table pointers needed by the mesh front end.
		extern MeshProc::ViewportInfo *g_bucketedViewportInfo;
		extern MeshProc::DmInfo *g_dmInfo;

		// Small table data are stored here.
		extern U64 g_meshDataBufSize;             // in bytes
		extern U64 g_meshDataDblBuf;
		extern U8 g_meshDataBuf[2][MESH_DATA_BUF_SIZE];

		extern bool g_showVertexSets;

		extern SMath::Mat44 g_transposeMvpMatrix;

		// For rigid instanced objects (NULL for worldspace or skinned objects)
		extern const SMath::Transform *g_rigidObjectTransform;
		extern SMath::Mat44 *g_transposeRigidObjectTransform;
	}
}

#endif // ICEOBJECTBATCH_H

