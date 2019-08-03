//------------------------------------------------------------------------------------------
//!
//!	\file combatstyle.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "objectdatabase/dataobject.h"
#include "game/combatstyle.h"
#include "gui/guimanager.h"
#include "game/checkpointmanager.h"

#include "core/visualdebugger.h" 

// Interfaces


START_CHUNKED_INTERFACE( StyleEventDef, Mem::MC_ENTITY  )
	PUBLISH_VAR_WITH_DEFAULT_AS ( m_iDefaultStyleValue, 0,		DefaultStyleValue )
	PUBLISH_VAR_WITH_DEFAULT_AS ( m_obEventString,		"",		EventString )
	PUBLISH_VAR_WITH_DEFAULT_AS ( m_obEventImage,		"",		EventImage )
	PUBLISH_VAR_WITH_DEFAULT_AS ( m_iPriority,			0,		Priority )
END_STD_INTERFACE

START_CHUNKED_INTERFACE( StyleManagerDef, Mem::MC_ENTITY  )
	PUBLISH_PTR_AS(m_pobLifeClockDef, LifeClockDef)
	PUBLISH_PTR_AS(m_pobHitCounterDef, HitCounterDef)
	PUBLISH_PTR_AS(m_pobArcherHitCounterDef, ArcherHitCounterDef)
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fEventCooldownTime, 2.0f, EventCooldownTime ) 

	DECLARE_POSTCONSTRUCT_CALLBACK		(	PostConstruct )
END_STD_INTERFACE

START_STD_INTERFACE( StylePoints )
	PUBLISH_VAR_AS( m_iCausedKO, CausedKO )
	PUBLISH_VAR_AS( m_iCausedKill, CausedKill )
	PUBLISH_VAR_AS( m_iSuccessfulGrab, SuccessfulGrab )
	PUBLISH_VAR_AS( m_iCausedRecoil, CausedRecoil )
	PUBLISH_VAR_AS( m_iCausedImpactStagger, CausedImpactStagger )
	PUBLISH_VAR_AS( m_iCausedBlockStagger, CausedBlockStagger )
	PUBLISH_VAR_AS( m_iCounterAttack, CounterAttack )
	PUBLISH_VAR_AS( m_iEvadedIncomingAttack, EvadedIncomingAttack )
	PUBLISH_VAR_AS( m_iStartedAerial, StartedAerial )
	PUBLISH_VAR_AS( m_fComboMultiple, ComboMultiple )
END_STD_INTERFACE

START_CHUNKED_INTERFACE( LifeClockDef, Mem::MC_ENTITY )
	PUBLISH_VAR_AS(m_iDays, DaysToLive);
	PUBLISH_VAR_AS(m_iHours, HoursToLive);
	PUBLISH_VAR_AS(m_iMinutes, MinutesToLive);
	PUBLISH_VAR_AS(m_fSeconds, SecondsToLive);

	PUBLISH_VAR_WITH_DEFAULT_AS ( m_iLowDays,		0,		DaysLowWarningTime )
	PUBLISH_VAR_WITH_DEFAULT_AS ( m_iLowHours,		1,		HoursLowWarningTime )
	PUBLISH_VAR_WITH_DEFAULT_AS ( m_iLowMinutes,	0,		MinutesLowWarningTime )
	PUBLISH_VAR_WITH_DEFAULT_AS ( m_fLowSeconds,	0.0f,	SecondsLowWarningTime )

	PUBLISH_VAR_AS(m_bKillPlayer, KillPlayer);
	PUBLISH_VAR_AS(m_fTimeToTakeUpdating, TimeToTakeUpdating);
END_STD_INTERFACE

/***************************************************************************************************
*
*	FUNCTION		LifeClock::LifeClock
*
*	DESCRIPTION		LifeClock construction
*
***************************************************************************************************/
LifeClock::LifeClock(LifeClockDef* pobLCD) 
{ 
	if (pobLCD)
	{
		m_pobDef = pobLCD;
		m_bOwnsDef = false;

		m_dTime =  m_pobDef->m_fSeconds;
		m_dTime += m_pobDef->m_iMinutes * 60.0;
		m_dTime += m_pobDef->m_iHours   * 3600.0;
		m_dTime += m_pobDef->m_iDays    * 86400.0;

		m_dLowTime =  m_pobDef->m_fLowSeconds;
		m_dLowTime += m_pobDef->m_iLowMinutes * 60.0;
		m_dLowTime += m_pobDef->m_iLowHours   * 3600.0;
		m_dLowTime += m_pobDef->m_iLowDays    * 86400.0;
	}
	else
	{
		m_pobDef = NT_NEW LifeClockDef();
		m_bOwnsDef = true;
		m_dTime = 86400.0f + 3600.0f + 60.0f; // Yuck
		m_dLowTime = 3600.0f; // Yuck
	}

	m_dInitialTime = m_dTime;

	m_bKilledPlayer = false;
	m_bLowWarning	= false;

	m_fLifeClockScalar = 1.0f;

	m_bActive = true;
};

/***************************************************************************************************
*
*	FUNCTION		LifeClock::~LifeClock
*
*	DESCRIPTION		LifeClock destruction
*
***************************************************************************************************/
LifeClock::~LifeClock() 
{ 
	if (m_bOwnsDef)
	{
		NT_DELETE( m_pobDef ); 
	}
};

/***************************************************************************************************
*
*	FUNCTION		LifeClock::GetTotalInSeconds
*
*	DESCRIPTION		Get total number of seconds from days/hours/mins/seconds
*
***************************************************************************************************/
double LifeClock::GetTotalInSeconds()
{
	return m_dTime;
}


/***************************************************************************************************
*
*	FUNCTION		LifeClock::SetTotalInSeconds
*
*	DESCRIPTION		Sets the life clock days/hours/mins/seconds from a number of seconds
*
***************************************************************************************************/
void LifeClock::SetTotalInSeconds( double dSeconds )
{
	m_dTime = dSeconds;
}

/***************************************************************************************************
*
*	FUNCTION		LifeClock::DecrementLifeClock
*
*	DESCRIPTION		Start deducting time off the lifeclock
*
***************************************************************************************************/
void LifeClock::DecrementLifeClock(float fSeconds)
{
	ntError(fSeconds >= 0);

	m_dTime += -fSeconds;
}

/***************************************************************************************************
*
*	FUNCTION		LifeClock::IncrementLifeClock
*
*	DESCRIPTION		Start incrementing time to the lifeclock
*
***************************************************************************************************/
void LifeClock::IncrementLifeClock(float fSeconds)
{
	ntError(fSeconds >= 0);

	m_dTime += fSeconds;

	// Are we low on time?
	if ( m_bLowWarning && m_dTime > m_dLowTime )
	{
		m_bLowWarning = false;
		CLuaGlobal::CallLuaFunc("OnLifeclockLowEnd");
	}
}

/***************************************************************************************************
*
*	FUNCTION		LifeClock::Update
*
*	DESCRIPTION		Update everything
*
***************************************************************************************************/
void LifeClock::Update(float fTimeDelta) 
{	
	Player* pobPlayer=CEntityManager::Get().GetPlayer();
	ntAssert(pobPlayer);

	if ( m_bActive && pobPlayer->IsHero() )
	{
		m_dTime -= fTimeDelta * m_fLifeClockScalar;
	}

	// Are we low on time?
	if ( !m_bLowWarning && m_dTime <= m_dLowTime )
	{
		m_bLowWarning = true;
		CLuaGlobal::CallLuaFunc("OnLifeclockLowStart");
	}

	// Are we out of time?
	if ( m_dTime <= 0.0 && m_pobDef->m_bKillPlayer && !m_bKilledPlayer)
	{
		// Time to die!
		Player* pobPlayer=CEntityManager::Get().GetPlayer();
		ntAssert(pobPlayer);
		ntAssert( pobPlayer->GetMessageHandler() );
		if ( pobPlayer->IsHero() )
		{
			pobPlayer->Kill();
		}
		m_bKilledPlayer = true;

		CLuaGlobal::CallLuaFunc("OnLifeclockRunOut");
	}
}

//------------------------------------------------------------------------------------------
//!
//!	StyleEventDef::StyleEventDef
//!	Construction
//!
//------------------------------------------------------------------------------------------
StyleEventDef::StyleEventDef()
{

}

//------------------------------------------------------------------------------------------
//!
//!	StyleEventDef::StyleEventDef
//!	Destruction
//!
//------------------------------------------------------------------------------------------
StyleEventDef::~StyleEventDef()
{

}

//------------------------------------------------------------------------------------------
//!
//!	StyleEvent::StyleEvent
//!	Destruction
//!
//------------------------------------------------------------------------------------------
StyleEvent::~StyleEvent()
{
}

//------------------------------------------------------------------------------------------
//!
//!	StyleManagerDef::StyleManagerDef
//!	Construction
//!
//------------------------------------------------------------------------------------------
StyleManagerDef::StyleManagerDef( void )
{
}

StyleManagerDef::~StyleManagerDef()
{
	if ( StyleManager::Exists() )
		StyleManager::Get().Kill();
}

//------------------------------------------------------------------------------------------
//!
//!	StyleManagerDef::PostConstruct
//!
//------------------------------------------------------------------------------------------
void StyleManagerDef::PostConstruct( void )
{
	ntAssert ( !StyleManager::Exists() );
	NT_NEW StyleManager( this );
}

//------------------------------------------------------------------------------------------
//!
//!	StyleManager::StyleManager
//!	Construction
//!
//------------------------------------------------------------------------------------------
StyleManager::StyleManager( StyleManagerDef* pDef )
:	m_pobDefinition( pDef )
,	m_pobLifeClock ( 0 )
,	m_pobHitCounter ( 0 )
,	m_pobArcherHitCounter ( 0 )
,	m_fCurrEventTime ( 0.0f )
,	m_bActive ( true )
,	m_bPrologMode ( false )
{
	ntAssert( m_pobDefinition ); 

	ntAssert( m_pobDefinition->m_pobLifeClockDef );
	m_pobLifeClock = NT_NEW_CHUNK( Mem::MC_ENTITY ) LifeClock( m_pobDefinition->m_pobLifeClockDef );

	if ( m_pobDefinition->m_pobHitCounterDef )
		m_pobHitCounter = NT_NEW_CHUNK( Mem::MC_ENTITY ) HitCounter( m_pobDefinition->m_pobHitCounterDef );

	if ( m_pobDefinition->m_pobArcherHitCounterDef )
		m_pobArcherHitCounter = NT_NEW_CHUNK( Mem::MC_ENTITY ) ArcherHitCounter( m_pobDefinition->m_pobArcherHitCounterDef );
}

//------------------------------------------------------------------------------------------
//!
//!	StyleManager::~StyleManager
//!	Construction
//!
//------------------------------------------------------------------------------------------
StyleManager::~StyleManager( )
{
	// If we have a life clock, free it
	if ( m_pobLifeClock )
	{
		NT_DELETE_CHUNK( Mem::MC_ENTITY, m_pobLifeClock );
	}

	// If we have a hit counter, free that too
	if ( m_pobHitCounter )
	{
		NT_DELETE_CHUNK( Mem::MC_ENTITY, m_pobHitCounter );
	}

	// If we have an archer hit counter, free that as well
	if ( m_pobArcherHitCounter )
	{
		NT_DELETE_CHUNK( Mem::MC_ENTITY, m_pobArcherHitCounter );
	}

	// Clean up any remaining events
	StyleEventIter obEnd = m_aobStyleEventQue.end();
	for ( StyleEventIter obIt = m_aobStyleEventQue.begin(); obIt != obEnd; ++obIt)
	{
		NT_DELETE_CHUNK ( Mem::MC_ENTITY, ( *obIt ) );
	}
	m_aobStyleEventQue.clear();

	obEnd = m_aobStyleEventRenderList.end();
	for ( StyleEventIter obIt = m_aobStyleEventRenderList.begin(); obIt != obEnd; ++obIt)
	{
		NT_DELETE_CHUNK ( Mem::MC_ENTITY, ( *obIt ) );
	}
	m_aobStyleEventRenderList.clear();
}

//------------------------------------------------------------------------------------------
//!
//!	StyleManager::Update( float fTimeStep )
//!
//------------------------------------------------------------------------------------------
//#define _STYLEPOINT_DEBUG

void StyleManager::Update( float fTimeStep )
{
	StyleManager::Get().GetStats().m_fTimePlayed += fTimeStep;

	if ( m_pobLifeClock )
		m_pobLifeClock->Update( fTimeStep );

	if ( m_pobHitCounter ) 
		m_pobHitCounter->Update( fTimeStep );

	if ( m_pobArcherHitCounter )
		m_pobArcherHitCounter->Update( fTimeStep );

	if ( m_fCurrEventTime > 0.0f )
	{
		m_fCurrEventTime -= fTimeStep;
	}

#ifdef _STYLEPOINT_DEBUG
	if ( IsActive() )
		g_VisualDebug->Printf2D(200.0f, 20.0f,  DC_GREEN, 0, "Style Points Active" );
	else
		g_VisualDebug->Printf2D(200.0f, 20.0f,  DC_GREEN, 0, "Style Points Inactive" );
#endif //_STYLEPOINT_DEBUG

	// All done if event list is empty
	if ( m_aobStyleEventQue.empty() )
		return;

	// Ignore events while inactive
	if ( !IsActive() )
	{
		m_aobStyleEventQue.clear();
		return;
	}

	// Archer and Basic Heroine do not gather style
	// Might be better to use active flag when switching between players
	CEntity* pobEntity = CEntityManager::Get().GetPlayer();
	ntAssert ( pobEntity && pobEntity->IsPlayer() );
	Player* pobCharacter = pobEntity->ToPlayer();

	if ( pobCharacter->IsArcher() || ( pobCharacter->IsHero() && !pobCharacter->ToHero()->HasHeavenlySword() ) )
	{
		m_aobStyleEventQue.clear();
		return;
	}

	int iChangeInStyle = 0;

	// Do we have a list and passed the cooldown time
	if ( ( !m_aobStyleEventQue.empty() ) && ( m_fCurrEventTime <= 0.0f ) )
	{
		StyleEvent* pobBestEvent = m_aobStyleEventQue.front();
			
		while ( !m_aobStyleEventQue.empty() )
		{
			// Process top event	
			StyleEvent* pobEvent = m_aobStyleEventQue.front();
			m_aobStyleEventQue.pop_front();

			// Is this a better event to display to the player?
			if ( (pobEvent->m_pobDef->m_iPriority > pobBestEvent->m_pobDef->m_iPriority) || 
					( (pobEvent->m_pobDef->m_iPriority == pobBestEvent->m_pobDef->m_iPriority) && (pobEvent->m_iStyleValue > pobBestEvent->m_iStyleValue ) ) )
			{
				pobBestEvent = pobEvent;
			}

			// Style gained
			if ( pobEvent->m_iStyleValue > 0 )
			{
				StyleStats& obStats = StyleManager::Get().GetStats();

				switch ( pobEvent->m_eStyleType )
				{
					case STYLE_TYPE_SPEED:
						obStats.m_obStylePointsForSection.m_iStylePointsSpeed += pobEvent->m_iStyleValue;
						break;

					case STYLE_TYPE_POWER:
						obStats.m_obStylePointsForSection.m_iStylePointsPower += pobEvent->m_iStyleValue;
						break;

					case STYLE_TYPE_RANGE:
						obStats.m_obStylePointsForSection.m_iStylePointsRange += pobEvent->m_iStyleValue;
						break;

					case STYLE_TYPE_AERIAL:
						obStats.m_obStylePointsForSection.m_iStylePointsAerial += pobEvent->m_iStyleValue;
						break;

					case STYLE_TYPE_MISC:
						obStats.m_obStylePointsForSection.m_iStylePointsMisc += pobEvent->m_iStyleValue;
						break;

					default:
						ntAssert(0);
						break;
				} // switch ( pobEvent->m_iStyleType )

				obStats.m_obStylePointsForSection.m_iStylePointsOverall += pobEvent->m_iStyleValue;

			}	
		
			// Sum the change in style
			iChangeInStyle += pobEvent->m_iStyleValue;
		}

		// Do in-game combo unlocking, whereby...
		// New Points = Previous Totals + Positive Gains In Each Stance
		StanceStylePoints obNewUnlockPoints = m_obStyleStats.m_obCachedStylePointTotals;

		// Calculate the positive gains for each stance, removing any negative values (we don't want to relock moves!)
		StanceStylePoints obNewUnlockDeltas = m_obStyleStats.m_obStylePointsForSection - m_obStyleStats.m_obStylePointsDeficitForSection;
		obNewUnlockDeltas.SetNegativeValuesToZero();

		// New unlock points is the previous total plus the new gains.
		obNewUnlockPoints += obNewUnlockDeltas;

		// Do the unlocking
		CheckpointManager::Get().UnlockCombos( obNewUnlockPoints, true );

		// Effect events style change
		m_pobHitCounter->AddStylePoints( iChangeInStyle );
		
		/*	// As we no longer have negative style events this code should not be called
			// Commented out just to be extra sure		
		
		// Effect events style change
		int iRemainingStylePoints = m_pobHitCounter->AddStylePoints( iChangeInStyle );
		
		if ( iRemainingStylePoints < 0 )
		{
			// Style Loss not absorbed by stylebar - take off lifeclock
			m_pobLifeClock->DecrementLifeClock( (float)- iRemainingStylePoints );
		}*/

		// Add event to the event rendering list
		m_aobStyleEventRenderList.push_back( pobBestEvent );

		// Restart the cooldown timer
		m_fCurrEventTime += m_pobDefinition->m_fEventCooldownTime;
	}

	// Debug style points display
#ifdef _STYLEPOINT_DEBUG
	g_VisualDebug->Printf2D(200.0f, 40.0f,  DC_GREEN, 0, "Speed   %i", m_obStyleStats.m_obStylePointsForSection.m_iStylePointsSpeed );
	g_VisualDebug->Printf2D(200.0f, 52.0f,  DC_GREEN, 0, "Power   %i", m_obStyleStats.m_obStylePointsForSection.m_iStylePointsPower );
	g_VisualDebug->Printf2D(200.0f, 64.0f,  DC_GREEN, 0, "Range   %i", m_obStyleStats.m_obStylePointsForSection.m_iStylePointsRange );
	g_VisualDebug->Printf2D(200.0f, 78.0f,  DC_GREEN, 0, "Aerial  %i", m_obStyleStats.m_obStylePointsForSection.m_iStylePointsAerial );
	g_VisualDebug->Printf2D(200.0f, 90.0f,  DC_GREEN, 0, "Misc    %i", m_obStyleStats.m_obStylePointsForSection.m_iStylePointsMisc );
	g_VisualDebug->Printf2D(200.0f, 102.0f, DC_GREEN, 0, "Overall %i", m_obStyleStats.m_obStylePointsForSection.m_iStylePointsOverall );
#endif //_STYLEPOINT_DEBUG
}


//------------------------------------------------------------------------------------------
//!
//!	StyleManager::RegisterStyleEvent( CHashedString obEvent )
//!	Register a style event with the style system.
//! Takes style to add from the StyleEventDef's default time.
//!
//------------------------------------------------------------------------------------------
void StyleManager::RegisterStyleEvent(CHashedString obEvent, STYLE_TYPE eStyleType)
{
	StyleEventDef* pobEventDef = GetEventDef( obEvent );
	if ( pobEventDef && pobEventDef->m_iDefaultStyleValue != 0)
	{
		StyleEvent* pobNewEvent = NT_NEW_CHUNK( Mem::MC_ENTITY ) StyleEvent( pobEventDef, pobEventDef->m_iDefaultStyleValue, eStyleType );
		m_aobStyleEventQue.push_back ( pobNewEvent );
	}
}

//------------------------------------------------------------------------------------------
//!
//!	StyleManager::RegisterStyleEvent( CHashedString obEvent, int iStyle, STYLE_TYPE eStyleType )
//!	Register a style event with the style system.
//!
//------------------------------------------------------------------------------------------
void StyleManager::RegisterStyleEvent(CHashedString obEvent, int iStyle, STYLE_TYPE eStyleType)
{
	StyleEventDef* pobEventDef = GetEventDef( obEvent );
	if ( pobEventDef && iStyle != 0)
	{
		StyleEvent* pobNewEvent = NT_NEW_CHUNK( Mem::MC_ENTITY ) StyleEvent( pobEventDef, iStyle, eStyleType );
		m_aobStyleEventQue.push_back ( pobNewEvent );
	}
}

//------------------------------------------------------------------------------------------
//!
//!	StyleManager::GetEventDef( obEvent )
//!	Get the event def named in obEvent
//!
//------------------------------------------------------------------------------------------
StyleEventDef* StyleManager::GetEventDef(CHashedString obEvent)
{
	ntAssert( ObjectDatabase::Exists() );
	return ObjectDatabase::Get().GetPointerFromName<StyleEventDef*>( obEvent );
}

//------------------------------------------------------------------------------------------
//!
//!	StyleManager::GetRenderableEvent( obEvent )
//!	Get any renderable events
//!
//------------------------------------------------------------------------------------------
bool  StyleManager::GetRenderableEvent(StyleEvent& obRenderableEvent)
{
	if ( !m_aobStyleEventRenderList.empty() )
	{
		StyleEvent* pobEvent = m_aobStyleEventRenderList.front();
		m_aobStyleEventRenderList.pop_front();	

		ntAssert ( pobEvent );
		obRenderableEvent = *pobEvent;
		NT_DELETE_CHUNK ( Mem::MC_ENTITY, pobEvent );
		return true;
	}
	else
	{
		return false;
	}
}

//------------------------------------------------------------------------------------------
//!
//!	StyleManager::HasRenderableEvent( void )
//!	Get any renderable events
//!
//------------------------------------------------------------------------------------------
bool  StyleManager::HasRenderableEvent( void )
{
	return ( !m_aobStyleEventRenderList.empty() );
}

//------------------------------------------------------------------------------------------
//!
//!	StyleManager::GetCurrentStylePoints( StylePoints& rNewPoints )
//!	Get the current amount of points the player has.
//!
//------------------------------------------------------------------------------------------
void StyleManager::GetCurrentStylePoints( StanceStylePoints& rNewPoints )
{
	// New Points = Previous Totals + Positive Gains In Each Stance
	rNewPoints = m_obStyleStats.m_obCachedStylePointTotals;
	// Calculate the positive gains for each stance, removing any negative values (we don't want to relock moves!)
	StanceStylePoints obNewUnlockDeltas = m_obStyleStats.m_obStylePointsForSection - m_obStyleStats.m_obStylePointsDeficitForSection;
	obNewUnlockDeltas.SetNegativeValuesToZero();
	// New unlock points is the previous total plus the new gains.
	rNewPoints += obNewUnlockDeltas;
}


//------------------------------------------------------------------------------------------
//!
//!	StanceStylePoints::operator -
//!	- operator
//!
//------------------------------------------------------------------------------------------
StanceStylePoints StanceStylePoints::operator - ( const StanceStylePoints& obOther )
{
	StanceStylePoints obResult;

	obResult.m_iStylePointsAerial = m_iStylePointsAerial - obOther.m_iStylePointsAerial;
	obResult.m_iStylePointsPower = m_iStylePointsPower - obOther.m_iStylePointsPower;
	obResult.m_iStylePointsRange = m_iStylePointsRange - obOther.m_iStylePointsRange;
	obResult.m_iStylePointsSpeed = m_iStylePointsSpeed - obOther.m_iStylePointsSpeed;
	obResult.m_iStylePointsMisc = m_iStylePointsMisc - obOther.m_iStylePointsMisc;
	obResult.m_iStylePointsOverall = m_iStylePointsOverall - obOther.m_iStylePointsOverall;

	return obResult;
}


//------------------------------------------------------------------------------------------
//!
//!	StanceStylePoints::operator +
//!	+ operator
//!
//------------------------------------------------------------------------------------------
StanceStylePoints StanceStylePoints::operator + ( const StanceStylePoints& obOther )
{
	StanceStylePoints obResult;

	obResult.m_iStylePointsAerial = m_iStylePointsAerial + obOther.m_iStylePointsAerial;
	obResult.m_iStylePointsPower = m_iStylePointsPower + obOther.m_iStylePointsPower;
	obResult.m_iStylePointsRange = m_iStylePointsRange + obOther.m_iStylePointsRange;
	obResult.m_iStylePointsSpeed = m_iStylePointsSpeed + obOther.m_iStylePointsSpeed;
	obResult.m_iStylePointsMisc = m_iStylePointsMisc + obOther.m_iStylePointsMisc;
	obResult.m_iStylePointsOverall = m_iStylePointsOverall + obOther.m_iStylePointsOverall;

	return obResult;
}


//------------------------------------------------------------------------------------------
//!
//!	StanceStylePoints::operator -=
//!	-= operator
//!
//------------------------------------------------------------------------------------------
StanceStylePoints& StanceStylePoints::operator -= ( const StanceStylePoints& obOther )
{
	m_iStylePointsAerial -= obOther.m_iStylePointsAerial;
	m_iStylePointsOverall -= obOther.m_iStylePointsOverall;
	m_iStylePointsPower -= obOther.m_iStylePointsPower;
	m_iStylePointsRange -= obOther.m_iStylePointsRange;
	m_iStylePointsSpeed -= obOther.m_iStylePointsSpeed;
	m_iStylePointsMisc -= obOther.m_iStylePointsMisc;

	return *this;
}


//------------------------------------------------------------------------------------------
//!
//!	StanceStylePoints::operator +=
//!	+= operator
//!
//------------------------------------------------------------------------------------------
StanceStylePoints& StanceStylePoints::operator += ( const StanceStylePoints& obOther )
{
	m_iStylePointsAerial += obOther.m_iStylePointsAerial;
	m_iStylePointsOverall += obOther.m_iStylePointsOverall;
	m_iStylePointsPower += obOther.m_iStylePointsPower;
	m_iStylePointsRange += obOther.m_iStylePointsRange;
	m_iStylePointsSpeed += obOther.m_iStylePointsSpeed;
	m_iStylePointsMisc += obOther.m_iStylePointsMisc;

	return *this;
}


//------------------------------------------------------------------------------------------
//!
//!	StanceStylePoints::Negate( void )
//!	Negates the values.
//!
//------------------------------------------------------------------------------------------
void StanceStylePoints::Negate( void )
{
	m_iStylePointsSpeed = -m_iStylePointsSpeed;
	m_iStylePointsPower = -m_iStylePointsPower;
	m_iStylePointsRange = -m_iStylePointsRange;
	m_iStylePointsAerial = -m_iStylePointsAerial;
	m_iStylePointsMisc = -m_iStylePointsMisc;

	m_iStylePointsOverall = -m_iStylePointsOverall;
}


//------------------------------------------------------------------------------------------
//!
//!	StanceStylePoints::SetNegativeValuesToZero( void )
//!	Sets any negative values to zero.  Useful for when wanting to add on the positive deltas.
//!
//------------------------------------------------------------------------------------------
void StanceStylePoints::SetNegativeValuesToZero()
{
	if ( m_iStylePointsSpeed < 0 )	{ m_iStylePointsSpeed = 0; }
	if ( m_iStylePointsPower < 0 )	{ m_iStylePointsPower = 0; }
	if ( m_iStylePointsRange < 0 )	{ m_iStylePointsRange = 0; }
	if ( m_iStylePointsAerial < 0 ) { m_iStylePointsAerial = 0; }
	if ( m_iStylePointsMisc < 0 )	{ m_iStylePointsMisc = 0; }
	if ( m_iStylePointsOverall < 0 ){ m_iStylePointsOverall = 0; }
}


//------------------------------------------------------------------------------------------
//!
//!	StanceStylePoints::CompareAndStoreHighest
//!	Compares the two sets of style points and stores the highest values for each stance
//!
//------------------------------------------------------------------------------------------
void StanceStylePoints::CompareAndStoreHighest( const StanceStylePoints& obPoints1, const StanceStylePoints& obPoints2 )
{
	// Speed
	if ( obPoints1.m_iStylePointsSpeed > obPoints2.m_iStylePointsSpeed )
		m_iStylePointsSpeed = obPoints1.m_iStylePointsSpeed;
	else
		m_iStylePointsSpeed = obPoints2.m_iStylePointsSpeed;

	// Power
	if ( obPoints1.m_iStylePointsPower > obPoints2.m_iStylePointsPower )
		m_iStylePointsPower = obPoints1.m_iStylePointsPower;
	else
		m_iStylePointsPower = obPoints2.m_iStylePointsPower;

	// Range
	if ( obPoints1.m_iStylePointsRange > obPoints2.m_iStylePointsRange )
		m_iStylePointsRange = obPoints1.m_iStylePointsRange;
	else
		m_iStylePointsRange = obPoints2.m_iStylePointsRange;

	// Aerial
	if ( obPoints1.m_iStylePointsAerial > obPoints2.m_iStylePointsAerial )
		m_iStylePointsAerial = obPoints1.m_iStylePointsAerial;
	else
		m_iStylePointsAerial = obPoints2.m_iStylePointsAerial;

	// Misc
	if ( obPoints1.m_iStylePointsMisc > obPoints2.m_iStylePointsMisc )
		m_iStylePointsMisc = obPoints1.m_iStylePointsMisc;
	else
		m_iStylePointsMisc = obPoints2.m_iStylePointsMisc;

	// Overall
	if ( obPoints1.m_iStylePointsOverall > obPoints2.m_iStylePointsOverall )
		m_iStylePointsOverall = obPoints1.m_iStylePointsOverall;
	else
		m_iStylePointsOverall = obPoints2.m_iStylePointsOverall;
}


//------------------------------------------------------------------------------------------
//!
//!	StyleStats::DoSectionTotals(  )
//!	Somewhere to collect together all the updates we want to do on the stats
//! when the player activates a check point
//!
//------------------------------------------------------------------------------------------
void StyleStats::DoSectionTotals( void )
{
	m_iTotalKills += m_iKills;
	m_iKills = 0;

	// (chipb) Clear interactive music related stats
	m_iSuccessfulBlocks = 0;
	m_iUnsuccessfulBlocks = 0;
	m_iGotKOs = 0;
	m_iCausedKOs = 0;
	m_bSuperStyleActive = false;

	m_dTotalTimePlayed += m_fTimePlayed;
	m_fTimePlayed = 0.0f;
}
