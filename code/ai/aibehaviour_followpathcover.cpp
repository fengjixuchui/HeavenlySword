//! -------------------------------------------
//! aibehaviour_followpathcover.cpp
//!
//! FOLLOW PATH AND COVER behaviour for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#include "ai/aibehaviour_followpathcover.h"
#include "ai/ainavigationsystem/ainavigsystemman.h"
#include "ai/aiuseobjectqueueman.h"
#include "game/randmanager.h"
#include "game/movement.h"
#include "game/messagehandler.h"
#include "game/interactioncomponent.h"
#include "game/interactiontransitions.h"
#include "game/targetedtransition.h"
#include "anim/animator.h"

#define GOING_TO_NODE ( m_pMov->GetIntention()->eNavIntentions == NAVINT_FOLLOWPATHCOVER_TO_NODE )
#define INVESTIGATING ( false )//m_pMov->GetIntention()->eNavIntentions == NAVINT_FOLLOWPATHCOVER_INVESTIGATE )

//--------------------------------------------------
//!	CAIFollowPathCoverBehaviour::Destructor
//--------------------------------------------------
CAIFollowPathCoverBehaviour::~CAIFollowPathCoverBehaviour( void ) 
{
	PRINT_STATE(FollowPathWithCover_Dario::DESTRUCTOR)
	m_pMov->SetMovingWhileFacingTgt(false);
	if (m_pMov->GetCoverPoint()) 
		m_pMov->GetCoverPoint()->SetAvailabe(true);
}

//--------------------------------------------------
//!	SnapAngle
//--------------------------------------------------
bool CAIFollowPathCoverBehaviour::SnapAngle( void )
{
	CAINavigCoverPoint* pCoverPoint = m_pMov->GetCoverPoint();

	if ( !pCoverPoint || !m_pobEnt->GetMovement() )
		return false;
		
	CPoint obCoverPos = CPoint(pCoverPoint->GetPos().X(), m_pobEnt->GetPosition().Y(),pCoverPoint->GetPos().Z());

	CDirection CurrentDir			= m_pobEnt->GetMatrix().GetZAxis();
	CDirection ValidCoverDirection	= pCoverPoint->GetValidDir();
	//ValidCoverDirection.Normalise();

	if (ValidCoverDirection.Compare(CurrentDir,0.1f))
	{
		m_pobEnt->SetPosition(obCoverPos);
		return true;
	}

	// Clear previous animations
	m_pobAIComp->CompleteSimpleAction();
	m_pobAIComp->SetPlayingFacingAction(false);
	
	// Correct Angle
	CorrectiveMovementTransitionDef obDef;
	obDef.SetDebugNames( "", "CorrectiveTransitionDef" );
	obDef.m_obTargetPostion = obCoverPos;
	obDef.m_obTargetRotation = CQuat(CQuat(CurrentDir, ValidCoverDirection)); //CVector(WMDir)); 
	bool ret = m_pobEnt->GetMovement()->BringInNewController( obDef, CMovement::DMM_STANDARD, 0.0f );
	return ret;
}

//--------------------------------------------------
//!	CAIFollowPathAndCoverBehaviour::States
//--------------------------------------------------

bool CAIFollowPathCoverBehaviour::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
{
	const float	fInitTimeMax			(0.0f);
	const float	fFollowPathTime			(120.0f);
	const float fMoveToCoverPointTime	(4.0f);
	const float fCheckEnemyPosTime		(2.0f);
	const float fSquaredTolerance		(9.0f);
	//const float fMaxUseObjectTime		(15.0f);
	const float fMaxWaitForAvailability (20.0f);
	const float fMaxValidDistanceToObject (1.5f);
	const float	fAlmostOne				(0.98f);
	
	// Check if has an enemy
	m_pEnemy = m_pMov->GetEntityToAttack();
	if ( !m_pEnemy && !GOING_TO_NODE && !INVESTIGATING )
	{
		if (!m_bOneTimeWarning)
		{
			m_bOneTimeWarning = true;
			user_warn_msg( ("CAIFollowPathCoverBehaviour: AI [%s] does NOT have an ENEMY and is meant to go to him\nThis is a ONE TIME WARNING",ntStr::GetString(m_pobEnt->GetName())));
			return false;
		}
		m_bOneTimeWarning = true;
		return false;
	}

	if (m_pEnemy && m_pEnemy->ToCharacter()->IsDead())
	{
		AI_BEHAVIOUR_SEND_MSG(ENEMY_DEAD);
		return false;
	}

	if (!m_pobAIComp->GetAIVision()->DoISeeNothing() && IsSaveToPushAController())
	{
		// Here it comes... the enemy!!!
		m_pMov->SetGoingToCover(false);
		m_fTimerEnemyPosCheck = 0.0f;
		if (m_pMov->GetCoverPoint() && (m_pMov->IsMovingToCover() || iState == STATE_BREAKING_COVER ))
		{
			m_pMov->GetCoverPoint()->SetAvailabe(true);
		}
		AI_BEHAVIOUR_SEND_MSG( SEENENEMY );
	}
	else if ( !GOING_TO_NODE && !INVESTIGATING )
	{
		// Update the enemy pos check timer
		m_fTimerEnemyPosCheck += fTimeChange;

		// Do we need to get a new path?
		if ( (m_fTimerEnemyPosCheck > fCheckEnemyPosTime) &&
			(iState == STATE_MOVING || iState == STATE_IDLE || iState == STATE_PATH_FINISHED)
			)
		{
			m_fTimerEnemyPosCheck = 0.0f;
			CPoint obCurrentEnemyPos = m_pEnemy->GetPosition();

			if (m_obLastEnemyPos.Compare(obCurrentEnemyPos, fSquaredTolerance) == false)
			{
				SetState(STATE_INITIALISE);
				return true;
			}
		}
	}

BeginStateMachine
	// ============= STATE_INITIALISE ==========================

	AIState( STATE_ZERO )
		OnEnter
			PRINT_STATE(FollowPathWithCover_Dario::STATE_ZERO)
			m_fTimer = 0.0f;
			m_fInitTime = grandf(fInitTimeMax);
			m_uiInitialMaxSpeed = m_pMov->GetMaxSpeed();
			m_uiInitialFlags = m_pMov->GetSteeringFlags();
			unsigned int uiIntentions = m_pMov->GetIntention()->eNavIntentions;
			if (uiIntentions != NAVINT_FOLLOWPATHCOVER_TO_NODE &&
				uiIntentions != NAVINT_FOLLOWPATHCOVER_TO_ENEMY)
			{
				user_error_p(0,("Bad/None Intentions Set in a LUA Script for FollowPathCover Behaviour (AI: %s.)\n-Use SetIntention(...) with either NAVINT.FOLLOW_PATH_COVER or NAVINT.FOLLOW_PATH_COVER_TO_NODE\n-Check my lovely written Confluence pages. Dario",ntStr::GetString(m_pobEnt->GetName())));
				return false;
			}

		OnUpdate
			m_fTimer += fTimeChange;
			if( m_fTimer > m_fInitTime )
			{
				SetState( STATE_INITIALISE );
			}
			
		OnExit
			m_fTimer = 0.0f;



	// ============= STATE_INITIALISE ==========================

	AIState( STATE_INITIALISE )
		OnEnter
			PRINT_STATE(FollowPathWithCover_Dario::STATE_INITIALISE)
			m_fTimer = 0.0f;
			
			m_bStrafingDueToMinRange = false;
			m_pMov->SetMovingWhileFacingTgt(false);

			m_pMov->SetFollowPathCompleted(false);
			if (m_pMov->GetCoverPoint()) 
				m_pMov->GetCoverPoint()->SetAvailabe(true);

		OnUpdate

			// Store the enemy's position and reset the enemy's check timer
			if (m_pEnemy)
			{
				m_obLastEnemyPos = m_pEnemy->GetPosition();
				m_fTimerEnemyPosCheck = 0.0f;
			}

			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_INITIALISE);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}

			bool bCommandExecuted = false;
			bool bExistPath = false;
			if ( !GOING_TO_NODE )
			{
				// Calculate path to Enemy
				bExistPath = CAINavigationSystemMan::Get().FollowPathTo(	m_pobEnt, 
																			m_pEnemy,
																			&bCommandExecuted,
																			m_uiInitialMaxSpeed,
																			m_uiInitialFlags );
			}
			else
			{
				bExistPath = CAINavigationSystemMan::Get().FollowPath(	m_pobEnt, & bCommandExecuted, m_uiInitialFlags );
				m_pMov->SetMaxSpeed(m_uiInitialMaxSpeed);
				m_pMov->ClearStartNode();
			}

			m_pobAIComp->ActivateController(CAIComponent::MC_WALKING);
	
			if (!bExistPath)
			{
				CAINavigPath* pPath = m_pMov->GetPathContainer();

				if (pPath->GetDifNavigGraphError() == true ||
					!pPath->IsEndNodeValid() && !pPath->IsStartNodeValid() 
					)
				{
					// Path with different start/end naviggraph regions or bad start/end nodes
					AI_BEHAVIOUR_SEND_MSG( PATH_NOT_FOUND );
					SetState(STATE_IDLE);
					return false;
				}
				
				if (!pPath->IsStartNodeValid())
				{
					CAINavigNode* pClosestNode = CAINavigationSystemMan::Get().GetFirstVisibleNode(m_pMov->GetPosition());
					if (!pClosestNode)
					{	// The AI went inside an AI volume
						// Place it at the nearest node (temporary fix)
						CAINavigNode* pClosestNode = CAINavigationSystemMan::Get().GetAbsoluteClosestNode(m_pMov->GetPosition());					
						m_pobEnt->SetPosition(pClosestNode->GetPos());
						SetUserTimer(2.0f);
						SetReturnToState(STATE_INITIALISE);
						SetState(STATE_WAIT_FOR_TIMER);
						return false;
					}
				}

				//if (!pPath->IsEndNodeValid() && !GOING_TO_NODE)
				//{
				//	CAINavigNode* pClosestNode = CAINavigationSystemMan::Get().GetFirstVisibleNode(m_pMov->GetEntityToAttack()->GetPosition());
				//	if (!pClosestNode)
				//	{	// The Enemy went inside an AI volume
				//		// Place it at the nearest node (temporary fix)
				//		CAINavigNode* pClosestNode = CAINavigationSystemMan::Get().GetAbsoluteClosestNode(m_pMov->GetEntityToAttack()->GetPosition());					
				//		m_pMov->GetEntityToAttack()->SetPosition(pClosestNode->GetPos());
				//		SetUserTimer(0.1f);
				//		SetReturnToState(STATE_INITIALISE);
				//		SetState(STATE_WAIT_FOR_TIMER);
				//		return false;
				//	}
				//}

				AI_BEHAVIOUR_SEND_MSG( PATH_NOT_FOUND );
				SetState(STATE_IDLE);
			}
			else if ( bCommandExecuted )
			{
				m_uiWalkCtrlId = m_pobEnt->GetMovement()->GetNewControllerCount();
				SetState(STATE_MOVING);			
			}


		OnExit
			m_fTimer = 0.f;

	// ============= STATE_MOVING ==========================
	
	AIState( STATE_MOVING )
		OnEnter
			PRINT_STATE(FollowPathWithCover_Dario::STATE_MOVING)
			m_fTimer = 0.0f;

		OnUpdate
		//	m_fTimer += fTimeChange;
			
			if (m_pobAIComp->GetDisabled() == true )
			{
				SetReturnToState(STATE_MOVING);
				SetState(STATE_WAIT_UNTIL_RECOVERED);
				return true;
			}
			if (m_pMov->IsUsingObjectInPath())
			{
				// Register Queue user
				bool bOK = CAIQueueManager::Get().RegisterUser(m_pobEnt->ToAI(), m_pMov->GetEntityToUseInPath());

				// Use the Object
				m_pobAIComp->CompleteSimpleAction();
				m_pMov->DeactivateMotion();

				if (g_ShellOptions->m_bQueueOnObjects && bOK)
					SetState(STATE_QUEUING);
				else
					SetState(STATE_WAIT_FOR_OBJECT_AVAILABLE);

				return true;
			}
			if (m_pMov->IsMovingToCover())
			{
				// Move to the cover point set in my new intention
				AI_BEHAVIOUR_SEND_MSG ( MSG_MOVING_TO_COVER_POINT );
//				SetState(STATE_MOVING_TO_COVER_POINT);
				SetState(STATE_SCALED_TAKING_COVER);
				m_pMov->SetGoingToCover(false);
				return true;
			}
			if (m_pMov->IsFollowPathCompleted())
			{
				// End of the path reached
				//AI_BEHAVIOUR_SEND_MSG( DESTINATION_REACHED );
				SetState(STATE_PATH_FINISHED);
				return true;
			}

			//// =============================================================
			//// Switch to strafing if Enenmy is KAI and AI is close to her
			//// =============================================================
			//if (!GOING_TO_NODE)
			//{
			//	if ( m_pMov->IsPointCloserThanMinRange(m_pMov->GetPosition()))
			//	{
			//		if (!m_bStrafingDueToMinRange)
			//		{
			//			m_bStrafingDueToMinRange = true;
			//			CDirection LineAI2Enemy(m_pEnemy->GetPosition() - m_pMov->GetPosition());
			//			m_pMov->SetFacingAction(LineAI2Enemy);
			//			m_pMov->SetMovingWhileFacingTgt(true);
			//			m_pobAIComp->ActivateController(CAIComponent::MC_STRAFING);
			//			m_uiWalkCtrlId = m_pobEnt->GetMovement()->GetNewControllerCount();
			//		}
			//	}
			//	else if ( m_bStrafingDueToMinRange )
			//	{
			//		m_bStrafingDueToMinRange = false;
			//		m_pMov->SetMovingWhileFacingTgt(false);
			//		m_pobAIComp->ActivateController(CAIComponent::MC_WALKING);
			//		m_uiWalkCtrlId = m_pobEnt->GetMovement()->GetNewControllerCount();
			//	}
			//}

			bool bExecuted = false;
			ENUM_DIVE_FULL_DIRECTION eDiveAction = CAINavigationSystemMan::Get().GetDivingAction(m_pMov->GetParent(), &bExecuted);

			if (bExecuted && eDiveAction != E_DONT_DIVE)
			{
				// Play a Taking_Cover animation
				m_eDivingAction = eDiveAction;
				SetState(STATE_START_DIVING);
				//PRINT_STATE(FollowPathWithCover_Dario::STATE_MOVING_PREPARING_TO_DIVE)					
			}

			// =============================================================
			// Watch Dog and Stuck checks
			// =============================================================
			if (m_fTimer > fFollowPathTime )
			{
				// Something bad happened (got stuck?)
				AI_BEHAVIOUR_SEND_MSG( TIME_OUT );
				AI_BEHAVIOUR_SEND_MSG( DESTINATION_UNREACHABLE );
				SetState(STATE_IDLE);				
			}
			// ----------------------
			// -- Stuck prevention --
			// ----------------------
			CAINavigPath* pPath = m_pMov->GetPathContainer();
			CAINavigNode* pCurrentNode = pPath->GetCurrentNode();
			if (pCurrentNode && !CAINavigationSystemMan::Get().HasLineOfSight(m_pMov->GetPosition(),pCurrentNode->GetPos()) )
			{
				PRINT_STATE(FollowPathWithCover_Dario::STATE_MOVING -> LOS FAILED!)
				//return true;
			}

			// If for some reason the entity has lost it's movement controller - then re-apply the needed controller
			if( m_uiWalkCtrlId != m_pobEnt->GetMovement()->GetNewControllerCount() )
			{
				if( m_pobAIComp->ActivateController(CAIComponent::MC_WALKING) )
				{
					m_uiWalkCtrlId = m_pobEnt->GetMovement()->GetNewControllerCount();
				}
			}
			
		OnExit
			m_fTimer = 0;
			m_bStrafingDueToMinRange = false;
			m_pMov->SetMovingWhileFacingTgt(false);

		// ============= STATE_START_DIVING ==========================
		AIState( STATE_START_DIVING )
		OnEnter
			PRINT_STATE(FollowPathWithCover_Dario::STATE_START_DIVING)
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

	// ============= STATE_MOVING_TO_COVER_POINT ==========================

	AIState( STATE_MOVING_TO_COVER_POINT )
		OnEnter
			PRINT_STATE(FollowPathWithCover_Dario::STATE_MOVING_TO_COVER_POINT)
			m_fTimer = 0.0f;
	
			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_MOVING_TO_COVER_POINT);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}

			// Give the order (it disables the follow path movement)
			CAINavigationSystemMan::Get().SteerToDestinationPoint(	m_pobEnt, 
																	m_pMov->GetCoverPoint()->GetPos(),
																	sqrt(m_pMov->GetCoverPoint()->GetRadiusSQR())
																);		

		OnUpdate
			
			m_fTimer += fTimeChange; 

			if (m_pobAIComp->GetDisabled() == true )
			{
				SetReturnToState(STATE_MOVING_TO_COVER_POINT);
				SetState(STATE_WAIT_UNTIL_RECOVERED);
				return true;
			}
			if (m_pMov->IsMoveToSelectedPointCompleted())
			{
				// Disable move to point
				m_pMov->DeactivateFlag(NF_ARRIVE_AT_POINT);
				// flag that the cover point is reached
				m_pMov->SetGoingToCover(false);
				AI_BEHAVIOUR_SEND_MSG( COVER_POINT_REACHED );
				SetState(STATE_SCALED_TAKING_COVER);
				return true;
			} 
			if ( m_pMov->GetCoverPoint() && (m_fTimer > fMoveToCoverPointTime ))
			{
				m_pMov->GetCoverPoint()->SetAvailabe(true);
				SetState(STATE_INITIALISE);

				AI_BEHAVIOUR_SEND_MSG( COVER_POINT_TIME_OUT );
			}
			// ----------------------
			// -- Stuck prevention --
			// ----------------------
			if ( !CAINavigationSystemMan::Get().HasLineOfSight(m_pMov->GetPosition(),m_pMov->GetDestinationPos()) )
			{
				PRINT_STATE(FollowPathWithCover_Dario::STATE_MOVING -> CP LOS FAILED!)
				//return true;
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
			bool bFoundAnim;
			CHashedString hsCoverAnim = m_pMov->GetCoverPoint()->GetAnimation(ANIM_TO_COVER, &bFoundAnim, m_pobEnt );
			if (bFoundAnim)
			{
				m_pobAIComp->ActivateSingleAnim(hsCoverAnim);
			}
			else
			{
				// Do nothing? Well at least warn
				ntAssert_p(bFoundAnim,("STATE_TAKING_COVER : No going to cover animation found"));
			}

		OnUpdate
			//m_fTimer += fTimeChange;

			// As soon as the animation is finished, move on.
			if( m_pobAIComp->IsSimpleActionComplete() )
			{
				SetState( STATE_ADJUSTING_COVER_ANGLE );
			}
			
		OnExit
			m_fTimer = 0.0f;

	// ============= STATE_SCALED_TAKING_COVER ==========================

	AIState( STATE_SCALED_TAKING_COVER )
		OnEnter
			PRINT_STATE(FollowPathWithCover_Dario::STATE_SCALED_TAKING_COVER)
			m_fTimer = 0.0f;

			// Clear previous animations
			m_pobAIComp->CompleteSimpleAction();

			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_SCALED_TAKING_COVER);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}

			// Play a Taking_Cover animation
			bool bFoundAnim;
			CHashedString hsCoverAnim = m_pMov->GetCoverPoint()->GetAnimation(ANIM_TO_COVER, &bFoundAnim, m_pobEnt );
			if (bFoundAnim)
			{
				CDirection Line2Cover(m_pMov->GetPosition() - m_pMov->GetCoverPoint()->GetPos());
				float fDist2Cover = Line2Cover.Length()+0.1f; // 10 centimetres tolerance

				CAnimationPtr pobAnim = m_pobEnt->GetAnimator()->CreateAnimation( hsCoverAnim );
				m_fCoverAnimDuration = pobAnim->GetDuration();

				ScaledTargetedTransitionDef obScaledCoverDefinition;
				obScaledCoverDefinition.SetDebugNames( "Moving To Cover Scaled", "ScaledTargetedTransitionDef" );
				// Set up the parameters
				obScaledCoverDefinition.m_bApplyGravity = true;
				obScaledCoverDefinition.m_fMaxRange = fDist2Cover; 
				obScaledCoverDefinition.m_fMovementOffset = 0;
				obScaledCoverDefinition.m_pobAnimation = pobAnim;
				obScaledCoverDefinition.m_fMovementDuration = m_fCoverAnimDuration;
				obScaledCoverDefinition.m_bTrackAfterScale = true;
				obScaledCoverDefinition.m_b3DScaling = false;
				obScaledCoverDefinition.m_bScaleDown = false;
				obScaledCoverDefinition.m_bNoDirectionCorrectionScaling = false;
				obScaledCoverDefinition.m_fScaleToTime = m_fCoverAnimDuration;

				m_pobEnt->GetMovement()->m_obMovementInput.m_obTargetPoint = m_pMov->GetCoverPoint()->GetPos();
				m_pobEnt->GetMovement()->m_obMovementInput.m_bTargetPointSet = true;

				// Push the controller on to the movement component
				bool bNoBlend = erandf(1.0f) < 0.5f;
				if (!bNoBlend)
					m_pobEnt->GetMovement()->BringInNewController( obScaledCoverDefinition, CMovement::DMM_STANDARD, 0.5f );
				else
				{
					m_pobEnt->GetMovement()->ClearControllers();
					m_pobEnt->GetMovement()->BringInNewController( obScaledCoverDefinition, CMovement::DMM_STANDARD, 0.0f );
				}


				//ZAxisAlignTargetedTransitionDef obScaledCoverDefinition;
				//obScaledCoverDefinition.SetDebugNames("Moving To Cover Scaled","ZAxisAlignTargetedTransitionDef");
				////obScaledCoverDefinition.m_pobEntityAlignZTowards = pobPlayer;
				//obScaledCoverDefinition.m_obAnimationName = hsCoverAnim;
				//obScaledCoverDefinition.m_bApplyGravity = true;
				//obScaledCoverDefinition.m_obScaleToCoverDistance = m_pMov->GetCoverPoint()->GetPos() - m_pMov->GetPosition();
				//obScaledCoverDefinition.m_obAlignZTo = m_pMov->GetCoverPoint()->GetValidDir();

				//m_pobEnt->GetMovement()->BringInNewController( obScaledCoverDefinition, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );
			}
			else
			{
				// Do nothing? Well at least warn
				ntAssert_p(bFoundAnim,("STATE_TAKING_COVER : No going to cover animation found"));
			}

		OnUpdate
			m_fTimer += fTimeChange;

			// As soon as the animation is finished, move on.
			if( m_fTimer < m_fCoverAnimDuration )
			{
				m_pobEnt->GetMovement()->m_obMovementInput.m_obTargetPoint = m_pMov->GetCoverPoint()->GetPos();
				m_pobEnt->GetMovement()->m_obMovementInput.m_bTargetPointSet = true;
			}
			else
			{
				SetState( STATE_ADJUSTING_COVER_ANGLE );
			}
			
		OnExit
			m_fTimer = 0.0f;
		
	// ========= STATE_ADJUSTING_COVER_ANGLE =================
	AIState( STATE_ADJUSTING_COVER_ANGLE )
		OnEnter
			PRINT_STATE(FollowPathWithCover_Dario::STATE_ADJUSTING_COVER_ANGLE);
			m_fTimer = 0.0f;

			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_ADJUSTING_COVER_ANGLE);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}

			m_pobAIComp->CompleteSimpleAction();
	/*		SnapAngle();*/
			m_pobAIComp->SetPlayingFacingAction(true);
			m_pobAIComp->ActivateController(CAIComponent::MC_WALKING);

			// Get the facing direction 
			CAINavigCoverPoint* pCoverPoint = m_pMov->GetCoverPoint();
			m_obValidCoverDirection = pCoverPoint->GetValidDir();
			m_obValidCoverDirection.Normalise();
			
			
		///////	m_pobEnt->SetRotation(CQuat(CVector(m_obValidCoverDirection)));
			
			UNUSED(fAlmostOne);
		OnUpdate
			
			m_fTimer += fTimeChange;

			if ( m_fTimer > grandf(0.3f) + 0.1f )
			{
				//if (IsSaveToPushAController())
				//{
				//	SnapAngle();
				//	SetState(STATE_COVERING);
				//}
				CAINavigCoverPoint* pCoverPoint = m_pMov->GetCoverPoint();
				CPoint obCoverPos = CPoint(pCoverPoint->GetPos().X(), m_pobEnt->GetPosition().Y(),pCoverPoint->GetPos().Z());
				m_pobEnt->SetPosition(obCoverPos);
				//SetState(STATE_COVERING);
			}

			m_pMov->SetFacingAction(m_obValidCoverDirection);
			CDirection currentFacing;
			m_pobEnt->GetLookDirection( currentFacing );
			currentFacing.Normalise();
			float fFacingDotProduct = m_obValidCoverDirection.Dot(currentFacing);
			if ( fFacingDotProduct > fAlmostOne )
			{
				m_pobAIComp->SetPlayingFacingAction(false);
				m_fTimeInCover = m_pMov->GetCoverTime();
				SetState(STATE_COVERING);
			}
			
		OnExit
			m_pobAIComp->SetPlayingFacingAction(false); 
			m_fTimer = 0.0f;

	// ============= STATE_COVERING ==========================
	AIState( STATE_COVERING )
		OnEnter
			PRINT_STATE(FollowPathWithCover_Dario::STATE_COVERING);
			m_fTimer = 0.0f;
			
			// Clear previous animations
			m_pobAIComp->CompleteSimpleAction();

			// Can I see the enemy?
			if (!m_pobAIComp->GetAIVision()->DoISeeNothing())
			{
				SetState(STATE_INITIALISE);
				return true;
			}

			// Play a In-Cover animation
			bool bFoundAnim;
			CHashedString hsCoverAnim = m_pMov->GetCoverPoint()->GetAnimation(ANIM_IN_COVER, &bFoundAnim );
			if (bFoundAnim)
			{
				m_pobAIComp->ActivateSingleAnim(hsCoverAnim);
			}
			else
			{
				// Do nothing? Well at least warn
				ntAssert_p(bFoundAnim,("STATE_COVERING : No in-cover animation found"));
			}          

		OnUpdate
			m_fTimeInCover -= fTimeChange;
			
			// Can I see the enemy?
			if (!m_pobAIComp->GetAIVision()->DoISeeNothing())
			{
				m_pobAIComp->CompleteSimpleAction();
				SetState(STATE_INITIALISE);
				return true;
			}

			if ( m_pobAIComp->IsSimpleActionComplete())
			{
				if (m_fTimeInCover < m_pMov->GetInCoverPeekChance())
				{
					// No time for any other animation
					SetState(STATE_BREAKING_COVER);
					return true;
				}

				SetState(STATE_PLAYING_PEEKING_ANIM);
			}
			
		OnExit
			m_fTimer = 0.0f;

	// ============= STATE_PLAYING_PEEKING_ANIM ==========================
	AIState( STATE_PLAYING_PEEKING_ANIM )
		OnEnter
			PRINT_STATE(FollowPathWithCover_Dario::STATE_PLAYING_PEEKING_ANIM)
			m_fTimer = 0.0f;

			// Clear previous animations
			m_pobAIComp->CompleteSimpleAction();

			// Can I see the enemy?
			if (!m_pobAIComp->GetAIVision()->DoISeeNothing())
			{
				SetState(STATE_INITIALISE);
				return true;
			}

			// Play a Peek animation
			bool bFoundAnim;
			CHashedString hsCoverAnim = m_pMov->GetCoverPoint()->GetAnimation(ANIM_PEEK_COVER, &bFoundAnim );
			if (bFoundAnim)
			{
				m_pobAIComp->ActivateSingleAnim(hsCoverAnim);
			}
			else
			{
				// Do nothing? Well at least warn
				ntAssert_p(bFoundAnim,("STATE_PLAYING_PEEKING_ANIM : No peek animation found"));
			}
			
		OnUpdate
			m_fTimer += fTimeChange;

			// Can I see the enemy?
			if (!m_pobAIComp->GetAIVision()->DoISeeNothing())
			{
				m_pobAIComp->CompleteSimpleAction();
				SetState(STATE_INITIALISE);
				return true;
			}

			if( m_pobAIComp->IsSimpleActionComplete() )
			{
				float fRand = erandf(1.0f);
				if (fRand < 0.5f)
					SetState(STATE_PLAYING_PEEKING_ANIM);
				else
					SetState(STATE_COVERING);
			}
			
		OnExit
			m_fTimer = 0.0f;

	// ============= STATE_BREAKING_COVER ==========================
	AIState( STATE_BREAKING_COVER )
		OnEnter
			PRINT_STATE(FollowPathWithCover_Dario::STATE_BREAKING_COVER)
			m_fTimer = 0.0f;

			// Check that there is space to break cover
			CEntity* pAI = NULL;
			pAI = CAINavigationSystemMan::Get().TestDive_AICollision(m_pMov->GetParent(),E_DIVE_LONG_LEFT);
			if (pAI && !pAI->ToAI()->GetAIComponent()->GetCAIMovement()->IsInFormationMovement())
			{
				m_fTimeInCover = 3.0f; // 3 more sec. of covering
				SetState(STATE_COVERING);
				return true;
			}

			pAI = CAINavigationSystemMan::Get().TestDive_AICollision(m_pMov->GetParent(),E_DIVE_LONG_RIGHT);
			if (pAI && !pAI->ToAI()->GetAIComponent()->GetCAIMovement()->IsInFormationMovement())
			{
				m_fTimeInCover = 3.0f; // 3 more sec. of covering
				SetState(STATE_COVERING);
				return true;
			}
	
			// Clear previous animations
			m_pobAIComp->CompleteSimpleAction();

			// Play a Break-Cover animation
			bool bFoundAnim;
			CHashedString hsCoverAnim = m_pMov->GetCoverPoint()->GetAnimation(ANIM_BREAK_COVER, &bFoundAnim );
			if (bFoundAnim)
			{
				m_pobAIComp->ActivateSingleAnim(hsCoverAnim);
			}
			else
			{
				// Do nothing? Well at least warn
				ntAssert_p(bFoundAnim,("STATE_BREAKING_COVER : No Break-Cover animation found"));
			}

		OnUpdate
			
			// As soon as the animation is finished, move on.
			if( m_pobAIComp->IsSimpleActionComplete() )
			{
				m_pMov->GetCoverPoint()->SetAvailabe(true);
				
				// Do I have LOS with a node?
				CAINavigNode* pClosestNode = CAINavigationSystemMan::Get().GetFirstVisibleNode(m_pMov->GetPosition());
				if (!pClosestNode)
				{
					// The AI went inside an AI volume
					// Place it at the cover point (temporary fix)
					m_pobEnt->SetPosition(m_pMov->GetCoverPoint()->GetPos());
				}

				// Generate a new path to the current enemy's position
				SetState(STATE_INITIALISE);
			}
			
		OnExit
			m_fTimer = 0.0f;

	// ============= STATE_IDLE ==========================
	AIState( STATE_IDLE )
		OnEnter
			PRINT_STATE(FollowPathWithCover_Dario::STATE_IDLE)
			// Clear previous animations
			m_pobAIComp->CompleteSimpleAction();
		//	m_pMov->DeactivateFlag
			// After couple of seconds, try again
			float fIdleTime = erandf(m_pMov->GetMaxWaitingTimeAfterPathNotFound());
			SetUserTimer(fIdleTime);
			SetReturnToState(STATE_INITIALISE);
			SetState(STATE_WAIT_FOR_TIMER);

		OnUpdate
			
		OnExit

		// ============= STATE_PATH_FINISHED ==========================
		AIState( STATE_PATH_FINISHED )
		OnEnter
			PRINT_STATE(FollowPathWithCover_Dario::STATE_PATH_FINISHED)
			m_fTimer = 0.0f;

			// Clear previous animations
			m_pobAIComp->CompleteSimpleAction();

			AI_BEHAVIOUR_SEND_MSG(PATH_FINISHED);
			
		OnUpdate
			
			// AI has walked the path that suposedly led to the enemy
			// ... but he has not encontered the enemy

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

	// ====================================================
	// ============= USE OBJECT  ==========================
	// ====================================================
	
	// ============= STATE_QUEUING ==========================
	AIState( STATE_QUEUING )
		OnEnter
			PRINT_STATE(FollowPathWithCover_Dario::STATE_QUEUING);
			m_fTimer = 0.0f;
			
		OnUpdate
		        	        
		        if (!m_pMov->IsQueuingNeeded())
		        {
		            // SetState(STATE_MOVE_TO_OBJECT_TO_USE);
		            SetState(STATE_MOVING_TO_OBJECT_TO_USE);
		            return true;
		        }
		        
		        if (m_pMov->IsQueueIndexUpdated())
		        {
			        SetState(STATE_MOVING_TO_UPDATED_QUEUE_POS);
			        return true;
		        }
		
        OnExit
			m_fTimer = 0.0f;
			
	// ============= STATE_MOVING_TO_UPDATED_QUEUE_POS ==========================
	AIState( STATE_MOVING_TO_UPDATED_QUEUE_POS )
		OnEnter
			PRINT_STATE(FollowPathWithCover_Dario::STATE_MOVING_TO_UPDATED_QUEUE_POS);
			m_fTimer = 0.0f;

			bool bSuccess = false;
			float fRadius = 0.0f;

			CPoint obNewQueuePos = CAIQueueManager::Get().GetQueuePointAndRadius(m_pobEnt->ToAI(), &fRadius, &bSuccess);
			
			ntAssert(bSuccess);

			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_MOVING_TO_UPDATED_QUEUE_POS);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}

			// Give the order (it disables the follow path movement)
			CAINavigationSystemMan::Get().SteerToDestinationPoint(	m_pobEnt, 
																	obNewQueuePos,
																	fRadius,
																	false,
																	NF_ARRIVE_AT_POINT
																  );
			m_pMov->SetMaxSpeed(0.4f);

		OnUpdate

			if (m_pobAIComp->GetDisabled() == true )
			{
				SetReturnToState(STATE_MOVING_TO_UPDATED_QUEUE_POS);
				SetState(STATE_WAIT_UNTIL_RECOVERED);
				return true;
			}
			if (m_pMov->IsMoveToSelectedPointCompleted())
			{
				// Disable move to point
				m_pMov->DeactivateFlag(NF_ARRIVE_AT_POINT);
				
				// Report to the Queue manager
				//CAIQueueManager::Get().ReportNewQueuePointReached(m_pobEnt->ToAI());
				
				AI_BEHAVIOUR_SEND_MSG( NEW_QUEUE_POINT_REACHED );
				SetState(STATE_QUEUING);
				return true;
			} 
		
        OnExit
			m_fTimer = 0.0f;

	// ============= STATE_MOVING_TO_OBJECT_TO_USE ==========================
	AIState( STATE_MOVING_TO_OBJECT_TO_USE )
		OnEnter
			PRINT_STATE(FollowPathWithCover_Dario::STATE_MOVING_TO_OBJECT_TO_USE);
			m_fTimer = 0.0f;

			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_MOVING_TO_OBJECT_TO_USE);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}

			// Give the order (it disables the follow path movement)
			m_pMov->SetEntityToGoTo(m_pMov->GetEntityToUseInPath());
			m_pMov->SetDestinationRadius(fMaxValidDistanceToObject);
			m_pMov->SetMoveToSelectedPointCompleted(false);
			CAINavigationSystemMan::Get().SteerToEntity(m_pobEnt,CAINavigationSystemMan::NF_DEF_STEER_TO_ENTITY);
			

		OnUpdate

			if (m_pobAIComp->GetDisabled() == true )
			{
				SetReturnToState(STATE_MOVING_TO_UPDATED_QUEUE_POS);
				SetState(STATE_WAIT_UNTIL_RECOVERED);
				return true;
			}

			if (m_pMov->IsMoveToSelectedPointCompleted())
			{
				// Disable move to point
				m_pMov->DeactivateFlag(NF_ARRIVE_AT_POINT);
				
				// Report to the Queue manager
				//CAIQueueManager::Get().ReportNewQueuePointReached(m_pobEnt->ToAI());
				
				AI_BEHAVIOUR_SEND_MSG( OBJECT_TO_USE_REACHED );
				SetState(STATE_WAIT_FOR_OBJECT_AVAILABLE);
				return true;
			} 
		
        OnExit
			m_fTimer = 0.0f;

	// ============= STATE_WAIT_FOR_OBJECT_AVAILABLE ==========================
	AIState( STATE_WAIT_FOR_OBJECT_AVAILABLE )
		OnEnter
			PRINT_STATE(FollowPathWithCover_Dario::STATE_WAIT_FOR_OBJECT_AVAILABLE);
			m_fTimer = 0.0f;

		OnUpdate
			m_fTimer += fTimeChange;
			
			// Is the object available ?
			if (m_pMov->GetEntityToUseInPath()->GetInteractionComponent()->GetInteractionPriority() == USE)
			{
				if (!IsSaveToPushAController())
				{
					SetReturnToState(STATE_WAIT_FOR_OBJECT_AVAILABLE);
					SetState(STATE_WAIT_FOR_CS_STANDARD);
					return true;
				}
				
				// Send Request to use object
				Message obMessage(msg_interact);
				obMessage.SetEnt("target",m_pMov->GetEntityToUseInPath());
				m_pobEnt->GetMessageHandler()->Receive(obMessage);
	
				// Wait some frames to allow the interaction message to get there
				SetUserTimer(0.1f);
				SetReturnToState(STATE_USING_OBJECT);
				SetState(STATE_WAIT_FOR_TIMER);
				PRINT_STATE(FollowPathWithCover_Dario::STATE_USING_OBJECT - OK);
				return true;
			}

			if(m_fTimer > fMaxWaitForAvailability)
			{
				AI_BEHAVIOUR_SEND_MSG( OBJECT_WAIT_AVAILABLE_TIMEOUT );
				PRINT_STATE(FollowPathWithCover_Dario::STATE_WAIT_FOR_OBJECT_AVAILABLE - OBJECT_WAIT_AVAILABLE_TIMEOUT);
				SetState( STATE_INITIALISE );
			}

		OnExit
			m_fTimer = 0.0f;

	// ============= STATE_USING_OBJECT ==========================
	AIState( STATE_USING_OBJECT )
		OnEnter
			PRINT_STATE(FollowPathWithCover_Dario::STATE_USING_OBJECT);
			m_fTimer = 0.0f;
			
			AI_BEHAVIOUR_SEND_MSG( USING_OBJECT );

		OnUpdate
			m_fTimer += fTimeChange;
			
			// Check if the AI has finished using the object
			CEntity* pIntTarget = ((Character*)m_pobEnt)->GetInteractionTarget();
			CEntity* pRangedWeapon = ((Character*)m_pobEnt)->GetRangedWeapon();
			if ( pIntTarget == NULL || pIntTarget == pRangedWeapon )
			{
				m_pMov->SetEntityToUseInPath(0);
				m_pMov->SetUsingObjectInPath(false);
				CAIQueueManager::Get().ReportObjectUsed(m_pobEnt->ToAI());
				AI_BEHAVIOUR_SEND_MSG( OBJECT_USED );
				SetState( STATE_INITIALISE );
				return true;
			}
			
			//if(m_fTimer > fMaxUseObjectTime)
			//{
			//	AI_BEHAVIOUR_SEND_MSG( OBJECT_USING_TIMEOUT );
			//	PRINT_STATE(FollowPathWithCover_Dario::STATE_USING_OBJECT - OBJECT_USING_TIME_OUT);
			//	SetState( STATE_INITIALISE );
			//}

		OnExit
			m_fTimer = 0.0f;
		

EndStateMachine
}


