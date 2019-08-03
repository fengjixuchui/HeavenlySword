/***************************************************************************************************
*
*	DESCRIPTION
*
*	NOTES
*
***************************************************************************************************/

#ifndef CORE_PROFILING_H
#define CORE_PROFILING_H

// Enable Hierarchical load-time profiling.
#define _LOAD_PROFILING
#include "core/hierarchicalprofiler.h"

class	CGraph;
class	CGraphSampleSet;

/***************************************************************************************************
*	 
*	CLASS			CMicroTimer
*
*	DESCRIPTION		Useful timer for 1-stop timing needs, ideal for bracketing function calls.
*   NOTE : This is slightly less accurate than it used to been (extra function call overhead)
*        : to reduce compile times/ header coupling.
*
***************************************************************************************************/

class	CMicroTimer
{
public:
	CMicroTimer( void );
	
	void	Start( void );
	void	Stop( void );
	
	int		GetTicks( void )		const;
	float	GetSecs( void )			const;
	float	GetMilliSecs( void )	const;
	float	GetMicroSecs( void )	const;
	float	GetNanoSecs( void )		const;
	float	GetFramePercent( void ) const;

private:
	void		Check( void ) const;
	int64_t			m_lStart, m_lStop;
};

/***************************************************************************************************
*
*	CLASS			CEstimateTimer
*
*	DESCRIPTION		Timer that estimates what time we'd be taking up on a target machine.
*
*	NOTES			provides smoothed frame time, milli and micro seconds, start and stop must be
*					called once per frame, use pause and resume to accumulate timings.
*
***************************************************************************************************/
class CEstimateTimer 
{
public:
	// create some smoothers, setup scale factor and timers
	CEstimateTimer( float fSrcClock = 1.0f, float fTargetClock = 1.0f );


	// kickoff for this frame
	void	Start( void );

	// reset for this frame
	void	Reset( void );

	// stop for this frame
	void	Stop( void );

	// pause this frames timing
	void	Pause( void );

	// resume this frames timing
	void	Resume( void );

	// special ones for uncommon use (cos theyre fraaaaactionally slower)
	void	ResumeOrStart( void );
	void	StopNoPause( void );

	// dump to screen
	void	PrintScrn(float fX, float fY, const char* pcString, uint32_t dwCol = (uint32_t)NTCOLOUR_ARGB(255,255,255,255), bool bShowEstimate = false) const;


	// get info
	float	GetFrameTime( void ) const;
	float	GetMilliSecs( void ) const;
	float	GetMicroSecs( void ) const;

	float	GetEstFrameTime( void ) const;
	float	GetEstMilliSecs( void ) const;
	float	GetEstMicroSecs( void ) const;

private:
	void	Check( void ) const;

	bool			m_bRunning;
	bool			m_bPaused;
	int64_t			m_lThisFrame;
	CMicroTimer		m_obMTimer;
	float			m_fScaleFact;
	float			m_fCumFrame;
	float			m_fCumTime;
};

/***************************************************************************************************
*	 
*	CLASS			CTimedBlock
*
*	DESCRIPTION		Hangs in constructor untill input time has passed
*
***************************************************************************************************/
class	CTimedBlock
{
public:
	CTimedBlock( float fBlockTime );
};

#define BLOCK_FOR_N_SECONDS(n)		CTimedBlock	obPause(n)
#define BLOCK_FOR_N_MILLISECS(n)	CTimedBlock	obPause(n * 0.001f)
#define BLOCK_FOR_N_MICROSECS(n)	CTimedBlock	obPause(n * 0.000001f)
#define BLOCK_FOR_N_NANOSECS(n)		CTimedBlock	obPause(n * 0.000000001f)


/***************************************************************************************************
*	 
*	CLASS			TimedOperationWarning
*
*	DESCRIPTION		Prints a user-specified warning message to the console if an operation
*					takes unusually long to complete. This assumes pOperationID and pMessage
*					will still be available at destruction time.
*
***************************************************************************************************/
#ifdef _PROFILING
class TimedOperationWarning
{
public:
	TimedOperationWarning( const char* pOperationID, const char* pMessage, float fThresholdSeconds );
	~TimedOperationWarning() { Stop(); }
	
	float ElapsedSeconds() const;
	void Stop();

private:
	int64_t m_iStartTime;
	const char* m_pOperationID;
	const char* m_pMessage;
	float m_fThresholdSeconds;
};

#define PROFILE_TIMED_OPERATION_WARNING( operation_id, message, threshold_seconds ) TimedOperationWarning TimedOperationWarning_##operation_id( #operation_id, ( message ), ( threshold_seconds ) );
#define PROFILE_TIMED_OPERATION_WARNING_END( operation_id ) TimedOperationWarning_##operation_id .Stop();
#else
#define PROFILE_TIMED_OPERATION_WARNING( operation_id, message, threshold_seconds )
#define PROFILE_TIMED_OPERATION_WARNING_END( operation_id )
#endif

/***************************************************************************************************
*	 
*	CLASS			LoadTimeProfiler
*
*	DESCRIPTION		A simple little helper class (and macros) for doing a timing not via gatso
*					as gatso are frame based
*
***************************************************************************************************/
class	LoadTimeProfiler
{
public:
	LoadTimeProfiler( );

	// the pText should be a printf style with a single %f to take the time in seconds
	void StopAndPrint( const char* pText);

	int64_t m_iStart;

	// simple for of indenting
	static int s_iIndentCount;
};

// very simple macros. place around a function to get some stuff dump to the log
#ifdef _PROFILING
#define START_LOAD_TIME_PROFILER( X ) LoadTimeProfiler SLTP_Var_##X;
#define STOP_LOAD_TIME_PROFILER( X ) SLTP_Var_##X .StopAndPrint( "LOAD TIME PROFILE: " #X " %f Secs\n" );
#else
#define START_LOAD_TIME_PROFILER( X )
#define STOP_LOAD_TIME_PROFILER( X )  
#endif

/***************************************************************************************************
*	 
*	CLASS			LoadTimeProfilerAcc
*
*	DESCRIPTION		A simple little helper class (and macros) for doing a timing not via gatso
*					as gatso are frame based. This allows you to start/stop many times but
*					produce one single time value
*
***************************************************************************************************/
template< int iIndex>
class	LoadTimeProfilerAcc
{
public:

	static const int MAX_STACK_DEPTH = 8;

	static void Start()
	{
		s_Timers[ s_iCurStackDepth++ ].Start();
		s_iCallCount++;
		ntAssert( s_iCurStackDepth < MAX_STACK_DEPTH );
	}

	static void AccResult()
	{
		s_Timers[ --s_iCurStackDepth ].Stop();
		s_fAccTime += s_Timers[ s_iCurStackDepth ].GetSecs();
	}

	// the pText should be a printf style with a single %f to take the time in seconds and a %i for call count
	static void PrintAccum( const char* pText)
	{
		// this dump to the console the elapsed time in seconds
		char pBuffer[1024];
		sprintf( pBuffer, pText, s_fAccTime, s_iCallCount );
		Debug::AlwaysOutputString( pBuffer );
	}

	//! result time and call count
	static void Reset()
	{
		s_fAccTime = 0.f;
		s_iCallCount = 0;
	}

	static float		s_fAccTime;
	static int			s_iCurStackDepth;
	static CMicroTimer	s_Timers[ MAX_STACK_DEPTH ];
	static int			s_iCallCount;

private:
	LoadTimeProfilerAcc(){};
};

template< int iIndex > float LoadTimeProfilerAcc<iIndex>::s_fAccTime = 0.f;
template< int iIndex > int LoadTimeProfilerAcc<iIndex>::s_iCurStackDepth = 0;
template< int iIndex > int LoadTimeProfilerAcc<iIndex>::s_iCallCount = 0;
template< int iIndex > CMicroTimer LoadTimeProfilerAcc<iIndex>::s_Timers[ MAX_STACK_DEPTH ];

//#define DO_LOAD_TIME_PROFILE

#if defined(DO_LOAD_TIME_PROFILE)
// very simple macros. place around a function to get some stuff dump to the log
#define RESET_LOAD_TIME_PROFILER_ACC( iIndex ) LoadTimeProfilerAcc<iIndex>::Reset();
#define START_LOAD_TIME_PROFILER_ACC( iIndex ) LoadTimeProfilerAcc<iIndex>::Start();
#define STOP_LOAD_TIME_PROFILER_ACC( iIndex ) LoadTimeProfilerAcc<iIndex>::AccResult();
#define PRINT_LOAD_TIME_PROFILER_ACC(X, iIndex) LoadTimeProfilerAcc<iIndex>::PrintAccum( "LOAD TIME PROFILE ACCUM: " #X " %f Secs %d Calls\n" );

#else

#define RESET_LOAD_TIME_PROFILER_ACC( iIndex ) 
#define START_LOAD_TIME_PROFILER_ACC( iIndex ) 
#define STOP_LOAD_TIME_PROFILER_ACC( iIndex ) 
#define PRINT_LOAD_TIME_PROFILER_ACC(X, iIndex)

#endif


/***************************************************************************************************
*
*	CLASS			CProfiler
*
*	DESCRIPTION		Singleton that wraps up multiple CEstimateTimer objects
*
***************************************************************************************************/
class CProfiler : public Singleton<CProfiler>
{
public:
#if defined( _PROFILING )
	CProfiler( void );
	~CProfiler( void );
#endif
	friend class CProfilerInternal;
	
	void	StartProfile( void );
	void	StopProfile( void );

	void	PauseProfile( void )
	{
		ntAssert( m_bProfiling );
		
		m_runningTimer.Pause();
		m_pausedTimer.ResumeOrStart();
	}

	void	ResumeProfile( void )
	{
		ntAssert( m_bProfiling );

		m_pausedTimer.Pause();
		m_runningTimer.Resume();
	}

	void	Display();
	void	SetNameFilter(const char * pNameFilter = 0); // zero value remove the name filter

	float	GetRunningPercent()		const { return m_fRunningPercent; }
	float	GetPausedPercent()		const { return m_fPresentPercent; }
	float	GetCPUUpdatePercent()	const { return m_fGamePercent; }
	float	GetCPURenderPercent()	const { return m_fCPURenderPercent; }
	float	GetGPURenderPercent()	const { return m_fGPURenderPercent; }

private:

	bool				m_bProfiling;
	CEstimateTimer		m_runningTimer;
	CEstimateTimer		m_pausedTimer;
	bool				m_bShowMenu;

	float				m_fRunningPercent;
	float				m_fPresentPercent;
	float				m_fProfileTotal;
	float				m_fFPS;

	float				m_fGamePercent;
	float				m_fCPURenderPercent;
	float				m_fGPURenderPercent;


};

//--------------------------------------------------
//!
//! RenderCounter
//! Structure for embedding within rending classes
//!
//--------------------------------------------------
struct RenderCounter
{
	RenderCounter() { Reset(); }

	void Reset()
	{
		m_iDraws = 0;
		m_iVerts = 0;
		m_iPolys = 0;
	};

	void AddVertList( uint32_t iVerts )		{ m_iDraws++; m_iVerts+=iVerts; }
	void AddTriList( uint32_t iVerts )		{ m_iDraws++; m_iVerts+=iVerts; m_iPolys += iVerts/3; }
	void AddTriStrip( uint32_t iVerts )		{ m_iDraws++; m_iVerts+=iVerts; m_iPolys += iVerts-2; }
	void AddQuadList( uint32_t iVerts )		{ m_iDraws++; m_iVerts+=iVerts; m_iPolys += iVerts/2; }
	void AddPolygon( uint32_t iVerts )		{ m_iDraws++; m_iVerts+=iVerts; m_iPolys++; }
	void AddBatchTriStrip( uint32_t iVerts, uint32_t instanceCount )	{ m_iDraws += instanceCount; m_iVerts+=iVerts * instanceCount; m_iPolys += instanceCount * (iVerts - 2 ); }

	inline RenderCounter& operator += ( const RenderCounter &rhs )
	{
		m_iDraws += rhs.m_iDraws;
		m_iVerts += rhs.m_iVerts;
		m_iPolys += rhs.m_iPolys;
		return *this;
	}

	inline RenderCounter operator - ( const RenderCounter &rhs )
	{
		RenderCounter result;
		result.m_iDraws = this->m_iDraws - rhs.m_iDraws;
		result.m_iVerts = this->m_iVerts - rhs.m_iVerts;
		result.m_iPolys = this->m_iPolys - rhs.m_iPolys;
		return result;
	}

	uint32_t	m_iDraws;
	uint32_t	m_iVerts;
	uint32_t	m_iPolys;
};

/***************************************************************************************************
*
*	MACROS			
*
***************************************************************************************************/

#ifdef _PROFILING

#define PROFILER_START				CProfiler::Get().StartProfile()
#define PROFILER_STOP				CProfiler::Get().StopProfile()
#define PROFILER_PAUSE				CProfiler::Get().PauseProfile()
#define PROFILER_RESUME				CProfiler::Get().ResumeProfile()
#define PROFILER_DISPLAY			CProfiler::Get().Display()

#else

#define PROFILER_START				( ( void )0 )
#define PROFILER_STOP				( ( void )0 )
#define PROFILER_PAUSE				( ( void )0 )
#define PROFILER_RESUME				( ( void )0 )
#define PROFILER_DISPLAY			( ( void )0 )

#endif


#endif // _PROFILING_H
