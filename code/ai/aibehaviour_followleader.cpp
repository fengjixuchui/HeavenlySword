//! -------------------------------------------
//! aibehaviour_followleader.cpp
//!
//! CHASE behaviour for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------
                                                                                         
                                                                                         
//------------------------------------------------------------------------------------------
// Includes                                                                                 
//------------------------------------------------------------------------------------------

#include "ai/aibehaviour_followleader.h"
#include "ai/ainavigationsystem/ainavigsystemman.h"
#include "game/messagehandler.h"
#include "game/aicomponent.h"

bool CAIFollowLeaderBehaviour::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
{
	UNUSED( fTimeChange );

//	const float fInitTime			(0.3f);

BeginStateMachine

	// ============= STATE_INITIALISE ==========================

	AIState( STATE_INITIALISE )
		OnEnter
			PRINT_STATE(FollowEntity_Dario::STATE_INITIALISE)
			m_fTimer = 0.0f;
			
			// Set any running SimpleAnimation as completed
			m_pobAIComp->CompleteSimpleAction();
			// Give the command to follow the selected leader
			CAINavigationSystemMan::Get().FollowEntity(m_pobEnt);

			SetState( STATE_FOLLOWING_ENTITY );

		OnUpdate
			
		OnExit
			m_fTimer = 0.0f;

	// ============= STATE_FOLLOWING_ENTITY ==========================

	AIState( STATE_FOLLOWING_ENTITY )
		OnEnter
			PRINT_STATE(FollowEntity_Dario::STATE_FOLLOWLEADER)
			m_fTimer = 0.0f;

		OnUpdate
			m_fTimer += fTimeChange;

			if( m_pobAIComp->GetAIVision()->DoISeeTheEnemy() )
			{
				AI_BEHAVIOUR_SEND_MSG( SEENENEMY );
			}

			if (m_pMov->IsFollowEntityCompleted())
			{
				AI_BEHAVIOUR_SEND_MSG( DESTINATION_REACHED );
			}

		OnExit
			m_fTimer = 0.0f;

	// ============= STATE_IDLE ==========================
	
	AIState( STATE_IDLE )
		OnEnter
			PRINT_STATE(FollowEntity_Dario::STATE_IDLE)
			m_fTimer = 0.0f;

		OnUpdate
			// Stay here until new orders come

		OnExit
			m_fTimer = 0.f;

EndStateMachine
}

