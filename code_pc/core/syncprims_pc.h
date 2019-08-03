#if !defined(CORE_SYNCPRIMS_PC_H)
#define CORE_SYNCPRIMS_PC_H
//-----------------------------------------------------
//!
//!	\file core\syncprims_pc.h
//! platform specific bit for PC of sync prims
//!
//-----------------------------------------------------

//! platform specifc bit of a critical section
class CriticalSectionPlatform
{
public:
	CRITICAL_SECTION	m_Crit;
};

//! platform specific bit of a sync point
class SyncPointPlatform
{
public:
	HANDLE				m_Event;
};

//! platform specific bit of a Waitable Event
class WaitableEventPlatform
{
public:
	HANDLE				m_Event;
};

// internal platform specific versions of the atomic ops (so we get inline on PS3)
int32_t AtomicSetPlatform( volatile int32_t* a, const int32_t b );
uint32_t AtomicSetPlatform( volatile uint32_t* a, const uint32_t b );
int64_t AtomicSetPlatform( volatile int64_t* a, const int64_t b );
uint64_t AtomicSetPlatform( volatile uint64_t* a, const uint64_t b );
int32_t AtomicIncrementPlatform( volatile int32_t* a );
uint32_t AtomicIncrementPlatform( volatile uint32_t* a );
int64_t AtomicIncrementPlatform( volatile int64_t* a );
uint64_t AtomicIncrementPlatform( volatile uint64_t* a );
int32_t AtomicDecrementPlatform( volatile int32_t* a );
uint32_t AtomicDecrementPlatform( volatile uint32_t* a );
int64_t AtomicDecrementPlatform( volatile int64_t* a );
uint64_t AtomicDecrementPlatform( volatile uint64_t* a );

#endif // end CORE_SYNCPRIMS_PC_H