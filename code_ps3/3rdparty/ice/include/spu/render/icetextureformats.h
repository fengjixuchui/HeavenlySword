/*
 * Copyright (c) 2003-2006 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_TEXTUREFORMATS_H
#define ICE_TEXTUREFORMATS_H

namespace Ice
{
	namespace Render
	{
		/*
		** Enumerations and a macro are provided as documentation for
		** creating new texture formats.
		*/

		//! Base texture format encodings.
		enum
		{
			kBtfB8             = 0x01,
			kBtfArgb1555       = 0x02,
			kBtfArgb4444       = 0x03,
			kBtfRgb565         = 0x04,
			kBtfArgb8888       = 0x05,
			kBtfDxt1           = 0x06,
			kBtfDxt3           = 0x07,
			kBtfDxt5           = 0x08,
			/*! Legacy Format! */
			kBtfSY8            = 0x09,
			/*! Legacy Format! */
			kBtfX7SY9          = 0x0A,
			kBtfGb88           = 0x0B,
			/*! Legacy Format! */
			kBtfSG8SB8         = 0x0C,
			kBtfB8R8G8R8       = 0x0D,
			kBtfR8B8R8G8       = 0x0E,
			kBtfRgb655         = 0x0F,
			kBtfDepth24X8      = 0x10,
			//! The depth value is a 24-bit floating point number. 
			/*! It consists of an 8-bit exponent and a 16-bit mantissa. 
			    It supports non-normalized numbers, infinite numbers, and NaNs.
			*/
			kBtfDepth24FX8     = 0x11,
			kBtfDepth16        = 0x12,
			//! The depth value is a 16-bit floating point number. 
			/*! It consists of a 4-bit exponent and a 12-bit mantissa. 
			    Non-normalized numbers, infinite numbers, and NaN's are *not* supported.
			*/
			kBtfDepth16F       = 0x13,
			kBtfR16            = 0x14,
			kBtfGr16           = 0x15,
			/*! Legacy Format! */
			kBtfA4V6YB6A4U6YA6 = 0x16,
			kBtfRgba5551       = 0x17,
			kBtfHiLo8          = 0x18,
			kBtfHiLoS8         = 0x19,
			kBtfAbgr16f        = 0x1A,
			kBtfAbgr32f        = 0x1B,
			kBtfR32f           = 0x1C,
			kBtfXrgb1555       = 0x1D,
			kBtfXrgb8888       = 0x1E,
			kBtfGr16f          = 0x1F
		};
		
		//! Texture component swizzle values.
		enum
		{
			kSwizzleX			= 0x00,
			kSwizzleY			= 0x01,
			kSwizzleZ			= 0x02,
			kSwizzleW			= 0x03
		};
		
		#define MAKE_SWIZZLE(x, y, z, w) ((x << 6) | (y << 4) | (z << 2) | w)
		
		//! Texture component mux values.
		enum
		{
			kMuxZero			= 0x00,
			kMuxOne				= 0x01,
			kMuxPass			= 0x02
		};
		
		#define MAKE_MUX(x, y, z, w) ((x << 6) | (y << 4) | (z << 2) | w)
		
		//! Texture component signed values.
		enum
		{
			kCompUnsigned		= 0x00,
			kCompSigned			= 0x01
		};
		
		#define MAKE_SIGNS(x, y, z, w) ((x << 3) | (y << 2) | (z << 1) | w)
		
		//! Texture remap modes.
		enum
		{
			kRemapIdentity,
			kRemapExpandNormal
		};
		
		
		//! Texture format codes.
		/*! 8-bit swizzle: 2 bits/comp, 00=x 01=y 10=z 11=w
		    8-bit mux: 2 bits/comp, 00=pass, 01=zero, 10=one
		    Bit D set for LA16 format
		    4-bit signs: 1 bit/comp
		    7-bit format: see table
		    Bit R set for normal expand: c' = c * 2 - 1
		
		    |0             7|8            15|16           23|24           31|
		    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		    |R|   format    | signs |0 0 0|D|      mux      |   swizzle     |
		    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		
		    NOTE: custom formats using the kBtfGr16 base format will not work
		    as expected for now.
		*/
		
		#define MAKE_TEXTURE_FORMAT(base, swiz, mux, signs, remap) \
			((base << 24) | swiz | (mux << 8) | (signs << 20) | (remap << 31))
		
		
	}
}

#endif // ICE_TEXTUREFORMATS_H
