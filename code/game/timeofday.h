//------------------------------------------------------------------------------------------
//!
//!	\file timeofday.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_TIMEOFDAY_H
#define	_TIMEOFDAY_H

// Necessary includes

//------------------------------------------------------------------------------------------
//!
//!	TimeOfDayDef
//!	An array of prefered times of day for the game be in.  Allows content to be produced
//!	based on a limited number of settings
//!
//------------------------------------------------------------------------------------------
class TimeOfDayDef
{
public:

	// Construction destruction
	TimeOfDayDef( void );
	~TimeOfDayDef( void );

	// Checks the data we have been given after serialisation
	void PostConstruct( void );

	// Proceed throught the prefered times loop
	void GoToNextPreferedTime( void );

	// Get the current prefered time
	float GetCurrentPreferedTime( void ) const;

	// These times should ideally be a list when there is support in 
	// welder so we can define an arbitrary number of times of day
	float	m_fPreferedTime1;
	float	m_fPreferedTime2;
	float	m_fPreferedTime3;

private:

	// The list that we will eventually expose
	ntstd::List<float> m_obPreferedTimesOfDay;

	// The current time of day
	ntstd::List<float>::const_iterator m_obCurrent;
	bool m_bCurrentValid;
};


//------------------------------------------------------------------------------------------
//!
//!	TimeOfDay
//!	Looks after the time of day for the game.  A nice interface to the DayNightCycle in the
//! renderer for game code.
//!
//------------------------------------------------------------------------------------------
class TimeOfDay : public Singleton<TimeOfDay>
{
public:

	// Construction destruction
	TimeOfDay( TimeOfDayDef* pDef );

	// To be called after data has been serialised
	void PostSerialisation( void );

	// This updates it
	void Update( void );

	// Proceed to next prefered time of day - false if request unsuccessful
	bool MoveToNextPreferedTime( float fMoveTimeSpan );

private:

	// Our definition
	TimeOfDayDef* m_pobDefinition;

	// Are we currently moving to a new time of day
	bool m_bChangingDayTime;

	// How long have we got to change it
	float m_fDayTimeChangeTime;
};

#endif // _TIMEOFDAY_H
