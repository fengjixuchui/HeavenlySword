//! -------------------------------------------
//! aibehaviour_gotoentity.cpp
//!
//! STEER TO LOCATOR NODE behaviour for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#include "ai/aibehaviour_gotoentity.h"
#include "ai/ainavigationsystem/ainavigsystemman.h"

#include "game/aicomponent.h"
#include "game/messagehandler.h"

bool CAIGoToEntityBehaviour::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
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
			PRINT_STATE(GoToEntity_Dario::STATE_INITIALISE);
			m_fTimer = 0.0f;
			
			// Check that the destination entity exists
			const CEntity* pEnt = m_pobAIComp->GetCAIMovement()->GetEntityToGoTo();
			if (pEnt)
			{
				if (!IsSaveToPushAController())
				{
					SetReturnToState(STATE_INITIALISE);
					SetState(STATE_WAIT_FOR_CS_STANDARD);
					return true;
				}

				m_pMov->SetMoveToSelectedPointCompleted(false);
				CAINavigationSystemMan::Get().SteerToEntity(m_pobEnt,CAINavigationSystemMan::NF_DEF_STEER_TO_ENTITY);
				SetState(STATE_MOVING);
			}
			else
			{
				AI_BEHAVIOUR_SEND_MSG( DESTINATION_UNREACHABLE );
				SetState(STATE_IDLE);
			}			

		OnUpdate
			
		OnExit
			m_fTimer = 0.0f;

	// ============= STATE_MOVING ==========================
	AIState( STATE_MOVING )
		OnEnter
			PRINT_STATE(GoToEntity_Dario::STATE_MOVING);
			m_fTimer = 0.0f;

		OnUpdate
			m_fTimer += fTimeChange;
			
		if (m_pobAIComp->GetCAIMovement()->IsMoveToSelectedPointCompleted())
		{
			AI_BEHAVIOUR_SEND_MSG( DESTINATION_REACHED );
			PRINT_STATE(GoToEntity_Dario::STATE_MOVING - DESTINATION_REACHED );
			SetState(STATE_IDLE);
		} else if ( m_fTimer > fMaxWalkingTime )
		{
			AI_BEHAVIOUR_SEND_MSG( TIME_OUT );
			AI_BEHAVIOUR_SEND_MSG( DESTINATION_UNREACHABLE );
			PRINT_STATE(GoToEntity_Dario::STATE_IDLE - Due to DESTINATION_UNREACHABLE);
			SetState(STATE_IDLE);
		}

			
		OnExit
			m_fTimer = 0.0f;
	// ============= STATE_IDLE ==========================
	AIState( STATE_IDLE )
		OnEnter
			PRINT_STATE(GoToEntity_Dario::STATE_IDLE);
			
			// Stop Moving
			m_pMov->DeactivateFlag(NF_ARRIVE_AT_POINT);
			
		OnUpdate

		OnExit

	// ============= STATE_WAIT_FOR_CS_STANDARD ==========================
	AIState( STATE_WAIT_FOR_CS_STANDARD )
	OnEnter
		PRINT_STATE(GoToEntity_Dario::STATE_WAIT_FOR_CS_STANDARD)
		m_fTimer = 0.0f;
	
	OnUpdate
		
		// Wait here until AI is in CS_STANDARD
	
		if (IsSaveToPushAController())
			SetState(GetReturnToState());

	OnExit
	
		m_fTimer = 0.0f;

EndStateMachine
}

