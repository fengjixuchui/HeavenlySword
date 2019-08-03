//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file aibehaviour_lockon.cpp
//!                                                                                         
//------------------------------------------------------------------------------------------
                                                                                         
                                                                                         
//------------------------------------------------------------------------------------------
// Includes                                                                                 
//------------------------------------------------------------------------------------------

#include "ai/aibehaviour_lockon.h"
#include "game/aicomponent.h"
#include "game/messagehandler.h"

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIBehaviour_LockOn::States
//! AIStates Update
//!                                                                                         
//------------------------------------------------------------------------------------------
bool AIBehaviour_LockOn::States(const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeDelta)
{

BeginStateMachine

	// Locked on and in the stance!
	AIState(LOCKON_INITIAL)
		OnEnter

		OnUpdate
			CPoint ptTarg = m_target.GetPosition();
			CPoint ptCurr = m_pobEnt->GetPosition();
			CDirection dir = ptTarg ^ ptCurr;
			float fDistSqrd = dir.LengthSquared();

			m_pobAIComp->SetAction(ACTION_STRAFE);
			m_pobAIComp->SetActionFacing(dir);
			m_pobAIComp->SetActionDest(ptTarg);
			m_pobAIComp->SetActionMoveSpeed(fDistSqrd > m_fRangeSqrd ? 1.0f : 0.0f);

			// In this state we can update the combat component
			m_pobAIComp->UpdateCombatComponent(fTimeDelta,true);

			if(fDistSqrd > 25.0f)
			{
				AI_BEHAVIOUR_SEND_MSG( LOCKON_LOCKOFF );
			}

		OnExit

EndStateMachine

}

