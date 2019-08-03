//! -------------------------------------------
//! aibehaviour_gotonode.h
//!
//! FOLLOW PATH TO NODE behaviour for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#include "aibehaviour_gotonode.h"
#include "ai/ainavigationsystem/ainavigsystemman.h"

#include "ai/aipatrolmanager.h"

#include "game/aicomponent.h"
#include "game/entitymanager.h"
#include "game/randmanager.h"
#include "game/messagehandler.h"

//--------------------------------------------------
//!
//!	CAIPatrolBehaviour::States
//!
//--------------------------------------------------

bool
CAIGoToNodeBehaviour::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
{

	const float	fMaxWalkTime		(200.0f);

	user_error_p(0,("AI: [%s] is using the DEPRECATED behaviour -GoToNodeDario-. Please, use -FollowPathCoverDario- instead",ntStr::GetString(m_pobEnt->GetName())));
	return false;

	// In any case, if the enemy is spotted, inform!

	if (m_pobAIComp->GetAIVision()->DoISeeMyTarget())
	{
		AI_BEHAVIOUR_SEND_MSG( PATROL_SEENENEMY );
	}


BeginStateMachine

	// ============= STATE_INITIALISE ==========================

	AIState( STATE_INITIALISE )
		OnEnter
			PRINT_STATE(GotToNode_Dario::STATE_INITIALISE)
			m_fTimer = 0.0f;
			
			// Check if I can find a path to the selected node

			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_INITIALISE);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}

			bool bCommandExecuted = false; 
			bool bExistPath = CAINavigationSystemMan::Get().FollowPath( m_pobEnt, &bCommandExecuted );
			
			//m_pobAIComp->ActivateController(CAIComponent::MC_WALKING);
			if (!bExistPath)
			{
				AI_BEHAVIOUR_SEND_MSG( PATH_NOT_FOUND );
				SetState(STATE_IDLE);
			}
			else
			{
				SetState(STATE_MOVING);			
			}

		OnUpdate

		OnExit
			m_fTimer = 0.f;

	// ============= STATE_MOVING ==========================
	
	AIState( STATE_MOVING )
		OnEnter
			PRINT_STATE(GotToNode_Dario::STATE_MOVING)
			m_fTimer = 0.0f;

		OnUpdate
			m_fTimer += fTimeChange;
			
			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_MOVING);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}
			/*if (m_pobAIComp->GetDisabled() == true )
			{
				SetState(STATE_WAIT_UNTIL_RECOVERED);
				return true;
			}*/
			if (m_pMov->IsFollowPathCompleted())
			{
				AI_BEHAVIOUR_SEND_MSG( DESTINATION_REACHED );
				SetState(STATE_IDLE);	
				return true;
			}
			if ( m_fTimer  > fMaxWalkTime  )
			{
				AI_BEHAVIOUR_SEND_MSG( TIME_OUT );
				SetState(STATE_IDLE);	
			}

		OnExit
			m_fTimer = 0.f;

	// ============= STATE_WAIT_UNTIL_RECOVERED ==========================
	
	AIState( STATE_WAIT_UNTIL_RECOVERED )
		OnEnter
			PRINT_STATE(GotToNode_Dario::STATE_WAIT_UNTIL_RECOVERED)
			m_fTimer = 0.0f;

		OnUpdate
			// Stay here until the AI recovers
			if ( !m_pobAIComp->GetDisabled() )
			{
				// Push back the controller
				m_pobAIComp->ActivateController(CAIComponent::MC_WALKING);
				SetState(STATE_MOVING);
			}

		OnExit
			m_fTimer = 0.f;

	// ============= STATE_IDLE ==========================
	
	AIState( STATE_IDLE )
		OnEnter
			PRINT_STATE(GotToNode_Dario::STATE_IDLE)
			m_fTimer = 0.0f;

		OnUpdate
			// Stay here until new orders come


		OnExit
			m_fTimer = 0.f;

	// ============= STATE_WAIT_FOR_CS_STANDARD ==========================
	AIState( STATE_WAIT_FOR_CS_STANDARD )
	OnEnter
		PRINT_STATE(GotToNode_Dario::STATE_WAIT_FOR_CS_STANDARD)
		m_fTimer = 0.0f;
	
	OnUpdate
		
		// Wait here until AI is in CS_STANDARD
	
		if (IsSaveToPushAController())
			SetState(GetReturnToState());

	OnExit
	
		m_fTimer = 0.0f;

EndStateMachine
}
