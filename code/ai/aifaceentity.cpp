//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file aifaceentity.cpp
//!                                                                                         
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Includes                                                                                 
//------------------------------------------------------------------------------------------

#include "game/messagehandler.h"
#include "game/query.h"
#include "game/aicomponent.h"
#include "game/entitymanager.h"
#include "game/entityinfo.h"
#include "game/movementcontrollerinterface.h"

//#include "aistates.h"
#include "aibehaviourmanager.h"
#include "aibehaviourcontroller.h"
#include "aistatemachine.h"

#include "aifaceentity.h"


void
AIFaceEntity::FindThing( const char* substring, bool& thingFound, CPoint& thingPos )
{
	thingFound = false;

	if (strcmp( substring, "" ) != 0)
	{
		CEntityQuery obQuery;
		CEQCIsSubStringInName	obNameClause( substring );
		obQuery.AddClause( obNameClause );
		CEntityManager::Get().FindEntitiesByType( obQuery, CEntity::EntType_AllButStatic );

		ntstd::List<CEntity*>::iterator obEnd = obQuery.GetResults().end();
		for ( ntstd::List<CEntity*>::iterator obIt = obQuery.GetResults().begin(); obIt != obEnd; ++obIt )
		{
			thingFound = true;

			// if it was a success, store the entity's location
			m_pobFacingEntity = (*obIt);
			thingPos = (*obIt)->GetPosition();

			break;
		}
	}
}

bool
AIFaceEntity::AtTargetPos( const CPoint& target, const float threshold ) const
{
	CPoint obPos( m_pobEnt->GetPosition() );

	// XXX: make it 2d for now, but for distance check we really need the waypoint Y to
	// be the same as the AI's y when stood at that point on the map
	CPoint obDiff = obPos - target;
	obDiff.Y() = 0.0f;

	if (obDiff.LengthSquared() < (threshold * threshold))
	{
		return true;
	}

	return false;
}


bool
AIFaceEntity::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
{
	const float fInitTime			(0.0f);
	if (m_pobAIComp->CanSeePlayer())
	{
		AI_BEHAVIOUR_SEND_MSG( PATROL_SEENENEMY );
	}

BeginStateMachine

	/***************************************************************/
	AIState( STATE_INITIALISE )
		OnEnter
			ntPrintf("FaceEntity::STATE_INITIALISE\n");
			m_fTimer = 0.0f;

		OnUpdate
			m_fTimer += fTimeChange;
			if( m_fTimer > fInitTime )
			{
				SetState( STATE_FINDENTITY );
			}
			
		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_FINDENTITY )
		OnEnter
			ntPrintf("FaceEntity::STATE_FINDENTITY\n");
			m_fTimer = 0.0f;

			m_pobAIComp->SetAction( ACTION_NONE );
			m_pobAIComp->SetActionMoveSpeed( 0.0f );
			FindThing( m_pobAIComp->GetScriptFacingEntityName(), m_bEntityFound, m_obEntityPos );
			if (m_bEntityFound)
			{
				SetState( STATE_TURNTOFACE );
			}
			else
			{
				user_warn_msg( ("AIFaceEntity: Entity not found, %s\n", m_pobAIComp->GetScriptLocatorName()) );
				AI_BEHAVIOUR_SEND_MSG( OBJECT_USED );
			}

		OnUpdate
		
		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_TURNTOFACE )
		OnEnter
			ntPrintf("FaceEntity::STATE_TURNTOFACE\n");
			m_fTimer = 0.0f;
			
			float speed = m_pobAIComp->GetScriptRun() ? 1.0f : 0.5f;
			speed = 0.0f;
			m_pobAIComp->SetAction( ACTION_STRAFE );
			m_pobAIComp->SetActionMoveSpeed( speed );

		OnUpdate
			m_fTimer += fTimeChange;
			
			m_obEntityPos = m_pobFacingEntity->GetPosition();
			m_obFacing = m_obEntityPos ^ m_pobEnt->GetPosition();
			m_pobAIComp->SetActionFacing( m_obFacing );
			m_pobAIComp->SetActionDest( m_obEntityPos );
			CDirection currentFacing;
			m_pobEnt->GetLookDirection( currentFacing );

			if (MovementControllerUtilities::RotationAboutY( currentFacing, m_obFacing) < 0.05f)
			{
				AI_BEHAVIOUR_SEND_MSG( FACING_ENTITY );
			}
			
		OnExit
			m_fTimer = 0.0f;

EndStateMachine
}

