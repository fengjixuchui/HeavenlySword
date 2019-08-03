/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HKBASE_HK_CRITICAL_SECTION_H
#define HKBASE_HK_CRITICAL_SECTION_H

#include <hkbase/config/hkConfigThread.h>
#include <hkbase/thread/hkSpuUtils.h>

#ifdef HK_PLATFORM_WIN32
#	include <hkbase/fwd/hkwindows.h>
#elif defined(HK_PLATFORM_XBOX360)
#	include <xtl.h>
#elif defined(HK_PLATFORM_PS3) || defined(HK_PLATFORM_PS3SPU) 
#	include <cell/sync.h>
#	if defined(HK_PLATFORM_PS3) 
#		include <sys/synchronization.h>	
#	endif
#endif

#include <hkbase/thread/hkThreadLocalData.h>

	/// Set this define if you want a timer begin and timer end call 
	/// if a thread has to wait for a critical section
	/// You also have to call hkCriticalSection::setTimersEnabled()
#define HK_TIME_CRITICAL_SECTION_LOCKS

class hkCriticalSection;

#if !defined (HK_TIME_CRITICAL_SECTION_LOCKS) && defined(HK_SIMULATE_SPU_DMA_ON_CPU)
#	define HK_TIME_CRITICAL_SECTION_LOCKS
#endif

#if HK_CONFIG_THREAD == HK_CONFIG_MULTI_THREADED
#	include <hkbase/thread/hkSemaphoreBusyWait.h>

	// The hkCriticalSectionBase class manages list of
	// critical section wrappers for multi-threaded configuration
	// of Havok libraries.
	// The class has public members to access the critical section
	// wrappers created by Havok.
	// This class is a base class for the hkCriticalSection class
	// only in multi-threaded configuration of Havok libraries.
class hkCriticalSectionBase
{
	protected:
		inline hkCriticalSectionBase(int spinCount);
		inline ~hkCriticalSectionBase();

	public:
			// The spin count value of created critical section.
		int m_spinCount;
			// Link to the previous critical section wrapper in the list.
		hkCriticalSectionBase* m_prev;
			// Link to the next critical section wrapper in the list.
		hkCriticalSectionBase* m_next;
			// Head of the critical section wrappers list.
		static hkCriticalSectionBase* sHead;
			// Semaphore to control access to the critical section wrappers list.
		static hkSemaphoreBusyWait sListSemaphore;
};

inline hkCriticalSectionBase::hkCriticalSectionBase(int spinCount)
{
#ifndef HK_PLATFORM_PS3SPU
	hkCriticalSectionBase::sListSemaphore.acquire();
#endif

	m_spinCount = spinCount;
	// add to the list
	m_prev = HK_NULL;
	m_next = sHead;
	if( hkCriticalSectionBase::sHead )
	{
		hkCriticalSectionBase::sHead->m_prev = this;
	}
	hkCriticalSectionBase::sHead = this;

#ifndef HK_PLATFORM_PS3SPU
	hkCriticalSectionBase::sListSemaphore.release();
#endif
}

inline hkCriticalSectionBase::~hkCriticalSectionBase()
{
#ifndef HK_PLATFORM_PS3SPU
	hkCriticalSectionBase::sListSemaphore.acquire();
#endif

	if( m_prev )
	{
			// If you get crash here:
			// It seems some critical sections were not destroyed and removed
			// from the list of the critical section wrappers, and links are
			// referring to the already invalid objects.
			// To resolve the issue make sure there are no memory leaks of the
			// critical section wrappers.
		m_prev->m_next = m_next;
	}
	else
	{
		hkCriticalSectionBase::sHead = m_next;
	}
	if( m_next )
	{
			// If you get crash here:
			// It seems some critical sections were not destroyed and removed
			// from the list of the critical section wrappers, and links are
			// referring to the already invalid objects.
			// To resolve the issue make sure there are no memory leaks of the
			// critical section wrappers.
		m_next->m_prev = m_prev;
	}

#ifndef HK_PLATFORM_PS3SPU
	hkCriticalSectionBase::sListSemaphore.release();
#endif
}
#endif // HK_CONFIG_THREAD == HK_CONFIG_MULTI_THREADED

//#define HK_PS3_CRITICAL_SECTION_SYSTEM_WAIT

	/// Critical section wrapper. This can be used to guard access
	/// to shared data structures between threads.
	/// Note that critical sections are fast but have serious drawbacks.
	/// Check windows help for details. 
	/// Note that including this file means including a system file, such as windows.h, 
	/// which makes compile time significantly slower.

class hkCriticalSection
#if HK_CONFIG_THREAD == HK_CONFIG_MULTI_THREADED
	: public hkCriticalSectionBase
#endif
{
	public:
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_BASE_CLASS, hkCriticalSection);

			/// Init a critical section with spin count. 
			/// Read MSDN help of InitializeCriticalSectionAndSpinCount for details.
			/// In short: positive spinCount value results in threads doing busy waiting and is good
			///           when you know that the critical section will only be locked for a short period of time.
			///           zero value causes the thread to immediately go back to the system when waiting.
		inline hkCriticalSection( int spinCount = 0);

			/// Quit a critical section
		inline ~hkCriticalSection();

			/// Lock a critical section
		inline void enter();

			/// Returns whether the current thread has entered this critical section.
		inline bool haveEntered();

		/// Returns if any thread has entered this critical section, returns false
		/// if multithreading is disabled.
		inline bool isEntered() const;

			/// Try to lock a critical section, return !=0 when successful
		inline int tryEnter();

			/// Unlock a critical section
		inline void leave();


			/// Tell the critical section to time blocking locks. HK_TIME_CRITICAL_SECTION_LOCKS must be
			/// defined at compile time for this flag to have any effect
		static inline void HK_CALL setTimersEnabled();

			/// Stop timing blocking locks
		static inline void HK_CALL setTimersDisabled();

			/// adds a value to a variable in a thread safe way and returns the old value
		static HK_FORCE_INLINE hkUint32 HK_CALL atomicExchangeAdd(hkUint32* var, int value);

	public:

#if HK_CONFIG_THREAD == HK_CONFIG_MULTI_THREADED
#	if defined(HK_PLATFORM_PS3) || defined(HK_PLATFORM_PS3SPU) 
		
		// Busy wait mutex used on both PPU and SPU,  a uint32, 
		// not used as this data on SPU, but used through the ptr to the actual one in main mem.
		CellSyncMutex m_mutex; // 32bits
		hkInt32 m_recursiveLockCount;
		HK_CPU_PTR(CellSyncMutex*) m_mutexPtr;  // always 64bit
		hkUint64 m_currentThread; // if this == current, then we have the lock already and just inc recursion count
		
#		if defined(HK_PS3_CRITICAL_SECTION_SYSTEM_WAIT)
#			if defined(HK_PLATFORM_PS3)// PPU  
		// have a kinder system level lock first, so that PPU threads
		// can sleep on locks with each other. ie, if this is locked, then a 
		// ppu thread has the lock, and we can sleep on it
		sys_mutex_t m_ppuMutex;
#			else  // SPU 
		hkUint32   m_ppuMutexPadding;
#			endif
#		endif


#	else // other threaded platforms:

		CRITICAL_SECTION  m_section;
		hkUint64 m_currentThread;

#		ifdef HK_TIME_CRITICAL_SECTION_LOCKS
			static HK_THREAD_LOCAL( int ) m_timeLocks;
#		endif


			// points to this. This is useful for SPU simulation where this pointer
			// points to the main memory critical section
#		ifdef HK_SIMULATE_SPU_DMA_ON_CPU
			hkCriticalSection* m_this;
#		endif

#	endif
#endif
};

// include the inl before the hkCriticalSectionLock def so 
// that gcc can inline the enter and leave properly
#if HK_CONFIG_THREAD != HK_CONFIG_MULTI_THREADED 
#		include <hkbase/thread/impl/hkEmptyCriticalSection.inl>
#else // HK_CONFIG_THREAD == HK_CONFIG_MULTI_THREADED 
#	include <hkbase/thread/hkThread.h>
#	define HK_INVALID_THREAD_ID (hkUint64(-1))

#	if defined(HK_PLATFORM_PS3) || defined(HK_PLATFORM_PS3SPU) 
#		include <hkbase/thread/ps3/hkPs3CriticalSection.inl>
#	else
#		include <hkbase/thread/win32/hkWin32CriticalSection.inl>
#	endif
#endif


	/// Helper class which locks a critical section as long is this object exists. 
class hkCriticalSectionLock
{
	public:
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_BASE_CLASS, hkCriticalSectionLock);

			/// Create a lock by entering a critical section.
		inline hkCriticalSectionLock( hkCriticalSection* section )
		{
			m_section = section;
			section->enter();
		}

			/// Destructor leaves the critical section
		inline ~hkCriticalSectionLock()
		{
			m_section->leave();
		}

	protected:
		hkCriticalSection* m_section;
};



	/// A base class to ensure locks are not sharing cache lines with other data.
class hkSynchronized
{
	public:
		hkSynchronized( int spinCount ):m_criticalSection(spinCount){}

		void lockMt(){ m_criticalSection.enter(); }
		void unlockMt() { m_criticalSection.leave(); }
	public:
		HK_ALIGN( hkCriticalSection m_criticalSection, 64 );
};

	/// Put this macro as the first line of each member function of 
	/// derived classes of hkSynchronized
#define HK_SYNCRONIZE hkCriticicalSectionLock lock(&m_criticalSection);


#endif // HKBASE_HK_CRITICAL_SECTION_H

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
