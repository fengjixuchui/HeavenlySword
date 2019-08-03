/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#if defined(HK_COMPILER_HAS_INTRINSICS_PS2) 
#	include <mmintrin.h>
#	include <vu0intrin.h>
 	typedef __v4sf hkQuadReal;
#else
	struct hkQuadReal
	{
		hkUint128 m_quad;
	};
#endif

class hkVector4;
typedef const hkVector4& hkVector4Parameter;


#include <hkbase/fwd/hkcmath.h>
#include <hkbase/fwd/hkcfloat.h>

union hkQuadRealUnion
{
	hkReal r[4];
	hkQuadReal q;
};
	
#define HK_QUADREAL_CONSTANT(a,b,c,d) ((hkQuadRealUnion){{a,b,c,d}}).q
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
#	if defined(HK_COMPILER_HAS_INTRINSICS_PS2)
			_vaddf(m_real, vu0_field_X, vu0_vf0, x);
#	else
			asm( "	qmtc2 %1, %0 " : "=j"(this->m_real):  "r"(x)  );
			//hkQuadRealUnion u;	u.r[0] = x;		m_real = u.q;
#	endif
		}

		operator hkReal() const
		{
			//hkQuadRealUnion u;	u.q = m_real;	return u.r[0];
			float ret;
			asm( "	qmfc2 %0, %1 " : "=r"(ret):  "j"(this->m_real)  );
			return ret;
		}

		inline const hkQuadReal& getQuad() const
		{
			return m_real;
		}

	private:

		hkQuadReal m_real;
};

typedef const hkSimdReal& hkSimdRealParameter;

inline const hkSimdReal HK_CALL operator* (hkSimdReal v0, hkSimdReal v1)
{
	hkQuadReal ret;
#	if defined(HK_COMPILER_HAS_INTRINSICS_PS2)
	_vmul(ret, vu0_field_X, v0.getQuad(), v1.getQuad());
#	else
	asm("vmul.x %0, %1, %2" : "=j"(ret) : "j"(v0.getQuad()), "j"(v1.getQuad())  );
#	endif
	return ret;
}

inline const hkSimdReal HK_CALL operator- (hkSimdReal v0, hkSimdReal v1)
{
	hkQuadReal ret;
#	if defined(HK_COMPILER_HAS_INTRINSICS_PS2)
	_vsub(ret, vu0_field_X, v0.getQuad(), v1.getQuad());
#	else
	asm("vsub.x %0, %1, %2" : "=j"(ret) : "j"(v0.getQuad()), "j"(v1.getQuad())  );
#	endif
	return ret;
}

inline const hkSimdReal HK_CALL operator+ (hkSimdReal v0, hkSimdReal v1)
{
	hkQuadReal ret;
#	if defined(HK_COMPILER_HAS_INTRINSICS_PS2)
	_vadd(ret, vu0_field_X, v0.getQuad(), v1.getQuad());
#	else
	asm("vadd.x %0, %1, %2" : "=j"(ret) : "j"(v0.getQuad()), "j"(v1.getQuad())  );
#	endif
	return ret;
}

inline const hkSimdReal HK_CALL operator-(hkSimdReal v)
{
	hkQuadReal ret;
#	if defined(HK_COMPILER_HAS_INTRINSICS_PS2)
	_vsub(ret, vu0_field_X, vu0_vf0, v.getQuad());
#	else
	asm("vsub.x %0, vf0, %1" : "=j"(ret) : "j"(v.getQuad()) );
#	endif
	return ret;
}

inline const hkSimdReal HK_CALL operator/ (hkSimdReal v0, hkSimdReal v1)
{
	hkQuadReal ret;
	asm(
		"vdiv Q, %1x, %2x \n"
		"vwaitq \n"
		"vaddq.x %0, vf0, Q"
		: "=j"(ret) :
			"j"(v0.getQuad()), "j"(v1.getQuad())  );
	return ret;
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
