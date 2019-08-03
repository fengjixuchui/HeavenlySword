#if !defined(CORE_SYNCPRIMS_H)
#define CORE_SYNCPRIMS_H

//-----------------------------------------------------------
//!
//!	\file core\syncprims.h
//! A bunch of primitives for thread syncing. Critical, atomic,
//! mutuxes for starters. 
//!
//! SUPER C:
//! This is a 'super' C implementation, it should be 
//! treated as C code using C++ style syntax and not 
//! true C++ code. In particular, ctors must not be 
//! required, nor dtors (they can exist for convience but
//! must not be required). No vtables and types must be PODs
//! \see http://kong:8080/confluence/display/CH/SuperC
//!
//-----------------------------------------------------------

#if defined( PLATFORM_PC )
#include "core/syncprims_pc.h"
#elif defined( PLATFORM_PS3 )
#include "core/syncprims_ps3.h"
#endif

//--------------------------------------------------
//!
//!	A critical section object.
//!	Entering a critical section that already another 
//! thread has already entered if will cause a block
//! until the other thread is finished. The most basic
//! all all sync prims, allows only one thread to exectute
//! a particular bit of code at once (code delimated by
//! enter/leave)
//!
//--------------------------------------------------
class CriticalSection
{
public:
	//! ctor. Calls Initilise for you
	CriticalSection()
	{
		Initialise();
	}

	//! dtor. Calls Kill for you
	~CriticalSection()
	{
		Kill();
	}

	//! setup the critical section
	void Initialise();

	//! tear down the critical section
	void Kill();

	//! Enter this section.
	void Enter();

	//! Leave this section
	void Leave();

	//! try and enter, but don't block if can't
	bool TryEnter();

private:
	CriticalSectionPlatform m_Platform;
};

//--------------------------------------------------
//!
//!	Enters a critical section will the object exists.
//! A simple helper class, that enters and leave a
//! critical based on the C++ scope. Easy to protect
//! function or bits of code.
//! Usage:
//!	{
//!		ScopedCritical sc( g_GlobalCritical );
//! 
//!		// Only one thread at a time can enter here
//! }
//!
//--------------------------------------------------
class ScopedCritical
{
public:
	//! ctor enters the critical section provided
	ScopedCritical( CriticalSection& crit ) :
		m_critSec( crit )
	{
		m_critSec.Enter();
	}
	
	//! dtor leaves the critical section
	~ScopedCritical()
	{
		m_critSec.Leave();
	}
	ScopedCritical& operator=( const ScopedCritical& a )
	{
		m_critSec = a.m_critSec;
		return *this;
	}
private:
	CriticalSection& m_critSec;
};

//--------------------------------------------------
//!
//! A object that can be waited on until signelled.
//! While waiting, if doesn't consume processor resources
//! A signal that occurs before a wait will cause the 
//! wait to become a null op.
//! When a wait has finished, the signel state is reset
//! ready for another wake/signal cycle
//!
//--------------------------------------------------
class WaitableEvent
{
public:
	//! ctor
	WaitableEvent()
	{
		Initialise();
	}

	//! dtor
	~WaitableEvent()
	{
		Kill();
	}

	//! setup the critical section
	void Initialise();

	//! tear down the critical section
	void Kill();

	//! Wait for the signal to occur. NOP if a signal occured
	//! before this wait cycle (i.e. wait | signal | signal | wait is safe..
	void Wait();

	//! Wake any wait'ee to continue
	void Wake();

private:
	WaitableEventPlatform m_Platform;
};

//--------------------------------------------------
//
// Platform specific Note:
// On PS3 the atomic functions actually lock an entire
// cache line (128 bytes). This means the multiple atomic
// ops to a single cache, we have potentially cause stalls
// for no reason. If this poses a problem, you should ensure
// the each atomically fiddled with address leaves in a 
// different cache line (worse case allocate 128 bytes per
// variable...)
//
//--------------------------------------------------

//! an atomic a = b  for int32_t types
inline int32_t AtomicSet( volatile int32_t* a, const int32_t b )
{
	return AtomicSetPlatform( a, b );
}
//! an atomic a = b for uint32_t types
inline uint32_t AtomicSet( volatile uint32_t* a, const uint32_t b )
{
	return AtomicSetPlatform( a, b );
}
//! an atomic a = b for int64_t types
inline int64_t AtomicSet( volatile int64_t* a, const int64_t b )
{
	return AtomicSetPlatform( a, b );
}
//! an atomic a = b for uint64_t types
inline uint64_t AtomicSet( volatile uint64_t* a, const uint64_t b )
{
	return AtomicSetPlatform( a, b );
}

//! an atomic a=a+1 for int32_t types
inline int32_t AtomicIncrement( volatile int32_t* a )
{
	return AtomicIncrementPlatform( a );
}
//! an atomic a=a+1 for uint32_t types
inline uint32_t AtomicIncrement( volatile uint32_t* a )
{
	return AtomicIncrementPlatform( a );
}
//! an atomic a=a+1 for int64_t types
inline int64_t AtomicIncrement( volatile int64_t* a )
{
	return AtomicIncrementPlatform( a );
}
//! an atomic a=a+1 for uint64_t types
inline uint64_t AtomicIncrement( volatile uint64_t* a )
{
	return AtomicIncrementPlatform( a );
}

//! an atomic a=a-1 for int32_t types
inline int32_t AtomicDecrement( volatile int32_t* a )
{
	return AtomicDecrementPlatform( a );
}
//! an atomic a=a-1 for uint32_t types
inline uint32_t AtomicDecrement( volatile uint32_t* a )
{
	return AtomicDecrementPlatform( a );
}
//! an atomic a=a-1 for int64_t types
inline int64_t AtomicDecrement( volatile int64_t* a )
{
	return AtomicDecrementPlatform( a );
}
//! an atomic a=a-1 for uint64_t types
inline uint64_t AtomicDecrement( volatile uint64_t* a )
{
	return AtomicDecrementPlatform( a );
}

#endif // end CORE_SYNCPRIMS_H
