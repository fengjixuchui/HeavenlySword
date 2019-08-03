//--------------------------------------------------------------------------------------------------
/**
	@file		FwEndian.h

	@brief		endianness helper functions

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef FW_ENDIAN_H
#define FW_ENDIAN_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  SWITCHES
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  MACRO DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  DECLARATIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@func			FwByteSwap

	@brief			converts a number from big-endian to little-endian format, and vice-versa

	@param			a				-	number to byteswap (any base type that's valid in a structure)

	@return			byteswapped number
**/
//--------------------------------------------------------------------------------------------------

inline u8	FwByteSwap(u8 a)	{ return a; }
inline s8	FwByteSwap(s8 a)	{ return s8(FwByteSwap(u8(a))); }
inline u16	FwByteSwap(u16 a)	{ return (a << 8) | (a >> 8); }
inline u32	FwByteSwap(u32 a)	{ return (u32(FwByteSwap(u16(a))) << 16) | u32(FwByteSwap(u16(a >> 16))); }
inline u64	FwByteSwap(u64 a)	{ return (u64(FwByteSwap(u32(a))) << 32) | u64(FwByteSwap(u32(a >> 32))); }
inline s16	FwByteSwap(s16 a)	{ return s16(FwByteSwap(u16(a))); }
inline s32	FwByteSwap(s32 a)	{ return s32(FwByteSwap(u32(a))); }
inline s64	FwByteSwap(s64 a)	{ return s64(FwByteSwap(u64(a))); }
inline u128	FwByteSwap(u128 a)
{
	union
	{
		u128	r;
		u64		d[2];
	}	u,v;

	u.r = a;

	v.d[0] = FwByteSwap(u.d[1]);
	v.d[1] = FwByteSwap(u.d[0]);
    
	return v.r;
}

//--------------------------------------------------------------------------------------------------
/**
	@func			FwBigEndian

	@brief			converts a number to/from big-endian format

	This function converts a number to/from big-endian format.  Principally it converts to big-
	endian format, but since this is a nop for big-endian systems and a byteswap for little-endian
	systems, the same function can be used to convert *from* big-endian format.

	@param			a		-	number to convert (any non-FPU base type that's valid in a structure)

	@return			converted number

	@note			As a result of FPU mangling of potential non-numeric float representations when
					passing endian-swapped data by value, the versions of FwBigEndian that take f32
					and f64 types have been removed. Instead, applications must deal with the
					endian-swapping process differently based on direction of swap. For example, PC
					floats to PS3 floats (exported in a file) would set the m_f32 field of an
					FwFloatUnion, call FwBigEndian() on the m_u32 field, then write the m_u32 field.
**/
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@func			FwLittleEndian

	@brief			converts a number to/from little-endian format

	This function converts a number to/from little-endian format.  Principally it converts to little-
	endian format, but since this is a nop for little-endian systems and a byteswap for big-endian
	systems, the same function can be used to convert *from* little-endian format.

	@param			a		-	number to convert (any non-FPU base type that's valid in a structure)

	@return			converted number

	@note			As a result of FPU mangling of potential non-numeric float representations when
					passing endian-swapped data by value, the versions of FwLittleEndian that take f32
					and f64 types have been removed. Instead, applications must deal with the
					endian-swapping process differently based on direction of swap. For example, PC
					floats to PS3 floats (exported in a file) would set the m_f32 field of an
					FwFloatUnion, call FwBigEndian() on the m_u32 field, then write the m_u32 field.

**/
//--------------------------------------------------------------------------------------------------

union	FwFloatUnion
{
	f32		m_f32;
	u32		m_u32;
};

union	FwDoubleUnion
{
	f64		m_f64;
	u64		m_u64;
};

#if FW_BIG_ENDIAN

inline u8	FwBigEndian(u8 a)		{ return a; }
inline s8	FwBigEndian(s8 a)		{ return a; }
inline u16	FwBigEndian(u16 a)		{ return a; }
inline s16	FwBigEndian(s16 a)		{ return a; }
inline u32	FwBigEndian(u32 a)		{ return a; }
inline s32	FwBigEndian(s32 a)		{ return a; }
inline u64	FwBigEndian(u64 a)		{ return a; }
inline s64	FwBigEndian(s64 a)		{ return a; }
inline u128	FwBigEndian(u128 a)		{ return a; }
inline u8	FwLittleEndian(u8 a)	{ return FwByteSwap(a); }
inline s8	FwLittleEndian(s8 a)	{ return s8(FwLittleEndian(u8(a))); }
inline u16	FwLittleEndian(u16 a)	{ return FwByteSwap(a); }
inline s16	FwLittleEndian(s16 a)	{ return s16(FwLittleEndian(u16(a))); }
inline u32	FwLittleEndian(u32 a)	{ return FwByteSwap(a); }
inline s32	FwLittleEndian(s32 a)	{ return s32(FwLittleEndian(u32(a))); }
inline u64	FwLittleEndian(u64 a)	{ return FwByteSwap(a); }
inline s64	FwLittleEndian(s64 a)	{ return s64(FwLittleEndian(u64(a))); }
inline u128	FwLittleEndian(u128 a)	{ return FwByteSwap(a); }

#else

inline u8	FwBigEndian(u8 a)		{ return FwByteSwap(a); }
inline s8	FwBigEndian(s8 a)		{ return s8(FwBigEndian(u8(a))); }
inline u16	FwBigEndian(u16 a)		{ return FwByteSwap(a); }
inline s16	FwBigEndian(s16 a)		{ return s16(FwBigEndian(u16(a))); }
inline u32	FwBigEndian(u32 a)		{ return FwByteSwap(a); }
inline s32	FwBigEndian(s32 a)		{ return s32(FwBigEndian(u32(a))); }
inline u64	FwBigEndian(u64 a)		{ return FwByteSwap(a); }
inline s64	FwBigEndian(s64 a)		{ return s64(FwBigEndian(u64(a))); }
inline u128	FwBigEndian(u128 a)		{ return FwByteSwap(a); }
inline u8	FwLittleEndian(u8 a)	{ return a; }
inline s8	FwLittleEndian(s8 a)	{ return a; }
inline u16	FwLittleEndian(u16 a)	{ return a; }
inline s16	FwLittleEndian(s16 a)	{ return a; }
inline u32	FwLittleEndian(u32 a)	{ return a; }
inline s32	FwLittleEndian(s32 a)	{ return a; }
inline u64	FwLittleEndian(u64 a)	{ return a; }
inline s64	FwLittleEndian(s64 a)	{ return a; }
inline u128	FwLittleEndian(u128 a)	{ return a; }

#endif

//--------------------------------------------------------------------------------------------------

#endif // FW_ENDIAN_H
