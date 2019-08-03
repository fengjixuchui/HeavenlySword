//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file aifollow.cpp
//!                                                                                         
//------------------------------------------------------------------------------------------
                                                                                         
                                                                                         
//------------------------------------------------------------------------------------------
// Includes                                                                                 
//------------------------------------------------------------------------------------------

#include "ai/aifollow.h"
#include "ai/ainavnodes.h"
#include "ai/ainavgraphmanager.h"

#include "game/aicomponent.h"
#include "game/randmanager.h"

bool
AIFollow::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
{
	const float fInitTime			(4.0f);
	const float fRepathTime			(2.0f);

BeginStateMachine

	/***************************************************************/
	AIState( STATE_INITIALISE )
		OnEnter
			m_fTimer = 0.0f;
			m_pobAIComp->SetAction( ACTION_NONE );
			m_pobAIComp->SetActionMoveSpeed( 0.0f );
			m_pobTarget = m_pobAIComp->GetDirectTarget();
			ntAssert( m_pobTarget );

		OnUpdate
			m_fTimer += fTimeChange;
			if( m_fTimer > fInitTime )
			{
				SetState( STATE_GO );
			}
			
		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_GO )
		OnEnter
			m_fTimer = 0.0f;

			// get the target's position and walk to it
			m_obDest = m_pobTarget->GetPosition(); 
			m_pobAIComp->SetAction( ACTION_WALK );
			m_pobAIComp->SetActionMoveSpeed( 0.5f );
			m_pobAIComp->SetActionDest( m_obDest );

		OnUpdate
			m_fTimer += fTimeChange;

			//reinitialise if we've reached our target
			CPoint obPos( m_pobEnt->GetPosition() );
			CPoint obDiff = obPos - m_obDest;
			obDiff.Y() = 0.0f;

			if (obDiff.LengthSquared() < 3.0f || m_fTimer > fRepathTime)
			{
				SetState( STATE_NEWDEST );
			}

		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_NEWDEST )
		OnEnter
			m_fTimer = 0.0f;

		OnUpdate
			//reinitialise if we've reached our target
			CPoint obPos( m_pobTarget->GetPosition() );
			CPoint obDiff = obPos - m_obDest;
			obDiff.Y() = 0.0f;

			if (obDiff.LengthSquared() > 4.0f)
			{
				SetState( STATE_GO );
			}
			else
			{
				m_pobAIComp->SetAction( ACTION_NONE );
				m_pobAIComp->SetActionMoveSpeed( 0.0f );
			}

		OnExit
			m_fTimer = 0.0f;

EndStateMachine
}

