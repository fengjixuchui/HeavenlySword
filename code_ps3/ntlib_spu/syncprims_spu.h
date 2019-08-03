#if !defined(NTLIB_SYNCPRIMS_H)
#define NTLIB_SYNCPRIMS_H

//-----------------------------------------------------------
//!
//!	\file ntlib\syncprims_spu.h
//! A bunch of primitives for thread syncing. Critical, atomic,
//! mutuxes for starters. 
//!
//! NOTE: The SPU versions are much reduced currently cos its
//! not a SMP architecture and not all makes sense (CriticalSection anbody??)
//! and also i have needed the more complex ones yet...
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

#ifndef __SPU__
#	error This file can only be included in an SPU project.
#endif // !__SPU__

#include <cell/atomic.h>

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
// On SPU the atomic op is to main main 
// so it returns the old value direct from EA, 
// in effect its a true shared variable. it only exists in main memory and
// accessable thro the function...
//
//--------------------------------------------------

//! an atomic a = b  for int32_t types
inline int32_t AtomicSet( uint32_t aEA, const int32_t b )
{
	uint32_t buf[32] __attribute__((aligned(128)));
	int32_t old;

	DisableInterrupts();
	do 
	{
		old = (int32_t) cellAtomicLockLine32(buf,(const uint64_t)aEA);
	} while ( cellAtomicStoreConditional32( buf, (const uint64_t)aEA, (uint32_t)(b)));
	EnableInterrupts();

	return old;
}


//! an atomic a = b for uint32_t types
inline uint32_t AtomicSet( uint32_t aEA, const uint32_t b )
{
	uint32_t buf[32] __attribute__((aligned(128)));
	uint32_t old;
	DisableInterrupts();
	do 
	{
		old = (uint32_t) cellAtomicLockLine32(buf,(const uint64_t)aEA);
	} while ( cellAtomicStoreConditional32( buf, (const uint64_t)aEA, (uint32_t)(b)));
	EnableInterrupts();

	return old;
}

//! an atomic a = b for int64_t types
inline int64_t AtomicSet( const uint32_t aEA, const int64_t b )
{
	uint64_t buf[16] __attribute__((aligned(128)));
	int64_t old;
	DisableInterrupts();
	do 
	{
		old = (int64_t) cellAtomicLockLine64(buf,(const uint64_t)aEA);
	} while ( cellAtomicStoreConditional64( buf, (const uint64_t) aEA, (uint64_t)(b)));
	EnableInterrupts();

	return old;
}

//! an atomic a = b for uint64_t types
inline uint64_t AtomicSet( const uint32_t aEA, const uint64_t b )
{
	uint64_t buf[16] __attribute__((aligned(128)));
	uint64_t old;
	DisableInterrupts();
	do 
	{
		old = (uint64_t) cellAtomicLockLine64(buf,(const uint64_t)aEA);
	} while ( cellAtomicStoreConditional64( buf, (const uint64_t)aEA, (uint64_t)(b)));
	EnableInterrupts();
	return old;
}

//! an atomic a=a+1 for int32_t types
inline int32_t AtomicIncrement( const uint32_t aEA, int32_t incr = 1 )
{
	uint32_t buf[32] __attribute__((aligned(128)));
	int32_t old;
	DisableInterrupts();
	do 
	{
		old = (int32_t) cellAtomicLockLine32(buf,(const uint64_t)aEA);
	} while ( cellAtomicStoreConditional32( buf, (const uint64_t)aEA, (uint32_t)(old + incr)));
	EnableInterrupts();

	return old;
}

//! an atomic a=a+1 for uint32_t types
inline uint32_t AtomicIncrementU( const uint32_t aEA, uint32_t incr = 1 )
{
	uint32_t buf[32] __attribute__((aligned(128)));
	uint32_t old;
	DisableInterrupts();
	do 
	{
		old = (uint32_t) cellAtomicLockLine32(buf,(const uint64_t)aEA);
	} while ( cellAtomicStoreConditional32( buf, (const uint64_t)aEA, (uint32_t)(old + incr)));
	EnableInterrupts();

	return old;
}

//! an atomic a=a+1 for int64_t types
inline int64_t AtomicIncrement64( const uint32_t aEA, int64_t incr = 1 )
{
	uint64_t buf[16] __attribute__((aligned(128)));
	int64_t old;
	DisableInterrupts();
	do 
	{
		old = (int64_t) cellAtomicLockLine64(buf,(const uint64_t)aEA);
	} while ( cellAtomicStoreConditional64( buf, (const uint64_t)aEA, (uint64_t)(old + incr)));
	EnableInterrupts();

	return old;
}
//! an atomic a=a+1 for uint64_t types
inline uint64_t AtomicIncrementU64( const uint32_t aEA, uint64_t incr = 1 )
{
	uint64_t buf[16] __attribute__((aligned(128)));
	uint64_t old;
	DisableInterrupts();
	do 
	{
		old = (uint64_t) cellAtomicLockLine64(buf,(const uint64_t)aEA);
	} while ( cellAtomicStoreConditional64( buf, (const uint64_t)aEA, (uint64_t)(old + incr)));
	EnableInterrupts();

	return old;
}

//! an atomic a=a-1 for int32_t types
inline int32_t AtomicDecrement( const uint32_t aEA, int32_t incr = 1 )
{
	uint32_t buf[32] __attribute__((aligned(128)));
	int32_t old;
	DisableInterrupts();
	do 
	{
		old = (int32_t) cellAtomicLockLine32(buf,(const uint64_t)aEA);
	} while ( cellAtomicStoreConditional32( buf, (const uint64_t)aEA, (uint32_t)(old - incr)));
	EnableInterrupts();

	return old;
}

//! an atomic a=a-1 for uint32_t types
inline uint32_t AtomicDecrementU( const uint32_t aEA, uint32_t incr = 1 )
{
	uint32_t buf[32] __attribute__((aligned(128)));
	uint32_t old;
	DisableInterrupts();
	do 
	{
		old = (uint32_t) cellAtomicLockLine32(buf,(const uint64_t)aEA);
	} while ( cellAtomicStoreConditional32( buf, (const uint64_t)aEA, (uint32_t)(old - incr)));
	EnableInterrupts();

	return old;
}

//! an atomic a=a-1 for int64_t types
inline int64_t AtomicDecrement64( const uint32_t aEA, int64_t incr = 1 )
{
	uint64_t buf[16] __attribute__((aligned(128)));
	int64_t old;
	DisableInterrupts();
	do 
	{
		old = (int64_t) cellAtomicLockLine64(buf,(const uint64_t)aEA);
	} while ( cellAtomicStoreConditional64( buf, (const uint64_t)aEA, (uint64_t)(old - incr)));
	EnableInterrupts();

	return old;
}

//! an atomic a=a-1 for uint64_t types
inline uint64_t AtomicDecrementU64( const uint32_t aEA, uint64_t incr = 1 )
{
	uint64_t buf[16] __attribute__((aligned(128)));
	uint64_t old;
	DisableInterrupts();
	do 
	{
		old = (uint64_t) cellAtomicLockLine64(buf,(const uint64_t)aEA);
	} while ( cellAtomicStoreConditional64( buf, (const uint64_t)aEA, (uint64_t)(old - incr)));
	EnableInterrupts();

	return old;
}

//----
// These two functions are used differntly and are a paired set.
// AtomicLockLine returns a 128 BYTE cache line from memory and locks it
// then AtomicStoreConditionalLine try to write it back, return non-zero if
// it can't.. this allows atomic update of a 128 byte structure as efficently as possible
// NOT INTERUPT SAFE (DO IT YOURSELF OUTSIDE THE do{}while loop
inline void AtomicLockCacheLine( uint32_t aEa, void* pAligned128bytes )
{
#if !defined( _RELEASE )
	// these assert the values are correct tho only through a
	// halt compare (it will just lock up)... the reason it that
	// as this is part of an atomic op it has to be v.fast (even in debug)
	// and race conditions with assert/printf etc. mean halt is safest
	spu_hcmpeq(((uintptr_t)aEa & 0x7f) == 0, 0);
	spu_hcmpeq(((uintptr_t)pAligned128bytes & 0x7f) == 0, 0);
#endif
	mfc_getllar(pAligned128bytes, (const uint64_t)aEa, 0, 0);
	mfc_read_atomic_status();
	spu_dsync();

}

inline int AtomicStoreConditionalCacheLine( uint32_t aEa, void* pAligned128bytes )
{
#if !defined( _RELEASE )
	// these assert the values are correct tho only through a
	// halt compare (it will just lock up)... the reason it that
	// as this is part of an atomic op it has to be v.fast (even in debug)
	// and race conditions with assert/printf etc. mean halt is safest
	spu_hcmpeq(((uintptr_t)aEa & 0x7f) == 0, 0);
	spu_hcmpeq(((uintptr_t)pAligned128bytes & 0x7f) == 0, 0);
#endif

	spu_dsync();
	mfc_putllc(pAligned128bytes, (const uint64_t)aEa, 0, 0);
	return mfc_read_atomic_status();
}

#endif // end CORE_SYNCPRIMS_H
