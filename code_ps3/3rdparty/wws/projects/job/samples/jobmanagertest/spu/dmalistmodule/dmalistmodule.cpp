/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Test dmalist functionality
**/
//--------------------------------------------------------------------------------------------------

#include <jobapi/jobapi.h>
#include <jobapi/jobspudma.h>
#include <jobapi/jobprintf.h>

#include "dmalistmodule.h"

//--------------------------------------------------------------------------------------------------

extern "C" void JobMain( void );

//--------------------------------------------------------------------------------------------------

void JobMain( void )
{
	JobPrintf( "***Job DmaListModule: Begin\n" );

	WwsJob_BufferTag buffTag = WwsJob_JobApiGetBufferTag( 1, 0, WwsJob_kAllocDmaTag );

	//Read out parameters before starting next job loading
	DmaListModuleParams params;
	WwsJob_JobApiCopyParams( &params, sizeof(params) );
	U32 ea			= params.m_eaOutputAddr;
	U8 multiplier	= params.m_multiplier;
	JobPrintf( "***Job DmaListModule: ea = 0x%08X\n", ea );
	JobPrintf( "***Job DmaListModule: multiplier = %d\n", multiplier );

	//Start next job loading
	WwsJob_JobApiLoadNextJob();

	//do all the actual work *after* we've started the next job loading


	char* lsAddr	= (char*) buffTag.GetAddr();
	U32 size		= buffTag.GetLength();

	JobPrintf( "***Job DmaListModule: input buffer is at 0x%X (size = %d)\n", (U32)lsAddr, size );

	//double the contents of the buffer
	for (U32 i = 0; i < size; ++i)
	{
		//JobPrintf( "***Job DmaListModule: LS buffer[%d] = 0x%X\n", i, lsAddr[i] );
		lsAddr[i] = lsAddr[i] * multiplier;
	}

	//JobPrintf( "***Job DmaListModule: Put buffer from 0x%08X to 0x%016llX (size = 0x%X, tag = %d)\n", lsAddr, ea, size, apiReturn.m_bufferTag.m_dmaTagId );
	//Send the buffer back to main memory
	JobDmaPut( lsAddr, ea, size, buffTag.GetDmaTag() );

	JobPrintf( "***Job DmaListModule: Done\n" );
}

//--------------------------------------------------------------------------------------------------
