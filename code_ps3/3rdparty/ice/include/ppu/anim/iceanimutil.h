/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_ANIMUTIL_H
#define ICE_ANIMUTIL_H

#include "icejointanim.h"

namespace Ice
{
	namespace Anim
	{

		/// Describes absolute endianness of data
		enum Endianness {
			kEndiannessInvalid =			-1,	//!< data is invalid
			kEndiannessBig = 				 0,	//!< data is big endian; MSB ... LSB
			kEndiannessLittle = 		 	 1,	//!< data is little endian; LSB ... MSB
		};
		/// Describes relative endianness of data
		enum RelativeEndianness {
			kRelativeEndiannessInvalid =	-1,	//!< data is invalid
			kRelativeEndiannessUnswapped =	 0,	//!< data byte order matches host byte order
			kRelativeEndiannessSwapped =	 1,	//!< data byte order is swapped relative to host byte order
		};

		/** Returns the absolute endianness of this host machine. */
		Endianness GetHostEndianness();

		/** Returns the absolute endianness of the given JointHierarchy based its the magic value */
		Endianness GetJointHierarchyEndianness(JointHierarchy const* pJointHierarchy);
		/** Returns the relative endianness of the given JointHierarchy based its the magic value */
		RelativeEndianness GetJointHierarchyRelativeEndianness(JointHierarchy const* pJointHierarchy);

		/** Returns the absolute endianness of the given ClipData based its the magic value */
		Endianness GetClipDataEndianness(ClipData const* pClipData);
		/** Returns the relative endianness of the given ClipData based its the magic value */
		RelativeEndianness GetClipDataRelativeEndianness(ClipData const* pClipData);

		/**
		 * Traverses all data in the given JointHierarchy, swapping the byte
		 * order of any multi-byte values.
		 * Returns false if the JointHierarchy appears to be invalid.
		 */
		bool EndianSwapJointHierarchy(JointHierarchy* pJointHierarchy);
		/**
		 * Traverses all data in the given JointHierarchy, swapping the byte
		 * order of any multi-byte values if necessary to convert to the requested
		 * absolute or relative endianness.
		 * Returns false if the JointHierarchy appears to be invalid.
		 */
		inline bool EndianSwapJointHierarchy(JointHierarchy* pJointHierarchy, RelativeEndianness eTargetRelativeEndianness)
		{
			if (GetJointHierarchyRelativeEndianness(pJointHierarchy) == eTargetRelativeEndianness)
				return true;
			return EndianSwapJointHierarchy(pJointHierarchy);
		}
		inline bool EndianSwapJointHierarchy(JointHierarchy* pJointHierarchy, Endianness eTargetEndianness)
		{
			if (GetJointHierarchyEndianness(pJointHierarchy) == eTargetEndianness)
				return true;
			return EndianSwapJointHierarchy(pJointHierarchy);
		}

		/**
		 * Traverses all data in the given ClipData, swapping the byte
		 * order of any multi-byte values.
		 * Returns false if the ClipData appears to be invalid.
		 */
		bool EndianSwapClipData(ClipData* pClipData);
		/**
		 * Traverses all data in the given ClipData, swapping the byte
		 * order of any multi-byte values if necessary to convert to the requested
		 * absolute or relative endianness.
		 * Returns false if the ClipData appears to be invalid.
		 */
		inline bool EndianSwapClipData(ClipData* pClipData, RelativeEndianness eTargetRelativeEndianness)
		{
			if (GetClipDataRelativeEndianness(pClipData) == eTargetRelativeEndianness)
				return true;
			return EndianSwapClipData(pClipData);
		}
		inline bool EndianSwapClipData(ClipData* pClipData, Endianness eTargetEndianness)
		{
			if (GetClipDataEndianness(pClipData) == eTargetEndianness)
				return true;
			return EndianSwapClipData(pClipData);
		}

	}
}

#endif // ICE_ANIMUTIL_H
