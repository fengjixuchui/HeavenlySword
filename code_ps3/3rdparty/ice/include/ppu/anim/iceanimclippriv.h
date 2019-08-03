/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_ANIMCLIPPRIV_H
#define ICE_ANIMCLIPPRIV_H

/*! 
 * \file 	iceanimclippriv.h
 * \brief 	Internal structure of ClipData data.
 */
 
namespace Ice 
{
    namespace Anim
    {
		/// magic version constants stored in ClipData::m_magic
		static const U32 kClipDataMagicVersion1_03 =	0x43440103;

		/// max key cache size with slop to allow decompression commands to operate on 4 entries at a time
		static const U32 kMaxKeyCacheSize = 128 /* kJointBatchSize */ + 8;

		/*!
		 * Structure of the key table for shared non-uniform key lookup:
		 */
		struct AnimClipKeyTableHeader {
			U16 m_totalKeys;								//!< total number of keys in animation
			//U16 m_frames[ m_totalKeys ];					//!< list of frame indices for each key
			//pad to qword multiple
		};

		/*!
		 * Each clip group contains constant data (accessed through m_constOffset), and
		 * a variable number of blocks starting with block[m_firstBlock].
		 */
		struct AnimClipGroupHeader {
			U32 m_constOffset;			//!< Offset from the data header to the AnimClipGroupConstData.
			U16 m_firstBlock;			//!< The block number relative to header->m_blockData
			U16 pad;
		};

		/*!
		 * The AnimClipGroupConstData is a header for data that is constant for this animation joint group.
		 */
		struct AnimClipGroupConstData {
			U8  m_numOps;				//!< total number of operations to decompress the data for this group
			U8  m_numKeyCopyOps;		//!< the number of operations which copy data to the key cache for this group
			U16 m_qwcKeyDmaSize;		//!< for shared keys, total size in qwords of one key worth of data for DMA
			U32	m_constDmaSize;			//!< total size of all constant data blocks to DMA for this joint group's decompression operations.  Does not include sizeof(m_validBits[]).
			U32 m_opHeadersOffset;		//!< Offset from start of AnimClipGroupConstData to the AnimClipOperationHeader's defining how to unpack the data for this group.
			U32 pad;

			// DMA data begins here, beginning with optional m_validBits
			//ValidBits m_validBits[];  // The elements in this group which this clip writes, represented as 1 or 2 128-bit fields.
										// if (m_numJoints > 0) includes one ValidBits for joints
										// if (m_numFloatChannels > 0) includes one ValidBits for float channels
			//<various formats>			m_constDataBlocks[ m_numOps ];		// constant data for clip decompression operations

			// Non-DMA data begins here
			//AnimClipOperationHeader	m_clipOperations[ m_numOps ];		// headers for clip decompression operations
		};

		/*!
		 * Information required in order to be able to set up a DMA of a block
		 * of variable sized data with shared (uniform or non-uniform shared) key times.
		 *
		 * A SharedKeysBlock generally has the format:
		 *   { QuatValues[iQuat], ScaleValues[iScale], TransValues[iTrans], FloatChannelValues[iFloat] }[key]
		 * which allows all data for one or two (or more) adjacent keys to be sent in one DMA.
		 *
		 * Note that the ordering of data within a block may be adjusted in order to combine 
		 * compatible decompression operations within a parameter or even from multiple 
		 * parameters (for instance, scale and translation vectors with the same compression 
		 * may be combined if the total number fits within the keycache, in order to reduce 
		 * SPU function call overhead and data padding overhead).
		 */
		struct AnimClipSharedKeysBlockHeader {
			U32 m_blockOffset;				//!< Offset to block data (and scale data)
			U16 m_qwcTotal;					//!< Total size of block in qwords
			U16 pad;
		};

		/*!
		 * Information required in order to be able to set up a DMA of a block
		 * of variable sized data with unshared (per-joint) non-uniform key times.
		 *
		 * An UnsharedKeysBlock generally has the format:
		 *   { QuatValues [ numQuatKeys][joint], { U8  numQuatKeys, U8  quatTimeVals[ numQuatKeys-1] }[joint], {pad to qword},
		 *     ScaleValues[numScaleKeys][joint], { U8 numScaleKeys, U8 scaleTimeVals[numScaleKeys-1] }[joint], {pad to qword},
		 *     TransValues[numTransKeys][joint], { U8 numTransKeys, U8 scaleTimeVals[numTransKeys-1] }[joint], {pad to qword},
		 *     FloatValues[numFloatKeys][joint], { U8 numFloatKeys, U8 floatTimeVals[numFloatKeys-1] }[joint], {pad to qword},
		 *   };
		 * and is always sent as one DMA. 
		 *
		 * Note that the ordering of data within a block may be adjusted in order to combine 
		 * compatible decompression operations within a parameter or even from multiple 
		 * parameters (for instance, scale and translation vectors with the same compression 
		 * may be combined if the total number fits within the keycache, in order to reduce 
		 * SPU function call overhead and data padding overhead).
		 */
		struct AnimClipUnsharedKeysBlockHeader {
			U32 m_blockOffset;				//!< Offset to block data (and scale data)
			U16 m_qwcTotal;					//!< Total size of block in qwords
			U16 m_numFrames;				//!< Number of frames in this block (startTime = (m_endFrame - m_numFrames + 1) * clip->m_framesPerSecond)
			F32 m_endFrame;					//!< index of last frame in this block
			U32 m_layoutDataOffset;			//!< Offset to per-block data offsets; AnimClipUnsharedKeysBlockLayoutData[AnimClipGroupConstData::m_numKeyCopyOps]
		};
		struct AnimClipUnsharedKeysBlockLayoutData {
			U16 m_dataOffset;				//!< offset from the start of the block to the key data for this operation (aligned)
			U16 m_keySize;					//!< offset from key data to start of value data for this operation (non-aligned)
		};

		/*!
		 * Information about a single data-driven operation to apply to the animation clip data for a group.
		 */
		struct AnimClipOperationHeader {
			U16 m_cmd;						//!< clip decompression command to apply
			U16	m_numItems;					//!< number of data elements operated on, possibly or with AnimClipOperationFlags
			U16 m_keyCacheOffset;			//!< offset to first key cache element to operate on.
			U16 m_blockDataOffset;			//!< for shared keys data copy operations, the offset within a shared key to the data to copy.  0 otherwise.
			U32	m_dataOffset;				//!< offset to constant data for this operation (indices, format data, constant values)
			U32	m_dataSize;					//!< total size of constant input data to this operation
		};
		/*!
		 * Flags possibly or'd with number of items to indicate special behavior of an operation.
		 */
		enum AnimClipOperationFlags {
			kAnimClipOperationVariableFormat = 	0x8000,		//!< this clip operation reads a format or size per item, rather than sharing one for all items
		};

		/*!
		 * Internal structure to represent quad-word aligned joint indices where we process 4 at a time.
		 * 2 groups per quad-word.
		 */
		struct AnimClipIndexQuad
		{
			U16 m_elem[4];		// generally index * 0x30 + (0x00 (scale), 0x10 (quat), or 0x20 (trans)), possibly or'd with flags in lower 4 bits
		};

		/*!
		 * Internal structure to represent quad-word aligned joint indices where we process 8 at a time.
		 * 1 group per quad-word.
		 */
		struct AnimClipIndexOctet
		{
			U16 m_elem[8];		// generally index * 0x30 + (0x00 (scale), 0x10 (quat), or 0x20 (trans)), possibly or'd with flags in lower 4 bits
		};

		/*!
		 * Internal structure which specifies a bit-packing scheme for an animated joint parameter.
		 * Bit packing schemes may be specified per parameter type (i.e. for all animated scales, rotations, 
		 * or translations) or per joint parameter (i.e. per animated scale, rotation, or translation per joint).
		 */
		struct AnimClipBitPackingFormat
		{
			U8	m_numBitsX;							//!< number of bits in x channel [0..31]
			U8	m_numBitsY;							//!< number of bits in y channel [0..31]
			U8	m_rotqZ_rotY;						//!< (byte rotate for z channel + 3 [0..7] << 5) | (bit rotate for y channel [0..31])
			U8	m_numBitsZ;							//!< number of bits in z channel [0..31]

			AnimClipBitPackingFormat() {}
			AnimClipBitPackingFormat(U8 numBitsX, U8 numBitsY, U8 numBitsZ) 
				: m_numBitsX(numBitsX), m_numBitsY(numBitsY), m_numBitsZ(numBitsZ)
			{
				m_rotqZ_rotY = (((numBitsX+numBitsY+numBitsZ-1)/8)<<5) | ((numBitsY + (numBitsX&0x7))&0x1F);
			}
		};

		/*!
		 * Internal structure to hold linear range scaled data header.
		 */
		struct AnimClip3VecRangeHeader
		{
			U64 m_scale;	// (max - min)/(2^numbits - 1) for x, y, z, w;	unsigned float 0.8.14 - 0.8.13 - 0.8.13
			U64 m_bias;		// minimum values of x, y, z, w;				float 1.8.13 - 1.8.12 - 1.8.12
		};

		/*!
		 * Internal structure to hold a quaternion log compression data header.
		 * 2 quaternions stored in signed fixed point s1.15 format.
		 * If LogPca compression, holds qPre, qPost for 1 entry.  If Log compression, holds qMean for 2 entries.
		 */
		struct AnimClipQuatLogHeader
		{
			I16 x[2];
			I16 y[2];
			I16 z[2];
			I16 w[2];
		};
	}	// namespace Anim
}	// namespace Ice

#endif //ICE_ANIMCLIPPRIV_H
