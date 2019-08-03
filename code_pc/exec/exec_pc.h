//----------------------------------------------------------------------------------------
//! 
//! \filename exec\exec_pc.h
//! 
//----------------------------------------------------------------------------------------
#if !defined( EXEC_PPU_EXEC_PC_H )
#define EXEC_PPU_EXEC_PC_H


// forward decl
class SPUTask;

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

	//! Called when all reservation are made and we are ready to go.
	static void Init( void );

	//! When we have finished and want to put Exec to bed
	static void Shutdown( void );

	//! frame reset, should be called once every frame
	static void FrameReset( void );

	//! Frame End
	static void FrameEnd( void );

	static void RunAsyncFunction( FunctionTask pTask, void* pParam0, void* pParam1 );

	//! this waits until all the function task (when this is called) have completed
	//! NOTE is a busy wait with this thread assisting in the emptying of the list
	static void WaitUntilFunctionTasksComplete();

private:
	Exec();
	~Exec();

	static bool ExecNextFunctionTask();
	static void SecondHardwareThread( void* pThreadParam );
	
};

#endif // end EXEC_PPU_EXEC_PC_H
