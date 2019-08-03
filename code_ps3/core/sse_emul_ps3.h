#ifndef _SSE_EMUL_PS3_H_
#define _SSE_EMUL_PS3_H_

struct __m128
{
	union
	{
		float		afVal[ 4 ];
		uint32_t	auiVal[ 4 ];
		vector float vReg;
	};
};

typedef	const __m128& __m128_arg;

static __m128 vzero = { 0.0f, 0.0f, 0.0f, 0.0f };
static vector unsigned int vselect = { 0xffffffffL, 0x0L, 0x0L, 0x0L };
// Oh sweet Lord, How I love to swizzle, to permute and to shuffle!

static vector unsigned char vperm_movehl = {0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f};
static vector unsigned char vperm_movelh = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17};
static vector unsigned char vperm_unpackhi = {0x08,0x09,0x0a,0x0b,0x18,0x19,0x1a,0x1b,0x0c,0x0d,0x0e,0x0f,0x1c,0x1d,0x1e,0x1f};
static vector unsigned char vperm_unpacklo = {0x00,0x01,0x02,0x03,0x10,0x11,0x12,0x13,0x04,0x05,0x06,0x07,0x14,0x15,0x16,0x17};



// Set operations
inline __m128	_mm_setzero_ps( void )
{
/*	__m128	obResult; 
	obResult.auiVal[ 0 ] = 0;
	obResult.auiVal[ 1 ] = 0;
	obResult.auiVal[ 2 ] = 0;
	obResult.auiVal[ 3 ] = 0;
	return obResult;*/

	//a faster version would just subtract a register by itself but we don't know if a register does contain a valid 4-vector
	__m128	obResult; 
	obResult.vReg = vec_splat( vzero.vReg, 0);
	return obResult;
};

inline __m128	_mm_set1_ps( float a )
{
/*	__m128	obResult;
	obResult.afVal[ 0 ] = a;
	obResult.afVal[ 1 ] = a;
	obResult.afVal[ 2 ] = a;
	obResult.afVal[ 3 ] = a;
	return obResult; */

	__m128	obResult; 
	__m128	scalar;
	scalar.afVal[0] = a;
	obResult.vReg = vec_splat( scalar.vReg, 0);
	return obResult;
};

inline __m128	_mm_set_ss( float a )
{
/*	__m128	obResult;
	obResult.afVal[ 0 ] = a;
	obResult.afVal[ 1 ] = 0.0f;
	obResult.afVal[ 2 ] = 0.0f;
	obResult.afVal[ 3 ] = 0.0f;
	return obResult;*/
	__m128	obResult; 
	__m128	scalar;
	scalar.afVal[0] = a;
	obResult.vReg = vec_sel( vzero.vReg, scalar.vReg, vselect);
	return obResult;
};

inline __m128	_mm_setr_ps( float x, float y, float z, float w )
{
	__m128	obResult;
	obResult.afVal[ 0 ] = x;
	obResult.afVal[ 1 ] = y;
	obResult.afVal[ 2 ] = z;
	obResult.afVal[ 3 ] = w;
	return obResult; 
};

// Load operations

inline  __m128	_mm_load_ss( float const* a )
{
	__m128	obResult;
	obResult.vReg = vec_splat( vzero.vReg, 0);
	obResult.afVal[ 0 ] = *a;
	return obResult;
};

inline  __m128	_mm_load_ps( float const* a )
{
/*	__m128	obResult;
	obResult.afVal[ 0 ] = a[ 0 ];
	obResult.afVal[ 1 ] = a[ 1 ];
	obResult.afVal[ 2 ] = a[ 2 ];
	obResult.afVal[ 3 ] = a[ 3 ];
	return obResult;*/
	__m128	obResult; 
	obResult.vReg = vec_ldl( 0, static_cast<const float*>(a));
	return obResult;
};
  
// Store operations

inline  void	_mm_store_ss( float* v, __m128_arg a )
{
	v[ 0 ] = a.afVal[ 0 ];
};

// Logical (bitwise) operations

inline  __m128	_mm_and_ps( __m128_arg a, __m128_arg b )
{
/*	__m128	obResult;
	obResult.auiVal[ 0 ] = a.auiVal[ 0 ] & b.auiVal[ 0 ];
	obResult.auiVal[ 1 ] = a.auiVal[ 1 ] & b.auiVal[ 1 ];
	obResult.auiVal[ 2 ] = a.auiVal[ 2 ] & b.auiVal[ 2 ];
	obResult.auiVal[ 3 ] = a.auiVal[ 3 ] & b.auiVal[ 3 ];
	return obResult;*/
	__m128	obResult;
	obResult.vReg = vec_and( a.vReg, b.vReg );
	return obResult;
};

inline  __m128	_mm_xor_ps( __m128_arg a, __m128_arg b )
{
/*	__m128	obResult;
	obResult.auiVal[ 0 ] = a.auiVal[ 0 ] ^ b.auiVal[ 0 ];
	obResult.auiVal[ 1 ] = a.auiVal[ 1 ] ^ b.auiVal[ 1 ];
	obResult.auiVal[ 2 ] = a.auiVal[ 2 ] ^ b.auiVal[ 2 ];
	obResult.auiVal[ 3 ] = a.auiVal[ 3 ] ^ b.auiVal[ 3 ];
	return obResult;*/
	__m128	obResult;
	obResult.vReg = vec_xor( a.vReg, b.vReg );
	return obResult;
};

inline  __m128	_mm_or_ps( __m128_arg a, __m128_arg b )
{
/*	__m128	obResult;
	obResult.auiVal[ 0 ] = a.auiVal[ 0 ] | b.auiVal[ 0 ];
	obResult.auiVal[ 1 ] = a.auiVal[ 1 ] | b.auiVal[ 1 ];
	obResult.auiVal[ 2 ] = a.auiVal[ 2 ] | b.auiVal[ 2 ];
	obResult.auiVal[ 3 ] = a.auiVal[ 3 ] | b.auiVal[ 3 ];
	return obResult;*/
	__m128	obResult;
	obResult.vReg = vec_xor( a.vReg, b.vReg );
	return obResult;
};

// Basic maths operations (parallel)

inline  __m128	_mm_sub_ss( __m128_arg a, __m128_arg b )
{
/*
	__m128	obResult;
	obResult.afVal[ 0 ] = a.afVal[ 0 ] - b.afVal[ 0 ];
	obResult.afVal[ 1 ] = a.afVal[ 1 ];
	obResult.afVal[ 2 ] = a.afVal[ 2 ];
	obResult.afVal[ 3 ] = a.afVal[ 3 ];
	return obResult;*/
	__m128	obResult;
	__m128	temp;
	temp.vReg = vec_sub( a.vReg, b.vReg );
	obResult.vReg = vec_sel(a.vReg, temp.vReg, vselect);
	return obResult;
};

inline  __m128	_mm_add_ss( __m128_arg a, __m128_arg b )
{
/*	__m128	obResult;
	obResult.afVal[ 0 ] = a.afVal[ 0 ] + b.afVal[ 0 ];
	obResult.afVal[ 1 ] = a.afVal[ 1 ];
	obResult.afVal[ 2 ] = a.afVal[ 2 ];
	obResult.afVal[ 3 ] = a.afVal[ 3 ];
	return obResult;*/
	__m128	obResult;
	__m128	temp;
	temp.vReg = vec_add( a.vReg, b.vReg );
	obResult.vReg = vec_sel(a.vReg, temp.vReg, vselect);
	return obResult;
};

inline  __m128	_mm_add_ps( __m128_arg a, __m128_arg b )
{ 
/*	__m128	obResult;
	obResult.afVal[ 0 ] = a.afVal[ 0 ] + b.afVal[ 0 ];
	obResult.afVal[ 1 ] = a.afVal[ 1 ] + b.afVal[ 1 ];
	obResult.afVal[ 2 ] = a.afVal[ 2 ] + b.afVal[ 2 ];
	obResult.afVal[ 3 ] = a.afVal[ 3 ] + b.afVal[ 3 ];
	return obResult; */

/*	__m128 obResult;

	const vector float vA = *( reinterpret_cast<const vector float*>( &a ));
	const vector float vB = *( reinterpret_cast<const vector float*>( &b ));
	vector float vResult = vec_add( vA, vB );

	obResult = *( reinterpret_cast<__m128*>(&vResult ));
	return obResult;*/

	__m128 obResult;
	obResult.vReg  = vec_add(a.vReg, b.vReg);
	return obResult;
};

inline  __m128	_mm_sub_ps( __m128_arg a, __m128_arg b )
{
/*	__m128	obResult;
	obResult.afVal[ 0 ] = a.afVal[ 0 ] - b.afVal[ 0 ];
	obResult.afVal[ 1 ] = a.afVal[ 1 ] - b.afVal[ 1 ];
	obResult.afVal[ 2 ] = a.afVal[ 2 ] - b.afVal[ 2 ];
	obResult.afVal[ 3 ] = a.afVal[ 3 ] - b.afVal[ 3 ];
	return obResult;*/

	__m128 obResult;
	obResult.vReg  = vec_sub(a.vReg, b.vReg);
	return obResult;
};

inline  __m128	_mm_mul_ps( __m128_arg a, __m128_arg b )
{
/*	__m128	obResult;
	obResult.afVal[ 0 ] = a.afVal[ 0 ] * b.afVal[ 0 ];
	obResult.afVal[ 1 ] = a.afVal[ 1 ] * b.afVal[ 1 ];
	obResult.afVal[ 2 ] = a.afVal[ 2 ] * b.afVal[ 2 ];
	obResult.afVal[ 3 ] = a.afVal[ 3 ] * b.afVal[ 3 ];
	return obResult;*/
	__m128 obResult;
	obResult.vReg  = vec_madd(a.vReg, b.vReg, vzero.vReg);
	return obResult;
};

inline  __m128	_mm_div_ps( __m128_arg a, __m128_arg b )
{
/*	__m128	obResult;
	obResult.afVal[ 0 ] = a.afVal[ 0 ] / b.afVal[ 0 ];
	obResult.afVal[ 1 ] = a.afVal[ 1 ] / b.afVal[ 1 ];
	obResult.afVal[ 2 ] = a.afVal[ 2 ] / b.afVal[ 2 ];
	obResult.afVal[ 3 ] = a.afVal[ 3 ] / b.afVal[ 3 ];
	return obResult;*/
	__m128 obResult, reciprocal;
	reciprocal.vReg = vec_re( b.vReg );
	obResult.vReg  = vec_madd(a.vReg, reciprocal.vReg, vzero.vReg);
	return obResult;

};

inline  __m128	_mm_div_ss( __m128_arg a, __m128_arg b )
{
/*	__m128	obResult;
	obResult.afVal[ 0 ] = a.afVal[ 0 ] / b.afVal[ 0 ];
	obResult.afVal[ 1 ] = a.afVal[ 1 ];
	obResult.afVal[ 2 ] = a.afVal[ 2 ];
	obResult.afVal[ 3 ] = a.afVal[ 3 ];
	return obResult;*/
	__m128	obResult, temp, reciprocal;
	reciprocal.vReg = vec_re( b.vReg );
	temp.vReg = vec_madd( a.vReg, reciprocal.vReg, vzero.vReg );
	obResult.vReg = vec_sel(a.vReg, temp.vReg, vselect);
	return obResult;
};

inline  __m128	_mm_min_ps( __m128_arg a, __m128_arg b )
{
/*	__m128	obResult;
	obResult.afVal[ 0 ] = ntstd::Min( a.afVal[ 0 ], b.afVal[ 0 ] );
	obResult.afVal[ 1 ] = ntstd::Min( a.afVal[ 1 ], b.afVal[ 1 ] );
	obResult.afVal[ 2 ] = ntstd::Min( a.afVal[ 2 ], b.afVal[ 2 ] );
	obResult.afVal[ 3 ] = ntstd::Min( a.afVal[ 3 ], b.afVal[ 3 ] );
	return obResult;*/
	__m128	obResult;
	obResult.vReg = vec_min(a.vReg, b.vReg);
	return obResult;
};

inline  __m128	_mm_max_ps( __m128_arg a, __m128_arg b )
{
/*	__m128	obResult;
	obResult.afVal[ 0 ] = ntstd::Max( a.afVal[ 0 ], b.afVal[ 0 ] );
	obResult.afVal[ 1 ] = ntstd::Max( a.afVal[ 1 ], b.afVal[ 1 ] );
	obResult.afVal[ 2 ] = ntstd::Max( a.afVal[ 2 ], b.afVal[ 2 ] );
	obResult.afVal[ 3 ] = ntstd::Max( a.afVal[ 3 ], b.afVal[ 3 ] );
	return obResult;*/
	__m128	obResult;
	obResult.vReg = vec_max(a.vReg, b.vReg);
	return obResult;
};

inline  __m128	_mm_sqrt_ss( __m128_arg a )
{
/*	__m128	obResult;
	obResult.afVal[ 0 ] = sqrtf( a.afVal[ 0 ] );
	obResult.afVal[ 1 ] = a.afVal[ 1 ];
	obResult.afVal[ 2 ] = a.afVal[ 2 ];
	obResult.afVal[ 3 ] = a.afVal[ 3 ];
	return obResult;*/
	__m128	obResult, temp, reciprocalsquareroot;
	reciprocalsquareroot.vReg = vec_rsqrte( a.vReg );
	temp.vReg = vec_re( reciprocalsquareroot.vReg );
	obResult.vReg = vec_sel(a.vReg, temp.vReg, vselect);
	return obResult;

};

inline  __m128	_mm_rsqrt_ss( __m128_arg a )
{
/*	__m128	obResult;
	obResult.afVal[ 0 ] = 1.0f / ( sqrtf( a.afVal[ 0 ] ) );
	obResult.afVal[ 1 ] = a.afVal[ 1 ];
	obResult.afVal[ 2 ] = a.afVal[ 2 ];
	obResult.afVal[ 3 ] = a.afVal[ 3 ];
	return obResult;*/
	__m128	obResult, reciprocalsquareroot;
	reciprocalsquareroot.vReg = vec_rsqrte( a.vReg );
	obResult.vReg = vec_sel(a.vReg, reciprocalsquareroot.vReg, vselect);
	return obResult;
};

// Miscellaneous operations

inline  __m128	_mm_shuffle_ps( __m128_arg a, __m128_arg b, unsigned int imm8 )
{
	// This function could be implemented via vec_vperm intrinsic but to generate the shuffling argument
	// vec_vperm needs it's more complex than just doing what we are doing now with the 'old' see function..
	__m128	obResult;
	obResult.auiVal[ 0 ] = a.auiVal[ ( imm8 & 0x03 ) >> 0 ];
	obResult.auiVal[ 1 ] = a.auiVal[ ( imm8 & 0x0c ) >> 2 ];
	obResult.auiVal[ 2 ] = b.auiVal[ ( imm8 & 0x30 ) >> 4 ];
	obResult.auiVal[ 3 ] = b.auiVal[ ( imm8 & 0xc0 ) >> 6 ];
	return obResult;
};

inline  __m128	_mm_movehl_ps( __m128_arg a, __m128_arg b )
{
	/*__m128	obResult;
	obResult.afVal[ 3 ] = a.afVal[ 3 ];
	obResult.afVal[ 2 ] = a.afVal[ 2 ];
	obResult.afVal[ 1 ] = b.afVal[ 3 ];
	obResult.afVal[ 0 ] = b.afVal[ 2 ];
	return obResult;*/
	__m128	obResult;
	obResult.vReg = vec_perm( a.vReg, b.vReg, vperm_movehl);
	return obResult;

};

inline __m128	_mm_movelh_ps( __m128_arg a, __m128_arg b )
{
/*	__m128	obResult;
	obResult.afVal[ 3 ] = b.afVal[ 1 ];
	obResult.afVal[ 2 ] = b.afVal[ 0 ];
	obResult.afVal[ 1 ] = a.afVal[ 1 ];
	obResult.afVal[ 0 ] = a.afVal[ 0 ];
	return obResult;*/
	__m128	obResult;
	obResult.vReg = vec_perm( a.vReg, b.vReg, vperm_movelh);
	return obResult;
};

inline __m128	_mm_unpackhi_ps( __m128_arg a, __m128_arg b )
{
/*	__m128	obResult;
	obResult.afVal[ 0 ] = a.afVal[ 2 ];
	obResult.afVal[ 1 ] = b.afVal[ 2 ];
	obResult.afVal[ 2 ] = a.afVal[ 3 ];
	obResult.afVal[ 3 ] = b.afVal[ 3 ];
	return obResult;*/
	__m128	obResult;
	obResult.vReg = vec_perm( a.vReg, b.vReg, vperm_unpackhi);
	return obResult;
}

inline __m128	_mm_unpacklo_ps( __m128_arg a, __m128_arg b )
{
/*	__m128	obResult;
	obResult.afVal[ 0 ] = a.afVal[ 0 ];
	obResult.afVal[ 1 ] = b.afVal[ 0 ];
	obResult.afVal[ 2 ] = a.afVal[ 1 ];
	obResult.afVal[ 3 ] = b.afVal[ 1 ];
	return obResult;*/
	__m128	obResult;
	obResult.vReg = vec_perm( a.vReg, b.vReg, vperm_unpacklo);
	return obResult;
}

inline int		_mm_comieq_ss( __m128_arg a, __m128_arg b )
{
	return ( a.afVal[ 0 ] == b.afVal[ 0 ] ) ? 0x01 : 0x00;
}


#endif //_SSE_PS3_EMUL_H_
