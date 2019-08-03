/*
 * Copyright (c) 2005 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 * 
 * The purpose of these functionality set is to act as an high level
 * communicator to the job manager and mesh processing. 
 *
 * Revision History:
 *  - Created 12/16/05 
 */

#ifndef ICEMESHSTRUCTS_H
#define ICEMESHSTRUCTS_H

namespace Ice
{
	//! A DMA Tag specifies data which should be copied into the Mesh Processing's input buffer.
	struct DmaTag
	{
		//! The size of the DMA tag.
		U32 m_size;
		//! The address to the data to be copied.
		U32 m_source;
	};

	namespace MeshProc
	{
		//! A Command list is a set of instructions for an SPU to follow in order to correctly process
		//! a set of information.
		struct CommandList
		{
			//! The size in bytes of the commands array.
			U16 m_length;
			//! The commands to be executed.
			U16 m_commands[0];
		};

		//! The definition of a single light's information in the context of a NVControl structure.
		/*! Size: 0x18 */
		struct NvControlLight
		{
			//! A pointer to the extruded profile edge push buffer hole that will be patched up with
			//! draw calls by the mesh processor.
			/*! Offset: 0x0 */
			U32 m_profilePBHole;
			//! The size of the extruded profile edge push buffer hole.
			/*! Offset: 0x4 */
			U32 m_profilePBHoleSize;
			//! A pointer to the front cap of the shadow volume push buffer hole that will be patched up
			//! with draw calls by the mesh processor.
			/*! Offset: 0x8 */
			U32 m_frontCapPBHole;
			//! The size of the front cap push buffer hole.
			/*! Offset: 0xC */
			U32 m_frontCapPBHoleSize;
			//! A pointer to the back cap of the shadow volume push buffer hole that will be patched up
			//! with draw calls by the mesh processor.
			/*! Offset: 0x10 */
			U32 m_backCapPBHole;
			//! The size of the back cap push buffer hole.
			/*! Offset: 0x14 */
			U32 m_backCapPBHoleSize;

		};

		//! NV Control structure
		//! Holds the information necessary to describe this command.
		/*! Size: 0x90 */
		struct NvControl
		{
			//! Each attribute's associated nvidia mapping
			/*! Offset: 0x00 */
			U8 m_attributeMap[16]; 
			//! Address of the output buffer allocator (either combined single or multiple ring).
			/*! Offset: 0x10 */
			U32 m_outputBufferAllocator;  
			//! The difference between the EA of the output buffer and the GPU offset of the output buffer.
			/*! Offset: 0x14 */
			U32 m_outputBufferOffset;
			//! The gpu synchronization mutex address.
			/*! Offset: 0x18 */
			U32 m_gpuSyncMutex;
			//! The value to be placed in the GPU's put pointer after this job is complete.
			/*! Offset: 0x1C */
			U32 m_putPtrValue;
			//! A pointer to the primary push buffer hole that will be patched up with draw calls by the
			//! mesh processor.
			/*! Offset: 0x20 */
			U32 m_primaryPbHole;
			//! The primary push buffer hole size.
			/*! Offset: 0x24 */
			U32 m_primaryPbHoleSize;
			//! Pointer to the hole in the push buffer where the commands are added to write to a GPU
			//! semaphore to be used with multiple ring output buffers.
			//! A value of 0 means that a single buffer is used instead of multiple ring buffers.
			/*! Offset: 0x28 */
			U32 m_semaphorePbHole;
			//! The number of lights to create profile edges for.
			/*! Offset: 0x2C */
			U32 m_numLights;
			//! An inline array describing the per-light information required by the mesh processing.
			/*! Offset: 0x30 */
			NvControlLight m_perLightData[4];
		};

		//! The LOD vertex table includes information on which subset of vertexes.
		//! are used in each LOD. An array of these structures is uplaoded to the SPU.
		/*! Note that this table has one 'extra' row in vertex sets belonging to 'open-ended' LOD
		    groups (those that include interpolation to the first LOD in the next LOD group).
		    This table proveds the information needed for the last LOD in the LOD group to smoothly
		    interpolate to first LOD in the next LOD group.
		    First one in array is 16-byte aligned.
		*/
		struct LodVertexTable
		{
			//! Index (into overall vertex table) of first vertex in this LOD.
			U16 m_firstVertex;
			//! Number of vertexes in this LOD (including halo vertexes).
			U16 m_vertexCount;
			//! Number of halo vertexes in this LOD.
			U16 m_haloVertexCount;
			//! In row 0, contains the number of rows in the table (not the same as the number of LODs)
			U16 m_rowCount;
		};

		//! Contains information on the viewport
		struct ViewportInfo
		{
			//! This is non-zero if 4x rotated grid multisampling is enabled.
			U8 m_4xRgmsFlag;
			//! This is non-zero if the mesh processor should invert the back facing tests.
			U8 m_reverseBackFaceFlag;
			//! This is not used.
			U8 m_padding[6];
			//! The scissor area of the viewport in the following order: left, right, top, and bottom.
			U16 m_scissorArea[4];
			//! The world-space to homogenized device coordinates transformation matrix.
			//! AKA the "view-projection matrix."
			F32 m_worldToHdcMatrix[16];
			//! The homogenized device coordinates to screen coordinates scale and offsets in the following
			//! order: h scale, h offset, v scale, v offset.
			F32 m_hdcToScreen[4];
			//! The depth bounds to test against in the following order: near plane distance, far plane distance.
			F32 m_depthBounds[2];
			//! Padding to quad word alignment.
			U8 m_padding2[8];
		};

		//! Contains the information needed for a rigid object.
		struct RigidObjectTransform
		{
			//! The object-space to world-space transformation matrix.
			F32 m_objectToWorldMatrix[16];
		};

		//! Defines the type of light that the mesh processor is using.
		enum ShadowInfoLightType
		{
			//! Directional Lights always shine in a single direction only and have no location.
			kDirectional = 0,
			//! Point lights have a location and shine in all directions.
			kPoint       = 1
		};

		//! Contains shadow related information for a single light source.
		struct ShadowInfo
		{
			//! Contains the X,Y, and Z location of a light in object space.
			F32 m_lightPosition[3];
			//! Defines the type of light. See ShadowInfoLightType enum.
			U8 m_lightType;
			//! If this is non-zero, shadow caps will be included in the output.
			U8 m_outputShadowCaps;
			//! Padding to quad word alignment.
			U8 m_padding[2];
		};

		//! Contains information required for discrete progressive meshing.
		struct DiscretePmInfo
		{
			//! The delta between the different levels of detail to blend to.
			//! 0 through 1 is the valid range. 
			F32 m_alpha;
			//! Padding to quad word alignment.
			U32 m_padding0;
			//! Padding to quad word alignment.
			U64 m_padding1;
		};

		//! Contains the near and far distances for a single level of detail.
		struct ContinuousPmInfoDistance
		{
			//! The distance at which the detail level is at its highest polygon count. 
			F32 m_near;
			//! The distance at which the detail level is at its lowest polygon count. 
			F32 m_far;
		};

		//! Contains information required for continuous progressive meshing.
		struct ContinuousPmInfo
		{
			//! The lowest level of detail that will be blended
			U32 m_lowestLodToBlend;
			//! The location of the camera in world space
			F32 m_eyeVec[3];
			//! Near and Far blend distances for each level of detail.
			ContinuousPmInfoDistance m_distances[8];
		};

		//! Contains information required for displacement mapping.
		struct DmInfo
		{
			//! The delta between the different levels of detail to blend to.
			//! 0 through 1 is the valid range. 
			F32 m_alpha;
			//! DM is either in the spawn new triangle phase or in the lofting phase.
			U16 m_phase;
			//! Which iteration of DM are we in.  This is either 1 or 2.
			U16 m_IterationCount;
			//! Padding to quad word alignment.
			U64 m_padding;
		};

		//! Required for some parts of mesh processing which need to know the sidedness of the object
		//! being processed.
		struct ObjectInfo
		{
			//! non-zero if the object is double-sided (this will disable backface culling in mesh processing)
			U8 m_isDoubleSided;
			//! Padding to quad word alignment.
			U8 m_padding[15];
		};

		//! Specifies which winding is front-facing
		enum FrontFace
		{
			//! Clockwise
			kFrontFaceCW  = 0,
			//! Counter Clockwise
			kFrontFaceCCW = 1
		};

		//! Specifies what kind of scale/bias type is enabled.
		enum ScaleBiasFlagBits
		{
			//! This attribute uses scale and bias (NV streams only).
			kScaleBiasEnabled   = 0x8,
			//! This attribute uses variable scale and bias.
			kScaleBiasVariable  = 0x4,
			//! This attribute uses intoff (SW streams only).
			kScaleBiasIntoff    = 0x2
		};

		//! Specifies the information required to decompress a nvidia stream's attribute.
		/*! 4-byte aligned */
		struct NvAttributeFormat
		{
			//! Semantic ID
			U8 m_name;
			//! The NV format of the components.
			/*! fp32, fp16, s16n, etc.
			    uses enum in icemesh.h.
			*/
			U8 m_nvFormatType : 4;
			//! The number of components (1-4) in the attribute.
			U8 m_componentCount : 4;
			//! Uses subset of ScaleBiasFlagBits enum.
			U8 m_scaleBiasFlags : 4;
			//! If uses scale and bias, indexes into the scale/bias locator table.
			U8 m_scaleBiasIndex : 4;
			//! The byte offset of this attribute within the stream.
			U8 m_attributeOffset;
		};

		//! Specifies the information required to decompress a nvidia stream.
		/*! 4-byte aligned
		    This is immediately followed by the scale/bias locator table: 
		    U16 offsets into the appropriate stream info.
		*/
		struct NvStreamFormat
		{
			//! The number of attributes in this stream.
			U16 m_attributeCount;
			//! The size of one whole element in the stream, in bytes.
			U16 m_elementSize;
			//! Variable sized table of attribute formats.
			NvAttributeFormat m_attributeFormat[0];
		};
		// NOTE: m_attributeFormat is an array with one entry per attribute.
		// This structure is immediately followed by the scale/bias locator table, which contains a U16 offset for each
		// attribute that uses scale and bias.  This offset points (from the start of the FixedStreamFormatInfo structure)
		// to the scale and bias values used for the corresponding attribute.
		// This table may be followed by a 2 byte pad in order to bring the structure to 4 byte alignment.

		//! For fixed number of bits streams.
		/*! 16-byte aligned
		    This is immediately followed by the NvStreamFormats, then by the fixed scale/bias component values.
		    The fixed scale/bias values are ordered: a scale float for each component of the first scaled/biased attribute, 
		    then a bias float for each component of that attribute, then the values for the next attribute, etc. 
		*/
		struct FixedStreamFormatInfo 
		{
			//! The total length of the info, in bytes.
			U16 m_length;
			//! The offsets to each NvStreamFormat 
			/*! The offset is from the start of the info. */
			U16 m_formatOffsets[0];
		};
		// NOTE: m_formatOffsets is an array of offsets to NvStreamFormat structures.  There is one entry per NV stream.
		// This may be followed by a 2 byte pad in order to reach 4 byte alignment.  This is then followed by the
		// formats for each NV stream (see NvSteamFormat above).  Following the NV stream formats are the scale and
		// bias values which are referenced in the scale/bias locator tables in each stream.  The scale and bias
		// values are ordered such that all of the scale values for one attribute come before the bias values for that
		// attribute.  The scale and bias values for other attributes follow.

		//! For Spu readable only streams.
		/*! 8-byte aligned
		    NOTE: these are not stored with the material descriptor, but with the vertex set. 
		*/
		struct SwStreamFormat
		{
			//! Semantic name, uses AttributeNames enum.
			U8 m_name;
			//! Uses subset of ScaleBiasFlagBits enum.
			U8 m_scaleBiasIntoffFlags : 4;
			//! The number of values (1-4) per array element.
			U8 m_componentCount : 4;
			//! Offset of scale/bias/intoff information in appropriate stream format info.
			U16 m_scaleBiasIntoffOffset;
			//! Bitcount of each component.
			/*! Not necessarily always of size 4! */
			U8 m_componentBitcounts[4];
		};

		//! Header for all software stream formats and related data.
		/*! 16-byte aligned
		    This is immediately followed by the variable scale/bias/intoff values.
		    The variable scale/bias/intoff values are ordered: a scale float for each component of the first scaled/biased
		    attribute, then a bias float for each component of that attribute, then a U32 intoff for each component (only if
		    intoff used for this attribute), then the values for the next attribute, etc.
		*/
		struct VariableStreamFormatInfo 
		{
			//! The total length of the info, in bytes.
			U16 m_length;
			//! Unused space.
			U16 m_padding[3];
			//! Variable sized table of SW stream formats.
			SwStreamFormat m_streamFormats[0];
		};

		//! Describes a single run of delta vertexes.
		/*! First one is 16-byte aligned. */
		struct DeltaRun 
		{
			//! The index to start blending deltas from this run.
			U16 m_startIndex : 10;
			//! The count of indexes in this run.
			U16 m_count : 6;
		};

		//! Contains the global job completion information from the perspective of a particular SPU.
		/*! Must be 16 byte aligned. */
		struct SpuCompletion
		{
			U32 m_base;                       ///< All jobs less than the base have completed.
			U32 m_completionBits[3];          ///< Completion bits for job numbers m_base to m_base + 95.
		};

		//! The GpuSyncMutex is used by the SPU to inform the GPU when data has been produced and is safe to consume.
		/*! Must be 128 byte aligned. */
		struct GpuSyncMutex
		{
			U32 m_lock;                       ///< Mutex lock (0 = unlocked).
			U32 m_reserved0[3];

			U32 m_lastJobToPut;               ///< The number of the last job to update the put pointer.  Init to 0.
			U32 m_reserved1[3];

			U32 m_putPtrPtr;                  ///< EA of the GPU's put pointer.
			U32 m_reserved2[3];

			U32 m_totalJobCount;              ///< The total number of jobs to be processed.
			U32 m_lastJobPutPtrValue;         ///< The GPU put pointer value to be written by the last job.
			U32 m_reserved3[2];

			U32 m_padding[16];                ///< Padding to 128 byte boundary.

			SpuCompletion spuCompletion[6];   ///< Job complettion information for each of the 6 SPUs.
		};

		//! Data required to perform allocations from a single combine buffer.
		/*! Must be 128 byte aligned. */
		struct SingleBufferMutex
		{
			U32 m_freeStartPtr;               ///< Pointer to the start of the free area.
			U32 m_reserved0[3];

			U32 m_endPtr;                     ///< Pointer to the end of the buffer.
			U32 m_reserved1[3];

			U32 m_sizeOfFailedAllocations;    ///< The total size in bytes of all failed allocations.
			U32 m_reserved2[3];

			U32 m_padding[20];                ///< Padding to a 128 byte boundary.
		};

		//! Data required to perform allocations from a ring buffer.
		/*! Must be 16 byte aligned. */
		struct RingBufferAllocator
		{
			U32 m_startPtr;                   ///< Pointer to the start of a ring buffer.
			U32 m_endPtr;                     ///< Pointer to the end of a ring buffer.
			U32 m_freeStartPtr;               ///< Pointer to the start of the current free area of the work buffer.
			U32 m_gpuSemaphorePtr;            ///< Pointer to the GPU semaphore where the end of of the current free
			                                  ///< can be found.
		};

		//! Each of the six of the SPUs requires its own RingBufferAllocator.
		/*! Must be 16 byte aligned. */
		typedef RingBufferAllocator RingBufferAllocators[6];
	}
}

#endif // ICEMESHSTRUCTS_H

