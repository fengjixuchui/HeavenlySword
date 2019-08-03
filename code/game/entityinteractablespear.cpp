//--------------------------------------------------
//!
//!	\file game/entityinteractablespear.cpp
//!	Definition of the Interactable Spear entity object
//!
//--------------------------------------------------


#include "objectdatabase/dataobject.h"
#include "game/luaattrtable.h"
#include "Physics/system.h"
#include "physics/collisionbitfield.h"
#include "game/movement.h"
#include "anim/animator.h"
#include "messagehandler.h"
#include "camera/camman.h"
#include "camera/camview.h"
#include "effect/fxhelper.h"
#include "audio/audiohelper.h"
#include "game/renderablecomponent.h"

#include "game/entityinteractablespear.h"

// Components needed
#include "game/interactioncomponent.h"
#include "physics/spearlg.h"


void ForceLinkFunctionSpear()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionSpear() !ATTN!\n");
}

START_CHUNKED_INTERFACE(Interactable_Spear, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(Interactable)
	COPY_INTERFACE_FROM(Interactable)

	OVERRIDE_DEFAULT(Clump, "Spear\\spear.clump")

	PUBLISH_VAR_AS(m_Description, Description)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_AnimationContainer, "SpearAnimContainer", AnimationContainer)
	PUBLISH_PTR_WITH_DEFAULT_AS(m_pSharedAttributesPtr, SharedAttributes, Att_Spear_StandardSpear)

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )	
END_STD_INTERFACE


//--------------------------------------------------
//!
//! Interactable Spear State Machine
//!
//--------------------------------------------------
STATEMACHINE(INTERACTABLE_SPEAR_FSM, Interactable_Spear)
	INTERACTABLE_SPEAR_FSM()
	{
		SET_INITIAL_STATE(DEFAULT);
	}

	STATE(DEFAULT)
		BEGIN_EVENTS
			ON_ENTER
			{
				ME->m_pOther = NULL;
				ME->m_bAiming = false;
				ME->m_nThrownCameraHandle = 0;
				ME->m_bHitRagdoll = false;
				ME->m_bHitSolid = false;
				ME->m_bCollision = false;

				ME->GetInteractionComponent()->Lua_SetInteractionPriority(PICKUP);
			}
			END_EVENT(true)

			EVENT(msg_action)
			{
				ME->m_pOther = (Character*)msg.GetEnt("Other");
				ntError(!ME->m_pOther || ME->m_pOther->IsCharacter());
				SET_STATE(MOVETO);
			}
			END_EVENT(true)

			EVENT(msg_running_action)
			{
				ME->m_pOther = (Character*)msg.GetEnt("Other");
				ntError(!ME->m_pOther || ME->m_pOther->IsCharacter());
				SET_STATE(RUNTO);
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(MOVETO)
		BEGIN_EVENTS
			ON_ENTER
			{
				LuaAttributeTable* pSharedAttr = ME->GetSharedAttributes();

				//Player moves into position before starting the pickup
				ME->m_pOther->GetMovement()->Lua_StartMoveToTransition(pSharedAttr->GetString("AnimPlayerMoveTo").c_str(), ME, 1.0f, 1.0f);
				ME->m_pOther->GetMovement()->Lua_AltSetMovementCompleteMessage("msg_movementdone", ME->m_pOther);

				//Change our interaction priority to none, no other object can interract with this spear now.
				ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);
			}
			END_EVENT(true)

			EVENT(msg_interrupt)
			{
				SET_STATE(INTERRUPT);
			}
			END_EVENT(true)

			EVENT(msg_movementdone)
			{
				SET_STATE(PICKUPSTATE);
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	STATE(RUNTO)
		BEGIN_EVENTS
			ON_ENTER
			{
				LuaAttributeTable* pSharedAttr = ME->GetSharedAttributes();

				//Player moves into position before starting the pickup.
				ME->m_pOther->GetMovement()->Lua_StartMoveToTransition(pSharedAttr->GetString("AnimPlayerRunTo").c_str(), ME, 1.0f, 1.0f);
				ME->m_pOther->GetMovement()->Lua_AltSetMovementCompleteMessage("msg_movementdone", ME->m_pOther);

				//Change our interaction priority to none, no other object can interract with this spear now.
				ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);
			}
			END_EVENT(true)

			EVENT(msg_interrupt)
			{
				SET_STATE(INTERRUPT);
			}
			END_EVENT(true)

			EVENT(msg_movementdone)
			{
				SET_STATE(RUNPICKUP);
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	STATE(PICKUPSTATE)
		BEGIN_EVENTS
			EVENT(msg_think_onquickthrow)
			{
				if(ME->m_bDoQuickThrow == true)
				{
					SET_STATE(THROW);
				}
			}
			END_EVENT(true)

			EVENT(msg_think_onquickthrowcheck)
			{
				ME->m_bCheckQuickThrow = true;

				Message msgOnQuickThrow(msg_think_onquickthrow);
				ME->GetMessageHandler()->QueueMessageDelayed(msgOnQuickThrow, 0.7f);
			}
			END_EVENT(true)

			ON_ENTER
			{
				LuaAttributeTable* pSharedAttr = ME->GetSharedAttributes();
				ME->m_pOther->GetMovement()->Lua_AltStartFacingMovement(pSharedAttr->GetString("AnimPlayerPickup").c_str(), 360.0f, 1.0f, 0.0f, 0.0f, 0.0f);
				ME->m_pOther->GetMovement()->Lua_AltSetMovementCompleteMessage("msg_movementdone", ME->m_pOther);

				ME->GetInteractionComponent()->ExcludeCollisionWith(ME->m_pOther);	//Allow the object and player collision primitives to overlap.
				ME->GetPhysicsSystem()->Lua_Spear_SetMotionType(Physics::HS_MOTION_KEYFRAMED);	//Take control away from havok.
				ME->Lua_SetIdentity();
				ME->Lua_Reparent(ME->m_pOther, "r_weapon");

				ME->m_bCheckQuickThrow = false;
				ME->m_bDoQuickThrow = false;

				Message msgOnQuickThrowCheck(msg_think_onquickthrowcheck);
				ME->GetMessageHandler()->QueueMessageDelayed(msgOnQuickThrowCheck, 0.1f);
			}
			END_EVENT(true)

			EVENT(msg_interrupt)
			{
				SET_STATE(INTERRUPT);
			}
			END_EVENT(true)

			EVENT(msg_movementdone)
			{
				SET_STATE(HELD);
			}
			END_EVENT(true)

			EVENT(msg_action)
			{
				if(ME->m_bCheckQuickThrow == true)
				{
					ME->m_bDoQuickThrow = true;
				}
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	STATE(RUNPICKUP)
		BEGIN_EVENTS
			EVENT(msg_think_onquickthrow)
			{
				if(ME->m_bDoQuickThrow == true)
				{
					SET_STATE(THROW);
				}
			}
			END_EVENT(true)

			EVENT(msg_think_onquickthrowcheck)
			{
				ME->m_bCheckQuickThrow = true;

				Message msgOnQuickThrow(msg_think_onquickthrow);
				ME->GetMessageHandler()->QueueMessageDelayed(msgOnQuickThrow, 0.7f);
			}
			END_EVENT(true)

			ON_ENTER
			{
				LuaAttributeTable* pSharedAttr = ME->GetSharedAttributes();
				//Player starts their pickup anim.
				ME->m_pOther->GetMovement()->Lua_AltStartFacingMovement(pSharedAttr->GetString("AnimPlayerRunPickup").c_str(), 360.0f, 1.0f, 0.0f, 0.0f, 0.0f);
				ME->m_pOther->GetMovement()->Lua_AltSetMovementCompleteMessage("msg_movementdone", ME->m_pOther);

				//Object plays it's pickup anim relative to the player's root.
				ME->GetInteractionComponent()->ExcludeCollisionWith(ME->m_pOther);	//Allow the object and player collision primitives to overlap.
				ME->GetPhysicsSystem()->Lua_Spear_SetMotionType(Physics::HS_MOTION_KEYFRAMED);	//Take control away from havok.
				ME->Lua_SetIdentity();
				ME->Lua_Reparent(ME->m_pOther, "r_weapon");

				ME->m_bCheckQuickThrow = false;
				ME->m_bDoQuickThrow = false;

				Message msgOnQuickThrowCheck(msg_think_onquickthrowcheck);
				ME->GetMessageHandler()->QueueMessageDelayed(msgOnQuickThrowCheck, 0.1f);
			}
			END_EVENT(true)

			EVENT(msg_interrupt)
			{
				SET_STATE(INTERRUPT);
			}
			END_EVENT(true)

			EVENT(msg_movementdone)
			{
				SET_STATE(HELD);
			}
			END_EVENT(true)

			EVENT(msg_action)
			{
				if(ME->m_bCheckQuickThrow == true)
				{
					ME->m_bDoQuickThrow = true;
				}
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	STATE(HELD)
		BEGIN_EVENTS
			ON_ENTER
			{
				//ntPrintf("Spear -> HeldState");
				LuaAttributeTable* pSharedAttr = ME->GetSharedAttributes();

				//Player begins their holding movement, chase cam is activated
				ME->m_pOther->GetPhysicsSystem()->Lua_SetHoldingCapsule(true);
				ME->m_pOther->GetMovement()->Lua_StartMovementFromXMLDef(pSharedAttr->GetString("PlayerHoldingMovement").c_str());
				ME->m_pOther->Lua_ResetAimingComponent();

				CamMan::Get().GetView(0)->ActivateChaseAimCam(ME->m_pOther, pSharedAttr->GetString("ChasecamProperties").c_str(),
					pSharedAttr->GetString("AimcamProperties").c_str());

				ME->m_bAiming = false;
			}
			END_EVENT(true)

			EVENT(msg_interrupt)
			{
				SET_STATE(INTERRUPT);
			}
			END_EVENT(true)

			//Power fires the weapon
			EVENT(msg_power_on)
			EVENT(msg_attack_on)
			EVENT(msg_action_on)
			{
				SET_STATE(THROW);
			}
			END_EVENT(true)

			EVENT(msg_grab_on)
			{
				SET_STATE(DROP);
			}
			END_EVENT(true)

			//Handle going into and out of aiming mode.
			EVENT(msg_aim_on)
			{
				LuaAttributeTable* pSharedAttr = ME->GetSharedAttributes();
				ME->m_pOther->GetMovement()->Lua_StartMovementFromXMLDef(pSharedAttr->GetString("PlayerAimingMovement").c_str());
				ME->m_pOther->Lua_ResetAimingComponent();

				ME->m_bAiming = true;
			}
			END_EVENT(true)

			EVENT(msg_aim_off)
			{
				LuaAttributeTable* pSharedAttr = ME->GetSharedAttributes();
				ME->m_pOther->Lua_ResetAimingComponent();
				ME->m_pOther->GetMovement()->Lua_StartMovementFromXMLDef(pSharedAttr->GetString("PlayerHoldingMovement").c_str());

				ME->m_bAiming = false;
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	STATE(THROW)
		BEGIN_EVENTS
			EVENT(msg_think_onaftertouch)
			{
				//ntPrintf("Spear -> ThrowState -> OnAfterTouch");

				if((ME->m_bCollision == false) && (ME->m_bHitSolid == false))
				{
					if((ME->m_pOther->Lua_IsPowerHeld(0.3f)) || (ME->m_pOther->Lua_IsAttackHeld(0.3f)) || (ME->m_pOther->Lua_IsActionHeld(0.3f)))
					{
						SET_STATE(AFTERTOUCH);
					}
				}
			}
			END_EVENT(true)

			EVENT(msg_think_onthrow)
			{
				LuaAttributeTable* pSharedAttr = ME->GetSharedAttributes();
				ME->Lua_ReparentToWorld();	//Reparent the object back to the world.
				ME->GetPhysicsSystem()->Lua_Spear_StartThrownBehaviour();	//Allow havok to take control of the object

				CDirection ThrowVelocity(pSharedAttr->GetVector("ThrowVelocity"));
				if (ME->m_bAiming)
				{
					ME->GetPhysicsSystem()->Lua_AltSetLinearVelocityFromCamera(ThrowVelocity);
				}
				else
				{
					ME->GetPhysicsSystem()->Lua_AltSetLinearVelocityFromTarget(ME->m_pOther, ThrowVelocity);
				}

				//Added to delay the aftertouch for one frame to give the physics system time to get the initial position/velocity correct
				//before we change the time-scalar.

				Message msgOnAftertouch(msg_think_onaftertouch);
				ME->GetMessageHandler()->QueueMessage(msgOnAftertouch);
			}
			END_EVENT(true)

			ON_ENTER
			{
				LuaAttributeTable* pSharedAttr = ME->GetSharedAttributes();
				if (!ME->m_bAiming)
				{
					ME->m_pOther->GetMovement()->Lua_AltStartTargetedFacingMovement(pSharedAttr->GetString("AnimPlayerThrow").c_str(), 360.0f, 1.0f, 0.0f, 0.05f);
					ME->m_pOther->GetMovement()->Lua_AltSetMovementCompleteMessage("msg_movementdone", ME->m_pOther);
				}
				else
				{
					ME->m_pOther->GetMovement()->Lua_StartMovementFromXMLDef(pSharedAttr->GetString("PlayerThrowMovement").c_str());
					ME->m_pOther->GetMovement()->Lua_AltSetMovementCompleteMessage("msg_movementdone", ME->m_pOther);
				}

				ME->m_bHitRagdoll = false;
				ME->m_bHitSolid = false;
				ME->m_bCollision = false;

				Message msgOnThrow(msg_think_onthrow);

				if(ME->m_bAiming == false)
				{
					ME->GetMessageHandler()->QueueMessageDelayed(msgOnThrow, pSharedAttr->GetNumber("ThrowTime"));
				}
				else
				{
					ME->GetMessageHandler()->QueueMessageDelayed(msgOnThrow, pSharedAttr->GetNumber("AimedThrowTime"));
				}
			}
			END_EVENT(true)

			EVENT(msg_interrupt)
			{
				SET_STATE(INTERRUPT);
			}
			END_EVENT(true)

			EVENT(msg_atrest)
			{
				//Spear is at rest, if there are ragdolls attached to the spear, the spear is no-longer interactive.
				//Otherwise, the spear returns to a default state where it can be picked up again.

				if(ME->m_bHitRagdoll == true)
				{
					SET_STATE(INACTIVE);
				}
				else
				{
					SET_STATE(DEFAULT);
				}
			}
			END_EVENT(true)

			EVENT(msg_movementdone)
			{
				//Player has finished their throw animation, so we tell the player to return to exit their interaction state.
				//This object then remains in it's thrown state until it is at rest before it can return to its default state.
				Message msgExitState(msg_exitstate);
				ME->m_pOther->GetMessageHandler()->QueueMessage(msgExitState);

				CamMan::Get().GetView(0)->RemoveAllCoolCameras();

				ME->GetPhysicsSystem()->Lua_Spear_GenerateAtRestMessage();

				ME->m_pOther = NULL;
			}
			END_EVENT(true)

			EVENT(msg_removeragdoll)	//Ragdolls parented to this spear have become detached.
			{
				ME->m_bHitRagdoll = false;
			}
			END_EVENT(true)

			EVENT(msg_hitsolid)
			{
				LuaAttributeTable* pSharedAttr = ME->GetSharedAttributes();
				ME->m_bHitSolid = true;

				if(pSharedAttr->GetString("SfxHitEnvironment") != "")
				{
					//AudioHelper::PlaySound(pSharedAttr->GetString("SfxHitEnvironment").c_str());
				}

				if(pSharedAttr->GetString("PfxHitEnvironment") != "")
				{
					FXHelper::Pfx_CreateStatic(pSharedAttr->GetString("PfxHitEnvironment").c_str(), ME, "ROOT");
				}

				if(ME->m_pOther != NULL)
				{
					ME->m_pOther->SetExitOnMovementDone(true);
				}

				SET_STATE(INACTIVE);
			}
			END_EVENT(true)

			EVENT(msg_hitragdoll)
			{
				LuaAttributeTable* pSharedAttr = ME->GetSharedAttributes();
				ME->m_bHitRagdoll = true;

				if(pSharedAttr->GetString("SfxHitCharacter") != "")
				{
					//AudioHelper::PlaySound(pSharedAttr->GetString("SfxHitCharacter").c_str());
				}

				if(pSharedAttr->GetString("PfxHitCharacter") != "")
				{
					FXHelper::Pfx_CreateStatic(pSharedAttr->GetString("PfxHitCharacter").c_str(), ME, "ROOT");
				}

				if(ME->m_pOther != NULL)
				{
					ME->m_pOther->SetExitOnMovementDone(true);
				}

				SET_STATE(INACTIVE);
			}
			END_EVENT(true)

			EVENT(msg_collision)
			{
				//CEntity *pColEnt = msg.GetEnt("Collidee");
				//ntAssert(pColEnt != NULL);
				//ntPrintf("Spear collided with %s\n", pColEnt->GetName().c_str());

				//ntPrintf("Spear collision -> Throwstate\n");

				LuaAttributeTable *pSharedAttr = ME->GetSharedAttributes();

				if(ME->m_bCollision == false)
				{
					ME->GetPhysicsSystem()->Lua_Spear_StopThrownBehaviour();
					ME->m_bCollision = true;
				}

				if((msg.GetFloat("ProjVel") > -pSharedAttr->GetNumber("ImpactThreshold")) &&
					(msg.GetFloat("ProjVel") < pSharedAttr->GetNumber("ImpactThreshold")))
				{
					END_EVENT(true);
				}

				if(pSharedAttr->GetString("PfxHitEnvironment") != "")
				{
					FXHelper::Pfx_CreateStatic(pSharedAttr->GetString("PfxHitEnvironment").c_str(), ME, "ROOT");	//Spawn a particle effect.
				}
			}
			END_EVENT(true)

			EVENT(msg_aim_on)
			{
				ME->m_bAiming = true;
			}
			END_EVENT(true)

			EVENT(msg_aim_off)
			{
				ME->m_bAiming = false;
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	STATE(AFTERTOUCH)
		BEGIN_EVENTS
			ON_ENTER
			{
				//ntPrintf("Spear -> Aftertouch State\n");
				LuaAttributeTable* pSharedAttr = ME->GetSharedAttributes();

				//(Lua global function stuff).
				if(pSharedAttr->GetString("OnAftertouchStart") != "")
				{
					CLuaGlobal::CallLuaFunc(pSharedAttr->GetString("OnAftertouchStart").c_str(), ME);
				}

/*
//TODO: This is just block-comment in lua right? so I can remove it.
				--[[
				this.attrib.HitSolid = false
				this.attrib.HitRagdoll = false
				this.attrib.Collision = false
				]]--
*/

				//Enter our aftertouch controller.
				ME->m_nThrownCameraHandle = CamMan::Get().GetView(0)->ActivateAfterTouchCoolCam(ME, pSharedAttr->GetString("AftertouchCamProperties").c_str());
//				ntstd::String ATPs = pSharedAttr->GetString("AftertouchProperites");
//				const char* ATPChar = ATPs.c_str();
//				UNUSED(ATPChar);
				ME->GetPhysicsSystem()->Lua_Spear_SetController(ME->m_pOther, pSharedAttr->GetString("AftertouchProperites").c_str());
			}
			END_EVENT(true)

			EVENT(msg_interrupt)
			{
				//Player was interrupted whilst in aftertouch, therefore we kill cool cams, but still need to allow the spear
				//to continue it's flight and normal behaviour!

				//ntPrintf("Spear -> Aftertouch State -> msg_interrupt\n");

				LuaAttributeTable* pSharedAttr = ME->GetSharedAttributes();
				//(Lua global function stuff).
				if(pSharedAttr->GetString("OnAftertouchEnd") != "")
				{
					CLuaGlobal::CallLuaFunc(pSharedAttr->GetString("OnAftertouchEnd").c_str(), ME);
				}

				CamMan::Get().GetView(0)->RemoveAllCoolCameras();
				SET_STATE(INFLIGHT);
			}
			END_EVENT(true)

			EVENT(msg_attack_off)
			EVENT(msg_action_off)
			EVENT(msg_power_off)
			{
				LuaAttributeTable* pSharedAttr = ME->GetSharedAttributes();
				//(Lua global function stuff).
				if(pSharedAttr->GetString("OnAftertouchEnd") != "")
				{
					CLuaGlobal::CallLuaFunc(pSharedAttr->GetString("OnAftertouchEnd").c_str(), ME);
				}
				
				CamMan::Get().GetView(0)->RemoveAllCoolCameras();
				
				//Player can return to their default state
				Message msgExitState(msg_exitstate);
				ME->m_pOther->GetMessageHandler()->QueueMessage(msgExitState);

				ME->m_pOther = NULL;

				ME->GetMovement()->ClearControllers();

				if(ME->m_bHitSolid == true)
				{
					SET_STATE(INACTIVE);
				}
				else
				{
					SET_STATE(INFLIGHT);
				}
			}
			END_EVENT(true)


			EVENT(msg_removeragdoll)
			{
				ME->m_bHitRagdoll = false;
			}
			END_EVENT(true)

			EVENT(msg_hitsolid)
			{
				ME->GetMovement()->ClearControllers();

				ME->m_bHitSolid = true;

				LuaAttributeTable* pSharedAttr = ME->GetSharedAttributes();
				if(pSharedAttr->GetString("SfxHitEnvironment") != "")
				{
					//AudioHelper::PlaySound(pSharedAttr->GetString("SfxHitEnvironment").c_str());
				}

				if(pSharedAttr->GetString("PfxHitEnvironment") != "")
				{
					FXHelper::Pfx_CreateStatic(pSharedAttr->GetString("PfxHitEnvironment").c_str(), ME, "ROOT");
				}
			}
			END_EVENT(true)

			EVENT(msg_hitragdoll)
			{
				ME->m_bHitRagdoll = true;

				LuaAttributeTable* pSharedAttr = ME->GetSharedAttributes();
				if(pSharedAttr->GetString("SfxHitCharacter") != "")
				{
					//AudioHelper::PlaySound(pSharedAttr->GetString("SfxHitCharacter").c_str());
				}

				if(pSharedAttr->GetString("PfxHitCharacter") != "")
				{
					FXHelper::Pfx_CreateStatic(pSharedAttr->GetString("PfxHitCharacter").c_str(), ME, "ROOT");
				}
			}
			END_EVENT(true)

			EVENT(msg_collision)
			{
				//ntPrintf("Spear collision -> Aftertouch state -> msg_collision\n");
				LuaAttributeTable* pSharedAttr = ME->GetSharedAttributes();

				if(ME->m_bCollision == false)
				{
					ME->GetPhysicsSystem()->Lua_Spear_StopThrownBehaviour();
					ME->GetMovement()->ClearControllers();
					ME->m_bCollision = true;
				}

				//Make sure the object was travelling above the threshold velocity.
				if((msg.GetFloat("ProjVel") > -pSharedAttr->GetNumber("ImpactThreshold")) &&
					(msg.GetFloat("ProjVel") < pSharedAttr->GetNumber("ImpactThreshold")))
				{
					END_EVENT(true);
				}

				if(pSharedAttr->GetString("PfxHitEnvironment") != "")
				{
					FXHelper::Pfx_CreateStatic(pSharedAttr->GetString("PfxHitEnvironment").c_str(), ME, "ROOT");	//Spawn a particle effect.
				}
			}
			END_EVENT(true)

			EVENT(msg_aim_on)
			{
				ME->m_bAiming = true;
			}
			END_EVENT(true)

			EVENT(msg_aim_off)
			{
				ME->m_bAiming = false;
			}
			END_EVENT(true);

		END_EVENTS
	END_STATE

	STATE(INFLIGHT)
		BEGIN_EVENTS
			ON_ENTER
			{
				//Spear is in flight with no influence from any other entity.
				//It can exit this state either when it sticks in a wall or when it is at rest.

				//ntPrintf("Spear -> Inflight state\n");
			}
			END_EVENT(true)

			EVENT(msg_atrest)
			{
				SET_STATE(DEFAULT);
			}
			END_EVENT(true)

			EVENT(msg_removeragdoll)
			{
				ME->m_bHitRagdoll = false;
			}
			END_EVENT(true)

			EVENT(msg_hitsolid)
			{
				LuaAttributeTable* pSharedAttr = ME->GetSharedAttributes();
				if(pSharedAttr->GetString("SfxHitEnvironment") != "")
				{
					//AudioHelper::PlaySound(pSharedAttr->GetString("SfxHitEnvironment").c_str());
				}

				if(pSharedAttr->GetString("PfxHitEnvironment") != "")
				{
					FXHelper::Pfx_CreateStatic(pSharedAttr->GetString("PfxHitEnvironment").c_str(), ME, "ROOT");
				}

				SET_STATE(INACTIVE);
			}
			END_EVENT(true)

			EVENT(msg_hitragdoll)
			{
				LuaAttributeTable* pSharedAttr = ME->GetSharedAttributes();
				if(pSharedAttr->GetString("SfxHitCharacter") != "")
				{
					//AudioHelper::PlaySound(pSharedAttr->GetString("SfxHitCharacter").c_str());
				}

				if(pSharedAttr->GetString("PfxHitCharacter") != "")
				{
					FXHelper::Pfx_CreateStatic(pSharedAttr->GetString("PfxHitCharacter").c_str(), ME, "ROOT");
				}

				SET_STATE(INACTIVE);
			}
			END_EVENT(true)

			EVENT(msg_collision)
			{
				LuaAttributeTable* pSharedAttr = ME->GetSharedAttributes();
				if(ME->m_bCollision == false)
				{
					ME->GetPhysicsSystem()->Lua_Spear_StopThrownBehaviour();
					ME->GetPhysicsSystem()->Lua_Spear_GenerateAtRestMessage();
					ME->m_bCollision = true;
				}

				//Make sure the object was travelling above the threshold velocity.
				if((msg.GetFloat("ProjVel") > -pSharedAttr->GetNumber("ImpactThreshold")) &&
					(msg.GetFloat("ProjVel") < pSharedAttr->GetNumber("ImpactThreshold")))
				{
					END_EVENT(true);
				}

				if(pSharedAttr->GetString("PfxHitEnvironment") != "")
				{
					FXHelper::Pfx_CreateStatic(pSharedAttr->GetString("PfxHitEnvironment").c_str(), ME, "ROOT");	//Spawn a particle effect.
				}
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	STATE(DROP)
		BEGIN_EVENTS
			EVENT(msg_think_ondrop)
			{
				LuaAttributeTable* pSharedAttr = ME->GetSharedAttributes();

				ME->GetInteractionComponent()->AllowCollisionWith(ME->m_pOther);	//Disallow collision primitive overlap between player and object.
				ME->Lua_ReparentToWorld();	//Reparent the object back to the world.
				ME->GetPhysicsSystem()->Lua_Spear_SetMotionType(Physics::HS_MOTION_DYNAMIC);	//Allow havok to take control of the object

				//Set the velocity of the object relative to the player's direction OR towards their current throw target
				CDirection vDropVelocity(pSharedAttr->GetVector("DropVelocity"));
				CDirection vVelocity = vDropVelocity * ME->m_pOther->GetMatrix();
				ME->GetPhysicsSystem()->Lua_AltSetLinearVelocity(vVelocity);

				ME->GetPhysicsSystem()->Lua_Spear_GenerateAtRestMessage();	//Make sure the rigid body generates an AtRest message.
			}
			END_EVENT(true)

			ON_ENTER
			{
				//ntPrintf("Spear -> Dropstate\n");
				LuaAttributeTable* pSharedAttr = ME->GetSharedAttributes();
				CamMan::Get().GetView(0)->RemoveAllCoolCameras();
				ME->m_pOther->GetMovement()->Lua_AltStartFacingMovement(pSharedAttr->GetString("AnimPlayerDrop").c_str(), 360.0f, 1.0f, 0.0f, 0.0f, 0.0f);
				ME->m_pOther->GetMovement()->Lua_AltSetMovementCompleteMessage("msg_movementdone", ME->m_pOther);

				//Tell the player to automatically return to default state when their movement is completed
				ME->m_pOther->SetExitOnMovementDone(true);
				Message msgOnDrop(msg_think_ondrop);
				ME->GetMessageHandler()->QueueMessageDelayed(msgOnDrop, pSharedAttr->GetNumber("DropTime"));
			}
			END_EVENT(true)

			EVENT(msg_interrupt)
			{
				SET_STATE(INTERRUPT);
			}
			END_EVENT(true)

			EVENT(msg_atrest)
			{
				SET_STATE(DEFAULT);
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	STATE(INTERRUPT)
		BEGIN_EVENTS
			ON_ENTER
			{
				//ntPrintf("Spear -> Interrupt state\n");

				//Kill any animations that might be playing on this object.
				ME->GetAnimator()->RemoveAllAnimations();

				ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);
				ME->Lua_ReparentToWorld();	//Reparent the object back to the world.

				ME->GetPhysicsSystem()->Lua_Spear_SetMotionType(Physics::HS_MOTION_DYNAMIC);	//Allow havok to take control of the object again.

				if(ME->m_pOther != NULL)
				{
					ME->GetInteractionComponent()->AllowCollisionWith(ME->m_pOther);	//Allow object and player to collide again.
					ME->m_pOther = NULL;
				}

				ME->GetPhysicsSystem()->Lua_Spear_GenerateAtRestMessage();	//make sure the rigid body generates an AtRest message.
			}
			END_EVENT(true)

			EVENT(msg_atrest)
			{
				SET_STATE(DEFAULT);
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	STATE(INACTIVE)
		BEGIN_EVENTS
			ON_ENTER
			{
				//ntPrintf("Spear -> Inactive state\n");
				ME->m_pOther = NULL;
				ME->GetAnimator()->RemoveAllAnimations();
				ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	STATE(DESTROY)			//Spears are currently indestructable.
		BEGIN_EVENTS
			EVENT(msg_think_onremoveobject)
			{
				if(ME->m_pOther != NULL)
				{
					Message msgExitState(msg_exitstate);
					ME->m_pOther->GetMessageHandler()->QueueMessage(msgExitState);
					ME->m_pOther = NULL;

					CamMan::Get().GetView(0)->RemoveAllCoolCameras();
				}

				ME->Lua_RemoveSelfFromWorld();
			}
			END_EVENT(true)

			ON_ENTER
			{
				LuaAttributeTable* pSharedAttr = ME->GetSharedAttributes();

				ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);
				ME->Lua_ReparentToWorld();	//Reparent the object back to the world.
				ME->GetPhysicsSystem()->Deactivate();

				ME->m_pOther->GetPhysicsSystem()->Lua_RemoveChildEntities();	//Destroy any projectiles that may be parented to this object.
				ME->GetRenderableComponent()->AddRemoveAll_Game( false );	//Hide this object.

				ME->GetAnimator()->RemoveAllAnimations();	//Kill any anims that might be playing on this object.

				if(pSharedAttr->GetString("PfxDestroyed") != "")
				{
					FXHelper::Pfx_CreateStatic(pSharedAttr->GetString("PfxDestroyed").c_str(), ME, "ROOT");
				}

				if(pSharedAttr->GetString("SfxDestroy") != "")
				{
					//AudioHelper::PlaySound(pSharedAttr->GetString("SfxDestroy").c_str());
				}

				Message OnRemoveMessage(msg_think_onremoveobject);
				ME->GetMessageHandler()->QueueMessageDelayed(OnRemoveMessage, 1.0f);
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE

END_STATEMACHINE //INTERACTABLE_SPEAR_FSM


//--------------------------------------------------
//!
//!	Interactable_Spear::Interactable_Spear()
//!	Default constructor
//!
//--------------------------------------------------
Interactable_Spear::Interactable_Spear()
{
	m_eType = 				EntType_Interactable;
	m_eInteractableType = 	EntTypeInteractable_Spear;

	m_pSharedAttributesPtr = 0;
	m_pSharedAttributes = 0;

	m_pOther = NULL;
	m_nThrownCameraHandle = 0;
	m_bAiming = false;
	m_bDoQuickThrow = false;
	m_bCheckQuickThrow = false;
	
	m_bHitRagdoll = false;
	m_bHitSolid = false;
	m_bCollision = false;

}

//--------------------------------------------------
//!
//!	Interactable_Spear::OnPostConstruct()
//!	Post Construct
//!
//--------------------------------------------------
void Interactable_Spear::OnPostConstruct()
{
	Interactable::OnPostConstruct();

	//Add components.
	InstallMessageHandler();
	InstallAnimator(m_AnimationContainer);
	//InstallDynamics();

	// Create rigid body physics
	Lua_CreatePhysicsSystem();


	//Dynamics components.
	m_pSharedAttributes = NT_NEW LuaAttributeTable;
	m_pSharedAttributes->SetDataObject(ObjectDatabase::Get().GetDataObjectFromPointer(m_pSharedAttributesPtr));

	hkRigidBodyCinfo obInfo;

	obInfo.m_motionType = hkMotion::MOTION_DYNAMIC;
	obInfo.m_qualityType = HK_COLLIDABLE_QUALITY_MOVING;							
	obInfo.m_maxLinearVelocity = m_pSharedAttributes->GetNumber("MaxLinearVelocity");
	obInfo.m_maxAngularVelocity = m_pSharedAttributes->GetNumber("MaxAngularVelocity");

	NinjaLua::LuaObject LuaMotionType = m_pSharedAttributes->GetAttribute("MotionType");
	if (LuaMotionType.IsString())
	{
		if (strcmp(	LuaMotionType.GetString(), "FIXED" )==0)
		{
			obInfo.m_motionType = hkMotion::MOTION_FIXED;
			obInfo.m_qualityType = HK_COLLIDABLE_QUALITY_FIXED;
		}
		else if (strcmp(	LuaMotionType.GetString(), "KEYFRAMED" )==0)
		{
			obInfo.m_motionType = hkMotion::MOTION_KEYFRAMED;
			obInfo.m_qualityType = HK_COLLIDABLE_QUALITY_KEYFRAMED;
		}
		else if (strcmp(	LuaMotionType.GetString(), "DYNAMIC" )==0)
		{
			obInfo.m_motionType = hkMotion::MOTION_DYNAMIC;
			obInfo.m_qualityType = HK_COLLIDABLE_QUALITY_MOVING;
		}
	}

	// 2 - Create a group
	//Physics::LogicGroup * lg = (Physics::LogicGroup *)Physics::ClumpTools::ConstructSpearLGFromClump( this, &obInfo );
	Physics::SpearLG* lg = NT_NEW Physics::SpearLG(GetName(), this);
	lg->Load();
	// 3 - Create a system
	if(lg)
	{
		// Add the group
		m_pobPhysicsSystem->AddGroup( (Physics::LogicGroup *) lg );
		lg->Activate();	
	}
	else	
		ntPrintf("%s(%d): ### PHYSICS ERROR - Logic group not created for entity %s with clump %s\n", __FILE__, __LINE__, GetName().c_str(), GetClumpString().c_str());	

	//Register physics sound.
	ntstd::String PhysicsSoundDef = m_pSharedAttributes->GetString("PhysicsSoundDef");

	if (strcmp(PhysicsSoundDef.c_str(), "") != 0)
	{
		m_pobPhysicsSystem->RegisterCollisionEffectFilterDef(PhysicsSoundDef.c_str());
	}

	// Create and attach the statemachine
	INTERACTABLE_SPEAR_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) INTERACTABLE_SPEAR_FSM();
	ATTACH_FSM(pFSM);
}

//--------------------------------------------------
//!
//!	Interactable_Spear::~Interactable_Spear()
//!	Default destructor
//!
//--------------------------------------------------
Interactable_Spear::~Interactable_Spear()
{
	NT_DELETE(m_pSharedAttributes);
}

