//! -------------------------------------------
//! aibehaviour_faceentity.cpp
//!
//! Face Entity behaviour for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

//------------------------------------------------------------------------------------------
// Includes                                                                                 
//------------------------------------------------------------------------------------------


#include "ai/aibehaviour_faceentity.h"
#include "game/messagehandler.h"
#include "game/aicomponent.h"
#include "game/entitymanager.h"
#include "game/entityinfo.h"
#include "game/query.h"
#include "game/movementcontrollerinterface.h"

//--------------------------------------------------
//!	CAIRangeDynamicCoverBehaviour::Destructor
//--------------------------------------------------
CAIFaceEntityBehaviour::~CAIFaceEntityBehaviour( void ) 
{
	PRINT_STATE(FaceEntity_Dario::DESTRUCTOR)
	m_pobAIComp->SetPlayingFacingAction(false);
}

bool CAIFaceEntityBehaviour::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
{
	const float fAlmostOne = 0.98f;

	// Only do something is the AI is active
	if (m_pobEnt->IsDead() || m_pobEnt->IsPaused())
		return false;

	// Check for a valid enemy
	const CEntity* pCharTgt = m_pMov->GetEntityToAttack(); 

	// Cache a vision object pointer 
	const CAIVision* pVis = m_pobAIComp->GetAIVision();

	// If Range AI take care of mele and range distances
	if( pCharTgt && ((Character*)m_pobEnt)->GetRangedWeapon())
	{
		// Update Shooting Timer
		m_fTimeBetweenShots+=fTimeChange;

		// ==================================================================
		// Is a range guy with his enemy in shooting range or in mele range?
		// ==================================================================

		if (pVis->IsTargetInMeleRange())
		{
			AI_BEHAVIOUR_SEND_MSG( ATTACK_IN_MELE_RANGE );
			return true;
		}
		
		if (pVis->IsTargetInShootingRange())
		{
			// Shoot
			if (m_fTimeBetweenShots>m_fShootingTimeThreshold)
			{
				m_fTimeBetweenShots=0.0f;
				m_fShootingTimeThreshold = m_pMov->GetTimeBetweenShoots();
				AI_BEHAVIOUR_SEND_MSG( ATTACK_IN_SHOOTING_RANGE );
			}
			return true;
		} 
	}

BeginStateMachine

	// ============= STATE_INITIALISE ==========================
	AIState( STATE_INITIALISE )
		OnEnter
			PRINT_STATE(FaceEntity_Dario::STATE_INITIALISE);
			m_fTimer = 0.0f;

			m_pobFacingEntity = CEntityManager::Get().FindEntity( m_pobAIComp->GetScriptFacingEntityName() );

			if (m_pobFacingEntity)
			{
				m_obEntityPos = m_pobFacingEntity->GetPosition();
				m_bEntityFound = true;
				SetState( STATE_TURNTOFACE );
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
	AIState( STATE_TURNTOFACE )
		OnEnter
			PRINT_STATE(FaceEntity_Dario::STATE_TURNTOFACE);
			m_fTimer = 0.0f;
			
			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_TURNTOFACE);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}
			m_pobAIComp->CompleteSimpleAction();
			m_pobAIComp->SetPlayingFacingAction(true);
			m_pobAIComp->ActivateController(CAIComponent::MC_WALKING);

		OnUpdate
			m_fTimer += fTimeChange;
			
			m_obEntityPos = m_pobFacingEntity->GetPosition();
			m_obFacing = CDirection(m_obEntityPos - m_pobEnt->GetPosition());
			m_obFacing.Normalise();
			m_pobAIComp->GetCAIMovement()->SetFacingAction(m_obFacing);
			CDirection currentFacing;
			
			m_pobEnt->GetLookDirection( currentFacing );
			currentFacing.Normalise();
			
			float fFacingDotProduct = m_obFacing.Dot(currentFacing);
			if ( fFacingDotProduct > fAlmostOne )
			{
				//m_pobAIComp->SetPlayingFacingAction(false);
				AI_BEHAVIOUR_SEND_MSG( FACING_ENTITY );
			}
			
		OnExit
			m_pobAIComp->SetPlayingFacingAction(false); 
			m_fTimer = 0.0f;

	// ============= STATE_INITIALISE ==========================
	AIState( STATE_ENTITY_NOT_FOUND )
		OnEnter
			PRINT_STATE(FaceEntity_Dario::STATE_ENTITY_NOT_FOUND);
			
		OnUpdate

		OnExit

	// ============= STATE_WAIT_FOR_CS_STANDARD ==========================
	AIState( STATE_WAIT_FOR_CS_STANDARD )
	OnEnter
		PRINT_STATE(FaceEntity_Dario::STATE_WAIT_FOR_CS_STANDARD)
		m_fTimer = 0.0f;
	
	OnUpdate
		
		// Wait here until AI is in CS_STANDARD
	
		if (IsSaveToPushAController())
			SetState(GetReturnToState());

	OnExit
	
		m_fTimer = 0.0f;

EndStateMachine
}

