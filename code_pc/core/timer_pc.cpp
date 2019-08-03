/***************************************************************************************************
*
*	Implementation for CTimer class. This uses system hardware to give the project access to several
*	timer values. Currently the following timers are maintained
*
*	System Tick			:	Number of 'ticks' (ie update loops) since the game started
*
*	System Timer		:	Length of time in seconds since the application started
*
*	System Time Change	:	Time (in seconds) since the last call to CTimer::Update
*
*	Game Time			:	Time (in seconds, but scaled by our global time scalar) since ResetGameTime() 
*							was called
*
*	Game Time Change	:	Length of time (in seconds, scaled by our global time scalar) since
*							the last call to CTimer::Update
*
*	CHANGES		
*
*	25/07/2005	Wil	Created
*
***************************************************************************************************/

#include "core/timer_pc.h"

bool	CTimer::m_bUsedFixedTimeChange = false;
float	CTimer::m_fFixedTimeChange = 1.0f / 60.0f;

float	CTimer::s_fGameRefreshRate	= 30.0f;

/***************************************************************************************************
*	
*	FUNCTION		CTimer::CTimer
*
*	DESCRIPTION		This initialises timer systems
*
***************************************************************************************************/

CTimer::CTimer()
{
	// Get the current CPU counter
	m_lPreviousCount		= GetHWTimer();

	// Clear our system tick & time variables

	m_uiSystemTicks			= 1000;
	m_dSystemTime			= 0.0f;
	m_fSystemTimeChange		= 0.0;

	// Clear our game time & scalar variables

	m_fGameTimeScalar		= 1.0f;
	m_dGameTime				= 0.0;
	m_dOldGameTime			= 0.0;
	m_fGameTimeChange		= 0.0f;

	// Initialise flag for forced update for one frame 

	m_bUpdateOneFrame		= false;

	m_fDebugTimeScalar		= 1.0f; m_bDebugTimeScalar   = false;
	m_fCameraTimeScalar		= 1.0f; m_bCameraTimeScalar  = false;
	m_fSpecialTimeScalar	= 1.0f; m_bSpecialTimeScalar = false;
	m_fFailureTimeScalar	= 1.0f; m_bFailureTimeScalar = false;
	m_fTutorialTimeScalar	= 1.0f; m_bTutorialTimeScalar = false;
}

CTimer::~CTimer()
{
}

/***************************************************************************************************
*	
*	FUNCTION		CTimer::ResetGameTime
*
*	DESCRIPTION		Reset game time parameters for inbetween levels
*
***************************************************************************************************/

void CTimer::ResetGameTime()
{
	m_dGameTime = 0.0;
	m_fGameTimeChange = 0.0f;
	m_lPreviousCount = CTimer::GetHWTimer();
	m_dOldGameTime = 0.0;

	// flag the game time as paused untill we get a valid first update
	m_gameTimeInfo.GetMask().Set(TimeInfo::F_PAUSE);
}


/***************************************************************************************************
*	
*	FUNCTION		CTimer::Update
*
*	DESCRIPTION		Called at the start of each update loop, this routine fetches the current 
*					timer count and adjusts our timer variables accordingly. The timer is then 
*					reset to a zero state.
*
***************************************************************************************************/
void CTimer::Update( bool bGameTimePaused )
{
	// Read our cycle counter - we combine with the previous count to determine the number of
	// seconds passed since the last update..
	int64_t lCurrentCount = GetHWTimer();
	int64_t lElapsedCount;
	lElapsedCount = lCurrentCount - m_lPreviousCount;

	// Calculate the delta values from the old values. This maintains accuracy when performing normal update and when performing a forced update
    double dOldSystemTime = m_dSystemTime;

	// Calculate our floating point timer value and store the original and scaled versions of it...
	float fTimer = (m_bUsedFixedTimeChange) ? m_fFixedTimeChange : static_cast<float>( lElapsedCount ) * GetHWTimerPeriod();

	// There was an issue here with a buggy cast - ( float )( int )( lElapsedCount ) was occasionally
	// giving negative numbers when lElapsed count was very large.  This shouldn't have been the case
	// since a __int64 - int - float should be a well defined cast.  The new direct cast seems to 
	// remedy this issue - but i am making doubly sure here - GH
	ntAssert( static_cast<float>( lElapsedCount ) > 0.0f );

	// Update our game time - note that this is capped, so that we can debug the game properly
	// without the time going mad. 
	// The maximum time change is now 0.1 seconds, this is also the minimum length of an animation
	// as the animation system cannot cope with a anim shorter than an update. Deano
	const float fMaxFrameTime = 0.1f;
	if ( ( fTimer > fMaxFrameTime ) || ( fTimer < 0.0f ) )
		fTimer = fMaxFrameTime;

	// Reset the hardware timer
	m_lPreviousCount = lCurrentCount;

	// Update our system time & tick count
	m_dSystemTime += fTimer;

	// Recalculate delta rather than using fTimer to avoid rounding errors
	m_fSystemTimeChange = (float)(m_dSystemTime - dOldSystemTime); 
	m_uiSystemTicks++;

	// make sure we have the latest game time scalar
	CalcGameTimeScalar();

	// If one frame update is requested, set the game time scalar accordingly
	if( m_bUpdateOneFrame )
	{
		m_fGameTimeScalar = m_fOneFrameTimeScalar;
	}

	// Complete overide of game time
	if( bGameTimePaused )
	{
		m_fGameTimeScalar = 0.0f;
	}

	// Update the game time
	float	fScaledTimer = fTimer * m_fGameTimeScalar;
	m_dGameTime += fScaledTimer;

	// Recalculate delta rather than use fScaledTimer to avoid rounding errors
	m_fGameTimeChange = ( float )( m_dGameTime - m_dOldGameTime ); 

	// Store the current game time so that it can be used later to calculate the change in game time. 
	m_dOldGameTime = m_dGameTime;

	// Reset the update one frame parameters
	if( m_bUpdateOneFrame )
	{
		m_fGameTimeScalar = 0.0f;
		m_bUpdateOneFrame = false;
	}

	// time info stuff, detect when time pause
	// NOTE this is deprecated on PS3 an should be removed ASAP
	if(m_fGameTimeChange<0.0001f)
	{
		m_gameTimeInfo.GetMask().Set(TimeInfo::F_PAUSE);
	}
	else
	{
		m_gameTimeInfo.UpdateTimeChange(m_fGameTimeChange);
		m_gameTimeInfo.GetMask().Unset(TimeInfo::F_PAUSE);	
	}
}

void CTimer::SetDebugTimeScalar(float fScalar)			
{
	m_fDebugTimeScalar = fScalar;
	m_bDebugTimeScalar = true;

	CalcGameTimeScalar();
}


void CTimer::SetCameraTimeScalar(float fScalar)
{
	m_fCameraTimeScalar = fScalar;
	m_bCameraTimeScalar = true;

	CalcGameTimeScalar();
}


void CTimer::SetSpecialTimeScalar(float fScalar)
{
	if(fScalar > 1.0f-EPSILON && fScalar < 1.0f+EPSILON)
		ClearSpecialTimeScalar();
	else
	{
		m_fSpecialTimeScalar = fScalar;
		m_bSpecialTimeScalar = true;

		CalcGameTimeScalar();
	}
}

void CTimer::SetFailureTimeScalar(float fScalar)
{
	if(fScalar > 1.0f-EPSILON && fScalar < 1.0f+EPSILON)
		ClearFailureTimeScalar();
	else
	{
		m_fFailureTimeScalar = fScalar;
		m_bFailureTimeScalar = true;

		CalcGameTimeScalar();
	}
}

void CTimer::SetTutorialTimeScalar(float fScalar)
{
	if(fScalar > 1.0f-EPSILON && fScalar < 1.0f+EPSILON)
		ClearTutorialTimeScalar();
	else
	{
		m_fTutorialTimeScalar = fScalar;
		m_bTutorialTimeScalar = true;

		CalcGameTimeScalar();
	}
}


void CTimer::ClearDebugTimeScalar()
{
	m_bDebugTimeScalar = false;
	m_fDebugTimeScalar = 1.0f;
	
	CalcGameTimeScalar();
}


void CTimer::ClearCameraTimeScalar()
{
	m_bCameraTimeScalar = false;

	CalcGameTimeScalar();
}


void CTimer::ClearSpecialTimeScalar()
{
	m_bSpecialTimeScalar = false;

	CalcGameTimeScalar();
}

void CTimer::ClearFailureTimeScalar()
{
	m_bFailureTimeScalar = false;

	CalcGameTimeScalar();
}

void CTimer::ClearTutorialTimeScalar()
{
	m_bTutorialTimeScalar = false;

	CalcGameTimeScalar();
}

void CTimer::CalcGameTimeScalar()
{
	if(m_bTutorialTimeScalar)
		m_fGameTimeScalar = m_fTutorialTimeScalar;
	else if(m_bFailureTimeScalar)
		m_fGameTimeScalar = m_fFailureTimeScalar;
	else if(m_bSpecialTimeScalar)
		m_fGameTimeScalar = m_fSpecialTimeScalar;
	else if(m_bCameraTimeScalar)
		m_fGameTimeScalar = m_fCameraTimeScalar;
	else
		m_fGameTimeScalar = 1.0f;

	m_fGameTimeScalar *= m_fDebugTimeScalar;
}
