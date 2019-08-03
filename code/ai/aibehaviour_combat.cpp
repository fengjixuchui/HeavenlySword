//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file aibehaviour_combat.cpp
//!                                                                                         
//------------------------------------------------------------------------------------------
                                                                                         
                                                                                         
//------------------------------------------------------------------------------------------
// Includes                                                                                 
//------------------------------------------------------------------------------------------

#include "core/visualdebugger.h"
#include "ai/aibehaviour_combat.h"
#include "game/entitymanager.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "game/aicomponent.h"
#include "game/attacks.h"
#include "game/randmanager.h"

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIBehaviour_Combat::States
//! State Update - This is really just stopping other behaviours from doing anything to
//!                screw up combat.
//!                                                                                         
//------------------------------------------------------------------------------------------
bool AIBehaviour_Combat::States(const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeDelta)
{
	UNUSED( fTimeDelta );

	if(!CEntityManager::Get().GetPlayer())
		return false;
		
	CPoint ptPlayer = CEntityManager::Get().GetPlayer()->GetPosition();
	CPoint ptMyPos = m_pobAIComp->GetParent()->GetPosition();
	COMBAT_STATE eState = m_pobAIComp->GetParent()->GetAttackComponent()->AI_Access_GetState();


BeginStateMachine

	// Update the combat component first
	OnUpdate

	AIState(COMBAT_CHOOSE_POSITION)
		OnEnter
		OnUpdate
			if( (eState == CS_STANDARD) )
			{
				CDirection dirToMe(ptMyPos - ptPlayer);
				dirToMe.Normalise();

				m_fMovementSpeed	= 0.5f + grandf( 0.4f );
				float fAngle		= dirToMe.Dot( CDirection( 0.0f, 0.0f, 1.0f ) );
				fAngle				+= (grandf(PI) - HALF_PI);

				float fRadius = 3.0f;
				NinjaLua::LuaFunctionRet<float> obGetRadius = NinjaLua::LuaFunctionRet<float>( CLuaGlobal::Get().State().GetGlobals()["GavShieldsmanAICombat"]["Radius"] );
				
				if( !obGetRadius.IsNil() )
					fRadius = obGetRadius();
				
				m_obDestPoint = CPoint( ptPlayer.X() + (fRadius * fcosf(fAngle)), 
										ptPlayer.Y(), 
										ptPlayer.Z() + (fRadius * fsinf(fAngle))); 


				SetState(COMBAT_MOVING_TO_POSITION);
			}
		OnExit
	
	AIState(COMBAT_MOVING_TO_POSITION)
		OnEnter
		OnUpdate
			//g_VisualDebug->RenderLine( ptPlayer, m_obDestPoint, 0xFFFFFFFF );

			if( eState != CS_STANDARD )
				SetState(COMBAT_CHOOSE_POSITION);

			if( (ptMyPos - m_obDestPoint).LengthSquared() > (0.5f * 0.5f) )
			{
				m_pobAIComp->SetAction(ACTION_STRAFE);
				m_pobAIComp->SetActionStyle(AS_AGGRESSIVE);
				m_pobAIComp->SetActionFacing(CDirection(ptPlayer - ptMyPos));
				m_pobAIComp->SetActionDest(m_obDestPoint);
				m_pobAIComp->SetActionMoveSpeed(m_fMovementSpeed);
			}
			else
			{
				SetState(COMBAT_READY);
			}

		OnExit

	AIState(COMBAT_READY)
		OnEnter
			m_pobAIComp->SetAction(ACTION_INFORMATION_ANIM);
			m_pobAIComp->SetActionStyle(AS_AGGRESSIVE);
			//m_fCombatReadyDelay = 1.0f + grandf( 2.f );
		OnUpdate

			if( eState != CS_STANDARD )
				SetState(COMBAT_CHOOSE_POSITION);

			m_fCombatReadyDelay -= fTimeDelta;

			if( m_fCombatReadyDelay <= 0.0f )
				SetState(COMBAT_CHOOSE_POSITION);

		OnExit

EndStateMachine
}

