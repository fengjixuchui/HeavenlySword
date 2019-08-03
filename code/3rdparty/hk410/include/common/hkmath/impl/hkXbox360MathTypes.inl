/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkbase/fwd/hkcmath.h>
#include <hkbase/fwd/hkcfloat.h>

#include <ppcintrinsics.h>
#include <VectorIntrinsics.h>
typedef __vector4 hkQuadReal;

class hkVector4;
typedef const hkVector4& hkVector4Parameter;

union hkQuadRealUnion
{
	hkReal r[4];
	hkQuadReal q;
};

extern const hkQuadReal hkQuadReal1111;
extern const hkQuadReal hkQuadReal0000;
extern const hkQuadReal hkQuadRealHalf;

#define HK_QUADREAL_CONSTANT(a,b,c,d) {a,b,c,d}

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


		hkSimdReal(const hkReal& x)
		{
			m_real = __lvlx(&x, 0);
			m_real = __vspltw( m_real, 0 );
		}

		void set( const hkQuadReal& x ){ m_real = x; }
		void set( const hkReal& x )
		{
			m_real = __lvlx(&x, 0);
			m_real = __vspltw( m_real, 0 );
		}

		operator hkReal() const
		{
			float x;
			__stvewx( m_real, &x, 0);
			return x;
		}

		inline hkQuadReal& getQuad()
		{
			return m_real;
		}

		inline const hkQuadReal& getQuad() const
		{
			return m_real;
		}

		inline static hkSimdReal HK_CALL create( hkReal a)
		{
			hkSimdReal sr; sr.set(a);
			return sr;
		}
	private:

		hkQuadReal m_real;
};

#define HK_SIMD_REAL(a) hkSimdReal::create(a)

namespace hkMath
{
	inline hkQuadReal quadReciprocal( hkQuadReal q )
	{
		__vector4 e = __vrefp( q );
		//One round of Newton-Raphson refinement
		return __vmaddfp( __vnmsubfp( e, q, hkQuadReal1111 ), e, e);
	}

	inline hkQuadReal quadReciprocalSquareRoot( hkQuadReal q )
	{
		__vector4 estimate = __vrsqrtefp( q );
		//One round of Newton-Raphson refinement
		// Refinement (Newton-Raphson) for 1.0 / sqrt(x)
		//     y0 = reciprocal_sqrt_estimate(x)
		//     y1 = y0 + 0.5 * y0 * (1.0 - x * y0 * y0) 
		//        = y0 + y0 * (0.5 - 0.5 * x * y0 * y0)
		// (from xmvector.inl)
		hkQuadReal OneHalfV = __vmulfp(q, hkQuadRealHalf);
		hkQuadReal Reciprocal = __vmulfp(estimate, estimate);
		hkQuadReal Scale = __vnmsubfp(OneHalfV, Reciprocal, hkQuadRealHalf);
		return __vmaddfp(estimate, Scale, estimate);
	}
}

typedef const hkSimdReal& hkSimdRealParameter;

inline const hkSimdReal HK_CALL operator* (hkSimdReal v0, hkSimdReal v1)
{
	hkSimdReal res; res.getQuad() = __vmulfp( v0.getQuad(), v1.getQuad() );
	return res;
}

inline const hkSimdReal HK_CALL operator- (hkSimdReal v0, hkSimdReal v1)
{
	hkSimdReal res; res.getQuad() = __vsubfp( v0.getQuad(), v1.getQuad() );
	return res;
}

inline const hkSimdReal HK_CALL operator+ (hkSimdReal v0, hkSimdReal v1)
{
	hkSimdReal res; res.getQuad() = __vaddfp( v0.getQuad(), v1.getQuad() );
	return res;
}

inline const hkSimdReal HK_CALL operator/ (hkSimdReal v0, hkSimdReal v1)
{
	__vector4 v1Recip = hkMath::quadReciprocal(v1.getQuad());
	hkSimdReal res; res.getQuad() = __vmulfp( v0.getQuad(), v1Recip );
	return res;
}

inline const hkSimdReal HK_CALL operator- (hkSimdReal v0)
{
	hkSimdReal res; res.getQuad() = __vsubfp( hkQuadReal0000, v0.getQuad() );
	return res;
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

#include <hkmath/impl/hkPowerPcMathFuncs.inl>

// An indication of which registers are used on Xbox 360
// We use registers in the range HK_USED_VMX_TOP to HK_USED_VMX_BOTTOM inclusive
#define HK_NUM_USED_VMX_REGS	17
#define HK_USED_VMX_TOP			95
#define HK_USED_VMX_BOTTOM		(HK_USED_VMX_TOP - HK_NUM_USED_VMX_REGS)

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
