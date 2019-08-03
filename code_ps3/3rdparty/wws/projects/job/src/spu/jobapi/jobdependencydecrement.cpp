/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Job API function for doing an immediate dependency decrement from within an SPU job
**/
//--------------------------------------------------------------------------------------------------

#include <jobapi/jobapi.h>
#include <jobapi/jobdefinition.h>
#include <jobapi/jobspudma.h>

#include <cell/spurs/ready_count.h>

//--------------------------------------------------------------------------------------------------

U16 WwsJob_JobApiDecrementDependencyImmediate( U32 eaSpurs, U32 eaDependencyCounter, U32 decrementVal )
{
	WWSJOB_UNUSED( eaSpurs );
	WWSJOB_ASSERT( decrementVal >= 1 );
	WWSJOB_ASSERT( eaDependencyCounter );
	WWSJOB_ASSERT( WwsJob_IsAligned( eaDependencyCounter, 4 ) );

	U32 cacheLineAddr	= eaDependencyCounter & 0xFFFFFF80;
	U32 cacheLineOffset = eaDependencyCounter & 0x7F;
	U32 newVal;
	U16 oldVal;

	void* pAtomicBuffer = (void*)CELL_SPURS_LOCK_LINE;

	DependencyCounter* pCounterAddrInLs = (DependencyCounter*)(CELL_SPURS_LOCK_LINE + cacheLineOffset);

	while ( true )
	{
		JobDmaGetllar( pAtomicBuffer, cacheLineAddr );
		JobDmaWaitAtomicStatus();

		oldVal = pCounterAddrInLs->m_counter;

		WWSJOB_ASSERT( oldVal >= decrementVal );
		newVal = oldVal - decrementVal;

		pCounterAddrInLs->m_counter = newVal;

		JobDmaPutllc( pAtomicBuffer, cacheLineAddr );
		U32 putLlcSuccess = JobDmaWaitAtomicStatus();

		if ( (putLlcSuccess&1) == 0 )
			break;
	}

	//If this write has just satisfied the dependency, then apply the ready count
	if ( newVal == 0 )	//ie. we've just decremented to zero
	{
		//g_countToNextPoll = 0;	//:NOTE: This isn't currently reset by this jobapi call,
									// so there could be a slight pause if this SPU was meant
									// to have caused itself to be pre-empted

		U8 workloadId = pCounterAddrInLs->m_workloadId;
		U8 readyCount = pCounterAddrInLs->m_readyCount;

		//Then apply the ready count to the workload
		int ret = cellSpursReadyCountStore(	NULL,
											NULL,
											workloadId,
											readyCount );
		//Actually I only really want this to set the workload higher, never lower
		WWSJOB_ASSERT( CELL_OK == ret );
		WWSJOB_UNUSED( ret );
	}

	return oldVal;
}

//--------------------------------------------------------------------------------------------------
