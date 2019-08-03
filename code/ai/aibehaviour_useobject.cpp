//! -------------------------------------------
//! aibehaviour_useobject.cpp
//!
//! USE OBJECT behaviour for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#include "ai/aibehaviour_useobject.h"
#include "ai/ainavigationsystem/ainavigsystemman.h"
#include "ai/aiuseobjectqueueman.h"
#include "game/aicomponent.h"
#include "game/messagehandler.h"
#include "game/entitymanager.h"
#include "game/interactioncomponent.h"
#include "game/entityprojectile.h"
#include "physics/projectilelg.h"
#include "game/entityinteractableturretweapon.h"
#include "objectdatabase/dataobject.h"
#include "objectdatabase/objectdatabase.h"

CAIUseObjectBehaviour::~CAIUseObjectBehaviour()
{
	if (m_pobAIComp && m_pMov)
	{
		m_pMov->SetUsingCannon(false);
	}
}
//---------------------
// States                                                                                 
//---------------------
bool CAIUseObjectBehaviour::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
{
//  static const float fMaxUseObjectTime = 15.0f;
	static const float fMaxWaitForAvailability = 30.0f;

BeginStateMachine

	// ============= STATE_INITIALISE ==========================
	AIState( STATE_INITIALISE )
		OnEnter
			PRINT_STATE(UseObject_Dario::STATE_INITIALISE);
			m_fTimer = 0.0f;
			
			m_pobAIComp->CompleteSimpleAction();
			m_pobAIComp->GetCAIMovement()->DeactivateMotion();

			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_INITIALISE);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}

			// Check that the selected object exists
			const char* psObjectName = m_pobAIComp->GetScriptObjectName();
			m_pobEntityToUse = CEntityManager::Get().FindEntity(psObjectName);
			if (!m_pobEntityToUse)
			{
				user_warn_msg( ("CAIUseObjectBehaviour: Object not found, %s\n",psObjectName ) );
				AI_BEHAVIOUR_SEND_MSG( OBJECT_NOT_FOUND );
				SetState(STATE_IDLE);
				return false;
			}
			else
			{
				// Is an interactable object?
				if (!m_pobEntityToUse->IsInteractable())
				{
					user_warn_msg( ("CAIUseObjectBehaviour: Object %s is not interactable!\n",psObjectName ) );
					AI_BEHAVIOUR_SEND_MSG( OBJECT_TO_USE_NOT_INTERACTABLE );
					SetState(STATE_IDLE);
					return false;
				}
			
				m_bUseACannon = ((Interactable*)m_pobEntityToUse)->GetInteractableType()==Interactable::EntTypeInteractable_TurretWeapon;
				
                SetState(STATE_WAIT_FOR_OBJECT_AVAILABLE);
			}

		OnUpdate
			
		OnExit
			m_fTimer = 0.0f;

	// ============= STATE_MOVING ==========================
	AIState( STATE_WAIT_FOR_OBJECT_AVAILABLE )
		OnEnter
			PRINT_STATE(UseObject_Dario::STATE_WAIT_FOR_OBJECT_AVAILABLE);
			m_fTimer = 0.0f;

		OnUpdate
			m_fTimer += fTimeChange;
			
			// Is the object available ?
			if (m_pobEntityToUse->GetInteractionComponent()->GetInteractionPriority() == USE)
			{
				if (!IsSaveToPushAController())
				{
					SetReturnToState(STATE_WAIT_FOR_OBJECT_AVAILABLE);
					SetState(STATE_WAIT_FOR_CS_STANDARD);
					return true;
				}
				
				// Send Request to use object
				Message obMessage(msg_interact);
				obMessage.SetEnt("target",m_pobEntityToUse);
				m_pobEnt->GetMessageHandler()->Receive(obMessage);
	
				AI_BEHAVIOUR_SEND_MSG( USING_OBJECT );
				PRINT_STATE(UseObject_Dario::STATE_USING_OBJECT - OK);
				if (m_bUseACannon)
				{
					// Wait some frames to ensure that the interaction target is updated
					SetUserTimer(0.5f);
					SetReturnToState(STATE_USING_CANNON);
					SetState(STATE_WAIT_FOR_TIMER);
				}
				else
				{
					SetState(STATE_USING_OBJECT);
				}
				
				return true;
			}

			if(m_fTimer > fMaxWaitForAvailability)
			{
				AI_BEHAVIOUR_SEND_MSG( OBJECT_WAIT_AVAILABLE_TIMEOUT );
				PRINT_STATE(UseObject_Dario::STATE_WAIT_FOR_OBJECT_AVAILABLE - OBJECT_WAIT_AVAILABLE_TIMEOUT);
				SetState( STATE_IDLE );
			}

		OnExit
			m_fTimer = 0.0f;

	// ============= STATE_MOVING ==========================
	AIState( STATE_USING_OBJECT )
		OnEnter
			PRINT_STATE(UseObject_Dario::STATE_USING_OBJECT);
			m_fTimer = 0.0f;

		OnUpdate
			//m_fTimer += fTimeChange;
			
			// Check if the AI has finished using the object
			CEntity* pIntTarget = ((Character*)m_pobEnt)->GetInteractionTarget();
			CEntity* pRangedWeapon = ((Character*)m_pobEnt)->GetRangedWeapon();
			if ( pIntTarget == NULL || pIntTarget == pRangedWeapon )
			{
				// These were copied from AI follow path cover. 
				m_pMov->SetEntityToUseInPath(0);
				m_pMov->SetUsingObjectInPath(false);
				CAIQueueManager::Get().ReportObjectUsed(m_pobEnt->ToAI());


				AI_BEHAVIOUR_SEND_MSG( OBJECT_USED );
				SetState( STATE_IDLE );
				return true;
			}
			
			//if(m_fTimer > fMaxUseObjectTime)
			//{
			//	AI_BEHAVIOUR_SEND_MSG( OBJECT_USING_TIMEOUT );
			//	PRINT_STATE(UseObject_Dario::STATE_USING_OBJECT - OBJECT_USING_TIME_OUT);
			//	SetState( STATE_IDLE );
			//}

		OnExit
			m_fTimer = 0.0f;

		// ============= STATE_MOVING ==========================
	AIState( STATE_USING_CANNON )
		OnEnter
			PRINT_STATE(UseObject_Dario::STATE_USING_CANNON);
			m_fTimer = 0.0f;
			m_pMov->SetUsingCannon(true);
			m_pMov->DeactivateMotion();

			// Read and Store the CannonBall Parameters
			CEntity* pIntTarget = ((Character*)m_pobEnt)->GetInteractionTarget();
			Interactable_TurretWeapon* pCannon = (Interactable_TurretWeapon*)pIntTarget;
			Projectile_Attributes* pProyAttr = pCannon->GetProjectileAttribs();
			ProjectileProperties* pobProperties = ObjectDatabase::Get().GetPointerFromName<ProjectileProperties*>(pProyAttr->m_Properties);
			
			m_pMov->SetCannonBallParams(pobProperties->m_fInitialSpeed, fabsf(pobProperties->m_fGravity));

		OnUpdate

			CPoint obTgtPos(CONSTRUCT_CLEAR);

			const CEntity* pTarget = m_pMov->GetCannonTarget();
			//if (!pTarget)
			//	obTgtPos = m_pMov->GetCannonTargetLocatorPos();
			//else
			//	obTgtPos = pTarget->GetPosition();

			if (!pTarget)
			{
				CEntity* pIntTarget = ((Character*)m_pobEnt)->GetInteractionTarget();
				user_warn_p(0,("AI: [%s] is using cannon [%s], but was NOT given a TARGET.\nA LUA script is missing something like this.AI.SetCannonTarget(target)", ntStr::GetString(m_pobEnt->GetName()), ntStr::GetString(pIntTarget->GetName())));
				UNUSED(pIntTarget);
				return false;
			}

			obTgtPos = pTarget->GetPosition();
			CDirection dir = CDirection(obTgtPos - m_pMov->GetPosition());

			m_pMov->SetFacingAction(dir);
			
			if (m_pMov->HasShootCannonRequest() && m_pMov->IsCannonFacingTarget())
			{
				Message obMessage(msg_action_on);
				m_pobEntityToUse->GetMessageHandler()->Receive( msg_action_on );
				m_pMov->ClearShootCannonRequest();
			}

		OnExit
			m_fTimer = 0.0f;
			m_pMov->SetUsingCannon(false);

	// ============= STATE_IDLE ==========================
	AIState( STATE_IDLE )
		OnEnter
			PRINT_STATE(UseObject_Dario::STATE_IDLE);

			m_pobAIComp->CompleteSimpleAction();
			m_pobAIComp->GetCAIMovement()->DeactivateMotion();

			if (!IsSaveToPushAController())
			{
				SetReturnToState(STATE_IDLE);
				SetState(STATE_WAIT_FOR_CS_STANDARD);
				return true;
			}

			m_pobAIComp->ActivateController(CAIComponent::MC_WALKING);
			
		OnUpdate

		OnExit

	// ============= STATE_WAIT_FOR_CS_STANDARD ==========================
	AIState( STATE_WAIT_FOR_CS_STANDARD )
	OnEnter
		PRINT_STATE(UseObject_Dario::STATE_WAIT_FOR_CS_STANDARD)
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

