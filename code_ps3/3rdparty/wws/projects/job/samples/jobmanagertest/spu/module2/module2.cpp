/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Default test module 2
**/
//--------------------------------------------------------------------------------------------------

#include <jobapi/jobapi.h>
#include <jobapi/jobspudma.h>
#include <jobapi/jobdefinition.h>
#include <jobapi/jobprintf.h>
#include <jobapi/commandlistchecker.h>

#include "module2.h"

//--------------------------------------------------------------------------------------------------

extern "C" void JobMain( void );

//--------------------------------------------------------------------------------------------------

//This large buffer is in the ".uninitialized" section.
//It is not part of the minimal binary file that is uploaded to LS as it has no contents.
//However it is also not zero initialized like the ".bss" would be.
//The contents of an ".uninitialized" buffer are, of course, uninitialized which means that
//they could be any random data at the start of a job and it's up to the job to fill them in.
U8 g_uninitializedBuffer[16*1024] WWSJOB_ALIGNED( 128 ) WWSJOB_UNINITIALIZED();

//--------------------------------------------------------------------------------------------------

class MyClass
{
public:
	MyClass();
	~MyClass();
};

bool gGlobalConstructorHasBeenCalled = false;

MyClass::MyClass()
{
	JobPrintf( "***Job Module 2: MyClass global constructor called\n" );
	gGlobalConstructorHasBeenCalled = true;;
}

MyClass::~MyClass()
{
	JobPrintf( "***Job Module 2: MyClass global destructor called\n" );
	WWSJOB_ASSERT( gGlobalConstructorHasBeenCalled );
}

MyClass gMyClass;

//--------------------------------------------------------------------------------------------------

void JobMain( void )
{
	// enable this line if you want to single step every time this job starts
	//WWSJOB_BREAKPOINT();
  	
	JobPrintf( "***Job Module 2: Begin***\n" );

	WWSJOB_ASSERT( gGlobalConstructorHasBeenCalled );

	WwsJob_BufferTag buffTag = WwsJob_JobApiGetBufferTag( 1, 0, WwsJob_kAllocDmaTag );

	//Read out parameters before starting next job loading
	Module2Params module2Params;
	WwsJob_JobApiCopyParams( &module2Params, sizeof(module2Params) );
	U32 ea			= module2Params.m_eaOutputAddr;
	U8 multiplier	= module2Params.m_multiplier;
	//JobPrintf( "***Job Module 2: ea = 0x%08X\n", ea );
	//JobPrintf( "***Job Module 2: multiplier = %d\n", multiplier );

	// Start next job loading
	// NOTE: this code has been moved up from below to ensure the new code works
	WwsJob_JobApiLoadNextJob();

  #if 0
	// do a bunch of prints, to ensure the next job can load and
	// exercise the reservedPages bits
	for( U32 i = 0 ; i < 10 ; i++ )
		JobPrintf( "***Job Module 2: dummy print just for delay\n" );
  #endif
  
  #if 1
	// Get bufferSet 1, buffer 1 as a temp buffer (just to test code)
	// everything set just to highlight what there is
	WwsJob_Command commandArray[6];
	commandArray[0].m_u64 = 0;
	commandArray[0].m_useBufferCommand.m_flags.m_mmLengthInQwords			= 0;
	commandArray[0].m_useBufferCommand.m_flags.m_continueAfterDiscardWrite	= 0;
	commandArray[0].m_useBufferCommand.m_flags.m_inputRead					= 0;                 
	commandArray[0].m_useBufferCommand.m_flags.m_inputGather               	= 0;
	commandArray[0].m_useBufferCommand.m_flags.m_outputShareable			= 0;
	commandArray[0].m_useBufferCommand.m_flags.m_shareableWriteIfDiscarded 	= 0;
	commandArray[0].m_useBufferCommand.m_flags.m_commandNum                	= CommandNum::kUseBuffer;
	commandArray[0].m_useBufferCommand.m_flags.m_logicalBufferSetNum       	= 1;
	commandArray[0].m_useBufferCommand.m_flags.m_logicalBufferNum          	= 1;
	commandArray[0].m_useBufferCommand.m_mmAddressInQwords					= 0;
	commandArray[0].m_useBufferCommand.m_pad								= 0;
	commandArray[0].m_useBufferCommand.m_numPadQwordsBelowListElements		= 0;

	// Get bufferSet 1, buffer 3 as a temp buffer (just to test code)
	// Note I'm not going in sequential order
	commandArray[1].m_u64 = 0;
	commandArray[1].m_useBufferCommand.m_flags.m_commandNum                	= CommandNum::kUseBuffer;
	commandArray[1].m_useBufferCommand.m_flags.m_logicalBufferSetNum       	= 1;
	commandArray[1].m_useBufferCommand.m_flags.m_logicalBufferNum          	= 3;
	
	// Get bufferSet 1, buffer 2 as a temp buffer (just to test code)
	// Note I'm not going in sequential order
	// Also clear reservedPages for bufferSet 1 (don't "use" buffers in bufferSet 1 after this)
	commandArray[2].m_u64 = 0;
	commandArray[2].m_useBufferCommand.m_flags.m_commandNum                	= CommandNum::kUseBuffer;
	commandArray[2].m_useBufferCommand.m_flags.m_logicalBufferSetNum       	= 1;
	commandArray[2].m_useBufferCommand.m_flags.m_logicalBufferNum          	= 2;

	// unreserve bufferSet1 (this will clear the reservedPage bits for them)
	commandArray[3].m_u64 = 0;
	commandArray[3].m_unreserveBufferSetsCommand.m_flags.m_commandNum		= CommandNum::kUnreserveBufferSets;
	commandArray[3].m_unreserveBufferSetsCommand.m_bufferSetMask		 	= 1 << 1/*bufSet#*/;
	
	// command nop (to test code)
	commandArray[4].m_u32[0] = 0; // nop

	// end commands		
	commandArray[5].m_u64 = 0;
	commandArray[5].m_useBufferCommand.m_flags.m_commandNum					= CommandNum::kEndCommand;

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
		JobPrintf( "***Job Module 2: For logicalBufferSetNum = %d, logicalBufferNum = %d :\n",
				bufferTagInputOutput[i].m_bufferTagOutput.m_logicalBufferSetNum, 
				bufferTagInputOutput[i].m_bufferTagOutput.m_logicalBufferNum );		
		JobPrintf( "***Job Module 2:   dmaTagId = 0x%02x, lsAdrs = 0x%x, lsLength = 0x%x, mmAdrs = 0x%x, mmLength = 0x%x\n",
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


	// Start next job loading
	// NOTE: this code has been moved up (before some of the useBuffer commands to ensure the new code works)
	//WwsJob_JobApiLoadNextJob();

	//do all the actual work *after* we've started the next job loading

	//JobPrintf( "***Job Module 2: m_logicalBufferSetNum = %d\n", apiReturn.m_bufferTag.m_logicalBufferSetNum );
	//JobPrintf( "***Job Module 2: m_logicalBufferNum = %d\n", apiReturn.m_bufferTag.m_logicalBufferNum );
	//JobPrintf( "***Job Module 2: m_dmaTagId = %d\n", apiReturn.m_bufferTag.m_dmaTagId );
	//JobPrintf( "***Job Module 2: m_lsAddressInWords = %d\n", apiReturn.m_bufferTag.m_lsAddressInWords );
	//JobPrintf( "***Job Module 2: m_lsLengthInWords = %d\n", apiReturn.m_bufferTag.m_lsLengthInWords );
	//JobPrintf( "***Job Module 2: m_mmAddress = 0x%X\n", apiReturn.m_bufferTag.m_mmAddress );
	//JobPrintf( "***Job Module 2: m_mmLength = 0x%X\n", apiReturn.m_bufferTag.m_mmLength );

	char* lsAddr	= (char*) buffTag.GetAddr();
	U32 size		= buffTag.GetLength();

	//JobPrintf( "***Job Module 2: input buffer is at 0x%X (size = %d)\n", (U32)lsAddr, size );

	//double the contents of the buffer
	for (U32 i = 0; i < size; ++i)
	{
		//JobPrintf( "***Job Module 2: LS buffer[%d] = 0x%X\n", i, lsAddr[i] );
		lsAddr[i] = lsAddr[i] * multiplier;
	}

	//JobPrintf( "***Job Module 2: Put buffer from 0x%08X to 0x%016llX (size = 0x%X, tag = %d)\n", lsAddr, ea, size, apiReturn.m_bufferTag.m_dmaTagId );
	//Send the buffer back to main memory
	JobDmaPut( lsAddr, ea, size, buffTag.GetDmaTag() );

	JobPrintf( "***Job Module 2: Done*\n" );
}

//--------------------------------------------------------------------------------------------------
