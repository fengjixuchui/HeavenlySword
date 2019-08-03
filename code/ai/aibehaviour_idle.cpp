//! -------------------------------------------
//! aibehaviour_idle.cpp
//!
//! IDLE behaviour for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#include "aibehaviour_idle.h"
#include "ai/ainavigationsystem/ainavigsystemman.h"
#include "game/aicomponent.h"
#include "game/messagehandler.h"

//!--------------------------------------------
//! States (FSM)
//!--------------------------------------------
bool CAIIdleBehaviour::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
{
	UNUSED( fTimeChange );

	const float fInitTime			(0.0f);

	// Cache a vision object pointer 
	const CAIVision* pVis = m_pobAIComp->GetAIVision();
	if (!pVis) return false;

BeginStateMachine

	/***************************************************************/
	AIState( STATE_INITIALISE )
		OnEnter

			PRINT_STATE(Idle_Dario::STATE_INITIALISE)
			m_fTimer = 0.0f;
			
			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_INITIALISE);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}

			// Stop previous animations
			m_pobAIComp->CompleteSimpleAction();

			// Clear Intentions
			if (m_pMov->GetIdleClearsIntention())
			{
				// Clear intentions
				m_pMov->SetIntention(NAVINT_NONE,0,m_pMov->GetIdleFlags());
				m_pMov->ClearDestinationNode();
			}			

			// Play Anim?
			CHashedString hsAnimName = m_pobAIComp->GetScriptAnimName();
			if (m_pMov->GetIdlePlaysAnim() && !hsAnimName.IsNull())
			{
				m_pMov->SetIdlePlaysAnim(false);
				SetState(STATE_PLAY_ANIM);
				return true;
			}
			else
			{
				// Push a default controller 
				m_pobAIComp->ActivateController(CAIComponent::MC_WALKING);
			}

		OnUpdate
			m_fTimer += fTimeChange;
			if( m_fTimer > fInitTime )
			{
				SetState( STATE_IDLE );
			}
			
		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_IDLE )
		OnEnter

			PRINT_STATE(Idle_Dario::STATE_IDLE)
			m_fTimer = 0.0f;

		OnUpdate

			if (!m_pobAIComp->GetAIVision()->DoISeeNothing())
			{
				AI_BEHAVIOUR_SEND_MSG( PATROL_SEENENEMY );
				AI_BEHAVIOUR_SEND_MSG( SEENENEMY );
			}

			bool bExecuted = false;
			ENUM_DIVE_FULL_DIRECTION eDiveAction = CAINavigationSystemMan::Get().GetDivingAction(m_pMov->GetParent(), &bExecuted);

			if (bExecuted && eDiveAction != E_DONT_DIVE)
			{
				// Play a Taking_Cover animation
				PRINT_STATE(Idle_Dario::STATE_IDLE::PREPARING_TO_DIVE)
				bool bOK = m_pMov->PlayDiveAnimation(eDiveAction);
				if (bOK)
					SetState(STATE_START_DIVING);
			}
		
		OnExit
			m_fTimer = 0.0f;

		// ============= STATE_START_DIVING ==========================
		AIState( STATE_START_DIVING )
		OnEnter
			PRINT_STATE(Idle_Dario::STATE_START_DIVING)
			m_fTimer = 0.0f;

			// Clear previous animations
			//m_pobAIComp->CompleteSimpleAction();
			//m_pobAIComp->SetPlayingFacingAction(false);

			//// Play a Taking_Cover animation
			//m_pMov->PlayAnimation( ACTION_DUCK_LONG_LEFT );
			
		OnUpdate

			if( m_pobAIComp->IsSimpleActionComplete() )
			{
				// A cycle is finished
				SetState(STATE_IDLE);
			}
			

		OnExit
		
			m_fTimer = 0.0f;
		
		// ============= STATE_PLAY_ANIM ==========================
		AIState( STATE_PLAY_ANIM )
		OnEnter
			PRINT_STATE(Idle_Dario::STATE_PLAY_ANIM)
			m_fTimer = 0.0f;

			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_PLAY_ANIM);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}

			// Clear previous animations
			m_pobAIComp->CompleteSimpleAction();
			m_pobAIComp->SetPlayingFacingAction(false);

			// Play anim
			m_pMov->PlayAnimation( ACTION_SCRIPTANIM );
			
		OnUpdate

			// Loop here and report enemy seen

			if ( !pVis->DoISeeNothing() )
			{
				AI_BEHAVIOUR_SEND_MSG (SEENENEMY);
			}
			
		OnExit
		
			m_fTimer = 0.0f;

		// ============= STATE_WAIT_FOR_CS_STANDARD ==========================
		AIState( STATE_WAIT_FOR_CS_STANDARD )
		OnEnter
			PRINT_STATE(Idle_Dario::STATE_WAIT_FOR_CS_STANDARD)
			m_fTimer = 0.0f;
		
		OnUpdate
			
			// Wait here until AI is in CS_STANDARD
		
			if (IsSaveToPushAController())
				SetState(GetReturnToState());

		OnExit
		
			m_fTimer = 0.0f;


EndStateMachine
}
