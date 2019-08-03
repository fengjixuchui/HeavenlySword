/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Modify the data in the accumulation buffer
				This module may be run many times to keep adding to the data in the accumulation
				buffer, and the buffer will not be flushed back to main memory unless it has to
				because an incompatible job gets in the middle
**/
//--------------------------------------------------------------------------------------------------

#include <jobapi/jobapi.h>
#include <jobapi/jobspudma.h>
#include <jobapi/jobprintf.h>

#include "fillaccbuff.h"

//--------------------------------------------------------------------------------------------------

extern "C" void JobMain( void );

//--------------------------------------------------------------------------------------------------

void JobMain( void )
{
	JobPrintf( "***Fill Acc Buffer Module: Begin\n" );

	WwsJob_BufferTag buffTag = WwsJob_JobApiGetBufferTag( 1, 0, WwsJob_kDontAllocDmaTag );

	//Read out parameters before starting next job loading
	FillAccBufferModuleParams fillAccBufferParams;
	WwsJob_JobApiCopyParams( &fillAccBufferParams, sizeof(fillAccBufferParams) );
	U32 multiplier = fillAccBufferParams.m_multiplier;

	//Start next job loading
	WwsJob_JobApiLoadNextJob();

	char* lsAddr	= (char*) buffTag.GetAddr();
	U32 size		= buffTag.GetLength();

	JobPrintf( "***Fill Acc Buffer Module: Acc buffer is at 0x%X (size = %d)\n", (U32)lsAddr, size );

	//accumulate values into the accumulation buffer with 1s
	for (U32 i = 0; i < size; ++i)
	{
		lsAddr[i] = lsAddr[i] * multiplier;
	}


	JobPrintf( "***Fill Acc Buffer Module: Done\n" );
}

//--------------------------------------------------------------------------------------------------
