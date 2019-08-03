//--------------------------------------------------
//!
//!	\file game/entitypickup.cpp
//!	Definition of the pickup entity object
//!
//--------------------------------------------------

#include "objectdatabase/dataobject.h"
#include "game/luaattrtable.h"
#include "Physics/system.h"
#include "physics/collisionbitfield.h"
#include "game/movement.h"
#include "anim/animator.h"
#include "core/exportstruct_anim.h"
#include "messagehandler.h"
#include "effect/fxhelper.h"
#include "audio/audiohelper.h"
#include "game/renderablecomponent.h"

#include "game/entitypickup.h"

// Components needed
#include "game/interactioncomponent.h"

void ForceLinkFunction40()
{
	ntPrintf("!ATTN! Calling ForceLinkFunction40() !ATTN!\n");
}

START_CHUNKED_INTERFACE(Object_Pickup, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(Interactable)
	COPY_INTERFACE_FROM(Interactable)

	PUBLISH_VAR_AS(m_Description, Description)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_InitialState, "Active", InitialState)
	PUBLISH_VAR_AS(m_AnimationContainer, AnimationContainer)
	PUBLISH_VAR_AS(m_AnimPlayerActivate, AnimPlayerActivate)
	PUBLISH_VAR_AS(m_AnimObjectActivate, AnimObjectActivate)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_MoveToPosition, CVector(0.0f, 0.0f, 0.0f, 0.0f), MoveToPosition)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_MoveToRotation, CQuat(0.0f, 0.0f, 0.0f, 1.0f), MoveToRotation)
	PUBLISH_VAR_AS(m_PickupFlag, PickupFlag)

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )	
END_STD_INTERFACE


//--------------------------------------------------
//!
//! Pickup State Machine
//!
//--------------------------------------------------
STATEMACHINE(PICKUP_FSM, Object_Pickup)
	PICKUP_FSM(bool bActive)
	{
		if(bActive)
		{
			SET_INITIAL_STATE(ACTIVE);
		}
		else
		{
			SET_INITIAL_STATE(INACTIVE);
		}
	}

	STATE(ACTIVE)
		BEGIN_EVENTS
			ON_ENTER
			{
				ME->m_pOther = NULL;
				ME->GetInteractionComponent()->Lua_SetInteractionPriority(USE);
			}
			END_EVENT(true)

			EVENT(msg_action)
			{
				ME->m_pOther = (Character*)msg.GetEnt("Other");
				ntError(!ME->m_pOther || ME->m_pOther->IsCharacter());
				SET_STATE(INTERACTION);
			}
			END_EVENT(true)

			EVENT(msg_running_action)
			{
				ME->m_pOther = (Character*)msg.GetEnt("Other");
				ntError(!ME->m_pOther || ME->m_pOther->IsCharacter());
				SET_STATE(INTERACTION);
			}
			END_EVENT(true)

			EVENT(msg_deactivate)
			{
				SET_STATE(INACTIVE);
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	STATE(INTERACTION)
		BEGIN_EVENTS
			ON_ENTER
			{
				Character* pUser = ME->m_pOther;

				ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);	//Nothing else can interact with this object at the moment.

				//Get the player to play the pickup animation.
				const char* pcAnimName = ntStr::GetString(ME->m_AnimPlayerActivate);
				CPoint		obPosition(ME->m_MoveToPosition);
				CQuat		obRotation(ME->m_MoveToRotation);

				pUser->GetMovement()->Lua_AltStartSnapToMovement(pcAnimName, ME, obPosition, obRotation);
				pUser->GetMovement()->Lua_AltSetMovementCompleteMessage("msg_movementdone", pUser);
				pUser->SetExitOnMovementDone(true); //Tell the player to automatically return to default state when their movement is completed

				//Play our pickup animation.
				ME->PlayAnim(ME->m_AnimObjectActivate);
			}
			END_EVENT(true)

			EVENT(msg_animdone)
			{
				ME->m_PickupFlag = true;
				SET_STATE(INACTIVE);
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	STATE(INACTIVE)
		BEGIN_EVENTS
			ON_ENTER
			{
				ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);
			}
			END_EVENT(true)

			EVENT(msg_activate)
			{
				SET_STATE(ACTIVE);
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

END_STATEMACHINE //PICKUP_FSM


//--------------------------------------------------
//!
//!	Object_Pickup::Object_Pickup()
//!	Default constructor
//!
//--------------------------------------------------
Object_Pickup::Object_Pickup()
{
	m_eType = EntType_Interactable;
	m_eInteractableType = 	EntTypeInteractable_Object_Pickup;

}

//--------------------------------------------------
//!
//!	Object_Pickup::OnPostConstruct()
//!	Post Construct
//!
//--------------------------------------------------
void Object_Pickup::OnPostConstruct()
{
	Interactable::OnPostConstruct();

	InstallMessageHandler();
	InstallAnimator(m_AnimationContainer);

	bool bActive = false;
	if(m_InitialState == "Active")
	{
		bActive = true;
	}
	// Create and attach the statemachine
	PICKUP_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) PICKUP_FSM(bActive);
	ATTACH_FSM(pFSM);
}

//--------------------------------------------------
//!
//!	Object_Pickup::~Object_Pickup()
//!	Default destructor
//!
//--------------------------------------------------
Object_Pickup::~Object_Pickup()
{
}

//--------------------------------------------------
//!
//!	void Object_Pickup::PlayAnim(const char *anim)
//!	Play requested animation.
//!
//--------------------------------------------------
void Object_Pickup::PlayAnim(CHashedString anim)
{
	//Check we've been given an animation to play.
	if(ntStr::IsNull(anim))
	{
		return;
	}

	//Play the anim (torn from Anim_Play, including the rest of this comment block).
	CAnimationPtr obNewAnim = GetAnimator()->CreateAnimation(anim);
	if(obNewAnim != 0)
	{
		obNewAnim->SetSpeed(1.0f);	//TODO: Read this from attributes?

		int iFlags = 0;

		if(true)					//TODO: Read locomoting flag from attributes? Unlikely, seems necessary (wierd results otherwise).
		{
			iFlags |= ANIMF_LOCOMOTING;
		}

		if(false)					//TODO: Read Looping flag from attributes?	Unlikely, msg_animdone never recieved if looping.
		{
			iFlags |= ANIMF_LOOPING;
		}

		obNewAnim->SetFlagBits(iFlags);

		GetAnimator()->AddAnimation(obNewAnim);
		GetAnimator()->GetAnimEventHandler().GenerateAnimDoneMessage(anim);
	}
	else
	{
		ntPrintf("Warning: Object_Pickup::PlayAnim() - Entity %s has no anim called %s\n", ntStr::GetString(GetName()), ntStr::GetString(anim));
	}
}
