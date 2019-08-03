//! -------------------------------------------
//! aibehaviour_goaroundvolumes.cpp
//!
//! GO AROUND VOLUMES behaviour for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#include "aibehaviour_goaroundvolumes.h"
#include "ai/ainavigationsystem/ainavigsystemman.h"

#include "game/aicomponent.h"
#include "game/entitymanager.h"
#include "game/messagehandler.h"
#include "game/movement.h"

//--------------------------------------------------
//!	DESTRUCTOR
//--------------------------------------------------
CAIGoAroundVolumesBehaviour::~CAIGoAroundVolumesBehaviour()
{
	if (!m_pMov)
	{
		ntPrintf("!!!! ~CAIGoAroundVolumesBehaviour() -> m_pMov is NULL !!!!\n");
		return;
	}

	CAINavigPath* pPath = m_pMov->GetPathContainer();
    
	if (pPath)
	{
		pPath->clear();
	}
}

//--------------------------------------------------
//!	CAIGoAroundVolumesBehaviour::States
//--------------------------------------------------

bool CAIGoAroundVolumesBehaviour::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
{
	const float	fInitTimeMax			(1.0f);
//	const float fCheckEnemyPosTime		(5.0f);
//	const float fSquaredTolerance		(9.0f);
	
	// Check if has an enemy
	m_pEnemy = m_pMov->GetEntityToAttack();
	if ( !m_pEnemy )
	{
		if (!m_bOneTimeWarning)
		{
			user_warn_msg( ("CAIGoAroundVolumesBehaviour: AI [%s] does NOT have an ENEMY and is meant to go to him\nThis is a ONE TIME WARNING",ntStr::GetString(m_pobEnt->GetName())));
			return false;
		}
		m_bOneTimeWarning = true;
		return false;
	}

	if (!m_pobAIComp->GetAIVision()->DoISeeNothing() &&
		!m_pobAIComp->GetAIVision()->DoISeeEnemyThroughGoAroundVolumes() && 
		IsSaveToPushAController())
	{
		// Here it comes... the enemy!!!
		AI_BEHAVIOUR_SEND_MSG( SEENENEMY );
	}

BeginStateMachine
	// ============= STATE_INITIALISE ==========================

	AIState( STATE_ZERO )
		OnEnter
			PRINT_STATE(GoAroundVolumes_Dario::STATE_ZERO)
			
			m_pMov->SetMovingWhileFacingTgt(false);
			m_pobAIComp->SetPlayingFacingAction(false); // NOTE: this should be done in the destructors of beh_attacking and beh_formation.
			m_fTimer = 0.0f;
			m_fInitTime = grandf(fInitTimeMax);
			// Store speed, navigation flags and intetion parameters
			m_uiInitialMaxSpeed = m_pMov->GetMaxSpeed();
			m_uiInitialFlags = m_pMov->GetSteeringFlags();
			unsigned int uiIntentions = m_pMov->GetIntention()->eNavIntentions;
			if ( uiIntentions != NAVINT_GO_AROUND_VOLUMES )
			{
				user_error_p(0,("Bad/None Intentions Set in a LUA Script for GoAroundVolumes Behaviour (AI: %s.)\n-Use SetIntention(...) with GO_AROUND_VOLUMES\n-Check my lovely written Confluence pages. Dario",ntStr::GetString(m_pobEnt->GetName())));
				return false;
			}

		OnUpdate
			m_fTimer += fTimeChange;
			SetState( STATE_INITIALISE );
			return true;
			//if( m_fTimer > m_fInitTime )
			//{
			//	SetState( STATE_INITIALISE );
			//}
			
		OnExit
			m_fTimer = 0.0f;

		// ============= STATE_INITIALISE ==========================

	AIState( STATE_INITIALISE )
		OnEnter
			PRINT_STATE(GoAroundVolumes_Dario::STATE_INITIALISE)
			m_fTimer = 0.0f;
			
			// Store the enemy's position and reset the enemy's check timer
			if (m_pEnemy)
			{
				m_obLastEnemyPos = m_pEnemy->GetPosition();
				m_fTimerEnemyPosCheck = 0.0f;
			}

			m_pMov->SetGoAroundCompleted(false);
		
		OnUpdate

			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_INITIALISE);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}

			// Calculate path to Enemy
			bool bCommandExecuted = false;
			bool bOk = CAINavigationSystemMan::Get().GoAroundVolume(	m_pobEnt, 
																		m_pEnemy,
																		&bCommandExecuted,
																		m_uiInitialFlags);
	
			if (!bOk)
			{
				AI_BEHAVIOUR_SEND_MSG( GO_AROUND_FAILURE );
				SetState(STATE_IDLE);
			}
			else if ( bCommandExecuted )
			{
				m_uiWalkCtrlId = m_pobEnt->GetMovement()->GetNewControllerCount();
				SetState(STATE_MOVING);			
			}

			// Otherwise, try in the next frame

		OnExit
			m_fTimer = 0.f;

	// ============= STATE_MOVING ==========================
	
	AIState( STATE_MOVING )
		OnEnter
			PRINT_STATE(GoAroundVolumes_Dario::STATE_MOVING)
			m_fTimer = 0.0f;

		OnUpdate
		//	m_fTimer += fTimeChange;
			
			if (m_pobAIComp->GetDisabled() == true )
			{
				SetReturnToState(STATE_MOVING);
				SetState(STATE_WAIT_UNTIL_RECOVERED);
				return true;
			}
			
			if (m_pMov->IsGoAroundCompleted())
			{
				// End of the goaround path reached
				SetState(STATE_PATH_FINISHED);
				return true;
			}
			//if (m_fTimer > fFollowPathTime )
			//{
			//	// Something bad happened (got stuck?)
			//	AI_BEHAVIOUR_SEND_MSG( TIME_OUT );
			//	SetState(STATE_IDLE);				
			//}
			
		OnExit
			m_fTimer = 0;

	// ============= STATE_WAIT_UNTIL_RECOVERED ==========================
	
	AIState( STATE_WAIT_UNTIL_RECOVERED )
		OnEnter
			PRINT_STATE(FollowPathWithCover_Dario::STATE_WAIT_UNTIL_RECOVERED)
			m_fTimer = 0.0f;

		OnUpdate
			// Stay here until the AI recovers
			if ( !m_pobAIComp->GetDisabled() )
			{
				// Push back the controller
				m_pobAIComp->ActivateController(CAIComponent::MC_WALKING);
				SetState(GetReturnToState());
			}

		OnExit
			m_fTimer = 0.f;

	// ============= STATE_IDLE ==========================
	AIState( STATE_IDLE )
		OnEnter
			PRINT_STATE(FollowPathWithCover_Dario::STATE_IDLE)
			// Clear previous animations
			m_pobAIComp->CompleteSimpleAction();
		//	m_pMov->DeactivateFlag
			// After couple of seconds, try again
			SetUserTimer(grandf(2.0f));
			SetReturnToState(STATE_INITIALISE);
			SetState(STATE_WAIT_FOR_TIMER);

		OnUpdate
			
		OnExit

		// ============= STATE_PATH_FINISHED ==========================
		AIState( STATE_PATH_FINISHED )
		OnEnter
			PRINT_STATE(FollowPathWithCover_Dario::STATE_PATH_FINISHED)
			m_fTimer = 0.0f;
			
		OnUpdate
			
			// AI has walked around the first of the volumes in front of him.
			// ... but there may be more

			if ( m_pobAIComp->GetAIVision()->DoISeeEnemyThroughGoAroundVolumes() )
			{
				SetState(STATE_INITIALISE);
				return true;
			}

		OnExit
		
			m_fTimer = 0.0f;

		// ============= STATE_WAIT_FOR_CS_STANDARD ==========================
		AIState( STATE_WAIT_FOR_CS_STANDARD )
		OnEnter
			PRINT_STATE(FollowPathWithCover_Dario::STATE_WAIT_FOR_CS_STANDARD)
			m_fTimer = 0.0f;
		
		OnUpdate
			
			// Wait here until AI is in CS_STANDARD
		
			if (IsSaveToPushAController())
				SetState(GetReturnToState());

		OnExit
		
			m_fTimer = 0.0f;

		// ============= STATE_WAIT_FOR_TIMER ==========================
		AIState( STATE_WAIT_FOR_TIMER )
		OnEnter
			PRINT_STATE(FollowPathWithCover_Dario::STATE_WAIT_FOR_TIMER)
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
			

EndStateMachine
}

