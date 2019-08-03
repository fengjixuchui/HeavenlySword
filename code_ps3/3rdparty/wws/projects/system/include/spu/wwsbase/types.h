/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Define base types for SPU
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_SPU_TYPES_H
#define WWS_SPU_TYPES_H

//--------------------------------------------------------------------------------------------------

typedef	signed char					I8;
typedef	unsigned char				U8;
typedef	signed short				I16;
typedef	unsigned short				U16;
typedef	signed int					I32;
typedef	unsigned int				U32;
typedef	long long					I64;
typedef	unsigned long long			U64;
//typedef qword						U128;
typedef	float						F32;
typedef	double						F64;

typedef U8							Bool8;
typedef U32							Bool32;

// vector types
// all vector types are 16-bytes in size
typedef vector unsigned char		VU8;    // 16 elements
typedef vector signed char			VI8;    // 16 elements

typedef vector unsigned short		VU16;   // 8 elements
typedef vector signed short			VI16;   // 8 elements

typedef vector unsigned int			VU32;   // 4 elements
typedef vector signed int			VI32;   // 4 elements

//typedef vector unsigned long		VU64;   // 2 elements
//typedef vector signed long		VI64;   // 2 elements

typedef vector signed short			VF16;   // 8 elements
typedef vector float				VF32;   // 4 elements
//typedef vector double				VF64;   // 2 elements

union QuadWord
{
	VU32	m_vu32;
	U64		m_u64[2];
	U32		m_u32[4];
	U16		m_u16[8];
	U8		m_u8[16];
};

//--------------------------------------------------------------------------------------------------

#endif /* WWS_SPU_TYPES_H */
