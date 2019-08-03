//--------------------------------------------------
//!
//!	\file game/entityinteractablerigidbody.cpp
//!	Definition of the Interactable Rigid Body entity object
//!
//--------------------------------------------------

#include "objectdatabase/dataobject.h"
#include "game/luaattrtable.h"
#include "Physics/system.h"
#include "physics/collisionbitfield.h"
#include "messagehandler.h"

#include "game/entityinteractablerigidbody.h"

void ForceLinkFunctionRigidBody()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionRigidBody() !ATTN!\n");
}


START_CHUNKED_INTERFACE(Interactable_RigidBody, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(CEntity)
	COPY_INTERFACE_FROM(CEntity)

	OVERRIDE_DEFAULT(DefaultDynamics, "Rigid")

	PUBLISH_VAR_AS(m_Description, Description)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_InitialState, "Active", InitialState);
	PUBLISH_VAR_AS(m_AnimationContainer, AnimationContainer)
	//PUBLISH_PTR_AS(m_pSharedAttributesPtr, SharedAttributes)

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )
END_STD_INTERFACE


//--------------------------------------------------
//!
//! Interactable Rigid Body State Machine
//!
//--------------------------------------------------
STATEMACHINE(INTERACTABLE_RIGID_BODY_FSM, Interactable_RigidBody)
	INTERACTABLE_RIGID_BODY_FSM(bool bAttached)
	{
		if(bAttached)
			SET_INITIAL_STATE(ATTACHED);
		else
			SET_INITIAL_STATE(DEFAULT);
	}

	STATE(DEFAULT)
	END_STATE

	STATE(ATTACHED)
		BEGIN_EVENTS
			EVENT(msg_detach)
				ME->Lua_ReparentToWorld();
				ME->GetPhysicsSystem()->Lua_ActivateState("Rigid");
				SET_STATE(DEFAULT);
			END_EVENT(true)
		END_EVENTS
	END_STATE

END_STATEMACHINE


//--------------------------------------------------
//!
//!	Interactable_RigidBody::Interactable_RigidBody()
//!	Default constructor
//!
//--------------------------------------------------
Interactable_RigidBody::Interactable_RigidBody()
{
//	m_pSharedAttributesPtr = 0;
//	m_pSharedAttributes = 0;

	m_eType = EntType_Object;
}

//--------------------------------------------------
//!
//!	Interactable_RigidBody::OnPostConstruct()
//!	Post Construct
//!
//--------------------------------------------------
void Interactable_RigidBody::OnPostConstruct()
{
	CEntity::OnPostConstruct();
	
	// Create components
	InstallMessageHandler();
	InstallDynamics();

	// Set attributes
	/*m_pSharedAttributes = NT_NEW LuaAttributeTable;
	m_pSharedAttributes->SetDataObject(ObjectDatabase::Get().GetDataObjectFromPointer(m_pSharedAttributesPtr));
	m_pSharedAttributes->SetInteger("IAmFlags", Physics::LARGE_INTERACTABLE_BIT);
	m_pSharedAttributes->SetInteger("CollideWithFlags", Physics::CHARACTER_CONTROLLER_PLAYER_BIT |
										   Physics::CHARACTER_CONTROLLER_ENEMY_BIT |
										   Physics::RAGDOLL_BIT |
										   Physics::SMALL_INTERACTABLE_BIT |
										   Physics::LARGE_INTERACTABLE_BIT);

	// Create rigid body physics
	Lua_CreatePhysicsSystem();
	m_pobPhysicsSystem->Lua_Rigid_ConstructFromClump( m_pSharedAttributes );	

	if (m_pSharedAttributes->GetBool("Keyframed") == true)
	{
		m_pobPhysicsSystem->Lua_Rigid_SetKeyframedMotion(true);
	}

	ntstd::String PhysicsSoundDef = m_pSharedAttributes->GetString("PhysicsSoundDef");

	if (strcmp(PhysicsSoundDef.c_str(), "") != 0)
	{
		m_pobPhysicsSystem->RegisterPhysicsSoundDef(PhysicsSoundDef.c_str());
	}*/

	// State machine initial state
	bool bAttached = false;

	if (m_InitialState == CHashedString("Attached") )
	{
		bAttached = true;
		m_pobPhysicsSystem->Lua_DeactivateState("Rigid");
	}

	if (m_InitialState == CHashedString("Active") )
	{
		bAttached = false;
		m_pobPhysicsSystem->Activate(true);
	}


	// Create and attach the statemachine
	INTERACTABLE_RIGID_BODY_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) INTERACTABLE_RIGID_BODY_FSM(bAttached);
	ATTACH_FSM(pFSM);
}

//--------------------------------------------------
//!
//!	Interactable_RigidBody::~Interactable_RigidBody()
//!	Default destructor
//!
//--------------------------------------------------
Interactable_RigidBody::~Interactable_RigidBody()
{
	/*if (m_pSharedAttributes)
	{
		NT_DELETE(m_pSharedAttributes);
	}*/
}
