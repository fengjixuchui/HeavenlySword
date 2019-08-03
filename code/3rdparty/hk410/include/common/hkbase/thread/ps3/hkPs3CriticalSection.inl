/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

	//
	// PS3
	//
inline hkCriticalSection::hkCriticalSection( int spinCount )
#if HK_CONFIG_THREAD == HK_CONFIG_MULTI_THREADED
 : hkCriticalSectionBase(spinCount)
#endif
{
#if defined(HK_PLATFORM_PS3) 
	
#	if defined(HK_PS3_CRITICAL_SECTION_SYSTEM_WAIT)
		sys_mutex_attribute_t mutex_attr;
		sys_mutex_attribute_initialize(mutex_attr);
		mutex_attr.attr_protocol = SYS_SYNC_PRIORITY; 
		int ret = sys_mutex_create(&m_ppuMutex, &mutex_attr);
		if (ret != CELL_OK) 
		{
			HK_WARN(0x0, "sys_mutex_create failed with " << ret);
			return;
		}
		
#	endif
	m_currentThread = HK_INVALID_THREAD_ID;
	{
		m_mutexPtr = &m_mutex;
		HK_ON_DEBUG( int ret = ) cellSyncMutexInitialize(m_mutexPtr);
		HK_ASSERT(0x0, ret == CELL_OK);
	}
	
	

#else // SPU

	HK_ASSERT(0x0, "Can not create a Havok Critical Section on an SPU at the moment, just use them.");

#endif
}

inline hkCriticalSection::~hkCriticalSection()
{
#if defined(HK_PLATFORM_PS3) && defined(HK_PS3_CRITICAL_SECTION_SYSTEM_WAIT) 
	int ret = sys_mutex_destroy(m_ppuMutex);
	if (ret != CELL_OK) 
	{
		HK_WARN(0x0,"sys_mutex_destroy failed with " << ret);
	}
#endif
}

inline bool hkCriticalSection::haveEntered()
{
#if defined(HK_PLATFORM_PS3) 
	return ( m_currentThread == hkThread::getMyThreadId() );
#else
	return false; // Not yet supported on SPU - TO DO
#endif
}

inline bool hkCriticalSection::isEntered() const
{
	return m_currentThread != HK_INVALID_THREAD_ID;
}


inline int hkCriticalSection::tryEnter()
{
#if defined(HK_PLATFORM_PS3) 

	const hkUint64 tid = hkThread::getMyThreadId();
	bool haveLock = m_currentThread == tid;

	int retVal = 1;
	if (!haveLock) 
	{	
#if defined(HK_PS3_CRITICAL_SECTION_SYSTEM_WAIT)

		int ret = sys_mutex_trylock(m_ppuMutex);
		if (ret == CELL_OK) // got the outer one
		{
			retVal = cellSyncMutexTryLock( m_mutexPtr ) == CELL_OK? 1 : 0;
			if ( !retVal ) // didn't get the inner one
			{
				// let go of the outer one as we did not succeed
				sys_mutex_unlock(m_ppuMutex);
				return 0; // didn't get it
			}
			else
			{
				m_currentThread = tid;
				m_recursiveLockCount = 1;
				return 1;
			}
		}
		else
		{
			return 0; // didn't get it
		}

#else

		retVal = cellSyncMutexTryLock( m_mutexPtr ) == CELL_OK? 1 : 0;
		if (retVal)
		{
			m_currentThread = tid; // we own it now.
			m_recursiveLockCount = 1;
			return 1;
		}

#endif
	}
	else
	{
		// know we have it already
		m_recursiveLockCount++;
	}

#else

	int retVal = cellSyncMutexTryLock( (hkUlong)m_mutexPtr ) == CELL_OK? 1 : 0;

#endif

	return retVal;
}

inline void hkCriticalSection::setTimersEnabled()
{
}

inline void hkCriticalSection::setTimersDisabled()
{
}

inline void hkCriticalSection::enter()
{
	// will busy wait as is a low level mutext
#if defined(HK_PLATFORM_PS3) 

	const hkUint64 tid = hkThread::getMyThreadId();
	bool haveLock = m_currentThread == tid;

	if (!haveLock) // don't have it already 
	{
#	if defined(HK_PS3_CRITICAL_SECTION_SYSTEM_WAIT)
		// get outer lock
		/*int ret = */ sys_mutex_lock(m_ppuMutex, 0 /*== no timeout*/);
#	endif
		// get inner lock
		cellSyncMutexLock(m_mutexPtr);

		m_currentThread = tid; //we own it now.
		m_recursiveLockCount = 1;		
		return;
	}
	else
	{
		m_recursiveLockCount++;
	}
	
#else
	cellSyncMutexLock((hkUlong)m_mutexPtr);
#endif
}

inline void hkCriticalSection::leave()
{
#if defined(HK_PLATFORM_PS3) 
	// only leave if we have the lock
#ifdef HK_DEBUG
	const hkUint64 tid = hkThread::getMyThreadId();
	HK_ASSERT2(0x0, m_currentThread == tid, "Releasing lock that you don't hold");
#endif

	m_recursiveLockCount--;
	HK_ASSERT2(0x0, m_recursiveLockCount >= 0, "hkCriticalSection::leave() without matching ::enter!" );

	if (m_recursiveLockCount == 0) // actually release it
	{
		m_currentThread = HK_INVALID_THREAD_ID; //we no longer own it.

		// unlock inner
		cellSyncMutexUnlock(m_mutexPtr);

		// unlock outer
#	if defined(HK_PS3_CRITICAL_SECTION_SYSTEM_WAIT)
		/*int ret = */ sys_mutex_unlock(m_ppuMutex);
#	endif
	}

#else

	cellSyncMutexUnlock((hkUlong)m_mutexPtr);
#endif
}


hkUint32 HK_CALL hkCriticalSection::atomicExchangeAdd(hkUint32* var, int value)
{
#if defined(HK_PLATFORM_PS3) 
	return cellAtomicAdd32(var, value);
#else
	HK_ASSERT2( 0xf0323454, false, "NotImplemented" );
	return 0;
#endif
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
