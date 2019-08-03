#if !defined(CORE_SYNCPRIMS_PS3_H)
#define CORE_SYNCPRIMS_PS3_H

#include <cell/atomic.h>
#include <sys/system_types.h>
#include <sys/synchronization.h>

//-----------------------------------------------------
//!
//!	\file core\syncprims_ps3.h
//! platform specific bit for PS3 of sync prims
//!
//-----------------------------------------------------

#define USE_LIGHTWEIGHT_MUTEX

class CriticalSectionPlatform
{
public:
#ifdef USE_LIGHTWEIGHT_MUTEX
	sys_lwmutex_t		m_Crit;
#else
	sys_mutex_t			m_Crit;
#endif
};

class SyncPointPlatform
{
public:
	sys_sync_point_t		m_SyncPoint;
};

class WaitableEventPlatform
{
public:
	sys_cond_t				m_Conditional;
	sys_mutex_t				m_Mutex;
};
//--------------------------------------------------
//!
//! Thread Safe equivilent to 
//! ret = *a
//! *a = b;
//! return ret;
//!
//--------------------------------------------------
inline int32_t AtomicSetPlatform( volatile int32_t* a, const int32_t b )
{
	return (int32_t) cellAtomicStore32( (uint32_t*) a, b );
}

//--------------------------------------------------
//!
//! Thread Safe equivilent to 
//! ret = *a
//! *a = b;
//! return ret;
//!
//--------------------------------------------------
inline uint32_t AtomicSetPlatform( volatile uint32_t* a, const uint32_t b )
{
	return (uint32_t) cellAtomicStore32( (uint32_t*) a, b );
}

//--------------------------------------------------
//!
//! Thread Safe equivilent to 
//! ret = *a
//! *a = b;
//! return ret;
//!
//--------------------------------------------------
inline int64_t AtomicSetPlatform( volatile int64_t* a, const int64_t b )
{
	return (int32_t) cellAtomicStore64( (uint64_t*) a, b );
}

//--------------------------------------------------
//!
//! Thread Safe equivilent to 
//! ret = *a
//! *a = b;
//! return ret;
//!
//--------------------------------------------------
inline uint64_t AtomicSetPlatform( volatile uint64_t* a, const uint64_t b )
{
	return (uint32_t) cellAtomicStore64( (uint64_t*) a, b );
}

//--------------------------------------------------
//!
//! Thread Safe equivilent to 
//! ret = *a
//! *a = *a + 1;
//! return ret;
//!
//--------------------------------------------------
inline int32_t AtomicIncrementPlatform( volatile int32_t* a )
{
	return (int32_t) cellAtomicIncr32( (uint32_t*) a );
}

//--------------------------------------------------
//!
//! Thread Safe equivilent to 
//! ret = *a
//! *a = *a + 1;
//! return ret;
//!
//--------------------------------------------------
inline uint32_t AtomicIncrementPlatform( volatile uint32_t* a )
{
	return (int32_t) cellAtomicIncr32( (uint32_t*) a );
}

//--------------------------------------------------
//!
//! Thread Safe equivilent to 
//! ret = *a
//! *a = *a + 1;
//! return ret;
//!
//--------------------------------------------------
inline int64_t AtomicIncrementPlatform( volatile int64_t* a )
{
	return (int64_t) cellAtomicIncr64( (uint64_t*) a );
}

//--------------------------------------------------
//!
//! Thread Safe equivilent to 
//! ret = *a
//! *a = *a + 1;
//! return ret;
//!
//--------------------------------------------------
inline uint64_t AtomicIncrementPlatform( volatile uint64_t* a )
{
	return (uint64_t) cellAtomicIncr64( (uint64_t*) a );
}

//--------------------------------------------------
//!
//! Thread Safe equivilent to 
//! ret = *a
//! *a = *a - 1;
//! return ret;
//!
//--------------------------------------------------
inline int32_t AtomicDecrementPlatform( volatile int32_t* a )
{
	return (int32_t) cellAtomicDecr32( (uint32_t*) a );
}

//--------------------------------------------------
//!
//! Thread Safe equivilent to 
//! ret = *a
//! *a = *a - 1;
//! return ret;
//!
//--------------------------------------------------
inline uint32_t AtomicDecrementPlatform( volatile uint32_t* a )
{
	return (uint32_t) cellAtomicDecr32( (uint32_t*) a );
}

//--------------------------------------------------
//!
//! Thread Safe equivilent to 
//! ret = *a
//! *a = *a - 1;
//! return ret;
//!
//--------------------------------------------------
inline int64_t AtomicDecrementPlatform( volatile int64_t* a )
{
	return (int64_t) cellAtomicDecr64( (uint64_t*) a );
}

//--------------------------------------------------
//!
//! Thread Safe equivilent to 
//! ret = *a
//! *a = *a - 1;
//! return ret;
//!
//--------------------------------------------------
inline uint64_t AtomicDecrementPlatform( volatile uint64_t* a )
{
	return (uint64_t) cellAtomicDecr64( (uint64_t*) a );
}

#endif // end CORE_SYNCPRIMS_PS3_H
