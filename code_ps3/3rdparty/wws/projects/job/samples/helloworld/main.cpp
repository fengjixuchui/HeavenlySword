/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		This sample shows a basic "Hello World" job being run by the job manager
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

#include "helloworldjob/helloworldjob.h"


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
	extern char _binary_helloworldjob_file_start[];
	extern char _binary_helloworldjob_file_size;
	SpuModuleHandle spuHelloWorldModuleHandle ( _binary_helloworldjob_file_start, (U32)&_binary_helloworldjob_file_size, "helloworldjob.spu.mod" );


	//Create a job list
	static U8 s_bufferForJobList[ 1024 ] WWSJOB_ALIGNED( 128 );
	MultiThreadSafeJobList jobList( s_bufferForJobList, sizeof(s_bufferForJobList), "HelloWorldList" );


	U32 maxContention			= kNumSpus;
	const uint8_t standardWorkPrios[8]		= { 8, 8, 8, 8, 8, 8, 0, 0 };

	//Tell SPURS about the job list
	jobList.AttachToSpurs( &g_spurs, standardWorkPrios, maxContention );


	//CommandListBuilder helper class for building jobs
	static U32 s_commandListBuilderDataBuffer[ 2 * 1024 ] WWSJOB_ALIGNED( 128 );
	CommandListBuilder commandListBuilder( s_commandListBuilderDataBuffer, sizeof( s_commandListBuilderDataBuffer ) );


	//Create a job.
	//Memory map is just one buffer for the code
	const U32 kCodeBufferSet			= 0;					//bufferset 0
	const U32 kNumCodeBuffers			= 1;					//contains 1 buffer
	const U32 kCodeBufferSizeInPages	= spuHelloWorldModuleHandle.GetRequiredBufferSizeInPages();	//big enough for the code and bss
	const U32 kCodeBufferSetBasePageNum	= LsMemoryLimits::kJobAreaBasePageNum;			//The first buffer of this bufferset starts at this address

	const U32 kLogicalBuffer0 = 0;

	//Create the load commands for the job
	commandListBuilder.InitializeJob();
	commandListBuilder.ReserveBufferSet( kCodeBufferSet, kNumCodeBuffers, kCodeBufferSetBasePageNum, kCodeBufferSizeInPages );	
	commandListBuilder.UseInputBuffer( kCodeBufferSet, kLogicalBuffer0, spuHelloWorldModuleHandle, WwsJob_Command::kReadOnlyCached );
	commandListBuilder.RunJob( kCodeBufferSet, kLogicalBuffer0 );
	JobHeader job = commandListBuilder.FinalizeJob();


	//Add the job to our jobList
	jobList.AddJob( job );


	//Tell SPURS that there's work waiting to be processed.
	jobList.SetReadyCount( MultiThreadSafeJobList::kRequestOneSpu );


	//Stall until all jobs on the joblist are done
	jobList.WaitForJobListEnd();
	//printf( "jobList work is now finished\n" );


	//Tidy up everything and shut down cleanly.
	jobList.Shutdown();

	eventHandler.RemoveEventHandler( &g_spurs );

	ret = cellSpursFinalize( &g_spurs );
	WWSJOB_ASSERT( CELL_OK == ret );

	printf( "End of Hello World sample\n" );

	return 0;
}

//--------------------------------------------------------------------------------------------------
