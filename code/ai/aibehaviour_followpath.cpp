//! -------------------------------------------
//! aibehaviour_followpath.cpp
//!
//! PATROL behaviour for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#include "aibehaviour_followpath.h"
#include "ai/ainavigationsystem/ainavigsystemman.h"

#include "ai/aipatrolmanager.h"

#include "game/entityinfo.h"
#include "game/aicomponent.h"
#include "game/entitymanager.h"
#include "game/randmanager.h"
#include "game/messagehandler.h"

//--------------------------------------------------
//!	CAIFollowPathAndCoverBehaviour::States
//--------------------------------------------------

bool CAIFollowPathAndCoverBehaviour::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
{
	//const float fChanceOfIdleStart	(0.5f);
//	const float	fInitTime			(0.5f);
	const float	fFollowPathTime			(60.0f);
	const float fMoveToCoverPointTime	(10.0f);

	if (!m_pMov) return false;

BeginStateMachine

	// ============= STATE_INITIALISE ==========================

	AIState( STATE_INITIALISE )
		OnEnter
			PRINT_STATE(FollowPathWithCover_Dario::STATE_INITIALISE)
			m_fTimer = 0.0f;
			
			m_pMov->SetFollowPathCompleted(false);

			// Check the validity of the start - end nodes
			//if (!m_pMov->IsDestinationNodeValid())
			//{
			//	SetState(STATE_INVALID_DEST_NODE);
			//	return true;
			//}
			//
			//if (!m_pMov->IsStartNodeValid())
			//{
			//	SetState(STATE_);
			//	return true;
			//}

			bool bExistPath = CAINavigationSystemMan::Get().FollowPath(	m_pobEnt );

			m_pobAIComp->ActivateController(CAIComponent::MC_WALKING);
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
			PRINT_STATE(FollowPathWithCover_Dario::STATE_MOVING)
			m_fTimer = 0.0f;

		OnUpdate
			m_fTimer += fTimeChange;
			
			if (m_pMov->IsMovingToCover())
			{
				// Move to the cover point set in my new intention
				AI_BEHAVIOUR_SEND_MSG ( MSG_MOVING_TO_COVER_POINT );
				SetState(STATE_MOVING_TO_COVER_POINT);
				return true;
			}
			if (m_pMov->IsFollowPathCompleted())
			{
				// End of the path reached
				AI_BEHAVIOUR_SEND_MSG( DESTINATION_REACHED );
				SetState(STATE_IDLE);
				return true;
			}
			if (m_fTimer > fFollowPathTime )
			{
				// Something bad happened (got stuck?)
				AI_BEHAVIOUR_SEND_MSG( DESTINATION_UNREACHABLE );
				SetState(STATE_IDLE);				
			}
			
		OnExit
			m_fTimer = 0;

	// ============= STATE_MOVING_TO_COVER_POINT ==========================

	AIState( STATE_MOVING_TO_COVER_POINT )
		OnEnter
			PRINT_STATE(FollowPathWithCover_Dario::STATE_MOVING_TO_COVER_POINT)
			m_fTimer = 0.0f;
	
			// Check that there is a valid cover node
			if (!m_pMov->GetCoverNode()) 
			{
				// Keep on with the path
				AI_BEHAVIOUR_SEND_MSG( DESTINATION_UNREACHABLE );
				m_pMov->SetGoingToCover(false);
				SetState(STATE_MOVING);
				return false;
			}

			// Store movement flags and set destination (cover point)
			m_uiPathMovFlags = m_pMov->GetMovementParams()->m_uSteeringFlags;

			// Give the order (it disables the follow path movement)
			CAINavigationSystemMan::Get().SteerToDestinationPoint(	m_pobEnt, 
																	m_pMov->GetCoverNode()->GetPos(),
																	sqrt(m_pMov->GetCoverNode()->GetRadiusSQR())
																);		

		OnUpdate
			
			if (m_pMov->IsMoveToSelectedPointCompleted())
			{
				AI_BEHAVIOUR_SEND_MSG( COVER_POINT_REACHED );
				SetState(STATE_TAKING_COVER);
				return true;
			} 
			if ( m_fTimer > fMoveToCoverPointTime )
			{
				AI_BEHAVIOUR_SEND_MSG( DESTINATION_UNREACHABLE );
				
				// Push the old flags and contonue with the path
				m_pMov->GetMovementParams()->m_uSteeringFlags = m_uiPathMovFlags;
				SetState(STATE_MOVING);
			}
			
		OnExit
			m_fTimer = 0;

	// ============= STATE_TAKING_COVER ==========================

	AIState( STATE_TAKING_COVER )
		OnEnter
			PRINT_STATE(FollowPathWithCover_Dario::STATE_TAKING_COVER)
			m_fTimer = 0.0f;

			// Clear previous animations
			m_pobAIComp->CompleteSimpleAction();

			// Play a Taking_Cover animation
	//		CHashedString hsTakeCoverAnim = m_pMov->GetCoverNode()->GetTakingCoverAnim();

//			m_pobAIComp->ActivateSingleAnim(hsTakeCoverAnim);

		OnUpdate
			m_fTimer += fTimeChange;

			// As soon as the animation is finished, move on.
			if( m_pobAIComp->IsSimpleActionComplete() )
			{
				SetState( STATE_COVERING );
			}
			
		OnExit
			m_fTimer = 0.0f;

	// ============= STATE_COVERING ==========================
	AIState( STATE_COVERING )
		OnEnter
			PRINT_STATE(FollowPathWithCover_Dario::STATE_COVERING);
			m_fTimer = 0.0f;
			
			// Decide what to do (which anims to play)

		OnUpdate
			m_fTimer += fTimeChange;
			
			if ( m_fTimer > 5.0f)
				SetState(STATE_BREAKING_COVER);
			
		OnExit
			m_fTimer = 0.0f;

	// ============= STATE_PLAYING_PEEKING_ANIM ==========================
	AIState( STATE_PLAYING_PEEKING_ANIM )
		OnEnter
			PRINT_STATE(FollowPathWithCover_Dario::STATE_PLAYING_PEEKING_ANIM)
			m_fTimer = 0.0f;

			
		OnUpdate
			m_fTimer += fTimeChange;

			if( m_pobAIComp->IsSimpleActionComplete() )
			{
				// Back to covering
				SetState( STATE_COVERING );
			}
			
		OnExit
			m_fTimer = 0.0f;

	// ============= STATE_BREAKING_COVER ==========================
	AIState( STATE_BREAKING_COVER )
		OnEnter
			PRINT_STATE(FollowPathWithCover_Dario::STATE_BREAKING_COVER)
			m_fTimer = 0.0f;

			// Clear previous animations
			m_pobAIComp->CompleteSimpleAction();

			// Play a Breaking_Cover animation
//			CHashedString hsBreakCoverAnim = m_pMov->GetCoverNode()->GetBreakingCoverAnim();

	//		m_pobAIComp->ActivateSingleAnim(hsBreakCoverAnim.GetCharString());

		OnUpdate
			m_fTimer += fTimeChange;

			// As soon as the animation is finished, move on.
			if( m_pobAIComp->IsSimpleActionComplete() )
			{
				// Push the follow path flags and continue with the path
				m_pMov->GetMovementParams()->m_uSteeringFlags = m_uiPathMovFlags;
				SetState( STATE_MOVING );
			}
			
		OnExit
			m_fTimer = 0.0f;

	// ============= STATE_BREAKING_COVER ==========================
	AIState( STATE_IDLE )
		OnEnter
			PRINT_STATE(FollowPathWithCover_Dario::STATE_IDLE)
			// Clear previous animations
			m_pobAIComp->CompleteSimpleAction();

		OnUpdate
			
		OnExit

EndStateMachine
}



