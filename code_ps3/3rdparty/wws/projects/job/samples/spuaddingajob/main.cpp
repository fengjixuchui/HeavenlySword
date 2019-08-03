/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		This sample shows one SPU job creating and executing another SPU job
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
#include <jobapi/eventhandler.h>

#include "jobadderjob/jobadderjob.h"

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

	//Install the event handler to handle JobPrintf
	EventHandler eventHandler( &g_spurs, ppuThreadPrio );


	//Get job module handle
	extern char _binary_jobadderjob_file_start[];
	extern char _binary_jobadderjob_file_size;
	SpuModuleHandle spuJobAdderModuleHandle ( _binary_jobadderjob_file_start, (U32)&_binary_jobadderjob_file_size, "jobadderjob.spu.mod" );
	extern char _binary_secondaryjob_file_start[];
	extern char _binary_secondaryjob_file_size;
	SpuModuleHandle secondaryModuleHandle ( _binary_secondaryjob_file_start, (U32)&_binary_secondaryjob_file_size, "secondaryjob.spu.mod" );


	//Create a job list
	static U8 s_bufferForJobList[ 1024 ] WWSJOB_ALIGNED( 128 );
	MultiThreadSafeJobList jobList( s_bufferForJobList, sizeof(s_bufferForJobList), "SpuAddingJob" );


	U32 maxContention			= kNumSpus;
	const uint8_t standardWorkPrios[8]		= { 8, 8, 8, 8, 8, 8, 0, 0 };

	//Tell SPURS about the job list
	jobList.AttachToSpurs( &g_spurs, standardWorkPrios, maxContention );


	//CommandListBuilder helper class for building jobs
	static U32 s_commandListBuilderDataBuffer[ 2 * 1024 ] WWSJOB_ALIGNED( 128 );
	CommandListBuilder commandListBuilder( s_commandListBuilderDataBuffer, sizeof( s_commandListBuilderDataBuffer ) );


	//Create a job.
	//Memory map is just one buffer for code
	const U32 kCodeBufferSet			= 0;					//bufferset 0
	const U32 kNumCodeBuffers			= 1;					//contains 1 buffer
	const U32 kCodeBufferSizeInPages	= spuJobAdderModuleHandle.GetRequiredBufferSizeInPages();	//big enough for the code and bss
	const U32 kCodeBufferSetBasePageNum	= LsMemoryLimits::kJobAreaBasePageNum;			//The first buffer of this bufferset starts at this address

	const U32 kLogicalBuffer0 = 0;

	static WwsJob_Command s_newJobCommandListBuffer[ 32 ] WWSJOB_ALIGNED( 128 );

	JobAdderJobModuleParams params;
	params.m_eaNewJobBuffer						= (U32) s_newJobCommandListBuffer;
	params.m_newJobBufferSize					= sizeof( s_newJobCommandListBuffer );
	params.m_eaSpuModule						= (U32) secondaryModuleHandle.GetAddress();
	params.m_spuModuleFileSize					= secondaryModuleHandle.GetFileSize();
	params.m_spuModuleRequiredBufferSizeInPages	= secondaryModuleHandle.GetRequiredBufferSizeInPages();
	params.m_eaJobList							= (U32) jobList.GetWQAddr();
	params.m_workloadId							= jobList.GetWorkloadId();

	//Create the load commands for the job
	commandListBuilder.InitializeJob();
	commandListBuilder.ReserveBufferSet( kCodeBufferSet, kNumCodeBuffers, kCodeBufferSetBasePageNum, kCodeBufferSizeInPages );
	commandListBuilder.UseInputBuffer( kCodeBufferSet, kLogicalBuffer0, spuJobAdderModuleHandle, WwsJob_Command::kReadOnlyCached );
	commandListBuilder.UnreserveBufferSets( 1 << kCodeBufferSet );	
	commandListBuilder.RunJob( kCodeBufferSet, kLogicalBuffer0 );
	commandListBuilder.AddParams( &params, sizeof(params) );
	JobHeader job = commandListBuilder.FinalizeJob();


	//Add the job to our jobList
	jobList.AddJob( job );


	//Tell SPURS that there's working waiting to be processed.
	jobList.SetReadyCount( MultiThreadSafeJobList::kRequestOneSpu );


	//Stall until all jobs on the joblist are done
	jobList.WaitForJobListEnd();
	//printf( "jobList work is now finished\n" );


	//Tidy up everything and shut down cleanly.
	jobList.Shutdown();

	eventHandler.RemoveEventHandler( &g_spurs );

	ret = cellSpursFinalize( &g_spurs );
	WWSJOB_ASSERT( CELL_OK == ret );

	printf( "End of SPU Adding A Job sample\n" );

	return 0;
}

//--------------------------------------------------------------------------------------------------
