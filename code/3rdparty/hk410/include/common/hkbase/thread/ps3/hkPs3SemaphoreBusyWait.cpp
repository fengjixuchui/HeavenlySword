/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkbase/hkBase.h>
#include <hkbase/thread/hkSemaphoreBusyWait.h>

#if (HK_CONFIG_THREAD == HK_CONFIG_MULTI_THREADED)

#include <cell/atomic.h>

hkSemaphoreBusyWait::hkSemaphoreBusyWait( int initialCount, int maxCount )
{
	if (maxCount < 1 || initialCount > maxCount) 
	{
		return;
	}
#if defined(HK_PLATFORM_PS3)
	m_semphoreValue = initialCount;
	m_semaphore = &m_semphoreValue;
#else 
	HK_ASSERT(0x0, "Can not create a Havok Semaphores on an SPU at the moment, just use them.");
#endif
}

hkSemaphoreBusyWait::~hkSemaphoreBusyWait()
{
}

void hkSemaphoreBusyWait::acquire()
{
#if defined(HK_PLATFORM_PS3)
	while (cellAtomicTestAndDecr32((hkUint32*)m_semaphore) == 0)
#else
	while (cellAtomicTestAndDecr32((hkUint32*)m_cacheLine, (hkUlong)m_semaphore) == 0)
#endif
	{
		for ( int i=0; i<1000; i++)
		{
			volatile int dummy = 0;
			dummy = dummy;
		}
	}
}

void hkSemaphoreBusyWait::release(int count)
{
	for (int i=0; i < count; ++i)
	{
#if defined(HK_PLATFORM_PS3)
		cellAtomicIncr32((hkUint32*)m_semaphore);
#else
		cellAtomicIncr32(m_cacheLine, (hkUlong)m_semaphore);
#endif
	}
}

#endif

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
