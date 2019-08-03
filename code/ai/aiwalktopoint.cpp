//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file aiwalktopoint.cpp
//!                                                                                         
//------------------------------------------------------------------------------------------
                                                                                         
                                                                                         
//------------------------------------------------------------------------------------------
// Includes                                                                                 
//------------------------------------------------------------------------------------------

#include "ai/aiwalktopoint.h"
#include "ai/ainavnodes.h"
#include "ai/ainavgraphmanager.h"

#include "game/aicomponent.h"
#include "game/randmanager.h"

bool
AIWalkToPoint::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
{
	const float fInitTime			(1.0f);

BeginStateMachine

	/***************************************************************/
	AIState( STATE_INITIALISE )
		OnEnter
			m_fTimer = 0.0f;
			m_pobAIComp->SetAction( ACTION_NONE );
			m_pobAIComp->SetActionMoveSpeed( 0.0f );

			// set the target point to be at our start pos, and set the speed to slow
			CAINavGraphManager::Get().SetPathfindTestTarget( m_pobEnt->GetPosition() );
			CAINavGraphManager::Get().SetPathfindTestRun( false );
			CAINavGraphManager::Get().AcknowledgePathfindTestChange();
			m_bNeedToAcknowledge = false;

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

		OnUpdate

			// if we received a pathfind change command last update, acknowledge it here
			// (this allows multiple AIs to receive the command without the first oen blocking it for the others)
			if (m_bNeedToAcknowledge)
			{
				CAINavGraphManager::Get().AcknowledgePathfindTestChange();
				m_bNeedToAcknowledge = false;
			}

			// if the position or speed we're meant to run at has changed, then update them
			if (CAINavGraphManager::Get().HasPathfindTestChanged())
			{
				float fSpeed = CAINavGraphManager::Get().GetPathfindTestRun() ? 1.0f : 0.5f;
				m_pobAIComp->SetAction( ACTION_WALK );
				m_pobAIComp->SetActionMoveSpeed( fSpeed );
				m_pobAIComp->SetActionDest( CAINavGraphManager::Get().GetPathfindTestTarget() );
				m_bNeedToAcknowledge = true;
			}
			
			//reinitialise if we've reached our target
			CPoint obPos( m_pobEnt->GetPosition() );
			CPoint obDiff = obPos - CAINavGraphManager::Get().GetPathfindTestTarget();
			//obDiff.Y() = 0.0f;

			const float fWalkDistSq = 10.0f;
			const float fStopDistSq = 3.0f;
			float		fLengthSq = obDiff.LengthSquared();

			if (fLengthSq < fWalkDistSq)
			{
				float fSpeed = ((fLengthSq - fStopDistSq) / (fWalkDistSq - fStopDistSq)) * 0.5f;
				m_pobAIComp->SetActionMoveSpeed( fSpeed );

				if (fLengthSq < fStopDistSq)
				{
					SetState( STATE_AT_TARGET );
				}
			}

		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_AT_TARGET )
		OnEnter
			m_fTimer = 0.0f;
			m_pobAIComp->SetAction( ACTION_NONE );
			m_pobAIComp->SetActionMoveSpeed( 0.0f );

		OnUpdate
			if (CAINavGraphManager::Get().HasPathfindTestChanged())
			{
				SetState( STATE_GO );
			}

		OnExit
			m_fTimer = 0.0f;
EndStateMachine
}

