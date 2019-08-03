/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */



inline hkBoxBoxCollisionDetection::hkBoxBoxCollisionDetection(
		const hkCdBody& bodyA, const hkCdBody& bodyB,
		const hkProcessCollisionInput* env, hkContactMgr* mgr,	hkProcessCollisionOutput* result,
		const hkTransform& aTb, 
		const hkTransform& wTa, const hkVector4 & radiusA, 
		const hkTransform& wTb, const hkVector4 & radiusB, hkReal tolerance )
	
: 	m_bodyA( bodyA ), 
	m_bodyB( bodyB ), 
	m_env( env ),
	m_contactMgr( mgr ),
	m_result( result ),
	m_wTa(wTa), 
	m_wTb(wTb), 
	m_aTb(aTb),
	m_radiusA(radiusA), 
	m_radiusB(radiusB), 
	m_tolerance(tolerance),
	m_boundaryTolerance( 0.01f )
{
	m_tolerance4.setAll3 ( tolerance );
	m_keepRadiusA.setAdd4( m_tolerance4, m_radiusA );
	m_keepRadiusB.setAdd4( m_tolerance4, m_radiusB );
}
 

inline void hkBoxBoxCollisionDetection::setvdProj( const hkRotation& bRa, int edgeNext, int edgeNextNext, hkVector4& vdproj ) const
{
	hkVector4 a; a.setMul4( m_dinA.getSimdAt(edgeNextNext), bRa.getColumn(edgeNext ));
	hkVector4 b; b.setMul4( m_dinA.getSimdAt(edgeNext), bRa.getColumn(edgeNextNext ));
	vdproj.setSub4( a,b );
	vdproj.setAbs4( vdproj );
}

void hkBoxBoxCollisionDetection::initWorkVariables() const
{
	m_dinA = m_aTb.getTranslation();
	m_dinB._setRotatedInverseDir( m_aTb.getRotation(), m_dinA );
}


inline hkBool hkBoxBoxCollisionDetection::getPenetrations()
{
	initWorkVariables();
	return HK_SUCCESS == checkIntersection( m_tolerance4 );
}


/*
* Havok SDK - CLIENT RELEASE, BUILD(#20060902)
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
