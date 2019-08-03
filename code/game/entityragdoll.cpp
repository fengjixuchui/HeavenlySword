//--------------------------------------------------
//!
//!	\file game/entityragdoll.cpp
//!	Definition of the ragdoll entity object
//!
//--------------------------------------------------

#include "objectdatabase/dataobject.h"
#include "game/luaattrtable.h"
#include "Physics/system.h"
#include "physics/collisionbitfield.h"
#include "Physics/advancedcharactercontroller.h"
#include "game/movement.h"
#include "anim/animator.h"
#include "core/exportstruct_anim.h"
#include "messagehandler.h"
#include "effect/fxhelper.h"
#include "audio/audiohelper.h"
#include "game/renderablecomponent.h"

#include "game/entityragdoll.h"
#include "core/exportstruct_clump.h"

// Components needed
#include "game/interactioncomponent.h"

START_CHUNKED_INTERFACE(Object_Ragdoll, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(Interactable)
	COPY_INTERFACE_FROM(Interactable)

	OVERRIDE_DEFAULT(Position, "0.0,0.0,0.0")
	OVERRIDE_DEFAULT(Orientation, "0.0,0.0,0.0,-1.0")

	PUBLISH_VAR_AS(m_Description, Description)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_InitialState, "DefaultState", InitialState)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_RagdollClump, "default_ragdoll", RagdollClump)

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )	
END_STD_INTERFACE


//--------------------------------------------------
//!
//! Ragdoll State Machine
//!
//--------------------------------------------------
STATEMACHINE(RAGDOLL_FSM, Object_Ragdoll)
	RAGDOLL_FSM()
	{
		SET_INITIAL_STATE(DEFAULT);
	}

	STATE(DEFAULT)
		BEGIN_EVENTS
			ON_ENTER
			{
				//Anything for ragdolls to do?
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

END_STATEMACHINE //RAGDOLL_FSM


//--------------------------------------------------
//!
//!	Object_Ragdoll::Object_Ragdoll()
//!	Default constructor
//!
//--------------------------------------------------
Object_Ragdoll::Object_Ragdoll()
{
	m_eType = EntType_Character;
}

//--------------------------------------------------
//!
//!	Object_Ragdoll::OnPostConstruct()
//!	Post Construct
//!
//--------------------------------------------------
void Object_Ragdoll::OnPostConstruct()
{
	CEntity::OnPostConstruct();

	//Add Components.
	InstallMessageHandler();
	InstallAnimator("NULL");
	InstallAudioChannel();	//What on earth does a ragdoll need an audio channel for?

	//Sort out both 'character' and 'ragdoll' states in the dynamics.
	// Install the dynamics component
	InstallDynamics();

	// We need a ragdoll clump.
	ntError_p(m_RagdollClump != "" && m_RagdollClump != "NULL", ("Attempting to create dynamics state on a character with no ragdoll data specified."));

	// Go through the volumes on the clump and find one with a character volume reference
	// TEMP MAKE A DEBUG SHAPE FOR THE CHARACTER VOLUME
	CColprimDesc obDebugVolume;
	//obDebugVolume.m_eType = CV_TYPE_OBB;
	obDebugVolume.m_eType = CV_TYPE_CAPSULE;

#ifdef PLATFORM_PS3
	// XXX: Not sure if this is the right thing to do. Problem is that m_pcType is an ntDiskPointer< const char > on PS3
	//		so you can't actually just assign "" to it. Buggerations.
	//obDebugVolume.m_pcType = "";
	const char *pcType = static_cast< const char * >( obDebugVolume.m_pcType );
	pcType = "";
	// XXX END
#else
	obDebugVolume.m_pcType = "";
#endif // PLATFORM_PS3


	obDebugVolume.m_iTransform = ROOT_TRANSFORM;
	obDebugVolume.m_obRotation.SetIdentity();
	obDebugVolume.m_obTranslation.Clear();

	// Right - i am not sure about these collision shapes - i am using an ntError of 0.09 to 
	// make things look correct but i have no idea where this ntError comes from - GH
	obDebugVolume.m_obTranslation.Y() = ( ( 0.9f + ( 0.35f * 2.0f ) ) / 2.0f ) + 0.09f;

	obDebugVolume.m_obCapsuleData.fLength = 0.9f;
	obDebugVolume.m_obCapsuleData.fRadius = 0.35f;

//	//############################################################################################################################
//	// TODO: THIS NEEDS TO BE FIXED IF WE DECIDE TO REVIVE THIS OBJECT TYPE
//	//############################################################################################################################
//	Physics::AdvancedCharacterController* lg = NT_NEW Physics::AdvancedCharacterController(this, &obDebugVolume, m_RagdollClump);
//
//	if(GetPhysicsSystem() == 0)
//	{
//		Physics::System* system = NT_NEW_CHUNK(Mem::MC_ENTITY) Physics::System(this, GetName());
//		SetPhysicsSystem( system );
//	}
//
//	GetPhysicsSystem()->AddGroup(lg);
//
//	GetPhysicsSystem()->Lua_ActivateState("CharacterState");

	//Set up the movement, must be done after the dynamics.
	CMovement* pobMovement = NT_NEW_CHUNK(Mem::MC_ENTITY) CMovement(this,	GetAnimator(), GetPhysicsSystem());
	SetMovement(pobMovement);

	CEntity::GetPhysicsSystem()->Deactivate();
	GetPhysicsSystem()->Lua_ActivateState("Ragdoll");

	GetPhysicsSystem()->RegisterCollisionEffectFilterDef("RagdollPhysicsSoundDef");

	if (m_InitialState == "Attached")
	{
		//Do the attaching bit.
		GetPhysicsSystem()->Lua_AttachedFromLeftFoot();
	}

	// Create and attach the statemachine
	RAGDOLL_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) RAGDOLL_FSM();
	ATTACH_FSM(pFSM);
}

//--------------------------------------------------
//!
//!	Object_Ragdoll::~Object_Ragdoll()
//!	Default destructor
//!
//--------------------------------------------------
Object_Ragdoll::~Object_Ragdoll()
{
}
