/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkinternal/hkInternal.h>

#include <hkbase/thread/hkCriticalSection.h>

#include <hkinternal/collide/broadphase/hkBroadPhase.h>



hkBroadPhase::hkBroadPhase()
{
	m_criticalSection = HK_NULL;
	m_multiThreadLock.disableChecks();
}

void hkBroadPhase::enableMultiThreading(int spinCountForCriticalSection)
{
	if (!m_criticalSection)	
	{
		m_criticalSection = new hkCriticalSection(spinCountForCriticalSection);
		m_multiThreadLock.enableChecks();
	}
}

hkBroadPhase::~hkBroadPhase()
{
	if ( m_criticalSection )
	{
		delete m_criticalSection;
		m_criticalSection = HK_NULL;
	}
}

void hkBroadPhase::lockImplementation()
{
	m_criticalSection->enter();
	markForWrite();
}

void hkBroadPhase::unlockImplementation()
{
	unmarkForWrite();
	m_criticalSection->leave();
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
