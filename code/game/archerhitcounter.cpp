//------------------------------------------------------------------------------------------
//!
//!	\file archerhitcounter.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "game/archerhitcounter.h"
#include "game/combatstyle.h"
#include "game/messagehandler.h"
#include "game/entity.h"
#include "game/entityboss.h"
#include "game/entity.inl"
#include "game/luaexptypes.h"
#include "objectdatabase/dataobject.h"
#include "hud/hudmanager.h"

#include "game/attacks.h"
#include "game/shellconfig.h"

// For the current basis output
#include "core/visualdebugger.h"


// Foward declarations

// Interfaces
START_STD_INTERFACE( ArcherHitCounterDef )
	PUBLISH_PTR_AS( m_pobStylePoints, StylePoints )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fTimeout, 5.0f, Timeout )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bIveStartedSoIllFinish, true, IveStartedSoIllFinish )
END_STD_INTERFACE

// Interfaces

/***************************************************************************************************
* Start exposing the element to Lua
***************************************************************************************************/

/*LUA_EXPOSED_START(ArcherHitCounter)

	LUA_EXPOSED_METHOD(ResetHistory,		ResetHistory,      "", "", "") 
	LUA_EXPOSED_METHOD(TestStrikeHistory,	TestStrikeHistory, "", "", "") 
	LUA_EXPOSED_METHOD(TestStruckHistory,	TestStruckHistory, "", "", "") 

LUA_EXPOSED_END(ArcherHitCounter)*/


//------------------------------------------------------------------------------------------
//!
//!	ArcherHitCounter::ArcherHitCounter
//! Construction
//!
//------------------------------------------------------------------------------------------
ArcherHitCounter::ArcherHitCounter( ArcherHitCounterDef* obDef )
:	m_pobDefinition( obDef ),
	m_fTimeSinceLastHit( 0.0f ),
	m_iHitCount( 0 ),
	m_iLastHitCount( -1 ),
	m_fTimeSinceLastHitCountDisplayed( 0.0f ),
	m_bDisplayLastHitCount( false ),
	m_iCharacterAttacking( 0 ),
	m_HistoryIndex(0),
	m_HistorySize(0)
{
	//ATTACH_LUA_INTERFACE(ArcherHitCounter);
	// Create a new combat event log of what we're interested in, it'll get plugged in by the attack component
	m_pobCombatEventLog = NT_NEW CombatEventLog( -1 );
}

//------------------------------------------------------------------------------------------
//!
//!	ArcherHitCounter::~ArcherHitCounter
//! Destruction
//!
//------------------------------------------------------------------------------------------
ArcherHitCounter::~ArcherHitCounter( void )
{
	NT_DELETE( m_pobCombatEventLog );
}

//------------------------------------------------------------------------------------------
//!
//!	ArcherHitCounter::Update
//! 
//!
//------------------------------------------------------------------------------------------
void ArcherHitCounter::Update( float fTimeStep )
{	
	ntAssert ( m_pobDefinition );

	if (m_bDisplayLastHitCount) 
		m_fTimeSinceLastHitCountDisplayed += fTimeStep;

	if (m_bDisplayLastHitCount && m_fTimeSinceLastHitCountDisplayed > 3.0f)
	{
		m_bDisplayLastHitCount = false;
		m_fTimeSinceLastHitCountDisplayed = 0.0f;
	}

	// Increment the timer
	if ( m_iHitCount > 0 )
		m_fTimeSinceLastHit += fTimeStep;

	// If we have passed our timeout we may need to reset our counter
	if ( m_fTimeSinceLastHit > m_pobDefinition->m_fTimeout ) 
	{
		// If we are using the 'I've started so i'll finish rule we have complications for reset time
		if ( m_pobDefinition->m_bIveStartedSoIllFinish )
		{
			// Only reset when we are not in the middle of an attack
			if ( m_iCharacterAttacking == 0 )
				ResetHitCount();
		}

		// Otherwise we just do it
		else
		{
			ResetHitCount();
		}
	}

	// Loop through combat events and collect style points
	int iCombatEventCount = m_pobCombatEventLog->GetEventCount();
	if (m_pobDefinition->m_pobStylePoints && iCombatEventCount > 0 && StyleManager::Exists() )
	{		
		for (int i = 0; i < iCombatEventCount; i++)
		{
			const CombatEvent* pEvent = m_pobCombatEventLog->GetEvents() + i;
			ntAssert( pEvent );
			//STYLE_TYPE eStyleType;
			//const CAttackData* pAttackData = 0;
			
			HitCounterCharacterMulDef* pobHCMultiplierDef = 0;
			//float fScale = 1.0f;
			
			if( pEvent->m_pobTargetEntity && pEvent->m_pobTargetEntity->GetAttackComponent() &&
				pEvent->m_pobTargetEntity->GetAttackComponent()->GetAttackDefinition() )
			{
				pobHCMultiplierDef = pEvent->m_pobTargetEntity->GetAttackComponent()->GetAttackDefinition()->m_pobHCMultiplierDef;
			}


			switch ( pEvent->m_eEventType )
			{

			///////////////////////////////////
			//		Positive style events    //
			///////////////////////////////////

				case CE_CAUSED_KO:
					// (chipb) Interactive music related stat
					StyleManager::Get().GetStats().m_iCausedKOs++;
				break;
				case CE_CAUSED_KILL:
					StyleManager::Get().GetStats().m_iKills++;
				break;
				case CE_SUCCESSFUL_GRAB:
				break;
				case CE_CAUSED_RECOIL:
				break;
				case CE_CAUSED_IMPACT_STAGGER:
				break;
				case CE_CAUSED_BLOCK_STAGGER:
				break;
				case CE_COUNTER_ATTACK:
				break;
				case CE_EVADED_INCOMING_ATTACK:
				break;
				case CE_STARTED_AERIAL:
				break;
				case CE_COMBO:
				break;
				case CE_GOT_DEFLECT:
					// (chipb) Interactive music related stat
					StyleManager::Get().GetStats().m_iSuccessfulBlocks++;
				break;


			///////////////////////////////////
			//		Negitive style events    //
			///////////////////////////////////
				case CE_GOT_KOED:
					// (chipb) Interactive music related stat
					StyleManager::Get().GetStats().m_iGotKOs++;
				break;

				case CE_GOT_KILLED:
				break;
		
				case CE_GOT_IMPACT_STAGGERED:
					// (chipb) Interactive music related stat
					StyleManager::Get().GetStats().m_iUnsuccessfulBlocks++;
				break;

				case CE_GOT_BLOCK_STAGGERED:
					// (chipb) Interactive music related stat
					StyleManager::Get().GetStats().m_iUnsuccessfulBlocks++;
				break;

				case CE_GOT_RECOILED:
					// (chipb) Interactive music related stat
					StyleManager::Get().GetStats().m_iUnsuccessfulBlocks++;
				break;


			///////////////////////////////////
			//		Other style events       //
			///////////////////////////////////
				case CE_STARTED_SUPERSTYLE:
				break;
				
				case CE_FINISHED_SUPERSTYLE:
				break;


				default:
				break;
			}
		}

		m_pobCombatEventLog->ClearEvents();
	}
}

//------------------------------------------------------------------------------------------
//!
//!	ArcherHitCounter::ResetHitCount
//! Pretty simple really...
//!
//------------------------------------------------------------------------------------------
void ArcherHitCounter::ResetHitCount( void )
{
	// Save hit count for post-display, only if there's a score to save
	if (m_iHitCount != 0)
	{
		m_iLastHitCount = m_iHitCount;
		m_bDisplayLastHitCount = true;
	}

	// Reset the hit count
	m_iHitCount = 0;
}


//------------------------------------------------------------------------------------------
//!
//!	ArcherHitCounter::SuccessfulHit
//! To be called when the owning character has successfully struck an opponent
//!
//------------------------------------------------------------------------------------------
void ArcherHitCounter::SuccessfulHit( void )
{
	// Update the hit count and reset the timer
	++m_iHitCount;
	m_fTimeSinceLastHit = 0.0f;

	// Update the history
	m_History[m_HistoryIndex].m_Type = History::ENTITY_STRIKE;
	m_History[m_HistoryIndex].m_Time = CTimer::Get().GetGameTime();
	
	// Forward the index in the history table
	m_HistoryIndex = (m_HistoryIndex + 1) % HISTORY_ARRAY_SIZE;
	m_HistorySize = ++m_HistorySize > HISTORY_ARRAY_SIZE ? HISTORY_ARRAY_SIZE : m_HistorySize;
}


//------------------------------------------------------------------------------------------
//!
//!	ArcherHitCounter::UnSuccessfulHit
//! To be called when the owning character has successfully struck an opponent
//!
//------------------------------------------------------------------------------------------
void ArcherHitCounter::UnSuccessfulHit( void )
{
	// If a hit has missed and the last successful hit was a while back...
	if ( m_fTimeSinceLastHit > m_pobDefinition->m_fTimeout ) 
		ResetHitCount();
}


//------------------------------------------------------------------------------------------
//!
//!	ArcherHitCounter::PlayerStruck
//! To be called when the owning player has been struck by an opponent
//!
//------------------------------------------------------------------------------------------
void ArcherHitCounter::PlayerStruck( void )
{
	// Update the hit count and reset the timer
	m_iHitCount = 0;
	m_fTimeSinceLastHit = 0.0f;

	// Update the history
	m_History[m_HistoryIndex].m_Type = History::ENTITY_STRUCK;
	m_History[m_HistoryIndex].m_Time = CTimer::Get().GetGameTime();
	
	// Forward the index in the history table
	m_HistoryIndex = (m_HistoryIndex + 1) % HISTORY_ARRAY_SIZE;
	m_HistorySize = ++m_HistorySize > HISTORY_ARRAY_SIZE ? HISTORY_ARRAY_SIZE : m_HistorySize;

}


//------------------------------------------------------------------------------------------
//!
//!	ArcherHitCounter::StrikeStarted
//! To be called when the owning character has started an attack
//!	If the attack system had a 'proper' state system, i.e. official state entry and exit,
//!	then this reference counting would not be necessary.  Sort this out when the state gets
//!	changed - GH
//!
//------------------------------------------------------------------------------------------
void ArcherHitCounter::StrikeStarted( void )
{
	// Increment the attacking reference
	m_iCharacterAttacking++;
}  


//------------------------------------------------------------------------------------------
//!
//!	ArcherHitCounter::StrikeFinished
//! To be called when the owning character has finished a strike
//!	If the attack system had a 'proper' state system, i.e. official state entry and exit,
//!	then this reference counting would not be necessary.  Sort this out when the state gets
//!	changed - GH
//!
//------------------------------------------------------------------------------------------
void ArcherHitCounter::StrikeFinished( void )
{
	// Decrement the attacking reference
	if ( m_iCharacterAttacking > 0 )
		m_iCharacterAttacking--;

	// In attack component a call to StrikeFinished was being followed by a call to StrikeStarted within a single frame
	// before a call to ArcherHitCounter::Update was being done (i.e. if in IveStartedSoIllFinish, there was no timeout check if you 
	// were continually in an attacking state). So, do a check here whenever a strike finishes to eliminate the within 1 frame problem.
	
	// If we have passed our timeout we may need to reset our counter
	if ( m_fTimeSinceLastHit > m_pobDefinition->m_fTimeout ) 
	{
		// If we are using the 'I've started so i'll finish rule we have complications for reset time
		if ( m_pobDefinition->m_bIveStartedSoIllFinish )
		{
			// Only reset when we are not in the middle of an attack
			if ( m_iCharacterAttacking == 0 )
			{
				ResetHitCount();
			}
		}

		// Otherwise we just do it
		else
		{			
			ResetHitCount();
		}
	}
}

//------------------------------------------------------------------------------------------
//!
//!	ArcherHitCounter::TestHistory
//! 
//!
//------------------------------------------------------------------------------------------
int ArcherHitCounter::TestHistory( double WindowStart, double WindowEnd, History::STRIKETYPE eType)
{
	// Cache a local game time reference
	double dGameTime = CTimer::Get().GetGameTime();

	// adjust the windows in to game time domain
	WindowStart += dGameTime;
	WindowEnd += dGameTime;

	// The number of history elements that match
	int Count = 0;

	// Loop through all the history elements. 
	for( int iHistoryCount = m_HistorySize; iHistoryCount; --iHistoryCount )
	{
		History& obHistory = m_History[ (m_HistoryIndex - iHistoryCount) % HISTORY_ARRAY_SIZE ];

		// If this is an element that is fit the description, then increase the counter
		if( obHistory.m_Type == eType && obHistory.m_Time >= WindowStart && obHistory.m_Time <= WindowEnd )
		{
			++Count;
		}

	}

	return Count;
}
