/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_MATH_MATH_H
#define HK_MATH_MATH_H

#include <hkbase/hkBase.h>
#include <hkbase/config/hkConfigSimd.h>

// Temporarily disable the alignment warning for AMD64
#if defined(HK_ARCH_X64)
#	pragma warning( disable: 4328 )
#endif

typedef float hkReal;

#if HK_CONFIG_SIMD == HK_CONFIG_SIMD_ENABLED
#	if defined(HK_COMPILER_HAS_INTRINSICS_IA32)
#		include <hkmath/impl/hkSseMathTypes.inl>
#	elif defined(HK_ARCH_PS2) // ps2 intrinsics or asm blocks
#		include <hkmath/impl/hkPs2MathTypes.inl>
#		include <hkmath/impl/hkPs2MathTypesCommon.inl>
#	elif defined(HK_PLATFORM_PSP )
#		error simd should be disabled
#	elif defined(HK_PLATFORM_XBOX360)
#		include <hkmath/impl/hkXbox360MathTypes.inl>
#	elif defined(HK_ARCH_PS3) || defined(HK_ARCH_PS3SPU)
#		include <hkmath/impl/hkPs3MathTypes.inl>
#	else // simd
#		error Unknown platform for SIMD
#	endif
#else // not simd
#	include <hkmath/impl/hkFpuMathTypes.inl>
#	if defined(HK_ARCH_PS2)
#		include <hkmath/impl/hkPs2MathTypesCommon.inl>
#	elif defined (HK_PLATFORM_GC)
#		include <hkmath/impl/hkGcMathFuncs.inl>	
#	elif defined(HK_PLATFORM_PSP) && defined( HK_COMPILER_GCC )
#		include <hkmath/impl/hkPspGccMathTypes.inl>
#	endif
#endif
#include <hkmath/impl/hkFpuMathFuncs.inl>

extern const hkQuadReal hkQuadReal0000;
extern const hkQuadReal hkQuadReal1111;
extern const hkQuadReal hkQuadReal1000;
extern const hkQuadReal hkQuadReal0100;
extern const hkQuadReal hkQuadReal0010;
extern const hkQuadReal hkQuadReal0001;
extern const hkQuadReal hkQuadReal3333;
extern const hkQuadReal hkQuadRealHalf;
extern const hkQuadReal hkQuadRealMinusHalf;

class hkVector4;
class hkQuaternion;
class hkMatrix3;
class hkRotation;
class hkTransform;
class hkQsTransform;

#include <hkmath/linear/hkVector4.h>
#include <hkmath/linear/hkQuaternion.h>
#include <hkmath/linear/hkMatrix3.h>
#include <hkmath/linear/hkRotation.h>
#include <hkmath/linear/hkTransform.h>
#include <hkmath/linear/hkSweptTransform.h>
#include <hkmath/linear/hkQsTransform.h>
#include <hkmath/linear/hkMatrix4.h>

// platform implementations

#if HK_CONFIG_SIMD == HK_CONFIG_SIMD_ENABLED
#	if defined(HK_COMPILER_HAS_INTRINSICS_IA32)
#		include <hkmath/linear/impl/hkSseVector4.inl>
#	elif defined(HK_COMPILER_HAS_INTRINSICS_PS2)
#		include <hkmath/linear/impl/hkPs2Vector4.inl>
#	elif defined(HK_PS2)
		// to work around inlining issues, Ps2AsmVector4 is split into a and b parts
#		include <hkmath/linear/impl/hkPs2AsmVector4a.inl>
#	elif defined( HK_PLATFORM_PSP )
#		error simd should be disabled
#	elif defined(HK_PLATFORM_XBOX360)
#		include <hkmath/linear/impl/hkXbox360Vector4.inl>
#	elif defined(HK_PLATFORM_PS3) || defined(HK_PLATFORM_PS3SPU)
#		include <hkmath/linear/impl/hkPs3Vector4.inl>
#	else
#		error Dont know how to do simd on this platform
#	endif
#else // no SIMD
#	if defined(HK_PLATFORM_PSP) && defined(HK_COMPILER_GCC)
#		include <hkmath/linear/impl/pspgcc/hkPspGccVector4.inl>
#		include <hkmath/linear/impl/pspgcc/hkPspGccMatrix3.inl>
#	endif // PSP && GCC
#	include <hkmath/linear/impl/hkFpuVector4.inl>
#endif // HK_CONFIG_SIMD


	// common implementations after inline definitions
#include <hkmath/linear/hkVector4.inl>

#include <hkmath/linear/hkMatrix3.inl>
#include <hkmath/linear/hkQuaternion.inl>
#include <hkmath/linear/hkTransform.inl>
#include <hkmath/linear/hkQsTransform.inl>
#include <hkmath/linear/hkMatrix4.inl>

	// to work around inlining issues, Ps2AsmVector4 is split
#if HK_CONFIG_SIMD == HK_CONFIG_SIMD_ENABLED && !defined(HK_COMPILER_HAS_INTRINSICS_PS2) && defined(HK_PS2)
#	include <hkmath/linear/impl/hkPs2AsmVector4b.inl>
#endif

#include <hkmath/linear/hkSweptTransform.inl>


#endif // HK_MATH_MATH_H


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
