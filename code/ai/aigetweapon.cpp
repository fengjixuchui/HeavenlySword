//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file aigetweapon.cpp
//!                                                                                         
//------------------------------------------------------------------------------------------
                                                                                         
                                                                                         
//------------------------------------------------------------------------------------------
// Includes                                                                                 
//------------------------------------------------------------------------------------------

#include "ai/aigetweapon.h"
#include "game/aicomponent.h"
#include "game/query.h"
#include "game/entitymanager.h"
#include "game/messagehandler.h"

void
AIGetWeapon::FindWeapon()
{
	// setup entity query to match preferred weapon type, in this test case a bazooka
	CEntityQuery obWeaponQuery;
	CEQCIsSubStringInName	obNameClause( "bazooka" );
	obWeaponQuery.AddClause( obNameClause );
	CEntityManager::Get().FindEntitiesByType( obWeaponQuery, CEntity::EntType_Weapon );

	// set the weapon found flag based on the query's success XXX: doesn't check to see if the weapon is already held
	m_bWeaponFound = false;
	QueryResultsContainerType::iterator obEnd = obWeaponQuery.GetResults().end();
	for ( QueryResultsContainerType::iterator obIt = obWeaponQuery.GetResults().begin(); obIt != obEnd; ++obIt )
	{
		m_bWeaponFound = true;

		// if it was a success, store the entity's location
		m_obWeaponLocation = (*obIt)->GetPosition();

		// store the pointer to the entity for later message sending

		break;
	}
}

void
AIGetWeapon::PickupWeapon()
{
	m_pobEnt->GetMessageHandler()->ReceiveMsg<msg_buttonaction>();
}

void
AIGetWeapon::FireWeapon()
{
	m_pobEnt->GetMessageHandler()->ReceiveMsg<msg_buttonaction>();
}

bool
AIGetWeapon::AtWeaponPos( void ) const
{
	CPoint obPos( m_pobEnt->GetPosition() );

	// XXX: make it 2d for now, but for distance check we really need the waypoint Y to
	// be the same as the AI's y when stood at that point on the map
	CPoint obDiff = obPos - m_obWeaponLocation;
	obDiff.Y() = 0.0f;

	if (obDiff.LengthSquared() < 2.0f)
	{
		return true;
	}

	return false;
}



bool
AIGetWeapon::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
{
	UNUSED( fTimeChange );

	const float fInitTime			(0.3f);
	const float	fFireTime			(6.5f);

BeginStateMachine

	/***************************************************************/
	AIState( STATE_INITIALISE )
		OnEnter
			m_fTimer = 0.0f;

		OnUpdate
			m_fTimer += fTimeChange;
			if( m_fTimer > fInitTime )
			{
				SetState( STATE_LOCATEWEAPON );
			}
			
		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_LOCATEWEAPON )
		OnEnter
			m_fTimer = 0.0f;

			m_pobAIComp->SetAction( ACTION_NONE );
			FindWeapon();
			if (m_bWeaponFound) // distance to player is greater than attack distance
			{
				SetState( STATE_GOTOWEAPON );
			}
			else
			{
				//m_pobParent->SendMsg( GETWEAPON_NOWEAPONFOUND ); 
			}

		OnUpdate
			//m_fTimer += fTimeChange;
			
		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_GOTOWEAPON )
		OnEnter
			m_fTimer = 0.0f;

			m_pobAIComp->SetAction( ACTION_WALK );
			m_pobAIComp->SetActionDest( m_obWeaponLocation );

		OnUpdate
			m_fTimer += fTimeChange;

			// if we've reached the weapon (XXX: and it's still where we thought it was)
			// then go to the pickup state
			if (AtWeaponPos())
			{
                SetState( STATE_GRABATTEMPT );                
			}
			
		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_GRABATTEMPT )
		OnEnter
			m_fTimer = 0.0f;

			// stand still
			m_pobAIComp->SetAction( ACTION_NONE );

			// try to pick up the weapon
			PickupWeapon();

		OnUpdate
			m_fTimer += fTimeChange;

			if( m_fTimer > fInitTime )
			{
				SetState( STATE_ATTACK );
			}

		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_ATTACK )
		OnEnter
			m_fTimer = 0.0f;

			m_pobAIComp->SetAction( ACTION_WALK );
			m_pobAIComp->SetActionDest( m_pobAIComp->GetActualPlayerPos() );

		OnUpdate
			m_fTimer += fTimeChange;

			if( m_fTimer > fFireTime )
			{
				// try to fire up the weapon
				FireWeapon();

				SetState( STATE_RELOAD );
			}

		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_RELOAD )
		OnEnter
			m_fTimer = 0.0f;

			// stand still
			m_pobAIComp->SetAction( ACTION_NONE );
			m_pobAIComp->SetActionDest( m_obWeaponLocation );

		OnUpdate
			m_fTimer += fTimeChange;

			if( m_fTimer > fInitTime )
			{
				// try to fire up the weapon
				FireWeapon();

				SetState( STATE_ATTACK );
			}

		OnExit
			m_fTimer = 0.0f;
EndStateMachine
}

