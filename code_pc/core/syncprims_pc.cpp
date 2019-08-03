//------------------------------------------------------------
//!
//! \file core/syncprims_pc.cpp
//! Implementation of the NT specific synchronisation primitives
//!
//! \note 64 bit operations are MUCH slower on PC than PS3
//! \note the Win32 OS doesn't support them, so I've added a hacky
//! \note emulation but its not very fast.
//!
//------------------------------------------------------------

#include "core/syncprims.h"

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------

//--------------------------------------------------
//!
//!	Critical Section ctor.
//! calls the OS function initialize CriticalSection
//!
//--------------------------------------------------
void CriticalSection::Initialise()
{
	InitializeCriticalSection( &m_Platform.m_Crit );
};

//--------------------------------------------------
//!
//!	Critical Section dtor.
//! calls the OS function DeleteCriticalSection
//! Asserts that no thread owns this critical section 
//! (cause that would be bad)
//!
//--------------------------------------------------
void CriticalSection::Kill()
{
	CRITICAL_SECTION* pCritSec = &m_Platform.m_Crit;

	if( pCritSec )
	{
		ntError_p( pCritSec->OwningThread == 0, ("A thread is still accessing this critical section") );

		DeleteCriticalSection( pCritSec );
	}
};

//--------------------------------------------------
//!
//! Enter the critical section and block until it
//! can. No guarentee's of exectution order
//! calls the OS function EnterCriticalSection
//!
//--------------------------------------------------
void CriticalSection::Enter()
{
	CRITICAL_SECTION* pCritSec = &m_Platform.m_Crit;
	EnterCriticalSection( pCritSec );
}

//--------------------------------------------------
//!
//! Leaves the critical section and allow other threads
//! to enter this section. No guarentee's of exectution order
//! calls the OS function LeaveCriticalSection
//!
//--------------------------------------------------
void CriticalSection::Leave()
{
	CRITICAL_SECTION* pCritSec = &m_Platform.m_Crit;
	LeaveCriticalSection( pCritSec );
}

//--------------------------------------------------
//!
//! Trys to enter the critical section and returns the and block until it
//! result. Higher level code can use this to 'back' off rather
//! than block the thread if the section is already entered
//! calls the OS function TryEnterCriticalSection
//! \return true if the section was entered, false if another thread is currently in this section
//!
//--------------------------------------------------
bool CriticalSection::TryEnter()
{
	CRITICAL_SECTION* pCritSec = &m_Platform.m_Crit;
	return (TryEnterCriticalSection( pCritSec ) != 0);
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------

//--------------------------------------------------
//!
//!	Waitable Event ctor.
//! Windows use an auto reset event object
//!
//--------------------------------------------------
void WaitableEvent::Initialise()
{
	m_Platform.m_Event = CreateEvent( NULL, FALSE, FALSE, 0 );
}

//--------------------------------------------------
//!
//!	Waitable Event dtor.
//!
//--------------------------------------------------
void WaitableEvent::Kill()
{
	CloseHandle( m_Platform.m_Event );
}

//--------------------------------------------------
//!
//!	Waitable Event Wait.
//! sleep this thread until woken by wake
//!
//--------------------------------------------------
void WaitableEvent::Wait()
{
	WaitForSingleObject( m_Platform.m_Event, 0 );
}

//--------------------------------------------------
//!
//!	Waitable Event Wake.
//! Wake any waiting threads
//!
//--------------------------------------------------
void WaitableEvent::Wake()
{
	SetEvent( m_Platform.m_Event );
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
// Atomic operations. 
// NOTE Win32 on PC doesn't support the 64 bit versions so we have to emulate via a fairly slow
// and probably not perfect critical section hack.
//----------------------------------------------------------------------------------------------------
namespace 
{
	CriticalSection s_AtomicCritSec;
}

//--------------------------------------------------
//!
//! Thread Safe equivilent to 
//! ret = *a
//! *a = b;
//! return ret;
//!
//--------------------------------------------------
int32_t AtomicSetPlatform( volatile int32_t* a, const int32_t b )
{
	return (int32_t) InterlockedExchange( (volatile LONG*)a, (LONG) b );
}

//--------------------------------------------------
//!
//! Thread Safe equivilent to 
//! ret = *a
//! *a = b;
//! return ret;
//!
//--------------------------------------------------
uint32_t AtomicSetPlatform( volatile uint32_t* a, const uint32_t b )
{
	return (uint32_t) InterlockedExchange( (volatile LONG*)a, (LONG) b );
}

//--------------------------------------------------
//!
//! Thread Safe equivilent to 
//! ret = *a
//! *a = b;
//! return ret;
//!
//--------------------------------------------------
int64_t AtomicSetPlatform( volatile int64_t* a, const int64_t b )
{

	// this unfortanately sync ALL 64 bit atomics regardless if they are working on
	// different variables!
	s_AtomicCritSec.Enter();
	int64_t ret = *a;
	*a = b;
	s_AtomicCritSec.Leave();
	return ret;
}

//--------------------------------------------------
//!
//! Thread Safe equivilent to 
//! ret = *a
//! *a = b;
//! return ret;
//!
//--------------------------------------------------
uint64_t AtomicSetPlatform( volatile uint64_t* a, const uint64_t b )
{
	// this unfortanately sync ALL 64 bit atomics regardless if they are working on
	// different variables!
	s_AtomicCritSec.Enter();
	int64_t ret = *a;
	*a = b;
	s_AtomicCritSec.Leave();
	return ret;
}

//--------------------------------------------------
//!
//! Thread Safe equivilent to 
//! ret = *a
//! *a = *a + 1;
//! return ret;
//!
//--------------------------------------------------
int32_t AtomicIncrementPlatform( volatile int32_t* a )
{
	return (int32_t) InterlockedExchangeAdd( (volatile LONG*)a, 1 );
}

//--------------------------------------------------
//!
//! Thread Safe equivilent to 
//! ret = *a
//! *a = *a + 1;
//! return ret;
//!
//--------------------------------------------------
uint32_t AtomicIncrementPlatform( volatile uint32_t* a )
{
	return (uint32_t) InterlockedExchangeAdd( (volatile LONG*)a, 1 );
}

//--------------------------------------------------
//!
//! Thread Safe equivilent to 
//! ret = *a
//! *a = *a + 1;
//! return ret;
//!
//--------------------------------------------------
int64_t AtomicIncrementPlatform( volatile int64_t* a )
{
	// this unfortanately sync ALL 64 bit atomics regardless if they are working on
	// different variables!
	s_AtomicCritSec.Enter();
	int64_t ret = *a;
	*a = *a + 1;
	s_AtomicCritSec.Leave();
	return ret;
}

//--------------------------------------------------
//!
//! Thread Safe equivilent to 
//! ret = *a
//! *a = *a + 1;
//! return ret;
//!
//--------------------------------------------------
uint64_t AtomicIncrementPlatform( volatile uint64_t* a )
{
	// this unfortanately sync ALL 64 bit atomics regardless if they are working on
	// different variables!
	s_AtomicCritSec.Enter();
	uint64_t ret = *a;
	*a = *a + 1;
	s_AtomicCritSec.Leave();
	return ret;
}

//--------------------------------------------------
//!
//! Thread Safe equivilent to 
//! ret = *a
//! *a = *a - 1;
//! return ret;
//!
//--------------------------------------------------
int32_t AtomicDecrementPlatform( volatile int32_t* a )
{
	return (int32_t) InterlockedExchangeAdd( (volatile LONG*)a, -1 );
}

//--------------------------------------------------
//!
//! Thread Safe equivilent to 
//! ret = *a
//! *a = *a - 1;
//! return ret;
//!
//--------------------------------------------------
uint32_t AtomicDecrementPlatform( volatile uint32_t* a )
{
	return (uint32_t) InterlockedExchangeAdd( (volatile LONG*)a, -1  );
}

//--------------------------------------------------
//!
//! Thread Safe equivilent to 
//! ret = *a
//! *a = *a - 1;
//! return ret;
//!
//--------------------------------------------------
int64_t AtomicDecrementPlatform( volatile int64_t* a )
{
	// this unfortanately sync ALL 64 bit atomics regardless if they are working on
	// different variables!
	s_AtomicCritSec.Enter();
	int64_t ret = *a;
	*a = *a - 1;
	s_AtomicCritSec.Leave();
	return ret;
}

//--------------------------------------------------
//!
//! Thread Safe equivilent to 
//! ret = *a
//! *a = *a - 1;
//! return ret;
//!
//--------------------------------------------------
uint64_t AtomicDecrementPlatform( volatile uint64_t* a )
{
	// this unfortanately sync ALL 64 bit atomics regardless if they are working on
	// different variables!
	s_AtomicCritSec.Enter();
	uint64_t ret = *a;
	*a = *a - 1;
	s_AtomicCritSec.Leave();
	return ret;
}

