//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file aiwalktolocator.cpp
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
#include "objectdatabase/dataobject.h"
#include "objectdatabase/objectdatabase.h"

//#include "aistates.h"
#include "aibehaviourmanager.h"
#include "aibehaviourcontroller.h"
#include "aistatemachine.h"

#include "aiwalktolocator.h"


void
AIWalkToLocator::FindThing( const char* substring, bool& thingFound, CPoint& thingPos )
{
	thingFound = false;

	if (strcmp( substring, "" ) != 0)
	{
		CEntity* pobThing = ObjectDatabase::Get().GetPointerFromName<CEntity*>( substring );

		if (pobThing != NULL)
		{
			thingFound = true;

			// if it was a success, store the entity's location
			thingPos = pobThing->GetPosition();
		}
	}
}

bool
AIWalkToLocator::AtTargetPos( const CPoint& target, const float threshold ) const
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
AIWalkToLocator::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
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
			//ntPrintf("WalkToLocator::STATE_INITIALISE\n");
			m_fTimer = 0.0f;

		OnUpdate
			m_fTimer += fTimeChange;
			if( m_fTimer > fInitTime )
			{
				SetState( STATE_FINDLOCATOR );
			}
			
		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_FINDLOCATOR )
		OnEnter
			//ntPrintf("WalkToLocator::STATE_FINDLOCATOR\n");
			m_fTimer = 0.0f;

			m_pobAIComp->SetAction( ACTION_NONE );
			m_pobAIComp->SetActionMoveSpeed( 0.0f );
			FindThing( m_pobAIComp->GetScriptLocatorName(), m_bLocatorFound, m_obLocatorPos );
			if (m_bLocatorFound)
			{
				SetState( STATE_GOTOLOCATOR );
			}
			else
			{
				user_warn_msg( ("AIWalkToLocator: Locator not found, %s\n", m_pobAIComp->GetScriptLocatorName()) );
				AI_BEHAVIOUR_SEND_MSG( OBJECT_USED );
			}

		OnUpdate
		
		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_GOTOLOCATOR )
		OnEnter
			//ntPrintf("WalkToLocator::STATE_GOTOLOCATOR\n");
			m_fTimer = 0.0f;
			
			float speed = m_pobAIComp->GetScriptRun() ? 1.0f : 0.5f;
			m_pobAIComp->SetAction( ACTION_WALK );
			m_pobAIComp->SetActionMoveSpeed( speed );
			m_pobAIComp->SetActionDest( m_obLocatorPos );

		OnUpdate
			m_fTimer += fTimeChange;

			if (AtTargetPos( m_obLocatorPos, m_pobAIComp->GetScriptLocatorThreshold() ))
			{
                SetState( STATE_FINDFACINGLOCATOR );                
			}
			if (m_pobAIComp->GetPathfindStatus() == CAIComponent::PF_FAILED)
			{
                SetState( STATE_LOCATORUNREACHABLE );                
			}
			
		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_FINDFACINGLOCATOR )
		OnEnter
			//ntPrintf("WalkToLocator::STATE_FINDFACINGLOCATOR\n");
			m_fTimer = 0.0f;

			m_pobAIComp->SetAction( ACTION_NONE );
			m_pobAIComp->SetActionMoveSpeed( 0.0f );
			FindThing( m_pobAIComp->GetScriptFacingLocatorName(), m_bFacingLocatorFound, m_obFacingLocatorPos );
			if (m_bFacingLocatorFound)
			{
				SetState( STATE_TURNTOFACE );
			}
			else
			{
				SetState( STATE_STOPANDNOTIFY );
			}

		OnUpdate
		
		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_TURNTOFACE )
		OnEnter
			//ntPrintf("WalkToLocator::STATE_TURNTOFACE\n");
			m_fTimer = 0.0f;
			
			float speed = m_pobAIComp->GetScriptRun() ? 1.0f : 0.5f;
			speed = 0.0f;
			m_pobAIComp->SetAction( ACTION_STRAFE );
			m_pobAIComp->SetActionMoveSpeed( speed );
			m_obFacing = m_obFacingLocatorPos ^ m_pobEnt->GetPosition();
			m_pobAIComp->SetActionFacing( m_obFacing );
			m_pobAIComp->SetActionDest( m_obFacingLocatorPos );

		OnUpdate
			m_fTimer += fTimeChange;

			//if (AtTargetPos( m_obLocatorPos, m_pobAIComp->GetScriptLocatorThreshold() ))
			m_pobAIComp->SetActionFacing( m_obFacing );
			m_pobAIComp->SetActionDest( m_obFacingLocatorPos );
			CDirection currentFacing;
			m_pobEnt->GetLookDirection( currentFacing );

			if (MovementControllerUtilities::RotationAboutY( currentFacing, m_obFacing) < 0.5f || m_fTimer > 1.0f)
			{
                SetState( STATE_STOPANDNOTIFY );                
			}
			
		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_LOCATORUNREACHABLE )
		OnEnter
			//ntPrintf("WalkToLocator::STATE_LOCATORUNREACHABLE\n");
			m_fTimer = 0.0f;
			m_pobAIComp->SetAction( ACTION_NONE );
			m_pobAIComp->SetActionMoveSpeed( 0.0f );
			AI_BEHAVIOUR_SEND_MSG( LOCATOR_UNREACHABLE );
		OnUpdate
			
		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_STOPANDNOTIFY )
		OnEnter
			//ntPrintf("WalkToLocator::STATE_STOPANDNOTIFY\n");
			m_fTimer = 0.0f;
			m_pobAIComp->SetAction( ACTION_NONE );
			m_pobAIComp->SetActionMoveSpeed( 0.0f );
			AI_BEHAVIOUR_SEND_MSG( LOCATOR_REACHED );
		OnUpdate
			
		OnExit
			m_fTimer = 0.0f;


EndStateMachine
}

