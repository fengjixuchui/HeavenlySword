//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Timer Functionality

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef	FP_TIMER_H
#define	FP_TIMER_H

#include <sys/time_util.h>

//--------------------------------------------------------------------------------------------------
/**
	@class			FpTimer

	@brief			A class that manages the update & retrival of timer related inforamtion			
**/
//--------------------------------------------------------------------------------------------------

class	FpTimer : public FwSingleton< FpTimer >
{ 
public:

	// Initialise & shutdown
	static void				Initialise( void )	{	FW_NEW FpTimer();	}
	static void				Shutdown( void )	{	FpTimer::Destroy();	}

	// Update
	void					Update( void );

	// Fixed update control
	void					EnableFixedUpdate( float fixedUpdateTime );
	void					DisableFixedUpdate( void );

	// Accessors
	inline	u32				GetSystemTicks( void ) const;
	inline	double			GetSystemTime( void ) const;
	inline	float			GetSystemTimeChange( void ) const;

	// Hardware timers & conversion scalars
	static	inline	float	GetHwTimerPeriod( void );
	static	inline	float	GetHwTimerPeriodFrame( void );
	static	inline	u64		GetHwTimer( void );

	// Temporary - this will be removed when the rendering subsystem arrives..
	static	inline	int		GetRefreshRate( void );

private:
	FpTimer();

	static	const int	kMaxFrameTimeCap = 30;		///< If more than 'kMaxFrameTimeCap' frames pass between updates, we assume 1 frame has passed.

	static	float		ms_timerPeriod;				///< Floating point scalar to convert from HW timer clocks to seconds
	static	float		ms_timerPeriodFrame;		///< Floating point scalar to convert from HW timer clocks to frames
	static	int			ms_refreshRate;				///< TEMPORARY : Holds the refresh rate of the screen in Hz 

	bool				m_useFixedUpdate;			///< 'true' if we want to wire our time updates to return a fixed value.
	float				m_fixedUpdateTime;			///< The time that we return when fixed updates are enabled.
	
	u64					m_previousCount;			///< Used to hold the CPU performance counter for the last update.
	u32					m_systemTicks;				///< Number of update frames since initialisation
	double				m_systemTime;				///< Number of seconds since initialisation
	float				m_systemTimeChange;			///< Change in system time since last call to FpTimer::Update
};

//--------------------------------------------------------------------------------------------------
/**
	@brief			Enables fixed update processing. Every call to Update() will return the specified
					update time, until DisableFixedUpdate() is called. 

	@param			fixedUpdateTime			Time (in seconds) that will always be returned by timer.
**/
//--------------------------------------------------------------------------------------------------

inline	void		FpTimer::EnableFixedUpdate( float fixedUpdateTime )
{
	FW_ASSERT( fixedUpdateTime >= 0.0f );
	m_useFixedUpdate	= true;
	m_fixedUpdateTime	= fixedUpdateTime;
}


//--------------------------------------------------------------------------------------------------
/**
	@brief			Disable fixed update processing. Every call to Update() will return the specified
					update time, until DisableFixedUpdate() is called. 
**/
//--------------------------------------------------------------------------------------------------

inline	void		FpTimer::DisableFixedUpdate( void )
{
	m_useFixedUpdate	= false;
	m_fixedUpdateTime	= 0.0f;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Retrieve the number of calls made to FpTimer::Update since FpTimer construction.

	@return			System tick count.
**/
//--------------------------------------------------------------------------------------------------

inline	u32		FpTimer::GetSystemTicks( void ) const
{
	return	m_systemTicks;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Retrieve the system time (seconds since initialisation of FpTimer object)

	@return			System time (in seconds)

	@note			Soak testing of projects often shows up problems with floating point precision
					when the title has been running for an extremely long period of time. To combat
					this, the system time is stored as a double precision value. Ideally project
					code would not use this value directly, instead choosing to use the change in
					time (which is a single precision floating point value).
**/
//--------------------------------------------------------------------------------------------------

inline	double	FpTimer::GetSystemTime( void ) const
{
	return	m_systemTime;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Retrieve the change in system time since the last call to FpTimer::Update()

	@return			Change in system time (in seconds)
**/
//--------------------------------------------------------------------------------------------------

inline	float	FpTimer::GetSystemTimeChange( void ) const
{
	return m_systemTimeChange;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Retrieve a scalar used to convert hardware timer values into seconds

	@return			A scalar.
**/
//--------------------------------------------------------------------------------------------------

inline	float	FpTimer::GetHwTimerPeriod( void )
{
	return ms_timerPeriod;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Retrieve a scalar used to convert hardware timer values into frames

	@return			A scalar.
**/
//--------------------------------------------------------------------------------------------------

inline	float	FpTimer::GetHwTimerPeriodFrame( void )
{
	return ms_timerPeriodFrame;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Retrieve the current refresh rate

	@return			Refresh rate in Hz

	@todo			This is temporary.. we also don't want the timer class to pull anything from 
					the rendering system (which is ultimately responsible for refresh rate issues).
**/
//--------------------------------------------------------------------------------------------------

inline	int		FpTimer::GetRefreshRate( void )
{
	return ms_refreshRate;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Retrieves the current hardware timer value

	@return			The current hardware timer value

	@note			Please do not rely on this being a number of cycles/clocks. The value should
					be scaled by the results of GetHwTimerPeriod() or GetHwTimerPeriodFrame() before
					being interpreted as being in a particular unit.

	@note			This function does not manage timer wraparound. It is the responsibility of
					the caller to deal with this, should they feel the need to.
**/
//--------------------------------------------------------------------------------------------------

inline u64	FpTimer::GetHwTimer( void )
{
	u64	tb;
	SYS_TIMEBASE_GET( tb );
	return	tb;
}

#endif	// FP_TIMER_H
