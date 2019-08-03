//! -------------------------------------------
//! aibehaviour_whackamole.cpp
//!
//! Shoot on Spot and Cover behaviour for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

//------------------------------------------------------------------------------------------
// Includes                                                                                 
//------------------------------------------------------------------------------------------


#include "ai/aibehaviour_whackamole.h"
#include "game/messagehandler.h"
#include "game/aicomponent.h"
#include "game/entitymanager.h"
#include "game/entityinfo.h"
#include "game/query.h"
#include "game/movementcontrollerinterface.h"
#include "game/interactiontransitions.h"
#include "game/movement.h"
#include "game/chatterboxman.h"
#include "ai/AINavigationSystem/ainavigsystemman.h"

#ifndef _RELEASE
#include "core/visualdebugger.h"
#endif // _RELEASE

//--------------------------------------------------
//!	Destructor
//--------------------------------------------------
CAIWhackAMoleBehaviour::~CAIWhackAMoleBehaviour( void ) 
{
	PRINT_STATE(WhackAMole_Dario::DESTRUCTOR)
	m_pobAIComp->SetPlayingFacingAction(false);
}

//--------------------------------------------------
//!	SnapAngle
//--------------------------------------------------
bool CAIWhackAMoleBehaviour::SnapAngle( void )
{
	if ( !m_pTCNode || !m_pobEnt->GetMovement() )
		return false;
	CDirection CurrentDir	= m_pobEnt->GetMatrix().GetZAxis();
	CDirection WMDir		= m_pTCNode->GetWhackAMoleDir();
	if (WMDir.Compare(CurrentDir,0.1f))
	{
		m_pobEnt->SetPosition(m_SnapPoint);
		return true;
	}

	// Clear previous animations
	m_pobAIComp->CompleteSimpleAction();
	m_pobAIComp->SetPlayingFacingAction(false);
	
	// Correct Angle
	CorrectiveMovementTransitionDef obDef;
	obDef.SetDebugNames( "", "CorrectiveTransitionDef" );
	obDef.m_obTargetPostion = m_SnapPoint;
	obDef.m_obTargetRotation = CQuat(CQuat(CurrentDir, WMDir)); //CVector(WMDir)); 
	bool ret = m_pobEnt->GetMovement()->BringInNewController( obDef, CMovement::DMM_STANDARD, 0.0f );
	return ret;
}

CEntity* CAIWhackAMoleBehaviour::GetVolleyCommander(int iSquad)
{
	QueryResultsContainerType* pLocalAIList = CAINavigationSystemMan::Get().GetAIList();

	if (pLocalAIList)
	{
		QueryResultsContainerType::iterator obEnd = pLocalAIList->end();

		for (QueryResultsContainerType::iterator obIt = pLocalAIList->begin(); obIt != obEnd; ++obIt)
		{
			CEntity* pEntity = *obIt;

			if (pEntity->ToCharacter()->GetRangedWeapon() &&
				!pEntity->ToCharacter()->IsPaused() &&
				!pEntity->ToCharacter()->IsDead() &&
				pEntity->ToAI()->GetAIComponent()->GetCAIMovement()->GetVolleySquad() == iSquad &&
				pEntity->ToAI()->GetAIComponent()->GetCAIMovement()->GetVolleyShots() >= 0)
			{
				return pEntity;
			}
		}
	}

	return 0;
}

bool CAIWhackAMoleBehaviour::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
{
	const float fInitTimeMax				(0.3f);
	//const float fMinTimeWithoutCover		(5.0f);
//	const float fMaxDeltaTimeWithoutCover	(5.0f);
	const float fMaxCoverTime				(4.0f);
	const float fCoverAnimDuration			(0.5f);
	const float fAlmostOne					(0.93f);

	// Only proceed when the AI is alive and kicking
	if ( m_pobEnt->IsDead() || m_pobEnt->IsPaused() )
		return false;
	
	// Cache a vision object pointer 
	const CAIVision* pVis = m_pobAIComp->GetAIVision();

	// Check for a valid enemy
	const CEntity* pCharTgt = m_pMov->GetEntityToAttack(); 
	if (!pCharTgt)
	{
		user_error_p(0,("AI: [%s] is in Whack-A-Mole behaviour without a TARGET",ntStr::GetString(m_pobEnt->GetName())));
		return false;
	}

	// Check that it is a ranged AI
	if (((Character*)m_pobEnt)->GetRangedWeapon() == NULL)
	{
		user_warn_msg(("AI: [%s] is in Whack-A-Mole behaviour but is not a RANGED AI",ntStr::GetString(m_pobEnt->GetName())));
		return false;
	}

	m_pobAIComp->SetInCover(true);

BeginStateMachine
	// ============= STATE_ZERO ==========================
	AIState( STATE_ZERO )
		OnEnter
			PRINT_STATE(WhackAMole_Dario::STATE_ZERO)
			m_fTimer = 0.0f;
			m_fInitTime = grandf(fInitTimeMax);
	
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
			PRINT_STATE(WhackAMole_Dario::STATE_INITIALISE)
			m_fTimer = 0.0f;

			m_pTCNode = m_pMov->GetWhackAMoleNode();

			m_pobFacingEntity = CEntityManager::Get().FindEntity( m_pobAIComp->GetScriptFacingEntityName() );

			if (m_pobFacingEntity)
			{
				m_obEntityPos = m_pobFacingEntity->GetPosition();
				m_bEntityFound = true;
				if (m_pTCNode)
				{
					//m_pMov->SetWhackAMoleDir(m_pobEnt->GetMatrix().GetZAxis());
					m_pTCNode->SetWhackAMoleDir(m_pobEnt->GetMatrix().GetZAxis());
					SetState(STATE_HIDDEN);
				}
				else
					SetState( STATE_TURNTOFACE_AND_AIM );
				return true;
			}
			else
			{
				m_bEntityFound = false;
				user_warn_msg( ("AIFaceEntity: Entity not found, %s\n", m_pobAIComp->GetScriptLocatorName()) );
				AI_BEHAVIOUR_SEND_MSG( ENTITY_NOT_FOUND );
				SetState( STATE_ENTITY_NOT_FOUND );
				return false;
			}

		OnUpdate
			
		OnExit
			m_fTimer = 0.0f;

	// ============= STATE_INITIALISE ==========================
	AIState( STATE_TURNTOFACE_AND_AIM )
		OnEnter
			PRINT_STATE(WhackAMole_Dario::STATE_TURNTOFACE_AND_AIM);
			m_fTimer = 0.0f;
			
			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_TURNTOFACE_AND_AIM);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}
			m_pobAIComp->CompleteSimpleAction();
			if (!m_pTCNode)
			{
				m_pobAIComp->SetPlayingFacingAction(true);
				m_pobAIComp->ActivateController(CAIComponent::MC_WALKING);
			}

			// Free fire
			if (m_pMov->GetVolleyShots() < 0)
			{
				m_fTimeBetweenShots = m_pMov->GetTimeBetweenShoots();
			}
			// Volley fire.
			else
			{
				m_iVolleyCount = m_pMov->GetVolleyShots();

				if (m_pobEnt == GetVolleyCommander(m_pMov->GetVolleySquad()))
					CChatterBoxMan::Get().Trigger("AI_XBow_Ready", m_pobEnt);

				m_fTimeBetweenShots = m_pMov->GetVolleyPauseBetweenShotsMin();

				if (m_pobEnt != GetVolleyCommander(m_pMov->GetVolleySquad()))
					m_fTimeBetweenShots += drandf(m_pMov->GetVolleyPauseBetweenShotsMax() - m_fTimeBetweenShots);
			}

			m_pMov->ClearShotsCount();

		OnUpdate
			
			// -------------------------------------------------------------
			// Face Enemy --------------------------------------------------
			// -------------------------------------------------------------
			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_TURNTOFACE_AND_AIM);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}
			if (!m_pTCNode)
			{
				m_obEntityPos = m_pobFacingEntity->GetPosition();
				m_obFacing = CDirection(m_obEntityPos - m_pobEnt->GetPosition());
				m_obFacing.Normalise();
				m_pMov->SetFacingAction(m_obFacing);
				CDirection currentFacing;
				
				m_pobEnt->GetLookDirection( currentFacing );
				currentFacing.Normalise();
				
				float fFacingDotProduct = m_obFacing.Dot(currentFacing);
				if ( fFacingDotProduct > fAlmostOne )
				{
					//m_pobAIComp->SetPlayingFacingAction(false);
					AI_BEHAVIOUR_SEND_MSG( FACING_ENTITY );
				}
				else
				{
					return false;
				}
			}

			// BEWARE OF THIS LINE WHEN A NODE EXISTS!!!!!!!!!
			m_pobAIComp->SetInCover(false);

			// Aiming period before first shot.
			if (m_fTimer < m_pMov->GetVolleyAimPause() && m_pMov->GetVolleyShots() >= 0)
			{
				bool bAllOtherComradesOutOfCover = true;

				// Check other entities are all out of cover before starting volley. Once started finish regardless of other entities.
				QueryResultsContainerType* pLocalAIList = CAINavigationSystemMan::Get().GetAIList();

				if (pLocalAIList)
				{
					QueryResultsContainerType::iterator obEnd = pLocalAIList->end();

					for (QueryResultsContainerType::iterator obIt = pLocalAIList->begin(); obIt != obEnd; ++obIt)
					{
						CEntity* pEntity = *obIt;

						if (pEntity->ToCharacter()->GetRangedWeapon() &&
							!pEntity->ToCharacter()->IsPaused() &&
							!pEntity->ToCharacter()->IsDead() &&
							pEntity->ToAI()->GetAIComponent()->GetCAIMovement()->GetVolleySquad() == m_pMov->GetVolleySquad() &&
							pEntity->ToAI()->GetAIComponent()->GetCAIMovement()->GetVolleyShots() >= 0 &&
							pEntity->ToAI()->GetAIComponent()->InCover())
						{
							bAllOtherComradesOutOfCover = false;
							break;
						}
					}
				}

				if (bAllOtherComradesOutOfCover)
				{  
					if (m_fTimer == 0.0f)
					{
						if (m_pobEnt == GetVolleyCommander(m_pMov->GetVolleySquad()))
							CChatterBoxMan::Get().Trigger("AI_XBow_Aim", m_pobEnt);

						m_pobAIComp->ActivateSingleAnim("CrossbowmanAimIdle", 0.9f + drandf(0.2f));
					}

					m_fTimer += fTimeChange;
				}
			}
			else
			{
				SetState(STATE_SHOOT);
			}

		OnExit
			m_fTimer = 0.0f;

	AIState(STATE_SHOOT)

		OnEnter
			PRINT_STATE(WhackAMole_Dario::STATE_SHOOT)

		OnUpdate
			if (pVis->IsTargetInShootingRange())
			{
				// Stop aiming animation.
				m_pobAIComp->CancelSingleAnim();

				// Update Shooting Timer
				m_fTimeBetweenShots -= fTimeChange;

				// Shoot
				if (m_fTimeBetweenShots <= 0.0f && (m_pMov->EnoughShooting() || m_pMov->GetVolleyShots() >= 0))
				{
					m_pMov->IncrementShotsCount();
					AI_BEHAVIOUR_SEND_MSG( ATTACK_IN_SHOOTING_RANGE );

					// Dialogue on first shot only, for volley fire and from guy in charge.
					if (m_iVolleyCount == m_pMov->GetVolleyShots() && m_pMov->GetVolleyShots() >= 0 && m_pobEnt == GetVolleyCommander(m_pMov->GetVolleySquad()))
						CChatterBoxMan::Get().Trigger("AI_XBow_Fire", m_pobEnt);

					if (m_iVolleyCount > 0)
						--m_iVolleyCount;

					if (m_iVolleyCount <= 0)
					{
						// -------------------------------------------------------------
						// Move to Cover (Preparations) ---------------------------------
						// -------------------------------------------------------------
						float fCoverTime = drandf(fMaxCoverTime);
						unsigned int uiTotalCoverAnimCycles = (unsigned int)(fCoverTime/fCoverAnimDuration);
						if (uiTotalCoverAnimCycles < 6)
							uiTotalCoverAnimCycles=6;

						m_pMov->SetTotalCoverCycles(uiTotalCoverAnimCycles);
						m_pMov->ClearCurrentCoverCycle();
						// -- Hack to ensure the firing anim is finished when crouching
						SetUserTimer(1.5f);
						SetReturnToState(STATE_FINISH_SHOOTING_ANIM);
						SetState(STATE_WAIT_FOR_TIMER);

						return true;
					}
					else
					{
						m_fTimeBetweenShots = m_pMov->GetVolleyPauseBetweenShotsMin();
					}
				}
			} 
			else
			{
				// Jeering when out of range.
				CChatterBoxMan::Get().Trigger("PlayerOutOfRange", m_pobEnt);
			}

		OnExit
			m_pobAIComp->SetPlayingFacingAction(false); 

	// ============= STATE_FINISH_SHOOTING_ANIM ==========================

	AIState( STATE_FINISH_SHOOTING_ANIM )
		OnEnter
			if (m_iVolleyCount == 0)
				m_pobAIComp->SetInCover(true);

			PRINT_STATE(WhackAMole_Dario::STATE_FINISH_SHOOTING_ANIM)
			m_fTimer = 0.0f;

		OnUpdate

			if (m_pobAIComp->IsSimpleActionComplete() )
			{
				if (!m_pTCNode)
					SetState(STATE_CROUCH_DOWN);
				else
				{
					//m_SnapPoint = m_pMov->GetPosition();
					//SetReturnToState(STATE_RETURN_TO_COVER);
					//SetState(STATE_SNAP);
					SetState(STATE_RETURN_TO_COVER);
				}
				return true;
			}
		OnExit
			m_pobAIComp->CompleteSimpleAction();
			m_fTimer = 0.0f;

	// ============= STATE_SNAP ==========================

	AIState( STATE_SNAP )
		OnEnter
			PRINT_STATE(WhackAMole_Dario::STATE_SNAP)

		OnUpdate
			
			if (IsSaveToPushAController())
			{
				SnapAngle();
				SetState(GetReturnToState());
			}

		OnExit
			m_pobAIComp->CompleteSimpleAction();

	// ==============================================

	AIState(STATE_CROUCH_DOWN)
		OnEnter
			PRINT_STATE(WhackAMole_Dario::STATE_CROUCH_DOWN)
			m_pobAIComp->ActivateSingleAnim("crossbowman_stand2crouch", 0.9f + drandf(0.2f)) ;

		OnUpdate
			if (m_pobAIComp->IsSimpleActionComplete())
			{
				SetState(STATE_CROUCHING);
			}

		OnExit

	// ==============================================

	AIState( STATE_CROUCHING )
		OnEnter
			PRINT_STATE(WhackAMole_Dario::STATE_CROUCHING)
			m_fTimer = 0.0f;

			// Clear previous animations
			m_pobAIComp->CompleteSimpleAction();
			m_pobAIComp->SetPlayingFacingAction(false);

			// Send ChatterBox Trigger
			CChatterBoxMan::Get().Trigger("WhackamoleTakingCover", m_pMov->GetParent());

			// Play a Taking_Cover animation
			m_pMov->PlayAnimation( ACTION_WHACK_A_MOLE_CROUCH );
			
		OnUpdate

			if( m_pobAIComp->IsSimpleActionComplete() )
			{
				// A cycle is finished
				if (m_pMov->AreCoverCyclesLeft())
				{
					// More cycles are needed
					m_pMov->PlayAnimation( ACTION_WHACK_A_MOLE_CROUCH );
					m_pMov->IncCurrentCoverCycle();
				}
				else
				{
					// Cover Cycles finished
					m_pMov->SetTotalCoverCycles(0);
					m_pMov->ClearCurrentCoverCycle();

					// Send ChatterBox Trigger
					CChatterBoxMan::Get().Trigger("WhackamoleBreakingCover", m_pMov->GetParent());
					SetState(STATE_RELOADING);
				}
			}
			
		OnExit
			m_fTimer = 0.0f;

		// =================================================
		AIState (STATE_RELOADING)

			OnEnter
				PRINT_STATE(WhackAMole_Dario::STATE_RELOADING)

				if (m_pMov->GetVolleyShots() >= 0)
				{
					if (m_pobEnt == GetVolleyCommander(m_pMov->GetVolleySquad()))
						CChatterBoxMan::Get().Trigger("AI_XBow_Reload", m_pobEnt);

					m_pobAIComp->ActivateSingleAnim("crossbowman_crouched_reload", 0.9f + drandf(0.2f)) ;
				}

				m_fTimer = m_pMov->GetVolleyReloadPauseMin();

				if (m_pobEnt != GetVolleyCommander(m_pMov->GetVolleySquad()))
					m_fTimer += drandf(m_pMov->GetVolleyReloadPauseMax() - m_fTimer);


			OnUpdate

				if (m_pobAIComp->IsSimpleActionComplete())
				{
					m_fTimer -= fTimeChange;

					if (m_fTimer <= 0.0f)
					{
						SetState(STATE_STAND_UP);
					}
				}

			OnExit

		// ==============================================

		AIState(STATE_STAND_UP)
			OnEnter
			PRINT_STATE(WhackAMole_Dario::STATE_STAND_UP)
			m_pobAIComp->ActivateSingleAnim("crossbowman_crouch2stand", 0.9f + drandf(0.2f)) ;

		OnUpdate
			if (m_pobAIComp->IsSimpleActionComplete())
			{
				SetState(STATE_TURNTOFACE_AND_AIM);
			}

			OnExit

	// ============= STATE_HIDDEN ==========================
		AIState( STATE_HIDDEN )
		OnEnter
			PRINT_STATE(WhackAMole_Dario::STATE_HIDDEN)
			m_fTimer = 0.0f;

			// Clear previous animations
			m_pobAIComp->CompleteSimpleAction();
			m_pobAIComp->SetPlayingFacingAction(false);
			
			// Get the cover time
			m_fHiddingTime = drandf(fMaxCoverTime-1.0f) + 2.0f;
			
		OnUpdate

			m_fTimer += fTimeChange;
			
			// For the time being just stand there for 5 sec.
			if (m_fTimer > m_fHiddingTime )
			{
				m_SnapPoint = m_pTCNode->GetPos();
				SetReturnToState(STATE_BREAK_COVER);
				SetState(STATE_SNAP);
			}
			
		OnExit
			m_fTimer = 0.0f;

	// ============= STATE_BREAK_COVER ==========================
	AIState( STATE_BREAK_COVER )
		OnEnter
			PRINT_STATE(WhackAMole_Dario::STATE_BREAK_COVER);
			
			// Clear previous animations
			m_pobAIComp->CompleteSimpleAction();

			// Send ChatterBox Trigger
			CChatterBoxMan::Get().Trigger("WhackamoleBreakingCover", m_pMov->GetParent());

			// Play a Break_Cover animation
			bool bFoundAnim;
			CHashedString hsCoverAnim = m_pTCNode->GetTCBreakCoverAnimation(&bFoundAnim);
			if (bFoundAnim)
			{
				if (!IsSaveToPushAController())
				{
					SetReturnToState(STATE_BREAK_COVER);
					SetState(STATE_WAIT_FOR_CS_STANDARD);
					return true;
				}

				m_pobAIComp->ActivateSingleAnim(hsCoverAnim);
			}
			else
			{
				ntPrintf("WhackAMole_Dario::STATE_BREAK_COVER - AI: [%s] : No breaking cover animations.",ntStr::GetString(m_pobEnt->GetName()));
				SetState( STATE_NO_BREAK_COVER_ANIMS );
			}

		OnUpdate

			// Start shooting as soon as the animation is finished
			if(	m_pobAIComp->IsSimpleActionComplete() )
			{
				// Get Aiming Time
				float fMin = 0.0f, fMax =0.0f;
				m_pMov->GetMinMaxWMAimingTime(&fMin,&fMax);
				m_fAimingTime = erandf(fMax-fMin) + fMin;
				
				SetState( STATE_IDLE_PRESHOOTING_ACTION );
			}

		OnExit
			m_fTimer = 0.0f;

	// ============= STATE_IDLE_PRESHOOTING_ACTION ==========================
	AIState( STATE_IDLE_PRESHOOTING_ACTION )
		OnEnter
			PRINT_STATE(WhackAMole_Dario::STATE_IDLE_PRESHOOTING_ACTION);
			
			// Clear previous animations
			m_pobAIComp->CompleteSimpleAction();

			// Play an idle aiming animation
			bool bOK = m_pMov->PlayAnimation(ACTION_IDLE_AIMING_CROSSBOWMAN);
			if (!bOK)
				SetState(STATE_RETURN_TO_COVER);

		OnUpdate
			m_fAimingTime -= fTimeChange;
        	// Back to cover as soon as the animation is finished
			if(	m_pobAIComp->IsSimpleActionComplete() )
			{
				if (m_fAimingTime > 0.5f) // The idle anim takes around 1 sec.
					SetState( STATE_IDLE_PRESHOOTING_ACTION );
				else
					SetState( STATE_TURNTOFACE_AND_AIM );
			}

		OnExit
			m_fAimingTime = 0.0f;

	// ============= STATE_RETURN_TO_COVER ==========================
	AIState( STATE_RETURN_TO_COVER )
		OnEnter
			PRINT_STATE(WhackAMole_Dario::STATE_RETURN_TO_COVER);
			
			// Clear previous animations
			m_pobAIComp->CompleteSimpleAction();

			// Send ChatterBox Trigger
			CChatterBoxMan::Get().Trigger("WhackamoleTakingCover", m_pMov->GetParent());

			// Play a Break_Cover animation
			bool bFoundAnim;
			CHashedString hsCoverAnim = m_pTCNode->GetTCReturnToCoverAnimation(&bFoundAnim);
			if (bFoundAnim)
			{
				if (!IsSaveToPushAController())
				{
					SetReturnToState(STATE_RETURN_TO_COVER);
					SetState(STATE_WAIT_FOR_CS_STANDARD);
					return true;
				}
				m_pobAIComp->ActivateSingleAnim(hsCoverAnim);
			}
			else
			{
				ntPrintf("WhackAMole_Dario::STATE_RETURN_TO_COVER - AI: [%s] : No return-to-cover animations.",ntStr::GetString(m_pobEnt->GetName()));
				SetState( STATE_NO_RETURN_TO_COVER_ANIMS );
				return false;
			}

		OnUpdate

			// Hide as soon as the animation is finished
			if(	m_pobAIComp->IsSimpleActionComplete() )
			{
				SetState( STATE_HIDDEN );
			}

		OnExit

			
	
	// ============= STATE_NO_BREAK_COVER_ANIMS ======================
	AIState( STATE_NO_BREAK_COVER_ANIMS )
		OnEnter
			PRINT_STATE(WhackAMole_Dario::STATE_NO_BREAK_COVER_ANIMS);
			
		OnUpdate

		OnExit

	// ============= STATE_NO_RETURN_TO_COVER_ANIMS ======================
	AIState( STATE_NO_RETURN_TO_COVER_ANIMS )
		OnEnter
			PRINT_STATE(WhackAMole_Dario::STATE_NO_RETURN_TO_COVER_ANIMS);
			
		OnUpdate

		OnExit

	// ============= STATE_ENTITY_NOT_FOUND ==========================
	AIState( STATE_ENTITY_NOT_FOUND )
		OnEnter
			PRINT_STATE(WhackAMole_Dario::STATE_ENTITY_NOT_FOUND);
			
		OnUpdate

		OnExit

	// ============= STATE_WAIT_FOR_CS_STANDARD ==========================
	AIState( STATE_WAIT_FOR_CS_STANDARD )
		OnEnter
			PRINT_STATE(WhackAMole_Dario::STATE_WAIT_FOR_CS_STANDARD)
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
			PRINT_STATE(WhackAMole_Dario::STATE_WAIT_FOR_TIMER)
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

