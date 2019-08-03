/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_MESH_FRONTEND_H
#define ICE_MESH_FRONTEND_H

#include "jobapi/joblist.h"
#include "jobapi/spumodule.h"
#include "icemeshstructs.h"
#include "icerender.h"

namespace Ice
{
	//! Locator structure with both a CPU pointer and an RSX offset (context selector in MSB).
	struct Locator
	{
		void *m_ptr;
		U32   m_ofs;
	};

	//! Enum for the context selector in the MSB of the Locator offset.
	enum LocatorContext
	{
		kLocatorVideoMemory  = 0x00000000,
		kLocatorMainMemory =   0x80000000
	};

	enum
	{
		kLocatorNullGpuOffset = 0xFFFFFFFF ///< Locator GPU offset value for unmappable or NULL addresses
	};

	namespace Mesh
	{
		//! Describes a single run of matrices.
		/*! NOTE that this data is generated differently if the matrices are 48 byte instead of 64 byte. 
		    4-byte aligned.
		*/
		struct MatrixRun   
		{
			//! This is an index into the OBJECT's transforms table.
			U16 m_firstIndex;
			//! This is the number of matrices to sent to the SPU or GPU for skinning.
			U16 m_length;
		};

		//! Structure of a vertex set that is to be processed directly by the GPU.
		/*! 4-byte aligned */
		struct GpuVertexSet
		{
			//! Number of streams.
			U16        m_streamCount;
			//! Number of triangles.
			U16        m_triangleCount;
			//! Number of matrix runs.
			U16        m_matrixRunsCount;
			//! Unused space.
			U16        m_padding;
			//! Stream data.
			Locator    *m_streams;
			//! Index list (128-byte aligned).
			Locator    m_indexes;
			//! Pointer to the matrix runs.
			MatrixRun  *m_matrixRuns;
		};

		//! Contains all of the extra info required to perform SPU skinning.
		/*! 4-byte aligned */
		struct SkinningTables
		{
			//! Length of the control byte stream.
			U16 m_controlStreamLength;
			//! Length of the same stream in bytes.
			U16 m_sameStreamLength;
			//! Length of the diff stream in bytes.
			U16 m_diffStreamLength;
			//! Length of the weights stream in bytes.
			U16 m_weightsStreamLength;
			//! Number of matrix runs.
			U16 m_matrixRunsCount;
			//! Unused space.
			U16 m_padding;
			//! Pointer to the control byte stream.
			/*! 16-byte-aligned 
			    Padded to multiple of 16 bytes. 
			*/
			U8 *m_controlStream;
			//! Pointer to the stream of same type indexes.
			/*! 16-byte-aligned 
			    Padded to multiple of 16 bytes. 
			    2 bytes for each element.
			*/
			U16 *m_sameStream;
			//! Pointer to the stream of different type indexes.
			/*! 16-byte-aligned 
			    Padded to multiple of 16 bytes. 
			    2 bytes for each element, but in groups of 4. 
			*/
			U16 *m_diffStream;
			//! Pointer to the weight stream.
			/*! 16-byte-aligned 
			    Padded to multiple of 16 bytes. 
			    In groups of 4 elements.
			*/
			void *m_weightsStream;
			//! Pointer to the weight stream for transitioning to next joint LOD.
			/*! 16-byte-aligned 
			    Padded to multiple of 16 bytes. 
			*/
			void *m_auxWeightsStream;
			//! Pointer to matrix runs.
			MatrixRun  *m_matrixRuns;
		};

		//! Structure containing all of the necessary data to add stencil shadows to a non-PM vertex set.
		/*! 4-byte aligned. */
		struct StencilShadowInfo
		{
			//! Pointer to the edge table.
			void *m_edgeTable;
			//! Number of halo vertexes.
			U16 m_haloVertexCount;
			//! Number of halo triangles.
			/*! Immediately followed in the index list. */
			U16 m_haloTriangleCount;
			//! Number of edges in the edge table.
			U16 m_edgeCount;
		};

		//! Stream Format information for fixed bit width structures.
		/*! 4-byte aligned */
		struct FixedFormatInfo
		{
			//! Format code for index table. 
			/*! Ice::MeshProc::IndexFormat */
			U8 m_indexFormat;
			//! Format code for skinning weight table.
			/*! Ice::MeshProc::SkinningWeightsFormat */
			U8 m_skinWeightFormat;
			//! Format code for skinning matrix table. 
			/*! Ice::MeshProc::MatrixFormat */
			U8 m_matrixFormat;
			//! Format code for parent table. 
			/*! Ice::MeshProc::PmParentFormat */
			U8 m_parentFormat;
			//! Format code for edge table. 
			/*! Ice::MeshProc::EdgeFormat */
			U8 m_edgeFormat;
			//! Format code for displacement table.
			/*! Ice::MeshProc::DisplacementFormat */
			U8 m_dmDisplacementFormat;
			//! Unused space.
			U8 m_padding[2];
			//! Table of stream formats.
			MeshProc::FixedStreamFormatInfo *m_streamFormatInfo;
		};

		//! Structure of a basic vertex set with no CLOD.
		/*! 4-byte aligned */
		struct BasicVertexSet
		{
			//! Number of base streams.
			/*! Not including delta streams. */
			U16 m_streamCount;
			//! Number of base NV streams.
			U16 m_nvStreamCount;
			//! Number of vertexes (including halo vertexes).
			U16 m_vertexCount;
			//! Number of triangles (including halo triangles).
			U16 m_triangleCount;
			//! Pointer to info on SW formats and variable scale/bias.
			MeshProc::VariableStreamFormatInfo *m_streamFormatInfo;
			//! Stream data (NV streams, then SW streams). Streams are 16-byte aligned in main memory.
			Locator *m_streams;
			//! Index list (128-byte aligned).
			Locator m_indexes;
			//! Pointer to the skinning tables for SPU skinning.
			SkinningTables *m_skinningTables;
			//! If using stencil shadows, points to the stencil shadow info.
			StencilShadowInfo *m_stencilInfo;
		};

		//! Contains information about the number of triangles and the index table of triangles.
		/*! 4-byte aligned */
		struct IndexTable
		{
			//! The number of triangles (including halo triangles).
			U16 m_triangleCount;
			//! The number of halo triangles (immediately follow in the index list).
			U16 m_haloTriangleCount;
			//! Index list (128-byte aligned).
			Locator m_indexes;
		};

		//! Contains information about an edge table.
		/*! 4-byte aligned */
		struct EdgeList
		{
			//! Number of edges in the edge table.
			U16 m_edgeCount;
			//! Unused space.
			U16 m_padding;
			//! Pointer to the table of edges.
			void *m_edgeTable;
		};

		//! Describes a vertex set which can be used with either discrete PM or continuous PM.
		/*! 4-byte aligned */
		struct PmVertexSet
		{
			//! Number of base streams (not including delta streams).
			U16 m_streamCount;
			//! Number of base NV streams.
			U16 m_nvStreamCount;
			//! Pointer to info on SW formats and variable scale/bias.
			MeshProc::VariableStreamFormatInfo *m_streamFormatInfo;
			//! Stream data (NV streams, then SW streams). Streams are 16-byte aligned in main memory.
			Locator *m_streams;
			//! Pointer to the vertex table (see comments on struct definition).
			MeshProc::LodVertexTable *m_lodVertexTable;
			//! Pointer to the index table for each LOD.
			IndexTable *m_lodIndexTables;
			//! Pointer to the parent table for each LOD.
			void **m_lodParentTables;
			//! If doing SPU skinning, pointer to skinning tables for each LOD.
			SkinningTables *m_lodSkinningTables;
			//! Pointer to the edge list for each LOD (if using shadow volumes).
			EdgeList *m_lodEdgeLists;
		};

		//! Contains the necessary info for each LOD of a blend shape.
		/*! 4-byte aligned */
		struct BlendShapeLodInfo
		{
			//! The count of runs from the blend shape run list used for this LOD.
			U16 m_runCount;
			//! The count of nonzero deltas used for the blend shape delta streams for this LOD.
			U16 m_deltaCount;
		};

		//! Contains the info for a single blend shape.
		/*! 4-byte aligned */
		struct BlendShapeInfo
		{
			//! Length in bytes of the delta stream format info.
			U16 m_streamFormatInfoLen;
			//! Unused space.
			U16 m_padding;
			//! Format information for the blend shape.
			MeshProc::VariableStreamFormatInfo *m_streamFormatInfo;
			//! Table of bit counts for each stream.
			U8 *m_streamBitCounts;
			//! Pointers to the streams. Streams are 16-byte aligned in main memory.
			void **m_streams;
			//! Info table per LOD.  
			/*! Non-PM vertex sets have exactly one of these. */
			BlendShapeLodInfo *m_lodTables;
			//! Run list for this blend shape.
			MeshProc::DeltaRun *m_runList;
		};


		//! Contains all of the blend shape data.
		/*! This structure is the same of both PM and non-PM vertex sets. */
		/*! 4-byte aligned */
		struct VertexSetBlendShapes
		{
			//! Number of blend shapes.
			U16 m_shapeCount;
			//! Unused space.
			U16 m_padding;
			//! Indexes of vertex set blend shapes in object blend shape table.
			U16 *m_shapeRemapTable;
			//! Pointer to an array of structures containg the blend shape information.
			BlendShapeInfo *m_blendShapeInfos;
		};

		// Mesh Processing flags. Speficies various additional pieces of functionality that can be used.
		/*! Flags for the m_procFlags member of the InputInfo structure. */
		enum ProcessingFlags
		{
			//! Specifies whether or not hidden triangle trimming should be performed.
			/*! Applicable to all cases. */
			kDoTrimming              = 0x00000001,
			//! Specifies whether gpu skinning is in use.
			/*! Applicable to only the general case. */
			kDoGpuSkinning           = 0x00000002,
			//! Specifies that shadow volumes will be extruded.
			/*! Applicable to only the general case. */
			kDoShadowVolumeExtrusion = 0x00000004,
			//! Specifies that the triangles are two-sided. 
			/*! Applicable to all cases.  
			    For trimming tests. 
			*/
			kDoubleSided             = 0x00000008
		};

		//! Gives the origin of each hardware attribute.
		/*! 16-byte aligned. */
		struct VertexInputInfo
		{
			//! Each byte gives the origin of each possible hardware attribute.
			/*!   0x00       Attribute is not used.
			      0x01-0xEF  Attribute is generated by the SPU.  This value is the ID of that attribute.
			      0xF0-0xF7  Attribute is passed directly to the GPU.  This value minus 0xF0 is the stream
			                 index that contains this attribute.
			*/
			U8 m_attribIdMap[16];
			//! Byte offset of this attribute from the start of the stream.
			U8 m_attribOffset[16];
		};

		//! Custom compression/decompression info.
		/*! 16-byte aligned. */
		struct CustomCodecInfo
		{
			//! Size of the custom compression code.
			U32 m_compressionCodeSize;
			//! Pointer to the custom compression code in main memory.
			U32 m_compressionCodePtr;
			//! Size of the custom decompression code.
			U32 m_decompressionCodeSize;
			//! Pointer to the custom decompression code in main memory.
			U32 m_decompressionCodePtr;
		};

		//! Custom compression/decompression info for general mesh processing.
		/*! 4-byte aligned. */
		struct CustomCodecInfoGeneral
		{
			//! Size of the custom compression or decompression code.
			U32 m_codeSize;
			//! Pointer to the custom compression or decompression code in main memory.
			U32 m_codePtr;
		};

		//! Settings to configure the frontend.
		/*! 16-byte aligned. */
		struct MeshProgramInfo
		{
			//! If a stream is active, the corresponding bit is set.  
			/*! Stream 0 is LSB. */
			U16 m_nvInputMask;
			//! If a stream is active, the corresponding bit is set.
			/*! Stream 0 is LSB. */
			U16 m_swInputMask;
			//! If a stream is active, the corresponding bit is set.
			/*! Stream 0 is LSB. */
			U16 m_nvOutputMask;
			//! Bits that determine which streams the skinning code should affect.
			/*! 0x01 is pos, 0x02 is norm, 0x04 is tan, 0x08 is displacement norm. */
			U16 m_skinningBits;
			//! Custom compress/decompress offsets.  Offset of zero means use generic decompression routines.
			U8 m_customCodecOffsets[8];
			//! For each Nv stream, if the bit is set, the corresponding parameter is active.  
			/*! Parameter 0 is LSB. */
			U16 m_nvStreamParameterMask[8];
			//! Variable sized array of custom compression/decompression info structures.
			CustomCodecInfo m_customCodecInfos[];
		};

		//! Input information for the mesh processing frontend that is changed less frequently than once a vertex set.
		/*! Most data pointed to by members of the InputInfo structure must be retained until the job created by the
		    mesh processing frontend that uses this data has completed.  Data that must be retained is pointed to by
		    the members m_pMatrices, m_pViewport, m_pRootTransform, m_pDmInfo, m_pContinuousPmInfo, and m_pShadowInfos.
		    Also, the data that is pointed to by m_pFixedFormatInfo, m_pVertexInputInfo, m_pMeshProcCommandList, and
			m_pMeshProgramInfo also needs to be retained, but is more than likely constant in any case.
		*/
		struct InputInfo
		{
			U64 m_procFlags;                                          ///< Processing flags.
			FixedFormatInfo const *m_pFixedFormatInfo;                ///< Pointer to the fixed format info.
			VertexInputInfo const *m_pVertexInputInfo;                ///< Pointer to the vertex input info.
			union 
			{
				U16 const *m_pMeshProcCommandList;                    ///< Command list (for general case).
				MeshProgramInfo const *m_pMeshProgramInfo;            ///< Mesh program info (for fixed function).
			};
			F32 const *m_pMatrices;                                   ///< Pointer to the matrices.
			MeshProc::ViewportInfo const *m_pViewport;                ///< Pointer to the viewport.
			MeshProc::RigidObjectTransform const *m_pRootTransform;   ///< Pointer to the root transformation.
			MeshProc::DmInfo const *m_pDmInfo;                        ///< Pointer to the DM info.
			MeshProc::ContinuousPmInfo const *m_pContinuousPmInfo;    ///< Pointer to the Continuous PM info.
			U32 m_numPointLights;                                     ///< The number of point lights.
			U32 m_numDirectionalLights;                               ///< The number of directional lights.
			MeshProc::ShadowInfo const *m_pShadowInfos;               ///< Pointer to the array of shadow infos. 
			                                                          ///< The size of the array should be
			                                                          ///< m_numPointLights + m_numDirectionalLights
			I16 const *m_pBlendShapeFactors;                          ///< Pointer to the table of blend shape factors.
			                                                          ///< These are in 1:3:12 fixed point format.
			U32 m_lodLevel;                                           ///< The integer LOD level.
			F32 m_lodLevelFractional;                                 ///< The fractional part of the LOD level.
			U32 m_lowestLodLevel;                                     ///< Lowest LOD level for use with continuous PM.
			                                                          ///< Larger numbers refer to lower LOD levels.
			U32 m_deltaStreamUsageBits;                               ///< Delta streams used.
			                                                          ///< High halfword contains the count.
			U32 m_gpuSkinningConstantIndex;                           ///< GPU index to in which to place the first
			                                                          ///< constant used for GPU skinning.
		};

		//! If the mesh frontend is used with the "save for later" flag set, then the members in the cut and paste structure
		//! are set to be used later by the cut and paste routine.
		/*! This structure is not quite final. */
		struct CutAndPasteInfo
		{
			void *m_pushBuffer;
			U32  m_pbSize;
			U32  m_pbHoleSize;
			U8   *m_jobData;
			U32  m_jobDataSize;
			U32  m_tagsSize;
			U32  m_trimmingTagsOffset;
			U32  m_discretePmInfoTagOffset;
			U32  m_continuousPmInfoTagOffset;
			U32  m_dmInfoTagOffset;
			U32  m_blendShapeTagOffset;
			U32  m_matrixRunsCount;
			U32  m_matricesPtrOffset;
			U32  m_matrixTagsOffset;
			U32  m_numParentTablesForContinuousPm;
			U32  m_parentTableTagsOffset;
			U32  m_commandListOffset;
		};

		//! Structure from which allocations for mesh processing job related data will be made.
		struct JobDataAllocator
		{
			U8 *m_pCurrent;    ///< Pointer from which new job data will try to be allocated from.
			U8 *m_pEnd;        ///< Pointer to the end of the curret available memory for job data.

			//! If an allocation at m_pCurrent goes beyond m_pEnd, then the function stored in this pointer is called.
			/*! This pointer can be set to NULL in which case the code should assert if it needs to be called.
			*/
			bool (*m_outOfMemoryCallback)(U8 **jobData, U8 **jobDataEnd, U32 size);

			//! Used to reserve memory for new job data.  Size is in bytes.
			bool Reserve(U32 size)
			{
				// Check to see if the desired memory can be allocated.
				if (m_pCurrent + size > m_pEnd)
				{
					// If not call the callback to (hopefully) get more memory.
					ICE_ASSERT(m_outOfMemoryCallback != NULL);
					return m_outOfMemoryCallback(&m_pCurrent, &m_pEnd, size);
				}
				return true;
			}
		};

		enum MeshOutputBufferType
		{
			kMeshOutputSingleBuffer = 0,
			kMeshOutputRingBuffers = 1
		};

		//! This structure contains info about where the mesh processing frontend should put information.
		/*! Memory for the push buffer is allocated through a push buffer CommandContextData.
		    Memory for the job data is allocated through a JobDataAllocator.  The current address in the job data
		    allocator MUST be 16 byte aligned, but should be 64 byte aligned for better performance.
		*/
		struct OutputLocations
		{
			//! Pointer to the SPU module handle for the mesh processing SPU module.
			SpuModuleHandle *m_spuModule;
			//! Pointer to the job list.
			SingleThreadJobList *m_jobList;
			//! Job data allocator used to allocate data for small data tables, job info, DMA tags, and command list.
			JobDataAllocator *m_jobDataAllocator;
			//! Pointer to the command context used to allocate and output the push buffer contents.
			/*! If NULL is specified, the default Ice::Render::g_currentCommandContext is used instead.
			*/
			Render::CommandContextData *m_commandContext;
			//! The type of the mesh processing output buffer (single combined big buffer, or multiple ring buffers).
			U32 m_meshOutputBufferType;
			//! Pointer to the memory allocator to be used for mesh processing output.
			/*! If the mesh output buffer is of type kMeshOutputSingleBuffer then this points to a 128 byte
			    mutex used to allocate memory in the single buffer.  Otherwise, if the mesh output buffer
			    is of type kMeshOutputRingBuffers then this points to a set of six ring buffer allocators
			    one for each SPU).  These allocators are contiguous and must be 16 byte aligned.
			*/
			void *m_meshOutputBufferAllocator;
			//! Offset between where mesh processing will output data and where the GPU will see the data.
			/*! This offset is used by mesh processing when it writes addresses into the push buffer.  It both handles the case
			    when the mesh output buffer is moved between when mesh processing outputs and when the GPU consumes it as well
			    as the case that the GPU sees addresses differently than mesh processing.  Typically this would be something like:
			    Ice::Render::TranslateOffsetToAddress(0) + meshOutputBufPtrNow - meshOutputBufPtrLater.
			*/
			U32 m_meshOutputBufferOffset;
			//! Offset between the where the push buffer is now and where mesh processing will eventually patch the push buffer.
			/*! This is used when the push buffer is copied between the time the holes are made and when mesh processing fills
			    the holes.  Typically this should be something like: pushBufferPtrLater - pushBufferPtrNow.
			*/
			U32 m_cmdBufferOffset;
			//! Pointer to the mutex that is used to synchronize the gpu and the spus. 
			/*! The mutex is required to be 256 bytes in size and also 128 byte aligned.
			    Setting this to 0 will disable GPU synchronization.
			*/
			MeshProc::GpuSyncMutex *m_gpuSyncMutex;
		};

		//! This structure contains info about where the mesh processing frontend should put data relating to stencil shadows.
		struct ShadowOutputLocations
		{
			Render::CommandContextData *m_profileCommandContext[4];     ///< Command context pointers for the profile edges.
			Render::CommandContextData *m_frontCapCommandContext[4];    ///< Command context pointers for the front caps.
			Render::CommandContextData *m_backCapCommandContext[4];     ///< Command context pointers for the end caps.
		};

		//! This structure contains format information on a single vertex data array.
		/*! 4-byte aligned.
		    Must be sorted by hardware index. 
		*/
		struct AttributeArrayFormatInfo
		{
			//! The Gpu attribute index the array is mapped to.
			U16 m_hardwareIndex;
			//! The stride in bytes of a single vertex.
			U8 m_elementStride;
			//! The number of components in the attribute. 
			/*! Ice::Render::AttribCount */
			U8 m_attribCount : 4;
			//! The type of the attribute. 
			/*! Ice::Render::AttribType */
			U8 m_attribType : 4;
		};

		//! Setup the Attribute Array Formats for the inputs of the vertex program.
		/*! \param attribFormatInfo  An array of information required to set up the Gpu correctly.
		    \param attribCount       The size of the format info array.
		    \param commandContext    Information on where and how to allocate memory for the push buffer.
		*/
		void SetupArrayFormats(AttributeArrayFormatInfo const * const attribFormatInfo,
			U8 const attribCount,
			Ice::Render::CommandContextData * __restrict const commandContext = Ice::Render::g_currentCommandContext);

		//! Setup the Attribute Array Formats for the inputs of the vertex program for stencil shadows.
		/*! \param commandContext    Information on where and how to allocate memory for the push buffer.
		*/
		void SetupShadowArrayFormats(Ice::Render::CommandContextData * __restrict const commandContext = Ice::Render::g_currentCommandContext);

		//! Sets up the push buffer contents for a vertex set that is to be rendered directly on the GPU.
		/*! \param vertexSet                  The vertex set to render.
		    \param pVertexInputInfo           Pointer to information about the vertex attributes.
		    \param pMatrices                  Pointer to the skinning matrices.
		    \param gputSkinningConstantIndex  GPU index at which to start placeing constants for skinning.
		    \param commandContext             Information on where and how to allocate memory for the push buffer.
		*/
		void RenderGpuVertexSet(
			GpuVertexSet const * const vertexSet,
			VertexInputInfo const * const pVertexInputInfo,
			F32 const * const pMatrices,
			U8 const matrixFormat,
			U32 const gpuSkinningConstantIndex,
			Ice::Render::CommandContextData * __restrict const commandContextData = Ice::Render::g_currentCommandContext);

		//! Sets up all of the mesh processing job data and push buffer contents for a vertex set that uses 
		//! no CLOD, optional skinning, optional trimming, and optional blend shapes.
		//! Skinning is performed if skinning bits are present in the mesh program info.
		//! Trimming is performed if the kDoTrimming flag is set in the processing flags.
		//! Blend shapes are used if the pointer to the blend shapes is valid.
		//! If inputInfo->m_pMeshProgramInfo == NULL, then the vertex set is routed directly to the GPU.
		/*! \param vertexSet    The vertex set to render.
		    \param blendShapes  Various Blend Shape information.
		    \param inputInfo    Configuration information required to generate the output data.
		    \param outputLocs   The information of where to put the output of the function.
		    \param cutAndPaste  A pointer to a structure where information needed to quickly modify the generated data is put.
		*/
		void RenderBasicVertexSet(
			BasicVertexSet const * const vertexSet, 
			VertexSetBlendShapes const * const blendShapes,
			InputInfo const * const inputInfo,
			OutputLocations * __restrict const outputLocs,
			CutAndPasteInfo * __restrict const cutAndPaste);

		//! Sets up all of the mesh processing job data and push buffer contents for a vertex set that uses 
		//! discrete PM, optional skinning, optional trimming, and optional blend shapes.
		//! Skinning is performed if skinning bits are present in the mesh program info.
		//! Trimming is performed if the kDoTrimming flag is set in the processing flags.
		//! Blend shapes are used if the pointer to the blend shapes is valid.
		//! If inputInfo->m_pMeshProgramInfo == NULL, then the vertex set is routed directly to the GPU.
		/*! \param vertexSet    The vertex set to render.
		    \param blendShapes  Various Blend Shape information.
		    \param inputInfo    Configuration information required to generate the output data.
		    \param outputLocs   The information of where to put the output of the function.
		    \param cutAndPaste  A pointer to a structure where information needed to quickly modify the generated data is put.
		*/
		void RenderDiscretePmVertexSet(
			PmVertexSet const * const vertexSet, 
			VertexSetBlendShapes const * const blendShapes,
			InputInfo const * const inputInfo,
			OutputLocations * __restrict const outputLocs,
			CutAndPasteInfo * __restrict const cutAndPaste);

		//! Sets up all of the mesh processing job data and push buffer contents for a vertex set that uses 
		//! continuous PM, and optional trimming.
		//! Trimming is performed if the kDoTrimming flag is set in the processing flags.
		//! If inputInfo->m_pMeshProgramInfo == NULL, then the vertex set is routed directly to the GPU.
		/*! \param vertexSet    The vertex set to render.
		    \param inputInfo    Configuration information required to generate the output data.
		    \param outputLocs   The information of where to put the output of the function.
		    \param cutAndPaste  A pointer to a structure where information needed to quickly modify the generated data is put.
		*/
		void RenderContinuousPmVertexSet(
			PmVertexSet const * const vertexSet, 
			InputInfo const * const inputInfo,
			OutputLocations * __restrict const outputLocs,
			CutAndPasteInfo * __restrict const cutAndPaste);

		//! Sets up all of the mesh processing job data and push buffer contents for a vertex set that uses 
		//! displacement mapping, optional skinning, and optional trimming.
		//! Skinning is performed if skinning bits are present in the mesh program info.
		//! Trimming is performed if the kDoTrimming flag is set in the processing flags.
		//! Blend shapes are not currently supported, but maybe in the future.
		/*! \param vertexSet    The vertex set to render.
		    \param blendShapes  Various Blend Shape information. NOT CURRENTLY SUPPORTED.  PLEASE PASS NULL.
		    \param inputInfo    Configuration information required to generate the output data.
		    \param outputLocs   The information of where to put the output of the function.
		    \param cutAndPaste  A pointer to a structure where information needed to quickly modify the generated data is put.
		*/
		void RenderDmVertexSet(
			PmVertexSet const * const vertexSet, 
			VertexSetBlendShapes const * const blendShapes,
			InputInfo const * const inputInfo,
			OutputLocations * __restrict const outputLocs,
			CutAndPasteInfo * __restrict const cutAndPaste);

		//! Sets up all of the mesh processing job data and push buffer contents for a vertex set that uses 
		//! a provided mesh program.
		//! It is necessary to set the kDoTrimming flag in the processing flags if trimming is to be done.
		//! It is necessary to set the kDoShadowVolumeExtrusion flag in the processing flags if stencil shadows are to be used.
		//! Blend shapes are used if the pointer to the blend shapes is valid.
		//! If inputInfo->m_pMeshProcCommandList == NULL, then the vertex set is routed directly to the GPU.
		/*! \param vertexSet         The vertex set to render.
		    \param blendShapes       Various Blend Shape information.
		    \param inputInfo         Configuration information required to generate the output data.
		    \param outputLocs        The information of where to put the output of the function.
		    \param shadowOutputLocs  The information of where to put the shadow push buffer output of the function.
		*/
		void RenderGeneralPmVertexSet(
			PmVertexSet const * const vertexSet, 
			VertexSetBlendShapes const * const blendShapes,
			InputInfo const * const inputInfo,
			OutputLocations * __restrict const outputLocs,
			ShadowOutputLocations * __restrict const shadowOutputLocs);

		//! Takes the previously generated mesh processing job data and quickly patches it given current information.
		/*! \param inputInfo    Configuration information required to generate the output data.
		    \param outputLocs   The information of where to put the output of the function.
		    \param cutAndPaste  A pointer to a structure where information needed to quickly modify the generated data is put.
		*/
		void CutAndPasteVertexSet(
			InputInfo const * const inputInfo,
			OutputLocations * __restrict const outputLocs,
			CutAndPasteInfo const * const cutAndPaste);

	}
}

#endif

