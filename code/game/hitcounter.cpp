//------------------------------------------------------------------------------------------
//!
//!	\file hitcounter.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "game/hitcounter.h"
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

#define E3_SUPERSTYLE_BAR

// Foward declarations

// Interfaces
START_STD_INTERFACE( HitCounterDef )
	PUBLISH_VAR_AS( m_fTimeout, Timeout )
	PUBLISH_VAR_AS( m_iLevelOneSuperStyleThreshold, LevelOneSuperStyleThreshold )
	PUBLISH_VAR_AS( m_iLevelTwoSuperStyleThreshold, LevelTwoSuperStyleThreshold )
	PUBLISH_VAR_AS( m_iLevelThreeSuperStyleThreshold, LevelThreeSuperStyleThreshold )
	PUBLISH_VAR_AS( m_iLevelFourSuperStyleThreshold, LevelFourSuperStyleThreshold )
	PUBLISH_VAR_AS( m_iLevelSpecialSuperStyleThreshold, LevelSpecialSuperStyleThreshold )

	PUBLISH_VAR_AS( m_iLevelOneStyleProgessionThreshold, LevelOneStyleProgessionThreshold )
	PUBLISH_VAR_AS( m_iLevelTwoStyleProgessionThreshold, LevelTwoStyleProgessionThreshold )
	PUBLISH_VAR_AS( m_iLevelThreeStyleProgessionThreshold, LevelThreeStyleProgessionThreshold )
	PUBLISH_VAR_AS( m_iLevelFourStyleProgessionThreshold, LevelFourStyleProgessionThreshold )	
	PUBLISH_VAR_AS( m_iLevelSpecialStyleProgessionThreshold, LevelSpecialStyleProgessionThreshold )

	PUBLISH_PTR_AS( m_pobStylePoints, StylePoints )

	PUBLISH_GLOBAL_ENUM_WITH_DEFAULT_AS		(m_eStartSSLevel, StartSSLevel, HIT_LEVEL, HL_FOUR)

	PUBLISH_VAR_AS( m_bIveStartedSoIllFinish, IveStartedSoIllFinish )
END_STD_INTERFACE

// Interfaces
START_STD_INTERFACE( HitCounterCharacterMulDef )
	PUBLISH_VAR_AS( m_fScalePointsForKO,					ScalePointsForKO)
	PUBLISH_VAR_AS( m_fScalePointsForKill,					ScalePointsForKill)
	PUBLISH_VAR_AS( m_fScalePointsForGrab,					ScalePointsForGrab)
	PUBLISH_VAR_AS( m_fScalePointsForRecoil,				ScalePointsForRecoil)
	PUBLISH_VAR_AS( m_fScalePointsForImpactStagger,			ScalePointsForImpactStagger)
	PUBLISH_VAR_AS( m_fScalePointsForBlockStagger,			ScalePointsForBlockStagger)
	PUBLISH_VAR_AS( m_fScalePointsForCounterAttack,			ScalePointsForCounterAttack)
	PUBLISH_VAR_AS( m_fScalePointsForEvadeIncomingAttack,	ScalePointsForEvadeIncomingAttack)
	PUBLISH_VAR_AS( m_fScalePointsForAerial,				ScalePointsForAerial)
END_STD_INTERFACE

/***************************************************************************************************
* Start exposing the element to Lua
***************************************************************************************************/

LUA_EXPOSED_START(HitCounter)

	LUA_EXPOSED_METHOD(ResetHistory,		ResetHistory,      "", "", "") 
	LUA_EXPOSED_METHOD(TestStrikeHistory,	TestStrikeHistory, "", "", "") 
	LUA_EXPOSED_METHOD(TestStruckHistory,	TestStruckHistory, "", "", "") 

LUA_EXPOSED_END(HitCounter)


//------------------------------------------------------------------------------------------
//!
//!	HitCounter::HitCounter
//! Construction
//!
//------------------------------------------------------------------------------------------
HitCounter::HitCounter( HitCounterDef* obDef )
:	m_pobDefinition( obDef ),
	m_fTimeSinceLastHit( 0.0f ),
	m_iHitCount( 0 ),
	m_iLastHitCount( -1 ),
	m_fTimeSinceLastHitCountDisplayed( 0.0f ),
	m_bDisplayLastHitCount( false ),
	m_iStylePoints( 0 ),
	m_iStyleProgression( 0 ),
	m_iOldStylePoints( 0 ),
	m_iOldStyleProgression( 0 ),
	m_pobCombatEventLog( 0 ),
	m_eStyleProgressionLevel( HL_ONE ),
	m_iCharacterAttacking( 0 ),
	m_HistoryIndex(0),
	m_HistorySize(0),
	m_bForceActive ( false )
{
	ATTACH_LUA_INTERFACE(HitCounter);

	ActivateStyleProgression( obDef->m_eStartSSLevel );

	m_eOldStyleProgressionLevel = obDef->m_eStartSSLevel;
	m_iCurrentSpecialStyleThreshold = m_pobDefinition->m_iLevelSpecialSuperStyleThreshold;

	// Create a new combat event log of what we're interested in, it'll get plugged in by the attack component
	m_pobCombatEventLog = NT_NEW CombatEventLog( -1 );
    m_pobBoss = 0;
}

//------------------------------------------------------------------------------------------
//!
//!	HitCounter::~HitCounter
//! Destruction
//!
//------------------------------------------------------------------------------------------
HitCounter::~HitCounter( void )
{
	NT_DELETE( m_pobCombatEventLog );

	m_aobEntList.clear();
}


//------------------------------------------------------------------------------------------
//!
//!	HitCounter::Update
//! 
//!
//------------------------------------------------------------------------------------------
void HitCounter::Update( float fTimeStep )
{	
	ntAssert ( m_pobDefinition );

	if (!m_pobBoss)
	{
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
			STYLE_TYPE eStyleType;
			const CAttackData* pAttackData = 0;
			
			HitCounterCharacterMulDef* pobHCMultiplierDef = 0;
			float fScale = 1.0f;
			
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
					fScale = (pobHCMultiplierDef) ? pobHCMultiplierDef->m_fScalePointsForKO : fScale;
					pAttackData = (CAttackData*)pEvent->m_pData;
					ntAssert ( pAttackData );
					eStyleType = pAttackData->GetStyleType();
					StyleManager::Get().RegisterStyleEvent( CHashedString(HASH_STRING_EVENTKO), (int) (fScale * (float)m_pobDefinition->m_pobStylePoints->m_iCausedKO), eStyleType );

					// (chipb) Interactive music related stat
					StyleManager::Get().GetStats().m_iCausedKOs++;
				break;
				case CE_CAUSED_KILL:
					fScale = (pobHCMultiplierDef) ? pobHCMultiplierDef->m_fScalePointsForKill : fScale;
					pAttackData = (CAttackData*)pEvent->m_pData;
					ntAssert ( pAttackData );
					eStyleType = pAttackData->GetStyleType();
					StyleManager::Get().RegisterStyleEvent( CHashedString(HASH_STRING_EVENTKILL), (int) (fScale * (float)m_pobDefinition->m_pobStylePoints->m_iCausedKill), eStyleType );
					StyleManager::Get().GetStats().m_iKills++;
				break;
				case CE_SUCCESSFUL_GRAB:
					fScale = (pobHCMultiplierDef) ? pobHCMultiplierDef->m_fScalePointsForGrab : fScale;
					StyleManager::Get().RegisterStyleEvent( CHashedString(HASH_STRING_EVENTGRAB), (int) (fScale * (float) m_pobDefinition->m_pobStylePoints->m_iSuccessfulGrab) );

					// Switch off the grab hit - so that it can be removed at the start of the paralayis sequence and allow the triangles to be seen
					CHud::Get().SetGrabHint( false );
				break;
				case CE_CAUSED_RECOIL:
					fScale = (pobHCMultiplierDef) ? pobHCMultiplierDef->m_fScalePointsForRecoil : fScale;
					pAttackData = (CAttackData*)pEvent->m_pData;
					ntAssert ( pAttackData );
					eStyleType = pAttackData->GetStyleType();
					StyleManager::Get().RegisterStyleEvent( CHashedString(HASH_STRING_EVENTRECOIL), (int) (fScale * (float)m_pobDefinition->m_pobStylePoints->m_iCausedRecoil), eStyleType );
				break;
				case CE_CAUSED_IMPACT_STAGGER:
					fScale = (pobHCMultiplierDef) ? pobHCMultiplierDef->m_fScalePointsForImpactStagger : fScale;
					pAttackData = (CAttackData*)pEvent->m_pData;
					ntAssert ( pAttackData );
					eStyleType = pAttackData->GetStyleType();
					StyleManager::Get().RegisterStyleEvent( CHashedString(HASH_STRING_EVENTSTAGGER), (int) (fScale * (float)m_pobDefinition->m_pobStylePoints->m_iCausedImpactStagger), eStyleType );
				break;
				case CE_CAUSED_BLOCK_STAGGER:
					fScale = (pobHCMultiplierDef) ? pobHCMultiplierDef->m_fScalePointsForBlockStagger : fScale;
					pAttackData = (CAttackData*)pEvent->m_pData;
					ntAssert ( pAttackData );
					eStyleType = pAttackData->GetStyleType();
					StyleManager::Get().RegisterStyleEvent( CHashedString(HASH_STRING_EVENTBLOCKSTAGGER), (int) (fScale * (float)m_pobDefinition->m_pobStylePoints->m_iCausedBlockStagger), eStyleType );
				break;
				case CE_COUNTER_ATTACK:
					fScale = (pobHCMultiplierDef) ? pobHCMultiplierDef->m_fScalePointsForCounterAttack : fScale;
					pAttackData = (CAttackData*)pEvent->m_pData;
					ntAssert ( pAttackData );
					eStyleType = pAttackData->GetStyleType();
					StyleManager::Get().RegisterStyleEvent( CHashedString(HASH_STRING_EVENTCOUNTER), (int) (fScale * (float)m_pobDefinition->m_pobStylePoints->m_iCounterAttack), eStyleType );
				break;
				case CE_EVADED_INCOMING_ATTACK:
					fScale = (pobHCMultiplierDef) ? pobHCMultiplierDef->m_fScalePointsForEvadeIncomingAttack : fScale;
					StyleManager::Get().RegisterStyleEvent( CHashedString(HASH_STRING_EVENTEVADE), (int) (fScale * (float)m_pobDefinition->m_pobStylePoints->m_iEvadedIncomingAttack) );
				break;
				case CE_STARTED_AERIAL:
					fScale = (pobHCMultiplierDef) ? pobHCMultiplierDef->m_fScalePointsForAerial : fScale;
					StyleManager::Get().RegisterStyleEvent( CHashedString(HASH_STRING_EVENTAERIAL), (int) (fScale * (float)m_pobDefinition->m_pobStylePoints->m_iStartedAerial) );
				break;
				case CE_COMBO:
#ifdef PLATFORM_PS3
					StyleManager::Get().RegisterStyleEvent( CHashedString(HASH_STRING_EVENTCOMBO), (int)(m_pobDefinition->m_pobStylePoints->m_fComboMultiple*(int)((long int)m_pobCombatEventLog->GetEvents()[i].m_pData)) );
#else
					StyleManager::Get().RegisterStyleEvent( CHashedString(HASH_STRING_EVENTCOMBO), (int)(m_pobDefinition->m_pobStylePoints->m_fComboMultiple*(int)(m_pobCombatEventLog->GetEvents()[i].m_pData)) );
#endif
				break;
				case CE_GOT_DEFLECT:
					// (chipb) Interactive music related stat
					StyleManager::Get().GetStats().m_iSuccessfulBlocks++;
				break;


			///////////////////////////////////
			//		Negitive style events    //
			///////////////////////////////////
				case CE_GOT_KOED:
					StyleManager::Get().RegisterStyleEvent( CHashedString(HASH_STRING_EVENTGOTKOED));

					// (chipb) Interactive music related stat
					StyleManager::Get().GetStats().m_iGotKOs++;
				break;

				case CE_GOT_KILLED:
					StyleManager::Get().RegisterStyleEvent( CHashedString(HASH_STRING_EVENTGOTKILLED) );
				break;
		
				case CE_GOT_IMPACT_STAGGERED:
					StyleManager::Get().RegisterStyleEvent( CHashedString(HASH_STRING_EVENTGOTIMPACTSTAGGERED) );

					// (chipb) Interactive music related stat
					StyleManager::Get().GetStats().m_iUnsuccessfulBlocks++;
				break;

				case CE_GOT_BLOCK_STAGGERED:
					StyleManager::Get().RegisterStyleEvent( CHashedString(HASH_STRING_EVENTGOTBLOCKSTAGGERED) );

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
					Used();

					// Disconnect the style system
					if ( !g_ShellOptions->m_bStyleDuringSS )
						StyleManager::Get().SetActive( false );

					// (chipb) Interactive music related stat
					StyleManager::Get().GetStats().m_bSuperStyleActive = true;
				break;
				
				case CE_FINISHED_SUPERSTYLE:
					// Reconnect the style system
					if ( !g_ShellOptions->m_bStyleDuringSS )
						StyleManager::Get().SetActive( true );

					// (chipb) Interactive music related stat
					StyleManager::Get().GetStats().m_bSuperStyleActive = false;
				break;


				default:
				break;
			}
		}

		m_pobCombatEventLog->ClearEvents();
	}

	// Update our style progression level
	if ( m_eStyleProgressionLevel < HL_FOUR && m_iStyleProgression >= m_pobDefinition->m_iLevelFourStyleProgessionThreshold )
	{
		m_eStyleProgressionLevel = HL_FOUR;
		m_iStyleProgression = 0;
	}
	else if ( m_eStyleProgressionLevel < HL_THREE && m_iStyleProgression >= m_pobDefinition->m_iLevelThreeStyleProgessionThreshold )
	{
		m_eStyleProgressionLevel = HL_THREE;
		m_iStyleProgression = 0;
	}
	else if ( m_eStyleProgressionLevel < HL_TWO && m_iStyleProgression >= m_pobDefinition->m_iLevelTwoStyleProgessionThreshold )
	{
		m_eStyleProgressionLevel = HL_TWO;
		m_iStyleProgression = 0;
	}
	else if ( m_eStyleProgressionLevel < HL_ONE && m_iStyleProgression >= m_pobDefinition->m_iLevelOneStyleProgessionThreshold )
	{
		m_eStyleProgressionLevel = HL_ONE;
		m_iStyleProgression = 0;
	}
}
	else
	{
		// Don't need any of these
		m_pobCombatEventLog->ClearEvents();

		m_iStylePoints = (int)(m_pobBoss->GetStartHealth() - m_pobBoss->GetCurrHealth());

		// We're special all the time we're in boss mode 
		if (m_iStylePoints < ( m_pobBoss->GetStartHealth() - m_pobDefinition->m_iBossSpecialThreshold ) )
			m_eStyleProgressionLevel = HL_ZERO;
		else
			m_eStyleProgressionLevel = HL_SPECIAL;
	}
}

//------------------------------------------------------------------------------------------
//!
//!	HitCounter::AddStylePoints(int iPoints)
//! Adds on our style points
//!
//------------------------------------------------------------------------------------------
int HitCounter::AddStylePoints(int iPoints)
{
	if (m_pobBoss)
		return 0;

	m_iStylePoints += iPoints; 

	// Progression only increases
	if ( iPoints >= 0 )
		m_iStyleProgression += iPoints; 

#ifdef E3_SUPERSTYLE_BAR
	// Capped style points for E3
	if (GetCurrentStyleProgressionLevel() == HL_SPECIAL )
	{
		if ( m_iStylePoints > m_iCurrentSpecialStyleThreshold )
		{
			m_iStylePoints = m_iCurrentSpecialStyleThreshold;
		}
		// Probably dont care about this as already prograssed as far as we can go
		if ( m_iStyleProgression > m_pobDefinition->m_iLevelSpecialStyleProgessionThreshold )
		{
			m_iStyleProgression = m_pobDefinition->m_iLevelSpecialStyleProgessionThreshold;
		}	
	}
	else
	{
		if ( m_iStylePoints > m_pobDefinition->m_iLevelFourSuperStyleThreshold )
		{
			m_iStylePoints = m_pobDefinition->m_iLevelFourSuperStyleThreshold;
		}
		if ( m_iStyleProgression > m_pobDefinition->m_iLevelFourStyleProgessionThreshold )
		{
			m_iStyleProgression = m_pobDefinition->m_iLevelFourStyleProgessionThreshold;
		}
	}
#endif

	// We can now loose style points too
	if ( m_iStylePoints < 0 )
	{
		int iRemainingStyleLoss = m_iStylePoints;
		m_iStylePoints = 0;
		return iRemainingStyleLoss;
	}
	else
	{
		return 0;
	}
}

//------------------------------------------------------------------------------------------
//!
//!	HitCounter::Used( void )
//! To be called when a player has used the count for reward
//!
//------------------------------------------------------------------------------------------
void HitCounter::Used( void ) 
{
	if (m_pobBoss)
		return;

	if ( g_ShellOptions->m_bFullSuperStyleCost )
	{
		if ( StyleManager::Exists() && !StyleManager::Get().IsPrologMode() )
		{
			ntAssert ( StyleManager::Get().GetLifeClock() );
			StyleManager::Get().GetLifeClock()->IncrementLifeClock( (float)m_iStylePoints );
		}
		m_iStylePoints = 0; 
		ResetHitCount();
	}
	else
	{
		int iSSCost = 0;

		switch ( GetCurrentHitLevel() )
		{
		case HL_ZERO:					
			break;	

		case HL_ONE: 
			iSSCost = m_pobDefinition->m_iLevelOneSuperStyleThreshold;	
			break;
		
		case HL_TWO:				
			iSSCost = m_pobDefinition->m_iLevelTwoSuperStyleThreshold - m_pobDefinition->m_iLevelOneSuperStyleThreshold;	
			break;

		case HL_THREE: 
			iSSCost = m_pobDefinition->m_iLevelThreeSuperStyleThreshold - m_pobDefinition->m_iLevelTwoSuperStyleThreshold;
			break;

		case HL_FOUR:		
			iSSCost = m_pobDefinition->m_iLevelFourSuperStyleThreshold - m_pobDefinition->m_iLevelThreeSuperStyleThreshold;
			break;

		case HL_SPECIAL:		
			iSSCost = m_iStylePoints;
			break;

		default:			
			ntAssert( 0 );		
			break;
		}

		if ( StyleManager::Exists() && !StyleManager::Get().IsPrologMode() )
		{
			ntAssert ( StyleManager::Get().GetLifeClock() );
			StyleManager::Get().GetLifeClock()->IncrementLifeClock( (float)iSSCost );
		}

		m_iStylePoints -= iSSCost;

		if (m_iStylePoints < 0)
			m_iStylePoints = 0; 
		ResetHitCount();
	}
}

//------------------------------------------------------------------------------------------
//!
//!	HitCounter::GetNextProgressionThreshold( HIT_LEVEL obHitLevel )
//! Returns the number of points needed for a particular level
//!
//------------------------------------------------------------------------------------------
int HitCounter::GetSuperStyleThreshold( HIT_LEVEL obHitLevel )
{
	if (!m_pobBoss)
	{
	int iStyleThreshold;

	switch ( obHitLevel )
	{
	case HL_SPECIAL:
		iStyleThreshold = m_iCurrentSpecialStyleThreshold;
		break;

	case HL_FOUR:
		iStyleThreshold = m_pobDefinition->m_iLevelFourSuperStyleThreshold;
		break;

	case HL_THREE:
		iStyleThreshold = m_pobDefinition->m_iLevelThreeSuperStyleThreshold;
		break;

	case HL_TWO:
		iStyleThreshold = m_pobDefinition->m_iLevelTwoSuperStyleThreshold;
		break;

	case HL_ONE:
		iStyleThreshold = m_pobDefinition->m_iLevelOneSuperStyleThreshold;
		break;

	case HL_ZERO:
		iStyleThreshold = 0;
		break;

	default:
		iStyleThreshold = -1;
		break;
	}

	return iStyleThreshold;
}
	else
	{
		return (int)m_pobBoss->GetStartHealth();
	}
}

//------------------------------------------------------------------------------------------
//!
//!	HitCounter::GetNextProgressionThreshold
//! Returns the number of points needed until another superstyle reached
//!
//------------------------------------------------------------------------------------------
int HitCounter::GetNextSuperStyleThreshold()
{	
	if (m_pobBoss)
	{
		return (int)m_pobBoss->GetStartHealth();
	}
	else if (GetCurrentStyleProgressionLevel() == HL_SPECIAL )
	{
		if (GetCurrentHitLevel() == HL_SPECIAL)
			return -1;	// Already have special - we don't have super extra special superstyles yet!
		else
			return m_iCurrentSpecialStyleThreshold;
	}
	else
	{
		if (GetCurrentHitLevel() == HL_FOUR || m_eStyleProgressionLevel == GetCurrentHitLevel())
			return -1; // Already at the top!
		if (GetCurrentHitLevel() == HL_THREE)
			return m_pobDefinition->m_iLevelFourSuperStyleThreshold;
		if (GetCurrentHitLevel() == HL_TWO)
			return m_pobDefinition->m_iLevelThreeSuperStyleThreshold;
		if (GetCurrentHitLevel() == HL_ONE)
			return m_pobDefinition->m_iLevelTwoSuperStyleThreshold;
		if (GetCurrentHitLevel() == HL_ZERO)
			return m_pobDefinition->m_iLevelOneSuperStyleThreshold;
	}	

	// Bit of an error if we're here
	return -1;
}

//------------------------------------------------------------------------------------------
//!
//!	HitCounter::GetCurrentProgressionThreshold
//! Returns the number of points accounted for in the current super style level
//!
//------------------------------------------------------------------------------------------
int HitCounter::GetCurrentSuperStyleThreshold()
{	
	if (m_pobBoss)
	{
		return 0;
	}
	else if (GetCurrentStyleProgressionLevel() == HL_SPECIAL )
	{
		if (GetCurrentHitLevel() == HL_SPECIAL)
			return m_iCurrentSpecialStyleThreshold;
		else
			return 0;
	}
	else
	{
		if (GetCurrentHitLevel() == HL_FOUR)
			return m_pobDefinition->m_iLevelFourSuperStyleThreshold;
		if (GetCurrentHitLevel() == HL_THREE)
			return m_pobDefinition->m_iLevelThreeSuperStyleThreshold;
		if (GetCurrentHitLevel() == HL_TWO)
			return m_pobDefinition->m_iLevelTwoSuperStyleThreshold;
		if (GetCurrentHitLevel() == HL_ONE)
			return m_pobDefinition->m_iLevelOneSuperStyleThreshold;
		if (GetCurrentHitLevel() == HL_ZERO)
			return 0;
	}	

	// Bit of an error if we're here
	return -1;
}

//------------------------------------------------------------------------------------------
//!
//!	HitCounter::ActivateStyleProgression
//! Moves to a specific Style Progression level
//!
//------------------------------------------------------------------------------------------
bool HitCounter::ActivateStyleProgression( HIT_LEVEL StyleLevel)
{
	m_pobBoss = 0;

	m_eStyleProgressionLevel = StyleLevel;
	switch ( StyleLevel )
	{
	case HL_SPECIAL:
		m_iStyleProgression = m_pobDefinition->m_iLevelSpecialStyleProgessionThreshold;
		break;

	case HL_FOUR:
		m_iStyleProgression = m_pobDefinition->m_iLevelFourStyleProgessionThreshold;
		break;

	case HL_THREE:
		m_iStyleProgression = m_pobDefinition->m_iLevelThreeStyleProgessionThreshold;
		break;

	case HL_TWO:
		m_iStyleProgression = m_pobDefinition->m_iLevelTwoStyleProgessionThreshold;
		break;

	case HL_ONE:
		m_iStyleProgression = m_pobDefinition->m_iLevelOneStyleProgessionThreshold;
		break;

	case HL_ZERO:
		m_iStyleProgression = 0;
		break;
	}

	return true;
}

//------------------------------------------------------------------------------------------
//!
//!	HitCounter::ActivateSpecialStyleLevel
//! Activates the special style level for boss encounters and paralysis arena
//!
//------------------------------------------------------------------------------------------
void HitCounter::ActivateSpecialStyleLevel( void )
{
	user_warn_p( m_eStyleProgressionLevel != HL_SPECIAL, ("ActivateSpecialStyleLevel: Special level already active. May result in unexpected behaviour.\n") );

	m_iOldStylePoints			= m_iStylePoints;
	m_iOldStyleProgression		= m_iStyleProgression;
	m_eOldStyleProgressionLevel	= m_eStyleProgressionLevel;

	m_iStylePoints = 0;

	ActivateStyleProgression( HL_SPECIAL );
}

//------------------------------------------------------------------------------------------
//!
//!	HitCounter::ActivateSpecialStyleLevel
//! Activates the special style level for boss encounters and paralysis arena
//!
//------------------------------------------------------------------------------------------
void HitCounter::DeactivateSpecialStyleLevel( void )
{
	user_warn_p( m_eStyleProgressionLevel == HL_SPECIAL, ("DeactivateSpecialStyleLevel: Special level is not active. May result in unexpected behaviour.\n") );

	m_iStylePoints				= m_iOldStylePoints;
	m_eStyleProgressionLevel	= m_eOldStyleProgressionLevel;

	ActivateStyleProgression( m_eStyleProgressionLevel );

	// Here so we keep any progression gained beyond the activated level
	m_iStyleProgression = m_iOldStyleProgression;
}

//------------------------------------------------------------------------------------------
//!
//!	HitCounter::GetNextProgressionThreshold
//! Returns the number of points needed until another superstyle reached
//!
//------------------------------------------------------------------------------------------
int HitCounter::GetNextStyleProgressionThreshold()
{
	if (!m_pobBoss)
	{
	if (m_eStyleProgressionLevel == HL_SPECIAL)
		return -1; // Already have special - we don't have super extra special superstyles yet!

	if (m_eStyleProgressionLevel == HL_FOUR)
		return -1; // Already at the top!
	if (m_eStyleProgressionLevel == HL_THREE)
		return m_pobDefinition->m_iLevelFourStyleProgessionThreshold;
	if (m_eStyleProgressionLevel == HL_TWO)
		return m_pobDefinition->m_iLevelThreeStyleProgessionThreshold;
	if (m_eStyleProgressionLevel == HL_ONE)
		return m_pobDefinition->m_iLevelTwoStyleProgessionThreshold;
	if (m_eStyleProgressionLevel == HL_ZERO)
		return m_pobDefinition->m_iLevelOneStyleProgessionThreshold;

	// Bit of an error if we're here
	return -1;
}
	else
	{
		if ( m_eStyleProgressionLevel == HL_SPECIAL )
			return -1;
		else
			return (int)m_pobBoss->GetStartHealth();
	}
}

//------------------------------------------------------------------------------------------
//!
//!	HitCounter::ResetHitCount
//! Pretty simple really...
//!
//------------------------------------------------------------------------------------------
void HitCounter::ResetHitCount( void )
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
//!	HitCounter::SuccessfulHit
//! To be called when the owning character has successfully struck an opponent
//!
//------------------------------------------------------------------------------------------
void HitCounter::SuccessfulHit( void )
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
//!	HitCounter::UnSuccessfulHit
//! To be called when the owning character has successfully struck an opponent
//!
//------------------------------------------------------------------------------------------
void HitCounter::UnSuccessfulHit( void )
{
	// If a hit has missed and the last successful hit was a while back...
	if ( m_fTimeSinceLastHit > m_pobDefinition->m_fTimeout ) 
		ResetHitCount();
}


//------------------------------------------------------------------------------------------
//!
//!	HitCounter::PlayerStruck
//! To be called when the owning player has been struck by an opponent
//!
//------------------------------------------------------------------------------------------
void HitCounter::PlayerStruck( void )
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
//!	HitCounter::StrikeStarted
//! To be called when the owning character has started an attack
//!	If the attack system had a 'proper' state system, i.e. official state entry and exit,
//!	then this reference counting would not be necessary.  Sort this out when the state gets
//!	changed - GH
//!
//------------------------------------------------------------------------------------------
void HitCounter::StrikeStarted( void )
{
	// Increment the attacking reference
	m_iCharacterAttacking++;
}  


//------------------------------------------------------------------------------------------
//!
//!	HitCounter::StrikeFinished
//! To be called when the owning character has finished a strike
//!	If the attack system had a 'proper' state system, i.e. official state entry and exit,
//!	then this reference counting would not be necessary.  Sort this out when the state gets
//!	changed - GH
//!
//------------------------------------------------------------------------------------------
void HitCounter::StrikeFinished( void )
{
	// Decrement the attacking reference
	if ( m_iCharacterAttacking > 0 )
		m_iCharacterAttacking--;

	// In attack component a call to StrikeFinished was being followed by a call to StrikeStarted within a single frame
	// before a call to HitCounter::Update was being done (i.e. if in IveStartedSoIllFinish, there was no timeout check if you 
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
//!	HitCounter::GetCurrentHitLevel
//! Get the current 'hit level' according to the scripted thresholds.
//!
//------------------------------------------------------------------------------------------
HIT_LEVEL HitCounter::GetCurrentHitLevel( void ) const
{
	return GetHitLevel( m_iStylePoints );
}

HIT_LEVEL HitCounter::GetHitLevel( int iStylePoints ) const
{
	if (m_pobBoss)
	{
		// Bit of a cheat, when we're in boss mode, we just store current hit level in m_eStyleProgressionLevel
		// It's updated each frame to track the bosses health
		return m_eStyleProgressionLevel;
	}
	else if (GetCurrentStyleProgressionLevel() == HL_SPECIAL )
	{
		if ( iStylePoints >= m_iCurrentSpecialStyleThreshold)
			return HL_SPECIAL;
		else
			return HL_ZERO;
	}
	else
	{	
		if ( iStylePoints >= m_pobDefinition->m_iLevelFourSuperStyleThreshold && GetCurrentStyleProgressionLevel() >= HL_FOUR )
			return HL_FOUR;
		else if ( iStylePoints >= m_pobDefinition->m_iLevelThreeSuperStyleThreshold && GetCurrentStyleProgressionLevel() >= HL_THREE )
			return HL_THREE;
		else if ( iStylePoints >= m_pobDefinition->m_iLevelTwoSuperStyleThreshold && GetCurrentStyleProgressionLevel() >= HL_TWO )
			return HL_TWO;
		else if ( iStylePoints >= m_pobDefinition->m_iLevelOneSuperStyleThreshold && GetCurrentStyleProgressionLevel() >= HL_ONE )
			return HL_ONE;
		else
			return HL_ZERO;
	}
}

HIT_LEVEL HitCounter::GetCurrentStyleProgressionLevel( void ) const
{
	return m_eStyleProgressionLevel;	
}

//------------------------------------------------------------------------------------------
//!
//!	HitCounter::TestHistory
//! 
//!
//------------------------------------------------------------------------------------------
int HitCounter::TestHistory( double WindowStart, double WindowEnd, History::STRIKETYPE eType)
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

//------------------------------------------------------------------------------------------
//!
//!	HitCounter::SetSpecialStyleThreshold
//!
//------------------------------------------------------------------------------------------
void HitCounter::SetSpecialStyleThreshold ( int iThreshold )
{
	m_iCurrentSpecialStyleThreshold = iThreshold;
}

//------------------------------------------------------------------------------------------
//!
//!	HitCounter::ResetSpecialStyleThreshold
//!
//------------------------------------------------------------------------------------------
void HitCounter::ResetSpecialStyleThreshold ( void )
{
	m_iCurrentSpecialStyleThreshold = m_pobDefinition->m_iLevelSpecialSuperStyleThreshold;
}

//------------------------------------------------------------------------------------------
//!
//!	HitCounter::RegisterHintEntity
//!
//------------------------------------------------------------------------------------------
void HitCounter::RegisterHintEntity ( CEntity* pobEnt )
{
	if ( pobEnt )
	{
		m_aobEntList.push_back( pobEnt );
	}
}

//------------------------------------------------------------------------------------------
//!
//!	HitCounter::ClearHintEntities
//!
//------------------------------------------------------------------------------------------
void HitCounter::ClearHintEntities ( void )
{
	m_aobEntList.clear();
}
