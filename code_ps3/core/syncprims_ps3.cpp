//------------------------------------------------------------
//!
//! \file core/syncprims_ps3.cpp
//! Implementation of the Lv2 PS3 OS specific synchronisation primitives
//!
//!
//------------------------------------------------------------

#include "core/syncprims.h"

//--------------------------------------------------
//!
//!	Critical Section ctor.
//! creates a FIFO mutex
//!
//--------------------------------------------------
void CriticalSection::Initialise()
{
#ifdef USE_LIGHTWEIGHT_MUTEX
	sys_lwmutex_attribute_t attr;
	attr.attr_protocol = SYS_SYNC_FIFO;
	attr.attr_recursive = SYS_SYNC_RECURSIVE;
	sys_lwmutex_create( &m_Platform.m_Crit, &attr );
#else
	sys_mutex_attribute_t attr;
	attr.attr_protocol = SYS_SYNC_FIFO;
	attr.attr_recursive = SYS_SYNC_RECURSIVE;
	attr.attr_pshared = SYS_SYNC_NOT_PROCESS_SHARED;
	attr.attr_adaptive = SYS_SYNC_NOT_ADAPTIVE;
	sys_mutex_create( &m_Platform.m_Crit, &attr );
#endif
};

//--------------------------------------------------
//!
//!	Critical Section dtor.
//!
//--------------------------------------------------
void CriticalSection::Kill()
{
#ifdef USE_LIGHTWEIGHT_MUTEX
	sys_lwmutex_destroy( &m_Platform.m_Crit );
#else
	sys_mutex_destroy( m_Platform.m_Crit );
#endif
};

//--------------------------------------------------
//!
//! Enter the critical section and block until it
//! can. No guarentee's of exectution order
//!
//--------------------------------------------------
void CriticalSection::Enter()
{
#ifdef USE_LIGHTWEIGHT_MUTEX
	sys_lwmutex_lock( &m_Platform.m_Crit, 0 );
#else
	sys_mutex_lock( m_Platform.m_Crit, 0 );
#endif
}

//--------------------------------------------------
//!
//! Leaves the critical section and allow other threads
//! to enter this section. No guarentee's of exectution order
//!
//--------------------------------------------------
void CriticalSection::Leave()
{
#ifdef USE_LIGHTWEIGHT_MUTEX
	sys_lwmutex_unlock( &m_Platform.m_Crit );
#else
	sys_mutex_unlock( m_Platform.m_Crit );
#endif
}

//--------------------------------------------------
//!
//! Trys to enter the critical section and returns the and block until it
//! result. Higher level code can use this to 'back' off rather
//! than block the thread if the section is already entered
//! \return true if the section was entered, false if another thread is currently in this section
//!
//--------------------------------------------------
bool CriticalSection::TryEnter()
{
#ifdef USE_LIGHTWEIGHT_MUTEX
	return (sys_lwmutex_trylock( &m_Platform.m_Crit ) == (int)EBUSY);
#else
	return (sys_mutex_trylock( m_Platform.m_Crit ) == (int)EBUSY);
#endif
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------

//--------------------------------------------------
//!
//!	Waitable Event ctor.
//! CellOS uses a conditional variable
//!
//--------------------------------------------------
void WaitableEvent::Initialise()
{
	sys_mutex_attribute_t mattr;
	mattr.attr_protocol = SYS_SYNC_FIFO;
	mattr.attr_recursive = SYS_SYNC_RECURSIVE;
	mattr.attr_pshared = SYS_SYNC_NOT_PROCESS_SHARED;
	mattr.attr_adaptive = SYS_SYNC_NOT_ADAPTIVE;
	int mres = sys_mutex_create( &m_Platform.m_Mutex, &mattr );
	ntAssert( mres == CELL_OK );
	UNUSED( mres );


	sys_cond_attribute_t cattr;
	cattr.attr_pshared = SYS_SYNC_NOT_PROCESS_SHARED;
	cattr.key = 0;
	cattr.flags = 0;
	int cres = sys_cond_create( &m_Platform.m_Conditional, m_Platform.m_Mutex, &cattr );
	ntAssert( cres == CELL_OK );
	UNUSED( cres );

}

//--------------------------------------------------
//!
//!	Waitable Event dtor.
//!
//--------------------------------------------------
void WaitableEvent::Kill()
{
	sys_cond_destroy( m_Platform.m_Conditional );
	sys_mutex_destroy( m_Platform.m_Mutex );
}

//--------------------------------------------------
//!
//!	Waitable Event Wait.
//! sleep this thread until woken by wake
//!
//--------------------------------------------------
void WaitableEvent::Wait()
{
	sys_mutex_lock( m_Platform.m_Mutex, 0 );

	sys_cond_wait( m_Platform.m_Conditional, 0 );
	sys_mutex_unlock( m_Platform.m_Mutex );

}

//--------------------------------------------------
//!
//!	Waitable Event Wake.
//! Wake any waiting threads
//!
//--------------------------------------------------
void WaitableEvent::Wake()
{
	sys_cond_signal_all(  m_Platform.m_Conditional );
}
