//--------------------------------------------------
//!
//!	\file game/entityrangedweapon.cpp
//!	Definition of the ranged weapon object
//!
//--------------------------------------------------


#include "game/entityprojectile.h"
#include "game/entityarcherxbow.h"
#include "game/entityarcher.h"
#include "game/movement.h"
#include "game/luaattrtable.h"
#include "game/combathelper.h"
#include "game/entitymanager.h"
#include "game/messagehandler.h"
#include "game/renderablecomponent.h"
#include "game/interactioncomponent.h"
#include "Physics/system.h"
#include "physics/collisionbitfield.h"
#include "anim/animator.h"
#include "core/exportstruct_anim.h"
#include "effect/fxhelper.h"
#include "audio/audiohelper.h"
#include "camera/camman.h"
#include "camera/camview.h"
#include "objectdatabase/dataobject.h"
#include "game/simpletransition.h"


void ForceLinkFunctionArcherXBow()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionArcherXBow() !ATTN!\n");
}

START_CHUNKED_INTERFACE(Object_ArcherXBow, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(CEntity)
	COPY_INTERFACE_FROM(CEntity)

	PUBLISH_VAR_AS(m_sAnimationContainer,		AnimationContainer)

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )
	DECLARE_POSTPOSTCONSTRUCT_CALLBACK( OnPostPostConstruct )
END_STD_INTERFACE


//--------------------------------------------------
//!
//! Archers Crossbow State Machine
//!
//--------------------------------------------------
STATEMACHINE(XBOW_FSM, Object_ArcherXBow)
	XBOW_FSM()
	{
		SET_INITIAL_STATE(DefaultState);
	}

	STATE(DefaultState)
		BEGIN_EVENTS
			ON_ENTER
			{
				//ME->Lua_SetIdentity();
				//ME->Lua_Reparent((CEntity*)ME->GetParent(), "clasp");
				//ME->Lua_SetLocalTransform(-0.131f, -0.403f, -0.062f, -93.22f, 10.536f, -18.063f);

				Message message(msg_activate);
				ME->GetMessageHandler()->QueueMessage(message);
			}
			END_EVENT(true)

			EVENT(msg_combat_state)
				//ntPrintf("archerXBOW - msg_combat_state");
				SET_STATE(CombatState);
			END_EVENT(true)

			EVENT(msg_activate)
			{
				ME->Lua_SetIdentity();
				ME->Lua_Reparent((CEntity*)ME->GetParent(), "clasp");

				// Play a closed anim for the xbow
				SimpleTransitionDef obDef;
				obDef.m_bApplyGravity			= false;
				obDef.m_bLooping				= true;
				obDef.m_obAnimationName			= "Xbow_IdleClosed";
				obDef.m_fSpeed					= 1.0f;
				obDef.m_fTimeOffsetPercentage	= 0.0f;
			
				// Push an instance of the controller on to our movement component
				ME->GetMovement()->BringInNewController( obDef, CMovement::DMM_STANDARD, 0.0f);
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE //DefaultState

	STATE(CombatState)
		BEGIN_EVENTS
			ON_ENTER
			{
				ME->Lua_SetIdentity();
				ME->Lua_Reparent((CEntity*)ME->GetParent(), "r_weapon");
				
				// Get a transition that will do its best to put us in the best movement state
				const AimingWeaponControllerDef* pobDefintion = ObjectDatabase::Get().GetPointerFromName<AimingWeaponControllerDef*>("ArcherXbow1stPerson_Movement");
				ntError_p( pobDefintion, ("error: XML movement controller ArcherXbow1stPerson_Movement is missing") );

				// Create a local copy of the aiming controller
				AimingWeaponControllerDef obAimingControllerDef( *pobDefintion );

				// Get the owner
				obAimingControllerDef.m_pPlayer = ME->GetParent();

				// Push an instance of the controller on to our movement component
				ME->GetMovement()->BringInNewController( obAimingControllerDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND);
			}
			END_EVENT(true)


			ON_EXIT
			{
			}
			END_EVENT(true)

			EVENT(msg_combat_exit)
				SET_STATE(DefaultState);
			END_EVENT(true)
			
			EVENT(msg_combat_attack)
			{
				float		fShotCharge	= msg.IsFloat("Charge") ? msg.GetFloat("Charge") : 0.0f;
				CDirection	dirShot		= CDirection( msg.GetFloat("X"), msg.GetFloat("Y"), msg.GetFloat("Z") );
				CEntity*	pNewBolt	= ME->GetParent()->Lua_CreateBolt(ME, msg.IsPtr("target") ? msg.GetEnt("target") : 0, dirShot, fShotCharge);
				
				if(pNewBolt)
				{
					ME->SetBolt(pNewBolt);
					CEntity* pEnt = msg.GetEnt("Sender");

					if(pEnt)
					{
						Message message(msg_combat_firing_xbow);

						// Set the charge at the time the player released the fire
						message.SetFloat("Charge", fShotCharge );

						pEnt->GetMessageHandler()->QueueMessage(message);
					}
					else
					{
						ntPrintf("%s(%d) - msg_combat_attack, no Sender specified\n", __FILE__, __LINE__);
					}
				}
			}
			END_EVENT(true)
			
			EVENT(msg_combat_startaftertouch)
				SET_STATE(AfterTouchState);
			END_EVENT(true)
		END_EVENTS

		STATE(AfterTouchState)
			BEGIN_EVENTS
				ON_ENTER
				{
					ntAssert( ME->GetBolt()->IsProjectile() );

					const char* pcParamNames = ME->GetBolt()->ToProjectile()->GetCharge() >= 1.0f ? "ArcherFullyChargedAftertouchedControlParameters" : "ArcherCrossbowAftertouchControlParameters";
					AftertouchControlParameters* pobAftertouchParams = ObjectDatabase::Get().GetPointerFromName<AftertouchControlParameters*>( pcParamNames );

					ME->GetBolt()->GetPhysicsSystem()->Lua_Projectile_EnableDualSteering(ME->GetParent(), pobAftertouchParams);

					// Make sure this entity is aware of the aftertouch completing
					ME->GetBolt()->GetMovement()->SetCompletionMessage( "msg_projectile_aftertouch_complete", ME );
					ME->GetBolt()->GetMovement()->SetInterruptMessage( "msg_projectile_aftertouch_complete", ME );

					// Tell the projectile to enter aftertouch state
					Message message(msg_aftertouch);
					message.SetEnt("Sender", ME);
					ME->GetBolt()->GetMessageHandler()->QueueMessage(message);
				}
				END_EVENT(true)

				EVENT(msg_projectile_aftertouch_complete)
				{
					// Send word back to the archer that the after has finished
					Message message(msg_aftertouch_done);
					ME->GetParent()->GetMessageHandler()->QueueMessage(message);

					ME->GetBolt()->GetPhysicsSystem()->Lua_Projectile_DisableSteering();

					POP_STATE();
				}
				END_EVENT(true)

				EVENT(msg_combat_endaftertouch)
				{
					// Tell the projectile to enter aftertouch state
					Message message(msg_aftertouch_done);
					ME->GetBolt()->GetMessageHandler()->QueueMessage(message);

					ME->GetBolt()->GetPhysicsSystem()->Lua_Projectile_DisableSteering();

					POP_STATE();
				}
				END_EVENT(true)
			END_EVENTS
		END_STATE //AfterTouchState

	END_STATE //CombatState

END_STATEMACHINE //RANGED_WEAPON_FSM


//--------------------------------------------------
//!
//!	Object_Ranged_Weapon::Object_Ranged_Weapon()
//!	Default constructor
//!
//--------------------------------------------------
Object_ArcherXBow::Object_ArcherXBow()
:	m_pBolt(0)
{
	m_eType = EntType_Object;

}

//--------------------------------------------------
//!
//!	Object_ArcherXBow::OnPostConstruct()
//!	Post Construct
//!
//--------------------------------------------------
void Object_ArcherXBow::OnPostConstruct()
{
	CEntity::OnPostConstruct();

	InstallMessageHandler();
	InstallAnimator(m_sAnimationContainer);
	SetMovement(NT_NEW_CHUNK(Mem::MC_ENTITY) CMovement(this, GetAnimator(), 0));

	// Create and attach the statemachine
	XBOW_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) XBOW_FSM();
	ATTACH_FSM(pFSM);
}

//--------------------------------------------------
//!
//!	Object_ArcherXBow::OnPostPostConstruct()
//!	Post Construct
//!
//--------------------------------------------------
void Object_ArcherXBow::OnPostPostConstruct()
{
	Lua_SetIdentity();
	Lua_Reparent(GetParent(), "clasp");
}


//--------------------------------------------------
//!
//!	Object_ArcherXBow::~Object_ArcherXBow()
//!	Default destructor
//!
//--------------------------------------------------
Object_ArcherXBow::~Object_ArcherXBow()
{

}
