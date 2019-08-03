//! -------------------------------------------
//! aibehaviour_gotolocatornode.cpp
//!
//! STEER TO LOCATOR NODE behaviour for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#include "ai/aibehaviour_gotolocatornode.h"
#include "ai/ainavigationsystem/ainavigsystemman.h"

#include "game/aicomponent.h"
#include "game/messagehandler.h"

bool CAIGoToLocatorNodeBehaviour::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
{
	float fMaxWalkingTime = 60.0f;
	//if (m_pobAIComp->CanSeePlayer())
	//{
	//	AI_BEHAVIOUR_SEND_MSG( PATROL_SEENENEMY );
	//}

BeginStateMachine

	// ============= STATE_INITIALISE ==========================
	AIState( STATE_INITIALISE )
		OnEnter
			PRINT_STATE(GoToLocatorNode_Dario::STATE_INITIALISE);
			m_fTimer = 0.0f;
			
			// Stop Ani previous animation
			//m_pobAIComp->CompleteSimpleAction();
			m_pobAIComp->SetPlayingFacingAction(false);

			// Give the command
			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_INITIALISE);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}
			//m_pobAIComp->ActivateController(CAIComponent::MC_STRAFING); // Hack
			bool bValidNode = CAINavigationSystemMan::Get().SteerToLocatorNode(m_pobEnt);
			//m_pobAIComp->ActivateController(CAIComponent::MC_WALKING);  // Hack
			if (bValidNode)
			{
				SetState(STATE_MOVING);
			}
			else
			{
				AI_BEHAVIOUR_SEND_MSG( DESTINATION_UNREACHABLE );
				SetState(STATE_IDLE);
			}			

		OnUpdate
			
			// Hack!
			//m_pobAIComp->CompleteSimpleAction();
			m_pobAIComp->SetPlayingFacingAction(false);

		OnExit
			m_fTimer = 0.0f;

	// ============= STATE_MOVING ==========================
	AIState( STATE_MOVING )
		OnEnter
			PRINT_STATE(GoToLocatorNode_Dario::STATE_MOVING);
			m_fTimer = 0.0f;

		OnUpdate
			m_fTimer += fTimeChange;
			
		if (m_pMov->IsMoveToSelectedPointCompleted())
		{
			AI_BEHAVIOUR_SEND_MSG( DESTINATION_REACHED );
			SetState(STATE_IDLE);
		} else if ( m_fTimer > fMaxWalkingTime )
		{
			AI_BEHAVIOUR_SEND_MSG( TIME_OUT );
			AI_BEHAVIOUR_SEND_MSG( DESTINATION_UNREACHABLE );
			SetState(STATE_IDLE);
		}

			
		OnExit
			m_fTimer = 0.0f;
	// ============= STATE_IDLE ==========================
	AIState( STATE_IDLE )
		OnEnter
			PRINT_STATE(GoToLocatorNode_Dario::STATE_IDLE);
			
		OnUpdate

			m_fTimer += fTimeChange;

			if( m_fTimer > 0.1f )
			{
				SetState(STATE_INITIALISE);
			}

		OnExit
			m_fTimer = 0.0f;

	// ============= STATE_WAIT_FOR_CS_STANDARD ==========================
	AIState( STATE_WAIT_FOR_CS_STANDARD )
	OnEnter
		PRINT_STATE(GoToLocatorNode_Dario::STATE_WAIT_FOR_CS_STANDARD)
		m_fTimer = 0.0f;
	
	OnUpdate
		
		// Wait here until AI is in CS_STANDARD
	
		if (IsSaveToPushAController())
			SetState(GetReturnToState());

	OnExit
	
		m_fTimer = 0.0f;


EndStateMachine
}

