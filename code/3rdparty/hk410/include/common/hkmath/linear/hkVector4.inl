/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

inline void hkVector4::operator=(const hkQuadReal& r)
{
	getQuad() = r;
}

#ifndef HK_VECTOR4_setRotatedDir
HK_FORCE_INLINE void hkVector4::setRotatedDir(const hkQuaternion& quat, const hkVector4& direction)
{
	hkVector4 minusHalf; minusHalf.getQuad() = hkQuadRealMinusHalf;
	hkVector4 qreal;
	qreal.setBroadcast(quat.m_vec, 3);
	hkVector4 q2minus1;
	q2minus1.setAddMul4( minusHalf, qreal, qreal );

	hkVector4 ret;
	ret.setMul4( direction, q2minus1 );

#if HK_CONFIG_SIMD == HK_CONFIG_SIMD_ENABLED
	hkVector4 imagDotDir; 
	imagDotDir.getQuad() = quat.getImag().dot3( direction ).getQuad();
	imagDotDir.broadcast(0);
#else
	hkReal imagDotDir = quat.getImag().dot3( direction );
#endif

	ret.addMul4( imagDotDir, quat.getImag() ); 

	hkVector4 imagCrossDir;
	imagCrossDir.setCross( quat.getImag(), direction );
	ret.addMul4( imagCrossDir, qreal );

	this->setAdd4( ret, ret );
}
#endif

#ifndef HK_VECTOR4_setRotatedInverseDir
HK_FORCE_INLINE void hkVector4::setRotatedInverseDir(const hkQuaternion& quat, const hkVector4& direction)
{
	hkVector4 minusHalf; minusHalf.getQuad() = hkQuadRealMinusHalf;
	hkVector4 qreal;
	qreal.setBroadcast(quat.m_vec, 3);
	hkVector4 q2minus1;
	q2minus1.setAddMul4( minusHalf, qreal, qreal );

	hkVector4 ret;
	ret.setMul4( direction, q2minus1 );

#if HK_CONFIG_SIMD == HK_CONFIG_SIMD_ENABLED
	hkVector4 imagDotDir; imagDotDir.m_quad = quat.getImag().dot3( direction ).getQuad();
	imagDotDir.broadcast(0);
#else
	hkReal imagDotDir = quat.getImag().dot3( direction );
#endif

	ret.addMul4( imagDotDir, quat.getImag() ); 

	hkVector4 imagCrossDir;
	imagCrossDir.setCross( direction, quat.getImag() );
	ret.addMul4( imagCrossDir, qreal );

	this->setAdd4( ret, ret );
}
#endif

inline hkBool hkVector4::equals3(const hkVector4 &v, hkReal epsilon) const
{
	//non_euclidean, manhattan based
	hkVector4 t;
	t.setSub4(*this, v);
	t.setAbs4( t );
	hkVector4 epsilon_v;
	epsilon_v.setAll3( epsilon );
	int compare_mask = epsilon_v.compareLessThan4( t ) & HK_VECTOR3_COMPARE_MASK_XYZ;
	return (compare_mask == 0);
}

inline hkBool hkVector4::equals4(const hkVector4 &v, hkReal epsilon) const
{
	//non_euclidean, manhattan based
	hkVector4 t;
	t.setSub4(*this, v);
	t.setAbs4( t );
	hkVector4 epsilon_v;
	epsilon_v.setAll( epsilon );
	int compare_mask = epsilon_v.compareLessThan4( t );
	return (compare_mask == 0);
}

inline int	hkVector4::getMajorAxis() const
{
	hkVector4 tmp;
	tmp.setAbs4(*this);
	return tmp(0) < tmp(1) ? (tmp(1) < tmp(2) ? 2 : 1) : (tmp(0) < tmp(2) ? 2 : 0);
}

inline const hkVector4& HK_CALL hkVector4::getZero()
{
	extern const hkVector4 hkVector4Zero;
	return hkVector4Zero;
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
