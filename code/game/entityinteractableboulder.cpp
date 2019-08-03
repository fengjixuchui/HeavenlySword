//--------------------------------------------------
//!
//!	\file game/entityinteractablerigidbody.cpp
//!	Definition of the Interactable Rigid Body entity object
//!
//--------------------------------------------------

#include "objectdatabase/dataobject.h"
#include "game/luaattrtable.h"
#include "Physics/system.h"
#include "Physics/singlerigidlg.h"
#include "physics/collisionbitfield.h"
#include "messagehandler.h"
#include "game/interactioncomponent.h"
#include "game/interactiontransitions.h"
#include "game/movement.h"

#include "game/entityinteractableboulder.h"

void ForceLinkFunctionBoulder()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionBoulder() !ATTN!\n");
}


START_CHUNKED_INTERFACE(Interactable_Boulder, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(Interactable)
	COPY_INTERFACE_FROM(Interactable)

	OVERRIDE_DEFAULT(Clump, "Resources\\Objects\\Fort\\RockRoll\\rock.clump")
	OVERRIDE_DEFAULT(DefaultDynamics, "Rigid")


	PUBLISH_VAR_AS(m_Description, Description)
	PUBLISH_VAR_AS(m_InitialState, InitialState);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_AnimationContainer, "BoulderAnimContainer", AnimationContainer)
	PUBLISH_PTR_WITH_DEFAULT_AS(m_pSharedAttributesPtr, SharedAttributes, Att_Boulder_Default)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_AnimBoulderStart, "BoulderPushStart", BoulderStartAnim)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_AnimBoulderButtonMash, "BoulderPushButtonMash", BoulderButtonMashAnim)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_AnimBoulderEnd, "BoulderPushEnd", BoulderEndAnim)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_AnimOperatorStart, "BoulderPushOperatorStart", OperatorStartAnim)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_AnimOperatorButtonMash, "BoulderPushOperatorButtonMash", OperatorButtonMashAnim)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_AnimOperatorEnd, "BoulderPushOperatorEnd", OperatorEndAnim)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_CharacterPositionOffset, CPoint(0.0f, -1.01f, -1.316f), CharacterPositionOffset)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_CharacterRotationOffset, CPoint(0.0f, 0.0f, 0.0f), CharacterRotationOffset)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_fRequiredMPSStart, 1.0f, RequiredMPSAtStart)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fRequiredMPSEnd, 4.0f, RequiredMPSAtEnd)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fMPSRateVariation, 50.0f, PercentageMPSVariationRate)

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )
END_STD_INTERFACE


//--------------------------------------------------
//!
//! Interactable Boulder State Machine
//!
//--------------------------------------------------
STATEMACHINE(INTERACTABLE_BOULDER_FSM, Interactable_Boulder)
	INTERACTABLE_BOULDER_FSM()
	{
		SET_INITIAL_STATE(DEFAULT);
	}

	STATE(DEFAULT)
		BEGIN_EVENTS
			ON_ENTER
				ME->m_pOther = 0;

				ME->GetPhysicsSystem()->Lua_ActivateState("Rigid");
				ME->GetPhysicsSystem()->Lua_Rigid_SetMotionType(Physics::HS_MOTION_FIXED);
				
				// Make usable
				ME->GetInteractionComponent()->SetInteractionType(USE);
			END_EVENT(true)

			EVENT(msg_action)
			EVENT(msg_running_action)
			{
				ME->m_pOther = (Character*)msg.GetEnt("Other");
				ntAssert(!ME->m_pOther || ME->m_pOther->IsCharacter());

				
				// Play quick intro animations
                SET_STATE(PUSH);
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE // DEFAULT

	STATE(PUSHINTRO)
		BEGIN_EVENTS
			ON_ENTER
			{
				ntAssert( !ntStr::IsNull(ME->m_AnimBoulderStart) );
				ntAssert( !ntStr::IsNull(ME->m_AnimOperatorStart) );

				// Keyframed animation
				ME->GetPhysicsSystem()->Lua_Rigid_SetMotionType(Physics::HS_MOTION_KEYFRAMED);

				// Disable collision between boulder and operator
				ME->GetInteractionComponent()->ExcludeCollisionWith( ME->m_pOther );

				// Nothing else can use the boulder at the time
				ME->GetInteractionComponent()->SetInteractionType(NONE);

				// Disable movement components
				ME->GetMovement()->SetEnabled(false);

				// Boulder animation
				ME->Lua_AnimMessageOnCompletion( ME->m_AnimBoulderStart );
				ME->Lua_AnimPlay( ME->m_AnimBoulderStart, 1.0f, true, false);

				// Operator animation
				CQuat  rot(0.f, 0.f, 0.f, 1.f);
				ME->m_pOther->GetMovement()->Lua_AltStartSnapToMovement( ME->m_AnimOperatorStart, ME, ME->m_CharacterPositionOffset, rot );
				ME->m_pOther->GetMovement()->SetCompletionMessage("msg_movementdone");
				//ME->m_pOther->GetMovement()->Lua_AltSetMovementCompleteMessage( "msg_movementdone", ME->m_pOther );
			}
			END_EVENT(true)

			EVENT(msg_animdone)
			{
				// Enable movement component
				ME->GetMovement()->SetEnabled(true);

				SET_STATE(PUSH);
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE // PUSHINTRO

	STATE(PUSH)
		BEGIN_EVENTS
			ON_ENTER
			{
				// Nothing else can use the boulder at the time
				ME->GetInteractionComponent()->SetInteractionType(NONE);
				ME->GetPhysicsSystem()->Lua_Rigid_SetMotionType(Physics::HS_MOTION_KEYFRAMED);

				// Disable collision between boulder and operator
				ME->GetInteractionComponent()->ExcludeCollisionWith( ME->m_pOther );

				// Update shared params with pointers to the two interacting entities
				ME->m_SharedParams.m_pObjectEntity = ME;
				ME->m_SharedParams.m_pOperatorEntity = ME->m_pOther;

				ButtonMashControllerDef ControllerDef;
				ButtonMashOperatorControllerDef OperatorDef;

				ControllerDef.m_pSharedParams = &ME->m_SharedParams;
				OperatorDef.m_pSharedParams = &ME->m_SharedParams;

				ME->GetMovement()->BringInNewController( ControllerDef, CMovement::DMM_STANDARD, 0.0f );
				ME->m_pOther->GetMovement()->BringInNewController( OperatorDef, CMovement::DMM_STANDARD, 0.0f );

				// Nothing else can use the boulder at the time
				ME->GetInteractionComponent()->SetInteractionType(NONE);
			}
			END_EVENT(true)

			ON_UPDATE
			{
				// Check for end of button mashing
				if (ME->m_SharedParams.m_fCurrentInterpolant >= 1.0f)
				{
					// DONE!!!
					// Process any OnAction events
					ME->GetMessageHandler()->ProcessEvent("OnAction");
			
					// Clear controllers and exit state for player
					Message msgExitState(msg_exitstate);
					msgExitState.SetString(CHashedString(HASH_STRING_MSG), CHashedString(HASH_STRING_MSG_EXITSTATE));
					ME->m_pOther->GetMessageHandler()->QueueMessage(msgExitState);
					
					// Play outro animations
					SET_STATE(ROLLINGUNDERPHYSICS);
				}
			}
			END_EVENT(true)

			EVENT(msg_interrupt)
				SET_STATE(DEFAULT);
			END_EVENT(true)
		END_EVENTS
	END_STATE // PUSH

	STATE(PUSHOUTRO)
		BEGIN_EVENTS
			ON_ENTER
			{
				ntAssert( !ntStr::IsNull(ME->m_AnimBoulderEnd) );
				ntAssert( !ntStr::IsNull(ME->m_AnimOperatorEnd) );

				ME->GetPhysicsSystem()->Lua_Rigid_SetMotionType(Physics::HS_MOTION_KEYFRAMED);

				// Re-enable collision between boulder and operator
				ME->GetInteractionComponent()->AllowCollisionWith( ME->m_pOther );

				// Nothing else can use the boulder at the time
				ME->GetInteractionComponent()->SetInteractionType(NONE);

				// Enable movement component
				ME->GetMovement()->SetEnabled(false);

				// Boulder animation
				ME->Lua_AnimMessageOnCompletion( ME->m_AnimBoulderEnd );
				ME->Lua_AnimPlay( ME->m_AnimBoulderEnd, 1.0f, true, false);

				// Player animation
				CQuat  rot(0.f, 0.f, 0.f, 1.f);
				ME->m_pOther->GetMovement()->Lua_AltStartSnapToMovement( ME->m_AnimOperatorEnd, ME, ME->m_CharacterPositionOffset, rot );
				ME->m_pOther->GetMovement()->SetCompletionMessage("msg_movementdone");
				//ME->m_pOther->GetMovement()->Lua_AltSetMovementCompleteMessage( "msg_movementdone", ME->m_pOther );
				
				//ME->m_pOther->GetMovement()->Lua_StartRelativeMovement( ME, ME->m_AnimOperatorEnd, false, true );
				//ME->m_pOther->GetMovement()->Lua_AltSetMovementCompleteMessage( "msg_movementdone", ME->m_pOther );
			}
			END_EVENT(true)

			EVENT(msg_animdone)
			{
				// Enable movement component
				ME->GetMovement()->SetEnabled(true);
				
				SET_STATE(ROLLINGUNDERPHYSICS);
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE // PUSHOUTRO

	STATE(ROLLINGUNDERPHYSICS)
		BEGIN_EVENTS
			ON_ENTER
			{
				// Enable movement component
				ME->GetMovement()->SetEnabled(true);

				// Go under dynamic motion
				ME->GetPhysicsSystem()->Lua_Rigid_SetMotionType(Physics::HS_MOTION_DYNAMIC);

				// Nothing else can use it.
				ME->GetInteractionComponent()->SetInteractionType(NONE);

				// Disable collision between boulder and operator
				ME->GetInteractionComponent()->AllowCollisionWith( ME->m_pOther );

				// Temporary!  Give boulder some velocity
				CDirection LinearVelocity = ME->m_obFacingDirection * 1000.0f;
				ME->GetPhysicsSystem()->ApplyLinearImpulse( LinearVelocity );
				
				// No longer associate operator with the boulder
				ME->m_pOther = 0;
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE // ROLLINGUNDERPHYSICS

	STATE(DESTROYED)
		BEGIN_EVENTS
			ON_ENTER
				
			END_EVENT(true)
		END_EVENTS
	END_STATE // DESTROYED

END_STATEMACHINE


//--------------------------------------------------
//!
//!	Interactable_Boulder::Interactable_Boulder()
//!	Default constructor
//!
//--------------------------------------------------
Interactable_Boulder::Interactable_Boulder()
{
	m_pSharedAttributesPtr = 0;
	m_pSharedAttributes = 0;

	m_eType = EntType_Interactable;
	m_eInteractableType = EntTypeInteractable_Boulder;

	// Set defaults
	m_pOther = 0;
	m_fRequiredMPSStart = 1.0f;
	m_fRequiredMPSEnd = 4.0f;
	m_fMPSRateVariation = 50.0f;
}

//--------------------------------------------------
//!
//!	Interactable_Boulder::OnPostConstruct()
//!	Post Construct
//!
//--------------------------------------------------
void Interactable_Boulder::OnPostConstruct()
{
	Interactable::OnPostConstruct();
	
	// Create components
	InstallMessageHandler();
	if ( !m_AnimationContainer.IsNull() )
		InstallAnimator(m_AnimationContainer);
	
	// Don't need to install dynamics here if you're gonna use specific shared attributes through Lua below
	// All InstallDynamics does is look for a PS.XML or clump header info, this is also done in the Lua ones
	InstallDynamics(); 

	// Set attributes
	m_pSharedAttributes = NT_NEW LuaAttributeTable;
	m_pSharedAttributes->SetDataObject(ObjectDatabase::Get().GetDataObjectFromPointer(m_pSharedAttributesPtr));

	// Create movement component
	CMovement* pobMovement = NT_NEW_CHUNK(Mem::MC_ENTITY) CMovement( this,
											GetAnimator(),
											//pobDynamicsState);
											GetPhysicsSystem() );
	SetMovement(pobMovement);

	// Setup Shared Params
	// ----------------------------------------------------------------------
	m_SharedParams.m_obObjectButtonMashAnimName = m_AnimBoulderButtonMash;
	m_SharedParams.m_obOperatorButtonMashAnimName = m_AnimOperatorButtonMash;
	m_SharedParams.m_obCharacterTranslationOffset = m_CharacterPositionOffset;
	m_SharedParams.m_obCharacterRotationOffset = m_CharacterRotationOffset;

	m_SharedParams.m_fSecondsPerMashStart = 1.0f / m_fRequiredMPSStart;
	m_SharedParams.m_fSecondsPerMashEnd = 1.0f / m_fRequiredMPSEnd;
	m_SharedParams.m_fMPSPveRateVariation = m_fMPSRateVariation * 0.01f;
	m_SharedParams.m_fMPSNveRateVariation = m_fMPSRateVariation * 0.01f;
	// ----------------------------------------------------------------------

	ntstd::String PhysicsSoundDef = m_pSharedAttributes->GetString("PhysicsSoundDef");

	if (strcmp(PhysicsSoundDef.c_str(), "") != 0)
	{
		m_pobPhysicsSystem->RegisterCollisionEffectFilterDef(PhysicsSoundDef.c_str());
	}

	// TEMP
	m_obFacingDirection = GetRootTransformP()->GetWorldMatrix().GetZAxis();

	// Create and attach the statemachine
	INTERACTABLE_BOULDER_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) INTERACTABLE_BOULDER_FSM();
	ATTACH_FSM(pFSM);
}

//--------------------------------------------------
//!
//!	Interactable_Boulder::~Interactable_Boulder()
//!	Default destructor
//!
//--------------------------------------------------
Interactable_Boulder::~Interactable_Boulder()
{
	if (m_pSharedAttributes)
	{
		NT_DELETE(m_pSharedAttributes);
	}
}
