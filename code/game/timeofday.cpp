//------------------------------------------------------------------------------------------
//!
//!	\file timeofday.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "timeofday.h"
#include "objectdatabase/dataobject.h"
#include "core/timer.h"
//#include "objectdatabase/objectdatabase.h"
#include "gfx/levellighting.h"

// The interface for serealising the time of day settings
START_STD_INTERFACE( TimeOfDayDef )
	PUBLISH_VAR_AS( m_fPreferedTime1,		PreferedTime1 )
	PUBLISH_VAR_AS(	m_fPreferedTime2,		PreferedTime2 )
	PUBLISH_VAR_AS(	m_fPreferedTime3,		PreferedTime3 )

	DECLARE_POSTCONSTRUCT_CALLBACK		(	PostConstruct )
END_STD_INTERFACE

//------------------------------------------------------------------------------------------
//!
//!	TimeOfDayDef::TimeOfDayDef
//!	Construction
//!
//------------------------------------------------------------------------------------------
TimeOfDayDef::TimeOfDayDef( void )
:	m_fPreferedTime1( 0.0f ),
	m_fPreferedTime2( 8.0f ),
	m_fPreferedTime3( 16.0f ),
	m_obPreferedTimesOfDay(),
	m_obCurrent(),
	m_bCurrentValid( false )
{
}

TimeOfDayDef::~TimeOfDayDef()
{
	if ( TimeOfDay::Exists() )
		TimeOfDay::Get().Kill();
}

//------------------------------------------------------------------------------------------
//!
//!	TimeOfDayDef::PostConstruct
//!	Checks the data that we have been given
//!
//------------------------------------------------------------------------------------------
void TimeOfDayDef::PostConstruct( void )
{
	// Make sure the list is clear - we do this for just edited too
	m_obPreferedTimesOfDay.clear();

	// Push our serealised values onto into our list
	m_obPreferedTimesOfDay.push_back( m_fPreferedTime1 );
	m_obPreferedTimesOfDay.push_back( m_fPreferedTime2 );
	m_obPreferedTimesOfDay.push_back( m_fPreferedTime3 );

#ifndef _RELEASE

	// Loop through our list and check that the values are sensible
	float fLastValue = m_obPreferedTimesOfDay.back();
	bool bLooped = false;
	ntstd::List<float>::const_iterator obEnd = m_obPreferedTimesOfDay.end();
	for ( ntstd::List<float>::const_iterator obIt = m_obPreferedTimesOfDay.begin(); obIt != obEnd; ++obIt )
	{
		// Get this time value
		float fThisValue = *obIt;

		// Check that this time is one on the 24 hour clock
		if ( ( fThisValue < 0.0f ) || ( fThisValue >= 24.0f ) )
			user_warn_p( 0, ( "%s(%d):\tWARNING: The time %.2f\n is not on the 24 hour clock", __FILE__, __LINE__, fThisValue ) );

		// If this value is less than the last one
		if ( fThisValue < fLastValue )
		{
			// If we haven't loop yet, this is the one
			if ( !bLooped )
				bLooped = true;

			// Otherwise we have passed 
			else
				user_warn_p( 0, ( "%s(%d):\tWARNING: The prefered times of day must describe a single loop around the clock", __FILE__, __LINE__ ) );
		}

		// Update the last value
		fLastValue = fThisValue;
	}

#endif

	// Set up the current value
	if ( m_obPreferedTimesOfDay.size() > 0 )
	{
		m_obCurrent = m_obPreferedTimesOfDay.begin();
		m_bCurrentValid = true;
	}

	// If we already have a time of day...
	if (TimeOfDay::Exists())
	{
		TimeOfDay::Kill();
		// Time of day is soon to make a departure - so i have taken out this warning
		// user_warn_p( 0, ( "Time of Day parameters have been defined more than once in the imported content. \n" ) );
	}

	// Set ourselves up as the time of day parameters
	NT_NEW TimeOfDay( this );
}


//------------------------------------------------------------------------------------------
//!
//!	TimeOfDayDef::GoToNextPreferedTime
//!	Proceed throught the prefered times loop
//!
//------------------------------------------------------------------------------------------
void TimeOfDayDef::GoToNextPreferedTime( void )
{
	// If our current time is not valid we just drop out
	if ( !m_bCurrentValid )
		return;

	// Move on to the next prefered time
	++m_obCurrent;

	// Check for boundary conditions
	if ( m_obCurrent == m_obPreferedTimesOfDay.end() )
		m_obCurrent = m_obPreferedTimesOfDay.begin();
}

//------------------------------------------------------------------------------------------
//!
//!	TimeOfDayDef::GetCurrentPreferedTime
//!	Get the current prefered time
//!
//------------------------------------------------------------------------------------------
float TimeOfDayDef::GetCurrentPreferedTime( void ) const
{
	// If we have a current valid time return it
	if ( m_bCurrentValid )
		return * m_obCurrent;
	else
		return 0.0f;
}


//------------------------------------------------------------------------------------------
//!
//!	TimeOfDay::TimeOfDay
//!	Construction
//!
//------------------------------------------------------------------------------------------
TimeOfDay::TimeOfDay( TimeOfDayDef* pDef )
:	m_pobDefinition( pDef ),
	m_bChangingDayTime( false ),
	m_fDayTimeChangeTime( 0.0f )
{
	ntAssert( m_pobDefinition ); 
}

//------------------------------------------------------------------------------------------
//!
//!	TimeOfDay::PostSerialisation
//!	To be called after data has been serialised
//!
//------------------------------------------------------------------------------------------
void TimeOfDay::PostSerialisation( void )
{
	// Make sure the day night cycle is set up
	LevelLighting::Get().SetTimeOfDay( m_pobDefinition->GetCurrentPreferedTime() );
}

//------------------------------------------------------------------------------------------
//!
//!	TimeOfDay::Update
//!	What do you think this does then smart-arse
//!
//------------------------------------------------------------------------------------------
void TimeOfDay::Update( void )
{
	// Get the game update
	float fTimeStep = CTimer::Get().GetGameTimeChange();

	// See if we have work to do
	if ( m_bChangingDayTime )
	{
		// What time are we moving from
		float fCurrentTime = LevelLighting::Get().GetTimeOfDay();

		// Where are we going to
		float fTargetTime = m_pobDefinition->GetCurrentPreferedTime();

		// Calculate the time difference we need to make up
		float fTimeDifference = 0.0f;
		if ( fTargetTime < fCurrentTime )
			fTimeDifference = ( ( 24.0f - fCurrentTime ) + ( fTargetTime ) ); 
		else
			fTimeDifference = ( fTargetTime - fCurrentTime );

		// Set a new cycle time
		LevelLighting::Get().SetTimeOfDay( fCurrentTime + ( ( fTimeDifference / m_fDayTimeChangeTime ) * fTimeStep ) );

		// Decrement our time left
		m_fDayTimeChangeTime -= fTimeStep;

		// Check the bounds
		if ( m_fDayTimeChangeTime <= 0.0f )
		{
			// Our work is done
			m_bChangingDayTime = false;
			LevelLighting::Get().SetTimeOfDay( fTargetTime );
		}
	}
}

//------------------------------------------------------------------------------------------
//!
//!	TimeOfDay::MoveToNextPreferedTime
//!	Should be called to change the time of day on the renderer.  Game code should ideally
//! not be looking to the renderer.
//!
//------------------------------------------------------------------------------------------
bool TimeOfDay::MoveToNextPreferedTime( float fMoveTimeSpan )
{
	// We can't do this if we are already changing times - if we stack up requests
	// we may loop so that we don't make any change at all.  I'm not writing the
	// code to deal with that situation at this point in time
	if ( m_bChangingDayTime )
		return false;

	// Check the validity of our input
	if ( fMoveTimeSpan <= 0.0f )
		return false;

	// Proceed to the next prefered time
	m_pobDefinition->GoToNextPreferedTime();

	// Add the time span to our current time 
	m_fDayTimeChangeTime = fMoveTimeSpan;

	// We have got work to do on the update
	m_bChangingDayTime = true;

	// We have been successful
	return true;
}


