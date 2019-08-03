//--------------------------------------------------
//!
//!	\file game/entityinteractablethrown.cpp
//!	Definition of the Interactable Thrown entity object
//!
//--------------------------------------------------

#include "objectdatabase/dataobject.h"
#include "game/luaattrtable.h"
#include "Physics/system.h"
#include "physics/compoundlg.h"
#include "physics/singlerigidlg.h"

#include "game/movement.h"

#include "messagehandler.h"
#include "messages.h"
#include "effect/fxhelper.h"
#include "audio/audiohelper.h"
#include "game/combathelper.h"
#include "game/renderablecomponent.h"
#include "camera/camman.h"
#include "camera/camview.h"

#include "game/entityinteractablethrown.h"

// Components needed
#include "game/interactioncomponent.h"

void ForceLinkFunctionThrown()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionThrown() !ATTN!\n");
}

START_CHUNKED_INTERFACE(Att_Thrown, Mem::MC_ENTITY)
	PUBLISH_VAR_AS     (m_obThrownAttackData, ThrownAttackData)					// The definition that defines the strike data
	PUBLISH_VAR_AS     (m_obThrowVelocity, ThrowVelocity)						// The object velocity when thrown
	PUBLISH_VAR_AS     (m_obThrowAngularVelocity, ThrowAngularVelocity)			// The object spin rate when thrown
	PUBLISH_VAR_AS     (m_fLinearDamping, LinearDamping)						// The rate at which velocity is lost when thrown
	PUBLISH_VAR_AS     (m_fAngularDamping, AngularDamping)						// The rate at which spin is lost when thrown
	PUBLISH_VAR_AS     (m_fMaxLinearVelocity, MaxLinearVelocity)				// Defines the maximum velocity of the object
	PUBLISH_VAR_AS     (m_fMaxAngularVelocity, MaxAngularVelocity)				// Defines the maximum spin rate of the object
	PUBLISH_VAR_AS     (m_fImpactThreshold, ImpactThreshold)					// Magic number to indicate the threshold before an object will break on impact
	PUBLISH_VAR_AS     (m_fRestitution, Restitution)							// The rate at which energy is lost from impacts with other objects
	PUBLISH_VAR_AS     (m_fFriction, Friction)									// The rate at which energy is lost from sliding against other objects
	PUBLISH_VAR_AS     (m_fMass, Mass)											// Sets the mass of the object effecting things like inertia and bounce
	PUBLISH_VAR_AS     (m_obCenterOfMass, CenterOfMass)							// Adjusts the centre of mass, offset from the root
	PUBLISH_VAR_AS     (m_fThrowTime, ThrowTime)								// Time offset from the start of the anim before the object is thrown
	PUBLISH_VAR_AS     (m_fDropTime, DropTime)									// Time offset from the start of the anim before the object is dropped
	PUBLISH_VAR_AS     (m_obThrowTranslation, ThrowTranslation)					// Translation offset from the root of the character where the object is released
	PUBLISH_VAR_AS     (m_obThrowOrientation, ThrowOrientation)					// Orientation offset from the root of the character where the object is released
	PUBLISH_VAR_AS     (m_obDropVelocity, DropVelocity)							// The objects direction when dropped
	PUBLISH_VAR_AS     (m_bOrientateToVelocity, OrientateToVelocity)			// Sets the object orientation to be the same as the velocity, used for spears and swords
	PUBLISH_VAR_AS     (m_bDamageOnChar, DamageOnChar)							// Flag if the object will break in contact with a character
	PUBLISH_VAR_AS     (m_bDamageOnEnv, DamageOnEnv)							// Flag if the object will break in contact with the environment
	PUBLISH_VAR_AS     (m_bCollapseOnDamage, CollapseOnDamage)					// Do compound rigid collapse on damage
	PUBLISH_VAR_AS     (m_bRemoveOnDamage, RemoveOnDamage)						// Remove mesh on damage
	PUBLISH_VAR_AS     (m_bRebound, Rebound)									// If the object will rebound for more fun, shields
	PUBLISH_VAR_AS     (m_bStickInChar, StickInChar)							// If the object will stick in characters, spears and swords
	PUBLISH_VAR_AS     (m_bStickInEnv, StickInEnv)								// If the object will stick in the environment, spears and swords
	PUBLISH_VAR_AS     (m_bInstantKill, InstantKill)							// If the object will kill on impact
	PUBLISH_VAR_AS     (m_bImpaleKill, ImpaleKill)								// If the object will kill on impale
	PUBLISH_VAR_AS     (m_obAnimPlayerMoveTo, AnimPlayerMoveTo)					// Anim for the goto
	PUBLISH_VAR_AS     (m_obAnimPlayerRunTo, AnimPlayerRunTo)					// Anim for the run to
	PUBLISH_VAR_AS     (m_obAnimPlayerPickup, AnimPlayerPickup)					// Anim for the pickup
	PUBLISH_VAR_AS     (m_obAnimPlayerRunPickup, AnimPlayerRunPickup)			// Anim for the run pickup
	PUBLISH_VAR_AS     (m_obAnimPlayerThrow, AnimPlayerThrow)					// Anim for the throw
	PUBLISH_VAR_AS     (m_obAnimPlayerDrop, AnimPlayerDrop)						// Anim for the drop
	PUBLISH_PTR_AS     (m_pobPlayerHoldingMovement, PlayerHoldingMovement)		// Definition for the partial WalkRun def
	PUBLISH_PTR_AS     (m_pobAftertouchProperites, AftertouchProperites)		// Aftertouch parameters for the object
	PUBLISH_PTR_AS     (m_pobAftertouchCamProperties, AftertouchCamProperties)	// Aftertouch camera paramters
	PUBLISH_VAR_AS     (m_bAIAvoid, AIAvoid)									// Sets if the AI will try to avoid this object
	PUBLISH_VAR_AS     (m_fAIAvoidRadius, AIAvoidRadius)						// Sets the radius that the AI will try to avoid the object by
	PUBLISH_VAR_AS     (m_obPhysicsSoundDef, PhysicsSoundDef)					// Sound definitions for the physics stuff
	PUBLISH_VAR_AS     (m_obSfxImpaleRagdoll, SfxImpaleRagdoll)
	PUBLISH_VAR_AS     (m_obSfxImpaleSolid, SfxImpaleSolid)
	PUBLISH_VAR_AS     (m_obSfxDestroy, SfxDestroy)
	PUBLISH_VAR_AS     (m_obSfxCollapse, SfxCollapse)
	PUBLISH_VAR_AS     (m_obPfxImpaleRagdoll, PfxImpaleRagdoll)
	PUBLISH_VAR_AS     (m_obPfxImpaleSolid, PfxImpaleSolid)
	PUBLISH_VAR_AS     (m_obPfxDestroy, PfxDestroy)
	PUBLISH_VAR_AS     (m_obPfxCollapse, PfxCollapse)
	PUBLISH_VAR_AS     (m_obOnAftertouchStart, OnAftertouchStart)
	PUBLISH_VAR_AS     (m_obOnAftertouchEnd, OnAftertouchEnd)



/* // Damage meshes removed
	PUBLISH_VAR_AS     (m_iHitCount, HitCount)
	PUBLISH_VAR_AS     (m_obDamageMesh, DamageMesh)					
	PUBLISH_VAR_AS     (m_iDamageMeshCount, DamageMeshCount)
*/

/* // First person aiming removed
	PUBLISH_VAR_AS     (m_fAimedThrowTime, AimedThrowTime)
	PUBLISH_PTR_AS     (m_pobPlayerAimingMovement, PlayerAimingMovement)
	PUBLISH_PTR_AS     (m_pobPlayerThrowMovement, PlayerThrowMovement)
	PUBLISH_PTR_AS     (m_pobChasecamProperties, ChasecamProperties)
	PUBLISH_PTR_AS     (m_pobAimcamProperties, AimcamProperties)
*/
END_STD_INTERFACE

START_CHUNKED_INTERFACE(Interactable_Thrown, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(Interactable)
	COPY_INTERFACE_FROM(Interactable)

	OVERRIDE_DEFAULT(DefaultDynamics, "Rigid")

	PUBLISH_VAR_AS(m_Description, Description)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_InitialState, "Active", InitialState)
	PUBLISH_VAR_AS(m_AnimationContainer, AnimationContainer)
	PUBLISH_PTR_AS(m_pSharedAttributes, SharedAttributes)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bAttached, false, Attached)

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )	
END_STD_INTERFACE


//--------------------------------------------------
//!
//! Interactable Pushable State Machine
//!
//--------------------------------------------------
STATEMACHINE(INTERACTABLE_THROWN_FSM, Interactable_Thrown)
	INTERACTABLE_THROWN_FSM(bool bAttached)
	{
		if(bAttached)
			SET_INITIAL_STATE(ATTACHED);
		else
			SET_INITIAL_STATE(DEFAULT);
	}

	STATE(DEFAULT)
		BEGIN_EVENTS
			ON_ENTER

				//ntPrintf("Thrown %s: State Default ON_ENTER\n", ME->GetName().c_str() );	

				ME->m_pOther = 0;
				ME->m_pAttacker = 0;
				//ME->m_bAiming = false;

				ME->GetInteractionComponent()->Lua_SetInteractionPriority(PICKUP);
				ME->GetPhysicsSystem()->Lua_SetControllerDef(0, 0);

				// In default state this object is not being used.
				ME->m_bBeingUsed = false;
			END_EVENT(true)

			ON_EXIT
				// In other states this object is being used.
				ME->m_bBeingUsed = true;
			END_EVENT(true)

			EVENT(msg_collision)
			{
				//ntPrintf("Thrown %s: State Default msg_collision\n", ME->GetName().c_str() );	

				Att_Thrown* pSharedAttrs = ME->GetSharedAttributes();

				// Make sure the object was travelling above the threshold velocity
				if (msg.GetFloat("ProjVel") > -pSharedAttrs->m_fImpactThreshold && msg.GetFloat("ProjVel") < pSharedAttrs->m_fImpactThreshold )
				{
					END_EVENT(true);
				}

				/*
				// FIX ME Commented out as Att_Thrown doesn't appear to have PfxImpact at the moment
				if (!ntStr::IsNull(pSharedAttrs->m_obPfxImpact))
				{
					FXHelper::Pfx_CreateStatic(pSharedAttrs->m_obPfxImpact, ME, CHashedString("ROOT"));
				}*/

				/* // Damage meshes removed
				CEntity* pCollidee = msg.GetEnt("Collidee");

				if (pCollidee)
				{

					// FIX ME this should be reworked for CHashedStrings
					LuaAttributeTable* pCollideeAttrs = pCollidee->GetAttributeTable();
					ntstd::String weaponClass = pCollideeAttrs->GetString("WeaponClass");
					const char * pcWeaponClass = weaponClass.c_str();

					if (stricmp(pcWeaponClass, "speed") == 0)
					{
						ME->OnDamage(2);
					}
					else if (stricmp(pcWeaponClass, "power") == 0)
					{
						ME->OnDamage(4);
					}
					else if (stricmp(pcWeaponClass, "range") == 0)
					{
						ME->OnDamage(1);
					}
					//--------------------------------------------------
				}*/
			}
			END_EVENT(true)

			EVENT(msg_action)
				
				//ntPrintf("Thrown %s: State Default msg_action\n", ME->GetName().c_str() );	

				ME->m_pOther = (Character*)msg.GetEnt(CHashedString(HASH_STRING_OTHER));
				ntError(!ME->m_pOther || ME->m_pOther->IsCharacter());
				ME->m_pAttacker = ME->m_pOther;
				
				SET_STATE(MOVETO);
			END_EVENT(true)

			EVENT(msg_running_action)

				//ntPrintf("Thrown %s: State Default msg_running_action\n", ME->GetName().c_str() );	

				ME->m_pOther = (Character*)msg.GetEnt(CHashedString(HASH_STRING_OTHER));
				ntError(!ME->m_pOther || ME->m_pOther->IsCharacter());
				ME->m_pAttacker = ME->m_pOther;
				
				SET_STATE(RUNTO);
			END_EVENT(true)

			EVENT(msg_action_specific)
				ME->m_pOther = (Character*)msg.GetEnt(CHashedString(HASH_STRING_OTHER));
				ntError(!ME->m_pOther || ME->m_pOther->IsCharacter());
				ME->m_pAttacker = ME->m_pOther;

				SET_STATE(NORMALPICKUP);
				//SET_STATE(CLIMB);
			END_EVENT(true)
			
			EVENT(msg_action_specific_run)
				ME->m_pOther = (Character*)msg.GetEnt(CHashedString(HASH_STRING_OTHER));
				ntError(!ME->m_pOther || ME->m_pOther->IsCharacter());
				ME->m_pAttacker = ME->m_pOther;
	
				SET_STATE(RUNPICKUP);
				//SET_STATE(CLIMB);
			END_EVENT(true)
			
			EVENT(msg_goto_attachedstate)
			{
				SET_STATE(ATTACHED);
			}
			END_EVENT(true)

			EVENT(msg_equip)
			{
				CEntity* pUser = msg.GetEnt(CHashedString(HASH_STRING_OTHER));
				ntError_p(pUser, ("No user passed with msg_equip"));
				ntError_p(pUser->IsCharacter(), ("Non-character user passed with msg_equip"));
				pUser->ToCharacter()->SetInteractionTarget(ME);
				ME->ReparentObject(pUser);
				ME->GetPhysicsSystem()->Activate();
				SET_STATE(HELD);
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	STATE(ATTACHED)
		BEGIN_EVENTS
			ON_ENTER
				ME->GetInteractionComponent()->SetInteractionType(NONE);
				ME->GetPhysicsSystem()->Lua_DeactivateState("Rigid");
			END_EVENT(true)

			EVENT(msg_detach)
			{
				ME->Lua_ReparentToWorld();
				ME->GetPhysicsSystem()->Lua_ActivateState("Rigid");
				
				CDirection ZeroDir(CVecMath::GetZeroVector());
				ME->GetPhysicsSystem()->Lua_AltSetLinearVelocity(ZeroDir);
				
				ME->GetPhysicsSystem()->Lua_CompoundRigid_CheckAtRest();

				SET_STATE(DEFAULT);
			}
			END_EVENT(true)

			EVENT(msg_detach_velocity)
			{
				ME->Lua_ReparentToWorld();
				ME->GetPhysicsSystem()->Lua_ActivateState("Rigid");

				//Retrieve new velocity from message parameters.
				float fXVel = msg.GetFloat("XVel");
				float fYVel = msg.GetFloat("YVel");
				float fZVel = msg.GetFloat("ZVel");
				//Set the velocity of this piece.
				CDirection NewVel(fXVel, fYVel, fZVel);
				ME->GetPhysicsSystem()->Lua_AltSetLinearVelocity(NewVel);

				ME->GetPhysicsSystem()->Lua_CompoundRigid_CheckAtRest();

				SET_STATE(DEFAULT);
			}

			EVENT(msg_atrest)
				SET_STATE(DEFAULT);
			END_EVENT(true)

			EVENT(msg_deactivate)
			{
				SET_STATE(INACTIVE);
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	STATE(MOVETO)
		BEGIN_EVENTS
			ON_ENTER
			{
				//ntPrintf("Thrown %s: State MOVETO ON_ENTER\n", ME->GetName().c_str() );	

				Att_Thrown* pSharedAttrs = ME->GetSharedAttributes();
				CEntity* pPlayer = ME->m_pOther;

				pPlayer->GetMovement()->Lua_StartMoveToTransition(pSharedAttrs->m_obAnimPlayerMoveTo, ME, 1, 1);
				pPlayer->GetMovement()->Lua_AltSetMovementCompleteMessage( "msg_movementdone", pPlayer );

				ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);
			}
			END_EVENT(true)

			EVENT(msg_interrupt)

				//ntPrintf("Thrown %s: State MOVETO msg_interrupt\n", ME->GetName().c_str() );	

				SET_STATE(DEFAULT);
			END_EVENT(true)

			EVENT(msg_movementdone)
				//ntPrintf("Thrown %s: State MOVETO msg_movementdone\n", ME->GetName().c_str() );	
				SET_STATE(NORMALPICKUP);
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(RUNTO)
		BEGIN_EVENTS
			ON_ENTER
			{
				//ntPrintf("Thrown %s: State RUNTO ON_ENTER\n", ME->GetName().c_str() );	
				Att_Thrown* pSharedAttrs = ME->GetSharedAttributes();
				CEntity* pPlayer = ME->m_pOther;

				pPlayer->GetMovement()->Lua_StartMoveToTransition(pSharedAttrs->m_obAnimPlayerRunTo, ME, 1, 1);
				pPlayer->GetMovement()->Lua_AltSetMovementCompleteMessage( "msg_movementdone", pPlayer );

				ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);
			}
			END_EVENT(true)

			EVENT(msg_interrupt)
				//ntPrintf("Thrown %s: State RUNTO msg_interrupt\n", ME->GetName().c_str() );	
				SET_STATE(DEFAULT);
			END_EVENT(true)

			EVENT(msg_movementdone)
				//ntPrintf("Thrown %s: State RUNTO msg_movementdone\n", ME->GetName().c_str() );	
				SET_STATE(RUNPICKUP);
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(NORMALPICKUP)
		BEGIN_EVENTS
			EVENT(msg_think_onquickthrow)
				//ntPrintf("Thrown %s: State NORMALPICKUP msg_think_onquickthrow\n", ME->GetName().c_str() );	
				if (ME->m_bDoQuickThrow)
				{
					SET_STATE(THROW);
				}
			END_EVENT(true)

			EVENT(msg_think_onquickthrowcheck)
			{
				//ntPrintf("Thrown %s: State NORMALPICKUP msg_think_onquickthrowcheck\n", ME->GetName().c_str() );	
				ME->m_bCheckQuickThrow = true;

				Message msgOnQuickThrow(msg_think_onquickthrow);
				ME->GetMessageHandler()->QueueMessageDelayed(msgOnQuickThrow, 0.6f);
			}
			END_EVENT(true)

			EVENT(msg_think_onreparent)
			{
				ME->ReparentObject(ME->m_pOther);

				Message msgOnQuickThrowCheck(msg_think_onquickthrowcheck);
				ME->GetMessageHandler()->QueueMessageDelayed(msgOnQuickThrowCheck, 0.1f);
			}
			END_EVENT(true)

			ON_ENTER
			{
				//ntPrintf("Thrown %s: State NORMALPICKUP ON_ENTER\n", ME->GetName().c_str() );
				CEntity* pPlayer = ME->m_pOther;
				Att_Thrown* pSharedAttrs = ME->GetSharedAttributes();

				pPlayer->GetMovement()->Lua_AltStartFacingMovement(pSharedAttrs->m_obAnimPlayerPickup, 360.0f, 1.0f, 0.0f, 0.0f, 0.01f);
				pPlayer->GetMovement()->Lua_AltSetMovementCompleteMessage( "msg_movementdone", pPlayer );

				ME->m_bDoQuickThrow = false;
				ME->m_bCheckQuickThrow = false;

				Message msgOnReparent(msg_think_onreparent);
				ME->GetMessageHandler()->QueueMessageDelayed(msgOnReparent, 0.05f);
			}
			END_EVENT(true)

			ON_EXIT
			{
				// Reactivate once pick is complete/abandoned.
				ME->GetPhysicsSystem()->Activate();
			}
			END_EVENT(true)
			
			EVENT(msg_interrupt)
				//ntPrintf("Thrown %s: State NORMALPICKUP msg_interrupt\n", ME->GetName().c_str() );
				SET_STATE(INTERUPT);
			END_EVENT(true)

			EVENT(msg_movementdone)
				//ntPrintf("Thrown %s: State NORMALPICKUP msg_movementdone\n", ME->GetName().c_str() );
				SET_STATE(HELD);
			END_EVENT(true)

			EVENT(msg_action_on)
				//ntPrintf("Thrown %s: State NORMALPICKUP msg_action_on\n", ME->GetName().c_str() );
				if (ME->m_bCheckQuickThrow)
				{
					ME->m_bDoQuickThrow = true;
				}
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(RUNPICKUP)
		BEGIN_EVENTS
			EVENT(msg_think_onquickthrow)
				//ntPrintf("Thrown %s: State RUNPICKUP msg_think_onquickthrow\n", ME->GetName().c_str() );
				if (ME->m_bDoQuickThrow)
				{
					SET_STATE(THROW);
				}
			END_EVENT(true)

			EVENT(msg_think_onquickthrowcheck)
			{
				//ntPrintf("Thrown %s: State RUNPICKUP msg_think_onquickthrowcheck\n", ME->GetName().c_str() );
				ME->m_bCheckQuickThrow = true;

				Message msgOnQuickThrow(msg_think_onquickthrow);
				ME->GetMessageHandler()->QueueMessageDelayed(msgOnQuickThrow, 0.6f);
			}
			END_EVENT(true)

			EVENT(msg_think_onreparent)
			{
				ME->ReparentObject(ME->m_pOther);

				Message msgOnQuickThrowCheck(msg_think_onquickthrowcheck);
				ME->GetMessageHandler()->QueueMessageDelayed(msgOnQuickThrowCheck, 0.1f);
			}
			END_EVENT(true)

			ON_ENTER
			{
				//ntPrintf("Thrown %s: State RUNPICKUP ON_ENTER\n", ME->GetName().c_str() );
				CEntity* pPlayer = ME->m_pOther;
				Att_Thrown* pSharedAttrs = ME->GetSharedAttributes();

				pPlayer->GetMovement()->Lua_AltStartFacingMovement(pSharedAttrs->m_obAnimPlayerRunPickup, 360.0f, 1.0f, 0.0f, 0.0f, 0.01f);
				pPlayer->GetMovement()->Lua_AltSetMovementCompleteMessage( "msg_movementdone", pPlayer );

				ME->m_bDoQuickThrow = false;
				ME->m_bCheckQuickThrow = false;

				Message msgOnReparent(msg_think_onreparent);
				ME->GetMessageHandler()->QueueMessageDelayed(msgOnReparent, 0.05f);
			}
			END_EVENT(true)
			
			ON_EXIT
			{
				// Reactivate once pick is complete/abandoned.
				ME->GetPhysicsSystem()->Activate();
			}
			END_EVENT(true)

			EVENT(msg_interrupt)
				//ntPrintf("Thrown %s: State RUNPICKUP msg_interrupt\n", ME->GetName().c_str() );
				SET_STATE(INTERUPT);
			END_EVENT(true)

			EVENT(msg_movementdone)
				//ntPrintf("Thrown %s: State RUNPICKUP msg_movementdone\n", ME->GetName().c_str() );
				SET_STATE(HELD);
			END_EVENT(true)

			EVENT(msg_action_on)
				//ntPrintf("Thrown %s: State RUNPICKUP msg_action_on\n", ME->GetName().c_str() );
				if (ME->m_bCheckQuickThrow)
				{
					ME->m_bDoQuickThrow = true;
				}
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(HELD)
		BEGIN_EVENTS
			ON_ENTER
			{
				//ntPrintf("Thrown %s: State HELD ON_ENTER\n", ME->GetName().c_str() );
				CEntity* pPlayer = ME->m_pOther;
				Att_Thrown* pSharedAttrs = ME->GetSharedAttributes();

				pPlayer->GetPhysicsSystem()->Lua_SetHoldingCapsule(true);
				pPlayer->GetMovement()->BringInNewController(*pSharedAttrs->m_pobPlayerHoldingMovement, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND);

				// First person aiming removed
				//pPlayer->Lua_ResetAimingComponent();
				//CamMan::Get().GetView(0)->ActivateChaseAimCamDef(pPlayer, pSharedAttrs->m_pobChasecamProperties, pSharedAttrs->m_pobAimcamProperties);
				//ME->m_bAiming = false;
			}
			END_EVENT(true)

			EVENT(msg_interrupt)
				//ntPrintf("Thrown %s: State HELD msg_interrupt\n", ME->GetName().c_str() );
				SET_STATE(INTERUPT);
			END_EVENT(true)

			//EVENT(msg_power_on)
			//EVENT(msg_attack_on)
			EVENT(msg_action_on)
				//ntPrintf("Thrown %s: State HELD msg_action_on\n", ME->GetName().c_str() );
				SET_STATE(THROW);
			END_EVENT(true)

			EVENT(msg_grab_on)
				//ntPrintf("Thrown %s: State HELD msg_grab_on\n", ME->GetName().c_str() );
				SET_STATE(DROP);
			END_EVENT(true)

/*			// First person aiming removed
			EVENT(msg_aim_on)
			{
				CEntity* pPlayer = ME->m_pOther;
				Att_Thrown* pSharedAttrs = ME->GetSharedAttributes();

				pPlayer->GetMovement()->BringInNewController(*pSharedAttrs->m_pobPlayerAimingMovement);
				pPlayer->Lua_ResetAimingComponent();

				ME->m_bAiming = true;

				if (pSharedAttrs->m_bRebound && !pSharedAttrs->m_bCollapseOnDamage)
				{
					ME->GetPhysicsSystem()->Lua_Rigid_DeflectionRender(true);
				}
			}
			END_EVENT(true)

			EVENT(msg_aim_off)
			{
				CEntity* pPlayer = ME->m_pOther;
				Att_Thrown* pSharedAttrs = ME->GetSharedAttributes();

				pPlayer->Lua_ResetAimingComponent();
				pPlayer->GetMovement()->BringInNewController(*pSharedAttrs->m_pobPlayerHoldingMovement);

				ME->m_bAiming = false;

				if (pSharedAttrs->m_bRebound && !pSharedAttrs->m_bCollapseOnDamage)
				{
					ME->GetPhysicsSystem()->Lua_Rigid_DeflectionRender(false);
				}
			}
			END_EVENT(true)
*/
		END_EVENTS
	END_STATE

	STATE(THROW)
		BEGIN_EVENTS
			EVENT(msg_think_onaftertouch)
			{
				//ntPrintf("Thrown %s: State THROW msg_think_onaftertouch\n", ME->GetName().c_str() );
				CEntity* pPlayer = ME->m_pOther;

				// Only Action to AT
				//if (pPlayer->Lua_IsPowerHeld(0.2f) || pPlayer->Lua_IsAttackHeld(0.2f) || pPlayer->Lua_IsActionHeld(0.2f))
				if (pPlayer->Lua_IsActionHeld(0.2f))
				{
					SET_STATE(AFTERTOUCH);
				}
			}
			END_EVENT(true)

			EVENT(msg_think_onthrow)
			{
				//ntPrintf("Thrown %s: State THROW msg_think_onthrow\n", ME->GetName().c_str() );
				CEntity* pPlayer = ME->m_pOther;
				Att_Thrown* pSharedAttrs = ME->GetSharedAttributes();

				// Reparent the object back to the world
				ME->Lua_ReparentToWorld();

				bool stillHoldByCharacter; // id any collision appear during throwing, item is allready released
				if ( pSharedAttrs->m_bCollapseOnDamage )				
					stillHoldByCharacter = ME->GetPhysicsSystem()->Lua_CompoundRigid_IsInBreakableKeyframedMotion();				
				else
					stillHoldByCharacter = ME->GetPhysicsSystem()->Lua_Rigid_IsInBreakableKeyframedMotion();

				if (stillHoldByCharacter)
				{
					if ( pSharedAttrs->m_bCollapseOnDamage )	
					{
						// Re-position and reorientate the object relative to the player
						CPoint Pos = pSharedAttrs->m_obThrowTranslation;
						ME->GetPhysicsSystem()->Lua_CompoundRigid_MoveRelativeToEntitySafe (pPlayer, Pos.X(), Pos.Y(), Pos.Z());

						CQuat Rot = pSharedAttrs->m_obThrowOrientation;
						ME->GetPhysicsSystem()->Lua_CompoundRigid_SetRotationRelativeToEntitySafe (pPlayer, Rot.X(), Rot.Y(), Rot.Z(), Rot.W());

						ME->GetPhysicsSystem()->Lua_CompoundRigid_SetBreakableKeyframedMotion(false);

					}
					else
					{
						// Re-position and reorientate the object relative to the player
						CPoint Pos = pSharedAttrs->m_obThrowTranslation;
						ME->GetPhysicsSystem()->Lua_Rigid_MoveRelativeToEntitySafe (pPlayer, Pos.X(), Pos.Y(), Pos.Z());

						CQuat Rot = pSharedAttrs->m_obThrowOrientation;
						ME->GetPhysicsSystem()->Lua_Rigid_SetRotationRelativeToEntitySafe (pPlayer, Rot.X(), Rot.Y(), Rot.Z(), Rot.W());

						ME->GetPhysicsSystem()->Lua_Rigid_SetBreakableKeyframedMotion(false);
					}

					// Disregard colprim overlap between player and object
					ME->GetInteractionComponent()->Lua_AllowCollisionWith(pPlayer);

					CDirection ThrowVelocity(pSharedAttrs->m_obThrowVelocity);
					// First Person aiming removed
					/*if (ME->m_bAiming)
					{
					// Set the velocity of the object relative to the players direction 
					ME->GetPhysicsSystem()->Lua_AltSetLinearVelocityFromCamera(ThrowVelocity);
					}
					else*/
					{
						// OR towards their current throw target
						ME->GetPhysicsSystem()->Lua_AltSetLinearVelocityFromTarget(pPlayer, ThrowVelocity);
					}

					ME->GetPhysicsSystem()->Lua_AltSetAngularVelocity(CDirection(pSharedAttrs->m_obThrowAngularVelocity));

					if ( pSharedAttrs->m_bCollapseOnDamage )
					{
						ME->GetPhysicsSystem()->Lua_CompoundRigid_AntiGravity(10.0f, 1.0f);
						ME->GetPhysicsSystem()->Lua_CompoundRigid_CheckAtRest();
					}
					else
					{
						ME->GetPhysicsSystem()->Lua_Rigid_AntiGravity(10.0f, 1.0f);
						ME->GetPhysicsSystem()->Lua_Rigid_CheckAtRest();
					}

					if (pSharedAttrs->m_bOrientateToVelocity  && !pSharedAttrs->m_bCollapseOnDamage)
					{
						//Reorientate this entity to face the direction they are travelling in
						ME->GetPhysicsSystem()->Lua_Rigid_OrientateToVelocity(true);
					}

					if (pSharedAttrs->m_bRebound && !pSharedAttrs->m_bCollapseOnDamage)
					{
						ME->GetPhysicsSystem()->Lua_Rigid_EnableVelocityReflection(true);
						ME->GetPhysicsSystem()->Lua_Rigid_DeflectionRender(true);
					}

					ME->GetPhysicsSystem()->Lua_CollisionStrike(true);
				}
				else
				{
					// item was already released
					ME->GetInteractionComponent()->Lua_AllowCollisionWith(pPlayer);

					if ( pSharedAttrs->m_bCollapseOnDamage )
					{
						ME->GetPhysicsSystem()->Lua_CompoundRigid_CheckAtRest();
					}
					else
					{
						ME->GetPhysicsSystem()->Lua_Rigid_CheckAtRest();
					}
				}

				// Added this in to delay the aftertouch for one frame to give the physics 
				// system time to get the initial position/velocity correct BEFORE we change the time scalar!
				Message msgOnAftertouch(msg_think_onaftertouch);
				ME->GetMessageHandler()->QueueMessage(msgOnAftertouch);

				ME->m_bThrown = true;

				// Set a collision msg callback
				ME->GetPhysicsSystem()->GetCollisionCallbackHandler()->SetCollisionCallback(5.0f);
			}
			END_EVENT(true)

			ON_ENTER // THROW
			{
				//ntPrintf("Thrown %s: State THROW ON_ENTER\n", ME->GetName().c_str() );
				CEntity* pPlayer = ME->m_pOther;
				Att_Thrown* pSharedAttrs = ME->GetSharedAttributes();

				// Switch from keyframed -> breakablekeyframed motion (avoids penetration)
				if ( pSharedAttrs->m_bCollapseOnDamage )
					ME->GetPhysicsSystem()->Lua_CompoundRigid_SetBreakableKeyframedMotion(true);
				else
					ME->GetPhysicsSystem()->Lua_Rigid_SetBreakableKeyframedMotion(true);

				//if (!ME->m_bAiming)
				{
					pPlayer->GetMovement()->Lua_AltStartTargetedFacingMovement(pSharedAttrs->m_obAnimPlayerThrow, 360.0f, 1.0f, 0.0f, 0.1f, 0.0f);
					pPlayer->GetMovement()->Lua_AltSetMovementCompleteMessage( "msg_movementdone", pPlayer );
				}
				// First person aiming removed
				/*else
				{
					pPlayer->GetMovement()->BringInNewController(*pSharedAttrs->m_pobPlayerThrowMovement);
					pPlayer->GetMovement()->Lua_AltSetMovementCompleteMessage( "msg_movementdone", pPlayer );
				}*/

				ME->m_bMovementDone = false;
				ME->m_bImpaled = false;
				ME->m_bThrown = false;

				Message msgOnThrow(msg_think_onthrow);

				//if (!ME->m_bAiming)
				{
					ME->GetMessageHandler()->QueueMessageDelayed(msgOnThrow, pSharedAttrs->m_fThrowTime);
				}
				/*else
				{
					ME->GetMessageHandler()->QueueMessageDelayed(msgOnThrow, pSharedAttrs->m_fAimedThrowTime);
				}*/
			}
			END_EVENT(true)

			EVENT(msg_interrupt)
				//ntPrintf("Thrown %s: State THROW msg_interrupt\n", ME->GetName().c_str() );
				// Should only interupt when attached to player.
				if (! ME->m_bThrown )
				{
					SET_STATE(INTERUPT);
				}
			END_EVENT(true)

			EVENT(msg_atrest)
			{
				//ntPrintf("Thrown %s: State THROW msg_atrest\n", ME->GetName().c_str() );
				Att_Thrown* pSharedAttrs = ME->GetSharedAttributes();

				ME->GetPhysicsSystem()->Lua_CollisionStrike(false);

				if (pSharedAttrs->m_bRebound && !pSharedAttrs->m_bCollapseOnDamage )
				{
					ME->GetPhysicsSystem()->Lua_Rigid_EnableVelocityReflection(false);
					ME->GetPhysicsSystem()->Lua_Rigid_DeflectionRender(false);
				}

				SET_STATE(DEFAULT);
			}
			END_EVENT(true)

			EVENT(msg_movementdone)
			{
				//ntPrintf("Thrown %s: State THROW msg_movementdone\n", ME->GetName().c_str() );
				CEntity* pPlayer = ME->m_pOther;

				CamMan::Get().GetView(0)->RemoveCoolCamera( ME->m_nThrownCameraHandle );

				Message msgExitState(msg_exitstate);
				msgExitState.SetEnt( CHashedString(HASH_STRING_SENDER), ME);
				pPlayer->GetMessageHandler()->QueueMessage(msgExitState);

				ME->m_pOther = 0;
				ME->m_bMovementDone = true;

				if (ME->m_bImpaled)
				{
					SET_STATE(INACTIVE);
				}
			}
			END_EVENT(true)

			EVENT(msg_deflection)
				//CamMan::Get().GetView(0)->AfterTouchCoolCamLookAt(ME, 1);
			END_EVENT(true)

			EVENT(msg_collision)
			{
				//ntPrintf("Thrown %s: State THROW msg_collision\n", ME->GetName().c_str() );
				Att_Thrown* pSharedAttrs = ME->GetSharedAttributes();

				if ( !pSharedAttrs->m_bCollapseOnDamage )
				{
				    ME->GetPhysicsSystem()->Lua_Rigid_OrientateToVelocity(false);
				    ME->GetPhysicsSystem()->Lua_Rigid_DeflectionRender(false);
				}

				if (msg.GetFloat("ProjVel") > -pSharedAttrs->m_fImpactThreshold && msg.GetFloat("ProjVel") < pSharedAttrs->m_fImpactThreshold )
				{
					END_EVENT(true);
				}

				/*
				// FIX ME Commented out as Att_Thrown doesn't appear to have PfxImpact at the moment
				if (!ntStr::IsNull(pSharedAttrs->m_obPfxImpact))
				{
					FXHelper::Pfx_CreateStatic(pSharedAttrs->m_obPfxImpact, ME, CHashedString("ROOT"));
				}
				*/

				//ME->OnDamage(1);

				if (!ntStr::IsNull(pSharedAttrs->m_obThrownAttackData))
				{
					CombatHelper::Combat_GenerateStrike(ME, ME->m_pAttacker, msg.GetEnt("Collidee"), pSharedAttrs->m_obThrownAttackData);
				}

				if ( ME->m_bThrown && pSharedAttrs->m_bRemoveOnDamage && pSharedAttrs->m_bDamageOnEnv )
				{
					SET_STATE(DESTROY);
				}
			}
			END_EVENT(true)

			EVENT(msg_antigrav_off)
			{
				//ntPrintf("Thrown %s: State THROW msg_antigrav_off\n", ME->GetName().c_str() );
				Att_Thrown* pSharedAttrs = ME->GetSharedAttributes();
				if (pSharedAttrs->m_bRebound && !pSharedAttrs->m_bCollapseOnDamage )
				{
					ME->GetPhysicsSystem()->Lua_Rigid_EnableVelocityReflection(false);
				}
			}
			END_EVENT(true)

			EVENT(msg_hitsolid) // Static collision
			{
				//ntPrintf("Thrown %s: State THROW msg_hitsolid\n", ME->GetName().c_str() );
				Att_Thrown* pSharedAttrs = ME->GetSharedAttributes();

				// Sound Effect - AudioHelper:: work in progress
				//if (!ntStr::IsNull(pSharedAttrs->m_obSfxImpaleSolid)
					// AudioHelper::PlayPhysicsSound(pSharedAttrs->m_obSfxImpaleSolid, ME);
				
				//Particle Effect
				if (!ntStr::IsNull(pSharedAttrs->m_obPfxImpaleSolid))
					FXHelper::Pfx_CreateStatic(pSharedAttrs->m_obPfxImpaleSolid, ME, CHashedString("ROOT"));

				if (ME->m_bThrown && pSharedAttrs->m_bCollapseOnDamage && pSharedAttrs->m_bDamageOnEnv)
				{
					SET_STATE( COLLAPSED );
				}
            }
			END_EVENT(true)

			EVENT(msg_hitragdoll)  // Percing collision
			{
				//ntPrintf("Thrown %s: State THROW msg_hitragdoll\n", ME->GetName().c_str() );
				Att_Thrown* pSharedAttrs = ME->GetSharedAttributes();
				
				// Sound Effect - AudioHelper:: work in progress
				/*
				if (!ntStr::IsNull(pSharedAttrs->m_obSfxImpaleRagdoll))
				{
					AudioHelper::PlayPhysicsSound(pSharedAttrs->m_obSfxImpaleRagdoll, ME);
					AudioHelper::PlayPannedSound("vox_sb", "shd_death", "vo_character", 8.0f, 4.0f, ME);
					}
				*/
				
				//Particle Effect
				if (!ntStr::IsNull(pSharedAttrs->m_obPfxImpaleSolid))
					FXHelper::Pfx_CreateStatic(pSharedAttrs->m_obPfxImpaleSolid, ME, CHashedString("ROOT"));

				ME->m_bImpaled = true;

				if (ME->m_bMovementDone)
				{
					SET_STATE(INACTIVE);
				}
			}
			END_EVENT(true)

			EVENT(msg_hitcharacter) // Charachter collision
			{
				//ntPrintf("Thrown %s: State THROW msg_hitcharacter\n", ME->GetName().c_str() );
				Att_Thrown* pSharedAttrs = ME->GetSharedAttributes();

				// Sound Effect - AudioHelper:: work in progress
				//if (!ntStr::IsNull(pSharedAttrs->m_obSfxImpaleSolid)
					// AudioHelper::PlayPhysicsSound(pSharedAttrs->m_obSfxImpaleSolid, ME);
				
				//Particle Effect
				if (!ntStr::IsNull(pSharedAttrs->m_obPfxImpaleSolid))
					FXHelper::Pfx_CreateStatic(pSharedAttrs->m_obPfxImpaleSolid, ME, CHashedString("ROOT"));


				if (ME->m_bThrown && pSharedAttrs->m_bCollapseOnDamage && pSharedAttrs->m_bDamageOnChar)
				{
					SET_STATE( COLLAPSED );
				}
            }
			END_EVENT(true)

			// First person aiming removed
			/*EVENT(msg_aim_on)
				ME->m_bAiming = true;
			END_EVENT(true)

			EVENT(msg_aim_off)
				ME->m_bAiming = false;
			END_EVENT(true)*/
		END_EVENTS
	END_STATE

	STATE(AFTERTOUCH)
		BEGIN_EVENTS
			ON_ENTER
			{
				//ntPrintf("Thrown %s: State AFTERTOUCH ON_ENTER\n", ME->GetName().c_str() );
				CEntity* pPlayer = ME->m_pOther;
				Att_Thrown* pSharedAttrs = ME->GetSharedAttributes();

				// Enter our aftertouch controller
				ME->m_nThrownCameraHandle = CamMan::Get().GetView(0)->ActivateAfterTouchCoolCamDef(ME, pSharedAttrs->m_pobAftertouchCamProperties);

				if (pPlayer)
				{
					ME->GetPhysicsSystem()->Lua_SetControllerDef(pPlayer, pSharedAttrs->m_pobAftertouchProperites);
				}

				ME->m_bImpaled = false;

				// LUA GLOBAL FUNCTION STUFF
				if (!ntStr::IsNull(pSharedAttrs->m_obOnAftertouchStart))
				{
					CLuaGlobal::CallLuaFunc(pSharedAttrs->m_obOnAftertouchStart, ME);
				}

				ME->GetPhysicsSystem()->Lua_SendMessageOnCollision(true);
			}
			END_EVENT(true)

			ON_EXIT
				//ntPrintf("Thrown %s: State AFTERTOUCH ON_EXIT\n", ME->GetName().c_str() );
				ME->GetPhysicsSystem()->Lua_SendMessageOnCollision(false);
			END_EVENT(true)

			EVENT(msg_interrupt)
			{
				//ntPrintf("Thrown %s: State AFTERTOUCH msg_interrupt\n", ME->GetName().c_str() );
				Att_Thrown* pSharedAttrs = ME->GetSharedAttributes();

				// LUA GLOBAL FUNCTION STUFF
				if (!ntStr::IsNull(pSharedAttrs->m_obOnAftertouchEnd))
				{
					CLuaGlobal::CallLuaFunc(pSharedAttrs->m_obOnAftertouchEnd, ME);
				}

				if (!ME->m_bImpaled)
				{
					SET_STATE(INTERUPT);
				}
				else
				{
					CamMan::Get().GetView(0)->RemoveCoolCamera( ME->m_nThrownCameraHandle );
					SET_STATE(INACTIVE);
				}
			}
			END_EVENT(true)
		
			EVENT(msg_power_off)
			EVENT(msg_attack_off)
			EVENT(msg_action_off)
				//ntPrintf("Thrown %s: State AFTERTOUCH msg_action_off\n", ME->GetName().c_str() );
				ME->Aftertouch_Power_Off();
			END_EVENT(true)

			EVENT(msg_deflection)
				//ntPrintf("Thrown %s: State AFTERTOUCH msg_deflection\n", ME->GetName().c_str() );
				CamMan::Get().GetView(0)->AfterTouchCoolCamLookAt(ME, 2);
			END_EVENT(true)

			EVENT(msg_collision)
			{
				//ntPrintf("Thrown %s: State AFTERTOUCH msg_collision\n", ME->GetName().c_str() );
				Att_Thrown* pSharedAttrs = ME->GetSharedAttributes();

				// Make sure the object was travelling above the threshold velocity
				if (msg.GetFloat("ProjVel") > -pSharedAttrs->m_fImpactThreshold && msg.GetFloat("ProjVel") < pSharedAttrs->m_fImpactThreshold )
				{
					END_EVENT(true)
				}

				/*
				// FIX ME Commented out as Att_Thrown doesn't appear to have PfxImpact at the moment
				if (!ntStr::IsNull(pSharedAttrs->m_obPfxImpact))
				{
					FXHelper::Pfx_CreateStatic(pSharedAttrs->m_obPfxImpact, ME, CHashedString("ROOT"));
				}
				*/

				if (!ntStr::IsNull(pSharedAttrs->m_obThrownAttackData))
				{
					CombatHelper::Combat_GenerateStrike(ME, ME->m_pAttacker, msg.GetEnt("Collidee"), pSharedAttrs->m_obThrownAttackData);
				}

				// LUA GLOBAL FUNCTION STUFF
				if (!ntStr::IsNull(pSharedAttrs->m_obOnAftertouchEnd))
				{
					CLuaGlobal::CallLuaFunc(pSharedAttrs->m_obOnAftertouchEnd, ME);
				}

				//ME->OnDamage(1);

				if ( ME->m_bThrown && pSharedAttrs->m_bRemoveOnDamage && pSharedAttrs->m_bDamageOnEnv )
				{
					SET_STATE(DESTROY);
				}
			}
			END_EVENT(true)

			EVENT(msg_atrest)
			{
				//ntPrintf("Thrown %s: State AFTERTOUCH msg_atrest\n", ME->GetName().c_str() );
				Att_Thrown* pSharedAttrs = ME->GetSharedAttributes();

				// LUA GLOBAL FUNCTION STUFF
				if (!ntStr::IsNull(pSharedAttrs->m_obOnAftertouchEnd))
				{
					CLuaGlobal::CallLuaFunc(pSharedAttrs->m_obOnAftertouchEnd, ME);
				}

				ME->GetPhysicsSystem()->Lua_CollisionStrike(false);

				if ( pSharedAttrs->m_bRebound && !pSharedAttrs->m_bCollapseOnDamage )
				{
					ME->GetPhysicsSystem()->Lua_Rigid_DeflectionRender(false);
					ME->GetPhysicsSystem()->Lua_Rigid_EnableVelocityReflection(false);
				}

				SET_STATE(DEFAULT);
			}
			END_EVENT(true)

			EVENT(msg_antigrav_off)
			{
				//ntPrintf("Thrown %s: State AFTERTOUCH msg_antigrav_off\n", ME->GetName().c_str() );
				Att_Thrown* pSharedAttrs = ME->GetSharedAttributes();

				if ( pSharedAttrs->m_bRebound && !pSharedAttrs->m_bCollapseOnDamage )
				{
					ME->GetPhysicsSystem()->Lua_Rigid_DeflectionRender(false);
					ME->GetPhysicsSystem()->Lua_Rigid_EnableVelocityReflection(false);
				}
			}
			END_EVENT(true)

			EVENT(msg_obj_collision)
				//ntPrintf("Thrown %s: State AFTERTOUCH msg_obj_collision\n", ME->GetName().c_str() );
				CamMan::Get().GetView(0)->AfterTouchCoolCamLookAt(ME, 0);
			END_EVENT(true)

			EVENT(msg_hitsolid)
			{
				//ntPrintf("Thrown %s: State AFTERTOUCH msg_hitsolid\n", ME->GetName().c_str() );
				Att_Thrown* pSharedAttrs = ME->GetSharedAttributes();

				// Sound Effect - AudioHelper:: work in progress
				//if (!ntStr::IsNull(pSharedAttrs->m_obSfxImpaleSolid))
					// AudioHelper::PlayPhysicsSound(pSharedAttrs->m_obSfxImpaleSolid, ME);
				
				//Particle Effect
				if (!ntStr::IsNull(pSharedAttrs->m_obPfxImpaleSolid))
					FXHelper::Pfx_CreateStatic(pSharedAttrs->m_obPfxImpaleSolid, ME, CHashedString("ROOT"));

				if (ME->m_bThrown && pSharedAttrs->m_bCollapseOnDamage && pSharedAttrs->m_bDamageOnEnv)
				{
					SET_STATE( COLLAPSED );
				}
			}
			END_EVENT(true)

			EVENT(msg_hitragdoll)
			{
				//ntPrintf("Thrown %s: State AFTERTOUCH msg_hitragdoll\n", ME->GetName().c_str() );
				Att_Thrown* pSharedAttrs = ME->GetSharedAttributes();
				
				// Sound Effect
				//if (!ntStr::IsNull(pSharedAttrs->m_obSfxImpaleRagdoll))
					// AudioHelper::PlayPhysicsSound(pSharedAttrs->m_obSfxImpaleRagdoll, ME);

				//Particle Effect
				if (!ntStr::IsNull(pSharedAttrs->m_obPfxImpaleSolid))
					FXHelper::Pfx_CreateStatic(pSharedAttrs->m_obPfxImpaleSolid, ME, CHashedString("ROOT"));

				ME->m_bImpaled = true;
			}
			END_EVENT(true)

			EVENT(msg_hitcharacter)
			{
				//ntPrintf("Thrown %s: State AFTERTOUCH msg_hitcharacter\n", ME->GetName().c_str() );
				Att_Thrown* pSharedAttrs = ME->GetSharedAttributes();

				// Sound Effect - AudioHelper:: work in progress
				//if (!ntStr::IsNull(pSharedAttrs->m_obSfxImpaleSolid)
					// AudioHelper::PlayPhysicsSound(pSharedAttrs->m_obSfxImpaleSolid, ME);
				
				//Particle Effect
				if (!ntStr::IsNull(pSharedAttrs->m_obPfxImpaleSolid))
					FXHelper::Pfx_CreateStatic(pSharedAttrs->m_obPfxImpaleSolid, ME, CHashedString("ROOT"));

				if (ME->m_bThrown && pSharedAttrs->m_bCollapseOnDamage && pSharedAttrs->m_bDamageOnChar)
				{
					SET_STATE( COLLAPSED );
				}
            }
			END_EVENT(true)

			/*EVENT(msg_aim_on)
				ME->m_bAiming = true;
			END_EVENT(true)

			EVENT(msg_aim_off)
				ME->m_bAiming = false;
			END_EVENT(true)*/
		END_EVENTS
	END_STATE

	STATE(DROP)
		BEGIN_EVENTS
			EVENT(msg_think_ondrop)
			{
				//ntPrintf("Thrown %s: State DROP msg_think_ondrop\n", ME->GetName().c_str() );
				CEntity* pPlayer = ME->m_pOther;
				Att_Thrown* pSharedAttrs = ME->GetSharedAttributes();
				
				CDirection vDropVelocity(pSharedAttrs->m_obDropVelocity);
				CDirection vVelocity = vDropVelocity * pPlayer->GetMatrix();
				ME->GetPhysicsSystem()->Lua_AltSetLinearVelocity(vVelocity);

				ME->Lua_ReparentToWorld();

				// Allow havok to take control of the object again
				if ( pSharedAttrs->m_bCollapseOnDamage )
				{
					ME->GetPhysicsSystem()->Lua_CompoundRigid_SetKeyframedMotion(false);
					ME->GetPhysicsSystem()->Lua_CompoundRigid_CheckAtRest();
				}
				else
				{
				ME->GetPhysicsSystem()->Lua_Rigid_SetKeyframedMotion(false);
					ME->GetPhysicsSystem()->Lua_Rigid_CheckAtRest();
				}

				ME->GetInteractionComponent()->Lua_AllowCollisionWith(pPlayer);
			}
			END_EVENT(true)

			ON_ENTER
			{
				//ntPrintf("Thrown %s: State DROP ON_ENTER\n", ME->GetName().c_str() );
				Character* pPlayer = ME->m_pOther;
				Att_Thrown* pSharedAttrs = ME->GetSharedAttributes();

				CamMan::Get().GetView(0)->RemoveCoolCamera( ME->m_nThrownCameraHandle );

				pPlayer->GetMovement()->Lua_AltStartFacingMovement(pSharedAttrs->m_obAnimPlayerDrop, 360.0f, 1.0f, 0.0f, 0.0f, 0.05f);
				pPlayer->GetMovement()->Lua_AltSetMovementCompleteMessage( "msg_movementdone", pPlayer );
				pPlayer->SetExitOnMovementDone(true);

				Message msgOnDropThing(msg_think_ondrop);
				ME->GetMessageHandler()->QueueMessageDelayed(msgOnDropThing, pSharedAttrs->m_fDropTime);
			}
			END_EVENT(true)

			EVENT(msg_interrupt)
				//ntPrintf("Thrown %s: State DROP msg_interrupt\n", ME->GetName().c_str() );
				SET_STATE(INTERUPT);
			END_EVENT(true)

			EVENT(msg_atrest)
				//ntPrintf("Thrown %s: State DROP msg_atrest\n", ME->GetName().c_str() );
				SET_STATE(DEFAULT);
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(INTERUPT)
		BEGIN_EVENTS
			ON_ENTER
			{
				//ntPrintf("Thrown %s: State INTERUPT ON_ENTER\n", ME->GetName().c_str() );
				Att_Thrown* pSharedAttrs = ME->GetSharedAttributes();

				CamMan::Get().GetView(0)->RemoveCoolCamera( ME->m_nThrownCameraHandle );

				ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);
				ME->Lua_ReparentToWorld();
				ME->GetPhysicsSystem()->Lua_SetControllerDef(0, 0);

				if (pSharedAttrs->m_bCollapseOnDamage)
				{
					ME->GetPhysicsSystem()->Lua_CompoundRigid_SetKeyframedMotion(false);
					ME->GetPhysicsSystem()->Lua_CompoundRigid_CheckAtRest();
				}
				else
				{
				ME->GetPhysicsSystem()->Lua_Rigid_SetKeyframedMotion(false);
					ME->GetPhysicsSystem()->Lua_Rigid_CheckAtRest();
				}

				if (ME->m_pOther)
				{
					ME->GetInteractionComponent()->Lua_AllowCollisionWith(ME->m_pOther);
					ME->m_pOther = 0;
				}

				if ( pSharedAttrs->m_bRebound && !pSharedAttrs->m_bCollapseOnDamage )
				{
					ME->GetPhysicsSystem()->Lua_Rigid_EnableVelocityReflection(false);
					ME->GetPhysicsSystem()->Lua_Rigid_DeflectionRender(false);
				}
			}
			END_EVENT(true)

			EVENT(msg_atrest)
				//ntPrintf("Thrown %s: State INTERUPT msg_atrest\n", ME->GetName().c_str() );
				SET_STATE(DEFAULT);
			END_EVENT(true)

			EVENT(msg_antigrav_off)
			{
				//ntPrintf("Thrown %s: State INTERUPT msg_antigrav_off\n", ME->GetName().c_str() );
				Att_Thrown* pSharedAttrs = ME->GetSharedAttributes();
				if (pSharedAttrs->m_bRebound && !pSharedAttrs->m_bCollapseOnDamage )
				{
					ME->GetPhysicsSystem()->Lua_Rigid_EnableVelocityReflection(false);
					ME->GetPhysicsSystem()->Lua_Rigid_DeflectionRender(false);
				}
			}
			END_EVENT(true)

			EVENT(msg_hitsolid)
			{
				//ntPrintf("Thrown %s: State INTERUPT msg_hitsolid\n", ME->GetName().c_str() );
				Att_Thrown* pSharedAttrs = ME->GetSharedAttributes();

				// Sound Effect - AudioHelper:: work in progress
				//if (!ntStr::IsNull(pSharedAttrs->m_obSfxImpaleSolid))
				//	AudioHelper::PlayPhysicsSound(pSharedAttrs->m_obSfxImpaleSolid, ME);

				//Particle Effect
				if (!ntStr::IsNull(pSharedAttrs->m_obPfxImpaleSolid))
					FXHelper::Pfx_CreateStatic(pSharedAttrs->m_obPfxImpaleSolid, ME, CHashedString("ROOT"));
			}
			END_EVENT(true)

			EVENT(msg_hitragdoll)
			{
				//ntPrintf("Thrown %s: State INTERUPT msg_hitragdoll\n", ME->GetName().c_str() );
				Att_Thrown* pSharedAttrs = ME->GetSharedAttributes();
				
				// Sound Effect - AudioHelper:: work in progress
				//if (!ntStr::IsNull(pSharedAttrs->m_obSfxImpaleRagdoll))
					// AudioHelper::PlayPhysicsSound(pSharedAttrs->m_obSfxImpaleRagdoll, ME);

				//Particle Effect
				if (!ntStr::IsNull(pSharedAttrs->m_obPfxImpaleSolid))
					FXHelper::Pfx_CreateStatic(pSharedAttrs->m_obPfxImpaleSolid, ME, CHashedString("ROOT"));

				ME->m_bImpaled = true;
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(INACTIVE)
		BEGIN_EVENTS
			ON_ENTER
				//ntPrintf("Thrown %s: State INACTIVE ON_ENTER\n", ME->GetName().c_str() );
				ME->m_pOther = 0;
				ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);
				ME->Lua_AnimStopAll();
				ME->GetPhysicsSystem()->Lua_DeactivateState("Rigid");
			END_EVENT(true)

			EVENT(msg_removefromworld)
			{
				//ntPrintf("Thrown %s: State INACTIVE msg_removefromworld\n", ME->GetName().c_str() );
				SET_STATE(DESTROY);
			}
			END_EVENT(true)

			EVENT(msg_activate)
			{
				//ntPrintf("Thrown %s: State INACTIVE msg_activate\n", ME->GetName().c_str() );
				SET_STATE(DEFAULT);
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(DESTROY)
		BEGIN_EVENTS
			EVENT(msg_think_onremoveobject)
				//ntPrintf("Thrown %s: State DESTROY msg_think_onremoveobject\n", ME->GetName().c_str() );
				if (ME->m_bMovementDone)
				{
					ME->Lua_RemoveSelfFromWorld();
				}
				else
				{
					Message OnRemoveMessage(msg_think_onremoveobject);
					ME->GetMessageHandler()->QueueMessageDelayed(OnRemoveMessage, 1.0f);
				}
			END_EVENT(true)

			ON_ENTER
			{
				//ntPrintf("Thrown %s: State DESTROY ON_ENTER\n", ME->GetName().c_str() );
				Att_Thrown* pSharedAttrs = ME->GetSharedAttributes();

				ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);
				ME->Lua_ReparentToWorld();
				ME->GetPhysicsSystem()->Deactivate();
				ME->GetPhysicsSystem()->Lua_RemoveChildEntities();
				ME->GetRenderableComponent()->AddRemoveAll_Game( false );
				ME->Lua_AnimStopAll();

				if (!ntStr::IsNull(pSharedAttrs->m_obPfxDestroy))
				{
					FXHelper::Pfx_CreateStatic(pSharedAttrs->m_obPfxDestroy, ME, CHashedString("ROOT"));
				}

				// Sound Effect - AudioHelper:: work in progress
				//if (!ntStr::IsNull(pSharedAttrs->m_obSfxDestroy))
					// AudioHelper::PlayPhysicsSound(pSharedAttrs->m_obSfxDestroy, ME);

				Message OnRemoveMessage(msg_think_onremoveobject);
				ME->GetMessageHandler()->QueueMessageDelayed(OnRemoveMessage, 1.0f);
			}
			END_EVENT(true)

			EVENT(msg_movementdone)
			{
				//ntPrintf("Thrown %s: State DESTROY msg_movementdone\n", ME->GetName().c_str() );
				CEntity* pPlayer = ME->m_pOther;

				CamMan::Get().GetView(0)->RemoveCoolCamera( ME->m_nThrownCameraHandle );

				Message msgExitState(msg_exitstate);
				msgExitState.SetEnt( CHashedString(HASH_STRING_SENDER), ME);
				pPlayer->GetMessageHandler()->QueueMessage(msgExitState);

				ME->m_pOther = 0;
				ME->m_bMovementDone = true;
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(COLLAPSED)
		BEGIN_EVENTS
			ON_ENTER
			{
				Att_Thrown* pSharedAttrs = ME->GetSharedAttributes();
				
				if (pSharedAttrs->m_bCollapseOnDamage)
				{
					ME->GetPhysicsSystem()->Lua_RemoveChildEntities();
					ME->GetPhysicsSystem()->Lua_CompoundRigid_Collapse();

					ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);

					if (!ntStr::IsNull(pSharedAttrs->m_obPfxCollapse) )
					{
						FXHelper::Pfx_CreateStatic(pSharedAttrs->m_obPfxCollapse, ME, "ROOT");
					}

					// Sound Effect - AudioHelper:: work in progress
					/*if (!ntStr::IsNull(pSharedAttrs->m_obSfxCollapse) )
					{
						AudioHelper::PlaySound(pSharedAttrs->m_obSfxCollapse);
					}*/
				}
			}
			END_EVENT(true)

			EVENT(msg_movementdone)
			{
				CEntity* pPlayer = ME->m_pOther;

				CamMan::Get().GetView(0)->RemoveCoolCamera( ME->m_nThrownCameraHandle );

				Message msgExitState(msg_exitstate);
				msgExitState.SetEnt( CHashedString(HASH_STRING_SENDER), ME);
				pPlayer->GetMessageHandler()->QueueMessage(msgExitState);

				ME->m_pOther = 0;
				ME->m_bMovementDone = true;
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE

END_STATEMACHINE //INTERACTABLE_THROWN_FSM


//--------------------------------------------------
//!
//!	Interactable_Thrown::Interactable_Thrown()
//!	Default constructor
//!
//--------------------------------------------------
Interactable_Thrown::Interactable_Thrown()
{
	m_eType = EntType_Interactable;
	m_eInteractableType = EntTypeInteractable_Thrown;

	m_pSharedAttributes = 0;

	// Default variable values
	m_pOther				= 0;
	m_pAttacker				= 0;
	//m_bAiming				= false;
	//m_nHits					= 0;
	m_nThrownCameraHandle	= 0;
	//m_nLastMesh				= 1;
	m_bDoQuickThrow			= false;
	m_bCheckQuickThrow		= false;
	m_bMovementDone			= false;
	m_bImpaled				= false;
	m_bAttached				= false;
	m_bThrown				= false;

	// Is this thrown object being used by someone?
	m_bBeingUsed			= false;
}

//--------------------------------------------------
//!
//!	Interactable_Thrown::OnPostConstruct()
//!	Post Construct
//!
//--------------------------------------------------
void Interactable_Thrown::OnPostConstruct()
{
	Interactable::OnPostConstruct();

	InstallMessageHandler();
	InstallAnimator("NULL");
	// Install network component here (if needed)

	//InstallDynamics();

	// Create rigid body physics
	Lua_CreatePhysicsSystem();	

	/* // Motion type appears obsolete - T McK

	LuaAttributeTable* pAttributeTable = NT_NEW LuaAttributeTable;
	pAttributeTable->SetDataObject(ObjectDatabase::Get().GetDataObjectFromPointer( (void*) m_pSharedAttributes ));	

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

	NT_DELETE(pAttributeTable);
	*/

	user_error_p( (GetSharedAttributes() != 0), (" The shared attributes missing for object %s", GetName().c_str()) ) ; 
	Physics::LogicGroup* lg = 0;
	if ( GetSharedAttributes()->m_bCollapseOnDamage )
	{
		// Create a compund rigid group for collapsable
		lg = NT_NEW Physics::CompoundLG(GetName(), this);
		lg->Load();
		//lg = (Physics::LogicGroup*)Physics::ClumpTools::ConstructCompoundLGFromClump( this, &obInfo );
	}
	else
	{
		//  Create a single rigid group 
		//lg = (Physics::LogicGroup*)Physics::ClumpTools::ConstructRigidLGFromClump( this,  &obInfo );
		lg = NT_NEW Physics::SingleRigidLG(GetName(), this);
		lg->Load();
	}

	if(lg)
	{
		// Add the group
		m_pobPhysicsSystem->AddGroup( (Physics::LogicGroup *) lg );
		lg->Activate();	
	}
	else	
		ntPrintf("%s(%d): ### PHYSICS ERROR - %s logic group not created for entity %s with clump %s\n", __FILE__, __LINE__, GetSharedAttributes()->m_bCollapseOnDamage ? "Compound Rigid" : "Single Rigid", GetName().c_str(), GetClumpString().c_str());	

	
	// Register physics sound
	if (!ntStr::IsNull(m_pSharedAttributes->m_obPhysicsSoundDef))
	{
		m_pobPhysicsSystem->RegisterCollisionEffectFilterDef(m_pSharedAttributes->m_obPhysicsSoundDef);
	}

	// Set the correct mesh if multiple meshes are defined
	/*int nDamageMeshCount = m_pSharedAttributes->m_iDamageMeshCount;
	if (nDamageMeshCount > 0)
	{
		// Hide all the damaged meshes
		char acMeshName[MAX_PATH];
		char acSpecificMeshName[MAX_PATH];

		sprintf( acMeshName, "%s", m_pSharedAttributes->m_obDamageMesh.c_str() );

		int i = 1;
		for (i = 1; i <= nDamageMeshCount; i++)
		{
			sprintf( acSpecificMeshName, "%s%d", acMeshName, i );
			Lua_SetMeshVisibility( acSpecificMeshName, false );
		}

		sprintf( acSpecificMeshName, "%s1", acMeshName );
		// FIX ME this func call needs the characters - could be solved with an ordered list of damage 
		// mesh names in Mr Ed rather than assuming %sN naming 
		Lua_SetMeshVisibility( acSpecificMeshName, true );
		m_nLastMesh = 1;
		}*/

	if ( GetSharedAttributes()->m_bCollapseOnDamage && GetSharedAttributes()->m_bOrientateToVelocity )
		ntPrintf("Entity %s: CollapseOnDamage incompatable with OrientateToVelocity\n", GetName().c_str());	

	if ( GetSharedAttributes()->m_bCollapseOnDamage && GetSharedAttributes()->m_bOrientateToVelocity )
		ntPrintf("Entity %s: CollapseOnDamage incompatable with Rebound\n", GetName().c_str());	

	// JML - Changing Duncans change back so people have weapons again!
	//       Do we even need this, no-one seems to be using it on the initialstate anyway...
	// State machine initial state
	if (m_InitialState == "Attached")
		m_bAttached = true;

	// Create and attach the statemachine
	INTERACTABLE_THROWN_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) INTERACTABLE_THROWN_FSM(m_bAttached);
	ATTACH_FSM(pFSM);
}


//----------------------------------------------------------------------------------------------------
//!
//!	Interactable_Thrown::InteractableInUse()
//! Checks if the thrown object is in use by someone
//!
//----------------------------------------------------------------------------------------------------
bool Interactable_Thrown::InteractableInUse( void ) const
{
	return m_bBeingUsed;
}


void Interactable_Thrown::Reset()
{
	EXTERNALLY_SET_STATE(INTERACTABLE_THROWN_FSM, DEFAULT);
}

//----------------------------------------------------------------------------------------------------
//!
//!	Interactable_Thrown::Attach()
//!	Force the weapon into it's attached state
//!
//----------------------------------------------------------------------------------------------------
void Interactable_Thrown::Attach()
{
	EXTERNALLY_SET_STATE(INTERACTABLE_THROWN_FSM, ATTACHED);
	Message msg(State_Enter);
	m_pFSM->ProcessMessage(msg);
}


//--------------------------------------------------
//!
//!	Interactable_Thrown::~Interactable_Thrown()
//!	Default destructor
//!
//--------------------------------------------------
Interactable_Thrown::~Interactable_Thrown()
{
}

// Damage meshes removed
//--------------------------------------------------
//!
//!	Interactable_Thrown::OnDamage()
//!	Applies damage
//!
//--------------------------------------------------
/*void Interactable_Thrown::OnDamage(int nDamage)
{
	if (m_pSharedAttributes->m_iHitCount > 0)
	{
		// Apply Damage
		m_nHits += nDamage;

		// Change the mesh if applicable
		int nDamageMeshCount = m_pSharedAttributes->m_iDamageMeshCount;

		if (nDamageMeshCount > 0)
		{
			int nIndex = nDamageMeshCount * m_nHits / m_pSharedAttributes->m_iHitCount;
			nIndex += 1;

			// Check bounds
			if (nIndex < 1)
				nIndex = 1;

			if (nIndex > nDamageMeshCount)
				nIndex = nDamageMeshCount;

			char acMeshName[MAX_PATH];
			sprintf( acMeshName, "%s%d", m_pSharedAttributes->m_obDamageMesh.c_str(), nIndex );

			if (m_nLastMesh != nIndex)
			{
				char acOldMeshName[MAX_PATH];
				sprintf( acMeshName, "%s%d", m_pSharedAttributes->m_obDamageMesh.c_str(), m_nLastMesh );

				// FIX ME these func calls need the characters - could be solved with an ordered list of damage 
				// mesh names in Mr Ed rather than assuming %sN naming 
				Lua_SetMeshVisibility(acOldMeshName, false);
				Lua_SetMeshVisibility(acMeshName, true);
				m_nLastMesh = nIndex;
			}

			if (m_nHits >= m_pSharedAttributes->m_iHitCount)
			{
				m_pFSM->SetState(INTERACTABLE_THROWN_FSM::DESTROY::GetInstance());
			}
		}
	}
}*/


//--------------------------------------------------
//!
//!	Interactable_Thrown::Aftertouch_Power_Off()
//!	Power off function - call by multiple aftertouch events
//! so its in its own function.
//!
//--------------------------------------------------
void Interactable_Thrown::Aftertouch_Power_Off()
{
	CEntity* pPlayer = m_pOther;
	Att_Thrown* pSharedAttrs = GetSharedAttributes();

	// LUA GLOBAL FUNCTION STUFF
	if (!ntStr::IsNull(pSharedAttrs->m_obOnAftertouchEnd))
	{
		CLuaGlobal::CallLuaFunc(pSharedAttrs->m_obOnAftertouchEnd, this);
	}

	CamMan::Get().GetView(0)->RemoveCoolCamera( m_nThrownCameraHandle );

	// Player can return to their default state
	Message msgExitState(msg_exitstate);
	msgExitState.SetEnt( CHashedString(HASH_STRING_SENDER), this);
	pPlayer->GetMessageHandler()->QueueMessage(msgExitState);

	m_pOther = 0;

	if (m_bImpaled)
	{
		m_pFSM->SetState(INTERACTABLE_THROWN_FSM::INACTIVE::GetInstance());
	}
	else
	{
		GetPhysicsSystem()->Lua_SetControllerDef(0, 0);
		if ( pSharedAttrs->m_bCollapseOnDamage )
			GetPhysicsSystem()->Lua_CompoundRigid_CheckAtRest();
		else
		GetPhysicsSystem()->Lua_Rigid_CheckAtRest();
	}

	if (pSharedAttrs->m_bRebound && !pSharedAttrs->m_bCollapseOnDamage)
	{
		GetPhysicsSystem()->Lua_Rigid_DeflectionRender(false);
	}
}


//----------------------------------------------------------------------------------------------------
//!
//!	Interactable_Thrown::ReparentObject()
//!	Reparent the object to the character
//!
//----------------------------------------------------------------------------------------------------
void Interactable_Thrown::ReparentObject(CEntity* pParent)
{
	ntError_p(pParent && pParent->IsCharacter(), ("Non characters cannot equip thrown objects!\n"));

	m_pOther = pParent->ToCharacter();

	GetInteractionComponent()->ExcludeCollisionWith(pParent);
	if ( GetSharedAttributes()->m_bCollapseOnDamage )
		GetPhysicsSystem()->Lua_CompoundRigid_SetKeyframedMotion(true);
	else
		GetPhysicsSystem()->Lua_Rigid_SetKeyframedMotion(true);
	Lua_SetIdentity();
	Lua_Reparent(pParent, "r_weapon");
	
	// Deactivate to stop the keyframed motion pinging things out of the way.
	GetPhysicsSystem()->Deactivate();
}
