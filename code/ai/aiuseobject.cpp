//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file aiuseobject.cpp
//!                                                                                         
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Includes                                                                                 
//------------------------------------------------------------------------------------------

#include "game/messagehandler.h"
#include "game/query.h"
#include "game/aicomponent.h"
#include "game/entitymanager.h"
#include "game/interactioncomponent.h"

#include "editable/enumlist.h"

//#include "aistates.h"
#include "aibehaviourmanager.h"
#include "aibehaviourcontroller.h"
#include "aistatemachine.h"

#include "aiuseobject.h"


void
AIUseObject::FindThing( const char* substring, bool& thingFound, CPoint& thingPos )
{
	// setup entity query to match preferred weapon type, in this test case a bazooka
	CEntityQuery obQuery;
	CEQCIsSubStringInName	obNameClause( substring );
	obQuery.AddClause( obNameClause );
	CEntityManager::Get().FindEntitiesByType( obQuery, CEntity::EntType_AllButStatic );

	// set the weapon found flag based on the query's success XXX: doesn't check to see if the weapon is already held
	thingFound = false;
	ntstd::List<CEntity*>::iterator obEnd = obQuery.GetResults().end();
	for ( ntstd::List<CEntity*>::iterator obIt = obQuery.GetResults().begin(); obIt != obEnd; ++obIt )
	{
		thingFound = true;

		// if it was a success, store the entity's location
		thingPos = (*obIt)->GetPosition();

		break;
	}

	if (thingFound)
	{
		ntPrintf( "USEOBJECT: found object" );
	}
	else
	{
		ntPrintf( "USEOBJECT: failed to find object" );
	}
	ntPrintf( " %s\n", substring );
	
}


void
AIUseObject::UseObject()
{
	CEntity* pobEntity=CEntityManager::Get().FindEntity(m_pobAIComp->GetScriptObjectName());

	//ntPrintf( "AIUseObject::UseObject, entity = %x\n", pobEntity );

	if (pobEntity && pobEntity->GetInteractionComponent()) // && pobEntity->GetInteractionComponent()->GetInteractionPriority()!=NONE)
	{
		Message obMessage(msg_interact);
		obMessage.SetEnt("target",pobEntity);

		m_pobEnt->GetMessageHandler()->Receive(obMessage);
	}
	else
	{
		//CMessageSender::SendEmptyMessage( "OBJECT_UNREACHABLE", m_pobEnt->GetMessageHandler() );
		AI_BEHAVIOUR_SEND_MSG( OBJECT_UNREACHABLE );
	}
}


bool
AIUseObject::AtTargetPos( const CPoint& target, const float threshold ) const
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
AIUseObject::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
{
	UNUSED( fTimeChange );

	const float fInitTime			(0.0f);
	const float fPauseTime			(0.8f);
	const float fUseTime			(1.0f);

BeginStateMachine

	/***************************************************************/
	AIState( STATE_INITIALISE )
		OnEnter
			m_fTimer = 0.0f;

		OnUpdate
			m_fTimer += fTimeChange;
			if( m_fTimer > fInitTime )
			{
				SetState( STATE_FINDOBJECT );
			}
			
		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_FINDOBJECT )
		OnEnter
			m_fTimer = 0.0f;

			m_pobAIComp->SetAction( ACTION_NONE );
			m_pobAIComp->SetActionMoveSpeed( 0.0f );
			FindThing( m_pobAIComp->GetScriptObjectName(), m_bObjectFound, m_obObjectPos );

		OnUpdate
			m_fTimer += fTimeChange;
			if (m_fTimer > fPauseTime)
			{
				if (m_bObjectFound)
				{
					SetState( STATE_GOTOOBJECT );
				}
				else
				{
					user_warn_msg( ("AIUseObject: Object not found, %s\n", m_pobAIComp->GetScriptObjectName()) );
					AI_BEHAVIOUR_SEND_MSG( OBJECT_USED );
				}
			}
			
		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_GOTOOBJECT )
		OnEnter
			m_fTimer = 0.0f;

			float speed = m_pobAIComp->GetScriptRun() ? 1.0f : 0.5f;
			m_pobAIComp->SetAction( ACTION_WALK );
			m_pobAIComp->SetActionMoveSpeed( speed );
			m_pobAIComp->SetActionDest( m_obObjectPos );

		OnUpdate
			m_fTimer += fTimeChange;

			// if we've reached the object
			// then go to the use state
			if (AtTargetPos( m_obObjectPos, 2.5f ))
			{
                SetState( STATE_USEOBJECT );                
			}
			if (m_pobAIComp->GetPathfindStatus() == CAIComponent::PF_FAILED)
			{
				SetState( STATE_USEOBJECT );
                //SetState( STATE_OBJECTUNREACHABLE );                
			}
		
		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_USEOBJECT )
		OnEnter
			m_fTimer = 0.0f;
			m_bObjectUsed = false;

			// stand still
			m_pobAIComp->SetAction( ACTION_NONE );
			m_pobAIComp->SetActionMoveSpeed( 0.0f );

		OnUpdate
			m_fTimer += fTimeChange;

			if( !m_bObjectUsed && m_fTimer > fPauseTime )
			{
				// try to use the object
				UseObject();
				m_bObjectUsed = true;
				AI_BEHAVIOUR_SEND_MSG( USING_OBJECT );
			}
			if(m_fTimer > fPauseTime + fUseTime)
			{
				SetState( STATE_STOPANDNOTIFY );
			}

		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_OBJECTUNREACHABLE )
		OnEnter
			ntPrintf("AIUseObject::STATE_OBJECTUNREACHABLE\n");
			m_fTimer = 0.0f;
			m_pobAIComp->SetAction( ACTION_NONE );
			m_pobAIComp->SetActionMoveSpeed( 0.0f );
			AI_BEHAVIOUR_SEND_MSG( OBJECT_UNREACHABLE );
		OnUpdate
			
		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_STOPANDNOTIFY )
		OnEnter
			m_fTimer = 0.0f;
			m_pobAIComp->SetAction( ACTION_NONE );
			m_pobAIComp->SetActionMoveSpeed( 0.0f );
			AI_BEHAVIOUR_SEND_MSG( OBJECT_USED );

		OnUpdate
			
		OnExit
			m_fTimer = 0.0f;


EndStateMachine
}

