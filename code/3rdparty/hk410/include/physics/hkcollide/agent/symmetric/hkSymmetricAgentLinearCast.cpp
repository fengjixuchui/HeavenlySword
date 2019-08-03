/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>

void hkSymmetricAgentFlipCollector::addCdPoint( const hkCdPoint& point )
{
	hkCdPoint p( point.m_cdBodyB, point.m_cdBodyA);
	p.m_contact.setFlipped( point.m_contact );
	m_collector.addCdPoint(p);
	this->m_earlyOutDistance = m_collector.getEarlyOutDistance();
}

void hkSymmetricAgentFlipCastCollector::addCdPoint( const hkCdPoint& point )
{
	hkCdPoint p( point.m_cdBodyB, point.m_cdBodyA);
	p.m_contact.getPosition().setAddMul4( point.m_contact.getPosition(), m_path, point.m_contact.getDistanceSimdReal());
	p.m_contact.getSeparatingNormal().setNeg3(point.m_contact.getSeparatingNormal());

	m_collector.addCdPoint(p);
	this->m_earlyOutDistance = m_collector.getEarlyOutDistance();
}


void hkSymmetricAgentFlipBodyCollector::addCdBodyPair( const hkCdBody& bodyA, const hkCdBody& bodyB )
{
	m_collector.addCdBodyPair( bodyB, bodyA );
	m_earlyOut = m_collector.getEarlyOut();
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
