//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file aiidle.cpp
//!                                                                                         
//------------------------------------------------------------------------------------------
                                                                                         
                                                                                         
//------------------------------------------------------------------------------------------
// Includes                                                                                 
//------------------------------------------------------------------------------------------

#include "ai/aiidle.h"
#include "game/aicomponent.h"

bool
AIIdle::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
{
	UNUSED( fTimeChange );

	const float fInitTime			(0.3f);

BeginStateMachine

	/***************************************************************/
	AIState( STATE_INITIALISE )
		OnEnter
			m_fTimer = 0.0f;

		OnUpdate
			m_fTimer += fTimeChange;
			if( m_fTimer > fInitTime )
			{
				SetState( STATE_IDLE );
			}
			
		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_IDLE )
		OnEnter
			m_fTimer = 0.0f;

			m_pobAIComp->SetAction( ACTION_PATROL_LOOK );

		OnUpdate
			//m_fTimer += fTimeChange;
			
		OnExit
			m_fTimer = 0.0f;

EndStateMachine
}

