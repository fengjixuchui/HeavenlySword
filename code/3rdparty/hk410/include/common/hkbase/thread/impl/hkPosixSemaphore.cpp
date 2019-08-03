/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkbase/hkBase.h>
#include <hkbase/thread/hkSemaphore.h>
#include <semaphore.h>
#include <hkbase/fwd/hkcstdio.h>

#define CHECK( A ) if( A != 0 ) { perror(#A); HK_BREAKPOINT(); } else

hkSemaphore::hkSemaphore( int initialCount, int maxCount )
{
	sem_t* sem = hkAllocate<sem_t>(1, HK_MEMORY_CLASS_BASE);
	CHECK( sem_init( sem, 0, initialCount ) );
	m_semaphore = sem;
}

hkSemaphore::~hkSemaphore()
{
	CHECK( sem_destroy( static_cast<sem_t*>(m_semaphore) ) );
}

void hkSemaphore::acquire()
{
	CHECK( sem_wait( static_cast<sem_t*>(m_semaphore) ) );
}
	
void hkSemaphore::release(int count)
{
	for( int i = 0; i < count; ++i )
	{
		CHECK( sem_post( static_cast<sem_t*>(m_semaphore) ) );
	}
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
