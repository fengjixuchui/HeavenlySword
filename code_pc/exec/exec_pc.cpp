//----------------------------------------------------------------------------------------
//! 
//! \filename exec\exec_pc.h
//! 
//----------------------------------------------------------------------------------------
#include "exec_pc.h"

//#define EXEC_DEBUG_MODE

//----------------------------------------------------------------------------------------
// Static members of exec
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// local namespace for really private stuff
//----------------------------------------------------------------------------------------
namespace
{
#if defined( EXEC_DEBUG_MODE )
#	define	ExecMessage( msg )	ntPrintf msg
#else
#	define 	ExecMessage( msg ) do {} while ( false )
#endif

	//! Max number of Function Tasks per frame
	static const unsigned int MAXIMUM_NUMBER_OF_FUNCTION_TASKS = 1024 * 4;

	struct
	{
		Exec::FunctionTask pTask;
		void* pParam0;
		void* pParam1;
	} s_FunctionTasks[ MAXIMUM_NUMBER_OF_FUNCTION_TASKS ];
	//! 0 if the second hardware thread isn't running
	static volatile uint32_t s_iSecondThreadRunning;

	//! 0 if the second hardware thread is ideling
	static volatile uint32_t s_iSecondThreadIsIdle;

	// these must be only touched by thread-safe access, they make a very 
	// simple FIFO
	//! the current put index (where a new task is added)
	static volatile uint32_t	s_iPutFunctionTaskIndex;

	//! the current get index (where a task will be pulled from and run)
	static volatile uint32_t	s_iGetFunctionTaskIndex;

	static WaitableEvent* s_pSecondThreadEvent;

	static HANDLE s_secondHardwareThread; //!< thread handle of the second core thread

	static CriticalSection	s_FunctionTaskGetCrit;

}
//----------------------------------------------------------------------------------------
//!
//! Set up and init the task system. 
//! Should occur as early as possible
//!
//----------------------------------------------------------------------------------------
void Exec::Init( void )
{
	s_pSecondThreadEvent = NT_NEW WaitableEvent();

	AtomicSet( &s_iSecondThreadRunning, 0 );
	AtomicSet( &s_iSecondThreadIsIdle, 0 );
	// start a second thread to run on the second core
	s_secondHardwareThread = CreateThread(
			NULL,		//LPSECURITY_ATTRIBUTES ThreadAttributes,
			0 ,			//DWORD StackSize,
			(LPTHREAD_START_ROUTINE)SecondHardwareThread,	//LPTHREAD_START_ROUTINE StartAddress,
			0,		//LPVOID Parameter,
			0,			//DWORD CreationFlags,
			NULL		//LPDWORD ThreadId 
		);



	FrameReset();
}

//----------------------------------------------------------------------------------------
//!
//! Free up the memory and shut everything down
//!
//----------------------------------------------------------------------------------------
void Exec::Shutdown( void )
{
	AtomicSet( &s_iSecondThreadRunning, 0 );
	AtomicSet( &s_iSecondThreadIsIdle, 0 );
	s_pSecondThreadEvent->Wake();

	NT_DELETE( s_pSecondThreadEvent );
}

//----------------------------------------------------------------------------------------
//!
//! Frame Reset
//!
//----------------------------------------------------------------------------------------
void Exec::FrameReset( void )
{
	ExecMessage( (Debug::DCU_EXEC, "---------------- FRAME START -----------------------\n") );

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
	ExecMessage( (Debug::DCU_EXEC, "------------------ FRAME END -----------------------\n") );
}

//----------------------------------------------------------------------------------------
//!
//! SecondHardwareThread... This is designed to run on a second core (or SMT) and
//! use the games relatively simple co-operative thread system
//!
//----------------------------------------------------------------------------------------
void Exec::SecondHardwareThread( void* pThreadParam )
{
	UNUSED( pThreadParam );
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
