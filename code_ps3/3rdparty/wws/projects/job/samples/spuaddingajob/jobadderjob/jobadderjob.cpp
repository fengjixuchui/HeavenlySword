/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		This SPU module creates a new job and adds it to a joblist
**/
//--------------------------------------------------------------------------------------------------

#include <jobapi/jobdefinition.h>

#include <jobapi/jobapi.h>
#include <jobapi/jobspudma.h>
#include <jobapi/jobprintf.h>

#include "jobadderjob.h"

#include "../secondaryjob/secondaryjob.h"

//--------------------------------------------------------------------------------------------------

extern "C" void JobMain( void );

//--------------------------------------------------------------------------------------------------

void JobMain( void )
{
	JobAdderJobModuleParams jobAdderParams;
	WwsJob_JobApiCopyParams( &jobAdderParams, sizeof(jobAdderParams) );

	//Read out the params
	U32 eaNewJobBuffer						= jobAdderParams.m_eaNewJobBuffer;
	U32 newJobBufferOutputSize				= jobAdderParams.m_newJobBufferSize;
	U32 eaSpuModule							= jobAdderParams.m_eaSpuModule;
	U32 spuModuleFileSize					= jobAdderParams.m_spuModuleFileSize;
	U32 spuModuleRequiredBufferSizeInPages	= jobAdderParams.m_spuModuleRequiredBufferSizeInPages;
	U32 eaJobList							= jobAdderParams.m_eaJobList;
	CellSpursWorkloadId workloadId			= jobAdderParams.m_workloadId;


	//Start the next job loading
	WwsJob_JobApiLoadNextJob();


	WwsJob_Command commandList[32];

	const U32 kNewJobCodeBufferSet				= 0;					//bufferset 0
	const U32 kNewJobNumCodeBuffers				= 1;					//contains 1 buffer
	const U32 kNewJobCodeBufferSizeInPages		= spuModuleRequiredBufferSizeInPages;
	const U32 kNewJobCodeBufferSetBasePageNum	= LsMemoryLimits::kJobAreaBasePageNum;			//The first buffer of this bufferset starts at this address

	const U32 kLogicalBuffer0 = 0;

	SecondaryJobModuleParams secondJobParams;
	secondJobParams.m_someValue = 0x76543210;

	//Create the new job
	commandList[0].ReserveBufferSet( kNewJobCodeBufferSet, kNewJobNumCodeBuffers, kNewJobCodeBufferSetBasePageNum, kNewJobCodeBufferSizeInPages );
	commandList[1].UseInputBuffer( kNewJobCodeBufferSet, kLogicalBuffer0, eaSpuModule, spuModuleFileSize, WwsJob_Command::kReadOnlyCached );
	commandList[2].UnreserveBufferSets( 1 << kNewJobCodeBufferSet );
	commandList[3].RunJob( kNewJobCodeBufferSet, kLogicalBuffer0 );
	// If needed, you can store a command of 0 (which is not executed) to ensure the params are qword aligned
	WwsJob_MemcpyQword( &commandList[4], &secondJobParams, sizeof(secondJobParams) );

	U32 newJobBufferFilledSize = 4 * sizeof( WwsJob_Command ) + sizeof( JobAdderJobModuleParams );
	WWSJOB_ASSERT( newJobBufferFilledSize <= newJobBufferOutputSize );
	WWSJOB_UNUSED( newJobBufferOutputSize );


	//Send the buffer of commands for the new job out to main memory
	U32 dmaTagId = WwsJob_JobApiUseDmaTag();
	JobDmaPut( commandList, eaNewJobBuffer, newJobBufferFilledSize, dmaTagId );
	JobDmaWaitTagStatusAll( 1 << dmaTagId );
	WwsJob_JobApiFreeDmaTagId( dmaTagId );

	JobPrintf( "Job%d: Adding a job\n", WwsJob_JobApiGetJobId() );

	//And add the job to the joblist
	JobHeader jobHeader;
	jobHeader.m_mmaLoadCommands			= (U32) eaNewJobBuffer;
	jobHeader.m_loadCommandsSize		= newJobBufferFilledSize;
	jobHeader.m_enableBreakpoint		= 0;
	jobHeader.m_jobHeaderCommand		= JobHeaderCommand::kJobExists;
	WwsJob_JobApiAddJobToJobList( eaJobList, jobHeader );
	WwsJob_JobApiSetReadyCount( workloadId, 1 );
}

//--------------------------------------------------------------------------------------------------
