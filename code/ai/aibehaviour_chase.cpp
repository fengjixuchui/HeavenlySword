//! -------------------------------------------
//! aibehavior_chase.cpp
//!
//! PATROL behaviour for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#include "aibehaviour_chase.h"
#include "ai/ainavigationsystem/ainavigsystemman.h"

#include "ai/aipatrolmanager.h"
#include "aivision.h"

//#include "game/aicomponent.h"
//#include "game/entitymanager.h"
//#include "game/randmanager.h"
#include "game/messagehandler.h"

bool CAIChaseBehaviour::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
{
	UNUSED( fTimeChange );

	const float fInitTime			(0.3f);
	const float	fWalkTowardsTime	(10.0f); /// !!! - (Dario) To be parameterised 
	const float m_fMaxChaseTime		(20.0f);
	//const float	fReAcquireTime		(20.0f);

	user_error_p(0,("AI: [%s] is using the DEPRECATED behaviour -ChaseDario-. Please, use -AttackDario- instead",ntStr::GetString(m_pobEnt->GetName())));
	return false;

BeginStateMachine

	// ============= STATE_INITIALISE ===================

	AIState( STATE_INITIALISE )
		OnEnter
			PRINT_STATE(ChasePlayer_Dario::STATE_INITIALISE)
			m_fTimer = 0.0f;

			m_pobAIComp->CompleteSimpleAction();
			m_pobAIComp->SetPlayingFacingAction(false);
			m_pobAIComp->ActivateController(CAIComponent::MC_WALKING);

		OnUpdate
			m_fTimer += fTimeChange;
			m_pobAIComp->SetPlayingFacingAction(false); // Hack !!!
			if( m_fTimer > fInitTime )
			{
				SetState( STATE_CHASE_ENEMY );
			}

		OnExit
			m_fTimer = 0.0f;

	// ============= STATE_CHASE_ENEMY ===================

	AIState( STATE_CHASE_ENEMY )
		OnEnter
			PRINT_STATE(ChasePlayer_Dario::STATE_CHASE_ENEMY)
			m_fTimer = 0.0f;
			// Set any running SimpleAnimation as completed
			m_pobAIComp->CompleteSimpleAction();
			// Give the command to walk towards the player
			CAINavigationSystemMan::Get().SteerToEntity(m_pobEnt, CAINavigationSystemMan::NF_DEF_CHASE_ENEMY);
			m_pobAIComp->GetCAIMovement()->SetMaxSpeed(m_pobAIComp->GetCAIMovement()->GetChaseSpeed());
		OnUpdate
			m_fTimer += fTimeChange;
			
			if (m_pobAIComp->GetAIVision()->IsTargetInAttackRange())
			{
				// Attack!
				AI_BEHAVIOUR_SEND_MSG( CHASE_INATTACKRANGE );
				return true;
			}
			if (m_pobAIComp->GetAIVision()->DoISeeTheEnemy())
			{
				return true;
			}
			if (m_pobAIComp->GetAIVision()->DoISeeSomething())
			{
				// Move towards what seems to be the enemy
				SetState(STATE_WALK_TOWARDS_ENEMY);
			} 
			else if (m_pobAIComp->GetCAIMovement()->IsChaseTargetCompleted())
			{
				// Somehow, the player managed to desapear!! Well, then...
				SetState( STATE_LOST_ENEMY );
			}

			// Otherwise it keeps moving towards the last known position of the player

			if ( m_fTimer > m_fMaxChaseTime)
			{
				SetState(STATE_LOST_ENEMY);
			}

		OnExit
			m_fTimer = 0.0f;

	// ============= STATE_CHASE_ENEMY ===================
	AIState( STATE_WALK_TOWARDS_ENEMY )
		OnEnter
			PRINT_STATE(ChasePlayer_Dario::STATE_WALK_TOWARDS_ENEMY)
			m_fTimer = 0.0f;

			// Walk to the last enemy's pos.
			CAINavigationSystemMan::Get().SteerToEntity(m_pobEnt, CAINavigationSystemMan::NF_DEF_CHASE_ENEMY);
			m_pobAIComp->GetCAIMovement()->SetMaxSpeed(m_pobAIComp->GetCAIMovement()->GetApproachSpeed());

		OnUpdate
			m_fTimer += fTimeChange;
			
			if (m_fTimer > fWalkTowardsTime)
			{
				// Something has happend... the AI couldn't reach the last known player's pos. in a reasonable time
				// is he stucked? or the player move away long ago..
				SetState( STATE_LOST_ENEMY );
			}
			if (m_pobAIComp->GetAIVision()->DoISeeMyTarget())
			{
				// Chase the player
				SetState(STATE_CHASE_ENEMY);
			}
			else
			{
				if (m_pobAIComp->GetCAIMovement()->IsChaseTargetCompleted())
				{
					// We walk until the last known player's position.
					SetState( STATE_LOST_ENEMY );
				}
			}
			
		OnExit
			m_fTimer = 0.0f;

	// ============= STATE_LOST_ENEMY ===================

		AIState( STATE_LOST_ENEMY )
		OnEnter
			PRINT_STATE(ChasePlayer_Dario::STATE_LOST_ENEMY)
			m_fTimer = 0.0f;
			// Set any running SimpleAnimation as completed
			m_pobAIComp->CompleteSimpleAction();
			// Play Animation
			m_pobAIComp->ActivateSingleAnim("lookaround_01", 1.f, 0.f, 1.f);

		OnUpdate
			m_fTimer += fTimeChange;
			
			if( m_pobAIComp->IsSimpleActionComplete() )
			{
				if (m_pobAIComp->GetAIVision()->DoISeeMyTarget())
				{
					// Chase the player
					SetState(STATE_CHASE_ENEMY);
				}
				else
				{
					// This will put me back into patrolling mode
					AI_BEHAVIOUR_SEND_MSG( CHASE_LOSTTARGET );
				}
			}
			
		OnExit
			m_fTimer = 0.0f;

EndStateMachine

}
