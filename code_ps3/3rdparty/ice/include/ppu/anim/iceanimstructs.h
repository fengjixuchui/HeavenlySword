/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_ICEANIMSTRUCTS_H
#define ICE_ICEANIMSTRUCTS_H

#include "shared/math/point.h"
#include "shared/math/vector.h"
#include "shared/math/quat.h"
#include "shared/math/mat34.h"
#include "shared/math/transform.h"

#include "icebase.h"
#if ICE_TARGET_PS3_SPU
#include <spu_intrinsics.h>
#endif

namespace Ice
{
	namespace Anim
    {
		/*!
		 * Defines the size of a joint group, for the purposes of clip and blending operations.
		 * NOTE: this value can not be changed without matching modifications to the data build process
		 * and the ValidBits structure.
		 */
		const U32 kJointGroupSize = 128;
		/*!
		 * Defines the size of a float channel group, for the purposes of clip and blending operations.
		 * NOTE: this value can not be changed without matching modifications to the data build process
		 * and the ValidBits structure.
		 */
		const U32 kFloatChannelGroupSize = 128;

		// ----------------------------------------------------------------------------------------------------

		/*!
         * This general structure is used to represent the local scale, orientation, and
		 * position of a joint, which is used by clips, poses, blending operations, and SDKs.
		 * It has scale, quaternion and translate, thus it is often called an "SQT".
         */
        struct JointParams
        {
            SMath::Vector m_scale;   //!< Scale of this joint.
            SMath::Quat   m_quat;    //!< Rotation (in unit quaternion) form of this joint.
            SMath::Point  m_trans;   //!< Translation of this joint.
        };

		// ----------------------------------------------------------------------------------------------------

		/*!
		 * The ValidBits structure is a quadword of bits. We use it to define whether a joint
		 * in a joint group (128 joints), or a float channel in a float channel group is valid or not.
		 */
        struct ValidBits {
#if ICE_TARGET_PS3_SPU
			explicit ValidBits() {}
			explicit ValidBits(VU8 bits) : m_bits(bits) {}
			explicit ValidBits(U64 bits0_63, U64 bits64_127 = 0) { Set(bits0_63, bits64_127); }
			ValidBits(ValidBits const &to_copy) { Set(to_copy); }
			void Set(U64 bits0_63, U64 bits64_127 = 0)
			{ 
				m_bits = (VU8)spu_sel(spu_splats(bits0_63), spu_splats(bits64_127), spu_maskb(0x00FF)); 
			}
			void Set(ValidBits const &to_copy) { m_bits = to_copy.m_bits; }
			void SetEmpty() { m_bits = (VU8)spu_splats((U32)0); }
			void SetAllBits() { m_bits = (VU8)spu_splats((U32)-1); }
			void SetAllBits(U32 numBits) 
			{ 
				U32 shift = 128 - numBits;
				m_bits = spu_slqw(spu_slqwbytebc((VU8)spu_splats((U32)-1), shift), shift);
			}
			void Clear() { SetEmpty(); }

			// returns 0xFFFFFFFF if all bits are 0, 0 if not.
			U32 IsEmpty() const { return spu_extract(spu_cmpeq( spu_orx((VU32)m_bits), 0 ), 0); }

			void Invert() { m_bits = spu_xor(m_bits, (U8)0xFF); }

			// returns 0xFFFFFFFF if bit[iBit] is 1, 0 if not.
			U32 IsBitSet(U32 iBit) const { return spu_extract( spu_cmpgt( spu_splats((I32)0), (VI32)spu_slqw(spu_slqwbytebc(m_bits, iBit), iBit) ), 0 ); }
			void SetBit(U32 iBit) 
			{
				U32 shift = 127 - iBit;
				VU8 bit127 = spu_rlmaskqwbyte( (VU8)spu_splats((U32)1), -12 );
				m_bits = spu_or( m_bits, spu_rlqw(spu_rlqwbytebc(bit127, shift), shift) );
			}
			void ClearBit(U32 iBit)
			{
				U32 shift = 127 - iBit;
				VU8 bit127 = spu_rlmaskqwbyte( (VU8)spu_splats((U32)1), -12 );
				m_bits = spu_andc( m_bits, spu_rlqw(spu_rlqwbytebc(bit127, shift), shift) );
			}

			ValidBits const& operator=(ValidBits const &to_copy) { Set(to_copy); return *this; }

			// returns 0xFFFFFFFF if this is identical to other, 0 if not.
			U32 operator==(ValidBits const &other) const { return spu_extract( spu_cmpeq( spu_orx((VU32)spu_xor(m_bits, other.m_bits)), 0 ), 0 ); }
			// returns 0xFFFFFFFF if this is not identical to other, 0 if not.
			U32 operator!=(ValidBits const &other) const { return spu_extract( spu_cmpgt( spu_orx((VU32)spu_xor(m_bits, other.m_bits)), 0 ), 0 ); }

			ValidBits operator~() const { return ValidBits( spu_xor(m_bits, (U8)0xFF) ); }
			ValidBits operator&(ValidBits const &v) const { return ValidBits( spu_and(m_bits, v.m_bits) ); }
			ValidBits operator|(ValidBits const &v) const { return ValidBits( spu_or(m_bits, v.m_bits) ); }
			ValidBits operator^(ValidBits const &v) const { return ValidBits( spu_xor(m_bits, v.m_bits) ); }
			ValidBits const& operator&=(ValidBits const &v) { m_bits = spu_and(m_bits, v.m_bits); return *this; }
			ValidBits const& operator|=(ValidBits const &v) { m_bits = spu_or(m_bits, v.m_bits); return *this; }
			ValidBits const& operator^=(ValidBits const &v) { m_bits = spu_xor(m_bits, v.m_bits); return *this; }

			VU8 m_bits;
#else
			explicit ValidBits() {}
			explicit ValidBits(U64 bits0_63, U64 bits64_127 = 0) { Set(bits0_63, bits64_127); }
			ValidBits(ValidBits const &to_copy) { Set(to_copy); }
			void Set(U64 bits0_63, U64 bits64_127 = 0) { m_bits[0] = bits0_63; m_bits[1] = bits64_127; }
			void Set(ValidBits const &to_copy) { m_bits[0] = to_copy.m_bits[0]; m_bits[1] = to_copy.m_bits[1]; }
			void SetEmpty() { m_bits[0] = 0; m_bits[1] = 0; }
			void SetAllBits(U32F numBits = 128) { if (numBits > 64) { m_bits[0] = (U64)-1; m_bits[1] = (U64)-((I64)1 << (128-numBits)); } else { m_bits[0] = (U64)-((I64)1 << (64-numBits)); m_bits[1] = 0; } }
			void Clear() { SetEmpty(); }

			bool IsEmpty() const { return (m_bits[0] | m_bits[1]) == 0; }

			void Invert() { m_bits[0] = ~m_bits[0]; m_bits[1] = ~m_bits[1]; }

			bool IsBitSet(U32F iBit) const { return (m_bits[iBit>>6] & (0x8000000000000000ULL>>(iBit&0x3F))) != 0; }
			void SetBit(U32F iBit) { m_bits[iBit>>6] |= (0x8000000000000000ULL>>(iBit&0x3F)); }
			void ClearBit(U32F iBit) { m_bits[iBit>>6] &=~(0x8000000000000000ULL>>(iBit&0x3F)); }

			ValidBits const& operator=(ValidBits const &to_copy) { Set(to_copy); return *this; }
			bool operator==(ValidBits const &other) const { return m_bits[0] == other.m_bits[0] && m_bits[1] == other.m_bits[1]; }
			bool operator!=(ValidBits const &other) const { return !operator==(other); }
			ValidBits operator~() const { return ValidBits(~m_bits[0], ~m_bits[1]); }
			ValidBits operator&(ValidBits const &v) const { return ValidBits(m_bits[0] & v.m_bits[0], m_bits[1] & v.m_bits[1]); }
			ValidBits operator|(ValidBits const &v) const { return ValidBits(m_bits[0] | v.m_bits[0], m_bits[1] | v.m_bits[1]); }
			ValidBits operator^(ValidBits const &v) const { return ValidBits(m_bits[0] ^ v.m_bits[0], m_bits[1] ^ v.m_bits[1]); }
			ValidBits const& operator&=(ValidBits const &v) { m_bits[0] &= v.m_bits[0]; m_bits[1] &= v.m_bits[1]; return *this; }
			ValidBits const& operator|=(ValidBits const &v) { m_bits[0] |= v.m_bits[0]; m_bits[1] |= v.m_bits[1]; return *this; }
			ValidBits const& operator^=(ValidBits const &v) { m_bits[0] ^= v.m_bits[0]; m_bits[1] ^= v.m_bits[1]; return *this; }

			U64 m_bits[2];
#endif
        };

		// ----------------------------------------------------------------------------------------------------

		//! Buffer position independent representation of a pointer used to pass local store pointers
		//! to SPU functions in a batched task
		typedef U16 Location;
		/// Size of safe output padding on joint and float channel groups during blending
		static const U32 kBlendSafeSize = 0x30;

		/// Each channel group in a blend group has an associated ValidBits containing a bitfield
		/// describing which joints or float channels in the group are defined.
		/// GetBlendGroupJointValidBits() returns the location of the joint ValidBits given a pointer
		/// to the channel group data, and the number of joints.
		inline ValidBits* GetBlendGroupJointValidBits(JointParams *pJointParams, U32F numJoints) { return (ValidBits*)((U8*)(pJointParams + numJoints) + kBlendSafeSize); }
		inline ValidBits const* GetBlendGroupJointValidBits(JointParams const* pJointParams, U32F numJoints) { return GetBlendGroupJointValidBits(const_cast<JointParams*>(pJointParams), numJoints); }
		inline Location GetBlendGroupJointValidBits(Location jointParamsLoc, U32F numJoints) { return jointParamsLoc + numJoints*sizeof(JointParams) + kBlendSafeSize; }
		/// GetBlendGroupFloatChannelValidBits() returns the location of the float channel ValidBits
		/// given a pointer to the channel group data, and the number of float channels.
		inline ValidBits* GetBlendGroupFloatChannelValidBits(float* pFloatChannel, U32F numFloatChannels) { return (ValidBits*)((U8*)(pFloatChannel + ((numFloatChannels + 0x3)&~0x3)) + kBlendSafeSize); }
		inline ValidBits const* GetBlendGroupFloatChannelValidBits(float const* pFloatChannel, U32F numFloatChannels) { return GetBlendGroupFloatChannelValidBits(const_cast<float*>(pFloatChannel), numFloatChannels); }
		inline Location GetBlendGroupFloatChannelValidBits(Location floatChannelLoc, U32F numFloatChannels) { return floatChannelLoc + ((numFloatChannels + 0x3)&~0x3)*sizeof(float) + kBlendSafeSize; }

		//----------------------------------------------------------------------------------------

		/*!
		 * The blending operation to apply.
		 * Note that BlendAdditive and BlendMultiply likely require animation clips created specifically
		 * for use using these blend types.
		 */
		enum BlendMode {
			kBlendLerp, kBlendSlerp, kBlendAdditive, kBlendMultiply, kMake32bit = 0xffffffff
		};

		/*!
		 * A helper structure for blend nodes, to allow blend factors
		 * to be specified per joint or float channel.
		 *
		 * Channels are specified using ids relative to the current group -
		 * m_channelId = (index % (kJointGroupSize or kFloatChannelGroupSize)).
		 */
		struct ChannelFactor {
			U32		m_channelId;	//!< The joint or float channel to act on within this channel group (i.e. index is relative to the group, and must be between 0 and (kJointGroupSize-1) or (kFloatChannelGroupSize-1).
			float	m_blendFactor;	//!< The override blend factor to use for this channel.
		};

		//----------------------------------------------------------------------------------------

		/*!
		 *	Unary flips apply their operation to one joint.  Binary flips first swap two joints,
		 *  then apply the same flip operation to each.
		 */
		enum FlipType {
			kFlipUnary,
			kFlipBinary,
		};
		/*!
		 *	Each FlipOp contains a 32 bit specification on how to flip each joint translation
		 *  and quaternion.
		 *  FlipOps can be specified using a union of FlipOpComponentSource flags shifted
		 *	left by FlipOpComponentDest shifts.  The FLIP_OP* macros are provided to make these
		 *	operations more compact.
		 */
		enum FlipOpComponentSource {
			kFlipOpFromX = 			0x0,
			kFlipOpFromY = 			0x1,
			kFlipOpFromZ = 			0x2,
			kFlipOpFromW = 			0x3,
			kFlipOpNegate = 		0x8,
			kFlipOpFromNegX =		0x8,
			kFlipOpFromNegY =		0x9,
			kFlipOpFromNegZ =		0xa,
			kFlipOpFromNegW =		0xb,
		};
		enum FlipOpComponentDest {
			kFlipOpQuatX =			 0,
			kFlipOpQuatY =			 8,
			kFlipOpQuatZ =			16,
			kFlipOpQuatW =			24,
			kFlipOpTransX =			 4,
			kFlipOpTransY =			12,
			kFlipOpTransZ =			20,
			kFlipOpTransW =			28,
		};
#define FLIP_OP_Q(srcx, srcy, srcz, srcw)	(((srcx)<<kFlipOpQuatX) | ((srcy)<<kFlipOpQuatY) | ((srcz)<<kFlipOpQuatZ) | ((srcw)<<kFlipOpQuatW))
#define FLIP_OP_T(srcx, srcy, srcz)			(((srcx)<<kFlipOpTransX) | ((srcy)<<kFlipOpTransY) | ((srcz)<<kFlipOpTransZ) | (kFlipOpFromW<<kFlipOpTransW))
#define FLIP_OP(qx, qy, qz, qw, tx, ty, tz)	(FLIP_OP_Q(qx,qy,qz,qw) | FLIP_OP_T(tx, ty, tz))
		enum FlipOp {
			kFlipOpNop = FLIP_OP(kFlipOpFromX, kFlipOpFromY, kFlipOpFromZ, kFlipOpFromW, kFlipOpFromX, kFlipOpFromY, kFlipOpFromZ),
			kFlipOpQNop = FLIP_OP_Q(kFlipOpFromX, kFlipOpFromY, kFlipOpFromZ, kFlipOpFromW),	// Bitwise-or with FLIP_OP_T to create a T-only flip
			kFlipOpTNop = FLIP_OP_T(kFlipOpFromX, kFlipOpFromY, kFlipOpFromZ),					// Bitwise-or with FLIP_OP_Q to create a Q-only flip
		};

		//----------------------------------------------------------------------------------------
	}
};

#endif //ICE_ICEANIMSTRUCTS_H
