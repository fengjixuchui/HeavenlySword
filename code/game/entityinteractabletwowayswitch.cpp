//--------------------------------------------------
//!
//!	\file game/enitityinteractabletwowayswitch.cpp
//!	Definition of the Interactable Two way switch object
//!
//--------------------------------------------------

#include "objectdatabase/dataobject.h"
#include "game/luaattrtable.h"

#include "messagehandler.h"
#include "game/movement.h"
#include "camera/camman.h"
#include "camera/camview.h"

#include "anim/animator.h"
#include "core/exportstruct_anim.h"

#include "game/entityinteractabletwowayswitch.h"

// Components needed
#include "game/interactioncomponent.h"

void ForceLinkFunctionTwoWaySwitch()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionTwoWaySwitch() !ATTN!\n");
}

START_CHUNKED_INTERFACE(Interactable_TwoWaySwitch, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(Interactable_Switch_Trigger)
	COPY_INTERFACE_FROM(Interactable_Switch_Trigger)

	OVERRIDE_DEFAULT(Clump, "Switches\\lever.clump")

	PUBLISH_VAR_WITH_DEFAULT_AS(m_bActive, true, Active)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bPositionA, true, InitialPositionA)	
	PUBLISH_VAR_WITH_DEFAULT_AS(m_AnimationContainer, "LeverAnimContainer", AnimationContainer)
	PUBLISH_PTR_WITH_DEFAULT_AS(m_pSharedAttributesPtr, SharedAttributes, Att_TwoWaySwitch_Default)

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )	
END_STD_INTERFACE


//--------------------------------------------------
//!
//! Interactable Two Way Switch State Machine
//!
//--------------------------------------------------
STATEMACHINE(INTERACTABLE_TWOWAYSWITCH_FSM, Interactable_TwoWaySwitch)
	INTERACTABLE_TWOWAYSWITCH_FSM(bool bActive)
	{
		if(bActive)
			SET_INITIAL_STATE(ACTIVE);
		else
			SET_INITIAL_STATE(INACTIVE);
	}

	STATE(ACTIVE)
		BEGIN_EVENTS
			ON_ENTER
				ME->m_pOther = 0;
				ME->GetInteractionComponent()->Lua_SetInteractionPriority(USE);
			END_EVENT(true)

			EVENT(msg_action)
			EVENT(msg_running_action)
				ME->m_pOther = (Character*)msg.GetEnt("Other");
				ntError(!ME->m_pOther || ME->m_pOther->IsCharacter());
				
					
				SET_STATE(INTERACTION);
			END_EVENT(true)

			EVENT(msg_deactivate)
			EVENT(Deactivate)
				SET_STATE(INACTIVE);
			END_EVENT(true)
		END_EVENTS
	END_STATE // ACTIVE

	STATE(INTERACTION)
		BEGIN_EVENTS
			ON_ENTER
			{
				Character*			pPlayer			= ME->m_pOther;
				LuaAttributeTable*	pSharedAttrs	= ME->GetSharedAttributes();

				ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);

				ntstd::String obUserAnimName;
				ntstd::String obObjAnimName;
				CPoint		obPosition( pSharedAttrs->GetVector("MoveToPosition") );
				CQuat		obRotation(	pSharedAttrs->GetVector("MoveToRotation") );

				if ( ME->m_bPositionA )
				{				
					obUserAnimName = pSharedAttrs->GetString("AnimPlayerPositionB");
					obObjAnimName = pSharedAttrs->GetString("AnimObjectPositionB");
					ME->GetMessageHandler()->ProcessEvent("OnActivateB");
				}
				else
				{				
					obUserAnimName = pSharedAttrs->GetString("AnimPlayerPositionA");
					obObjAnimName = pSharedAttrs->GetString("AnimObjectPositionA");
					ME->GetMessageHandler()->ProcessEvent("OnActivateA");
				}

				// User Stuff
				pPlayer->GetMovement()->Lua_AltStartSnapToMovement(obUserAnimName.c_str(), ME, obPosition, obRotation);
				pPlayer->GetMovement()->Lua_AltSetMovementCompleteMessage("msg_movementdone");
				// Tell the player to automatically return to default state when their movement is completed
				pPlayer->SetExitOnMovementDone(true);
				

				// Object stuff
				ME->GetInteractionComponent()->ExcludeCollisionWith(pPlayer);
				ME->Lua_AnimPlay(obObjAnimName.c_str(), 1.0f, true, false);
				ME->Lua_AnimMessageOnCompletion( obObjAnimName.c_str() );
				ME->m_bDeactivateOnFinish = false;

				ME->m_bPositionA = !ME->m_bPositionA;		
			}
			END_EVENT(true)

			EVENT(msg_interrupt)
				if (ME->m_bDeactivateOnFinish)
					SET_STATE(INACTIVE);
				else
					SET_STATE(ACTIVE);
			END_EVENT(true)

			EVENT(msg_animdone)
				ME->GetInteractionComponent()->AllowCollisionWith(ME->m_pOther);

				if ( ME->m_bDeactivateOnFinish)
					SET_STATE(INACTIVE);
				else
					SET_STATE(ACTIVE);
			END_EVENT(true);

			// If the switch receives a deactivate message whilst the player is interacting with it, 
			// then flag the switch to jump to inactive on completion of the interaction
			EVENT(msg_deactivate)
			EVENT(Deactivate)
				ME->m_bDeactivateOnFinish = true;
			END_EVENT(true);
		END_EVENTS
	END_STATE // INTERACTION

	STATE(INACTIVE)
		BEGIN_EVENTS
			ON_ENTER
				ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);
			END_EVENT(true)

			EVENT(msg_activate)
			EVENT(Activate)
				SET_STATE(ACTIVE);
			END_EVENT(true)
		END_EVENTS
	END_STATE // INACTIVE

END_STATEMACHINE //INTERACTABLE_TWOWAYSWITCH_FSM

//--------------------------------------------------
//!
//!	Interactable_TwoWaySwitch::Interactable_TwoWaySwitch()
//!	Default constructor
//!
//--------------------------------------------------
Interactable_TwoWaySwitch::Interactable_TwoWaySwitch()
{
	m_eType = EntType_Interactable;
	m_eInteractableType = EntTypeInteractable_TwoWaySwitch;

	m_pSharedAttributesPtr = 0;
	m_pSharedAttributes = 0;

	// Default variable values
	m_pOther = 0;
	m_bDeactivateOnFinish = false;
}


//--------------------------------------------------
//!
//!	Interactable_TwoWaySwitch::OnPostConstruct()
//!	Post Construct
//!
//--------------------------------------------------
void Interactable_TwoWaySwitch::OnPostConstruct()
{
	Interactable_Switch_Trigger::OnPostConstruct();

	InstallMessageHandler();
	InstallAnimator(m_AnimationContainer);
	InstallDynamics();

	m_pSharedAttributes = NT_NEW_CHUNK( Mem::MC_ENTITY ) LuaAttributeTable;
	m_pSharedAttributes->SetDataObject(ObjectDatabase::Get().GetDataObjectFromPointer(m_pSharedAttributesPtr));

	//if ( ! m_TargetName.IsNull() )
	//	m_pTarget = ObjectDatabase::Get().GetPointerFromName<CEntity*>(*m_TargetName);
}

//--------------------------------------------------
//!
//!	Interactable_TwoWaySwitch::OnLevelStart()
//!	Called for each ent on level startup
//!
//--------------------------------------------------
void Interactable_TwoWaySwitch::OnLevelStart()
{
	// Create and attach the statemachine. Must be done AFTER anim containers fixed up by area system
	// i.e. after XML serialisation. OR this shouldnt play an animation

	if ( m_bPositionA )
	{
		if ( m_pSharedAttributes->GetString("AnimObjectPositionA").c_str() )
		{
			CAnimator* pobAnimator = GetAnimator();	
			CAnimationPtr obNewAnim = pobAnimator->CreateAnimation( m_pSharedAttributes->GetString("AnimObjectPositionA").c_str() );
			obNewAnim->SetFlagBits( ANIMF_LOCOMOTING );
			obNewAnim->SetTime( obNewAnim->GetDuration() );
			pobAnimator->AddAnimation( obNewAnim );
		}
	}
	else
	{
		if ( m_pSharedAttributes->GetString("AnimObjectPositionB").c_str() )
		{
			CAnimator* pobAnimator = GetAnimator();	
			CAnimationPtr obNewAnim = pobAnimator->CreateAnimation( m_pSharedAttributes->GetString("AnimObjectPositionB").c_str() );
			obNewAnim->SetFlagBits( ANIMF_LOCOMOTING );
			obNewAnim->SetTime( obNewAnim->GetDuration() );
			pobAnimator->AddAnimation( obNewAnim );
		}
	}

	INTERACTABLE_TWOWAYSWITCH_FSM* pFSM = NT_NEW_CHUNK( Mem::MC_ENTITY ) INTERACTABLE_TWOWAYSWITCH_FSM(m_bActive);
	ATTACH_FSM(pFSM);
}


//--------------------------------------------------
//!
//!	Interactable_TwoWaySwitch::~Interactable_TwoWaySwitch()
//!	Default destructor
//!
//--------------------------------------------------
Interactable_TwoWaySwitch::~Interactable_TwoWaySwitch()
{
	NT_DELETE_CHUNK( Mem::MC_ENTITY, m_pSharedAttributes );
	m_pSharedAttributes = NULL;
}
