//! -------------------------------------------
//! aibehaviour_playanim.h
//!
//! Play Animation behaviour for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

//------------------------------------------------------------------------------------------
// Includes                                                                                 
//------------------------------------------------------------------------------------------

#include "ai/aibehaviour_playanim.h"
#include "game/messagehandler.h"
#include "game/aicomponent.h"

CAIPlayAnimBehaviour::~CAIPlayAnimBehaviour( void ) 
{
	PRINT_STATE(PlayAnimDario_Dario::DESTRUCTOR_CALLED);
}


bool CAIPlayAnimBehaviour::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
{

BeginStateMachine

	// ============= STATE_INITIALISE ==========================
	AIState( STATE_INITIALISE )
		OnEnter
			PRINT_STATE(PlayAnimDario_Dario::STATE_INITIALISE);
			m_fTimer = 0.0f;

			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_INITIALISE);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}

			// Play the Selected Animation
			m_pobAIComp->CompleteSimpleAction();
			bool bOK = m_pMov->PlayAnimation( ACTION_SCRIPTANIM );
			if (!bOK)
			{
				AI_BEHAVIOUR_SEND_MSG( ANIM_FAILED );
				SetState( STATE_ANIM_FAILED );
				return false;
			}


		OnUpdate
			m_fTimer += fTimeChange;

			if( m_fTimer > 0.5f )
			{
				// If the animation has taken longer than x to start then there is something wrong... 
				AI_BEHAVIOUR_SEND_MSG(TIME_OUT);
 				AI_BEHAVIOUR_SEND_MSG( ANIM_FAILED );
				SetState( STATE_COMPLETED );
				m_pobAIComp->CompleteSimpleAction();
			}
			else if( !m_pobAIComp->IsSimpleActionComplete() )
			{
				AI_BEHAVIOUR_SEND_MSG( ANIM_STARTED );
				SetState( STATE_PLAYINGANIM );
			}

			
		OnExit
			m_fTimer = 0.0f;

	// ============= STATE_PLAYINGANIM ==========================

	AIState( STATE_PLAYINGANIM )
		OnEnter
			PRINT_STATE(PlayAnimDario_Dario::STATE_PLAYINGANIM);
			m_fTimer = 0.0f;

		OnUpdate
			//m_fTimer += fTimeChange;

			if( m_pobAIComp->IsSimpleActionComplete() )
			{
				// Do we have to stay looping in this anim. forever?
				if ( m_pobAIComp->GetScriptAnimLooping() )
				{
					// Looping forever...
					AI_BEHAVIOUR_SEND_MSG( ANIM_LOOP_COMPLETED );
					m_pobAIComp->GetCAIMovement()->PlayAnimation( ACTION_SCRIPTANIM );
				}
				else
				{
					// Nop...
					AI_BEHAVIOUR_SEND_MSG( ANIM_COMPLETE );
					SetState( STATE_COMPLETED );
				}
			}
			
		OnExit
			m_fTimer = 0;

	// ============= STATE_COMPLETED ==========================
	AIState( STATE_COMPLETED )
		OnEnter
			PRINT_STATE(PlayAnimDario_Dario::STATE_COMPLETED);
			m_pobAIComp->CompleteSimpleAction();

		OnUpdate
			
		OnExit

	// ============= STATE_WAIT_FOR_CS_STANDARD ==========================
	AIState( STATE_ANIM_FAILED )
		OnEnter
			PRINT_STATE(PlayAnimDario_Dario::STATE_ANIM_FAILED);
			m_pobAIComp->CompleteSimpleAction();

		OnUpdate
			
		OnExit

	// ============= STATE_WAIT_FOR_CS_STANDARD ==========================
	AIState( STATE_WAIT_FOR_CS_STANDARD )
	OnEnter
		PRINT_STATE(PlayAnimDario_Dario::STATE_WAIT_FOR_CS_STANDARD)
		m_fTimer = 0.0f;
	
	OnUpdate
		
		// Wait here until AI is in CS_STANDARD
	
		if (IsSaveToPushAController())
			SetState(GetReturnToState());

	OnExit
	
		m_fTimer = 0.0f;

EndStateMachine
}

