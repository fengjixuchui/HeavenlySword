//--------------------------------------------------
//!
//!	\file game/entityiteractablesimpleusable.cpp
//!	Definition of the interactive "simple usable" entity object
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

#include "game/entityinteractablesimpleusable.h"

// Components needed
#include "game/interactioncomponent.h"

void ForceLinkFunction41()
{
	ntPrintf("!ATTN! Calling ForceLinkFunction41() !ATTN!\n");
}

START_CHUNKED_INTERFACE(Interactable_Simple_Usable, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(Interactable)
	COPY_INTERFACE_FROM(Interactable)

	PUBLISH_VAR_AS(m_Description, Description)
	PUBLISH_VAR_AS(m_AnimationContainer, AnimationContainer)
	PUBLISH_VAR_AS(m_AnimPlayerActivate, AnimPlayerActivate)
	PUBLISH_VAR_AS(m_AnimObjectActivate, AnimObjectActivate)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_MoveToPosition, CVector(0.0f, 0.0f, 0.0f, 0.0f), MoveToPosition)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_MoveToRotation, CQuat(0.0f, 0.0f, 0.0f, 1.0f), MoveToRotation)

	DECLARE_POSTCONSTRUCT_CALLBACK(OnPostConstruct)
END_STD_INTERFACE


//--------------------------------------------------
//!
//! "Simple Usable"-Object State Machine
//!
//--------------------------------------------------
STATEMACHINE(SIMPLE_USABLE_FSM, Interactable_Simple_Usable)
	SIMPLE_USABLE_FSM()
	{
		SET_INITIAL_STATE(DEFAULT);
	}

	STATE(DEFAULT)
		BEGIN_EVENTS
			ON_ENTER
			{
				ME->m_pOther = 0;
				ME->GetInteractionComponent()->Lua_SetInteractionPriority(USE);		//Open-house.
			}
			END_EVENT(true)

			EVENT(msg_action)
			{
				ME->m_pOther = (Character*)msg.GetEnt("Other");
				ntAssert(!ME->m_pOther || ME->m_pOther->IsCharacter());
				SET_STATE(INTERACTION);
			}
			END_EVENT(true)

			EVENT(msg_running_action)
			{
				ME->m_pOther = (Character*)msg.GetEnt("Other");
				ntAssert(!ME->m_pOther || ME->m_pOther->IsCharacter());
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
				//Play our activate animation.
				ME->PlayAnim(ME->m_AnimObjectActivate);

				//Get the player to play the use animation.
				Character* pUser = ME->m_pOther;
				const char* pcAnimName = ntStr::GetString(ME->m_AnimPlayerActivate);
				CPoint		obPosition(ME->m_MoveToPosition);
				CQuat		obRotation(ME->m_MoveToRotation);

				pUser->GetMovement()->Lua_AltStartSnapToMovement(pcAnimName, ME, obPosition, obRotation);
				pUser->GetMovement()->Lua_AltSetMovementCompleteMessage("msg_movementdone", pUser);
				//Tell the user to return to normal after the animation?
				pUser->SetExitOnMovementDone(true); //Automatically return to default state when movement is completed.

			}
			END_EVENT(true)

			EVENT(msg_animdone)
			{
				//The animation is over, now trigger the attached game event and then return to default state.
				//If necessary, the game event can then always send a msg_deactivate message to the switch to make it non-reusable.
				ME->GetMessageHandler()->ProcessEvent("OnAction");
				SET_STATE(DEFAULT);
			}
			END_EVENT(true)

			EVENT(msg_deactivate)
			{
				SET_STATE(INACTIVE);
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	STATE(INACTIVE)
		BEGIN_EVENTS
			ON_ENTER
			{
				ME->m_pOther = 0;
				ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);
			}
			END_EVENT(true)

			EVENT(msg_activate)
			{
				SET_STATE(DEFAULT);
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE

END_STATEMACHINE //SIMPLE_USABLE_FSM


//--------------------------------------------------
//!
//!	Interactable_Simple_Usable::Interactable_Simple_Usable()
//!	Default constructor
//!
//--------------------------------------------------
Interactable_Simple_Usable::Interactable_Simple_Usable()
{
	m_eType 				= EntType_Interactable;
	m_eInteractableType		= EntTypeInteractable_Simple_Usable;
	m_pOther = 0;
}

//--------------------------------------------------
//!
//!	Interactable_Simple_Usable::OnPostConstruct()
//!	Post Construct
//!
//--------------------------------------------------
void Interactable_Simple_Usable::OnPostConstruct()
{
	Interactable::OnPostConstruct();

	InstallMessageHandler();
	InstallAnimator(m_AnimationContainer);
	InstallDynamics();

	//TODO: Do we need physics etc here? InstallDynamics() is enough? etc.

	// Create and attach the statemachine
	SIMPLE_USABLE_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) SIMPLE_USABLE_FSM();
	ATTACH_FSM(pFSM);
}

//--------------------------------------------------
//!
//!	Interactable_Simple_Usable::~Interactable_Simple_Usable()
//!	Default destructor
//!
//--------------------------------------------------
Interactable_Simple_Usable::~Interactable_Simple_Usable()
{
}

//--------------------------------------------------
//!
//!	void Interactable_Simple_Usable::PlayAnim(const char *anim)
//!	Play requested animation.
//!
//--------------------------------------------------
void Interactable_Simple_Usable::PlayAnim(CHashedString anim)
{
	//Check we've been given an animation to play.
	if(ntStr::GetString(anim))
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
		ntPrintf("Warning: Interactable_Simple_Usable::PlayAnim() - Entity %s has no anim called %s\n", ntStr::GetString(GetName()), ntStr::GetString(anim));
	}
}
