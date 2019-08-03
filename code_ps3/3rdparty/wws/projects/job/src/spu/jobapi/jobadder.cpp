/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Job API functions for adding jobs to joblists from the SPU side
**/
//--------------------------------------------------------------------------------------------------

#include <jobapi/jobapi.h>
#include <jobapi/jobdefinition.h>
#include <jobapi/jobspudma.h>

#include <cell/spurs/ready_count.h>

//--------------------------------------------------------------------------------------------------

static U64 s_atomicBuffer[16] WWSJOB_ALIGNED( 128 ) WWSJOB_UNINITIALIZED();

//--------------------------------------------------------------------------------------------------

void WwsJob_JobApiSetReadyCount( CellSpursWorkloadId workloadId, U32 readyCount )
{
	WWSJOB_ASSERT( AreInterruptsEnabled() );
	DisableInterrupts();

	int ret = _cellSpursReadyCountSwap( workloadId, readyCount );
	WWSJOB_ASSERT( CELL_OK == ret );
	WWSJOB_UNUSED( ret );

	EnableInterrupts();
}

//--------------------------------------------------------------------------------------------------

void WwsJob_JobApiAddJobToJobList( U32 eaJobList, JobHeader jobHeader )
{
	//:TODO: This function can be optimized to not do as many getllars when it's looping for a zero
	WWSJOB_ASSERT( eaJobList );
	WWSJOB_ASSERT( WwsJob_IsAligned( eaJobList, 128 ) );
	WWSJOB_ASSERT( (jobHeader.m_loadCommandsSize >= 0x10) || (jobHeader.m_jobHeaderCommand == JobHeaderCommand::kGeneralBarrier ) );
	WWSJOB_ASSERT( jobHeader.m_loadCommandsSize <= MAX_LOAD_COMMANDS_SIZE );
	WWSJOB_ASSERT( jobHeader.m_jobHeaderCommand != JobHeaderCommand::kNoJobYet );

	//Is it worth doing a blocking dma pull to get the list header so we know how far in to start?
	U32 numJobsInList = 0;//m_pJobListHeader->m_numJobsInListHint;

	U64 old;
	do
	{
		U32 eaJobHeader = eaJobList + sizeof(JobListHeaderCore) + (numJobsInList * sizeof(jobHeader));
		++numJobsInList;	//Move onto the next job

		U32 eaBase		= eaJobHeader & ~0x7f;
		U32 index		= (eaJobHeader & 0x7f) / sizeof(U64);

		//Let's try and store our new job into the list here
		do
		{
			JobDmaGetllar( s_atomicBuffer, eaBase );
			JobDmaWaitAtomicStatus();

			old		= s_atomicBuffer[index];
			//If there's already a job there, then abort this store and move onto the next slot
			if ( old != 0LL )
				break;

			//No job there yet, so attempt to store our job there
			s_atomicBuffer[index]	= jobHeader.m_u64;

			JobDmaPutllc( s_atomicBuffer, eaBase );
		} while ( JobDmaWaitAtomicStatus() );

		//If old is non-zero then the store aborted.  Another job must already be there,
		// so move onto the next slot and try and store it again.
	} while ( old );

	//WWSJOB_ASSERT( numJobsInList <= m_maxNumElts );

	//This write is optional
	//AtomicNumJobsInListHintSetToMaxAgainstValue( m_pJobListHeader, numJobsInList );

	//return JobListMarker( numJobsInList, this );
}

//--------------------------------------------------------------------------------------------------
