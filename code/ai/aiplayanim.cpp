//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file aiplayanim.cpp
//!                                                                                         
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Includes                                                                                 
//------------------------------------------------------------------------------------------

#include "game/messagehandler.h"
#include "game/query.h"
#include "game/aicomponent.h"

//#include "aistates.h"
#include "aibehaviourmanager.h"
#include "aibehaviourcontroller.h"
#include "aistatemachine.h"

#include "aiplayanim.h"


bool
AIPlayAnim::States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange )
{
	const float fInitTime			(0.0f);
/*
	if (m_pobAIComp->CanSeePlayer())
	{
		AI_BEHAVIOUR_SEND_MSG( PATROL_SEENENEMY );
	}
*/

BeginStateMachine

	/***************************************************************/
	AIState( STATE_INITIALISE )
		OnEnter
			//ntPrintf("PlayAnim::STATE_INITIALISE\n");
			m_fTimer = 0.0f;

		OnUpdate
			m_fTimer += fTimeChange;
			if( m_fTimer > fInitTime )
			{
				SetState( STATE_PLAYANIM );
			}
			
		OnExit
			m_fTimer = 0.0f;

	/***************************************************************/
	AIState( STATE_PLAYANIM )
		OnEnter
			//ntPrintf("PlayAnim::STATE_PLAYANIM\n");
			m_fTimer = 0.0f;

			m_pobAIComp->CompleteSimpleAction();
			m_pobAIComp->SetAction( ACTION_SCRIPTANIM );

		OnUpdate
			m_fTimer += fTimeChange;

			if( !m_pobAIComp->IsSimpleActionComplete() )
			{
				AI_BEHAVIOUR_SEND_MSG( ANIM_STARTED );
				SetState( STATE_PLAYINGANIM );
			}

			// If the animation has taken longer than x to start then there is something wrong... 
			if( m_fTimer > 0.5f )
			{
 				AI_BEHAVIOUR_SEND_MSG( ANIM_FAILED );
				SetState( STATE_COMPLETED );
				m_pobAIComp->CompleteSimpleAction();
			}
			
		OnExit
			m_fTimer = 0;

	/***************************************************************/
	AIState( STATE_PLAYINGANIM )
		OnEnter
			//ntPrintf("PlayAnim::STATE_PLAYINGANIM\n");
			m_fTimer = 0.0f;

		OnUpdate
			m_fTimer += fTimeChange;

			if( m_pobAIComp->IsSimpleActionComplete() )
			{
				SetState( STATE_STOPANDNOTIFY );
			}
			
		OnExit
			m_fTimer = 0;

	/***************************************************************/
	AIState( STATE_STOPANDNOTIFY )
		OnEnter
			//ntPrintf("\nPlayAnim::STATE_STOPANDNOTIFY\n");
			m_fTimer = 0.0f;
			if ( m_pobAIComp->GetScriptAnimLooping() )
			{
				SetState( STATE_PLAYANIM );
			}
			else
			{
				m_pobAIComp->SetAction( ACTION_NONE );
				m_pobAIComp->SetActionMoveSpeed( 0.0f );
			}

		OnUpdate
			/*
			if (m_pobEnt)
			{
				ntPrintf( "sending anim_complete, %s\n", m_pobEnt->GetName().c_str() );
			}
			else
			{
				ntPrintf( "OH SHIT! sending anim_complete without an entity!\n" );
			}
			*/
			AI_BEHAVIOUR_SEND_MSG( ANIM_COMPLETE );
			SetState( STATE_COMPLETED );
			
		OnExit
			m_fTimer = 0.0f;


	/***************************************************************/
	AIState( STATE_COMPLETED )
		OnEnter
			//ntPrintf("PlayAnim::STATE_COMPLETED\n");

		OnUpdate
			
		OnExit
		


EndStateMachine
}

