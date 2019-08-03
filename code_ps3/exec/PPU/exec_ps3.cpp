//----------------------------------------------------------------------------------------
//! 
//! \filename exec\ppu\exec_ps3.h
//! 
//----------------------------------------------------------------------------------------

#include <sys/spu_initialize.h>
#include <sys/ppu_thread.h>

#include <cell/spurs/types.h>
#include <cell/spurs/control.h>

#include "jobapi/joblist.h"
#include "jobapi/spumodule.h"
#include "jobapi/commandlistbuilder.h"
#include "jobapi/auditmanager.h"
#include "jobapi/eventhandler.h"

#include "exec/ppu/spuprogram_ps3.h"
#include "exec/ppu/sputask_ps3.h"
#include "exec/ppu/spuargument_ps3.h"
#include "exec/ppu/dmabuffer_ps3.h"
#include "exec/execspujobadder_ps3.h"
#include "exec_ps3.h"
#include "core/gatso.h"

//#define EXEC_DEBUG_MODE

//----------------------------------------------------------------------------------------
// Static members of exec
//----------------------------------------------------------------------------------------
unsigned int Exec::s_iReservedRawSPUs = 0;
unsigned int Exec::s_iReservedSPUThreads = 0;
#if defined(_DEBUG )
	const char*	Exec::s_aReservedSPUNames[ Exec::TOTAL_NUMBER_OF_SPU ];
#endif


//----------------------------------------------------------------------------------------
// local namespace for really private stuff
//----------------------------------------------------------------------------------------
namespace
{
#if defined( _DEBUG )
	//! Debug text for the SPU Thread reserved
	const char* f_SPUThreadName = "Lv2 OS SPU Thread reservation";
#endif
	void SPUPrintfCallback( U32 spuNum, const char* pPrintBuffer )
	{
		ntPrintf( "SPU %i : %s\n", spuNum, pPrintBuffer );
	}

	//
	//	WWS Job Manager Data.
	//

	static CellSpurs *		sSpurs;

	static EventHandler *	sEventHandler;

	static const int32_t	MaxNumJobs = 1280;	// Guess...
	static const int32_t	JobListBufferSize = ROUND_POW2( MaxNumJobs * sizeof( JobHeader ) + sizeof( JobListHeaderCore ), 128 );
	static int8_t			sJobListBuffer[ JobListBufferSize ] __attribute__ ((aligned( 128 )));
	MultiThreadSafeJobList *sJobList;

	static const int32_t	CommandListBufferSize = ROUND_POW2( MaxNumJobs * 0x100, 128 );
	static int8_t			sCommandListBuffer[ CommandListBufferSize ] __attribute__ ((aligned( 128 )));
	CommandListBuilder *	sCommandListBuilder;

	static SPUArgumentList	sArgumentListSpace[ MaxNumJobs ] __attribute__ ((aligned( 128 )));
	
	// align to cache line cos of atomic update on Cell uses a entire cache line of locking and make it volatile to ensure
	// no register caching... lets all sing the multi-threading is fun song...
	static volatile int32_t	sCurrentArgumentListIndex  __attribute__ ((aligned( 128 ))) = 0;


	// each job take 8*32 = 256 .. so we reserve enough for MaxNumJobs jobs (128K) we should really share with PPU command list but 
	// need a new commandlistbuilder for that...
	static const int32_t	MaxSPUCommands = 32;
	static const int32_t	SPUCommandListBufferSize = (MaxSPUCommands * sizeof(WwsJob_Command)) * MaxNumJobs;
	static uint8_t			sSPUCommandListBuffer[ SPUCommandListBufferSize ] __attribute__(( aligned( 128 )));

	static volatile int32_t	sCurrentSPUCommandListIndex __attribute__ ((aligned( 128 ))) = 0;

	static uint32_t			sNumWWSJobManagerSPUs = 0;	// Calculated in Init()...

	// removed as not safe over level restart
/*
	// ppu callbacks 
	static const uint32_t	sMaxPPUCallbacks = 2;

	static EventHandler		sPPUCallbacks[ sMaxPPUCallbacks ];
*/
	//
	//	PPU Threading data.
	//

	//! Max number of Function Tasks per frame
	static const unsigned int MAXIMUM_NUMBER_OF_FUNCTION_TASKS = 128 * 4;

	struct AsyncTask
	{
		Exec::FunctionTask pTask;
		void* pParam0;
		void* pParam1;
	};
	AsyncTask s_FunctionTasks[ MAXIMUM_NUMBER_OF_FUNCTION_TASKS ];

	//! 0 if the second hardware thread isn't running
	static volatile uint32_t	s_iSecondThreadRunning;

	//! 0 if the second hardware thread is ideling
	static volatile uint32_t	s_iSecondThreadIsIdle;

	// these must be only touched by thread-safe access, they make a very 
	// simple FIFO
	//! the current put index (where a new task is added)
	static volatile uint32_t	s_iPutFunctionTaskIndex;

	//! the current get index (where a task will be pulled from and run)
	static volatile uint32_t	s_iGetFunctionTaskIndex;

	static WaitableEvent*		s_pSecondThreadEvent;

	static sys_ppu_thread_t 	s_secondHardwareThread; //!< thread handle of the second core thread

	static CriticalSection		s_FunctionTaskGetCrit;

	//
	//	Debug functions.
	//
#	if defined( EXEC_DEBUG_MODE )
#		define	ExecMessage( msg )	ntPrintf( "[Exec]:  " ); ntPrintf msg
#	else
#		define 	ExecMessage( msg ) do {} while ( false )
#	endif
}

MultiThreadSafeJobList *Exec::GetWWSJobList()
{
	return sJobList;
}

CellSpurs *Exec::GetSpurs()
{
	return sSpurs;
}

//----------------------------------------------------------------------------------------
//! 
//! This should be called before Exec::Init, it tells exec to leave 1 SPU for the use of
//! named system. 
//! The more reserverations the slower exec will be.
//! 
//! \param pName Description of what the reservation for debug purposes
//! \note pName isn't copied so should be valid for the lifetime of the program
//!
//----------------------------------------------------------------------------------------
void Exec::ReserveRawSPU( const char *pName )
{
	ntAssert_p(	((s_iReservedRawSPUs + s_iReservedSPUThreads + 1) <= TOTAL_NUMBER_OF_SPU), 
				("We have reserved all SPUs, no space for %s left\n", pName ) );
	ntPrintf( Debug::DCU_EXEC, "Reserving an SPU for %s\n", pName );

#if defined( _DEBUG )
	// store the name to thi reservation
	s_aReservedSPUNames[s_iReservedRawSPUs] = pName;
#endif
	// allocate it
	s_iReservedRawSPUs++;
}

//----------------------------------------------------------------------------------------
//! 
//! This should be called before Exec::Init, it tells exec to leave N SPU for the use of
//! OS SPU Thread system.
//! The more reserverations the slower exec will be.
//! 
//! \param iNumSPUs Number of SPUs to reserver for the OS
//! \note The debug reservation name is "Lv2 OS SPU Thread reservation"
//!
//----------------------------------------------------------------------------------------
void Exec::ReserveSPUThreads( const int iNumSPUs )
{
	// allow 0 as a valid call to allow quick mods from the user code
	if( iNumSPUs == 0 )
		return;

	ntAssert_p(	(s_iReservedRawSPUs + s_iReservedSPUThreads + iNumSPUs) <= TOTAL_NUMBER_OF_SPU,
				("We have reserved all SPUs, no space for %d SPU Threads\n", iNumSPUs ) );
	ntPrintf( Debug::DCU_EXEC, "Reserving %d SPU for Lv2 OS SPU Thread system\n", iNumSPUs );

#if defined( _DEBUG )
	// store the name to these reservation
	for( int i=0;i < iNumSPUs;i++)
	{
		s_aReservedSPUNames[s_iReservedRawSPUs+i] = f_SPUThreadName;
	}
#endif
	// allocate them
	s_iReservedSPUThreads+=iNumSPUs;
}

//----------------------------------------------------------------------------------------
//!
//! Set up the spu's and init the task system. 
//! Should occur as early as possible, but does require a working filesystem
//!
//----------------------------------------------------------------------------------------
void Exec::Init( void )
{
	ntError_p( s_iReservedSPUThreads + s_iReservedRawSPUs < TOTAL_NUMBER_OF_SPU, ("You are attempting to reserve more SPUs than we actually have! Exec requires at least one for the SPURS job manager.") );
	sNumWWSJobManagerSPUs =	TOTAL_NUMBER_OF_SPU - s_iReservedRawSPUs - s_iReservedSPUThreads;

	// Initialise SPU management.
	int32_t result = sys_spu_initialize( TOTAL_NUMBER_OF_SPU, 0 );
	ntError_p(result == SUCCEEDED, ("sys_spu_initialize() failed!"));

	int32_t spuThreadGroupPriority	= 200;

	// E We need to figure out the priority for the SPURS handler thread.
	// E We'll do this by getting the current thread priority, and making
	// E the SPURS handler thread slightly higher priority.
	sys_ppu_thread_t idCurrentThread;
	int iCurrentThreadPriority;

	int iReturn=sys_ppu_thread_get_id(&idCurrentThread);
	ntError_p(iReturn==CELL_OK, ("Cannot get current thread ID (%d)\n", iReturn));

	iReturn=sys_ppu_thread_get_priority(idCurrentThread,
										&iCurrentThreadPriority);
	ntError_p(iReturn==CELL_OK, ("Cannot get current thread priority (%d)\n", iReturn));

	sSpurs = (CellSpurs *)NT_MEMALIGN_CHUNK( Mem::MC_MISC, sizeof( CellSpurs ), 128 );
	result = cellSpursInitialize(	sSpurs, sNumWWSJobManagerSPUs, spuThreadGroupPriority,
									iCurrentThreadPriority-1, false );

	char text[ 1024 ];
	sprintf( text, "spurs = 0x%08X\n", (uintptr_t)sSpurs );
	Debug::AlwaysOutputString( text );

	ntError_p( result == CELL_OK, ("cellSpursInitialize failed (spurs=0x%p)", &sSpurs) );

	sEventHandler = NT_NEW EventHandler( sSpurs, 200 );
	sEventHandler->SetPrintfCallback( SPUPrintfCallback );
	sJobList = NT_NEW MultiThreadSafeJobList( sJobListBuffer, sizeof( sJobListBuffer ) );

	uint8_t standardWorkPriorities[ 8 ] = { 8, 8, 8, 8, 8, 8, 0, 0 };
	sJobList->AttachToSpurs( sSpurs, &( standardWorkPriorities[ 0 ] ), sNumWWSJobManagerSPUs );

	sCommandListBuilder = NT_NEW CommandListBuilder( sCommandListBuffer, sizeof( sCommandListBuffer ) );

	//---- async function bit of Exec
	s_pSecondThreadEvent = NT_NEW WaitableEvent();

	AtomicSet( &s_iSecondThreadRunning, 0 );
	AtomicSet( &s_iSecondThreadIsIdle, 0 );

	// start a second thread to run on the second core
	int res = sys_ppu_thread_create(
			&s_secondHardwareThread, 
			&SecondHardwareThread,
			0,
			1000,
			1024 * 32,
			0,
			"Second Hard Thread" 
		);
	ntError_p( res == CELL_OK, ("Could create second PPU thread" ) );
	UNUSED( res );

	FrameReset();
}

//----------------------------------------------------------------------------------------
//!
//! Free up the memory and shut everything down
//!
//----------------------------------------------------------------------------------------
void Exec::Shutdown( void )
{
	sJobList->Shutdown();
	sEventHandler->RemoveEventHandler( sSpurs );

	// Exit the second thread.
	AtomicSet( &s_iSecondThreadRunning, 0 );
	s_pSecondThreadEvent->Wake();				// Must wake the thread in order for it to exit!
	while ( s_iSecondThreadRunning )
	{
		// Wait until the thread has quit.
	}

	NT_DELETE( s_pSecondThreadEvent );
	s_pSecondThreadEvent = NULL;

	NT_DELETE( sCommandListBuilder );
	sCommandListBuilder = NULL;

	NT_DELETE( sJobList );
	sJobList = NULL;

	Debug::AlwaysOutputString( "Attempting to shutdown SPURS.\n" );

	int32_t ret_val;
	int32_t spurs_finalise_count = 0;
	do
	{
		ret_val = cellSpursFinalize( sSpurs );
	}
	while ( ret_val != CELL_OK && ++spurs_finalise_count < 200 );

	NT_FREE_CHUNK( Mem::MC_MISC, (uintptr_t)sSpurs );
	sSpurs = NULL;

	if ( spurs_finalise_count >= 200 )
	{
		Debug::AlwaysOutputString( "GGAAAAAHHHHHHH - cellSpursFinalize not working!\n" );
	}

	ntError( ret_val == CELL_OK );
	UNUSED( ret_val );

	NT_DELETE( sEventHandler );
	sEventHandler = NULL;
}

//----------------------------------------------------------------------------------------
//!
//! Creates and runs a new ppu-thread.
//!
//----------------------------------------------------------------------------------------
void Exec::CreatePPUThread( PPUThreadFunction func, int64_t thread_arg, int32_t priority, const char *thread_name )
{
	sys_ppu_thread_t new_thread;
	int32_t res = sys_ppu_thread_create( &new_thread, func, thread_arg, priority, 1024 * 16, 0, thread_name );
	ntError_p( res == CELL_OK, ("Couldn't create thread %s", thread_name) );
	UNUSED(res);
}

//----------------------------------------------------------------------------------------
//!
//! Frame Reset
//!
//----------------------------------------------------------------------------------------
void Exec::FrameReset( void )
{
#if defined( EXEC_DEBUG_MODE )
	ntPrintf( Debug::DCU_EXEC, "---------------- FRAME START -----------------------\n" );
#endif

	sJobList->ResetList();
	sCommandListBuilder->ResetCommandListBuilder();

	AtomicSet( &sCurrentArgumentListIndex, 0 );
	AtomicSet( &sCurrentSPUCommandListIndex, 0 );

	// reset the function task FIFO
	s_FunctionTaskGetCrit.Enter();
	AtomicSet( &s_iPutFunctionTaskIndex, 0 );
	AtomicSet( &s_iGetFunctionTaskIndex, 0 );
	s_FunctionTaskGetCrit.Leave();

	s_pSecondThreadEvent->Wake();
}

//----------------------------------------------------------------------------------------
//!
//! Frame End
//!
//----------------------------------------------------------------------------------------
void Exec::FrameEnd( void )
{
	WaitUntilFunctionTasksComplete();
	CGatso::Start( "Exec::FrameEnd Stall" );
	sJobList->WaitForJobListEnd();
	CGatso::Stop( "Exec::FrameEnd Stall" );

#if defined( EXEC_DEBUG_MODE )
	ntPrintf( Debug::DCU_EXEC, "------------------ FRAME END -----------------------\n" );
#endif
}

//----------------------------------------------------------------------------------------
//!
//! Add a task to the normal task list
//!
//----------------------------------------------------------------------------------------
void Exec::RunTask( SPUTask *task )
{
	const SPUProgram *program( task->GetProgram() );
	ExecMessage( ("--------------------------------------------------\n") );
	ExecMessage( ("RunTask->%s\n", *( program->GetName() ) ) );

	//
	// ---------- Load stage -------------
	//


	static_assert_in_class( ( sizeof( SPUArgumentList ) & ( ALIGNOF( WwsJob_Command ) - 1 ) ) == 0, SPUArgumentList_must_have_at_least_the_same_alignment_as_WwsJob_Command );

	// argument list are shared by ppu and spu so now have to be atomically updated...

	// increment first to ensure this argument list is ours to abuse as we see fit
	int32_t iArgListIndex = AtomicIncrement( &sCurrentArgumentListIndex );
	ntError_p( iArgListIndex >= 0 && iArgListIndex < MaxNumJobs, ("We've run out of job space. Increase MaxNumJobs and recompile.") );
	SPUArgumentList *param_space = &( sArgumentListSpace[ iArgListIndex ] );

	// Start the job data.
	sCommandListBuilder->InitializeJob();

	// Reserve the code buffer for our use.
	const uint32_t CodeBufferSet = 0;												// We always use bufferset 0 for our code.
	const uint32_t NumCodeBuffers = 1;												// No need to have more than one code buffer.
	const uint32_t CodeBufferSetBasePageNum = LsMemoryLimits::kJobAreaBasePageNum;	// Start the buffers from here.
	sCommandListBuilder->ReserveBufferSet(	CodeBufferSet, NumCodeBuffers, CodeBufferSetBasePageNum,
											program->GetModule().GetRequiredBufferSizeInPages() );

	// Setup the code as an input buffer to be DMAd into LS.
	sCommandListBuilder->UseInputBuffer( CodeBufferSet, 0, program->GetModule(), WwsJob_Command::kReadOnlyCached );

	// Keep track of the buffersets we reserve.
	uint32_t reserved_buffer_sets = 1 << CodeBufferSet;

	// Setup the data buffers for our use.
	uint32_t DataBufferSet = 1;
	const uint32_t DataPageBaseIndex = CodeBufferSetBasePageNum + program->GetModule().GetRequiredBufferSizeInPages();
	uint32_t CurrentDataPageBase = DataPageBaseIndex;

	// Run through our requested parameters and setup input buffers for each of our input or input+output arguments.
	for ( int32_t i=0;i<SPUArgumentList::MaxNumArguments;i++ )
	{
		// Grab the ith argument.
		SPUArgument *arg = task->GetArgument( i );

		// Arguments must be allocated sequentially, so a NULL or Invalid
		// argument indicates there are no remaining arguments.
		if ( arg == NULL || arg->GetType() == SPUArgument::Type_Invalid )
		{
			break;
		}

		if ( arg->GetType() == SPUArgument::Type_DMABuffer )
		{
			ntError( arg->GetBuffer()->GetRequiredNumPages() * 1024 >= arg->GetBuffer()->GetSize() );
#			if defined( EXEC_DEBUG_MODE )
				ntPrintf( "BufferSet %i, PageBase = %i, NumPages = %i, ActualSize = %i.\n", DataBufferSet, CurrentDataPageBase, arg->GetBuffer()->GetRequiredNumPages(), arg->GetBuffer()->GetSize() );
#			endif

			// Reserve the next bufferset - one buffer of the correct length for this dma argument.
			ntError_p( DataBufferSet < 32, ("Can't have more than 32 buffersets") );
			sCommandListBuilder->ReserveBufferSet( DataBufferSet, 1, CurrentDataPageBase, arg->GetBuffer()->GetRequiredNumPages() ); 

			if ( arg->GetMode() == SPUArgument::Mode_OutputOnly )
			{
				// Setup this dma buffer as an uninitialised output buffer.
				sCommandListBuilder->UseUninitializedBuffer( DataBufferSet, 0, arg->GetBuffer()->Get(), arg->GetBuffer()->GetSize(), WwsJob_Command::kNonCached );
			}
			else
			{
				// Setup this dma buffer as an input. Setup as non-cached as now.
				sCommandListBuilder->UseInputBuffer( DataBufferSet, 0, arg->GetBuffer()->Get(), arg->GetBuffer()->GetSize(), WwsJob_Command::kNonCached );
			}

			// Store which bufferset this is with the argument.
			arg->m_BufferSet = DataBufferSet;
			ntError( arg->m_BufferSet != SPUArgument::InvalidBufferSet );

			// Add this bufferset to our list of sets we've reserved.
			reserved_buffer_sets |= ( 1 << DataBufferSet );

			// Increment to the next bufferset.
			DataBufferSet++;
			CurrentDataPageBase += arg->GetBuffer()->GetRequiredNumPages();
		}
	}

	ntError_p( DataBufferSet < 14, ("There're only 16 buffersets, 14 and 15 are reserved by Exec for use with parameter passing and the Allocate function on SPUs.") );

	// Allocate an input bufferset for the parameters.
	const int32_t pages_for_parameters = ROUND_POW2( sizeof( SPUArgumentList ), 1024 ) >> 10;
#	if defined( EXEC_DEBUG_MODE )
		ntPrintf( "ParameterBufferSet is set 14, PageBase = %i, NumPages = %i, ActualSize = %i.\n", CurrentDataPageBase, pages_for_parameters, sizeof( SPUArgumentList ) );
#	endif
	*param_space = *( task->GetArgumentList() );
	sCommandListBuilder->ReserveBufferSet( 14, 1, CurrentDataPageBase, pages_for_parameters );
	sCommandListBuilder->UseInputBuffer( 14, 0, param_space, sizeof( SPUArgumentList ), WwsJob_Command::kNonCached );
	CurrentDataPageBase += pages_for_parameters;
	reserved_buffer_sets |= ( 1 << 14 );

	// Allocate our temporary scratch buffer for use with the Allocate() function on SPUs.
	const int32_t num_pages_remaining = LsMemoryLimits::kJobAreaEndPageNum - CurrentDataPageBase;
#	if defined( EXEC_DEBUG_MODE )
		ntPrintf( "AllcoateBuffer is set 15, PageBase = %i, NumPages = %i, ActualSize = %i.\n", CurrentDataPageBase, num_pages_remaining, num_pages_remaining << 10 );
#	endif
	sCommandListBuilder->ReserveBufferSet( 15, 1, CurrentDataPageBase, num_pages_remaining );
	sCommandListBuilder->UseUninitializedBuffer( 15, 0 );
	reserved_buffer_sets |= ( 1 << 15 );

	if( task->GetDependency() )
	{
		sCommandListBuilder->RequestDependencyDecrement( task->GetDependency() );
	}

	// Free up the buffer sets.
	sCommandListBuilder->UnreserveBufferSets( reserved_buffer_sets );

	//
	// ---------- Execute stage -------------
	//

	// Run the job.
	sCommandListBuilder->RunJob( CodeBufferSet, 0 );

	// Add the parameters/arguments.
//	sCommandListBuilder->AddParams( task->GetArgumentList(), sizeof( SPUArgumentList ) );

	// Finish the job.
	JobHeader job = sCommandListBuilder->FinalizeJob();

	// Add the job header to the job list.
	JobListMarker marker_after_job = sJobList->AddJob( job );
	task->SetMarker( marker_after_job );

	// Tell SPURS that we have work to do and we should use as many SPUs as we can.
	sJobList->SetReadyCount( sNumWWSJobManagerSPUs );

	ExecMessage( ("--------------------------------------------------\n") );
}

void Exec::InitDependency( DependencyCounter* pCounter, uint32_t iCount )
{
	pCounter->m_readyCount = sNumWWSJobManagerSPUs;
	pCounter->m_counter = iCount;
	pCounter->m_workloadId = sJobList->GetWorkloadId();
}

void Exec::AddBarrierJob( DependencyCounter* pCounter )
{
	sJobList->AddGeneralBarrier( pCounter );
}

// removed as not safe over level restart
/*
uint32_t Exec::AddPPUCallbackHandler( Exec::PPUCallback func, uint32_t iQueueDepth )
{
	for(uint32_t i=0;i < sMaxPPUCallbacks;i++) 
	{
		if( sPPUCallbacks[i].GetPortNum() == 0xFF )
		{

			sPPUCallbacks[i].InstallEventHandler( &sSpurs, GetThisPPUThreadPriority() - 1, 0, func );
			return sPPUCallbacks[i].GetPortNum();
		}
	}
	return 0xFFFFFFFF;
}
void Exec::RemovePPUCallbackHandler( uint32_t portnum )
{
	ntAssert( portnum != 0xFFFFFFFF );

	for(uint32_t i=0;i < sMaxPPUCallbacks;i++) 
	{
		if( sPPUCallbacks[i].GetPortNum() == portnum )
		{
			sPPUCallbacks[i].RemoveEventHandler( &sSpurs );
			return;
		}
	}

	ntAssert_p( false, ("Invalid portnum passed to RemovePPUCallbackHandler") );
}
*/
void Exec::PrepareSPUJobAdder( const SPUTask* pTask, ExecSPUJobAdder* pAdder )
{
	const SPUProgram *program( pTask->GetProgram() );

	pAdder->m_eaSpuModule							= (uint32_t) program->GetModule().GetAddress();
	pAdder->m_spuModuleFileSize						= program->GetModule().GetFileSize();
	pAdder->m_spuModuleRequiredBufferSizeInPages	= program->GetModule().GetRequiredBufferSizeInPages();
	pAdder->m_eaJobList								= (uint32_t) sJobList->GetWQAddr();
	pAdder->m_eaSpurs								= (uint32_t) &sSpurs;
	pAdder->m_workloadId							= sJobList->GetWorkloadId();
	pAdder->m_NumWWSJobManagerSPUs					= sNumWWSJobManagerSPUs;

	pAdder->m_eaArgumentListSpace					= (uint32_t) sArgumentListSpace;
	pAdder->m_eaSPUCommandListBuffer				= (uint32_t) sSPUCommandListBuffer;
	pAdder->m_eaCurrentArgumentListIndex			= (uint32_t) &sCurrentArgumentListIndex;
	pAdder->m_eaCurrentSPUCommandListIndex			= (uint32_t) &sCurrentSPUCommandListIndex;
}


//----------------------------------------------------------------------------------------
//!
//! SecondHardwareThread... This is designed to run on a second core (or SMT) and
//! use the games relatively simple co-operative thread system
//!
//----------------------------------------------------------------------------------------
void Exec::SecondHardwareThread( uint64_t iThreadParam )
{
	UNUSED( iThreadParam );
	AtomicSet( &s_iSecondThreadRunning, 1 );

	while( s_iSecondThreadRunning )
	{
		while( ExecNextFunctionTask() )
		{
			// do nothing
		}

		// wait safely
		if( s_iSecondThreadRunning )
		{
			AtomicSet( &s_iSecondThreadIsIdle, 1 );
			s_pSecondThreadEvent->Wait();
			AtomicSet( &s_iSecondThreadIsIdle, 0 );
		}
	}

	sys_ppu_thread_exit( 0 );
}

void Exec::RunAsyncFunction( FunctionTask pTask, void* pParam0, void* pParam1 )
{
	s_FunctionTaskGetCrit.Enter();
	
	ntAssert_p( s_iPutFunctionTaskIndex < MAXIMUM_NUMBER_OF_FUNCTION_TASKS-1, ("Out of Function tasks") );
	s_FunctionTasks[ s_iPutFunctionTaskIndex ].pTask = pTask;
	s_FunctionTasks[ s_iPutFunctionTaskIndex ].pParam0 = pParam0;
	s_FunctionTasks[ s_iPutFunctionTaskIndex++ ].pParam1 = pParam1;

	s_FunctionTaskGetCrit.Leave();

	s_pSecondThreadEvent->Wake();

}

bool Exec::ExecNextFunctionTask()
{
	bool bRet;
	s_FunctionTaskGetCrit.Enter();
	if( s_iGetFunctionTaskIndex < s_iPutFunctionTaskIndex )
	{
		uint32_t iTaskIndex = AtomicIncrement( &s_iGetFunctionTaskIndex );
		s_FunctionTaskGetCrit.Leave();
		
		s_FunctionTasks[ iTaskIndex ].pTask( s_FunctionTasks[ iTaskIndex ].pParam0,  s_FunctionTasks[ iTaskIndex ].pParam1 );
		bRet = true;
	}
	else
	{	
		s_FunctionTaskGetCrit.Leave();
		bRet = false;
	}

	return bRet;
}

void Exec::WaitUntilFunctionTasksComplete()
{
	s_FunctionTaskGetCrit.Enter();
	uint32_t iHighTask = s_iPutFunctionTaskIndex;
	while( s_iGetFunctionTaskIndex < iHighTask )
	{
		uint32_t iTaskIndex = AtomicIncrement( &s_iGetFunctionTaskIndex );
		s_FunctionTaskGetCrit.Leave();
		
		s_FunctionTasks[ iTaskIndex ].pTask( s_FunctionTasks[ iTaskIndex ].pParam0, s_FunctionTasks[ iTaskIndex ].pParam1 );
		s_FunctionTaskGetCrit.Enter();
	}
	s_FunctionTaskGetCrit.Leave();

	// busy wait...
	while( !s_iSecondThreadIsIdle )
	{
	}
}

//----------------------------------------------------------------------------------------
//!
//! Returns number of SPUs controled by Spurs
//!
//----------------------------------------------------------------------------------------
unsigned int Exec::NumberOfSPUsInSpurs()
{
	return TOTAL_NUMBER_OF_SPU - s_iReservedRawSPUs - s_iReservedSPUThreads;
}

//--------------------------------------------------------------------------------------------------

int Exec::GetThisPPUThreadPriority( void )
{
	sys_ppu_thread_t thisPpuThreadId;
	int ret;
	UNUSED( ret );

	ret = sys_ppu_thread_get_id( &thisPpuThreadId );
	ntAssert( CELL_OK == ret );

	int thisPpuThreadPriority;
	ret = sys_ppu_thread_get_priority( thisPpuThreadId, &thisPpuThreadPriority );
	ntAssert( CELL_OK == ret );

	return thisPpuThreadPriority;
}

