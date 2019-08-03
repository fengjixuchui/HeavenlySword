/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkbase/fwd/hkcmath.h>
#include <hkbase/fwd/hkcfloat.h>

enum hkVector4Shuffle
{
	HK_VECTOR4_SHUF_X = 0x00010203,
	HK_VECTOR4_SHUF_Y = 0x04050607,
	HK_VECTOR4_SHUF_Z = 0x08090a0b,
	HK_VECTOR4_SHUF_W = 0x0c0d0e0f,
	HK_VECTOR4_SHUF_A = 0x10111213,
	HK_VECTOR4_SHUF_B = 0x14151617,
	HK_VECTOR4_SHUF_C = 0x18191a1b,
	HK_VECTOR4_SHUF_D = 0x1c1d1e1f,
	HK_VECTOR4_SHUF__ = 0x00000000
};
#define HK_VECTOR4_SHUF_0 HK_VECTOR4_SHUF_X
#define HK_VECTOR4_SHUF_1 HK_VECTOR4_SHUF_Y
#define HK_VECTOR4_SHUF_2 HK_VECTOR4_SHUF_Z
#define HK_VECTOR4_SHUF_3 HK_VECTOR4_SHUF_W

#define HK_VECTOR4_SHUFFLE(_a,_b,_c,_d) ((vector unsigned char)(vector unsigned int){HK_VECTOR4_SHUF_##_a, HK_VECTOR4_SHUF_##_b, HK_VECTOR4_SHUF_##_c,HK_VECTOR4_SHUF_##_d })

#if defined(__PPU__)
#	include <altivec.h>
	typedef vector float hkQuadReal;
#	define vec_rotl(_a,_n) vec_sld(_a,_a,_n)
#	define vec_nonzero_x(_a) vec_any_ne(vec_splat(_a, 0), hkQuadReal(0))
#	define vec_mul(_a,_b) vec_madd(_a,_b, hkQuadReal(0) )
#else
#	include <spu_internals.h>
	typedef vector float hkQuadReal;
	typedef vector signed int hkQuadInt;
#	define vec_andc	spu_andc
#	define vec_madd		spu_madd
#	define vec_re		spu_re
#	define vec_sub		spu_sub
#	define vec_xor		spu_xor
#	define vec_add		spu_add
#	define vec_and		spu_and
#	define vec_nmsub	spu_nmsub
#	define vec_mul		spu_mul
#	define vec_rsqrte	spu_rsqrte
#	define vec_perm		spu_shuffle
#	define vec_sel 		spu_sel
#	define vec_rotl		spu_rlqwbyte
#	define vec_nonzero_x(_a)	si_to_int((qword)_a)
#	define vec_cmplt(_a, _b) 	spu_cmpgt(_b, _a)
#	define vec_splat(_a, _b)	spu_shuffle(_a,_a, HK_VECTOR4_SHUFFLE(_b,_b,_b,_b))
#	define vec_max(_a, _b) 		spu_sel(_b, _a, spu_cmpgt(_a, _b))
#	define vec_min(_a, _b) 		spu_sel(_a, _b, spu_cmpgt(_a, _b))
#	define vec_stl(_a, _o, _p)	((*(hkQuadReal*)((char*)_p + _o)) = _a)
#	define vec_mergeh(_a, _b)	spu_shuffle(_a,_b,HK_VECTOR4_SHUFFLE(X,A,Y,B))
#	define vec_mergel(_a, _b)	spu_shuffle(_a,_b,HK_VECTOR4_SHUFFLE(Z,C,W,D))
#endif

union hkQuadRealUnion
{
	hkReal r[4];
	hkQuadReal q;
};

#define HK_QUADREAL_CONSTANT(a,b,c,d) {a,b,c,d}
#define HK_SIMD_REAL(a) hkSimdReal(a)

class hkSimdReal
{
	public:

		hkSimdReal()
		{
		}

		hkSimdReal(const hkQuadReal& x)
			: m_real(x)
		{
		}

		hkSimdReal(hkReal x)
		{
			hkQuadRealUnion u;
			u.r[0] = x;
			m_real = u.q;
		}

		void set(hkReal x)
		{
			hkQuadRealUnion u;
			u.r[0] = x;
			m_real = u.q;
		}


		operator hkReal() const
		{
			hkQuadRealUnion u;
			u.q = m_real;
			return u.r[0];
		}

		inline const hkQuadReal& getQuad() const
		{
			return m_real;
		}

	private:

		hkQuadReal m_real;
};

typedef const hkSimdReal hkSimdRealParameter;

#define HK_SIMD_REAL(a) hkSimdReal(a)

#if defined(__PPU__)
#	include <ppu_intrinsics.h>
#	include <hkmath/impl/hkPowerPcMathFuncs.inl>
	namespace hkMath
	{
#		define HK_MATH_ceil
		hkReal ceil(hkReal);
#		define HK_MATH_sin
		hkReal sin(hkReal);
#		define HK_MATH_cos
		hkReal cos(hkReal);
#		define HK_MATH_floor
		hkReal floor(hkReal);
	}
#endif

extern const hkQuadReal hkQuadRealHalf;
extern const hkQuadReal hkQuadReal1111;

namespace hkMath
{
	inline hkQuadReal quadReciprocal( hkQuadReal v )
	{
		vector float estimate = vec_re( v );
		//One round of Newton-Raphson refinement
		return vec_madd( vec_nmsub( estimate, v, hkQuadReal1111 ), estimate, estimate );
	}

	inline hkQuadReal quadReciprocalSquareRoot( hkQuadReal r )
	{
		hkQuadReal est = vec_rsqrte( r );
		//One round of Newton-Raphson refinement
		hkQuadReal est2 = vec_mul( est, est );
		hkQuadReal halfEst = vec_mul( est, hkQuadRealHalf );
		hkQuadReal slope = vec_nmsub( r, est2, hkQuadReal1111 );
		return vec_madd( slope, halfEst, est);
	}

	inline int isNegative(hkSimdReal r0)
	{
#if defined(__PPU__)
		return vec_any_lt( vec_splat(r0.getQuad(),0), hkQuadReal(0) );
#else
		return si_to_int( (qword)spu_cmpgt(hkQuadReal(0), r0.getQuad()) );
#endif
	}
}

class hkVector4;

typedef const hkVector4& hkVector4Parameter;

inline const hkSimdReal HK_CALL operator* (hkSimdReal v0, hkSimdReal v1)
{
	return vec_madd(v0.getQuad(), v1.getQuad(), hkQuadReal(0));
}

inline const hkSimdReal HK_CALL operator- (hkSimdReal v0, hkSimdReal v1)
{
	return vec_sub( v0.getQuad(), v1.getQuad() );
}

inline const hkSimdReal HK_CALL operator+ (hkSimdReal v0, hkSimdReal v1)
{
	return vec_add( v0.getQuad(), v1.getQuad() );
}

inline const hkSimdReal HK_CALL operator/ (hkSimdReal v0, hkSimdReal v1)
{
	return vec_madd( v0.getQuad(), hkMath::quadReciprocal(v1.getQuad()), hkQuadReal(0) );
}

inline const hkSimdReal HK_CALL operator- (hkSimdReal v0)
{
	return vec_sub( hkQuadReal(0), v0.getQuad() );
}

enum hkVector4CompareMask
{
	HK_VECTOR3_COMPARE_MASK_NONE	= 0,
	HK_VECTOR3_COMPARE_MASK_W		= 1,
	HK_VECTOR3_COMPARE_MASK_Z		= 2,
	HK_VECTOR3_COMPARE_MASK_ZW		= 3,

	HK_VECTOR3_COMPARE_MASK_Y		= 4,
	HK_VECTOR3_COMPARE_MASK_YW		= 5,
	HK_VECTOR3_COMPARE_MASK_YZ		= 6,
	HK_VECTOR3_COMPARE_MASK_YZW		= 7,

	HK_VECTOR3_COMPARE_MASK_X		= 8,
	HK_VECTOR3_COMPARE_MASK_XW		= 9,
	HK_VECTOR3_COMPARE_MASK_XZ		= 10,
	HK_VECTOR3_COMPARE_MASK_XZW		= 11,

	HK_VECTOR3_COMPARE_MASK_XY		= 12,
	HK_VECTOR3_COMPARE_MASK_XYW		= 13,
	HK_VECTOR3_COMPARE_MASK_XYZ		= 14,
	HK_VECTOR3_COMPARE_MASK_XYZW	= 15
};
#define HK_SIMD_COMPARE_MASK_X 8

/*
* Havok SDK - PUBLIC RELEASE, BUILD(#20060902)
*
* Confidential Information of Havok.  (C) Copyright 1999-2006 
* Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
* Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
* rights, and intellectual property rights in the Havok software remain in
* Havok and/or its suppliers.
*
* Use of this software for evaluation purposes is subject to and indicates 
* acceptance of the End User licence Agreement for this product. A copy of 
* the license is included with this software and is also available from salesteam@havok.com.
*
*/
