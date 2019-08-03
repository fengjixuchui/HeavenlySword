/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		This SPU module triggers a callback on the PPU
**/
//--------------------------------------------------------------------------------------------------

#include <jobapi/jobapi.h>

#include "ppucallbackjob.h"

//--------------------------------------------------------------------------------------------------

extern "C" void JobMain( void );

//--------------------------------------------------------------------------------------------------

void JobMain( void )
{
	PpuCallbackModuleParams params;
	WwsJob_JobApiCopyParams( &params, sizeof(params) );

	U32 portNum = params.m_port;

	//Start the next job loading
	WwsJob_JobApiLoadNextJob();

	//Send two events for the PPU to pick up on.
	WwsJob_JobApiSendEvent( portNum, 0x123456, 0x12345678 );

	//Sending a second event is a really bad plan as the mailbox queue is only 1 deep, so you guarantee blocking here.
	//Only send SPU events *very* rarely.
	//So don't do this.
	//Sending events should only be used to mark specific section of a frame's processing, say.
	//They are not intended for frequent use or for any fine grained calling back.
	//WwsJob_JobApiSendEvent( portNum, 0xFEDCBA, 0xFEDCBA98);
}

//--------------------------------------------------------------------------------------------------
