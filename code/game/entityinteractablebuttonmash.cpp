//--------------------------------------------------
//!
//!	\file game/entityinteractablebuttonmash.cpp
//!	Definition of the Interactable Button-Mash entity object
//!
//--------------------------------------------------


//---------------------------------------------- TRIGGER ----------------------------------------------


#include "objectdatabase/dataobject.h"
#include "game/luaattrtable.h"
#include "Physics/system.h"
#include "physics/collisionbitfield.h"
#include "game/movement.h"
#include "messagehandler.h"

#include "game/entityinteractablebuttonmash.h"

// Components needed
#include "game/interactioncomponent.h"

void ForceLinkFunctionButtonMash()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionButtonMash() !ATTN!\n");
}


START_STD_INTERFACE( Att_ButtonMash )
	DEFINE_INTERFACE_INHERITANCE(Attr_Interactable)
	COPY_INTERFACE_FROM(Attr_Interactable)
END_STD_INTERFACE


START_CHUNKED_INTERFACE(Interactable_ButtonMash, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(Interactable_Switch_Trigger)
	COPY_INTERFACE_FROM(Interactable_Switch_Trigger)

	PUBLISH_VAR_AS(m_AnimationContainer, AnimationContainer)
	PUBLISH_PTR_AS(m_pobSharedAttributes, SharedAttributes)
	
	PUBLISH_VAR_AS(m_AnimStart, StartAnim)
	PUBLISH_VAR_AS(m_AnimButtonMash, ButtonMashAnim)
	PUBLISH_VAR_AS(m_AnimEnd, EndAnim)
	PUBLISH_VAR_AS(m_AnimFail, FailAnim)

	PUBLISH_VAR_AS(m_AnimOperatorStart, OperatorStartAnim)
	PUBLISH_VAR_AS(m_AnimOperatorButtonMash, OperatorButtonMashAnim)
	PUBLISH_VAR_AS(m_AnimOperatorEnd, OperatorEndAnim)
	PUBLISH_VAR_AS(m_AnimOperatorFail, OperatorFailAnim)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_iButtonMashRepeats, 1, ButtonMashRepeats)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_CharacterPositionOffset, CPoint(0.0f, 0.0f, 0.0f), CharacterPositionOffset)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_CharacterRotationOffset, CPoint(0.0f, 0.0f, 0.0f), CharacterRotationOffset)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_fRequiredMPSStart, 1.0f, RequiredMPSAtStart)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fRequiredMPSEnd, 4.0f, RequiredMPSAtEnd)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fMPSPveRateVariation, 50.0f, PositiveMPSVariationRate)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fMPSNveRateVariation, 50.0f, NegitiveMPSVariationRate)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_bExitOnFail, true, ExitOnFail)

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )	
END_STD_INTERFACE


//--------------------------------------------------
//!
//! Interactable Button-Mash-Usable State Machine
//!
//--------------------------------------------------
STATEMACHINE(INTERACTABLE_BUTTONMASH_FSM, Interactable_ButtonMash)
	INTERACTABLE_BUTTONMASH_FSM()
	{
		SET_INITIAL_STATE(DEFAULT);
	}

	STATE(DEFAULT)
		BEGIN_EVENTS
			ON_ENTER
			{
				ME->m_pOther = 0;
				ME->GetInteractionComponent()->Lua_SetInteractionPriority(USE);
			}
			END_EVENT(true)

			EVENT(msg_action)
			EVENT(msg_running_action)
			{
				ME->m_pOther = (Character*)msg.GetEnt("Other");
				ntAssert(!ME->m_pOther || ME->m_pOther->IsCharacter());
				SET_STATE(INTERACT_INTRO);
			}
			END_EVENT(true)

			EVENT(msg_action_specific)
			EVENT(msg_action_specific_run)
			{
				ME->m_pOther = (Character*)msg.GetEnt("Other");
				ntAssert(!ME->m_pOther || ME->m_pOther->IsCharacter());
				SET_STATE(INTERACT_INTRO);
			}
            END_EVENT(true)

			EVENT(Deactivate)
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
			
			EVENT(Activate)
			{
				SET_STATE(DEFAULT);
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(INTERACT_INTRO)
		BEGIN_EVENTS
			ON_ENTER
			{
				ME->m_bMovedOffStart = false;

				// Disable collision between button mash and operator
				ME->GetInteractionComponent()->ExcludeCollisionWith( ME->m_pOther );

				// Nothing else can use the button mash at the time
				ME->GetInteractionComponent()->SetInteractionType(NONE);

				// Using
				// Process any OnAction events
				ME->GetMessageHandler()->ProcessEvent("OnAction");

				if  ( ( !ntStr::IsNull(ME->m_AnimStart) ) && ( !ntStr::IsNull(ME->m_AnimOperatorStart) ) )
				{
					// Object animation
					ME->Lua_AnimMessageOnCompletion( ME->m_AnimStart );
					ME->Lua_AnimPlay( ME->m_AnimStart, 1.0f, true, false);

					// Operator animation
					ME->m_pOther->GetMovement()->Lua_StartRelativeMovement( ME, ME->m_AnimOperatorStart, false, true );
					ME->m_pOther->GetMovement()->Lua_AltSetMovementCompleteMessage( "msg_movementdone", ME->m_pOther );
				}
				else
					SET_STATE(INTERACT);
			}
			END_EVENT(true)

			EVENT(msg_animdone)
				SET_STATE(INTERACT);
			END_EVENT(true)
		END_EVENTS
	END_STATE // INTERACT_INTRO

	STATE(INTERACT)
		BEGIN_EVENTS
			ON_ENTER
			{
				// Update shared params with pointers to the two interacting entities
				ME->m_SharedParams.m_pObjectEntity = ME;
				ME->m_SharedParams.m_pOperatorEntity = ME->m_pOther;

				ButtonMashControllerDef ControllerDef;
				ButtonMashOperatorControllerDef OperatorDef;

				ControllerDef.m_pSharedParams = &ME->m_SharedParams;
				OperatorDef.m_pSharedParams = &ME->m_SharedParams;

				ControllerDef.SetDebugNames( ntStr::GetString( ME->GetName() ), "ButtonMashController");
				OperatorDef.SetDebugNames( ntStr::GetString( ME->m_pOther->GetName() ), "ButtonMashOperatorController");

				ME->GetMovement()->BringInNewController( ControllerDef, CMovement::DMM_STANDARD, 0.0f );
				ME->m_pOther->GetMovement()->BringInNewController( OperatorDef, CMovement::DMM_STANDARD, 0.0f );
			}
			END_EVENT(true)

			ON_UPDATE
			{
				// Pass on interpolat for connected objects
				ME->m_fStateValue = ME->m_SharedParams.m_fCurrentInterpolant;

				// Check for end of button mashing
				if ( ME->m_SharedParams.m_fCurrentInterpolant >= 1.0f )
				{
					// DONE!!!
					// Process any OnComplete events
					ME->GetMessageHandler()->ProcessEvent("OnComplete");
			
					// Clear controllers and exit state for player
					Message msgExitState(msg_exitstate);
					msgExitState.SetEnt( CHashedString(HASH_STRING_SENDER), ME);
					ME->m_pOther->GetMessageHandler()->QueueMessage(msgExitState);
					
					ME->GetMovement()->ClearControllers();
					
					// Play outro animations
					SET_STATE(INTERACT_OUTRO);
				}
				
				// Must have moved off start before we start to check for start
				if (!ME->m_bMovedOffStart && ME->m_SharedParams.m_fCurrentInterpolant > 0.0f)
					ME->m_bMovedOffStart = true;

				// Check for start of button mashing
				if ( (ME->m_bMovedOffStart && ME->m_SharedParams.m_fCurrentInterpolant <= 0.0f) 
				// Or we should exit when the button mashing fails
					|| (ME->m_bExitOnFail && ME->m_SharedParams.m_eButtonMashState == ButtonMashSharedParams::FAILED))
				{
					// All the way back to the start!!!
					ME->GetMessageHandler()->ProcessEvent("OnFail");
					
					// Clear controllers and exit state for player
					Message msgExitState(msg_exitstate);
					msgExitState.SetEnt( CHashedString(HASH_STRING_SENDER), ME);
					ME->m_pOther->GetMessageHandler()->QueueMessage(msgExitState);
					
					ME->GetMovement()->ClearControllers();
					
					// Play fail outro animations
					SET_STATE(INTERACT_FAIL_OUTRO);
				}
			}
			END_EVENT(true)

			EVENT(msg_grab_on)	//Exit out of the interaction with the crank if the player presses the grab button
			EVENT(msg_interrupt)
			{
				// Clear controllers and exit state for player
				Message msgExitState(msg_exitstate);
				msgExitState.SetEnt( CHashedString(HASH_STRING_SENDER), ME);
				msgExitState.SetString(CHashedString(HASH_STRING_MSG), CHashedString(HASH_STRING_MSG_EXITSTATE));
				ME->m_pOther->GetMessageHandler()->QueueMessage(msgExitState);
				
				//ME->GetMovement()->ClearControllers();
				
				// Play outro animations
				//SET_STATE(INTERACT_OUTRO);
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE // INTERACT

	STATE(INTERACT_OUTRO)
		BEGIN_EVENTS
			ON_ENTER
			{
				// Re-enable collision between object and operator
				ME->GetInteractionComponent()->AllowCollisionWith( ME->m_pOther );

				if ( ( !ntStr::IsNull(ME->m_AnimEnd) ) && ( !ntStr::IsNull(ME->m_AnimOperatorEnd) ) )
				{
					// Object animation
					ME->Lua_AnimMessageOnCompletion( ME->m_AnimEnd );
					ME->Lua_AnimPlay( ME->m_AnimEnd, 1.0f, true, false);

					// Operator animation
					ME->m_pOther->GetMovement()->Lua_StartRelativeMovement( ME, ME->m_AnimOperatorEnd, false, true );
					ME->m_pOther->GetMovement()->Lua_AltSetMovementCompleteMessage( "msg_movementdone", ME->m_pOther );

					ME->m_pOther->SetExitOnMovementDone(true);
				}
				else
					SET_STATE(INACTIVE);
			}
			END_EVENT(true)

			EVENT(msg_animdone)
				SET_STATE(INACTIVE);
			END_EVENT(true)
		END_EVENTS
	END_STATE // INTERACT_OUTRO

	STATE(INTERACT_FAIL_OUTRO)
		BEGIN_EVENTS
			ON_ENTER
			{
				// Re-enable collision between object and operator
				ME->GetInteractionComponent()->AllowCollisionWith( ME->m_pOther );

				if ( (  !ntStr::IsNull(ME->m_AnimFail) ) && (  !ntStr::IsNull(ME->m_AnimOperatorFail) ) )
				{
					// Object animation
					ME->Lua_AnimMessageOnCompletion( ME->m_AnimFail );
					ME->Lua_AnimPlay( ME->m_AnimFail, 1.0f, true, false);

					// Operator animation
					ME->m_pOther->GetMovement()->Lua_StartRelativeMovement( ME, ME->m_AnimOperatorFail, false, true );
					ME->m_pOther->GetMovement()->Lua_AltSetMovementCompleteMessage( "msg_movementdone", ME->m_pOther );

					ME->m_pOther->SetExitOnMovementDone(true);
				}
				else
					SET_STATE(DEFAULT);
			}
			END_EVENT(true)

			EVENT(msg_animdone)
				SET_STATE(DEFAULT);
			END_EVENT(true)
		END_EVENTS
	END_STATE // INTERACT_FAIL_OUTRO

END_STATEMACHINE //INTERACTABLE_BUTTONMASH_FSM


//--------------------------------------------------
//!
//!	Interactable_ButtonMash::Interactable_ButtonMash()
//!	Default constructor
//!
//--------------------------------------------------
Interactable_ButtonMash::Interactable_ButtonMash()
{
	m_eType = EntType_Interactable;
	m_eInteractableType = EntTypeInteractable_ButtonMash;

	m_pOther = 0;
}

//--------------------------------------------------
//!
//!	Interactable_ButtonMash::OnPostConstruct()
//!	Post Construct
//!
//--------------------------------------------------
void Interactable_ButtonMash::OnPostConstruct()
{
	Interactable_Switch_Trigger::OnPostConstruct();

	//Add Components.
	InstallMessageHandler();
	if ( ! m_AnimationContainer.IsNull() )
		InstallAnimator(m_AnimationContainer);
	InstallDynamics();

	//TODO: Does the crank need to be a rigid body? Optional parameter?

	//Movement
	CMovement* pobMovement = NT_NEW_CHUNK(Mem::MC_ENTITY) CMovement(this, GetAnimator(), GetPhysicsSystem());
	ntAssert(pobMovement != NULL);
	SetMovement(pobMovement);

	// Setup Shared Params
	// ----------------------------------------------------------------------
	m_SharedParams.m_obObjectButtonMashAnimName = m_AnimButtonMash;
	m_SharedParams.m_obOperatorButtonMashAnimName = m_AnimOperatorButtonMash;
	m_SharedParams.m_obCharacterTranslationOffset = m_CharacterPositionOffset;
	m_SharedParams.m_obCharacterRotationOffset = m_CharacterRotationOffset;

	m_SharedParams.m_fSecondsPerMashStart = 1.0f / m_fRequiredMPSStart;
	m_SharedParams.m_fSecondsPerMashEnd = 1.0f / m_fRequiredMPSEnd;
	m_SharedParams.m_fMPSPveRateVariation = m_fMPSPveRateVariation * 0.01f;
	m_SharedParams.m_fMPSNveRateVariation = m_fMPSNveRateVariation * 0.01f;

	m_SharedParams.m_iButtonMashRepeats = m_iButtonMashRepeats;
	// ----------------------------------------------------------------------

	// Create and attach the statemachine
	INTERACTABLE_BUTTONMASH_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) INTERACTABLE_BUTTONMASH_FSM();
	ATTACH_FSM(pFSM);
}

//--------------------------------------------------
//!
//!	Interactable_ButtonMash::~Interactable_ButtonMash()
//!	Default destructor
//!
//--------------------------------------------------
Interactable_ButtonMash::~Interactable_ButtonMash()
{
}
