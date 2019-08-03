/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Allocate a job off of a job list to this SPU
**/
//--------------------------------------------------------------------------------------------------

#include <cell/dma.h>
#include <cell/spurs/types.h>
#include <cell/spurs/policy_module.h>
#include <cell/spurs/ready_count.h>
#include <spu_mfcio.h>
#include <stddef.h>

#include <jobapi/jobdefinition.h>
#include <jobapi/audittypes.h>
#include <jobapi/jobapi.h>
#include <jobapi/spumoduleheader.h>
#include <jobapi/jobdefinition.h>
#include <jobmanager/interrupthandler.h>
#include <jobmanager/allocatejob.h>
#include <jobmanager/auditwriter.h>
#include <jobmanager/jobmanagerdmatags.h>
#include <jobmanager/spustack.h>
#include <jobmanager/jobheadercache.h>
#include <jobmanager/jobmanager.h>
#include <jobmanager/data.h>

//--------------------------------------------------------------------------------------------------
#if WWS_JOB_USE_C_VERSION!=0
void DecrementDependency( U32 mmaDependencyCounter )
{
	//JobPrintf( "DecrementDependency:\n" );
	U32 cacheLineAddr	= mmaDependencyCounter & 0xFFFFFF80;
	U32 depQwordIdx		= ((mmaDependencyCounter >> 4) & 0x7);
	U32 depWordIdx		= (mmaDependencyCounter & 0xF) >> 2;
	U16 oldVal;

	while ( true )
	{
		DMA_GETLLAR( g_tempUsageAtomicBuffer.m_depCounters, cacheLineAddr );
		DMA_WAITATOMICSTATUS();

		oldVal = g_tempUsageAtomicBuffer.m_depCounters[depQwordIdx][depWordIdx].m_counter;

		WWSJOB_ASSERT( oldVal > 0 );

		g_tempUsageAtomicBuffer.m_depCounters[depQwordIdx][depWordIdx].m_counter = oldVal - 1;

		DMA_PUTLLC( g_tempUsageAtomicBuffer.m_depCounters, cacheLineAddr );
		U32 putLlcSuccess = DMA_WAITATOMICSTATUS();

		if ( (putLlcSuccess&1) == 0 )
			break;
	}

	//If this write has just satisfied the dependency, then apply the ready count
	if ( oldVal == 1 )	//ie. we'll have decremented to zero
	{
		//invalidate our count to next poll to make sure we poll next time around
		//as this change might effect it
		g_countToNextPoll = 0;

		U8 workloadId = g_tempUsageAtomicBuffer.m_depCounters[depQwordIdx][depWordIdx].m_workloadId;
		U8 readyCount = g_tempUsageAtomicBuffer.m_depCounters[depQwordIdx][depWordIdx].m_readyCount;

		//Then apply the ready count to the workload
		int ret = _cellSpursReadyCountSwap(	workloadId,
											readyCount );
		//Actually I only really want this to set the workload higher, never lower
		WWSJOB_VERBOSE_ASSERT( CELL_OK == ret );
		WWSJOB_UNUSED( ret );
	}
}
#endif

//--------------------------------------------------------------------------------------------------

#if WWS_JOB_USE_C_VERSION!=0
extern "C" SpuJobHeader WwsJob_AllocateJob( U32 eaWorkload, U32 spuNum )
{
	//JobPrintf( "AllocateJob:\n" );

	SpuJobHeader spuJobHeader;

	do
	{
		while ( true )
		{
			//Get an atomic reservation on our copy of the gJobListHeader
			DMA_GETLLAR( &g_tempUsageAtomicBuffer.m_jobListHeader, eaWorkload );
			DMA_WAITATOMICSTATUS();

			//JobPrintf( "m_numJobsTaken = %d\n", gJobListHeader.m_numJobsTaken );
			//JobPrintf( "numJobsInListHint = %d\n", gJobListHeader.m_numJobsInListHint );

			//Find out which job we're after
			U16 jobIndex = g_tempUsageAtomicBuffer.m_jobListHeader.m_numJobsTaken;

			//What's the data in this job header?  Get it from the job header cache
			spuJobHeader.m_jobHeader	= GetJobHeaderFromCache( eaWorkload + sizeof(JobListHeaderCore) + jobIndex*8 );//gJobListHeader.m_jobHeader[jobIndex].jobHeaderData;
			spuJobHeader.m_jobIndex		= jobIndex;
			//JobPrintf( "JOB%d: m_mmaLoadCommands = 0x%X (size = 0x%X)\n", jobIndex, jobHeader.m_mmaLoadCommands, jobHeader.m_loadCommandsSize );

			switch ( spuJobHeader.m_jobHeader.m_jobHeaderCommand )
			{
			case JobHeaderCommand::kGeneralBarrier:
				U32 mmaDependencyCounter = spuJobHeader.m_jobHeader.m_mmaLoadCommands;
				WWSJOB_ASSERT( mmaDependencyCounter );

				U32 qwordAddr	= mmaDependencyCounter & 0xFFFFFFF0;
				U32 depWordIdx	= (mmaDependencyCounter & 0xF) >> 2;

				//If we've already fetched this one
				if ( mmaDependencyCounter == g_currentDependencyCacheEa )
				{
					//If the dependency was unsatisfied, the cache would have been cleared, so we wouldn't be inside this if.
					//Therefore, we already know the dependency was satisfied and don't have to check it :).
					WWSJOB_VERBOSE_ASSERT( g_dependencyCache[depWordIdx].m_counter == 0 );
					
					//Then go with it
					//This is useful, since given how long it takes to pull the test, we could lose our atomic reservation
					break;
				}

				//Fetch the dependency and test it.
				DMA_READ( &g_dependencyCache, qwordAddr, 16, DmaTagId::kBlockingLoad );
				g_currentDependencyCacheEa = mmaDependencyCounter;
				cellDmaWaitTagStatusAll( 1 << DmaTagId::kBlockingLoad );

				if( g_dependencyCache[depWordIdx].m_counter == 0 )
					//Then we can allocate this job to ourselves and move onto the next one
					break;

				//JobPrintf( "AllocateJob: dependency not satisfied\n" );
				//This dependency isn't satisfied so give up on this workload
				spuJobHeader.m_jobHeader.m_jobHeaderCommand = JobHeaderCommand::kNoJobYet;
				/* intentional fall-through */
			case JobHeaderCommand::kNoJobYet:
				//JobPrintf( "AllocateJob: No job yet\n" );
				//It's important that the second scan of the job list during workload shutdown checks for a job on an empty stomach
				//Therefore, if we get told that there is no job available yet, then clear the job header cache here.
				g_currentJobHeaderCacheEa = 0;
				goto exit;	//return kNoJobYet since no jobs exist right now

			case JobHeaderCommand::kJobExists:
				//Make sure the minActive knows we're working on at least this job.
				//(We may already have been working on previous jobs on this list anyway.)
				//Note that, if the currently being allocated job is a barrier, we don't update the minActive
				// since we're not actually "active" on this job, because it's already finished via the act of allocation
				g_tempUsageAtomicBuffer.m_jobListHeader.m_minActive[spuNum]	= WwsJob_min( jobIndex, g_tempUsageAtomicBuffer.m_jobListHeader.m_minActive[spuNum] );
				break;

			case JobHeaderCommand::kNopJob:
				//Job is allocated and immediately finished.
				//Note that we don't update the minActive for a nop job.
				break;

			default:
				WWSJOB_ASSERT( false );
			}

			//This is a valid job or barrier to allocate

			//Allocate the job to ourselves
			++jobIndex;

			//And store that fact back into the job list header
			g_tempUsageAtomicBuffer.m_jobListHeader.m_numJobsTaken		= jobIndex;

			//Update the numJobsInListHint
			g_tempUsageAtomicBuffer.m_jobListHeader.m_numJobsInListHint = WwsJob_max( g_tempUsageAtomicBuffer.m_jobListHeader.m_numJobsInListHint, jobIndex );

			DMA_PUTLLC( &g_tempUsageAtomicBuffer.m_jobListHeader, eaWorkload );
			U32 putLlcSuccess = DMA_WAITATOMICSTATUS();

			if ( (putLlcSuccess&1) == 0 )
				break;
		}

		//If the job we just allocated was a kGeneralBarrier, then the outer code isn't interested
		//in this, so immediately re-allocate the next job and return that instead.
	} while ( spuJobHeader.m_jobHeader.m_jobHeaderCommand != JobHeaderCommand::kJobExists );

	//JobPrintf( "AllocateJob: job allocated\n" );

	//JobPrintf( "AllocateJob: Job%d (m_mmaLoadCommands = 0x%X, size = 0x%X)\n", (U32)spuJobHeader.m_jobIndex, spuJobHeader.m_jobHeader.m_mmaLoadCommands, spuJobHeader.m_jobHeader.m_loadCommandsSize );

exit:
	//Note: We must clear the dependency cache regardless of which exit path we take from this funciton
	//since it's possible we used the dependency cache to pass a barrier, but the very next job was taken by another SPU,
	//so we might have been exiting this function via kNoJobYet, say.

	//Clear out the satisfied dependency cache because there's a ridiculous potential code path
	//involving succeeding a dependency on list A, switching to list B (which doesn't use
	//dependencies), meanwhile the PPU resets list A, the SPU switches back to list A and tests
	//this dependency again, and *believes* it to be passed due to this cache.
	//No, I don't actually believe this is ever likely to happen, but best to protect against it
	//just in case.
	g_currentDependencyCacheEa = 0;

	return spuJobHeader;	// return job header (if a job is available then we've allocated it)
}
#endif

//--------------------------------------------------------------------------------------------------
