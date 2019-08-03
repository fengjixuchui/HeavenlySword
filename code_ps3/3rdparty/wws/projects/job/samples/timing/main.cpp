/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		This sample shows how to do timings
**/
//--------------------------------------------------------------------------------------------------

#include <string.h>
#include <stdio.h>

#include <sys/spu_initialize.h>
#include <sys/sys_time.h>
#include <sys/ppu_thread.h>

#include <cell/sysmodule.h>
#include <cell/spurs/types.h>
#include <cell/spurs/control.h>

#include <jobapi/joblist.h>
#include <jobapi/spumodule.h>
#include <jobapi/commandlistbuilder.h>
#include <jobapi/auditmanager.h>
#include <jobapi/audittypes.h>
#include <jobapi/jobmanagerauditids.h>
#include <jobapi/eventhandler.h>

#include "job1/job.h"
#include "job2/job.h"
#include "job3/job.h"
#include "jobx/job.h"


//--------------------------------------------------------------------------------------------------

#define AUDIT_DATA( kEnumName, kString )        kString , 
const char* const g_timingJobText[] =
{
	"kTimingJob1_auditsBegin",
	#include "job1/jobdata.inc"
	"kTimingJob1_auditsEnd",

	"kTimingJob2_auditsBegin",
	#include "job2/jobdata.inc"
	"kTimingJob2_auditsEnd",

	"kTimingJob3_auditsBegin",
	#include "job3/jobdata.inc"
	"kTimingJob3_auditsEnd",

	"kTimingJobx_auditsBegin",
	#include "jobx/jobdata.inc"
	"kTimingJobx_auditsEnd",

};
#undef AUDIT_DATA 

//--------------------------------------------------------------------------------------------------

enum
{
	WwsJob_kAuditTicksAssignedToUninitialized, // ignore ticks for this mode
	WwsJob_kAuditTicksAssignedToSpursKernel,
	WwsJob_kAuditTicksAssignedToWwsJob,
	WwsJob_kAuditTicksAssignedToJob,

	WwsJob_kNumAuditTicksAssignmentTypes
};

static const char *s_auditTicksTitle[] = { "Unitialized", "SpursKernel (polling only)", "JobManager", "Jobs" };

struct AuditTickTracker
{
	U32		m_auditTicksAssignedTo;
	U32		m_auditTicksAssignedToPushed;
	U64		m_auditTicksAssigned[WwsJob_kNumAuditTicksAssignmentTypes];
	U32		m_auditTicksLast;
	U32		m_auditTicksNumJobs;

	U32		m_clocksPerTick;

			AuditTickTracker();
	F32		GetAverageNumCycles( U32 assignmentType ) const;
};

AuditTickTracker::AuditTickTracker()
{
	m_auditTicksAssignedTo			= WwsJob_kAuditTicksAssignedToUninitialized;
	m_auditTicksAssignedToPushed	= WwsJob_kAuditTicksAssignedToUninitialized;
	m_auditTicksLast				= 0;
	m_auditTicksNumJobs				= 0;

	m_clocksPerTick					= (U32)( 3200000000LL / sys_time_get_timebase_frequency() );

	for ( U32 assignmentType = 0; assignmentType < WwsJob_kNumAuditTicksAssignmentTypes; ++assignmentType )
	{
		m_auditTicksAssigned[assignmentType] = 0;
	}
}

F32 AuditTickTracker::GetAverageNumCycles( U32 assignmentType ) const
{
	return (F32)(m_auditTicksAssigned[assignmentType] * m_clocksPerTick) / m_auditTicksNumJobs;
}


// Note this function must match the AuditManager::ProcessAuditFunction typedef
bool AssignAuditTicks( const AuditManager* pAuditManager, U32 spuNum, U16 id, U32 time, bool hwordIsValid, U16 hData, U16 numDwords, const void* pAuditData, void* pUserData )
{
	AuditTickTracker* pTimingTracker = (AuditTickTracker*) pUserData;

	WWSJOB_UNUSED( pAuditManager );
	WWSJOB_UNUSED( spuNum );
	WWSJOB_UNUSED( hwordIsValid );
	WWSJOB_UNUSED( hData );
	WWSJOB_UNUSED( numDwords );
	WWSJOB_UNUSED( pAuditData );

	WWSJOB_ASSERT( pTimingTracker->m_auditTicksAssignedTo <= WwsJob_kAuditTicksAssignedToJob );

	U32 cycles;				
	switch( id )
	{
	// future cycles assigned to spursKernel
	case AuditId::kWwsJob_end:
	case AuditId::kWwsJob_PollSpursKernel_begin:
	case AuditId::kWwsJob_setSpursKernelReadyCount_begin_zero:
	case AuditId::kWwsJob_setSpursKernelReadyCount_begin_old:
		cycles = time -/*allow wrap*/ pTimingTracker->m_auditTicksLast;
		pTimingTracker->m_auditTicksAssigned[pTimingTracker->m_auditTicksAssignedTo] += cycles;
		pTimingTracker->m_auditTicksLast = time;
		pTimingTracker->m_auditTicksAssignedTo = WwsJob_kAuditTicksAssignedToSpursKernel;
		break;

	// push state, future cycles assigned to wwsjob manager
	case AuditId::kWwsJob_InterruptHandler_begin:
		WWSJOB_ASSERT( pTimingTracker->m_auditTicksAssignedToPushed == WwsJob_kAuditTicksAssignedToUninitialized );
		pTimingTracker->m_auditTicksAssignedToPushed = pTimingTracker->m_auditTicksAssignedTo;
		// fall thru!
	// future cycles assigned to wwsjob manager
	case AuditId::kWwsJob_begin:
	case AuditId::kWwsJob_ChangeLoadToRunJob_jobEnd:
	case AuditId::kWwsJob_JobApi_begin:
	case AuditId::kWwsJob_PollSpursKernel_end:
	case AuditId::kWwsJob_setSpursKernelReadyCount_end_zero:
	case AuditId::kWwsJob_setSpursKernelReadyCount_end_old:
		cycles = time -/*allow wrap*/ pTimingTracker->m_auditTicksLast;
		pTimingTracker->m_auditTicksAssigned[pTimingTracker->m_auditTicksAssignedTo] += cycles;
		pTimingTracker->m_auditTicksLast = time;
		pTimingTracker->m_auditTicksAssignedTo = WwsJob_kAuditTicksAssignedToWwsJob;
		break;

	// future cycles assigned to job
	case AuditId::kWwsJob_ChangeLoadToRunJob_jobBegin:
		pTimingTracker->m_auditTicksNumJobs++;

	  #if 0 // If you want to assign time to particular jobs, you have to know which job it was.
			// This snippet shows you how to get the main mem adrs of the job, the workloadId, and the jobHeaderIndex within that workload.
			// You could use this info (and rearrange how this code is written) so that you could get timings for any combination of jobs you want
			// This snippet just exists to show you how to get the data (and print it to validate the data is correct).
			// Note: the text which tells the job manager how to decode the kWwsJob_ChangeLoadToRunJob_jobBegin audit is in jobmanagerauditdata.h
		// get data for job in extra 64 bits
		union JobData // call whatever you want
		{
			U64		m_u64;
			struct
			{
				U32	m_mmaJob;
				U16	m_workloadId;
				U16 m_jobHeaderIndex;
			};
		};
		JobData jobData;
		jobData.m_u64 = *(U64*)pAuditData;
		printf( "mmaJob=0x%08X, workloadId=0x%04X, jobHeaderIndex=0x%04X\n", jobData.m_mmaJob, jobData.m_workloadId, jobData.m_jobHeaderIndex );
	  #endif
	  		
		// fall thru to code below

	case AuditId::kWwsJob_JobApi_end:
		cycles = time -/*allow wrap*/ pTimingTracker->m_auditTicksLast;
		pTimingTracker->m_auditTicksAssigned[pTimingTracker->m_auditTicksAssignedTo] += cycles;
		pTimingTracker->m_auditTicksLast = time;
		pTimingTracker->m_auditTicksAssignedTo = WwsJob_kAuditTicksAssignedToJob;
		break;

	// future cycles assigned to (pop state)
	case AuditId::kWwsJob_InterruptHandler_end:
		cycles = time -/*allow wrap*/ pTimingTracker->m_auditTicksLast;
		pTimingTracker->m_auditTicksAssigned[pTimingTracker->m_auditTicksAssignedTo] += cycles;
		pTimingTracker->m_auditTicksLast = time;
		pTimingTracker->m_auditTicksAssignedTo = pTimingTracker->m_auditTicksAssignedToPushed;
		pTimingTracker->m_auditTicksAssignedToPushed = WwsJob_kAuditTicksAssignedToUninitialized;
		break;

	// any other audit does not affect basic timing
	default:
		break;
	}

	// this routine always returns true
	return true;
}

//--------------------------------------------------------------------------------------------------

static int GetPpuThreadPriority( void )
{
	sys_ppu_thread_t thisPpuThreadId;
	int ret;
	WWSJOB_UNUSED( ret );

	ret = sys_ppu_thread_get_id( &thisPpuThreadId );
	WWSJOB_ASSERT( CELL_OK == ret );

	int thisPpuThreadPriority;
	ret = sys_ppu_thread_get_priority( thisPpuThreadId, &thisPpuThreadPriority );
	WWSJOB_ASSERT( CELL_OK == ret );

	return thisPpuThreadPriority;
}

//--------------------------------------------------------------------------------------------------

CellSpurs g_spurs;			//Spurs object

int main()
{
	int kNumSpus			= 1;


	//Spurs kernel runs on SPU Threads
	int ret = sys_spu_initialize( kNumSpus, 0 );
	WWSJOB_ASSERT( CELL_OK == ret );

	//Initialize Spurs
	int ppuThreadPrio	= GetPpuThreadPriority() - 1;	//Higher priority than main thread
	int spuThgrpPrio	= 128;
	int isExit			= 0;	// Must be false
	ret = cellSpursInitialize( &g_spurs, kNumSpus, spuThgrpPrio, ppuThreadPrio, isExit );
	WWSJOB_ASSERT( CELL_OK == ret );
	printf( "Spurs initialized (spurs=0x%p)\n", &g_spurs );


	//Get job module handle
	extern char _binary_timingjob1_file_start[];
	extern char _binary_timingjob1_file_size;
	SpuModuleHandle spuJob1Handle ( _binary_timingjob1_file_start, (U32)&_binary_timingjob1_file_size, "timingjob1.spu.mod" );
	extern char _binary_timingjob2_file_start[];
	extern char _binary_timingjob2_file_size;
	SpuModuleHandle spuJob2Handle ( _binary_timingjob2_file_start, (U32)&_binary_timingjob2_file_size, "timingjob2.spu.mod" );
	extern char _binary_timingjob3_file_start[];
	extern char _binary_timingjob3_file_size;
	SpuModuleHandle spuJob3Handle ( _binary_timingjob3_file_start, (U32)&_binary_timingjob3_file_size, "timingjob3.spu.mod" );
	extern char _binary_timingjobx_file_start[];
	extern char _binary_timingjobx_file_size;
	SpuModuleHandle spuJobxHandle ( _binary_timingjobx_file_start, (U32)&_binary_timingjobx_file_size, "timingjobx.spu.mod" );


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//Init our audit buffers
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	const U32 kAuditBufferSize = 8192*1024;	//Shared between 6 SPUs
	static U8 g_buffersForAudits[kAuditBufferSize] WWSJOB_ALIGNED(128);
	memset( g_buffersForAudits, 0, sizeof(g_buffersForAudits) );

	AuditManager auditManager;

	auditManager.InitAuditMemory( g_buffersForAudits, kAuditBufferSize, kNumSpus, AuditManager::kSingleBuffered );

	// enable jobmanager audits
	auditManager.SetJobManagerAuditsEnabled( true );

	// enable job audits
	auditManager.SetJobAuditsEnabled( true );

	// register audits for jobs
	auditManager.RegisterAuditData( AuditId::kTimingJob1_auditsBegin, AuditId::kTimingJobx_auditsEnd, "Timing_", g_timingJobText );


	//Install the event handler to handle JobPrintf
	EventHandler eventHandler( &g_spurs, ppuThreadPrio, &auditManager );


	//This is the struct we'll be using to accumulate timing for our audits
	AuditTickTracker tickTracker;


	//Create a job list
	static U8 s_bufferForJobList[ 1024 ] WWSJOB_ALIGNED( 128 );
	MultiThreadSafeJobList jobList( s_bufferForJobList, sizeof(s_bufferForJobList), "TimingJobList" );


	//Note that since this joblist uses kReadWriteCached buffers, this means that consecutive jobs should not go to different SPUs.
	//For this reason, the maxContention on the joblist is set to just 1 SPU at once.
	U32 maxContention			= 1;
	const uint8_t standardWorkPrios[8]		= { 8, 8, 8, 8, 8, 8, 0, 0 };

	//Tell SPURS about the job list
	jobList.AttachToSpurs( &g_spurs, standardWorkPrios, maxContention, &auditManager );


	//CommandListBuilder helper class for building jobs
	static U32 s_commandListBuilderDataBuffer[ 2 * 1024 ] WWSJOB_ALIGNED( 128 );
	CommandListBuilder commandListBuilder( s_commandListBuilderDataBuffer, sizeof( s_commandListBuilderDataBuffer ) );


	const U32 kLogicalBuffer0 = 0;

	// test parameters to job1:3 and x
	// in normal use you would pass valid data through the parameters
	U32 WWSJOB_ALIGNED(16) jobParams[8] = { 0x80000000, 0x80000001, 0x80000002, 0x80000003, 0x80000004, 0x80000005, 0x80000006, 0x80000007 };


	// get size of code for job1, job2, job3, and jobx
	const U32 kJob1CodeBufferSizeInPages	= spuJob1Handle.GetRequiredBufferSizeInPages();	//big enough for the code and bss
	const U32 kJob2CodeBufferSizeInPages	= spuJob2Handle.GetRequiredBufferSizeInPages();	//big enough for the code and bss
	const U32 kJob3CodeBufferSizeInPages	= spuJob3Handle.GetRequiredBufferSizeInPages();	//big enough for the code and bss
	const U32 kJobxCodeBufferSizeInPages	= spuJobxHandle.GetRequiredBufferSizeInPages();	//big enough for the code and bss

	// get max size of buffer to hold any of the 4 codes
	U32 kJobCodeBufferSizeInPages = WwsJob_max( kJob1CodeBufferSizeInPages, WwsJob_max( kJob2CodeBufferSizeInPages,
			WwsJob_max( kJob3CodeBufferSizeInPages, kJobxCodeBufferSizeInPages ) ) );

	// for all 4 codes, we'll use bufferSet 0, with 2 buffers, starting at the first available page
	const U32 kJobCodeBufferSet 			= 0;
	const U32 kJobNumCodeBuffers			= 2;
	const U32 kJobCodeBufferSetBasePageNum	= LsMemoryLimits::kJobAreaBasePageNum;


	// for job1:3 data, we'll use bufferSet 1, with 2 buffers of 3KB, but with an ea specific to job1:3
	// for jobx   data, we'll use bufferSet 1, with 2 buffers of 3KB, but they are just work buffers
	const U32 kJobDataBufferSet 				= 1;
	const U32 kJobNumDataBuffers				= 2;
	const U32 kJobDataBufferSizeInPages			= 3;
	const U32 kJobDataBufferSetBasePageNum		= kJobCodeBufferSetBasePageNum +
			kJobCodeBufferSizeInPages * kJobNumCodeBuffers;
	static U8 WWSJOB_ALIGNED(128) s_job123DataTbl[kJobDataBufferSizeInPages << 10];


	// Create the load commands for job1
	// For the code: the job manager returns the first of 2 buffers.
	//  	Since we only have one job1 job, there's not much point making it shareable, so we won't
	commandListBuilder.InitializeJob();
	commandListBuilder.ReserveBufferSet( kJobCodeBufferSet, kJobNumCodeBuffers,
			kJobCodeBufferSetBasePageNum, kJobCodeBufferSizeInPages );	
	commandListBuilder.UseInputBuffer( kJobCodeBufferSet, kLogicalBuffer0, spuJob1Handle, WwsJob_Command::kNonCached );
	// For the data: the job manager returns the first of 2 buffers,
	// which job1 fills with 10.0, and will share with job2
	commandListBuilder.ReserveBufferSet( kJobDataBufferSet, kJobNumDataBuffers,
			kJobDataBufferSetBasePageNum, kJobDataBufferSizeInPages );	
	commandListBuilder.UseUninitializedBuffer( kJobDataBufferSet, kLogicalBuffer0,
			&s_job123DataTbl[0], kJobDataBufferSizeInPages << 10, WwsJob_Command::kReadWriteCached );
	// We can remove the reservations as we won't grab more buffers
	commandListBuilder.UnreserveBufferSets( (1 << kJobCodeBufferSet) | (1 << kJobDataBufferSet) );	
	// Run the job we loaded
	commandListBuilder.RunJob( kJobCodeBufferSet, kLogicalBuffer0 );
	// Pass params to the job
	jobParams[0] = 0xffff0001; // test value only for job1, param0
	commandListBuilder.AddParams( &jobParams[0], sizeof(U32) * 8 );
	JobHeader job1 = commandListBuilder.FinalizeJob();

	
	// Create the load commands for job2
	// For the code: the job manager returns the second of 2 buffers (since job1 is still running)
	//  	Since we have multiple job2 jobs, we declare it shareable, in hopes it just gets loaded once.
	commandListBuilder.InitializeJob();
	commandListBuilder.ReserveBufferSet( kJobCodeBufferSet, kJobNumCodeBuffers, kJobCodeBufferSetBasePageNum, kJobCodeBufferSizeInPages );	
	commandListBuilder.UseInputBuffer( kJobCodeBufferSet, kLogicalBuffer0, spuJob2Handle, WwsJob_Command::kReadOnlyCached );
	// For the data: the job manager returns the first of 2 buffers (shared from job1),
	//	job2 modifies it to 11.0, and will share with job2 or 3
	commandListBuilder.ReserveBufferSet( kJobDataBufferSet, kJobNumDataBuffers,
			kJobDataBufferSetBasePageNum, kJobDataBufferSizeInPages );	
	commandListBuilder.UseInputBuffer( kJobDataBufferSet, kLogicalBuffer0,
			&s_job123DataTbl[0], kJobDataBufferSizeInPages << 10, WwsJob_Command::kReadWriteCached );
	// We can remove the reservations as we won't grab more buffers
	commandListBuilder.UnreserveBufferSets( (1 << kJobCodeBufferSet) | (1 << kJobDataBufferSet) );	
	// Run the job we loaded
	commandListBuilder.RunJob( kJobCodeBufferSet, kLogicalBuffer0 );
	// Pass params to the job
	jobParams[0] = 0xffff0002; // test value only for job2, param0
	commandListBuilder.AddParams( &jobParams[0], sizeof(U32) * 8 );
	JobHeader job2 = commandListBuilder.FinalizeJob();


	//Create jobx (which is not part of the job1-2-3 sequence)
	// Create the load commands for jobx
	// For the code: the job manager returns the first of 2 buffers
	//  	Since we have multiple job2 jobs, we declare it shareable, in hopes it just gets loaded once.
	commandListBuilder.InitializeJob();
	commandListBuilder.ReserveBufferSet( kJobCodeBufferSet, kJobNumCodeBuffers, kJobCodeBufferSetBasePageNum, kJobCodeBufferSizeInPages );	
	commandListBuilder.UseInputBuffer( kJobCodeBufferSet, kLogicalBuffer0, spuJobxHandle, WwsJob_Command::kReadOnlyCached );
	// For the data: the job manager returns the second of 2 buffers (a temp buffer)
	// It fills the buffer with 99.0 (but the data is never stored to memory)
	commandListBuilder.ReserveBufferSet( kJobDataBufferSet, kJobNumDataBuffers,
			kJobDataBufferSetBasePageNum, kJobDataBufferSizeInPages );	
	commandListBuilder.UseUninitializedBuffer( kJobDataBufferSet, kLogicalBuffer0 );
	// We can remove the reservations as we won't grab more buffers
	commandListBuilder.UnreserveBufferSets( (1 << kJobCodeBufferSet) | (1 << kJobDataBufferSet) );	
	// Run the job we loaded
	commandListBuilder.RunJob( kJobCodeBufferSet, kLogicalBuffer0 );
	// Pass params to the job
	jobParams[0] = 0xffff000f; // test value only for jobx, param0
	commandListBuilder.AddParams( &jobParams[0], sizeof(U32) * 8 );
	JobHeader jobx = commandListBuilder.FinalizeJob();


	// Create the load commands for job3
	// For the code: the job manager returns the first of 2 buffers
	//  	Since we only have one job3 job, there's not much point making it shareable, so we won't
	commandListBuilder.InitializeJob();
	commandListBuilder.ReserveBufferSet( kJobCodeBufferSet, kJobNumCodeBuffers, kJobCodeBufferSetBasePageNum, kJobCodeBufferSizeInPages );	
	commandListBuilder.UseInputBuffer( kJobCodeBufferSet, kLogicalBuffer0, spuJob3Handle, WwsJob_Command::kNonCached );
	// For the data: the job manager returns the second of 2 buffers (shared from job2).
	// The job stores it out (but the buffer is *not* shared to subsequent jobs)
	commandListBuilder.ReserveBufferSet( kJobDataBufferSet, kJobNumDataBuffers,
			kJobDataBufferSetBasePageNum, kJobDataBufferSizeInPages );	
	commandListBuilder.UseInputBuffer( kJobDataBufferSet, kLogicalBuffer0,
		&s_job123DataTbl[0], kJobDataBufferSizeInPages << 10, WwsJob_Command::kNonCached );
	// We can remove the reservations as we won't grab more buffers
	commandListBuilder.UnreserveBufferSets( (1 << kJobCodeBufferSet) | (1 << kJobDataBufferSet) );	
	// Run the job we loaded
	commandListBuilder.RunJob( kJobCodeBufferSet, kLogicalBuffer0 );
	// Pass params to the job
	jobParams[0] = 0xffff0003; // test value only for job3, param0
	commandListBuilder.AddParams( &jobParams[0], sizeof(U32) * 8 );
	JobHeader job3 = commandListBuilder.FinalizeJob();


	for( U32 frameNum = 0 ; frameNum < 20 ; frameNum++ )
	{
		// reset the list since we're doing frames now
		jobList.ResetList();

		//Add the job to our jobList
		jobList.AddJob( job1 );
	
		// Add the job to our jobList
		jobList.AddJob( job2 );
	
		// Add the job to our jobList a second time
		// Note the code will reuse  the second buffer (so it won't load it again)
		// Note the data will modify the first  buffer to 12.0 (so it won't load it again)
		jobList.AddJob( job2 );
	
		//Add jobx to our jobList
		jobList.AddJob( jobx );
	
		//Add jobx to our jobList a second time
		// Note the code will reuse the first buffer (so it won't load it again)
		// Note the data will use   the first buffer (the previous jobx did *not* share the buffer)
		//		Since the 2nd jobx load commands execute while the first jobx is running, the second data buffer
		//		is in use and thus not available.  So the 2nd jobx takes over the first data buffer,
		//		which was the job2 persistant buffer (thus forcing it to be saved to memory).
		//
		//		Note further that if for some reason the 2nd jobx was delayed and the first jobx had finished
		//		(thus the 2nd data buffer was no longer in use), then the second jobx job would
		//		have taken the *2nd* data buffer since the first data buffer was persistant and the 2nd
		//		buffer was unused.  You can also force this condition if the jobx load commands do not
		//		unreserve the data buffer, which forces the 2nd jobx reserveBufferSet load command to wait until
		//		the first jobx finished (& the job manager does the unreserve since the jobx forgot to).
		//
		//		Fill the buffer with 99.0.  This is a test to ensure it doesn't overwrite the job2 persistant buffer.
		jobList.AddJob( jobx );
	
		//Add job2 to our jobList a third time
		// Note the code will reuse  the second buffer (so it won't load it again)
		// Note the data will modify the second buffer to 13.0 (it will have to reload it from memory)
		jobList.AddJob( job2 );

		//Add job2 to our jobList a whole bunch of times so the timing is mostly this job
		const U32 NUM_EXTRA_JOB2S = 90;
		for( U32 i = 0 ; i < NUM_EXTRA_JOB2S ; i++ )
		{
			// Note the code will reuse  the second buffer (so it won't load it again)
			// Note the data will modify the second buffer by +1 each time
			jobList.AddJob( job2 );
		}
		
		//Add the job to our jobList
		jobList.AddJob( job3 );
	
		//Tell SPURS that there's work waiting to be processed.
		jobList.SetReadyCount( MultiThreadSafeJobList::kRequestOneSpu );
	
		//Stall until all jobs on the joblist are done
		jobList.WaitForJobListEnd();
	
		// Verify that (second) data buffer is set to 13.0
		F32* pData = (F32*)&s_job123DataTbl[0];
		WWSJOB_UNUSED( pData );
		U32 numWords = kJobDataBufferSizeInPages << (10-2);
		for( U32 i = 0 ; i < numWords ; i++ )
		{
			WWSJOB_ASSERT( pData[i] == 13.0f + NUM_EXTRA_JOB2S );
		}
		//printf( "Buffer value verfied correct\n" );
		
		//Assign the audit cycles, empty them.
		//Note: in this sample we don't want anything to print while the audits are scanned
		tickTracker.m_auditTicksAssignedTo = WwsJob_kAuditTicksAssignedToUninitialized;
		auditManager.ProcessAuditBuffersForAllSpus( 0, AssignAuditTicks, AuditManager::kDontPrintHeaders, &tickTracker );
		auditManager.EmptyAuditBuffersForAllSpus( 0 );
	}

	// print the audit cycles
	printf( "Note: cycles for SpursKernel only includes time to poll for a job/workload from the WwsJob manager\n  and does not include any other cycles which it does take\n" );
	printf( "#cycles taken per job (%d jobs):\n", tickTracker.m_auditTicksNumJobs );
	for( U32 i = WwsJob_kAuditTicksAssignedToSpursKernel ; i <= WwsJob_kAuditTicksAssignedToJob ; i++ )
	{
		printf( "\t%9.0f: %s\n", tickTracker.GetAverageNumCycles(i), s_auditTicksTitle[i] );
	}


	//Tidy up everything and shut down cleanly.
	jobList.Shutdown();

	eventHandler.RemoveEventHandler( &g_spurs );

	ret = cellSpursFinalize( &g_spurs );
	WWSJOB_ASSERT( CELL_OK == ret );

	printf( "End of timing sample\n" );

	return 0;
}


