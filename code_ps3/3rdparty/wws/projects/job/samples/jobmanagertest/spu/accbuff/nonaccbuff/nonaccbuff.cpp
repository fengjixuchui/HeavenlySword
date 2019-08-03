/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Depending on the memory map used, this job may or may not clash with the
				accumulation buffer used by the other "accumulationbuffer" modules
**/
//--------------------------------------------------------------------------------------------------

#include <jobapi/jobapi.h>
#include <jobapi/jobspudma.h>
#include <jobapi/jobprintf.h>

#include "nonaccbuff.h"

//--------------------------------------------------------------------------------------------------

extern "C" void JobMain( void );

//--------------------------------------------------------------------------------------------------

void JobMain( void )
{
	JobPrintf( "***Non Acc Buffer Module: Begin\n" );

	WwsJob_BufferTag buffTag = WwsJob_JobApiGetBufferTag( 2, 0, WwsJob_kDontAllocDmaTag );

	//Read out parameters before starting next job loading
	NonAccBufferModuleParams nonAccBuffModParams;
	WwsJob_JobApiCopyParams( &nonAccBuffModParams, sizeof(nonAccBuffModParams) );
	U32 clearVal = nonAccBuffModParams.m_clearVal;

	//Start next job loading
	WwsJob_JobApiLoadNextJob();

	//do all the actual work *after* we've started the next job loading

	char* lsAddr	= (char*) buffTag.GetAddr();
	U32 size		= buffTag.GetLength();

	//Clear out the buffer
	for (U32 i = 0; i < size; ++i)
	{
		lsAddr[i] = clearVal;
	}

	JobPrintf( "***Non Acc Buffer Module: Input buffer is at 0x%X (size = %d)\n", (U32)lsAddr, size );

	JobPrintf( "***Non Acc Buffer Module: Done\n" );
}

//--------------------------------------------------------------------------------------------------
