//! -------------------------------------------
//! aibehaviour_followpathcover.cpp
//!
//! PATROL behaviour for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#include "ai/aibehaviour_rangedynamiccover.h"
#include "ai/ainavigationsystem/ainavigsystemman.h"
#include "game/randmanager.h"
#include "game/messagehandler.h"
#include "game/movement.h"
#include "anim/animator.h"
#include "game/targetedtransition.h"

//--------------------------------------------------
//!	CAIRangeDynamicCoverBehaviour::Destructor
//--------------------------------------------------
CAIRangeDynamicCoverBehaviour::~CAIRangeDynamicCoverBehaviour( void ) 
{
	PRINT_STATE(RangeDynCover_Dario::DESTRUCTOR)
	if (m_pMov->GetCoverPoint()) m_pMov->GetCoverPoint()->SetAvailabe(true);
	m_pMov->SetCoverPoint(NULL);
	m_pMov->SetClaimCoverPoint(NULL);
	m_pobAIComp->SetPlayingFacingAction(false);
	CAINavigationSystemMan::Get().MovingVolley_RemoveAI(m_pobEnt);
}

//--------------------------------------------------
//!	CAIRangeDynamicCoverBehaviour::FaceEnemy
//--------------------------------------------------
bool CAIRangeDynamicCoverBehaviour::FaceEnemy( void ) 
{
	CPoint obEnemyPosition = m_pMov->GetEntityToAttack()->GetPosition();
	CDirection obDesiredFacing = CDirection(obEnemyPosition - m_pobEnt->GetPosition());
	obDesiredFacing.Normalise();
	m_pMov->SetFacingAction(obDesiredFacing);
	CDirection currentFacing;
		
	m_pobEnt->GetLookDirection( currentFacing );
	currentFacing.Normalise();
		
	float fFacingDotProduct = obDesiredFacing.Dot(currentFacing);
	if ( fFacingDotProduct > 0.985f )
	{
		//m_pobAIComp->SetPlayingFacingAction(false);
		AI_BEHAVIOUR_SEND_MSG( FACING_ENTITY );
		return true;
	}
	return false;
}

//--------------------------------------------------
//!	CAIRangeDynamicCoverBehaviour::States
//--------------------------------------------------

bool CAIRangeDynamicCoverBehaviour::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
{
	//const float	fInitTimeMax			(0.5f);
	const float	fFollowPathTime			(120.0f);
	const float fMoveToCoverPointTime	(4.0f);
	const float fCheckEnemyPosTime		(0.5f);
	const float fMaxIdleTime			(5.0f);
	const float	fAlmostOne				(0.98f);
//	const float fShootingTimeThreshold	(2.0f);
	
	// Cache Vision Module
    m_pVis = m_pobAIComp->GetAIVision();
	ntAssert(m_pVis);
    
	if (!m_pMov->GetEntityToAttack())
	{
		user_error_p(0,("AI: [%s] is in Range Dynamic Cover behaviour without a TARGET",ntStr::GetString(m_pobEnt->GetName())));
		return false;
	}

	// Cache Volley Commander
	m_pCommander = CAINavigationSystemMan::Get().MovingVolley_GetCommander();

	// Cache AI and Enemy's Position
	CPoint obEnemyPos	= m_pMov->GetEntityToAttack()->GetPosition();
	CPoint obAIPos		= m_pMov->GetPosition();

	// Cache Line, Distance to Enemy, and relative location in the MIN-MAX area
	CDirection LineAI2Enemy(obEnemyPos - obAIPos);
	float fAI2EnemyDistSQR	= LineAI2Enemy.LengthSquared();
	m_bCloserThanMinRange	= (fAI2EnemyDistSQR < m_pMov->GetRangeMinDistSQR());
	m_bFarFromMaxRange		= (fAI2EnemyDistSQR > m_pMov->GetRangeMaxDistSQR());

	// Update Shooting Timer
	fTimeBetweenShootsMoving+=fTimeChange;
	
	// Always check if we are in MELE RANGE (i.e. we have to fight!)

    if (m_pVis->IsTargetInMeleRange())
	{
		// Here it comes... the enemy!!!
		m_pMov->SetGoingToCover(false);
		m_fTimerEnemyPosCheck = 0.0f;
		if (m_pMov->IsMovingToCover() || iState == STATE_BREAKING_COVER )
		{
			m_pMov->GetCoverPoint()->SetAvailabe(true);
		}
		AI_BEHAVIOUR_SEND_MSG( ATTACK_IN_MELE_RANGE );
	}
	//else if (m_pVis->IsTargetInShootingRange())
	//{
	//	// Report
	//	if (fTimeBetweenShootsMoving>m_pMov->GetTimeBetweenShoots())
	//	{
	//		fTimeBetweenShootsMoving=0.0f;
	//		AI_BEHAVIOUR_SEND_MSG( ATTACK_IN_SHOOTING_RANGE );
	//	}
	//}
	else
	{
		// Update the enemy pos check timer
		m_fTimerEnemyPosCheck += fTimeChange;
	}

BeginStateMachine
	// ============= STATE_INITIALISE ==========================

	AIState( STATE_ZERO )
		OnEnter
			PRINT_STATE(RangeDynCover_Dario::STATE_ZERO)
			m_fTimer = 0.0f;
			
			// Add myself to the VolleyList
			CAINavigationSystemMan::Get().MovingVolley_AddAI(m_pobEnt);
			SetState( STATE_INITIALISE );
			
		OnUpdate
										
		OnExit
			m_fTimer = 0.0f;



	// ============= STATE_INITIALISE ==========================

	AIState( STATE_INITIALISE )
		OnEnter
			PRINT_STATE(RangeDynCover_Dario::STATE_INITIALISE)
			m_fTimer = 0.0f;
			
			m_pMov->SetMovingWhileFacingTgt(false);

		OnUpdate
		
			// Store the enemy's position and reset the enemy's check timer
			m_obLastEnemyPos = m_pMov->GetEntityToAttack()->GetPosition();
			m_fTimerEnemyPosCheck = 0.0f;

			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_INITIALISE);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}
			// Calculate path to a safe cover point within the Min-Max Range
			bool bCommandExecuted = false;
			bool bForceExecution = (m_iRejections > 3);
			bool bExistPath = CAINavigationSystemMan::Get().FollowPathToCoverPointInMinMaxRange
															(	m_pobEnt,
																bForceExecution,
																&bCommandExecuted,
																m_pMov->GetMaxSpeed(),
																!m_bFarFromMaxRange,
																CAINavigationSystemMan::NF_DEF_FOLLOW_PATH_DYN_COVER 
															);
				
			if (!bExistPath)
			{
				AI_BEHAVIOUR_SEND_MSG( PATH_NOT_FOUND_TO_SUITABLE_COVER_POINT );
				SetState(STATE_IDLE);
			}
			else if ( bCommandExecuted )
			{
				SetState(STATE_MOVING);			
			}
			else
			{
				m_iRejections++;
			}
			if (bForceExecution)
				m_iRejections = 0;
			

		OnExit
			m_fTimer = 0.f;

	// ============= STATE_MOVING ==========================
	
	AIState( STATE_MOVING )
		OnEnter
			PRINT_STATE(RangeDynCover_Dario::STATE_MOVING)
			m_fTimer = 0.0f;

			m_pMov->SetMovingWhileFacingTgt(true);

		OnUpdate
			m_fTimer += fTimeChange;

			if (!m_bFarFromMaxRange)
				m_pobAIComp->ActivateController(CAIComponent::MC_WALKING); //MC_STRAFING);
			else
				m_pobAIComp->ActivateController(CAIComponent::MC_WALKING);

			if (m_pobAIComp->GetDisabled() == true )
			{
				SetReturnToState(STATE_MOVING);
				SetState(STATE_WAIT_UNTIL_RECOVERED);
				return true;
			}
			if (m_pMov->IsMovingToCover())
			{
				// Move to the cover point set in my new intention
				AI_BEHAVIOUR_SEND_MSG ( MSG_MOVING_TO_COVER_POINT );
				//SetState(STATE_MOVING_TO_COVER_POINT);
				SetState(STATE_SCALED_TAKING_COVER);
				m_pMov->SetGoingToCover(false);
				m_pMov->SetCoverPoint(NULL);
				return true;
			}
			if (m_pMov->IsFollowPathCompleted())
			{
				// End of the path reached
				//AI_BEHAVIOUR_SEND_MSG( DESTINATION_REACHED );
				SetState(STATE_PATH_FINISHED_FACING_ENEMY);
				return true;
			}
			if (m_fTimer > fFollowPathTime )
			{
				// Something bad happened (got stuck?)
				AI_BEHAVIOUR_SEND_MSG( TIME_OUT );
				SetState(STATE_IDLE);				
			}

			// ----------------------
			// -- Stuck prevention --
			// ----------------------
			CAINavigPath* pPath = m_pMov->GetPathContainer();
			CAINavigNode* pCurrentNode = pPath->GetCurrentNode();
			if (pCurrentNode && !CAINavigationSystemMan::Get().HasLineOfSight(m_pMov->GetPosition(),pPath->GetPointWithinCurrentNode()) )
			{
				if (++m_iLostLOS > 3)
				{
					SetState(STATE_INITIALISE);
					m_iLostLOS = 0;
					return true;
				}
				PRINT_STATE(RangeDynCover_Dario::STATE_MOVING -> LOS FAILED!)
				//return true;
			}
			else
			{
				m_iLostLOS = 0;
			}

			m_pMov->SetFacingAction(LineAI2Enemy);
			
		OnExit
			m_fTimer = 0;
			m_iLostLOS = 0;
			m_pMov->DeactivateFlag(CAINavigationSystemMan::NF_DEF_FOLLOW_PATH_DYN_COVER);

	// ============= STATE_WAIT_UNTIL_RECOVERED ==========================
	
	AIState( STATE_WAIT_UNTIL_RECOVERED )
		OnEnter
			PRINT_STATE(RangeDynCover_Dario::STATE_WAIT_UNTIL_RECOVERED)
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
			PRINT_STATE(RangeDynCover_Dario::STATE_MOVING_TO_COVER_POINT)
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
																	sqrt(m_pMov->GetCoverPoint()->GetRadiusSQR()),
																	true
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
				SetState(STATE_TAKING_COVER);
				return true;
			} 
			if ( m_fTimer > fMoveToCoverPointTime )
			{
				m_pMov->GetCoverPoint()->SetAvailabe(true);
				SetState(STATE_INITIALISE);

				AI_BEHAVIOUR_SEND_MSG( COVER_POINT_TIME_OUT );
				
			}
			
		OnExit
			m_fTimer = 0;

	// ============= STATE_TAKING_COVER ==========================

	AIState( STATE_TAKING_COVER )
		OnEnter
			PRINT_STATE(RangeDynCover_Dario::STATE_TAKING_COVER)
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
				user_warn_p(bFoundAnim, ("STATE_TAKING_COVER : No going to cover animation found"));
			}

		OnUpdate
			//m_fTimer += fTimeChange;

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
			PRINT_STATE(RangeDynCover_Dario::STATE_COVERING);
			CAINavigationSystemMan::Get().MovingVolley_SetVolleyStatus(m_pobEnt, VOLLEY_FREE);
			m_fTimer = 0.0f;
			
			// Clear previous animations
			m_pobAIComp->CompleteSimpleAction();
			
			// Can I see the enemy?
			if (CAINavigationSystemMan::Get().HasLineOfSight(obAIPos, obEnemyPos))
			{
				SetState(STATE_INITIALISE);
				return true;
			}

			// Play an In-Cover animation
			bool bFoundAnim;
			CHashedString hsCoverAnim = m_pMov->GetCoverPoint()->GetAnimation(ANIM_IN_COVER, &bFoundAnim );
			if (bFoundAnim)
			{
				m_pobAIComp->ActivateSingleAnim(hsCoverAnim);
			}
			else
			{
				// Do nothing? Well at least warn
				user_warn_p(bFoundAnim, ("STATE_COVERING : No in-cover animation found"));
			}

		OnUpdate
			m_fTimer += fTimeChange;
			
			// Can I see the enemy?
			if (CAINavigationSystemMan::Get().HasLineOfSight(obAIPos, obEnemyPos))
			{
				m_pobAIComp->CompleteSimpleAction();
				SetState(STATE_INITIALISE);
				return true;
			}

			// Do we have to go to Volley?
			if ( CAINavigationSystemMan::Get().MovingVolley_GetVolleyStatus() != VOLLEY_FREE )
			{
				// Is there a Commander?
				if ( !m_pCommander )
				{
					// I am the commander
					CAINavigationSystemMan::Get().MovingVolley_SetCommander(m_pobEnt);
					CAINavigationSystemMan::Get().MovingVolley_SetVolleyStatus(VOLLEY_READY);
				}
				SetState(STATE_VOLLEY_READY);
				return true;
			}

			if (  m_fTimerEnemyPosCheck > fCheckEnemyPosTime && 
				  m_bCloserThanMinRange || m_bFarFromMaxRange )
			{
				// Find new cover
				m_fTimerEnemyPosCheck = 0.0f;
				if ( m_bFarFromMaxRange )
					SetState(STATE_BREAKING_COVER);
				else
					SetState(STATE_INITIALISE);
			}

			if ( m_pobAIComp->IsSimpleActionComplete() )
			{
				if (grandf(1.0f) < m_pMov->GetInCoverPeekChance() )
					SetReturnToState(STATE_PLAYING_PEEKING_ANIM);
				else
					SetReturnToState(STATE_COVERING);
			}

		OnExit
			m_fTimer = 0.0f;

	// ============= STATE_PLAYING_PEEKING_ANIM ==========================
	AIState( STATE_PLAYING_PEEKING_ANIM )
		OnEnter
			PRINT_STATE(RangeDynCover_Dario::STATE_PLAYING_PEEKING_ANIM)
			m_fTimer = 0.0f;

			// Clear previous animations
			m_pobAIComp->CompleteSimpleAction();

			// Can I see the enemy?
			if (CAINavigationSystemMan::Get().HasLineOfSight(obAIPos, obEnemyPos))
			{
				m_pobAIComp->CompleteSimpleAction();
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
				user_warn_p(bFoundAnim, ("STATE_PLAYING_PEEKING_ANIM : No peek animation found"));
			}
			
		OnUpdate
			m_fTimer += fTimeChange;

			// Do we have to go to Volley?
			if ( CAINavigationSystemMan::Get().MovingVolley_GetVolleyStatus() != VOLLEY_FREE )
			{
				// Is there a Commander?
				if ( !m_pCommander )
				{
					// I am the commander
					CAINavigationSystemMan::Get().MovingVolley_SetCommander(m_pobEnt);
				}
				SetState(STATE_VOLLEY_READY);
				return true;
			}

			// Can I see the enemy?
			if (CAINavigationSystemMan::Get().HasLineOfSight(obAIPos, obEnemyPos))
			{
				m_pobAIComp->CompleteSimpleAction();
				SetState(STATE_INITIALISE);
				return true;
			}

			if( m_pobAIComp->IsSimpleActionComplete() )
			{
				// Cover again
				SetUserTimer(grandf(1.0f));
				SetReturnToState(STATE_COVERING);
				SetState(STATE_WAIT_FOR_TIMER);
			}

			if (  m_fTimerEnemyPosCheck > fCheckEnemyPosTime && 
				  m_bCloserThanMinRange || m_bFarFromMaxRange )
			{
				// Find new cover
				m_fTimerEnemyPosCheck = 0.0f;
				SetState(STATE_INITIALISE);
			}

			
		OnExit
			m_fTimer = 0.0f;

	// ============= STATE_VOLLEY_READY ==========================
	AIState( STATE_VOLLEY_READY )
		OnEnter
			PRINT_STATE(RangeDynCover_Dario::STATE_VOLLEY_READY)
			m_fTimer = 0.0f;
			CAINavigationSystemMan::Get().MovingVolley_SetVolleyStatus(m_pobEnt, VOLLEY_READY);
			if ( m_pCommander && !m_pCommander->ToAI()->IsDead() )
			{
				// Is me?
				if ( m_pCommander == m_pobEnt )
				{
					CChatterBoxMan::Get().Trigger("AI_XBow_Ready", m_pobEnt);
					CAINavigationSystemMan::Get().MovingVolley_SetVolleyStatus(VOLLEY_AIM);
					SetState(STATE_VOLLEY_COMMANDER_AIM);
					return true;
				}
			}
			else
			{
				// I am the commander
				CAINavigationSystemMan::Get().MovingVolley_SetCommander(m_pobEnt);
				CChatterBoxMan::Get().Trigger("AI_XBow_Ready", m_pobEnt);
				CAINavigationSystemMan::Get().MovingVolley_SetVolleyStatus(VOLLEY_READY);
				SetState(STATE_VOLLEY_COMMANDER_AIM);
				return true;
			}
			
		OnUpdate
			m_fTimer += fTimeChange;

			if ( !m_pCommander || m_pCommander->ToAI()->IsDead() )
			{
				// I am the commander
				CAINavigationSystemMan::Get().MovingVolley_SetCommander(m_pobEnt);
				SetState(STATE_VOLLEY_COMMANDER_AIM);
				CAINavigationSystemMan::Get().MovingVolley_SetVolleyStatus(VOLLEY_READY);
				return true;
			}

			// Do we have to go to Volley?
			if ( CAINavigationSystemMan::Get().MovingVolley_GetVolleyStatus() != VOLLEY_FREE )
			{
				SetState(STATE_VOLLEY_AIM);
				return true;
			}
			
		OnExit
			m_fTimer = 0.0f;

	// ============= STATE_VOLLEY_COMMANDER_AIM ==========================
	AIState( STATE_VOLLEY_COMMANDER_AIM )
		OnEnter
			PRINT_STATE(RangeDynCover_Dario::STATE_VOLLEY_COMMANDER_AIM)
			m_fTimer = 0.0f;
			
			CChatterBoxMan::Get().Trigger("AI_XBow_Aim", m_pobEnt);
			CAINavigationSystemMan::Get().MovingVolley_SetVolleyStatus(m_pobEnt, VOLLEY_AIM);
			
		OnUpdate
			m_fTimer += fTimeChange;

			if ( m_fTimer > 3.0f )
			{
				SetState(STATE_VOLLEY_COMMANDER_FIRE);
				CChatterBoxMan::Get().Trigger("AI_XBow_Fire", m_pobEnt);
				CAINavigationSystemMan::Get().MovingVolley_SetVolleyStatus(VOLLEY_FIRE);
			}
			
		OnExit
			m_fTimer = 0.0f;

	// ============= STATE_VOLLEY_COMMANDER_FIRE ==========================
	AIState( STATE_VOLLEY_COMMANDER_FIRE )
		OnEnter
			PRINT_STATE(RangeDynCover_Dario::STATE_VOLLEY_COMMANDER_FIRE)
			m_fTimer = 0.0f;
			CAINavigationSystemMan::Get().MovingVolley_SetVolleyStatus(m_pobEnt, VOLLEY_FIRE);
						
		OnUpdate
			m_fTimer += fTimeChange;

			if ( m_fTimer > 0.1f )
			{
				SetState(STATE_VOLLEY_SHOOTING);
				CAINavigationSystemMan::Get().MovingVolley_SetVolleyStatus(VOLLEY_FREE);
				CAINavigationSystemMan::Get().MovingVolley_SetCommander(NULL);
			}
			
		OnExit
			m_fTimer = 0.0f;


	// ============= STATE_VOLLEY_AIM ==========================
	AIState( STATE_VOLLEY_AIM )
		OnEnter
			PRINT_STATE(RangeDynCover_Dario::STATE_VOLLEY_AIM)
			m_fTimer = 0.0f;

			CAINavigationSystemMan::Get().MovingVolley_SetVolleyStatus(m_pobEnt, VOLLEY_AIM);

			// Clear previous animations
			m_pobAIComp->CompleteSimpleAction();

			// Can I see the enemy?
			//if (CAINavigationSystemMan::Get().HasLineOfSight(obAIPos, obEnemyPos))
			//{
			//	m_pobAIComp->CompleteSimpleAction();
			//	SetState(STATE_INITIALISE);
			//	return true;
			//}

			// Play a AIM animation
			//m_pMov->PlayAnimation( ACTION_IDLE_AIMING_CROSSBOWMAN );
			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_VOLLEY_AIM);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}
			
			m_pobAIComp->ActivateController(CAIComponent::MC_WALKING);
			m_pobAIComp->SetPlayingFacingAction(true);
	
		OnUpdate

				FaceEnemy();

				//CDirection obFacing = CDirection(obEnemyPos - m_pobEnt->GetPosition());
				//obFacing.Normalise();
				//m_pMov->SetFacingAction(obFacing);
				//CDirection currentFacing;
				//
				//m_pobEnt->GetLookDirection( currentFacing );
				//currentFacing.Normalise();
				//
				//float fFacingDotProduct = obFacing.Dot(currentFacing);
				//if ( fFacingDotProduct > 0.985f )
				//{
				//	AI_BEHAVIOUR_SEND_MSG( FACING_ENTITY );
				//}
	

			if ( CAINavigationSystemMan::Get().MovingVolley_GetVolleyStatus() == VOLLEY_FIRE )
			{
				SetState(STATE_VOLLEY_SHOOTING);
			}

			// Can I see the enemy?
			//if (CAINavigationSystemMan::Get().HasLineOfSight(obAIPos, obEnemyPos))
			//{
			//	m_pobAIComp->CompleteSimpleAction();
			//	SetState(STATE_INITIALISE);
			//	return true;
			//}

		OnExit
			m_pobAIComp->SetPlayingFacingAction(false);

	// ============= STATE_VOLLEY_SHOOTING ==========================
	AIState( STATE_VOLLEY_SHOOTING )
		OnEnter
			PRINT_STATE(RangeDynCover_Dario::STATE_VOLLEY_SHOOTING)

			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_BREAKING_COVER);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}

			CAINavigationSystemMan::Get().MovingVolley_SetVolleyStatus(m_pobEnt, VOLLEY_FIRE);
			
			// Set-Up Timers
			m_fTimer = 0.0f;
			//fTimeBeetweenShoots = m_pMov->GetTimeBetweenShoots();
			m_iVolleyCount = CAINavigationSystemMan::Get().MovingVolley_GetVolleyShots();
			m_fVolleyTimeBtwShots = CAINavigationSystemMan::Get().MovingVolley_GetVolleyPauseBetweenShots();
			m_bInPauseBtwShots = false;

			// Clear previous animations
			m_pobAIComp->CompleteSimpleAction();

			// Stop his movement
			m_pMov->SetChaseTargetCompleted(true);

			m_pobAIComp->ActivateController(CAIComponent::MC_WALKING);
			m_pobAIComp->SetPlayingFacingAction(true);
		
		OnUpdate
			m_fTimer += fTimeChange;

			//if (!m_pVis->IsTargetInShootingRange())
			//{
			//	// The AI has lost LOS with the enemy
			//	if (grandf(1.0f) > 0.2f )
			//	{
			//		// 80% of the time move along
			//		if ( m_bCloserThanMinRange || m_bFarFromMaxRange )
			//		{
			//			SetState(STATE_INITIALISE);
			//			return true;
			//		}
			//		else
			//		{
			//			SetState(STATE_COVERING);
			//			return true;
			//		}
			//	}
			//}
			
			if (m_bInPauseBtwShots)
			{
				m_fVolleyTimeBtwShots -= fTimeChange;
				m_bInPauseBtwShots = (m_fVolleyTimeBtwShots<0.0f) ? false : true;
				FaceEnemy();
			}
			else
			{
				m_fVolleyTimeBtwShots = CAINavigationSystemMan::Get().MovingVolley_GetVolleyPauseBetweenShots();
				// Have we shoot enough?
				if (m_iVolleyCount--  > 0)
				{
					// Keep on shooting
					AI_BEHAVIOUR_SEND_MSG( ATTACK_IN_SHOOTING_RANGE );
					m_bInPauseBtwShots = true;
				}
				else
				{
					// Finished
					SetState(STATE_COVERING);
				}
			}

		OnExit
			m_fTimer = 0.0f;
			//fTimeBeetweenShoots = 0.0f;
			m_pobAIComp->SetPlayingFacingAction(false);
			
	// ============= STATE_SHOOTING ==========================
	AIState( STATE_SHOOTING )
		OnEnter
			PRINT_STATE(RangeDynCover_Dario::STATE_SHOOTING)
			
			// Set-Up Timers
			m_fTimer = 0.0f;
			//fTimeBeetweenShoots = m_pMov->GetTimeBetweenShoots();
			
			// Clear previous animations
			m_pobAIComp->CompleteSimpleAction();

			// Stop his movement
			m_pMov->SetChaseTargetCompleted(true);
			//m_pMov->SetMaxSpeed(0.0f);
			m_pMov->SetMovingWhileFacingTgt( true );

			// Get Times
			//fTimeBeetweenShoots = m_pMov->GetTimeBetweenShoots(false);
		
		OnUpdate
			m_fTimer += fTimeChange;

			if (!m_pVis->IsTargetInShootingRange())
			{
				// The AI has lost LOS with the enemy
				if (grandf(1.0f) > 0.2f )
				{
					// 80% of the time move along
					if ( m_bCloserThanMinRange || m_bFarFromMaxRange )
					{
						SetState(STATE_INITIALISE);
						return true;
					}
					else
					{
						SetState(STATE_COVERING);
						return true;
					}
				}
			}

			// Have we shoot enough?
			if ( m_pMov->EnoughShooting() )
			{
				m_pMov->ClearShotsCount();
				
				// What to do next?
				if (grandf(1.0f) < 0.4f )
				{
					// Shoot again
					SetUserTimer(m_pMov->GetTimeBetweenShoots(false));
					SetReturnToState(STATE_SHOOTING);
					SetState(STATE_WAIT_FOR_TIMER);
				}
				else
				{
					// Relax for a bit
					SetUserTimer(grandf(1.0f));
					SetReturnToState(STATE_COVERING);
					SetState(STATE_WAIT_FOR_TIMER);
				}
			}
			else
			{
				// Report 
				AI_BEHAVIOUR_SEND_MSG( ATTACK_IN_SHOOTING_RANGE );
			}

			if (  m_fTimerEnemyPosCheck > fCheckEnemyPosTime && 
				  m_bCloserThanMinRange || m_bFarFromMaxRange )
			{
				// Find new cover
				m_fTimerEnemyPosCheck = 0.0f;
				SetState(STATE_INITIALISE);
			}


		OnExit
			m_fTimer = 0.0f;
			//fTimeBeetweenShoots = 0.0f;

	// ============= STATE_BREAKING_COVER ==========================
	AIState( STATE_BREAKING_COVER )
		OnEnter
			PRINT_STATE(RangeDynCover_Dario::STATE_BREAKING_COVER)
			m_fTimer = 0.0f;

			// Clear previous animations
			m_pobAIComp->CompleteSimpleAction();

			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_BREAKING_COVER);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}

			// Check that there is space to break cover
			CEntity* pAI = NULL;
			pAI = CAINavigationSystemMan::Get().TestDive_AICollision(m_pMov->GetParent(),E_DIVE_LONG_LEFT);
			if (pAI && !pAI->ToAI()->GetAIComponent()->GetCAIMovement()->IsInFormationMovement())
			{
				SetUserTimer(1.0f);
				SetReturnToState(STATE_BREAKING_COVER);
				SetState(STATE_WAIT_FOR_TIMER);
				return true;
			}

			pAI = CAINavigationSystemMan::Get().TestDive_AICollision(m_pMov->GetParent(),E_DIVE_LONG_RIGHT);
			if (pAI && !pAI->ToAI()->GetAIComponent()->GetCAIMovement()->IsInFormationMovement())
			{
				SetUserTimer(1.0f);
				SetReturnToState(STATE_BREAKING_COVER);
				SetState(STATE_WAIT_FOR_TIMER);
				return true;
			}

			// Play a Break-Cover animation
			bool bFoundAnim;
			CHashedString hsCoverAnim = m_pMov->GetCoverPoint()->GetAnimation(ANIM_BREAK_COVER, &bFoundAnim, m_pMov->GetParent() );
			if (bFoundAnim)
			{
				m_pobAIComp->ActivateSingleAnim(hsCoverAnim);
			}
			else
			{
				// Do nothing? Well at least warn
				user_warn_p(bFoundAnim,("STATE_BREAKING_COVER : No Break-Cover animation found"));
			}

		OnUpdate
			
			// As soon as the animation is finished, move on.
			if( m_pobAIComp->IsSimpleActionComplete() )
			{
				m_pMov->GetCoverPoint()->SetAvailabe(true);
				
				// Generate a new path to the current enemy's position
				SetState(STATE_INITIALISE);
			}
			
		OnExit
			m_fTimer = 0.0f;

	// ============= STATE_IDLE ==========================
	AIState( STATE_IDLE )
		OnEnter
			PRINT_STATE(RangeDynCover_Dario::STATE_IDLE)
			m_fTimer = 0.0f;
			
			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_IDLE);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}

			m_pobAIComp->CompleteSimpleAction();
			m_pobAIComp->SetPlayingFacingAction(true);
			m_pobAIComp->ActivateController(CAIComponent::MC_WALKING);

		OnUpdate
			m_fTimer += fTimeChange;

			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_IDLE);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}

			FaceEnemy();

			if (  m_fTimerEnemyPosCheck > fCheckEnemyPosTime && 
				  m_bCloserThanMinRange || m_bFarFromMaxRange )
			{
				// Find new cover
				m_fTimerEnemyPosCheck = 0.0f;
				SetState(STATE_INITIALISE);
			}

			if ( m_fTimer > fMaxIdleTime )
				SetState(STATE_INITIALISE);
			
		OnExit
			m_fTimer = 0.0f;
			m_pobAIComp->SetPlayingFacingAction(false);

		// ============= STATE_PATH_FINISHED_FACING_ENEMY ==========================
		AIState( STATE_PATH_FINISHED_FACING_ENEMY )
		OnEnter
			PRINT_STATE(RangeDynCover_Dario::STATE_PATH_FINISHED_FACING_ENEMY)
			m_fTimer = 0.0f;

			m_pobAIComp->CompleteSimpleAction();

			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_PATH_FINISHED_FACING_ENEMY);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}
			
			m_pobAIComp->SetPlayingFacingAction(true);
			m_pobAIComp->ActivateController(CAIComponent::MC_WALKING);
			
		OnUpdate
			
			// AI has walked the path that suposedly led to the enemy
			// ... but he has not encontered the enemy

			m_fTimer += fTimeChange;

			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_PATH_FINISHED_FACING_ENEMY);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}
			
			FaceEnemy();

			if (  m_fTimerEnemyPosCheck > fCheckEnemyPosTime && 
				  m_bCloserThanMinRange || m_bFarFromMaxRange )
			{
				// Find new cover
				m_fTimerEnemyPosCheck = 0.0f;
				SetState(STATE_INITIALISE);
			}

			if ( m_fTimer > fMaxIdleTime )
				SetState(STATE_INITIALISE);

		OnExit
		
			m_fTimer = 0.0f;
			m_pobAIComp->SetPlayingFacingAction(false);

		// ============= STATE_SCALED_TAKING_COVER ==========================

		AIState( STATE_SCALED_TAKING_COVER )
		OnEnter
			PRINT_STATE(RangeDynCover_Dario::STATE_SCALED_TAKING_COVER)
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
				obScaledCoverDefinition.m_bNoDirectionCorrectionScaling = m_pMov->GetCoverPoint()->IsJumpOver();//false;
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
			}
			else
			{
				// Do nothing? Well at least warn
				//ntAssert_p(bFoundAnim,("STATE_TAKING_COVER : No going to cover animation found"));
				ntPrintf("STATE_TAKING_COVER : No going to cover animation found");
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
			PRINT_STATE(RangeDynCover_Dario::STATE_ADJUSTING_COVER_ANGLE);
			m_fTimer = 0.0f;

			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_ADJUSTING_COVER_ANGLE);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}

			m_pobAIComp->CompleteSimpleAction();
			m_pobAIComp->SetPlayingFacingAction(true);
			m_pobAIComp->ActivateController(CAIComponent::MC_WALKING);

			// Get the facing direction 
			CAINavigCoverPoint* pCoverPoint = m_pMov->GetCoverPoint();
			m_obValidCoverDirection = pCoverPoint->GetValidDir();
			m_obValidCoverDirection.Normalise();
						
			UNUSED(fAlmostOne);
		OnUpdate
			
			m_fTimer += fTimeChange;

			if ( m_fTimer > grandf(0.3f) + 0.1f )
			{
				CAINavigCoverPoint* pCoverPoint = m_pMov->GetCoverPoint();
				CPoint obCoverPos = CPoint(pCoverPoint->GetPos().X(), m_pobEnt->GetPosition().Y(),pCoverPoint->GetPos().Z());
				m_pobEnt->SetPosition(obCoverPos);
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

			
		// ============= STATE_WAIT_FOR_CS_STANDARD ==========================
		AIState( STATE_WAIT_FOR_CS_STANDARD )
		OnEnter
			PRINT_STATE(RangeDynCover_Dario::STATE_WAIT_FOR_CS_STANDARD)
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
			PRINT_STATE(RangeDynCover_Dario::STATE_WAIT_FOR_TIMER)
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



