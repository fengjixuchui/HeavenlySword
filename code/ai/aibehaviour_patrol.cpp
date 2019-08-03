//! -------------------------------------------
//! aibehaviour_patrol.cpp
//!
//! PATROL behaviour for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#include "aibehaviour_patrol.h"
#include "ai/ainavigationsystem/ainavigsystemman.h"
#include "aiinitialreactionman.h"

#include "ai/aipatrolmanager.h"

//#include "game/entitymanager.h"
#include "game/entityinfo.h"
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
CAIPatrolBehaviour::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
{
	//const float fChanceOfIdleStart	(0.5f);
//	const float	fInitTime			(0.5f);
//	const float	fWalkTime			(200.0f);
//	const float	fApproachTime		(0.1f);
	const float	fAlertedDelayTime	(0.7f);
	const float	fSeenTime			(1.0f);
	
	CAIVision* pVis = m_pobAIComp->GetAIVision();
	if (!pVis) return false;

BeginStateMachine

	// ============= STATE_INITIALISE ==========================

	AIState( STATE_INITIALISE )
		OnEnter
			PRINT_STATE(Patrol_Dario::STATE_INITIALISE)
			m_fTimer = 0.0f;
			
			// Check if AI is in an initial reaction group
			m_bHasInitialReaction =  false; //CAIInitialReactionMan::Get().IsParticipant(m_pobEnt->ToAI());

			// Check if I can see my patrol node

			bool bSuccess		= false;
			bool bISeeMyPath	= false;

			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_INITIALISE);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}
			// Can I find my way to the patrol path?
			bISeeMyPath = CAINavigationSystemMan::Get().FindPatrolPath(m_pobEnt,0.7f, &bSuccess);

			if (bISeeMyPath)
			{
				ntPrintf("Walking towards my patrol path...");
				SetState( STATE_WALK );
			}
			else
			{
				// Check if I will find it
				if (!bSuccess)
				{
					// It doesn't look good... I quit patrolling
					SetState( STATE_IDLE );
				}
				// Otherwise I will stay here until I reach my patrol path
			}

		OnUpdate

			// Walking towards my patrol path

			if (pVis->DoISeeTheEnemy())
			{
				SetState( STATE_SPOTTED );
			}
			if (pVis->DoISeeSomething())
			{
				SetState( STATE_SPOTTED );
			}	
			if ( m_pobAIComp->GetCAIMovement()->IsFollowPathCompleted() )
			{
				// I reached my patrol path
				SetState( STATE_WALK );
			}

		OnExit
			m_fTimer = 0.f;

	// ============= STATE_WALK ==========================
	
	AIState( STATE_WALK )
		OnEnter
			PRINT_STATE(Patrol_Dario::STATE_WALK)
			m_fTimer = 0.0f;

			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_WALK);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}
			// Give the command to start walking the path
			CAINavigationSystemMan::Get().FollowPatrolPath(m_pobEnt,0.5,CAINavigationSystemMan::NF_DEF_PATROL_WALK);

		OnUpdate
			m_fTimer += fTimeChange;

			const AI* pAI = CAIInitialReactionMan::Get().GetFirstReporter();

			if ( m_bHasInitialReaction && pAI )
			{
				// Some AI has reported the presence of the enemy
				m_bHasInitialReaction = false;
				//CAIInitialReactionMan::Get().RemoveAI(m_pobEnt);
				SetState(STATE_MOVE_TO_REPORTER);
				return true;
			}
			
			//CPoint obAlertPlayerPos;
			//if (AIAlertManager::Get().ActiveAlertInRange( m_pobEnt->GetPosition(), obAlertPlayerPos ))
			//{
			//	SetState( STATE_ALERTED );
			//}
			if (m_pobAIComp->GetAIVision()->DoISeeTheEnemy())
			{
				m_AfterTurnState = STATE_SPOTTED;
				SetState( STATE_TURNTOFACE );
			}
			if (m_pobAIComp->GetAIVision()->DoISeeSomething())
			{
				m_AfterTurnState = STATE_SPOTTED;
				SetState( STATE_TURNTOFACE );
			}
			//if( m_fTimer > fWalkTime )
			//{
			//	SetState( STATE_IDLE );
			//}
			
		OnExit
			m_fTimer = 0;

	// ============= STATE_IDLE ==========================

	AIState( STATE_IDLE )
		OnEnter
			PRINT_STATE(Patrol_Dario::STATE_IDLE)
			m_fTimer = 0.0f;
	
			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_IDLE);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}

			// Play Animation
			//m_pobAIComp->ActivateSingleAnim("PatrolLook1");

		OnUpdate
			//m_fTimer += fTimeChange;
			//if( m_pobAIComp->IsSimpleActionComplete() )
			//{
			//	if (m_fTimer > erandf(10.0f))
			//	{
			//		SetState( STATE_WALK );
			//	}
			//}
			
		OnExit
			m_fTimer = 0;

	// ============= STATE_SPOTTED ==========================

	AIState( STATE_SPOTTED )
		OnEnter
			PRINT_STATE(Patrol_Dario::STATE_SPOTTED)
			m_fTimer = 0.0f;

			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_SPOTTED);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}

			// At this point, the AI already faces the right direction
			// (1) Clear previous animations
			m_pobAIComp->CompleteSimpleAction();
			// (2) Play Single Animation and VO
			//m_pobAIComp->ActivateSingleAnim("PatrolSpotted");
			//m_pobAIComp->PlayVO( "voice_sb", "vo_shld_enter_chase" );
			AI_BEHAVIOUR_SEND_MSG( PATROL_SEENENEMY );

		OnUpdate
			m_fTimer += fTimeChange;

			// As soon as the animation is finished, move on.
			//if( m_pobAIComp->IsSimpleActionComplete() )
			//{
			//	AI_BEHAVIOUR_SEND_MSG( PATROL_SEENENEMY );
			//	//SetState( STATE_ALERT );
			//}
			//
		OnExit
			m_fTimer = 0.0f;

	// ============= STATE_TURNTOFACE ==========================
	AIState( STATE_TURNTOFACE )
		OnEnter
			PRINT_STATE(Patrol_Dario::STATE_TURNTOFACE);
			m_fTimer = 0.0f;
			
			m_pobAIComp->SetPlayingFacingAction(true); 

		OnUpdate
			m_fTimer += fTimeChange;
			
			m_obEntityPos = CEntityManager::Get().GetPlayer()->GetPosition();
			m_obFacing = CDirection(m_obEntityPos - m_pobEnt->GetPosition());
			m_obFacing.Normalise();
			m_pobAIComp->GetCAIMovement()->SetFacingAction(m_obFacing);
			CDirection currentFacing;
			
			m_pobEnt->GetLookDirection( currentFacing );
			currentFacing.Normalise();
			
			float fFacingDotProduct = m_obFacing.Dot(currentFacing);
			if ( fFacingDotProduct > 0.98 )
			{
				m_pobAIComp->SetPlayingFacingAction(false);
				SetState( m_AfterTurnState );
			}
			
		OnExit
			m_pobAIComp->SetPlayingFacingAction(false);
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_ALERTED )
		OnEnter
			PRINT_STATE(Patrol_Dario::STATE_ALERTED)
			m_fTimer = 0.0f;

			// on entry, set the action to alerted
			m_pobAIComp->CompleteSimpleAction();
			m_pMov->PlayAnimation( ACTION_PATROL_ALERTED );

		OnUpdate
			m_fTimer += fTimeChange;

			if( m_pobAIComp->IsSimpleActionComplete() )
			{
				m_pobAIComp->CanAlwaysSeePlayer( true ); // Needed?
				SetState( STATE_SEENSOMETHING );
			}
			
		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_SEENSOMETHING )
		OnEnter
			PRINT_STATE(Patrol_Dario::STATE_SEENSOMETHING)
			m_fTimer = 0.0f;

			AI_BEHAVIOUR_SEND_MSG( PATROL_SEENENEMY );

		OnUpdate
			m_fTimer += fTimeChange;
	
			if( m_fTimer > fAlertedDelayTime )
			{
				m_pobAIComp->SetAlerted( true );
			}

			if( m_fTimer > fSeenTime )
			{
				// raise an alert for other AIs
				AIAlertManager::Get().SendAlert( m_pobEnt->GetPosition(), m_pobAIComp->GetActualPlayerPos() );

				if (m_pobAIComp->CanSeePlayer())
				{
					AI_BEHAVIOUR_SEND_MSG( PATROL_SEENENEMY );
				}
				else
				{
					AI_BEHAVIOUR_SEND_MSG( PATROL_SEENSOMETHING );
				}

				// set the state for when we get back
				SetState( STATE_RETURN );
			}
			
		OnExit
			m_fTimer = 0.0f;

	// ============= STATE_RETURN ==========================
	AIState( STATE_RETURN )
		OnEnter
			PRINT_STATE(Patrol_Dario::STATE_RETURN)
		OnUpdate
			SetState( STATE_WALK );
		OnExit

	// ============= STATE_MOVE_TO_REPORTER ==========================
	AIState( STATE_MOVE_TO_REPORTER )
		OnEnter
			PRINT_STATE(Patrol_Dario::STATE_MOVE_TO_REPORTER)
			AI* pAI = CAIInitialReactionMan::Get().GetFirstReporter();
			bool bOk = false;

	//		if ( m_bHasInitialReaction && pAI )
			{
				// Some AI has reported the presence of the enemy
				bOk = CAINavigationSystemMan::Get().FollowPathToReporter(m_pobEnt->ToAI(), pAI, 1.0f);

				if (bOk)
				{
					ntPrintf("Moving towards the reporter...");
				}
				else
				{
					// It doesn't look good... Stop
					SetState( STATE_IDLE );
				}
			}
		OnUpdate
			
			if (!pVis->DoISeeNothing())
			{
				AI_BEHAVIOUR_SEND_MSG( SEENENEMY );
				AI_BEHAVIOUR_SEND_MSG( PATROL_SEENENEMY );
			}
			
			if (m_pMov->IsFollowPathCompleted())
			{
				// mmm... I have not seen the enemy ... 
				AI_BEHAVIOUR_SEND_MSG( PATH_FINISHED );
			}

		OnExit

	// ============= STATE_WAIT_FOR_CS_STANDARD ==========================
	AIState( STATE_WAIT_FOR_CS_STANDARD )
	OnEnter
		PRINT_STATE(Patrol_Dario::STATE_WAIT_FOR_CS_STANDARD)
		m_fTimer = 0.0f;
	
	OnUpdate
		
		// Wait here until AI is in CS_STANDARD
	
		if (IsSaveToPushAController())
			SetState(GetReturnToState());

	OnExit
	
		m_fTimer = 0.0f;

EndStateMachine
}

