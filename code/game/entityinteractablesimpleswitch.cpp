//--------------------------------------------------
//!
//!	\file game/entityinteractablethrown.cpp
//!	Definition of the Interactable Thrown entity object
//!
//--------------------------------------------------

#include "objectdatabase/dataobject.h"
#include "game/luaattrtable.h"

#include "messagehandler.h"
#include "game/movement.h"
#include "camera/camman.h"
#include "camera/camview.h"

#include "game/entityinteractablesimpleswitch.h"

// Components needed
#include "game/interactioncomponent.h"

void ForceLinkFunctionSimpleSwitch()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionSimpleSwitch() !ATTN!\n");
}

START_CHUNKED_INTERFACE(Interactable_SimpleSwitch, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(Interactable_Switch_Trigger)
	COPY_INTERFACE_FROM(Interactable_Switch_Trigger)

	OVERRIDE_DEFAULT(Clump, "Switches\\lever.clump")

	PUBLISH_VAR_WITH_DEFAULT_AS(m_bActive, true, Active);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_AnimationContainer, "LeverAnimContainer", AnimationContainer)
	PUBLISH_PTR_WITH_DEFAULT_AS(m_pSharedAttributesPtr, SharedAttributes, Att_SimpleSwitch_Default)

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )	
END_STD_INTERFACE


//--------------------------------------------------
//!
//! Interactable Simple Switch State Machine
//!
//--------------------------------------------------
STATEMACHINE(INTERACTABLE_SIMPLESWITCH_FSM, Interactable_SimpleSwitch)
	INTERACTABLE_SIMPLESWITCH_FSM(bool bActive)
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
			{
				LuaAttributeTable* pSharedAttrs = ME->GetSharedAttributes();

				int nActivateCount = pSharedAttrs->GetInteger("ActivateCount");
				if (nActivateCount == 0 || ME->m_nUseCount < nActivateCount)
				{
					ME->m_pOther = (Character*)msg.GetEnt("Other");
					ntAssert(!ME->m_pOther || ME->m_pOther->IsCharacter());
					ME->m_nUseCount++;
					
					SET_STATE(INTERACTION);
				}
				else
				{
					ntPrintf("Usage count for switch %s has reached its limit\n", pSharedAttrs->GetString("Name").c_str() );
					SET_STATE(INACTIVE);
				}
			}
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

				// Player stuff
				ntstd::String obUserAnimName = pSharedAttrs->GetString("AnimPlayerActivate");
				CPoint		obPosition( pSharedAttrs->GetVector("MoveToPosition") );
				CQuat		obRotation(	pSharedAttrs->GetVector("MoveToRotation") );

				ntstd::String obObjAnimName = pSharedAttrs->GetString("AnimObjectActivate");

				pPlayer->GetMovement()->Lua_AltStartSnapToMovement(obUserAnimName.c_str(), ME, obPosition, obRotation);
				pPlayer->GetMovement()->Lua_AltSetMovementCompleteMessage("msg_movementdone");
				pPlayer->SetExitOnMovementDone(true);

				// Object stuff
				ME->GetInteractionComponent()->ExcludeCollisionWith(pPlayer);
				ME->Lua_AnimPlay(obObjAnimName.c_str(), 1.0f, true, false);

				ME->Lua_AnimMessageOnCompletion( obObjAnimName.c_str() );
				ME->m_bDeactivateOnFinish = false;

				// Camera
				if (pSharedAttrs->GetString("CameraAnimation") != "" && pPlayer->IsPlayer())
				{
					CamMan::Get().GetView(0)->ActivateMayaCam(pSharedAttrs->GetString("CameraAnimation").c_str(), 0, ME);
				}				

				// Think
				Message msgThink(msg_think_onactivate);
				ME->GetMessageHandler()->QueueMessageDelayed(msgThink, pSharedAttrs->GetNumber("ActivateDuration"));
			}
			END_EVENT(true)

			EVENT(msg_interrupt)
				ME->m_nUseCount--;

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

			EVENT(msg_think_onactivate)
				ME->GetMessageHandler()->ProcessEvent("OnAction");
			END_EVENT(true)
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

END_STATEMACHINE //INTERACTABLE_SIMPLESWITCH_FSM

//--------------------------------------------------
//!
//!	Interactable_Thrown::Interactable_Thrown()
//!	Default constructor
//!
//--------------------------------------------------
Interactable_SimpleSwitch::Interactable_SimpleSwitch()
{
	m_eType = EntType_Interactable;
	m_eInteractableType = EntTypeInteractable_SimpleSwitch;

	m_pSharedAttributesPtr = 0;
	m_pSharedAttributes = 0;

	// Default variable values
	m_pOther = 0;
	m_nUseCount	= 0;
	m_bDeactivateOnFinish = false;
}


//--------------------------------------------------
//!
//!	Interactable_SimpleSwitch::OnPostConstruct()
//!	Post Construct
//!
//--------------------------------------------------
void Interactable_SimpleSwitch::OnPostConstruct()
{
	Interactable_Switch_Trigger::OnPostConstruct();

	InstallMessageHandler();
	if(!m_AnimationContainer.IsNull())
	InstallAnimator(m_AnimationContainer);
	InstallDynamics();

	m_pSharedAttributes = NT_NEW_CHUNK( Mem::MC_ENTITY ) LuaAttributeTable;
	m_pSharedAttributes->SetDataObject(ObjectDatabase::Get().GetDataObjectFromPointer(m_pSharedAttributesPtr));

	//if ( ! m_TargetName.IsNull() )
	//	m_pTarget = ObjectDatabase::Get().GetPointerFromName<CEntity*>(*m_TargetName);

	// Create and attach the statemachine
	INTERACTABLE_SIMPLESWITCH_FSM* pFSM = NT_NEW_CHUNK( Mem::MC_ENTITY ) INTERACTABLE_SIMPLESWITCH_FSM(m_bActive);
	ATTACH_FSM(pFSM);
}

//--------------------------------------------------
//!
//!	Interactable_SimpleSwitch::~Interactable_SimpleSwitch()
//!	Default destructor
//!
//--------------------------------------------------
Interactable_SimpleSwitch::~Interactable_SimpleSwitch()
{
	NT_DELETE_CHUNK( Mem::MC_ENTITY, m_pSharedAttributes );
	m_pSharedAttributes = NULL;
}
