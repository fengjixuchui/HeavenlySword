/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

inline int hkProcessCollisionData::getNumContactPoints() const
{
	return int(m_firstFreeContactPoint - &m_contactPoints[0]);
}

inline hkProcessCdPoint* hkProcessCollisionData::getFirstContactPoint()
{
	return &m_contactPoints[0];
}

inline hkProcessCdPoint* hkProcessCollisionData::getEnd()
{
	return m_firstFreeContactPoint;
}

inline hkProcessCdPoint& hkProcessCollisionData::getContactPoint( int i )
{
	return m_contactPoints[i];
}

inline hkBool hkProcessCollisionData::isEmpty() const
{
	return m_firstFreeContactPoint == &m_contactPoints[0];
}

inline hkBool hkProcessCollisionData::hasToi() const
{
	return m_toi.m_time != HK_REAL_MAX;
}

inline hkContactPoint& hkProcessCollisionData::getToiContactPoint()
{
	return m_toi.m_contactPoint;
}

inline hkTime hkProcessCollisionData::getToi() const
{
	return m_toi.m_time;
}

inline hkContactPointMaterial& hkProcessCollisionData::getToiMaterial()
{
	return m_toi.m_material;
}


hkProcessCollisionData::ToiInfo::ToiInfo() : m_time( HK_REAL_MAX ) 
{
}

hkProcessCollisionData::hkProcessCollisionData( hkCollisionConstraintOwner* owner )
{
	m_constraintOwner = owner;
}


void hkProcessCollisionData::ToiInfo::flip()
{
	hkVector4& norm = m_contactPoint.getSeparatingNormal();
	norm.setNeg3( norm );
	m_gskCache.flip();
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
