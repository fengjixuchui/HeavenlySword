/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Utility function for disassembling a command list and checking its validity
**/
//--------------------------------------------------------------------------------------------------

#include <jobapi/commandlistchecker.h>
#include <jobapi/jobdefinition.h>
#include <jobapi/jobprintf.h>
#include <jobapi/spumoduleheader.h>
#include <string.h>
#ifndef __SPU__
#include <sys/process.h>
#endif

//Since the point of this file is for error checking, the asserts are not removed on a release build
//and the printfs remain in.  The whole function can just be not called instead.

//--------------------------------------------------------------------------------------------------

static inline void PrintText( const char* text )
{
	WWSJOB_UNUSED( text );
	JobBasePrintf( "\t\t%s\n", text );
}

//--------------------------------------------------------------------------------------------------

static inline void PrintValU32( const char* name, U32 val )
{
	WWSJOB_UNUSED( name );
	WWSJOB_UNUSED( val );
	JobBasePrintf( "\t\t%s =\t0x%X\n", name, val );
}

//--------------------------------------------------------------------------------------------------

static inline void PrintValU64( const char* name, U64 val )
{
	WWSJOB_UNUSED( name );
	WWSJOB_UNUSED( val );
	JobBasePrintf( "\t\t%s = 0x%016llX\n", name, val );
}

//--------------------------------------------------------------------------------------------------

static inline void PrintActive( const char* onName, const char* offName, bool active )
{
	const char* name = active ? onName : offName;
	WWSJOB_UNUSED( name );
	JobBasePrintf( "\t\t%s\n", name);
}

//--------------------------------------------------------------------------------------------------

void CheckJob( JobHeader jobHeader, U32 options, U32 numSpus )
{
	switch ( jobHeader.m_jobHeaderCommand )
	{
	case JobHeaderCommand::kGeneralBarrier:
		{
			U32 mmDependencyAddress = jobHeader.m_mmaLoadCommands;
			if ( options & kPrintCommands )
			{
				JobBasePrintf( "  kGeneralBarrier:\n" );
				PrintValU32( "mmDependencyAddress", mmDependencyAddress );
				{
					PrintText( "{" );
					PrintText( "\tAt time of disassembly:" );
					const DependencyCounter* pDependency = (const DependencyCounter*) jobHeader.m_mmaLoadCommands;
					PrintValU32( "\tmmDependencyAddress->m_counter", pDependency->m_counter );
					PrintValU32( "\tmmDependencyAddress->m_workloadId", pDependency->m_workloadId );
					PrintValU32( "\tmmDependencyAddress->m_readyCount", pDependency->m_readyCount );
					PrintText( "}" );
				}
			}
			WWSJOB_ALWAYS_ASSERT( mmDependencyAddress );
			WWSJOB_ALWAYS_ASSERT( WwsJob_IsAligned( mmDependencyAddress, 4 ) );
		}
		return;

	case JobHeaderCommand::kNoJobYet:
		if ( options & kPrintCommands )
		{
			JobBasePrintf( "  kNoJobYet:\n" );
		}
		return;

	case JobHeaderCommand::kJobExists:
		//This is the main case, and the code for it is below
		WWSJOB_ALWAYS_ASSERT( WwsJob_IsAligned( jobHeader.m_mmaLoadCommands, 16 ) );
		WWSJOB_ALWAYS_ASSERT( WwsJob_IsAligned( jobHeader.m_loadCommandsSize, 16 ) );
		WWSJOB_ALWAYS_ASSERT( jobHeader.m_loadCommandsSize <= MAX_LOAD_COMMANDS_SIZE );

		if ( options & kPrintCommands )
		{
			JobBasePrintf( "  kJobExists (ea = 0x%08X, size = 0x%X):\n", jobHeader.m_mmaLoadCommands, jobHeader.m_loadCommandsSize );
		}

		//Whatever options were passed in to this function, we also know this is a load-commands list for a job, so we also set kErrorCheckJob
		CheckCommandList( (const WwsJob_Command*) jobHeader.m_mmaLoadCommands, jobHeader.m_loadCommandsSize / sizeof(U64), options | kErrorCheckJob, numSpus );
		break;

	case JobHeaderCommand::kNopJob:
		WWSJOB_ALWAYS_ASSERT( jobHeader.m_mmaLoadCommands == 0 );
		WWSJOB_ALWAYS_ASSERT( jobHeader.m_loadCommandsSize == 0 );
		WWSJOB_ALWAYS_ASSERT( jobHeader.m_enableBreakpoint == 0 );
		if ( options & kPrintCommands )
		{
			JobBasePrintf( "  kNopJob:\n" );
		}
		break;

	default:
		WWSJOB_ALWAYS_ASSERT( false );
		break;
	}
}

//--------------------------------------------------------------------------------------------------

void CheckCommandList( const WwsJob_Command* pCommandList, U32 numU64s, U32 options, U32 numSpus )
{
	ReserveBufferSetCommand reserveBufferSetCommand[WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB];	//Maybe use a more appropriate structure for this data
	U32 numBuffersReservedInBufferSet[WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB];
	memset( numBuffersReservedInBufferSet, 0, sizeof(numBuffersReservedInBufferSet) );	//0 buffers reserved in each buffer set
	
	UseBufferCommand useBufferCommand[WWSJOB_MAX_NUM_BUFFERS_PER_JOB];

	U32 numBufferSetsInJob	= 0;
	U32 numBuffersInJob		= 0;

	U32 numDependencyDecrementsRequested = 0;	//A job cannot request more than 4 dependency decrements via the job manager

	if ( options & kPrintCommands )
	{
		JobBasePrintf( "    Command list (ea = 0x%08X):\n", (U32)pCommandList );
	}

	bool disassemblingInstructions	= true;	//Set to false once we reach the data

	if ( options & kErrorCheckJob )	//This assert is only for load commands, not run-time commands
	{
		WWSJOB_ALWAYS_ASSERT( numU64s <= ((MAX_LOAD_COMMANDS_SIZE)/sizeof(U64)) );
	}

	//Loop over all the U64s for this job or command list
	for ( U32 commandIndex = 0; commandIndex < numU64s; ++commandIndex )
	{
		const WwsJob_Command* pCmd = &pCommandList[commandIndex];

		if ( disassemblingInstructions )
		{
			//JobBasePrintf( "\t0x%016llX\n", pCommand );
			switch ( pCmd->m_reserveBufferSetCommand.m_flags.m_commandNum )
			{
			case CommandNum::kReserveBufferSet:
				{
					U32 firstPageNum		= pCmd->m_reserveBufferSetCommand.m_bufferSet.m_firstPageNum;
					U32 numBuffers			= pCmd->m_reserveBufferSetCommand.m_bufferSet.m_numBuffers;
					U32 numPagesPerBuffer	= pCmd->m_reserveBufferSetCommand.m_bufferSet.m_numPagesPerBuffer;
					U32 logicalBufferSetNum	= pCmd->m_reserveBufferSetCommand.m_flags.m_logicalBufferSetNum;
					if ( options & kPrintCommands )
					{
						JobBasePrintf( "      0x%016llX  kReserveBufferSet\n", pCmd->m_u64 );
						PrintValU32( "logicalBufferSetNum", logicalBufferSetNum );
						PrintValU32( "firstPageNum", firstPageNum );
						PrintValU32( "numPagesPerBuffer", numPagesPerBuffer );
						PrintValU32( "numBuffers", numBuffers );
					}
					WWSJOB_ALWAYS_ASSERT( firstPageNum >= LsMemoryLimits::kJobAreaBasePageNum );
					WWSJOB_ALWAYS_ASSERT( (firstPageNum + numBuffers*numPagesPerBuffer) <= LsMemoryLimits::kJobAreaEndPageNum );

					U32 endPageNum = firstPageNum + (numBuffers * numPagesPerBuffer);	//The next buffer set can start at this address
					WWSJOB_ALWAYS_ASSERT( firstPageNum < endPageNum );	//Be suspicious if we're reserving a zero-sized buffer
					WWSJOB_UNUSED( endPageNum );

					//Check through the list of buffersets reserved so far
					for ( U32 i = 0; i < numBufferSetsInJob; ++i )
					{
						const ReserveBufferSetCommand* pCheckBufferSet = &reserveBufferSetCommand[i];

						//The same logicalBufferSetNum can't be reserved a second time
						WWSJOB_ALWAYS_ASSERT( logicalBufferSetNum != pCheckBufferSet->m_flags.m_logicalBufferSetNum );

						//We mustn't overlap with a previously reserved bufferset
						U32 checkFirstPageNum = pCheckBufferSet->m_bufferSet.m_firstPageNum;
						U32 checkEndPageNum = checkFirstPageNum + (pCheckBufferSet->m_bufferSet.m_numBuffers
																	* pCheckBufferSet->m_bufferSet.m_numPagesPerBuffer);
						WWSJOB_ALWAYS_ASSERT( (endPageNum <= checkFirstPageNum) || (checkEndPageNum <= firstPageNum ) );
						WWSJOB_UNUSED( checkFirstPageNum );
						WWSJOB_UNUSED( checkEndPageNum );
					}

					//Remember the data for error checking on future commands
					reserveBufferSetCommand[numBufferSetsInJob] = pCmd->m_reserveBufferSetCommand;
					++numBufferSetsInJob;
					WWSJOB_ALWAYS_ASSERT( numBufferSetsInJob <= WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB );	//Don't go over the upper limit of num buffer sets per job

					//Note that if this is a run-time command list, other buffer sets may already have been reserved in the load-commands that we don't know about here
				}
				break;

			case CommandNum::kUnreserveBufferSets:
				if ( options & kPrintCommands )
				{
					JobBasePrintf( "      0x%016llX  kUnreserveBufferSets\n", pCmd->m_u64 );
					PrintValU32( "bufferSetMask", pCmd->m_unreserveBufferSetsCommand.m_bufferSetMask );
				}
				break;

			case CommandNum::kUseBuffer:
				{
					U32 logicalBufferSetNum				= pCmd->m_useBufferCommand.m_flags.m_logicalBufferSetNum;
					U32 logicalBufferNum				= pCmd->m_useBufferCommand.m_flags.m_logicalBufferNum;
					U32 mmAddressInQwords				= pCmd->m_useBufferCommand.m_mmAddressInQwords;
					U32 mmLengthInQwords				= pCmd->m_useBufferCommand.m_flags.m_mmLengthInQwords;
					U32 inputRead						= pCmd->m_useBufferCommand.m_flags.m_inputRead;
					U32 inputGather						= pCmd->m_useBufferCommand.m_flags.m_inputGather;
					U32 outputShareable					= pCmd->m_useBufferCommand.m_flags.m_outputShareable;
					U32 shareableWriteIfDiscarded		= pCmd->m_useBufferCommand.m_flags.m_shareableWriteIfDiscarded;
					U32 numPadQwordsBelowListElements	= pCmd->m_useBufferCommand.m_numPadQwordsBelowListElements;
					if ( options & kPrintCommands )
					{
						JobBasePrintf( "      0x%016llX  kUseBuffer\n", pCmd->m_u64 );
						PrintActive( "Read Input", "No Input", inputRead );
						PrintActive( "ListDma Input", "Not ListDma Input", inputGather );
						PrintActive( "Output Persists", "Output Does Not Persist", outputShareable );
						PrintActive( "If Discarded Write Shareable", "If Discarded Don't Write Shareable", shareableWriteIfDiscarded );
						PrintValU32( "logicalBufferSetNum", logicalBufferSetNum );
						PrintValU32( "logicalBufferNum", logicalBufferNum );
						PrintValU32( "mmAddressInQwords", mmAddressInQwords );
						PrintValU32( "mmLengthInQwords", mmLengthInQwords );
						PrintValU32( "numPadQwordsBelowListElements", numPadQwordsBelowListElements );
					}
					WWSJOB_ALWAYS_ASSERT( pCmd->m_useBufferCommand.m_flags.m_continueAfterDiscardWrite == 0 );

					U32 mmAddress = mmAddressInQwords * 16;

#ifndef __SPU__
					if ( inputRead || shareableWriteIfDiscarded )
					{
						int isStack = sys_process_is_stack( (void*)mmAddress );
						WWSJOB_ALWAYS_ASSERT( !isStack );	//Note that the SPUs may not read from the PPU's stack
					}
#endif

					//Find the bufferset this buffer is in
					const ReserveBufferSetCommand* pBufferSet = NULL;
					U32 buffsetIndex;
					for ( buffsetIndex = 0; buffsetIndex < numBufferSetsInJob; ++buffsetIndex )
					{
						const ReserveBufferSetCommand* pCheckBufferSet = &reserveBufferSetCommand[buffsetIndex];
						if ( logicalBufferSetNum == pCheckBufferSet->m_flags.m_logicalBufferSetNum )
						{
							pBufferSet = pCheckBufferSet;
							break;
						}
					}

					if ( options & kErrorCheckJob )	//This assert is only for load commands, not run-time commands
					{
						//Check that the used buffers have actually been reserved
						WWSJOB_ALWAYS_ASSERT( pBufferSet );

						//If this is a run-time command list, then it's possible the buffer set
						//was reserved in a load-command, so pBufferSet will be NULL here
					}

					if ( pBufferSet )
					{
						//Check that we aren't using more buffers in a bufferset than we've actually reserved
						WWSJOB_ALWAYS_ASSERT( numBuffersReservedInBufferSet[buffsetIndex] < pBufferSet->m_bufferSet.m_numBuffers );
						++numBuffersReservedInBufferSet[buffsetIndex];
					}

					//Don't go over the upper limit of num buffers per job
					WWSJOB_ALWAYS_ASSERT( numBuffersInJob < WWSJOB_MAX_NUM_BUFFERS_PER_JOB );

					//The same logical buffer index of a bufferset can't be used a second time
					for ( U32 i = 0; i < numBuffersInJob; ++i )
					{
						//If it's the same bufferset
						if ( logicalBufferSetNum == useBufferCommand[i].m_flags.m_logicalBufferSetNum )
						{
							//Then it must be different logical buffer num
							WWSJOB_ALWAYS_ASSERT( logicalBufferNum != useBufferCommand[i].m_flags.m_logicalBufferNum );
						}
					}

					//Remember the data for error checking on future commands in this command list
					useBufferCommand[numBuffersInJob] = pCmd->m_useBufferCommand;
					++numBuffersInJob;

					//If it's uninitialized, it can't be read-only cached
					if ( (inputRead == 0) && (inputGather == 0) )
					{
						//If it's uninitialized,
						//Then either the output isn't offered up for sharing
						//Or if it is offered up for sharing, then it must also be written out if discarded
						WWSJOB_ALWAYS_ASSERT( (outputShareable == 0) || (shareableWriteIfDiscarded == 1) );
					}

					if ( inputRead || shareableWriteIfDiscarded )
					{
						if ( pBufferSet )
						{
							//Check dma fits into buffer
							WWSJOB_ALWAYS_ASSERT( (mmLengthInQwords * 16) <= NumPagesToNumBytes(pBufferSet->m_bufferSet.m_numPagesPerBuffer) );
						}

						if ( kPrintHints )
						{
							//128-byte alignment of buffers allows for faster DMAs
							if ( !WwsJob_IsAligned( mmAddress, 128 ) )
							{
								JobBasePrintf( "HINT: Buffer at 0x%08X could be 128-byte aligned\n", mmAddress );
							}
						}

						if ( inputGather )
						{
							//Dma list size must be under 16K
							WWSJOB_ALWAYS_ASSERT( mmLengthInQwords <= 16*1024 );

							//:TODO: ERROR: If it's a dmalist, check the elements (and maybe check for self corruption when used in LS?)
							//:TODO: HINT: Could hint about moving the alignment of the input dma list

							//DMA List buffers can't be read-write cached
							WWSJOB_ALWAYS_ASSERT( shareableWriteIfDiscarded == 0 );
						}
					}

					//kReadWriteCached buffers aren't valid on joblists that can go to multiple SPUs
					if ( outputShareable && shareableWriteIfDiscarded )
					{
						WWSJOB_ALWAYS_ASSERT( numSpus == 1 );
						WWSJOB_UNUSED( numSpus );
					}					

					//:TODO: HINT: If it's an input, maybe spot if it looks like a plugin header and warn appropriately?
				}
				break;

			case CommandNum::kRequestDependencyDecrement:
				{
					U32 mmDependencyAddress = pCmd->m_depDecCommand.m_mmDependencyAddress;
					if ( options & kPrintCommands )
					{
						JobBasePrintf( "      0x%016llX  kRequestDependencyDecrement\n", pCmd->m_u64 );
						PrintValU32( "mmDependencyAddress", mmDependencyAddress );
						{
							//:TODO: If this is complied on the SPU then we can't (easily) read the dependency values here
							PrintText( "{" );
							PrintText( "\tAt time of disassembly:" );
							const DependencyCounter* pDependency = (const DependencyCounter*) mmDependencyAddress;
							PrintValU32( "\tmmDependencyAddress->m_counter", pDependency->m_counter );
							PrintValU32( "\tmmDependencyAddress->m_workloadId", pDependency->m_workloadId );
							PrintValU32( "\tmmDependencyAddress->m_readyCount", pDependency->m_readyCount );
							PrintText( "}" );
						}
					}
					WWSJOB_ALWAYS_ASSERT( mmDependencyAddress );
					WWSJOB_ALWAYS_ASSERT( WwsJob_IsAligned( mmDependencyAddress, 4 ) );
					++numDependencyDecrementsRequested;
					WWSJOB_ALWAYS_ASSERT( numDependencyDecrementsRequested <= 4 );	//Each job can only request 4 dependencies to be decremented by the job manager
				}
				break;

			case CommandNum::kRunJob:
				{
					U32 logicalBufferSetNum		= pCmd->m_runJobCommand.m_flags.m_logicalBufferSetNum;
					U32 logicalBufferNum		= pCmd->m_runJobCommand.m_flags.m_logicalBufferNum;
					if ( options & kPrintCommands )
					{
						JobBasePrintf( "      0x%016llX  kRunJob\n", pCmd->m_u64 );
						PrintValU32( "logicalBufferSetNum", logicalBufferSetNum );
						PrintValU32( "logicalBufferNum", logicalBufferNum );
					}
					if ( ( commandIndex & 1 ) == 0 )
					{
						++commandIndex;
					}
					WWSJOB_ALWAYS_ASSERT( options & kErrorCheckJob );	// This instruction is only valid if this *is* load-commands

					//Find the UseBufferCommand that put the job code into LS
					const UseBufferCommand* pUseBufferCommand = NULL;
					for ( U32 i = 0; i < numBuffersInJob; ++i )
					{
						const UseBufferCommand* pCheckUseBufferCommand = &useBufferCommand[i];
						if ( (pCheckUseBufferCommand->m_flags.m_logicalBufferSetNum == logicalBufferSetNum )
							&& ( pCheckUseBufferCommand->m_flags.m_logicalBufferNum == logicalBufferNum ) )
						{
							//This is the UseBufferCommand that transferred the job code to LS
							pUseBufferCommand = pCheckUseBufferCommand;
						}
					}
					WWSJOB_ALWAYS_ASSERT( pUseBufferCommand );	//Check that we did actually transfer the job code into LS

					//Find the bufferset this buffer is in
					const ReserveBufferSetCommand* pCodeBufferSet = NULL;
					for ( U32 buffsetIndex = 0; buffsetIndex < numBufferSetsInJob; ++buffsetIndex )
					{
						const ReserveBufferSetCommand* pCheckBufferSet = &reserveBufferSetCommand[buffsetIndex];
						if ( logicalBufferSetNum == pCheckBufferSet->m_flags.m_logicalBufferSetNum )
						{
							pCodeBufferSet = pCheckBufferSet;
							break;
						}
					}
					WWSJOB_ALWAYS_ASSERT( pCodeBufferSet ); //This is the bufferset that the code is in


					if ( options & kPrintHints )
					{
						if ( pUseBufferCommand->m_flags.m_outputShareable == 0 )
						{
							JobBasePrintf( "HINT: Job code could be marked as shareable to next job\n" );
						}
						WWSJOB_ALWAYS_ASSERT( pUseBufferCommand->m_flags.m_shareableWriteIfDiscarded == 0 );//Job code probably shouldn't be written back
					}

					//:TODO: If this is complied on the SPU then we can't (easily) read the SPU module header here
					const SpuModuleHeader* pCodeBuffer = (const SpuModuleHeader*)(pUseBufferCommand->m_mmAddressInQwords * 16);

					//For the buffer being used for code, check it really has got a module header in it
					WWSJOB_ALWAYS_ASSERT( pCodeBuffer->m_codeMarker == kSpuModuleMarker );

					WWSJOB_ALWAYS_ASSERT( (pCodeBuffer->m_ila0 & 0xFE0001FF) == 0x42000002 );	//ila $2, (val16 << 2) | 0b00
					WWSJOB_ALWAYS_ASSERT( (pCodeBuffer->m_ila1 & 0xFE0001FF) == 0x42000082 );	//ila $2, (val16 << 2) | 0b00
					WWSJOB_ALWAYS_ASSERT( (pCodeBuffer->m_ila2 & 0xFE0001FF) == 0x42000102 );	//ila $2, (val16 << 2) | 0b00
					WWSJOB_ALWAYS_ASSERT( (pCodeBuffer->m_ila3 & 0xFE0001FF) == 0x42000182 );	//ila $2, (val16 << 2) | 0b00

					WWSJOB_ALWAYS_ASSERT( ( (pCodeBuffer->m_ila0 & 0x01FFFE00) != 0 )
										|| ( (pCodeBuffer->m_ila1 & 0x01FFFE00) != 0 )
										|| ( (pCodeBuffer->m_ila2 & 0x01FFFE00) != 0 )
										|| ( (pCodeBuffer->m_ila3 & 0x01FFFE00) != 0 ) );  //All values being zero is a bad sign

					WWSJOB_ALWAYS_ASSERT( WwsJob_IsAligned( pCodeBuffer->m_bssOffset, 16 ) );
					WWSJOB_ALWAYS_ASSERT( WwsJob_IsAligned( pCodeBuffer->m_bssSize, 16 ) );

					//Check that the module+bss does actually fit in the reserved buffer
					WWSJOB_ALWAYS_ASSERT( (pCodeBuffer->m_bssOffset + pCodeBuffer->m_bssSize) <= NumPagesToNumBytes( pCodeBufferSet->m_bufferSet.m_numPagesPerBuffer ) );

					//Check entry point is within file
					WWSJOB_ALWAYS_ASSERT( pCodeBuffer->m_entryOffset < pCodeBuffer->m_bssOffset ); //.text comes before .bss

					if ( pCodeBuffer->m_uploadAddr )	//if it's a fixed addr module
					{
						//check that it's being uploaded to the correct address
						WWSJOB_ALWAYS_ASSERT( pCodeBufferSet->m_bufferSet.m_numBuffers == 1 );	//Check there's only one buffer in the buffer set
						WWSJOB_ALWAYS_ASSERT( NumPagesToNumBytes(pCodeBufferSet->m_bufferSet.m_firstPageNum) == pCodeBuffer->m_uploadAddr );	//And check the address is the correct one
					}

					//Warn on ".data" section existing
					//Or the existence of global constructors/destructors
					if ( options & kPrintWarnings )
					{
						if ( pCodeBuffer->m_dataSize )
							JobBasePrintf( "WARNING: Module at 0x%08X has 0x%X bytes in its \".data\" section\n", (U32)pCodeBuffer, pCodeBuffer->m_dataSize );
						if ( pCodeBuffer->m_ctorDtroListSize )
							JobBasePrintf( "WARNING: Module at 0x%08X has a total of %d global constructors or destructors to call\n", (U32)pCodeBuffer, (pCodeBuffer->m_ctorDtroListSize/4) );
					}
					if ( options & kPrintHints )
					{
						if ( pCodeBuffer->m_bssSize )
							JobBasePrintf( "HINT: Module at 0x%08X has 0x%X bytes in its \".bss\" section\n", (U32)pCodeBuffer, pCodeBuffer->m_bssSize );
					}

					disassemblingInstructions = false;	//Once we've reached a RunJob, the rest are params
				}
				break;

			case CommandNum::kNop:
				if ( options & kPrintCommands )
				{
					JobBasePrintf( "      0x%016llX  kNop\n", pCmd->m_u64 );
				}
				if ( options & kPrintHints )
				{
					JobBasePrintf( "HINT: Nops aren't needed\n" );
				}
				break;

			case CommandNum::kEndCommand:
				if ( options & kPrintCommands )
				{
					JobBasePrintf( "      0x%016llX  kEndCommand\n", pCmd->m_u64 );
				}
				WWSJOB_ALWAYS_ASSERT( ! (options & kErrorCheckJob) );	// This instruction is only valid if this is *not* load-commands
				return;
				//break;

			default:
				if ( options & kPrintCommands )
				{
					JobBasePrintf( "      0x%016llX  **INVALID JOB COMMAND(%d)**\n", pCmd->m_u64, pCmd->m_reserveBufferSetCommand.m_flags.m_commandNum );
				}
				WWSJOB_ALWAYS_ASSERT( false );
				break;
			}
		}
		else
		{
			if ( options & kPrintCommands )
			{
				PrintValU64( "Params", pCmd->m_u64 );
			}
		}
	}

	//:TODO: HINT: Is there any performance advantage to do with loop of reserving buffer sets in incrementing numbers (rather than leaving gaps)?
	//:TODO: HINT: Can we hint about unreserving a buffer set?
}

//--------------------------------------------------------------------------------------------------
