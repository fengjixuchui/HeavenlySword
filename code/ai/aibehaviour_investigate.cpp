//! -------------------------------------------
//! aibehaviour_investigate.cpp
//!
//! INVESTIGATE behaviour for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#include "aibehaviour_investigate.h"
#include "ai/ainavigationsystem/ainavigsystemman.h"
#include "game/messagehandler.h"

bool CAIInvestigateBehaviour::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
{
	const float	fWalkTime			(200.0f);

BeginStateMachine

	// ============= STATE_INITIALISE ==========================

	AIState( STATE_INITIALISE )
		OnEnter
			PRINT_STATE(Investigate_Dario::STATE_INITIALISE)
			
			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_INITIALISE);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}
			// Give the command to move to the investigation point
			CAINavigationSystemMan::Get().SteerToDestinationPoint(	m_pobEnt,
																	m_pobAIComp->GetAIHearing()->GetSoundSourceLocation()
																  );

			SetState(STATE_MOVING_TO_POINT);
			
		OnUpdate

		OnExit

	// ============= STATE_MOVING_TO_POINT ==========================
	
	AIState( STATE_MOVING_TO_POINT )
		
		OnEnter
			PRINT_STATE(Investigate_Dario::STATE_MOVING_TO_POINT)
			m_fTimer = 0.f;

		OnUpdate
			m_fTimer += fTimeChange;

			// Do I see the enemy
			
			if (m_pobAIComp->GetAIVision()->DoISeeTheEnemy() || m_pobAIComp->GetAIVision()->DoISeeSomething())
			{
				AI_BEHAVIOUR_SEND_MSG( INVESTIGATE_FOUNDTARGET );
				SetState(STATE_IDLE);
				return true;
			}
			
			if ( m_pMov->IsMoveToSelectedPointCompleted() )
			{
				AI_BEHAVIOUR_SEND_MSG( INVESTIGATE_FOUNDNOTHING );
				SetState(STATE_IDLE);
				return true;
			}
			
			if ( m_fTimer > fWalkTime )
			{
				AI_BEHAVIOUR_SEND_MSG( TIME_OUT );
				AI_BEHAVIOUR_SEND_MSG( DESTINATION_UNREACHABLE );
				SetState(STATE_IDLE);
			}

		OnExit
			m_fTimer = 0.f;

	// ============= STATE_IDLE ==========================
	
	AIState( STATE_IDLE )
		OnEnter
			PRINT_STATE(Investigate_Dario::STATE_IDLE)
			m_fTimer = 0.0f;

			m_pMov->SetMovingWhileFacingTgt(false);
			m_pobAIComp->CompleteSimpleAction();

			// Stop his movement
			m_pMov->SetChaseTargetCompleted(true);
			m_pMov->SetMaxSpeed(0.0f);

		OnUpdate
	
			// Stay here until a new behaviour is pushed

		OnExit
			m_fTimer = 0.f;

	// ============= STATE_WAIT_FOR_CS_STANDARD ==========================
	AIState( STATE_WAIT_FOR_CS_STANDARD )
	OnEnter
		PRINT_STATE(Investigate_Dario::STATE_WAIT_FOR_CS_STANDARD)
		m_fTimer = 0.0f;
	
	OnUpdate
		
		// Wait here until AI is in CS_STANDARD
	
		if (IsSaveToPushAController())
			SetState(GetReturnToState());

	OnExit
	
		m_fTimer = 0.0f;

EndStateMachine
}





