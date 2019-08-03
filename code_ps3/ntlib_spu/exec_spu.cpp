/***************************************************************************************************
*
*	exec_spu.cpp
*
***************************************************************************************************/


#ifndef __SPU__
#	error This file can only be included in an SPU project.
#endif // !__SPU__

#include <cell/spurs/ready_count.h>
#include <cell/atomic.h>
#include <jobapi/jobdefinition.h>
#include <jobapi/jobapi.h>
#include <jobapi/jobspudma.h>
#include <jobapi/jobprintf.h>
#include <jobapi/jobatomic.h>
#include <jobapi/commandlistchecker.h>

#include "ntlib_spu/fixups_spu.h"
#include "ntlib_spu/ntDma.h"
#include "ntlib_spu/syncprims_spu.h"
#include "exec/PPU/spuargument_ps3.h"
#include "exec/PPU/dmabuffer_ps3.h"
#include "exec_spu.h"


void Exec::JobApiSetReadyCount( uint64_t eaSpurs, CellSpursWorkloadId workloadId, uint32_t readyCount )
{
	DisableInterrupts();
	uint8_t atomicBuffer[128] WWSJOB_ALIGNED( 128 );
	int ret = cellSpursReadyCountStore( atomicBuffer, eaSpurs, workloadId, readyCount );
	WWSJOB_ASSERT( CELL_OK == ret );
	WWSJOB_UNUSED( ret );
	EnableInterrupts();
}

void Exec::RunTask( ExecSPUJobAdder* pParams, SPUArgumentList* pArgumentList, uint32_t dependency_counter_to_deprecate/* = 0*/ )
{
	//Read out the params
	uint32_t eaSpuModule						= pParams->m_eaSpuModule;
	uint32_t spuModuleFileSize					= pParams->m_spuModuleFileSize;
	uint32_t spuModuleRequiredBufferSizeInPages	= pParams->m_spuModuleRequiredBufferSizeInPages;
	uint32_t eaJobList							= pParams->m_eaJobList;

	static const uint32_t iMaxCommands				= 32;
	WwsJob_Command commandList[iMaxCommands];
	const uint16_t kNewJobCodeBufferSet				= 0;					//bufferset 0
	const uint16_t kNewJobNumCodeBuffers			= 1;					//contains 1 buffer
	const uint32_t kNewJobCodeBufferSizeInPages		= spuModuleRequiredBufferSizeInPages;
	const uint32_t kNewJobCodeBufferSetBasePageNum	= LsMemoryLimits::kJobAreaBasePageNum;			//The first buffer of this bufferset starts at this address

	const uint32_t kLogicalBuffer0 = 0;

	uint16_t	iCommandNum = 0;

	//Create the new job
	commandList[iCommandNum++].ReserveBufferSet( kNewJobCodeBufferSet, kNewJobNumCodeBuffers, kNewJobCodeBufferSetBasePageNum, kNewJobCodeBufferSizeInPages );
	commandList[iCommandNum++].UseInputBuffer( kNewJobCodeBufferSet, kLogicalBuffer0, eaSpuModule, spuModuleFileSize, WwsJob_Command::kReadOnlyCached );

	// Keep track of the buffersets we reserve.
	uint32_t reserved_buffer_sets = 1 << kNewJobCodeBufferSet;

	// Setup the data buffers for our use.
	uint32_t DataBufferSet = 1;
	const uint32_t DataPageBaseIndex = kNewJobCodeBufferSetBasePageNum + spuModuleRequiredBufferSizeInPages;
	ntError( DataPageBaseIndex < 250 );	// This job won't fit in local store!

	uint32_t CurrentDataPageBase = DataPageBaseIndex;

	// Run through our requested parameters and setup input buffers for each of our input or input+output arguments.
	for ( int16_t i=0;i<SPUArgumentList::MaxNumArguments;i++ )
	{
		// Grab the ith argument.
		SPUArgument *arg = pArgumentList->Get( i );

		// Arguments must be allocated sequentially, so a NULL or Invalid
		// argument indicates there are no remaining arguments.
		if ( arg == NULL || arg->GetType() == SPUArgument::Type_Invalid )
		{
			break;
		}

		if ( arg->GetType() == SPUArgument::Type_DMABuffer )
		{
			ntError( arg->GetBuffer()->GetRequiredNumPages() * 1024 >= arg->GetBuffer()->GetSize() );

			// Reserve the next bufferset - one buffer of the correct length for this dma argument.
			ntError_p( DataBufferSet < 32, ("Can't have more than 32 buffersets") );
			commandList[iCommandNum++].ReserveBufferSet( DataBufferSet, 1, CurrentDataPageBase, arg->GetBuffer()->GetRequiredNumPages() ); 

			uint32_t addrEA; 
			if( arg->GetBuffer()->IsAddrEA() )
			{
				addrEA = arg->GetBuffer()->GetEA();
			} else
			{
				addrEA = g_DMAEffectiveAddresses[ i ];
			}

			if ( arg->GetMode() == SPUArgument::Mode_OutputOnly )
			{
				// Setup this dma buffer as an uninitialised output buffer.
				commandList[iCommandNum++].UseUninitializedBuffer( DataBufferSet, 0, addrEA , arg->GetBuffer()->GetSize(), WwsJob_Command::kNonCached );
			}
			else
			{
				// Setup this dma buffer as an input. Setup as non-cached as now.
				commandList[iCommandNum++].UseInputBuffer( DataBufferSet, 0, addrEA, arg->GetBuffer()->GetSize(), WwsJob_Command::kNonCached );
			}

			// Store which bufferset this is with the argument.
			arg->m_BufferSet = DataBufferSet;
			ntError( arg->m_BufferSet != SPUArgument::InvalidBufferSet );

			// Add this bufferset to our list of sets we've reserved.
			reserved_buffer_sets |= ( 1 << DataBufferSet );

			// Increment to the next bufferset.
			DataBufferSet++;
			CurrentDataPageBase += arg->GetBuffer()->GetRequiredNumPages();
			ntError( CurrentDataPageBase < 250 );	// This job won't fit in local store!
		}
	}
	// we have updated the bufferset numbers so now its safe to DMA out to main memory
	// okay we need to reserve an main mem Exec argument list and kick off a dma to put this into it
	int32_t iArgListIndex = AtomicIncrement( pParams->m_eaCurrentArgumentListIndex );
	ntError_p( iArgListIndex >= 0 && iArgListIndex < 512, ("We've run out of job space. Increase MaxNumJobs and recompile.") );
	uint32_t param_space = pParams->m_eaArgumentListSpace + iArgListIndex * sizeof(SPUArgumentList);
	ntDMA::Params arguDmaParams;
	ntDMA_ID arguId = ntDMA::GetFreshID();
	arguDmaParams.Init32( pArgumentList, param_space, sizeof(SPUArgumentList), arguId );
	ntDMA::DmaToPPU( arguDmaParams );

	// Allocate an input bufferset for the parameters.
	const int32_t pages_for_parameters = ROUND_POW2( sizeof( SPUArgumentList ), 1024 ) >> 10;
	commandList[iCommandNum++].ReserveBufferSet( 14, 1, CurrentDataPageBase, pages_for_parameters );
	commandList[iCommandNum++].UseInputBuffer( 14, 0, param_space, sizeof( SPUArgumentList ), WwsJob_Command::kNonCached );
	CurrentDataPageBase += pages_for_parameters;
	reserved_buffer_sets |= ( 1 << 14 );
	ntError( CurrentDataPageBase < 250 );	// This job won't fit in local store!

	const int32_t num_pages_remaining = LsMemoryLimits::kJobAreaEndPageNum - CurrentDataPageBase;
	ntError( num_pages_remaining >= 0 );
	commandList[iCommandNum++].ReserveBufferSet( 15, 1, CurrentDataPageBase, num_pages_remaining );
	commandList[iCommandNum++].UseUninitializedBuffer( 15, 0 );
	reserved_buffer_sets |= ( 1 << 15 );

	// Decrement the dependency counter if we have one...
	if ( dependency_counter_to_deprecate != 0 )
	{
		commandList[ iCommandNum++ ].RequestDependencyDecrement( dependency_counter_to_deprecate );
	}

	// Free up the buffer sets.
	commandList[iCommandNum++].UnreserveBufferSets( reserved_buffer_sets );
	// add the run command
	commandList[iCommandNum++].RunJob( kNewJobCodeBufferSet, kLogicalBuffer0 );

	// align command buffer
	if( iCommandNum & 0x1 )
	{
		commandList[iCommandNum++].AddNopCommand();
	}

	// yer this assert is too late... and should be done with every increment but your hosed anyway by this point...
	ntAssert( iCommandNum < iMaxCommands );

	
	
	
	
	
//	CheckCommandList( &( commandList[ 0 ] ), iCommandNum, kPrintCommands|kPrintWarnings|kPrintHints|kErrorCheckJob, pParams->m_NumWWSJobManagerSPUs );

	
	
	
	
	

	static const uint32_t MaxJobBufferOutputSize	= (sizeof(WwsJob_Command) * iMaxCommands) ;
	const uint32_t newJobBufferFilledSize			= Util::Align( (sizeof(WwsJob_Command) * iCommandNum), 16 );
	ntAssert( newJobBufferFilledSize <= MaxJobBufferOutputSize );

	//Send the buffer of commands for the new job out to main memory

	// allocate space in the shared pool
	int32_t commandListIndex = AtomicIncrement( pParams->m_eaCurrentSPUCommandListIndex, MaxJobBufferOutputSize );
	uint32_t eaNewJobBuffer	= pParams->m_eaSPUCommandListBuffer + commandListIndex;

	ntDMA::Params commandDmaParams;
	ntDMA_ID commandId = ntDMA::GetFreshID();
	commandDmaParams.Init32( commandList, eaNewJobBuffer, newJobBufferFilledSize, commandId );
	ntDMA::DmaToPPU( commandDmaParams );

	ntDMA::StallForCompletion( arguId );
	ntDMA::StallForCompletion( commandId );
	ntDMA::FreeID( arguId );
	ntDMA::FreeID( commandId );

	//And add the job to the joblist
	JobHeader jobHeader;
	jobHeader.m_mmaLoadCommands			= (U32) eaNewJobBuffer;
	jobHeader.m_loadCommandsSize		= newJobBufferFilledSize;
	jobHeader.m_enableBreakpoint		= 0;
	jobHeader.m_jobHeaderCommand		= JobHeaderCommand::kJobExists;

	WwsJob_JobApiAddJobToJobList( eaJobList, jobHeader );
}

void Exec::AddBarrierJob( ExecSPUJobAdder* pParams, uint32_t counter )
{
	ntError( pParams != NULL );
	ntError( counter != NULL );

	JobHeader jobHeader;
	jobHeader.m_mmaLoadCommands			= counter;
	jobHeader.m_loadCommandsSize		= 0;
	jobHeader.m_enableBreakpoint		= 0;
	jobHeader.m_jobHeaderCommand		= JobHeaderCommand::kGeneralBarrier;





//	CheckJob( jobHeader, kPrintCommands|kPrintWarnings|kPrintHints|kErrorCheckJob, pParams->m_NumWWSJobManagerSPUs );





	WwsJob_JobApiAddJobToJobList( pParams->m_eaJobList, jobHeader );
}

void Exec::InitDependency( uint32_t dependency_counter_ea, uint16_t count, const ExecSPUJobAdder *jobAdder )
{
	static_assert_in_class( sizeof( DependencyCounter ) == sizeof( uint32_t ), Must_be_same_size );
	ntError( sizeof( DependencyCounter ) == sizeof( uint32_t ) );

	union
	{
		uint32_t			m_32;
		DependencyCounter	m_D;

	} dep_counter;
	dep_counter.m_D.m_counter = count;
	dep_counter.m_D.m_readyCount = jobAdder->m_NumWWSJobManagerSPUs;
	dep_counter.m_D.m_workloadId = jobAdder->m_workloadId;

	JobAtomicStore32( dependency_counter_ea, dep_counter.m_32 );
}


