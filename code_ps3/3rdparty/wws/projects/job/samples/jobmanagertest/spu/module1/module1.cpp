/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Default test module 1
**/
//--------------------------------------------------------------------------------------------------

#include <jobapi/jobapi.h>
#include <jobapi/jobspudma.h>
#include <jobapi/jobdefinition.h>
#include <jobapi/jobprintf.h>
#include <jobapi/commandlistchecker.h>

#include "module1.h"

//--------------------------------------------------------------------------------------------------

extern "C" void JobMain( void );

//--------------------------------------------------------------------------------------------------

void JobMain( void )
{
	JobPrintf( "***Job Module 1: Begin\n" );

	WwsJob_BufferTag buffTag = WwsJob_JobApiGetBufferTag( 1, 0, WwsJob_kAllocDmaTag );

	//Read out parameters before starting next job loading
	const Module1Params* pParams = (const Module1Params*) WwsJob_JobApiGetUnsafePointerToParams();
	U32 ea			= pParams->m_eaOutputAddr;
	U8 multiplier	= pParams->m_multiplier;
	JobPrintf( "***Job Module 1: ea = 0x%08X, multiplier = %d\n", ea, multiplier );


  #if 1

	WwsJob_Command commandArray[6];

	// Get bufferSet 1, buffer 1 as a temp buffer (just to test code)
	commandArray[0].UseUninitializedBuffer( 1, 1, NULL, 0, WwsJob_Command::kNonCached );

	// Get bufferSet 1, buffer 3 as a temp buffer (just to test code)
	// Note I'm not going in sequential order
	commandArray[1].UseUninitializedBuffer( 1, 3 );
	
	// Get bufferSet 1, buffer 2 as a temp buffer (just to test code)
	// Note I'm not going in sequential order
	commandArray[2].UseUninitializedBuffer( 1, 2 );

	// unreserve data bufferSet
	commandArray[3].UnreserveBufferSets( 1 << 1/*dataBufSet#*/ );

	// command nop (to test code)
	commandArray[4].AddNopCommand();

	// end commands		
	commandArray[5].AddEndCommand();

	//CheckCommandList( commandArray, 6, kPrintCommands | kPrintWarnings | kPrintHints, 1 );

	// Execute the commands
	WwsJob_JobApiExecuteCommands( &commandArray[0] );

  #endif

  #if 1
	// Test GetBufferTags code
	WwsJob_BufferTagInputOutput bufferTagInputOutput[5];

	bufferTagInputOutput[0].m_bufferTagInput.m_logicalBufferSetNum	= 0;
	bufferTagInputOutput[0].m_bufferTagInput.m_logicalBufferNum		= 0;
	bufferTagInputOutput[0].m_bufferTagInput.m_useDmaTagId			= 1;
	
	bufferTagInputOutput[1].m_bufferTagInput.m_logicalBufferSetNum	= 1;
	bufferTagInputOutput[1].m_bufferTagInput.m_logicalBufferNum		= 0;
	bufferTagInputOutput[1].m_bufferTagInput.m_useDmaTagId			= 0; // note first GetBufferTag call said TRUE
	
	bufferTagInputOutput[2].m_bufferTagInput.m_logicalBufferSetNum	= 1;
	bufferTagInputOutput[2].m_bufferTagInput.m_logicalBufferNum		= 1;
	bufferTagInputOutput[2].m_bufferTagInput.m_useDmaTagId			= 1;

	bufferTagInputOutput[3].m_bufferTagInput.m_logicalBufferSetNum	= 1;
	bufferTagInputOutput[3].m_bufferTagInput.m_logicalBufferNum		= 2;
	bufferTagInputOutput[3].m_bufferTagInput.m_useDmaTagId			= 0;

	bufferTagInputOutput[4].m_bufferTagInput.m_logicalBufferSetNum	= 1;
	bufferTagInputOutput[4].m_bufferTagInput.m_logicalBufferNum		= 3;
	bufferTagInputOutput[4].m_bufferTagInput.m_useDmaTagId			= 1;

	// Execute the commands
	WwsJob_JobApiGetBufferTags( &bufferTagInputOutput[0], 5 );

	for( U32 i = 0 ; i <= 4 ; i++ )
	{
		JobPrintf( "***Job Module 1: For logicalBufferSetNum = %d, logicalBufferNum = %d :\n",
				bufferTagInputOutput[i].m_bufferTagOutput.m_logicalBufferSetNum, 
				bufferTagInputOutput[i].m_bufferTagOutput.m_logicalBufferNum );		
		JobPrintf( "***Job Module 1:   dmaTagId = 0x%02x, lsAdrs = 0x%x, lsLength = 0x%x, mmAdrs = 0x%x, mmLength = 0x%x\n",
				bufferTagInputOutput[i].m_bufferTagOutput.m_dmaTagId,
				bufferTagInputOutput[i].m_bufferTagOutput.m_lsAddressInWords,
				bufferTagInputOutput[i].m_bufferTagOutput.m_lsLengthInWords,
				bufferTagInputOutput[i].m_bufferTagOutput.m_mmAddress,
				bufferTagInputOutput[i].m_bufferTagOutput.m_mmLength );
	}
  #endif

  #if 1
	// Use an extra dmaTagId, and free it, just to test code
	U32 dmaTagId = WwsJob_JobApiUseDmaTag();
	WwsJob_JobApiFreeDmaTagId( dmaTagId );
  #endif

  #if 1
	// Test code to free a logical buffer
	WwsJob_JobApiFreeLogicalBuffer( 1, 1 );

	// Test code to free logical buffers
	U8 pairs[2*2];

	pairs[0*2 + 0] = 1; // bufferSet
	pairs[0*2 + 1] = 2; // buffer

	pairs[1*2 + 0] = 1; // bufferSet
	pairs[1*2 + 1] = 3; // buffer
	
	WwsJob_JobApiFreeLogicalBuffers( &pairs[0], 2 );
  #endif


	//Start next job loading
	WwsJob_JobApiLoadNextJob();

	//do all the actual work *after* we've started the next job loading

	//JobPrintf( "***Job Module 1: m_logicalBufferSetNum = %d\n", apiReturn.m_bufferTag.m_logicalBufferSetNum );
	//JobPrintf( "***Job Module 1: m_logicalBufferNum = %d\n", apiReturn.m_bufferTag.m_logicalBufferNum );
	//JobPrintf( "***Job Module 1: m_dmaTagId = %d\n", apiReturn.m_bufferTag.m_dmaTagId );
	//JobPrintf( "***Job Module 1: m_lsAddressInWords = %d\n", apiReturn.m_bufferTag.m_lsAddressInWords );
	//JobPrintf( "***Job Module 1: m_lsLengthInWords = %d\n", apiReturn.m_bufferTag.m_lsLengthInWords );
	//JobPrintf( "***Job Module 1: m_mmAddress = 0x%X\n", apiReturn.m_bufferTag.m_mmAddress );
	//JobPrintf( "***Job Module 1: m_mmLength = 0x%X\n", apiReturn.m_bufferTag.m_mmLength );

	char* lsAddr	= (char*) buffTag.GetAddr();
	U32 size		= buffTag.GetLength();

	//JobPrintf( "***Job Module 1: Input buffer is at 0x%X (size = %d)\n", (U32)lsAddr, size );

	//double the contents of the buffer
	for (U32 i = 0; i < size; ++i)
	{
		//JobPrintf( "***Job Module 1: LS buffer[%d] = 0x%X\n", i, lsAddr[i] );
		lsAddr[i] = lsAddr[i] * multiplier;
	}

	//JobPrintf( "***Job Module 1: Put buffer from 0x%08X to 0x%016llX (size = 0x%X, tag = %d)\n", lsAddr, ea, size, apiReturn.m_bufferTag.m_dmaTagId );
	//Send the buffer back to main memory
	JobDmaPut( lsAddr, ea, size, buffTag.GetDmaTag() );

	JobPrintf( "***Job Module 1: Done\n" );
}

//--------------------------------------------------------------------------------------------------
