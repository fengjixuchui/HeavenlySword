//! -------------------------------------------
//! aibehaviour_initialreaction.h
//!
//! Initial Reaction behaviour for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#include "aibehaviour_initialreaction.h"
#include "aiinitialreactionman.h"
#include "game/aicomponent.h"
#include "game/chatterboxman.h"

//--------------------------------------------------
//!	CAIInitialReactionBehaviour::States
//--------------------------------------------------

bool CAIInitialReactionBehaviour::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
{
	UNUSED(fTimeChange);

	CAIVision* pVis = m_pobAIComp->GetAIVision();
	CAIHearing* pHear = m_pobEnt->GetAIComponent()->GetAIHearing();
	
	if (!pVis) return false;

BeginStateMachine

	// ============= STATE_INITIALISE ==========================
	AIState( STATE_INITIALISE )
		OnEnter
			PRINT_STATE(InitialReaction_Dario::STATE_INITIALISE);
			m_fTimer = 0.0f;

			// Initialise
			pHear->SetSoundSourceEntity( NULL );
			CAIInitialReactionMan::Get().SetReporterAnimFinished(false);
			CAIInitialReactionMan::Get().ResetClosestAIToSound();

			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_INITIALISE);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}

			// Play the Selected Idle Animation
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
				SetState( STATE_PLAYING_IDLE_ANIM );
				m_pobAIComp->CompleteSimpleAction();
			}
			else if( !m_pobAIComp->IsSimpleActionComplete() )
			{
				m_pobAIComp->SetScriptAnimLooping(true);
				AI_BEHAVIOUR_SEND_MSG( ANIM_STARTED );
				SetState( STATE_PLAYING_IDLE_ANIM );
			}

			
		OnExit
			m_fTimer = 0.0f;

	// ============= STATE_PLAYINGANIM ==========================

	AIState( STATE_PLAYING_IDLE_ANIM )
		OnEnter
			PRINT_STATE(InitialReaction_Dario::STATE_PLAYING_IDLE_ANIM);
			m_fTimer = 0.0f;

		OnUpdate

			// ----------------------------
			// Keep track of the animation
			// ----------------------------

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
					SetState( STATE_COMPLETED_IDLE_ANIM );
				}
			}
			
			// ---------------------------
			// Be aware of visual contact
			// ---------------------------

			if ( CAIInitialReactionMan::Get().GetFirstReporter() != NULL )
			{
				SetState( STATE_WAIT_FOR_FINISHED_SPOTTER );
				return true;
			}

			if (!pVis->DoISeeNothing())
			{
				//AI_BEHAVIOUR_SEND_MSG( INITIAL_REACTION_FIRST_REPORTER );
				CAIInitialReactionMan::Get().Report(m_pobEnt->ToAI());
				CAIInitialReactionMan::Get().SetReporterAnimFinished(false);
				pHear->SetSoundSourceEntity( NULL );
				SetState( STATE_SPOTTER );
				return true;
			}

			// ---------------------------
			// Be aware of sounds
			// ---------------------------
			if ( !pHear || pHear->IsDeaf() || !pHear->GetSoundSourceEntity() )
				return false;
			else
			{
				CPoint obSndPos = pHear->GetSoundSourceLocation();
				CAIInitialReactionMan::Get().PropagateAlertSound( m_pobEnt );
				CEntity* pCE = CAIInitialReactionMan::Get().UpdateClosestAIToSound(obSndPos);
				if (!pCE || !pCE->IsAI())
				{
					PRINT_STATE(InitialReaction_Dario::STATE_PLAYING_IDLE_ANIM--->ERRORRRRRR!!!);
					return false;
				}
				pHear->SetSoundSourceEntity( NULL );
				SetState( STATE_SOUND_ALERT );
				return true;
			}

		OnExit
			m_fTimer = 0;

	// ============= STATE_SOUND_ALERT ==========================
	AIState( STATE_SOUND_ALERT )
		OnEnter
			PRINT_STATE(InitialReaction_Dario::STATE_SOUND_ALERT);
	
			// Play the "Alerted by sound" anim
			m_pobAIComp->CompleteSimpleAction();
			m_pobAIComp->SetScriptAnimName(ntStr::GetString(CAIInitialReactionMan::Get().GetSoundAlertedAnim()));
			bool bOK = m_pMov->PlayAnimation( ACTION_SCRIPTANIM );
			if (!bOK)
			{
				AI_BEHAVIOUR_SEND_MSG( ANIM_FAILED );
				SetState( STATE_ANIM_FAILED );
				return false;
			}
			// Play the "Alerted by sound" VO
			CChatterBoxMan::Get().Trigger("HeardSomething",CEntityManager::Get().GetPlayer());

		OnUpdate
			if	( m_pobAIComp->IsSimpleActionComplete() )
			{
				CEntity* pCE = CAIInitialReactionMan::Get().GetClosestAIToSound();
				if ( pCE == m_pobEnt)
				{
					CAIInitialReactionMan::Get().SetReporterAnimFinished(false);
					SetState(STATE_SPOTTER);
				}
				else
				{
					SetState(STATE_WAIT_FOR_FINISHED_SOUND_SPOTTER);
				}
			}
		OnExit

	// ============= STATE_COMPLETED_IDLE_ANIM ==========================
	AIState( STATE_COMPLETED_IDLE_ANIM )
		OnEnter
			PRINT_STATE(InitialReaction_Dario::STATE_COMPLETED_IDLE_ANIM);
			m_pobAIComp->CompleteSimpleAction();

		OnUpdate
			
		OnExit

	// ============= STATE_SPOTTER ==========================
	AIState( STATE_SPOTTER )
		OnEnter
			PRINT_STATE(InitialReaction_Dario::STATE_SPOTTER);
			m_pobAIComp->CompleteSimpleAction();
			
			m_pobAIComp->SetScriptAnimName(ntStr::GetString(CAIInitialReactionMan::Get().GetReportEnemyAnim()));
			bool bOK = m_pMov->PlayAnimation( ACTION_SCRIPTANIM );
			if (!bOK)
			{
				AI_BEHAVIOUR_SEND_MSG( ANIM_FAILED );
				SetState( STATE_ANIM_FAILED );
				return false;
			}

			// Play the "Player Spotted" VO
			CChatterBoxMan::Get().Trigger("PlayerSpotted",CEntityManager::Get().GetPlayer());

		OnUpdate
			if	( m_pobAIComp->IsSimpleActionComplete() )
			{
				CAIInitialReactionMan::Get().SetReporterAnimFinished(true);
				AI_BEHAVIOUR_SEND_MSG( INITIAL_REACTION_FIRST_REPORTER );
				SetState(STATE_IDLE);
			}

		OnExit

	// ============= STATE_WAIT_FOR_SPOTTER ==========================
	AIState( STATE_WAIT_FOR_FINISHED_SPOTTER )
		OnEnter
			PRINT_STATE(InitialReaction_Dario::STATE_WAIT_FOR_FINISHED_SPOTTER);
			m_pobAIComp->CompleteSimpleAction();
			m_pobAIComp->SetScriptAnimName(ntStr::GetString(CAIInitialReactionMan::Get().GetResponseEnemyAnim()));

		OnUpdate
	
			if ( CAIInitialReactionMan::Get().GetReporterAnimFinished() == true || !CAIInitialReactionMan::Get().IsFirstReporterAlive() )
			{
				AI_BEHAVIOUR_SEND_MSG( INITIAL_REACTION_NON_FIRST_REPORTER );
				bool bOK = m_pMov->PlayAnimation( ACTION_SCRIPTANIM );
				if (!bOK)
				{
					AI_BEHAVIOUR_SEND_MSG( ANIM_FAILED );
					SetState( STATE_ANIM_FAILED );
					return false;
				}

				// Play the "Player Spotted Response" VO
				CChatterBoxMan::Get().Trigger("SpottedResponse",CEntityManager::Get().GetPlayer());
				
				SetState(STATE_WAIT_FOR_RESPONSE_ANIM_FINISHED);
			}

		OnExit

		// ============= STATE_WAIT_FOR_SPOTTER ==========================
	AIState( STATE_WAIT_FOR_FINISHED_SOUND_SPOTTER )
		OnEnter
			PRINT_STATE(InitialReaction_Dario::STATE_WAIT_FOR_FINISHED_SPOTTER);
			m_pobAIComp->CompleteSimpleAction();
			m_pobAIComp->SetScriptAnimName(ntStr::GetString(CAIInitialReactionMan::Get().GetResponseEnemyAnim()));

		OnUpdate
			
			CEntity* pCE = CAIInitialReactionMan::Get().GetClosestAIToSound();

			if ( CAIInitialReactionMan::Get().GetReporterAnimFinished() == true || !pCE || pCE->ToAI()->IsDead() )
			{
				AI_BEHAVIOUR_SEND_MSG( INITIAL_REACTION_NON_FIRST_REPORTER );
				bool bOK = m_pMov->PlayAnimation( ACTION_SCRIPTANIM );
				if (!bOK)
				{
					AI_BEHAVIOUR_SEND_MSG( ANIM_FAILED );
					SetState( STATE_ANIM_FAILED );
					return false;
				}

				// Play the "Player Spotted Response" VO
				CChatterBoxMan::Get().Trigger("SpottedResponse",CEntityManager::Get().GetPlayer());
				
				SetState(STATE_WAIT_FOR_RESPONSE_ANIM_FINISHED);
			}

		OnExit

	// ============= STATE_WAIT_FOR_RESPONSE_ANIM_FINISHED ==========================
	AIState( STATE_WAIT_FOR_RESPONSE_ANIM_FINISHED )
		OnEnter
			PRINT_STATE(InitialReaction_Dario::STATE_WAIT_FOR_RESPONSE_ANIM_FINISHED);
		OnUpdate
			if	( m_pobAIComp->IsSimpleActionComplete() )
			{
				AI_BEHAVIOUR_SEND_MSG( INITIAL_REACTION_RESPONSE_FINISHED );
				SetState(STATE_IDLE);
			}
		OnExit

	// ============= STATE_IDLE ==========================
		AIState( STATE_IDLE )
		OnEnter
			PRINT_STATE(InitialReaction_Dario::STATE_IDLE)
			m_fTimer = 0.0f;
		
		OnUpdate
			
		OnExit
		
			m_fTimer = 0.0f;

	// ============= STATE_ANIM_FAILED ==========================
		AIState( STATE_ANIM_FAILED )
		OnEnter
			PRINT_STATE(InitialReaction_Dario::STATE_ANIM_FAILED)
			m_fTimer = 0.0f;
		
		OnUpdate
			
		OnExit
		
			m_fTimer = 0.0f;
	
	// ============= STATE_WAIT_FOR_TIMER ==========================
		AIState( STATE_WAIT_FOR_TIMER )
		OnEnter
			PRINT_STATE(InitialReaction_Dario::STATE_WAIT_FOR_TIMER)
			m_fTimer = 0.0f;
		
		OnUpdate
			
			m_fTimer += fTimeChange;
			// Wait here until the timer expires
		
			if ( m_fTimer > m_fUserTimer )
			{
				SetState(GetReturnToState());
			}

		OnExit
		
			m_fTimer = 0.0f;
			m_fUserTimer = 0.0f;

	// ============= STATE_WAIT_FOR_CS_STANDARD ==========================
	AIState( STATE_WAIT_FOR_CS_STANDARD )
	OnEnter
		PRINT_STATE(InitialReaction_Dario::STATE_WAIT_FOR_CS_STANDARD)
		m_fTimer = 0.0f;
	
	OnUpdate
		
		// Wait here until AI is in CS_STANDARD
	
		if (IsSaveToPushAController())
			SetState(GetReturnToState());

	OnExit
	
		m_fTimer = 0.0f;

EndStateMachine
}



