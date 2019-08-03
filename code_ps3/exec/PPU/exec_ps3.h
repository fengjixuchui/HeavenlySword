//----------------------------------------------------------------------------------------
//! 
//! \filename exec\ppu\exec_ps3.h
//! 
//----------------------------------------------------------------------------------------
#if !defined( EXEC_PPU_EXEC_PS3_H )
#define EXEC_PPU_EXEC_PS3_H


// forward decl
class	SPUTask;
class	MultiThreadSafeJobList;
struct	DependencyCounter;
struct	ExecSPUJobAdder;
struct	CellSpurs;

//----------------------------------------------------------------------------------------
//! 
//! Exec
//! \note Exec is not a singleton as some need to be called incredible early. Potentially
//! long before even the memory subsytem is set up, so I figured rather than doing half
//! as static and the other half as a singleton, I'd do the whole thing using static.
//! It however is a singleton and can only ever be one of it
//! 
//----------------------------------------------------------------------------------------
class Exec
{
public:
	typedef void (*FunctionTask)( void*, void* );

	//! Reserve a SPU in RAW mode for a system outside of exec's control.
	static void ReserveRawSPU( const char* pName );
	//! Reserve N SPUs for the OS level SPU threads system outside of exec's control
	static void ReserveSPUThreads( const int iNumSPUs );

	//! Called when all reservation are made and we are ready to go.
	static void Init( void );

	//! When we have finished and want to put Exec to bed
	static void Shutdown( void );

	//! frame reset, should be called once every frame
	static void FrameReset( void );

	//! Frame End
	static void FrameEnd( void );

	//! runs a SPU task at normal priority
	static void RunTask( SPUTask* pTask );

	//! runs a PPU async function
	static void RunAsyncFunction( FunctionTask pTask, void* pParam0, void* pParam1 );

	//! this waits until all the function task (when this is called) have completed
	//! NOTE is a busy wait with this thread assisting in the emptying of the list
	static void WaitUntilFunctionTasksComplete();

	// Initialise the depedency counter to the specified semaphore value (and other stuff)
	static void InitDependency( DependencyCounter* pCounter, uint32_t iCount );

	// add this will stop the any later jobs being run until the dependecny counter == 0
	static void AddBarrierJob( DependencyCounter* pCounter );

	typedef void (*PPUCallback)( uint32_t data0, uint32_t data1 );

	// removed as not safe over level restart
/*
	// add a PPU callback handler return portnum or 0xFFFFFFFF if no spare callbacks spaces
	static uint32_t AddPPUCallbackHandler( PPUCallback func, uint32_t iQueueDepth );

	// remove a PPU callback handler 
	static void RemovePPUCallbackHandler( uint32_t portnum );
*/
	// prepare a SPU job adder (what Exec SPU used to fire a task) from a SPUTask structure)
	static void PrepareSPUJobAdder( const SPUTask* pTask, ExecSPUJobAdder* pAdder );

	typedef void (*PPUThreadFunction)( uint64_t param );
	static void CreatePPUThread( PPUThreadFunction func, int64_t thread_arg = 0, int32_t priority = 1500, const char *thread_name = "HS PPU Thread" );

	// return how many spurs are being used by spurs
	static unsigned int NumberOfSPUsInSpurs();

	// Return a pointer to our SPURS instance.
	static CellSpurs *	GetSpurs();

	// returns the ppu thread priority of the calling thread
	static int GetThisPPUThreadPriority( void );

	static MultiThreadSafeJobList *GetWWSJobList();

private:
	Exec();
	~Exec();

	static bool ExecNextFunctionTask();
	static void SecondHardwareThread( uint64_t iThreadParam );

	//! Number of SPU in a PS3 for us to use
	static const unsigned int TOTAL_NUMBER_OF_SPU = 5;

	static unsigned int s_iReservedRawSPUs;		//!< how many spus have been reserved in raw mode	
	static unsigned int s_iReservedSPUThreads;	//!< how many spus have been reserved for PS SPUThreads

	// to help us from the madness in debug mode
#if defined(_DEBUG )
	//! names of names of who has reserved SPU from us
	static const char* s_aReservedSPUNames[ TOTAL_NUMBER_OF_SPU ];
#endif
};
#endif // end EXEC_PPU_EXEC_PS3_H
