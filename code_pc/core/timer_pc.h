/***************************************************************************************************
*
*	Timer classes
*
*	CHANGES		
*
*	02/08/2005	Deano	Created
*
***************************************************************************************************/

#ifndef	CORE_TIMER_PC_H
#define	CORE_TIMER_PC_H

#include "game/commandresult.h"
#include "core/timeinfo.h"

//#define BAD_QPF

#if defined(BAD_QPF)
//#define MMNODRV         
#define MMNOSOUND       
//#define MMNOWAVE        
#define MMNOMIDI        
#define MMNOAUX         
#define MMNOMIXER       
//#define MMNOTIMER 
#define MMNOJOY         
#define MMNOMCI         
//#define MMNOMMIO        
#define MMNOMMSYSTEM    
#include "mmsystem.h"
#endif

/***************************************************************************************************
*	 
*	CLASS			CTimer
*
*	DESCRIPTION		A class dealing with interrogation of system timers, implemented using 
*					cycle counters. Please see CTimer::CTimer for a more thorough
*					description.
*
***************************************************************************************************/

class	CTimer : public	Singleton<CTimer>
{ 
public:
	CTimer();
	~CTimer();

	void		Update( bool bGameTimePaused = false );

	// System time manipulation

	uint32_t	GetSystemTicks( void ) const				{ return m_uiSystemTicks; }			// Number of calls to CTimer::Update made during game
	double		GetSystemTime( void ) const					{ return m_dSystemTime; }			// System time (double precision to allow better precision)
	float		GetSystemTimeChange( void ) const			{ return m_fSystemTimeChange; };	// Change in system time since last call to CTimer::Update

	// Game time manipulation

	void		ResetGameTime( void );

	// This still needs fixing up better than this I think, some kind of priority queue or something...
	void		SetDebugTimeScalar(float fScalar);
	void		ClearDebugTimeScalar();
	float		GetDebugTimeScalar()						{return m_fDebugTimeScalar;}
	void		SetCameraTimeScalar(float fScalar);
	void		ClearCameraTimeScalar();
	float		GetCameraTimeScalar()						{return m_fCameraTimeScalar;}
	void		SetSpecialTimeScalar(float fScalar);
	void		ClearSpecialTimeScalar();
	float		GetSpecialTimeScalar()						{return m_fSpecialTimeScalar;}
	void		SetFailureTimeScalar(float fScalar);
	void		ClearFailureTimeScalar();
	float		GetFailureTimeScalar()						{return m_fFailureTimeScalar;}
	void		SetTutorialTimeScalar(float fScalar);
	void		ClearTutorialTimeScalar();
	float		GetTutorialTimeScalar()						{return m_fTutorialTimeScalar;}

	void		CalcGameTimeScalar();

	float		GetGameTimeScalar( void ) const				{ return m_fGameTimeScalar; };
	double		GetGameTime( void ) const					{ return m_dGameTime; };
	float		GetGameTimeChange( void ) const				{ return m_fGameTimeChange; };

	COMMAND_RESULT	UpdateOneFrame(const float& fTimeScalar=1.0f)		{ m_bUpdateOneFrame = true; m_fOneFrameTimeScalar=fTimeScalar; return CR_SUCCESS;};
	void		ModifyGameTime(float fVal)					{ m_dGameTime += fVal;};

	// Timer hardware interfaces are static - otherwise we incur a penalty when using them for going through the 
	// singleton again. 

	static	float	GetHWTimerPeriod( void )				
	{ 
#if !defined( BAD_QPF )
		LARGE_INTEGER	stFrequency;
		QueryPerformanceFrequency( &stFrequency );

		return 1.0f / ( float )( stFrequency.QuadPart );
#else
		return 1.f / 1000.f;
#endif
	};
	static	float	GetHWTimerPeriodFrame( void )			
	{ 
#if !defined( BAD_QPF )
		LARGE_INTEGER	stFrequency;
		QueryPerformanceFrequency( &stFrequency );

		return 1.0f / ( float )( stFrequency.QuadPart / s_fGameRefreshRate );
#else
		return 1.f / (1000.f / s_fGameRefreshRate);
#endif
	};

	static	int64_t	GetHWTimer( void )
	{
#if !defined( BAD_QPF )
		LARGE_INTEGER	stHWTimer;
		QueryPerformanceCounter( &stHWTimer );
		return (int64_t) stHWTimer.QuadPart;
#else
		uint32_t iTimeMS = timeGetTime();
		return (int64_t)iTimeMS; 
#endif
	}
	
	// added to help outside systems know where they are in the frame
	static float GetElapsedSystemTimeSinceHere( int64_t lPreviousCount )
	{
		int64_t lCurrentCount = GetHWTimer();
		int64_t lElapsedCount = lCurrentCount - lPreviousCount;

		// this is the elapsed time in seconds since the start of the frame
		return static_cast<float>( lElapsedCount ) * GetHWTimerPeriod();
	}

	inline float GetElapsedSystemTimeSinceFrameStart()
	{
		return GetElapsedSystemTimeSinceHere(m_lPreviousCount);
	}

	static	bool	m_bUsedFixedTimeChange;
	static	float	m_fFixedTimeChange;

	//! this is set my the graphics system once the display is up and running to give us the refresh rate
	//! before hand its set to 30 (its actually half the refresh rate...)
	static float s_fGameRefreshRate;

	// NOTE this is deprecated on PS3 an should be removed ASAP
	const TimeInfo& GetGameTimeInfo() const { return m_gameTimeInfo; }

private:	
	TimeInfo		m_gameTimeInfo; // another version of the game time, supposedly more friendly

	int64_t			m_lPreviousCount;		// Used to hold the CPU performance counter for the last update.

	// System times hold number of ticks & seconds since CTimer::Initialise() was called. These values
	// are not adjusted by any form of time scalar, and should be used with care.

	uint32_t		m_uiSystemTicks;		// Number of update frames since initialisation
	double			m_dSystemTime;			// Number of seconds since initialisation - this isn't scaled at all!
	float			m_fSystemTimeChange;	// Change in system time since last call to CTimer::Update

	// Game time values respect the game time scalar value. 

	float			m_fGameTimeScalar;		// Scalar for game time (normally 1.0f);
	double			m_dGameTime;			// Amount of time (in seconds) since ResetGameTime() was called
	double			m_dOldGameTime;			// A copy of the last frames game time
	float			m_fGameTimeChange;		// Amount of time (in seconds) since last call to CTimer::Update()
	bool			m_bUpdateOneFrame;		// Is set for one frame and then cleared
	float			m_fOneFrameTimeScalar;	// This enables finer time stepping

	float			m_fDebugTimeScalar;		bool m_bDebugTimeScalar;
	float			m_fCameraTimeScalar;	bool m_bCameraTimeScalar;
	float			m_fSpecialTimeScalar;	bool m_bSpecialTimeScalar;
	float			m_fFailureTimeScalar;	bool m_bFailureTimeScalar;
	float			m_fTutorialTimeScalar;	bool m_bTutorialTimeScalar;
};

#endif	//_TIMER_PS3_H


