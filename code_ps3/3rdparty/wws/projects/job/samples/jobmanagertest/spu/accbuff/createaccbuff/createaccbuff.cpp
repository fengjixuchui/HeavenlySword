/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Create an accumulation buffer in LS for passing on to the next job
**/
//--------------------------------------------------------------------------------------------------

#include <jobapi/jobapi.h>
#include <jobapi/jobspudma.h>
#include <jobapi/jobprintf.h>

#include "createaccbuff.h"

//--------------------------------------------------------------------------------------------------

extern "C" void JobMain( void );

//--------------------------------------------------------------------------------------------------

void JobMain( void )
{
	JobPrintf( "***Create Acc Buffer Module: Begin\n" );

	WwsJob_BufferTag buffTag = WwsJob_JobApiGetBufferTag( 1, 0, WwsJob_kDontAllocDmaTag );

	//Read out parameters before starting next job loading
	CreateAccBufferModuleParams createAccBuffParams;
	WwsJob_JobApiCopyParams( &createAccBuffParams, sizeof(createAccBuffParams) );
	U32 initValue = createAccBuffParams.m_initValue;

	//Start next job loading
	WwsJob_JobApiLoadNextJob();

	char* lsAddr	= (char*) buffTag.GetAddr();
	U32 size		= buffTag.GetLength();

	JobPrintf( "***Create Acc Buffer Module: Acc buffer is at 0x%X (size = %d)\n", (U32)lsAddr, size );

	//init the accumulation buffer
	for (U32 i = 0; i < size; ++i)
	{
		lsAddr[i] = initValue;
	}

	JobPrintf( "***Create Acc Buffer Module: Done\n" );
}

//--------------------------------------------------------------------------------------------------
