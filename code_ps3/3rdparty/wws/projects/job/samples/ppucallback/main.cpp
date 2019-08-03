/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		This sample shows a PPU callback being triggered from an SPU job
**/
//--------------------------------------------------------------------------------------------------

#include <string.h>
#include <stdio.h>

#include <cell/sysmodule.h>
#include <sys/spu_initialize.h>
#include <sys/ppu_thread.h>

#include <cell/spurs/types.h>
#include <cell/spurs/control.h>

#include <jobapi/joblist.h>
#include <jobapi/spumodule.h>
#include <jobapi/commandlistbuilder.h>
#include <jobapi/auditmanager.h>
#include <jobapi/eventhandler.h>

#include "ppucallbackjob/ppucallbackjob.h"

//--------------------------------------------------------------------------------------------------

void MyEventCallbackFunction( U32 data1, U32 data2 )	//Note: data1 can only ever 24bits max
{
	printf( "Event received: data = 0x%06X, 0x%08X\n", data1, data2 );
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

	//Install the event handler to handle JobPrintf
	//Calling JobApiSendEvent on the SPU will cause MyEventCallbackFunction to be triggered
	EventHandler eventHandler( &g_spurs, ppuThreadPrio, NULL, MyEventCallbackFunction );


	//Get job module handle
	extern char _binary_ppucallbackjob_file_start[];
	extern char _binary_ppucallbackjob_file_size;
	SpuModuleHandle spuPpuCallbackModuleHandle ( _binary_ppucallbackjob_file_start, (U32)&_binary_ppucallbackjob_file_size, "ppucallbackjob.spu.mod" );


	//Create a job list
	static U8 s_bufferForJobList[ 1024 ] WWSJOB_ALIGNED( 128 );
	MultiThreadSafeJobList jobList( s_bufferForJobList, sizeof(s_bufferForJobList), "PpuCallbackList" );


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
	const U32 kCodeBufferSizeInPages	= spuPpuCallbackModuleHandle.GetRequiredBufferSizeInPages();	//big enough for the code and bss
	const U32 kCodeBufferSetBasePageNum	= LsMemoryLimits::kJobAreaBasePageNum;			//The first buffer of this bufferset starts at this address

	const U32 kLogicalBuffer0 = 0;

	PpuCallbackModuleParams params;
	params.m_port = eventHandler.GetPortNum();	//Pass in the port number so that the SPU knows where to send its event

	//Create the load commands for the job
	commandListBuilder.InitializeJob();
	commandListBuilder.ReserveBufferSet( kCodeBufferSet, kNumCodeBuffers, kCodeBufferSetBasePageNum, kCodeBufferSizeInPages );	
	commandListBuilder.UseInputBuffer( kCodeBufferSet, kLogicalBuffer0, spuPpuCallbackModuleHandle, WwsJob_Command::kReadOnlyCached );
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


	//The event might potentially not be processed until some time after the job list has been finished,
	//so stall until all queued events have been processed
	eventHandler.StallForQueuedEvents();


	//Tidy up everything and shut down cleanly.
	jobList.Shutdown();

	eventHandler.RemoveEventHandler( &g_spurs );

	ret = cellSpursFinalize( &g_spurs );
	WWSJOB_ASSERT( CELL_OK == ret );

	printf( "End of ppucallback sample\n" );

	return 0;
}

//--------------------------------------------------------------------------------------------------
