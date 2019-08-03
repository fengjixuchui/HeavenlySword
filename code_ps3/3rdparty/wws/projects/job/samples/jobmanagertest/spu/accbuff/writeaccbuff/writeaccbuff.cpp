/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Finalize the accumulation buffer and send it back to main memory
**/
//--------------------------------------------------------------------------------------------------

#include <jobapi/jobapi.h>
#include <jobapi/jobspudma.h>
#include <jobapi/jobprintf.h>

#include "writeaccbuff.h"

//--------------------------------------------------------------------------------------------------

extern "C" void JobMain( void );

//--------------------------------------------------------------------------------------------------

void JobMain( void )
{
	JobPrintf( "***Write Acc Buffer Module: Begin\n" );

	WwsJob_BufferTag buffTag = WwsJob_JobApiGetBufferTag( 1, 0, WwsJob_kAllocDmaTag );

	//Read out parameters before starting next job loading
	WriteAccBufferModuleParams writeAccBuffModParams;
	WwsJob_JobApiCopyParams( &writeAccBuffModParams, sizeof(writeAccBuffModParams) );
	U32 addValue = writeAccBuffModParams.m_addValue;

	//Start next job loading
	WwsJob_JobApiLoadNextJob();

	char* lsAddr	= (char*) buffTag.GetAddr();
	U32 size		= buffTag.GetLength();

	JobPrintf( "***Write Acc Buffer Module: Acc buffer is at 0x%X (size = %d)\n", (U32)lsAddr, size );

	//finalise the accumulation buffer
	for (U32 i = 0; i < size; ++i)
	{
		lsAddr[i] = lsAddr[i] + addValue;
	}


	//I think I should be able to wait for the JM to flush this for me.
	//Sadly, that would mean that since it's shareable, it isn't actually flushed until
	//something else re-uses that area.  However, the job is marked as done much earlier
	//than that, so the PPU will carry on to read the results *before* they're actually
	//written back to main memory.
	//This dma is a temporary solution in order to make sure the results are back in time.
	{
		U32 ea			= buffTag.m_mmAddress;
		U32 length		= buffTag.m_mmLength;
		U32 dmaTagId	= buffTag.GetDmaTag();
		JobDmaPut( lsAddr, ea, length, dmaTagId );
	}


	JobPrintf( "***Write Acc Buffer Module: Done\n" );
}

//--------------------------------------------------------------------------------------------------
