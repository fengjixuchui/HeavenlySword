/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_JOINTANIMCORE_H
#define ICE_JOINTANIMCORE_H

#include "icebatchjobcore.h"
#include "iceanimstructs.h"

/*!
 * \file icejointanimcore.h
 * \brief Core joint animation functions implemented in SPU code.
 *
 * When animating on PPU, these functions are implemented by icejointanimcore.cpp.
 * If PPU_IMMEDIATE_FUNCTIONS is defined to 1, the old style calling convention (with
 * separate arguments rather than standardized U16 array command list arguments) version of
 * the functions will be defined.  You will also have to #include the cpp file
 * into another cpp file with the PPU_IMMEDIATE_FUNCTIONS defined first, in order
 * to compile the immediate version of the functions.  This relies on C++ function
 * overloading to resolve which function call is intended.
 */

namespace Ice
{
    namespace Anim
    {
#if	!ICE_TARGET_PS3_SPU
		// forward declarations
		class JointTransform;
		struct JointParams;
		struct ValidBits;
		struct AnimClipIndexQuad;
		struct ChannelFactor;
		struct JointParentingQuad;
		struct UnpackVec4DestOctet;
		struct SdkDriversRotSourceQuad;
		struct SdkDriversQuad;
		struct SdkDrivenRotSourceQuad;
		struct SdkDrivenRotDestQuad;
		struct SdkDrivenPair;
		struct SdkCopyScalarsQuad;
		struct ConstraintPatchQuad;

# if !PPU_IMMEDIATE_FUNCTIONS
		// Main dispatcher function for PPU batched mode, implemented in iceanimtask.cpp -> icebatchjobdispatcher.inl
		void PpuJobMain(U32 const* pInitialDmaList, U32 initialDmaListSize);
# endif
#endif

#if ICE_TARGET_PS3_SPU
		// Begin functions implemented in SPU code
		extern "C" {
#endif

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdCopyBytes.
		 *
		 * Copies unaligned data from pSource to pDest, with byte resolution.
		 */
		void CopyBytes					DISPATCHER_FN((U16 size,			// unaligned
													   void const *pSource,	// unaligned
													   void *pDest));		// unaligned

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdUnpackVec4Components.
		 *
		 * Copies aligned quadwords from pDataTable, outputing to pOutputTable with an offset and
		 * word mask per quadword read from pDestIndexTable.  The offset and mask are stored as U16's
		 * in the format (qword_offset << 4) | (write_x<<3) | (write_y<<2) | (write_z<<1) || (write_w).
		 * pDestIndexTable should be padded out to a multiple of 8 U16's with zero entries
		 * (which write no data).
		 */
		void UnpackVec4Components		DISPATCHER_FN((U16 dataTableSize, 	//== numDests*16
													   UnpackVec4DestOctet const *pDestIndexTable,
													   SMath::Vec4 const *pDataTable,
													   SMath::Vec4 *pOutputTable));

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kInitAnimClipGroup.
		 * InitAnimClipGroup and all clip decompression commands below are generated for
		 * each joint batch as a result of clipNodes in the animation task node tree.
		 *
		 * Simply copies a quadword ValidBits sequentially from pValidBits
		 * and writes them to pOutputValidBits0 then pOutputValidBits1.  May
		 * have other tasks associated with setting up an animation clip joint group in
		 * the future.
		 */
		void InitAnimClipGroup			DISPATCHER_FN((U16 numOutputValidBits,
													   ValidBits const *pValidBits,
													   ValidBits *pOutputValidBits0,
													   ValidBits *pOutputValidBits1));

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdCopyUniformDataToKeyCache.
		 *
		 * Unpacks variable sized data from pKeyFrame0Table and pKeyFrame1Table into the
		 * top size bytes of sequential quadwords of pKeyCache, one from each table per
		 * item.  Each item has a one byte size associated with it in pSizeData (or all
		 * items may share pSizeData[0] if perJointFlag is 0).  Bytes beyond the size of
		 * the item have undefined values.
		 *
		 * pSizeData may contain 0 entries, which cause key cache entries to be skipped,
		 * which is used to leave space where decompression functions may overwrite
		 * entries.  Decompression functions are guaranteed to handle no more than 8
		 * entries (4 joints) at a time.
		 */
		void CopyUniformDataToKeyCache	DISPATCHER_FN((U16 numItemsAndFlags,		// perJointFlag<<15 | numItems
													   U8 const *pSizeData,		   	// contains size of each element in bytes
													   U8 const *pKeyframe0Table,
													   U8 const *pKeyframe1Table,
													   U8 *pKeyCache));

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdCopyUniformDataToKeyCacheFloats.
		 *
		 * Unpacks variable sized data from pKeyFrame0Table and pKeyFrame1Table into the
		 * key cache.  Each four items expand into two qwords; the 4 values from the
		 * preceeding keyframe into the upper bytes of sequential words of the first
		 * qword, and the 4 values from the following keyframe into the upper bytes of
		 * sequential words of the second qword.  Each item has a one byte size
		 * associated with it in pSizeData (or all items may share pSizeData[0] if
		 * perChannelFlag is 0).  Bytes beyond the size of the item have undefined values.
		 *
		 * pSizeData must be padded to a multiple of 4 elements with 0 entries.
		 * pSizeData may contain 0 entries, which cause key cache entries to be skipped,
		 * which is used to leave space where decompression functions may overwrite
		 * entries.  Decompression functions are guaranteed to handle no more than 8
		 * entries (4 joints) at a time.
		 */
		void CopyUniformDataToKeyCacheFloats DISPATCHER_FN((U16 numItemsAndFlag,	// perChannelFlag<<15 | numItems
													   U8 const *pSizeData,			// contains size of each element in bytes
													   U8 const *pKeyframe0Table,
													   U8 const *pKeyframe1Table,
													   U8 *pKeyCache));

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdFindKeyAndFillKeyCache.
		 *
		 * For each joint, locates the samples with key times bracketting fSample in
		 * among the one byte key times at (pDataTable+keyOffset) and copies the associated
		 * variable sized key data from pDataTable into the top size bytes of sequential
		 * entries in pKeyCache, two per item.  Calculates the tween factor for fSample
		 * between the bracketing key times and stores it to sequential entries in
		 * pfTweenFactors.  Each item has a one byte size associated with it in pSizeData
		 * (or all items may share pSizeData[0] if perJointFlag is 0).  Bytes beyond the
		 * size of the item have undefined values.
		 *
		 * As in CopyUniformDataToKeyCache, pSizeData may contain 0 entries to leave space
		 * in the key cache which must be mirrored by numKeys=1 entires in pKeyTable.
		 */
		void FindKeyAndFillKeyCache		DISPATCHER_FN((U16 numItemsAndFlags,		// perJointFlag<<15 | numItems
													   U8 const *pSizeData,			// contains size of each element in bytes
													   U8 const *pKeyTable,
													   U16 dataOffset,				// offset in bytes from pKeyTable to pDataTable
													   F32 fSample,
													   U8 *pKeyCache,
													   F32 *pfTweenFactors));

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdFindKeyAndFillKeyCacheFloats.
		 *
		 * For each joint, locates the samples with key times bracketting fSample in
		 * among the one byte key times at (pDataTable+keyOffset) and copies the associated
		 * variable sized key data from pDataTable into the key cache.  Each four items
		 * expand into two qwords; the 4 values from the preceeding keyframe into the upper
		 * bytes of sequential words of the first qword, and the 4 values from the following
		 * keyframe into the upper bytes of sequential words of the second qword.  Also
		 * calculates the tween factor for fSample between the bracketing key times and
		 * stores it to sequential entries in pfTweenFactors.  Each item has a one byte size
		 * associated with it in pSizeData (or all items may share pSizeData[0] if
		 * perChannelFlag is 0).  Bytes beyond the size of the item have undefined values.
		 *
		 * pSizeData must be padded to a multiple of 4 elements with 0 entries.
		 * As in CopyUniformDataToKeyCache, pSizeData may contain 0 entries to leave space
		 * in the key cache which must be mirrored by numKeys=1 entires in pKeyTable.
		 */
		void FindKeyAndFillKeyCacheFloats DISPATCHER_FN((U16 numItemsAndFlags,		// perChannelFlag<<15 | numItems
													   U8 const *pSizeData,			// contains size of each element in bytes
													   U8 const *pKeyTable,
													   U16 dataOffset,				// offset in bytes from pKeyTable to pDataTable
													   F32 fSample,
													   U8 *pKeyCache,
													   F32 *pfTweenFactors));

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdKeyCacheUniformLerp.
		 *
		 * For each item, or pair of key cache entries, calculates Lerp(a, b, fTweenFactor)
		 * and stores it to pOutputJointParams at the 16 bit byte offset read sequentially from
		 * pDestIndexTable.  Assumes the key cache currently contains valid float 3 vectors.
		 *
		 * pDestIndexTable must be padded with the safe offset (numJointsInBlendGroup * 0x30) out
		 * to a multiple of 4 entries.  The safe offset is also used to pad the list internally
		 * wherever 0 size entries were used to leave space in the key cache.
		 */
		void KeyCacheUniformLerp		DISPATCHER_FN((U16 numItems,
													   AnimClipIndexQuad const *pDestIndexTable,
													   U8 const *pKeyCache,
													   float fTweenFactor,
													   JointParams *pOutputJointParams));

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdKeyCacheUniformSlerp.
		 *
		 * For each item, or pair of keycache entries, calculates Slerp(a, b, fTweenFactor)
		 * and stores it to pOutputJointParams at the 16 bit byte offset read sequentially from
		 * pDestIndexTable.  Assumes the key cache currently contains valid float 4 vector
		 * quaternions.
		 *
		 * pDestIndexTable must be padded with the safe offset (numJointsInBlendGroup * 0x30) out
		 * to a multiple of 4 entries.  The safe offset is also used to pad the list internally
		 * wherever 0 size entries were used to leave space in the key cache.
		 */
		void KeyCacheUniformSlerp		DISPATCHER_FN((U16 numItems,
													   AnimClipIndexQuad const *pDestIndexTable,
													   U8 const *pKeyCache,
													   float fTweenFactor,
													   JointParams *pOutputJointParams));

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdKeyCacheUniformLerpFloats.
		 *
		 * For each item, or pair of key cache entries, calculates Lerp(a[0..3], b[0..3], fTweenFactor)
		 * and stores the 4 results to pOutputFloatChannels at the 4 16 bit byte offsets read
		 * sequentially from pDestIndexTable.  Assumes the key cache currently contains valid float
		 * data packed 4 to a qword.
		 *
		 * pDestIndexTable must be padded with the safe offset ((numFloatChannelsInBlendGroup + 3) &~ 3) * 0x4
		 * out to a multiple of 4 entries.  The safe offset is also used to pad the list internally
		 * wherever 0 size entries were used to leave space in the key cache.
		 */
		void KeyCacheUniformLerpFloats	DISPATCHER_FN((U16 numItems,
													   AnimClipIndexQuad const *pDestIndexTable,
													   U8 const *pKeyCache,
													   float fTweenFactor,
													   float *pOutputFloatChannels));
		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdKeyCacheNonUniformLerp.
		 *
		 * For each item, or pair of key cache entries, calculates Lerp(a, b, pfTweenFactor[i])
		 * and stores it to pOutputJointParams at the 16 bit byte offset read sequentially from
		 * pDestIndexTable.  Assumes the key cache currently contains valid float 3 vectors,
		 * and the tween factor table has been filled out by a call to FindKeyAndFillKeyCache.
		 *
		 * pDestIndexTable must be padded with the safe offset (numJointsInBlendGroup * 0x30) out
		 * to a multiple of 4 entries.  The safe offset is also used to pad the list internally
		 * wherever 0 size entries were used to leave space in the key cache.
		 */
		void KeyCacheNonUniformLerp		DISPATCHER_FN((U16 numItems,
													   AnimClipIndexQuad const *pDestIndexTable,
													   U8 const *pKeyCache,
													   F32 const *pfTweenFactors,
                                                       JointParams *pOutputJointParams));

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdKeyCacheNonUniformSlerp.
		 *
		 * For each item, or pair of keycache entries, calculates Slerp(a, b, pfTweenFactor[i])
		 * and stores it to pOutputJointParams at the 16 bit byte offset read sequentially from
		 * pDestIndexTable.  Assumes the key cache currently contains valid float 4 vector
		 * quaternions, and the tween factor table has been filled out by a call to FindKeyAndFillKeyCache.
		 *
		 * pDestIndexTable must be padded with the safe offset (numJointsInBlendGroup * 0x30) out
		 * to a multiple of 4 entries.  The safe offset is also used to pad the list internally
		 * wherever 0 size entries were used to leave space in the key cache.
		 */
		void KeyCacheNonUniformSlerp	DISPATCHER_FN((U16 numItems,
													   AnimClipIndexQuad const *pDestIndexTable,
													   U8 const *pKeyCache,
													   F32 const *pfTweenFactors,
													   JointParams *pOutputJointParams));
		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdKeyCacheNonUniformLerpFloats.
		 *
		 * For each item, or pair of key cache entries, calculates Lerp(a[0..3], b[0..3], pfTweenFactor[i + 0..3])
		 * and stores the 4 results to pOutputFloatChannels at the 4 16 bit byte offsets read
		 * sequentially from pDestIndexTable.  Assumes the key cache currently contains valid float
		 * data packed 4 to a qword and that the tween factor table contains a tween factor for
		 * each pair of floats.
		 *
		 * pDestIndexTable must be padded with the safe offset ((numFloatChannelsInBlendGroup + 3) &~ 3) * 0x4
		 * out to a multiple of 4 entries.  The safe offset is also used to pad the list internally
		 * wherever 0 size entries were used to leave space in the key cache.
		 */
		void KeyCacheNonUniformLerpFloats DISPATCHER_FN((U16 numItems,
													   AnimClipIndexQuad const *pDestIndexTable,
													   U8 const *pKeyCache,
													   F32 const *pfTweenFactors,
													   float *pOutputFloatChannels));

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdConstDecompressQuatUncompressed.
		 *
		 * Reads data 12 bytes at a time from the data table at (pDestIndexTable + ((numItems+7)&~0x7)),
		 * and writes to pOutputTable at byte offsets read sequentially from pDestIndexTable.
		 *
		 * pDestIndexTable should be padded out to the nearest quadword in size with safe offsets
		 * (numJointsInBlendGroup * 0x30).
		 */
		void ConstDecompress3VecUncompressed DISPATCHER_FN((U16 numItems,
														   U16 const *pDestIndexTable,	// joint*0x30 + (0x00 (scale) or 0x20 (trans))
														   U8 *pOutputTable));

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdConstDecompress3VecFloat16.
		 *
		 * Reads data 6 bytes at a time from the data table at (pDestIndexTable + ((numItems+7)&~0x7)),
		 * decompresses from 3 float16 (1.5.10) to float 3 vectors, and writes the result
		 * to pOutputTable at byte offsets read sequentially from pDestIndexTable.
		 *
		 * pDestIndexTable should be padded out to the nearest quadword in size with safe offsets
		 * (numJointsInBlendGroup * 0x30).
		 */
		void ConstDecompress3VecFloat16		DISPATCHER_FN((U16 numItems,
														   U16 const *pDestIndexTable,	// joint*0x30 + (0x00 (scale) or 0x20 (trans))
														   U8 *pOutputTable));

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdConstDecompressQuatUncompressed.
		 *
		 * Reads data 16 bytes at a time from the data table at (pDestIndexTable + ((numItems+7)&~0x7)),
		 * and writes to pOutputTable at byte offsets read sequentially from pDestIndexTable.
		 *
		 * pDestIndexTable should be padded out to the nearest quadword in size with safe offsets
		 * (numJointsInBlendGroup * 0x30).
		 */
		void ConstDecompressQuatUncompressed DISPATCHER_FN((U16 numItems,
														   U16 const *pDestIndexTable,	// joint*0x30 + (0x00 (scale) or 0x10 (quat) or 0x20 (trans))
														   U8 *pOutputTable));

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command
		 * kCmdConstDecompressQuat48SmallestThree.
		 *
		 * Reads data 6 bytes at a time from the data table at (pDestIndexTable + ((numItems+7)&~0x7)),
		 * decompresses from 16-15-15-2 smallest 3 format to float 4 vector quaternions,
		 * and writes the result to pOutputTable at byte offsets read sequentially from
		 * pDestIndexTable.
		 *
		 * pDestIndexTable should be padded out to the nearest quadword in size with safe offsets
		 * (numJointsInBlendGroup * 0x30).
		 */
		void ConstDecompressQuat48SmallestThree	DISPATCHER_FN((U16 numItems,
														   U16 const *pDestIndexTable,	// joint*0x30 + 0x10
														   U8 *pOutputTable));

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdConstDecompressFloatUncompressed.
		 *
		 * Reads data 4 bytes at a time from the data table at (pDestIndexTable + ((numItems+7)&~0x7)),
		 * and writes to pOutputTable at byte offsets read sequentially from pDestIndexTable.
		 *
		 * pDestIndexTable should be padded out to the nearest quadword in size with safe offsets
		 * ((numFloatChannelsInBlendGroup + 3) &~ 3) * 0x4.  pDestIndexTable is also assumed to be
		 * ordered such that pDestIndexTable[i+4] is never in the same qword as pDestIndexTable[i].
		 */
		void ConstDecompressFloatUncompressed DISPATCHER_FN((U16 numItems,
														   U16 const *pDestIndexTable,	// float_channel * 0x4
														   U8 *pOutputTable));

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdDecompress3VecFloat16.
		 *
		 * Decompresses the first 6 bytes of numItems*2 key cache entries from 3 float16
		 * 1.5.10 to float 3 vectors, overwriting each cache entry with the result.
		 * pFormatData is unused.
		 *
		 * May operate on as many as 8 cache entries at a time, possibly overwriting
		 * as many as 3 items beyond numItems in the process.
		 */
		void Decompress3VecFloat16		DISPATCHER_FN((U16 numItemsAndFlags,			// perJointFlag<<15 | numItems
													   U8 const *pFormatData,			// NULL
													   U8 *pKeyCache));					// numItems*2 Vec4 entries

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdDecompress3VecRange.
		 *
		 * Decompresses the top format.m_numBytes of numItems*2 key cache entries from a
		 * variable bitpacked format (x-y-z) to float 3 vectors, overwriting each cache entry
		 * with the result.  pFormatData contains either one shared 4 byte entry (if
		 * perJointFlag is 0) or one entry per item (key cache pair), in the format
		 * detailed by AnimClip3VecRangeHeader in iceanimclippriv.h.
		 *
		 * May operate on as many as 8 cache entries at a time, possibly overwriting
		 * as many as 3 items beyond numItems in the process.
		 */
		void Decompress3VecRange		DISPATCHER_FN((U16 numItemsAndFlags,			// perJointFlag<<15 | numItems
													   U8 const *pFormatData,			// AnimClip3VecRangeHeader[numItems], bitFormat[ perJointFlag ? numItems : 1 ]
													   U8 *pKeyCache));					// numItems*2 Vec4 entries

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdDecompressQuatSmallestThree.
		 *
		 * Decompresses the top format.m_numBytes of numItems*2 key cache entries from a
		 * variable bitpacked smallest 3 format (x-y-z-2) to float 4 vector quaternions,
		 * overwriting each cache entry with the result.  pFormatData contains either one
		 * shared 4 byte entry (if perJointFlag is 0) or one entry per item (key cache
		 * pair), in the format detailed by AnimClip3VecRangeHeader in iceanimclippriv.h.
		 *
		 * May operate on as many as 8 cache entries at a time, possibly overwriting
		 * as many as 3 items beyond numItems in the process.
		 */
		void DecompressQuatSmallestThree DISPATCHER_FN((U16 numItemsAndFlags,			// perJointFlag<<15 | numItems
													   U8 const *pFormatData,			// bitFormat[ perJointFlag ? numItems : 1 ]
													   U8 *pKeyCache));					// numItems*2 Vec4 entries

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdPostDecompressQuatLog.
		 *
		 * Converts numItems*2 sequential float 3 vectors in the pKeyCache into
		 * 4 vector quaternions by calculating (qMean * exp(v)).  qMean is stored
		 * in pFormatData as numItems sequential entries compressed in the format
		 * specified by AnimClipQuatLogHeader, defined in iceanimclippriv.h.
		 *
		 * May operate on as many as 8 cache entries at a time, possibly overwriting
		 * as many as 3 items beyond numItems in the process.
		 */
		void PostDecompressQuatLog		DISPATCHER_FN((U16 numItemsAndFlags,			// perJointFlag<<15 | numItems
													   U8 const *pFormatData,			// AnimClipQuatLogHeader[numItems]
													   U8 *pKeyCache));					// numItems*2 Vec4 entries

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdPostDecompressQuatLogPca.
		 *
		 * Converts numItems*2 sequential float 3 vectors in the pKeyCache into
		 * 4 vector quaternions by calculating (qPre * exp(v) * qPost).  qPre and
		 * qPost are stored in pFormatData as numItems sequential entries compressed
		 * in the format specified by AnimClipQuatLogHeader, defined in iceanimclippriv.h.
		 *
		 * May operate on as many as 8 cache entries at a time, possibly overwriting
		 * as many as 3 items beyond numItems in the process.
		 */
		void PostDecompressQuatLogPca	DISPATCHER_FN((U16 numItemsAndFlags,			// perJointFlag<<15 | numItems
													   U8 const *pFormatData,			// AnimClipQuatLogHeader[numItems]
													   U8 *pKeyCache));					// numItems*2 Vec4 entries

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdFlip.
		 * Flip commands are generated per joint batch based on flip nodes in the animation node task tree.
		 *
		 * Executes the flip operations read sequentially from pFlipOpTable on the
		 * joint params in pOutputJointParams at the byte offsets listed sequentially
		 * in pIndexTable.  If binaryFlip is 0, each operation is applied in place on
		 * one joint and numFlips must be a multiple of 2 (padded with kFlipOpNop).
		 * If binaryFlip is 1, each operation is applied to two joints and the results
		 * are exchanged before writing them back to the two joints.
		 */
		void Flip						DISPATCHER_FN((U16 numFlips,	// numFlips | (binaryFlip<<15), multiple of 2 if !binaryFlip
													   U16 const *pIndexTable,
													   U32 const *pFlipOpTable,
													   JointParams *pOutputJointParams));

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch commands kCmdBlendJoints and kCmdBlendJointsWithFactors.
		 * ExecuteBlend commands are generated per joint blend group based on blend nodes in the animation node task tree.
		 *
		 * Applies a blend operation to any joints which are defined in both pLeftJointParams
		 * and pRightJointParams (based on the associated ValidBits), and copies any joints
		 * that are only defined in one.  If useChannelFactors is 1, reads numChannelFactors override
		 * blend factors for specific joints from pChannelFactors.  If outputToRight is 1,
		 * overwrites pRightJointParams with the results; otherwise, overwrites pLeftJointParams.
		 *
		 * Up to 1.75 kbytes of memory is required at pTempMemory to store temporary tables.
		 */
		void ExecuteBlend				DISPATCHER_FN((U16 blendOp, // = (numJoints<<8) | (blendMode<<2) | (useChannelFactors<<1) | outputToRight
													   JointParams *pLeftJointParams, JointParams *pRightJointParams,
													   float fBlendFactor, U8 *pTempMemory,
													   U16 numChannelFactors, ChannelFactor const *pChannelFactors));

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch commands kCmdBlendJoints and kCmdBlendJointsWithFactors.
		 * ExecuteBlend commands are generated per float channel blend group based on blend nodes in the animation node task tree.
		 *
		 * Applies a blend operation to any float channels which are defined in both pLeftFloatChannels
		 * and pRightFloatChannels (based on the associated ValidBits), and copies any float channels
		 * that are only defined in one.  If useChannelFactors is 1, reads numChannelFactors override
		 * blend factors for specific float channels from pChannelFactors.  If outputToRight is 1,
		 * overwrites pRightFloatChannels with the results; otherwise, overwrites pLeftFloatChannels.
		 *
		 * Up to 0.5 kbytes of memory is required at pTempMemory to store temporary tables.
		 */
		void ExecuteBlendFloats			DISPATCHER_FN((U16 blendOp, // = (numFloatChannels<<8) | (blendMode<<2) | (useChannelFactors<<1) | outputToRight
													   float *pLeftFloatChannels, float *pRightFloatChannels,
													   float fBlendFactor, U8 *pTempMemory,
													   U16 numChannelFactors, ChannelFactor const *pChannelFactors));

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdFillInUndefined.
		 *
		 * Copies default pose from pDefaultJointParams, overwriting any joints of
		 * pOutputJointParams that are undefined (based on the associated ValidBits).
		 *
		 * Up to 0.25 kbytes of memory is required at pTempMemory to store temporary tables.
		 */
		void FillInUndefined			DISPATCHER_FN((U16 numJoints,
													   JointParams const *pDefaultJointParams,
													   JointParams *pOutputJointParams,
													   U8 *pTempMemory));

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdFillInUndefinedFloats.
		 *
		 * Copies default pose from pDefaultFloatChannels, overwriting any float channels of
		 * pOutputFloatChannels that are undefined (based on the associated ValidBits).
		 */
		void FillInUndefinedFloats		DISPATCHER_FN((U16 numFloatChannels,
													   float const *pDefaultFloatChannels,
													   float *pOutputFloatChannels));

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdParenting.
		 *
		 * Composes each joint's joint params SQT triplet with its parent's, generating
		 * object space JointTransforms.  Each entry of pIndexTable is two 16 bit offsets
		 * stored as (jointindex * 0x10), to the joint and its parent, respectively, the
		 * parent offset optionally or'd with 1 to indicate that the joint should not
		 * inherit its parent's scale.
		 *
		 * JointTransforms are an intermediate format, with the parent scale split out
		 * of the matrix in order to allow children the option of ignoring it or not.
		 *
		 * pIndexTable is typically passed data stored in the JointHierarchy.
		 */
		void Parenting					DISPATCHER_FN((U16 numJointQuads,
													   JointParentingQuad const *pIndexTable,
													   JointParams const *pJointParams,
													   JointTransform *pOutputJointTransforms));

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdFinalizeAndPrepare.
		 *
		 * Converts JointTransforms into world space skinning matrices in the format used
		 * by the renderer.
		 */
		void FinalizeAndPrepareForRendering	DISPATCHER_FN((U16 numJoints,
														   JointTransform const *pJointTransforms,
														   SMath::Mat34 const *pInvBindPoseTransforms,
														   SMath::Mat34 const *pObjectTransform,
														   SMath::Mat44 *pOutputTransforms));

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdFinalize.
		 *
		 * Converts JointTransforms into world space joint matrices.
		 */
		void Finalize					DISPATCHER_FN((U16 numJoints,
													   JointTransform const *pJointTransforms,
													   SMath::Mat34 const *pObjectTransform,
													   SMath::Mat44 *pOutputTransforms));

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdFinalize.
		 *
		 * Converts world space joint matrices into world space skinning matrices in the format used
		 * by the renderer.
		 */
		void PrepareForRendering		DISPATCHER_FN((U16 numJoints,
													   SMath::Mat44 const *pInputTransforms,
													   SMath::Mat34 const *pInvBindPoseTransforms,
													   SMath::Mat44 *pOutputTransforms));

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdSdkDriversRot.
		 *
		 * Converts quaternion values from joints in pJointParams at byte offsets read
		 * sequentially from pSourceIndexTable into euler angles and stores the results
		 * into entries in pOutputScalarTable at byte offsets read sequentially from
		 * pDestIndexTable.  Each quaternion requires a rotation order (included in
		 * pSourceIndexTable), joint orientation, and rotation axis quaternion (read
		 * sequentially from pJointOrientTable and pRotAxisTable, respectively) to
		 * calculate the euler angles.  A write mask stored in pSourceIndexTable determines
		 * how many and which of the three calculated angles (x, y, and z) are stored
		 * to sequential entries in pOutputScalarTable.
		 *
		 * Typically driven by constant sdk network data stored in the JointHierarchy.
		 */
		void SdkQuaternionToEuler		DISPATCHER_FN((U16 numItems,		// Multiple of 4
													   JointParams const *pJointParams,
													   SdkDriversRotSourceQuad const *pSourceIndexTable,
													   U16 const *pDestIndexTable,
													   SMath::Quat const *pJointOrientTable,
													   SMath::Quat const *pRotAxisTable,
													   SMath::Scalar *pOutputScalarTable));
		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdSdkDrivers.
		 *
		 * Reads vector values from joints in pJointParams at byte offsets read
		 * sequentially from pIndexTable and writes up to 3 components specified
		 * by a write mask to sequential entries in pOutputScalarTable starting
		 * at a byte offset also specified in pIndexTable.
		 *
		 * Typically driven by constant sdk network data stored in the JointHierarchy.
		 */
		void SdkLoadVecs				DISPATCHER_FN((U16 numItems,		// Multiple of 4
													   JointParams const *pJointParams,
													   SdkDriversQuad const *pIndexTable,
													   SMath::Scalar *pOutputScalarTable));
		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdSdkCopyIn.
		 *
		 * Copies values from a float table typically provided as constants in the hierarchy
		 * plus external inputs in AnimControl::inputControls into the input controls section of
		 * pOutputScalarTable.  Copies with a resolution of 4 items.
		 *
		 * The number of sdk input or control values is stored in the JointHierarchy.
		 */
		void SdkCopyInScalars			DISPATCHER_FN((U16 numItems,
													   float const *pInputFloatTable,
													   SMath::Scalar *pOutputScalarTable));
		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdSdkDrivenRot.
		 *
		 * Converts euler angles in pScalarTable into quaternions to be stored back into
		 * pOutputJointParams.  Reads up to 3 sequential scalar values at a byte offset
		 * and based on a read mask read sequentially from pSourceIndexTable.
		 * Substitutes default values from pDefaultValueTable at a byte offset read from
		 * pDestIndexTable for unread values.  Uses a joint orientation and rotation
		 * axis quaternion read sequentially from pJointOrientTable and pRotAxisTable,
		 * respectively, and a rotation order read from pSourceIndexTable to convert
		 * the euler angles into a quaternion, and writes the result to pOutputJointParams
		 * at a byte offset read from pDestIndexTable.
		 *
		 * Typically driven by constant sdk network data stored in the JointHierarchy.
		 */
		void SdkEulerToQuaternion		DISPATCHER_FN((U16 numItems, 		// Multiple of 4
													   SdkDrivenRotSourceQuad const *pSourceIndexTable,
													   SdkDrivenRotDestQuad const *pDestIndexTable,
													   SMath::Quat const *pJointOrientTable,
													   SMath::Quat const  *pRotAxisTable,
													   SMath::Vector const *pDefaultValueTable,
													   SMath::Scalar const *pScalarTable,
													   JointParams *pOutputJointParams));
		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdSdkDriven.
		 *
		 * Reads 1 to 3 sequential entries from pScalarTable at a byte offset and based on a
		 * read mask read from pIndexTable, and substitutes values from pDefaultValueTable at
		 * byte offsets also in pIndexTable for any unread values.  Writes the resulting
		 * 3 vectors to pOutputJointParams at byte offsets also encoded in pIndexTable.
		 *
		 * Typically driven by constant sdk network data stored in the JointHierarchy.
		 */
		void SdkStoreVecs				DISPATCHER_FN((U16 numItems,		// Multiple of 2
													   SdkDrivenPair const *pIndexTable,
													   SMath::Vec4 const *pDefaultValueTable,
													   SMath::Scalar const *pScalarTable,
													   JointParams *pOutputJointParams));

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdSdkCopyOut.
		 *
		 * Copies sequential scalar values from pScalarTable into a float table, typically
		 * returned to the caller as part of AnimControl::outputControls.  Copies multiples
		 * of 4 items to a quad word aligned output location.
		 *
		 * The number of sdk output values is typically stored in the JointHierarchy.
		 */
		void SdkCopyOutScalars			DISPATCHER_FN((U16 numItems,
													   float *pOutputFloatTable,
													   SMath::Scalar const *pScalarTable));

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdSdkCopy.
		 *
		 * Copies scalar values within pScalarTable from sourceOffset to destOffset read
		 * sequentially from pIndexTable.
		 */
		void SdkCopyScalars				DISPATCHER_FN((U16 numItems,
													   SdkCopyScalarsQuad const *pIndexTable,
													   SMath::Scalar *pScalarTable));

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdSdkRunSegs1-4.
		 *
		 * Evaluates numSegments segment piece-wise Bezier functions defined in pDataTables.
		 * Reads from one scalar and outputs to another at byte offsets defined in pDataTables,
		 * optionally allowing the output value to accumulate with the value already in the
		 * destination scalar and/or with the value from the function evaluated 4 items ago,
		 * based on flags in the lower 2 bits of the destination offsets.
		 *
		 * Typically driven by constant sdk network data stored in the JointHierarchy.
		 */
		void SdkEvalCurves				DISPATCHER_FN((U16 numItems, 		// Multiple of 4
													   U16 numSegments,
													   U8 const *pDataTables,	// Index table concatenated with Min, Max, Key, and Bezier tables
													   SMath::Scalar *pScalarTable));

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdSdkRunBaked.
		 *
		 * Evaluates sampled piece-wise linear functions defined in pDataTables.
		 * Reads from one scalar and outputs to another at byte offsets defined in pDataTables,
		 * optionally allowing the output value to accumulate with the value already in the
		 * destination scalar and/or with the value from the function evaluated 4 items ago,
		 * based on flags in the lower 2 bits of the destination offsets.
		 *
		 * Typically driven by constant sdk network data stored in the JointHierarchy.
		 */
		void SdkEvalBaked				DISPATCHER_FN((U16 numItems, 		// Multiple of 4
													   U8 const *pDataTables,	// Index table concatenated with Bias, InvWidth, MinMax, and Sample tables
													   SMath::Scalar *pScalarTable));

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdConstraintParentPos.
		 *
		 * pDataTables contains :
		 * 	ConstraintInfo	pIndexTable[ numDrivers ];					// source offset, dest offset
		 *  Vec4			pLocalOffsetAndWeightTable[ numDrivers ];	// weight in w component
		 *  U32				numNormalizes;								// multiple of 8
		 *  U32				pad[3];
		 *  U16				pNormalizeIndexTable[ numNormalizes ];
		 *
		 * Applies the translation part of a parent space constraint to a joint, accumulating
		 * the weighted average of an arbitrary number of drivers per driven.  Each driver has
		 * a byte offset into pJointTransforms read from pIndexTable specifying the driving
		 * joint, and a weight and local space offset stored sequentially at pLocalOffsetTable
		 * with the weight in the w component.  The resulting parent space translation is
		 * accumulated to pOutputJointParams at a byte offset also read from pIndexTable.  The
		 * first index for each driven is tagged with a clear bit to zero the value before
		 * accumulating the first weighted offset.  The total weight is accumulated in the
		 * w component of the translation.
		 *
		 * If weights are animated, a second pass is required to normalize the translation by
		 * dividing by w.
		 *
		 * Typically driven by constant constraint data stored in the JointHierarchy.
		 */
		void ParentPosConstraint		DISPATCHER_FN((U16 numDrivers,		// Multiple of 4
													   U8 const *pDataTables,
													   JointTransform const *pJointTransforms,
													   JointParams *pOutputJointParams));

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdConstraintParentRot.
		 *
		 * pDataTables contains :
		 * 	ConstraintInfo	pIndexTable[ numDrivers ];					// source offset, dest offset
		 *  float			pWeightTable[ numDrivers ];
		 *  TransposedOrientation	pOffsetTable[ numDrivers ];			// (inverse rotation axis * offset rotation) for each driver
		 *	U32				numDestQuads;								// number of destinations / 4
		 *  U32				pad[3];
		 *	U16				pDestIndexTable[ numDestQuads * 4 ];		// padded to 16 byte alignment
		 *  TransposedOrientation pRotAxisTable[ numDestQuads * 4];		// (rotation axis) for each destination
		 *
		 * Applies the rotation part of a parent space constraint to a joint, accumulating
		 * the weighted average of an arbitrary number of drivers per driven.  Each driver has
		 * a byte offset into pJointTransforms read from pIndexTable specifying the driving
		 * joint, and a weight stored sequentially in pWeightTable.  An offset per driver is
		 * required to calculate the parent space quaternion to accumulate to the destination
		 * quaternion in pOutputJointParams.
		 *
		 * A second pass normalizes and offsets the accumulated quaternions, applying the
		 * rotation axis quaternion from pRotAxisTable to each driven joint.
		 *
		 * Typically driven by constant constraint data stored in the JointHierarchy.
		 */
		void ParentRotConstraint		DISPATCHER_FN((U16 numDrivers,		// Multiple of 4
													   U8 const *pDataTables,
													   JointTransform const *pJointTransforms,
													   JointParams *pOutputJointParams));

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdConstraintPoint.
		 *
		 * pDataTables contains :
		 * 	ConstraintInfo	pIndexTable[ numDrivers ];					// source offset, dest offset
		 *  float			pWeightTable[ numDrivers ];
		 *  U32				numNormalizes;								// multiple of 8
		 *  U32				numDests;
		 *  U32				pad[3];
		 *  U16				pNormalizeIndexTable[ numNormalizes ];
		 * 	Vec4			pOffsetTable[ numDests ];					// w component must be 0.0
		 *
		 * Applies an object space translation constraint to a joint, accumulating the weighted
		 * average translation of an arbitrary number of driver joints and applying a final
		 * object space offset.  One weight and one byte offset per driver is read from
		 * pWeightTable and pIndexTable respectively, and the result is accumulated to
		 * pOutputJointParams at the destination byte offset in pIndexTable, with a clear bit
		 * set on the first driver for each driver causing one entry from pOffsetTable to be
		 * consumed and written to the destination translation before accumulation.  The total
		 * weight is accumulated in the w component of the translation.
		 *
		 * If weights are animated, a second pass is required to normalize the translation by
		 * dividing by w.
		 *
		 * Typically driven by constant constraint data stored in the JointHierarchy.
		 */
		void PointConstraint			DISPATCHER_FN((U16 numDrivers,		// Multiple of 4
													   U8 const *pDataTables,
													   JointTransform const *pJointTransforms,
													   JointParams *pOutputJointParams));

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdConstraintOrient.
		 *
		 * pDataTables contains :
		 * 	ConstraintInfo	pIndexTable[ numDrivers ];					// source offset, dest offset
		 *  float			pWeightTable[ numDrivers ];
		 *  U32				numDestsQuads;								// number of destinations / 4
		 *  U32				pad[3];
		 * 	U16				pDestIndexTable[ numDestsQuads*4 ];
		 *	TransposedOrientation pOffsetRotAxisTable[ numDestQuads*4 ];// (offset * rotation axis) per destination
		 *
		 * Applies an object space rotation constraint to a joint, accumulating the weighted
		 * average object space rotation of an arbitrary number of drivers per driven.  Each
		 * driver has a byte offset into pJointTransforms read from pIndexTable specifying the
		 * driving joint, and a weight stored sequentially in pWeightTable.
		 *
		 * A second pass normalizes and offsets the accumulated quaternions, applying an offset
		 * and rotation axis quaternion from pOffsetRotAxisTable to each driven joint.
		 *
		 * Typically driven by constant constraint data stored in the JointHierarchy.
		 */
		void OrientConstraint			DISPATCHER_FN((U16 numDrivers,		// Multiple of 4
													   U8 const *pDataTables,
													   JointTransform const *pJointTransforms,
													   JointParams *pOutputJointParams));
		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdConstraintAim.
		 *
		 * pDataTables contains :
		 * 	ConstraintInfo	pIndexTable[ numDrivers ];					// source offset, dest offset
		 *  float			pWeightTable[ numDrivers ];
		 *  U32				numNormalizes;								// multiple of 8
		 *  U32				numDestsQuads;
		 *  U32				pad[3];
		 *  U16				pNormalizeIndexTable[ numNormalizes ];
		 * 	AimConstraintDestInfo pDestIndexTable[ numDestsQuads*4 ];	// up source offset/type, dest offset
		 *	TransposedOrientation pOffsetRotAxisTable[ numDestQuads*4 ];// (offset * rotation axis) per destination
		 *	Vec4			pUpVectorTable[ numUpVecs ];				// numUpVecs is implicit in up source types
		 *
		 * Applies an object space rotation constraint to aim a driven joint at a target point.
		 *
		 * The target point is the weighted average object space translation of an arbitrary
		 * number of driver joints, calculated similarly to a PointConstraint but with no local
		 * offsets and stored temporarily in the quaternion parameter of each joint.  The total
		 * weight is accumulated in the w component.
		 *
		 * If weights are animated, a second pass is required to normalize the translation by
		 * dividing by w.
		 *
		 * A third and final pass calculates an aim quaternion based on the relative target direction
		 * and an up vector either read from pUpVectorTable, the object y axis, a third joint's
		 * relative position, or a third joint's orientation based on flags in pDestIndexTable.
		 * The resulting quaternion is then written to pOutputJointParams at the byte offset
		 * in pDestIndexTable.
		 *
		 * Typically driven by constant constraint data stored in the JointHierarchy.
		 */
		void AimConstraint				DISPATCHER_FN((U16 numDrivers,		// Multiple of 4
													   U8 const *pDataTables,
													   JointTransform const *pJointTransforms,
													   JointParams *pOutputJointParams));

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdPatchConstraintData.
		 *
		 * Copies floating point override values from either the scalar table or the sdk output vector table one at a time.
		 * pIndexTable consists of a list of source and destination byte offsets, with the least bit of the source offset
		 * indicating input from the sdk vector table.  Patches typically consist of single values from the scalar table or
		 * sequences of 4 word patches from the sdk vector table into often non-sequential words of the constraint data.
		 * All destination offsets are word aligned.
		 */
		void PatchConstraintData		DISPATCHER_FN((U16 numPatchItems,
													   ConstraintPatchQuad *pIndexTable,
													   SMath::Scalar const *pScalarTable,
													   SMath::Vec4 const *pSdkVectorTable,
													   U8 *pConstraintData));

		/**
		 * PPU debug implementation of the SPU function by the same name, called internally by ExecuteBlend.
		 *
		 * Writes out a table of 128 copies of fBlendFactor at pOutputBlendFactorTable,
		 * one per joint, and overwrites those at indices read from pChannelFactors with
		 * the substitute value read from pChannelFactors.
		 */
		void ExpandChannelFactors(U16 numChannelFactors,
								  ChannelFactor const *pChannelFactors,
								  float fBlendFactor,
								  float *pOutputBlendFactorTable);

		/**
		 * PPU debug implementation of the SPU function by the same name, called internally by ExecuteBlend.
		 *
		 * Writes out offset table containing index of each set bit in validBits times
		 * 0x30 (== sizeof(JointParams)) to pOutputOffsetTable.  Writes safe offsets
		 * (0x30 * numJoints) to the end of the list as needed to pad it to a multiple
		 * of 8 entries.
		 */
		void ExpandValidBits(U16 numJoints,
							 ValidBits validBits,
							 U16 *pOutputOffsetTable);

		/**
		 * PPU debug implementation of the SPU function by the same name, called internally by ExecuteBlend.
		 *
		 * For each 4 byte offsets in pOffsetTable, reads 4 blend factors from pFactorTable and
		 * increments pFactorTable by factorTableInc bytes (generally 0 or 0x10).  For each offset
		 * and associated blend factor, reads a JointParams at the given byte offset from each of
		 * pLeftJointParams and pRightJointParams, applies a Lerp to the scale and translation and
		 * a normalized Lerp to the quaternions, and writes the result at the same byte offset in
		 * pOutputJointParams.
		 *
		 * May use pOutputJointParams[numJoints] as a safe output offset.
		 */
		void BlendNlerp(U16 numBlendQuads,
						U16 const *pOffsetTable,
						F32 const *pFactorTable,
						U32 factorTableInc,	// increment pFactorTable by factorTableInc bytes every 4 factors
						JointParams const *pLeftJointParams,
						JointParams const *pRightJointParams,
						JointParams *pOutputJointParams,
						U16 numJoints);
		/**
		 * PPU debug implementation of the SPU function by the same name, called internally by ExecuteBlend.
		 *
		 * For each 4 byte offsets in pOffsetTable, reads 4 blend factors from pFactorTable and
		 * increments pFactorTable by factorTableInc bytes (generally 0 or 0x10).  For each offset
		 * and associated blend factor, reads a JointParams at the given byte offset from each of
		 * pLeftJointParams and pRightJointParams, applies a Lerp to the scale and translation and
		 * a Slerp to the quaternions, and writes the result at the same byte offset in
		 * pOutputJointParams.
		 *
		 * May use pOutputJointParams[numJoints] as a safe output offset.
		 */
		void BlendSlerp(U16 numBlendQuads,
						U16 const *pOffsetTable,
						F32 const *pFactorTable,
						U32 factorTableInc,	// increment pFactorTable by factorTableInc bytes every 4 factors
						JointParams const *pLeftJointParams,
						JointParams const *pRightJointParams,
						JointParams *pOutputJointParams,
						U16 numJoints);
		/**
		 * PPU debug implementation of the SPU function by the same name, called internally by ExecuteBlend.
		 *
		 * For each 4 byte offsets in pOffsetTable, reads 4 blend factors from pFactorTable and
		 * increments pFactorTable by factorTableInc bytes (generally 0 or 0x10).  For each offset
		 * and associated blend factor, reads a JointParams at the given byte offset from each of
		 * pLeftJointParams and pRightJointParams, applies an additive Lerp (i.e. a + f * b) to
		 * the scale and translation and an additive Slerp to the quaternion, and writes the result
		 * at the same byte offset in pOutputJointParams.
		 *
		 * May use pOutputJointParams[numJoints] as a safe output offset.
		 */
		void BlendAdditive(U16 numBlendQuads,
						   U16 const *pOffsetTable,
						   F32 const *pFactorTable,
						   U32 factorTableInc,	// increment pFactorTable by factorTableInc bytes every 4 factors
						   JointParams const *pLeftJointParams,
						   JointParams const *pRightJointParams,
						   JointParams *pOutputJointParams,
						   U16 numJoints);
		/**
		 * PPU debug implementation of the SPU function by the same name, called internally by ExecuteBlend.
		 *
		 * For each 4 byte offsets in pOffsetTable, reads 4 blend factors from pFactorTable and
		 * increments pFactorTable by factorTableInc bytes (generally 0 or 0x10).  For each offset
		 * and associated blend factor, reads a JointParams at the given byte offset from each of
		 * pLeftJointParams and pRightJointParams, applies a Lerp to the scale and translation and
		 * multiplies the quaternions, and writes the result at the same byte offset in
		 * pOutputJointParams.
		 *
		 * May use pOutputJointParams[numJoints] as a safe output offset.
		 *
		 * Note that a BlendMultiply with blend factor 0.0 is NOT an identity operation!
		 */
		void BlendMultiply(U16 numBlendQuads,
						   U16 const *pOffsetTable,
						   F32 const *pFactorTable,
						   U32 factorTableInc,	// increment pFactorTable by factorTableInc bytes every 4 factors
						   JointParams const *pLeftJointParams,
						   JointParams const *pRightJointParams,
						   JointParams *pOutputJointParams,
						   U16 numJoints);

		/**
		 * PPU debug implementation of the SPU function by the same name, called internally by ExecuteBlend and FillInUndefined.
		 *
		 * For each byte offset in pOffsetTable, copies a JointParams from pJointParams to
		 * pOutputJointParams.
		 */
		void BlendCopy(U16 numBlendCopyQuads,
					   U16 const *pOffsetTable,
					   JointParams const *pJointParams,
					   JointParams *pOutputJointParams);

		/**
		 * PPU debug implementation of the SPU function by the same name, called internally by ExecuteBlendFloats.
		 *
		 * For each float channel, perform the appropriate operation based on the blend type
		 * and valid bits for the left and right channels.  Float channels and pfBlendFactor
		 * are read in sets of 4.  pfBlendFactor is advanced by blendFactorStep bytes
		 * (typically 0 or 16) after each set of 4.
		 *
		 * The operation performed is Output[i] = A[i] * fA + B[i] * fB according to the
		 * following table, where f == pfBlendFactor[i]:
		 * 				additive==0		additive==1
		 *		Valid	fA	fB			fA	fB
		 *		 - -	0	0			0	0
		 *		 A -	1	0			1	0
		 *		 - B	0	1			0	f
		 * 		 A&B	1-f	f			1	f
		 */
		void BlendFloats(U16 numFloatChannels,
						 F32 const *pLeftFloatChannels,
						 F32 const *pRightFloatChannels,
						 ValidBits const *pLeftValidBits,
						 ValidBits const *pRightValidBits,
						 F32 const *pfBlendFactor,
						 U32 blendFactorStep,
						 F32 *pOutputFloatChannels,
						 U32 additive);

#if ICE_TARGET_PS3_SPU
		}	// extern "C"
		// End functions implemented in SPU code
#endif

		/**
		 * Signiture shared by clip decompression functions used to decompress
		 * constant data directly to pOutputTable.
		 */
		typedef void (*AnimClipConstDecompressFunction)(U16 numJoints,
														U16 const *pDestIndexTable,		// floatChannel*0x4 or joint*0x30 + (0x00 (scale) or 0x10 (quat) or 0x20 (trans))
														U8 *pOutputTable);				// float channel table or JointParam table
		/**
		 * Signiture shared by clip decompression functions used to decompress
		 * animated data in place in the key cache.
		 */
		typedef void (*AnimClipDecompressFunction)(U16 numJointsAndPerJointFlag,	// perJointFlag<<15 | numJoints
												   U8 const *pFormatData,			// { if (perJointFlag) bitFormat[numJoints] else bitFormat[1] }, { range_scale }[numJoints]
												   U8 *pKeyCache);					// numJoints*2 Vec4 entries

		/**
		 * Signiture shared by functions called by ExecuteBlend to apply
		 * various JointParams blending operations.
		 */
		typedef void (*BlendFunction)(U16 numBlendQuads,
									  U16 const *pOffsetTable,
									  F32 const *pFactorTable,
									  U32 factorTableInc,	// increment pFactorTable by factorTableInc bytes every 4 factors
									  JointParams const *pLeftJointParams,
									  JointParams const *pRightJointParams,
									  JointParams *pOutputJointParams,
									  U16 numJoints);
	}
};

#endif //ICE_JOINTANIMCORE_H
