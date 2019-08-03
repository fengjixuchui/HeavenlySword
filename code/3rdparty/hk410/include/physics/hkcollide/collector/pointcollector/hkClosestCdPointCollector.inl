/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

void hkClosestCdPointCollector::reset()
{
	m_hitPoint.m_rootCollidableA = HK_NULL;
	m_hitPoint.m_contact.setDistance( HK_REAL_MAX );
	hkCdPointCollector::reset();
}

hkClosestCdPointCollector::hkClosestCdPointCollector  ()
{
	reset();
}

hkClosestCdPointCollector::~hkClosestCdPointCollector () 
{
}


hkBool hkClosestCdPointCollector::hasHit( ) const
{
	return m_hitPoint.m_rootCollidableA != HK_NULL;
}

const hkRootCdPoint& hkClosestCdPointCollector::getHit() const
{
	HK_ASSERT2(0x41384ce1, hasHit(), "You cannot call getHit() if there is no hit");
	return m_hitPoint;
}

const hkContactPoint& hkClosestCdPointCollector::getHitContact() const
{
	HK_ASSERT2(0x7368e429, hasHit(), "You cannot call getHit() if there is no hit");
	return m_hitPoint.m_contact;
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
