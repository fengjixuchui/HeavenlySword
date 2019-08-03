/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		This is an SPU module to be used by the gameloop sample
**/
//--------------------------------------------------------------------------------------------------

#include <jobapi/jobapi.h>
#include <jobapi/jobspudma.h>
#include <jobapi/jobprintf.h>

#include "simplemodule.h"

//--------------------------------------------------------------------------------------------------

extern "C" void JobMain( void );

//--------------------------------------------------------------------------------------------------

void JobMain( void )
{
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//This is the entry point function that will be called for this module
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

//	JobPrintf( "***Simple Module: Begin (JobId = 0x%08X)\n", WwsJob_JobApiGetJobId() );

	// enable this line if you want to single step every time this job starts
	//WWSJOB_BREAKPOINT();

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//Using the jobApiContext value that is passed into our job, we can call
	//various useful Job Manager functionality/
	//
	//Firstly, we'll request information about the input buffer that we told
	//the Job Manager to load for us in the Load Commands.
	//We know from when we set this up on the PPU that this buffer was in
	//logicalBufferSet 1, and is in logicalBuffer 0.
	//Later on we will be dmaing this buffer out to a main memory address,
	//there at this point we also request a dmatag to use with this buffer.
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	const U32 kDataBufferSet			= 1;		//bufferset 1
	const U32 kLogicalBuffer0			= 0;		//logical buffer 0

	WwsJob_BufferTag buffTag = WwsJob_JobApiGetBufferTag( kDataBufferSet, kLogicalBuffer0, WwsJob_kAllocDmaTag );



	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//We also get the parameters for this job.
	//Note that when we start loading the next job, the params buffer
	//will get over-written.  That's why we copy them out beforehand
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	SimpleModuleParams simpleModuleParams;

	WwsJob_JobApiCopyParams( &simpleModuleParams, sizeof(simpleModuleParams) );

	U32 ea								= simpleModuleParams.m_eaOutputAddr;
	U32 outputBufferSize				= simpleModuleParams.m_outputBufferSize;
	U8 multiplier						= simpleModuleParams.m_multiplier;



	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//Now that we've finished allocating any extra resources this job needs,
	//and we've read up our input parameters, we can start the next job
	//loading.
	//
	//The next job will load in the start loading in the background in
	//parallel with this current job executing.
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	WwsJob_JobApiLoadNextJob();



	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//Now, we do all the actual work of this job *AFTER* we've started the
	//next job loading.
	//All this job actually does is multiplies every byte in the input buffer
	//by the multiplier parameter that we passed in.
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	char* lsAddr	= (char*) buffTag.GetAddr();
	U32 size		= buffTag.GetLength();

	for ( U32 i = 0; i < size; ++i )
	{
		lsAddr[i] = lsAddr[i] * multiplier;
	}



	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//
	//And then, once we've finished processing, we initiate the dma send of
	//this processed buffer we've just processed, back to main memory.
	//
	//:Note: Since the Job Manager uses SPU Event interrupts to, any job code
	//which does writes to the channel register *MUST* do so in a critical
	//section.
	//*************************THIS IS VERY IMPORTANT*************************
	//You *MUST* disable interrupts around any dma code so that an interrupt
	//does not occur in the middle of your channel writes.
	//Functions like JobDmaPut are provided for you as equivalents to
	//cellDmaPut, but which are interrupt safe and so are valid for job usage.
	//*DO NOT* use the cellDma* functions.
	//If you do channel writes in assembler, make sure you disable interrupts
	//around them.
	//*************************THIS IS VERY IMPORTANT*************************
	//
	//Note also that the dma send must use the dmatag that was allocated to
	//you by the Job Manager above, so that that way, the Job Manager can
	//track when this buffer is available again.
	//
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	//Just check that the input and output buffers were of the same size.
	WWSJOB_ASSERT( size == outputBufferSize );

	//Send the buffer back to main memory
	JobDmaPut( lsAddr, ea, outputBufferSize, buffTag.GetDmaTag() );



	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//And we're done.  We return to the Job Manager.
	//The dma will continue on in the background in parallel with the next
	//job processing.  When the Job Manager notices that the allocated dmatag
	//has completed its dma it knows that this buffer has finished being used
	//and may be reused by the next job.
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

//	JobPrintf( "***Simple Module: Done\n" );
}

//--------------------------------------------------------------------------------------------------
