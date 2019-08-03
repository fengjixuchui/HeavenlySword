//--------------------------------------------------
//!
//!	\file game/entitycatapultrock.cpp
//!	Definition of the Catapult rock object  based on Interactable Thrown entity object
//! could be merged back later if the majority of functionality remains the same. T McK
//!
//--------------------------------------------------

#include "game/entitycatapultrock.h"
#include "game/entitycatapult.h"

#include "objectdatabase/dataobject.h"
#include "game/luaattrtable.h"
#include "Physics/system.h"
#include "physics/world.h"
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

// Debug includes 
#include "core/OSDDisplay.h"

// Borrowing projectile manager functionality, could add a new specific manager if needed
#include "game/projectilemanager.h"

// Components needed
#include "game/interactioncomponent.h"

#ifdef PLATFORM_PS3
#include "army/armymanager.h"	// For bazooka position and explosion notification
#endif // PLATFORM_PS3

#define ROCK_TIME_OUT_IN_SECS 20.f

//Global ammo ID value (this is how it was done in LUA for projectiles).
int g_iAmmoID = 0;

void ForceLinkFunctionCatapultRock()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionCatapultRock() !ATTN!\n");
}

START_STD_INTERFACE(Att_Ammo)
	PUBLISH_VAR_AS	(m_obLaunchTransform,		LaunchTransform)
	PUBLISH_VAR_AS	(m_obLaunchTranslation,		LaunchTranslation)
	PUBLISH_VAR_AS	(m_obLaunchOrientation,		LaunchOrientation)
	//PUBLISH_VAR_AS	(m_fLaunchLinearSpeed,		LaunchLinearSpeed)
	PUBLISH_VAR_AS	(m_fLaunchHeight,			LaunchHeight)
	PUBLISH_VAR_AS	(m_obLaunchAngularVelocity,	LaunchAngularVelocity)
	PUBLISH_VAR_AS	(m_fLaunchTime,				LaunchTime)
	PUBLISH_VAR_AS	(m_obTarget,				Target)
	PUBLISH_VAR_AS	(m_obNameAppend,			NameAppend)
	PUBLISH_VAR_AS	(m_obClump,					Clump)
	PUBLISH_VAR_AS	(m_obFireAnim,				AnimationFire)
	PUBLISH_VAR_AS	(m_obReloadAnim,			AnimationReload)
	PUBLISH_VAR_AS	(m_fRespawnTime,			RespawnTime)
	PUBLISH_VAR_AS	(m_fAutoResetTime,			AutoReloadTime)
END_STD_INTERFACE

START_STD_INTERFACE(Att_Catapult_Rock)
	PUBLISH_VAR_AS     (m_obThrownAttackData,			ThrownAttackData)			// The definition that defines the strike data
	PUBLISH_VAR_AS     (m_bDamageOnChar,				DamageOnChar)				// Flag if the object will break in contact with a character
	PUBLISH_VAR_AS     (m_bDamageOnEnv,					DamageOnEnv)				// Flag if the object will break in contact with the environment
	PUBLISH_VAR_AS     (m_bCollapseOnDamage,			CollapseOnDamage)			// Do compound rigid collapse on damage
	PUBLISH_VAR_AS     (m_bRemoveOnDamage,				RemoveOnDamage)				// Remove mesh on damage
	PUBLISH_VAR_AS     (m_bRebound,						Rebound)					// If the object will rebound for more fun, shields
	PUBLISH_VAR_AS     (m_bInstantKill,					InstantKill)				// If the object will kill on impact
	PUBLISH_PTR_AS     (m_pobAftertouchProperites,		AftertouchProperites)		// Aftertouch parameters for the object
	PUBLISH_PTR_AS     (m_pobAftertouchCamProperties,	AftertouchCamProperties)	// Aftertouch camera paramters
	PUBLISH_VAR_AS     (m_bAIAvoid,						AIAvoid)					// Sets if the AI will try to avoid this object
	PUBLISH_VAR_AS     (m_fAIAvoidRadius,				AIAvoidRadius)				// Sets the radius that the AI will try to avoid the object by

	// TODO (chipb) replace
	// PUBLISH_VAR_AS     (m_obPhysicsSoundDef,			PhysicsSoundDef)			// Sound definitions for the physics stuff

	PUBLISH_VAR_AS     (m_fImpactThreshold,				ImpactThreshold)
	PUBLISH_VAR_AS     (m_obSfxDestroy,					SfxDestroy)
	PUBLISH_VAR_AS     (m_obSfxCollapse,				SfxCollapse)
	PUBLISH_VAR_AS     (m_obPfxDestroy,					PfxDestroy)
	PUBLISH_VAR_AS     (m_obPfxCollapse,				PfxCollapse)
	PUBLISH_VAR_AS	   (m_fExplosionPush,				ExplosionPush)
	PUBLISH_VAR_AS     (m_fExplosionPushDropoff,		ExplosionPushDropoff)
	PUBLISH_VAR_AS     (m_fExplosionRadius,				ExplosionRadius)
	PUBLISH_CONTAINER_AS (m_aobPfxLoaded,				LoadedPfxList)
	PUBLISH_CONTAINER_AS (m_aobPfxFlight,				FlightPfxList)
	PUBLISH_CONTAINER_AS (m_aobPfxExplosion,			ExplosionPfxList)
	PUBLISH_VAR_AS     (m_obOnAftertouchStart,			OnAftertouchStart)
	PUBLISH_VAR_AS     (m_obOnAftertouchEnd,			OnAftertouchEnd)

	// Projectile audio
	PUBLISH_VAR_AS     (m_obLoopingSoundCue,			LoopingSoundCue);
	PUBLISH_VAR_AS     (m_obLoopingSoundBank,			LoopingSoundBank);
	PUBLISH_VAR_AS     (m_fLoopingSoundRange,			LoopingSoundRange);
	PUBLISH_VAR_AS     (m_obPassbySoundCue,				PassbySoundCue);
	PUBLISH_VAR_AS     (m_obPassbySoundBank,			PassbySoundBank);
	PUBLISH_VAR_AS     (m_fPassbySoundRange,			PassbySoundRange);
	PUBLISH_VAR_AS     (m_obIdleSoundCue,				IdleSoundCue);
	PUBLISH_VAR_AS     (m_obIdleSoundBank,				IdleSoundBank);
	PUBLISH_VAR_AS     (m_obImpactSoundCue,				ImpactSoundCue);
	PUBLISH_VAR_AS     (m_obImpactSoundBank,			ImpactSoundBank);
END_STD_INTERFACE

START_CHUNKED_INTERFACE(Object_Catapult_Rock, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(Interactable)
	COPY_INTERFACE_FROM(Interactable)

	PUBLISH_VAR_AS(m_Description, Description)
	PUBLISH_VAR_AS(m_InitialState, InitialState)
	PUBLISH_VAR_AS(m_AnimationContainer, AnimationContainer)
	PUBLISH_PTR_AS(m_pSharedAttributes, SharedAttributes)
	PUBLISH_VAR_AS(m_bAttached, Attached)

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )	
END_STD_INTERFACE


//--------------------------------------------------
//!
//! Catapult Rock State Machine
//!
//--------------------------------------------------
STATEMACHINE(OBJECT_CATAPULT_ROCK_FSM, Object_Catapult_Rock)
	OBJECT_CATAPULT_ROCK_FSM(bool bAttached)
	{
		if(bAttached)
			SET_INITIAL_STATE(ATTACHED);
		else
			SET_INITIAL_STATE(DEFAULT);
	}

	STATE(DEFAULT)
		BEGIN_EVENTS
			ON_ENTER
				ME->m_pOther = 0;

				ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);
				ME->GetPhysicsSystem()->Lua_SetControllerDef(0, 0);

				// Audio control
				ME->EnableProjectileIdleAudio(true);
				ME->EnableProjectileFlightAudio(true);
			END_EVENT(true)

			ON_UPDATE
			{
				// Maintain projectile idle audio
				ME->UpdateProjectileIdleAudio();
			}
			END_EVENT(true)

			EVENT(msg_collision)
			{
				Att_Catapult_Rock* pSharedAttrs = ME->GetSharedAttributes();

				// Make sure the object was travelling above the threshold velocity
				if (msg.GetFloat("ProjVel") > -pSharedAttrs->m_fImpactThreshold && msg.GetFloat("ProjVel") < pSharedAttrs->m_fImpactThreshold )
				{
					END_EVENT(true);
				}

				/*
				// FIX ME Commented out as Att_Catapult_Rock doesn't appear to have PfxImpact at the moment
				if (!ntStr::IsNull(pSharedAttrs->m_obPfxImpact))
				{
					FXHelper::Pfx_CreateStatic(pSharedAttrs->m_obPfxImpact, ME, CHashedString("ROOT"));
				}*/
			}
			END_EVENT(true)
			
			EVENT(msg_goto_attachedstate)
				SET_STATE(ATTACHED);
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(ATTACHED)
		BEGIN_EVENTS
			ON_ENTER
				//Add to the projectile manager.
				ProjectileManager::Get().AddProjectileToList(ME);


				//ME->GetInteractionComponent()->SetInteractionType(NONE);
				ME->GetPhysicsSystem()->Lua_DeactivateState("Rigid");

				// Audio control
				// ME->EnableProjectileIdleAudio(true);
				// ME->EnableProjectileFlightAudio(true);

				ME->Ignition();
			END_EVENT(true)

			ON_UPDATE
			{
				// Maintain projectile idle audio
				// ME->UpdateProjectileIdleAudio();
			}
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

			EVENT(msg_fire)
			{
				CPoint obPoint(CONSTRUCT_CLEAR);
				ME->m_pOther = msg.GetEnt(CHashedString(HASH_STRING_OTHER));
				
				ME->GetPhysicsSystem()->Lua_ActivateState("Rigid");
				
				//Retrieve world position from message parameters.
				if ( msg.IsNumber("X") || msg.IsNumber("Y") || msg.IsNumber("Z") )
				{
					if ( msg.IsNumber("X") )
						obPoint.X() = msg.GetFloat("X");
					if ( msg.IsNumber("Y") )
						obPoint.Y() = msg.GetFloat("Y");
					if ( msg.IsNumber("Z") )
						obPoint.Z() = msg.GetFloat("Z");
				}
				ME->m_obTarget = obPoint;
				
				SET_STATE(THROW);
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(THROW)
		BEGIN_EVENTS
			/*EVENT(msg_think_onaftertouch)
			{
				CEntity* pobOther = ME->m_pOther;

				// Only Action to AT
				//if (pobOther->Lua_IsPowerHeld(0.2f) || pobOther->Lua_IsAttackHeld(0.2f) || pobOther->Lua_IsActionHeld(0.2f))
				if (pobOther->Lua_IsActionHeld(0.2f))
				{
					SET_STATE(AFTERTOUCH);
				}
			}
			END_EVENT(true)*/

			EVENT(msg_think_onthrow)
			{
				CEntity* pobOther = ME->m_pOther;
				Att_Catapult_Rock* pSharedAttrs = ME->GetSharedAttributes();
				ntAssert(pSharedAttrs);

				// Reparent the object back to the world
				ME->Lua_ReparentToWorld();
			
				// If we have an other then we can reposition relative to that entity
				if ( pobOther )
				{
					CPoint Pos = ME->m_obLaunchTranslation;
					CQuat Rot = ME->m_obLaunchOrientation;
							
					if ( pSharedAttrs->m_bCollapseOnDamage )
					{
						if ( ME->GetPhysicsSystem()->Lua_CompoundRigid_IsInBreakableKeyframedMotion() )
						{
							// Re-position and reorientate the object relative to the player
							ME->GetPhysicsSystem()->Lua_CompoundRigid_MoveRelativeToEntitySafe (pobOther, Pos.X(), Pos.Y(), Pos.Z());
							ME->GetPhysicsSystem()->Lua_CompoundRigid_SetRotationRelativeToEntitySafe (pobOther, Rot.X(), Rot.Y(), Rot.Z(), Rot.W());

							ME->GetPhysicsSystem()->Lua_CompoundRigid_SetBreakableKeyframedMotion(false);
						}
					}
					else
					{
						if ( ME->GetPhysicsSystem()->Lua_Rigid_IsInBreakableKeyframedMotion() )
						{
							// Re-position and reorientate the object relative to the player
							ME->GetPhysicsSystem()->Lua_Rigid_MoveRelativeToEntitySafe (pobOther, Pos.X(), Pos.Y(), Pos.Z());
							ME->GetPhysicsSystem()->Lua_Rigid_SetRotationRelativeToEntitySafe (pobOther, Rot.X(), Rot.Y(), Rot.Z(), Rot.W());

							ME->GetPhysicsSystem()->Lua_Rigid_SetBreakableKeyframedMotion(false);
						}
					}
				}
				//... Otherwise, could assume spawned position/orintaion will be correct, or use parameters on the rock
				/*else
				{
					ME->SetPosition( ME->m_obLaunchTranslation );
					ME->SetRotation( ME->m_obLaunchOrientation );
				}*/


				// Disregard colprim overlap between rock and catapult (coz its special to deal with breaky catapult parts)
				if ( ME->GetCatapult() )
				{
					ME->GetCatapult()->AllowCollisionWith( ME, true );
				}
				// otherwise Disregard colprim overlap between rock and other
				else if ( pobOther )
				{
					ME->GetInteractionComponent()->Lua_AllowCollisionWith(pobOther);
				}

				CDirection obLaunchVelocity;

				// If I have a catapult, use it to calculate correct trajectory
				if ( ME->GetCatapult() )
				{
					// Origin in world space
					CPoint obOrigin = ME->m_obLaunchTranslation * ME->GetCatapult()->GetMatrix();

					// Parabola height in worldspace
					float fHeight = ME->GetCatapult()->GetPosition().Y() + ME->m_fLaunchHeight;

					obLaunchVelocity = ME->LaunchVelocity( obOrigin, ME->m_obTarget, fHeight );
				}
				// ... Otherwise use my velocity  (ie set this in barrage create static function)
				else
				{
					obLaunchVelocity = ME->m_obLaunchVelocity;
				}

				ME->GetPhysicsSystem()->Lua_AltSetLinearVelocity( obLaunchVelocity );

				ME->GetPhysicsSystem()->Lua_AltSetAngularVelocity( ME->m_obAngularVelocity );
				
				if ( pSharedAttrs->m_bCollapseOnDamage )
				{
					ME->GetPhysicsSystem()->Lua_CompoundRigid_CheckAtRest();
				}
				else
				{
				    ME->GetPhysicsSystem()->Lua_Rigid_CheckAtRest();
				}

				ME->GetPhysicsSystem()->Lua_CollisionStrike(true);

				// Added this in to delay the aftertouch for one frame to give the physics 
				// system time to get the initial position/velocity correct BEFORE we change the time scalar!
				//Message msgOnAftertouch(msg_think_onaftertouch);
				//ME->GetMessageHandler()->QueueMessage(msgOnAftertouch);
				
				ME->m_bThrown = true;

				// Set a collision msg callback
				ME->GetPhysicsSystem()->GetCollisionCallbackHandler()->SetCollisionCallback(5.0f);

				ME->OnLaunch();
			}
			END_EVENT(true)

			ON_ENTER // THROW
			{
				CEntity* pobOther = ME->m_pOther;
				Att_Catapult_Rock* pSharedAttrs = ME->GetSharedAttributes();
				ntAssert (pSharedAttrs);
				
				// Allow colprim overlap between rock and catapult (coz its special to deal with breaky catapult parts)
				if ( ME->GetCatapult() )
				{
					ME->GetCatapult()->AllowCollisionWith( ME, false );
				}
				// otherwise Allow colprim overlap between rock and other
				else if ( pobOther )
				{
					ME->GetInteractionComponent()->ExcludeCollisionWith(pobOther);
				}

				// Switch from keyframed -> breakablekeyframed motion (avoids penetration)
				if ( pobOther || ME->GetCatapult() )
				{
					if ( pSharedAttrs->m_bCollapseOnDamage )
						ME->GetPhysicsSystem()->Lua_CompoundRigid_SetBreakableKeyframedMotion(true);
					else
						ME->GetPhysicsSystem()->Lua_Rigid_SetBreakableKeyframedMotion(true);
				}

				ME->m_bMovementDone = false;
				ME->m_bThrown = false;

				Message msgOnThrow(msg_think_onthrow);
				ME->GetMessageHandler()->QueueMessageDelayed(msgOnThrow, ME->m_fLaunchTime);

				Message msgRemove(msg_removefromworld);
				ME->GetMessageHandler()->QueueMessageDelayed(msgRemove, ROCK_TIME_OUT_IN_SECS);

				// Audio control
				ME->EnableProjectileIdleAudio(false);
				ME->EnableProjectileFlightAudio(true);
			}
			END_EVENT(true)

			EVENT(msg_atrest)
			{
				ME->GetPhysicsSystem()->Lua_CollisionStrike(false);

				SET_STATE(DEFAULT);
			}
			END_EVENT(true)

			EVENT(msg_deflection)
				//CamMan::Get().GetView(0)->AfterTouchCoolCamLookAt(ME, 1);
			END_EVENT(true)

			EVENT(msg_collision)
			{
				Att_Catapult_Rock* pSharedAttrs = ME->GetSharedAttributes();

				if (msg.GetFloat("ProjVel") > -pSharedAttrs->m_fImpactThreshold && msg.GetFloat("ProjVel") < pSharedAttrs->m_fImpactThreshold )
				{
					END_EVENT(true);
				}

				if (!ntStr::IsNull(pSharedAttrs->m_obThrownAttackData))
				{
					CombatHelper::Combat_GenerateStrike(ME, ME, msg.GetEnt("Collidee"), pSharedAttrs->m_obThrownAttackData);
				}

				if ( ME->m_bThrown && pSharedAttrs->m_bRemoveOnDamage && pSharedAttrs->m_bDamageOnEnv )
				{
					SET_STATE(DESTROY);
				}
			}
			END_EVENT(true)

			/*EVENT(msg_antigrav_off)
			{
				Att_Catapult_Rock* pSharedAttrs = ME->GetSharedAttributes();
				if (pSharedAttrs->m_bRebound && !pSharedAttrs->m_bCollapseOnDamage )
				{
					ME->GetPhysicsSystem()->Lua_Rigid_EnableVelocityReflection(false);
				}
			}
			END_EVENT(true)*/

			EVENT(msg_hitsolid) // Static collision
			{
				Att_Catapult_Rock* pSharedAttrs = ME->GetSharedAttributes();

				// Sound Effect - AudioHelper:: work in progress
				//if (!ntStr::IsNull(pSharedAttrs->m_obSfxImpaleSolid)
					// AudioHelper::PlayPhysicsSound(pSharedAttrs->m_obSfxImpaleSolid, ME);
				
				//Particle Effect
				//if (!ntStr::IsNull(pSharedAttrs->m_obPfxImpaleSolid))
				//	FXHelper::Pfx_CreateStatic(pSharedAttrs->m_obPfxImpaleSolid, ME, CHashedString("ROOT"));

				if (ME->m_bThrown && pSharedAttrs->m_bCollapseOnDamage && pSharedAttrs->m_bDamageOnEnv)
				{
					SET_STATE( COLLAPSED );
				}
            }
			END_EVENT(true)

			EVENT(msg_hitragdoll)  // Percing collision
			{
				//Att_Catapult_Rock* pSharedAttrs = ME->GetSharedAttributes();
				
				// Sound Effect - AudioHelper:: work in progress
				/*
				if (!ntStr::IsNull(pSharedAttrs->m_obSfxImpaleRagdoll))
				{
					AudioHelper::PlayPhysicsSound(pSharedAttrs->m_obSfxImpaleRagdoll, ME);
					AudioHelper::PlayPannedSound("vox_sb", "shd_death", "vo_character", 8.0f, 4.0f, ME);
					}
				*/
				
				//Particle Effect
				//if (!ntStr::IsNull(pSharedAttrs->m_obPfxImpaleSolid))
				//	FXHelper::Pfx_CreateStatic(pSharedAttrs->m_obPfxImpaleSolid, ME, CHashedString("ROOT"));

				if (ME->m_bMovementDone)
				{
					SET_STATE(INACTIVE);
				}
			}
			END_EVENT(true)

			EVENT(msg_hitcharacter) // Charachter collision
			{
				Att_Catapult_Rock* pSharedAttrs = ME->GetSharedAttributes();

				// Sound Effect - AudioHelper:: work in progress
				//if (!ntStr::IsNull(pSharedAttrs->m_obSfxImpaleSolid)
					// AudioHelper::PlayPhysicsSound(pSharedAttrs->m_obSfxImpaleSolid, ME);
				
				//Particle Effect
				//if (!ntStr::IsNull(pSharedAttrs->m_obPfxImpaleSolid))
				//	FXHelper::Pfx_CreateStatic(pSharedAttrs->m_obPfxImpaleSolid, ME, CHashedString("ROOT"));

				if (ME->m_bThrown && pSharedAttrs->m_bCollapseOnDamage && pSharedAttrs->m_bDamageOnChar)
				{
					SET_STATE( COLLAPSED );
				}
            }
			END_EVENT(true)

			ON_UPDATE
			{
#ifdef _DEBUG
				ME->RenderDebugTarget(ME->m_obTarget);
#endif // _DEBUG

				// Maintain projectile flight audio
				ME->UpdateProjectileFlightAudio();
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE

	/*STATE(AFTERTOUCH)
		BEGIN_EVENTS
			ON_ENTER
			{
				CEntity* pobOther = ME->m_pOther;
				Att_Catapult_Rock* pSharedAttrs = ME->GetSharedAttributes();

				// Enter our aftertouch controller
				ME->m_nThrownCameraHandle = CamMan::Get().GetView(0)->ActivateAfterTouchCoolCamDef(ME, pSharedAttrs->m_pobAftertouchCamProperties);

				if (pobOther)
				{
					ME->GetPhysicsSystem()->Lua_SetControllerDef(pobOther, pSharedAttrs->m_pobAftertouchProperites);
				}

				// LUA GLOBAL FUNCTION STUFF
				if (!ntStr::IsNull(pSharedAttrs->m_obOnAftertouchStart))
				{
					CLuaGlobal::CallLuaFunc(pSharedAttrs->m_obOnAftertouchStart, ME);
				}

				ME->GetPhysicsSystem()->Lua_SendMessageOnCollision(true);
			}
			END_EVENT(true)

			ON_EXIT
				ME->GetPhysicsSystem()->Lua_SendMessageOnCollision(false);
			END_EVENT(true)

			EVENT(msg_interrupt)
			{
				Att_Catapult_Rock* pSharedAttrs = ME->GetSharedAttributes();

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
					CamMan::Get().GetView(0)->RemoveAllCoolCameras();
					SET_STATE(INACTIVE);
				}
			}
			END_EVENT(true)
		
			EVENT(msg_power_off)
			EVENT(msg_attack_off)
			EVENT(msg_action_off)
				ME->Aftertouch_Power_Off();
			END_EVENT(true)

			EVENT(msg_deflection)
				CamMan::Get().GetView(0)->AfterTouchCoolCamLookAt(ME, 2);
			END_EVENT(true)

			EVENT(msg_collision)
			{
				Att_Catapult_Rock* pSharedAttrs = ME->GetSharedAttributes();

				// Make sure the object was travelling above the threshold velocity
				if (msg.GetFloat("ProjVel") > -pSharedAttrs->m_fImpactThreshold && msg.GetFloat("ProjVel") < pSharedAttrs->m_fImpactThreshold )
				{
					END_EVENT(true)
				}

				
				// FIX ME Commented out as Att_Catapult_Rock doesn't appear to have PfxImpact at the moment
				//if (!ntStr::IsNull(pSharedAttrs->m_obPfxImpact))
				//{
				//	FXHelper::Pfx_CreateStatic(pSharedAttrs->m_obPfxImpact, ME, CHashedString("ROOT"));
				//}
				

				if (!ntStr::IsNull(pSharedAttrs->m_obThrownAttackData))
				{
					CombatHelper::Combat_GenerateStrike(ME, ME, msg.GetEnt("Collidee"), pSharedAttrs->m_obThrownAttackData);
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
				Att_Catapult_Rock* pSharedAttrs = ME->GetSharedAttributes();

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
				Att_Catapult_Rock* pSharedAttrs = ME->GetSharedAttributes();

				if ( pSharedAttrs->m_bRebound && !pSharedAttrs->m_bCollapseOnDamage )
				{
					ME->GetPhysicsSystem()->Lua_Rigid_DeflectionRender(false);
					ME->GetPhysicsSystem()->Lua_Rigid_EnableVelocityReflection(false);
				}
			}
			END_EVENT(true)

			EVENT(msg_obj_collision)
				CamMan::Get().GetView(0)->AfterTouchCoolCamLookAt(ME, 0);
			END_EVENT(true)

			EVENT(msg_hitsolid)
			{
				Att_Catapult_Rock* pSharedAttrs = ME->GetSharedAttributes();

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
				Att_Catapult_Rock* pSharedAttrs = ME->GetSharedAttributes();
				
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
				Att_Catapult_Rock* pSharedAttrs = ME->GetSharedAttributes();

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
		END_EVENTS
	END_STATE // AFTERTOUCH
	*/

	STATE(INACTIVE)
		BEGIN_EVENTS
			ON_ENTER
				ME->m_pOther = 0;
				//ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);
				ME->Lua_AnimStopAll();
				ME->GetPhysicsSystem()->Lua_DeactivateState("Rigid");

				// Audio control
				ME->EnableProjectileIdleAudio(false);
				ME->EnableProjectileFlightAudio(false);
			END_EVENT(true)

			EVENT(msg_activate)
			{
				SET_STATE(DEFAULT);
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE // INACTIVE

	STATE(DESTROY)
		BEGIN_EVENTS
			EVENT(msg_think_onremoveobject)
				// FIX ME RemoveSelfFromWorld leading to an assert, look at this again later
				ME->Lua_RemoveSelfFromWorld();

				//Remove from the projectile manager.
				ProjectileManager::Get().RemoveProjectileFromList(ME);
			END_EVENT(true)

			ON_ENTER
			{
				Att_Catapult_Rock* pSharedAttrs = ME->GetSharedAttributes();

				ME->OnDestroy();

				//ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);
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

				// Audio control
				ME->EnableProjectileIdleAudio(false);
				ME->EnableProjectileFlightAudio(false);
				ME->ProjectileImpactAudio(msg.GetEnt("Collidee")); // (Hopefully) temporary impact audio
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE // DESTROY

	STATE(COLLAPSED)
		BEGIN_EVENTS
			ON_ENTER
			{
				Att_Catapult_Rock* pSharedAttrs = ME->GetSharedAttributes();
				
				if (pSharedAttrs->m_bCollapseOnDamage)
				{
					ME->GetPhysicsSystem()->Lua_RemoveChildEntities();
					ME->GetPhysicsSystem()->Lua_CompoundRigid_Collapse();

					//ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);

					if (!ntStr::IsNull(pSharedAttrs->m_obPfxCollapse) )
					{
						FXHelper::Pfx_CreateStatic(pSharedAttrs->m_obPfxCollapse, ME, "ROOT");
					}

					// Sound Effect - AudioHelper:: work in progress
					/*if (!ntStr::IsNull(pSharedAttrs->m_obSfxCollapse) )
					{
						AudioHelper::PlaySound(pSharedAttrs->m_obSfxCollapse);
					}*/

					// Audio control
					ME->EnableProjectileIdleAudio(false);
					ME->EnableProjectileFlightAudio(false);
					ME->ProjectileImpactAudio(msg.GetEnt("Collidee")); // (Hopefully) temporary impact audio
				}
			}
			END_EVENT(true)

			EVENT(msg_movementdone)
			{
				CEntity* pobOther = ME->m_pOther;

				//CamMan::Get().GetView(0)->RemoveAllCoolCameras(); JML - Why is the catapult rock doing anything with cameras!!!

				if ( pobOther )
				{
					Message msgExitState(msg_exitstate);
					msgExitState.SetEnt( CHashedString(HASH_STRING_SENDER), ME);
					pobOther->GetMessageHandler()->QueueMessage(msgExitState);
				}

				ME->m_pOther = 0;
				ME->m_pobCatapult = 0;
				ME->m_bMovementDone = true;
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE

	BEGIN_EVENTS
		EVENT(msg_removefromworld)
		{
			SET_STATE(DESTROY);
		}
		END_EVENT(true)
	END_EVENTS
	

	// Helper Funcs
	//void Aftertouch_Power_Off();

END_STATEMACHINE //OBJECT_CATAPULT_ROCK_FSM


//--------------------------------------------------
//!
//!	Object_Catapult_Rock::Object_Catapult_Rock()
//!	Default constructor
//!
//--------------------------------------------------
Object_Catapult_Rock::Object_Catapult_Rock()
:	m_pOther ( 0 )
,	m_nThrownCameraHandle	( 0 )
,	m_bMovementDone			( false )
,	m_bThrown				( false )
,	m_obTarget( CONSTRUCT_CLEAR )
,	m_obLaunchTranslation( CONSTRUCT_CLEAR )
,	m_obLaunchOrientation( CONSTRUCT_CLEAR )
,	m_obAngularVelocity( CONSTRUCT_CLEAR )
,	m_obLaunchVelocity( CONSTRUCT_CLEAR )
,	m_fLaunchTime ( 0.0f )
,	m_fLaunchHeight ( 0.0f )
,	m_pSharedAttributes ( 0 )	
,	m_pAmmoAttributes ( 0 )
,	m_pobCatapult ( 0 )
,	m_uiLoopingSoundId( 0 )
,	m_uiPassbySoundId( 0 )
,	m_uiIdleSoundId( 0 )
,	m_bProjectileFlightAudio( true )
,	m_bProjectileIdleAudio( true )
,	m_bAttached	( false )
{
	m_eType = EntType_Interactable;
}

//--------------------------------------------------
//!
//!	Object_Catapult_Rock::OnPostConstruct()
//!	Post Construct
//!
//--------------------------------------------------
void Object_Catapult_Rock::OnPostConstruct()
{
	Interactable::OnPostConstruct();

	//m_pSharedAttributes = ObjectDatabase::Get().GetDataObjectFromName<Att_Catapult_Rock*>( m_pSharedAttributes ));	


	InstallMessageHandler();
	InstallAnimator("NULL");
	// Install network component here (if needed)

	//InstallDynamics();

	// Create rigid body physics
	Lua_CreatePhysicsSystem();

	

	Physics::LogicGroup* lg = 0;
	if ( GetSharedAttributes()->m_bCollapseOnDamage )
	{
		// Create a compund rigid group for collapsable
		//lg = (Physics::LogicGroup*)Physics::ClumpTools::ConstructCompoundLGFromClump( this, &obInfo );
		lg = NT_NEW Physics::CompoundLG(GetName(), this); 
		lg->Load(); 
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

	// TODO (chipb) replace
	// Register physics sound
	// if (!ntStr::IsNull(m_pSharedAttributes->m_obPhysicsSoundDef))
	// {
	//	m_pobPhysicsSystem->RegisterPhysicsSoundDef(m_pSharedAttributes->m_obPhysicsSoundDef);
	// }

	// Projectile audio defaults
	m_uiLoopingSoundId = 0;
	m_uiPassbySoundId = 0;
	m_uiIdleSoundId = 0;
	m_bProjectileFlightAudio = true;
	m_bProjectileIdleAudio = true;

	// State machine initial state
	if (m_InitialState == "Attached")
		m_bAttached = true;

	// Create and attach the statemachine
	OBJECT_CATAPULT_ROCK_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) OBJECT_CATAPULT_ROCK_FSM(m_bAttached);
	ATTACH_FSM(pFSM);
}

//--------------------------------------------------
//!
//!	Object_Catapult_Rock::~Object_Catapult_Rock()
//!	Default destructor
//!
//--------------------------------------------------
Object_Catapult_Rock::~Object_Catapult_Rock()
{
}

//--------------------------------------------------
//!
//!	Object_Catapult_Rock::Aftertouch_Power_Off()
//!	Power off function - call by multiple aftertouch events
//! so its in its own function.
//!
//--------------------------------------------------
/*void Object_Catapult_Rock::Aftertouch_Power_Off()
{
	CEntity* pobOther = m_pOther;
	Att_Catapult_Rock* pSharedAttrs = GetSharedAttributes();

	// LUA GLOBAL FUNCTION STUFF
	if (!ntStr::IsNull(pSharedAttrs->m_obOnAftertouchEnd))
	{
		CLuaGlobal::CallLuaFunc(pSharedAttrs->m_obOnAftertouchEnd, this);
	}

	CamMan::Get().GetView(0)->RemoveAllCoolCameras();

	// Player can return to their default state
	Message msgExitState(msg_exitstate);
	msgExitState.SetEnt( CHashedString(HASH_STRING_SENDER), this);
	pobOther->GetMessageHandler()->QueueMessage(msgExitState);

	m_pOther = 0;

	if (m_bImpaled)
	{
		m_pFSM->SetState(OBJECT_CATAPULT_ROCK_FSM::INACTIVE::GetInstance());
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
}*/

CEntity* Object_Catapult_Rock::ConstructCatapultRockObject(Object_Catapult* pobCatapult, Att_Ammo* pobAmmoAttrs, Att_Catapult_Rock* pobSharedAttrs)
{
	ntError(pobCatapult);
	ntError(pobAmmoAttrs);

	CLuaGlobal::Get().State().Push((CEntity*)pobCatapult);
	NinjaLua::LuaObject obLuaCatapult(-1, CLuaGlobal::Get().State(), false);

	// Create the rock
	//-----------------------------------
	// Generate a name for this projectile	
	char name[64] = {0};
	sprintf(name, "%s%s%d", pobCatapult->GetName().c_str(), pobAmmoAttrs->m_obNameAppend.c_str(), g_iAmmoID);
	g_iAmmoID++;

	DataObject* pDO = ObjectDatabase::Get().ConstructObject("Object_Catapult_Rock", name, GameGUID(), 0, true, false);
	CEntity* pobEnt = (CEntity*)pDO->GetBasePtr();

	LuaAttributeTable* pobEntAttrs = LuaAttributeTable::Create();
	pobEnt->SetAttributeTable(pobEntAttrs);
	pobEnt->GetAttributeTable()->SetDataObject(pDO);

	pobEntAttrs->SetString("Name", name);
	pobEntAttrs->SetString("Clump", ntStr::GetString( pobAmmoAttrs->m_obClump ) );
	//pobEntAttrs->SetString("SharedAttributes", pobAmmoAttrs->m_pobAtts);
	pobEntAttrs->SetAttribute("ParentEntity", obLuaCatapult);
	
	if ( !ntStr::IsNull( pobAmmoAttrs->m_obLaunchTransform ) )
	{
		
#ifndef _MASTER
		if (! pobCatapult->DoesTransformExist( pobAmmoAttrs->m_obLaunchTransform.c_str() ) )
		{	
			user_warn_p( 0, ("Catapult: Transform %s missing on entity %s\n", ntStr::GetString( pobAmmoAttrs->m_obLaunchTransform ), 
				ntStr::GetString( pobCatapult->GetName() ) ) );
		}
		else
#endif
		{
			pobEntAttrs->SetString("ParentTransform", ntStr::GetString( pobAmmoAttrs->m_obLaunchTransform ) );
			pobEntAttrs->SetBool("Attached", true);
		}
	}

	pobEntAttrs->SetInteger("SectorBits", pobCatapult->GetMappedAreaInfo() );
	//pobEntAttrs->m_pobSharedAttrs = pobAmmoAttrs;

	Object_Catapult_Rock* pobRock = reinterpret_cast<Object_Catapult_Rock*>(pobEnt);

	// Keep hold of my catapult for the correct time to calculate the launch velocity - won't be needed for barrage
	pobRock->m_pobCatapult = pobCatapult;

	pobRock->m_pSharedAttributes = pobSharedAttrs;

	// Take useful parameters from ammo attributes - set these in barrage spawn
	pobRock->m_obLaunchTranslation = pobAmmoAttrs->m_obLaunchTranslation;
	pobRock->m_obLaunchOrientation = pobAmmoAttrs->m_obLaunchOrientation;
	pobRock->m_obAngularVelocity = CDirection(pobAmmoAttrs->m_obLaunchAngularVelocity);
	//pobRock->m_obLaunchVelocity = // Can't set this now as it needs to be calculated at launch time.  This needn't be the case for barrages
	pobRock->m_fLaunchTime = pobAmmoAttrs->m_fLaunchTime;
	pobRock->m_fLaunchHeight = pobAmmoAttrs->m_fLaunchHeight;

	ObjectDatabase::Get().DoPostLoadDefaults( pDO );

	pobEnt->Lua_SetIdentity();

	return pobEnt;
}


CEntity* Object_Catapult_Rock::ConstructBarrageCatapultRockObject( uint32_t uSectorBits, const CPoint& obOrigin, const CPoint& obTarget )
{
	// Get the shared attributes for the rock
	Att_Catapult_Rock* pobSharedAttrs = (Att_Catapult_Rock*)ObjectDatabase::Get().GetPointerFromName<Att_Catapult_Rock*>( CHashedString( "Att_Simple_CatapultRock" ) );
	ntError(pobSharedAttrs);

	// Create the rock
	//-----------------------------------
	// Generate a name for this projectile	
	char name[64] = {0};
	sprintf(name, "BarrageCatapultRock%d", g_iAmmoID);
	g_iAmmoID++;

	DataObject* pDO = ObjectDatabase::Get().ConstructObject("Object_Catapult_Rock", name, GameGUID(), 0, true, false);
	CEntity* pobEnt = (CEntity*)pDO->GetBasePtr();

	LuaAttributeTable* pobEntAttrs = LuaAttributeTable::Create();
	pobEnt->SetAttributeTable(pobEntAttrs);
	pobEnt->GetAttributeTable()->SetDataObject(pDO);

	pobEntAttrs->SetString("Name", name);
	pobEntAttrs->SetString("Clump", "entities/Resources/Objects/Generic/Weaponry/catapult/catapult_rock.clump" );
	//pobEntAttrs->SetString("SharedAttributes", pobAmmoAttrs->m_pobAtts);
	pobEntAttrs->SetAttribute("ParentEntity", "");

	pobEntAttrs->SetInteger("SectorBits", uSectorBits );
	//pobEntAttrs->m_pobSharedAttrs = pobAmmoAttrs;

	Object_Catapult_Rock* pobRock = reinterpret_cast<Object_Catapult_Rock*>(pobEnt);

	pobRock->m_pSharedAttributes = pobSharedAttrs;

	// Take useful parameters from ammo attributes - set these in barrage spawn
	pobRock->m_obLaunchTranslation = CPoint(CONSTRUCT_CLEAR);
	pobRock->m_obLaunchOrientation = CQuat(CONSTRUCT_CLEAR);
	pobRock->m_obAngularVelocity = CDirection(CONSTRUCT_CLEAR);
	pobRock->m_fLaunchTime = 0.0f;
	pobRock->m_obTarget = obTarget;
	pobRock->SetPosition( obOrigin );

	// Scale height based on the distance the rock has to travel
	CDirection obOrigToTargetDir( obTarget - obOrigin );
	float fTravelDist = obOrigToTargetDir.Length();
	float fLaunchHeight = 0.25f * fTravelDist;

	// Make sure the top of the parabola is actually above both the origin and target
	float fHighestPoint = ( obOrigin.Y() < obTarget.Y() ) ? obTarget.Y() : obOrigin.Y();
	if ( fLaunchHeight < fHighestPoint )
	{
		fLaunchHeight = fHighestPoint + 0.5f;
	}
	
	// Set the launch height	
	pobRock->m_fLaunchHeight = fLaunchHeight;

	// Launch Velocity
	pobRock->m_obLaunchVelocity = pobRock->LaunchVelocity( obOrigin, obTarget, pobRock->m_fLaunchHeight );

	ObjectDatabase::Get().DoPostLoadDefaults( pDO );

	pobRock->ForceIntoThrowState();

	return pobEnt;
}


void Object_Catapult_Rock::ForceIntoThrowState( void )
{
	EXTERNALLY_SET_STATE( OBJECT_CATAPULT_ROCK_FSM, THROW );
}


//--------------------------------------------------
//!
//!	Object_Projectile::Ignition
//!
//--------------------------------------------------
void Object_Catapult_Rock::Ignition( void )
{
	Att_Catapult_Rock* pSharedAttrs = GetSharedAttributes();
	ntAssert( pSharedAttrs );

	uint iID;
	for ( PFXIter obIt = pSharedAttrs->m_aobPfxLoaded.begin(); obIt != pSharedAttrs->m_aobPfxLoaded.end(); obIt++ )
	{
		iID = FXHelper::Pfx_CreateAttached(*obIt, CHashedString(this->GetName().c_str()), "ROOT");
		m_aiPfxID.push_back(iID);
	}
}

//--------------------------------------------------
//!
//!	Object_Projectile::OnLaunch
//!
//--------------------------------------------------
void Object_Catapult_Rock::OnLaunch( void )
{
	Att_Catapult_Rock* pSharedAttrs = GetSharedAttributes();
	ntAssert( pSharedAttrs );

	// Kill particle effects
	for ( UIntIter obIt = m_aiPfxID.begin(); obIt != m_aiPfxID.end(); obIt++ )
	{
		FXHelper::Pfx_Destroy( *obIt, false );
	}
	m_aiPfxID.clear();

	uint iID;
	for ( PFXIter obIt = pSharedAttrs->m_aobPfxFlight.begin(); obIt != pSharedAttrs->m_aobPfxFlight.end(); obIt++ )
	{
		iID = FXHelper::Pfx_CreateAttached(*obIt, CHashedString(this->GetName().c_str()), "ROOT");
		m_aiPfxID.push_back(iID);
	}
}

//--------------------------------------------------
//!
//!	Object_Catapult_Rock::OnDestroy
//! Destroy function for the rock
//!
//--------------------------------------------------
void Object_Catapult_Rock::OnDestroy( void )
{
	Att_Catapult_Rock* pSharedAttrs = GetSharedAttributes();
	ntAssert( pSharedAttrs );

	// Kill particle effects
	for ( UIntIter obIt = m_aiPfxID.begin(); obIt != m_aiPfxID.end(); obIt++ )
	{
		FXHelper::Pfx_Destroy( *obIt, false );
	}
	m_aiPfxID.clear();

	//if ( m_pobPhysicsSystem->Lua_Projectile_GetStateTime() >= 0.5f)
	{
		for ( PFXIter obIt = pSharedAttrs->m_aobPfxExplosion.begin(); obIt != pSharedAttrs->m_aobPfxExplosion.end(); obIt++ )
		{
			FXHelper::Pfx_CreateStatic(*obIt, this, "ROOT");
		}

		Physics::CExplosionParams params;
		params.m_fPush = pSharedAttrs->m_fExplosionPush;
		params.m_fPushDropoff = pSharedAttrs->m_fExplosionPushDropoff;
		params.m_fRadius = pSharedAttrs->m_fExplosionRadius;
		params.m_pobOriginator = m_pOther;
		params.m_obPosition = GetPosition();
		m_pobPhysicsSystem->Lua_AltExplosion( params );

			// Notify army manager of bazooka explosion - PS3 only!
			#ifdef PLATFORM_PS3
//				ArmyManager::Get().ExplodeBazookaShot( GetPosition() );
			#endif

		//Play world sound.
		AudioHelper::PlaySound("misc_sb","rocket_explode",this);

		// Camera shake
		//CamView* pView = CamMan::Get().GetPrimaryView();
		//pView->ShakeView(0.0f, 0.0f, 0.0f);
	}
}

//--------------------------------------------------
//!
//!	Object_Catapult_Rock::LaunchVelocity
//! Helper function for parabolic linear motion gubbins
//!
//--------------------------------------------------
CDirection Object_Catapult_Rock::LaunchVelocity(const CPoint& obOrigin, const CPoint& obTarget, float fWorldHeight)
{
	CDirection obLaunchVelocity(CONSTRUCT_CLEAR);

	// world height for parabola
	float fHeight = fWorldHeight;
	
	float fFallTargetHeight = fHeight - obTarget.Y();
	float fFallOriginHeight = fHeight - obOrigin.Y();

	// Safety check.
	if ( fFallOriginHeight < 0.0f )
	{
		fFallOriginHeight = 0.0f;
	}

	static float fAccelDueToGravity = -1.0f * Physics::CPhysicsWorld::Get().GetGravity()(1);
	
	// Time to fall to Target
	float fTimeToFallTarget = sqrtf(  (2.0f * fFallTargetHeight) / fAccelDueToGravity );

	// Time to fall to Origin
	float fTimeToFallOrigin = sqrtf(  (2.0f * fFallOriginHeight) / fAccelDueToGravity );

	

	// XZ plane distance to target
	CDirection obDisplacement(obTarget - obOrigin);
	obDisplacement.Y() = 0;
	float fLinearDistance = obDisplacement.Length();

	float fLinearSpeed = fLinearDistance / (fTimeToFallTarget + fTimeToFallOrigin);

	// XZ plane velocity
	obDisplacement.Normalise();
	obLaunchVelocity = fLinearSpeed * obDisplacement;

	// Y Velocity at origin
	obLaunchVelocity.Y() = fAccelDueToGravity * fTimeToFallOrigin;

	return obLaunchVelocity;
}

//--------------------------------------------------
//	Object_Projectile::UpdateProjectileFlightAudio
//!	Starts, stops and maintains sound effects associated with projectile movement and passbys.
//!	Should be called whenever this entity is active and updating.
//--------------------------------------------------
void Object_Catapult_Rock::UpdateProjectileFlightAudio(void)
{
	// Check if audio enabled & verify
	if (!m_bProjectileFlightAudio || !m_pSharedAttributes)
		return;

	// Calculate projectile proximity to listener
	CPoint projPos = GetPosition();
	CPoint listenerPos = AudioSystem::Get().GetListenerPosition();
	CDirection projToListener = projPos^listenerPos;
	float fProximitySqr = projToListener.LengthSquared();

	// Start/stop loop as req'd
	if (!m_pSharedAttributes->m_obLoopingSoundBank.IsNull() && !m_pSharedAttributes->m_obLoopingSoundCue.IsNull())
	{
		// Test if looping sound currently playing
		if (0 == m_uiLoopingSoundId)
		{
			// Start looping sound when projectile within audible range (zero or negative range indicates ALWAYS audible)
			if (m_pSharedAttributes->m_fLoopingSoundRange <= 0.0f
				|| fProximitySqr <= m_pSharedAttributes->m_fLoopingSoundRange*m_pSharedAttributes->m_fLoopingSoundRange)
			{
				// Start sound
				if (AudioSystem::Get().Sound_Prepare(m_uiLoopingSoundId, ntStr::GetString(m_pSharedAttributes->m_obLoopingSoundBank), ntStr::GetString(m_pSharedAttributes->m_obLoopingSoundCue)))
				{
					AudioSystem::Get().Sound_Play(m_uiLoopingSoundId);
					AudioSystem::Get().Sound_SetPosition(m_uiLoopingSoundId, projPos);

					// ntPrintf("LOOPING audio STARTED\n");
				}
				else
				{
					// Ensure identifier remains invalid (possible for a valid handle to be supplied even on sound prep failure)
					m_uiLoopingSoundId = 0;
				}
			}
		}
		// Stop looping sound beyond audible range
		else if (m_pSharedAttributes->m_fLoopingSoundRange > 0.0f
			&& fProximitySqr > m_pSharedAttributes->m_fLoopingSoundRange*m_pSharedAttributes->m_fLoopingSoundRange)
		{
			// Stop sound
			AudioSystem::Get().Sound_Stop(m_uiLoopingSoundId);
			m_uiLoopingSoundId = 0;

			// ntPrintf("LOOPING audio STOPPED\n");
		}
		// Update looping sound position
		else
		{
			AudioSystem::Get().Sound_SetPosition(m_uiLoopingSoundId, projPos);
		}
	}

	// Trigger passby sound as req'd
	if (!m_pSharedAttributes->m_obPassbySoundBank.IsNull() && !m_pSharedAttributes->m_obPassbySoundCue.IsNull())
	{
		// Test if passby sound not currently playing
		if (0 == m_uiPassbySoundId)
		{
			// Trigger passby sound when projectile within audible range (zero or negative range indicates NEVER audible)
			if (m_pSharedAttributes->m_fPassbySoundRange > 0.0f
				&& fProximitySqr <= m_pSharedAttributes->m_fPassbySoundRange*m_pSharedAttributes->m_fPassbySoundRange)
			{
				// Trigger sound
				if (AudioSystem::Get().Sound_Prepare(m_uiPassbySoundId, ntStr::GetString(m_pSharedAttributes->m_obPassbySoundBank), ntStr::GetString(m_pSharedAttributes->m_obPassbySoundCue)))
				{
					AudioSystem::Get().Sound_Play(m_uiPassbySoundId);
					AudioSystem::Get().Sound_SetPosition(m_uiPassbySoundId, projPos);

					// ntPrintf("PASSBY audio STARTED\n");
				}
				else
				{
					// Ensure identifier remains invalid (possible for a valid handle to be supplied even on sound prep failure)
					m_uiPassbySoundId = 0;
				}
			}
		}
		// Update passby sound position
		else // if (AudioSystem::Get().Sound_IsPlaying(m_uiPassbySoundId))
		{
			AudioSystem::Get().Sound_SetPosition(m_uiPassbySoundId, projPos);
		}
	}
}

//--------------------------------------------------
//	Object_Projectile::UpdateProjectileIdleAudio
//!	Starts, stops and maintains sound effects associated with projectile idling (waiting for launch).
//!	Should be called continuously before entity is launched.
//--------------------------------------------------
void Object_Catapult_Rock::UpdateProjectileIdleAudio(void)
{
	// Check if audio enabled & verify
	if (!m_bProjectileIdleAudio || !m_pSharedAttributes)
		return;

	// Trigger idle sound as req'd
	if (!m_pSharedAttributes->m_obIdleSoundBank.IsNull() && !m_pSharedAttributes->m_obIdleSoundCue.IsNull())
	{
		// Test if idle sound not currently playing
		if (0 == m_uiIdleSoundId)
		{
			// Trigger sound
			if (AudioSystem::Get().Sound_Prepare(m_uiIdleSoundId, ntStr::GetString(m_pSharedAttributes->m_obIdleSoundBank), ntStr::GetString(m_pSharedAttributes->m_obIdleSoundCue)))
			{
				AudioSystem::Get().Sound_Play(m_uiIdleSoundId);
				AudioSystem::Get().Sound_SetPosition(m_uiIdleSoundId, GetPosition());

				// ntPrintf("IDLE audio STARTED\n");
			}
			else
			{
				// Ensure identifier remains invalid (possible for a valid handle to be supplied even on sound prep failure)
				m_uiIdleSoundId = 0;
			}
		}
		// Update idle sound position
		else
		{
			AudioSystem::Get().Sound_SetPosition(m_uiIdleSoundId, GetPosition());
		}
	}
}

//--------------------------------------------------
//	Object_Projectile::EnableProjectileFlightAudio
//!	Projectile flight audio is enabled by default. Disabling ensures all sounds associated
//!	with projectile movement are stopped. Projectile audio should be disabled whenever this
//!	entity deactivates, is destroyed, etc.
//!	@param bEnable	Use true to enable projectile flight audio, false to disable.
//!	@note Only audio associated with projectile flight (looping and passby sounds) is
//!	enabled/disabled.
//--------------------------------------------------
void Object_Catapult_Rock::EnableProjectileFlightAudio(bool bEnable)
{
	if (m_bProjectileFlightAudio == bEnable)
		return;

	m_bProjectileFlightAudio = bEnable;

	// If diabling ensure stopped
	if (!m_bProjectileFlightAudio)
	{
		// Stop looping sound
		if (0 != m_uiLoopingSoundId)
		{
			AudioSystem::Get().Sound_Stop(m_uiLoopingSoundId);
			m_uiLoopingSoundId = 0;
		}

		// Stop passby sound
		if (0 != m_uiPassbySoundId)
		{
			AudioSystem::Get().Sound_Stop(m_uiPassbySoundId);
			m_uiPassbySoundId = 0;
		}
	}

	// ntPrintf("FLIGHT audio %s\n", m_bProjectileIdleAudio ? "ENABLED":"DISABLED");
}

//--------------------------------------------------
//	Object_Projectile::EnableProjectileIdleAudio
//!	Projectile idle audio is enabled by default. Disabling ensures all sounds associated
//!	with projectile idling (i.e. waiting before launch) are stopped. Projectile audio 
//!	should be disabled whenever this entity deactivates, is destroyed, etc.
//!	@param bEnable	Use true to enable projectile idle audio, false to disable.
//!	@note Only audio associated with projectile idle is enabled/disabled.
//!	@note The idle sound is a bad idea really. It has been added to handle things like the
//!	flames audio because catapult ammunition is burning. Really though this should be
//!	triggered an controlled from any associated particle effect, so it is logically wedded
//!	to the visual.
//--------------------------------------------------
void Object_Catapult_Rock::EnableProjectileIdleAudio(bool bEnable)
{
	if (m_bProjectileIdleAudio == bEnable)
		return;

	m_bProjectileIdleAudio = bEnable;

	// If diabling ensure stopped
	if (!m_bProjectileIdleAudio)
	{
		// Stop idling sound
		if (0 != m_uiIdleSoundId)
		{
			AudioSystem::Get().Sound_Stop(m_uiIdleSoundId);
			m_uiIdleSoundId = 0;
		}
	}

	// ntPrintf("IDLE audio %s\n", m_bProjectileIdleAudio ? "ENABLED":"DISABLED");
}

//------------------------------------------------------------------------------------------
//	Object_Catapult_Rock::ProjectileImpactAudio
//!	Triggers any audio associated with projectile impact.
//!	@param pobCollidee	Entity projectile has impacted.
//!	@note Disables projectile flight and idle audio.
//------------------------------------------------------------------------------------------
void Object_Catapult_Rock::ProjectileImpactAudio(CEntity*)
{
	// Projectile flight audio control
	EnableProjectileFlightAudio(false);
	EnableProjectileIdleAudio(false);

	if (!m_pSharedAttributes || m_pSharedAttributes->m_obImpactSoundBank.IsNull() || m_pSharedAttributes->m_obImpactSoundCue.IsNull())
		return;

	AudioHelper::PlaySound(ntStr::GetString(m_pSharedAttributes->m_obImpactSoundBank), ntStr::GetString(m_pSharedAttributes->m_obImpactSoundCue), this);

	// ntPrintf("IMPACT audio PLAYED\n");
}

#ifdef _DEBUG
void Object_Catapult_Rock::RenderDebugTarget (CPoint& obTarget)
{
	static float fSize = 2.0f;
	g_VisualDebug->RenderPoint(obTarget,10.0f,DC_RED);
	g_VisualDebug->RenderArc(CMatrix (CQuat(CONSTRUCT_CLEAR), obTarget), fSize, TWO_PI, DC_RED);
	
	g_VisualDebug->RenderLine(CPoint(obTarget.X()+fSize,obTarget.Y(),obTarget.Z()), CPoint(obTarget.X()-fSize,obTarget.Y(),obTarget.Z()), DC_RED);
	g_VisualDebug->RenderLine(CPoint(obTarget.X(),obTarget.Y(),obTarget.Z()+fSize), CPoint(obTarget.X(),obTarget.Y(),obTarget.Z()-fSize), DC_RED);
}
#endif // _DEBUG
