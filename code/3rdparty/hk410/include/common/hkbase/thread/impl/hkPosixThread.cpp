/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkbase/hkBase.h>
#include <hkbase/thread/hkThread.h>
#include <pthread.h>
#include <hkbase/fwd/hkcstdio.h>

#define CHECK( A ) if( A != 0 ) { perror(#A); HK_BREAKPOINT(); } else

hkThread::hkThread()
	: m_thread(HK_NULL), m_threadId(THREAD_NOT_STARTED)
{
}

hkThread::~hkThread()
{
	if( m_thread )
	{
		CHECK( pthread_join( (pthread_t)m_thread, HK_NULL ) );
		m_thread = HK_NULL;
	}
}


hkResult hkThread::startThread( hkThread::StartFunction func, void* arg )
{
	pthread_t thread;
	CHECK( pthread_create( &thread, HK_NULL, func, arg) );
	m_thread = (void*)thread;
	m_threadId = THREAD_RUNNING;
	return HK_SUCCESS;
}

hkThread::Status hkThread::getStatus()
{
	return static_cast<Status>(m_threadId);
}

hkUint64 hkThread::getChildThreadId()
{
	return hkUlong(m_thread);
}

void* hkThread::getHandle()
{
	return m_thread;
}

hkUint64 hkThread::getMyThreadId()
{
	return pthread_self();
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
