//--------------------------------------------------
//!
//!	\file game/entityrangedweapon.cpp
//!	Definition of the ranged weapon object
//!
//--------------------------------------------------

#include "objectdatabase/dataobject.h"
#include "game/luaattrtable.h"
#include "Physics/system.h"
#include "physics/singlerigidlg.h"
#include "game/movement.h"
#include "anim/animator.h"
#include "core/exportstruct_anim.h"
#include "messagehandler.h"
#include "effect/fxhelper.h"
#include "audio/audiohelper.h"
#include "game/combathelper.h"
#include "game/renderablecomponent.h"
#include "camera/camman.h"
#include "camera/camview.h"
#include "game/entitymanager.h"
#include "game/entityplayer.h"
#include "game/interactiontransitions.h"
#include "hud/hudmanager.h"
#include "hud/objectivemanager.h"

#include "game/entityprojectile.h"
#include "game/entityrangedweapon.h"

// Components needed
#include "game/interactioncomponent.h"
#include "game/aicomponent.h"

void ForceLinkFunctionRangedWeapon()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionRangedWeapon() !ATTN!\n");
}

START_CHUNKED_INTERFACE(Att_RangedWeapon, Mem::MC_ENTITY)
	PUBLISH_VAR_AS     (m_fMass, Mass)
	PUBLISH_VAR_AS     (m_obCenterOfMass, CenterOfMass)
	PUBLISH_VAR_AS     (m_fRestitution, Restitution)
	PUBLISH_VAR_AS     (m_fFriction, Friction)
	PUBLISH_VAR_AS     (m_fLinearDamping, LinearDamping)
	PUBLISH_VAR_AS     (m_fAngularDamping, AngularDamping)
	PUBLISH_VAR_AS     (m_fMaxLinearVelocity, MaxLinearVelocity)
	PUBLISH_VAR_AS     (m_fMaxAngularVelocity, MaxAngularVelocity)
	PUBLISH_VAR_AS     (m_obPhysicsSoundDef, PhysicsSoundDef)
	PUBLISH_VAR_AS     (m_fImpactThreshold, ImpactThreshold)
	PUBLISH_VAR_AS     (m_iHitCount, HitCount)
	PUBLISH_VAR_AS     (m_bDamageOnChar, DamageOnChar)
	PUBLISH_VAR_AS     (m_bDamageOnEnv, DamageOnEnv)
	PUBLISH_VAR_AS     (m_obThrownAttackData, ThrownAttackData)		
	PUBLISH_VAR_AS     (m_iAmmo, Ammo)
	PUBLISH_VAR_AS     (m_bReloadAfterShot, ReloadAfterShot)
	PUBLISH_PTR_AS     (m_pobProjectileDef, ProjectileDef)
	PUBLISH_VAR_AS     (m_obTranslationOffset, TranslationOffset)	
	PUBLISH_VAR_AS     (m_fThrowTime, ThrowTime)
	PUBLISH_VAR_AS     (m_fDropTime, DropTime)
	PUBLISH_VAR_AS     (m_obThrowVelocity, ThrowVelocity)
	PUBLISH_VAR_AS     (m_obThrowAngularVelocity, ThrowAngularVelocity)
	PUBLISH_VAR_AS     (m_obDropVelocity, DropVelocity)
	PUBLISH_VAR_AS     (m_bAftertouchOnThrow, CanAftertouchOnThrow)
	PUBLISH_VAR_AS     (m_fAfterTouchWaitTime, AftertouchWaitTime)
	PUBLISH_VAR_AS     (m_obAnimObjectShoot, AnimObjectShoot)
	PUBLISH_VAR_AS     (m_obAnimObjectReload, AnimObjectReload)
	PUBLISH_VAR_AS     (m_obAnimPlayerMoveTo, AnimPlayerMoveTo)
	PUBLISH_VAR_AS     (m_obAnimPlayerRunTo, AnimPlayerRunTo)
	PUBLISH_VAR_AS     (m_obAnimPlayerPickup, AnimPlayerPickup)
	PUBLISH_VAR_AS     (m_obAnimPlayerRunPickup, AnimPlayerRunPickup)
	PUBLISH_VAR_AS     (m_obAnimPlayerShoot, AnimPlayerShoot)
	PUBLISH_VAR_AS     (m_obAnimPlayerReload, AnimPlayerReload)
	PUBLISH_VAR_AS     (m_obAnimPlayerThrow, AnimPlayerThrow)
	PUBLISH_VAR_AS     (m_obAnimPlayerDrop, AnimPlayerDrop)
	PUBLISH_VAR_AS     (m_obAnimIdleToAim, AnimIdleToAim)
	PUBLISH_VAR_AS     (m_obAnimAimToIdle, AnimAimToIdle)
	PUBLISH_PTR_AS     (m_pobPlayerHoldingMovement, PlayerHoldingMovement)
	PUBLISH_PTR_AS     (m_pobPlayerAimingMovement, PlayerAimingMovement)
	PUBLISH_PTR_AS     (m_pobPlayerThrowMovement, PlayerThrowMovement)
	PUBLISH_PTR_AS     (m_pobPlayerShootMovement, PlayerShootMovement)
	PUBLISH_PTR_AS     (m_pobAftertouchProperties, AftertouchProperties)
	PUBLISH_PTR_AS     (m_pobAftertouchCamProperties, AftertouchCamProperties)
	PUBLISH_PTR_AS     (m_pobChasecamProperties, ChasecamProperties)
	PUBLISH_PTR_AS     (m_pobAimcamProperties, AimcamProperties)
	PUBLISH_VAR_AS     (m_obPfxImpact, PfxImpact)
	PUBLISH_VAR_AS     (m_obPfxDestroyed, PfxDestroyed)
	PUBLISH_VAR_AS     (m_obSfxDestroyed, SfxDestroyed)
	PUBLISH_VAR_AS     (m_obOnAftertouchStart, OnAftertouchStart)
	PUBLISH_VAR_AS     (m_obOnAftertouchEnd, OnAftertouchEnd)
	IENUM			   (Att_RangedWeapon, RangedWeaponType, RANGED_WEAPON_TYPE )
END_STD_INTERFACE

START_CHUNKED_INTERFACE(Object_Ranged_Weapon, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(Interactable)
	COPY_INTERFACE_FROM(Interactable)

	OVERRIDE_DEFAULT(Orientation, "0.0,0.0,0.0,-1.0")

	PUBLISH_VAR_AS(m_Description, Description)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_InitialState, "DefaultState", InitialState)
	PUBLISH_VAR_AS(m_AnimationContainer, AnimationContainer)
	PUBLISH_PTR_AS(m_pSharedAttributes, SharedAttributes)

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )
END_STD_INTERFACE


//--------------------------------------------------
//!
//! Ranged Weapon State Machine
//!
//--------------------------------------------------
STATEMACHINE(RANGED_WEAPON_FSM, Object_Ranged_Weapon)
	RANGED_WEAPON_FSM()
	{
		SET_INITIAL_STATE(DEFAULT);
	}

	STATE(DEFAULT)
		BEGIN_EVENTS
			ON_ENTER
			{
				ME->m_pOther = 0;
				ME->m_bAiming = false;
				ME->m_bShotFired = false;
				ME->m_bMovementDone = false;
				ME->m_bAtRest = false;

				ME->GetInteractionComponent()->Lua_SetInteractionPriority(PICKUP);
			}
			END_EVENT(true)

			EVENT(msg_action)
			{
				ME->m_pOther = (Character*)msg.GetEnt("Other");
				ntAssert(ME->m_pOther != 0 && ME->m_pOther->IsCharacter());	//It should be the entity that sent this message.
				//Choose an appropriate move-to state depending on whether this is the player or an AI character.
				if(!ME->m_pOther->IsPlayer())
				{
					SET_STATE(AI_MOVETO);
				}
				else
				{
					SET_STATE(MOVETO);
				}
			}
			END_EVENT(true)

			EVENT(msg_running_action)
			{
				ME->m_pOther = (Character*)msg.GetEnt("Other");
				ntAssert(ME->m_pOther != 0 && ME->m_pOther->IsCharacter());	//It should be the entity that sent this message
				//Choose an appropriate run-to state depending on whether this is the player or an AI character.
				if(!ME->m_pOther->IsPlayer())
				{
					SET_STATE(AI_RUNTO);
				}
				else
				{
					SET_STATE(RUNTO);
				}
			}
			END_EVENT(true)

			EVENT(msg_equip)
			{
				ME->m_pOther = (Character*)msg.GetEnt("Other");
				ntError_p(ME->m_pOther, ("No Other character specified for equip message\n"));

				if(ME->m_pOther->IsPlayer())
				{
					SET_STATE(PLAYER_EQUIP);
				}
				else
				{
					SET_STATE(AI_EQUIP);
				}
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	STATE(MOVETO)
		BEGIN_EVENTS
			ON_ENTER
			{
				CEntity* player = ME->m_pOther;
				Att_RangedWeapon* pSharedAttr = ME->GetSharedAttributes();
				
				//Player moves into position before starting the pickup.
				player->GetMovement()->Lua_StartMoveToTransition(pSharedAttr->m_obAnimPlayerMoveTo, ME, 1, 1);
				player->GetMovement()->SetCompletionMessage("msg_movementdone");
				player->GetMovement()->SetInterruptMessage("msg_interrupt");

				//Change our priority (only one person can use this at a time).
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
				CEntity* player = ME->m_pOther;
				Att_RangedWeapon* pSharedAttr = ME->GetSharedAttributes();

				//Player moves into position before starting the pickup.
				player->GetMovement()->Lua_StartMoveToTransition(pSharedAttr->m_obAnimPlayerRunTo, ME, 1, 1);
				player->GetMovement()->SetCompletionMessage("msg_movementdone");
				player->GetMovement()->SetInterruptMessage("msg_interrupt");

				//Set our interaction priority to none (only one person can use this at a time).
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
			ON_ENTER
			{
				CEntity* player = ME->m_pOther;
				Att_RangedWeapon* pSharedAttr = ME->GetSharedAttributes();

				//Player starts their pickup anim.
				player->GetMovement()->Lua_AltStartFacingMovement(pSharedAttr->m_obAnimPlayerPickup, 360.0f, 1.0f, 0.0f, 0.0f, 0.0f);
				player->GetMovement()->SetCompletionMessage("msg_movementdone");
				player->GetMovement()->SetInterruptMessage("msg_interrupt");

				//Object plays it's pickup animation relative to the player's root.
				ME->GetInteractionComponent()->ExcludeCollisionWith(player);	//Allow the object and player collision primitives to overlap
				ME->GetPhysicsSystem()->Lua_Rigid_SetKeyframedMotion(true);		//Take control away from havok
				//We no-longer set to identity, but instead set to held-matrix.
//				ME->Lua_SetIdentity();
				Transform* pobTransform = ME->GetHierarchy()->GetRootTransform();
				ntError_p(pobTransform, ("Could not get root node transform on ranged weapon during AI_EQUIP"));
				if(pobTransform)
				{
					pobTransform->SetLocalMatrix(ME->GetHeldMatrix());
				}
				ME->Lua_Reparent(player, "r_weapon");
			}
			END_EVENT(true)

			EVENT(msg_interrupt)
			{
				SET_STATE(INTERRUPT);
			}
			END_EVENT(true)

			EVENT(msg_movementdone)
			{
/*
TODO: is this really not needed anymore? I think it was cut because AI can pick them up now, but might be needed for player still.
				local player=this.attrib.Other
				local params=this.attrib.SharedAttributes
				--Views.GetPrimary().ActivateChaseAimCam(player, params.ChasecamProperties, params.AimcamProperties)
*/
				SET_STATE(HELD);
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	STATE(RUNPICKUP)
		BEGIN_EVENTS
			ON_ENTER
			{
				CEntity* player = ME->m_pOther;
				Att_RangedWeapon* pSharedAttr = ME->GetSharedAttributes();

				//Player starts their pickup animation.
				player->GetMovement()->Lua_AltStartFacingMovement(pSharedAttr->m_obAnimPlayerRunPickup, 360.0f, 1.0f, 0.0f, 0.0f, 0.0f);
				player->GetMovement()->SetCompletionMessage("msg_movementdone");
				player->GetMovement()->SetInterruptMessage("msg_interrupt");

				//Object plays it's pickup animation relative to the player's root.
				ME->GetInteractionComponent()->ExcludeCollisionWith(player);	//Allow the object and player collision primitives to overlap.
				ME->GetPhysicsSystem()->Lua_Rigid_SetKeyframedMotion(true);		//Take control away from havok.
				//We no-longer set to identity, but instead set to held-matrix.
//				ME->Lua_SetIdentity();
				Transform* pobTransform = ME->GetHierarchy()->GetRootTransform();
				ntError_p(pobTransform, ("Could not get root node transform on ranged weapon during AI_EQUIP"));
				if(pobTransform)
				{
					pobTransform->SetLocalMatrix(ME->GetHeldMatrix());
				}
				ME->Lua_Reparent(player, "r_weapon");	//Reparent the weapon to the right weapon transform on the character.
			}
			END_EVENT(true)

			EVENT(msg_interrupt)
			{
				SET_STATE(INTERRUPT);
			}
			END_EVENT(true)

			EVENT(msg_movementdone)
			{
/*
TODO: is this really not needed anymore? I think it was cut because AI can pick them up now, but might be needed for player still.
				local player=this.attrib.Other
				local params=this.attrib.SharedAttributes
				--Views.GetPrimary().ActivateChaseAimCam(player, params.ChasecamProperties, params.AimcamProperties)
*/
				SET_STATE(HELD);
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	STATE(PLAYER_EQUIP)
		BEGIN_EVENTS
			ON_ENTER
			{
				CEntity* player = ME->m_pOther;
				CEntity* weapon = ME;
				ME->GetInteractionComponent()->ExcludeCollisionWith(player);	//Allow the object and player to overlap.
				ME->GetPhysicsSystem()->Lua_Rigid_SetKeyframedMotion(true);		//Take control away from havok
				//We no-longer set to identity, but instead set to held-matrix.
//				ME->Lua_SetIdentity();
				Transform* pobTransform = ME->GetHierarchy()->GetRootTransform();
				ntError_p(pobTransform, ("Could not get root node transform on ranged weapon during AI_EQUIP"));
				if(pobTransform)
				{
					pobTransform->SetLocalMatrix(ME->GetHeldMatrix());
				}
				ME->Lua_Reparent(player, "r_weapon");	//Reparent the weapon to the the right-weapon transform on the character.
				weapon = ME;

				ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);	//One user at a time.

				SET_STATE(HELD);
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	STATE(HELD)
		BEGIN_EVENTS
			ON_ENTER
			{
				CEntity*			pEntity			= ME->m_pOther;
				Att_RangedWeapon*	pSharedAttr		= ME->GetSharedAttributes();

				//Player begins their holding movement, chase cam is activated.	//TODO: Should it be activated still?
				//pEntity->GetPhysicsSystem()->Lua_SetHoldingCapsule(true);

				// If the owner entity is a player...
				if( pEntity->IsPlayer() )
				{
					if( ME->m_bDisableAiming )
					{
						//Send a message saying that we have successfully picked up the weapon.
						Message msg(msg_control_end);
						pEntity->GetMessageHandler()->QueueMessage(msg);

						ME->m_bDisableAiming = false;
						ME->m_bAiming = false;

						// Put the entity into their movement component
						pEntity->GetMovement()->BringInNewController(*pSharedAttr->m_pobPlayerHoldingMovement, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND);
					}
					// ... and if the entity is in an IDLE state
					else if( pEntity->ToPlayer()->GetState() == Player::ARC_IDLE )
					{
						// Put the entity into their movement component
						pEntity->GetMovement()->BringInNewController(*pSharedAttr->m_pobPlayerHoldingMovement, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND);
						
						//Send a message saying that we have successfully picked up the weapon.
						Message msgWeaponPickupSuccess(msg_weaponpickup_success);
						pEntity->GetMessageHandler()->QueueMessage(msgWeaponPickupSuccess);
					}
				}
				else
				{
					pEntity->GetMovement()->Lua_StartEmptyMovement();

					if(ME->m_bAiming == false)
					{
						pEntity->GetMovement()->BringInNewController(*pSharedAttr->m_pobPlayerHoldingMovement, CMovement::DMM_STANDARD, 0.0f);
						pEntity->Lua_ResetAimingComponent();
					}
					else
					{
						pEntity->GetMovement()->BringInNewController(*pSharedAttr->m_pobPlayerAimingMovement, CMovement::DMM_STANDARD, 0.0f);
					}
				}

				//Ensure all anims are finished.
				ME->GetAnimator()->RemoveAllAnimations();

				if( ME->m_bAimingEnableRequest )
				{
					// :D Evil!!!
					goto ENABLE_AIMING_ON;
				}

			}
			END_EVENT(true)

			EVENT(msg_interrupt)
			{
				SET_STATE(INTERRUPT);
			}
			END_EVENT(true)

			//------------------------------------------------
			// Power now fires the weapon
			//------------------------------------------------
			EVENT(msg_action_on)
			EVENT(msg_attack_on)
			EVENT(msg_power_on)
			{
				if(ME->m_iAmmo > 0)
				{
					SET_STATE(SHOOT);
				}
				else
				{
					SET_STATE(THROW);
				}
			}
			END_EVENT(true)

			EVENT(msg_grab_on)
			{
				SET_STATE(DROP);
			}
			END_EVENT(true)

			//------------------------------------------------
			// Handle going into and out of aiming mode
			//------------------------------------------------
			EVENT(msg_aim_on)
			{
ENABLE_AIMING_ON:
				CEntity*			player		= ME->m_pOther;
				Att_RangedWeapon*	pSharedAttr = ME->GetSharedAttributes();

				//Player begins their aiming movement.
				if(player && player->GetMovement() && pSharedAttr)
				{
					player->Lua_ResetAimingComponent();
					
					if( pSharedAttr->m_obAnimIdleToAim.IsNull() )
					{
						bool bNewContorllerInstalled = player->GetMovement()->BringInNewController(*pSharedAttr->m_pobPlayerAimingMovement, CMovement::DMM_STANDARD, 0.0f);
						UNUSED( bNewContorllerInstalled );
						ntError( bNewContorllerInstalled );
					}
					else
					{
						// Play the single animation to blend to
						FacingTransitionDef obDef;

						obDef.m_bApplyGravity		= true;
						obDef.m_fAngularSpeed		= 0.0f; 
						obDef.m_fEarlyOut			= 0.0f; 
						obDef.m_obAnimationName		= pSharedAttr->m_obAnimIdleToAim;
						obDef.m_fAnimSpeed			= 1.0f;
						obDef.m_fStartTurnControl	= 0.0f; 
						obDef.m_fEndTurnControl		= 0.0f; 
						obDef.m_fAlignToThrowTarget = 0.0f;

						player->GetMovement()->BringInNewController(obDef, CMovement::DMM_STANDARD, 0.0f);
						player->GetMovement()->AddChainedController( *pSharedAttr->m_pobPlayerAimingMovement, CMovement::DMM_STANDARD, 0.0f );
					}
				}
				else
				{
					//For debug purposes, check if on-shutdown we get here with no player pointer.
					ntPrintf("Assert: HELD - msg_aim_on - case 1 - No 'other' entity or no movement component or attribute-table for it\n");
					ntAssert(0);
				}
//				--Views.GetPrimary().ActivateAimingMode()	//TODO: Port if it needs to be uncommented.

				//Set aiming flag so that we know how to handle the throw.
				ME->m_bAiming = true;
				ME->m_bAimingEnableRequest = false;
			}
			END_EVENT(true)

			EVENT(msg_aim_off)
			{
				CEntity*			player		= ME->m_pOther;
				Att_RangedWeapon*	pSharedAttr = ME->GetSharedAttributes();

				if( pSharedAttr->m_obAnimAimToIdle.IsNull() )
				{
					bool bNewContorllerInstalled = player->GetMovement()->BringInNewController(*pSharedAttr->m_pobPlayerHoldingMovement, CMovement::DMM_STANDARD, 0.0f);
					UNUSED( bNewContorllerInstalled );
					ntError( bNewContorllerInstalled );
				}
				else
				{
						// Play the single animation to blend to
						FacingTransitionDef obDef;

						obDef.m_bApplyGravity		= true;
						obDef.m_fAngularSpeed		= 0.0f; 
						obDef.m_fEarlyOut			= 0.0f; 
						obDef.m_obAnimationName		= pSharedAttr->m_obAnimAimToIdle;
						obDef.m_fAnimSpeed			= 1.0f;
						obDef.m_fStartTurnControl	= 0.0f; 
						obDef.m_fEndTurnControl		= 0.0f; 
						obDef.m_fAlignToThrowTarget = 0.0f;

						player->GetMovement()->BringInNewController(obDef, CMovement::DMM_STANDARD, 0.0f);
						player->GetMovement()->AddChainedController( *pSharedAttr->m_pobPlayerHoldingMovement, CMovement::DMM_STANDARD, 0.0f );
				}

				// Reset the aiming component
				player->Lua_ResetAimingComponent();

				Message msg(msg_control_end);
				ME->m_pOther->GetMessageHandler()->QueueMessage(msg);

				//Reset aiming flag.
				ME->m_bAiming = false;
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	STATE(SHOOT)
		BEGIN_EVENTS

			///-----------------------------------
			ON_ENTER
			{
				CEntity*			player			= ME->m_pOther;
				Att_RangedWeapon*	pSharedAttr		= ME->GetSharedAttributes();

				// Clear out the projectile arrays
				ME->Projectile.clear();
				ME->ProjectileName.clear();

				if( !ME->m_bAiming && ME->m_bShotFired )
				{
					SET_STATE(RELOAD);
				}
				else
				{
					if( !player->IsPlayer() || !ME->m_bAiming )
					{
						player->GetMovement()->BringInNewController(*pSharedAttr->m_pobPlayerShootMovement, CMovement::DMM_STANDARD, 0.0f );
						player->GetMovement()->SetCompletionMessage("msg_movementdone");
						player->GetMovement()->SetInterruptMessage("msg_movementdone");
					}

					if(!ntStr::IsNull(pSharedAttr->m_obAnimObjectShoot) )
					{
						ME->PlayAnim(pSharedAttr->m_obAnimObjectShoot);
					}

					// Mark the weapon that a shot has been fired. 
					ME->m_bShotFired = true;

					// Decrease the ammo count
					ME->m_iAmmo = ME->m_iAmmo - 1;

					//------------------------------------------------------------
					// TGS BAZOOKA AMMO MESSAGES
					//------------------------------------------------------------
					if ( player->IsPlayer() && ME->m_iAmmo == 0 )
					{
						Message msgSpecial(msg_special_onscreen_message);
						ME->GetMessageHandler()->QueueMessageDelayed(msgSpecial, 5.0f);
					}
					//------------------------------------------------------------
					// END OF TGS BAZOOKA AMMO MESSAGES
					//------------------------------------------------------------

					//Launch projectile 1/10th of a second later.
					Message msgSpawnProjectile(msg_think_onspawnprojectile);
					ME->GetMessageHandler()->QueueMessageDelayed(msgSpawnProjectile, 0.1f);

					// Send a message informing of the fire request
					Message msg(msg_fire);
					ME->m_pOther->GetMessageHandler()->QueueMessage(msg);
				}
			}
			END_EVENT(true)


			///-----------------------------------
			EVENT(msg_think_onaftertouch)
			{
				bool bUseAftertouch = false;

				CEntity* pPlayer = ME->m_pOther;
				const float fWaitTime = ME->GetSharedAttributes()->m_fAfterTouchWaitTime;

				if (pPlayer->Lua_IsPowerHeld(fWaitTime) || 
					pPlayer->Lua_IsAttackHeld(fWaitTime) || 
					pPlayer->Lua_IsActionHeld(fWaitTime))
				{
					bUseAftertouch = true;
				}

				if(bUseAftertouch == true)
				{
					SET_STATE(SHOOT_WITH_AFTERTOUCH);
					END_EVENT(true);
				}
			}
			END_EVENT(true)

			///-----------------------------------
			EVENT(msg_think_onspawnprojectile)
			{
				Projectile_Attributes* pProjAttrs = ME->GetProjectileAttributes();
				Object_Projectile* pProjectile = 0;

				switch(pProjAttrs->m_eType)
				{
					case PROJECTILE_TYPE_CROSSBOW_BOLT:
						pProjectile = Object_Projectile::CreateCrossbowBolt(ME, pProjAttrs);

						ntAssert_p( pProjectile, ("Failed to create crossbow projectile") );
						break;

					case PROJECTILE_TYPE_BAZOOKA_ROCKET:
						pProjectile = Object_Projectile::CreateBazookaRockets(ME, pProjAttrs);

						ntAssert_p( pProjectile, ("Failed to create bazooka projectiles") );
						break;

					case PROJECTILE_TYPE_BALLISTA_BOLT:
						//TODO: Fill once we have the static spawn function for ballista bolts.
						ntPrintf("Ballista bolt creation not yet ported\n");
						break;

					case PROJECTILE_TYPE_AGEN_SWORD:
						//pProjectile = Object_Projectile::CreateAGenSword(ME, pProjAttrs);
						ntError(0);
						break;

					default:
						ntAssert(false && "Unknown projectile type");
						break;
				}

				Message msgOnAftertouch(msg_think_onaftertouch);
				ME->GetMessageHandler()->QueueMessageDelayed(msgOnAftertouch, 0.05f);
			}
			END_EVENT(true)

			///-----------------------------------
			EVENT(msg_interrupt)
			{
				SET_STATE(INTERRUPT);
			}
			END_EVENT(true)

			///-----------------------------------
			EVENT(msg_movementdone)
			{
				ntPrintf("Ranged weapon object msg_movementdone (shootstate)\n");

				Att_RangedWeapon* pSharedAttr = ME->GetSharedAttributes();

				// 
				ME->m_bShotFired = false;
				
				if( pSharedAttr->m_bReloadAfterShot )
				{
					SET_STATE(RELOAD);
				}
				else
				{
					SET_STATE(HELD);
				}

				// 
				Message msg(msg_fire_complete);
				ME->m_pOther->GetMessageHandler()->QueueMessage(msg);
			}
			END_EVENT(true)

			///-----------------------------------
			/*
			EVENT(msg_action_on)
			{
				Att_RangedWeapon* pSharedAttr = ME->GetSharedAttributes();
				if(ME->m_bShotFired)
				{
					if(ME->m_iAmmo < 1)
					{
						SET_STATE(THROW);
						END_EVENT(true);
					}

					if(pSharedAttr->m_bReloadAfterShot)
					{
						SET_STATE(RELOAD);
					}
					else
					{
						SET_STATE(SHOOT);
					}
				}
			}
			END_EVENT(true)
			*/

			EVENT(msg_aim_on)
			{
				ME->m_bAimingEnableRequest = true;
			}
			END_EVENT(true)

			EVENT(msg_aim_off)
			{
				ME->m_bDisableAiming = true;
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	STATE(SHOOT_WITH_AFTERTOUCH)
		BEGIN_EVENTS
			ON_ENTER
			{
				CEntity* player = ME->m_pOther;
				Att_RangedWeapon* pSharedAttr = ME->GetSharedAttributes();

//Remove this bit, it's just for data when debugging/stepping-through
//				ntstd::String FuncName = pSharedAttr->GetString("OnAftertouchStart");
//				const char* pcFuncName = FuncName.c_str();
//				UNUSED(pcFuncName);
//To here.
				if(!ntStr::IsNull(pSharedAttr->m_obOnAftertouchStart) )
				{
					CLuaGlobal::CallLuaFunc(pSharedAttr->m_obOnAftertouchStart, ME);
				}

				for(size_t i = 0 ; i < ME->Projectile.size() ; i++)
				{
					Object_Projectile* pProjectile = ME->Projectile.at(i);
//					pProjectile->GetPhysicsSystem()->Lua_Projectile_EnableSteering(player);
					pProjectile->GetPhysicsSystem()->Lua_Projectile_EnableMonoSteering(player, pSharedAttr->m_pobAftertouchProperties);	//TODO: Correct? or Dual?
				}

				if(ME->Projectile.size() != 0)
				{
					//Tell the projectile to enter aftertouch state.
					Message msgProjAftertouch(msg_aftertouch);
					msgProjAftertouch.SetEnt("Sender", ME);
					ME->Projectile.at(0)->GetMessageHandler()->QueueMessage(msgProjAftertouch);
				}

				// Send a message to the host that the game has entered an aftertouch state
				{
					Message msgProjAftertouch(msg_aftertouch);
					msgProjAftertouch.SetEnt("Sender", ME);
					player->GetMessageHandler()->QueueMessage(msgProjAftertouch);
				}

			}
			END_EVENT(true)

			ON_EXIT
			// Send a message to the host that the aftertouch state has finished
			{
				CEntity* player = ME->m_pOther;
				Message msgProjAftertouch(msg_aftertouch_done);
				msgProjAftertouch.SetEnt("Sender", ME);
				player->GetMessageHandler()->QueueMessage(msgProjAftertouch);
			}
			END_EVENT(true)


			EVENT(msg_interrupt)
			{
				CEntity* player = ME->m_pOther;
				UNUSED(player);
				Att_RangedWeapon* pSharedAttr = ME->GetSharedAttributes();

//Remove this bit, it's just for data when debugging/stepping-through
//				ntstd::String FuncName = pSharedAttr->GetString("OnAftertouchEnd");
//				const char* pcFuncName = FuncName.c_str();
//				UNUSED(pcFuncName);
//To here.

				if(!ntStr::IsNull(pSharedAttr->m_obOnAftertouchEnd) )
				{
					CLuaGlobal::CallLuaFunc(pSharedAttr->m_obOnAftertouchEnd, ME);
				}

				if(ME->Projectile.size() != 0)
				{
					//Tell the projectile to exit aftertouch state.
					//JML Destroy the bazooka rockets instead!
					if(ME->GetRangedWeaponType() == RWT_BAZOOKA)
					{
						for(size_t i = 0 ; i < ME->Projectile.size() ; i++)
						{
							Object_Projectile* pProjectile = ME->Projectile.at(i);
							pProjectile->Destroy();
						}
					}
					else
					{
						//Tell the projectile to exit aftertouch state.
						Message msgProjAftertouchDone(msg_aftertouch_done);
						
						Object_Projectile* pProjectile = ME->Projectile.at(0);
						pProjectile->GetMessageHandler()->QueueMessage(msgProjAftertouchDone);

						pProjectile->GetPhysicsSystem()->Lua_Projectile_DisableSteering();
					}
				}

				//--------------------------------------------------
				// TGS BAZOOKA DROP OFF HACK - TODO - TO BE FIXED
				//--------------------------------------------------

				// Go through remaining projectiles and get the earliest drop off time from them
				float fEarliestDropOffTime = 9999999.9f;

				Projectile_Attributes* pProjAttrs = ME->GetProjectileAttributes();
				ntAssert( pProjAttrs );

				if ( pProjAttrs->m_eType == PROJECTILE_TYPE_BAZOOKA_ROCKET )
				{
					if(ME->Projectile.size() > 1)
					{
						for(size_t j = 1 ; j < ME->Projectile.size() ; j++)
						{
							if(CEntityManager::Get().FindEntity(ME->ProjectileName.at(j).c_str()) != NULL)	//DoesEntityExist(ProjectileName[j])
							{
								Object_Projectile* pProjectile = ME->Projectile.at(j);

								float fProjDropOffTime = pProjectile->BazookaGetDropOffTime();

								if ( fProjDropOffTime < fEarliestDropOffTime )
								{
									fEarliestDropOffTime = fProjDropOffTime;
								}
							}
						}
					}
				}

				//--------------------------------------------------
				// END TGS BAZOOKA DROP OFF HACK - TODO - TO BE FIXED
				//--------------------------------------------------

				//If we are disabling steering for other projectiles, we must be sure they still exist.
				if(ME->Projectile.size() > 1)
				{
					for(size_t i = 1 ; i < ME->Projectile.size() ; i++)
					{
						if(CEntityManager::Get().FindEntity(ME->ProjectileName.at(i).c_str()) != NULL)	//DoesEntityExist(ProjectileName[i])
						{
							Object_Projectile* pProjectile = ME->Projectile.at(i);
							pProjectile->GetPhysicsSystem()->Lua_Projectile_DisableSteering();

							//--------------------------------------------------
							// TGS BAZOOKA DROP OFF HACK - TODO - TO BE FIXED
							//--------------------------------------------------

							// If it's a bazooka rocket then do the aftertouch drop if passed the threshold
							if ( pProjectile->m_eProjectileType == PROJECTILE_TYPE_BAZOOKA_ROCKET )
							{
								pProjectile->BazookaDoAftertouchDrop( fEarliestDropOffTime );
							}

							//--------------------------------------------------
							// END TGS BAZOOKA DROP OFF HACK - TODO - TO BE FIXED
							//--------------------------------------------------
						}
					}
				}

				SET_STATE(INTERRUPT);
			}
			END_EVENT(true)

			EVENT(msg_power_off)
			EVENT(msg_attack_off)
			EVENT(msg_action_off)
			{
				CEntity* player = ME->m_pOther;
				UNUSED(player);
				Att_RangedWeapon* pSharedAttr = ME->GetSharedAttributes();

				// Call LUA function if there is one
				if(!ntStr::IsNull(pSharedAttr->m_obOnAftertouchEnd))
				{
					CLuaGlobal::CallLuaFunc(pSharedAttr->m_obOnAftertouchEnd, ME);
				}

				if(ME->Projectile.size() != 0)
				{
					//Tell the projectile to exit aftertouch state.
					//JML Destroy the bazooka rockets instead!
					if(ME->GetRangedWeaponType() == RWT_BAZOOKA)
					{
						for(size_t i = 0 ; i < ME->Projectile.size() ; i++)
						{
							Object_Projectile* pProjectile = ME->Projectile.at(i);
							pProjectile->Destroy();
						}
					}
					else
					{
						//Tell the projectile to exit aftertouch state.
						Message msgProjAftertouchDone(msg_aftertouch_done);
						
						Object_Projectile* pProjectile = ME->Projectile.at(0);
						pProjectile->GetMessageHandler()->QueueMessage(msgProjAftertouchDone);

						pProjectile->GetPhysicsSystem()->Lua_Projectile_DisableSteering();
					}
				}

				//--------------------------------------------------
				// TGS BAZOOKA DROP OFF HACK - TODO - TO BE FIXED
				//--------------------------------------------------

				// Go through remaining projectiles and get the earliest drop off time from them
				float fEarliestDropOffTime = 9999999.9f;

				Projectile_Attributes* pProjAttrs = ME->GetProjectileAttributes();
				ntAssert( pProjAttrs );

				if ( pProjAttrs->m_eType == PROJECTILE_TYPE_BAZOOKA_ROCKET )
				{
					if(ME->Projectile.size() > 1)
					{
						for(size_t j = 1 ; j < ME->Projectile.size() ; j++)
						{
							if(CEntityManager::Get().FindEntity(ME->ProjectileName.at(j).c_str()) != NULL)	//DoesEntityExist(ProjectileName[j])
							{
								Object_Projectile* pProjectile = ME->Projectile.at(j);

								float fProjDropOffTime = pProjectile->BazookaGetDropOffTime();

								if ( fProjDropOffTime < fEarliestDropOffTime )
								{
									fEarliestDropOffTime = fProjDropOffTime;
								}
							}
						}
					}
				}

				//--------------------------------------------------
				// END TGS BAZOOKA DROP OFF HACK - TODO - TO BE FIXED
				//--------------------------------------------------

				//If we are disabling steering for other projectiles, we must be sure they still exist.
				if(ME->Projectile.size() > 1)
				{
					for(size_t i = 1 ; i < ME->Projectile.size() ; i++)
					{
						if(CEntityManager::Get().FindEntity(ME->ProjectileName.at(i).c_str()) != NULL)	//DoesEntityExist(ProjectileName[i])
						{
							Object_Projectile* pProjectile = ME->Projectile.at(i);
							pProjectile->GetPhysicsSystem()->Lua_Projectile_DisableSteering();

							//--------------------------------------------------
							// TGS BAZOOKA DROP OFF HACK - TODO - TO BE FIXED
							//--------------------------------------------------

							// If it's a bazooka rocket then do the aftertouch drop if passed the threshold
							if ( pProjectile->m_eProjectileType == PROJECTILE_TYPE_BAZOOKA_ROCKET )
							{
								pProjectile->BazookaDoAftertouchDrop( fEarliestDropOffTime );
							}

							//--------------------------------------------------
							// END TGS BAZOOKA DROP OFF HACK - TODO - TO BE FIXED
							//--------------------------------------------------
						}
					}
				}

				if(pSharedAttr->m_bReloadAfterShot)
				{
					CEntity* player = ME->m_pOther;

					// As the normal method for sequencing the statemachine goes a little
					// weird during aftertouch - there needs to be some special handling
					// to turn off messages that are sent during normal processing. This
					// is because too many messages are received and the state machines
					// become out of step. 
					{
						player->GetMovement()->ClearCompletionFeedback();
						player->GetMovement()->ClearInterruptFeedback();

						if( player->IsPlayer() )
							player->ToPlayer()->DisableFireFeedback(true);
					}

					SET_STATE(RELOAD);
				}
				else
				{
					SET_STATE(HELD);	//Probably want to check to see if the fire button was pressed so we can jump straight to shoot.
				}

				// 
				Message msg(msg_fire_complete);
				ME->m_pOther->GetMessageHandler()->QueueMessage(msg);
			}
			END_EVENT(true)

			EVENT(msg_aim_on)
			{
				ME->m_bAimingEnableRequest = true;
			}
			END_EVENT(true)

			EVENT(msg_aim_off)
			{
				ME->m_bDisableAiming = true;
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	STATE(RELOAD)
		BEGIN_EVENTS
			ON_ENTER
			{
				CEntity* player = ME->m_pOther;
				Att_RangedWeapon* pSharedAttr = ME->GetSharedAttributes();

				if( !player->IsPlayer() || !ME->m_bAiming )
				{
					//Play reload animation. Player may not turn during this.
					bool bStarted = player->GetMovement()->Lua_AltStartFacingMovement(pSharedAttr->m_obAnimPlayerReload, 0.0f, 1.0f, 0.0f, 0.0f, 0.1f);
					UNUSED( bStarted ); ntAssert_p( bStarted, ("Failed to start the movement controller") );
					
					player->GetMovement()->SetCompletionMessage("msg_movementdone");
					player->GetMovement()->SetInterruptMessage("msg_movementdone");

					// If the movement failed to start - then signal that the reload is complete. 
					if( !bStarted )
					{
						Message msg(msg_reload_complete);
						ME->GetMessageHandler()->QueueMessage(msg);
					}
				}

				//------------------------------------------------------------
				// TGS BAZOOKA AMMO MESSAGES
				//------------------------------------------------------------
				if ( player->IsPlayer() && ME->m_iAmmo == 3 && CHud::Exists() && CHud::Get().GetObjectiveManager())
				{
					int err = CHud::Get().GetObjectiveManager()->AddObjective("TGS_OBJECTIVE_3");
					UNUSED(err);
				}
				//------------------------------------------------------------
				// END OF TGS BAZOOKA AMMO MESSAGES
				//------------------------------------------------------------					

				// Send a message informing of the reload request
				Message msg(msg_reload);
				ME->m_pOther->GetMessageHandler()->QueueMessage(msg);

				if(!ntStr::IsNull(pSharedAttr->m_obAnimObjectReload) )
				{
					ME->PlayAnim(pSharedAttr->m_obAnimObjectReload);	//Play our flick anim for this object.
				}
			}
			END_EVENT(true)

			///-----------------------------------
			ON_EXIT
			{
				CEntity* player = ME->m_pOther;
				if( !player->IsPlayer() || !ME->m_bAiming )
				{
				// 
				Message msg(msg_reload_complete);
				ME->m_pOther->GetMessageHandler()->QueueMessage(msg);
			}
			}
			END_EVENT(true)


			EVENT(msg_interrupt)
			{
				SET_STATE(INTERRUPT);
			}
			END_EVENT(true)

			EVENT(msg_reload_complete)
			EVENT(msg_movementdone)
			{
				SET_STATE(HELD);
				ME->m_bShotFired = false;
			}
			END_EVENT(true)

			EVENT(msg_aim_on)
			{
				ME->m_bAimingEnableRequest = true;
			}
			END_EVENT(true)

			EVENT(msg_aim_off)
			{
				ME->m_bDisableAiming = true;
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	STATE(THROW)
		BEGIN_EVENTS
			EVENT(msg_think_onthrow)
			{
				bool bUseAftertouch = false;

				Att_RangedWeapon* pSharedAttr = ME->GetSharedAttributes();
				CEntity* pPlayer = ME->m_pOther;
				const float fWaitTime = ME->GetSharedAttributes()->m_fAfterTouchWaitTime;

				ntAssert (pSharedAttr);

				if ( pSharedAttr->m_bAftertouchOnThrow && pPlayer->Lua_IsActionHeld(fWaitTime) )
				{
					bUseAftertouch = true;
				}

				ME->Lua_ReparentToWorld();
				ME->GetPhysicsSystem()->Lua_Rigid_SetKeyframedMotion(false);
				ME->GetInteractionComponent()->AllowCollisionWith(pPlayer);

				CDirection ThrowVelocity(pSharedAttr->m_obThrowVelocity);
				if(ME->m_bAiming == true)
				{
					ME->GetPhysicsSystem()->Lua_AltSetLinearVelocityFromCamera(ThrowVelocity);
				}
				else
				{
					//Set velocity from the camera raycast.
					ME->GetPhysicsSystem()->Lua_AltSetLinearVelocityFromTarget(pPlayer, ThrowVelocity);
				}

				CDirection vAngularThrowVelocity(pSharedAttr->m_obThrowAngularVelocity);
				CDirection vAngularVelocity = vAngularThrowVelocity * pPlayer->GetMatrix();
				ME->GetPhysicsSystem()->Lua_AltSetAngularVelocity(vAngularVelocity);
				ME->GetPhysicsSystem()->Lua_Rigid_AntiGravity(10.0f, 1.0f);

				if(bUseAftertouch)
				{
					SET_STATE(AFTERTOUCH);
				}
			}
			END_EVENT(true)
		
			ON_ENTER
			{
				CEntity* player = ME->m_pOther;
				Att_RangedWeapon* pSharedAttr = ME->GetSharedAttributes();

				ME->m_bMovementDone = false;
				ME->m_bAtRest = false;

				player->GetMovement()->Lua_AltStartTargetedFacingMovement(pSharedAttr->m_obAnimPlayerThrow, 360.0f, 1.0f, 0.0f, 0.1f);
				player->GetMovement()->SetCompletionMessage("msg_movementdone");
				player->GetMovement()->SetInterruptMessage("msg_interrupt");

				ME->m_bAiming = false;

				//Send a message saying that we have successfully picked up the weapon.
				Message msg(msg_control_end);
				player->GetMessageHandler()->QueueMessage(msg);

				Message msgOnThrow(msg_think_onthrow);
				ME->GetMessageHandler()->QueueMessageDelayed(msgOnThrow, pSharedAttr->m_fThrowTime);
			}
			END_EVENT(true)

			EVENT(msg_interrupt)
			{
				SET_STATE(INTERRUPT);
			}
			END_EVENT(true)

			EVENT(msg_atrest)
			{
				ME->m_bAtRest = true;

				if(ME->m_bAiming == false)
				{
					SET_STATE(DEFAULT);
				}
			}
			END_EVENT(true)

			EVENT(msg_movementdone)
			{
				CEntity* player = ME->m_pOther;

				if(ME->m_bAiming == false)
				{
					Message msgExitState(msg_exitstate);
					msgExitState.SetEnt( CHashedString(HASH_STRING_SENDER), ME);
					player->GetMessageHandler()->QueueMessage(msgExitState);

					CamMan::Get().GetView(0)->RemoveAllCoolCameras();
				}

				ME->GetPhysicsSystem()->Lua_Rigid_CheckAtRest();	//Make sure the rigid body generates an AtRest message.

				ME->m_bMovementDone = true;
			}
			END_EVENT(true)

			EVENT(msg_power_off)
			{
				ME->m_bAiming = false;

				if(ME->m_bMovementDone == true)
				{
					CEntity* player = ME->m_pOther;
					Message msgExitState(msg_exitstate);
					msgExitState.SetEnt( CHashedString(HASH_STRING_SENDER), ME);
					player->GetMessageHandler()->QueueMessage(msgExitState);

					if(ME->GetRangedWeaponType() == RWT_BAZOOKA)
					{
						for(size_t i = 0 ; i < ME->Projectile.size() ; i++)
						{
							Object_Projectile* pProjectile = ME->Projectile.at(i);
							pProjectile->Destroy();
						}
					}
					else
					{
						CamMan::Get().GetView(0)->RemoveAllCoolCameras();
					}

					if(ME->m_bAtRest == true)
					{
						SET_STATE(DEFAULT);
					}
				}
			}
			END_EVENT(true)

			EVENT(msg_collision)
			{
				Att_RangedWeapon* pSharedAttr = ME->GetSharedAttributes();

				float fProjectileVelocity = msg.GetFloat("ProjVel");
				float fImpactThreshold = pSharedAttr->m_fImpactThreshold;

				//Make sure the object was travelling above the threshold velocity.
				if((fProjectileVelocity > -fImpactThreshold) && (fProjectileVelocity < fImpactThreshold))
				{
					END_EVENT(true);
				}

				if(!ntStr::IsNull(pSharedAttr->m_obPfxImpact) )
				{
					FXHelper::Pfx_CreateStatic(pSharedAttr->m_obPfxImpact, ME, "ROOT");
				}

				if(!ntStr::IsNull(pSharedAttr->m_obThrownAttackData) )
				{
//					Combat_GenerateStrike(this.attrib.Other.Entity,msg.Collidee,this.attrib.SharedAttributes.ThrownAttackData)
					CombatHelper::Combat_GenerateStrike(ME, ME->m_pOther, msg.GetEnt("Collidee"), pSharedAttr->m_obThrownAttackData);
				}
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	STATE(AFTERTOUCH)
		BEGIN_EVENTS
			ON_ENTER
			{
				CEntity* player = ME->m_pOther;
				Att_RangedWeapon* pSharedAttr = ME->GetSharedAttributes();

				ME->m_iThrownCameraHandle = CamMan::Get().GetView(0)->ActivateAfterTouchCoolCamDef(ME, pSharedAttr->m_pobAftertouchCamProperties);
				ME->GetPhysicsSystem()->Lua_SetControllerDef(player, pSharedAttr->m_pobAftertouchProperties);
			}
			END_EVENT(true)

			EVENT(msg_interrupt)
			{
				SET_STATE(INTERRUPT);
			}
			END_EVENT(true)

			EVENT(msg_collision)
			{
				CEntity* player = ME->m_pOther;
				UNUSED(player);
				Att_RangedWeapon* pSharedAttr = ME->GetSharedAttributes();

				//Make sure the object was travelling above the threshold velocity.
				float fProjectileVelocity = msg.GetFloat("ProjVel");
				float fImpactThreshold = pSharedAttr->m_fImpactThreshold;

				if((fProjectileVelocity > -fImpactThreshold) && (fProjectileVelocity < fImpactThreshold))
				{
					END_EVENT(true)
				}

				if(!ntStr::IsNull(pSharedAttr->m_obPfxImpact))
				{
					FXHelper::Pfx_CreateStatic(pSharedAttr->m_obPfxImpact, ME, "ROOT");
				}

				if(!ntStr::IsNull(pSharedAttr->m_obThrownAttackData) )
				{
					CombatHelper::Combat_GenerateStrike(ME, ME->m_pOther, msg.GetEnt("Collidee"), pSharedAttr->m_obThrownAttackData);
				}
			}
			END_EVENT(true)

			EVENT(msg_attack_off)
			EVENT(msg_power_off)
			EVENT(msg_action_off)
			{
				ME->GetPhysicsSystem()->Lua_SetControllerDef(NULL, 0);

				if(ME->GetRangedWeaponType() == RWT_BAZOOKA)
				{
					for(size_t i = 0 ; i < ME->Projectile.size() ; i++)
					{
						Object_Projectile* pProjectile = ME->Projectile.at(i);
						pProjectile->Destroy();
					}
				}
				else
				{
					CamMan::Get().GetView(0)->RemoveAllCoolCameras();
				}

				CEntity* player = ME->m_pOther;
				Message msgExitState(msg_exitstate);
				msgExitState.SetEnt( CHashedString(HASH_STRING_SENDER), ME);
				player->GetMessageHandler()->QueueMessage(msgExitState);

				SET_STATE(DEFAULT);
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	STATE(DROP)
		BEGIN_EVENTS

			EVENT(msg_think_ondrop)
			{
				CEntity* player = ME->m_pOther;
				Att_RangedWeapon* pSharedAttr = ME->GetSharedAttributes();

				ME->Lua_ReparentToWorld();	//Reparent the object back to the world.
				ME->GetPhysicsSystem()->Lua_Rigid_SetKeyframedMotion(false);	//Allow havok to take control of the object again.
				//Set the velocity of the object relative to the player's direction OR towards their current throw target.
				CDirection vDropVelocity(pSharedAttr->m_obDropVelocity);
				CDirection vVelocity = vDropVelocity * player->GetMatrix();
				ME->GetPhysicsSystem()->Lua_AltSetLinearVelocity(vVelocity);
				ME->GetPhysicsSystem()->Lua_Rigid_CheckAtRest();

				ME->GetInteractionComponent()->Lua_AllowCollisionWith(player);	//Disallow collision primitive overlap between player and object.
			}
			END_EVENT(true)

			ON_ENTER
			{
				Character* player = ME->m_pOther;
				Att_RangedWeapon* pSharedAttr = ME->GetSharedAttributes();

				player->GetMovement()->Lua_AltStartFacingMovement(pSharedAttr->m_obAnimPlayerDrop, 360.0f, 1.0f, 0.0f, 0.0f, 0.0f);
				player->GetMovement()->SetCompletionMessage("msg_movementdone");
				player->GetMovement()->SetInterruptMessage("msg_interrupt");

				player->SetExitOnMovementDone(true);	//Tell the player to automatically return to default state when movement is done.

				Message msgOnDrop(msg_think_ondrop);
				ME->GetMessageHandler()->QueueMessageDelayed(msgOnDrop, pSharedAttr->m_fDropTime);

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
				//TODO: I think we need an AI version of this, AI's don't drop their weapon immediately when hit... or should they?
				CEntity* player = ME->m_pOther;

				ME->GetAnimator()->RemoveAllAnimations();	//Kill any anims that might be playing on this object.
				ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);	//This object cannot be picked up or caught when being dropped.
				ME->Lua_ReparentToWorld();	//Reparent the object back to the world.
				ME->GetPhysicsSystem()->Lua_Rigid_SetKeyframedMotion(false);	//Allow havok to take control of the object again.

				if(ME->m_pOther != 0)
				{
					ME->GetInteractionComponent()->AllowCollisionWith(player);	//Allow object and player to collide with each other again.
					ME->m_pOther = 0;
				}

				CDirection vDir = CDirection(0.0f, 1.0f, 0.0f);
				ME->GetPhysicsSystem()->Lua_AltSetLinearVelocity(vDir);	//Toss the object in the air.
				// stop rotations, it can have some big rotations setup
				ME->GetPhysicsSystem()->SetAngularVelocity(CDirection(0.0f, 0.0f, 0.0f));
				ME->GetPhysicsSystem()->Lua_Rigid_CheckAtRest();
			}
			END_EVENT(true)

			EVENT(msg_atrest)
			{
				SET_STATE(DEFAULT);
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	STATE(INACTIVE)	//TODO: never used... should it be?
		BEGIN_EVENTS
			ON_ENTER
			{
				ME->m_pOther = 0;
				ME->GetAnimator()->RemoveAllAnimations();	//Kill any animations that might be playing on this object.
				ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);
				ME->GetPhysicsSystem()->Lua_DeactivateState("Rigid");
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	STATE(DESTROY)	//TODO: Never used... should it be?
		BEGIN_EVENTS

			EVENT(msg_think_onremoveobject)
			{
				if(ME->m_pOther != 0)
				{
					CEntity* player = ME->m_pOther;

					Message msgExitState(msg_exitstate);
					msgExitState.SetEnt( CHashedString(HASH_STRING_SENDER), ME);
					player->GetMessageHandler()->QueueMessage(msgExitState);

					CamMan::Get().GetView(0)->RemoveAllCoolCameras();
				}

				ME->Lua_RemoveSelfFromWorld();	//This is the end!!!
			}
			END_EVENT(true)

			ON_ENTER
			{
				Att_RangedWeapon* pSharedAttr = ME->GetSharedAttributes();

				ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);
				ME->Lua_ReparentToWorld();
				ME->GetPhysicsSystem()->Deactivate();	//DeactivateAllStates - Allow havok to take control of the object again.
				ME->GetPhysicsSystem()->Lua_RemoveChildEntities();
				ME->Hide();
				ME->GetAnimator()->RemoveAllAnimations();	//Kill any animations that may be playing on this object.

				if(!ntStr::IsNull(pSharedAttr->m_obPfxDestroyed) )
				{
					FXHelper::Pfx_CreateStatic(pSharedAttr->m_obPfxDestroyed, ME, "ROOT");	//Spawn destruction particle effect.
				}

				if(!ntStr::IsNull(pSharedAttr->m_obSfxDestroyed) )
				{
					//AudioHelper::PlaySound(pSharedAttr->GetString("SfxDestroyed").c_str()); // Note: 2 strings required...
				}

				Message msgOnRemoveObject(msg_think_onremoveobject);
				ME->GetMessageHandler()->QueueMessageDelayed(msgOnRemoveObject, 1.0f);
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE


//	--------------------------------------------------------------
//	----------------- Unique AI usability states -----------------
//	--------------------------------------------------------------

	STATE(AI_MOVETO)
		BEGIN_EVENTS
			ON_ENTER
			{
				CEntity* player = ME->m_pOther;
				Att_RangedWeapon* pSharedAttr = ME->GetSharedAttributes();

				//Player moves into position before starting pickup
				player->GetMovement()->Lua_StartMoveToTransition(pSharedAttr->m_obAnimPlayerMoveTo, ME, 1, 1);
				player->GetMovement()->Lua_AltSetMovementCompleteMessage("msg_movementdone", player);

				ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);	//Only one user at a time.
			}
			END_EVENT(true)

			EVENT(msg_interrupt)
			{
				SET_STATE(INTERRUPT);
			}
			END_EVENT(true)

			EVENT(msg_movementdone)
			{
				SET_STATE(AI_PICKUP);
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	STATE(AI_RUNTO)
		BEGIN_EVENTS
			ON_ENTER
			{
				CEntity* player = ME->m_pOther;
				Att_RangedWeapon* pSharedAttr = ME->GetSharedAttributes();

				//Player moves into position before starting the pickup
				player->GetMovement()->Lua_StartMoveToTransition(pSharedAttr->m_obAnimPlayerRunTo, ME, 1, 1);	//Move the player near the object.
				player->GetMovement()->Lua_AltSetMovementCompleteMessage("msg_movementdone", player);

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
				SET_STATE(AI_RUNPICKUP);
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	STATE(AI_PICKUP)
		BEGIN_EVENTS
			ON_ENTER
			{
				CEntity* player = ME->m_pOther;
				Att_RangedWeapon* pSharedAttr = ME->GetSharedAttributes();

				//Player starts their pickup animation.
				player->GetMovement()->Lua_AltStartFacingMovement(pSharedAttr->m_obAnimPlayerPickup, 360.0f, 1.0f, 0.0f, 0.0f, 0.0f);
				player->GetMovement()->SetCompletionMessage("msg_movementdone");
				player->GetMovement()->SetInterruptMessage("msg_interrupt");


				//Object plays its pickup animation relative to the player's root.
				ME->GetInteractionComponent()->ExcludeCollisionWith(player);	//Allow player and object to overlap.
				ME->GetPhysicsSystem()->Lua_Rigid_SetKeyframedMotion(true);		//Take control away from havok.
				//We no-longer set to identity, but instead set to held-matrix.
//				ME->Lua_SetIdentity();
				Transform* pobTransform = ME->GetHierarchy()->GetRootTransform();
				ntError_p(pobTransform, ("Could not get root node transform on ranged weapon during AI_EQUIP"));
				if(pobTransform)
				{
					pobTransform->SetLocalMatrix(ME->GetHeldMatrix());
				}
				ME->Lua_Reparent(player, "r_weapon");	//Reparent the weapon to the right-weapon transform on the character.
			}
			END_EVENT(true)

			EVENT(msg_interrupt)
			{
				SET_STATE(INTERRUPT);
			}
			END_EVENT(true)

			EVENT(msg_movementdone)
			{
				SET_STATE(AI_HELD);
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	STATE(AI_RUNPICKUP)
		BEGIN_EVENTS
			ON_ENTER
			{
				CEntity* player = ME->m_pOther;
				Att_RangedWeapon* pSharedAttr = ME->GetSharedAttributes();

				//Player starts their pickup anim.
				player->GetMovement()->Lua_AltStartFacingMovement(pSharedAttr->m_obAnimPlayerRunPickup, 360.0f, 1.0f, 0.0f, 0.0f, 0.0f);
				player->GetMovement()->SetCompletionMessage("msg_movementdone");
				player->GetMovement()->SetInterruptMessage("msg_interrupt");


				//Object plays its pickup animation relative to the player's root.
				ME->GetInteractionComponent()->ExcludeCollisionWith(player);	//Allow the object and player to overlap.
				ME->GetPhysicsSystem()->Lua_Rigid_SetKeyframedMotion(true);		//Take control away from havok.
				//We no-longer set to identity, but instead set to held-matrix.
//				ME->Lua_SetIdentity();
				Transform* pobTransform = ME->GetHierarchy()->GetRootTransform();
				ntError_p(pobTransform, ("Could not get root node transform on ranged weapon during AI_EQUIP"));
				if(pobTransform)
				{
					pobTransform->SetLocalMatrix(ME->GetHeldMatrix());
				}
				ME->Lua_Reparent(player, "r_weapon");	//Reparent the weapon to the right-weapon transform on the character.
			}
			END_EVENT(true)

			EVENT(msg_interrupt)
			{
				SET_STATE(INTERRUPT);
			}
			END_EVENT(true)

			EVENT(msg_movementdone)
			{
				SET_STATE(AI_HELD);
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	STATE(AI_EQUIP)
		BEGIN_EVENTS
			ON_ENTER
			{
				CEntity* player = ME->m_pOther;
				CEntity* weapon = ME;
				ME->GetInteractionComponent()->ExcludeCollisionWith(player);	//Allow the object and player to overlap.
				ME->GetPhysicsSystem()->Lua_Rigid_SetKeyframedMotion(true);		//Take control away from havok
				//We no-longer set to identity, but instead set to held-matrix.
//				ME->Lua_SetIdentity();
				Transform* pobTransform = ME->GetHierarchy()->GetRootTransform();
				ntError_p(pobTransform, ("Could not get root node transform on ranged weapon during AI_EQUIP"));
				if(pobTransform)
				{
					pobTransform->SetLocalMatrix(ME->GetHeldMatrix());
				}
				ME->Lua_Reparent(player, "r_weapon");	//Reparent the weapon to the the right-weapon transform on the character.
				weapon = ME;

				ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);	//One user at a time.

				SET_STATE(AI_HELD);
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	STATE(AI_HELD)
		BEGIN_EVENTS
			ON_ENTER
			{
				CEntity* player = ME->m_pOther;
				Att_RangedWeapon* pSharedAttr = ME->GetSharedAttributes();
				UNUSED(pSharedAttr);

				if(player && player->IsCharacter())
				{
					player->ToCharacter()->SetRangedWeapon(ME);
				}
				else
				{
					//For debug purposes, check if on-shutdown we sometimes get here with no player-pointer or attribute table.
					ntAssert_p(false, ("AI_HELD - case 1 - No 'other' entity, or that entity isn't a character"));
				}

				if(player && player->GetMessageHandler())
				{
					//Send a message saying that we have successfully picked up the weapon.
					Message msgWeaponPickupSuccess(msg_weaponpickup_success);
					player->GetMessageHandler()->QueueMessage(msgWeaponPickupSuccess);
				}
				else
				{
					//For debug purposes, check if on-shutdown we sometimes get here with no player-pointer or message handler.
					ntPrintf("Assert: AI_HELD case 2 - No 'other' entity or no message-handler for it\n");
					ntAssert(0);
				}

				if(player && player->GetMovement())
				{
					const CAIComponentDef* pobCompDef =  player->ToAI()->GetAIComponent()->GetDefinition();
					ntAssert(pobCompDef);
					ntAssert(pobCompDef->m_pobMovementSet->m_pobCrossbowWalkingController);
					player->GetMovement()->BringInNewController( *pobCompDef->m_pobMovementSet->m_pobCrossbowWalkingController, CMovement::DMM_STANDARD, 0.0f );
				}
				else
				{
					//For debug purposes, check if on-shutdown we sometimes get here with no player-pointer or movement component.
					ntPrintf("Assert: AI_HELD case 3 - No 'other' entity or no movement-component for it\n");
					ntAssert(0);
				}

				ME->m_bAiming = false;
			}
			END_EVENT(true)

			ON_EXIT
			{
//				ntPrintf("AI_HELD.OnExit\n");
			}
			END_EVENT(true)

			EVENT(msg_action_on)
			{
				SET_STATE(AI_SHOOT);
			}
			END_EVENT(true)

			EVENT(msg_start_aim)
			{
				SET_STATE(AI_SHOOT);
			}
			END_EVENT(true)

			EVENT(msg_detach)
			{
				SET_STATE(AI_DROP);
			}
			END_EVENT(true)

			EVENT(msg_grab_on)
			{
				SET_STATE(AI_DROP);
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	STATE(AI_SHOOT)
		BEGIN_EVENTS
			EVENT(msg_action_on)
			EVENT(msg_think_onspawnprojectile)
			{
				Projectile_Attributes* pProjAttrs = ME->GetProjectileAttributes();
				Object_Projectile* pProjectile = 0;

				// Retrieve the target from the shooter
				//Check that this shooter is valid and an AI before trying to get it's AI component!
				ntError_p(ME->m_pOther && ME->m_pOther->IsAI(), ("AI_SHOOT: Missing or Invalid ranged-weapon-user"));
				CAIComponent* pAIComp = ME->m_pOther->ToAI()->GetAIComponent();
				ntAssert_p(pAIComp, ("The Entity: %s is not an AI and is in AI_SHOOT state.",ntStr::GetString(ME->m_pOther->GetName())));
				CEntity* pTarget = (CEntity*)(pAIComp->GetCAIMovement()->GetEntityToAttack());
				
				switch(pProjAttrs->m_eType)
				{
					case PROJECTILE_TYPE_CROSSBOW_BOLT:
						ntAssert_p(pTarget, ("The Entity: %s is in AI_SHOOT state w/o a TARGET.",ntStr::GetString(ME->m_pOther->GetName())));
						pProjectile = Object_Projectile::CreateCrossbowBolt(ME, pProjAttrs, pTarget, true);
						ntAssert_p( pProjectile, ("Failed to create crossbow projectile") );
						break;

					case PROJECTILE_TYPE_BAZOOKA_ROCKET:
						pProjectile = Object_Projectile::CreateBazookaRockets(ME, pProjAttrs, CEntityManager::Get().GetPlayer(), true);

						ntAssert_p( pProjectile, ("Failed to create bazooka projectiles") );
						break;

					case PROJECTILE_TYPE_BALLISTA_BOLT:
						//TODO: Fill once we have the static spawn function for ballista bolts.
						ntPrintf("Ballista bolt creation not yet ported\n");
						break;

					case PROJECTILE_TYPE_AGEN_SWORD:
						//pProjectile = Object_Projectile::CreateAGenSword(ME, pProjAttrs, CEntityManager::Get().GetPlayer(), true);
						ntError(0);
						break;

					default:
						ntAssert(false && "Unknown projectile type");
						break;
				}
			}
			END_EVENT(true)
		
			ON_ENTER
			{
				CEntity* player = ME->m_pOther;
				Att_RangedWeapon* pSharedAttr = ME->GetSharedAttributes();
				//UNUSED(pSharedAttr);

				player->GetMovement()->Lua_AltStartSimpleMovement(pSharedAttr->m_obAnimPlayerShoot);
				player->GetMovement()->SetCompletionMessage("msg_movementdone");
				player->GetMovement()->SetInterruptMessage("msg_movementdone");


				Message msgSpawnProjectile(msg_think_onspawnprojectile);
				ME->GetMessageHandler()->QueueMessage(msgSpawnProjectile);
			}
			END_EVENT(true)

			ON_EXIT
			{
//				ntPrintf("AI_SHOOT.OnExit\n");
			}
			END_EVENT(true)

			EVENT(msg_movementdone)
			{
				SET_STATE(AI_HELD);
			}
			END_EVENT(true)

			EVENT(msg_detach)
			{
				SET_STATE(AI_DROP);
			}

		END_EVENTS
	END_STATE

	STATE(AI_DROP)
		BEGIN_EVENTS
			ON_ENTER
			{
				Character* player = ME->m_pOther;
				Att_RangedWeapon* pSharedAttr = ME->GetSharedAttributes();

				ME->Lua_ReparentToWorld();	//Reparent the object back to the world.
				ME->GetPhysicsSystem()->Lua_Rigid_SetKeyframedMotion(false);	//Allow havok to take control of the object again.
				//Set the velocity of the object relative to the player's direction OR towards their current throw target.
				CDirection vDropVelocity(pSharedAttr->m_obDropVelocity);
				CDirection vVelocity = vDropVelocity * player->GetMatrix();
				ME->GetPhysicsSystem()->Lua_AltSetLinearVelocity(vVelocity);
				ME->GetPhysicsSystem()->Lua_Rigid_CheckAtRest();

				ME->GetInteractionComponent()->Lua_AllowCollisionWith(player);	//Disallow collision primitive overlap between player and object.

				//Start the player drop anim.
				player->GetMovement()->Lua_AltStartFacingMovement(pSharedAttr->m_obAnimPlayerDrop, 360.0f, 1.0f, 0.0f, 0.0f, 0.0f);
				player->GetMovement()->SetCompletionMessage("msg_movementdone");
				player->GetMovement()->SetInterruptMessage("msg_movementdone");
				player->SetExitOnMovementDone(true);

			}
			END_EVENT(true)

			EVENT(msg_atrest)
			{
				SET_STATE(DEFAULT);
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	BEGIN_EVENTS
		EVENT(msg_special_onscreen_message)
		{
			//------------------------------------------------------------
			// TGS BAZOOKA AMMO MESSAGES
			//------------------------------------------------------------
			if ( CHud::Exists() && CHud::Get().GetObjectiveManager() )
			{
				CHud::Get().GetObjectiveManager()->AddObjective("TGS_OBJECTIVE_4");
			}
			//------------------------------------------------------------
			// END OF TGS BAZOOKA AMMO MESSAGES
			//------------------------------------------------------------
		}
		END_EVENT(true)
	END_EVENTS

END_STATEMACHINE //RANGED_WEAPON_FSM


//--------------------------------------------------
//!
//!	Object_Ranged_Weapon::Object_Ranged_Weapon()
//!	Default constructor
//!
//--------------------------------------------------
Object_Ranged_Weapon::Object_Ranged_Weapon()
{
	m_eType = 				EntType_Interactable;
	m_eInteractableType = 	EntTypeInteractable_Object_Ranged_Weapon;

	m_pSharedAttributes = 0;

	m_pOther = 0;
	m_bAiming = false;
	m_bShotFired = false;
	m_bMovementDone = false;
	m_bAtRest = false;
	m_iThrownCameraHandle = 0;
	m_obHeldMatrix = CMatrix(CONSTRUCT_IDENTITY);
}

//--------------------------------------------------
//!
//!	Object_Ranged_Weapon::OnPostConstruct()
//!	Post Construct
//!
//--------------------------------------------------
void Object_Ranged_Weapon::OnPostConstruct()
{
	Interactable::OnPostConstruct();

	InstallMessageHandler();
	InstallAnimator(m_AnimationContainer);

	//Dynamics components
	Lua_CreatePhysicsSystem();

	
	// Read configuration for rigid body from Atrribute Table
	hkRigidBodyCinfo obInfo;

	obInfo.m_qualityType = HK_COLLIDABLE_QUALITY_MOVING;
	obInfo.m_maxLinearVelocity = m_pSharedAttributes->m_fMaxLinearVelocity;
	obInfo.m_maxAngularVelocity = m_pSharedAttributes->m_fMaxAngularVelocity;

	obInfo.m_motionType = hkMotion::MOTION_DYNAMIC;

	/*
	LuaAttributeTable* pAttributeTable = NT_NEW Att_RangedWeapon;
	pAttributeTable->SetDataObject(ObjectDatabase::Get().GetDataObjectFromPointer( (void*) m_pSharedAttributes));

	NinjaLua::LuaObject LuaMotionType = pAttributeTable->GetAttribute("MotionType");
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
	NT_DELETE ( pAttributeTable );
	*/

	//  Create a group
	//Physics::LogicGroup* lg = (Physics::LogicGroup*)Physics::ClumpTools::ConstructRigidLGFromClump( this,  &obInfo );
	Physics::SingleRigidLG* lg = NT_NEW Physics::SingleRigidLG( GetName(),  this);
	lg->Load();
	if(lg)
	{
		// Add the group
		m_pobPhysicsSystem->AddGroup( (Physics::LogicGroup *) lg );
		lg->Activate();	
	}
	else
		ntPrintf("%s(%d): ### PHYSICS ERROR - Logic group not created for entity %s with clump %s\n", __FILE__, __LINE__, GetName().c_str(), GetClumpString().c_str());

	//Object-specific variables.
	m_pOther = NULL;
	m_bAiming = false;
	m_bAimingEnableRequest = false;
	m_bDisableAiming = false;
	m_bShotFired = false;

	Projectile.clear();
	ProjectileName.clear();

	m_iAmmo = m_pSharedAttributes->m_iAmmo;
	m_iProjectileCount = 0;
	m_iThrownCameraHandle = 0;
	m_bInAftertouch = false;
	m_bMovementDone = false;
	m_bAtRest = false;

	RANGED_WEAPON_TYPE eWeaponType = m_pSharedAttributes->m_eRangedWeaponType;
	UNUSED( eWeaponType );
	ntPrintf("***RangedWeaponType = %d\n", eWeaponType);

	// Create and attach the statemachine
	RANGED_WEAPON_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) RANGED_WEAPON_FSM();
	ATTACH_FSM(pFSM);
}

//--------------------------------------------------
//!
//!	Object_Ranged_Weapon::~Object_Ranged_Weapon()
//!	Default destructor
//!
//--------------------------------------------------
Object_Ranged_Weapon::~Object_Ranged_Weapon()
{

}

//--------------------------------------------------
//!
//!	void Object_Ranged_Weapon::PlayAnim(const char *anim)
//!	Play requested animation.
//!
//--------------------------------------------------
void Object_Ranged_Weapon::PlayAnim(CHashedString anim)
{
	//Check we've been given an animation to play.
	if( ntStr::IsNull(anim) )
	{
		return;
	}

	//Play the anim (torn from Anim_Play, including the rest of this comment block).
	CAnimationPtr obNewAnim = GetAnimator()->CreateAnimation(anim);
	if(obNewAnim != 0)
	{
		obNewAnim->SetSpeed(1.0f);

		int iFlags = 0;

		iFlags |= ANIMF_LOCOMOTING;
//		iFlags |= ANIMF_LOOPING;

		obNewAnim->SetFlagBits(iFlags);

		GetAnimator()->AddAnimation(obNewAnim);
		GetAnimator()->GetAnimEventHandler().GenerateAnimDoneMessage(anim);
	}
	else
	{
		ntPrintf("Warning: Interactable_Simple_Usable::PlayAnim() - Entity %s has no anim called %s\n", GetName().c_str(), anim.GetDebugString() );
	}
}
