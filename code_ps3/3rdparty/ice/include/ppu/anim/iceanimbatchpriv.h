/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_ANIMBATCHPRIV_H
#define ICE_ANIMBATCHPRIV_H

#include "iceanimdebug.h"
#include "icebatchjob.h"

/*!
 * \file 	iceanimbatchpriv.h
 * \brief	Internal data structures used by Ice::Anim::BatchManager (iceanimbatch.h/cpp)
 * 			for batching AnimationNode data to the iceanimtask.cpp task code.
 */
namespace Ice
{
    namespace Anim
    {
		/// Maximum size we'll allow blend groups to be.
		/// Chosen so that 7 blend groups will fit into the work buffer with the key cache.
		static const U32 kMaxBlendGroupSize = 6320;

		/*!
		 * Internal structure to represent quad-word aligned data for flipping purposes.
		 * Format is 2 U16's (4 data are needed for quad-word alignment).
		 */
		struct FlipBinaryData {
			U16 m_offsetA;			//!< joint_index_A * 0x30
			U16 m_offsetB;			//!< joint_index_B * 0x30
		};
		/*!
		 * Internal structure to represent quad-word aligned data for flipping purposes.
		 * Format is 2 U16's in a pair (4 data are needed for quad-word alignment).
		 */
		struct FlipUnaryPair {
			U16 m_offset[2];		//!< joint_index * 0x30
		};

		//===================================================================================================================

		/*!
		 * WorkBuffer allocation types:
		 */
		enum WorkBufferAllocationType {
			kWorkTypeJointParams,			// type JointParams, size 0x30 per element
			kWorkTypeFloatChannel,			// type float, size 0x10 per 4 elements
			kWorkTypeBlendState,			// safe location and ValidBits (size 0x40 or 0x20)
			kWorkTypeKeyCache,				// type Vec4/qword pair, size 0x20 per element
			kWorkTypeTweenFactorCache,		// type float, size 0x10 per 4 elements
			kWorkTypeBlendFactor,			// type float, size 0x10 per 4 elements
			kWorkTypeOffset,				// type U16, size 0x10 per 8 elements
			kWorkTypeJointTransform,		// type JointTransform, size 0x40 per element
			kWorkTypeScalar,				// type Scalar, size 0x10 per element
			kWorkTypeVector,				// type Vec4, size 0x10 per element
			kWorkTypeMatrix3x4,				// type Mat34, size 0x30 per element
			kWorkTypeMatrix4x4,				// type Mat44, size 0x40 per element
			kWorkTypePlugInCode,			// type code
			kWorkTypeCustom,				// start of custom types and sizes
		};

		//===================================================================================================================

		/*!
		 * Assorted buffer sizes in SPU local memory.
		 */
		enum LocalStoreBufferSize
		{
			kInputBufferSize  = 24 * 1024,
			kWorkBufferSize   = 48 * 1024,
			kOutputBufferSize = 24 * 1024,
			kTotalBufferSize  = kInputBufferSize*Ice::BatchJob::kNumInputBuffers + kWorkBufferSize*Ice::BatchJob::kNumWorkBuffers + kOutputBufferSize*Ice::BatchJob::kNumOutputBuffers
		};
		/*!
		 * Maximum size of an output Dma list:
		 */
		const U32 kDmaOutputListMaxSize = 16*2*sizeof(U32);
		/*!
		 * Resulting maximum size of an output DMA:
		 */
		const U32 kDmaOutputMaxSize = kOutputBufferSize - kDmaOutputListMaxSize;

		/*!
		 * Assorted buffer offsets in SPU local memory relative to start of task owned buffer.
		 *
		 * Note that only the task setup should depend on these, since we now use LocalStoreHandles
		 * to address local store memory.
		 */
		enum LocalStoreBufferOffset
		{
			kInputBufferOffset   = 0,
			kInputBufferOffset0  = kInputBufferOffset,
			kInputBufferOffset1  = kInputBufferOffset0 + kInputBufferSize,
			kWorkBufferOffset    = kInputBufferOffset1 + kInputBufferSize,
			kOutputBufferOffset  = kWorkBufferOffset + kWorkBufferSize,
			kOutputBufferOffset0 = kOutputBufferOffset,
			kOutputBufferOffset1 = kOutputBufferOffset0 + kOutputBufferSize
		};

		// ===================================================================================================================

		/*!
		 * Batch command list
		 */
		enum BatchCommand
		{
			// control commands - required by low level CommandStream interface; some implemented directly within dispatcher
//			/*00*/ kCmdEnd = 						Ice::BatchJob::kCmdEnd,
//			/*01*/ kCmdSwap = 						Ice::BatchJob::kCmdSwap,
//			/*02*/ kCmdReserveOutputBuffer = 		Ice::BatchJob::kCmdReserveOutputBuffer,
//			/*03*/ kCmdPluginLoad = 				Ice::BatchJob::kCmdPluginLoad,
//			/*04*/ kCmdPluginExecute = 				Ice::BatchJob::kCmdPluginExecute,
//			/*05*/ kCmdPluginUnload = 				Ice::BatchJob::kCmdPluginUnload,
//			/*06*/ kCmdCopyQuadwords = 				Ice::BatchJob::kCmdCopyQuadwords,

			// other utility functions
			/*07*/ kCmdCopyBytes =					Ice::BatchJob::kNumCoreCommands,
			/*08*/ kCmdUnpackVec4Components,

			// clip commands
			/*09*/ kCmdInitAnimClipGroup,
			// copy to keycache commands
			/*0a*/ kCmdCopyUniformDataToKeyCache,
			/*0b*/ kCmdCopyUniformDataToKeyCacheFloats,
			/*0c*/ kCmdFindKeyAndFillKeyCache,
			/*0d*/ kCmdFindKeyAndFillKeyCacheFloats,
			// copy from keycache commands
			/*0e*/ kCmdKeyCacheUniformLerp,
			/*0f*/ kCmdKeyCacheUniformSlerp,
			/*10*/ kCmdKeyCacheUniformLerpFloats,
			/*11*/ kCmdKeyCacheNonUniformLerp,
			/*12*/ kCmdKeyCacheNonUniformSlerp,
			/*13*/ kCmdKeyCacheNonUniformLerpFloats,
			// constant decompression commands
			/*14*/ kCmdConstDecompress3VecUncompressed,
			/*15*/ kCmdConstDecompress3VecFloat16,
			/*16*/ kCmdConstDecompressQuatUncompressed,
			/*17*/ kCmdConstDecompressQuat48SmallestThree,
			/*18*/ kCmdConstDecompressFloatUncompressed,
			// key cache decompression commands
			/*19*/ kCmdDecompress3VecFloat16,
			/*1a*/ kCmdDecompress3VecRange,
			/*1b*/ kCmdDecompressQuatSmallestThree,
			/*1c*/ kCmdPostDecompressQuatLog,		// run after any 3Vec decompression to perform in Qmean * exp(V)
			/*1d*/ kCmdPostDecompressQuatLogPca,	// run after any 3Vec decompression to multiply in Qpre * exp(V) * Qpost

			// blending and joint param manipulation commands
			/*1e*/ kCmdFlip,
			/*1f*/ kCmdBlendJoints,
			/*20*/ kCmdBlendJointsWithFactors,
			/*21*/ kCmdBlendFloats,
			/*22*/ kCmdBlendFloatsWithFactors,
			/*23*/ kCmdFillInUndefined,
			/*24*/ kCmdFillInUndefinedFloats,

			// parenting commands
			/*25*/ kCmdParenting,

			// finalizing commands
			/*26*/ kCmdFinalize,
			/*27*/ kCmdPrepare,
			/*28*/ kCmdFinalizeAndPrepare,

			// sdk commands
			//NOTE: The kCmdSdk* constants below must match the the order of kSdk* SdkDataType in icejointhierarchypriv.h
			/*29*/ kCmdSdkDriversRot,
			/*2a*/ kCmdSdkDrivers,
			/*2b*/ kCmdSdkCopyIn,
			/*2c*/ kCmdSdkDrivenRot,
			/*2d*/ kCmdSdkDriven,
			/*2e*/ kCmdSdkCopyOut,
			/*2f*/ kCmdSdkCopy,
			/*30*/ kCmdSdkRunBaked,
			/*31*/ kCmdSdkRunSegs,		//NOTE: used for all kSdkRunSegs1..4

			// constraint commands
			//NOTE: The kCmdConstraint* constants below must match the the order of kConstraint* ConstraintTypes in icejointhierarchypriv.h
			/*32*/ kCmdConstraintParentPos,
			/*33*/ kCmdConstraintParentRot,
			/*34*/ kCmdConstraintPoint,
			/*35*/ kCmdConstraintOrient,
			/*36*/ kCmdConstraintAim,
			/*37*/ kCmdPatchConstraintData,

			// Other useful values:
			kNumCommands,
			kClipCmd0				= kCmdCopyUniformDataToKeyCache,
			kClipReadCmd0			= kCmdCopyUniformDataToKeyCache,
			kClipWriteCmd0			= kCmdKeyCacheUniformLerp,
			kClipConstCmd0			= kCmdConstDecompress3VecUncompressed,
			kClipConstFloatCmd0		= kCmdConstDecompressFloatUncompressed,
			kClipDecompressCmd0		= kCmdDecompress3VecFloat16,
			kConstraintCmd0			= kCmdConstraintParentPos,
			kSdkCmd0				= kCmdSdkDriversRot,
			kNumClipCommands		= kCmdFlip - kClipCmd0,

			// Extra Audited commands:
			/*36*/ kSubCmdInit		= kNumCommands,
			/*37*/ kSubCmdBlendNlerp,
			/*38*/ kSubCmdBlendSlerp,
			/*39*/ kSubCmdBlendAdditive,
			/*3a*/ kSubCmdBlendMultiply,
			/*3b*/ kSubCmdBlendCopy,
			/*3c*/ kSubCmdExpandChannelFactors,
			/*3d*/ kSubCmdExpandValidBits,
			kNumAuditCommands,
		};

		enum BatchCommandSize // == 1 + num params
		{
			// control commands
//			kCmdSizeEnd = 								Ice::BatchJob::kCmdSizeEnd,
//			kCmdSizeSwap =							   	Ice::BatchJob::kCmdSizeSwap,
//			kCmdSizeReserveOutputBuffer =				Ice::BatchJob::kCmdSizeReserveOutputBuffer,
//			kCmdSizePluginLoad =						Ice::BatchJob::kCmdSizePluginLoad,
//			kCmdSizePluginExecute =						Ice::BatchJob::kCmdSizePluginExecute,
//			kCmdSizePluginUnload =						Ice::BatchJob::kCmdSizePluginUnload,
//			kCmdSizeCopyQuadwords =						Ice::BatchJob::kCmdSizeCopyQuadwords,
			// other utility commands
			kCmdSizeCopyBytes =							5,
			kCmdSizeUnpackVec4Components =				5,
			// clip commands
			kCmdSizeInitAnimClipGroup =					5,
			// copy to keycache commands
			kCmdSizeCopyUniformDataToKeyCache =			6,
			kCmdSizeCopyUniformDataToKeyCacheFloats =	6,
			kCmdSizeFindKeyAndFillKeyCache =			9,
			kCmdSizeFindKeyAndFillKeyCacheFloats =		9,
			// copy from keycache commands
			kCmdSizeKeyCacheUniformLerp =				7,
			kCmdSizeKeyCacheUniformSlerp =				7,
			kCmdSizeKeyCacheUniformLerpFloats =			7,
			kCmdSizeKeyCacheNonUniformLerp =			6,
			kCmdSizeKeyCacheNonUniformSlerp =			6,
			kCmdSizeKeyCacheNonUniformLerpFloats =		6,
			// constant decompression commands
			kCmdSizeConstDecompress =					4,
			kCmdSizeConstDecompress3VecUncompressed =	kCmdSizeConstDecompress,
			kCmdSizeConstDecompress3VecFloat16 =		kCmdSizeConstDecompress,
			kCmdSizeConstDecompressQuatUncompressed =	kCmdSizeConstDecompress,
			kCmdSizeConstDecompressQuat48SmallestThree =kCmdSizeConstDecompress,
			kCmdSizeConstDecompressFloatUncompressed =	kCmdSizeConstDecompress,
			// key cache decompression commands
			kCmdSizeDecompress =						kCmdSizeConstDecompress,
			kCmdSizeDecompress3VecFloat16 =				kCmdSizeDecompress,
			kCmdSizeDecompress3VecRange =				kCmdSizeDecompress,
			kCmdSizeDecompressQuatSmallestThree =		kCmdSizeDecompress,
			kCmdSizePostDecompressQuatLog =				kCmdSizeDecompress,
			kCmdSizePostDecompressQuatLogPca =			kCmdSizeDecompress,
			// blending and joint param manipulation commands
			kCmdSizeFlip =								5,
			kCmdSizeBlendJoints =						7,
			kCmdSizeBlendJointsWithFactors =			9,
			kCmdSizeBlendFloats =						7,
			kCmdSizeBlendFloatsWithFactors =			9,
			kCmdSizeFillInUndefined =					5,
			kCmdSizeFillInUndefinedFloats =				4,
			// parenting commands
			kCmdSizeParenting =							5,
			// finalizing commands
			kCmdSizeFinalize =							5,
			kCmdSizePrepare =							5,
			kCmdSizeFinalizeAndPrepare =				6,
			// sdk commands
			kCmdSizeSdkJoint =							5,
			kCmdSizeSdkScalar =							4,
			kCmdSizeSdkDriversRot =						kCmdSizeSdkJoint,
			kCmdSizeSdkDrivers =						kCmdSizeSdkJoint,
			kCmdSizeSdkCopyIn =							kCmdSizeSdkScalar,
			kCmdSizeSdkDrivenRot =						kCmdSizeSdkJoint,
			kCmdSizeSdkDriven =							kCmdSizeSdkJoint,
			kCmdSizeSdkCopyOut =						kCmdSizeSdkScalar,
			kCmdSizeSdkCopy =							kCmdSizeSdkScalar,
			kCmdSizeSdkRunBaked =						kCmdSizeSdkScalar,
			kCmdSizeSdkRunSegs =						5,
			// constraint commands
			kCmdSizeConstraint =						5,
			kCmdSizeConstraintParentPos =				kCmdSizeConstraint,
			kCmdSizeConstraintParentRot =				kCmdSizeConstraint,
			kCmdSizeConstraintPoint =					kCmdSizeConstraint,
			kCmdSizeConstraintOrient =					kCmdSizeConstraint,
			kCmdSizeConstraintAim =						kCmdSizeConstraint,
			kCmdSizePatchConstraintData =				6,
		};

		//===================================================================================================================

		/**
		 * Each job manager audit system needs an independent ID
		 */
		const U32 kAuditSystem = 2;

#if BATCHJOB_DPRINTS || ANIMJOB_DISPATCHER_DPRINTS || (ANIMJOB_DISPATCHER_AUDITS && !ICE_TARGET_PS3_SPU)
		static const char *s_szCommandNames[] = {
			"End",
			"Swap",
			"ReserveOutputBuffer",
			"PluginLoad",
			"PluginExecute",
			"PluginUnload",
			"CopyQuadwords",
			"CopyBytes",
			"UnpackVec4Components",
			"InitAnimClipGroup",
			"CopyUniformDataToKeyCache",
			"CopyUniformDataToKeyCacheFloats",
			"FindKeyAndFillKeyCache",
			"FindKeyAndFillKeyCacheFloats",
			"KeyCacheUniformLerp",
			"KeyCacheUniformSlerp",
			"KeyCacheUniformLerpFloats",
			"KeyCacheNonUniformLerp",
			"KeyCacheNonUniformSlerp",
			"KeyCacheNonUniformLerpFloats",
			"ConstDecompress3VecUncompressed",
			"ConstDecompress3VecFloat16",
			"ConstDecompressQuatUncompressed",
			"ConstDecompressQuat48SmallestThree",
			"ConstDecompressFloatUncompressed",
			"Decompress3VecFloat16",
			"Decompress3VecRange",
			"DecompressQuatSmallestThree",
			"PostDecompressQuatLog",
			"PostDecompressQuatLogPca",
			"Flip",
			"BlendJoints",
			"BlendJointsWithFactors",
			"BlendFloats",
			"BlendFloatsWithFactors",
			"FillInUndefined",
			"FillInUndefinedFloats",
			"Parenting",
			"Finalize",
			"Prepare",
			"FinalizeAndPrepare",
			"SdkDriversRot",
			"SdkDrivers",
			"SdkCopyIn",
			"SdkDrivenRot",
			"SdkDriven",
			"SdkCopyOut",
			"SdkCopy",
			"SdkRunBaked",
			"SdkRunSegs",
			"ConstraintParentPos",
			"ConstraintParentRot",
			"ConstraintPoint",
			"ConstraintOrient",
			"ConstraintAim",
			"PatchConstraintData",

			// Extra Audited sub-commands:
			"Sub Init",
			"Sub BlendNlerp",
			"Sub BlendSlerp",
			"Sub BlendAdditive",
			"Sub BlendMultiply",
			"Sub BlendCopy",
			"Sub ExpandChannelFactors",
			"Sub ExpandValidBits",
		};
		inline char const* DEBUG_CommandName(U16 cmd)
		{
			return (cmd < sizeof(s_szCommandNames)/sizeof(s_szCommandNames[0])) ? s_szCommandNames[cmd] : "INVALID";
		}
#elif !ICE_COMPILER_GCC
# define DEBUG_CommandName(cmd) "INVALID"
#endif
	}
};

#endif //ICE_ANIMBATCHPRIV_H
