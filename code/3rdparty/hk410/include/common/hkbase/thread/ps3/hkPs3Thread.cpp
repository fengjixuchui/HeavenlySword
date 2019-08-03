/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkbase/hkBase.h>
#include <hkbase/thread/hkThread.h>
#include <hkbase/thread/hkThreadLocalData.h>
#include <hkbase/fwd/hkcstdio.h>
#include <hkbase/config/hkConfigThread.h>

#if (HK_CONFIG_THREAD==HK_CONFIG_MULTI_THREADED)

#include <sys/ppu_thread.h>

#define STACK_SIZE 0x40000		/* 256KB */
#define PRIO 100 // 1001 == default thread priority of the main PPU thread, so want the same as that

hkThread::hkThread()
	: m_thread(HK_NULL)
{
}

hkThread::~hkThread()
{
	if( m_thread )
	{
		uint64_t exit_code;
		/*int ret = */ sys_ppu_thread_join( (sys_ppu_thread_t)m_thread, &exit_code);
	}
}

hkUint64 HK_CALL hkThread::getMyThreadId()
{
	hkUint64 tid; // a unit64
	__asm__ volatile ( "ori %0,13,0" : "=r"( tid ) ); //r13 == tid in our setup  (sys_ppu_thread_get_id(&tid) 
	return tid;
}

// so that thread funcs are the same on PC and PS3
// we can redirect the thread func through the PS3 style:
union ThreadStartArg
{
	struct Pair
	{
		hkThread::StartFunction func;
		void* arg;
	} pointers;
	hkUint64 combined;
};

HK_COMPILE_TIME_ASSERT( sizeof(ThreadStartArg) == sizeof(uint64_t) );

static void staticRedirectFunc(uint64_t arg64 )
{
	ThreadStartArg tsa; tsa.combined = arg64;
	tsa.pointers.func(tsa.pointers.arg);

	sys_ppu_thread_exit(0);
}

hkResult hkThread::startThread( hkThread::StartFunction func, void* arg )
{
	sys_ppu_thread_t pu_thr;
	ThreadStartArg tsa;
	tsa.pointers.func = func;
	tsa.pointers.arg = arg;

	int ret = sys_ppu_thread_create(&pu_thr, staticRedirectFunc,
		tsa.combined, PRIO, STACK_SIZE,
		SYS_PPU_THREAD_CREATE_JOINABLE, "Havok PPU Thread");

	m_thread = (void*)(pu_thr); // ok as void* == uint64 == sys_ppu_thread_t

	return ret == CELL_OK? HK_SUCCESS : HK_FAILURE;
}

HK_COMPILE_TIME_ASSERT( HK_TLS_MAX_THREADS == 4 );


void hkThreadLocalDataGetNewEntry(CellSyncMutex* mutex, hkUint64* tidToIndex, const sys_ppu_thread_t tid)
{
	// Special case if the mutex is not valid yet
	if ( mutex->uint_val == 0xffffffff )
		cellSyncMutexInitialize(mutex);

	// if we are here we do not have an entry yet
	cellSyncMutexLock(mutex);
	for(int i = 0 ; i < HK_TLS_MAX_THREADS; ++i)
	{
		if (tidToIndex[i] == HK_TLS_NO_INDEX)
		{
			tidToIndex[i] = tid; // take this entry
			cellSyncMutexUnlock(mutex);
			return;
		}
	}
	cellSyncMutexUnlock(mutex);
	
	// Too many threads on PPU: breakpoint
	HK_ASSERT2( 0xf0323456, false, "Havok physics does not support more than 4 ppu threads" );
	#ifdef HK_DEBUG
		__asm__ volatile ( "tw 31,1,1" );
	#endif

	return;
}	

void hkThreadLocalDataNotInitAssert()
{
	HK_ASSERT2( 0xf0323465, false , "You have to call ::init() before you can call getInstance()" );
}

#else // Single threaded

hkThread::hkThread()
{
	HK_ASSERT(0xf9178fa1, "hkSemaphone being used in a single threaded PS3 environment. It will have no effect.");
}

hkThread::~hkThread()
{
}

hkResult hkThread::startThread( hkThread::StartFunction func, void* arg )
{
	return HK_SUCCESS;
}

hkUint64 hkThread::getMyThreadId()
{
	hkUint64 tid; // a unit64
	__asm__ volatile ( "ori %0,13,0" : "=r"( tid ) ); //r13 == tid in our setup  (sys_ppu_thread_get_id(&tid) 
	return tid;
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
