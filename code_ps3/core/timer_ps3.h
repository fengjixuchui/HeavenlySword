/***************************************************************************************************
*
*	Timer classes
*
*	CHANGES		
*
*	25/07/2005	Wil	Created
*
***************************************************************************************************/

#ifndef	_TIMER_PS3_H
#define	_TIMER_PS3_H

#include "game/commandresult.h"
#include "core/timeinfo.h"

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

	u_int		GetSystemTicks( void ) const				{ return m_uiSystemTicks; }			// Number of calls to CTimer::Update made during game
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

	COMMAND_RESULT		UpdateOneFrame(const float& fTimeScalar=1.0f)		{ m_bUpdateOneFrame = true; m_fOneFrameTimeScalar=fTimeScalar; return CR_SUCCESS;};
	void		ModifyGameTime(float fVal)					{ m_dGameTime += fVal;};

	// Timer hardware interfaces are static - otherwise we incur a penalty when using them for going through the 
	// singleton again. 

	static	double	GetHWTimerPeriod( void )				{ return m_dTimerPeriod; };
	static	double	GetHWTimerPeriodFrame( void )			{ return m_dTimerPeriodFrame; };
	static	int64_t	GetHWTimer( void )
	{
		register int64_t tb;
		asm volatile ("mfspr %0, 268": [tb] "=r" (tb));
		return	( int64_t )tb;
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
	static float s_fGameRefreshRate;

	// get time info
	const TimeInfo& GetGameTimeInfo() const { return m_gameTimeInfo; }

private:
	
	TimeInfo		m_gameTimeInfo; // another version of the game time, obviously more friendly

	static	double	m_dTimerPeriod;			// Floating point scalar to convert from HW timer clocks to seconds
	static	double	m_dTimerPeriodFrame;	// Floating point scalar to convert from HW timer clocks to frames
	
	int64_t			m_lPreviousCount;		// Used to hold the CPU performance counter for the last update.

	// System times hold number of ticks & seconds since CTimer::Initialise() was called. These values
	// are not adjusted by any form of time scalar, and should be used with care.

	u_int			m_uiSystemTicks;		// Number of update frames since initialisation
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


