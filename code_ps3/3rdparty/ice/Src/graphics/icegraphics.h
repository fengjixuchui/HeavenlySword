/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_GRAPHICS_H
#define ICE_GRAPHICS_H

#include <cell/spurs/types.h>

#include "shared/math/vec4.h"
#include "shared/math/transform.h"

#include "icematerial.h"
#include "icecameras.h"
#include "icelights.h"
#include "icemeshstructs.h"
#include "icemeshfrontend.h"

namespace Ice
{
	// Forward declarations
	struct DmaTag;
	namespace Bucketer
	{
		struct CachedBucketData;
	}
}

namespace Ice
{
	namespace Graphics
	{
		// Forward declaration
		struct MaterialDescriptor;

		struct FadeFactorCoeffs // 4-byte aligned
		{
			float               m_coeffs[2];            ///< factor = clamp(coeff[0] * distance - coeff[1]).
		};

		/*!
		 * \brief The discrete PM renderable contains a set of vertex sets which share common
		 *        properties (they belong to the same object and part, and all use the same
		 *        material instance).
		 *
		 * Note that there is a table of pointers to vertex set blend shape structures which
		 * parallels the table of vertex sets. If none of the vertex sets in the renderable have
		 * blend shapes, the table pointer itself will be NULL. Otherwise, any specific vertex
		 * set which has no blend shapes affecting it will have the corresponding pointer in the
		 * table be NULL.
		 *
		 */
		struct DiscretePmRenderable   // 4-byte aligned
		{
			MaterialInstance                    *m_materialInstance;
			U16                                 m_vertexSetCount;
			U16                                 m_fadeFactorCount;
			Mesh::PmVertexSet					**m_vertexSets;
			Mesh::VertexSetBlendShapes			**m_vertexSetBlendShapes;
			FadeFactorCoeffs                    *m_fadeFactorCoeffs;
		};

		struct DiscretePmPart   // 4-byte aligned
		{
			U16                     m_renderableCount;  ///< Number of renderables
			U16                     m_padding;
			DiscretePmRenderable    **m_renderables;    ///< Table of PM renderables in part
		};

		enum DiscretePmLodGroupFlagBits
		{
			kDiscretePmLodGroupDisplacementMapping       = 1 << 0,  ///< This is a DM LOD group (must have only one LOD in this case)
			kDiscretePmLodGroupHasParts                  = 1 << 1   ///< This LOD group contains parts rather than renderables
		};

		struct DiscretePmLodGroup  // 4-byte aligned
		{
			U16 m_startLod;                             ///< First LOD blended from
			U16 m_endLod;                               ///< Last LOD blended to
			U16 m_count;                                ///< Number of renderables or parts
			U16 m_flags;                                ///< Uses DiscretePmLodGroupFlagBits enum
			union {
				DiscretePmRenderable **m_renderables;   ///< Table of PM renderables in object
				DiscretePmPart **m_parts;               ///< Table of PM parts in object (can contain NULL pointers)
			};
		};

		struct LodDistances // 4-byte aligned
		{
			F32 m_near;   ///< Distance to start switching to next-less-detailed LOD
			F32 m_far;    ///< Distance to finish switching to next-less-detailed LOD
		};

		struct DiscretePmObject   // 4-byte aligned
		{
			U8 m_lodGroupCount;                 ///< Number of LOD groups
			U8 m_lodCount;                      ///< Number of LOD levels in object
			U16 m_transformCount;               ///< Number of transforms
			DiscretePmLodGroup **m_lodGroups;   ///< Table of LOD groups
			LodDistances *m_lodDistances;       ///< Array of (m_lodCount - 1) LOD switching distance pairs
		};

		// These are organized in trees. Each level of the tree corresponds to one LOD group (the root to the least detailed LOD group).
		struct ContinuousPmNode   // 16-byte aligned
		{
			Mesh::PmVertexSet			*m_vertexSet;	    ///< The vertex set for this node
			ContinuousPmNode            **m_childNodes;     ///< Table of child nodes
			U16                         m_childCount;       ///< Number of child nodes
			U16                         m_padding[3];
			SMath::Vec4                 m_boundingSphere;   ///< World-space bounding sphere for node
		};

		struct ContinuousPmRenderable   // 4-byte aligned
		{
			MaterialInstance    *m_materialInstance;
			U16                 m_continuousPmTreeCount;        ///< Number of CPM trees in this renderable
			U16                 m_padding;
			ContinuousPmNode    **m_continuousPmTrees;          ///< Table of CPM trees
		};

		struct ContinuousPmObject   // 4-byte aligned
		{
			U8 m_lodGroupCount;                         ///< Number of LOD groups
			U8 m_lodCount;                              ///< Number of LOD levels in object
			U16 m_renderableCount;                      ///< Number of renderables (one per material)
			U32 *m_lodGroupStartLods;                   ///< Table of starting LOD for each LOD group (they all end at the lowest LOD)
			ContinuousPmRenderable **m_renderables;     ///< Table of renderables
			LodDistances *m_lodDistances;               ///< Array of (m_lodCount - 1) LOD switching distance pairs
		};

		enum MeshOutputBufferType
		{
			kMeshOutputDoubleBuffer,
			kMeshOutputSingleBufferWithSync,
			kMeshOutputRingBuffersWithSync
		};

		extern U64 g_toGpuIndexCount;
		extern U64 g_toSpuIndexCount;
		extern U64 g_throughSpuIndexCount;
		extern I32 g_referenceValue;
		extern Render::CommandContext *g_commandContextStack[8];
		extern U32 g_commandContextStackIndex;

		// Viewport information.
		extern MeshProc::ViewportInfo g_viewportInfo;

		// Camera information
		extern U32 g_cameraLeft;
		extern U32 g_cameraTop;
		extern U32 g_cameraWidth;
		extern U32 g_cameraHeight;
		extern F32 g_cameraFocalLength;
		extern SMath::Point g_cameraWorldPosition;
		extern SMath::Transform g_cameraTransform;

		// The camera frustum
		extern SMath::Vec4 g_frustumPlaneLeft;
		extern SMath::Vec4 g_frustumPlaneRight;
		extern SMath::Vec4 g_frustumPlaneTop;
		extern SMath::Vec4 g_frustumPlaneBottom;
		extern SMath::Vec4 g_frustumPlaneNear;
		extern SMath::Vec4 g_frustumPlaneFar;

		// Projection matrices.
		extern SMath::Mat44 g_projectionMatrix;
		extern SMath::Mat44 g_viewProjectionMatrix;
		extern SMath::Mat44 g_transposeViewProjectionMatrix;

		// Lights.
		extern U32 g_numPointLights;
		extern U32 g_numDirectionalLights;
		extern Bucketer::ShadowVolumeCastingPointLight **g_pPointLights;
		extern Bucketer::ShadowVolumeCastingDirectionalLight **g_pDirectionalLights;

		//! Initializes the graphics rendering system.
		/*! \param mode                   Output display mode.
		    \param pitch                  Pitch of the frame buffers.
		    \param format                 Format of the frame buffers.
		    \param multisample            Multisample mode.
		    \param meshOutputBufferSize   The total size in bytes of the mesh output buffer.  Ring buffers are allocated from this.
		                                  Must be a multiple of 1MB.
		*/
		void Initialize(Render::DisplayMode mode, U32 pitch, Render::ColorBufferFormat format,
			Render::MultisampleMode multisample, U64 meshOutputBufferSize, CellSpurs *pSpurs, AuditManager *pAuditManager = NULL);
		void Terminate(void);
		
		//! Sets up parameters needed by the rendering process for this frame.
		/*! \param doMeshOnSpu                     When true, mesh processing is done on SPUs, otherwise it is done on the PPU.
		    \param firstSpuForMesh                 The number of the first SPU to be used for mesh processing (0-5).
		    \param numSpusForMesh                  The number of SPUs to use for mesh processing (1-6).
		    \param outputBufferSizeForMesh         The total size in bytes to be used for all mesh processing output buffers.
		                                           This must be at least as large as the largest output from a vertex set
		                                           times the number of SPUs being used.  A safe minimum value would be 1728 KB.
		                                           Must be <= the specified value during initialization.
		    \param firstGpuSemaphoreIndexForMesh   The first of a block of GPU semaphores to be used to inform mesh processing
		                                           about the GPU's consumption of the ring buffer.  Must be >= 6.
		*/
		void BeginRendering(bool doMeshOnSpu, U64 firstSpuForMesh, U64 numSpusForMesh,
			U64 outputBufferTypeForMesh, U64 outputBufferSizeForMesh, U64 firstGpuSemaphoreIndexForMesh);
		void EndRendering();
		
		RenderCamera *GetRenderCamera(void);
		void SetRenderCamera(RenderCamera *camera);
		void SetRenderToTextureCamera(RenderToTextureCamera *camera);

		// Sphere frustum culling
		bool IsSphereInsideFrustum(const SMath::Vec4 &sphere);
		
		// Set the point lights that will be in use for casting shadow volumes
		void SetShadowVolumeCastingPointLights(Bucketer::ShadowVolumeCastingPointLight **lights, U32 count);
		// Set the directional lights that will be in use for casting shadow volumes
		void SetShadowVolumeCastingDirectionalLights(Bucketer::ShadowVolumeCastingDirectionalLight **lights, U32 count);

		// For debugging
		void GetFrameStats(U64 *vertexCount, U64 *toGpuIndexCount, U64 *toSpuIndexCount, U64 *throughSpuIndexCount);

		static const U64 kPushBufferChunk = 4096-4;

		// Allocate a kPushBufferChunk size chunk of push buffer space from the current buffer
		void *AllocNextPushBuffer();
		bool ReserveFailedCallback(Render::CommandContextData *context, U32 numWords);

		// Pushes a command context onto the stack of command contexts
		FORCE_INLINE void PushContext(Render::CommandContext *context)
		{
			g_commandContextStack[g_commandContextStackIndex] = context;
			++g_commandContextStackIndex;
			Render::BindCommandContext(context);
		}
		
		// Pops a command context from the stack of command contexts
		FORCE_INLINE Render::CommandContext *PopContext()
		{
			--g_commandContextStackIndex;
			Render::CommandContext *context = g_commandContextStack[g_commandContextStackIndex-1];
			Render::BindCommandContext(context);
			return context;
		}

	}
}


#endif
