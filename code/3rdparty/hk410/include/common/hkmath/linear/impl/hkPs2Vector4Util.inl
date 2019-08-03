/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#define HK_VECTOR4UTIL_mulSigns4
inline void HK_CALL hkVector4Util::mulSigns4( hkVector4& inout, const hkVector4& signs)
{
	extern const hkUint128 hkQuadSignMask;
	hkQuadReal itmp = inout.getQuad();
	register hkQuadReal stmp = signs.getQuad();
	asm("pand %1, %1, %2\n"
		"pxor %0, %0, %1\n"
			:	"+r" (itmp), //0
				"+r" (stmp)  //1
			:	"r" (hkQuadSignMask)); //2
	inout.getQuad() = itmp;
}

#define HK_VECTOR4UTIL_convertToUint16
inline void HK_CALL hkVector4Util::convertToUint16( const hkVector4& in, const hkVector4& offset, const hkVector4& scale, hkIntUnion64& out)
{
	hkQuadReal tmp_f;
	hkQuadReal tmp_r;

	asm("vadd.xyzw %2, %3, %4\n"
		"vmul.xyzw %2, %2, %5\n"
		"vftoi0.xyzw %2, %2\n"
		"qmfc2 %1, %2\n"
		"nop\n"
		"ppach %1, %1, %1\n"
		"sd %1, %0\n"
			:	"=m" (out), //0
				"=&r" (tmp_r), //1
				"=&j" (tmp_f) //2
			:	"j" (in.getQuad()), //3
				"j" (offset.getQuad()), //4
				"j" (scale.getQuad())); //5
}

/*
inline void HK_CALL hkVector4Util::convertToUint16WithClip( const hkVector4& in, const hkVector4& offset, const hkVector4& scale, const hkVector4& min, const hkVector4& max, hkIntUnion64& out)
{
	hkQuadReal tmp_f;
    __m128 tmp_r;

    asm("vadd.xyzw %2, %3, %4\n"
        "vmul.xyzw %2, %2, %5\n"
        "vmini.xyzw %2, %2, %7\n"
        "vmax.xyzw %2, %2, %6\n"
        "vftoi0.xyzw %2, %2\n"
        "qmfc2 %1, %2\n"
        "nop\n"
        "ppach %1, %1, %1\n"
        "sd %1, %0\n"
            :   "=m" (out), //0
                "=&r" (tmp_r), //1
                "=&j" (tmp_f) //2
            :   "j" (in.getQuad()), //3
                "j" (offset.getQuad()), //4
                "j" (scale.getQuad()), //5
                "j" (min.getQuad()), //6
                "j" (max.getQuad()) ); //7
				
}
*/

#ifndef HK_COMPILER_HAS_INTRINSICS_PS2
	// the compiler does a good job of these
	// with the intrinsics.

#	define HK_VECTOR4UTIL_transformPoints
inline void	HK_CALL hkVector4Util::transformPoints( const hkTransform& t, const hkVector4* vectorsIn, int numVectors, hkVector4* vectorsOut )
{
	checkRegisters( 0x0000f000 );
	__asm__ __volatile__ (
	"		.set noreorder\n"	\
	"		lqc2	vf12, 0x00(%0)	\n"	\
	"		lqc2	vf13, 0x10(%0)	\n"	\
	"		lqc2	vf14, 0x20(%0)	\n"	\
	"		lqc2	vf15, 0x30(%0)	\n" \
	"	.set reorder\n"
	: : "r"(&t.getRotation().getColumn(0) ): "vf12", "vf13", "vf14", "vf15", "memory" );

	do 
	{
		__asm__ __volatile__ (
		"		.set noreorder\n"	\
		"		lqc2	vf8, 0x00(%0)	\n"	\
		"		vmulax.xyzw		ACC, vf12, vf8 \n"	\
		"		vmadday.xyzw	ACC, vf13, vf8	\n"	\
		"		vmaddaz.xyzw	ACC, vf14, vf8	\n"	\
		"		vmaddw.xyzw		vf8, vf15, vf0	\n"	\
		"	.set reorder\n"
		:  : "r"(vectorsIn ): "vf8", "vf12", "vf13", "vf14", "vf15", "memory" );
		vectorsIn++;
		__asm__ __volatile__ (
		"		.set noreorder\n"	\
		"		sqc2	vf8, 0x0(%0)	\n"	\
		"	.set reorder\n"
		:  : "r"(vectorsOut ): "vf8", "memory" );
		vectorsOut++;
	}
	while ( --numVectors > 0 );
}

#	define HK_VECTOR4UTIL_mul4xyz1Points
inline void	HK_CALL hkVector4Util::mul4xyz1Points( const hkTransform& t, const hkVector4* vectorsIn, int numVectors, hkVector4* vectorsOut )
{
	transformPoints( t, vectorsIn, numVectors, vectorsOut );
}

#	define HK_VECTOR4UTIL_transformSpheres
inline void	HK_CALL hkVector4Util::transformSpheres( const hkTransform& t, const hkVector4* vectorsIn, int numVectors, hkVector4* vectorsOut )
{
	checkRegisters( 0x0000f000 );
	__asm__ __volatile__ (
	"		.set noreorder\n"	\
	"		lqc2	vf12, 0x00(%0)	\n"	\
	"		lqc2	vf13, 0x10(%0)	\n"	\
	"		lqc2	vf14, 0x20(%0)	\n"	\
	"		lqc2	vf15, 0x30(%0)	\n" \
	"	.set reorder\n"
	: : "r"(&t.getRotation().getColumn(0) ): "vf12", "vf13", "vf14", "vf15", "memory" );

	do 
	{
		__asm__ __volatile__ (
		"		.set noreorder\n"	\
		"		lqc2	vf8, 0x00(%0)	\n"	\
		"		vmulax.xyz		ACC, vf12, vf8 \n"	\
		"		vmadday.xyz		ACC, vf13, vf8	\n"	\
		"		vmaddaz.xyz		ACC, vf14, vf8	\n"	\
		"		vmaddw.xyz		vf8, vf15, vf0	\n"	\
		"	.set reorder\n"
		:  : "r"(vectorsIn ): "vf8", "vf12", "vf13", "vf14", "vf15", "memory" );
		vectorsIn++;
		__asm__ __volatile__ (
		"		.set noreorder\n"	\
		"		sqc2	vf8, 0x0(%0)	\n"	\
		"	.set reorder\n"
		:  : "r"(vectorsOut ): "vf8", "memory" );
		vectorsOut++;
	}
	while ( --numVectors > 0 );
}

#define HK_VECTOR4UTIL_rotatePoints
inline void	HK_CALL hkVector4Util::rotatePoints( const hkMatrix3& t, const hkVector4* vectorsIn, int numVectors, hkVector4* vectorsOut )
{
	checkRegisters( 0x0000f000 );		
	__asm__ __volatile__ (
	"		.set noreorder\n"	\
	"		lqc2	vf12, 0x00(%0)	\n"	\
	"		lqc2	vf13, 0x10(%0)	\n"	\
	"		lqc2	vf14, 0x20(%0)	\n"	\
	"	.set reorder\n"
	: : "r"(&t.getColumn(0) ): "vf12", "vf13", "vf14", "memory" );

	do 
	{
		__asm__ __volatile__ (
		"		.set noreorder\n"	\
		"		lqc2	vf15, 0x00(%0)	\n"	\
		"		vmulax.xyzw		ACC, vf12, vf15 \n"	\
		"		vmadday.xyzw	ACC, vf13, vf15	\n"	\
		"		vmaddz.xyzw	   vf15, vf14, vf15	\n"	\
		"	.set reorder\n"
		: : "r"(vectorsIn ): "vf12", "vf13", "vf14", "vf15", "memory" );
		vectorsIn++;
		__asm__ __volatile__ (
		"		.set noreorder\n"	\
		"		sqc2	vf15, 0x0(%0)	\n"	\
		"	.set reorder\n"
		:  : "r"(vectorsOut ): "vf15", "memory" );
		vectorsOut++;
	}
	while ( --numVectors > 0 );
}

#define HK_VECTOR4UTIL_rotateInversePoints
inline void	HK_CALL hkVector4Util::rotateInversePoints( const hkRotation& t, const hkVector4* vectorsIn, int numVectors, hkVector4* vectorsOut )
{
	checkRegisters( 0x0000f000 );
	__asm__ __volatile__ (
	"		.set noreorder\n"	\
	"		lqc2	vf12, 0x00(%0)	\n"	\
	"		lqc2	vf13, 0x10(%0)	\n"	\
	"		lqc2	vf14, 0x20(%0)	\n"	\
	"	.set reorder\n"
	:  : "r"(&t.getColumn(0) ): "vf12", "vf13", "vf14", "memory" );

	do 
	{
		__asm__ __volatile__ (
		"		.set noreorder\n"	\
		"	lqc2	vf15, 0x00(%0)	\n"	\
		"	vmul.xyz vf8,vf13,vf15		\n"	\
		"	vmul.xyz vf9,vf14,vf15		\n"	\
		"	vmul.xyz vf15,vf12,vf15		\n"	\
		"	vaddx.y vf8,vf8,vf8		\n"	\
		"	vaddy.z vf9,vf9,vf9		\n"	\
		"	vaddy.x vf15,vf15,vf15		\n"	\
		"	vaddz.x vf15,vf15,vf15		\n"	\
		"	vaddz.y vf15,vf8,vf8		\n"	\
		"	vaddx.z vf15,vf9,vf9		\n"	\
		"	.set reorder\n"
		:  : "r"(vectorsIn ): "vf8", "vf9", "vf15", "vf12", "vf13", "vf14", "vf15", "memory" );
		vectorsIn++;
		__asm__ __volatile__ (
		"		.set noreorder\n"	\
		"		sqc2	vf15, 0x0(%0)	\n"	\
		"	.set reorder \n"
		:  : "r"(vectorsOut ): "memory" );
		vectorsOut++;
	}
	while ( --numVectors > 0 );
}

#endif // HK_COMPILER_HAS_INTRINSICS_PS2

#define HK_VECTOR4UTIL_dot3_3vs3
inline void HK_CALL hkVector4Util::dot3_3vs3(const hkVector4& a0, const hkVector4& b0, const hkVector4& a1, const hkVector4& b1, const hkVector4& a2, const hkVector4& b2, hkVector4& dotsOut)
{
	register hkQuadReal A0 = a0.getQuad();
	register hkQuadReal A1 = a1.getQuad();
	register hkQuadReal A2 = a2.getQuad();

	register hkQuadReal B0 = b0.getQuad();
	register hkQuadReal B1 = b1.getQuad();
	register hkQuadReal B2 = b2.getQuad();

	register hkQuadReal h0;
	register hkQuadReal h1;
	asm (
	"	vmul.xyz %0, %3, %6		\n"
	"	vmul.xyz %1, %4, %7		\n"		
	"	vmul.xyz %2, %5, %8		\n"
	"	vaddy.x  %0, %0, %0		\n"
	"	vaddx.y  %1, %1, %1		\n"
	"	vaddy.z  %2, %2, %2		\n"
	"	vaddz.x  %0, %0, %0		\n"
	"	vaddz.y  %0, %1, %1		\n"
	"	vaddx.z  %0, %2, %2		"
	: "=&j"(dotsOut.getQuad()), "=&j"(h0), "=&j"(h1)	// 012
	: "j"(A0), "j"(A1), "j"(A2), "j"(B0), "j"(B1), "j"(B2) );	// 345 678
}

#define HK_VECTOR4UTIL_dot3_1vs4
inline void HK_CALL hkVector4Util::dot3_1vs4( const hkVector4& vector, const hkVector4& a0, const hkVector4& a1, const hkVector4& a2, const hkVector4& a3, hkVector4& dotsOut)
{
	register hkQuadReal A0 = a0.getQuad();
	register hkQuadReal A1 = a1.getQuad();
	register hkQuadReal A2 = a2.getQuad();
	register hkQuadReal A3 = a3.getQuad();

	register hkQuadReal B0 = vector.getQuad();

	register hkQuadReal h0;
	register hkQuadReal h1;
	register hkQuadReal h2;
	asm (
	"	vmul.xyz %3, %8, %4		\n"
	"	vmul.xyz %0, %5, %4		\n"
	"	vmul.xyz %1, %6, %4		\n"		
	"	vmul.xyz %2, %7, %4		\n"
	"	vmulz.w  %3,vf0, %3		# move z component into .w component\n"
	"	vaddy.x  %0, %0, %0		\n"
	"	vaddx.y  %1, %1, %1		\n"
	"	vaddy.z  %2, %2, %2		\n"
	"	vaddy.w  %3, %3, %3		\n"
	"	vaddz.x  %0, %0, %0		\n"
	"	vaddz.y  %0, %1, %1		\n"
	"	vaddx.z  %0, %2, %2		\n"
	"	vaddx.w  %0, %3, %3		"
	: "=&j"(dotsOut.getQuad()), "=&j"(h0), "=&j"(h1), "=&j"(h2)	// 0123
	: "j"(B0), "j"(A0), "j"(A1), "j"(A2), "j"(A3) );	// 4 5678
}

#define HK_VECTOR4UTIL_dot3_2vs2
inline void HK_CALL hkVector4Util::dot3_2vs2( const hkVector4& a0, const hkVector4& a2, const hkVector4& b0, const hkVector4& b1, hkVector4& dotsOut)
{
	register hkQuadReal A0 = a0.getQuad();
	register hkQuadReal A2 = a2.getQuad();

	register hkQuadReal B0 = b0.getQuad();
	register hkQuadReal B1 = b1.getQuad();

	register hkQuadReal h0;
	register hkQuadReal h1;
	register hkQuadReal h2;
	asm (
	"	vmul.xyz %3, %5, %7		\n"
	"	vmul.xyz %0, %4, %6		\n"
	"	vmul.xyz %1, %4, %7		\n"		
	"	vmul.xyz %2, %5, %6		\n"
	"	vmulz.w  %3,vf0, %3		# move z component into .w component\n"
	"	vaddy.x  %0, %0, %0		\n"
	"	vaddx.y  %1, %1, %1		\n"
	"	vaddy.z  %2, %2, %2		\n"
	"	vaddy.w  %3, %3, %3		\n"
	"	vaddz.x  %0, %0, %0		\n"
	"	vaddz.y  %0, %1, %1		\n"
	"	vaddx.z  %0, %2, %2		\n"
	"	vaddx.w  %0, %3, %3		"
	: "=&j"(dotsOut.getQuad()), "=&j"(h0), "=&j"(h1), "=&j"(h2)	// 0123
	: "j"(A0), "j"(A2), "j"(B0), "j"(B1) );	// 45 67
}

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
