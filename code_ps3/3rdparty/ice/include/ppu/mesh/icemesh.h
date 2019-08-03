/*
 * Copyright (c) 2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 *
 * This file contains the public interface to the mesh processor.
 */

#ifndef ICE_MESH_H
#define ICE_MESH_H

#include "icebase.h"

#if ICEDEBUG
#define COLLECT_MESH_STATS 0
#else
#define COLLECT_MESH_STATS 0
#endif

// If you want to use stencil shadows, please turn on this flag.
#define ICE_MESH_STENCIL_SHADOWS_ON 1

namespace Ice
{
	namespace MeshProc
	{
		/**
		 * Size of the buffers in SPU/main memory depending on how the code
		 * is compiled.
		 */
		const U32 kMeshInputBufferSize = 32 * 1024;     // Will probably be 40KB
		const U32 kMeshWorkBufferSize = 64 * 1024;      // Will probably be 72KB
#if WWS_JOB_USE_C_VERSION
		const U32 kMeshOutputBufferSize = 20 * 1024;    // Will probably be 32KB
		const U32 kMeshInputBufferSetOrg = 112 * 1024;
#else
		const U32 kMeshOutputBufferSize = 30 * 1024;
		const U32 kMeshInputBufferSetOrg = 88 * 1024;
#endif
		const U32 kMeshWorkBufferSetOrg = kMeshInputBufferSetOrg + kMeshInputBufferSize;
		const U32 kMeshOutputBufferSetOrg = kMeshWorkBufferSetOrg + kMeshWorkBufferSize;

		enum
		{
			kMeshCodeBufferSetNum = 0,
			kMeshInputBufferSetNum,
			kMeshWorkBufferSetNum,
			kMeshOutputBufferSetNum
		};

		/**
		 * Enumeration of commands.  Commands are ordered by how long the
		 * memory they allocate is needed (the longer a block of data is needed,
		 * the earlier the command is listed).  Unless otherwise noted,
		 * it's a good assumption that given any two commands, the command
		 * that comes earlier in the list should ALWAYS be called first.
		 */
		enum
		{
			// Special value to indicate the end of the command list:
			kCmdCleanupAndExit              = 0,

			// Flow control commands (these can appear anywhere):
			kCmdJump                        = 1,
			kCmdCall                        = 2,
			kCmdReturn                      = 3,

			// Transfer functions whose data is needed for the entire
			// process:
			kCmdSetupNvControlInfo          = 4,
			kCmdSetupObjectInfo             = 5,
			kCmdSetupVertexInfo             = 6,
			kCmdSetupPmVertexInfo           = 7,
			kCmdSetupFormatInfo             = 8,
			kCmdSetupNvStream               = 9,
			kCmdSetupSwStream               = 10,
			kCmdSetupCustomCompress         = 11,
			kCmdSetupIndexes                = 12,
			kCmdSetupPixelShader            = 13,

			// Transfer functions whose data is needed until the end of
			// vertex/index trimming:
			kCmdSetupViewportInfo           = 14,
			kCmdSetupRootTransformInfo      = 15,

			// Transfer functions whose data is needed until the end of
			// stencil shadows:
			kCmdSetupShadowInfo             = 16,
			kCmdSetupEdges                  = 17,

			// Transfer functions whose data is needed until the end of
			// CLOD calculations:
			kCmdSetupContinuousPmInfo       = 18,
			kCmdSetupDiscretePmInfo         = 19,
			kCmdSetupPmParent               = 20,

			// Transfer functions whose data is needed until the end of
			// DM calculations
			kCmdSetupDmInfo                 = 21,
			kCmdSetupDmDisplacements        = 22,

			// Transfer functions whose data is needed until the end of
			// skinning:
			kCmdSetupSkinning               = 23,
			kCmdSetupMatrices               = 24,

			// Blend shape commands.  These are the ONLY commands
			// (other than the flow control commands) which may be
			// called outside of their strict numerical order, and only
			// relative to each other (if you have several DMA-loads
			// full of blend shapes, you may need to trigger/blend/stall
			// multiple times):
			kCmdStartInputListDma           = 25,
			kCmdSetupRunTable               = 26,
			kCmdSetupDeltaFormatInfo        = 27,
			kCmdBlendDeltaStream            = 28,
			kCmdStallOnInputDma             = 29,

			// Main mesh processing commands; need to be called in this
			// order:
			kCmdEndSetup                    = 30,
			kCmdPerformDm                   = 31,
			kCmdSkinObject                  = 32,
			kCmdPerformPm                   = 33,
			kCmdExtrudeShadows              = 34,
			kCmdTrimIndexes                 = 35,
			kCmdTrimVertexes                = 36,

			// Output stage commands:
			kCmdOutputIndexes               = 37,
			kCmdInsertAttributeIntoNvStream = 38,
			kCmdOutputCopiedNvStream        = 39,
			kCmdOutputConvertedUniformTable = 40,

			// no more commands below here!
			kCmdNumCommands
		};

		/**
		 * List of decompression formats for various data types.
		 */

		enum IndexFormat
		{
			kFormatIndex3U16 = 0,
			kFormatIndex2U16 = 1,
		};

		enum SkinningWeightsFormat
		{
			kFormatWeightU8  = 0,
			kFormatWeightF32 = 1,
		};

		enum MatrixFormat
		{
			kFormatMatrix44 = 0,
			kFormatMatrix43 = 1,
		};

		enum PmParentFormat
		{
			kFormatPmParentU16 = 0,
		};

		enum EdgeFormat
		{
			kFormatEdgeU32 = 0,
		};

		enum DisplacementFormat
		{
			kFormatDmDispF32 = 0,
		};

		/**
		 * List of supported data element types
		 */
		enum
		{
			kF32 = 0,
			kF16,
			kI16n,
			kI16,
			kX11Y11Z10n,
			kU8n,
			kU8,
			kNumTypes
		};

		/**
		* Some attribute IDs have a specific meaning.
		* Here are their enumerations.
		*/
		enum
		{
			kPositionId = 1,
			kNormalId = 2,
			kTangentId = 3,
			kDispNormalId = 4,
			kOtherIdStart = 5
		};

		/**
		* Enumerations for scale and bias flags.
		*/
		enum
		{
			kScaleAndBiasOn = 0x80,
			kScaleAndBiasVariable = 0x40,
			kHasIntoff = 0x20
		};

		/**
		 * Each job manager audit system needs an independent ID
		 */
		const U32 kMeshAuditSystem = 1;

		/**
		 * List of possible job manager audit types
		 */
		enum
		{
			kMeshAuditCmd = 0,
			kMeshAuditCmdStart,
			kMeshAuditStart,
			kMeshAuditReserveBuffers,
			kMeshAuditInputBuffer,
			kMeshAuditWorkBuffer,
			kMeshAuditEnd
		};

		/**
		 * High-level routine to invoke the mesh processor.
		 *
		 * @param pSpud    Pointer to SPU DMA list.
		 * @param spudSize Size (in bytes) of the SPU DMA list.
		 * @param spuNum   The number of the SPU that this job is running on.
		 * @param jobNum   This job's number.
		 */
		void MeshProcessing(U32 *pSpud, U32 spudSize, U32 spuNum, U32 jobNum);

		/// Resets all of the Mesh Processing stats.
		void ClearMeshProcessingStats();

		/// Prints out the Mesh Processing stats in a nice format.
		void PrintMeshProcessingStats();
	}
}

#endif

