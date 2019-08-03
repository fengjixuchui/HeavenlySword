//--------------------------------------------------
//!
//!	\file half.cpp
//!	functionality for an s10e5 software float type,
//! for generation of 'half' floating point data 
//!
//--------------------------------------------------

#include "core/half.h"

//--------------------------------------------------
//!
//!	FloatToHalf
//!	Convert a s23e8 float to a s10e5 float.
//!
//--------------------------------------------------
int16_t NTFLOAT16::FloatToHalf( float value )
{
	// We need the float as bits..
	int32_t i = (int32_t) FloatBits( value );

	// Our floating point number, value, is represented by the bit
	// pattern in integer i.  Disassemble that bit pattern into
	// the sign, s, the exponent, e, and the significand, m.
	// Shift s into the position where it will go in in the
	// resulting half number.
	// Adjust e, accounting for the different exponent bias
	// of float and half (127 versus 15).

	register int32_t s =  (i >> 16)	& 0x00008000;
	register int32_t e = ((i >> 23)	& 0x000000ff) - (127 - 15);
	register int32_t m =   i		& 0x007fffff;

	if (e <= 0)
	{
		if (e < -10)
		{
			// E is less than -10.  The absolute value of f is
			// less than HALF_MIN (f may be a small normalized
			// float, a denormalized float or a zero).
			//
			// We convert f to a half zero.

			return 0;
		}

		// E is between -10 and 0.  F is a normalized float,
		// whose magnitude is less than HALF_NRM_MIN.
		//
		// We convert f to a denormalized half.

		m = (m | 0x00800000) >> (1 - e);

		// Round to nearest, round "0.5" up.
		//
		// Rounding may cause the significand to overflow and make
		// our number normalized.  Because of the way a half's bits
		// are laid out, we don't have to treat this case separately;
		// the code below will handle it correctly.

		if (m &  0x00001000)
			m += 0x00002000;

		// Assemble the half from s, e (zero) and m.

		return ( int16_t )( s | (m >> 13) );
	}
	else
	if (e == 0xff - (127 - 15))
    {
		if (m == 0)
		{

			// F is an infinity; convert f to a half
			// infinity with the same sign as f.

		    return ( int16_t )( s | 0x7c00 );
		}
		else
		{	
			// F is a NAN; we produce a half NAN that preserves
			// the sign bit and the 10 leftmost bits of the
			// significand of f, with one exception: If the 10
			// leftmost bits are all zero, the NAN would turn 
			// into an infinity, so we have to set at least one
			// bit in the significand.
	
			m >>= 13;
			return ( int16_t )( s | 0x7c00 | m | (m == 0) );
		}
	}
    else
    {
		// E is greater than zero.  F is a normalized float. We try to convert f to a normalized half.

		// Round to nearest, round "0.5" up

		if (m &  0x00001000)
		{
			m += 0x00002000;

			if (m & 0x00800000)
			{
				m =  0;		// overflow in significand,
				e += 1;		// adjust exponent
			}
		}
	}

	// Handle exponent overflow

	if (e > 30)
	{
		return ( int16_t )( s | 0x7c00 );	// if this returns, the half becomes an infinity with the same sign as f.
	}

	// Assemble the half from s, e and m.

	return ( int16_t )( s | (e << 10) | (m >> 13) );
}

//--------------------------------------------------
//!
//!	HalfToFloat
//!	Convert a s10e5 float to a s23e8 float.
//!
//--------------------------------------------------
float NTFLOAT16::HalfToFloat( int16_t half )
{
	int32_t s = ( half >> 15 ) & 0x00000001;
	int32_t e = ( half >> 10 ) & 0x0000001f;
	int32_t m =  half          & 0x000003ff;

	// We need to convert from float bit pattern to real floating point number
	union
	{
		float		val;
		uint32_t	valBits;
	} result;

	if (e == 0)
	{
		if (m == 0)
		{
			// Plus or minus zero
			result.valBits = s << 31;
			return result.val;
		}
		else
		{
			// Denormalized number -- renormalize it
			while ( !( m & 0x00000400 ) )
			{
				m <<= 1;
				e -=  1;
			}

			e += 1;
			m &= ~0x00000400;
		}
	}
	else if (e == 31)
	{
		if (m == 0)
		{
			// Positive or negative infinity
			result.valBits = ( s << 31 ) | 0x7f800000;
			return result.val;
		}
		else
		{
			// Nan -- preserve sign and significand bits
			result.valBits = ( s << 31 ) | 0x7f800000 | ( m << 13 );
			return result.val;
		}
	}

	// Normalized number
	e = e + ( 127 - 15 );
	m = m << 13;

	// Assemble s, e and m.
	result.valBits = ( s << 31 ) | ( e << 23 ) | m;
	return result.val;
}


