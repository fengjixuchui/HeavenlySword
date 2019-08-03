//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file aibehaviour_attack.cpp
//!                                                                                         
//------------------------------------------------------------------------------------------
                                                                                         
                                                                                         
//------------------------------------------------------------------------------------------
// Includes                                                                                 
//------------------------------------------------------------------------------------------

#include "ai/aibehaviour_attack.h"
#include "game/messagehandler.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "game/aicomponent.h"

bool
AIBehaviour_Attack::TooFarFromPlayer( void ) const
{
	CPoint obPos( m_pobEnt->GetPosition() );

	// XXX: make it 2d for now, but for distance check we really need the waypoint Y to
	// be the same as the AI's y when stood at that point on the map
	CPoint obDiff = obPos - m_pobAIComp->GetLastKnownPlayerPos();
	obDiff.Y() = 0.0f;

	if (obDiff.LengthSquared() > 16.0f)
	{
		return true;
	}

	return false;
}

bool
AIBehaviour_Attack::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
{
	UNUSED( fTimeChange );

	const float fInitTime			(0.3f);

BeginStateMachine

	/***************************************************************/
	AIState( STATE_INITIALISE )
		OnEnter
			m_fTimer = 0.0f;

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

			m_pobAIComp->SetAction( ACTION_ATTACK );

		OnUpdate
			//m_fTimer += fTimeChange;
			if (TooFarFromPlayer()) // distance to player is greater than attack distance
			{
				AI_BEHAVIOUR_SEND_MSG( ATTACK_TOFARFROMTARGET ); 
			}
			
		OnExit
			m_fTimer = 0.0f;

EndStateMachine
}

