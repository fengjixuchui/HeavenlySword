//! -------------------------------------------
//! aibehavior_attacking.cpp
//!
//! ATTACK behaviour for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------
                                                                                         
                                                                                         
//------------------------------------------------------------------------------------------
// Includes                                                                                 
//------------------------------------------------------------------------------------------

#include "ai/aibehaviour_attacking.h"
#include "ai/aiformationcomponent.h"
#include "ai/aiformationmanager.h"
#include "ai/ainavigationsystem/ainavigsystemman.h"
#include "ai/aivision.h"
#include "game/messagehandler.h"
#include "game/aicomponent.h"
#include "game/entitymanager.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "ai/aiattackselection.h"
#include "game/attacks.h"
#include "game/randmanager.h"
#include "core/visualdebugger.h"

#define ALLOW_3M_RADIUS (true)

//------------------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------------------

CAIAttackBehaviour::~CAIAttackBehaviour()
{
	if (m_pMov)
	{
#ifdef _DEBUG
		ntPrintf("AI: [%s] - BEHAVIOUR_ATTACKING, DESTRUCTOR CALLED\n",ntStr::GetString(m_pMov->GetParent()->GetName()));
#endif
		m_pMov->SetMovingWhileFacingTgt(false);
	}
	else
	{
#ifdef _DEBUG
		ntPrintf("AI: [???] - BEHAVIOUR_ATTACKING, DESTRUCTOR CALLED\n",ntStr::GetString(m_pMov->GetParent()->GetName()));
#endif
	}
}

//------------------------------------------------------------------------------------------
//!  private virtual  States
//!
//!  @param [in]       eEvent const STATE_MACHINE_EVENT 
//!  @param [in]       iState const int
//!  @param [in]       fTimeChange const float
//!
//!  @return bool 
//!
//!  @author GavB @date 25/07/2006
//------------------------------------------------------------------------------------------
bool CAIAttackBehaviour::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
{
	const float	fWalkTowardsTime	(10.0f); /// !!! - (Dario) To be parameterised 
	const float fMaxChaseTime		(20.0f);

	// Am I holding a ranged weapon?
	m_bDoIHoldARangedWeapon = ((Character*)m_pobEnt)->GetRangedWeapon() == NULL ? false : true;

	m_pobAIComp->GetCombatComponent().UpdateAttackTarget(fTimeChange);

	const CEntity* pEntityToAttack = m_pobAIComp->GetCAIMovement()->GetEntityToAttack();
	if (!pEntityToAttack) 
	{
		AI_BEHAVIOUR_SEND_MSG( ATTACK_LOSTTARGET );
		return false;
	}

	// Check for a valid enemy
	const Character* pCharTgt = pEntityToAttack->ToCharacter(); 
	if( !pCharTgt )
	{
		AI_BEHAVIOUR_SEND_MSG( ATTACK_LOSTTARGET );
		return false;
	}
	
	// Update combat component
	m_pobAIComp->UpdateCombatComponent(fTimeChange,false);

	// If on Formation Attack, just update combat component and leave.
	if (m_pobAIComp->GetFormFlagAttacking() ) 
	{
		AIFormationComponent* pFormationComp = m_pobAIComp->GetAIFormationComponent();

		// Sanity check
		ntAssert( pFormationComp && "Sanity check for the formation component" );

		if( !pFormationComp->GetNoFormationUpdate() )
		return true;
	}

	// Otherwise AI is in Hand2Hand combat
	static float m_fTimeAfterReachedPoint = 2.0f;

	// Get the selected combat point and AI's position
	CPoint ptTarget			= pCharTgt->GetPosition();
	CPoint ptMyPos			= m_pobAIComp->GetParent()->GetPosition();
	COMBAT_STATE eState		= m_pobAIComp->GetParent()->GetAttackComponent()->AI_Access_GetState();

	// Cache a vision object pointer 
	const CAIVision* pVis = m_pobAIComp->GetAIVision();
	
	// DebugRender Combat Point
#ifndef _GOLD_MASTER
	if (CAINavigationSystemMan::Get().m_bRenderWallAvoidance) 
	{
		CMatrix ArcMatrix(CONSTRUCT_IDENTITY);
		ArcMatrix.SetTranslation(m_pMov->GetAttackPoint() );
		g_VisualDebug->RenderArc(ArcMatrix, m_pMov->GetAttackPointRadiusSQR() , TWO_PI,  DC_YELLOW);
		g_VisualDebug->RenderSphere(CQuat(CONSTRUCT_IDENTITY), m_obMoveTarget,	0.1f, DC_WHITE  );
	}
#endif

	if ( pVis->DoISeeEnemyThroughGoAroundVolumes() )
	{
		AI_BEHAVIOUR_SEND_MSG( ATTACK_LOSTTARGET ); // Probably a new message is needed like ATTACK_ENEMY_BEHIND_GOAROUND_VOLUME
		AI_BEHAVIOUR_SEND_MSG (ATTACK_ENEMY_BEHIND_GOAROUND_VOLUME);
		return false;
	}

	if( !m_pobAIComp->IsInFormFormation() && pVis->IsTargetInAttackRange() && (m_fTestFormationEntryTimer -= fTimeChange) <= 0.0f )
	{
		// Find the best commander entity for the this entity.
		const CEntity* pCommanderEntity = AIFormationManager::Get().GetCommander( m_pobEnt );

		// If there is a commander entity
		if( pCommanderEntity )
		{
			FormationComponent* pFormationAid = pCommanderEntity->GetFormationComponent();

			if( pFormationAid )
			{
				if( pFormationAid->AddEntity( m_pobEnt, true ) )
				{
					AI_BEHAVIOUR_SEND_MSG( ENTER_FORMATION );
				}
			}
		}

		// try again another time
		m_fTestFormationEntryTimer = 1.0f + grandf(1.0f);
	}


BeginStateMachine

	// ============= STATE_INITIALISE ==========================

	AIState( STATE_INITIALISE )
		OnEnter
			PRINT_STATE(Attack_Dario::STATE_INITIALISE)
			m_fTimer = 0.0f;
			m_bStrafingDueToMinRange = false;
			m_pMov->SetMovingWhileFacingTgt(false);

			// Is the Enemy in Attack range?
			if (pVis->IsTargetInAttackRange())
			{
				SetState(STATE_SELECT_ATACK_POINT);
			}
			
			if( (pCharTgt->GetPosition() - m_pobEnt->GetPosition()).LengthSquared() < (40.0f * 40.0f) )
			{
				SetState(STATE_CHASING_TARGET);
			}

		OnUpdate


			if (pVis->IsTargetInAttackRange())
			{
				SetState(STATE_SELECT_ATACK_POINT);
			}
			else if (pVis->HaveIEverSeenTheEnemy())
			{
				SetState(STATE_CHASING_TARGET);
			}
			else
			{
				SetState( STATE_LOST_ENEMY );
			}

		OnExit
			m_fTimer = 0.0f;

	// ============= STATE_CHASING_TARGET ==========================
	
	AIState( STATE_CHASING_TARGET )
		OnEnter
			PRINT_STATE(Attack_Dario::STATE_CHASING_TARGET)
			m_fTimer = 0.0f;
			
			m_bStrafingDueToMinRange = false;
			m_pMov->SetMovingWhileFacingTgt(false);

			//if ( !pVis->DoISeeNothing() && pCharTgt->IsPlayer() && pCharTgt->ToPlayer()->IsArcher() )
			//{
			//	CDirection Line2Enemy	= CDirection(ptMyPos - ptTarget);
			//	float fDist2EnemySQR	= Line2Enemy.LengthSquared();
			//	if (fDist2EnemySQR < m_pMov->GetRangeMinDistSQR()) //GetResidentEvilRadiusSQR())
			//	{
			//		// Change the speed to walk
			//		m_pMov->SetMaxSpeed(m_pMov->GetWalkSpeed());
			//	}
			//	else
			//	{
			//		m_pMov->SetMaxSpeed( m_pMov->GetChaseSpeed() );
			//	}
			//}
			//else if ( m_pMov->GetSpeedMagnitude() < 0.3f )
			//{
			//	m_pMov->SetMaxSpeed( m_pMov->GetChaseSpeed() );
			//}

			m_pMov->SetMaxSpeed( m_pMov->GetChaseSpeed() );

			m_pMov->SetChaseTargetCompleted( false ); // !!! - Needed?
			
			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_CHASING_TARGET);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}
			//CAINavigationSystemMan::Get().SteerToEntity(m_pobEnt, CAINavigationSystemMan::NF_DEF_CHASE_ENEMY);

			// Run the entity to this point. 
			
			CPoint ptRunToHere(CONSTRUCT_CLEAR);

			// ====================================================
			// It seems that due to a change in design, once an enemy has spotted you they always know where you are..
			// Naturally this is no-good for the crossbowman who HAVE to lose you as their target when in melee mode, or
			// they'll never go back and get their crossbow (after which they can know where you are again through "psychic powers"
			// and come hunt you with a crossbow instead.
			// Due to this, we need this horrible string-based check here on the entity's keywords to check if we're a crossbowman with
			// no ranged weapon.
			//
			// TODO: It would be better if every character had an enum-value for character type that we could query here instead.
			// This way depends on every crossbowman having the keyword "crossbowman" on it, never good to assume.
			//
			// NOTE: Querying m_pobEnt->GetKeywords().Contains("crossbowman") doesn't work, even though when you step through it IS one
			// of the keywords. I guess the way the string is hashed is different than it used to be perhaps? (Two different values for the
			// same string) ???.
			// ====================================================
			if ( m_pMov->DoIGoToLastKnownPlayerPosInAttack() || !ALLOW_3M_RADIUS || (!m_bDoIHoldARangedWeapon && strstr(m_pobEnt->GetKeywords().GetKeywordString(), "crossbowman")) )
			{
				ptRunToHere = pVis->HaveIEverSeenTheEnemy() ? pVis->GetLastKnowEnemyPos() : pCharTgt->GetPosition();
			}
			else
			{
				// ==================================================================
				// Calculate a random suitable point around the enemy - TGS
				// - It is TGS specific because it has very wide spaces.
				// - In more tight environments should be checked first
				// ==================================================================

				//CDirection EnemyHeading = pCharTgt->GetMatrix().GetZAxis();
				//CDirection EnemyLeftSide = pCharTgt->GetMatrix().GetXAxis();
				//CDirection AIHeading = m_pobEnt->GetMatrix().GetZAxis();

				CDirection LineEnemy2AI = CDirection( m_pobEnt->GetPosition() - pCharTgt->GetPosition());
				LineEnemy2AI.Normalise();
		
				//float fDotSide = EnemyLeftSide.Dot(AIHeading);
				//float fRandAngle = fDotSide > 0.f ? -grandf( PI ) : grandf( PI );
				float fRandAngle = grandf(1.0f) > 0.5f ? grandf( HALF_PI ) : -grandf( HALF_PI );
		
				float fSin,fCos;
				CMaths::SinCos( fRandAngle, fSin, fCos );
				CDirection Rotation = CDirection (LineEnemy2AI.X()*fCos - LineEnemy2AI.Z()*fSin, LineEnemy2AI.Y(), LineEnemy2AI.X()*fSin + LineEnemy2AI.Z()*fCos);

				//float fRadius = grandf(5.0f); // Magic Number to be parameterised (dario) -- big for TGS
				
				ptRunToHere = CPoint(2.0f*Rotation + pCharTgt->GetPosition()); // Magic Number to be parameterised (dario) -- big for TGS

				// Check that I have line of sight with the selected attack point
				if (!CAINavigationSystemMan::Get().HasLineOfSight(m_pMov->GetPosition(),ptRunToHere,0.75f))
				{
					ptRunToHere = pVis->HaveIEverSeenTheEnemy() ? pVis->GetLastKnowEnemyPos() : pCharTgt->GetPosition();
					
					// Check that at least I have lin of sight with the traget or his last known position
					if (!CAINavigationSystemMan::Get().HasLineOfSight(m_pMov->GetPosition(),ptRunToHere,0.75f))
					{
						// Well... 
						SetState( STATE_LOST_ENEMY );
						return false;
					}
				}
			}

#ifndef _GOLD_MASTER
			if (CAINavigationSystemMan::Get().m_bRenderViewCones)
			{
				g_VisualDebug->RenderLine(pCharTgt->GetPosition(),ptRunToHere,DC_GREEN);
				g_VisualDebug->RenderPoint(ptRunToHere,8,DC_BLUE);
			}
#endif
			
			// ===========================================================================
			//  TGS - END
			// ===========================================================================


			// Tell the entity to move the to the last known good position of the target. 
			// Include AI avoidance
			
			if ( !CAINavigationSystemMan::Get().SteerToDestinationPoint(m_pobEnt, ptRunToHere, 1.0f, false, NF_ARRIVE_AT_POINT | NF_AI_OBSTACLE | NF_S_OBSTACLE ) )
			{
				//If the steer to dest point fails it is likely that the controllers are busy handling another
				// more important request. This entity is still interested in chasing the target but must wait until the
				// controller can be pushed on. 
				SetReturnToState(STATE_CHASING_TARGET);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}
			
			//m_pMov->ActivateFlag(NF_AI_OBSTACLE);
			
		OnUpdate
			m_fTimer += fTimeChange;

			//g_VisualDebug->RenderLine(pCharTgt->GetPosition(),m_pMov->GetDestinationPos(),DC_GREEN);
			
			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_CHASING_TARGET);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}

			// Diving Check

			bool bExecuted = false;
			ENUM_DIVE_FULL_DIRECTION eDiveAction = CAINavigationSystemMan::Get().GetDivingAction(m_pMov->GetParent(), &bExecuted);

			if (bExecuted && eDiveAction != E_DONT_DIVE)
			{
				// Play a Taking_Cover animation
				m_eDivingAction = eDiveAction;
				SetState(STATE_START_DIVING);					
			}

			// Check if target point is still valid
			CDirection Line = CDirection(m_pMov->GetDestinationPos() - ptTarget);
			float	fDistSQR = Line.LengthSquared();
			if (fDistSQR >  0.9f*m_pMov->GetAttackRangeSQR())
			{
				// Calculate a new target point
				SetUserTimer(0.05f);
				SetReturnToState(STATE_CHASING_TARGET);
				SetState(STATE_WAIT_FOR_TIMER);
			}

			// ===============================================================
			//    Is a shooter with range weapon in mele or shooting range?
			// ===============================================================

			if (pVis->IsTargetInShootingRange() && m_bDoIHoldARangedWeapon == true)
			{
				SetState(STATE_IN_SHOOTING_RANGE);
				return true;
			} 
			else if (pVis->IsTargetInMeleRange())
			{
				SetState(STATE_IN_MELE_RANGE);
				return true;
			}

			// ===============================================================
			//                   Normal chase behaviour
			// ===============================================================

			if (pVis->IsTargetInAttackRange())
			{
				SetState(STATE_SELECT_ATACK_POINT);
				return true;
			}
			
			// ===============================================================
			//    Is a AI in ResidentE Distance of the Archer?
			// ===============================================================
			//if ( !pVis->DoISeeNothing() && pCharTgt->IsPlayer() && pCharTgt->ToPlayer()->IsArcher() )
					//{
			//	CDirection Line2Enemy	= CDirection(ptMyPos - ptTarget);
			//	float fDist2EnemySQR	= Line2Enemy.LengthSquared();
			//	if (fDist2EnemySQR < m_pMov->GetRangeMinDistSQR()) //GetResidentEvilRadiusSQR())
					//	{
			//		// Change the speed to walk
			//		m_pMov->SetMaxSpeed(m_pMov->GetWalkSpeed());

			//		//// =============================================================
			//		//// Switch to strafing if Enenmy is KAI and AI is close to her
			//		//// =============================================================
			//		//if ( m_pMov->IsPointCloserThanMinRange(m_pMov->GetPosition()))
			//		//{
			//		//	if (!m_bStrafingDueToMinRange)
			//		//	{
			//		//		m_bStrafingDueToMinRange = true;
			//		//		CDirection LineAI2Enemy(pEntityToAttack->GetPosition() - m_pMov->GetPosition());
			//		//		m_pMov->SetFacingAction(LineAI2Enemy);
			//		//		m_pMov->SetMovingWhileFacingTgt(true);
			//		//		m_pobAIComp->ActivateController(CAIComponent::MC_STRAFING);
			//		//	//	m_uiWalkCtrlId = m_pobEnt->GetMovement()->GetNewControllerCount();
			//		//	}
			//		//}
			//		//else if ( m_bStrafingDueToMinRange )
			//		//{
			//		//	m_bStrafingDueToMinRange = false;
			//		//	m_pMov->SetMovingWhileFacingTgt(false);
			//		//	m_pobAIComp->ActivateController(CAIComponent::MC_WALKING);
			//		//	//m_uiWalkCtrlId = m_pobEnt->GetMovement()->GetNewControllerCount();
			//		//}

			//		return true;
					//	}
					//}
			if (!pVis->DoISeeNothing() )
			//if (pVis->DoISeeTheEnemy())
			{
				// Keep chasing the enemy
				return true;
			}
			else if ( !m_pMov->DoIGoToLastKnownPlayerPosInAttack())
			{
				SetState( STATE_LOST_ENEMY );
				return true;
			} 
			//if (!pVis->DoISeeNothing())
			//{
			//	// I don't see clearly the enemy, but I see something
			//	SetState(STATE_WALK_TOWARDS_ENEMY);
			//	return true;
			//} 

			// We can't see them at all, and we've reached the last-known position.
			//if (m_pMov->IsChaseTargetCompleted() )
			if (m_pMov->IsMoveToSelectedPointCompleted() )
			{
				// Somehow, the player managed to desapear!! Well, then...
				SetState( STATE_LOST_ENEMY );
				return true;
			}

			// Otherwise it keeps moving towards the last known position of the player

			if ( m_fTimer > fMaxChaseTime)
			{
				SetState(STATE_LOST_ENEMY);
			}

		OnExit
			m_fTimer = 0.0f;
			m_bStrafingDueToMinRange = false;
			m_pMov->SetMovingWhileFacingTgt(false);

		// ============= STATE_START_DIVING ==========================
		AIState( STATE_START_DIVING )
		OnEnter
			PRINT_STATE(Attack_Dario::STATE_START_DIVING)
			m_fTimer = 0.0f;

			// Clear previous animations
			m_pobAIComp->CompleteSimpleAction();
			m_pobAIComp->SetPlayingFacingAction(false);

			// Clear movement
			m_pMov->DeactivateMotion();

			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_START_DIVING);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}

			bool bOK = m_pMov->PlayDiveAnimation(m_eDivingAction);
			if (!bOK)
				return false;
			
		OnUpdate

			if( m_pobAIComp->IsSimpleActionComplete() )
			{
				// A cycle is finished
				SetState(STATE_INITIALISE);
			}

		OnExit
		
			m_fTimer = 0.0f;

	// ============= STATE_WALK_TOWARDS_ENEMY ===================

	AIState( STATE_WALK_TOWARDS_ENEMY )
		OnEnter
			PRINT_STATE(Attack_Dario::STATE_WALK_TOWARDS_ENEMY)
			m_fTimer = 0.0f;

			// Change the speed to walk
			m_pMov->SetMaxSpeed(m_pMov->GetApproachSpeed());

		OnUpdate
			m_fTimer += fTimeChange;
			
			if (m_fTimer > fWalkTowardsTime)
			{
				// Something has happend... the AI couldn't reach the last known player's pos. in a reasonable time
				// is he stucked? or the player move away long ago..
				SetState( STATE_LOST_ENEMY );
			}

			// ===============================================================
			//    Is a shooter with range weapon in mele or shooting range?
			// ===============================================================

			if (pVis->IsTargetInShootingRange() && m_bDoIHoldARangedWeapon == true)
			{
				SetState(STATE_IN_SHOOTING_RANGE);
				return true;
			} 
			else if (pVis->IsTargetInMeleRange())
			{
				SetState(STATE_IN_MELE_RANGE);
				return true;
			}

			// ===============================================================
			//                   Normal chase behaviour
			// ===============================================================

			if (pVis->DoISeeTheEnemy())
			{
				// Chase the player
				SetState(STATE_CHASING_TARGET);
				return true;
			}
			if (m_pMov->IsChaseTargetCompleted() && pVis->DoISeeNothing())
			{
				// We walk until the last known player's position.
				SetState( STATE_LOST_ENEMY );
			}
						
		OnExit
			m_fTimer = 0.0f;

	// ============= STATE_ATTACKING ==========================

	AIState( STATE_ATTACKING )
		OnEnter
			PRINT_STATE(Attack_Dario::STATE_ATTACKING)
			m_fTimer = 0.0f;
			
			// Reduce the AI avoidance radius
			m_pMov->ActivateCombatAIAvoidanceRadius(true);
			
		OnUpdate
			m_fTimer += fTimeChange;

			// Is my Enemy Dead? - Needed ?
			if (pCharTgt->IsDead()) 
			{	
				AI_BEHAVIOUR_SEND_MSG( ENEMY_DEAD );
				SetState(STATE_IDLE);
				return true;
			} 
			// Is the  combat system in standard mode?
			if( eState == CS_STANDARD ) 
			{
				// Then leave
				SetState(STATE_ATTACK_POINT_POINTREACHED);
				return true;
			}
			
		OnExit
			m_fTimer = 0.0f;

			// Restore the normal AI avoidance radius
			m_pMov->ActivateCombatAIAvoidanceRadius(false);
			
	// ============= STATE_IDLE ==========================
	
	AIState( STATE_IDLE )
		OnEnter
			PRINT_STATE(Attack_Dario::STATE_IDLE)
			m_fTimer = 0.0f;

			m_pMov->SetMovingWhileFacingTgt(false);
			m_pobAIComp->CompleteSimpleAction();
			m_pobAIComp->ActivateController(CAIComponent::MC_WALKING);

		OnUpdate

			// ===============================================================
			//    Is a shooter with range weapon in mele or shooting range?
			// ===============================================================

			if (pVis->IsTargetInShootingRange())
			{
				SetState(STATE_IN_SHOOTING_RANGE);
				return true;
			} 
			else if (pVis->IsTargetInMeleRange())
			{
				SetState(STATE_IN_MELE_RANGE);
				return true;
			}
			// Is my Enemy Alive?
			else if (pCharTgt->IsDead()) 
			{	
				AI_BEHAVIOUR_SEND_MSG( ENEMY_DEAD );
			}
			else
			{
				// 
				//AI_BEHAVIOUR_SEND_MSG( ATTACK_LOSTTARGET );
			}



			// ===============================================================
			//                   Normal chase behaviour
			// ===============================================================

		OnExit
			m_fTimer = 0.f;

	// ============= STATE_SELECT_ATACK_POINT ==========================
	
	AIState( STATE_SELECT_ATACK_POINT )
		OnEnter
			PRINT_STATE(Attack_Dario::STATE_SELECT_ATACK_POINT)
			m_fTimer = 0.0f;
			
		OnUpdate

			// Is my Enemy Alive?
			if (pCharTgt->IsDead()) 
			{	
				AI_BEHAVIOUR_SEND_MSG( ENEMY_DEAD );
				SetState(STATE_IDLE);
			}

			// Is it attaking?
			//if( eState != CS_STANDARD )
			//{
			//	// It is not in standard state, so wait.
			//	SetState(STATE_ATTACKING);
			//	return true;
			//}
			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_ATTACK_POINT_POINTREACHED);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}

			if (!pVis->IsTargetInAttackRange() || pVis->DoISeeNothing())
			{
				SetState(STATE_CHASING_TARGET);
				return true;
			}

			// Ask for a movement radius
			m_fRadius = m_pobAIComp->GetCombatComponent().RequestMovementRadius();

			// Don't allow the radius to be too small
			if (m_fRadius < 1.5f)
			{
				SetState( STATE_SELECT_ATACK_POINT );
			}

			// Determine where to move to and If the chosen point isn't valid after 10 attempts, then bail.
			for( int iTry = 10; iTry && m_fRadius > 1.0f; --iTry )
			{
				// Calc the movement angle.
				float fAngle		 = grandf(TWO_PI);

				// Save the enemy's position
				m_obEnemyInitialPos = ptTarget;

				// Generate a point relative to the target. 
				m_obMoveTarget = CPoint( ptTarget.X() + (m_fRadius * fcosf(fAngle)), 
										 ptMyPos.Y(), 
										 ptTarget.Z() + (m_fRadius * fsinf(fAngle))); 
				

				// Is the movement target valid? 
				//if (pVis->IsPosValidForFormation(m_obMoveTarget) )
				bool bCommandExecuted = false;
				bool bOk = CAINavigationSystemMan::Get().IsPosValidForFormation_OPF(m_pobEnt->GetPosition(),m_obMoveTarget,&bCommandExecuted,0.75f);
				if (bCommandExecuted)
				{
					if (bOk)
				{
					// If the position is too close to the target calculate another point
					if( (m_obMoveTarget - ptMyPos ).LengthSquared() < 1.0f )
					{
						continue;
					}

					// Move the state on to moving.
					SetState(STATE_MOVING_TO_ATTACK_POINT);
					return true;
					
				}
				else
				{
					// Target Pos. not valid, try another one
					m_fRadius -= m_fRadius / 1.5f;
				}
			}
				else
				{
					// Try again in the next frame
					return true;
				}
			}

			// If here, there isn't any valid point
			//if (!pVis->DoISeeTheEnemy())
			//{
			//	SetState(STATE_LOST_ENEMY);
			//	return true;
			//}

		OnExit
			m_fTimer = 0.f;

	// ============= STATE_MOVING_TO_ATTACK_POINT ==========================
	
	AIState( STATE_MOVING_TO_ATTACK_POINT )
		OnEnter
			PRINT_STATE(Attack_Dario::STATE_MOVING_TO_ATTACK_POINT)
			m_fTimer = 0.f;
			
			// Set any running SimpleAnimation as completed
			m_pobAIComp->CompleteSimpleAction();

			// Is already attaking?
			//if( eState != CS_STANDARD ) 
			//{
			//	// It is not in standard state, so wait.
			//	SetState(STATE_ATTACKING);
			//	return true;
			//}

			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_ATTACK_POINT_POINTREACHED);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}

			// Give the command to strafe to the combat point, facing the enemy
			bool bControllerActive = 
			CAINavigationSystemMan::Get().StrafeToCombatPoint(	m_pobEnt, 
																m_pobAIComp->GetCAIMovement()->GetEntityToAttack(), 
																m_pobAIComp->GetCombatComponent().RequestMovementSpeed(),
																m_obMoveTarget );

			if( !bControllerActive )
			{
				SetState(STATE_ATTACK_POINT_POINTREACHED);
				return true;
			}

		OnUpdate
				
			// Is my Enemy Alive?
			if (pCharTgt->IsDead()) 
			{	
				AI_BEHAVIOUR_SEND_MSG( ENEMY_DEAD );
				SetState(STATE_IDLE);
				return true;
			} 

			// Is already attaking?
			//if( eState != CS_STANDARD ) 
			//{
			//	// It is not in standard state, so wait.
			//	SetState(STATE_ATTACKING);
			//	return true;
			//}
			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_ATTACK_POINT_POINTREACHED);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}

			// ===============================================================
			//    Is a shooter with range weapon in mele or shooting range?
			// ===============================================================

			if (pVis->IsTargetInShootingRange())
			{
				SetState(STATE_IN_SHOOTING_RANGE);
				return true;
			} 
			else if (pVis->IsTargetInMeleRange())
			{
				SetState(STATE_IN_MELE_RANGE);
				return true;
			}

			// ===============================================================
			//                   Normal chase behaviour
			// ===============================================================


			// Are we far from the target for attempting an attack?
			if ( !m_pobAIComp->GetAIVision()->IsTargetInAttackRange() )
			{
				AI_BEHAVIOUR_SEND_MSG( ATTACK_TOFARFROMTARGET );
				SetState(STATE_CHASING_TARGET);
			} 
			// Have the AI arrived to the attack point?
			else if ( m_pMov->IsMoveToCombatPointCompleted() )
			{
				SetState(STATE_ATTACK_POINT_POINTREACHED);
				m_pMov->DeactivateFlag(NF_ARRIVE_AT_POINT);
				return true;
			}

		OnExit
			m_fTimer = 0.f;
			// Disable Facing the enemy while moving
			m_pMov->SetMovingWhileFacingTgt(false);

	// ============= STATE_ATTACK_POINT_POINTREACHED ==========================
	
	AIState( STATE_ATTACK_POINT_POINTREACHED )
		OnEnter
			PRINT_STATE(Attack_Dario::STATE_ATTACK_POINT_POINTREACHED)
			m_fTimer = 0.0f;

			// Is already attaking?
			//if( eState != CS_STANDARD ) 
			//{
			//	// It is not in standard state, so wait.
			//	SetState(STATE_ATTACKING);
			//	return true;
			//}

			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_ATTACK_POINT_POINTREACHED);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}

			// Play an anim. (it will never report as finished...)
			m_pobAIComp->SetActionStyle( AS_AGGRESSIVE );
			m_pobAIComp->SetFormationIdleAnim( "formationclose_inc_strafe" );
			m_pobAIComp->SetPlayingFacingAction( true );
			m_pMov->PlayAnimation(ACTION_INFORMATION_ANIM);
			m_fTimeAfterReachedPoint = m_pobAIComp->GetCombatComponent().RequestMovementPause();
			//ntPrintf("AI: [%s] - On Enter : Tick : %d\n",ntStr::GetString(m_pMov->GetParent()->GetName()),CTimer::Get().GetSystemTicks());

		OnUpdate

			// Make sure the entity is facing the target
			m_pMov->SetFacingAction(CDirection(ptTarget - ptMyPos));

			// Is already attaking?
			//if( eState != CS_STANDARD ) 
			//{
			//	// It is not in standard state, so wait.
			//	SetState(STATE_ATTACKING);
			//	return true;
			//}
			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_ATTACK_POINT_POINTREACHED);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}
			
			// Are we far from the target for attempting an attack?
			if ( !m_pobAIComp->GetAIVision()->IsTargetInAttackRange() )
			{
				m_pobAIComp->CompleteSimpleAction();
				AI_BEHAVIOUR_SEND_MSG( ATTACK_TOFARFROMTARGET );
			//	m_pobAIComp->ActivateController(CAIComponent::MC_WALKING); // Redundant... but useful ?
				SetState(STATE_CHASING_TARGET);
				return true;
			} 
			
			// Is it time to plan another attack? Or... has the enemy moved enough from the last attack?
			if ( (m_fTimer > m_fTimeAfterReachedPoint) || ( (ptTarget - m_obEnemyInitialPos).Length() > m_pobAIComp->GetCombatComponent().RequestMovementTargetThreshold() ) )
			{
				m_pobAIComp->CompleteSimpleAction();
				SetState(STATE_SELECT_ATACK_POINT);
				return true;
			}

			m_fTimer += fTimeChange;

		OnExit
			m_fTimer = 0.f;
			m_pobAIComp->CompleteSimpleAction();
			m_pobAIComp->SetPlayingFacingAction( false );

		// ============= STATE_LOST_ENEMY ===================

	AIState( STATE_LOST_ENEMY )
		OnEnter
			PRINT_STATE(Attack_Dario::STATE_LOST_ENEMY)
			m_fTimer = 0.0f;
			
			// Set any running SimpleAnimation as completed
			m_pobAIComp->CompleteSimpleAction();
			
			// Play Animation
			//m_pobAIComp->ActivateSingleAnim("lookaround_01");

			// 
			AI_BEHAVIOUR_SEND_MSG( ATTACK_LOSTTARGET );

		OnUpdate
			m_fTimer += fTimeChange;
			
			// If the look around animation it complete try chasing the target, if there
			// isn't a target to chase - then the state machine will just end up in this
			// state again. 
			if( m_pobAIComp->IsSimpleActionComplete() )
			{
					SetState(STATE_CHASING_TARGET);
					return true;
			}

			// Watchdog timer
			if( m_fTimer > 5.0f )
			{
				SetState(STATE_CHASING_TARGET);
			}
			
			
		OnExit
			m_fTimer = 0.0f;

		// ============= STATE_IN_SHOOTING_RANGE ===================

	AIState( STATE_IN_SHOOTING_RANGE )
		OnEnter
			PRINT_STATE(Attack_Dario::STATE_IN_SHOOTING_RANGE)
			m_fTimer = 0.0f;

			// Set any running SimpleAnimation as completed
			m_pobAIComp->CompleteSimpleAction();
		
			// Stop his movement
			m_pMov->SetChaseTargetCompleted(true);
			m_pMov->SetMaxSpeed(0.0f);
			m_pMov->SetMovingWhileFacingTgt( true );

		OnUpdate
			
			// Report
			AI_BEHAVIOUR_SEND_MSG( ATTACK_IN_SHOOTING_RANGE );

			// ===============================================================
			//    Is a shooter with range weapon in mele or shooting range?
			// ===============================================================

			if (!pVis->IsTargetInShootingRange())
			{
				SetState(STATE_CHASING_TARGET);
				return true;
			} 

			// ===============================================================
			//                   Normal chase behaviour
			// ===============================================================

		OnExit
			m_pMov->SetMovingWhileFacingTgt( false );
			m_fTimer = 0.0f;

		// ============= STATE_IN_MELE_RANGE ===================

		AIState( STATE_IN_MELE_RANGE )
		OnEnter
			PRINT_STATE(Attack_Dario::STATE_IN_MELE_RANGE)
			m_fTimer = 0.0f;

			// Set any running SimpleAnimation as completed
			m_pobAIComp->CompleteSimpleAction();
		
			// Report

			AI_BEHAVIOUR_SEND_MSG( ATTACK_IN_MELE_RANGE );

			m_pMov->SetChaseTargetCompleted(true);
			m_pMov->SetMaxSpeed(0.0f);

		OnUpdate

			if ( m_pobAIComp->Lua_GetControllerModifier() == AMCM_NONE )
			{
				// Chase/Attack again, as soon as the range weapon is droped
				SetState(STATE_CHASING_TARGET);
			}
			
						
		OnExit
			m_fTimer = 0.0f;

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

		// ============= STATE_WAIT_FOR_TIMER ==========================
		AIState( STATE_WAIT_FOR_TIMER )
		OnEnter
			PRINT_STATE(Attack_Dario::STATE_WAIT_FOR_TIMER)
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

