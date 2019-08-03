//--------------------------------------------------
//!
//!	\file game/entityinteractabletraverser.cpp
//!	Definition of the Interactable Traverser entity object
//!
//--------------------------------------------------

#include "objectdatabase/dataobject.h"
#include "game/luaattrtable.h"
#include "Physics/system.h"
#include "physics/collisionbitfield.h"
#include "messagehandler.h"

#include "interactioncomponent.h"
#include "movement.h"
#include "anim/animator.h"
#include "core/exportstruct_anim.h"
#include "game/combathelper.h"

#include "game/entityinteractabletraverser.h"
#include "game/entitybindings.h"

void ForceLinkFunctionTraverser()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionTraverser() !ATTN!\n");
}


START_CHUNKED_INTERFACE(Interactable_Traverser, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(Interactable)
	COPY_INTERFACE_FROM(Interactable)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_Description, "", Description)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_InitialState, "Active", InitialState)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_AnimationContainer, "", AnimationContainer)
	PUBLISH_VAR_WITH_DEFAULT_AS(	m_InteruptAttack, "atk_collision", InteruptAttack )
	PUBLISH_VAR_AS(	m_TraverserAnimation, TraverseAnimation)
	PUBLISH_VAR_AS(	m_UserAnimation, UserAnimation )
	PUBLISH_VAR_AS(	m_ResetAnimation, ResetAnimation )
	PUBLISH_VAR_AS(	m_ActivateAnimation, ActivateAnimation )
	PUBLISH_VAR_AS(	m_DeactivateAnimation, DeactivateAnimation )
	PUBLISH_VAR_AS(	m_InteruptAnimation, InteruptAnimation )
	PUBLISH_VAR_AS( m_bPairUsePoints, UseInteractionPoints )

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )
END_STD_INTERFACE


//--------------------------------------------------
//!
//! Interactable Traverser State Machine
//!
//--------------------------------------------------
STATEMACHINE(INTERACTABLE_TRAVERSER_FSM, Interactable_Traverser)
	INTERACTABLE_TRAVERSER_FSM(bool bActive)
	{
		if(bActive)
			SET_INITIAL_STATE(ACTIVE);
		else
			SET_INITIAL_STATE(DEACTIVE);
	}

	STATE(DEACTIVE)
		BEGIN_EVENTS
			ON_ENTER
				ME->m_pOther = 0;

				ME->GetInteractionComponent()->Lua_SetInteractionPriority(USE);
				//ME->GetPhysicsSystem()->Lua_Rigid_SetController(0, "");
			END_EVENT(true)

			EVENT(msg_running_action)
			EVENT(msg_action)
			{
				ME->m_pOther = (Character*)msg.GetEnt("Other");
				ntError(!ME->m_pOther || ME->m_pOther->IsCharacter());

				// player won't activate a traverser
				if ( ! ME->m_pOther->IsPlayer() )
					SET_STATE(ACTIVATE);
			
				Message msgExitState(msg_exitstate);
				msgExitState.SetEnt( CHashedString(HASH_STRING_SENDER), ME);
				ME->m_pOther->GetMessageHandler()->QueueMessage(msgExitState);
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(ACTIVE)
		BEGIN_EVENTS
			ON_ENTER
				ME->m_pOther = 0;

				ME->GetInteractionComponent()->Lua_SetInteractionPriority(USE);
				//ME->GetPhysicsSystem()->Lua_Rigid_SetController(0, "");
			END_EVENT(true)

			EVENT(msg_running_action)
			EVENT(msg_action)
			{
				ME->m_pOther = (Character*)msg.GetEnt("Other");
				ntError(!ME->m_pOther || ME->m_pOther->IsCharacter());
				
				CPoint obPosition( ME->m_pOther->GetHierarchy()->GetRootTransform()->GetWorldMatrix().GetTranslation() );
				
				if ( ME->IsNamedUsePoint( obPosition, CHashedString("UsePoint_Interrupt") ) )
					SET_STATE(DEACTIVATE);
				else 
					SET_STATE(TRAVERSE);
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(ACTIVATE)
		BEGIN_EVENTS
			ON_ENTER
				ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);

				if ( !ntStr::IsNull(ME->m_ActivateAnimation) )
				{
					ME->Lua_AnimMessageOnCompletion( ME->m_ActivateAnimation );
					ME->Lua_AnimPlay( ME->m_ActivateAnimation, 1.0f, true, false);
				}	
				else
				{
					SET_STATE(ACTIVE);
				}
			END_EVENT(true)

			EVENT(msg_animdone)
			{
				Message msgExitState(msg_exitstate);
				msgExitState.SetEnt( CHashedString(HASH_STRING_SENDER), ME);
				ME->m_pOther->GetMessageHandler()->QueueMessage(msgExitState);
				SET_STATE(ACTIVE);
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(DEACTIVATE)
		BEGIN_EVENTS
			ON_ENTER
				ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);

				if (! ME->m_obUsers.empty() )
				{
					for ( ntstd::List<CEntity*>::iterator obIt=ME->m_obUsers.begin(); obIt!= ME->m_obUsers.end(); obIt++ )
					{
						Message msgExitState(msg_exitstate);
						msgExitState.SetEnt( CHashedString(HASH_STRING_SENDER), ME);
						(*obIt)->GetMessageHandler()->QueueMessage(msgExitState);

						(*obIt)->GetMovement()->ClearControllers();
	
						CombatHelper::Combat_GenerateStrike(ME, ME->m_pOther, (*obIt), ME->m_InteruptAttack);
					}

					ME->m_obUsers.clear();
				}

				if ( !ntStr::IsNull(ME->m_DeactivateAnimation) )
				{
					ME->Lua_AnimMessageOnCompletion( ME->m_DeactivateAnimation );
					ME->Lua_AnimPlay( ME->m_DeactivateAnimation, 1.0f, true, false);

					if ( !ntStr::IsNull(ME->m_InteruptAnimation) )
					{
						ME->m_pOther->GetMovement()->Lua_AltStartFacingMovement( ME->m_InteruptAnimation, 0.0f, 1.0f, 0.0f, 0.0f );
						ME->m_pOther->GetMovement()->Lua_AltSetMovementCompleteMessage( "msg_movementdone", ME );
					}
				}	
				else
				{
					SET_STATE(DEACTIVE);
				}
			END_EVENT(true)


			EVENT(msg_movementdone)
			{
				Message msgExitState(msg_exitstate);
				msgExitState.SetEnt( CHashedString(HASH_STRING_SENDER), ME);
				ME->m_pOther->GetMessageHandler()->QueueMessage(msgExitState);
				SET_STATE(DEACTIVE);
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(TRAVERSE)
		BEGIN_EVENTS
			ON_ENTER
				ME->GetInteractionComponent()->Lua_SetInteractionPriority( NONE );
				ME->m_obUsers.push_back( ME->m_pOther );

				// Must have user animation
				if ( !ntStr::IsNull(ME->m_UserAnimation) )
				{
					// Movement must be soft, or they don't change position after Relative Movement
					ME->m_pOther->GetMovement()->Lua_StartRelativeMovement( ME, ME->m_UserAnimation, false, true );
					ME->m_pOther->GetMovement()->Lua_AltSetMovementCompleteMessage( "msg_movementdone", ME->m_pOther );
					ME->GetInteractionComponent()->ExcludeCollisionWith( ME->m_pOther );
					ME->m_pOther->GetPhysicsSystem()->Lua_Character_SetCollidable( false );
					

					if ( !ntStr::IsNull(ME->m_TraverserAnimation) )
					{
						ME->Lua_AnimMessageOnCompletion( ME->m_TraverserAnimation );
						ME->Lua_AnimPlay( ME->m_TraverserAnimation, 1.0f, true, false);
					}	
				}
			END_EVENT(true)

			EVENT(msg_animdone)
				ME->GetInteractionComponent()->AllowCollisionWith( ME->m_pOther );
				ME->m_pOther->GetPhysicsSystem()->Lua_Character_SetCollidable( true );
			END_EVENT(true)

			EVENT(msg_interrupt)
			EVENT(msg_movementdone)
			{
				CEntity* pobEnt = ME->m_obUsers.front();

				Message msgExitState(msg_exitstate);
				msgExitState.SetEnt( CHashedString(HASH_STRING_SENDER), ME);
				pobEnt->GetMessageHandler()->QueueMessage(msgExitState);
				//ME->m_obUsers.remove( ME->m_pOther );
				ME->m_obUsers.pop_front();
				SET_STATE(RESET);
			}
			END_EVENT(true)

			/*
			// No longer in use
			EVENT(msg_running_action)
			EVENT(msg_action)
			{
				ME->m_pOther = msg.GetEnt("Other");
					
				CPoint obPosition( ME->m_pOther->GetHierarchy()->GetRootTransform()->GetWorldMatrix().GetTranslation() );
				
				if ( ME->IsNamedUsePoint( obPosition, CHashedString("UsePoint_Interrupt") ) )
					SET_STATE(DEACTIVATE);
				else
				{
					Message msgExitState(msg_exitstate);
					msgExitState.SetEnt( CHashedString(HASH_STRING_SENDER), ME);
					ME->m_pOther->GetMessageHandler()->QueueMessage(msgExitState);
				}
			}
			END_EVENT(true)*/
		END_EVENTS
	END_STATE

	STATE(RESET)
		BEGIN_EVENTS
			ON_ENTER
				if ( !ntStr::IsNull(ME->m_ResetAnimation) )
				{
					ME->Lua_AnimMessageOnCompletion( ME->m_ResetAnimation );
					ME->Lua_AnimPlay( ME->m_ResetAnimation, 1.0f, true, false);
				}	
				else
				{
					SET_STATE(ACTIVE);
				}
			END_EVENT(true)

			EVENT(msg_animdone)
				SET_STATE(ACTIVE);
			END_EVENT(true)
		END_EVENTS
	END_STATE

END_STATEMACHINE


//--------------------------------------------------
//!
//!	Interactable_Traverser::Interactable_Traverser()
//!	Default constructor
//!
//--------------------------------------------------
Interactable_Traverser::Interactable_Traverser()
{
	m_pSharedAttributesPtr = 0;
	m_pSharedAttributes = 0;

	m_eType = EntType_Interactable;
	m_eInteractableType = EntTypeInteractable_Traverser;
}

//--------------------------------------------------
//!
//!	Interactable_Traverser::OnPostConstruct()
//!	Post Construct
//!
//--------------------------------------------------
void Interactable_Traverser::OnPostConstruct()
{
	Interactable::OnPostConstruct();
	
	// Create components
	InstallMessageHandler();
	InstallDynamics();

		// requires a hierarchy
	if( !GetHierarchy() )
		InstallHierarchy();
	if ( ! m_AnimationContainer.IsNull() )
		InstallAnimator(m_AnimationContainer);

	if(!GetMovement()) // Create a movement system if one doesn't already exist
	{
		CMovement* pobMovement = NT_NEW_CHUNK(Mem::MC_ENTITY) CMovement(this, GetAnimator(), GetPhysicsSystem());
		pobMovement->ClearControllers();
		pobMovement->SetEnabled(false);
		SetMovement(pobMovement);
	}
}

//--------------------------------------------------
//!
//!	Interactable_Traverser::OnLevelStart()
//!	Called for each ent on level startup
//!
//--------------------------------------------------
void Interactable_Traverser::OnLevelStart()
{
	// Create and attach the statemachine. Must be done AFTER anim containers fixed up by area system
	// i.e. after XML serialisation. OR this shouldnt play an animation

	bool bActive = false;

	// State machine initial state
	if (m_InitialState == "Active")
		bActive = true;

	if ( bActive )
	{
		if ( m_ActivateAnimation != "" )
		{
			CAnimator* pobAnimator = GetAnimator();	
			CAnimationPtr obNewAnim = pobAnimator->CreateAnimation( m_ActivateAnimation );
			obNewAnim->SetFlagBits( ANIMF_LOCOMOTING );
			obNewAnim->SetTime(obNewAnim->GetDuration());
			pobAnimator->AddAnimation( obNewAnim );
		}
	}
	else
	{
		if ( m_DeactivateAnimation != "" )
		{
			CAnimator* pobAnimator = GetAnimator();	
			CAnimationPtr obNewAnim = pobAnimator->CreateAnimation( m_DeactivateAnimation );
			obNewAnim->SetFlagBits( ANIMF_LOCOMOTING );
			obNewAnim->SetTime(obNewAnim->GetDuration());
			pobAnimator->AddAnimation( obNewAnim );
		}
	}

	INTERACTABLE_TRAVERSER_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) INTERACTABLE_TRAVERSER_FSM(bActive);
	ATTACH_FSM(pFSM);
}

//--------------------------------------------------
//!
//!	Interactable_Traverser::~Interactable_Traverser()
//!	Default destructor
//!
//--------------------------------------------------
Interactable_Traverser::~Interactable_Traverser()
{
}

bool Interactable_Traverser::IsNamedUsePoint(const CPoint& pobPos, const CHashedString obUseName)
{
	CUsePoint* pobUsePoint = GetInteractionComponent()->GetClosestUsePoint(pobPos);

	return (pobUsePoint->GetName() == obUseName);
}
