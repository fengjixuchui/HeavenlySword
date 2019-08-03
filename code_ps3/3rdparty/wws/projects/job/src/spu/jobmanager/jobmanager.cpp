/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file

	@brief		The main core of the job manager SPU Policy Module
**/
//--------------------------------------------------------------------------------------------------

#include <cell/spurs/types.h>
#include <cell/spurs/policy_module.h>
#include <cell/spurs/ready_count.h>
#include <spu_mfcio.h>
#include <stddef.h>

#include <jobapi/jobdefinition.h>
#include <jobapi/audittypes.h>
#include <jobapi/jobmanagerauditids.h>
#include <jobapi/jobapi.h>
#include <jobapi/spumoduleheader.h>
#include <jobapi/spuinterrupts.h>
#include <jobmanager/interrupthandler.h>
#include <jobmanager/allocatejob.h>
#include <jobmanager/auditwriter.h>
#include <jobmanager/jobmanagerdmatags.h>
#include <jobmanager/spustack.h>
#include <jobmanager/jobheadercache.h>
#include <jobmanager/jobmanager.h>
#include <jobmanager/data.h>

//--------------------------------------------------------------------------------------------------

//#define CHRIS_DOWNCODING

// Enable the first line of 2 below only for internal WWSJOB debugging
// This will break before any code that has yet to be seen executing there
//#define UNTESTED_CODE()		do { WWSJOB_BREAKPOINT(); } while (false)
#define UNTESTED_CODE()

extern U32 _end[];

// main loop
extern "C" I32 WwsJob_Main( uintptr_t spursContext, uint64_t eaWorkLoad ) __attribute__((noreturn));

//namespace Wws
//{
//namespace Job
//{


//====================================================================================================================
// global vars (these used to be statis, but must be globals for downcoding)
//====================================================================================================================

// useful constants
#if WWS_JOB_USE_C_VERSION!=0
static const VU32 WWSJOB_ALIGNED(16) g_WwsJob_v0    	= {0, 0, 0, 0};
static const VU32 WWSJOB_ALIGNED(16) g_WwsJob_vNeg1 	= {0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff};	///< all 1's
#endif


//====================================================================================================================
// code prototypes
//====================================================================================================================

// initialization code
extern "C" void WwsJob_Initialize( U64 workLoadData );

// interrupt handler
extern "C" void WwsJob_Interrupts( U32 dmaTagMask );

// misc routines
extern "C" Bool32 WwsJob_TryDumpAllStoreShareBufs( void );
extern "C" Bool32 WwsJob_TryDumpShareBuf( WwsJob_BufferSet bufferSet, WwsJob_Buffer *pBuffer, WwsJob_Buffer buffer, U32 bufferPageNum, U32 dmaTagId );
extern "C" void WwsJob_TryChangeFreeToLoadJob( void );
extern "C" void WwsJob_ChangeRunToStoreJob( void );
extern "C" Bool32 WwsJob_TryChangeStoreToFreeJob( void );
extern "C" Bool32 WwsJob_TryFreeTagAndUsedPages( WwsJob_BufferSet bufferSet, U32 bufferNum, WwsJob_Buffer buffer );
extern "C" void WwsJob_TryExecuteLoadCmds( void );
extern "C" Bool32 WwsJob_TryExecuteCmds( I32 prevJobNum, I32 jobNum, WwsJob_Command *pCommands );
extern "C" void WwsJob_ChangeLoadToRunJob( void );
extern "C" void WwsJob_FreeLogicalBuffer( I32 jobNum, U32 logicalBufferSetNum, U32 logicalBufferNum );
extern "C" WwsJob_BufferTag WwsJob_GetBufferTag( U32 logicalBufferSetNum, U32 logicalBufferNum, Bool32 useDmaTagId );
extern "C" void WwsJob_GetLogicalBuffer( I32 jobNum, U32 logicalBufferSetNum, U32 logicalBufferNum, WwsJob_MiscData &miscData );
extern "C" Bool32 WwsJob_PollSpursKernel( void );
extern "C" Bool32 WwsJob_PollSpursKernelQuick( void );
extern "C" Bool32 WwsJob_GetJobFromJobList( void );
extern "C" void WwsJob_ReturnToSpursKernel( void ) __attribute__((noreturn));
extern "C" void WwsJob_FreeDmaTagId( U32 dmaTagId );
extern "C" void WwsJob_GetPageMasks( U32 firstPageNum, U32 numPages, QuadWord *pPageMask );
extern "C" Bool32 WwsJob_IsDmaTagMaskDone( U32 dmaTagMask );
extern "C" void WwsJob_SetDwordMem( U64 *pMem, U32 size, U64 data );
extern "C" void WwsJob_SetQwordMem( void* pMem, U32 size, U64 data );
extern "C" U32	WwsJob_UseDmaTagId( void );

// JobManager Spu api
extern "C" void WwsJob_JobApi_ExecuteCommands( WwsJob_Command* pCommands );
extern "C" WwsJob_BufferTag WwsJob_JobApi_GetBufferTag( U32 unused, U32 logicalBufferSetNum, U32 logicalBufferNum, WwsJob_RequestDmaTag dmaTagRequest );
extern "C" void WwsJob_JobApi_GetBufferTags( WwsJob_BufferTagInputOutput* pBufferTagInput, U32 numBuffers );
extern "C" U32 WwsJob_JobApi_UseDmaTagId( void );
extern "C" void WwsJob_JobApi_FreeDmaTagId( U32 dmaTagId );
extern "C" void WwsJob_JobApi_LoadNextJob( void );
extern "C" void WwsJob_JobApi_FreeLogicalBuffer(  U32 unused, U32 logicalBufferSetNum, U32 logicalBufferNum );
extern "C" void WwsJob_JobApi_FreeLogicalBuffers( const U8* pLogicalBufferNumPairs, U32 numLogicalBufferNumPairs );
extern "C" void WwsJob_JobApi_StoreAudit( U32 parm1, U32 parm2, U32 parm3 );


//====================================================================================================================
// WwsJob_Initialize code
//====================================================================================================================

/**	\brief	initialize wwsjob manager

	\param	spursContext needed to work with spus
	\param	workLoadData = [main mem addr of audit buffer array base | main mem addr of spurs workload]
**/
#if WWS_JOB_USE_C_VERSION!=0
void WwsJob_Initialize( U64 workLoadData )
{
	//The job manager and spurs should fit within the space allocated
	WWSJOB_ASSERT( _end < PageNumToLsAddress(LsMemoryLimits::kJobAreaBasePageNum) );

	g_WwsJob_eaWorkLoad				= (U32)workLoadData;
	g_WwsJob_spursWorkloadId		= cellSpursGetWorkloadId();

	// set spuNum
	U32 spuNum = cellSpursGetCurrentSpuId();
	g_WwsJob_dataForJob.m_spuNum	= spuNum;
	g_WwsJob_dataForJob.m_pJobApi[0].m_executeCommands = (ExecuteCommandsPtr)&WwsJob_JobApi_ExecuteCommands;
	g_WwsJob_dataForJob.m_pJobApi[1].m_getBufferTag = WwsJob_JobApi_GetBufferTag;
	g_WwsJob_dataForJob.m_pJobApi[2].m_getBufferTags = WwsJob_JobApi_GetBufferTags;
	g_WwsJob_dataForJob.m_pJobApi[3].m_useDmaTagId = WwsJob_JobApi_UseDmaTagId;
	g_WwsJob_dataForJob.m_pJobApi[4].m_freeDmaTagId = WwsJob_JobApi_FreeDmaTagId;
	g_WwsJob_dataForJob.m_pJobApi[5].m_loadNextJob = WwsJob_JobApi_LoadNextJob;
	g_WwsJob_dataForJob.m_pJobApi[6].m_freeLogicalBuffer = WwsJob_JobApi_FreeLogicalBuffer;
	g_WwsJob_dataForJob.m_pJobApi[7].m_freeLogicalBuffers = WwsJob_JobApi_FreeLogicalBuffers;
	g_WwsJob_dataForJob.m_pJobApi[8].m_storeAudit = WwsJob_JobApi_StoreAudit;

	InstallInterruptHandler();

	InitAudits( spuNum, (U32)(workLoadData >> 32L) );

	STORE_WORKLOAD_AUDIT( AuditId::kWwsJob_begin, g_WwsJob_spursWorkloadId, workLoadData << 32 );

	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_Initialize, 0,
			( (U64)workLoadData << 32 ) | (U32)g_WwsJob_loadCommands,
			((U64)(U32)&g_WwsJob_bufferSetArray[0] << 32) | (U32)&g_WwsJob_bufferArray[0],
			(U32)&g_WwsJob_logicalToBufferNumArray[0] );

	//Given the spurs kernel is allowed to re-run our Job Manager without reloading out of main memory,
	//you can't assume things will have been re-initialised
	WwsJob_SetQwordMem( &g_WwsJob_bufferSetArray[0],
			sizeof(WwsJob_BufferSet) * 3/*jobs*/ * WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB, 0 );
	WwsJob_SetQwordMem( &g_WwsJob_bufferArray[0],
			sizeof(WwsJob_Buffer) * 3/*jobs*/ * WWSJOB_MAX_NUM_BUFFERS_PER_JOB, 0 );
	WwsJob_SetQwordMem( &g_WwsJob_logicalToBufferNumArray[0],
			sizeof(U8) * 3/*jobs*/ * WWSJOB_MAX_NUM_BUFFERS_PER_JOB, 0xFFFFFFFFFFFFFFFFULL );
	g_WwsJob_loadJobState		= 0;
	g_WwsJob_runJobState		= 0;
	g_WwsJob_nextLoadJobNum 	= 0;
	g_WwsJob_loadJobNum			= -1;
	g_WwsJob_runJobNum			= -1;
	g_WwsJob_storeJobNum		= -1;
	g_WwsJob_lastStoreJobNum	= 2;
	g_WwsJob_timeStamp			= 0;
	g_WwsJob_numUnassignedDmaTagIds = 0;

	// get initial page mask bits
	g_WwsJob_initialPageMask[0].m_vu32 = g_WwsJob_v0;
	g_WwsJob_initialPageMask[1].m_vu32 = g_WwsJob_v0;

	// mark how many KBytes are used for Spurs Kernel and PM at beginning, and stack at end
	WWSJOB_VERBOSE_ASSERT( LsMemoryLimits::kJobAreaBasePageNum <= 64 );	//Just because I haven't bothered to make this code work with higher values
	g_WwsJob_initialPageMask[0].m_u64[0] = 0xFFFFFFFFFFFFFFFFULL << ((U64)(64 - LsMemoryLimits::kJobAreaBasePageNum));

	WWSJOB_VERBOSE_ASSERT( LsMemoryLimits::kJobStackSizeInPages < 32 ); //Just because I haven't bothered to make this code work with higher values
	g_WwsJob_initialPageMask[1].m_u32[3] = (1 << LsMemoryLimits::kJobStackSizeInPages) - 1;

	SetStackOverflowMarker();

	// set page mask bits to initial value
	g_WwsJob_reservedPageMask[0] 	= g_WwsJob_initialPageMask[0];
	g_WwsJob_reservedPageMask[1] 	= g_WwsJob_initialPageMask[1];
	g_WwsJob_usedPageMask[0] 		= g_WwsJob_initialPageMask[0];
	g_WwsJob_usedPageMask[1] 		= g_WwsJob_initialPageMask[1];
	g_WwsJob_shareablePageMask[0].m_vu32 = g_WwsJob_v0;
	g_WwsJob_shareablePageMask[1].m_vu32 = g_WwsJob_v0;

	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_reservedPages, 0,
			g_WwsJob_reservedPageMask[0].m_u64[0], g_WwsJob_reservedPageMask[0].m_u64[1],
			g_WwsJob_reservedPageMask[1].m_u64[0], g_WwsJob_reservedPageMask[1].m_u64[1] );
	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_usedPages, 0,
			g_WwsJob_usedPageMask[0].m_u64[0], g_WwsJob_usedPageMask[0].m_u64[1],
			g_WwsJob_usedPageMask[1].m_u64[0], g_WwsJob_usedPageMask[1].m_u64[1] );
	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_shareablePages, 0,
			g_WwsJob_shareablePageMask[0].m_u64[0], g_WwsJob_shareablePageMask[0].m_u64[1],
			g_WwsJob_shareablePageMask[1].m_u64[0], g_WwsJob_shareablePageMask[1].m_u64[1] );

	// init dmaTagMask (note dmaTagId 0, at bit 31, must be taken)
	g_WwsJob_usedDmaTagMask = (1 << DmaTagId::kNumUsed) - 1;

	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_dmaTags, 0, g_WwsJob_usedDmaTagMask );

	g_WwsJob_jobDataArray[0].m_jobHasShareableBuffers	= 0;
	g_WwsJob_jobDataArray[1].m_jobHasShareableBuffers	= 0;
	g_WwsJob_jobDataArray[2].m_jobHasShareableBuffers	= 0;
	g_WwsJob_jobDataArray[0].m_jobIndex			= 0xFFFF;
	g_WwsJob_jobDataArray[1].m_jobIndex			= 0xFFFF;
	g_WwsJob_jobDataArray[2].m_jobIndex			= 0xFFFF;
	g_WwsJob_jobDataArray[0].m_numDependencies	= 0;
	g_WwsJob_jobDataArray[1].m_numDependencies	= 0;
	g_WwsJob_jobDataArray[2].m_numDependencies	= 0;

	//We must clear the job header cache to make sure it doesn't have invalid data cached
	//from a previous run of the job manager
	g_currentJobHeaderCacheEa = 0;
}
#endif



//====================================================================================================================
// Interrupt handler
//====================================================================================================================

/**	\brief	wwsjob interrupt code, called from actual interrupt handler

	\param	dmaTagMask	contains one or more bits for the required interrupts (bits 31:0 correspond to dmaTagId 0:31)
**/
#if WWS_JOB_USE_C_VERSION!=0
extern "C" void WwsJob_Interrupts( U32 dmaTagMask )	// dmaTagId(s) that caused interrupt
{
	// Note interrupts are disabled when the interrupt occurs

	STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_WwsInterrupts_begin, 0, dmaTagMask );

	// if interrupt is: finished read of loadJob commands
	//NOTE: somehow, we occasionally get readCommands interrupts when there is no load job.
	// For now, fix the crash by ignoring the interrupt if we aren't in the correct state:
	if( (dmaTagMask & (1 << DmaTagId::kLoadJob_readCommands)) &&
		(g_WwsJob_loadJobState == WwsJob_LoadJobState::kReadCommands) )
	{	// interrupt is: finished read of loadJob commands

		// NOTE: you do not have to ensure the stall & notify interrupt finished the last null element

		// ensure loadJobNum and state are valid
		WWSJOB_VERBOSE_ASSERT( g_WwsJob_loadJobNum >= 0  &&  g_WwsJob_loadJobNum <= 2 );
//		WWSJOB_VERBOSE_ASSERT( g_WwsJob_loadJobState == WwsJob_LoadJobState::kReadCommands );

		STORE_VERBOSE_AUDIT( AuditId::kWwsJob_WwsInterrupts_readLoadCommandsDone );

		// declare the load commands can execute
		g_WwsJob_loadJobState = WwsJob_LoadJobState::kExecuteCommands;
	}

	// if interrupt is: finished discard write of runJob shareable buffers
	if( dmaTagMask & (1 << DmaTagId::kRunJob_writeShareableBuffers) )
	{	// interrupt is: finished discard write of runJob shareable buffers

		STORE_VERBOSE_AUDIT( AuditId::kWwsJob_WwsInterrupts_writeRunJobShareBufsDone );

		// NOTE: you DO have to ensure the stall & notify interrupt finished the last null element
		do
		{
		   ; // do NOT enable-disable interrupts here
		}while( !WwsJob_IsDmaTagMaskDone( 1 << DmaTagId::kRunJob_writeShareableBuffers ) );

		// this will allow WwsJob_TryExecuteLoadCmds() to continue
		// so you don't have to do anything here
	}

	// if interrupt is: finished write of all lastStoreJob shareable buffers
	if( dmaTagMask & (1 << DmaTagId::kStoreJob_writeAllShareableBuffers) )
	{	// interrupt is: finished write of all lastStoreJob shareable buffers

		STORE_VERBOSE_AUDIT( AuditId::kWwsJob_WwsInterrupts_writeAllStoreJobShareBufsDone );

		// NOTE: you DO have to ensure the stall & notify interrupt finished the last null element
		// Note: this will also ensure all storeJob job dmas are done
		do
		{
			; // do NOT enable-disable interrupts here
		}while( !WwsJob_IsDmaTagMaskDone( 1 << DmaTagId::kStoreJob_writeAllShareableBuffers ) );

		// the lastStoreJobNum must be valid
		WWSJOB_VERBOSE_ASSERT( g_WwsJob_lastStoreJobNum >= 0  &&  g_WwsJob_lastStoreJobNum <= 2 );

		// Try to clear shared buffer states & pages.  If everything could not be cleared
		// Note: if everything could not be cleared, it will start an interrupt which will finish the clearing
		if( !WwsJob_TryDumpAllStoreShareBufs() )
		{	// everything could not be cleared

			// since this is the 2nd time this is called for this job, everything should have been cleared
			WWSJOB_ASSERT(0);
		}
	}

	// if interrupt is: finished write of storeJob job buffers
	if( dmaTagMask & (1 << DmaTagId::kStoreJob_writeJobBuffers) )
	{	// interrupt is: finished write of storeJob job buffers

		// NOTE: you do not have to ensure the stall & notify interrupt finished the last null element

		STORE_VERBOSE_AUDIT( AuditId::kWwsJob_WwsInterrupts_writeStoreJobBuffersDone );

		// change storeJob to freeJob.
		//In the new setup, it might be possible that we've already harvested the store job before the interrupt fires
		//so we should check if a store job still exists before trying to free it.
		if ( g_WwsJob_storeJobNum >= 0 )	
		{
			// Note WwsJob_TryChangeStoreToFreeJob should never return false here,
			//	but occasionally it is and this allows execution to continue properly
			// - This should be fixed now
			do
			{
				; // delay (don't enable interrupts here)
			}while( !WwsJob_TryChangeStoreToFreeJob() );
		}
	}

	// Note: all interrupts can cause load commands to proceed (if in that state)
	// if loadJob commands need executing
	if( g_WwsJob_loadJobState == WwsJob_LoadJobState::kExecuteCommands )
	{	// loadJob commands need executing

		STORE_VERBOSE_AUDIT( AuditId::kWwsJob_WwsInterrupts_tryToExecuteLoadCommands );

		// Try to execute the load commands.
		WwsJob_TryExecuteLoadCmds();
	}

	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_WwsInterrupts_end );
}
#endif


//====================================================================================================================
// Api used by running job
//====================================================================================================================

void WwsJob_JobApi_ExecuteCommands( WwsJob_Command* pCommands )
{
	//__asm__ volatile ("stopd $0, $0, $0");
	WWSJOB_ASSERT( !AreInterruptsEnabled() );	//The user should have interrupts disabled when entering the job manager
	WWSJOB_VERBOSE_ASSERT( g_WwsJob_runJobNum >= 0  &&  g_WwsJob_runJobNum <= 2);
	I32 prevJobNum = (g_WwsJob_runJobNum) ? g_WwsJob_runJobNum - 1 : 2;
	STORE_TIMING_AUDIT( AuditId::kWwsJob_JobApi_begin );

	STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_JobApi_executeCommands );

	while(1)
	{
		// delay
		__builtin_spu_ienable();
		__builtin_spu_idisable();

		// If the storeJob finished discarding the shareable buffers which are needed by the runJob
		if( WwsJob_IsDmaTagMaskDone( 1 << DmaTagId::kStoreJob_writeShareableBuffers ) )
		{	// the storeJob finished discarding the shareable buffers which are needed by the runJob

			STORE_TIMING_AUDIT( AuditId::kWwsJob_JobApi_TryExecuteCmds_begin );
			//STORE_VERBOSE_AUDIT( AuditId::kWwsJob_JobApi_tryToExecuteJobCommands, (U32)pCommands >> 2 );

			// Try to execute the commands.  If the commands finished
			if( WwsJob_TryExecuteCmds( prevJobNum, g_WwsJob_runJobNum, pCommands ) )
			{	// the commands finished

				// exit while(1)
				break;
			}

			if ( g_WwsJob_storeJobNum >= 0 )
				WwsJob_TryChangeStoreToFreeJob();
		}
	}

	//STORE_VERBOSE_AUDIT( AuditId::kWwsJob_JobApi_waitForRunJobReadsToFinish );
	STORE_TIMING_AUDIT( AuditId::kWwsJob_JobApi_TryExecuteCmds_end );

	// delay while runJob command dma reads are ongoing
	do
	{
		__builtin_spu_ienable();
		__builtin_spu_idisable();
	}while( !WwsJob_IsDmaTagMaskDone( 1 << DmaTagId::kRunJob_readBuffers ) );

	STORE_TIMING_AUDIT( AuditId::kWwsJob_JobApi_TryExecuteCmdsDma_end );

	// if we made a change that might allow the load commands to execute, and they need executing
	if( g_WwsJob_loadJobState == WwsJob_LoadJobState::kExecuteCommands )
	{	// we made a change that might allow the load commands to execute, and they need executing

		STORE_VERBOSE_AUDIT( AuditId::kWwsJob_JobApi_tryToExecuteLoadCommands );

		// try to execute the load commands
		WwsJob_TryExecuteLoadCmds();
	}

	STORE_TIMING_AUDIT( AuditId::kWwsJob_JobApi_end );
}

WwsJob_BufferTag WwsJob_JobApi_GetBufferTag( U32 /*unused*/, U32 logicalBufferSetNum, U32 logicalBufferNum, WwsJob_RequestDmaTag dmaTagRequest )
{
	//__asm__ volatile ("stopd $0, $0, $0");
	WWSJOB_ASSERT( !AreInterruptsEnabled() );	//The user should have interrupts disabled when entering the job manager
	WWSJOB_VERBOSE_ASSERT( g_WwsJob_runJobNum >= 0  &&  g_WwsJob_runJobNum <= 2);
	STORE_TIMING_AUDIT( AuditId::kWwsJob_JobApi_begin );

	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_JobApi_getBufferTag );

	// note WwsJob_GetBufferTag will ensure you can't use a dmaTagId after you load the next job
	WwsJob_BufferTag ret = WwsJob_GetBufferTag( logicalBufferSetNum, logicalBufferNum, dmaTagRequest );

	STORE_TIMING_AUDIT( AuditId::kWwsJob_JobApi_end );
	return ret;
}

void WwsJob_JobApi_GetBufferTags( WwsJob_BufferTagInputOutput* parameter0, U32 numBuffers )
{
	//__asm__ volatile ("stopd $0, $0, $0");
	WWSJOB_ASSERT( !AreInterruptsEnabled() );	//The user should have interrupts disabled when entering the job manager
	WWSJOB_VERBOSE_ASSERT( g_WwsJob_runJobNum >= 0  &&  g_WwsJob_runJobNum <= 2);
	STORE_TIMING_AUDIT( AuditId::kWwsJob_JobApi_begin );

	// Get bufferTags for buffers already allocated by command
	// parameter0: U32 ptr to buffer: on input contains WwsJob_BufferTagInput's, on output contains WwsJob_BufferTag's
	// return: bufferTag's stored over input data (input & output are each 16 bytes)

	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_JobApi_getBufferTags, 0,
		( (U64)parameter0/*pBuffer*/ << 32 ) | numBuffers );

	WwsJob_BufferTagInput *pBufferTagInput;
	pBufferTagInput = (WwsJob_BufferTagInput*)parameter0;
	WwsJob_BufferTag *pBufferTag;
	pBufferTag = (WwsJob_BufferTag*)parameter0;

	for( U32 count = 0 ; count < numBuffers; ++count )
	{
		// get logicalBufferSetNum and logicalBufferNum
		U32 logicalBufferSetNum = pBufferTagInput->m_logicalBufferSetNum;
		U32 logicalBufferNum    = pBufferTagInput->m_logicalBufferNum;
		U32 useDmaTagId			= pBufferTagInput->m_useDmaTagId;

		// get bufferTag, store over the input data
		*pBufferTag = WwsJob_GetBufferTag( logicalBufferSetNum, logicalBufferNum, useDmaTagId );

		// get ready for next buffer
		pBufferTagInput++;
		pBufferTag++;
	}

	STORE_TIMING_AUDIT( AuditId::kWwsJob_JobApi_end );
}

U32 WwsJob_JobApi_UseDmaTagId( void )
{
	//__asm__ volatile ("stopd $0, $0, $0");
	WWSJOB_ASSERT( !AreInterruptsEnabled() );	//The user should have interrupts disabled when entering the job manager
	WWSJOB_VERBOSE_ASSERT( g_WwsJob_runJobNum >= 0  &&  g_WwsJob_runJobNum <= 2);
	STORE_TIMING_AUDIT( AuditId::kWwsJob_JobApi_begin );

	// Use a dmaTagId from those available.  Returns dmaTagId(1:31) used, or 0 if none available
	// parameters: none
	// return: apiReturn.m_u32[0] = unique dmaTagId (non-zero)

	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_JobApi_useDmaTagId );

	// get a dmaTagId.  If it is not available then wait until it one is
	U32 ret = WwsJob_UseDmaTagId();

	// keep track of how many non-assigned dmaTagId's the job took
	g_WwsJob_numUnassignedDmaTagIds++;

	STORE_TIMING_AUDIT( AuditId::kWwsJob_JobApi_end );
	return ret;
}

// Free a dmaTagId previously used.
void WwsJob_JobApi_FreeDmaTagId( U32 dmaTagId )
{
	//__asm__ volatile ("stopd $0, $0, $0");
	WWSJOB_ASSERT( !AreInterruptsEnabled() );	//The user should have interrupts disabled when entering the job manager
	WWSJOB_VERBOSE_ASSERT( g_WwsJob_runJobNum >= 0  &&  g_WwsJob_runJobNum <= 2);
	STORE_TIMING_AUDIT( AuditId::kWwsJob_JobApi_begin );
	
	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_JobApi_freeDmaTagId, dmaTagId );

	WwsJob_FreeDmaTagId( dmaTagId );

	// keep track of how many non-assigned dmaTagId's the job took
	g_WwsJob_numUnassignedDmaTagIds--;

	STORE_TIMING_AUDIT( AuditId::kWwsJob_JobApi_end );
}

void WwsJob_JobApi_LoadNextJob( void )
{
	//__asm__ volatile ("stopd $0, $0, $0");
	WWSJOB_ASSERT( !AreInterruptsEnabled() );	//The user should have interrupts disabled when entering the job manager
	WWSJOB_VERBOSE_ASSERT( g_WwsJob_runJobNum >= 0  &&  g_WwsJob_runJobNum <= 2);
	STORE_TIMING_AUDIT( AuditId::kWwsJob_JobApi_begin );

	// Load the next job, if possible
	// parameters: none
	// return: none

	STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_JobApi_loadNextJob );

	// don't call this twice
	WWSJOB_ASSERT( g_WwsJob_runJobState == WwsJob_RunJobState::kLoadNotAllowed );

	// declare runJob now allows loading of the next job
	g_WwsJob_runJobState = WwsJob_RunJobState::kLoadAllowed;

	// Note: the code below used to be the WwsJob_CopyStoreShareBufsToRun( ) routine
	// The routine is big enough that I'll leave the name CopyStoreShareBufsToRun in the audit text.
	//
	//	This will scan the last storeJob buffers.  Any shareable buffers will be
	//	copied to the runJob buffers if it is possible to do so, which allows them
	//	to be passed on to future jobs.  All runJob buffers that are
	//	reserved (but not used yet) will be made inactive.
	//	g_WwsJob_runJobNum and g_WwsJob_lastStoreJobNum indicate the 2 job numbers.

	// if the last storeJob has ever existed
	if( g_WwsJob_jobDataArray[g_WwsJob_lastStoreJobNum].m_jobHasShareableBuffers )
	{	// the last storeJob has ever existed

		STORE_VERBOSE_AUDIT( AuditId::kWwsJob_CopyStoreShareBufsToRun, (g_WwsJob_runJobNum << 8) | g_WwsJob_lastStoreJobNum );

		// This first bufferSet loop exists to get the mask of all pages that were ever reserved in all bufferSets
		//	for the running job

		// get first bufferSetNum
		U32 bufferSetNum = g_WwsJob_runJobNum * WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB;

		// get ptr to first bufferSet
		WwsJob_BufferSet *pBufferSet = &g_WwsJob_bufferSetArray[bufferSetNum];

		// get mask of all pages used by all bufferSets
		QuadWord totalPageMask[2];
		totalPageMask[0].m_vu32 = g_WwsJob_v0;
		totalPageMask[1].m_vu32 = g_WwsJob_v0;
		// get ptr to first bufferSet (don't blow away original)
		WwsJob_BufferSet *pBufferSet2 = pBufferSet;

		/*	Jon: this is logic to downcode the loop just below
		which scans the bufferSets to get totalPageMask, into assy
		Timing is:
		4 cycles per bufferSet
		+ (~1.7 mispredictions + logic time) per ACTIVE bufferSet
		+ 1 misprediction at end
		which is about 175 + 3*logic  cycles if 16 bufferSets and 3 active bufferSets
		or about 140 + 3*logic  cycles if  8 bufferSets and 3 active bufferSets
		basic logic:

		LQ...	load bufferSet qword
		ROTQBY	get into pref dword
		AI add 4 to ptr to bufferSet
		CEQ compare if ptr to bufferSet reached end
		CEQ compare bufferSet prefWord to zero (inactive)
		ANDC true if bufferSet inactive and ptrs to not match

		So loop can be 4 cycles if you duplicate it into 2 parts (A and B) and interleave them,
		where entire loop takes 8 cycles per 2 bufferSets
		I'll denote the lines as "A" or "B" followed by "1", "2" or "3" for the pass the data is valid on

		setup, including hint main BRNZ below will go to loop
		===========================
		loop:		ANDC B3			LQ A1
		CEQ A2 ptrs
		CEQ A2 bufSet	ROTQ B2
		AI A2 ptr,4		BRZ B3 to BActive(mispredicted branch if active bufferSet)
		ANDC A2			LQ B1
		CEQ B2 ptrs
		CEQ B2 bufSet	ROTQ A1
		AI B2 ptr,4		BRNZ A2 to loop (predicted branch if A inactive)
		===========================(note: fallthrough is mispredicted)=============
		HINT branch below to go to loop
		BRZ A2 ptrs to exit (mispredicted branch if end of bufferSets)
		BJoinsA:					inline all active buffer logic here, including (if enough memory) getPageMasks
		...
		HINT main BRNZ loop above will go to loop
		... (a few instructions, I think 3???)
		BR loop
		-------------------------
		BActive:					HINT branch below to go to BJoinsA
		swap registers for A and B (ensure the 1,2,3 versions are correct)
		HINT branch above (BR loop) to go to loop
		BRNZ A2 ptrs to BJoinsA (predicted branch)
		-----------------------------(note: fallthrough is mispredicted)
		exit:
		*/

		// for all bufferSets
		for( bufferSetNum = 0 ; bufferSetNum < WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB ; pBufferSet2++, bufferSetNum++ )
		{
			// get bufferSet
			WwsJob_BufferSet bufferSet = *pBufferSet2;

			// if bufferSet is active
			if( bufferSet.m_u32 )
			{	// bufferSet is active

				STORE_VERBOSE_AUDIT( AuditId::kWwsJob_CopyStoreShareBufsToRun_bufSetActive, bufferSetNum,
					bufferSet.m_u32 );

				// get page configuration
				U32 bufferSetFirstPageNum	= bufferSet.m_firstPageNum;
				U32 numPagesPerBuffer		= bufferSet.m_numPagesPerBuffer;
				U32 numBuffers				= bufferSet.m_numBuffers;

				// get pageMasks
				QuadWord pageMask[2];
				WwsJob_GetPageMasks( bufferSetFirstPageNum, numPagesPerBuffer * numBuffers, &pageMask[0] );

				// accumulate pageMasks for all buffers
#ifdef CHRIS_DOWNCODING
				totalPageMask[0].m_vu32 = spu_or( totalPageMask[0].m_vu32, pageMask[0].m_vu32 );
				totalPageMask[1].m_vu32 = spu_or( totalPageMask[1].m_vu32, pageMask[1].m_vu32 );
#else
				for( U32 i = 0 ; i < 2 ; i++ )
				{
					for( U32 j = 0 ; j < 2 ; j++ )
					{
						// accumulate pageMasks for all buffers
						totalPageMask[i].m_u64[j] |= pageMask[i].m_u64[j];
					}
				}
#endif
			}
		}

		STORE_VERBOSE_AUDIT( AuditId::kWwsJob_CopyStoreShareBufsToRun_runPagesEverReserved, 0,
			totalPageMask[0].m_u64[0], totalPageMask[0].m_u64[1],
			totalPageMask[1].m_u64[0], totalPageMask[1].m_u64[1] );

		// This bufferSet loop exists to scan all runJob and prevJob bufferSets and buffers.
		//	The shareable buffers will be transferred from the prevJob to the runJob

#if 1 // 1 = new 2 single loop code, 0 = old 1 more complex loop code

		// get prevJob first bufferSetNum
		U32 prevJobBufferSetNum = g_WwsJob_lastStoreJobNum * WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB;

		// get prevJob ptr to first bufferSet
		WwsJob_BufferSet *pPrevJobBufferSet = &g_WwsJob_bufferSetArray[prevJobBufferSetNum];

		// hold stuff needed by 2nd loop below
		WwsJob_BufferSet *pBufferSetSave = pBufferSet;
		WwsJob_BufferSet *pPrevJobBufferSetSave = pPrevJobBufferSet;

		// for all bufferSets
		bufferSetNum = 0;
		do
		{
			// get bufferSet
			WwsJob_BufferSet bufferSet = *pBufferSet;

			// get prevJob bufferSet
			WwsJob_BufferSet prevJobBufferSet = *pPrevJobBufferSet;

			STORE_VERBOSE_AUDIT( AuditId::kWwsJob_CopyStoreShareBufsToRun_bufSet, bufferSetNum,
				( (U64)bufferSet.m_u32 << 32 ) | prevJobBufferSet.m_u32 );

			// get prevJob #buffers total
			// (this will be garbage and ignored if the prevJob bufferset is inactive)
			U32 prevJobNumBuffers = prevJobBufferSet.m_numBuffers;

			// if bufferSet is not active and prevJob bufferSet is active and ...
			//	... there are enough buffers to promote the prevJob buffers
			if( !bufferSet.m_u32  &&  prevJobBufferSet.m_u32  &&  prevJobNumBuffers <= g_WwsJob_numFreeBuffers )
			{	// there are enough buffers to promote the prevJob buffers

				// we must check if prevJob bufferSet pages collide with pages in any bufferSet

				// get prevJob bufferSet pageMasks
				QuadWord pageMask[2];
				WwsJob_GetPageMasks( prevJobBufferSet.m_firstPageNum,
					prevJobBufferSet.m_numPagesPerBuffer * prevJobNumBuffers, &pageMask[0] );

				// if prevJob bufferSet pages collide with pages in any bufferSet
#ifdef CHRIS_DOWNCODING
				VU32 and0 = spu_and( totalPageMask[0].m_vu32, pageMask[0].m_vu32 );
				VU32 and1 = spu_and( totalPageMask[1].m_vu32, pageMask[1].m_vu32 );
				VU32 combined = spu_or( and0, and1 );
				if ( si_to_uint( (VI8) spu_orx( combined ) ) )
				{
					STORE_VERBOSE_AUDIT( AuditId::kWwsJob_CopyStoreShareBufsToRun_storeBufSetCollides );
					goto NextBufferSet;
				}
#else
				for( U32 i = 0 ; i < 2 ; i++ )
				{
					for( U32 j = 0 ; j < 2 ; j++ )
					{
						// if prevJob bufferSet pages collide with pages in any bufferSet
						if( totalPageMask[i].m_u64[j] & pageMask[i].m_u64[j] )
						{	// prevJob bufferSet pages collide with pages in any bufferSet

							// get next bufferSet (can't use "continue" since we're inside 'for' loops here
							STORE_VERBOSE_AUDIT( AuditId::kWwsJob_CopyStoreShareBufsToRun_storeBufSetCollides );
							goto NextBufferSet;
						}
					}
				}
#endif

				// prevJob bufferSet pages do *not* collide with pages in any bufferSet
				// copy prevJob bufferSet to bufferSet
				// allocate buffers
				bufferSet = prevJobBufferSet/*ignore m_firstBufferNum*/;
				bufferSet.m_firstBufferNum = g_WwsJob_firstBufferNum;
				STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_CopyStoreShareBufsToRun_copyBufSetToRun, bufferSetNum,
					bufferSet.m_u32 );
				*pBufferSet = bufferSet;
				g_WwsJob_firstBufferNum += prevJobNumBuffers;
				g_WwsJob_numFreeBuffers -= prevJobNumBuffers;

				// get ptr to first buffer in bufferSet
				WwsJob_Buffer *pBuffer = &g_WwsJob_bufferArray[bufferSet.m_firstBufferNum];

				// declare all buffers are inactive
				// Note: data already zero'd for inactive bufferSet
				//WwsJob_SetDwordMem( (U64*)pBuffer, sizeof(WwsJob_Buffer) * prevJobNumBuffers, 0 );

				// for all buffers, declare buffer is reserved, and set bufferSetNum
				WwsJob_Buffer buffer;
				buffer.m_u64 = 0;
				buffer.m_reserved = 1;
				buffer.m_bufferSetNum = bufferSetNum;
				for( U32 bufferNum = 0 ; bufferNum < prevJobNumBuffers ; pBuffer++, bufferNum++ )
				{
					// declare buffer is reserved, and set bufferSetNum
					*pBuffer = buffer;
				}
			}

NextBufferSet:
			// inc misc stuff for next loop
			pBufferSet++;
			pPrevJobBufferSet++;
			bufferSetNum++;

		}while( bufferSetNum < WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB );


		// this is the new 2nd loop

		// get ptr to bufferSets
		WwsJob_BufferSet *pBufferSets = pBufferSetSave;

		// get ptr to prevJob bufferSets
		WwsJob_BufferSet *pPrevJobBufferSets = pPrevJobBufferSetSave;

		// get ptr to first buffer
		U32 bufferIndex/*including job#*/ = g_WwsJob_runJobNum * WWSJOB_MAX_NUM_BUFFERS_PER_JOB;
		WwsJob_Buffer *pBuffer = &g_WwsJob_bufferArray[bufferIndex];

		// get first free bufferNum (with job#)
		U32 firstFreeBufferNum = g_WwsJob_firstBufferNum/*g_WwsJob_jobDataArray[g_WwsJob_runJobNum].m_firstFreeBufferNum*/;

		// loop for all buffers
		do
		{
			// get buffer
			WwsJob_Buffer buffer = *pBuffer;

			STORE_VERBOSE_AUDIT( AuditId::kWwsJob_CopyStoreShareBufsToRun_bufIndex, bufferIndex,
				buffer.m_u64 );

			// if buffer is reserved
			if( buffer.m_reserved )
			{

				// get bufferSet#
				bufferSetNum = buffer.m_bufferSetNum;

				// get bufferSet
				WwsJob_BufferSet bufferSet = pBufferSets[bufferSetNum];

				// bufferSet must be active
				WWSJOB_VERBOSE_ASSERT( bufferSet.m_u32 );

				// get bufferNum
				U32 bufferNum = bufferIndex - bufferSet.m_firstBufferNum/*withJob#*/;

				STORE_VERBOSE_AUDIT( AuditId::kWwsJob_CopyStoreShareBufsToRun_reservedBuf, bufferSetNum,
					( (U64)bufferSet.m_u32 << 32) | bufferNum );

				// bufferNum must be valid
				WWSJOB_ASSERT( bufferNum < bufferSet.m_numBuffers );

				// get prevJob bufferSet
				WwsJob_BufferSet prevJobBufferSet = pPrevJobBufferSets[bufferSetNum];

				// get ptr to first buffer in prevJob bufferSet
				// (garbage is ignored if prevJob bufferSet is inactive)
				WwsJob_Buffer *pPrevJobBuffers = &g_WwsJob_bufferArray[prevJobBufferSet.m_firstBufferNum];

				// get ptr to prevJob buffer
				// (garbage is ignored if prevJob bufferSet is inactive)
				WwsJob_Buffer *pPrevJobBuffer = &pPrevJobBuffers[bufferNum];

				// get prevJob buffer
				// (garbage is ignored if prevJob bufferSet is inactive)
				WwsJob_Buffer prevJobBuffer = *pPrevJobBuffer;

				// see if prevJob bufferSet page configuration is compatible with bufferSet
				// Note: we don't care if the number of buffers is the same or not
				Bool32 prevJobConfigurationCompatible;
				prevJobConfigurationCompatible =
					(prevJobBufferSet.m_firstPageNum == bufferSet.m_firstPageNum) &&
					(prevJobBufferSet.m_numPagesPerBuffer == bufferSet.m_numPagesPerBuffer);

				STORE_VERBOSE_AUDIT( AuditId::kWwsJob_CopyStoreShareBufsToRun_prevJobBufSet, prevJobConfigurationCompatible,
					(U64)prevJobBufferSet.m_u32, prevJobBuffer.m_u64 );

				// if prevJob configuration matches and prevJob buffer is shareable
				// Note: if prevJob bufferSet is inactive then prevJobConfigurationCompatible is false
				//		and prevJobBuffer is ignored
				if( prevJobConfigurationCompatible && prevJobBuffer.m_shareable )
				{	// prevJob configuration matches and prevJob buffer is shareable

					STORE_VERBOSE_AUDIT( AuditId::kWwsJob_CopyStoreShareBufsToRun_copyShareStoreBufToRun );

					// for buffer:
					//		set to prevJob buffer
					// 		(keep shareableWriteIfDiscarded)
					//      used <- 0
					//		(keep tag)
					buffer = prevJobBuffer;
					buffer.m_used = 0;
					*pBuffer = buffer;

					// We've promoted a shareable buffer into this job
					g_WwsJob_jobDataArray[g_WwsJob_runJobNum].m_jobHasShareableBuffers = WWSJOB_TRUE;

					// if prevJob buffer is used
					if( prevJobBuffer.m_used )
					{	// prevJob buffer is used

						UNTESTED_CODE();

						STORE_VERBOSE_AUDIT( AuditId::kWwsJob_CopyStoreShareBufsToRun_prevJobBufUsed );

						// for prevJob buffer:
						//		shareable <- 0
						//		shareableWriteIfDiscarded <- 0
						//      tag <- 0
						prevJobBuffer.m_shareable = 0;
						prevJobBuffer.m_shareableWriteIfDiscarded = 0;
						prevJobBuffer.m_dmaTagId = 0;
						*pPrevJobBuffer = prevJobBuffer;
					}
					else
					{	// prevJob buffer is not used

						// prevJob buffer <- inactive
						prevJobBuffer.m_u32[0] = 0;
						*pPrevJobBuffer = prevJobBuffer;
					}
					STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_CopyStoreShareBufsToRun_afterCopy, (bufferSetNum << 8) | bufferNum,
						buffer.m_u64, prevJobBuffer.m_u64 );
				}
			}

			++pBuffer;
			++bufferIndex;
		} while(bufferIndex != firstFreeBufferNum);
		// this is the end of what used to be the WwsJob_CopyStoreShareBufsToRun( ) routine

#else // #if x // 1 = new 2 single loop code, 0 = old 1 more complex loop code

		// get prevJob first bufferSetNum
		U32 prevJobBufferSetNum = g_WwsJob_lastStoreJobNum * WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB;

		// get prevJob ptr to first bufferSet
		WwsJob_BufferSet *pPrevJobBufferSet = &g_WwsJob_bufferSetArray[prevJobBufferSetNum];

		/*	Jon, there is no easy way to get the entire code below into convenient single loops
		My suggestion is to do the following:

		pBufferSet--, pPrevJobBufferSet--, bufferSetNum--
		BufferSetNumLoopContinueRepredict:
		predict "predominant" main loop below
		BufferSetNumLoopContinue: ;===================================================================
		pBufferSet++, pPrevJobBufferSet++, bufferSetNum++
		if end of bufferSetNum's then branch to EndOfBufferSetNumLoop (not predicted)
		get bufferSet, ?pBuffers?, prevJobBufferSet
		if bufferSet is not active and prevJob bufferSet is active
		branch to codeA (which is rare and thus not predicted).
		Note codeA may rejoin below at CodeAMayJoinFlowHere
		if bufferSet is not active
		branch to BufferSetNumLoop above
		NOTE: this is the "predominant" case so this branch is predicted
		;==================================================================================
		CodeAMayJoinFlowHere:
		(Note this downcoded flow is commented further below)

		CodeA:
		predict call for getPageMask (unless it's inlined)
		get prevJobNumBuffers
		if( prevJobNumBuffers > g_WwsJob_numFreeBuffers )
		goto BufferSetNumLoopContinueRepredict above (rare and thus not predicted)
		Call routine getPageMask (or inline it)
		if( totalPageMask & pageMask )
		goto BufferSetNumLoopContinueRepredict above (not predicted, although maybe it should be???)
		copy prevJob bufferSet to bufferSet (etc)
		Note this has a tiny loop in it unless you branch to entry in fall-thru code
		goto CodeAMayJoinFlowHere above (predict this if possible)

		EndOfBufferSetNumLoop:
		return
		*/

		// for all bufferSets
		for( bufferSetNum = 0 ; bufferSetNum < WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB ;
			pBufferSet++, pPrevJobBufferSet++, bufferSetNum++ )
		{
			// get bufferSet
			WwsJob_BufferSet bufferSet = *pBufferSet;

			// get prevJob bufferSet
			WwsJob_BufferSet prevJobBufferSet = *pPrevJobBufferSet;

			STORE_VERBOSE_AUDIT( AuditId::kWwsJob_CopyStoreShareBufsToRun_bufSet, bufferSetNum,
				( (U64)bufferSet.m_u32 << 32 ) | prevJobBufferSet.m_u32 );

			// if bufferSet is not active
			if( !bufferSet.m_u32 )
			{	// bufferSet is not active

				// if prevJob bufferSet is active
				if ( prevJobBufferSet.m_u32 )
				{	// prevJob bufferSet is active
					// Note: this is what I called codeA in the downcoding comments above

					// we must check if prevJob bufferSet pages collide with pages in any bufferSet

					// get prevJob #buffers total
					U32 prevJobNumBuffers = prevJobBufferSet.m_numBuffers;

					// if there are enough buffers to promote the prevJob buffers
					if( prevJobNumBuffers <= g_WwsJob_numFreeBuffers )
					{	// there are enough buffers to promote the prevJob buffers

						// get prevJob bufferSet pageMasks
						QuadWord pageMask[2];
						WwsJob_GetPageMasks( prevJobBufferSet.m_firstPageNum,
							prevJobBufferSet.m_numPagesPerBuffer * prevJobNumBuffers, &pageMask[0] );

						// if prevJob bufferSet pages collide with pages in any bufferSet
#ifdef CHRIS_DOWNCODING
						VU32 and0 = spu_and( totalPageMask[0].m_vu32, pageMask[0].m_vu32 );
						VU32 and1 = spu_and( totalPageMask[1].m_vu32, pageMask[1].m_vu32 );
						VU32 combined = spu_or( and0, and1 );
						if ( si_to_uint( (VI8) spu_orx( combined ) ) )
						{
							STORE_VERBOSE_AUDIT( AuditId::kWwsJob_CopyStoreShareBufsToRun_storeBufSetCollides );
							goto NextBufferSet;
						}
#else
						for( U32 i = 0 ; i < 2 ; i++ )
						{
							for( U32 j = 0 ; j < 2 ; j++ )
							{
								// if prevJob bufferSet pages collide with pages in any bufferSet
								if( totalPageMask[i].m_u64[j] & pageMask[i].m_u64[j] )
								{	// prevJob bufferSet pages collide with pages in any bufferSet

									// get next bufferSet (can't use "continue" since we're inside 'for' loops here
									STORE_VERBOSE_AUDIT( AuditId::kWwsJob_CopyStoreShareBufsToRun_storeBufSetCollides );
									goto NextBufferSet;
								}
							}
						}
#endif

						// prevJob bufferSet pages do *not* collide with pages in any bufferSet
						// copy prevJob bufferSet to bufferSet
						// allocate buffers
						bufferSet = prevJobBufferSet/*ignore m_firstBufferNum*/;
						bufferSet.m_firstBufferNum = g_WwsJob_firstBufferNum;
						STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_CopyStoreShareBufsToRun_copyBufSetToRun, bufferSetNum,
							bufferSet.m_u32 );
						*pBufferSet = bufferSet;
						g_WwsJob_firstBufferNum += prevJobNumBuffers;
						g_WwsJob_numFreeBuffers -= prevJobNumBuffers;

						// get ptr to first buffer in bufferSet
						WwsJob_Buffer *pBuffer = &g_WwsJob_bufferArray[bufferSet.m_firstBufferNum];

						// declare all buffers are inactive
						// Note: data already zero'd for inactive bufferSet
						//WwsJob_SetDwordMem( (U64*)pBuffer, sizeof(WwsJob_Buffer) * prevJobNumBuffers, 0 );

						// for all buffers, declare buffer is reserved, and set bufferSetNum
						WwsJob_Buffer buffer;
						buffer.m_u64 = 0;
						buffer.m_reserved = 1;
						buffer.m_bufferSetNum = bufferSetNum;
						for( U32 bufferNum = 0 ; bufferNum < prevJobNumBuffers ; pBuffer++, bufferNum++ )
						{
							// declare buffer is reserved, and set bufferSetNum
							*pBuffer = buffer;
						}

						// Note: we will rejoin code below at what I called CodeAMayJoinFlowHere for downcoding
					}
					else
					{	// there are not enough buffers to promote the prevJob buffers
						// this bufferSet will remain inactive

						UNTESTED_CODE();

						STORE_VERBOSE_AUDIT( AuditId::kWwsJob_CopyStoreShareBufsToRun_tooFewBufsToPromote );

						// get next bufferSet
						continue;
					}
				}
				else
				{   // prevJob bufferSet is not active

					// get next bufferSet
					continue;
				}
			}
			// runJob bufferSet was either active, or is now active after above code
			// For downcoding, this is what I called CodeAMayJoinFlowHere

			/*	My suggestion for downcoding this section is:

			Do setup for bufferNum loop, including predicting frequent loop below
			pBuffer--, pPrevJobBuffer--, bufferNum--
			BufferNumLoopContinue: ;=====================================================
			pBuffer++, pPrevJobBuffer++, bufferNum++
			if this is last bufferNum
			goto BufferSetNumLoopContinueRepredict above (not predicted)
			Get buffer and prevJobBuffer
			If !( buffer is reserved && prevJobConfigurationCompatible && prevJobBuffer.m_shareable )
			branch to BufferNumLoopContinue above (very frequent, so it is predicted)
			;=========================================================================
			Rest of code here is so rare it's not that critical
			*/

			// get page configuration (just get what's needed)
			//bufferSetFirstPageNum		= bufferSet.m_firstPageNum;
			//numPagesPerBuffer	= bufferSet.m_numPagesPerBuffer;
			U32 numBuffers	= bufferSet.m_numBuffers;

			// see if prevJob bufferSet page configuration is compatible with bufferSet
			// Note: we don't care if the number of buffers is the same or not
			Bool32 prevJobConfigurationCompatible;
			prevJobConfigurationCompatible =
				(prevJobBufferSet.m_firstPageNum == bufferSet.m_firstPageNum) &&
				(prevJobBufferSet.m_numPagesPerBuffer == bufferSet.m_numPagesPerBuffer);

			// get ptr to first buffer of bufferSet
			WwsJob_Buffer *pBuffer = &g_WwsJob_bufferArray[bufferSet.m_firstBufferNum];

			// get ptr to prevJob first buffer (garbage if prevJob bufferSet is inactive)
			WwsJob_Buffer *pPrevJobBuffer = &g_WwsJob_bufferArray[prevJobBufferSet.m_firstBufferNum];

			// for all buffers
			for( U32 bufferNum = 0 ; bufferNum < numBuffers ; pBuffer++, pPrevJobBuffer++, bufferNum++ )
			{
				// get buffer
				WwsJob_Buffer buffer = *pBuffer;

				// if buffer is not reserved
				if( !buffer.m_reserved )
				{	// buffer is not reserved

					// get next buffer
					continue;
				}

				// get prevJob buffer (garbage if prevJob bufferSet is inactive)
				WwsJob_Buffer prevJobBuffer = *pPrevJobBuffer;

				STORE_VERBOSE_AUDIT( AuditId::kWwsJob_CopyStoreShareBufsToRun_runBufReserved, bufferNum,
					prevJobBuffer.m_u64 );

				// if prevJob configuration matches and prevJob buffer is shareable
				// Note: if prevJob bufferSet is inactive then prevJobConfigurationCompatible is false
				//		and prevJobBuffer is ignored
				if( prevJobConfigurationCompatible && prevJobBuffer.m_shareable )
				{	// prevJob configuration matches and prevJob buffer is shareable

					STORE_VERBOSE_AUDIT( AuditId::kWwsJob_CopyStoreShareBufsToRun_copyShareStoreBufToRun );

					// for buffer:
					//		set to prevJob buffer
					// 		(keep shareableWriteIfDiscarded)
					//      used <- 0
					//		(keep tag)
					buffer = prevJobBuffer;
					buffer.m_used = 0;
					*pBuffer = buffer;

					// We've promoted a shareable buffer into this job
					g_WwsJob_jobDataArray[g_WwsJob_runJobNum].m_jobHasShareableBuffers = WWSJOB_TRUE;

					// if prevJob buffer is used
					if( prevJobBuffer.m_used )
					{	// prevJob buffer is used

						UNTESTED_CODE();

						STORE_VERBOSE_AUDIT( AuditId::kWwsJob_CopyStoreShareBufsToRun_prevJobBufUsed );

						// for prevJob buffer:
						//		shareable <- 0
						//		shareableWriteIfDiscarded <- 0
						//      tag <- 0
						prevJobBuffer.m_shareable = 0;
						prevJobBuffer.m_shareableWriteIfDiscarded = 0;
						prevJobBuffer.m_dmaTagId = 0;
						*pPrevJobBuffer = prevJobBuffer;
					}
					else
					{	// prevJob buffer is not used

						// prevJob buffer <- inactive
						prevJobBuffer.m_u32[0] = 0;
						*pPrevJobBuffer = prevJobBuffer;
					}
					STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_CopyStoreShareBufsToRun_afterCopy, (bufferSetNum << 8) | bufferNum,
						buffer.m_u64, prevJobBuffer.m_u64 );
				}
			}

NextBufferSet:
			; // you can't put any code here

		}
		// this is the end of what used to be the WwsJob_CopyStoreShareBufsToRun( ) routine

#endif // #if x // 1 = new 2 single loop code, 0 = old 1 more complex loop code
	}

	// save first free buffer# (with job#) for runJob (it won't change now)
	g_WwsJob_jobDataArray[g_WwsJob_runJobNum].m_firstFreeBufferNum = g_WwsJob_firstBufferNum;

	// Try to clear shared buffer states & pages.
	// Note: if everything could not be cleared, it will start an interrupt which will finish the clearing
	(void)WwsJob_TryDumpAllStoreShareBufs();

	// print out bufferSet info
	STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_JobApi_bufferSets, WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB >> 1,
		(U64*)&g_WwsJob_bufferSetArray[g_WwsJob_runJobNum * WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB] );

	// print out all logical to buffer info
	STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_JobApi_logicalToBufNums, WWSJOB_MAX_NUM_BUFFERS_PER_JOB >> 3,
		(U64*)&g_WwsJob_logicalToBufferNumArray[g_WwsJob_runJobNum * WWSJOB_MAX_NUM_BUFFERS_PER_JOB] );

	// print out all buffers for job
	STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_JobApi_buffers, g_WwsJob_firstBufferNum & (WWSJOB_MAX_NUM_BUFFERS_PER_JOB - 1),
		(U64*)&g_WwsJob_bufferArray[g_WwsJob_runJobNum * WWSJOB_MAX_NUM_BUFFERS_PER_JOB] );

	// Poll Spurs Kernel to see which PM goes next, and set that workload.  Return true if the Wws jobManager runs and a job is taken
	if( WwsJob_PollSpursKernelQuick() )
	{	// our jobManager does next job

#if ENABLE_JOB_PIPELINE
		// Try to change a free job to a load job.
		(void)WwsJob_TryChangeFreeToLoadJob();
#endif // #if ENABLE_JOB_PIPELINE

	}
	else
	{	// our jobManager does *not* do next job

		STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_JobApi_stopLoadingJobs );

		// declare we are not to load another job
		// (note we must preserve the low bits in case we change our mind)
		g_WwsJob_nextLoadJobNum |= 0xFFFFFFF0;
	}

	STORE_TIMING_AUDIT( AuditId::kWwsJob_JobApi_end );
}

// Free a logical (non-shared) buffer which the job has used (but don't execute load commands)
void WwsJob_JobApi_FreeLogicalBuffer( U32 /*unused*/, U32 logicalBufferSetNum, U32 logicalBufferNum )
{
	//__asm__ volatile ("stopd $0, $0, $0");
	WWSJOB_ASSERT( !AreInterruptsEnabled() );	//The user should have interrupts disabled when entering the job manager
	WWSJOB_VERBOSE_ASSERT( g_WwsJob_runJobNum >= 0  &&  g_WwsJob_runJobNum <= 2);
	STORE_TIMING_AUDIT( AuditId::kWwsJob_JobApi_begin );

	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_JobApi_freeLogicalBuffer );

	WwsJob_FreeLogicalBuffer( g_WwsJob_runJobNum, logicalBufferSetNum, logicalBufferNum );

	// if we made a change that might allow the load commands to execute, and they need executing
	if( g_WwsJob_loadJobState == WwsJob_LoadJobState::kExecuteCommands )
	{	// we made a change that might allow the load commands to execute, and they need executing

		STORE_VERBOSE_AUDIT( AuditId::kWwsJob_JobApi_tryToExecuteLoadCommands );

		// try to execute the load commands
		WwsJob_TryExecuteLoadCmds();
	}

	STORE_TIMING_AUDIT( AuditId::kWwsJob_JobApi_end );
}

// Free multiple logical non-shared bufferNum pairs (bufferSetNum, bufferNum) which the job has used
void WwsJob_JobApi_FreeLogicalBuffers( const U8* pLogicalBufferNumPairs, U32 numLogicalBufferNumPairs )
{
	//__asm__ volatile ("stopd $0, $0, $0");
	WWSJOB_ASSERT( !AreInterruptsEnabled() );	//The user should have interrupts disabled when entering the job manager
	WWSJOB_VERBOSE_ASSERT( g_WwsJob_runJobNum >= 0  &&  g_WwsJob_runJobNum <= 2);
	STORE_TIMING_AUDIT( AuditId::kWwsJob_JobApi_begin );

	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_JobApi_freeLogicalBuffers, 0, ( (U64)pLogicalBufferNumPairs << 32 ) | numLogicalBufferNumPairs );

	// for all bufferNum pairs
	for( U32 numPairs = 0 ; numPairs < numLogicalBufferNumPairs ; numPairs++ )
	{
		U32 logicalBufferSetNum = *pLogicalBufferNumPairs++;
		U32 logicalBufferNum	= *pLogicalBufferNumPairs++;

		// Free a logical (non-shared) buffer which the job has used (but don't execute load commands)
		WwsJob_FreeLogicalBuffer( g_WwsJob_runJobNum, logicalBufferSetNum, logicalBufferNum );
	}

	// if we made a change that might allow the load commands to execute, and they need executing
	if( g_WwsJob_loadJobState == WwsJob_LoadJobState::kExecuteCommands )
	{	// we made a change that might allow the load commands to execute, and they need executing

		STORE_VERBOSE_AUDIT( AuditId::kWwsJob_JobApi_tryToExecuteLoadCommands );

		// try to execute the load commands
		WwsJob_TryExecuteLoadCmds();
	}

	STORE_TIMING_AUDIT( AuditId::kWwsJob_JobApi_end );
}

void WwsJob_JobApi_StoreAudit( U32 parameter0, U32 parameter1, U32 parameter2 )
{
	//__asm__ volatile ("stopd $0, $0, $0");
	WWSJOB_ASSERT( !AreInterruptsEnabled() );	//The user should have interrupts disabled when entering the job manager
	WWSJOB_VERBOSE_ASSERT( g_WwsJob_runJobNum >= 0  &&  g_WwsJob_runJobNum <= 2);

	// Store a user audit
	// parameter0: (auditId << 16) | hword of user data		(ignored if parameter1 bit 0 is 0)
	// parameter1: bit0 = 1 if hword is data and 1:6 dwords.  Lo 31 bits = #dwords
	// parameter2: pDwords
	// return: none

	if ( AreJobAuditsEnabled() )
	{
		U16 auditId = parameter0 >> 16;
		U16 hword   = parameter0 & 0xFFFF;
		U32 numDwords = parameter1 & 0x7FFFFFFF;
		U64 *pDwords = (U64*)parameter2;
		if( (U32)pDwords == 0 )
		{
			WWSJOB_VERBOSE_ASSERT( parameter1/*bit0 & #dwords*/ == 0 );
			StoreAuditInternal( auditId, hword );
		}
		else if( (I32)parameter1 < 0 )
		{	// hword has data, and there are 1:6 dwords

			WWSJOB_VERBOSE_ASSERT( numDwords >= 1  &&  numDwords <= 6 );
			StoreAuditInternal( auditId, hword, numDwords, pDwords[0], pDwords[1], pDwords[2], pDwords[3], pDwords[4], pDwords[5] );
		}
		else
		{
			WWSJOB_VERBOSE_ASSERT( numDwords >= 1 );
			StoreAuditInternal( auditId, numDwords, pDwords );
		}
	}
}

//====================================================================================================================
//
//	code for when a runJob allows the loadJob to start
//		the prevJob will pass on any compatible shareable buffers to the runJob
//		the prevJob will then discard all shareable buffers
//
//====================================================================================================================

/**	\brief	Clear all storeJob shareable buffers

	First call WwsJob_CopyStoreShareBufsToRun() to copy all the last storeJob shareable buffers to the runJob.
	Any shareable buffers that could not be copied (due to conflict) are cleared by this routine.
	g_WwsJob_lastStoreJobNum specifies the jobNum.
	This is called from main(), WwsJob_JobApi, or interrupt.
**/
#if WWS_JOB_USE_C_VERSION!=0
Bool32 WwsJob_TryDumpAllStoreShareBufs( void )
{
	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryDumpAllStoreShareBufs );

	// assume no discards are done (may change)
	Bool32 ongoingDma = WWSJOB_FALSE;

#if 1 // 1 = new single loop version, 0 = old double loop version

  #if 1 // 1 = Jon's downcoded version, 0 = old single loop version

	// if the last storeJob never existed
	if( !g_WwsJob_jobDataArray[g_WwsJob_lastStoreJobNum].m_jobHasShareableBuffers )
	{
		// then there is nothing to do
		return WWSJOB_TRUE;
	}

    // get ptr to bufferSets
    U32 lastStoreJobNum = g_WwsJob_lastStoreJobNum;
    WwsJob_BufferSet *pBufferSets = &g_WwsJob_bufferSetArray[lastStoreJobNum * WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB];

    // get ptr to first buffer
    U32 bufferIndex/*including job#*/ = lastStoreJobNum * WWSJOB_MAX_NUM_BUFFERS_PER_JOB;
    WwsJob_Buffer *pBuffer = &g_WwsJob_bufferArray[bufferIndex];

    // get first free bufferNum (with job#)
    U32 firstFreeBufferNum = g_WwsJob_jobDataArray[lastStoreJobNum].m_firstFreeBufferNum;

    // loop for all buffers
    do
    {
        // get buffer
        WwsJob_Buffer buffer = *pBuffer;

        STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryDumpAllStoreShareBufs_buf, bufferIndex,
                buffer.m_u64 );

        // if buffer is shareable
        if( buffer.m_shareable  )
        {
            // buffer can't be reserved
            // runJob buffer reservations were removed when runJob allowed loadJob to start
            WWSJOB_VERBOSE_ASSERT( !buffer.m_reserved );

            // get bufferSet#
            U32 bufferSetNum = buffer.m_bufferSetNum;

            // get bufferSet
            WwsJob_BufferSet bufferSet = pBufferSets[bufferSetNum];

            // bufferSet must be active
            WWSJOB_VERBOSE_ASSERT( bufferSet.m_u32 );

            // get bufferNum
            U32 bufferNum = bufferIndex - bufferSet.m_firstBufferNum/*withJob#*/;

            STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryDumpAllStoreShareBufs_activeBuf, bufferSetNum,
                    ( (U64)bufferSet.m_u32 << 32) | bufferNum );

            // bufferNum must be valid
            WWSJOB_ASSERT( bufferNum < bufferSet.m_numBuffers );

            // get bufferSet page configuration
            U32 bufferSetFirstPageNum    = bufferSet.m_firstPageNum;
            U32 numPagesPerBuffer        = bufferSet.m_numPagesPerBuffer;
            //U32 numBuffers                = bufferSet.m_numBuffers;

            STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_TryDumpAllStoreShareBufs_activeShareBuffer, (bufferSetNum << 8) | bufferNum,
                    buffer.m_u64 );

            // Try to discard the shareable buffer.  If it could not be discarded
            U32 bufferPageNum = bufferSetFirstPageNum + numPagesPerBuffer * bufferNum;
            ongoingDma |= !WwsJob_TryDumpShareBuf( bufferSet, pBuffer, buffer, bufferPageNum, DmaTagId::kStoreJob_writeAllShareableBuffers );
        }

        ++pBuffer;
        ++bufferIndex;
    } while(bufferIndex != firstFreeBufferNum);

  #else // #if x // 1 = Jon's downcoded version, 0 = old single loop version

	// get ptr to bufferSets
	WwsJob_BufferSet *pBufferSets = &g_WwsJob_bufferSetArray[g_WwsJob_lastStoreJobNum * WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB];

	// get ptr to first buffer
	U32 bufferIndex/*including job#*/ = g_WwsJob_lastStoreJobNum * WWSJOB_MAX_NUM_BUFFERS_PER_JOB;
	WwsJob_Buffer *pBuffer = &g_WwsJob_bufferArray[bufferIndex];

	// get first free bufferNum (with job#)
	U32 firstFreeBufferNum = g_WwsJob_jobDataArray[g_WwsJob_lastStoreJobNum].m_firstFreeBufferNum;

	// loop for all buffers
	for( /*no init*/ ; bufferIndex < firstFreeBufferNum ; bufferIndex++, pBuffer++ )
	{
		// get buffer
		WwsJob_Buffer buffer = *pBuffer;

		STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryDumpAllStoreShareBufs_buf, bufferIndex,
				buffer.m_u64 );

		// if buffer is active
		if( buffer.m_u32[0] )
		{	// buffer is active

			// buffer can't be reserved
			// runJob buffer reservations were removed in WwsJob_TryChangeStoreToFreeJob
			WWSJOB_VERBOSE_ASSERT( !buffer.m_reserved );

			// get bufferSet#
			U32 bufferSetNum = buffer.m_bufferSetNum;

			// get bufferSet
			WwsJob_BufferSet bufferSet = pBufferSets[bufferSetNum];

			// bufferSet must be active
			WWSJOB_VERBOSE_ASSERT( bufferSet.m_u32 );

			// if buffer is not shareable
			if( !buffer.m_shareable )
			{	// buffer is not shareable

				// get next buffer
				continue;
			}

			// get bufferNum
			U32 bufferNum = bufferIndex - bufferSet.m_firstBufferNum/*withJob#*/;

			STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryDumpAllStoreShareBufs_activeBuf, bufferSetNum,
					( (U64)bufferSet.m_u32 << 32) | bufferNum );

			// bufferNum must be valid
			WWSJOB_ASSERT( bufferNum < bufferSet.m_numBuffers );

			// get bufferSet page configuration
			U32 bufferSetFirstPageNum	= bufferSet.m_firstPageNum;
			U32 numPagesPerBuffer		= bufferSet.m_numPagesPerBuffer;
			//U32 numBuffers				= bufferSet.m_numBuffers;

			STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_TryDumpAllStoreShareBufs_activeShareBuffer, (bufferSetNum << 8) | bufferNum,
					buffer.m_u64 );

			// Try to discard the shareable buffer.  If it could not be discarded
			U32 bufferPageNum = bufferSetFirstPageNum + numPagesPerBuffer * bufferNum;
			if( !WwsJob_TryDumpShareBuf( bufferSet, pBuffer, buffer, bufferPageNum,
					DmaTagId::kStoreJob_writeAllShareableBuffers ) )
			{	// It could not be discarded

				// then there is ongoing (job or discard) dma
				ongoingDma = WWSJOB_TRUE;
			}
		}
	}

  #endif // #if x // 1 = Jon's downcoded version, 0 = old single loop version

#else // #if x // 1 = new single loop version, 0 = old double loop version

	// get first bufferSetNum
	U32 bufferSetNum = g_WwsJob_lastStoreJobNum * WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB;

	// get ptr to first bufferSet
	WwsJob_BufferSet *pBufferSet = &g_WwsJob_bufferSetArray[bufferSetNum];

	// for all bufferSets
	for( bufferSetNum = 0 ; bufferSetNum < WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB ; pBufferSet++, bufferSetNum++ )
	{
		// get bufferSet
		WwsJob_BufferSet bufferSet = *pBufferSet;

		// if bufferSet is not active
		if( bufferSet.m_u32 == 0 )
		{	// bufferSet is not active

			// get next bufferSet
			continue;
		}

		// audit removed when single loop version added
		//STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryDumpAllStoreShareBufs_activeBufSet, bufferSetNum,
		//		bufferSet.m_u32 );

		// get bufferSet page configuration
		U32 bufferSetFirstPageNum	= bufferSet.m_firstPageNum;
		U32 numPagesPerBuffer		= bufferSet.m_numPagesPerBuffer;
		U32 numBuffers				= bufferSet.m_numBuffers;

		// get ptr to first buffer
		WwsJob_Buffer *pBuffer = &g_WwsJob_bufferArray[bufferSet.m_firstBufferNum];

		// for all buffers
		for( U32 bufferNum = 0 ; bufferNum < numBuffers ; pBuffer++, bufferNum++ )
		{
			// get buffer
			WwsJob_Buffer buffer = *pBuffer;

			// if buffer is inactive
			if( !buffer.m_u32[0] )
			{	// buffer is inactive

				// get next buffer
				continue;
			}

			// buffer can't be reserved, since reservations are cleared in WwsJob_TryChangeStoreToFreeJob
			WWSJOB_VERBOSE_ASSERT( !buffer.m_reserved );

			// if buffer is not shareable
			if( !buffer.m_shareable )
			{	// buffer is not shareable

				// get next buffer
				continue;
			}

			STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_TryDumpAllStoreShareBufs_activeShareBuffer, (bufferSetNum << 8) | bufferNum,
					buffer.m_u64 );

			// Try to discard the shareable buffer.  If it could not be discarded
			U32 bufferPageNum = bufferSetFirstPageNum + numPagesPerBuffer * bufferNum;
			if( !WwsJob_TryDumpShareBuf( bufferSet, pBuffer, buffer, bufferPageNum,
					DmaTagId::kStoreJob_writeAllShareableBuffers ) )
			{	// It could not be discarded

				// then there is ongoing (job or discard) dma
				ongoingDma = WWSJOB_TRUE;
			}
		}
	}

#endif // #if x // 1 = new single loop version, 0 = old double loop version

	// if there is ongoing (job or discard) dma
	if( ongoingDma )
	{	// there is ongoing (job or discard) dma

		STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryDumpAllStoreShareBufs_outputWriteAllShareBufsInt );

		// output barriered dma with stall & notify interrupt
		//This interrupt fires after all dmas on DmaTagId::kStoreJob_writeAllShareableBuffers have completed
		WwsJob_StartTagSpecificBarrieredNullListWithInterrupt( DmaTagId::kStoreJob_writeAllShareableBuffers );

		// we could not clear all shared stuff
		return WWSJOB_FALSE;
	}
	else
	{	// there is no ongoing dma

		// we cleared all shared stuff
		return WWSJOB_TRUE;
	}
}
#endif


/**	\brief	Try to discard a run or storeJob buffer

	Try to discard a runJob or last storeJob shareable buffer.
	Return WWSJOB_TRUE if done, or WWSJOB_FALSE if job dma or discard dma is ongoing.
	Called from main(), WwsJob_JobApi, or interrupt.

	\param	bufferSet = data for the bufferSet
	\param	pBuffer = ptr to buffer entry in g_WwsJob_bufferArray[]
	\param	buffer = data of buffer entry in g_WwsJob_bufferArray[]
	\param	bufferPageNum = first page# (1K per page) of buffer
	\param	dmaTagId = tag to use if the buffer must be written to main memory
**/
#if WWS_JOB_USE_C_VERSION!=0
Bool32 WwsJob_TryDumpShareBuf( WwsJob_BufferSet bufferSet, WwsJob_Buffer *pBuffer, WwsJob_Buffer buffer, U32 bufferPageNum, U32 dmaTagId )
{
	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryDumpShareBuf, 0,
			buffer.m_u64 );

	// if write if discard is on
	if( buffer.m_shareableWriteIfDiscarded )
	{	// write if discard is on

		U32 lsAddress = bufferPageNum << 10;
		U32 mmAddress = buffer.m_mmAddressInQwords << 4;
		U32 mmLength  = buffer.m_mmLengthInQwords  << 4;

		//NOTE: this important audit could not be included in the downcoded version due to
		// size and register coloring issues:
		STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_TryDumpShareBuf_writeShareBuf, (bufferPageNum << 8) | bufferSet.m_numPagesPerBuffer,
				( (U64)mmLength << 40 ) | ( (U64)dmaTagId << 32 ) | mmAddress );

		// do write of buffer, which can be > 16K
		do
		{
			// max of 16K per dma
			U32 length = (mmLength > 0x4000) ? 0x4000 : mmLength;

			// start dma of buffer to main memory
			DMA_WRITE( lsAddress, mmAddress, length, dmaTagId )
			lsAddress += length;
			mmAddress += length;
			mmLength  -= length;
		}while( mmLength > 0 );

		// write if discard <- 0
		buffer.m_shareableWriteIfDiscarded = 0;
		*pBuffer = buffer;

		STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryDumpShareBuf_clearWriteIfDiscard, 0,
				buffer.m_u64 );

		// a shareable discard is started
		return WWSJOB_FALSE;
	}
	else
	{	// write if discard is not on

		// if unique dmaTagId exists
		U32 uniqueDmaTagId = buffer.m_dmaTagId;
		if( uniqueDmaTagId )
		{	// unique dmaTagId exists

			STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryDumpShareBuf_bufDmaTagIdExists, uniqueDmaTagId );

			// if job dma is done
			if( WwsJob_IsDmaTagMaskDone( 1 << uniqueDmaTagId ) )
			{	// job dma is done

				// free dmaTagId
				WwsJob_FreeDmaTagId( uniqueDmaTagId );
			}
			else
			{	// job dma is not done

				UNTESTED_CODE();

				STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryDumpShareBuf_ongoingDma );

				// job dma is ongoing
				return WWSJOB_FALSE;
			}

		}

		// buffer <- inactive
		buffer.m_u32[0] = 0;
		*pBuffer = buffer;

		STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryDumpShareBuf_inactivateBuf, 0,
				buffer.m_u64 );

		// get pageMasks
		U32 numPagesPerBuffer = bufferSet.m_numPagesPerBuffer;
		QuadWord pageMask[2];
		WwsJob_GetPageMasks( bufferPageNum, numPagesPerBuffer, &pageMask[0] );

		// clear shareable pages
#ifdef CHRIS_DOWNCODING
		g_WwsJob_shareablePageMask[0].m_vu32 = spu_andc( g_WwsJob_shareablePageMask[0].m_vu32, pageMask[0].m_vu32 );
		g_WwsJob_shareablePageMask[1].m_vu32 = spu_andc( g_WwsJob_shareablePageMask[1].m_vu32, pageMask[1].m_vu32 );
#else
		for( U32 i = 0 ; i < 2 ; i++ )
		{
			for( U32 j = 0 ; j < 2 ; j++ )
			{
				g_WwsJob_shareablePageMask[i].m_u64[j] &= ~pageMask[i].m_u64[j];
			}
		}
#endif

  		STORE_VERBOSE_AUDIT( AuditId::kWwsJob_shareablePages, 0,
				g_WwsJob_shareablePageMask[0].m_u64[0], g_WwsJob_shareablePageMask[0].m_u64[1],
				g_WwsJob_shareablePageMask[1].m_u64[0], g_WwsJob_shareablePageMask[1].m_u64[1] );

		// done (no ongoing dma)
		return WWSJOB_TRUE;
	}
}
#endif


/**	\brief	Try to change a free job to a load job

	Call SpursKernel to see if we have a workload job to do.
	If so, check if it is a normal job or general barrier.
	If a normal job, take job, init it, start read of loadJob commands.
	If a general barrier, ensure no outstanding dependencies before taking job.
	This is called from main() or WwsJob_JobApi.
**/
#if WWS_JOB_USE_C_VERSION!=0
void WwsJob_TryChangeFreeToLoadJob( void )
{
	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryChangeFreeToLoadJob );

	// we should not have a loadJob now
	WWSJOB_VERBOSE_ASSERT( g_WwsJob_loadJobNum == -1 );
	WWSJOB_VERBOSE_ASSERT( g_WwsJob_loadJobState == WwsJob_LoadJobState::kNone );

	// if there is a run job, it must have allowed loading the next job
	WWSJOB_ASSERT( g_WwsJob_runJobState != WwsJob_RunJobState::kLoadNotAllowed );

	// ensure we are allowed to get the next job
	WWSJOB_VERBOSE_ASSERT( g_WwsJob_nextLoadJobNum >= 0  &&  g_WwsJob_nextLoadJobNum <= 2 );

	// if job was taken (if so set g_WwsJob_jobIndex)
	if( WwsJob_GetJobFromJobList() )
	{	// job was taken (set g_WwsJob_jobIndex)

		switch( g_WwsJob_jobHeader.m_jobHeaderCommand )
		{

		case JobHeaderCommand::kJobExists:

			// get the next load jobNum
			// declare we are loading the commands
			g_WwsJob_loadJobNum = g_WwsJob_nextLoadJobNum;
			g_WwsJob_loadJobState = WwsJob_LoadJobState::kReadCommands;

			// declare this job has no shareable buffers yet
			g_WwsJob_jobDataArray[g_WwsJob_loadJobNum].m_jobHasShareableBuffers = WWSJOB_FALSE;

			STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryChangeFreeToLoadJob_jobExists, g_WwsJob_loadJobNum,
					*(U64*)&g_WwsJob_jobHeader );

			// get next load jobNum
			g_WwsJob_nextLoadJobNum++;
			if( g_WwsJob_nextLoadJobNum > 2 )
				g_WwsJob_nextLoadJobNum = 0;

			// remember job data
			g_WwsJob_jobDataArray[g_WwsJob_loadJobNum].m_eaWorkLoad = g_WwsJob_eaWorkLoad;
			g_WwsJob_jobDataArray[g_WwsJob_loadJobNum].m_jobIndex   = g_WwsJob_jobIndex;

			// size of load commands must not overflow buffer, which must be <= 16K
			WWSJOB_ASSERT(  g_WwsJob_jobHeader.m_loadCommandsSize >= 16 &&
							g_WwsJob_jobHeader.m_loadCommandsSize <= MAX_LOAD_COMMANDS_SIZE /*sizeof(g_WwsJob_loadCommands)*/ );

			STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_TryChangeFreeToLoadJob_readLoadCmds, 0,
					((U64)g_WwsJob_eaWorkLoad << 32) | g_WwsJob_jobIndex,
                    ((U64)g_WwsJob_jobHeader.m_mmaLoadCommands << 32) | g_WwsJob_jobHeader.m_loadCommandsSize );

			// start dma read of load commands (<= 16 KByte)
			DMA_READ( &g_WwsJob_loadCommands /*lsa*/, g_WwsJob_jobHeader.m_mmaLoadCommands /*mma*/, \
					g_WwsJob_jobHeader.m_loadCommandsSize /*length*/, DmaTagId::kLoadJob_readCommands )

			// output barriered dma with stall & notify interrupt
			//This interrupt fires after all dmas on DmaTagId::kLoadJob_readCommands have completed
			WwsJob_StartTagSpecificBarrieredNullListWithInterrupt( DmaTagId::kLoadJob_readCommands );

			// declare all bufferSets and buffers are inactive
			U32 bufferSetNum;
			bufferSetNum = g_WwsJob_loadJobNum * WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB;
			WwsJob_BufferSet *pBufferSets;
			pBufferSets = &g_WwsJob_bufferSetArray[bufferSetNum];
			g_WwsJob_firstBufferNum = g_WwsJob_loadJobNum * WWSJOB_MAX_NUM_BUFFERS_PER_JOB;
			g_WwsJob_numFreeBuffers = WWSJOB_MAX_NUM_BUFFERS_PER_JOB;
			WwsJob_Buffer *pBuffers;
			pBuffers = &g_WwsJob_bufferArray[g_WwsJob_firstBufferNum];
			U8 *pLogicalToBufferNums;
			pLogicalToBufferNums = &g_WwsJob_logicalToBufferNumArray[g_WwsJob_firstBufferNum];

			STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryChangeFreeToLoadJob_clearBufSetsAndBufs, g_WwsJob_loadJobNum,
					( (U64)(U32)pBufferSets << 40 ) | ( (U64)(U32)pBuffers << 20 ) | (U64)(U32)pLogicalToBufferNums );

			WwsJob_SetQwordMem( pBufferSets, sizeof(WwsJob_BufferSet) * WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB, 0 );
			WwsJob_SetQwordMem( pBuffers, sizeof(WwsJob_Buffer) * WWSJOB_MAX_NUM_BUFFERS_PER_JOB, 0 );
			WwsJob_SetQwordMem( pLogicalToBufferNums, sizeof(U8) * WWSJOB_MAX_NUM_BUFFERS_PER_JOB, 0xFFFFFFFFFFFFFFFFULL );

			// inc timeStamp, so oldest shareable buffer is taken, if need be
			g_WwsJob_timeStamp++;

			STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryChangeFreeToLoadJob_incTimeStamp, (U16)g_WwsJob_timeStamp );

			break;


		case JobHeaderCommand::kGeneralBarrier:
			//We should never see a kGeneralBarrier here.
			//Code to handle kGeneralBarrier exists inside AllocateJob, since a
			//general barrier *can not* be allocated if it isn't satisfied, whereas
			//if it is satisfied, allocation should immediately move onto the next job

			// fall through into assert

		default:
			// unknown jobHeaderCommand, or kGeneralBarrier flow which shouldn't be possible
			WWSJOB_ASSERT(0);
		}
	}
}
#endif



//====================================================================================================================
//
//	code for when a job exits, and we want to end the store job
//
//====================================================================================================================

/**	\brief	Change the runJob to the storeJob

	Call this after you ensure there is no storeJob.
	This will change the runJob to either the storeJob or a free job (if possible).
	This is called from main().
**/
#if WWS_JOB_USE_C_VERSION!=0
void WwsJob_ChangeRunToStoreJob( void )
{
	// Change runJob to storeJob
	g_WwsJob_lastStoreJobNum = g_WwsJob_storeJobNum = g_WwsJob_runJobNum;
	g_WwsJob_runJobNum = -1;
	g_WwsJob_runJobState = WwsJob_RunJobState::kNone;

	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_ChangeRunToStoreJob, g_WwsJob_storeJobNum );

	// loop below to ensure all bufferSets have cleared the reservedPages bits

	// get first bufferSetNum
	U32 bufferSetNum = g_WwsJob_storeJobNum * WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB;

	// get ptr to first bufferSet
	WwsJob_BufferSet *pBufferSet = &g_WwsJob_bufferSetArray[bufferSetNum];

	// for all bufferSets
	for( bufferSetNum = 0 ; bufferSetNum < WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB ; pBufferSet++, bufferSetNum++ )
	{
		// get bufferSet
		WwsJob_BufferSet bufferSet = *pBufferSet;

		// if bufferSet has reservedPages set
		if( bufferSet.m_reserved )
		{	// bufferSet has reservedPages set

			// clear reserved bit in bufferSet
			bufferSet.m_reserved = 0;
			*pBufferSet = bufferSet;

			// get pageMasks for all buffers
			QuadWord pageMask[2];
			WwsJob_GetPageMasks( bufferSet.m_firstPageNum,
					bufferSet.m_numPagesPerBuffer * bufferSet.m_numBuffers, &pageMask[0] );

			// clear reservedPage bits
			for( U32 i = 0 ; i < 2 ; i++ )
			{
				for( U32 j = 0 ; j < 2 ; j++ )
				{
					// all pages must have been reserved
					WWSJOB_ASSERT( (g_WwsJob_reservedPageMask[i].m_u64[j] & pageMask[i].m_u64[j]) == pageMask[i].m_u64[j] );

					// clear reserved pages
					g_WwsJob_reservedPageMask[i].m_u64[j] &= ~pageMask[i].m_u64[j];
				}
			}
		}
	}

	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_reservedPages, 0,
			g_WwsJob_reservedPageMask[0].m_u64[0], g_WwsJob_reservedPageMask[0].m_u64[1],
			g_WwsJob_reservedPageMask[1].m_u64[0], g_WwsJob_reservedPageMask[1].m_u64[1] );

	// Try to change storeJob to freeJob.  If done
	if( WwsJob_TryChangeStoreToFreeJob() )
	{	// done

		STORE_VERBOSE_AUDIT( AuditId::kWwsJob_ChangeRunToStoreJob_changeDone, g_WwsJob_loadJobState );

		// if there are load commands to do
		if( g_WwsJob_loadJobState == WwsJob_LoadJobState::kExecuteCommands )
		{	// there are load commands to do

			// tro to execute the load commands
			WwsJob_TryExecuteLoadCmds();
		}
	}
	else
	{	// there was ongoing dma

		STORE_VERBOSE_AUDIT( AuditId::kWwsJob_ChangeRunToStoreJob_ongoingDma );

		// output barriered dma with stall & notify interrupt
		//This must barrier against all previous dmas on any tag having completed before the interrupt fires
		//The interrupt handler then spots the dma tag was DmaTagId::kStoreJob_writeJobBuffers in order to know
		//what to process.
		//Note that we don't actually use this interrupt at present, but if we did it would have to be
		//a barrier against all previous tags (ie. tag agnostic)
		//WwsJob_StartTagAgnosticBarrieredNullListWithInterrupt( DmaTagId::kStoreJob_writeJobBuffers );
	}
}
#endif



/**	\brief	Try to change the storeJob to a freeJob

	Return WWSJOB_TRUE if done, or WWSJOB_FALSE if storeJob has job dma ongoing.
	This is called by main() or interrupt.
**/
#if WWS_JOB_USE_C_VERSION!=0
Bool32 WwsJob_TryChangeStoreToFreeJob( void )
{
	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryChangeStoreToFreeJob, g_WwsJob_storeJobNum );

	// For all buffers in the store job: try to clear the used bit and used pages.
	// This block will return TRUE if OK,
	//	 or FALSE if there is ongoing (job) dma so you have to try again later.

	// ensure storeJob exists
	WWSJOB_VERBOSE_ASSERT( g_WwsJob_storeJobNum >= 0  && g_WwsJob_storeJobNum <= 2 );

	// assume no ongoing dma, till shown otherwise
	Bool32 ongoingDma = WWSJOB_FALSE;
	// track if the store job has any shareable buffers active
	//NOTE: for lists of compatible jobs without write cached shareable buffers,
	// it is often the case that all read cached shareable buffers will have been
	// promoted to the run job in UseBuffer commands.  We can therefore optimize
	// out several loops in the following run job if the previous
	// job had no shareable buffers active.
	Bool32 activeShareableBuffers = WWSJOB_FALSE;

  #if 1 // 1 only // 1 = new single loop version (with reserved addition), 0 = orig double loop version (withOUT reserved addition)

	// get ptr to bufferSets
	WwsJob_BufferSet *pBufferSets = &g_WwsJob_bufferSetArray[g_WwsJob_storeJobNum * WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB];

	// get ptr to first buffer
	U32 bufferIndex/*including job#*/ = g_WwsJob_storeJobNum * WWSJOB_MAX_NUM_BUFFERS_PER_JOB;
	WwsJob_Buffer *pBuffer = &g_WwsJob_bufferArray[bufferIndex];

	// get first free bufferNum (with job#)
	U32 firstFreeBufferNum = g_WwsJob_jobDataArray[g_WwsJob_storeJobNum].m_firstFreeBufferNum;

	// loop for all buffers
	for( /*no init*/ ; bufferIndex < firstFreeBufferNum ; bufferIndex++, pBuffer++ )
	{
		// get buffer
		WwsJob_Buffer buffer = *pBuffer;

		STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryChangeStoreToFreeJob_buf, bufferIndex,
				buffer.m_u64 );

		// if buffer is active
		if( buffer.m_u32[0] )
		{	// buffer is active

			// if buffer is reserved
			if( buffer.m_reserved )
			{	// buffer is reserved

				STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryChangeStoreToFreeJob_reservedBufNowInactive );

				// make buffer inactive
				buffer.m_u32[0] = 0;
				*pBuffer = buffer;
			}
			else
			{	// buffer isn't reserved

				// get bufferSet#
				U32 bufferSetNum = buffer.m_bufferSetNum;

				// get bufferSet
				WwsJob_BufferSet bufferSet = pBufferSets[bufferSetNum];

				// bufferSet must be active
				WWSJOB_VERBOSE_ASSERT( bufferSet.m_u32 );

				// get bufferNum
				U32 bufferNum = bufferIndex - bufferSet.m_firstBufferNum/*withJob#*/;

				STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryChangeStoreToFreeJob_activeBuf, bufferSetNum,
						( (U64)bufferSet.m_u32 << 32) | bufferNum );

				// bufferNum must be valid
				WWSJOB_ASSERT( bufferNum < bufferSet.m_numBuffers );

				// if buffer used bit is on
				if( buffer.m_used )
				{	// buffer used bit is on

					STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryChangeStoreToFreeJob_bufUsed );

					// try to free dmaTagId and used pages.  If done
					if( WwsJob_TryFreeTagAndUsedPages( bufferSet, bufferNum, buffer ) )
					{	// done

						// used <- 0
						buffer.m_used = 0;
						*pBuffer = buffer;

						STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_TryChangeStoreToFreeJob_clearUsed, (bufferSetNum << 8) | bufferNum,
								buffer.m_u64 );
					}
					else
					{	// not done (ongoing dma)

						STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryChangeStoreToFreeJob_ongoingDma );

						// declare onoing dma
						ongoingDma = WWSJOB_TRUE;

						// we have to wait until dma is done before we can clear the dmaTagId and used page bits
						continue;
					}
				}

				// if buffer shareable
				if( buffer.m_shareable )
				{	// buffer shareable
					activeShareableBuffers = WWSJOB_TRUE;
					// leave shareable buffers alone
					continue;
				}

				// make buffer inactive
				buffer.m_u32[0] = 0;
				*pBuffer = buffer;

				STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_TryChangeStoreToFreeJob_inactivateNonShareBuf, (bufferSetNum << 8) | bufferNum,
						buffer.m_u64 );
			}
		}
	}

  #else // #if x // 1 = new single loop version, 0 = orig double loop version

	// get first bufferSetNum
	U32 bufferSetNum = g_WwsJob_storeJobNum * WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB;

	// get ptr to first bufferSet
	WwsJob_BufferSet *pBufferSet = &g_WwsJob_bufferSetArray[bufferSetNum];

	// for all bufferSets
	for( bufferSetNum = 0 ; bufferSetNum < WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB ; pBufferSet++, bufferSetNum++ )
	{
		// get bufferSet
		WwsJob_BufferSet bufferSet = *pBufferSet;

		// if bufferSet is active
		if( bufferSet.m_u32 )
		{	// bufferSet is active

			// Audit removed when single loop version added
			//STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryChangeStoreToFreeJob_activeBufSet, bufferSetNum,
			//		bufferSet.m_u32 );

			// get bufferSet page configuration (parts needed)
			U32 numBuffers = bufferSet.m_numBuffers;

			// get ptr to first buffer
			WwsJob_Buffer *pBuffer = &g_WwsJob_bufferArray[bufferSet.m_firstBufferNum];

			// for all buffers
			for( U32 bufferNum = 0 ; bufferNum < numBuffers ; pBuffer++, bufferNum++ )
			{
				// get buffer
				WwsJob_Buffer buffer = *pBuffer;

				// if buffer is inactive
				if( !buffer.m_u32[0] )
				{	// buffer is inactive

					// get next buffer
					continue;
				}

				// buffer can't be reserved
				// runJob buffer reservations were removed when runJob allowed loadJob to start
				WWSJOB_VERBOSE_ASSERT( !buffer.m_reserved );

				// Audit removed when single loop version added
				//STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryChangeStoreToFreeJob_activeBuf, bufferNum,
				//		buffer.m_u64 );

				// if buffer used bit is on
				if( buffer.m_used )
				{	// buffer used bit is on

					STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryChangeStoreToFreeJob_bufUsed );

					// try to free dmaTagId and used pages.  If done
					if( WwsJob_TryFreeTagAndUsedPages( bufferSet, bufferNum, buffer ) )
					{	// done

						// used <- 0
						buffer.m_used = 0;
						*pBuffer = buffer;

						STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_TryChangeStoreToFreeJob_clearUsed, (bufferSetNum << 8) | bufferNum,
								buffer.m_u64 );
					}
					else
					{	// not done (ongoing dma)

						STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryChangeStoreToFreeJob_ongoingDma );

						// declare onoing dma
						ongoingDma = WWSJOB_TRUE;

						// we have to wait until dma is done before we can clear the dmaTagId and used page bits
						continue;
					}
				}

				// if buffer shareable
				if( buffer.m_shareable )
				{	// buffer shareable
					activeShareableBuffers = WWSJOB_TRUE;
					// leave shareable buffers alone
					continue;
				}

				// make buffer inactive
				buffer.m_u32[0] = 0;
				*pBuffer = buffer;

				STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_TryChangeStoreToFreeJob_inactivateNonShareBuf, (bufferSetNum << 8) | bufferNum,
						buffer.m_u64 );
			}
		}
	}

  #endif // #if x // 1 = new single loop version, 0 = orig double loop version

	if (!activeShareableBuffers)
	{
		g_WwsJob_jobDataArray[g_WwsJob_storeJobNum].m_jobHasShareableBuffers = WWSJOB_FALSE;
	}
	if( !ongoingDma )
	{	// everything cleared
		// there is no ongoing (job) dma

		STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_TryChangeStoreToFreeJob_jobDone, g_WwsJob_storeJobNum );

		// Declare the storeJob is done (update ListHeader, check dependency)

		//Get job workload address for the job that's just finishing
		//This is the list that needs its minActive updating
		U32 eaJobEndingWorkLoad = g_WwsJob_jobDataArray[g_WwsJob_storeJobNum].m_eaWorkLoad;

		//This job is no longer considered to be active.
		g_WwsJob_jobDataArray[g_WwsJob_storeJobNum].m_jobIndex		= 0xFFFF;
		g_WwsJob_jobDataArray[g_WwsJob_storeJobNum].m_eaWorkLoad	= 0;

		//Get the lowest jobIndex that is still active on the workload that is just finishing
		U32 minActive = 0xFFFF;
		if ( g_WwsJob_jobDataArray[0].m_eaWorkLoad == eaJobEndingWorkLoad )
			minActive = WwsJob_min( g_WwsJob_jobDataArray[0].m_jobIndex, minActive );
		if ( g_WwsJob_jobDataArray[1].m_eaWorkLoad == eaJobEndingWorkLoad )
			minActive = WwsJob_min( g_WwsJob_jobDataArray[1].m_jobIndex, minActive );
		if ( g_WwsJob_jobDataArray[2].m_eaWorkLoad == eaJobEndingWorkLoad )
			minActive = WwsJob_min( g_WwsJob_jobDataArray[2].m_jobIndex, minActive );

		STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryChangeStoreToFreeJob_nowFree, minActive,
				eaJobEndingWorkLoad );

        // note eaWorkLoad may be *different* from current g_WwsJob_eaWorkLoad!
        while ( true )
        {
            //Get an atomic reservation on our copy of the gJobListHeader
            DMA_GETLLAR( &g_tempUsageAtomicBuffer.m_jobListHeader, eaJobEndingWorkLoad );
            DMA_WAITATOMICSTATUS();

			g_tempUsageAtomicBuffer.m_jobListHeader.m_minActive[g_WwsJob_dataForJob.m_spuNum] = minActive;

            DMA_PUTLLC( &g_tempUsageAtomicBuffer.m_jobListHeader, eaJobEndingWorkLoad );
            U32 putLlcSuccess = DMA_WAITATOMICSTATUS();

            if ( (putLlcSuccess&1) == 0 )
                break;
        }

		// Check if we've got any dependencies, and if so decrement each one
		WwsJob_JobData* pJobData = &g_WwsJob_jobDataArray[g_WwsJob_storeJobNum];

	  #if 1 // 1 = Jon's new code for downcoding, 0 = John's orig code

		U32 numDeps = pJobData->m_numDependencies;
        pJobData->m_numDependencies = 0;
        for ( U32 depNum = 0; depNum < numDeps; ++depNum )
		{
			U32 mmDependencyAddress = pJobData->m_eaDependency[depNum];
			if ( mmDependencyAddress )
			{
				DecrementDependency( mmDependencyAddress );
			}
		}

	  #else // #if x // 1 = Jon's new code for downcoding, 0 = John's orig code

		for ( U32 depNum = 0; depNum < pJobData->m_numDependencies; ++depNum )
		{
			U32 mmDependencyAddress = pJobData->m_eaDependency[depNum];
			if ( mmDependencyAddress )
			{
				DecrementDependency( mmDependencyAddress );
			}
		}
		pJobData->m_numDependencies = 0;

	  #endif // #if x // 1 = Jon's new code for downcoding, 0 = John's orig code

		// the storeJob is now free
		g_WwsJob_storeJobNum = -1;

		// declare storeJob is changed to freeJob
		return WWSJOB_TRUE;
	}
	else
	{	// everything was not cleared
		// there is ongoing (job) dma

		STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryChangeStoreToFreeJob_ongoingDmas );

		// declare there is ongoing dma
		return WWSJOB_FALSE;
	}
}
#endif


/**	\brief  Try to free dmaTagId and used pages for a buffer

	Return WWSJOB_TRUE if done, or WWSJOB_FALSE if not done (only possible if ongoing job dma).

	\param	bufferSet = data for bufferSet
	\param	bufferNum = physical buffer number in bufferSet
	\param	buffer = reference to local copy of buffer data from g_WwsJob_bufferArray[]
**/
#if WWS_JOB_USE_C_VERSION!=0
Bool32 WwsJob_TryFreeTagAndUsedPages( WwsJob_BufferSet bufferSet, U32 bufferNum, WwsJob_Buffer buffer )
{
	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryFreeTagAndUsedPages, bufferNum,
			buffer.m_u64 );

	// if dmaTagId exists
	U32 dmaTagId = buffer.m_dmaTagId;
	if( dmaTagId )
	{	// dmaTagId exists

		STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryFreeTagAndUsedPages_tagExists, dmaTagId );

		// if (job) dma is ongoing, with this dmaTagId
		if( !WwsJob_IsDmaTagMaskDone( 1 << dmaTagId ) )
		{	// (job) dma is ongoing, with this dmaTagId

			STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryFreeTagAndUsedPages_ongoingDma );

			// we are not done (there is ongoing dma)
			return WWSJOB_FALSE;
		}

		// if buffer is not shareable
		if( !buffer.m_shareable )
		{	// buffer is not shareable

			STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryFreeTagAndUsedPages_nonShareBufFreeTag );

			// free the dmaTagId
			WwsJob_FreeDmaTagId( dmaTagId );
		}
	}

	// clear the used pages (below)

	// get pageMasks
	U32 bufferSetFirstPageNum	= bufferSet.m_firstPageNum;
	U32 numPagesPerBuffer	 	= bufferSet.m_numPagesPerBuffer;
	U32 bufferPageNum 			= bufferSetFirstPageNum + numPagesPerBuffer * bufferNum;
	QuadWord pageMask[2];
	WwsJob_GetPageMasks( bufferPageNum, numPagesPerBuffer, &pageMask[0] );

	// clear the used pages
#ifdef CHRIS_DOWNCODING
	VU32 notAllocated0 = spu_andc( pageMask[0].m_vu32, spu_and( g_WwsJob_usedPageMask[0].m_vu32, pageMask[0].m_vu32 ) ); //should be zero
	VU32 notAllocated1 = spu_andc( pageMask[1].m_vu32, spu_and( g_WwsJob_usedPageMask[1].m_vu32, pageMask[1].m_vu32 ) ); //should be zero
	VU32 combined = spu_or( notAllocated0, notAllocated1 );
	WWSJOB_ASSERT( si_to_uint( (VI8) spu_orx( combined ) ) == 0 );

	g_WwsJob_usedPageMask[0].m_vu32 = spu_andc( g_WwsJob_usedPageMask[0].m_vu32, pageMask[0].m_vu32 );
	g_WwsJob_usedPageMask[1].m_vu32 = spu_andc( g_WwsJob_usedPageMask[1].m_vu32, pageMask[1].m_vu32 );
#else
	for( U32 i = 0 ; i < 2 ; i++ )
	{
		for( U32 j = 0 ; j < 2 ; j++ )
		{
			// ensure pages were used
			WWSJOB_ASSERT( (g_WwsJob_usedPageMask[i].m_u64[j] & pageMask[i].m_u64[j]) == pageMask[i].m_u64[j] );

			// clear the used pages
			g_WwsJob_usedPageMask[i].m_u64[j] &= ~pageMask[i].m_u64[j];
		}
	}
#endif

	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_usedPages, 0,
			g_WwsJob_usedPageMask[0].m_u64[0], g_WwsJob_usedPageMask[0].m_u64[1],
			g_WwsJob_usedPageMask[1].m_u64[0], g_WwsJob_usedPageMask[1].m_u64[1] );

	// declare done
	return WWSJOB_TRUE;
}
#endif


//====================================================================================================================
//
//	load commands (also usable by runJob)
//
//====================================================================================================================

/**	\brief  Try to execute the commands for the loadJob

	This is called (directly or indirectly) by main() or interrupt or WwsJob_JobApi.
**/
#if WWS_JOB_USE_C_VERSION!=0
void WwsJob_TryExecuteLoadCmds( void )
{
	STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_TryExecuteLoadCmds );

	// if the runJob is still discard writing the shareable buffers needed by the loadJob
	if( !WwsJob_IsDmaTagMaskDone( 1 << DmaTagId::kRunJob_writeShareableBuffers ) )
	{	// the runJob is still discard writing the shareable buffers needed by the loadJob

		STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryExecuteLoadCmds_runDmaOngoing );

		// we have to wait till that dma is done (note that it will generate an interrupt)
		return;
	}

	// get prev jobNum
	I32 prevJobNum = g_WwsJob_loadJobNum ? g_WwsJob_loadJobNum - 1 : 2;

	// Try to execute the loadJob commands.  If all commands finished
	if( WwsJob_TryExecuteCmds( prevJobNum, g_WwsJob_loadJobNum, &g_WwsJob_loadCommands[0] ) )
	{	// all commands finished

		// declare command finished
		g_WwsJob_loadJobState = WwsJob_LoadJobState::kCommandsExecuted;

		// Note: you do NOT have to output a barriered dma with stall & notify interrupt
		// because main() will poll to ensure there is none of this dma before proceeding

		STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryExecuteLoadCmds_allCmdsFinished );
	}

	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryExecuteLoadCmds_end );
}
#endif


/**	\brief	Try to execute the commands (for a load or runJob).

	Return WWSJOB_TRUE if all commands done.
	This is called by interrupt(loadJob) or WwsJob_JobApi(for runJob or loadJob)
	Note: all discards of shareable buffers previously needed must have been checked and dma is done before calling this!

	\param	prevJobNum = previous job number (job previous to load or runJob)
	\param	jobNum = job number (for load or runJob)
	\param	pCommands = ptr to start of commands to execute.
**/
#if WWS_JOB_USE_C_VERSION!=0
Bool32 WwsJob_TryExecuteCmds( I32 prevJobNum, I32 jobNum, WwsJob_Command *pCommands )
{
	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryExecuteCmds, (prevJobNum << 8) | jobNum,
			(U32)pCommands );

	// commands must start qword aligned
	WWSJOB_ASSERT( ( (U32)pCommands & 0xf ) == 0 );

	// assume all commands done (so far)
	Bool32 allCommandsDone = WWSJOB_TRUE;

	// assume no commands started any shareable discards
	Bool32 commandsStartedShareableDiscards = WWSJOB_FALSE;

	I32/*0 or -1*/ jobIsLoadJob = (jobNum == g_WwsJob_loadJobNum) ? -1 : 0;

	// loop for all commands
	for( /*no init*/ ; 1/*forever*/ ; pCommands++ )
	{
		// get command data into temp struct
		WwsJob_Command command = *pCommands;

		// check for CommandNum::kNop here, to speed it up a bit
		if( command.m_u32[0] == 0 )
		{	// nop

			continue;
		}

		// get logical bufferSetNum and bufferNum (if applicable to command)
		U32 logicalBufferSetNum = command.m_reserveBufferSetCommand.m_flags.m_logicalBufferSetNum;
		U32 logicalBufferNum    = command.m_reserveBufferSetCommand.m_flags.m_logicalBufferNum;

		// Set pBufferSet, bufferSet, bufferSetFirstPageNum, numPagesPerBuffer, numBuffers, pBuffers, bufferNum, pBuffer, buffer, bufferPageNum, pLogicalToBufferNums
		// (if applicable to command)
		WwsJob_MiscData miscData;
		WwsJob_GetLogicalBuffer( jobNum, logicalBufferSetNum, logicalBufferNum, miscData );

		// jump to command
		switch( command.m_reserveBufferSetCommand.m_flags.m_commandNum )
		{
			// Note CommandNum::kNop already checked for above

			case CommandNum::kReserveBufferSet:
            {
				// the runJob can't reserve bufferSets after it has allowed a load job
				WWSJOB_ASSERT( jobIsLoadJob  ||  g_WwsJob_runJobState != WwsJob_RunJobState::kLoadAllowed );

                // bufferSet must be inactive
                // Note: all bytes of bufferSet are init'd to 0 when job became a loadJob
                WWSJOB_ASSERT( miscData.m_bufferSet.m_u32 == 0 );

				// Our buffer sets can't overlap with the stack or job manager
				// Note: this also catches the case of buffer sets that wrap off the end of memory,
				// which cause GetPageMasks to return a 0 mask incorrectly.
				WWSJOB_ASSERT( command.m_reserveBufferSetCommand.m_bufferSet.m_firstPageNum >= LsMemoryLimits::kJobAreaBasePageNum && command.m_reserveBufferSetCommand.m_bufferSet.m_firstPageNum + command.m_reserveBufferSetCommand.m_bufferSet.m_numPagesPerBuffer * command.m_reserveBufferSetCommand.m_bufferSet.m_numBuffers <= LsMemoryLimits::kJobAreaEndPageNum );

			  #if 1 // 1 only!!! // 1 = Jon's new code to assist in downcoding, 0 = John's old C code (without reservedPage support)

				STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryExecuteCmds_tryReserveBufSet, logicalBufferSetNum );

				// get pageMasks for all buffers
				QuadWord pageMask[2];
				WwsJob_GetPageMasks( command.m_reserveBufferSetCommand.m_bufferSet.m_firstPageNum,
						command.m_reserveBufferSetCommand.m_bufferSet.m_numPagesPerBuffer *
						command.m_reserveBufferSetCommand.m_bufferSet.m_numBuffers, &pageMask[0] );

				// if any pages are already reserved
				// then this command will have to wait for reservations to clear
				for( U32 i = 0 ; i < 2 ; i++ )
				{
					for( U32 j = 0 ; j < 2 ; j++ )
					{
						// if any pages are already reserved
						if( g_WwsJob_reservedPageMask[i].m_u64[j] & pageMask[i].m_u64[j] )
						{
							// then this command will have to wait for reservations to clear
							goto CommandCantExecute;
						}
					}
				}

				// reserve the pages
				for( U32 i = 0 ; i < 2 ; i++ )
				{
					for( U32 j = 0 ; j < 2 ; j++ )
					{
						g_WwsJob_reservedPageMask[i].m_u64[j] |= pageMask[i].m_u64[j];
					}
				}

                U32 firstBufferNum = g_WwsJob_firstBufferNum;

                // Set bufferSet data
                WwsJob_BufferSet bufferSet;
                bufferSet = command.m_reserveBufferSetCommand.m_bufferSet/*m_reserved & m_firstBufferNum is not set*/;
				bufferSet.m_reserved = 1;
                bufferSet.m_firstBufferNum = firstBufferNum;
                *miscData.m_pBufferSet = bufferSet;

				STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_TryExecuteCmds_reserveBufSet, logicalBufferSetNum,
						bufferSet.m_u32 );

				STORE_VERBOSE_AUDIT( AuditId::kWwsJob_reservedPages, 0,
						g_WwsJob_reservedPageMask[0].m_u64[0], g_WwsJob_reservedPageMask[0].m_u64[1],
						g_WwsJob_reservedPageMask[1].m_u64[0], g_WwsJob_reservedPageMask[1].m_u64[1] );

                // set ptr to buffers
                WwsJob_Buffer *pBuffers = &g_WwsJob_bufferArray[firstBufferNum];

                // Reserve the buffers for the job
                U32 numBuffers = command.m_reserveBufferSetCommand.m_bufferSet.m_numBuffers;
                // ensure there are enough buffers
                WWSJOB_ASSERT( numBuffers  &&  numBuffers <= g_WwsJob_numFreeBuffers );
                g_WwsJob_firstBufferNum = firstBufferNum + numBuffers;
                g_WwsJob_numFreeBuffers -= numBuffers;

                // Note pBuffers already points to first buffer

				// Set all buffers to reserved, set bufferSetNum (other bits 0), save to array
				miscData.m_buffer.m_u64 = 0;
				miscData.m_buffer.m_reserved = 1;
				miscData.m_buffer.m_bufferSetNum = logicalBufferSetNum;
				WwsJob_SetDwordMem( (U64*)pBuffers, sizeof(WwsJob_Buffer) * numBuffers/*non-zero*/, miscData.m_buffer.m_u64 );

			  #else // #if x // 1 = Jon's new code, 0 = John's old code (without reservedPage support)

				// Set bufferSet info & save it
				miscData.m_bufferSet =
						command.m_reserveBufferSetCommand.m_bufferSet/*m_firstBufferNum is not set*/;
				miscData.m_bufferSet.m_firstBufferNum = g_WwsJob_firstBufferNum;
				*miscData.m_pBufferSet = miscData.m_bufferSet;

				// set ptr to buffers
				miscData.m_pBuffers = &g_WwsJob_bufferArray[g_WwsJob_firstBufferNum];

				// Reserve the buffers for the job
				miscData.m_numBuffers = command.m_reserveBufferSetCommand.m_bufferSet.m_numBuffers;
				// ensure there are enough buffers
				WWSJOB_ASSERT( miscData.m_numBuffers  &&  miscData.m_numBuffers <= g_WwsJob_numFreeBuffers );
				g_WwsJob_firstBufferNum += miscData.m_numBuffers;
				g_WwsJob_numFreeBuffers -= miscData.m_numBuffers;

				STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_TryExecuteCmds_reserveBufSet, logicalBufferSetNum,
						miscData.m_bufferSet.m_u32 );

				// Note miscData.m_pBuffers already points to first buffer

				// Set all buffers to reserved, set bufferSetNum (other bits 0), save to array
				miscData.m_buffer.m_u64 = 0;
				miscData.m_buffer.m_reserved = 1;
				miscData.m_buffer.m_bufferSetNum = logicalBufferSetNum;
				WwsJob_SetDwordMem( (U64*)miscData.m_pBuffers, sizeof(WwsJob_Buffer) * miscData.m_numBuffers/*non-zero*/, miscData.m_buffer.m_u64 );

			  #endif // #if x // 1 = Jon's new code, 0 = John's old code (without reservedPage support)

				// change command to CommandNum::kNop (which is 0!!!) so it won't execute again
				command.m_u64 = 0;
				*pCommands = command;

				// command executed
				continue;
			}
			case CommandNum::kUnreserveBufferSets:
            {
				U32 bufferSetMask = command.m_unreserveBufferSetsCommand.m_bufferSetMask;

				STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryExecuteCmds_tryUnreserveBufSet, (U16)bufferSetMask );

				// we can only execute this command if all previous commands executed
				// (so we don't clear reserved bits before the useBuffer command can execute)
				if( !allCommandsDone )
				{	// we have to wait

					goto CommandCantExecute;
				}

				// you must specify >= 1 valid bufferSet
				WWSJOB_ASSERT ( bufferSetMask != 0   &&   bufferSetMask < (1 << WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB) );

				// get start of bufferSets
				WwsJob_BufferSet *pBufferSets = &g_WwsJob_bufferSetArray[ jobNum * WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB ];

				do
				{
					// get bufferSet#
					U32 bufferSetNum = 31 - spu_extract( spu_cntlz( spu_splats(bufferSetMask) ), 0 );

					// clear bufferSet bit
					bufferSetMask = bufferSetMask & ~(1 << bufferSetNum);

					// get ptr to bufferSet
					WwsJob_BufferSet *pBufferSet = &pBufferSets[bufferSetNum];

					// get bufferSet data
					WwsJob_BufferSet bufferSet = *pBufferSet;

					// ensure bufferSet had pages reserved
					WWSJOB_ASSERT( bufferSet.m_reserved );

					// clear reserved bit in bufferSet
					bufferSet.m_reserved = 0;
					*pBufferSet = bufferSet;

					STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_TryExecuteCmds_unreserveBufSet, bufferSetNum,
							bufferSet.m_u32 );

					// get pageMasks for all buffers
					QuadWord pageMask[2];
					WwsJob_GetPageMasks( bufferSet.m_firstPageNum,
							bufferSet.m_numPagesPerBuffer * bufferSet.m_numBuffers, &pageMask[0] );

					// clear reservedPage bits
					for( U32 i = 0 ; i < 2 ; i++ )
					{
						for( U32 j = 0 ; j < 2 ; j++ )
						{
							// all pages must have been reserved
							WWSJOB_ASSERT( (g_WwsJob_reservedPageMask[i].m_u64[j] & pageMask[i].m_u64[j]) == pageMask[i].m_u64[j] );

							// clear reserved pages
							g_WwsJob_reservedPageMask[i].m_u64[j] &= ~pageMask[i].m_u64[j];
						}
					}

					STORE_VERBOSE_AUDIT( AuditId::kWwsJob_reservedPages, 0,
							g_WwsJob_reservedPageMask[0].m_u64[0], g_WwsJob_reservedPageMask[0].m_u64[1],
							g_WwsJob_reservedPageMask[1].m_u64[0], g_WwsJob_reservedPageMask[1].m_u64[1] );

				}while( bufferSetMask != 0 );

				// change command to CommandNum::kNop (which is 0!!!) so it won't execute again
				command.m_u64 = 0;
				*pCommands = command;

				// command executed
				continue;
			}
			case CommandNum::kUseBuffer:
			{
				STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_TryExecuteCmds_useBuffer, command.m_useBufferCommand.m_u32Flags & 0xFFFF );

				// declare no "best" m_buffer found yet
				// Note: this var & init are moved up to here from down below so that if you do a jump to
				//		ContinueAfterDiscardWrite (below), then bestTakePreference will be in a state which
				//		causes that code to know that there was a shareable collision and thus to scan for shareable bufs
				//		to discard (which needs to be done)
				U32 bestTakePreference;
				bestTakePreference = 0;

				// if bufferSet is not active
				if( !miscData.m_bufferSet.m_u32 )
				{	// bufferSet is not active

					// then this command will have to wait till the bufferSet is reserved
					goto CommandCantExecute;
				}

				// logicalBufferNum must be legal
				WWSJOB_ASSERT( logicalBufferNum < miscData.m_numBuffers );

				// bufferSet must be reserved
				WWSJOB_ASSERT( miscData.m_bufferSet.m_reserved );

				// get main mem adrs (used as match for shareing)
				U32 commandMmAddress = command.m_useBufferCommand.m_mmAddressInQwords << 4;

				// check if command must be continued after a previous discard of a conflicting shared buffer
				if( command.m_useBufferCommand.m_flags.m_continueAfterDiscardWrite )
				{	// command must be continued after a previous discard of a conflicting shared buffer

					STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryExecuteCmds_continueAfterDumpShare );

					// Note that caller already ensured that any discards of shareable buffers (needed
					//	by this command) have already finished

					// continue command
					// Note: bestTakePreference is 0 which forces code below to discard shareable buffers
					goto ContinueAfterDiscardWrite;
				}

				// get prevJob bufferSetNum
				U32 prevJobBufferSetNum;
				prevJobBufferSetNum = (prevJobNum * WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB) + logicalBufferSetNum;

				// get ptr to prevJob bufferSet
				WwsJob_BufferSet *pPrevJobBufferSet;
				pPrevJobBufferSet = &g_WwsJob_bufferSetArray[prevJobBufferSetNum];

				// get prevJob bufferSet
				WwsJob_BufferSet prevJobBufferSet = *pPrevJobBufferSet;

				// get ptr to first prevJob buffers (garbage if bufferSet inactive)
				WwsJob_Buffer *pPrevJobBuffers;
				pPrevJobBuffers = &g_WwsJob_bufferArray[prevJobBufferSet.m_firstBufferNum];

				// see if prevJob bufferSet page configuration is compatible with bufferSet
				// Note: we don't care if the number of buffers is the same or not
				Bool32 prevJobConfigurationCompatible;
				prevJobConfigurationCompatible =
						(prevJobBufferSet.m_firstPageNum == miscData.m_bufferSet.m_firstPageNum) &&
						(prevJobBufferSet.m_numPagesPerBuffer == miscData.m_bufferSet.m_numPagesPerBuffer);

				// if command wants buffer to input to be read and is creating a cached buffer and
				//	previous job buffer configuration matches
				if( command.m_useBufferCommand.m_flags.m_inputRead && prevJobConfigurationCompatible )
				{	// command wants buffer input to be from previous shared buffer  and
					//	previous job buffer configuration matches

					STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryExecuteCmds_findInputMatch );

					// Get ptr to first buffer
					miscData.m_pBuffer = miscData.m_pBuffers;

					// Get ptr to first buffer in previous job
					WwsJob_Buffer *pPrevJobBuffer = pPrevJobBuffers;

					// loop to find a shareable buffer match
					for( miscData.m_bufferNum = 0 ; miscData.m_bufferNum < miscData.m_numBuffers ;
							miscData.m_pBuffer++, pPrevJobBuffer++, miscData.m_bufferNum++ )
					{
						// get buffer
						miscData.m_buffer = *miscData.m_pBuffer;

						// get prevJob buffer
						WwsJob_Buffer prevJobBuffer = *pPrevJobBuffer;

						// if this buffer is reserved  and  prevJob buffer is shareable  and
						//		prevJob buffer is a match (via mmAddress)
						if( miscData.m_buffer.m_reserved  &&  prevJobBuffer.m_shareable  &&
								( (prevJobBuffer.m_mmAddressInQwords << 4) == commandMmAddress ) )
						{	// this buffer is reserved  and  prevJob buffer is shareable  and
							//		prevJob buffer is a match (via mmAddress)

							// for buffer:
							//		copy from prevJobBuffer
							//		used <- 1
							miscData.m_buffer = prevJobBuffer;
							miscData.m_buffer.m_used = 1;
							// note: buffer is not fully set, so it's not saved yet

							// for prevJob buffer:
							//		shareable <- 0
							//		shareableWriteIfDiscarded <- 0
							//		sharedToLaterJob <- 1
							//		used <- 0
							//		tag <- 0
							prevJobBuffer.m_shareable 					= 0;
							prevJobBuffer.m_shareableWriteIfDiscarded 	= 0;
							prevJobBuffer.m_sharedToLaterJob 			= 1;
							prevJobBuffer.m_used						= 0;
							prevJobBuffer.m_dmaTagId					= 0;
							*pPrevJobBuffer = prevJobBuffer;

							STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_TryExecuteCmds_foundMatch, miscData.m_bufferNum,
									prevJobBuffer.m_u64 , miscData.m_buffer.m_u64);

							// needed below
							miscData.m_bufferPageNum = miscData.m_bufferSetFirstPageNum + miscData.m_numPagesPerBuffer * miscData.m_bufferNum;

							goto CheckWriteBits;
						}
					}
					// can't find a shareable match
				}
				// there was no sharing desired or done
				// Now we must find the best buffer to use

				STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryExecuteCmds_getBufToTake );

				// get ptr to first buffer
				miscData.m_pBuffer = miscData.m_pBuffers;

				// get # of buffers in previous job
				U32 prevJobNumBuffers = prevJobBufferSet.m_numBuffers;

				// Get ptr to first buffer in previous job (garbage if bufferSet inactive)
				WwsJob_Buffer *pPrevJobBuffer;
				pPrevJobBuffer = pPrevJobBuffers;

				// Note this does not have to be init'd
				// It is init'd only to get rid of the compiler warning
				U32 bestBufferNum;
				bestBufferNum = 0;

				// for all buffers
				for( miscData.m_bufferNum = 0 ; miscData.m_bufferNum < miscData.m_numBuffers ;
						miscData.m_pBuffer++, pPrevJobBuffer++, miscData.m_bufferNum++ )
				{
					// get buffer
					miscData.m_buffer = *miscData.m_pBuffer;

					// if buffer is reserved
					if( miscData.m_buffer.m_reserved )
					{	// buffer is reserved

						STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryExecuteCmds_bufIsReserved, miscData.m_bufferNum );

						// get buffer first pageNum
						miscData.m_bufferPageNum = miscData.m_bufferSetFirstPageNum + miscData.m_numPagesPerBuffer * miscData.m_bufferNum;

						// get pageMasks
						QuadWord pageMask[2];
						WwsJob_GetPageMasks( miscData.m_bufferPageNum, miscData.m_numPagesPerBuffer, &pageMask[0] );

						// see if pages are used and/or shared
#ifdef CHRIS_DOWNCODING
						VU32 usedPages0 = spu_and( g_WwsJob_usedPageMask[0].m_vu32, pageMask[0].m_vu32 );
						VU32 usedPages1 = spu_and( g_WwsJob_usedPageMask[1].m_vu32, pageMask[1].m_vu32 );
						VU32 usedPagesCombined = spu_or( usedPages0, usedPages1 );
						U32 usedPages = si_to_uint( (VI8) spu_orx( usedPagesCombined ) );

						VU32 sharedPages0 = spu_and( g_WwsJob_shareablePageMask[0].m_vu32, pageMask[0].m_vu32 );
						VU32 sharedPages1 = spu_and( g_WwsJob_shareablePageMask[1].m_vu32, pageMask[1].m_vu32 );
						VU32 sharedPagesCombined = spu_or( sharedPages0, sharedPages1 );
						U32 sharedPages = si_to_uint( (VI8) spu_orx( sharedPagesCombined ) );
#else
						U64 usedPages = 0;
						U64 sharedPages = 0;
						for( U32 i = 0 ; i < 2 ; i++ )
						{
							for( U32 j = 0 ; j < 2 ; j++ )
							{
								// see if pages are used and/or shared
								usedPages   |= g_WwsJob_usedPageMask[i].m_u64[j]      & pageMask[i].m_u64[j];
								sharedPages |= g_WwsJob_shareablePageMask[i].m_u64[j] & pageMask[i].m_u64[j];
							}
						}
#endif

						// if no pages are used
						if( !usedPages )
						{	// no pages are used

							STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryExecuteCmds_noPagesUsed );

							// if pages are shared and a previous command started a discard write of shareable buffers
							if( sharedPages  &&  commandsStartedShareableDiscards )
							{	// pages are shared and a previous command started a discard write of shareable buffers

								// you can *not* continue until the shareable buffer dma write is done.
								// The reason is that the previous job could have started a discard write and cleared the
								// "write if discard" bit.  But the dma may be ongoing, and if you continue here you would
								// think you can use the shareable buffer since the WID bit is off.  But you can't until
								// the dma is complete.  So this check avoids that error.

								STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryExecuteCmds_sharedPagesAndPrevDiscards );

								// try the next buffer
								continue;
							}

							// if prevJob bufferSet inactive, then use age of 0xff
							U32 age = 0xff;
							U32 takePreference;
							WwsJob_Buffer prevJobBuffer;

							// if prevJob buffer is allocated
							if( miscData.m_bufferNum < prevJobNumBuffers )
							{	// prevJob buffer is allocated

								STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryExecuteCmds_prevJobBufAllocated, 0 );

								// Get prevJob buffer
								prevJobBuffer = *pPrevJobBuffer;

								// Get age of shareable buffer
								age/*0:0xFF*/ = ( (U32)(g_WwsJob_timeStamp - prevJobBuffer.m_timeStamp) ) & 0xFF;
							}

							#define TAKE_PREFERENCE_NOT_SHAREABLE   0x40000000
							#define TAKE_PREFERENCE_NO_SHARED_PAGES 0x20000000
							#define TAKE_PREFERENCE_BASE			0x10000000

							// this ensures the first buffer seen will be best (so far)
							takePreference = age | TAKE_PREFERENCE_BASE;

							// if prevJob buffer is not (compatible & shareable)
							// Note: if prevJob is inactive, then prevJobConfigurationCompatible is false
							//		and so prevJobBuffer is ignored
							if( !( prevJobConfigurationCompatible && prevJobBuffer.m_shareable ) )
							{	// prevJob buffer is not (compatible & shareable)

								// we prefer to take this buffer
								takePreference |= TAKE_PREFERENCE_NOT_SHAREABLE;
							}

							// if buffer does not overlap any other shared buffers
							if( !sharedPages )
							{	// buffer does not overlap any other shared buffers

								// we prefer to take this buffer
								takePreference |= TAKE_PREFERENCE_NO_SHARED_PAGES;
							}

							STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryExecuteCmds_takePreference, 0,
									takePreference );
							// if this is best buffer to take so far
							if( takePreference > bestTakePreference )
							{   // this is best buffer to take so far

								// take this buffer
								bestTakePreference = takePreference;
								bestBufferNum = miscData.m_bufferNum;

								STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryExecuteCmds_bestSoFar, miscData.m_bufferNum );
							}
						}
					}
				}

				// if we couldn't take a buffer (they all had used pages)
				if( bestTakePreference == 0 )
				{	// we couldn't take a buffer (they all had used pages)

					STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryExecuteCmds_allBufsInUse );

					// we can't execute the command
					goto CommandCantExecute;
				}
				// we have a buffer to take!

				// get best bufferNum, ptr to buffer, buffer, and its first pageNum
				miscData.m_bufferNum = bestBufferNum;
				miscData.m_pBuffer = &miscData.m_pBuffers[miscData.m_bufferNum];
				miscData.m_bufferPageNum = miscData.m_bufferSetFirstPageNum + miscData.m_numPagesPerBuffer * miscData.m_bufferNum;

				STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_TryExecuteCmds_takeBuf, miscData.m_bufferNum,
						( (U64)miscData.m_bufferPageNum << 48 ) | ( (U64)miscData.m_numPagesPerBuffer << 32 ) | (U32)miscData.m_pBuffer );

			  ContinueAfterDiscardWrite:

				// buffer <- usedUnshared (note we don't save it yet)
				miscData.m_buffer.m_u32[0] = 0;
				miscData.m_buffer.m_used = 1;

				// old: if some pages were shareable by other buffers
				// NOTE: this has been changed to always check for prevJob buffer collisions
				//	in case the shareable buffer with the same mmAdrs was in the prevJob in another bufferSet.
				if( 1 ) //(bestTakePreference & TAKE_PREFERENCE_NO_SHARED_PAGES) == 0 )
				{	// some pages were shareable by other buffers

					STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryExecuteCmds_someSharePageCollisions );

					// we have to scan all prevJob buffers and discard any that are
					// 	shareable and (have page collisions or the same mmAdrs)

					// assume this command did not start any shareable discards
					Bool32 commandStartedShareableDiscards = WWSJOB_FALSE;

				#if 1 // 1 only // 1 for new single loop version (with mmAdrs check), 0 for old double loop version (withOUT mmAdrs check)

					// if the prevJob ever existed
					if( g_WwsJob_jobDataArray[prevJobNum].m_jobHasShareableBuffers )
					{	// the prevJob ever existed

						// get end of buffer pages
						U32 bufferPageNumEnd;
						bufferPageNumEnd/*exclusive*/ = miscData.m_bufferPageNum + miscData.m_numPagesPerBuffer;

						// get ptr to prevJob bufferSets
						WwsJob_BufferSet *pPrevJobBufferSets = &g_WwsJob_bufferSetArray[prevJobNum * WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB];

						// get ptr to first prevJob buffer
						U32 prevJobBufferIndex/*including job#*/ = prevJobNum * WWSJOB_MAX_NUM_BUFFERS_PER_JOB;
						pPrevJobBuffer = &g_WwsJob_bufferArray[prevJobBufferIndex];

						// get first free prevJob bufferNum (with job#)
						U32 firstFreePrevJobBufferNum = g_WwsJob_jobDataArray[prevJobNum].m_firstFreeBufferNum;

						// loop for all prevjob buffers
						do
						{
							// get buffer
							WwsJob_Buffer prevJobBuffer = *pPrevJobBuffer;

							STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryExecuteCmds_prevJobBuf, prevJobBufferIndex,
									prevJobBuffer.m_u64 );

							// if prevJobBuffer is shareable
							if( prevJobBuffer.m_shareable  )
							{
								// get prevJob bufferSet#
								prevJobBufferSetNum = prevJobBuffer.m_bufferSetNum;

								// get prevJob bufferSet
								prevJobBufferSet = pPrevJobBufferSets[prevJobBufferSetNum];

								// prevJobBufferSet must be active
								WWSJOB_VERBOSE_ASSERT( prevJobBufferSet.m_u32 );

								// get prevJob bufferNum
								U32 prevJobBufferNum = prevJobBufferIndex - prevJobBufferSet.m_firstBufferNum/*withJob#*/;

								STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryExecuteCmds_prevJobBufShare, (prevJobBufferSetNum << 8) | prevJobBufferNum,
										prevJobBufferSet.m_u32 );

								// prevJobBufferNum must be valid
								WWSJOB_ASSERT( prevJobBufferNum < prevJobBufferSet.m_numBuffers );

								// if buffers have same main mem adrs and prevBuffer is used
								bool buffersHaveSameMMAdrs = (prevJobBuffer.m_mmAddressInQwords << 4) == commandMmAddress;
								if( buffersHaveSameMMAdrs && prevJobBuffer.m_used )
								{   // this can only happen if the prevJob buffer pages do *not* overlap the buffer pages

									// you have to wait till the prevJob buffer is no longer used
									goto CommandCantExecute;
								}

								// get prevJob page configuration
								U32 prevJobFirstPageNum      = prevJobBufferSet.m_firstPageNum;
								U32 prevJobNumPagesPerBuffer = prevJobBufferSet.m_numPagesPerBuffer;
								prevJobNumBuffers		   	 = prevJobBufferSet.m_numBuffers;

								// get prevJob buffer first and last(exclusive) pageNums
								U32 prevJobBufferPageNum = prevJobFirstPageNum +
										prevJobNumPagesPerBuffer * prevJobBufferNum;
								U32 prevJobBufferPageNumEnd = prevJobBufferPageNum + prevJobNumPagesPerBuffer;

								// if buffer pages overlap or main mem adrs matches
                                if( (prevJobBufferPageNum < bufferPageNumEnd  &&
										prevJobBufferPageNumEnd > miscData.m_bufferPageNum) || buffersHaveSameMMAdrs )
								{	// buffer pages overlap or have same main mem adrs
									// prevJob buffer must be discarded

									STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_TryExecuteCmds_prevJobBufShareCollide,
											(prevJobBufferSetNum << 8) | prevJobBufferNum );

									// Try to discard the shareable buffer.  If it could not be discarded
									// This can use DmaTagId::kRunJob_writeShareableBuffers
									//	 or DmaTagId::kStoreJob_writeShareableBuffers
									if( !WwsJob_TryDumpShareBuf( prevJobBufferSet, pPrevJobBuffer, prevJobBuffer,
											prevJobBufferPageNum,
											DmaTagId::kStoreJob_writeShareableBuffers + jobIsLoadJob/*-1 or 0*/ ) )
									{	// it could not be discarded

										STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryExecuteCmds_prevJobBufBeingDumped );

										// this command started any shareable discards
										commandStartedShareableDiscards  = WWSJOB_TRUE;
										commandsStartedShareableDiscards = WWSJOB_TRUE;
									}
								}
							}

							++pPrevJobBuffer;
							++prevJobBufferIndex;
						} while(prevJobBufferIndex != firstFreePrevJobBufferNum);
					}
				#else // #if x // 1 for new single loop version, 0 for old double loop version

					// get end of buffer pages
					U32 bufferPageNumEnd;
					bufferPageNumEnd/*exclusive*/ = miscData.m_bufferPageNum + miscData.m_numPagesPerBuffer;

					// get first prevJob bufferSetNum
					prevJobBufferSetNum = prevJobNum * WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB;

					// get ptr to first prevJob bufferSet
					pPrevJobBufferSet = &g_WwsJob_bufferSetArray[prevJobBufferSetNum];

					// for all prevJob bufferSets
					for( U32 prevJobBufferSetCount = 0 ; prevJobBufferSetCount < WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB ;
							pPrevJobBufferSet++, prevJobBufferSetCount++ )
					{
						// get prevJob bufferSet
						prevJobBufferSet = *pPrevJobBufferSet;

						// if prevJob bufferSet is active
						if( prevJobBufferSet.m_u32 )
						{	// prevJob bufferSet is active

							STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryExecuteCmds_prevBufSetActive, prevJobBufferSetCount,
									prevJobBufferSet.m_u32 );

							// get prevJob page configuration
							U32 prevJobFirstPageNum      = prevJobBufferSet.m_firstPageNum;
							U32 prevJobNumPagesPerBuffer = prevJobBufferSet.m_numPagesPerBuffer;
							prevJobNumBuffers		   	 = prevJobBufferSet.m_numBuffers;

							// get ptr to first prevJob buffer
							pPrevJobBuffer = &g_WwsJob_bufferArray[prevJobBufferSet.m_firstBufferNum];

							// for all prevJob buffers
							for( U32 prevJobBufferNum = 0 ; prevJobBufferNum < prevJobNumBuffers ;
									pPrevJobBuffer++, prevJobBufferNum++ )
							{
								// get prevJob buffer
								WwsJob_Buffer prevJobBuffer = *pPrevJobBuffer;

								// if prevJob buffer is shareable
								if( prevJobBuffer.m_shareable )
								{	// prevJob buffer is shareable

									// get prevJob buffer first and last(exclusive) pageNums
									U32 prevJobBufferPageNum = prevJobFirstPageNum +
											prevJobNumPagesPerBuffer * prevJobBufferNum;
									U32 prevJobBufferPageNumEnd = prevJobBufferPageNum + prevJobNumPagesPerBuffer;

									STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryExecuteCmds_prevBufShare, prevJobBufferNum,
											prevJobBuffer.m_u64 );

									// if buffer pages overlap
									if( prevJobBufferPageNum < bufferPageNumEnd  &&
											prevJobBufferPageNumEnd > miscData.m_bufferPageNum )
									{	// buffer pages overlap
										// prevJob buffer must be discarded

										STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_TryExecuteCmds_prevBufShareCollide, prevJobBufferSetCount );

										// Try to discard the shareable buffer.  If it could not be discarded
										// This can use DmaTagId::kRunJob_writeShareableBuffers
										//	 or DmaTagId::kStoreJob_writeShareableBuffers
										if( !WwsJob_TryDumpShareBuf( prevJobBufferSet, pPrevJobBuffer, prevJobBuffer,
												prevJobBufferPageNum,
												DmaTagId::kStoreJob_writeShareableBuffers + jobIsLoadJob/*-1 or 0*/ ) )
										{	// it could not be discarded

											STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryExecuteCmds_prevBufBeingDumped );

											// this command started any shareable discards
											commandStartedShareableDiscards  = WWSJOB_TRUE;
											commandsStartedShareableDiscards = WWSJOB_TRUE;
										}
									}
								}
							}
						}
					}

				#endif // #else // #if x // 1 for new single loop version, 0 for old double loop version

					// if there were shareable buffers discarded (which are not done)
					if( commandStartedShareableDiscards )
					{	// there were shareable buffers discarded (which are not done)

						// declare we have to continue after the discard writes are done
						command.m_useBufferCommand.m_flags.m_continueAfterDiscardWrite = 1;
						*pCommands = command;

						// save buffer
						*miscData.m_pBuffer = miscData.m_buffer;

						// set m_logicalToBufferNum in logical bufferNum
						// ensure this logical buffer has not been previously used by job
						WWSJOB_ASSERT( miscData.m_pLogicalToBufferNums[logicalBufferNum] == 0xFF );
						miscData.m_pLogicalToBufferNums[logicalBufferNum] = miscData.m_bufferNum;

						STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryExecuteCmds_mustContinueLater, 0,
								( (U64)(U32)miscData.m_pBuffer << 32 ) | (U64)(U32)&miscData.m_pLogicalToBufferNums[logicalBufferNum],
								miscData.m_buffer.m_u64 );

						goto CommandCantExecute; // command not done, but it did start and must continue after discard write
					}
				}
				// no buffers were discarded which required writing out to main mem

				// set mm address & length in buffer (but don't save it yet)
				miscData.m_buffer.m_mmLengthInQwords  = command.m_useBufferCommand.m_flags.m_mmLengthInQwords;
				miscData.m_buffer.m_mmAddressInQwords = command.m_useBufferCommand.m_mmAddressInQwords;

				// get end of buffer pages
				U32 bufferPageNumEnd;
				bufferPageNumEnd/*exclusive*/ = miscData.m_bufferPageNum + miscData.m_numPagesPerBuffer;

				// if we must read into buffer
				if( command.m_useBufferCommand.m_flags.m_inputRead )
				{	// we must read into buffer

					STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryExecuteCmds_inputReadOn );

					// if the lastStoreJob shareable dma writes are not done
					if( !WwsJob_IsDmaTagMaskDone( 1 << DmaTagId::kStoreJob_writeAllShareableBuffers ) )
					{	// the lastStoreJob shareable dma writes are not done

						STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryExecuteCmds_storeShareDmaOngoing );

						UNTESTED_CODE();

						// NOTE: instead of doing the goto below, you could instead
						//	output a general dma barrier to ensure this dma is done before the input dma starts

						// we have to wait for dma to finish
						goto CommandCantExecute;
					}

					// get dmaTagId to use
					// Note this uses either DmaTagId::kLoadJob_readBuffers or DmaTagId::kRunJob_readBuffers
					U32 dmaTagId = DmaTagId::kRunJob_readBuffers + jobIsLoadJob/*-1 or 0*/ /*dmaTagId*/;

					// get ls address of start of buffer
					U32 lsAddress = miscData.m_bufferPageNum << 10;

					// get main mem adrs & length
					U32 mmAddress = miscData.m_buffer.m_mmAddressInQwords << 4;
					U32 mmLength  = miscData.m_buffer.m_mmLengthInQwords << 4;

					// if read is a gather (list element) dma
					if( command.m_useBufferCommand.m_flags.m_inputGather )
					{	// read is a gather (list element) dma

						STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryExecuteCmds_inputGatherOn );

						// it is illegal to have shareableWriteIfDiscarded=1 if you have a gathered read
						WWSJOB_ASSERT( !command.m_useBufferCommand.m_flags.m_shareableWriteIfDiscarded );

						// get ls address for list elements (near bottom of destination buffer)
						U32 lsListElements = (bufferPageNumEnd << 10) -
								(command.m_useBufferCommand.m_numPadQwordsBelowListElements << 4) - mmLength;

						// ensure you don't start before first page of buffer
						WWSJOB_ASSERT( lsListElements >= lsAddress );

						// note mmLength is #bytes in list elements

						WWSJOB_ASSERT( mmLength <= 0x4000 );

						STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_TryExecuteCmds_gatherRead, 0,
								( (U64)lsListElements << 32 ) | mmAddress, ( (U64)mmLength << 32) | dmaTagId );

						// start read of list elements into bottom of buffer
						DMA_READ( lsListElements /*lsa*/, mmAddress /*mma*/, mmLength /*length*/, dmaTagId )

						// start barriered gather read of buffer, using list elements just dma'd in!!!
						DMA_BARRIERED_GATHER_READ( lsAddress /*lsa*/, lsListElements /*lsListElements*/, \
								mmLength /*listElementLength*/, dmaTagId )
					}
					else
					{	// normal (non-gather) read

						// ensure you won't overflow end of buffer
						WWSJOB_ASSERT( mmLength <= (miscData.m_numPagesPerBuffer << 10) );

						STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_TryExecuteCmds_read, 0,
								( (U64)lsAddress << 32 ) | mmAddress, ( (U64)mmLength << 32) | dmaTagId );

						// do read of buffer, which can be > 16K
						do
						{
							// max of 16K per dma
							U32 length = (mmLength > 0x4000) ? 0x4000 : mmLength;

							// start dma of main memory into buffer
							DMA_READ( lsAddress, mmAddress, length, dmaTagId )
							lsAddress += length;
							mmAddress += length;
							mmLength  -= length;
						}while( mmLength > 0 );
					}
				}

			  // joined from above code which got input from a shared buffer
			  CheckWriteBits:

				// it is illegal to have shareableWriteIfDiscarded=1 if outputShareable=0
				WWSJOB_ASSERT( ! (command.m_useBufferCommand.m_flags.m_shareableWriteIfDiscarded  && \
						!command.m_useBufferCommand.m_flags.m_outputShareable ) );

  				if (command.m_useBufferCommand.m_flags.m_outputShareable) {
					g_WwsJob_jobDataArray[jobNum].m_jobHasShareableBuffers = WWSJOB_TRUE;
				}

				// set buffer shareable and shareableWriteIfDiscarded bits (note we don't save it yet)
				miscData.m_buffer.m_shareable 		 = command.m_useBufferCommand.m_flags.m_outputShareable;
				miscData.m_buffer.m_shareableWriteIfDiscarded = command.m_useBufferCommand.m_flags.m_shareableWriteIfDiscarded;

				// set timeStamp (used so oldest shared buffer is taken over if something must be)
				miscData.m_buffer.m_timeStamp = g_WwsJob_timeStamp;

				// save buffer (finally)
				*miscData.m_pBuffer = miscData.m_buffer;

				STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryExecuteCmds_finallySaveBuf, (logicalBufferNum << 8) | miscData.m_bufferNum,
						miscData.m_buffer.m_u64 );

				// get pageMasks
				// (note bufferPageNum set)
				QuadWord pageMask[2];
				WwsJob_GetPageMasks( miscData.m_bufferPageNum, miscData.m_numPagesPerBuffer, &pageMask[0] );

				// ensure reservedPage bits were set
				// set used page bits to 1, shareable page bits to 0 or 1
				Bool32 shareable = miscData.m_buffer.m_shareable;
				for( U32 i = 0 ; i < 2 ; i++ )
				{
					for( U32 j = 0 ; j < 2 ; j++ )
					{
						// ensure reservedPage bits were set
						WWSJOB_ASSERT( (g_WwsJob_reservedPageMask[i].m_u64[j] & pageMask[i].m_u64[j]) == pageMask[i].m_u64[j] );

						// note used and shareable page bits might be 0 or 1, so don't assert on them

						// set used page bits to 1
						g_WwsJob_usedPageMask[i].m_u64[j] |= pageMask[i].m_u64[j];

						// set shareable page bits to 0 or 1
						if( shareable )
							g_WwsJob_shareablePageMask[i].m_u64[j] |=  pageMask[i].m_u64[j];
						else
							g_WwsJob_shareablePageMask[i].m_u64[j] &= ~pageMask[i].m_u64[j];
					}
				}

				STORE_VERBOSE_AUDIT( AuditId::kWwsJob_usedPages, 0,
						g_WwsJob_usedPageMask[0].m_u64[0], g_WwsJob_usedPageMask[0].m_u64[1],
						g_WwsJob_usedPageMask[1].m_u64[0], g_WwsJob_usedPageMask[1].m_u64[1] );
				STORE_VERBOSE_AUDIT( AuditId::kWwsJob_shareablePages, 0,
						g_WwsJob_shareablePageMask[0].m_u64[0], g_WwsJob_shareablePageMask[0].m_u64[1],
						g_WwsJob_shareablePageMask[1].m_u64[0], g_WwsJob_shareablePageMask[1].m_u64[1] );

				// get ptr to logicalBuffer
				//WwsJob_Buffer *pLogicalBuffer = &miscData.m_pBuffers[logicalBufferNum];

				// ensure logical buffer has not been used by job
				WWSJOB_ASSERT( command.m_useBufferCommand.m_flags.m_continueAfterDiscardWrite  ||  miscData.m_pLogicalToBufferNums[logicalBufferNum] == 0xFF );

				// set logicalToBufferNum
				miscData.m_pLogicalToBufferNums[logicalBufferNum] = miscData.m_bufferNum;

				STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryExecuteCmds_setLogicalToBufNum, (logicalBufferNum << 8) | miscData.m_bufferNum,
						(U64)(U32)&miscData.m_pLogicalToBufferNums[logicalBufferNum] );

				// change command to CommandNum::kNop (which is 0!!!) so it won't execute again
				command.m_u64 = 0;
				*pCommands = command;

				// command executed
				continue;
		   	}

			case CommandNum::kRequestDependencyDecrement:

				U32 mmDependencyAddress = command.m_depDecCommand.m_mmDependencyAddress;

				STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryExecuteCmds_requestDepDec, 0, mmDependencyAddress );

				//The job manager needs to remember that this job has requested for a dependency to be
				//decremented *after* the output dmas have all completed

				//So add it to the dependency list that will be managed later
				WwsJob_JobData* pJobData = &g_WwsJob_jobDataArray[jobNum];
				U32 depNum = pJobData->m_numDependencies;
				WWSJOB_ASSERT( depNum < WwsJob_kMaxDependenciesPerJob );
				pJobData->m_eaDependency[depNum]	= mmDependencyAddress;
				pJobData->m_numDependencies			= depNum + 1;

				// change command to CommandNum::kNop (which is 0!!!) so it won't execute again
				command.m_u64 = 0;
				*pCommands = command;

				continue;

			case CommandNum::kRunJob:

				STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryExecuteCmds_runJob );

				// only the loadJob can execute this command
				WWSJOB_ASSERT( jobIsLoadJob );

				// if any command started shareable discards
				if( commandsStartedShareableDiscards )
				{	// any command started shareable discards

					STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryExecuteCmds_startWriteShareBufInt );

					// output barriered dma with stall & notify interrupt
					//This interrupt fires after all dmas on DmaTagId::kRunJob_writeShareableBuffers have completed
					WwsJob_StartTagSpecificBarrieredNullListWithInterrupt( DmaTagId::kRunJob_writeShareableBuffers );
				}

				// if any commands before this are not done
				if( !allCommandsDone )
				{	// any commands before this are not done

					STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryExecuteCmds_abortRunSomeCmdsNotDone );

					// can't run the job, all commands are not done
					return WWSJOB_FALSE;
				}

				// bufferSet must be active before using a buffer in it
				WWSJOB_ASSERT( miscData.m_bufferSet.m_u32 );

				// logicalBufferNum must be legal
				WWSJOB_ASSERT( logicalBufferNum < miscData.m_numBuffers );

				// we are ready to run the job (as far as the load commands knows)

				// save lsa of job code buffer
				g_WwsJob_pJobCodeBufferSet		= miscData.m_pBufferSet;
				g_WwsJob_pJobCodeBuffer			= miscData.m_pBuffer;
				g_WwsJob_lsaJobCodeBuffer		= miscData.m_bufferPageNum << 10;
				g_WwsJob_bBreakpointRequested	= g_WwsJob_jobHeader.m_enableBreakpoint;
//				g_WwsJob_bBreakpointRequested	= command.m_runJobCommand.m_flags.m_enableBreakpoint;

        // debug code to catch spurious 2nd execution of commands after job is started
				command.m_u64 = 0;
				command.m_runJobCommand.m_flags.m_commandNum = CommandNum::kInvalidCommand;
				*pCommands = command;

                // get adrs of parameters (on qword bound)
				pCommands++;
				if( (U32)pCommands & 8 )
					pCommands++;

				// set ptr to parameters
				g_WwsJob_dataForJob.m_pParameters = pCommands;

				STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_TryExecuteCmds_readyToRunJob, miscData.m_bufferPageNum,
						( (U64)(U32)miscData.m_pBuffer << 32 ) | (U32)pCommands/*ptr to params*/,
						*(U64*)pCommands/*1st param dword*/ );

				// all commands were done
				return WWSJOB_TRUE;


			case CommandNum::kEndCommand:

				STORE_VERBOSE_AUDIT( AuditId::kWwsJob_TryExecuteCmds_endCmd );

				// only the runJob can execute this command
				WWSJOB_ASSERT( !jobIsLoadJob );

				return allCommandsDone;


			default:
				// unknown command
				WWSJOB_ASSERT(0);
		}
		// no code flow here

	  CommandCantExecute:

		STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_TryExecuteCmds_cmdCantExecute );

		allCommandsDone = WWSJOB_FALSE;
	}

	// no code flow here
}
#endif

inline U64 GetJobCodeUniqueId( const void* ptr )
{
	const vector signed char	shuffleMask=(vector signed char) { 0x00, 0x01, 0x04, 0x05, 0x08, 0x09, 0x0C, 0x0D, 0x00, 0x01, 0x04, 0x05, 0x08, 0x09, 0x0C, 0x0D };
	qword ilas = *(qword*)ptr;
	qword temp = si_shli( ilas, 7 );
	qword uniqueId = si_shufb( temp, temp, shuffleMask );
	return si_to_ullong( uniqueId );
}


//====================================================================================================================
//
//	misc mid level code
//
//====================================================================================================================

/**	\brief  Change the loadJob to a runJob (start running it).

	Before calling this, ensure there is a loadJob which is ready to become a runJob, and there is no runJob.
	This is called from main().
**/
#if WWS_JOB_USE_C_VERSION!=0
void WwsJob_ChangeLoadToRunJob( void )
{
	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_ChangeLoadToRunJob, g_WwsJob_loadJobNum );

	// Change loadJob to runJob, declare no loadJob
	g_WwsJob_runJobNum = g_WwsJob_loadJobNum;
	g_WwsJob_runJobState = WwsJob_RunJobState::kLoadNotAllowed;
	g_WwsJob_loadJobNum = -1;
	g_WwsJob_loadJobState = WwsJob_LoadJobState::kNone;

	// get ptr to start of code buffer, which is also ptr to CodeBufferHeader
	const SpuModuleHeader *pJobCodeHeader = (const SpuModuleHeader*)g_WwsJob_lsaJobCodeBuffer;

	// verify this is a valid SpuModuleHeader
	WWSJOB_ASSERT( pJobCodeHeader->m_codeMarker == kSpuModuleMarker );

	// ensure the bss fits within the code buffer
	WWSJOB_ASSERT( (pJobCodeHeader->m_bssOffset + pJobCodeHeader->m_bssSize) <= \
			((U32)g_WwsJob_pJobCodeBufferSet->m_numPagesPerBuffer << 10) );

	//Note: The bss is zeroed in the crt0 of the job rather than by the job manager to give control to the user

	//Fill in the jobId
	U32 jobIndex			= g_WwsJob_jobDataArray[g_WwsJob_runJobNum].m_jobIndex;
	g_WwsJob_dataForJob.m_jobId	= (g_WwsJob_spursWorkloadId << 16) | jobIndex;
	g_WwsJob_dataForJob.m_bJobAuditsEnabled = AreJobAuditsEnabled();

	// get ptr to start of code
	WwsJob_JobCodePtr pJobCode = (WwsJob_JobCodePtr)( g_WwsJob_lsaJobCodeBuffer + pJobCodeHeader->m_entryOffset );

	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_ChangeLoadToRunJob_aboutToRunJob, 0,
		( (U64)g_WwsJob_lsaJobCodeBuffer << 32 ) | jobIndex );

	STORE_TIMING_AUDIT( AuditId::kWwsJob_ChangeLoadToRunJob_jobBegin, jobIndex, GetJobCodeUniqueId( pJobCodeHeader ) );

  #ifndef PRESERVE_INTERRUPT_STATUS
	// interrupts are enabled while job runs
	__builtin_spu_ienable();
  #endif

	//If a breakpoint was requested on this RunJob then we pass it in as the second parameter
	//and it's up to the job's crt0 to implement the breakpoint

	// if you want to debug the jobcrt0 (without source, before executing the job), then enable this breakpoint
	//WWSJOB_BREAKPOINT();


	// run the job code
	//U32 initAndShutdownTime = (pJobCode) ( &g_WwsJob_dataForJob, g_WwsJob_bBreakpointRequested );
	(pJobCode) ( &g_WwsJob_dataForJob, g_WwsJob_bBreakpointRequested );

	// ... job is running


	// flow returns here when job exits


  #ifndef PRESERVE_INTERRUPT_STATUS
	//Check interrupts were still enabled in user code
	WWSJOB_ASSERT( !AreInterruptsEnabled() );
  #endif

	// interrupts are disabled in jobManager
	//	__builtin_spu_idisable();
	//jobctr0 now does bid at the end, so interrupts should already be disabled on return

	// job must free all unassigned dmaTagId's it took
	WWSJOB_VERBOSE_ASSERT( g_WwsJob_numUnassignedDmaTagIds == 0 );

	//STORE_TIMING_AUDIT( AuditId::kWwsJob_ChangeLoadToRunJob_jobEnd, 0, initAndShutdownTime );
	STORE_TIMING_AUDIT( AuditId::kWwsJob_ChangeLoadToRunJob_jobEnd );

	// runJob *must* enable next loadJob before exiting
	WWSJOB_ASSERT( g_WwsJob_runJobState == WwsJob_RunJobState::kLoadAllowed );
}
#endif


/**	\brief  Free a logical (non-shared) buffer which the job has used

	Note this will not try to execute the load commands
	This is called by WwsJob_JobApi only

	\param	jobNum = job number (0:2, currently only g_WwsJob_runJobNum in case you want to remove this param)
	\param  logicalBufferSetNum = bufferSet # from job
	\param	logicalBufferNum = buffer # from job (which might be different from physical buffer# returned)
**/
#if WWS_JOB_USE_C_VERSION!=0
void WwsJob_FreeLogicalBuffer( I32 jobNum, U32 logicalBufferSetNum, U32 logicalBufferNum )
{
	STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_FreeLogicalBuffer, (logicalBufferSetNum << 8) | logicalBufferNum );

	WWSJOB_VERBOSE_ASSERT( logicalBufferSetNum < WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB  &&
			logicalBufferNum < WWSJOB_MAX_NUM_BUFFERS_PER_JOB );

	// Set pBufferSet, bufferSet, bufferSetFirstPageNum, numPagesPerBuffer, numBuffers, pBuffers, bufferNum, pBuffer, buffer, bufferPageNum, pLogicalToBufferNums
	WwsJob_MiscData miscData;
	WwsJob_GetLogicalBuffer( jobNum, logicalBufferSetNum, logicalBufferNum, miscData );

	// BufferSet *must* be active (that is, it must have been reserved by loadJob or runJob)
	WWSJOB_ASSERT( miscData.m_bufferSet.m_u32 != 0 );

	// buffer must have been used by job
	WWSJOB_ASSERT( miscData.m_bufferNum != 0xff );

	// buffer can *not* be shareable
	WWSJOB_ASSERT( !( miscData.m_buffer.m_shareable || miscData.m_buffer.m_sharedToLaterJob ) );

	// buffer can *not* have shareableWriteIfDiscarded bit set
	WWSJOB_VERBOSE_ASSERT( !miscData.m_buffer.m_shareableWriteIfDiscarded );

	// WwsJob_Buffer *must* be used by job (not just reserved or already freed)
	WWSJOB_ASSERT( miscData.m_buffer.m_used );

    // if buffer has any ongoing dma, then wait till dma is done
    U32 dmaTagId = miscData.m_buffer.m_dmaTagId;

#if 1 // 1 = Jon's new code for downcoding, 0 = John's original C code

    U32 dmaTagMask = 1 << dmaTagId;
    do
    {
        // wait till dma is done
		__builtin_spu_ienable();
		__builtin_spu_idisable();
    }while( dmaTagId  &&  !WwsJob_IsDmaTagMaskDone( dmaTagMask ) );

    // declare buffer as inactive, save it
    *(U32*)miscData.m_pBuffer = 0;

    // try to free dmaTagId and used pages.
    // Note: it will be done since we just ensured there is no ongoing dma
    (void)WwsJob_TryFreeTagAndUsedPages( miscData.m_bufferSet, miscData.m_bufferNum, miscData.m_buffer );

	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_FreeLogicalBuffer_setInactive, 0,
			miscData.m_pBuffer->m_u64 );

#else // #if x // 1 = Jon's new code for downcoding, 0 = John's original C code

	do
	{
		// wait till dma is done
		__builtin_spu_ienable();
		__builtin_spu_idisable();
	}while( dmaTagId  &&  !WwsJob_IsDmaTagMaskDone( 1 << dmaTagId ) );

	// try to free dmaTagId and used pages.
	// Note: it will be done since we just ensured there is no ongoing dma
	(void)WwsJob_TryFreeTagAndUsedPages( miscData.m_bufferSet, miscData.m_bufferNum, miscData.m_buffer );

	// declare buffer is inactive, save it
	miscData.m_buffer.m_u32[0] = 0;
	*miscData.m_pBuffer = miscData.m_buffer;

	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_FreeLogicalBuffer_setInactive, 0,
			miscData.m_buffer.m_u64 );

#endif // #if x // 1 = Jon's new code for downcoding, 0 = John's original C code
}
#endif


/**	\brief	Get the info for a used buffer, optionally allocating a dmaTagId to it

	This is called by WwsJob_JobApi only.

	\param	logicalBufferSetNum = bufferSet# from job
	\param	logicalBufferNum = buffer# from job (which might be different from physical buffer# returned)
	\param	useDmaTagId = true if you want to allocate a dmaTagId and assign it to this buffer.
**/
#if WWS_JOB_USE_C_VERSION!=0
WwsJob_BufferTag WwsJob_GetBufferTag( U32 logicalBufferSetNum, U32 logicalBufferNum, Bool32 useDmaTagId )
{
	STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_GetBufferTag, logicalBufferSetNum,
			((U64)logicalBufferNum << 32) | useDmaTagId );

	// Set pBufferSet, bufferSet, bufferSetFirstPageNum, numPagesPerBuffer, numBuffers, pBuffers, bufferNum, pBuffer, buffer, bufferPageNum, pLogicalToBufferNums
	WwsJob_MiscData miscData;
	WwsJob_GetLogicalBuffer( g_WwsJob_runJobNum, logicalBufferSetNum, logicalBufferNum, miscData );

	// bufferSet must be active
	WWSJOB_ASSERT( miscData.m_bufferSet.m_u32 );

	// logicalBufferNum must be legal
	WWSJOB_ASSERT( logicalBufferNum < miscData.m_numBuffers );

	// buffer must have been used by job
	WWSJOB_ASSERT( miscData.m_bufferNum != 0xFF );

	// buffer must be active and not reserved (that is, it has to be in use)
	WWSJOB_ASSERT( miscData.m_buffer.m_u32[0]  &&  !miscData.m_buffer.m_reserved );

	// ensure m_bufferSetNum is valid
	WWSJOB_ASSERT( miscData.m_buffer.m_bufferSetNum == logicalBufferSetNum );

	// we will return a bufferTag
	WwsJob_BufferTag bufferTag;

	// we might need to load the loadJob buffer
	WwsJob_Buffer *pLoadJobBuffer = NULL;
	WwsJob_Buffer loadJobBuffer;

	U32 dmaTagId;

	// get dmaTagId from buffer (a unique dmaTagId could already exist)
	if( miscData.m_buffer.m_sharedToLaterJob )
	{	// rare case - we have to get dmaTagId from later loadJob

		// get loadJob bufferSet
		WwsJob_BufferSet loadJobBufferSet = g_WwsJob_bufferSetArray[
				g_WwsJob_loadJobNum * WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB + logicalBufferSetNum ];

		// get loadJob bufferNum (with jobNum)
		U32 loadJobBufferNum = loadJobBufferSet.m_firstBufferNum/*withJobNum*/ + miscData.m_bufferNum;

		// get ptr to loadJob buffer, and get that buffer
		pLoadJobBuffer = &g_WwsJob_bufferArray[ loadJobBufferNum ];
		loadJobBuffer = *pLoadJobBuffer;

		STORE_VERBOSE_AUDIT( AuditId::kWwsJob_GetBufferTag_sharedToLaterJob, (g_WwsJob_loadJobNum << 8) | loadJobBufferNum,
				loadJobBuffer.m_u64 );

		// get dmaTagId from loadJob
		dmaTagId = loadJobBuffer.m_dmaTagId;
	}
	else
	{	// normal case.  The dmaTagId is stored to this buffer
		dmaTagId = miscData.m_buffer.m_dmaTagId;
	}

	// set the bufferTag *before* WwsJob_UseDmaTagId is called, so that the s_... stuff is still valid
	bufferTag.m_logicalBufferSetNum = logicalBufferSetNum;
	bufferTag.m_logicalBufferNum	= logicalBufferNum;
	bufferTag.m_dmaTagId			= dmaTagId;
	bufferTag.m_lsAddressInWords	= miscData.m_bufferPageNum     << (10-2);
	bufferTag.m_lsLengthInWords		= miscData.m_numPagesPerBuffer << (10-2);
	bufferTag.m_mmAddress			= miscData.m_buffer.m_mmAddressInQwords << 4;
	bufferTag.m_mmLength			= miscData.m_buffer.m_mmLengthInQwords << 4;

	// if user wants a unique dmaTagId  and  there was no unique dmaTagId already in the buffer
	if( useDmaTagId  &&  !dmaTagId )
	{	// user wants a unique dmaTagId  and  there was no unique dmaTagId already in the buffer

		// get a dmaTagId.  If it is not available then wait until it one is
		dmaTagId = WwsJob_UseDmaTagId();

		// set bufferTag in buffer & save buffer
		if( miscData.m_buffer.m_sharedToLaterJob )
		{	// rare case

			loadJobBuffer.m_dmaTagId = dmaTagId;
			*pLoadJobBuffer = loadJobBuffer;

			STORE_VERBOSE_AUDIT( AuditId::kWwsJob_GetBufferTag_gotDmaTagFromLoadJob, dmaTagId,
				loadJobBuffer.m_u64 );
		}
		else
		{	// normal case

			miscData.m_buffer.m_dmaTagId = dmaTagId;
			*miscData.m_pBuffer = miscData.m_buffer;

			STORE_VERBOSE_AUDIT( AuditId::kWwsJob_GetBufferTag_gotDmaTag, dmaTagId,
				miscData.m_buffer.m_u64 );
		}

		// set in bufferTag
		bufferTag.m_dmaTagId = dmaTagId;
	}

	STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_GetBufferTag_returnBufTag, 0,
			bufferTag.m_u64[0], bufferTag.m_u64[1] );

	return bufferTag;
}
#endif


/**	\brief	Get misc data for a logical buffer

	This will set a bunch of data for the logical buffer, for subsequent use by the calling code.

	Input parameters:
		\param	jobNum = job number (0:2)
		\param	logicalBufferSetNum = bufferSet# from job
		\param	logicalBufferNum = buffer# within bufferSet from job

	Output parameter, set by reference:
		\param	miscData.  This data is a structure of the following (to reduce the number of things passed by reference):
							pBufferSet = ptr to bufferSet in g_WwsJob_bufferSetArray[]
							bufferSet = local copy of bufferSet data
							bufferSetFirstPageNum = page# of first physical buffer in bufferSet
							numPagesPerBuffer = #pages per buffer
							numBuffers = #buffers in bufferSet (declared by reserveBufferSet)
							pBuffers = ptr to first physical buffer (of all those in bufferSet) in g_WwsJob_bufferArray[]
							bufferNum = physical buffer# in bufferSet (may be different from input logical buffer#)
							pBuffer = ptr to physical buffer in g_WwsJob_bufferArray[]
							buffer = local copy of physical buffer data in g_WwsJob_bufferArray[]
							bufferPageNum = first page # of physical buffer
							pLogicalToBufferNums = ptr to first physical buffer (of all those in bufferSet) in g_WwsJob_logicalToBufferNumArray[]
**/
#if WWS_JOB_USE_C_VERSION!=0
void WwsJob_GetLogicalBuffer( I32 jobNum, U32 logicalBufferSetNum, U32 logicalBufferNum, WwsJob_MiscData &miscData )
{
	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_GetLogicalBuffer, (jobNum << 12) | (logicalBufferSetNum << 6) | logicalBufferNum );

	WWSJOB_VERBOSE_ASSERT( logicalBufferSetNum < WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB );
	WWSJOB_VERBOSE_ASSERT( logicalBufferNum    < WWSJOB_MAX_NUM_BUFFERS_PER_JOB );

	// get physical bufferSetNum{0:0x17}
	U32 bufferSetNum = (jobNum * WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB) + logicalBufferSetNum;

	// get pointer to bufferSet
	miscData.m_pBufferSet = &g_WwsJob_bufferSetArray[bufferSetNum];

	// get bufferSet in static struct
	miscData.m_bufferSet = *miscData.m_pBufferSet;

	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_GetLogicalBuffer_bufferSet, 0,
			( (U64)(U32)miscData.m_pBufferSet << 32 ) | miscData.m_bufferSet.m_u32 );

	// set page congiruation variables
	miscData.m_bufferSetFirstPageNum	= miscData.m_bufferSet.m_firstPageNum;
	miscData.m_numPagesPerBuffer		= miscData.m_bufferSet.m_numPagesPerBuffer;
	miscData.m_numBuffers		        = miscData.m_bufferSet.m_numBuffers;
	U32 firstBufferNum		= miscData.m_bufferSet.m_firstBufferNum;

	// get ptr to buffers
	miscData.m_pBuffers = &g_WwsJob_bufferArray[firstBufferNum];

	// get ptr to logicalToBufferNums
	miscData.m_pLogicalToBufferNums = &g_WwsJob_logicalToBufferNumArray[firstBufferNum];

	// get physical buffer for this logical buffer
	miscData.m_bufferNum = miscData.m_pLogicalToBufferNums[logicalBufferNum];

	// get ptr to bufferNum
	miscData.m_pBuffer = &miscData.m_pBuffers[miscData.m_bufferNum];

	// get buffer in static struct
	miscData.m_buffer = *miscData.m_pBuffer;

	// get buffer first pageNum
	miscData.m_bufferPageNum = miscData.m_bufferSetFirstPageNum + miscData.m_numPagesPerBuffer * miscData.m_bufferNum;

	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_GetLogicalBuffer_buffer, (miscData.m_bufferNum << 8) | miscData.m_bufferPageNum,
			( (U64)(U32)miscData.m_pBuffers << 40 ) | ( (U64)(U32)miscData.m_pBuffer << 20 ) | (U64)(U32)miscData.m_pLogicalToBufferNums,
			miscData.m_buffer.m_u64 );
}
#endif


/**	\brief	Poll Spurs kernel to see which PM goes next, and what workload it uses.

	This version tracks how often it is called and only calls through to WwsJob_PollSpursKernel occasionally

	Return true if the Wws jobManager is to run.
**/
#if WWS_JOB_USE_C_VERSION!=0
Bool32 WwsJob_PollSpursKernelQuick( void )
{
	enum
	{
		kNumCallsBetweenPolls = 4,
	};

	//Rather that polling the Spurs Kernel every time round,
	//just assume we're staying for a few runs and then only poll once in a while

	//Note that any time the job manager assigns to the ready count,
	//it resets this variable to make sure the kernel is polled again

	U32 currCount = g_countToNextPoll;

	if ( currCount )
	{
		g_countToNextPoll = currCount - 1;
		return true;
	}
	else
	{
		g_countToNextPoll = kNumCallsBetweenPolls;
		return WwsJob_PollSpursKernel();	//We don't call it as often now
	}
}
#endif

/**	\brief	Poll Spurskernel to see which PM goes next, and what workload it uses.

	Return true if the Wws jobManager is to run.
**/
#if WWS_JOB_USE_C_VERSION!=0
Bool32 WwsJob_PollSpursKernel( void )
{
	STORE_TIMING_AUDIT( AuditId::kWwsJob_PollSpursKernel_begin );

	// If Spurs wants us to shutdown, then stop allocating
	// new jobs and just wait for the pipeline to end

	Bool32 isPreempted = cellSpursPoll();
	if ( isPreempted )
	{
		//Policy Module is pre-empted by another policy module or another workload on this policy module
		g_countToNextPoll = 0;	//invalidate our count to next poll so that we don't go assuming we're running
	}

	STORE_TIMING_AUDIT( AuditId::kWwsJob_PollSpursKernel_end );

	return (!isPreempted);
}
#endif


/**	\brief	Try to take a job from the workload

	If a job is taken, then set g_WwsJob_jobIndex.
	Return true if we got a job.
**/
#if WWS_JOB_USE_C_VERSION!=0
Bool32 WwsJob_GetJobFromJobList( void )
{
	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_GetJobFromJobList );

	/* check if any work to be executed exists */
	SpuJobHeader spuJobHeader = WwsJob_AllocateJob( g_WwsJob_eaWorkLoad, g_WwsJob_dataForJob.m_spuNum );
	g_WwsJob_jobHeader	= spuJobHeader.m_jobHeader;
	g_WwsJob_jobIndex	= spuJobHeader.m_jobIndex;

	if ( g_WwsJob_jobHeader.m_jobHeaderCommand != JobHeaderCommand::kNoJobYet )
	{
		return true;
	}

	//No jobs left.
	//We're about to change the ready count value for our workload,
	//so invalidate our count to next poll to make sure we poll next time around
	g_countToNextPoll = 0;

	//No work waiting
	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_GetJobFromJobList_noWork );

	//So set the ready count to zero

	/*
	 *	:WARNING: New Spurs release has changed the format of the cellSpursReadyCountSwap
	 *	and cellSpursReadyCountCompareAndSwap functions.
	 *	I've taken guesses here at how to use these new functions, but I don't actually
	 *	know if this is right at all.  Needs checking once we get new documentation.
	 */
	STORE_TIMING_AUDIT( AuditId::kWwsJob_setSpursKernelReadyCount_begin_zero );

	unsigned int oldReadyCount = _cellSpursReadyCountSwap( g_WwsJob_spursWorkloadId, 0 );

	STORE_TIMING_AUDIT( AuditId::kWwsJob_setSpursKernelReadyCount_end_zero );

	//Double check that there definitely isn't work on the joblist
	spuJobHeader	= WwsJob_AllocateJob( g_WwsJob_eaWorkLoad, g_WwsJob_dataForJob.m_spuNum );
	g_WwsJob_jobHeader		= spuJobHeader.m_jobHeader;
	g_WwsJob_jobIndex		= spuJobHeader.m_jobIndex;

	if ( g_WwsJob_jobHeader.m_jobHeaderCommand != JobHeaderCommand::kNoJobYet )
	{
		STORE_VERBOSE_AUDIT( AuditId::kWwsJob_GetJobFromJobList_2ndTryHasWork );

		//There's work there now.  Must have been just added.
		/* restore readyCount by compare&swap */
		/* It may fail if someone sets new readyCount */
		//unsigned int newOldValue =

		STORE_TIMING_AUDIT( AuditId::kWwsJob_setSpursKernelReadyCount_begin_old );

		_cellSpursReadyCountCompareAndSwap(	g_WwsJob_spursWorkloadId,
											0,
											oldReadyCount );

		STORE_TIMING_AUDIT( AuditId::kWwsJob_setSpursKernelReadyCount_end_old );

		return true;
	}

	return false;
}
#endif


/**	\brief	Return to SpursKernel.

	Before returning, ensure pages and dmaTagId's are in same state as when initialized.
	This does not return.
	This is called from main().
**/
#if WWS_JOB_USE_C_VERSION!=0
void WwsJob_ReturnToSpursKernel( void )
{
	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_ReturnToSpursKernel );

	// Verify that dmaTagId's match initial value.
	// This will fail if a job uses a dmaTagId but forgets to free it.
	// The job may have executed very many jobs ago.
	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_dmaTags, 0, g_WwsJob_usedDmaTagMask );
	WWSJOB_ASSERT( g_WwsJob_usedDmaTagMask == (1 << DmaTagId::kNumUsed) - 1 );

	// verify that page masks are same as initial value!
	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_reservedPages, 0,
			g_WwsJob_reservedPageMask[0].m_u64[0], g_WwsJob_reservedPageMask[0].m_u64[1],
			g_WwsJob_reservedPageMask[1].m_u64[0], g_WwsJob_reservedPageMask[1].m_u64[1] );
	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_usedPages, 0,
			g_WwsJob_usedPageMask[0].m_u64[0], g_WwsJob_usedPageMask[0].m_u64[1],
			g_WwsJob_usedPageMask[1].m_u64[0], g_WwsJob_usedPageMask[1].m_u64[1] );
	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_shareablePages, 0,
			g_WwsJob_shareablePageMask[0].m_u64[0], g_WwsJob_shareablePageMask[0].m_u64[1],
			g_WwsJob_shareablePageMask[1].m_u64[0], g_WwsJob_shareablePageMask[1].m_u64[1] );

	for( U32 i = 0 ; i < 2 ; i++ )
	{
		for( U32 j = 0 ; j < 2 ; j++ )
		{
			// ensure reserved pages match value at initialization
			WWSJOB_ASSERT( g_WwsJob_reservedPageMask[i].m_u64[j] == g_WwsJob_initialPageMask[i].m_u64[j] );

			// ensure used pages match value at initialization
			WWSJOB_ASSERT( g_WwsJob_usedPageMask[i].m_u64[j] == g_WwsJob_initialPageMask[i].m_u64[j] );

			// ensure shareable pages match value at initialization
			WWSJOB_ASSERT( g_WwsJob_shareablePageMask[i].m_u64[j] == 0 );
		}
	}

	STORE_WORKLOAD_AUDIT( AuditId::kWwsJob_end );
	FinalizeAudits();

	//Clean up
	ShutdownInterruptHandler();

	CheckStackOverflowMarker();

	// and return to Spurs Kernel
	_cellSpursModuleExit();

	// there is no flow here
}
#endif




//====================================================================================================================
//
//	misc low level code
//
//====================================================================================================================

/**	\brief	Free a dmaTagId

	Free a dmaTagId which was previously used.
**/
#if WWS_JOB_USE_C_VERSION!=0
void WwsJob_FreeDmaTagId( U32 dmaTagId )
{
	// make sure dmaTagId is 1:31
	WWSJOB_ASSERT( dmaTagId > 0  &&  dmaTagId < 32 );

	U32 dmaTagMask = 1 << dmaTagId;

	// make sure dmaTagId was used
	WWSJOB_ASSERT( g_WwsJob_usedDmaTagMask & dmaTagMask );

	// clear used bit
	g_WwsJob_usedDmaTagMask &= ~dmaTagMask;

	STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_FreeDmaTadId, dmaTagId,
			g_WwsJob_usedDmaTagMask );
}
#endif


/**	\brief	Get page masks, used to test for used or shareable pages

	For the range of pages specified by the input, return 1 in the pages masks (the rest are 0)

	\param	firstPageNum = page# where pages start (1K per page)
	\param	numPages = #pages (including firstPageNum)
	\param	pPageMask = ptr where to store the output page masks (256 bits total).
**/
#if WWS_JOB_USE_C_VERSION!=0
void WwsJob_GetPageMasks( U32 firstPageNum, U32 numPages, QuadWord *pPageMask )
{
	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_GetPageMasks, (firstPageNum << 8) | numPages );

	//	leftShiftRight 		leftShiftLeft		rightShiftRight     rightShiftLeft
	//	------------->      <------------		-------------->     <-------------
	//	000000000000001111110000000000000		0000000000000001111100000000000000

	U32	pageEnd/*1:256 exclusive*/ = firstPageNum + numPages;
	U32	leftShiftRight 	= firstPageNum;
	I32 leftShiftLeft  	= 128 - pageEnd;
	if( leftShiftLeft < 0 )
		leftShiftLeft 	= 0;
	I32 rightShiftRight	= firstPageNum - 128;
	if( rightShiftRight < 0 )
		rightShiftRight = 0;
	U32 rightShiftLeft 	= 256 - pageEnd;

	VU32 vPageMaskLeftShiftRight , vPageMaskLeftShiftLeft ;
	VU32 vPageMaskRightShiftRight, vPageMaskRightShiftLeft;
	vPageMaskLeftShiftRight 	= spu_rlmaskqwbytebc( g_WwsJob_vNeg1, 7-leftShiftRight );
	vPageMaskLeftShiftRight		= spu_rlmaskqw( vPageMaskLeftShiftRight, -leftShiftRight );
	vPageMaskLeftShiftLeft		= spu_slqwbytebc( g_WwsJob_vNeg1, leftShiftLeft );
	vPageMaskLeftShiftLeft 		= spu_slqw( vPageMaskLeftShiftLeft, leftShiftLeft );
	pPageMask[0].m_vu32		 	= spu_and( vPageMaskLeftShiftRight, vPageMaskLeftShiftLeft );
	vPageMaskRightShiftRight	= spu_rlmaskqwbytebc( g_WwsJob_vNeg1, 7-rightShiftRight );
	vPageMaskRightShiftRight	= spu_rlmaskqw( vPageMaskRightShiftRight, -rightShiftRight );
	vPageMaskRightShiftLeft		= spu_slqwbytebc( g_WwsJob_vNeg1, rightShiftLeft );
	vPageMaskRightShiftLeft 	= spu_slqw( vPageMaskRightShiftLeft, rightShiftLeft );
	pPageMask[1].m_vu32			= spu_and( vPageMaskRightShiftRight, vPageMaskRightShiftLeft );
}
#endif


/**	\brief	Check if dma(s) are done

	\param	dmaTagMask is a mask of the dmaTagId's to check (bit 31:0 for dmaTagId's 0:31)
**/
#if WWS_JOB_USE_C_VERSION!=0
Bool32 WwsJob_IsDmaTagMaskDone( U32 dmaTagMask )
{
	//Clear MFC tag update
	spu_writech( MFC_WrTagUpdate, 0x0 );
	while ( spu_readchcnt( MFC_WrTagUpdate ) != 1 )
	{
		//Read the count until 1 is returned
	}
	spu_readch( MFC_RdTagStat );	// Then read tag-status

	// Set tag update and wait for completion
	spu_writech( MFC_WrTagMask, dmaTagMask );
	spu_writech( MFC_WrTagUpdate, 0x0 );
	Bool32 done = ( spu_readch( MFC_RdTagStat ) == dmaTagMask );

	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_IsDmaTagMaskDone, 0,
			( (U64)dmaTagMask << 32) | done );

	return done;
}
#endif


/**	\brief	Set dwords to a value

	\param	pMem = local dword aligned adrs to start the store
	\param	size = #bytes (8 aligned, non-zero!)
	\param	data = 64 bits of data to fill with
**/
#if WWS_JOB_USE_C_VERSION!=0
void WwsJob_SetDwordMem( U64 *pMem, U32 size, U64 data )
{
	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_SetDwordMem, 0,
			( (U64)(U32)pMem << 32 ) | size, data );

	// pMem must be dword aligned
	WWSJOB_VERBOSE_ASSERT( ( (U32)pMem & 0x7 ) == 0 );

	// size must be < 0x40000  and  dword aligned
	WWSJOB_ASSERT( (size & 0xFFFC0007) == 0 );

	// set the memory (size is non-zero)
	U32 pMemEnd = (U32)pMem + size;
	do
	{
		*pMem = data;
		pMem++;
	}while( (U32)pMem != pMemEnd );
}
#endif


/**	\brief	Set qwords to a value

	\param	pMem = local qword aligned adrs to start the store
	\param	size = #bytes (16 aligned, non-zero!)
	\param	data = 64 bits of data to fill with (duplicated to fill qword)
**/
#if WWS_JOB_USE_C_VERSION!=0
void WwsJob_SetQwordMem( void* pMem, U32 size, U64 data )
{
	STORE_VERBOSE_AUDIT( AuditId::kWwsJob_SetQwordMem, 0,
			( (U64)(U32)pMem << 32 ) | size, data );

	// pMem must be qword aligned
	WWSJOB_VERBOSE_ASSERT( ( (U32)pMem & 0xF ) == 0 );

	// size must be < 0x40000  and  qword aligned
	WWSJOB_ASSERT( (size & 0xFFFC000F) == 0 );

	QuadWord* pQwordMem = (QuadWord*)pMem;

	// set the memory (size is non-zero!)
	U32 pMemEnd = (U32)pQwordMem + size;
	do
	{
		pQwordMem->m_u64[0] = data;
		pQwordMem->m_u64[1] = data;
		pQwordMem++;
	}while( (U32)pQwordMem != pMemEnd );
}
#endif


/**	\brief	Allocate a dmaTagId from those available

    If none available then wait until it is
	Called directly or indirectly by WwsJob_JobApi only

	\return	dmaTagId = 1:31 (0 used by wwsjob manager)
**/
#if WWS_JOB_USE_C_VERSION!=0
U32	WwsJob_UseDmaTagId( void )
{
	// we have chosen to not allow dmaTagId 0 (bit 31) to be in the available pool of dmaTagId's
	WWSJOB_VERBOSE_ASSERT( (g_WwsJob_usedDmaTagMask & 1) );

	// while no dmaTagId's are available, wait until a dmaTagId frees up
	while ( g_WwsJob_usedDmaTagMask == 0xFFFFFFFF )
	{
		WWSJOB_ASSERT( g_WwsJob_storeJobNum >= 0 );
		//If the store job has completed, and yet all the dma tags are still
		//already in use, then there are none left for this function to allocate

		// wait until a dmaTagId frees up
		__builtin_spu_ienable();
		__builtin_spu_idisable();

		WwsJob_TryChangeStoreToFreeJob();
	}

	U32 dmaTagId/*1:31*/ = 31 - spu_extract( spu_cntlz( spu_splats(~g_WwsJob_usedDmaTagMask) ), 0 );
	g_WwsJob_usedDmaTagMask |= (1 << dmaTagId);

	STORE_IMPORTANT_AUDIT( AuditId::kWwsJob_UseDmaTagId, dmaTagId,
			g_WwsJob_usedDmaTagMask );

	return dmaTagId;
}
#endif


//} // namespace Job
//} // namespace Wws



//====================================================================================================================
// main loop of jobManager code
//====================================================================================================================

/**	\brief	main loop of wwsjob manager

	Do initialization, and then main loop of job manager.
	Each loop is for a new job.  Note there can be
	simultaneous processing for a load, run, and store job.
	\param	The spursContext param is no longer needed
	\param	workLoadData = [main mem addr of audit buffer array base | main mem addr of spurs workload]
**/
#if WWS_JOB_USE_C_VERSION!=0
I32 WwsJob_Main( uintptr_t spursContext, uint64_t workLoadData )
{
	// if you want to debug the C job manager, then enable this breakpoint
	//WWSJOB_BREAKPOINT();

	// This is the Wws jobManager
	// We have a workload to do

	// Within job manager, interrupts are always disabled unless otherwise noted
	WWSJOB_VERBOSE_ASSERT( !AreInterruptsEnabled() );

	WWSJOB_UNUSED( spursContext );

	// init everything
	WwsJob_Initialize( workLoadData );

	while(1)
	{
		// There is no runJob
		STORE_TIMING_AUDIT( AuditId::kWwsJob_WwsMain_beginLoop, g_WwsJob_loadJobState );

		// If we don't have a load job
		if( !g_WwsJob_loadJobState )
		{	// we don't have a loadJob yet
			// Note: flow is infrequent here, since a job normally starts loading while another job is running

	  	  #if !ENABLE_JOB_PIPELINE
			// wait till there is no storeJob
			while( g_WwsJob_storeJobNum >= 0 )
			{
				// give interrupts a chance to execute, to change store job to free job
				__builtin_spu_ienable();
				__builtin_spu_idisable();

				WwsJob_TryChangeStoreToFreeJob();
			}
		  #endif // #if !ENABLE_JOB_PIPELINE

			STORE_VERBOSE_AUDIT( AuditId::kWwsJob_WwsMain_noLoadJob, g_WwsJob_nextLoadJobNum );

			// If Spurs Kernel wants our PM to get the next job
			if( g_WwsJob_nextLoadJobNum >= 0 )
			{	// Spurs Kernel wants our PM to get the next job

				// Try to change a free job to a load job.
				WwsJob_TryChangeFreeToLoadJob();
			}

			// if we still don't have a loadJob
			if( !g_WwsJob_loadJobState )
			{	// we still don't have a loadJob
				// (none available or Spurs Kernel wants a new PM)

				// It is time to return to Spurs Kernel

				STORE_VERBOSE_AUDIT( AuditId::kWwsJob_WwsMain_stillNoLoadJob );

				// wait until all store jobs are done
				// (note there are no load or run jobs)
				while(1)
				{
					// give interrupts a chance to execute, to change store job to free job
					if ( g_WwsJob_storeJobNum >= 0 )
					{
						WwsJob_TryChangeStoreToFreeJob();
					}
					__builtin_spu_ienable();
					__builtin_spu_idisable();

					//Note that g_WwsJob_storeJobNum may be modified by WwsJob_TryChangeStoreToFreeJob,
					//which is why this isn't an if-else

					// If there is no storeJob
					if( g_WwsJob_storeJobNum < 0 )
					{	// there is no storeJob

						STORE_VERBOSE_AUDIT( AuditId::kWwsJob_WwsMain_noLoadOrStoreJob );

						// Note there are no load or run jobs

						// Try to clear shared buffer states & pages.
						// Note: if everything could not be cleared,
						// it will start an interrupt which will finish the clearing
						(void)WwsJob_TryDumpAllStoreShareBufs();

						STORE_VERBOSE_AUDIT( AuditId::kWwsJob_WwsMain_waitForStoreJobShareBufsDma );

						STORE_TIMING_AUDIT( AuditId::kWwsJob_WwsMain_waitForStoreDma_begin );

						// wait while the lastStoreJob shareable dma writes are done
						do
						{
							__builtin_spu_ienable();
							__builtin_spu_idisable();
						}while( !WwsJob_IsDmaTagMaskDone( 1 << DmaTagId::kStoreJob_writeAllShareableBuffers ) );

						STORE_TIMING_AUDIT( AuditId::kWwsJob_WwsMain_waitForStoreDma_end );

//CPC - I think that this bit is logically redundant
						// If writeAllShareableBuffers dma finished (above) while interrupts were disabled, then it
						// did not have a chance to call WwsJob_TryDumpAllStoreShareBufs
						// Enabling interrupts will ensure that code gets called before WwsJob_ReturnToSpursKernel
						__builtin_spu_ienable();
						__builtin_spu_idisable();

						// Return to Spurs Kernel.  This does not return
						WwsJob_ReturnToSpursKernel();

						// there is no code flow here
					}
				}
				// there is no code flow here
			}
			// we have a loadJob
		}
		// we have a loadJob

		STORE_TIMING_AUDIT( AuditId::kWwsJob_WwsMain_waitForLoadDma_begin );

		// delay while !(the load commands have finished executing and their dma reads are done)
		do
		{
			if ( g_WwsJob_storeJobNum >= 0 )
			{
				WwsJob_TryChangeStoreToFreeJob();
			}
			if( g_WwsJob_loadJobState == WwsJob_LoadJobState::kExecuteCommands )
			{
				WwsJob_TryExecuteLoadCmds();
			}

			// give interrupts a chance to execute
			__builtin_spu_ienable();
			__builtin_spu_idisable();
		}
		while( !( ( g_WwsJob_loadJobState == WwsJob_LoadJobState::kCommandsExecuted )  &&
				WwsJob_IsDmaTagMaskDone( 1 << DmaTagId::kLoadJob_readBuffers ) ) );

		STORE_TIMING_AUDIT( AuditId::kWwsJob_WwsMain_waitForLoadDma_end );

		// Start running the loaded job
		WwsJob_ChangeLoadToRunJob();
				// ...	The job is running.
				// ...	The job can get parameters, do more buffer commands, and get buffers and dmaTagIds
				// ...	The job executes the WwsJob_ApiCommand::kLoadNextJob
				//			Spurs Kernel is checked to see what PM and workload to do next
				//			If this PM is done next
				//				It tries to take a job and start it loading
				// ...	The job continues running while the next job loads
				// ...	The job exits

		// the runJob has exited
		// there may or may not be a loadJob

		STORE_TIMING_AUDIT( AuditId::kWwsJob_WwsMain_waitForStoreDma_begin );

		// delay while the storeJob exists, or any kStoreJob_writeAllShareableBuffers dma is ongoing
		// Note: the 2nd test (dma ongoing) must be done, otherwise if you change the runJob to a storeJob
		//		then you change the g_WwsJob_lastStoreJobNum, and then when the kStoreJob_writeAllShareableBuffers interrupt occurs
		//		then it has an incorrect g_WwsJob_lastStoreJobNum value.
		do
		{
			// give interrupts a chance to execute, to change store job to free job
			if ( g_WwsJob_storeJobNum >= 0 )
			{
				WwsJob_TryChangeStoreToFreeJob();
			}
			__builtin_spu_ienable();
			__builtin_spu_idisable();
		}while( g_WwsJob_storeJobNum >= 0  ||  !WwsJob_IsDmaTagMaskDone( 1 << DmaTagId::kStoreJob_writeAllShareableBuffers ) );

		STORE_TIMING_AUDIT( AuditId::kWwsJob_WwsMain_waitForStoreDma_end );

		// there is no storeJob

		// Change runJob to storeJob (or maybe freeJob)
		WwsJob_ChangeRunToStoreJob();

		// there is no runJob
	}
}
#endif
