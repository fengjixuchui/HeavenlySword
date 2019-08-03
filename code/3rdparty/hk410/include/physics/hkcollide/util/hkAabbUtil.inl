/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


#include <hkmath/basetypes/hkAabb.h>

void HK_CALL hkAabbUtil::calcAabb( const hkTransform& localToWorld, const hkVector4& halfExtents, float extraRadius, hkAabb& aabbOut )
{
#if defined (HK_PS2) && defined (HK_COMPILER_GCC) && (HK_CONFIG_SIMD == HK_CONFIG_SIMD_ENABLED)
	asm (
"		lqc2		vf8, 0x00(%3)	#	column(0)			\n"		\
"		qmtc2			%4, vf12	# extra radius \n"				\
"		lqc2		vf9, 0x10(%3)	#	column(1)			\n"		\
"		lqc2		vf10, 0x20(%3)	#	column(2)			\n"		\
"		lqc2		vf11, 0x30(%3)	#	column(3)			\n"		\
"		vaddx.xyzw	vf12, vf0, vf12 #   broadcast extra radius \n"	\
"		vmulx.xyz	vf8, vf8,  %2		# transformedX.setMul4( halfExtents(0), localToWorld.getRotation().getColumn(0))		\n"		\
"		vmuly.xyz	vf9, vf9,  %2		# transformedY.setMul4( halfExtents(1), localToWorld.getRotation().getColumn(0))		\n"		\
"		vmulz.xyz	vf10,vf10, %2		# transformedZ.setMul4( halfExtents(2), localToWorld.getRotation().getColumn(0))		\n"		\
"		vabs.xyz    vf8, vf8			# abs x \n"		\
"		vabs.xyz    vf9, vf9			# abs y \n"		\
"		vabs.xyz    vf10, vf10			# abs z \n"		\
"		vmulaw.xyzw	     ACC, vf8, vf0	# x+y+z\n"		\
"		vmaddaw.xyzw	 ACC, vf9, vf0	# \n"			\
"		vmaddaw.xyzw	 ACC, vf12,vf0	# extra radius \n"			\
"		vmaddw.xyzw		%1, vf10, vf0	# extends = all together\n"		\
"		vsub.xyz	%0, vf11, %1		# min = offset - extends  \n"	\
"		vadd.xyz	%1, vf11, %1		# max += offset	\n"
 : "=j" (aabbOut.m_min.getQuad() ), "=j" (aabbOut.m_max.getQuad() )
 :  "j"(halfExtents.getQuad()), "r"( &localToWorld.getRotation().getColumn(0) ), "r" ( extraRadius )
 : "vf8", "vf9", "vf10", "vf11", "vf12", "memory"	);
#else

	hkVector4 he0; he0.setBroadcast3clobberW( halfExtents, 0 );
	hkVector4 he1; he1.setBroadcast3clobberW( halfExtents, 1 );
	hkVector4 he2; he2.setBroadcast3clobberW( halfExtents, 2 );

	hkVector4 transformedX; transformedX.setMul4( he0, localToWorld.getRotation().getColumn(0));
	hkVector4 transformedY; transformedY.setMul4( he1, localToWorld.getRotation().getColumn(1));
	hkVector4 transformedZ; transformedZ.setMul4( he2, localToWorld.getRotation().getColumn(2));
	
	transformedX.setAbs4( transformedX );
	transformedY.setAbs4( transformedY );
	transformedZ.setAbs4( transformedZ );

	hkVector4 max = aabbOut.m_max; // copy so local on xbox etc
	hkVector4 min = aabbOut.m_min;

	hkVector4 extra; extra.setAll3( extraRadius );
	transformedZ.add4( extra );
	max.setAdd4( transformedX, transformedY );
	max.add4( transformedZ );

	min.setNeg4(max);
	max.add4( localToWorld.getTranslation() );
	min.add4( localToWorld.getTranslation()  );

	aabbOut.m_max = max;
	aabbOut.m_min = min;
#endif
}


void HK_CALL hkAabbUtil::calcAabb( const hkTransform& localToWorld, const hkVector4& halfExtents, const hkVector4& center, float extraRadius, hkAabb& aabbOut )
{
#if defined HK_PS2 && defined (HK_COMPILER_GCC) && (HK_CONFIG_SIMD == HK_CONFIG_SIMD_ENABLED)
	asm (
"		lqc2		vf8, 0x00(%3)	#	column(0)			\n"		\
"		qmtc2			%4, vf12	# extra radius \n"				\
"		lqc2		vf9, 0x10(%3)	#	column(1)			\n"		\
"		lqc2		vf10, 0x20(%3)	#	column(2)			\n"		\
"		lqc2		vf11, 0x30(%3)	#	column(3)			\n"		\
"		vaddx.xyzw	vf12, vf0, vf12 #   broadcast extra radius \n"	\
"		vmulax.xyz	ACC,  vf8, %5	#   vf13.setTransformedPos(center)	\n" \
"		vmadday.xyz	ACC,  vf9, %5	#   vf13.setTransformedPos(center)	\n" \
"		vmaddaz.xyz	ACC,  vf10, %5	#   vf13.setTransformedPos(center)	\n" \
"		vmaddw.xyz	vf11, vf11, vf0	#   vf13.setTransformedPos(center)	\n" \
"		vmulx.xyz	vf8, vf8,  %2		# transformedX.setMul4( halfExtents(0), localToWorld.getRotation().getColumn(0))		\n"		\
"		vmuly.xyz	vf9, vf9,  %2		# transformedY.setMul4( halfExtents(1), localToWorld.getRotation().getColumn(0))		\n"		\
"		vmulz.xyz	vf10,vf10, %2		# transformedZ.setMul4( halfExtents(2), localToWorld.getRotation().getColumn(0))		\n"		\
"		vabs.xyz    vf8, vf8			# abs x \n"		\
"		vabs.xyz    vf9, vf9			# abs y \n"		\
"		vabs.xyz    vf10, vf10			# abs z \n"		\
"		vmulaw.xyzw	     ACC, vf8, vf0	# x+y+z\n"		\
"		vmaddaw.xyzw	 ACC, vf9, vf0	# \n"			\
"		vmaddaw.xyzw	 ACC, vf10,vf0	# extra radius \n"			\
"		vmaddw.xyzw		%1, vf12, vf0	# extends = all together\n"		\
"		vsub.xyz	%0, vf11, %1		# min = offset - extends  \n"	\
"		vadd.xyz	%1, vf11, %1		# max += offset	\n"
 : "=j" (aabbOut.m_min.getQuad() ), "=j" (aabbOut.m_max.getQuad() )
 :  "j"(halfExtents.getQuad()), "r"( &localToWorld.getRotation().getColumn(0) ), "r" ( extraRadius ), "j" (center.getQuad())
 : "vf8", "vf9", "vf10", "vf11", "vf12", "memory"	);
#else
	hkVector4 he0; he0.setBroadcast3clobberW( halfExtents, 0 );
	hkVector4 he1; he1.setBroadcast3clobberW( halfExtents, 1 );
	hkVector4 he2; he2.setBroadcast3clobberW( halfExtents, 2 );

	hkVector4 transformedX; transformedX.setMul4( he0, localToWorld.getRotation().getColumn(0));
	hkVector4 transformedY; transformedY.setMul4( he1, localToWorld.getRotation().getColumn(1));
	hkVector4 transformedZ; transformedZ.setMul4( he2, localToWorld.getRotation().getColumn(2));

	transformedX.setAbs4( transformedX );
	transformedY.setAbs4( transformedY );
	transformedZ.setAbs4( transformedZ );

	hkVector4 max = aabbOut.m_max; // copy so local on xbox etc
	hkVector4 min = aabbOut.m_min;

	hkVector4 extra; extra.setAll3( extraRadius );
	transformedZ.add4( extra );

	max.setAdd4( transformedX, transformedY );
	max.add4( transformedZ );

	min.setNeg4(max);

	hkVector4 temp;
	temp._setTransformedPos(localToWorld, center);

	max.add4( temp );
	min.add4( temp );

	aabbOut.m_max = max;
	aabbOut.m_min = min;
#endif
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
