//--------------------------------------------------------------------------------------------------
/**
	@file		FwTypes.h
	
	@brief		Core data types

	The core data types are intended as a complete set of base types for use in programs.  Framework
	code does not use any base types that are not defined or documented in this file.  The full set
	of permissible base types is:

	"Sized" types (suitable for structures)

	s8			u8
	s16			u16
	s32			u32
	s64			u64
	s128		u128
	f32			f64
	float		double

	"Unsized" types (unsuitable for structures)

	int			uint
	size_t		ptrdiff_t
	char
	wchar_t
	bool

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef	FW_TYPES_H
#define	FW_TYPES_H

//--------------------------------------------------------------------------------------------------
/**
	@typedef		s8

	@brief			signed integer type with 8 bits; expresses values -128 to +127
**/
//--------------------------------------------------------------------------------------------------

typedef	signed char			s8;

//--------------------------------------------------------------------------------------------------
/**
	@typedef		u8

	@brief			unsigned integer type with 8 bits; expresses values 0 to 255
**/
//--------------------------------------------------------------------------------------------------

typedef	unsigned char		u8;

//--------------------------------------------------------------------------------------------------
/**
	@typedef		s16

	@brief			signed integer type with 16 bits; expresses values -32768 to +32767
**/
//--------------------------------------------------------------------------------------------------

typedef	signed short		s16;

//--------------------------------------------------------------------------------------------------
/**
	@typedef		u16

	@brief			unsigned integer type with 16 bits; expresses values 0 to 65535
**/
//--------------------------------------------------------------------------------------------------

typedef	unsigned short		u16;

//--------------------------------------------------------------------------------------------------
/**
	@typedef		s32

	@brief			signed integer type with 32 bits; expresses values -2147483648 to +2147483647
**/
//--------------------------------------------------------------------------------------------------

typedef	signed int			s32;

//--------------------------------------------------------------------------------------------------
/**
	@typedef		u32

	@brief			unsigned integer type with 32 bits; expresses values 0 to 4294967295
**/
//--------------------------------------------------------------------------------------------------

typedef	unsigned int		u32;

//--------------------------------------------------------------------------------------------------
/**
	@typedef		s64

	@brief			signed integer type with 64 bits; expresses values -9223372036854775808 to
					9223372036854775807
**/
//--------------------------------------------------------------------------------------------------

typedef	signed long long	s64;

//--------------------------------------------------------------------------------------------------
/**
	@typedef		u64

	@brief			unsigned integer type with 64 bits; expresses values 0 to 18446744073709551616
**/
//--------------------------------------------------------------------------------------------------

typedef	unsigned long long	u64;

//--------------------------------------------------------------------------------------------------
/**
	@typedef		s128

	@brief			signed integer type with 128 bits; arithmetic is not possible with this type
	
	@note			not defined - there is no meaningful concept of an s128 on a PC ...
					if you find a use for it, please comment it back in and let us know what it is!
**/
//--------------------------------------------------------------------------------------------------

//typedef __m128				s128;

//--------------------------------------------------------------------------------------------------
/**
	@typedef		u128

	@brief			unsigned integer type with 128 bits; arithmetic is not possible with this type
**/
//--------------------------------------------------------------------------------------------------

#if defined(__PPC__)
typedef vector unsigned int	u128;
#elif defined(__SPU__)
typedef	qword	u128;
#elif defined(_WIN32) || defined(__i386__)
typedef __m128				u128;
#else
#error Target is not supported
#endif

//--------------------------------------------------------------------------------------------------
/**
	@typedef		f32

	@brief			floating-point type with 32 bits; used for pedantry only - float is acceptable
**/
//--------------------------------------------------------------------------------------------------

typedef	float				f32;

//--------------------------------------------------------------------------------------------------
/**
	@typedef		f64

	@brief			floating-point type with 64 bits; used for pedantry only - double is acceptable
**/
//--------------------------------------------------------------------------------------------------

typedef	double				f64;

//--------------------------------------------------------------------------------------------------

// Enable the documentation of various stddef.h types.
//
// @todo Get Doxygen to document these types; tried but couldn't find an adequate solution.

//--------------------------------------------------------------------------------------------------
/**
	@typedef		size_t

	@brief			unsigned integer type that is the same size as a pointer
					used for converting pointers to integer, or to hold addresses

	@note			Do not use in structures that need to be portable from 32-bit to/from 64-bit
**/
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@typedef		ptrdiff_t

	@brief			signed integer type that is the same size as a pointer
					used for converting the difference between two pointers to integer, or to hold
					offsets, or to hold the result of subtracting amything from a size_t (except
					size_t - u64 = u64).

	@note			Do not use in structures that need to be portable from 32-bit to/from 64-bit
**/
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@typedef		int

	@brief			signed integer type that is the "natural" size for the platform, and that is
					therefore optimal to do arithmetic on
					should be used in for loops or for function parameters, provided the range fits
					in an s32 (u8,u16,s8,s16,s32 or int)

	@note			Do not use in structures that need to be portable from 32-bit to/from 64-bit
**/
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@typedef		uint

	@brief			unsigned integer type that is the "natural" size for the platform, and that is
					therefore optimal to do arithmetic on
					
					should be used in for loops or for function parameters, provided the range fits
					in a u32 (u8,u16,u32 or uint)

	@note			Do not use in structures that need to be portable from 32-bit to/from 64-bit
**/
//--------------------------------------------------------------------------------------------------

typedef unsigned int uint;

//--------------------------------------------------------------------------------------------------
/**
	@typedef		char

	@brief			1-byte character type, MAY BE SIGNED OR UNSIGNED, should only be used for 7-bit
					ASCII (use u8 for UTF-8 or non-character data)
**/
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@typedef		wchar_t

	@brief			2-byte (UNICODE) character type, do not use for non-character data
**/
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@typedef		bool

	@brief			C++ boolean type - may only take the values true or false

	@note			Do not use in structures - the size is indeterminate
**/
//--------------------------------------------------------------------------------------------------

#include <Fw/FwPtrType.h>

#endif	//FW_TYPES_H
