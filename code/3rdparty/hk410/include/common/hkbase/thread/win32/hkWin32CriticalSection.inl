/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

	//
	// Win32
	//

#ifdef HK_TIME_CRITICAL_SECTION_LOCKS
#	include <hkbase/monitor/hkMonitorStream.h>
#endif

// Win32 style only impl here for the moment.
inline hkCriticalSection::hkCriticalSection( int spinCount )
#if HK_CONFIG_THREAD == HK_CONFIG_MULTI_THREADED
 : hkCriticalSectionBase(spinCount)
#endif
{
#if defined(HK_COMPILER_MSVC) && (HK_COMPILER_MSVC_VERSION < 1300)
	InitializeCriticalSection( &m_section );
#else // VC7 and higher
	InitializeCriticalSectionAndSpinCount( &m_section, spinCount );
#endif
#if defined HK_SIMULATE_SPU_DMA_ON_CPU
	m_this = this;
#endif
	m_currentThread = HK_INVALID_THREAD_ID;
}

inline hkCriticalSection::~hkCriticalSection()
{
	DeleteCriticalSection(&m_section );
}

inline bool hkCriticalSection::haveEntered()
{
	return m_currentThread == hkThread::getMyThreadId();
}

inline bool hkCriticalSection::isEntered() const
{
	return m_currentThread != HK_INVALID_THREAD_ID;
}

inline int hkCriticalSection::tryEnter()
{
	if ( TryEnterCriticalSection(&m_section ) )
	{
		m_currentThread = hkThread::getMyThreadId();
	}
}

inline void hkCriticalSection::setTimersEnabled()
{
#ifdef HK_TIME_CRITICAL_SECTION_LOCKS
	HK_THREAD_LOCAL_SET(m_timeLocks, 1);
#endif
}
inline void hkCriticalSection::setTimersDisabled()
{
#ifdef HK_TIME_CRITICAL_SECTION_LOCKS
	HK_THREAD_LOCAL_SET(m_timeLocks, 0);
#endif
}

#ifndef HK_TIME_CRITICAL_SECTION_LOCKS
	inline void hkCriticalSection::enter()
	{
		EnterCriticalSection(&m_section );
		m_currentThread = hkThread::getMyThreadId();
	}

	inline void hkCriticalSection::leave()
	{
		m_currentThread = HK_INVALID_THREAD_ID;
		LeaveCriticalSection(&m_section );
	}
#else // HK_TIME_CRITICAL_SECTION_LOCKS

#if defined HK_SIMULATE_SPU_DMA_ON_CPU
#	include <hkbase/thread/util/hkSpuSimulator.h>
#endif

	inline void hkCriticalSection::enter()
	{
#if defined HK_SIMULATE_SPU_DMA_ON_CPU
			// only forward critical sections if they are created on the server
		if ( (this->m_this != this) && hkSpuSimulator::Client::getInstance() )
		{
			hkSpuSimulator::Client::getInstance()->enterCriticalSection(this);
			return;
		}
#endif
		if ( TryEnterCriticalSection(&m_section) )
		{
		}
		else
		{
			if ( HK_THREAD_LOCAL_GET(m_timeLocks) )
			{
				HK_TIMER_BEGIN("CriticalLock", HK_NULL);
				EnterCriticalSection( &m_section );
				HK_TIMER_END();
			}
			else
			{
				EnterCriticalSection( &m_section );
			}
		}
		m_currentThread = hkThread::getMyThreadId();
	}

	inline void hkCriticalSection::leave()
	{
#if defined HK_SIMULATE_SPU_DMA_ON_CPU
		// only forward critical sections if they are created on the server
		if ( (this->m_this != this) && hkSpuSimulator::Client::getInstance() )
		{
			hkSpuSimulator::Client::getInstance()->leaveCriticalSection(this);
			return;
		}
#endif
		m_currentThread = HK_INVALID_THREAD_ID;
		LeaveCriticalSection(&m_section );
	}
#endif // HK_TIME_CRITICAL_SECTION_LOCKS


hkUint32 HK_CALL hkCriticalSection::atomicExchangeAdd(hkUint32* var, int value)
{
	return InterlockedExchangeAdd( (LONG*)var, value);
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
