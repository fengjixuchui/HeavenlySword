//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file aibehaviour_formation.cpp
//!                                                                                         
//------------------------------------------------------------------------------------------
                                                                                         
                                                                                         
//------------------------------------------------------------------------------------------
// Includes                                                                                 
//------------------------------------------------------------------------------------------

#include "ai/aibehaviour_formation.h"
#include "ai/aiformation.h"
#include "ai/aiformationcomponent.h"
#include "ai/aiformationattack.h"
#include "game/aicomponent.h"
#include "game/messagehandler.h"
#include "ai/ainavigationsystem/ainavigsystemman.h"
#include "ai/aiformation_circle.h"

//------------------------------------------------------------------------------------------
//!  public constructor  AIBehaviour_Formation
//!
//!  @param [in, out]  pobEnt AI *    
//!
//!  @author GavB @date 20/10/2006
//------------------------------------------------------------------------------------------
AIBehaviour_Formation::AIBehaviour_Formation(AI* pobEnt) : 
	CAIStateMachine(pobEnt)
{
	m_bBeenInFormation = false;

	// This behaviour has code that requires updateing even when the entity is reacting to combat
	m_UpdateFlags |= FORCE_UPDATE_WHEN_COMBAT_RECOVERING_F;

	// Formation component ref counting. 
	AIFormationComponent* pFormComp = pobEnt->GetAIComponent()->GetAIFormationComponent();
	ntAssert( pFormComp && "Error: Entity doesn't have an ai formation component" );

	// Increase the behaviour ref counts.
	pFormComp->BehaviourRefCountInc();

	// Code should handle the case to this problem but I'm just testing for the condition.
	ntAssert( pFormComp->BehaviourRefCount() == 1 && "Warning assert: If you get this please tell me, cheers. Gavb" );
}

//------------------------------------------------------------------------------------------
//!  public destructor  ~AIBehaviour_Formation
//!
//!  @author GavB @date 20/10/2006
//------------------------------------------------------------------------------------------
AIBehaviour_Formation::~AIBehaviour_Formation()
{
	// If the entity isn't running the formation behaviour, then remove it from the formation. 
	AIFormationComponent* pFormComp = m_pobEnt->GetAIComponent()->GetAIFormationComponent();
	ntAssert( pFormComp && "Error: Entity doesn't have an ai formation component" );

	// Decrement the refcount, remove the entity from the formation if the ref count is zero
	if( pFormComp->BehaviourRefCountDec() )
	{
		pFormComp->Remove();
	}
}

//------------------------------------------------------------------------------------------                                                                                      
//!	AIBehaviour_Formation::States			                                                
//! Update the formation behaviour state machine.                                                                                             
//------------------------------------------------------------------------------------------
bool AIBehaviour_Formation::States(const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange)
{
	UNUSED(fTimeChange);
	
	// Cache a vision object pointer 
	const CAIVision* pVis = m_pobAIComp->GetAIVision();
	if (!pVis) return false;

	// Is the AI behind a GoAround Volume
	if ( pVis->DoISeeEnemyThroughGoAroundVolumes() )
	{
		//AI_BEHAVIOUR_SEND_MSG( ATTACK_LOSTTARGET ); // Probably a new message is needed like ATTACK_ENEMY_BEHIND_GOAROUND_VOLUME
		AI_BEHAVIOUR_SEND_MSG (ATTACK_ENEMY_BEHIND_GOAROUND_VOLUME);
		//return false;
	}

	if ( pVis->DoISeeNothing() )
	{
		//AI_BEHAVIOUR_SEND_MSG( ATTACK_LOSTTARGET ); // Probably a new message is needed like ATTACK_ENEMY_BEHIND_GOAROUND_VOLUME
		AI_BEHAVIOUR_SEND_MSG (ATTACK_LOSTTARGET);
		//return false;
	}

	// Update the Combat System (no movement)
	m_pobAIComp->UpdateCombatComponent(fTimeChange,false);


	// Get the Formation
	m_pFormation = m_pobAIComp->GetAIFormationComponent()->GetFormation();
	
	if(!m_pFormation) 
		return false;

	// Get the slot assigned to this AI
	m_pSlot = m_pFormation->FindSlot(*m_pobEnt);
	
	if(!m_pSlot) 
		return false;

BeginStateMachine

	// ============= STATE_INITIALISE ==========================
	AIState(STATE_INITIALISE)
		OnEnter
			PRINT_STATE(Formation_Dario::STATE_INITIALISE)
			m_fTimer = 0.0f;

			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_INITIALISE);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}

			// Stop Ani animation
			m_pMov->CancelSingleAnim();
			// Push the strafe movement controller
			m_pobAIComp->ActivateController(CAIComponent::MC_STRAFING);
			// Activate Formation AI avoidance Radius
			m_pMov->ActivateFormationAIAvoidanceRadius(true);
		
		OnUpdate
			m_fTimer +=fTimeChange;
			
			// Hack to avoid people flying (at least 1 frame delay is needed)
			if (m_fTimer>0.5)
			{
				// Go to the main state
				SetState(STATE_FORMATION_MOVE_COMMAND);
			}
						
		OnExit
			m_fTimer = 0;

	// =========== STATE_FORMATION_POINT_REACHED ===============

	AIState(STATE_FORMATION_POINT_REACHED)
		OnEnter
			PRINT_STATE(Formation_Dario::STATE_FORMATION_POINT_REACHED)

			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_FORMATION_POINT_REACHED);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}

			// Play one of the in formation animations
			m_pobAIComp->GetCAIMovement()->PlayAnimation(ACTION_INFORMATION_ANIM);

			// Set Entity in Position
			m_pSlot->SetInPosition(true);
			m_pobAIComp->SetFormationIdleAnim( m_pFormation->GetIdleAnim().c_str() );

			// 
			m_bBeenInFormation = true;

			// 
			m_fSlotTolerance = 0.8f + grandf( 0.8f );
										
		OnUpdate
			
			// 
			m_pMov->SetMovingWhileFacingTgt(false);

			if (m_pobAIComp->GetFormFlagAttacking() )
			{
				m_pMov->CancelSingleAnim();
				SetState(STATE_FORMATION_ATTACKING);
				return true;
			}
			else if( !m_pSlot->IsInPosition() )
			{
				// AI is not in position
				// Stop Ani animation
				m_pMov->CancelSingleAnim();
				SetState(STATE_FORMATION_MOVE_COMMAND);

				return true;
			}
			else
			{
				// Check that while Shuffling it didn't go far away...
				if( m_pobEnt->GetPosition().Compare( m_pSlot->GetWorldPoint(), m_fSlotTolerance * m_fSlotTolerance ) )
				{
					// If locked on to a target, then make sure the target is followed. 
					m_pobAIComp->SetActionFacing( m_pFormation->GetLockonTarget() ^ m_pobEnt->GetPosition() );
				}
				else
				{
					// It went far away
					m_pSlot->SetInPosition(false);
				}
			}

			// Wait here until AI needs to move
			if (m_pMov->IsSimpleActionComplete())
			{
				if (!IsSaveToPushAController())
				{
					SetReturnToState(STATE_FORMATION_POINT_REACHED);
					SetState(STATE_WAIT_FOR_CS_STANDARD);
					return true;
				}
				
				m_pobAIComp->GetCAIMovement()->PlayAnimation(ACTION_INFORMATION_ANIM);
			}
						
		OnExit

	// =========== STATE_FORMATION_SET_POINT ===============
	AIState(STATE_FORMATION_MOVE_COMMAND)
		OnEnter
			PRINT_STATE(Formation_Dario::STATE_FORMATION_MOVE_COMMAND)

			
			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_FORMATION_MOVE_COMMAND);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}

			// If the AI is attacking, don't send any extra command.
			if (m_pobAIComp->GetFormFlagAttacking() )
			{	
				SetState(STATE_FORMATION_ATTACKING);
				return true;
			}
			else
			{
				// Check the distance the AI is from the formation to determine whether it should
				// join/leave the circle...

				if( !m_pMov->GetEntityToAttack() )
				{
					AI_BEHAVIOUR_SEND_MSG( ATTACK_LOSTTARGET );
					SetState(STATE_INITIALISE);
					return true;
				}

				//////////////////////////////////////////////////////////////////////////
				// Move into formation or position ourselves as appropriate
				//////////////////////////////////////////////////////////////////////////
				if (m_pSlot->IsInRange())
				{
					if( m_pobEnt->GetAIComponent()->GetAIFormationComponent() 
						&& m_pobEnt->GetAIComponent()->GetAIFormationComponent()->IsMovementPaused() )
					{
						SetState(STATE_FORMATION_PLAY_INFO_ANIM);
					}
					// Is the AI already in the destination slot?
					else  if(!m_pSlot->IsInPosition() )
					{
						SetState(STATE_FORMATION_MOVING_TO_POINT);
					}
				}
				else
				{
					// Approach the formation
					m_pobEnt->GetAIComponent()->CompleteSimpleAction();

					//m_pMov->GetMovementParams()->m_fFollowEntityRadiusSQR = ((AIFormation_Circle*)m_pFormation)->GetEntryDstSQR(); // TO BE CHANGED

					// Give the movement command
					CAINavigationSystemMan::Get().ChaseMovingEntity(m_pobEnt);
				}
			}

		OnUpdate

			SetState(STATE_FORMATION_MOVING_TO_POINT);
						
		OnExit

	// =========== STATE_FORMATION_MOVING_TO_POINT ===============

	AIState(STATE_FORMATION_MOVING_TO_POINT)
		OnEnter
			m_pMov->SetMoveToCombatPointCompleted( false );
			m_uiCtrlID = 0;
						
		OnUpdate

			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_FORMATION_MOVING_TO_POINT);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}

			if ( pVis->DoISeeEnemyThroughGoAroundVolumes() )
			{
				AI_BEHAVIOUR_SEND_MSG (ATTACK_ENEMY_BEHIND_GOAROUND_VOLUME);
				return false;
			}

			// Back to Formation Movement Update as soon as the attack is finished
			if ( m_pobAIComp->GetFormFlagAttacking() )
			{
				SetState(STATE_FORMATION_ATTACKING);
			} 
			else if (m_pMov->IsMoveToCombatPointCompleted())
			{
				SetState(STATE_FORMATION_POINT_REACHED);
			}
			else
			{
				if (!IsSaveToPushAController())
				{
					SetReturnToState(STATE_FORMATION_MOVING_TO_POINT);
					SetState(STATE_WAIT_FOR_CS_STANDARD);
					return true;
				}

				bool bAggressive = m_pFormation->IsAgressive();

				m_pobAIComp->GetCAIMovement()->SetActionStyle( bAggressive ? AS_AGGRESSIVE : AS_NORMAL);

				// Set the strafe to formation point. 

				// If in range - then move to the formation point.

				// When within the formation strafe, otherwise move directly to the formation point.
				if (m_pSlot->IsInRange())
				{
					CAINavigationSystemMan::Get().StrafeToFormationPoint(	m_pobEnt,							//CEntity* pEnt
																			m_pMov->GetEntityToAttack(),		//const CEntity* pEntTarget
																			1.0f,								//float fMaxSpeed
																			m_pSlot->GetWorldPoint(),			//const CPoint& obCombatPoint
																			m_uiCtrlID );						//Reference given to the ctrl id
				}
				else
				{
					CAINavigationSystemMan::Get().SteerToDestinationPoint(m_pobEnt, m_pSlot->GetWorldPoint());
				}
			} 
												
		OnExit

	// =========== STATE_FORMATION_ATTACKING ===============

	AIState(STATE_FORMATION_ATTACKING)
		OnEnter
			PRINT_STATE(Formation_Dario::STATE_FORMATION_ATTACKING)

			m_pSlot->SetInPosition(false);

			if ( !m_pobAIComp->GetFormFlagAttacking() )
				SetState(STATE_FORMATION_POINT_REACHED);

			m_fTimer = 0.0f;
						
		OnUpdate

			if( m_pobEnt->GetAttackComponent()->AI_Access_IsInCSStandard() && m_pobAIComp->IsSimpleActionComplete() )
			{
				m_fTimer += fTimeChange;

				static const float IDLE_WAIT_BEFORE_PLAYING_ANIM (0.25f);

				// If the entity has been waiting in their attacking state without attacking for a while, then play 
				// an animation until they're ready to attack
				if( m_fTimer >= IDLE_WAIT_BEFORE_PLAYING_ANIM )
				{
					// Play one of the in formation animations
					m_pobAIComp->SetFormationIdleAnim( m_pFormation->GetIdleAnim().c_str() );
					m_pobAIComp->GetCAIMovement()->PlayAnimation(ACTION_INFORMATION_ANIM);
				}
			}

			// Back to Formation Movement Update as soon as the attack is finished
			if ( !m_pobAIComp->GetFormFlagAttacking() )
			{
				SetState(STATE_FORMATION_POINT_REACHED);
			}
						
		OnExit

	// =========== STATE_FORMATION_ATTACKING ===============

	AIState(STATE_FORMATION_PLAY_INFO_ANIM)
		OnEnter
			PRINT_STATE(Formation_Dario::STATE_FORMATION_PLAY_INFO_ANIM)
			
			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_FORMATION_PLAY_INFO_ANIM);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}

			// Set up the anim.
			m_pobAIComp->SetFormationIdleAnim( m_pFormation->GetIdleAnim().c_str() );
			m_pMov->PlayAnimation(ACTION_INFORMATION_ANIM);

			OnUpdate
			
			// Stay here until the Single Anim is completed
			if (m_pMov->IsSimpleActionComplete())
			{
				SetState(STATE_INITIALISE);
			}
						
		OnExit
			
	// ============= STATE_WAIT_FOR_CS_STANDARD ==========================
		AIState( STATE_WAIT_FOR_CS_STANDARD )
		OnEnter
			PRINT_STATE(Attack_Dario::STATE_WAIT_FOR_CS_STANDARD)
			m_fTimer = 0.0f;
		
		OnUpdate
			
			m_fTimer += fTimeChange;

			// Wait here until AI is in CS_STANDARD
			if (m_fTimer >= (3.0f / 30.0f) && IsSaveToPushAController())
				SetState(GetReturnToState());

		OnExit
		
			m_fTimer = 0.0f;
		
EndStateMachine
}


