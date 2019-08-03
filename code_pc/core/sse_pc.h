/***************************************************************************************************
*
*	$Header:: /game/sse.h 5     21/07/03 16:37 Dean                                                $
*
*	Implementation of SSE functionality for machines without SSE hardware
*
*	CHANGES
*
*	24/4/2003	Dean	Created
*
***************************************************************************************************/

#ifndef	CORE_SSE_PC_H
#define	CORE_SSE_PC_H

#define	SSE_INLINE	__forceinline
// Alignment
#define	_MM_ALIGN16(var) ALIGNTO(var, 16)

// Base structures

struct _MM_ALIGN16 __m128
{
	union
	{
		float	afVal[ 4 ];
		u_int	auiVal[ 4 ];
	};
};

typedef	const __m128& __m128_arg;

// Set operations
SSE_INLINE __m128	_mm_setzero_ps( void )
{
	__m128	obResult;
	obResult.auiVal[ 0 ] = 0;
	obResult.auiVal[ 1 ] = 0;
	obResult.auiVal[ 2 ] = 0;
	obResult.auiVal[ 3 ] = 0;
	return obResult;
};

SSE_INLINE __m128	_mm_set1_ps( float a )
{
	__m128	obResult;
	obResult.afVal[ 0 ] = a;
	obResult.afVal[ 1 ] = a;
	obResult.afVal[ 2 ] = a;
	obResult.afVal[ 3 ] = a;
	return obResult;
};

SSE_INLINE __m128	_mm_set_ss( float a )
{
	__m128	obResult;
	obResult.afVal[ 0 ] = a;
	obResult.afVal[ 1 ] = 0.0f;
	obResult.afVal[ 2 ] = 0.0f;
	obResult.afVal[ 3 ] = 0.0f;
	return obResult;
};

SSE_INLINE __m128	_mm_setr_ps( float x, float y, float z, float w )
{
	__m128	obResult;
	obResult.afVal[ 0 ] = x;
	obResult.afVal[ 1 ] = y;
	obResult.afVal[ 2 ] = z;
	obResult.afVal[ 3 ] = w;
	return obResult;
};

// Load operations

SSE_INLINE __m128	_mm_load_ss( float const* a )
{
	__m128	obResult;
	obResult.afVal[ 0 ] = *a;
	obResult.afVal[ 1 ] = 0.0f;
	obResult.afVal[ 2 ] = 0.0f;
	obResult.afVal[ 3 ] = 0.0f;
	return obResult;
};

SSE_INLINE __m128	_mm_load_ps( float const* a )
{
	__m128	obResult;
	obResult.afVal[ 0 ] = a[ 0 ];
	obResult.afVal[ 1 ] = a[ 1 ];
	obResult.afVal[ 2 ] = a[ 2 ];
	obResult.afVal[ 3 ] = a[ 3 ];
	return obResult;
};

// Store operations

SSE_INLINE void	_mm_store_ss( float* v, __m128_arg a )
{
	v[ 0 ] = a.afVal[ 0 ];
};

// Logical (bitwise) operations

SSE_INLINE __m128	_mm_and_ps( __m128_arg a, __m128_arg b )
{
	__m128	obResult;
	obResult.auiVal[ 0 ] = a.auiVal[ 0 ] & b.auiVal[ 0 ];
	obResult.auiVal[ 1 ] = a.auiVal[ 1 ] & b.auiVal[ 1 ];
	obResult.auiVal[ 2 ] = a.auiVal[ 2 ] & b.auiVal[ 2 ];
	obResult.auiVal[ 3 ] = a.auiVal[ 3 ] & b.auiVal[ 3 ];
	return obResult;
};

SSE_INLINE __m128	_mm_xor_ps( __m128_arg a, __m128_arg b )
{
	__m128	obResult;
	obResult.auiVal[ 0 ] = a.auiVal[ 0 ] ^ b.auiVal[ 0 ];
	obResult.auiVal[ 1 ] = a.auiVal[ 1 ] ^ b.auiVal[ 1 ];
	obResult.auiVal[ 2 ] = a.auiVal[ 2 ] ^ b.auiVal[ 2 ];
	obResult.auiVal[ 3 ] = a.auiVal[ 3 ] ^ b.auiVal[ 3 ];
	return obResult;
};

SSE_INLINE __m128	_mm_or_ps( __m128_arg a, __m128_arg b )
{
	__m128	obResult;
	obResult.auiVal[ 0 ] = a.auiVal[ 0 ] | b.auiVal[ 0 ];
	obResult.auiVal[ 1 ] = a.auiVal[ 1 ] | b.auiVal[ 1 ];
	obResult.auiVal[ 2 ] = a.auiVal[ 2 ] | b.auiVal[ 2 ];
	obResult.auiVal[ 3 ] = a.auiVal[ 3 ] | b.auiVal[ 3 ];
	return obResult;
};

// Basic maths operations (parallel)

SSE_INLINE __m128	_mm_sub_ss( __m128_arg a, __m128_arg b )
{
	__m128	obResult;
	obResult.afVal[ 0 ] = a.afVal[ 0 ] - b.afVal[ 0 ];
	obResult.afVal[ 1 ] = a.afVal[ 1 ];
	obResult.afVal[ 2 ] = a.afVal[ 2 ];
	obResult.afVal[ 3 ] = a.afVal[ 3 ];
	return obResult;
};

SSE_INLINE __m128	_mm_add_ss( __m128_arg a, __m128_arg b )
{
	__m128	obResult;
	obResult.afVal[ 0 ] = a.afVal[ 0 ] + b.afVal[ 0 ];
	obResult.afVal[ 1 ] = a.afVal[ 1 ];
	obResult.afVal[ 2 ] = a.afVal[ 2 ];
	obResult.afVal[ 3 ] = a.afVal[ 3 ];
	return obResult;
};

SSE_INLINE __m128	_mm_add_ps( __m128_arg a, __m128_arg b )
{
	__m128	obResult;
	obResult.afVal[ 0 ] = a.afVal[ 0 ] + b.afVal[ 0 ];
	obResult.afVal[ 1 ] = a.afVal[ 1 ] + b.afVal[ 1 ];
	obResult.afVal[ 2 ] = a.afVal[ 2 ] + b.afVal[ 2 ];
	obResult.afVal[ 3 ] = a.afVal[ 3 ] + b.afVal[ 3 ];
	return obResult;
};

SSE_INLINE __m128	_mm_sub_ps( __m128_arg a, __m128_arg b )
{
	__m128	obResult;
	obResult.afVal[ 0 ] = a.afVal[ 0 ] - b.afVal[ 0 ];
	obResult.afVal[ 1 ] = a.afVal[ 1 ] - b.afVal[ 1 ];
	obResult.afVal[ 2 ] = a.afVal[ 2 ] - b.afVal[ 2 ];
	obResult.afVal[ 3 ] = a.afVal[ 3 ] - b.afVal[ 3 ];
	return obResult;
};

SSE_INLINE __m128	_mm_mul_ps( __m128_arg a, __m128_arg b )
{
	__m128	obResult;
	obResult.afVal[ 0 ] = a.afVal[ 0 ] * b.afVal[ 0 ];
	obResult.afVal[ 1 ] = a.afVal[ 1 ] * b.afVal[ 1 ];
	obResult.afVal[ 2 ] = a.afVal[ 2 ] * b.afVal[ 2 ];
	obResult.afVal[ 3 ] = a.afVal[ 3 ] * b.afVal[ 3 ];
	return obResult;
};

SSE_INLINE __m128	_mm_div_ps( __m128_arg a, __m128_arg b )
{
	__m128	obResult;
	obResult.afVal[ 0 ] = a.afVal[ 0 ] / b.afVal[ 0 ];
	obResult.afVal[ 1 ] = a.afVal[ 1 ] / b.afVal[ 1 ];
	obResult.afVal[ 2 ] = a.afVal[ 2 ] / b.afVal[ 2 ];
	obResult.afVal[ 3 ] = a.afVal[ 3 ] / b.afVal[ 3 ];
	return obResult;
};

SSE_INLINE __m128	_mm_div_ss( __m128_arg a, __m128_arg b )
{
	__m128	obResult;
	obResult.afVal[ 0 ] = a.afVal[ 0 ] / b.afVal[ 0 ];
	obResult.afVal[ 1 ] = a.afVal[ 1 ];
	obResult.afVal[ 2 ] = a.afVal[ 2 ];
	obResult.afVal[ 3 ] = a.afVal[ 3 ];
	return obResult;
};

SSE_INLINE __m128	_mm_min_ps( __m128_arg a, __m128_arg b )
{
	__m128	obResult;
	obResult.afVal[ 0 ] = ntstd::Min( a.afVal[ 0 ], b.afVal[ 0 ] );
	obResult.afVal[ 1 ] = ntstd::Min( a.afVal[ 1 ], b.afVal[ 1 ] );
	obResult.afVal[ 2 ] = ntstd::Min( a.afVal[ 2 ], b.afVal[ 2 ] );
	obResult.afVal[ 3 ] = ntstd::Min( a.afVal[ 3 ], b.afVal[ 3 ] );
	return obResult;
};

SSE_INLINE __m128	_mm_max_ps( __m128_arg a, __m128_arg b )
{
	__m128	obResult;
	obResult.afVal[ 0 ] = ntstd::Max( a.afVal[ 0 ], b.afVal[ 0 ] );
	obResult.afVal[ 1 ] = ntstd::Max( a.afVal[ 1 ], b.afVal[ 1 ] );
	obResult.afVal[ 2 ] = ntstd::Max( a.afVal[ 2 ], b.afVal[ 2 ] );
	obResult.afVal[ 3 ] = ntstd::Max( a.afVal[ 3 ], b.afVal[ 3 ] );
	return obResult;
};

SSE_INLINE __m128	_mm_sqrt_ss( __m128_arg a )
{
	__m128	obResult;
	obResult.afVal[ 0 ] = sqrtf( a.afVal[ 0 ] );
	obResult.afVal[ 1 ] = a.afVal[ 1 ];
	obResult.afVal[ 2 ] = a.afVal[ 2 ];
	obResult.afVal[ 3 ] = a.afVal[ 3 ];
	return obResult;
};

SSE_INLINE __m128	_mm_rsqrt_ss( __m128_arg a )
{
	__m128	obResult;
	obResult.afVal[ 0 ] = 1.0f / ( sqrtf( a.afVal[ 0 ] ) );
	obResult.afVal[ 1 ] = a.afVal[ 1 ];
	obResult.afVal[ 2 ] = a.afVal[ 2 ];
	obResult.afVal[ 3 ] = a.afVal[ 3 ];
	return obResult;
};

// Miscellaneous operations

SSE_INLINE __m128	_mm_shuffle_ps( __m128_arg a, __m128_arg b, unsigned int imm8 )
{
	__m128	obResult;
	obResult.auiVal[ 0 ] = a.auiVal[ ( imm8 & 0x03 ) >> 0 ];
	obResult.auiVal[ 1 ] = a.auiVal[ ( imm8 & 0x0c ) >> 2 ];
	obResult.auiVal[ 2 ] = b.auiVal[ ( imm8 & 0x30 ) >> 4 ];
	obResult.auiVal[ 3 ] = b.auiVal[ ( imm8 & 0xc0 ) >> 6 ];
	return obResult;
};

SSE_INLINE __m128	_mm_movehl_ps( __m128_arg a, __m128_arg b )
{
	__m128	obResult;
	obResult.afVal[ 3 ] = a.afVal[ 3 ];
	obResult.afVal[ 2 ] = a.afVal[ 2 ];
	obResult.afVal[ 1 ] = b.afVal[ 3 ];
	obResult.afVal[ 0 ] = b.afVal[ 2 ];
	return obResult;
};

SSE_INLINE __m128	_mm_movelh_ps( __m128_arg a, __m128_arg b )
{
	__m128	obResult;
	obResult.afVal[ 3 ] = b.afVal[ 1 ];
	obResult.afVal[ 2 ] = b.afVal[ 0 ];
	obResult.afVal[ 1 ] = a.afVal[ 1 ];
	obResult.afVal[ 0 ] = a.afVal[ 0 ];
	return obResult;
};

SSE_INLINE __m128	_mm_unpackhi_ps( __m128_arg a, __m128_arg b )
{
	__m128	obResult;
	obResult.afVal[ 0 ] = a.afVal[ 2 ];
	obResult.afVal[ 1 ] = b.afVal[ 2 ];
	obResult.afVal[ 2 ] = a.afVal[ 3 ];
	obResult.afVal[ 3 ] = b.afVal[ 3 ];
	return obResult;
}

SSE_INLINE __m128	_mm_unpacklo_ps( __m128_arg a, __m128_arg b )
{
	__m128	obResult;
	obResult.afVal[ 0 ] = a.afVal[ 0 ];
	obResult.afVal[ 1 ] = b.afVal[ 0 ];
	obResult.afVal[ 2 ] = a.afVal[ 1 ];
	obResult.afVal[ 3 ] = b.afVal[ 1 ];
	return obResult;
}

SSE_INLINE int		_mm_comieq_ss( __m128_arg a, __m128_arg b )
{
	return ( a.afVal[ 0 ] == b.afVal[ 0 ] ) ? 0x01 : 0x00;
}

#endif	//_SSE_H