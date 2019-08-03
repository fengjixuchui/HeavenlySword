/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		This sample shows how to use audits in the job manager
**/
//--------------------------------------------------------------------------------------------------

#include <string.h>
#include <stdio.h>

#include <sys/spu_initialize.h>
#include <sys/ppu_thread.h>

#include <cell/sysmodule.h>
#include <cell/spurs/types.h>
#include <cell/spurs/control.h>

#include <jobapi/joblist.h>
#include <jobapi/spumodule.h>
#include <jobapi/commandlistbuilder.h>
#include <jobapi/auditmanager.h>
#include <jobapi/eventhandler.h>

#include "auditsjob/auditsjob.h"

//--------------------------------------------------------------------------------------------------

#define AUDIT_DATA( kEnumName, kString )        kString , 
const char* const g_auditsJobText[] =
{
	#include "auditsjob/auditsjobdata.inc"
	"kAuditsJob_numAudits",
};
#undef AUDIT_DATA 

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
	int kNumSpus			= 5;


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
	extern char _binary_auditsjob_file_start[];
	extern char _binary_auditsjob_file_size;
	SpuModuleHandle spuAuditsJobHandle ( _binary_auditsjob_file_start, (U32)&_binary_auditsjob_file_size, "auditsjob.spu.mod" );


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//Init our audit buffers
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	const U32 kAuditBufferSize = 8*1024*1024;	//Shared between 6 SPUs
	static U8 g_buffersForAudits[kAuditBufferSize] WWSJOB_ALIGNED(128);
	memset( g_buffersForAudits, 0, sizeof(g_buffersForAudits) );

	AuditManager auditManager;
	auditManager.InitAuditMemory( g_buffersForAudits, kAuditBufferSize, kNumSpus, AuditManager::kDoubleBuffered );

	// disable jobmanager audits
	auditManager.SetJobManagerAuditsEnabled( false );

	// enable job audits
	auditManager.SetJobAuditsEnabled( true );

	//The value returned is the base of the audit id range that all the audit enums must be offset by
	U16 auditIdBase = auditManager.AllocIdRangeAndRegisterAuditData( AuditId::kAuditsJob_numAudits, "AuditsJob_", g_auditsJobText );

	//Send all audits to audit buffer 0
	auditManager.SetAuditOutputBufferNum( 0 );


	//Install the event handler to handle JobPrintf
	EventHandler eventHandler( &g_spurs, ppuThreadPrio, &auditManager );


	//Create a job list
	static U8 s_bufferForJobList[ 1024 ] WWSJOB_ALIGNED( 128 );
	MultiThreadSafeJobList jobList( s_bufferForJobList, sizeof(s_bufferForJobList), "AuditsJobList" );


	U32 maxContention			= kNumSpus;
	const uint8_t standardWorkPrios[8]		= { 8, 8, 0/*disable*/, 0/*disable*/, 8, 0, 0, 0 };

	//Tell SPURS about the job list
	jobList.AttachToSpurs( &g_spurs, standardWorkPrios, maxContention, &auditManager );


	//CommandListBuilder helper class for building jobs
	static U32 s_commandListBuilderDataBuffer[ 2 * 1024 ] WWSJOB_ALIGNED( 128 );
	CommandListBuilder commandListBuilder( s_commandListBuilderDataBuffer, sizeof( s_commandListBuilderDataBuffer ) );


	//Create a job.
	//Memory map is just one buffer for code
	const U32 kCodeBufferSet			= 0;					//bufferset 0
	const U32 kNumCodeBuffers			= 1;					//contains 1 buffer
	const U32 kCodeBufferSizeInPages	= spuAuditsJobHandle.GetRequiredBufferSizeInPages();	//big enough for the code and bss
	const U32 kCodeBufferSetBasePageNum	= LsMemoryLimits::kJobAreaBasePageNum;	//The first buffer of this bufferset starts at this address

	const U32 kLogicalBuffer0 = 0;

	//Create the load commands for the job
	commandListBuilder.InitializeJob();
	commandListBuilder.ReserveBufferSet( kCodeBufferSet, kNumCodeBuffers, kCodeBufferSetBasePageNum, kCodeBufferSizeInPages );	
	commandListBuilder.UseInputBuffer( kCodeBufferSet, kLogicalBuffer0, spuAuditsJobHandle, WwsJob_Command::kReadOnlyCached );
	commandListBuilder.UnreserveBufferSets( 1 << kCodeBufferSet );	
	commandListBuilder.RunJob( kCodeBufferSet, kLogicalBuffer0 );
	commandListBuilder.AddU32Param( 0, auditIdBase );
	JobHeader job = commandListBuilder.FinalizeJob();


	printf( "\nFirst frame will just print job audits for 1 job\n\n" );
	
	//Add the job to our jobList
	jobList.AddJob( job );

	//Tell SPURS that there's work waiting to be processed.
	jobList.SetReadyCount( MultiThreadSafeJobList::kRequestOneSpu );

	//Stall until all jobs on the joblist are done
	jobList.WaitForJobListEnd();

	//Print out the audit buffers and empty them.
	//We are printing from audit buffer 0
	auditManager.ProcessAuditBuffersForAllSpus( 0, AuditManager::PrintAudit, AuditManager::kPrintHeaders, NULL );
	auditManager.EmptyAuditBuffersForAllSpus( 0 );




	printf( "\nSecond frame will just print jobManager & job audits for many jobs on spus 0, 1 & 4\n\n" );

	//For the second frame, we'll send all audits to audit buffer 1
	auditManager.SetAuditOutputBufferNum( 1 );

	// enable jobmanager audits
	// prints out only what is needed to know whether to assign the time to:
	//		SpursKernel, wwsjob manager, or the job
	// NOTE: in the makeopts file, if you enable the line:
	//		SPU_DEFINES	+= -DENABLE_IMPORTANT_AUDITS
	//	then the job manager will also print the "important" audits, which gives more information
	//	on what is going on.
	auditManager.SetJobManagerAuditsEnabled( true );

	// enable job audits
	auditManager.SetJobAuditsEnabled( true );

	// reset the list
	jobList.ResetList();

	// add the job a number of times
	for( U32 i = 0 ; i < 6 ; i++ )
	{
		//Add the job to our jobList
		jobList.AddJob( job );
	}

	//Tell SPURS that there's work waiting to be processed.
	jobList.SetReadyCount( MultiThreadSafeJobList::kRequestAllSpus );	//There's enough work that we want all SPUs to come and work on it (if appropriate)

	//Stall until all jobs on the joblist are done
	jobList.WaitForJobListEnd();

	//Print out the audit buffers and empty them.
	//For the second frame we are now printing from audit buffer 0
	auditManager.ProcessAuditBuffersForAllSpus( 1, AuditManager::PrintAudit, AuditManager::kPrintHeaders, NULL );
	auditManager.EmptyAuditBuffersForAllSpus( 1 );



	//Tidy up everything and shut down cleanly.
	jobList.Shutdown();

	eventHandler.RemoveEventHandler( &g_spurs );

	ret = cellSpursFinalize( &g_spurs );
	WWSJOB_ASSERT( CELL_OK == ret );

	printf( "End of audits sample\n" );

	return 0;
}

//--------------------------------------------------------------------------------------------------
