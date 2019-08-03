/***************************************************************************************************
*
*	DESCRIPTION		Deals with the management of special moves
*
*	NOTES
*
***************************************************************************************************/

// Necessary includes
#include "special.h"
#include "timeofday.h"
#include "core/timer.h"
#include "hud/hudmanager.h"
#include "objectdatabase/dataobject.h"

START_STD_INTERFACE ( SpecialDef )
	IFLOAT		( SpecialDef, SpecialDuration )
	IFLOAT		( SpecialDef, StartingTime )
	IFLOAT		( SpecialDef, FinishingTime )
	IFLOAT		( SpecialDef, SpecialSpeedFactor )
	IFLOAT		( SpecialDef, SpeedSpecialTargetDistance )
	IFLOAT		( SpecialDef, SpeedSpecialTargetAngle )
	IFLOAT		( SpecialDef, PowerSpecialDamageMultiplier )
	IFLOAT		( SpecialDef, RangeSpecialDamageMultiplier )
	IFLOAT		( SpecialDef, RangeSpecialAttackSpeedMultiplier )
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
END_STD_INTERFACE

/***************************************************************************************************
*
*	FUNCTION		SpecialDef::SpecialDef
*
*	DESCRIPTION		Construction
*
*					Give some reasonable default values
*
***************************************************************************************************/
SpecialDef::SpecialDef( void )
:	m_fSpecialDuration( 3.0f ),
	m_fStartingTime( 0.5f ),
	m_fFinishingTime( 0.5f ),
	m_fSpecialSpeedFactor( 0.1f ),
	m_fSpeedSpecialTargetDistance( 5.0f ),
	m_fSpeedSpecialTargetAngle( 360.0f ),
	m_fPowerSpecialDamageMultiplier( 10.0f ),
	m_fRangeSpecialDamageMultiplier( 1.0f ),
	m_fRangeSpecialAttackSpeedMultiplier( 2.5f ) 
{
}


/***************************************************************************************************
*
*	FUNCTION		SpecialDef::~SpecialDef
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/
SpecialDef::~SpecialDef( void )
{
}


/***************************************************************************************************
*
*	FUNCTION		SpecialDef::PostConstruct
*
*	DESCRIPTION		Just does some basic checks of the serialised data
*
***************************************************************************************************/
void SpecialDef::PostConstruct( void )
{
	// Make sure all the below are non negative
	ntAssert( m_fSpecialDuration > 0.0f );
	ntAssert( m_fStartingTime > 0.0f );
	ntAssert( m_fFinishingTime > 0.0f );

	// Check that we don't have a speed factor that would damage the game
	if ( m_fSpecialSpeedFactor >= 0.1f )
	{
		ntAssert( 0 );
		ntPrintf( "%s(%d):\tWARNING: The Special Speed Factor is %.2f which is too low\n", __FILE__, __LINE__, m_fSpecialSpeedFactor );
	}

	// Check that we are slowing time down
	if ( m_fSpecialSpeedFactor >= 1.0f )
	{
		ntAssert( 0 );
		ntPrintf( "%s(%d):\tWARNING: The Special Speed Factor is %.2f, it should be less than one\n", __FILE__, __LINE__, m_fSpecialSpeedFactor );
	}
}


/***************************************************************************************************
*
*	FUNCTION		AttackSpecial::AttackSpecial
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/
AttackSpecial::AttackSpecial( CEntity* pobParentEntity, const SpecialDef* pobSpecialDef )
:	m_pobDef( pobSpecialDef ),
	m_pobParent( pobParentEntity ),
	m_eState( SPECIAL_STATE_IDLE ),
	m_fDurationInCurrentState( 0.0f )
{
}

float AttackSpecial::GetAttackSpeedMultiplier()
{	
	if (!IsActive())
		return 1.0f;

	if (m_eSpecialStance == ST_RANGE)
		return m_pobDef->m_fRangeSpecialAttackSpeedMultiplier;
	else 
		return 1.0f;
}

float AttackSpecial::GetAttackDamageMultiplier()
{
	if (!IsActive())
		return 1.0f;

	if (m_eSpecialStance == ST_RANGE)
		return m_pobDef->m_fRangeSpecialDamageMultiplier;
	else if ( m_eSpecialStance == ST_POWER)
		return m_pobDef->m_fPowerSpecialDamageMultiplier;
	else 
		return 1.0f;
}

/***************************************************************************************************
*
*	FUNCTION		AttackSpecial::SetState
*
*	DESCRIPTION		Kick off a special - this is now a one shot thing
*
***************************************************************************************************/
void AttackSpecial::SetState( SPECIAL_STATES eNewState )
{
	// Set the state and reset the state time
	m_eState = eNewState;
	m_fDurationInCurrentState = 0.0f;
}

void AttackSpecial::SetSpecialStance( STANCE_TYPE eStance )
{
	m_eSpecialStance = eStance;
}


/***************************************************************************************************
*
*	FUNCTION		AttackSpecial::StartSpecial
*
*	DESCRIPTION		Kick off a special - this is now a one shot thing
*
***************************************************************************************************/
bool AttackSpecial::StartSpecial( STANCE_TYPE eStance )
{
	// If we are active already we can't start another
	if ( IsActive() )
		return false;

	m_eSpecialStance = eStance;

	// Start from scratch
	m_obSlowedEntities.clear();
	FillTargetListForSlowDown(); // These'll be slowed down next Update

	// Set our state to starting
	SetState( SPECIAL_STATE_STARTING );

	// We have been successful
	return true;
}


/***************************************************************************************************
*
*	FUNCTION		AttackSpecial::Update
*
*	DESCRIPTION		The main update of this item
*
*					Deals with slowing down all the other entities
*
***************************************************************************************************/
void AttackSpecial::Update( float fTimeStep )
{
	// Update our state time
	m_fDurationInCurrentState += fTimeStep;

	// The rest of the update is dependant on state
	switch ( m_eState )
	{
	case SPECIAL_STATE_IDLE:
		// We don't need to do anything
		break;

	case SPECIAL_STATE_STARTING:
		{
			// If we have have passed the deceleration period
			if ( m_fDurationInCurrentState >= m_pobDef->m_fStartingTime )
			{
				// Set the new state and timer
				SetState( SPECIAL_STATE_SPECIAL );

				// Set all the entity multipliers to their special value
				SetEntityTimeMultipliers( m_pobDef->m_fSpecialSpeedFactor ); 
			}

			// Otherwise we set all the entities to a linear value based on how far through the period we are
			else
			{
				// What percentage are we through the deceleration - can't be a divide by zero because of the clause above
				float fDecelerationPercentage = m_fDurationInCurrentState / m_pobDef->m_fStartingTime;

				// Assuming that we are always slowing down time for now...
				SetEntityTimeMultipliers( 1.0f - ( ( 1.0f - m_pobDef->m_fSpecialSpeedFactor ) * fDecelerationPercentage ) );
			}
		}
		break;

	case SPECIAL_STATE_SPECIAL:
		{
			// If the button is no longer held and the minimum time is exceeded drop out
			if ( m_fDurationInCurrentState >= m_pobDef->m_fSpecialDuration )
			{	
				// Change the state
				SetState( SPECIAL_STATE_FINISHING );
			}

			ClearEntityTimeMultipliers(); // Speed everyone up again
			m_obSlowedEntities.clear(); // Clear our record so we can search again fresh
			FillTargetListForSlowDown(); // Search
			SetEntityTimeMultipliers( m_pobDef->m_fSpecialSpeedFactor ); // Slow anyone still in range down again
		}
		break;

	case SPECIAL_STATE_FINISHING:
		{
			// Otherwise if we have have passed the acceleration period
			if ( m_fDurationInCurrentState >= m_pobDef->m_fFinishingTime )
			{
				// Set the new state and timer
				SetState( SPECIAL_STATE_IDLE );

				// Clear up all of the timer stuff
				ClearEntityTimeMultipliers();
			}

			// Otherwise we set all the entities to a linear value based on how far through the period we are
			else
			{
				// What percentage are we through the acceleration - can't be a divide by zero because of the clause above
				float fAccelerationPercentage = 1.0f - ( m_fDurationInCurrentState / m_pobDef->m_fFinishingTime );

				// Assuming that we are always slowing down time for now...
				SetEntityTimeMultipliers( 1.0f - ( ( 1.0f - m_pobDef->m_fSpecialSpeedFactor ) * fAccelerationPercentage ) );
			}
		}
		break;

	default:
		// We don't even have one of these - that's weird
		ntAssert( 0 );
		break;
	}

	// Update our effect if we have one
	/*if ( m_pobEffect )
	{
		bool bReadyToDie = m_pobEffect->Update( fTimeStep, ( m_eState != SPECIAL_STATE_IDLE ) );
		if ( bReadyToDie )
		{
			m_pobEffect->Cleanup();
			m_pobEffect.Reset();
		}
	}*/
}

void AttackSpecial::FillTargetListForSlowDown()
{
	CEntityQuery obQuery;

	CEQCProximitySegment obProximitySub;

	obProximitySub.SetRadius( m_pobDef->m_fSpeedSpecialTargetDistance );
	obProximitySub.SetAngle( m_pobDef->m_fSpeedSpecialTargetAngle * DEG_TO_RAD_VALUE );
	obProximitySub.SetMatrix( m_pobParent->GetMatrix() );

	obQuery.AddClause( obProximitySub );

	obQuery.AddExcludedEntity( *m_pobParent );

	CEntityManager::Get().FindEntitiesByType( obQuery, CEntity::EntType_Character );

	QueryResultsContainerType::iterator obIt;
	for (obIt = obQuery.GetResults().begin(); obIt != obQuery.GetResults().end(); ++obIt)
	{
		m_obSlowedEntities.push_back((*obIt));
	}
}

/***************************************************************************************************
*
*	FUNCTION		AttackSpecial::SetEntityTimeMultipliers
*
*	DESCRIPTION		Set the time multiplier on all entities except the one that owns this component
*
***************************************************************************************************/
void AttackSpecial::SetEntityTimeMultipliers( float fMultiplier )
{
	ntstd::List<CEntity*>::iterator obIt;
	for (obIt = m_obSlowedEntities.begin(); obIt != m_obSlowedEntities.end(); ++obIt)
	{
		(*obIt)->SetTimeMultiplier(fMultiplier);
	}
}


/***************************************************************************************************
*
*	FUNCTION		AttackSpecial::ClearEntityTimeMultipliers
*
*	DESCRIPTION		Clear the time multiplier on all entities except the one that owns this component
*
***************************************************************************************************/
void AttackSpecial::ClearEntityTimeMultipliers( void )
{
	ntstd::List<CEntity*>::iterator obIt;
	for (obIt = m_obSlowedEntities.begin(); obIt != m_obSlowedEntities.end(); ++obIt)
	{
		(*obIt)->SetTimeMultiplier(1.0f);
	}
}
