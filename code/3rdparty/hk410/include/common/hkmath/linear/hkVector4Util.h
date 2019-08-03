/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_MATH_VECTOR3_UTIL_H
#define HK_MATH_VECTOR3_UTIL_H

#include <hkmath/hkMath.h>

union hkIntUnion64
{
	hkQuadReal quad;
	hkInt64 i64;
	hkUint64 u64;
	hkInt32 i32[2];
	hkUint32 u32[2];
	hkInt16 i16[4];
	hkUint16 u16[4];
	hkInt8 i8[8];
	hkUint8 u8[8];
};

namespace hkVector4Util
{
		/// Sets the calling vector to be the normal to the 2 vectors (b-a) and (c-a).<br>
		/// NOTE: The calculated result is not explicitly normalized.
		/// This function is particularly fast on PS2.
		/// Result = (b-a) cross (c-a)
	HK_FORCE_INLINE void HK_CALL setNormalOfTriangle(hkVector4& result, const hkVector4 &a, const hkVector4& b,const hkVector4& c);

		/// Multiplies sign bits from signsSource onto inout leaving the mantissa and exponent untouched.
	HK_FORCE_INLINE void HK_CALL mulSigns4( hkVector4& inout, const hkVector4& signsSource);

		/// 
	HK_FORCE_INLINE void HK_CALL convertToUint16( const hkVector4& in, const hkVector4& offset, const hkVector4& scale, hkIntUnion64& out );

		/// 
	HK_FORCE_INLINE void HK_CALL convertToUint16WithClip( const hkVector4& in, const hkVector4& offset, const hkVector4& scale, const hkVector4& min, const hkVector4& max, hkIntUnion64& out);

		/// calculates a value x so that convertToUint16WithClip( out, in + x/scale, ... ) == out = int(floor( (in+offset)*scale
	hkReal HK_CALL getFloatToInt16FloorCorrection();

		/// 
	HK_FORCE_INLINE void HK_CALL convertToUint32( const hkVector4& in, const hkVector4& offset, const hkVector4& scale, hkUint32* out );

		///
	HK_FORCE_INLINE void HK_CALL convertToUint32WithClip( const hkVector4& in, const hkVector4& offset, const hkVector4& scale, const hkVector4& min, const hkVector4& max, hkUint32* out);

		/// calculates a value x so that convertToUint32WithClip( out, in + x/scale, ... ) == out = int(floor( (in+offset)*scale
	hkReal HK_CALL getFloatToInt32FloorCorrection();

		/// Finds a vector that is perpendicular to a line segment.
		///
		/// Achieved by constructing a vector from the segment vector with the following properties:
		/// (Segment vector is any vector parallel to the line segment.)
		/// 1) component with the smallest index is set to 0
		/// 2) the remaining two component are copied into the new vector but are swapped in position
		/// 3) one of the copied components is multiplied by -1.0 (has it's sign flipped!)
		///
		/// leaving: (for example)
		/// segmentVector = (x, y, z), with let's say y as the smallest component
		/// segmentNormal = (-z, 0 , x) or (z, 0, -x), either will do.
		///
		/// This will in fact be orthogonal as (in the example) the dot product will be zero.
		/// i.e. x*-z + y*0 + z*x = 0 
		///
	HK_FORCE_INLINE void HK_CALL calculatePerpendicularVector(const hkVector4& vectorIn, hkVector4& biVectorOut);

		/// Transforms an array of points.
	HK_FORCE_INLINE void HK_CALL transformPoints( const hkTransform& t, const hkVector4* vectorsIn, int numVectors, hkVector4* vectorsOut );

		/// Transforms an array of points including the .w component
	HK_FORCE_INLINE void HK_CALL mul4xyz1Points( const hkTransform& t, const hkVector4* vectorsIn, int numVectors, hkVector4* vectorsOut );

		/// Transforms an array of spheres (keeps the .w component as the radius)
	HK_FORCE_INLINE void HK_CALL transformSpheres( const hkTransform& t, const hkVector4* vectorsIn, int numVectors, hkVector4* vectorsOut );

		/// Rotate an array of points.
	HK_FORCE_INLINE void HK_CALL rotatePoints( const hkMatrix3& t, const hkVector4* vectorsIn, int numVectors, hkVector4* vectorsOut );

		/// Invert rotate an array of points.
	HK_FORCE_INLINE void HK_CALL rotateInversePoints( const hkRotation& t, const hkVector4* vectorsIn, int numVectors, hkVector4* vectorsOut );

		/// set aTcOut = aTb * bTc
	HK_FORCE_INLINE void HK_CALL setMul( const hkMatrix3& aTb, const hkMatrix3& bTc, hkMatrix3& aTcOut );

		/// set aTcOut = bTa^1 * bTc
	HK_FORCE_INLINE void HK_CALL setInverseMul( const hkRotation& bTa, const hkMatrix3& bTc, hkMatrix3& aTcOut );

		/// set aTcOut = aTb * bTc
	HK_FORCE_INLINE void HK_CALL transpose( hkMatrix3& m );

		/// m += add
	HK_FORCE_INLINE void HK_CALL add( hkMatrix3& m, const hkMatrix3& add );


		/// Sets this vector components: this(0) = a0.dot3(b0), this(1) = a1.dot3(b1) ...
	HK_FORCE_INLINE void HK_CALL dot3_3vs3( const hkVector4& a0, const hkVector4& b0, const hkVector4& a1, const hkVector4& b1, const hkVector4& a2, const hkVector4& b2, hkVector4& dotsOut);

		/// Sets this vector components: this(0) = a0.dot3(b0) ... this(3) = a3.dot3(b3) 
	HK_FORCE_INLINE void HK_CALL dot3_4vs4( const hkVector4& a0, const hkVector4& b0, const hkVector4& a1, const hkVector4& b1, const hkVector4& a2, const hkVector4& b2, const hkVector4& a3, const hkVector4& b3, hkVector4& dotsOut);

		/// Sets this vector components: this(0) = a0.dot4(b0) ... this(3) = a3.dot4(b3) 
	HK_FORCE_INLINE void HK_CALL dot4_4vs4( const hkVector4& a0, const hkVector4& b0, const hkVector4& a1, const hkVector4& b1, const hkVector4& a2, const hkVector4& b2, const hkVector4& a3, const hkVector4& b3, hkVector4& dotsOut);

		/// Sets this vector components: this(i) = vector.dot3(ai) for i=0..3
	HK_FORCE_INLINE void HK_CALL dot3_1vs4( const hkVector4& vectorIn, const hkVector4& a0, const hkVector4& a1, const hkVector4& a2, const hkVector4& a3, hkVector4& dotsOut);

		/// Sets this vector components: this(i+j) = ai.dot3(bj)
	HK_FORCE_INLINE void HK_CALL dot3_2vs2( const hkVector4& a0, const hkVector4& a2, const hkVector4& b0, const hkVector4& b1, hkVector4& dotsOut);


		/// build an orthonormal matrix, where the first column matches the parameter dir.
		/// Note: Dir must be normalized
	HK_FORCE_INLINE void HK_CALL buildOrthonormal( const hkVector4& dir, hkMatrix3& out );

		/// build an orthonormal matrix, where the first column matches the parameter dir and the second
		/// column matches dir2 as close as possible
		/// Note: Dir must be normalized
	HK_FORCE_INLINE void HK_CALL buildOrthonormal( const hkVector4& dir, const hkVector4& dir2, hkMatrix3& out );

		/// out = in^-1, in must be symmetric and invertible
	HK_FORCE_INLINE void HK_CALL invertSymmetric( const hkMatrix3& in, hkReal eps, hkMatrix3& out );
		/// Resets the fpu after using MMX instructions on x86. No-op for other architectures.
	HK_FORCE_INLINE void HK_CALL exitMmx();

		/// Returns the squared distance from p to the line segment ab 
	HK_FORCE_INLINE hkSimdReal HK_CALL distToLineSquared( const hkVector4& a, const hkVector4& b, const hkVector4& p );

		/// check for free registers for computers with lot's of registers (e.g. PS2)
	HK_FORCE_INLINE void HK_CALL checkRegisters( hkUint32 registerMask );

		/// reserve registers for computers with lots of registers (e.g. PS2)
	HK_FORCE_INLINE void HK_CALL reserveRegisters( hkUint32 registerMask );

		/// release these registers
	HK_FORCE_INLINE void HK_CALL releaseRegisters( hkUint32 registerMask );

		//
		//	compression
		//

		// packs a normalized quaternion into a single 4*8 bit integer. The error is roughly 0.01f per component
	hkUint32 HK_CALL packQuaternionIntoInt32( const hkQuaternion& qin);

		// unpack an 32 bit integer into quaternion. Note: The resulting quaternion is not normalized ( |q.length4()-1.0f| < 0.04f )
	HK_FORCE_INLINE void HK_CALL unPackInt32IntoQuaternion( hkUint32 ivalue, hkQuaternion& qout );

	extern hkUint32 m_reservedRegisters;
}

// For internal use. Define this to check that register allocations
// between functions do not clash. Disabled by default
// because it does not work in a multithreaded environment.
//#	define HK_CHECK_REG_FOR_PS2
HK_FORCE_INLINE void HK_CALL hkVector4Util::reserveRegisters( hkUint32 registerMask )
{
#ifdef HK_CHECK_REG_FOR_PS2
	HK_ASSERT2(0x23c4e825,  (registerMask & m_reservedRegisters) == 0, "Register clash");
	m_reservedRegisters |= registerMask;
#endif
}

HK_FORCE_INLINE void HK_CALL hkVector4Util::checkRegisters( hkUint32 registerMask )
{
#ifdef HK_CHECK_REG_FOR_PS2
	HK_ASSERT2(0x2459dd7d,  (registerMask & m_reservedRegisters) == 0, "Register clash");
#endif
}

HK_FORCE_INLINE void HK_CALL hkVector4Util::releaseRegisters( hkUint32 registerMask )
{
#ifdef HK_CHECK_REG_FOR_PS2
	m_reservedRegisters &= ~registerMask;
#endif
}

#if HK_CONFIG_SIMD == HK_CONFIG_SIMD_ENABLED
#	if defined(HK_COMPILER_HAS_INTRINSICS_IA32)
#		include <hkmath/linear/impl/hkSseVector4Util.inl>
#	elif defined(HK_ARCH_PS2)
#		include <hkmath/linear/impl/hkPs2Vector4Util.inl>
#	elif defined(HK_PLATFORM_PS3SPU)
#		include <hkmath/linear/impl/hkPs3Vector4Util.inl>
#	elif defined(HK_PLATFORM_XBOX360)
#		include <hkmath/linear/impl/hkXbox360Vector4Util.inl>
#	endif
#else
#	if defined(HK_PLATFORM_PSP) && defined(HK_COMPILER_GCC)
#		include <hkmath/linear/impl/pspgcc/hkPspGccVector4Util.inl>
#	endif
#endif // HK_CONFIG_SIMD

#include <hkmath/linear/hkVector4Util.inl>

#endif // HK_MATH_VECTOR3_UTIL_H

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
