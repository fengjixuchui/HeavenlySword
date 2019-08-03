/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		This is just a simple secondary module.
				The job that uses this module will be added by an SPU job itself
**/
//--------------------------------------------------------------------------------------------------

#include <jobapi/jobapi.h>
#include <jobapi/jobspudma.h>
#include <jobapi/jobprintf.h>

#include "secondaryjob.h"

//--------------------------------------------------------------------------------------------------

extern "C" void JobMain( void );

//--------------------------------------------------------------------------------------------------

void JobMain( void )
{
	SecondaryJobModuleParams secondJobParams;
	WwsJob_JobApiCopyParams( &secondJobParams, sizeof(secondJobParams) );

	WWSJOB_ASSERT( secondJobParams.m_someValue == 0x76543210 );

	//Start the next job loading
	WwsJob_JobApiLoadNextJob();

	JobPrintf( "Job%d: Hello from the secondary job\n", WwsJob_JobApiGetJobId() );
}

//--------------------------------------------------------------------------------------------------
