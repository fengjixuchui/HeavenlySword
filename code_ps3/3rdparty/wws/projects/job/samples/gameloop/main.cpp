/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		This sample shows job manager usage in a game loop
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

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//This is the header file that is shared with the SPU for defining
//our input parameters structure
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "simplemodule/simplemodule.h"


//--------------------------------------------------------------------------------------------------

void CheckResults( const char* outputName, U32 numElts, const U8* pOutput, U8 expectedValue );
void VSync( void );

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

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//This is the Spurs object, used for interacting with the Spurs kernel.
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

CellSpurs g_spurs;			//Spurs object

//--------------------------------------------------------------------------------------------------

int main()
{
	int kNumSpus					= 5;

	const U32 kNumExtraJobsPerFrame	= 16;

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//Initialize the SPUs.
	//The Spurs Kernel runs on threaded SPUs.
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	int ret = sys_spu_initialize( kNumSpus, 0 );
	WWSJOB_ASSERT( CELL_OK == ret );


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//Initialized the SPURS object, and install a printf handler for it.
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	//Initialize Spurs
	int ppuThreadPrio	= GetPpuThreadPriority() - 1;	//Higher priority than main thread
	int spuThgrpPrio	= 128;
	int isExit			= 0;	// Must be false
	ret = cellSpursInitialize( &g_spurs, kNumSpus, spuThgrpPrio, ppuThreadPrio, isExit );
	WWSJOB_ASSERT( CELL_OK == ret );
	printf( "Spurs initialized (spurs=0x%p)\n", &g_spurs );


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//Install an event handler to handle JobPrintf.
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	EventHandler eventHandler( &g_spurs, ppuThreadPrio );



	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//Get job module handle.  The module has been embedded into the ppu elf
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	extern char _binary_simplemodule_file_start[];
	extern char _binary_simplemodule_file_size;
	SpuModuleHandle spuSimpleModuleHandle ( _binary_simplemodule_file_start, (U32)&_binary_simplemodule_file_size, "simplemodule.spu.mod" );



	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//We're not using audits in this sample so don't need to do anything
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////



	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//Now we need to create a JobList.
	//s_bufferForJobList is the memory buffer our job list will be built into.
	//The job list starts with a JobListHeader followed by an array of
	//JobHeaders.
	//The jobList object is just a helper class to simplify writing into this
	//buffer.
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	static U8 s_bufferForJobList[ 128 + kNumExtraJobsPerFrame*8 ] WWSJOB_ALIGNED( 128 );
	MultiThreadSafeJobList jobList( s_bufferForJobList, sizeof(s_bufferForJobList), "GameLoopJobList" );



	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//Next we need to tell Spurs about our jobList.
	//To Spurs this "workload" is just an opaque type.
	//When this workload has a non-zero ready count, Spurs will attempt to
	//schedule SPUs to run our Job Manager and process the jobs
	//
	//The jobs on this jobList are valid for sharing across multiple SPUs,
	//hence we set the maxContention to be kNumSpus.
	//We also set up priority values for this workload.  These can be
	//different values for different SPUs if you wish.  The highest priority
	//value is 1.
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	U32 maxContention			= kNumSpus;
	const uint8_t standardWorkPrios[8]		= { 8, 8, 8, 8, 8, 8, 0, 0 };

	//Tell SPURS about the job list
	jobList.AttachToSpurs( &g_spurs, standardWorkPrios, maxContention );



	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//s_commandListBuilderDataBuffer is the buffer into which our jobs will
	//be being built
	//We create a commandListBuilder helper object which will just help us
	//out with creating our jobs
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	static U32 s_commandListBuilderDataBuffer[ 2 * 1024 ] WWSJOB_ALIGNED( 128 );
	CommandListBuilder commandListBuilder( s_commandListBuilderDataBuffer, sizeof( s_commandListBuilderDataBuffer ) );



	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//Now, let's create our job.
	//On this occasion we're creating this job at initialisation time, but we
	//could equally have created this job at run time.
	//
	//The memory map of our job is as follows:
	//The first "LsMemoryLimits::kJobAreaBasePageNum" pages are reserved for
	//the JobManager.  Then, bufferset 0 is one 10K buffer which this job will
	//use for its code.  Following next to that, bufferset 1 contains two 8K
	//buffers.  This instance of the job will only use one of them, but
	//specifying space for two of them allows for pipelining of consecutive
	//jobs.
	//
	//The first thing the job commands do is reserve these two buffer sets.
	//Then, we use a buffer (in fact, the only buffer) in bufferset 0 to
	//upload the code into.  Since the code will not be modified in LS, we
	//mark that this input is shareable.  This means that if the next job uses
	//this same code, it can reuse it in Local Storage, rather than having
	//to reload out of main memory again.
	//Next, we use one of the input buffers out of bufferset 1 to upload the
	//input data into.  This command tells the job manager to pull the data
	//into LS.  Since we will be modifying this buffer in LS we do not mark
	//this one as shareable.
	//Now, we can add a command to start the job running.
	//The data that follows the RunJob command is made available to the job
	//to read as input parameters.  It's up to the job to choose the format
	//of this data.  In this case, it is in defined by the
	//"SimpleModuleParams" structure which is in header file shared between
	//the SPU job code and this file (ie. the JobManager doesn't know
	//anything about it).
	//When we close the job, a JobHeader is returned by value.  This is the
	//64-bit JobHeader structure which will later be written into the joblist.
	//
	//In this example, what the SPU job code will do is receive the input
	//buffer which we have initialised to 0x11 multiply it by the multiplier
	//value that we pass in, which in this case is 5.  The job will then
	//initiate a dma of this code back from LS to main memory.
	//One the job has completed, the PPU can then read this memory and in this
	//case we assert that it has correctly calculated the value 0x55 to fill
	//that output array with.	
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	const U32 kBufferSizeInPages	= 8;						// This sample will run with 8K buffers
	const U32 kBufferSize			= kBufferSizeInPages*1024;	//


	static U8 s_simpleModuleInputBuffer[kBufferSize] WWSJOB_ALIGNED(128);
	static U8 s_simplemoduleOutputBuffer1[kBufferSize] WWSJOB_ALIGNED(128);
	static U8 s_simplemoduleOutputBuffer2[kBufferSize] WWSJOB_ALIGNED(128);
	memset( s_simpleModuleInputBuffer, 0x11, kBufferSize );

	//one code buffer
	const U32 kCodeBufferSet			= 0;					//bufferset 0
	const U32 kNumCodeBuffers			= 1;					//contains 1 buffer
	const U32 kCodeBufferSizeInPages	= spuSimpleModuleHandle.GetRequiredBufferSizeInPages();	//big enough for the code and bss
	const U32 kCodeBufferSetBasePageNum	= LsMemoryLimits::kJobAreaBasePageNum;			//The first buffer of this bufferset starts at this address

	//followed by 2 4K data buffers
	const U32 kDataBufferSet			= 1;					//bufferset 1
	const U32 kNumDataBuffers			= 2;					//contains 2 buffers
	const U32 kDataBufferSizeInPages	= kBufferSizeInPages;	//and these buffers are 4K each
	const U32 kDataBufferSetBasePageNum	= kCodeBufferSetBasePageNum + kNumCodeBuffers*kCodeBufferSizeInPages;

	//The input parameters to the job (in the memory layout they'll be expected in)
	SimpleModuleParams moduleParams;
	moduleParams.m_eaOutputAddr		= (U32) s_simplemoduleOutputBuffer1;
	moduleParams.m_multiplier		= 5;
	moduleParams.m_outputBufferSize	= kBufferSize;

	const U32 kLogicalBuffer0 = 0;

	//Create the load commands for the job
	commandListBuilder.InitializeJob();
	commandListBuilder.ReserveBufferSet( kCodeBufferSet, kNumCodeBuffers, kCodeBufferSetBasePageNum, kCodeBufferSizeInPages );	
	commandListBuilder.ReserveBufferSet( kDataBufferSet, kNumDataBuffers, kDataBufferSetBasePageNum, kDataBufferSizeInPages );	
	commandListBuilder.UseInputBuffer( kCodeBufferSet, kLogicalBuffer0, spuSimpleModuleHandle, WwsJob_Command::kReadOnlyCached );
	commandListBuilder.UseInputBuffer( kDataBufferSet, kLogicalBuffer0, s_simpleModuleInputBuffer, kBufferSize, WwsJob_Command::kNonCached );
	commandListBuilder.UnreserveBufferSets( (1 << kCodeBufferSet) | (1 << kDataBufferSet) );
	commandListBuilder.RunJob( kCodeBufferSet, kLogicalBuffer0 );
	commandListBuilder.AddParams( &moduleParams, sizeof(moduleParams) );
	JobHeader job1 = commandListBuilder.FinalizeJob();


	//Create another job
	moduleParams.m_eaOutputAddr		= (U32) s_simplemoduleOutputBuffer2;
	moduleParams.m_multiplier		= 3;
	moduleParams.m_outputBufferSize	= kBufferSize;

	commandListBuilder.InitializeJob();
	commandListBuilder.ReserveBufferSet( kCodeBufferSet, kNumCodeBuffers, kCodeBufferSetBasePageNum, kCodeBufferSizeInPages );	
	commandListBuilder.ReserveBufferSet( kDataBufferSet, kNumDataBuffers, kDataBufferSetBasePageNum, kDataBufferSizeInPages );	
	commandListBuilder.UseInputBuffer( kCodeBufferSet, kLogicalBuffer0, spuSimpleModuleHandle, WwsJob_Command::kReadOnlyCached );
	commandListBuilder.UseInputBuffer( kDataBufferSet, kLogicalBuffer0, s_simpleModuleInputBuffer, kBufferSize, WwsJob_Command::kNonCached );
	commandListBuilder.UnreserveBufferSets( (1 << kCodeBufferSet) | (1 << kDataBufferSet) );
	commandListBuilder.RunJob( kCodeBufferSet, kLogicalBuffer0 );
	commandListBuilder.AddParams( &moduleParams, sizeof(moduleParams) );
	JobHeader job2 = commandListBuilder.FinalizeJob();


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//This is the start of the main game loop
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	U32 frameNo = 0;

	// this sample loops for only so many frames							  
	while ( frameNo <= 0x4000 )
	{



		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//At the start of the frame we reset the joblist.
		//This clears out all previous jobs so that we can start adding new jobs
		//to this list.
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

		jobList.ResetList();



		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//We also reset the output buffer to 0xCC so that later on this frame,
		//when the job has run, we can assert that the output value has
		//correctly been filled into this array.
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

		memset( s_simplemoduleOutputBuffer1, 0xCC, kBufferSize );
		memset( s_simplemoduleOutputBuffer2, 0xCC, kBufferSize );



		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//We created our job earlier.  Now we can add it to our job list.
		//
		//This function also returns a marker which can later query to check
		//whether this job has completed yet.
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

		JobListMarker markerAfterJob = jobList.AddJob( job1 );



		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//And let's add a load more jobs.  We're actually just adding the same
		//job repeatedly here as an example, so in fact it'll keep over-writing
		//the same output buffer
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

		for ( U32 i = 0; i < kNumExtraJobsPerFrame; ++i )
		{
			jobList.AddJob( job2 );
		}



		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//Once we've add this job, we need to notify Spurs that our workload has
		//work on it and that SPUs should be scheduled to process it.  So, we set
		//our ready count.
		//
		//Since we are only adding one job here, we'll only set the ready count to
		//one.  If we were adding more jobs, we'd probably set it to kNumSpus.
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

		jobList.SetReadyCount( MultiThreadSafeJobList::kRequestAllSpus );



		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//Now stall until our jobs are done.
		//Then assert that all the results were what we expected them to be.
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

		markerAfterJob.StallForJobMarker();
		CheckResults( "s_simplemoduleOutputBuffer1", kBufferSize, s_simplemoduleOutputBuffer1, 0x55 );



		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//Given that really there's likely to be more than just our one job on the
		//list, wait until all jobs on the list are done before ending the frame.
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

		jobList.WaitForJobListEnd();
		//printf( "jobList work is now finished\n" );

		CheckResults( "s_simplemoduleOutputBuffer2", kBufferSize, s_simplemoduleOutputBuffer2, 0x33 );


		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//Then Vsync and increment our frame counter
		//
		//Actually, the Vsync function in this sample doesn't do anything since
		//we haven't actually initialised the graphics system.
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

		VSync();

		if ( ( frameNo & 0x3FF ) == 0 )
		{
			printf( "Frame = 0x%X\n", frameNo );
		}
		++frameNo;
	}




	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//Tidy up everything and shut down cleanly.
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	//Remove the workloads and shut down
	jobList.Shutdown();
	printf( "Removed Workloads OK\n" );

	eventHandler.RemoveEventHandler( &g_spurs );

	ret = cellSpursFinalize( &g_spurs );
	WWSJOB_ASSERT( CELL_OK == ret );


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//And we're done.
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	printf( "End of gameloop sample\n" );

	return 0;
}

//--------------------------------------------------------------------------------------------------

void VSync( void )
{
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//In theory we're waiting no a vsync here, but since we haven't
	//initialised the graphics system, just return.
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
}

//--------------------------------------------------------------------------------------------------

void CheckResults( const char* outputName, U32 numElts, const U8* pOutput, U8 expectedValue )
{
	WWSJOB_UNUSED( outputName );
	WWSJOB_UNUSED( pOutput );
	WWSJOB_UNUSED( expectedValue );

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//Spin a loop to check that all elements in the buffer are what they are
	//expected to be.
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	for ( U32 i = 0; i < numElts; ++i )
	{
		WWSJOB_ASSERT_MSG( pOutput[i] == expectedValue, ( "ERROR: %s[%d] != %d\t\t\t0x%X != 0x%X \n", outputName, i, expectedValue, pOutput[i], expectedValue ) );
	}
}

//--------------------------------------------------------------------------------------------------
