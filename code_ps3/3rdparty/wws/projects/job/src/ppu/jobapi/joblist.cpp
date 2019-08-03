/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Manage a joblist from the PPU side.
				SingleThreadJobList may only be used if you are sure only a single PPU thread
				will be adding at a time.
				MultiThreadSafeJobList is for use if a joblist may be added to by multiple PPU
				threads at once or by SPUs.
**/
//--------------------------------------------------------------------------------------------------

#include <string.h>

#include <sys/process.h>
#include <cell/atomic.h>
#include <cell/spurs/types.h>
#include <cell/spurs/ready_count.h>
#include <cell/spurs/control.h>
#include <cell/spurs/exception.h>
#include <cell/spurs/workload.h>
#include <cell/spurs/policy_module.h>
#ifndef __SNC__
#include <ppu_intrinsics.h>
#endif

#include <jobapi/joblist.h>
#include <jobapi/jobdefinition.h>
#include <jobapi/audittypes.h>
#include <jobapi/embeddedjm.h>
#include <jobapi/commandlistbuilder.h>
#include <jobapi/commandlistchecker.h>
#include <jobapi/jobprintf.h>
#include <jobapi/auditmanager.h>
#include <jobapi/exceptionhandler.h>


//--------------------------------------------------------------------------------------------------
//
//  The JobListPrivate functions contain all the functionality that is used by both
//	the SingleThreadJobList and MultiThreadSafeJobList classes.
//
//--------------------------------------------------------------------------------------------------

void JobListPrivate::AttachToSpursPrivate( CellSpurs* pSpurs, const U8* workPrios, U32 maxContention, U32 baseAddr, AuditManager* pAuditManager )
{
	//Check that the base address as known by the application is consistent with that known by the pre-compiled library
	if ( baseAddr != LsMemoryLimits::kJobAreaBasePageNum )
	{
		JobBasePrintf( "Application thinks baseAddr is 0x%X, Library thinks it's 0x%X\n", baseAddr, LsMemoryLimits::kJobAreaBasePageNum );
		WWSJOB_ALWAYS_ASSERT( false );
	}

	CellSpursInfo spursInfo;
	int ret = cellSpursGetInfo( pSpurs, &spursInfo );
	WWSJOB_ASSERT( CELL_OK == ret );
	WWSJOB_UNUSED( ret );

	//user should be passing false as last param to cellSpursInitialize
	WWSJOB_ASSERT( spursInfo.exitIfNoWork == false );

	//Can only attach a joblist to one Spurs instance
	WWSJOB_ASSERT( m_pSpurs == NULL );
	m_pSpurs = pSpurs;

	void* workEa		 = GetWQAddr();

#if WWS_JOB_USE_C_VERSION
	const void* pmEa					= EmbeddedJobManager::GetBsBase();
	U32 pmSize							= EmbeddedJobManager::GetBsSize();
#else
	const void* pmEa					= EmbeddedJobManager::GetJmBase();
	U32 pmSize							= EmbeddedJobManager::GetJmSize();
#endif

	//We don't allow inter-SPU communication in the job manager, so min_contention is 1
	U32 minContention	= 1;

	WWSJOB_ASSERT( maxContention >= 1 );
	WWSJOB_ASSERT( maxContention <= 8 );
	m_maxContention = maxContention;

	U64 workloadData;
	if ( pAuditManager )
	{
		workloadData = ((U64)workEa) | (((U64)pAuditManager->GetAuditMemory())<<32L);
	}
	else
	{
		workloadData = (U64)workEa;
	}

	ret = cellSpursAddWorkload( pSpurs,
								&m_workloadId,
								pmEa, pmSize,
								workloadData,
								workPrios,
								minContention, maxContention );
	WWSJOB_ASSERT( CELL_OK == ret );
	WWSJOB_ASSERT( m_workloadId != kInvalidWorkloadId );
	//JobPrintf( "Job Manager workload at EA 0x%X, on Policy Module at ea = 0x%X (PM size = %d) attached to Spurs\n", (U32)workEa, (U32)pmEa, pmSize );

	WWSJOB_ASSERT( m_workloadId < 16 );

	//The spurs scheduling algorithm changed in favour of spurs tasks.  Put it back to what we want for our job model based workloads.
	ret = cellSpursRequestIdleSpu( pSpurs, m_workloadId, 7 );
	WWSJOB_ASSERT( CELL_OK == ret );

	ret = cellSpursSetExceptionEventHandler( pSpurs, m_workloadId, gpWwsJobExceptionEventHandlerCallback, this );
	WWSJOB_ASSERT( CELL_OK == ret );

	//This test needs to be done at some point, so best do it here.  Just make sure the build process is in sync with the code
	WWSJOB_ASSERT_MSG( JOB_MEMORY_BASE_ADDR == NumPagesToNumBytes( LsMemoryLimits::kJobAreaBasePageNum ),
		("Linker version of JOB_MEMORY_BASE_ADDR (0x%X) is inconsistent with LsMemoryLimits::kJobAreaBasePageNum (%d * 1024)",
			JOB_MEMORY_BASE_ADDR, LsMemoryLimits::kJobAreaBasePageNum ) );
}

//--------------------------------------------------------------------------------------------------

void JobListPrivate::SetReadyCount( U32 readyCount )
{
	WWSJOB_ASSERT( readyCount >= 1 );	//The user sets a non-zero ready count to say there exists work to be done.
										//The job manager sets it back to zero when the work is finished.
	WWSJOB_ASSERT( readyCount <= 8 );
	WWSJOB_ASSERT( m_workloadId != kInvalidWorkloadId );
	int ret = cellSpursReadyCountStore( m_pSpurs, m_workloadId, readyCount );
	WWSJOB_UNUSED( ret );
	WWSJOB_ASSERT( CELL_OK == ret );
}

//--------------------------------------------------------------------------------------------------

void JobListPrivate::Shutdown( void )
{
	//Remove the workload and shut down
	int ret = cellSpursShutdownWorkload( m_pSpurs, m_workloadId );
	WWSJOB_ASSERT( CELL_OK == ret );

	ret = cellSpursWaitForWorkloadShutdown( m_pSpurs, m_workloadId );
	WWSJOB_ASSERT( CELL_OK == ret );

	//Remove the workload as soon as it's finished with
	do
	{
		ret = cellSpursRemoveWorkload( m_pSpurs, m_workloadId );
	} while( ret == CELL_SPURS_POLICY_MODULE_ERROR_BUSY );

	WWSJOB_ASSERT( CELL_OK == ret );

	m_pSpurs = NULL;
	m_workloadId = kInvalidWorkloadId;

	m_maxContention = 0xFFFFFFFF;
}

//--------------------------------------------------------------------------------------------------

//static inline
uint32_t AtomicNumJobsInListHintSetToMaxAgainstValue( JobListHeader* pJobListHeader, U32 newVal )
{
	U16* ea = &(pJobListHeader->m_numJobsInListHint);

	WWSJOB_ASSERT( WwsJob_IsAligned( ea, 4 ) );

#if 1
	U32 oldNumJobsInListHint;
	bool storeFail;
	do
	{
		U32 old32 = cellAtomicLockLine32( (U32*)ea );

		oldNumJobsInListHint = old32 >> 16;
		if ( newVal <= oldNumJobsInListHint )
			return oldNumJobsInListHint;

		U32 new32 = (newVal << 16) | (old32 & 0xFFFF);
		storeFail = cellAtomicStoreConditional32( (U32*)ea, new32 );
	} while ( storeFail );

	return oldNumJobsInListHint;
#else
	U64 old32;
	U64 oldValShifted;
	U64 preserve;
	U64 newValShifted;
	U64 new32;
	U32 upperMask = 0xFFFF000;

	__asm__ volatile(
		"	lwsync																	\n"
		".loop%=:																	\n"
		"	lwarx		%[old32],			0,					%[ea]				\n"		//old32 = *ea
		"	clrlsldi	%[newValShifted],	%[newVal],			48,				16	\n"		//newValShifted = newVal << 16
		"	and			%[oldValShifted],	%[old32],			%[upperMask]		\n"		//oldValShifted = old32 & 0xFFFF0000
		"	clrldi		%[preserve],		%[old32],			48					\n"		//preserve = old32 & 0xFFFF
		"	cmplw		%[oldValShifted],	%[newValShifted]						\n"
		"	or			%[new32],			%[newValShifted],	%[preserve]			\n"		//new32 = newValShifted | preserve
		"	bge			.Ldone%=													\n"
		"	stwcx.		%[new32],			0,					%[ea]				\n"
		"	bne-		.loop%=														\n"
		".Ldone%=:																	\n"
		:	[old32]			"=&b"	(old32),
			[oldValShifted]	"=&b"	(oldValShifted),
			[newValShifted]	"=&b"	(newValShifted),
			[preserve]		"=&b"	(preserve),
			[new32]			"=&b"	(new32)
		:	[ea]			"b"		(ea),
			[newVal]		"b"		(newVal),
			[upperMask]		"b"		(upperMask)
		:					"cc",
							"memory"	);

	return (oldValShifted>>16);
#endif
}

//--------------------------------------------------------------------------------------------------

JobListPrivate::JobListPrivate( void )
:	m_pSpurs( NULL ),
	m_pJobListHeader( NULL ),
	m_maxNumElts( 0 ),
	m_maxContention( 0xFFFFFFFF ),
	m_workloadId( kInvalidWorkloadId )
{
}

//--------------------------------------------------------------------------------------------------

JobListPrivate::JobListPrivate( void* pBuffer, U32 bufferSize, const char* name )
:	m_pSpurs( NULL ),
	m_pJobListHeader( NULL ),
	m_maxNumElts( 0 ),
	m_maxContention( 0xFFFFFFFF ),
	m_workloadId( kInvalidWorkloadId )
{
	Init( pBuffer, bufferSize, name );
}

//--------------------------------------------------------------------------------------------------

void JobListPrivate::Init( void* pBuffer, U32 bufferSize, const char* name )
{
	WWSJOB_ASSERT( pBuffer );
	WWSJOB_ASSERT( WwsJob_IsAligned( pBuffer, 128 ) );
	WWSJOB_ASSERT( WwsJob_IsAligned( bufferSize, 128 ) );

	WWSJOB_ASSERT( m_pSpurs == NULL );					//Must only init once
	WWSJOB_ASSERT( m_pJobListHeader == NULL );
	WWSJOB_ASSERT( m_maxNumElts == 0 );
	WWSJOB_ASSERT( m_workloadId == kInvalidWorkloadId );

	WWSJOB_ASSERT( !sys_process_is_stack( pBuffer ) );//Buffer will be read by SPU so mustn't be on stack

	JobListHeader* pJobListHeader		= (JobListHeader*) pBuffer;

	const void* pmEa					= EmbeddedJobManager::GetJmBase();
	U32 pmSize							= EmbeddedJobManager::GetJmSize();;

	pJobListHeader->m_mmaJobManager		= (U32) pmEa;
	pJobListHeader->m_jobManagerSize	= (U32) pmSize;

	m_pJobListHeader					= pJobListHeader;
	m_maxNumElts						= (bufferSize - sizeof(JobListHeaderCore)) / sizeof(JobHeader);

	m_workloadId						= kInvalidWorkloadId;

	m_pSpurs							= NULL;

	if ( name == NULL )
	{
		name = "no name";
	}
	SetName( name );

	ResetList();
}

//--------------------------------------------------------------------------------------------------

void JobListPrivate::SetName( const char* name )
{
	WWSJOB_ASSERT( strlen( name ) < kMaxNameLength );
	strncpy( m_name, name, kMaxNameLength );
}

//--------------------------------------------------------------------------------------------------

void JobListPrivate::ResetList( void )
{
	JobListHeader* pJobListHeader		= m_pJobListHeader;

	pJobListHeader->m_numJobsInListHint	= 0;
	pJobListHeader->m_numJobsTaken		= 0;

	pJobListHeader->m_minActive[0]		= 0xFFFF;
	pJobListHeader->m_minActive[1]		= 0xFFFF;
	pJobListHeader->m_minActive[2]		= 0xFFFF;
	pJobListHeader->m_minActive[3]		= 0xFFFF;
	pJobListHeader->m_minActive[4]		= 0xFFFF;
	pJobListHeader->m_minActive[5]		= 0xFFFF;

#ifdef __SNC__
	//Reset all the elements to zero so that they are kNoJobYet
	JobHeader *pJobHeader = pJobListHeader->m_jobHeader;
	for (U32 i = 0; i < m_maxNumElts; ++i, ++pJobHeader)
		pJobHeader->m_u64 = 0;
#else
	//Now reset all the elements to zero so that they are all kNoJobYet

	//Note that we know that the buffer for the joblist is necessarily
	//128 byte aligned and a multiple of 128 bytes in size

	//Zero out the jobs in first 128 bytes one element at a time
	JobHeader* pJobHeader = pJobListHeader->m_jobHeader;
	for ( U32 i = 0; i < 12; ++i, ++pJobHeader )	//Could be 6 vmx stores
		pJobHeader->m_u64 = 0LL;

	//Then zero the rest of the cache lines out with dcbz
	U32 numElts = m_maxNumElts - 12;
	WWSJOB_ASSERT( WwsJob_IsAligned(numElts, 16) );

	U32 numCacheLines = numElts / 16;

	void* pCacheLine = &pJobListHeader[1];
	WWSJOB_ASSERT( WwsJob_IsAligned(pCacheLine, 128) );

	for ( U32 cacheLineNo = 0; cacheLineNo < numCacheLines; ++cacheLineNo )
	{
		__dcbz(pCacheLine);
		pCacheLine = (void*) ((U32)pCacheLine + 128);
	}
#endif
}

//--------------------------------------------------------------------------------------------------

bool JobListPrivate::IsJobMarkerPassed( U16 jobIndex ) const
{
	//Note that ordering of the reads is important here.
	//The m_numJobsTaken and m_minActive[] could get updated during
	//the course of the reads.  It's important that if anything we
	//get the lowest possible value at the end.
	//
	//Normally these values are just increasing and we're after the
	//minimum, so in this case the order of the reads doesn't matter
	//as we'd never get an inappropriate value.  The min of the old
	//values or the min of the new values or something in between
	//are all fine.
	//
	//The problem case comes when the first SPU allocates a job off
	//of a list.  At this point, the m_numJobsTaken increments but the
	//m_minActive gets changed from 0xFFFF down to the appropriate value.
	//If we did the read in the order m_minActive then m_numJobsTaken,
	//it's possible for the atomic update to happen inbetween those two
	//reads, and as such the minimum calculated would be the new value
	//of m_numJobsTaken.  This is not what we want since the previous
	//job is still active.  Instead, we will read m_numJobsTaken first
	//and then m_minActive.  In this case, if the write does happen
	//in between the reads, then we get the old version of m_numJobsTaken
	//which is acceptable.

	//The proper solution to this would be to do a 128-bit atomic read
	//of all the values in one go.  This *might* be doable via VMX and
	//then a VMX write to a local followed by PowerPC reads from that
	//copy.

	//Another alternative is to calculate the minActive repeatedly and
	//only accept the answer when we get the same answer twice in a row

	//Or we could have the SPUs calculate the parallel minimum of the
	//7 values and store it every time it does the write to the
	//JobListHeader.

	//The following code should be correct for now, but I may change it.

	//ChrisC 18/01/06

	volatile JobListHeader* pJobListHeader = m_pJobListHeader;

	U16 minActive = pJobListHeader->m_numJobsTaken; 

	__sync();

	for ( int spuNum = 0; spuNum < 6; ++spuNum )
	{
		U16 minActiveOnThisSpu = pJobListHeader->m_minActive[spuNum];
		minActive = WwsJob_min( minActiveOnThisSpu, minActive );
	}

	//All jobs before minActive are complete

	return (jobIndex <= minActive);
}

//--------------------------------------------------------------------------------------------------

void JobListPrivate::StallForJobMarker( U16 jobIndex ) const
{
	//Perhaps we can yield this PPU thread?
	//Maybe I should put in a timeout test
	while ( IsJobMarkerPassed( jobIndex ) == false )
	{
	}
}

//--------------------------------------------------------------------------------------------------

void JobListPrivate::SetNewPriorities( const U8* workPrios ) 
{ 
	int ret = cellSpursSetPriorities( m_pSpurs, m_workloadId, workPrios ); 
	WWSJOB_ASSERT( CELL_OK == ret ); 
	WWSJOB_UNUSED( ret ); 
}

//--------------------------------------------------------------------------------------------------

void JobListPrivate::CheckJobList( U32 options ) const
{
	if ( options & kPrintCommands )
	{
		JobPrintf( "Disassemble job list (Job List Header at 0x%X):\n", (U32)m_pJobListHeader );
	}

	const JobHeader* pJobHeader = &m_pJobListHeader->m_jobHeader[0];
 
	while ( pJobHeader->m_jobHeaderCommand != JobHeaderCommand::kNoJobYet )
	{
		CheckJob( *pJobHeader, options, m_maxContention );
		++pJobHeader;
	}
}

//--------------------------------------------------------------------------------------------------
//
//	The SingleThreadJobList class implements the functions which are differently implemented
//	from the MultiThreadSafeJobList class.  If appropriate, this may just call shared functionality
//	in the JobListPrivate class.
//
//--------------------------------------------------------------------------------------------------

SingleThreadJobList::SingleThreadJobList( void  )
:
	m_realNumJobsInList( 0 )
{
}

//--------------------------------------------------------------------------------------------------

SingleThreadJobList::SingleThreadJobList( void* pBuffer, U32 bufferSize, const char* name )
:
	JobListPrivate( pBuffer, bufferSize, name ),
	m_realNumJobsInList( 0 )
{
}

//--------------------------------------------------------------------------------------------------

void SingleThreadJobList::Init( void* pBuffer, U32 bufferSize, const char* name )
{
	WWSJOB_ASSERT( m_realNumJobsInList == 0 );
	JobListPrivate::Init( pBuffer, bufferSize, name );
}

//--------------------------------------------------------------------------------------------------

void SingleThreadJobList::ResetList( void )
{
	WWSJOB_ASSERT(IsListFinished());

	m_realNumJobsInList					= 0;
	JobListPrivate::ResetList();
}

//--------------------------------------------------------------------------------------------------

JobListMarker SingleThreadJobList::AddJob( const void* pJob, U16 jobSize )
{
	JobHeader jobHeader;
	jobHeader.m_mmaLoadCommands		= (U32) pJob;
	jobHeader.m_loadCommandsSize	= jobSize;
	jobHeader.m_enableBreakpoint	= 0;
	jobHeader.m_jobHeaderCommand	= JobHeaderCommand::kJobExists;

	//Note, must be written as a single 64-bit (ie. atomic) write.
	return AddJob( jobHeader );
}

//--------------------------------------------------------------------------------------------------

JobListMarker SingleThreadJobList::AddJobWithBreakpoint( const void* pJob, U16 jobSize )
{
	JobHeader jobHeader;
	jobHeader.m_mmaLoadCommands		= (U32) pJob;
	jobHeader.m_loadCommandsSize	= jobSize;
	jobHeader.m_enableBreakpoint	= 1;
	jobHeader.m_jobHeaderCommand	= JobHeaderCommand::kJobExists;

	//Note, must be written as a single 64-bit (ie. atomic) write.
	return AddJob( jobHeader );
}

//--------------------------------------------------------------------------------------------------

inline JobListMarker SingleThreadJobList::AddJobInternal( JobHeader jobHeader )
{
	//After all error checking etc... this is the function that actually stores the job into the list
	WWSJOB_ASSERT( m_realNumJobsInList < m_maxNumElts );

	JobListHeader* pJobListHeader = m_pJobListHeader;

	U32 numJobsInList = m_realNumJobsInList;
	JobHeader* pJobHeader = &pJobListHeader->m_jobHeader[numJobsInList];
	++numJobsInList;
	m_realNumJobsInList = numJobsInList;

	WWSJOB_ASSERT( pJobHeader->m_jobHeaderCommand == JobHeaderCommand::kNoJobYet );

	//Note, must be written as a single 64-bit (ie. atomic) write.
	*pJobHeader = jobHeader;

	//This write is optional
	pJobListHeader->m_numJobsInListHint = numJobsInList;

	return JobListMarker( m_realNumJobsInList, this );
}

//--------------------------------------------------------------------------------------------------

JobListMarker SingleThreadJobList::AddJob( JobHeader jobHeader )
{
	WWSJOB_ASSERT( (jobHeader.m_loadCommandsSize >= 0x10) || (jobHeader.m_jobHeaderCommand == JobHeaderCommand::kGeneralBarrier ) );
	WWSJOB_ASSERT( jobHeader.m_loadCommandsSize <= MAX_LOAD_COMMANDS_SIZE );

	return AddJobInternal( jobHeader );
}

//--------------------------------------------------------------------------------------------------

JobListMarker SingleThreadJobList::AddJobWithBreakpoint( JobHeader jobHeader )
{
	jobHeader.m_enableBreakpoint	= 1;

	//Note, must be written as a single 64-bit (ie. atomic) write.
	return AddJob( jobHeader );
}

//--------------------------------------------------------------------------------------------------

JobListMarker SingleThreadJobList::AddJobAndKick( const void* pJob, U16 jobSize, U32 readyCount )
{
	JobListMarker marker = AddJob( pJob, jobSize );
	SetReadyCount( readyCount );
	return marker;
}

//--------------------------------------------------------------------------------------------------

JobListMarker SingleThreadJobList::AddJobAndKickWithBreakpoint( const void* pJob, U16 jobSize, U32 readyCount )
{
	JobListMarker marker = AddJobWithBreakpoint( pJob, jobSize );
	SetReadyCount( readyCount );
	return marker;
}

//--------------------------------------------------------------------------------------------------

JobListMarker SingleThreadJobList::AddJobAndKick( JobHeader job, U32 readyCount )
{
	JobListMarker marker = AddJob( job );
	SetReadyCount( readyCount );
	return marker;
}

//--------------------------------------------------------------------------------------------------

JobListMarker SingleThreadJobList::AddJobAndKickWithBreakpoint( JobHeader job, U32 readyCount )
{
	JobListMarker marker = AddJobWithBreakpoint( job );
	SetReadyCount( readyCount );
	return marker;
}

//--------------------------------------------------------------------------------------------------

JobListMarker SingleThreadJobList::AddGeneralBarrier( const DependencyCounter* pBarrier )
{
	WWSJOB_ASSERT( pBarrier->m_workloadId == GetWorkloadId() );

	JobHeader jobHeader;
	jobHeader.m_mmaLoadCommands			= (U32) pBarrier;
	jobHeader.m_loadCommandsSize		= 0;
	jobHeader.m_enableBreakpoint		= 0;
	jobHeader.m_jobHeaderCommand		= JobHeaderCommand::kGeneralBarrier;

	return AddJob( jobHeader );
}

//--------------------------------------------------------------------------------------------------

JobListMarker SingleThreadJobList::AddGeneralBarrierAndKick( const DependencyCounter* pBarrier, U32 readyCount )
{
	JobListMarker marker = AddGeneralBarrier( pBarrier );
	SetReadyCount( readyCount );
	return marker;
}

//--------------------------------------------------------------------------------------------------

JobListMarker SingleThreadJobList::AddNopJob( void )
{
	JobHeader jobHeader;
	jobHeader.m_mmaLoadCommands			= NULL;
	jobHeader.m_loadCommandsSize		= 0;
	jobHeader.m_enableBreakpoint		= 0;
	jobHeader.m_jobHeaderCommand		= JobHeaderCommand::kNopJob;

	return AddJobInternal( jobHeader );
}

//--------------------------------------------------------------------------------------------------

JobHeader* SingleThreadJobList::InsertHole( U32 numJobs )
{
	JobHeader* pJobHeader = GetCurrentJobHeaderPtr();

	for ( U32 i = 0; i < numJobs; ++i )
	{
		AddNopJob();
	}

	return pJobHeader;
}

//--------------------------------------------------------------------------------------------------

JobListMarker SingleThreadJobList::CalculateJobListMarker( void ) const
{
	return JobListMarker( GetCurrentNumJobs(), this );
}

//--------------------------------------------------------------------------------------------------

bool SingleThreadJobList::IsListFinished( void ) const
{
	JobListMarker endOfListMarker = CalculateJobListMarker();

	return endOfListMarker.IsJobMarkerPassed();
}

//--------------------------------------------------------------------------------------------------

void SingleThreadJobList::WaitForJobListEnd( void ) const
{
	JobListMarker endOfListMarker = CalculateJobListMarker();

	while ( endOfListMarker.IsJobMarkerPassed() == false )
	{
	}
}

//--------------------------------------------------------------------------------------------------

void SingleThreadJobList::ForcedFinish( void )	//Intended for debug purposes only
{
	JobListHeader* pJobListHeader		= m_pJobListHeader;

	U32 numJobs = GetCurrentNumJobs();

	pJobListHeader->m_numJobsInListHint	= numJobs;
	pJobListHeader->m_numJobsTaken		= numJobs;

	pJobListHeader->m_minActive[0]		= 0xFFFF;
	pJobListHeader->m_minActive[1]		= 0xFFFF;
	pJobListHeader->m_minActive[2]		= 0xFFFF;
	pJobListHeader->m_minActive[3]		= 0xFFFF;
	pJobListHeader->m_minActive[4]		= 0xFFFF;
	pJobListHeader->m_minActive[5]		= 0xFFFF;
}

//--------------------------------------------------------------------------------------------------
//
//	The MultiThreadSafeJobList class implements the functions which are differently implemented
//	from the SingleThreadJobList class.  If appropriate, this may just call shared functionality
//	in the JobListPrivate class.
//
//--------------------------------------------------------------------------------------------------

void MultiThreadSafeJobList::ResetList( void )
{
	WWSJOB_ASSERT(IsListFinished());
	JobListPrivate::ResetList();
}

//--------------------------------------------------------------------------------------------------

JobListMarker MultiThreadSafeJobList::AddJob( const void* pJob, U16 jobSize )
{
	JobHeader jobHeader;
	jobHeader.m_mmaLoadCommands			= (U32) pJob;
	jobHeader.m_loadCommandsSize		= jobSize;
	jobHeader.m_enableBreakpoint		= 0;
	jobHeader.m_jobHeaderCommand		= JobHeaderCommand::kJobExists;

	return AddJob( jobHeader );
}

//--------------------------------------------------------------------------------------------------

JobListMarker MultiThreadSafeJobList::AddJobWithBreakpoint( const void* pJob, U16 jobSize )
{
	JobHeader jobHeader;
	jobHeader.m_mmaLoadCommands			= (U32) pJob;
	jobHeader.m_loadCommandsSize		= jobSize;
	jobHeader.m_enableBreakpoint		= 1;
	jobHeader.m_jobHeaderCommand		= JobHeaderCommand::kJobExists;

	return AddJob( jobHeader );
}

//--------------------------------------------------------------------------------------------------

inline JobListMarker MultiThreadSafeJobList::AddJobInternal( JobHeader jobHeader )
{
	//After all error checking etc... this is the function that actually stores the job into the list

	union
	{
		JobHeader m_jobHeader;
		U64 m_jobHeaderAsU64;
	} data;
	data.m_jobHeader	= jobHeader;

	JobListHeader* pJobListHeader = m_pJobListHeader;
	U32 numJobsInList = pJobListHeader->m_numJobsInListHint;
	U64 old;
	do
	{
		JobHeader* pJobHeader = &pJobListHeader->m_jobHeader[numJobsInList];
		++numJobsInList;	//Move onto the next job

		old = cellAtomicCompareAndSwap64( (uint64_t*)pJobHeader, 0LL, data.m_jobHeaderAsU64 );

		//If old is non-zero then the store aborted.  Another job must already be there,
		// so move onto the next slot and try and store it again.
	} while ( old );

	WWSJOB_ASSERT( numJobsInList <= m_maxNumElts );

	//This write is optional
	AtomicNumJobsInListHintSetToMaxAgainstValue( pJobListHeader, numJobsInList );

	return JobListMarker( numJobsInList, this );
}

//--------------------------------------------------------------------------------------------------

JobListMarker MultiThreadSafeJobList::AddJob( JobHeader jobHeader )
{
	WWSJOB_ASSERT( (jobHeader.m_loadCommandsSize >= 0x10) || (jobHeader.m_jobHeaderCommand == JobHeaderCommand::kGeneralBarrier ) );
	WWSJOB_ASSERT( jobHeader.m_loadCommandsSize <= MAX_LOAD_COMMANDS_SIZE );

	return AddJobInternal( jobHeader );
}

//--------------------------------------------------------------------------------------------------

JobListMarker MultiThreadSafeJobList::AddJobWithBreakpoint( JobHeader jobHeader )
{
	jobHeader.m_enableBreakpoint		= 1;
	return AddJob( jobHeader );
}

//--------------------------------------------------------------------------------------------------

JobListMarker MultiThreadSafeJobList::AddJobAndKick( const void* pJob, U16 jobSize, U32 readyCount )
{
	JobListMarker marker = AddJob( pJob, jobSize );
	SetReadyCount( readyCount );
	return marker;
}

//--------------------------------------------------------------------------------------------------

JobListMarker MultiThreadSafeJobList::AddJobAndKickWithBreakpoint( const void* pJob, U16 jobSize, U32 readyCount )
{
	JobListMarker marker = AddJobWithBreakpoint( pJob, jobSize );
	SetReadyCount( readyCount );
	return marker;
}

//--------------------------------------------------------------------------------------------------

JobListMarker MultiThreadSafeJobList::AddJobAndKick( JobHeader job, U32 readyCount )
{
	JobListMarker marker = AddJob( job );
	SetReadyCount( readyCount );
	return marker;
}

//--------------------------------------------------------------------------------------------------

JobListMarker MultiThreadSafeJobList::AddJobAndKickWithBreakpoint( JobHeader job, U32 readyCount )
{
	JobListMarker marker = AddJobWithBreakpoint( job );
	SetReadyCount( readyCount );
	return marker;
}

//--------------------------------------------------------------------------------------------------

JobListMarker MultiThreadSafeJobList::AddGeneralBarrier( const DependencyCounter* pBarrier )
{
	WWSJOB_ASSERT( pBarrier->m_workloadId == GetWorkloadId() );

	JobHeader jobHeader;
	jobHeader.m_mmaLoadCommands			= (U32) pBarrier;
	jobHeader.m_loadCommandsSize		= 0;
	jobHeader.m_enableBreakpoint		= 0;
	jobHeader.m_jobHeaderCommand		= JobHeaderCommand::kGeneralBarrier;

	return AddJob( jobHeader );
}

//--------------------------------------------------------------------------------------------------

JobListMarker MultiThreadSafeJobList::AddGeneralBarrierAndKick( const DependencyCounter* pBarrier, U32 readyCount )
{
	JobListMarker marker = AddGeneralBarrier( pBarrier );
	SetReadyCount( readyCount );
	return marker;
}

//--------------------------------------------------------------------------------------------------

JobListMarker MultiThreadSafeJobList::AddNopJob( void )
{
	JobHeader jobHeader;
	jobHeader.m_mmaLoadCommands			= NULL;
	jobHeader.m_loadCommandsSize		= 0;
	jobHeader.m_enableBreakpoint		= 0;
	jobHeader.m_jobHeaderCommand		= JobHeaderCommand::kNopJob;

	return AddJobInternal( jobHeader );
}

//--------------------------------------------------------------------------------------------------

//Since this is an intricate function to use correctly with MultiThreadSafeJobLists,
//it is currently disabled until a user actually requests it.
//Note that in the current implementation there is no way to give any guarantees about the
//contiguousness of the buffer allocated by this function in the case of MultiThreadSafeJobLists
//if other threads might be adding at the same time.
//Since the space may be dis-contiguous at allocation time, at patching time, this must be done
//with an atomic compare swap of the nop job that should be there from allocation time.
//If the nop job has perhaps been filled in by another thread, the joblist patcher must scan on
//to find then next job to attempt to patch
//etc...
/*JobHeader* MultiThreadSafeJobList::InsertHole( U32 numJobs )
{
	U16 firstNopJob = 0xFFFF;
	for ( U32 i = 0; i < numJobs; ++i )
	{
		JobListMarker marker = AddNopJob();
		firstNopJob = WwsJob_min( firstNopJob, marker.m_jobIndex );
	}
	WWSJOB_ASSERT( firstNopJob < 0xFFFF );

	JobHeader* pJobHeader = &m_pJobListHeader->m_jobHeader[firstNopJob-1];

	return pJobHeader;
}*/

//--------------------------------------------------------------------------------------------------

U32 MultiThreadSafeJobList::GetCurrentNumJobs( void ) const
{
	JobListHeader* pJobListHeader = m_pJobListHeader;
	U32 numJobsInList = pJobListHeader->m_numJobsInListHint;

	while ( true )
	{
		JobHeader* pJobHeader = &pJobListHeader->m_jobHeader[numJobsInList];

		if ( pJobHeader->m_jobHeaderCommand == JobHeaderCommand::kNoJobYet )
		{
			//this is the first non-allocated job

			//Optionally, we could update the m_numJobsInListHint
			//AtomicNumJobsInListHintSetToMaxAgainstValue( pJobListHeader, numJobsInList );

			return numJobsInList;
		}

		++numJobsInList;
	}
}

//--------------------------------------------------------------------------------------------------

JobListMarker MultiThreadSafeJobList::CalculateJobListMarker( void ) const
{
	return JobListMarker( GetCurrentNumJobs(), this );
}

//--------------------------------------------------------------------------------------------------

bool MultiThreadSafeJobList::IsListFinished( void ) const
{
	JobListMarker endOfListMarker = CalculateJobListMarker();

	return endOfListMarker.IsJobMarkerPassed();
}

//--------------------------------------------------------------------------------------------------

void MultiThreadSafeJobList::WaitForJobListEnd( void ) const
{
	JobListMarker checkEndOfListMarker;
	JobListMarker newEndOfListMarker = CalculateJobListMarker();

	do
	{
		checkEndOfListMarker = newEndOfListMarker;
		while ( checkEndOfListMarker.IsJobMarkerPassed() == false )
		{
		}

		//It's possible this list might have added more jobs to itself, say,
		//so re-check that the end marker hasn't moved

		newEndOfListMarker = CalculateJobListMarker();
	} while ( checkEndOfListMarker != newEndOfListMarker );
}

//--------------------------------------------------------------------------------------------------

void MultiThreadSafeJobList::ForcedFinish( void )	//Intended for debug purposes only
{
	JobListHeader* pJobListHeader		= m_pJobListHeader;

	U32 numJobs = GetCurrentNumJobs();

	pJobListHeader->m_numJobsInListHint	= numJobs;
	pJobListHeader->m_numJobsTaken		= numJobs;

	pJobListHeader->m_minActive[0]		= 0xFFFF;
	pJobListHeader->m_minActive[1]		= 0xFFFF;
	pJobListHeader->m_minActive[2]		= 0xFFFF;
	pJobListHeader->m_minActive[3]		= 0xFFFF;
	pJobListHeader->m_minActive[4]		= 0xFFFF;
	pJobListHeader->m_minActive[5]		= 0xFFFF;
}

//--------------------------------------------------------------------------------------------------
