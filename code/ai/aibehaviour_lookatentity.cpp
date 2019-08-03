//! -------------------------------------------
//! aibehaviour_lookatentity.cpp
//!
//! Look At Entity behaviour for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

//------------------------------------------------------------------------------------------
// Includes                                                                                 
//------------------------------------------------------------------------------------------

#if 0

#include "ai/aibehaviour_faceentity.h"
#include "game/messagehandler.h"
#include "game/aicomponent.h"
#include "game/entitymanager.h"
#include "game/entityinfo.h"
#include "game/query.h"
#include "game/movementcontrollerinterface.h"

//--------------------------------------------------
//!	
Destructor
//--------------------------------------------------
CAIFaceEntityBehaviour::~CAIFaceEntityBehaviour( void ) 
{
	PRINT_STATE(LookAtEntity_Dario::DESTRUCTOR)
	m_pobAIComp->SetPlayingFacingAction(false);
}

bool CAILookAtEntityBehaviour::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
{

BeginStateMachine

	// ============= STATE_INITIALISE ==========================
	AIState( STATE_INITIALISE )
		OnEnter
			PRINT_STATE(LookAtEntity_Dario::STATE_INITIALISE);
			m_fTimer = 0.0f;

			// Get (and Check) the LookAt component
			m_pLAC = m_pMov->GetParent()->GetLookAtComponent();
			if (!pLAC)
			{
				ntPrintf("CAILookAtEntityBehaviour: No LookAtComponent available for entity : [%s]\n",m_pMov->GetParent()->GetName().c_str());
				AI_BEHAVIOUR_SEND_MSG( FACING_ENTITY );
				SetState(STATE_ENTITY_OR_LAC_NOT_FOUND);
				return true;
			}

			// Get the entity to look at
			m_pobLookAtEntity = m_pMov->GetEntityToLookAt();
						
			if (m_pobLookAtEntity)
			{
				m_obEntityPos = m_pobLookAtEntity->GetPosition();
				m_bEntityFound = true;
				SetState( STATE_TURNTOFACE );
			}
			else
			{
				m_bEntityFound = false;
				user_warn_msg( ("CAILookAtEntityBehaviour: Entity not found, %s\n", m_pobLookAtEntity->GetName().c_str() );
				AI_BEHAVIOUR_SEND_MSG( ENTITY_NOT_FOUND );
				SetState( STATE_ENTITY_OR_LAC_NOT_FOUND );
			}

		OnUpdate
			
		OnExit
			m_fTimer = 0.0f;

	// ============= STATE_INITIALISE ==========================
	AIState( STATE_TURNTOFACE )
		OnEnter
			PRINT_STATE(LookAtEntity_Dario::STATE_TURNTOFACE);
			m_fTimer = 0.0f;
			
			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_TURNTOFACE);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}
			m_pobAIComp->CompleteSimpleAction();

			m_pLAC->LookAt(m_pobLookAtEntity,"root");
			//m_pobAIComp->SetPlayingFacingAction(true); 
			//m_pobAIComp->ActivateController(CAIComponent::MC_WALKING);

		OnUpdate
			m_fTimer += fTimeChange;
			
			m_obEntityPos = m_pobLookAtEntity->GetPosition();
			m_obFacing = CDirection(m_obEntityPos - m_pLAC->);
			m_obFacing.Normalise();
			m_pobAIComp->GetCAIMovement()->SetFacingAction(m_obFacing);
			CDirection currentFacing;
			
			m_pobEnt->GetLookDirection( currentFacing );
			currentFacing.Normalise();
			
			float fFacingDotProduct = m_obFacing.Dot(currentFacing);
			if ( fFacingDotProduct > 0.99f )
			{
				//m_pobAIComp->SetPlayingFacingAction(false);
				AI_BEHAVIOUR_SEND_MSG( FACING_ENTITY );
			}
			
		OnExit
			m_pobAIComp->SetPlayingFacingAction(false); 
			m_fTimer = 0.0f;

	// ============= STATE_ENTITY_OR_LAC_NOT_FOUND ==========================
	AIState( STATE_ENTITY_OR_LAC_NOT_FOUND )
		OnEnter
			PRINT_STATE(LookAtEntity_Dario::STATE_ENTITY_OR_LAC_NOT_FOUND);
			
		OnUpdate

		OnExit

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

EndStateMachine
}

#endif

