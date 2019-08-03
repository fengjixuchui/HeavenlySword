//--------------------------------------------------
//!
//!	\file game/entityinteractablehoiststatue.cpp
//!	Definition of the Interactable Hoist Statue entity object
//!
//--------------------------------------------------

#include "objectdatabase/dataobject.h"
#include "game/luaattrtable.h"
#include "Physics/system.h"
#include "Physics/singlerigidlg.h"
#include "physics/collisionbitfield.h"
#include "game/movement.h"

#include "messagehandler.h"
#include "effect/fxhelper.h"
#include "audio/audiohelper.h"
#include "game/fsm.h"
#include "game/combathelper.h"
#include "game/renderablecomponent.h"
#include "camera/camman.h"
#include "camera/camview.h"
#include "anim/animator.h"

#include "game/entityinteractablehoiststatue.h"

// Components needed
#include "game/interactioncomponent.h"

void ForceLinkFunctionHoistStatue()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionHoistStatue() !ATTN!\n");
}

START_CHUNKED_INTERFACE(Interactable_HoistStatue, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(Interactable)
	COPY_INTERFACE_FROM(Interactable)

	PUBLISH_VAR_AS(m_Description, Description)
	PUBLISH_VAR_AS(m_InitialState, InitialState)
	PUBLISH_PTR_WITH_DEFAULT_AS(m_pSharedAttributesPtr, SharedAttributes, Att_Statue_Default)
	PUBLISH_VAR_AS(m_AnimationContainer, AnimationContainer)
	PUBLISH_VAR_AS(m_obAnims, AnimsObjectHoist)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obCollisionAtk, "atk_collision", CollisionAttackData)

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )	
END_STD_INTERFACE


//--------------------------------------------------
//!
//! Interactable Hoist State Machine
//!
//--------------------------------------------------
STATEMACHINE(INTERACTABLE_HOISTSTATUE_FSM, Interactable_HoistStatue)
	INTERACTABLE_HOISTSTATUE_FSM()
	{
		SET_INITIAL_STATE(DEFAULT);
	}

	STATE(DEFAULT)
		BEGIN_EVENTS
			ON_ENTER
				ME->GetPhysicsSystem()->Lua_ActivateState("Rigid");
				ME->GetPhysicsSystem()->Lua_Rigid_SetMotionType(Physics::HS_MOTION_KEYFRAMED);
			END_EVENT(true)

			EVENT(msg_action)
				ME->m_fAnimSpeed = msg.GetFloat("Speed");
				SET_STATE( HOIST );
			END_EVENT(true)

			EVENT(msg_detach)
				
				SET_STATE( FALL );
			END_EVENT(true)
		END_EVENTS
	END_STATE // DEFAULT

	STATE(HOIST)
		BEGIN_EVENTS
			ON_ENTER
			{
				CKeyString obNextHoistAnim = ME->GetNextHoist();
				if ( ! obNextHoistAnim.IsNull() )
				{
					// Keyframed animation
					ME->GetPhysicsSystem()->Lua_Rigid_SetMotionType(Physics::HS_MOTION_KEYFRAMED);

					ME->Lua_AnimMessageOnCompletion( CHashedString(obNextHoistAnim) );
					ME->Lua_AnimPlay( CHashedString(obNextHoistAnim), ME->m_fAnimSpeed, true, false);
				}	
				else 
					SET_STATE(DEFAULT);
			}
			END_EVENT(true)

			EVENT(msg_animdone)
				SET_STATE(DEFAULT);
			END_EVENT(true)

			EVENT(msg_detach)	
				SET_STATE( FALL );
			END_EVENT(true)
		END_EVENTS
	END_STATE	// HOIST

	STATE(FALL)
		BEGIN_EVENTS
			ON_ENTER
			{
				CAnimator* pobAnimator = ME->GetAnimator();	
					
				pobAnimator->RemoveAllAnimations(  );

				//ME->GetPhysicsSystem()->Lua_DeactivateState("Rigid");

				// Go under dynamic motion
				ME->GetPhysicsSystem()->Lua_Rigid_SetMotionType(Physics::HS_MOTION_DYNAMIC);

				//ME->GetPhysicsSystem()->Lua_AltSetLinearVelocity(CDirection(0.0f,-0.8f,0.0f) );
				//ME->GetPhysicsSystem()->Lua_CompoundRigid_CheckAtRest();
				ME->GetPhysicsSystem()->Lua_CollisionStrike(true);

				//SET_STATE(DEFAULT);
			}
			END_EVENT(true)

			EVENT(msg_atrest)
				ME->GetPhysicsSystem()->Lua_CollisionStrike(false);
			END_EVENT(true)

			/*EVENT(msg_collapsed)
				//SET_STATE(COLLAPSED);
			END_EVENT(true)

			EVENT(msg_collision)
			{
				//ME->GetPhysicsSystem()->Lua_Rigid_OrientateToVelocity(false);
				//ME->GetPhysicsSystem()->Lua_Rigid_DeflectionRender(false);

				if (ME->m_obCollisionAtk != "")
				{
					CombatHelper::Combat_GenerateStrike(ME, ME, msg.GetEnt("Collidee"), *ME->m_obCollisionAtk);
				}
			}
			END_EVENT(true)*/

		END_EVENTS
	END_STATE // FALL

END_STATEMACHINE //INTERACTABLE_THROWN_FSM


//--------------------------------------------------
//!
//!	Interactable_HoistStatue::Interactable_HoistStatue()
//!	Default constructor
//!
//--------------------------------------------------
Interactable_HoistStatue::Interactable_HoistStatue()
{
	// scee.sbashow: This should be declared as an interactable
	// why was m_eType originally declared as unknown?
	m_eType = EntType_Interactable;
	m_eInteractableType = EntTypeInteractable_HoistStatue;
}

//--------------------------------------------------
//!
//!	Interactable_HoistStatue::OnPostConstruct()
//!	Post Construct
//!
//--------------------------------------------------
void Interactable_HoistStatue::OnPostConstruct()
{
	Interactable::OnPostConstruct();

	// Create components
	InstallMessageHandler();
	if(!m_AnimationContainer.IsNull())
		InstallAnimator(m_AnimationContainer);
	// Don't need to install dynamics here if you're gonna use specific shared attributes through Lua below
	// All InstallDynamics does is look for a PS.XML or clump header info, this is also done in the Lua ones
	InstallDynamics(); 

	// Set attributes
	/*m_pSharedAttributes = NT_NEW LuaAttributeTable;
	DataObject* pobBob = ObjectDatabase::Get().GetDataObjectFromPointer(m_pSharedAttributesPtr);
	m_pSharedAttributes->SetDataObject(pobBob);
	m_pSharedAttributes->SetInteger("IAmFlags", Physics::LARGE_INTERACTABLE_BIT);
	m_pSharedAttributes->SetInteger("CollideWithFlags", Physics::CHARACTER_CONTROLLER_PLAYER_BIT |
										   Physics::CHARACTER_CONTROLLER_ENEMY_BIT |
										   Physics::RAGDOLL_BIT |
										   Physics::SMALL_INTERACTABLE_BIT |
										   Physics::LARGE_INTERACTABLE_BIT);
	m_pSharedAttributes->SetAttribute("MotionType", "DYNAMIC");

	// Create rigid body physics
	Lua_CreatePhysicsSystem();
	m_pobPhysicsSystem->Lua_Rigid_ConstructFromClump( m_pSharedAttributes );*/

	GetPhysicsSystem()->Lua_Rigid_SetMotionType(Physics::HS_MOTION_KEYFRAMED);

	// Set up the requested anim set
	ntStr::Parse(m_obAnims, m_obHoistAnims, " ,;\t");
	m_pobCurrHoist = m_obHoistAnims.begin();

	/*for ( ntstd::List<CKeyString>::iterator obIt = m_obHoistAnims.begin(); obIt != m_obHoistAnims.end(); obIt++ )
	{
		ntPrintf( "%s\n", *(*obIt) );
	}*/

	// Create and attach the statemachine
	INTERACTABLE_HOISTSTATUE_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) INTERACTABLE_HOISTSTATUE_FSM();
	ATTACH_FSM(pFSM);
}

//--------------------------------------------------
//!
//!	Interactable_HoistStatue::~Interactable_HoistStatue()
//!	Default destructor
//!
//--------------------------------------------------
Interactable_HoistStatue::~Interactable_HoistStatue()
{

}

//--------------------------------------------------
//!
//!	Interactable_HoistStatue::GetNextHoist()
//!	Get the name for the next hoist animation
//!
//--------------------------------------------------
CKeyString Interactable_HoistStatue::GetNextHoist()
{
	if ( m_pobCurrHoist == m_obHoistAnims.end() )
		return CKeyString();

	return (*m_pobCurrHoist++);
}
