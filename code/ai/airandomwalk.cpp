//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file airandomwalk.cpp
//!                                                                                         
//------------------------------------------------------------------------------------------
                                                                                         
                                                                                         
//------------------------------------------------------------------------------------------
// Includes                                                                                 
//------------------------------------------------------------------------------------------

#include "ai/airandomwalk.h"
#include "ai/ainavnodes.h"
#include "ai/ainavgraphmanager.h"

#include "game/aicomponent.h"
#include "game/randmanager.h"

bool
AIRandomWalk::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
{
	const float fInitTime			(2.0f);

BeginStateMachine

	/***************************************************************/
	AIState( STATE_INITIALISE )
		OnEnter
			m_fTimer = 0.0f;
			m_pobAIComp->SetAction( ACTION_NONE );
			m_pobAIComp->SetActionMoveSpeed( 0.0f );

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

			// pick a random navgraph node and walk to its centre
			m_obDest = CAINavGraphManager::Get().GetRandomPosInGraph();
			m_pobAIComp->SetAction( ACTION_WALK );
			m_pobAIComp->SetActionMoveSpeed( 0.5f );
			m_pobAIComp->SetActionDest( m_obDest );

		OnUpdate
			//reinitialise if we've reached our target
			CPoint obPos( m_pobEnt->GetPosition() );
			CPoint obDiff = obPos - m_obDest;
			obDiff.Y() = 0.0f;

			if (obDiff.LengthSquared() < 3.0f)
			{
				SetState( STATE_INITIALISE );
			}

		OnExit
			m_fTimer = 0.0f;

EndStateMachine
}

