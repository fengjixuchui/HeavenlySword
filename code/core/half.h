//--------------------------------------------------
//!
//!	\file half.h
//!	functionality for an s10e5 software float type,
//! for generation of 'half' floating point data 
//!
//--------------------------------------------------

#ifndef CORE_HALF_H
#define CORE_HALF_H

//-----------------------------------------------------
//!
//!	NTFLOAT16
//! Software half format. Warning, this is SLOW!
//! DO NOT add aritmetic operators. this class is for
//! data generation / conversion purposes only.
//!
//-----------------------------------------------------
class NTFLOAT16
{
public:
    NTFLOAT16() {};
	NTFLOAT16( float fValue )		{ m_half = FloatToHalf(fValue); }
    NTFLOAT16( const NTFLOAT16& f ) { m_half = f.m_half; }

    // casting
	operator float () { return HalfToFloat(m_half); }

    // binary operators
	bool operator == ( const NTFLOAT16& f ) const { return m_half == f.m_half; }
	bool operator != ( const NTFLOAT16& f ) const { return m_half != f.m_half; }

protected:
	static uint32_t	FloatBits( float f ) { return *((uint32_t*)(&f)); }
	static int16_t	FloatToHalf( float value );
	static float	HalfToFloat( int16_t half );

	int16_t m_half;
};

#endif // CORE_HALF_H
