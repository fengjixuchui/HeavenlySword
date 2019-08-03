//--------------------------------------------------
//!
//!	\file game/entityinteractablepushable.cpp
//!	Definition of the Interactable Pushable entity object
//!
//--------------------------------------------------

#include "objectdatabase/dataobject.h"
#include "game/luaattrtable.h"
#include "Physics/system.h"
//#include "physics/collisionbitfield.h"
#include "physics/compoundLG.h"

#include "game/movement.h"

#include "messagehandler.h"
#include "effect/fxhelper.h"
#include "audio/audiohelper.h"
#include "game/combathelper.h"
#include "game/renderablecomponent.h"

#include "game/entityinteractablepushable.h"

// Components needed
#include "game/interactioncomponent.h"

void ForceLinkFunctionPushable()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionPushable() !ATTN!\n");
}


START_CHUNKED_INTERFACE(Interactable_Pushable, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(Interactable)
	COPY_INTERFACE_FROM(Interactable)

	PUBLISH_VAR_AS(m_Description, Description)
	PUBLISH_VAR_AS(m_AnimationContainer, AnimationContainer)
	PUBLISH_PTR_AS(m_pSharedAttributes, SharedAttributes)

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )	
END_STD_INTERFACE

START_STD_INTERFACE(Att_Pushable)
	PUBLISH_VAR_AS     (m_bAIAvoid, AIAvoid)
	PUBLISH_VAR_AS     (m_fAIAvoidRadius, AIAvoidRadius)
	PUBLISH_VAR_AS     (m_fMass, Mass)
	PUBLISH_VAR_AS     (m_obCenterOfMass, CenterOfMass)
	PUBLISH_VAR_AS     (m_fRestitution, Restitution)
	PUBLISH_VAR_AS     (m_fFriction, Friction)
	PUBLISH_VAR_AS     (m_fLinearDamping, LinearDamping)
	PUBLISH_VAR_AS     (m_fAngularDamping, AngularDamping)
	PUBLISH_VAR_AS     (m_fMaxLinearVelocity, MaxLinearVelocity)
	PUBLISH_VAR_AS     (m_fMaxAngularVelocity, MaxAngularVelocity)
	PUBLISH_VAR_AS     (m_obPhysicsSoundDef, PhysicsSoundDef)

	PUBLISH_VAR_AS     (m_fStrikeThreshold, StrikeThreshold)
	PUBLISH_VAR_AS     (m_obStrikeAttackData, StrikeAttackData)
	PUBLISH_VAR_AS     (m_fCollapseThreshold, CollapseThreshold)

	PUBLISH_VAR_AS     (m_obPushVelocity, PushVelocity)
	PUBLISH_VAR_AS     (m_fPushDelay, PushDelay)
	PUBLISH_VAR_AS     (m_obFastPushVelocity, FastPushVelocity)
	PUBLISH_VAR_AS     (m_fFastPushDelay, FastPushDelay)

	PUBLISH_VAR_AS     (m_obAnimPlayerMoveTo, AnimPlayerMoveTo)
	PUBLISH_VAR_AS     (m_obAnimPlayerRunTo, AnimPlayerRunTo)
	PUBLISH_VAR_AS     (m_obAnimPlayerPush, AnimPlayerPush)
	PUBLISH_VAR_AS     (m_obAnimPlayerFastPush, AnimPlayerFastPush)

	PUBLISH_VAR_AS     (m_obPfxImpact, PfxImpact)
	PUBLISH_VAR_AS     (m_obPfxCollapse, PfxCollapse)
	PUBLISH_VAR_AS     (m_obPfxDestroyed, PfxDestroyed)
	
	
	PUBLISH_VAR_AS     (m_obSfxCollapse, SfxCollapse)
	PUBLISH_VAR_AS     (m_obSfxDestroyed, SfxDestroyed)
	PUBLISH_VAR_AS     (m_bFixedAfterConstruction, FixedAfterConstruction)
END_STD_INTERFACE


//--------------------------------------------------
//!
//! Interactable Pushable State Machine
//!
//--------------------------------------------------
STATEMACHINE(INTERACTABLE_PUSHABLE_FSM, Interactable_Pushable)
	INTERACTABLE_PUSHABLE_FSM()
	{
		SET_INITIAL_STATE(DEFAULT);
	}

	STATE(DEFAULT)
		BEGIN_EVENTS
			ON_ENTER
				ME->m_poOtherEntity = 0;
				ME->GetInteractionComponent()->SetInteractionType(PUSH);
			END_EVENT(true)

			EVENT(msg_action)
				ME->m_poOtherEntity = (Character*)msg.GetEnt("Other");
				ntError(!ME->m_poOtherEntity || ME->m_poOtherEntity->IsCharacter());
				SET_STATE(MOVETO);
			END_EVENT(true)

			EVENT(msg_running_action)
				ME->m_poOtherEntity = (Character*)msg.GetEnt("Other");
				ntError(!ME->m_poOtherEntity || ME->m_poOtherEntity->IsCharacter());
				SET_STATE(RUNTO);
			END_EVENT(true)

			EVENT(msg_sword_strike)
			EVENT(msg_ragdoll_strike)
			EVENT(msg_hitsolid)
			EVENT(msg_hitcharacter)
				SET_STATE(COLLAPSED);
			END_EVENT(true)
		END_EVENTS
	END_STATE // DEFAULT

	STATE(MOVETO)
		BEGIN_EVENTS
			ON_ENTER
			{
				Att_Pushable* pSharedAttrs = ME->GetSharedAttributes();
				CEntity* pPlayer = ME->m_poOtherEntity;

				pPlayer->GetMovement()->Lua_StartMoveToTransition(pSharedAttrs->m_obAnimPlayerMoveTo, ME, 1, 1);
				pPlayer->GetMovement()->Lua_AltSetMovementCompleteMessage( "msg_movementdone", pPlayer );

				ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);
			}
			END_EVENT(true)

			EVENT(msg_interrupt)
				SET_STATE(INTERUPT);
			END_EVENT(true)

			EVENT(msg_movementdone)
				SET_STATE(NORMALPUSH);
			END_EVENT(true)

			EVENT(msg_sword_strike)
			EVENT(msg_ragdoll_strike)
			EVENT(msg_hitsolid)
			EVENT(msg_hitcharacter)
				SET_STATE(COLLAPSED);
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(RUNTO)
		BEGIN_EVENTS
			ON_ENTER
			{
				Att_Pushable* pSharedAttrs = ME->GetSharedAttributes();
				CEntity* pPlayer = ME->m_poOtherEntity;

				pPlayer->GetMovement()->Lua_StartMoveToTransition(pSharedAttrs->m_obAnimPlayerRunTo, ME, 1, 1);
				pPlayer->GetMovement()->Lua_AltSetMovementCompleteMessage( "msg_movementdone", pPlayer );

				ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);
			}
			END_EVENT(true)

			EVENT(msg_interrupt)
				SET_STATE(INTERUPT);
			END_EVENT(true)

			EVENT(msg_movementdone)
				SET_STATE(FASTPUSH);
			END_EVENT(true)

			EVENT(msg_sword_strike)
			EVENT(msg_ragdoll_strike)
			EVENT(msg_hitsolid)
			EVENT(msg_hitcharacter)
				SET_STATE(COLLAPSED);
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(NORMALPUSH)
		BEGIN_EVENTS
			ON_ENTER
			{
				Character* pPlayer = ME->m_poOtherEntity;
				Att_Pushable* pSharedAttrs = ME->GetSharedAttributes();
					
				pPlayer->GetMovement()->Lua_AltStartFacingMovement( pSharedAttrs->m_obAnimPlayerPush, 0.0f, 1.0f, 0.0f, 0.0f);
				pPlayer->GetMovement()->Lua_AltSetMovementCompleteMessage( "msg_movementdone", pPlayer );
				pPlayer->SetExitOnMovementDone(true);

				CDirection dStopped(0.0f, 0.0f, 0.0f);
				ME->GetPhysicsSystem()->Lua_AltSetLinearVelocity(dStopped);

				Message PushOnMessage(msg_think_pushon);
				ME->GetMessageHandler()->QueueMessageDelayed(PushOnMessage, pSharedAttrs->m_fPushDelay);
			}
			END_EVENT(true)

			EVENT(msg_think_pushon)
			{
				CEntity* pPlayer = ME->m_poOtherEntity;
				Att_Pushable* pSharedAttrs = ME->GetSharedAttributes();
				CDirection vImpulse(pSharedAttrs->m_obPushVelocity * pPlayer->GetMatrix());
					
				ME->GetPhysicsSystem()->Lua_AltSetLinearVelocity(vImpulse);
				ME->GetPhysicsSystem()->Lua_CompoundRigid_CheckAtRest();
				ME->GetPhysicsSystem()->Lua_CollisionStrike(true);
			}
			END_EVENT(true)

			EVENT(msg_interrupt)
				SET_STATE(INTERUPT);
			END_EVENT(true)

			EVENT(msg_atrest)
				ME->GetPhysicsSystem()->Lua_CollisionStrike(false);
				SET_STATE(DEFAULT);
			END_EVENT(true)

			EVENT(msg_sword_strike)
			EVENT(msg_ragdoll_strike)
			EVENT(msg_hitsolid)
			EVENT(msg_hitcharacter)
				SET_STATE(COLLAPSED);
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(FASTPUSH)
		BEGIN_EVENTS
			ON_ENTER
			{
				Character* pPlayer = ME->m_poOtherEntity;
				Att_Pushable* pSharedAttrs = ME->GetSharedAttributes();
					
				pPlayer->GetMovement()->Lua_AltStartFacingMovement( pSharedAttrs->m_obAnimPlayerFastPush, 0.0f, 1.0f, 0.0f, 0.0f);
				pPlayer->GetMovement()->Lua_AltSetMovementCompleteMessage( "msg_movementdone", pPlayer );
				pPlayer->SetExitOnMovementDone(true);

				CDirection dStopped(0.0f, 0.0f, 0.0f);
				ME->GetPhysicsSystem()->Lua_AltSetLinearVelocity(dStopped);

				Message PushOnMessage(msg_think_fastpushon);
				ME->GetMessageHandler()->QueueMessageDelayed(PushOnMessage, pSharedAttrs->m_fFastPushDelay);
			}
			END_EVENT(true)

			EVENT(msg_think_fastpushon)
			{
				CEntity* pPlayer = ME->m_poOtherEntity;
				Att_Pushable* pSharedAttrs = ME->GetSharedAttributes();
				CDirection vImpulse(pSharedAttrs->m_obFastPushVelocity * pPlayer->GetMatrix());
					
				ME->GetPhysicsSystem()->Lua_AltSetLinearVelocity(vImpulse);
				ME->GetPhysicsSystem()->Lua_CompoundRigid_CheckAtRest();
				ME->GetPhysicsSystem()->Lua_CollisionStrike(true);
			}
			END_EVENT(true)

			EVENT(msg_interrupt)
				SET_STATE(INTERUPT);
			END_EVENT(true)

			EVENT(msg_atrest)
				ME->GetPhysicsSystem()->Lua_CollisionStrike(false);
				SET_STATE(DEFAULT);
			END_EVENT(true)

			EVENT(msg_sword_strike)
			EVENT(msg_ragdoll_strike)
			EVENT(msg_hitsolid)
			EVENT(msg_hitcharacter)
				SET_STATE(COLLAPSED);
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(INTERUPT)
		BEGIN_EVENTS
			ON_ENTER
				ME->m_poOtherEntity = 0;
				ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);
				ME->GetPhysicsSystem()->Lua_Rigid_CheckAtRest();
			END_EVENT(true)

			EVENT(msg_atrest)
				SET_STATE(DEFAULT);
			END_EVENT(true)

			EVENT(msg_sword_strike)
			EVENT(msg_ragdoll_strike)
			EVENT(msg_hitsolid)
			EVENT(msg_hitcharacter)
				SET_STATE(COLLAPSED);
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(COLLAPSED)
		BEGIN_EVENTS
			ON_ENTER
			{
				ME->GetPhysicsSystem()->Lua_RemoveChildEntities();
				ME->GetPhysicsSystem()->Lua_CompoundRigid_Collapse();

				Att_Pushable* pSharedAttrs = ME->GetSharedAttributes();
				ME->m_poOtherEntity = 0;

				ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);

				if (!ntStr::IsNull(pSharedAttrs->m_obPfxCollapse) )
				{
					FXHelper::Pfx_CreateStatic(pSharedAttrs->m_obPfxCollapse, ME, "ROOT");
				}

				if (!ntStr::IsNull(pSharedAttrs->m_obSfxCollapse) )
				{
					//AudioHelper::PlaySound(pSharedAttrs->m_obSfxCollapse);
				}
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE
END_STATEMACHINE // INTERACTABLE_PUSHABLE_FSM


//--------------------------------------------------
//!
//!	Interactable_Pushable::Interactable_Pushable()
//!	Default constructor
//!
//--------------------------------------------------
Interactable_Pushable::Interactable_Pushable()
{
	m_eType = EntType_Interactable;
	m_eInteractableType = EntTypeInteractable_Pushable;


	m_pSharedAttributes = 0;

	// Initialise variables
	m_poOtherEntity = 0;
	m_nHits = 0;
}

//--------------------------------------------------
//!
//!	Interactable_Pushable::OnPostConstruct()
//!	Post Construct
//!
//--------------------------------------------------
void Interactable_Pushable::OnPostConstruct()
{
	Interactable::OnPostConstruct();

	InstallMessageHandler();
	InstallAnimator("NULL");
	// Install network component here (if needed)

	//InstallDynamics();

	// Create rigid body physics
	Lua_CreatePhysicsSystem();

	// Create a group
	//Physics::LogicGroup * lg = (Physics::LogicGroup*)Physics::ClumpTools::ConstructCompoundLGFromClump( this, &obInfo );
	Physics::CompoundLG * lg = NT_NEW Physics::CompoundLG( GetName(), this);
	lg->Load();
	if(lg)
	{
		// Add the group
		m_pobPhysicsSystem->AddGroup( lg );
		lg->Activate();	
	}
	else	
		ntPrintf("%s(%d): ### PHYSICS ERROR - Logic group not created for entity %s with clump %s\n", __FILE__, __LINE__, GetName().c_str(), GetClumpString().c_str());	

	// Register physics sound
	if (!ntStr::IsNull(m_pSharedAttributes->m_obPhysicsSoundDef))
	{
		m_pobPhysicsSystem->RegisterCollisionEffectFilterDef(m_pSharedAttributes->m_obPhysicsSoundDef);
	}

	INTERACTABLE_PUSHABLE_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) INTERACTABLE_PUSHABLE_FSM;
	ATTACH_FSM(pFSM);
}

//--------------------------------------------------
//!
//!	Interactable_Pushable::~Interactable_Pushable()
//!	Default destructor
//!
//--------------------------------------------------
Interactable_Pushable::~Interactable_Pushable()
{

}
