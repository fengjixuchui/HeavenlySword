/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


inline void HK_CALL hkTriangleUtil::calcNormal( hkVector4& normal, const hkVector4& a, const hkVector4& b, const hkVector4& c ) 
{
	hkVector4 cb;
	hkVector4 ab;
	cb.setSub4(c,b);
	ab.setSub4(a,b);

	normal.setCross( cb , ab);
}

inline void HK_CALL hkTriangleUtil::calcCentroid( hkVector4& centroid, const hkVector4& a, const hkVector4& b, const hkVector4& c ) 
{
	centroid.setAdd4(a,b);
	centroid.add4(c);
	centroid.mul4(1.0f/3.0f);
}

inline hkBool HK_CALL hkTriangleUtil::inFront( const hkVector4& point, const hkVector4& a, const hkVector4& b, const hkVector4& c ) 
{
	hkVector4 triangleNormal;
	calcNormal ( triangleNormal, a, b, c );

	hkVector4 d; d.setSub4(point, a);
	return 	hkReal(d.dot3(triangleNormal)) > 0;
}

 
inline hkBool HK_CALL hkTriangleUtil::containsPoint( const hkVector4 &pt, const hkVector4& a, const hkVector4& b, const hkVector4& c ) 
{
	hkVector4 triangleNormal;
	calcNormal ( triangleNormal, a, b, c );

	hkVector4 pta;
	pta.setSub4(pt,a);
	hkVector4 ba;
	ba.setSub4(b,a);
	pta.setCross(pta,ba);

	hkVector4 ptb;
	pta.setSub4(pt,b);
	hkVector4 cb;
	ba.setSub4(c,b);
	ptb.setCross(pta,cb);

	hkVector4 ptc;
	pta.setSub4(pt,c);
	hkVector4 ac;
	ba.setSub4(a,c);
	ptc.setCross(pta,ac);

	hkRotation simultaneousDots;

	//set column and then inverse mul
	//is actually faster than set row
	//and then mul
	simultaneousDots.setCols(pta,ptb,ptc);
	//pta now holds the dot products
	pta._setRotatedInverseDir(simultaneousDots,triangleNormal);
	
	// all dots less than zero means we are inside
	int mask = pta.compareLessThanZero4() & HK_VECTOR3_COMPARE_MASK_XYZ;
	return mask != HK_VECTOR3_COMPARE_MASK_XYZ;
}

// The Triangle lies in the plane defined by x.N+d, where x is any point in the 
// plane,  N is the normal to the plane and d is the distance from the plane to the 
// origin (the planar distance)
//inline hkReal HK_CALL hkTriangleUtil::calcPlaneConstant( const hkVector4& a, const hkVector4& b, const hkVector4& c ) 
//{
//	hkVector4 n;
//	calcNormal( n, a, b, c );
//	n.normalize3();
//
//	return n.dot3(a);
//}


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
