//--------------------------------------------------
//!
//!	\file game/entityprojectile.cpp
//!	Definition of the projectile entity object
//!
//--------------------------------------------------

#include "objectdatabase/dataobject.h"
#include "game/luaattrtable.h"
#include "Physics/system.h"
#include "physics/collisionbitfield.h"
#include "physics/physicsmaterial.h"
#include "physics/singlerigidlg.h"
#include "game/movement.h"
#include "anim/animator.h"
#include "core/exportstruct_anim.h"
#include "messagehandler.h"
#include "effect/fxhelper.h"
#include "effect/psystem_utils.h"
#include "game/combathelper.h"
#include "game/renderablecomponent.h"
#include "game/projectilemanager.h"
#include "game/entityarcher.h"
#include "game/entitymanager.h"
#include "game/attacks.h"
#include "game/entityinteractableturretweapon.h"
#include "camera/camman.h"
#include "camera/camview.h"
#include "camera/coolcam_aftertouch.h"
#include "camera/camtrans_lerp.h"
#include "physics/projectilelg.h"
#include "audio/gameaudiocomponents.h"
#include "audio/audiohelper.h"
#include "audio/collisioneffecthandler.h"

#include "game/entityprojectile.h"
#include "game/entityrangedweapon.h"
#include "game/interactioncomponent.h"
#include "game/entityaerialgeneral.h"
#include "game/entitykingbohan.h"
#include "game/randmanager.h"
#include "game/shellconfig.h"

#include "ai/aihearingman.h"	// For Investigate Behaviour (Dario)
#include "ai/ainavigationsystem/ainavigsystemman.h"
#include "ai/airangedtargeting.h"

#ifdef PLATFORM_PS3
#include "army/armymanager.h"	// For bazooka position and explosion notification
#endif // PLATFORM_PS3

// DO NOT DELETE - MB
// Fall Times and Accelerations for projectiles
// Ballista Bolt - Time: 5.0, Accel: -0.1
// Crossbow Bolt - Time: 2.5, Accel: -4.0

//Global projectile ID value (this is how it was done in LUA so don't shoot me! Replace later).
int g_iProjectileID = 0;
const int iNumBazookaRockets = 1;

void ForceLinkFunctionProjectile()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionProjectile() !ATTN!\n");
}

START_CHUNKED_INTERFACE(HitAreas_Attributes, Mem::MC_ENTITY)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_Head_AttackData, "NULL", Head_AttackData)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_Spine_AttackData, "NULL", Spine_AttackData)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_Pelvis_AttackData, "NULL", Pelvis_AttackData)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_L_UpperArm_AttackData, "NULL", L_UpperArm_AttackData)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_L_ForeArm_AttackData, "NULL", L_ForeArm_AttackData)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_R_UpperArm_AttackData, "NULL", R_UpperArm_AttackData)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_R_ForeArm_AttackData, "NULL", R_ForeArm_AttackData)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_L_Thigh_AttackData, "NULL", L_Thigh_AttackData)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_L_Calf_AttackData, "NULL", L_Calf_AttackData)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_R_Thigh_AttackData, "NULL", R_Thigh_AttackData)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_R_Calf_AttackData, "NULL", R_Calf_AttackData)	
END_STD_INTERFACE

// Projectile Attributes Interface
START_CHUNKED_INTERFACE(Projectile_Attributes, Mem::MC_ENTITY)
	IENUM(Projectile_Attributes, Type, PROJECTILE_TYPE )
	PUBLISH_VAR_AS(m_Clump, Clump)
	PUBLISH_VAR_AS(m_Properties, Properties)
	PUBLISH_VAR_AS(m_PropertiesFullyCharged, PropertiesFullyCharged)
	PUBLISH_VAR_AS(m_AttackData, AttackData)
	PUBLISH_VAR_AS(m_AttackDataFullyCharged, AttackDataFullyCharged)
	PUBLISH_VAR_AS(m_AOEAttackData, AreaOfEffectAttackData)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_NonPowered_Front_HitAreas, "NULL", NonPowered_Front_HitAreas)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_NonPowered_Back_HitAreas, "NULL", NonPowered_Back_HitAreas)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_Powered_Front_HitAreas, "NULL",Powered_Front_HitAreas)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_Powered_Back_HitAreas, "NULL", Powered_Back_HitAreas)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_AreaOfEffectRadius, 0.0f, AreaOfEffectRadius);
	PUBLISH_VAR_AS(m_CameraDef, CameraDef)
	PUBLISH_VAR_AS(m_CameraDefFullyCharged, CameraDefFullyCharged)
	PUBLISH_VAR_AS(m_TrackedEntityOffset, TrackedEntityOffset)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_TrackingSpeed, 0.0f, TrackingSpeed)
	PUBLISH_VAR_AS(m_SoundVolume, SoundVolume)
	PUBLISH_VAR_AS(m_SoundRadius, SoundRadius)
	PUBLISH_VAR_AS(m_IsVolumeConstant, IsVolumeConstant)
	PUBLISH_VAR_AS(m_LoopingSoundCue, LoopingSoundCue);
	PUBLISH_VAR_AS(m_LoopingSoundBank, LoopingSoundBank);
	PUBLISH_VAR_AS(m_LoopingSoundRange, LoopingSoundRange);
	PUBLISH_VAR_AS(m_PassbySoundCue, PassbySoundCue);
	PUBLISH_VAR_AS(m_PassbySoundBank, PassbySoundBank);
	PUBLISH_VAR_AS(m_PassbySoundRange, PassbySoundRange);
	PUBLISH_VAR_AS(m_ImpactSoundCue, ImpactSoundCue);
	PUBLISH_VAR_AS(m_ImpactSoundBank, ImpactSoundBank);
	PUBLISH_VAR_AS(m_FireSoundCue, FireSoundCue);
	PUBLISH_VAR_AS(m_FireSoundBank, FireSoundBank);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obPhysicsSoundDef, "NULL", PhysicsSoundDef);

	PUBLISH_CONTAINER_AS (m_aobPfxOnSpawnAttached,			PfxOnSpawnAttached)
	PUBLISH_CONTAINER_AS (m_aobPfxOnIgniteAttached,			PfxOnIgniteAttached)
	PUBLISH_CONTAINER_AS (m_aobPfxOnImpactAttached,			PfxOnImpactAttached)
	
	PUBLISH_CONTAINER_AS (m_aobPfxOnSpawnStatic,			PfxOnSpawnStatic)
	PUBLISH_CONTAINER_AS (m_aobPfxOnIgniteStatic,			PfxOnIgniteStatic)
	PUBLISH_CONTAINER_AS (m_aobPfxOnImpactStatic,			PfxOnImpactStatic)
	PUBLISH_CONTAINER_AS (m_aobPfxOnDestroyStatic,			PfxOnDestroyStatic)
END_STD_INTERFACE


// Object Projectile Interface
START_CHUNKED_INTERFACE(Object_Projectile, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(CEntity)
	COPY_INTERFACE_FROM(CEntity)

	PUBLISH_VAR_AS(m_Description, Description)
	PUBLISH_VAR_AS(m_InitialState, InitialState)

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )	
END_STD_INTERFACE


//--------------------------------------------------
//!
//! Projectile State Machine
//!
//--------------------------------------------------
STATEMACHINE(PROJECTILE_FSM, Object_Projectile)
	PROJECTILE_FSM()
	{
		SET_INITIAL_STATE(DEFAULT);
	}

	STATE(DEFAULT)
		BEGIN_EVENTS
			ON_ENTER
			{
				//The OnSpawn call was moved to ExtraSetup____* because it is not available at this point.

				//Add to the projectile manager.
				ProjectileManager::Get().AddProjectileToList(ME);

				ME->m_iCameraHandle = 0;
			}
			END_EVENT(true)

			ON_UPDATE
			{
				// Notify army manager of bazooka and cannonball positions - PS3 only!
			#ifdef PLATFORM_PS3
				if ( ME->m_eProjectileType == PROJECTILE_TYPE_BAZOOKA_ROCKET )
				{
					// Use only the dummy rocket for the army notification
					if ( ME->m_iProjectileIndex == iNumBazookaRockets )
					{
						ArmyManager::Get().SetBazookaShotPosition( ME->GetPosition(), CHashedString(ME->GetName().c_str()) );
					}
				}

				if ( ME->m_eProjectileType == PROJECTILE_TYPE_CANNON_BALL )
				{
					ArmyManager::Get().SetBazookaShotPosition( ME->GetPosition(), CHashedString(ME->GetName().c_str()) );
				}
			#endif

				if ( ME->m_eProjectileType == PROJECTILE_TYPE_KING_WINGATTACKCROW )
				{
					//Die out after a certain amount of time.
					//Note: Generally speaking the crows should disappear on-impact... but in-case any don't for any reason, we can kill them
					//off after a certain amount of time (if for any reason they don't hit the geometry etc).
					float fFrameUpdateTime = CTimer::Get().GetGameTimeChange();
					ME->m_fTimeSinceSpawn += fFrameUpdateTime;
					//For now just destroy after 30 seconds regardless of where it is.
					if(ME->m_fTimeSinceSpawn > 30.0f)
					{
						SET_STATE(DESTROY);
					}
				}

				//Handle updating orbiting lightningballs.
				if ( ME->m_eProjectileType == PROJECTILE_TYPE_KING_LIGHTNINGBALL )
				{
					//If we're currently orbiting the king (pre-fire), then update our location around the king.
					if(ME->m_bIsOrbitingOriginator)
					{
						float fFrameUpdateTime = CTimer::Get().GetGameTimeChange();

						Physics::ProjectileLG* pLG = (Physics::ProjectileLG*)ME->GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::PROJECTILE_LG);
						if(pLG)
						{
							pLG->SetLinearVelocity(CDirection(0.0f, 0.0f, 0.0f));
						}

						float fAnglesPerSecondRotation = 0.0f;
						if(ME->m_bLarge)
						{
							fAnglesPerSecondRotation = 110.0f * (3.14159265358979323846f / 180.0f);	//110 degrees per second to radians.
						}
						else
						{
							fAnglesPerSecondRotation = 180.0f * (3.14159265358979323846f / 180.0f);	//180 degrees per second to radians.
						}
						ME->m_fOrbitAngle += fAnglesPerSecondRotation * fFrameUpdateTime;

						//Set our new position based on the angle around the player.
						float fSin, fCos;
						CMaths::SinCos(ME->m_fOrbitAngle, fSin, fCos);
						CDirection obOffset(CONSTRUCT_CLEAR);
						if(ME->m_bLarge)
						{
							obOffset = CDirection(fSin, 0.0f, fCos);
						}
						else
						{
							obOffset = CDirection(0.0f, fSin, fCos);
						}
						obOffset.Normalise();
						if(ME->m_bLarge)
						{
							obOffset *= 2.0f;	//2 metre radius.
						}
						else
						{
							obOffset *= 3.0f;	//3 metre radius for the smaller ones.
						}

						CPoint obOriginatorPos(CONSTRUCT_CLEAR);
						ntError_p(ME->m_ProjectileData.pAttacker, ("King lightning ball should not exist without an attacker/originator"));
						if(ME->m_ProjectileData.pAttacker)
						{
							obOriginatorPos = ME->m_ProjectileData.pAttacker->GetPosition() + CPoint(0.0f, 1.5f, 0.0f);
						}

						CPoint obNewPos = obOriginatorPos + obOffset;
						ME->SetPosition(obNewPos);
					}
					else
					{
						//Die out after a certain amount of time.
						//Note: Generally speaking the lightningbolts should disappear on-impact...
						//...but in-case any don't for any reason, we can kill them off after a certain amount of time.
						float fFrameUpdateTime = CTimer::Get().GetGameTimeChange();
						ME->m_fTimeSinceSpawn += fFrameUpdateTime;
						//For now just destroy after 30 seconds regardless of where it is.
						if(ME->m_fTimeSinceSpawn > 30.0f)
						{
							SET_STATE(DESTROY);
						}
					}
				}

				/*
				if ( ME->m_eProjectileType == PROJECTILE_TYPE_CROSSBOW_BOLT )
				{
					static float fTest = 0.0f;
					fTest += 0.1f;
					ME->SetRotation( CQuat( CDirection( 1.0f, 1.0f, 1.0f ), fTest ) );
				}
				*/

				ME->UpdateProjectileAudio();
			}
			END_EVENT(true)

			EVENT(msg_think_ondestroyprojectile)
			{
				SET_STATE(DESTROY);
			}
			END_EVENT(true)

			EVENT(msg_think_bazookaignition)
			{
				ME->BazookaIgnition();
			}
			END_EVENT(true)

			EVENT(msg_combat_countered)
			{
				// Get the collidee
				CEntity* pCollidee = msg.GetEnt("Counterer");

				// Call OnImpact functions depending on projectile type
				switch( ME->m_eProjectileType )
				{
					case PROJECTILE_TYPE_CROSSBOW_BOLT:
					case PROJECTILE_TYPE_BALLISTA_BOLT:
						ME->CrossbowOnCountered( pCollidee );
						break;

					case PROJECTILE_TYPE_BAZOOKA_ROCKET:
						ME->BazookaOnCountered( pCollidee );
						break;
					
					case PROJECTILE_TYPE_AGEN_SWORD:
						ME->AGenSwordOnCountered( pCollidee );
						break;

					case PROJECTILE_TYPE_KING_LIGHTNINGBALL:
//						ntPrintf("msg_combat_countered recieved on Lightning Ball\n");
						ME->KingLightningBallOnCountered( pCollidee );
						break;

					case PROJECTILE_TYPE_KING_WINGATTACKCROW:
						break;

					case PROJECTILE_TYPE_CANNON_BALL:
						//ME->CannonBallOnCountered( pCollidee );
						break;

					default:
						ntAssert_p( 0, ("Unknown projectile type!") );
						break;
				}

				// Play impact sound as req'd (this will stop any projectile flight sound)
				//ME->ProjectileImpactAudio(pCollidee);
			}
			END_EVENT(true)

			EVENT(msg_combat_missedprojectilecounter)
			{
				// Get the collidee
				CEntity* pCollidee = msg.GetEnt("Counterer");

				// Call OnImpact functions depending on projectile type
				switch( ME->m_eProjectileType )
				{
					case PROJECTILE_TYPE_CROSSBOW_BOLT:
					case PROJECTILE_TYPE_BALLISTA_BOLT:
					case PROJECTILE_TYPE_BAZOOKA_ROCKET:
					case PROJECTILE_TYPE_AGEN_SWORD:
						ME->AGenSwordOnMissedCounter( pCollidee );
						break;
					case PROJECTILE_TYPE_KING_WINGATTACKCROW:
					case PROJECTILE_TYPE_CANNON_BALL:
						break;

					case PROJECTILE_TYPE_KING_LIGHTNINGBALL:
						ME->KingLightningBallOnMissedCounter( pCollidee );
						break;

					default:
						ntAssert_p( 0, ("Unknown projectile type!") );
						break;
				}
			}
			END_EVENT(true)

			EVENT(msg_collision)
			{
				// Get the collidee
				CEntity* pCollidee = msg.GetEnt("Collidee");

				// Call OnImpact functions depending on projectile type
				switch( ME->m_eProjectileType )
				{
					case PROJECTILE_TYPE_CROSSBOW_BOLT:
					case PROJECTILE_TYPE_BALLISTA_BOLT:
						ME->CrossbowOnImpact( msg); 
						break;

					case PROJECTILE_TYPE_BAZOOKA_ROCKET:
						ME->BazookaOnImpact( pCollidee );
						break;
					
					case PROJECTILE_TYPE_AGEN_SWORD:
						ME->AGenSwordOnImpact( pCollidee, CPoint( msg.GetFloat("pX"), msg.GetFloat("pY"), msg.GetFloat("pZ") ) );
						break;

					case PROJECTILE_TYPE_KING_LIGHTNINGBALL:
						ME->KingLightningBallOnImpact( pCollidee, CPoint( msg.GetFloat("pX"), msg.GetFloat("pY"), msg.GetFloat("pZ") ) );
						break;

					case PROJECTILE_TYPE_KING_WINGATTACKCROW:
						ME->KingWingAttackCrowOnImpact( pCollidee, CPoint( msg.GetFloat("pX"), msg.GetFloat("pY"), msg.GetFloat("pZ") ) );
						break;

					case PROJECTILE_TYPE_CANNON_BALL:
						ME->CannonBallOnImpact( pCollidee );

						break;

					default:
						ntAssert_p( 0, ("Unknown projectile type!") );
						break;
				}

				// Play impact sound as req'd (this will stop any projectile flight sound)
				ME->ProjectileImpactAudio(pCollidee);
			}
			END_EVENT(true)

			// looks like the projectile has been struck
			EVENT(msg_projectile_hitranged)
			EVENT(msg_combat_struck)
			{
				// Get the entity that struck this one
				CEntity* pStriker = msg.GetEnt("Sender");

				// Play impact sound as req'd (this will stop any projectile flight sound)
				ME->ProjectileImpactAudio(pStriker);

				// Call OnImpact functions depending on projectile type
				switch( ME->m_eProjectileType )
				{
					case PROJECTILE_TYPE_CROSSBOW_BOLT:
						ME->CrossbowOnStrike( pStriker );
						break;

					case PROJECTILE_TYPE_BALLISTA_BOLT:
						ME->BazookaOnStrike( pStriker );
						break;

					case PROJECTILE_TYPE_BAZOOKA_ROCKET:
						break;
	                case PROJECTILE_TYPE_AGEN_SWORD:
						break;
					case PROJECTILE_TYPE_KING_WINGATTACKCROW:
						ME->KingWingAttackCrowHitRanged();
						break;
					case PROJECTILE_TYPE_KING_LIGHTNINGBALL:
						break;
					case PROJECTILE_TYPE_CANNON_BALL:
						break;

					default:
						ntAssert_p( 0, ("Unknown projectile type!") );
						break;
				}

			}
			END_EVENT(true)

			// looks like the projectile has ricocheted
			EVENT(msg_ricochet)
			{
				// Get the new speed for the projectile
				float fProjectileSpeed = msg.GetFloat( 0 );

				// Call OnImpact functions depending on projectile type
				switch( ME->m_eProjectileType )
				{
					case PROJECTILE_TYPE_CROSSBOW_BOLT:
						break;

					case PROJECTILE_TYPE_BALLISTA_BOLT:
						break;

					case PROJECTILE_TYPE_BAZOOKA_ROCKET:
						break;

					case PROJECTILE_TYPE_KING_LIGHTNINGBALL:
						break;

					case PROJECTILE_TYPE_KING_WINGATTACKCROW:
						break;

					case PROJECTILE_TYPE_CANNON_BALL:

						if( fProjectileSpeed < 1.0f )
						{
							SET_STATE(DESTROY);
						}

						break;

					default:
						ntAssert_p( 0, ("Unknown projectile type!") );
						break;
				}
			}
			END_EVENT(true)


			EVENT(msg_ignite)
			{
				//ntPrintf("%s has been told to ignite\n", ME->GetName().c_str());
				ME->m_bOnFire = true;

				// Set the property on the attributes too
				LuaAttributeTable* pAttributes = ME->GetAttributeTable();
				ntAssert(pAttributes);

				pAttributes->SetBool("OnFire", true);

				if ( ME->m_eProjectileType == PROJECTILE_TYPE_CROSSBOW_BOLT )
				{
					ME->CrossbowIgniteEffects();
				}
			}
			END_EVENT(true)

			EVENT(msg_aftertouch)
			{
				ntPrintf("PROJECTILE AFTERTOUCH\n");
				ME->m_bInAftertouch = true;
				ME->m_ProjectileData.pOriginator = msg.GetEnt("Sender");
				ntAssert(ME->m_ProjectileData.pOriginator);
				//ntPrintf("Projectile:DefaultState:msg_aftertouch. 'Originator' entity is %s", ME->m_ProjectileData.pOriginator->GetName().c_str());

				ME->m_iCameraHandle = CamMan::Get().GetView(0)->ActivateAfterTouchCoolCam(ME, ME->m_ProjectileData.ProjectileCameraDef);
			}
			END_EVENT(true)

			EVENT(msg_aftertouch_done)
			{
				ntPrintf("Projectile:DefaultState:msg_aftertouch_done\n");

				if(ME->m_bInAftertouch == true)
				{
					CamMan::Get().GetPrimaryView()->RemoveCoolCamera(ME->m_iCameraHandle);
					ME->m_iCameraHandle = 0;
					ME->m_bInAftertouch = false;

					Message msgProjAftertouchComplete(msg_projectile_aftertouch_complete);
					msgProjAftertouchComplete.SetEnt("Sender", ME);
					ME->m_ProjectileData.pOriginator->GetMessageHandler()->QueueMessage(msgProjAftertouchComplete);
				}
			}
			END_EVENT(true)

			EVENT(msg_control_end)
			{
				ntPrintf("Projectile:DefaultState:msg_control_end. In aftertouch = %d\n", ME->m_bInAftertouch);

				if(ME->m_bInAftertouch == true)
				{
					//Let the weapon know the projectile has been destroyed.
					Message msgProjDestroyed(msg_projectile_destroyed);
					msgProjDestroyed.SetEnt("Sender", ME);
					ME->m_ProjectileData.pOriginator->GetMessageHandler()->QueueMessage(msgProjDestroyed);

					//Equivalent to msg_aftertouch_done. In lua it jumped to that event.
					ntPrintf("Projectile:DefaultState:msg_control_end (aftertouch_done bit)\n");

					if(ME->m_bInAftertouch == true)
					{
						CamMan::Get().GetPrimaryView()->RemoveCoolCamera(ME->m_iCameraHandle);
						ME->m_iCameraHandle = 0;
						ME->m_bInAftertouch = false;

						Message msgProjAftertouchComplete(msg_projectile_aftertouch_complete);
						msgProjAftertouchComplete.SetEnt("Sender", ME);
						ME->m_ProjectileData.pOriginator->GetMessageHandler()->QueueMessage(msgProjAftertouchComplete);
					}
				}
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	STATE(INACTIVE)
		BEGIN_EVENTS
			EVENT(msg_think_ondestroyprojectile)
			{
				SET_STATE(DESTROY);
			}
			END_EVENT(true)

			ON_ENTER
			{
				//DeactivateAllStates
				ME->GetPhysicsSystem()->Deactivate();	

				//Remove from the projectile manager.
				ProjectileManager::Get().RemoveProjectileFromList(ME);

				// Projectile audio control
				ME->EnableProjectileAudio(false);
			}
			END_EVENT(true)

			ON_EXIT
			{
				if( ME->m_bInAftertouch )
				{
					CamMan::Get().GetPrimaryView()->RemoveCoolCamera(ME->m_iCameraHandle);
					ME->m_iCameraHandle = 0;
					ME->m_bInAftertouch = false;

					ntPrintf("Projectile:InactiveState:msg_aftertouch_done\n");

					//Let the weapon know we're done with aftertouch.
					Message msgProjAftertouchComplete(msg_projectile_aftertouch_complete);
					msgProjAftertouchComplete.SetEnt("Sender", ME);
					ME->m_ProjectileData.pOriginator->GetMessageHandler()->QueueMessage(msgProjAftertouchComplete);
				}
			}
			END_EVENT(true)

			EVENT(msg_removefromworld)
			{
				SET_STATE(DESTROY);
			}
			END_EVENT(true)


			EVENT(msg_aftertouch_done)
			{
				if( ME->m_bInAftertouch )
				{
					CamMan::Get().GetPrimaryView()->RemoveCoolCamera(ME->m_iCameraHandle);
					ME->m_iCameraHandle = 0;
					ME->m_bInAftertouch = false;

					ntPrintf("Projectile:InactiveState:msg_aftertouch_done\n");

					//Let the weapon know we're done with aftertouch.
					Message msgProjAftertouchComplete(msg_projectile_aftertouch_complete);
					msgProjAftertouchComplete.SetEnt("Sender", ME);
					ME->m_ProjectileData.pOriginator->GetMessageHandler()->QueueMessage(msgProjAftertouchComplete);
				}
			}
			END_EVENT(true)

			EVENT(msg_control_end)
			{
				ntPrintf("Projectile:InactiveState:msg_control_end. Aftertouch status = %d\n", ME->m_bInAftertouch);

				if(ME->m_bInAftertouch == true)
				{
					//Equivalent of msg_aftertouch_done (in lua it jumped to that event).
					CamMan::Get().GetPrimaryView()->RemoveCoolCamera(ME->m_iCameraHandle);
					ME->m_iCameraHandle = 0;
					ME->m_bInAftertouch = false;

					ntPrintf("Projectile:InactiveState:msg_control_end (aftertouch_done bit)\n");

					//Let the weapon know we're done with aftertouch.
					Message msgProjAftertouchComplete(msg_projectile_aftertouch_complete);
					msgProjAftertouchComplete.SetEnt("Sender", ME);
					ME->m_ProjectileData.pOriginator->GetMessageHandler()->QueueMessage(msgProjAftertouchComplete);
				}
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	STATE(DESTROY)
		BEGIN_EVENTS
			EVENT(msg_think_onremoveobject)
			{
				ME->Lua_ReparentToWorld();	//Reparent the object back to the world.
				ME->GetPhysicsSystem()->Deactivate();	//DeactivateAllStates, allow havok to take control of the object again.

				//Remove from the world.
				ME->Lua_RemoveSelfFromWorld();

				//Remove from the projectile manager.
				ProjectileManager::Get().RemoveProjectileFromList(ME);
			}
			END_EVENT(true)

			ON_ENTER
			{
				if(ME->m_bInAftertouch == true)
				{
					//Let the weapon know this projectile has been destroyed.
					Message msgProjDestroyed(msg_projectile_destroyed);
					ME->m_ProjectileData.pOriginator->GetMessageHandler()->QueueMessage(msgProjDestroyed);

					//Run equivalent to msg_aftertouch_done. (in lua it jumped to that state here).
					if(ME->m_bInAftertouch == true)
					{
						//CamMan::Get().GetPrimaryView()->RemoveCoolCamera(ME->m_iCameraHandle);
						CoolCamera* pCam = CamMan::GetPrimaryView()->GetCoolCam(ME->m_iCameraHandle);
						if(pCam && pCam->GetType() == CT_AFTERTOUCH)
						{
							CoolCam_AfterTouch* pAftertouch = (CoolCam_AfterTouch*)pCam;
							CPoint pt = ME->GetPosition();
							pAftertouch->ActivateEndingCloseUp(pt);
							static CamTrans_LerpDef lerpTrans(1.6f, 0.5f);
							pAftertouch->SetEndingTransition(&lerpTrans);
						}

						ME->m_iCameraHandle = 0;
						ME->m_bInAftertouch = false;
					}
				}
				ME->Hide(); //Hide this object.

				if(ME->m_pNextProjectile)
				{
					ME->m_pNextProjectile->m_pPrevProjectile = ME->m_pPrevProjectile;
				}

				if(ME->m_pPrevProjectile)
				{
					ME->m_pPrevProjectile->m_pNextProjectile = ME->m_pNextProjectile;
				}


				Message msgOnRemoveFromWorld(msg_think_onremoveobject);
				ME->GetMessageHandler()->QueueMessageDelayed(msgOnRemoveFromWorld, 2.5f);

				switch( ME->m_eProjectileType )
				{
					case PROJECTILE_TYPE_CROSSBOW_BOLT:
					case PROJECTILE_TYPE_BALLISTA_BOLT:
						ME->CrossbowOnDestroy();
						break;

					case PROJECTILE_TYPE_BAZOOKA_ROCKET:
						ME->BazookaOnDestroy();
						break;
					case PROJECTILE_TYPE_AGEN_SWORD:
						ME->AGenSwordOnDestroy();
						break;
					case PROJECTILE_TYPE_KING_LIGHTNINGBALL:
						ME->KingLightningBallOnDestroy();
						break;
					case PROJECTILE_TYPE_KING_WINGATTACKCROW:
						ME->KingWingAttackCrowOnDestroy();
						break;
					case PROJECTILE_TYPE_CANNON_BALL:
						ME->CannonBallOnDestroy();
						break;

					default:
						ntAssert_p( 0, ("Unknown projectile type!") );
						break;
				}

				// Projectile audio control
				ME->EnableProjectileAudio(false);
			}
			END_EVENT(true)

			EVENT(msg_aftertouch_done)
			{
				if(ME->m_bInAftertouch == true)
				{
					CamMan::Get().GetPrimaryView()->RemoveCoolCamera(ME->m_iCameraHandle);

					ME->m_iCameraHandle = 0;
					ME->m_bInAftertouch = false;
				}
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

END_STATEMACHINE //PROJECTILE_FSM


//--------------------------------------------------
//!
//!	Object_Projectile::Object_Projectile()
//!	Default constructor
//!
//--------------------------------------------------
Object_Projectile::Object_Projectile()
{
	m_eType = EntType_Projectile;

	m_iCameraHandle = 0;
	m_bOnFire = false;
	m_bInAftertouch = false;
	m_iProjectileIndex = -1;
	m_bAlreadyExploded = false;

	// Projectile audio control
	m_uiLoopingSoundId = 0;
	m_iLoopingSoundArg = -1;
	m_uiPassbySoundId = 0;
	m_bProjectileAudio = true;

	m_bStickIntoStaticCollision = false;
	m_bAGenSwordBringBackOnImpact = true;
	m_bHasBeenCountered = false;
	m_bFailedToCounterFlagged = false;
	m_bIsCounterable = false;
	m_bIsOrbitingOriginator = false;
	m_fOrbitAngle = 0.0f;
	m_bLarge = false;
	m_bEffectOnDestroy = true;
	m_fTimeSinceSpawn = 0.0f;

	m_pobSharedAttributes = 0;

	m_bGenerateNoStrikes = false;
}

//--------------------------------------------------
//!
//!	Object_Projectile::OnPostConstruct()
//!	Post Construct
//!
//--------------------------------------------------
void Object_Projectile::OnPostConstruct()
{
	CEntity::OnPostConstruct();

	InstallMessageHandler();
	InstallDynamics();
	
	// Default data
	m_ProjectileData.pAttacker					= 0;
	m_ProjectileData.pOriginator				= 0;
	m_ProjectileData.pTarget					= 0;
	m_ProjectileData.vPosition					= CVecMath::GetZeroPoint();
	m_ProjectileData.vDirection					= CVecMath::GetZeroDirection();
	m_ProjectileData.ProjectileProperties		= "";
	m_ProjectileData.ProjectileAttackData		= "";
	m_ProjectileData.ProjectileAttackDataFullyCharged	= "";
	m_ProjectileData.AreaOfEffectRadius			= 0.0f;
	m_ProjectileData.ProjectileAttackDataAOE	= "";
	m_ProjectileData.ProjectileCameraDef		= "";
	//m_ProjectileData.bFirstPersonAim			= false;
	m_ProjectileData.bDestroyOnImpact			= false;
	m_ProjectileData.vTrackedEntityOffset		= CVecMath::GetZeroPoint();

	// Default misc data
	m_bOnFire = false;
	m_bInAftertouch		= false;
	m_iCameraHandle		= 0;
	m_iProjectileIndex	= -1;
	m_bAlreadyExploded	= false;

	// Default projectile audio data
	m_uiLoopingSoundId = 0;
	m_iLoopingSoundArg = -1;
	m_uiPassbySoundId = 0;
	m_bProjectileAudio = true;

	// Particle ID's
	m_BazookaPfxIDs.m_PfxIgnitionID = 0;
	m_BazookaPfxIDs.m_PfxSeederID = 0;
	m_BazookaPfxIDs.m_PfxFlameID = 0;
	m_BazookaPfxIDs.m_PfxSmokeID = 0;

	// No Linked Projectiles
	m_pNextProjectile = 0;
	m_pPrevProjectile = 0;

	//--------------------------------------------------
	// TGS MEGA SUPER DUPER, MUST BE REMOVED, HAXXXOR.
	//--------------------------------------------------
	BazookaChildren[0] = "";
	BazookaChildren[1] = "";
	BazookaChildren[2] = "";
	//--------------------------------------------------
	// TGS MEGA SUPER DUPER, MUST BE REMOVED, HAXXXOR.
	//--------------------------------------------------

	// Set the initial charge.
	m_fCharge = 0.0f;

	// Register physics sound
	if (m_pobPhysicsSystem && m_pobSharedAttributes && !ntStr::IsNull(m_pobSharedAttributes->m_obPhysicsSoundDef))
	{
		m_pobPhysicsSystem->RegisterCollisionEffectFilterDef(m_pobSharedAttributes->m_obPhysicsSoundDef);
	}

	// Create and attach the statemachine
	PROJECTILE_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) PROJECTILE_FSM();
	ATTACH_FSM(pFSM);
}

//--------------------------------------------------
//!
//!	Object_Projectile::~Object_Projectile()
//!	Default destructor
//!
//--------------------------------------------------
Object_Projectile::~Object_Projectile()
{
}

//--------------------------------------------------
//!
//!	Object_Projectile::CreateCrossbowBolt(CEntity* pRangedWeapon, CEntity* pTargetEnt, bool bDestroyOnImpact)
//!	Creates a Crossbow bolt.
//! Note: AIs should not be using this function to fire their crossbow bolts... they should be using the other CreateCrossbowBolt.
//!
//--------------------------------------------------
Object_Projectile* Object_Projectile::CreateCrossbowBolt(CEntity* pRangedWeapon, CEntity* pOther, const Projectile_Attributes* const pProjAttrs, CEntity* pTargetEnt, bool bDestroyOnImpact, const CDirection* pdirShot, float fCharge)
{
	ntAssert( pProjAttrs );

	//Calculate the position to spawn the projectile at using the weapon's position, it's z-axis, and
	//an XML-defined TranslationOffset.
	CPoint vPosition = pRangedWeapon->GetPosition();
	CDirection vDirection;


	//If we have no target entity, just fire straight, otherwise, fire in their direction.
	if(pTargetEnt == NULL)
	{
		vDirection = pdirShot ? *pdirShot : pRangedWeapon->GetMatrix().GetZAxis();

		// FIX ME SOON: GAV
		if( pOther->IsPlayer() && pOther->ToPlayer()->IsArcher() && pOther->ToPlayer()->ToArcher()->GetState() == Archer::ARC_FIRING )
		{
			vDirection.Y() = 0.0f;
		}
	}
	else
	{
		CPoint vTargetPos = pTargetEnt->GetPosition() + CPoint( 0.0f, 1.0f, 0.0f );
        vDirection = CDirection((vTargetPos.X() - vPosition.X()), (vTargetPos.Y() - vPosition.Y()), (vTargetPos.Z() - vPosition.Z()));
	}

	// 
	CDirection vTemp	= CDirection(0.0f, 0.0f, 0.0f);
	CDirection vOffset	= vTemp * pRangedWeapon->GetMatrix();

	// 
	vPosition.X() += vOffset.X();
	vPosition.Y() += vOffset.Y();
	vPosition.Z() += vOffset.Z();

	Object_Projectile* pProjectile = CreateNewProjectileInDatabase( pProjAttrs );

	pProjectile->m_pobSharedAttributes = pProjAttrs;

	// Setup the projectile attributes
	pProjectile->m_ProjectileData.pAttacker = pOther;
	pProjectile->m_ProjectileData.pOriginator = pRangedWeapon;
	pProjectile->m_ProjectileData.pTarget = pTargetEnt;
	pProjectile->m_ProjectileData.vPosition = vPosition;
	pProjectile->m_ProjectileData.vDirection = vDirection;
	pProjectile->m_ProjectileData.ProjectileProperties = fCharge >= 1.0f ? pProjAttrs->m_PropertiesFullyCharged : pProjAttrs->m_Properties;
	pProjectile->m_ProjectileData.ProjectileAttackData = pProjAttrs->m_AttackData;
	pProjectile->m_ProjectileData.ProjectileAttackDataFullyCharged = pProjAttrs->m_AttackDataFullyCharged;
	pProjectile->m_ProjectileData.ProjectileAttackDataAOE = pProjAttrs->m_AOEAttackData;

	pProjectile->m_ProjectileData.ProjectileNonPoweredFrontHitAreas = pProjAttrs->m_NonPowered_Front_HitAreas;
	pProjectile->m_ProjectileData.ProjectileNonPoweredBackHitAreas = pProjAttrs->m_NonPowered_Back_HitAreas;
	pProjectile->m_ProjectileData.ProjectilePoweredFrontHitAreas = pProjAttrs->m_Powered_Front_HitAreas;
	pProjectile->m_ProjectileData.ProjectilePoweredBackHitAreas = pProjAttrs->m_Powered_Back_HitAreas;

	pProjectile->m_ProjectileData.AreaOfEffectRadius = pProjAttrs->m_AreaOfEffectRadius;
	pProjectile->m_ProjectileData.ProjectileCameraDef = fCharge >= 1.0f ? pProjAttrs->m_CameraDefFullyCharged : pProjAttrs->m_CameraDef;
	//pProjectile->m_ProjectileData.bFirstPersonAim = false;
	pProjectile->m_ProjectileData.bDestroyOnImpact = bDestroyOnImpact;
	pProjectile->m_ProjectileData.vTrackedEntityOffset = pProjAttrs->m_TrackedEntityOffset;
	pProjectile->m_ProjectileData.fTrackingSpeed = pProjAttrs->m_TrackingSpeed;
	pProjectile->m_ProjectileData.SoundVolume = pProjAttrs->m_SoundVolume;
	pProjectile->m_ProjectileData.SoundRadius = pProjAttrs->m_SoundRadius;
	pProjectile->m_ProjectileData.IsVolumeConstant = pProjAttrs->m_IsVolumeConstant;
	// Projectile audio control
	pProjectile->m_ProjectileData.LoopingSoundCue = pProjAttrs->m_LoopingSoundCue;
	pProjectile->m_ProjectileData.LoopingSoundBank = pProjAttrs->m_LoopingSoundBank;
	pProjectile->m_ProjectileData.LoopingSoundRange = pProjAttrs->m_LoopingSoundRange;
	pProjectile->m_ProjectileData.PassbySoundCue = pProjAttrs->m_PassbySoundCue;
	pProjectile->m_ProjectileData.PassbySoundBank = pProjAttrs->m_PassbySoundBank;
	pProjectile->m_ProjectileData.PassbySoundRange = pProjAttrs->m_PassbySoundRange;
	pProjectile->m_ProjectileData.ImpactSoundCue = pProjAttrs->m_ImpactSoundCue;
	pProjectile->m_ProjectileData.ImpactSoundBank = pProjAttrs->m_ImpactSoundBank;
	pProjectile->m_ProjectileData.FireSoundCue = pProjAttrs->m_FireSoundCue;
	pProjectile->m_ProjectileData.FireSoundBank = pProjAttrs->m_FireSoundBank;

	// Set the charge (archer specific)
	pProjectile->SetCharge( fCharge );

	// Construct the actual projectile object
	pProjectile->ConstructCrossbowObject();

	// Kai has shot a new bolt. Inform the AIDiving System
	CAINavigationSystemMan::Get().AddBolt(pProjectile);
	return pProjectile;
}

//--------------------------------------------------
//!
//!	Object_Projectile::CreateAGenSword
//!
//--------------------------------------------------
Object_Projectile* Object_Projectile::CreateAGenSword(CEntity* pAGen, CEntity* pOther, const Projectile_Attributes* const pProjAttrs, CEntity* pTargetEnt, CDirection* pobDirection)
{
	ntAssert( pProjAttrs );

	//Calculate the position to spawn the projectile at using the weapon's position, it's z-axis, and
	//an XML-defined TranslationOffset.
	CPoint vPosition = pAGen->GetPosition();
	CDirection vDirection;

	//If we have no target entity, just fire straight, otherwise, fire in their direction.
	if (pobDirection)
	{
		vDirection = *pobDirection;
	}
	else if (pTargetEnt == NULL)
	{
		vDirection = pAGen->GetMatrix().GetZAxis();
	}
	else
	{
		CPoint vTargetPos = pTargetEnt->GetPosition() + CPoint( 0.0f, 1.0f, 0.0f );
        vDirection = CDirection((vTargetPos.X() - vPosition.X()), (vTargetPos.Y() - vPosition.Y()), (vTargetPos.Z() - vPosition.Z()));
	}

	// 
	CDirection vTemp	= CDirection(0.0f, 0.0f, 0.0f);
	CDirection vOffset	= vTemp * pAGen->GetMatrix();

	// 
	vPosition.X() += vOffset.X();
	vPosition.Y() += vOffset.Y();
	vPosition.Z() += vOffset.Z();

	Object_Projectile* pProjectile = CreateNewProjectileInDatabase( pProjAttrs );

	pProjectile->m_pobSharedAttributes = pProjAttrs;

	// Setup the projectile attributes
	pProjectile->m_ProjectileData.pAttacker = pAGen;
	pProjectile->m_ProjectileData.pOriginator = pAGen;
	pProjectile->m_ProjectileData.pTarget = pTargetEnt;
	pProjectile->m_ProjectileData.vPosition = vPosition;
	pProjectile->m_ProjectileData.vDirection = vDirection;
	pProjectile->m_ProjectileData.ProjectileProperties = pProjAttrs->m_Properties;
	pProjectile->m_ProjectileData.ProjectileAttackData = pProjAttrs->m_AttackData;
	pProjectile->m_ProjectileData.ProjectileAttackDataAOE = pProjAttrs->m_AOEAttackData;
	pProjectile->m_ProjectileData.AreaOfEffectRadius = pProjAttrs->m_AreaOfEffectRadius;
	pProjectile->m_ProjectileData.ProjectileCameraDef = pProjAttrs->m_CameraDef;
	//pProjectile->m_ProjectileData.bFirstPersonAim = false;
	pProjectile->m_ProjectileData.bDestroyOnImpact = false;
	pProjectile->m_ProjectileData.vTrackedEntityOffset = pProjAttrs->m_TrackedEntityOffset;
	pProjectile->m_ProjectileData.fTrackingSpeed = pProjAttrs->m_TrackingSpeed;
	pProjectile->m_ProjectileData.SoundVolume = pProjAttrs->m_SoundVolume;
	pProjectile->m_ProjectileData.SoundRadius = pProjAttrs->m_SoundRadius;
	pProjectile->m_ProjectileData.IsVolumeConstant = pProjAttrs->m_IsVolumeConstant;
	// Projectile audio control
	pProjectile->m_ProjectileData.LoopingSoundCue = pProjAttrs->m_LoopingSoundCue;
	pProjectile->m_ProjectileData.LoopingSoundBank = pProjAttrs->m_LoopingSoundBank;
	pProjectile->m_ProjectileData.LoopingSoundRange = pProjAttrs->m_LoopingSoundRange;
	pProjectile->m_ProjectileData.PassbySoundCue = pProjAttrs->m_PassbySoundCue;
	pProjectile->m_ProjectileData.PassbySoundBank = pProjAttrs->m_PassbySoundBank;
	pProjectile->m_ProjectileData.PassbySoundRange = pProjAttrs->m_PassbySoundRange;

	// Construct the actual projectile object
	pProjectile->ConstructAGenSwordObject();

	return pProjectile;
}

bool Object_Projectile::AGenSwordIsMoving()
{
	Physics::ProjectileLG* pobLG = (Physics::ProjectileLG*)GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::PROJECTILE_LG);
	return pobLG->IsMoving();
}


//--------------------------------------------------
//!
//!	Object_Projectile::CreateKingLightningBall
//!
//--------------------------------------------------
Object_Projectile* Object_Projectile::CreateKingLightningBall(CEntity* pKing, CEntity* pOther, const Projectile_Attributes* const pProjAttrs, CEntity* pTargetEnt, bool bCounterable,
															  bool bGoToOrbit, bool bLarge)
{
	ntAssert( pProjAttrs );

	ntError_p(pKing && pKing->IsBoss() && (((Boss*)pKing)->GetBossType() == Boss::BT_KING_BOHAN), ("Error: Only the king should try to fire these lightning balls"));
	//Spawn the projectile from the king's hand offset slightly so that it's not "through" it (although it wouldn't matter really).
	CHashedString obKingHandTransform("l_weapon");
	Transform* pobKingHandTransform = pKing->GetHierarchy()->GetTransform(obKingHandTransform);
	if(!pobKingHandTransform)
	{
		ntError_p(false, ("Could not find l_weapon transform on king to spawn lightning ball from"));
	}

	CPoint vPosition = pobKingHandTransform->GetWorldTranslation();
	CDirection vDirection;

	//If we have no target entity, just fire straight, otherwise, fire in their direction.
	if(pTargetEnt == NULL)
	{
		vDirection = pKing->GetMatrix().GetZAxis();
	}
	else
	{
		CPoint vTargetPos = pTargetEnt->GetPosition() + CPoint( 0.0f, 1.0f, 0.0f );
        vDirection = CDirection((vTargetPos.X() - vPosition.X()), (vTargetPos.Y() - vPosition.Y()), (vTargetPos.Z() - vPosition.Z()));
	}

	// 
	CDirection vTemp = vDirection;
	vTemp.Normalise();
	CDirection vOffset = vTemp * 0.1f;	//We'll offset 0.1 units towards the player.

	// 
	vPosition.X() += vOffset.X();
	vPosition.Y() += vOffset.Y();
	vPosition.Z() += vOffset.Z();

	Object_Projectile* pProjectile = CreateNewProjectileInDatabase( pProjAttrs );

	pProjectile->m_pobSharedAttributes = pProjAttrs;

	// Setup the projectile attributes
	pProjectile->m_ProjectileData.pAttacker = pKing;
	pProjectile->m_ProjectileData.pOriginator = pKing;
	pProjectile->m_ProjectileData.pTarget = pTargetEnt;
	pProjectile->m_ProjectileData.vPosition = vPosition;
	pProjectile->m_ProjectileData.vDirection = vDirection;
	pProjectile->m_ProjectileData.ProjectileProperties = pProjAttrs->m_Properties;
	pProjectile->m_ProjectileData.ProjectileAttackData = pProjAttrs->m_AttackData;
	pProjectile->m_ProjectileData.ProjectileAttackDataAOE = pProjAttrs->m_AOEAttackData;
	pProjectile->m_ProjectileData.AreaOfEffectRadius = pProjAttrs->m_AreaOfEffectRadius;
	pProjectile->m_ProjectileData.ProjectileCameraDef = pProjAttrs->m_CameraDef;
	//pProjectile->m_ProjectileData.bFirstPersonAim = false;
	pProjectile->m_ProjectileData.bDestroyOnImpact = false;
	pProjectile->m_ProjectileData.vTrackedEntityOffset = pProjAttrs->m_TrackedEntityOffset;
	pProjectile->m_ProjectileData.fTrackingSpeed = pProjAttrs->m_TrackingSpeed;
	pProjectile->m_ProjectileData.SoundVolume = pProjAttrs->m_SoundVolume;
	pProjectile->m_ProjectileData.SoundRadius = pProjAttrs->m_SoundRadius;
	pProjectile->m_ProjectileData.IsVolumeConstant = pProjAttrs->m_IsVolumeConstant;
	// Projectile audio control
	pProjectile->m_ProjectileData.LoopingSoundCue = pProjAttrs->m_LoopingSoundCue;
	pProjectile->m_ProjectileData.LoopingSoundBank = pProjAttrs->m_LoopingSoundBank;
	pProjectile->m_ProjectileData.LoopingSoundRange = pProjAttrs->m_LoopingSoundRange;
	pProjectile->m_ProjectileData.PassbySoundCue = pProjAttrs->m_PassbySoundCue;
	pProjectile->m_ProjectileData.PassbySoundBank = pProjAttrs->m_PassbySoundBank;
	pProjectile->m_ProjectileData.PassbySoundRange = pProjAttrs->m_PassbySoundRange;
	pProjectile->m_ProjectileData.ImpactSoundCue = pProjAttrs->m_ImpactSoundCue;
	pProjectile->m_ProjectileData.ImpactSoundBank = pProjAttrs->m_ImpactSoundBank;
	pProjectile->m_ProjectileData.FireSoundCue = pProjAttrs->m_FireSoundCue;
	pProjectile->m_ProjectileData.FireSoundBank = pProjAttrs->m_FireSoundBank;


	//Is counterable?
	pProjectile->m_bIsCounterable = bCounterable;
	pProjectile->m_bIsOrbitingOriginator = bGoToOrbit;
	pProjectile->m_bLarge = bLarge;

	// Construct the actual projectile object
	pProjectile->ConstructKingLightningBallObject(bLarge);

	return pProjectile;
}

bool Object_Projectile::KingLightningBallIsMoving()
{
	Physics::ProjectileLG* pobLG = (Physics::ProjectileLG*)GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::PROJECTILE_LG);
	return pobLG->IsMoving();
}


void Object_Projectile::KingLightningBallLaunchFromOrbit()
{
	//By this point the projectile itself already has all of the data it needs, so we just sent it from where it is towards the
	//target with tracking etc.
	if(m_bIsOrbitingOriginator == false)
	{
		return;
	}

	ProjectileProperties* pobProperties = ObjectDatabase::Get().GetPointerFromName<ProjectileProperties*>(m_ProjectileData.ProjectileProperties);

	CPoint obPosition = GetPosition();
	CPoint obTargetPosition(CONSTRUCT_CLEAR);
	if(m_ProjectileData.pTarget)
	{
		obTargetPosition = m_ProjectileData.pTarget->GetPosition();
	}
	
	CDirection obToTarget = CDirection(obTargetPosition - obPosition);
	obToTarget.Normalise();

	Physics::ProjectileLG* pLG = (Physics::ProjectileLG*)GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::PROJECTILE_LG);
	if(pLG)
	{
		pLG->SetLinearVelocity(obToTarget * pobProperties->m_fInitialSpeed);
	}

	if(m_ProjectileData.pTarget != 0)
	{
		//Tell this projectile to track with the offset and tracking-speed specified.
		CPoint vOffset = m_ProjectileData.vTrackedEntityOffset;
		GetPhysicsSystem()->Lua_Projectile_EnableEntityTracking(m_ProjectileData.pTarget, vOffset.X(), vOffset.Y(), vOffset.Z(), m_ProjectileData.fTrackingSpeed);
	}

	m_bIsOrbitingOriginator = false;	//So that we don't reset anything ON_UPDATE.
}



//--------------------------------------------------
//!
//!	Object_Projectile::CreateKingLightningBall
//!
//--------------------------------------------------
Object_Projectile* Object_Projectile::CreateKingWingAttackCrow(CEntity* pKing, CEntity* pOther, const Projectile_Attributes* const pProjAttrs, CEntity* pTargetEnt, const CPoint &obOffset, bool bRandomTargetOffset)
{
	ntAssert( pProjAttrs );

	//Calculate the position to spawn the projectile at using the weapon's position, it's z-axis, and
	//an XML-defined TranslationOffset.
	//TODO: Where exactly are we supposed to be spawning this from?
	CPoint vPosition = pKing->GetPosition();

	// Put our offset into the world's local-space.
	CDirection vTemp	= CDirection(obOffset);
	CDirection vOffset	= vTemp * pKing->GetMatrix();

	// Apply the offset.
	vPosition.X() += vOffset.X();
	vPosition.Y() += vOffset.Y();
	vPosition.Z() += vOffset.Z();

	//Calculate the direction from the new offset position to the player (if target is set).
	//For crows, if the bRandomTargetOffset flag is set, then generate the target-point
	//as the player position plus a random offset on x and z (but within a meter).
	CDirection vDirection;

	bool bManualTargetOffset = false;
	CPoint obTargetOffset(CONSTRUCT_CLEAR);

	//If we have no target entity, just fire straight, otherwise, fire in their direction.
	if(pTargetEnt == NULL)
	{
		vDirection = pKing->GetMatrix().GetZAxis();
	}
	else
	{
		//Generate random offsets from -3.0f -> 3.0f on X and Z if bRandomTargetOffset is set (otherwise no offset).
		float fRandXOffset = 0.0f;
		float fRandZOffset = 0.0f;
		if(bRandomTargetOffset)
		{
			fRandXOffset = grandf(6.0f) - 3.0f;
			fRandZOffset = grandf(6.0f) - 3.0f;
		}
		CPoint vTargetPos = pTargetEnt->GetPosition() + CPoint( fRandXOffset, 0.5f, fRandZOffset );
        vDirection = CDirection((vTargetPos.X() - vPosition.X()), (vTargetPos.Y() - vPosition.Y()), (vTargetPos.Z() - vPosition.Z()));

		//Store this target offset in the projectile below so that the crows don't converge to a point.
		obTargetOffset = CPoint(fRandXOffset, 0.5f, fRandZOffset);
		bManualTargetOffset = true;
	}


	Object_Projectile* pProjectile = CreateNewProjectileInDatabase( pProjAttrs );

	pProjectile->m_pobSharedAttributes = pProjAttrs;

	// Setup the projectile attributes
	pProjectile->m_ProjectileData.pAttacker = pKing;
	pProjectile->m_ProjectileData.pOriginator = pKing;
	pProjectile->m_ProjectileData.pTarget = pTargetEnt;
	pProjectile->m_ProjectileData.vPosition = vPosition;
	pProjectile->m_ProjectileData.vDirection = vDirection;
	pProjectile->m_ProjectileData.ProjectileProperties = pProjAttrs->m_Properties;
	pProjectile->m_ProjectileData.ProjectileAttackData = pProjAttrs->m_AttackData;
	pProjectile->m_ProjectileData.ProjectileAttackDataAOE = pProjAttrs->m_AOEAttackData;
	pProjectile->m_ProjectileData.AreaOfEffectRadius = pProjAttrs->m_AreaOfEffectRadius;
	pProjectile->m_ProjectileData.ProjectileCameraDef = pProjAttrs->m_CameraDef;
	//pProjectile->m_ProjectileData.bFirstPersonAim = false;
	pProjectile->m_ProjectileData.bDestroyOnImpact = false;
	pProjectile->m_ProjectileData.vTrackedEntityOffset = (bManualTargetOffset) ? obTargetOffset : pProjAttrs->m_TrackedEntityOffset;
	pProjectile->m_ProjectileData.fTrackingSpeed = pProjAttrs->m_TrackingSpeed;
	pProjectile->m_ProjectileData.SoundVolume = pProjAttrs->m_SoundVolume;
	pProjectile->m_ProjectileData.SoundRadius = pProjAttrs->m_SoundRadius;
	pProjectile->m_ProjectileData.IsVolumeConstant = pProjAttrs->m_IsVolumeConstant;
	// Projectile audio control
	pProjectile->m_ProjectileData.LoopingSoundCue = pProjAttrs->m_LoopingSoundCue;
	pProjectile->m_ProjectileData.LoopingSoundBank = pProjAttrs->m_LoopingSoundBank;
	pProjectile->m_ProjectileData.LoopingSoundRange = pProjAttrs->m_LoopingSoundRange;
	pProjectile->m_ProjectileData.PassbySoundCue = pProjAttrs->m_PassbySoundCue;
	pProjectile->m_ProjectileData.PassbySoundBank = pProjAttrs->m_PassbySoundBank;
	pProjectile->m_ProjectileData.PassbySoundRange = pProjAttrs->m_PassbySoundRange;
	pProjectile->m_ProjectileData.ImpactSoundCue = pProjAttrs->m_ImpactSoundCue;
	pProjectile->m_ProjectileData.ImpactSoundBank = pProjAttrs->m_ImpactSoundBank;
	pProjectile->m_ProjectileData.FireSoundCue = pProjAttrs->m_FireSoundCue;
	pProjectile->m_ProjectileData.FireSoundBank = pProjAttrs->m_FireSoundBank;

	//Is counterable?
	pProjectile->m_bIsCounterable = false;

	// Construct the actual projectile object
	pProjectile->ConstructKingWingAttackCrowObject();

	return pProjectile;
}









//--------------------------------------------------
//!
//!	Object_Projectile::CreateCrossbowBolt(Object_Ranged_Weapon* pRangedWeapon, CEntity* pTargetEnt, bool bDestroyOnImpact)
//!	Creates a Crossbow bolt.
//!
//--------------------------------------------------
Object_Projectile* Object_Projectile::CreateCrossbowBolt(Object_Ranged_Weapon* pRangedWeapon, const Projectile_Attributes* const pProjAttrs, CEntity* pTargetEnt, bool bDestroyOnImpact)
{
	ntAssert( pProjAttrs );

	//ntPrintf("Creating crossbow bolt\n");

	CEntity* pShooter = pRangedWeapon->m_pOther;
	//UNUSED(player);
	
	//Calculate the position to spawn the projectile at using the weapon's position, it's z-axis, and
	//an XML-defined TranslationOffset.
	CPoint vPosition = pRangedWeapon->GetPosition();
	CDirection vDirection;
	CPoint obAccuracyOffset = pProjAttrs->m_TrackedEntityOffset;

	Object_Projectile* pProjectile = CreateNewProjectileInDatabase( pProjAttrs );

	//Calculate exactly where the bolt will be spawned from (relative to the weapon).
	//We need to do this here, before calculating direction for the projectile or it can be off-direction.
	CDirection vTemp(pRangedWeapon->GetTranslationOffset());
	CDirection vOffset = vTemp * pRangedWeapon->GetMatrix();
	vPosition.X() += vOffset.X();
	vPosition.Y() += vOffset.Y();
	vPosition.Z() += vOffset.Z();

	//If we have no target entity, just fire straight, otherwise, fire in their direction.
	if(pTargetEnt == NULL)
	{
		vDirection = pRangedWeapon->GetMatrix().GetZAxis();
	}
	else
	{
		CPoint vTargetPos = pTargetEnt->GetPosition();
        vDirection = CDirection(vTargetPos - vPosition);

		if (pShooter->ToAI())
		{
			CAIComponent* pAIComp = pShooter->ToAI()->GetAIComponent();
			CAIMovement* pMov = pAIComp->GetCAIMovement();
			CAIRangedTargeting* pRangedTargeting = pAIComp->GetAIRangedTargeting();

			if(pRangedTargeting)
			{
				pMov->SetShootingPoint( pRangedTargeting->GetTargetPoint() );	//For debug-render.

				//We want to fire here.
				vTargetPos = pRangedTargeting->GetTargetPoint();
				vDirection = CDirection(vTargetPos - vPosition);
			}
		}
		else
		{
			pProjectile->m_ProjectileData.m_eWayOfShooting = SHOOT_NON_APPLICABLE;
			obAccuracyOffset = pProjAttrs->m_TrackedEntityOffset;
		}	
	}

	pProjectile->m_pobSharedAttributes = pProjAttrs;

	// Setup the projectile attributes
	pProjectile->m_ProjectileData.pAttacker = pRangedWeapon->m_pOther;
	pProjectile->m_ProjectileData.pOriginator = pRangedWeapon;
	pProjectile->m_ProjectileData.pTarget = pTargetEnt;
	pProjectile->m_ProjectileData.vPosition = vPosition;
	pProjectile->m_ProjectileData.vDirection = vDirection;
	pProjectile->m_ProjectileData.ProjectileProperties = pProjAttrs->m_Properties;
	pProjectile->m_ProjectileData.ProjectileAttackData = pProjAttrs->m_AttackData;
	pProjectile->m_ProjectileData.ProjectileAttackDataAOE = pProjAttrs->m_AOEAttackData;
	pProjectile->m_ProjectileData.AreaOfEffectRadius = pProjAttrs->m_AreaOfEffectRadius;
	pProjectile->m_ProjectileData.ProjectileCameraDef = pProjAttrs->m_CameraDef;
	//pProjectile->m_ProjectileData.bFirstPersonAim = pRangedWeapon->m_bAiming;
	pProjectile->m_ProjectileData.bDestroyOnImpact = bDestroyOnImpact;
	pProjectile->m_ProjectileData.vTrackedEntityOffset = obAccuracyOffset;
	pProjectile->m_ProjectileData.fTrackingSpeed = (pShooter->IsAI()) ? 0.0f : pProjAttrs->m_TrackingSpeed;	//No tracking for AI-fired bolts.
	// Projectile audio control
	pProjectile->m_ProjectileData.LoopingSoundCue = pProjAttrs->m_LoopingSoundCue;
	pProjectile->m_ProjectileData.LoopingSoundBank = pProjAttrs->m_LoopingSoundBank;
	pProjectile->m_ProjectileData.LoopingSoundRange = pProjAttrs->m_LoopingSoundRange;
	pProjectile->m_ProjectileData.PassbySoundCue = pProjAttrs->m_PassbySoundCue;
	pProjectile->m_ProjectileData.PassbySoundBank = pProjAttrs->m_PassbySoundBank;
	pProjectile->m_ProjectileData.PassbySoundRange = pProjAttrs->m_PassbySoundRange;
	pProjectile->m_ProjectileData.ImpactSoundCue = pProjAttrs->m_ImpactSoundCue;
	pProjectile->m_ProjectileData.ImpactSoundBank = pProjAttrs->m_ImpactSoundBank;
	pProjectile->m_ProjectileData.FireSoundCue = pProjAttrs->m_FireSoundCue;
	pProjectile->m_ProjectileData.FireSoundBank = pProjAttrs->m_FireSoundBank;

	// Construct the actual projectile object
	pProjectile->ConstructCrossbowObject();

	//If we made a projectile successfully, then add it to the weapon.
	if(pProjectile != NULL)
	{
		//TODO: Weapon needs to be able to store the projectiles for aftertouch purposes.
//		pRangedWeapon->Projectile.insert(0, pProjectile);
//		pRangedWeapon->ProjectileName.insert(0, name);
//		pRangedWeapon->m_iProjectileCount++;
	}

	return pProjectile;
}

//--------------------------------------------------
//!
//!	Object_Projectile::CreateCannonBall(Interactable_TurretWeapon* pInteractable, CEntity* pTargetEnt, bool bDestroyOnImpact)
//!	Creates a Crossbow bolt.
//!
//--------------------------------------------------
Object_Projectile* Object_Projectile::CreateCannonBall(Interactable_TurretWeapon* pInteractable, const Projectile_Attributes* const pProjAttrs, CEntity* pTargetEnt, bool bDestroyOnImpact)
{
	ntAssert( pProjAttrs );

	CEntity* player = pInteractable->m_pOther;
	UNUSED(player);
	
	//Calculate the position to spawn the projectile at using the weapon's position, it's z-axis, and
	//an XML-defined TranslationOffset.
	Transform* pobLaunchTransform = ((CEntity*)pInteractable)->GetHierarchy()->GetTransform( pInteractable->GetLaunchTransform() );
	CPoint vPosition = pobLaunchTransform->GetWorldTranslation();
	CDirection vDirection;

	//If we have no target entity, just fire straight, otherwise, fire in their direction.
	if(pTargetEnt == NULL)
	{
		vDirection =  pobLaunchTransform->GetWorldMatrix().GetZAxis();
	}
	else
	{
		CPoint vTargetPos = pTargetEnt->GetPosition() + CPoint( 0.0f, 1.0f, 0.0f );
        vDirection = CDirection((vTargetPos.X() - vPosition.X()), (vTargetPos.Y() - vPosition.Y()), (vTargetPos.Z() - vPosition.Z()));
	}

	CDirection vTemp(pInteractable->GetTranslationOffset());
	CDirection vOffset = vTemp * player->GetMatrix();

	vPosition.X() += vOffset.X();
	vPosition.Y() += vOffset.Y();
	vPosition.Z() += vOffset.Z();

	Object_Projectile* pProjectile = CreateNewProjectileInDatabase( pProjAttrs );

	pProjectile->m_pobSharedAttributes = pProjAttrs;

	// Setup the projectile attributes
	pProjectile->m_ProjectileData.pAttacker						= pInteractable->m_pOther;
	pProjectile->m_ProjectileData.pOriginator					= pInteractable;
	pProjectile->m_ProjectileData.pTarget						= pTargetEnt;
	pProjectile->m_ProjectileData.vPosition						= vPosition;
	pProjectile->m_ProjectileData.vDirection					= vDirection;
	pProjectile->m_ProjectileData.ProjectileProperties			= pProjAttrs->m_Properties;
	pProjectile->m_ProjectileData.ProjectileAttackData			= pProjAttrs->m_AttackData;
	pProjectile->m_ProjectileData.ProjectileAttackDataAOE		= pProjAttrs->m_AOEAttackData;
	pProjectile->m_ProjectileData.AreaOfEffectRadius			= pProjAttrs->m_AreaOfEffectRadius;
	pProjectile->m_ProjectileData.ProjectileCameraDef			= pProjAttrs->m_CameraDef;
	//pProjectile->m_ProjectileData.bFirstPersonAim				= pInteractable->m_bAiming;
	pProjectile->m_ProjectileData.bDestroyOnImpact				= bDestroyOnImpact;
	pProjectile->m_ProjectileData.vTrackedEntityOffset			= pProjAttrs->m_TrackedEntityOffset;
	pProjectile->m_ProjectileData.fTrackingSpeed				= pProjAttrs->m_TrackingSpeed;
	// Projectile audio control
	pProjectile->m_ProjectileData.LoopingSoundCue				= pProjAttrs->m_LoopingSoundCue;
	pProjectile->m_ProjectileData.LoopingSoundBank				= pProjAttrs->m_LoopingSoundBank;
	pProjectile->m_ProjectileData.LoopingSoundRange				= pProjAttrs->m_LoopingSoundRange;
	pProjectile->m_ProjectileData.PassbySoundCue				= pProjAttrs->m_PassbySoundCue;
	pProjectile->m_ProjectileData.PassbySoundBank				= pProjAttrs->m_PassbySoundBank;
	pProjectile->m_ProjectileData.PassbySoundRange				= pProjAttrs->m_PassbySoundRange;
	pProjectile->m_ProjectileData.ImpactSoundCue				= pProjAttrs->m_ImpactSoundCue;
	pProjectile->m_ProjectileData.ImpactSoundBank				= pProjAttrs->m_ImpactSoundBank;
	pProjectile->m_ProjectileData.FireSoundCue					= pProjAttrs->m_FireSoundCue;
	pProjectile->m_ProjectileData.FireSoundBank					= pProjAttrs->m_FireSoundBank;

	// Construct the actual projectile object
	pProjectile->ConstructCannonballObject();

	return pProjectile;
}


//--------------------------------------------------
//!
//!	Object_Projectile* Object_Projectile::CreateBazookaRockets(Object_Ranged_Weapon* pRangedWeapon, CEntity* pTargetEnt, bool bDestroyOnImpact)
//!	Creates a Bazooka rocket.
//!
//--------------------------------------------------
Object_Projectile* Object_Projectile::CreateBazookaRockets(Object_Ranged_Weapon* pRangedWeapon, const Projectile_Attributes* const pProjAttrs, CEntity* pTargetEnt, bool bDestroyOnImpact)
{
	ntPrintf("Creating bazooka rockets\n");
	Object_Projectile* pRocketToReturn = NULL;

	CEntity* pPlayer = pRangedWeapon->m_pOther;
	UNUSED(pPlayer);

	//Calculate the position to spawn the projectile at using the weapon's position, it's z-axis, and
	//an XML-defined TranslationOffset.
	CPoint vPosition = pRangedWeapon->GetPosition();
	CDirection vDirection;
	//If we have no target entity, just fire straight, otherwise, fire in their direction.
	if(pTargetEnt == NULL)
	{
		vDirection = pRangedWeapon->GetMatrix().GetZAxis();
	}
	else
	{
		CPoint vTargetPos = pTargetEnt->GetPosition();
        vDirection = CDirection((vTargetPos.X() - vPosition.X()), (vTargetPos.Y() - vPosition.Y()), (vTargetPos.Z() - vPosition.Z()));
	}

	CDirection vTemp(pRangedWeapon->GetTranslationOffset());
	CDirection vOffset = vTemp * pRangedWeapon->GetMatrix();

	vPosition.X() += vOffset.X();
	vPosition.Y() += vOffset.Y();
	vPosition.Z() += vOffset.Z();

	vDirection.Y() = 0.0f;

	//Trigger the particle effect.
	FXHelper::Pfx_CreateStatic("BazookaFire_Def", pRangedWeapon, "ROOT");
	FXHelper::Pfx_CreateStatic("BazookaPrimSmoke_Def", pRangedWeapon, "ROOT");
	FXHelper::Pfx_CreateStatic("BazookaMuzzle_Def", pRangedWeapon, "ROOT");
	FXHelper::Pfx_CreateAttached("BazookaSparks_Def", pRangedWeapon->GetName().c_str(), "ROOT");
	FXHelper::Pfx_CreateAttached("BazookaSmokeFlow_Def", pRangedWeapon->GetName().c_str(), "ROOT");

	// Bazooka exhaust effect
	FXHelper::Pfx_CreateAttached("BazookaExhaustSmoke_Def", pRangedWeapon->GetName().c_str(), "ROOT");
	FXHelper::Pfx_CreateAttached("BazookaExhaustSmokeTrail_Def", pRangedWeapon->GetName().c_str(), "ROOT");
	FXHelper::Pfx_CreateAttached("BazookaExhaustSparks_Def", pRangedWeapon->GetName().c_str(), "ROOT");

	//Play sound effect.
	// AudioHelper::PlaySound("misc_sb","h_bazooka_fire");
	// See below.


	// DUMMY ROCKET FOR AFTERTOUCH
	//------------------------------------------------------------

	//Create the projectile to be used for aftertouch.
	
	Object_Projectile* pDummyRocket = CreateNewProjectileInDatabase( pProjAttrs );

	pDummyRocket->m_pobSharedAttributes = pProjAttrs;

	//Do the extra (entity-based) setup.
	pDummyRocket->m_ProjectileData.pAttacker = pRangedWeapon->m_pOther;
	pDummyRocket->m_ProjectileData.pOriginator = pRangedWeapon;
	pDummyRocket->m_ProjectileData.pTarget = pTargetEnt;
	pDummyRocket->m_ProjectileData.vPosition = vPosition;
	pDummyRocket->m_ProjectileData.vDirection = vDirection;
	pDummyRocket->m_ProjectileData.ProjectileProperties = CHashedString("LeadBazoozaRocketProperties");
	pDummyRocket->m_ProjectileData.ProjectileAttackData = CHashedString("");
	pDummyRocket->m_ProjectileData.ProjectileCameraDef = CHashedString("RocketCameraDef");
	//pDummyRocket->m_ProjectileData.bFirstPersonAim = pRangedWeapon->m_bAiming;
	pDummyRocket->m_ProjectileData.bDestroyOnImpact = bDestroyOnImpact;
	pDummyRocket->m_ProjectileData.vTrackedEntityOffset = pProjAttrs->m_TrackedEntityOffset;
	pDummyRocket->m_ProjectileData.fTrackingSpeed = pProjAttrs->m_TrackingSpeed;
	pDummyRocket->m_iProjectileIndex = iNumBazookaRockets; // Make it the last index
	// ChipB: I've assumed projectile audio is not required for the dummy rocket...

	pDummyRocket->ConstructBazookaObject();

	// We need to hide this dummy rocket
	pDummyRocket->GetRenderableComponent()->AddRemoveAll_Game( false );

	// Get the name that was assigned to the projectile
	LuaAttributeTable* pDummyAttributes = pDummyRocket->GetAttributeTable();
	ntstd::String obProjName = pDummyAttributes->GetString("Name");
	pRangedWeapon->Projectile.push_back( pDummyRocket );
	pRangedWeapon->ProjectileName.push_back( obProjName );

	// Projectile audio control
	// Prepare looping sound argument
	int iProjArgCur = (int)(((unsigned int)grand())>>1); // Shift ensures positive

	// NORMAL CLUSTER OF ROCKETS
	//------------------------------------------------------------

	//Spawn a cluster of rockets.
	Object_Projectile* pLastProjectile = 0;
	for(int i = 0 ; i < iNumBazookaRockets ; i++)
	{
		//Note: The reason we don't just use SpawnSingleBazooka here is that it was created for event-spawned projectiles, and does not take certain
		//useful parameters that would be stored in, or passed-from, the weapon.
		//Instead then, we just duplicate it here with those changes. Not pretty I know, but hardly unmanageable!

		Object_Projectile* pProjectile = CreateNewProjectileInDatabase( pProjAttrs );

		pProjectile->m_pobSharedAttributes = pProjAttrs;

		//Do the extra (entity-based) setup.
		pProjectile->m_ProjectileData.pAttacker = pRangedWeapon->m_pOther;
		pProjectile->m_ProjectileData.pOriginator = pRangedWeapon;
		pProjectile->m_ProjectileData.pTarget = pTargetEnt;
		pProjectile->m_ProjectileData.vPosition = vPosition;
		pProjectile->m_ProjectileData.vDirection = vDirection;
		pProjectile->m_ProjectileData.ProjectileProperties = pProjAttrs->m_Properties;
		pProjectile->m_ProjectileData.ProjectileAttackData = pProjAttrs->m_AttackData;
		pProjectile->m_ProjectileData.ProjectileAttackDataAOE = pProjAttrs->m_AOEAttackData;
		pProjectile->m_ProjectileData.AreaOfEffectRadius = pProjAttrs->m_AreaOfEffectRadius;
		pProjectile->m_ProjectileData.ProjectileCameraDef = pProjAttrs->m_CameraDef;
		//pProjectile->m_ProjectileData.bFirstPersonAim = pRangedWeapon->m_bAiming;
		pProjectile->m_ProjectileData.bDestroyOnImpact = bDestroyOnImpact;
		pProjectile->m_ProjectileData.vTrackedEntityOffset = pProjAttrs->m_TrackedEntityOffset;
		pProjectile->m_ProjectileData.fTrackingSpeed = pProjAttrs->m_TrackingSpeed;
		pProjectile->m_iProjectileIndex = i;
		// Projectile audio control
		pProjectile->m_ProjectileData.LoopingSoundCue = pProjAttrs->m_LoopingSoundCue;
		pProjectile->m_ProjectileData.LoopingSoundBank = pProjAttrs->m_LoopingSoundBank;
		pProjectile->m_ProjectileData.LoopingSoundRange = pProjAttrs->m_LoopingSoundRange;
		pProjectile->m_ProjectileData.PassbySoundCue = pProjAttrs->m_PassbySoundCue;
		pProjectile->m_ProjectileData.PassbySoundBank = pProjAttrs->m_PassbySoundBank;
		pProjectile->m_ProjectileData.PassbySoundRange = pProjAttrs->m_PassbySoundRange;
		pProjectile->m_ProjectileData.ImpactSoundCue = pProjAttrs->m_ImpactSoundCue;
		pProjectile->m_ProjectileData.ImpactSoundBank = pProjAttrs->m_ImpactSoundBank;
		pProjectile->m_ProjectileData.FireSoundCue = pProjAttrs->m_FireSoundCue;
		pProjectile->m_ProjectileData.FireSoundBank = pProjAttrs->m_FireSoundBank;
		pProjectile->m_iLoopingSoundArg = iProjArgCur;
		// Adjust looping sound argument
		if (++iProjArgCur < 0)
			iProjArgCur = 0;

		pProjectile->m_pNextProjectile = pLastProjectile;
		if(pLastProjectile)
		{
			pLastProjectile->m_pPrevProjectile = pProjectile;
		}
		pLastProjectile = pProjectile;

		pProjectile->ConstructBazookaObject();

		//Store the first rocket to be returned as notification to the weapon that we successfully got to the spawning part!
		if(pRocketToReturn == NULL)
		{
			pRocketToReturn = pProjectile;
		}

		// Get the name that was assigned to the projectile
		LuaAttributeTable* pAttributes = pProjectile->GetAttributeTable();
		ntstd::String obProjName = pAttributes->GetString("Name");

		// Insert into the weapon's 'Projectiles' vector here.
		pRangedWeapon->Projectile.push_back( pProjectile );
		pRangedWeapon->ProjectileName.push_back( obProjName );

		//--------------------------------------------------
		// TGS MEGA SUPER DUPER, MUST BE REMOVED, HAXXXOR.
		//--------------------------------------------------
		pDummyRocket->BazookaChildren[i] = CHashedString(pProjectile->GetName().c_str());
		//--------------------------------------------------
		// TGS MEGA SUPER DUPER, MUST BE REMOVED, HAXXXOR.
		//--------------------------------------------------
	}

	// Dummy Rocket links the child rockets
	pDummyRocket->m_pNextProjectile = pLastProjectile;

	// Change the value of the weapon's ProjectileCount to be the number of projectiles it now has.
	pRangedWeapon->m_iProjectileCount = iNumBazookaRockets; // +1 for dummy?

	// Play sound effect
	// Bazooka fire audio handled in "create" method (as opposed to "on spawn" like other projectiles")
	// due to cluster fire behaviour (see below).
	if (pRocketToReturn)
	{
		pRocketToReturn->ProjectileFireAudio();
	}
	

	//Just return the first of our new projectiles.
	return pRocketToReturn;
}


//--------------------------------------------------
//!
//!	Object_Projectile::SpawnTrackedProjectile
//!	Spawns a tracked projectile
//!
//--------------------------------------------------
Object_Projectile* Object_Projectile::SpawnTrackedProjectile(const Projectile_Attributes* const pProjAttrs, const CPoint& obOrigin, CEntity* pobTarget, bool bDestroyOnImpact )
{
	ntAssert_p( pobTarget, ("You have to specify a target for a tracked projectile!") );
	ntAssert( pProjAttrs );

	// Generate direction towards target entity (bearing in mind the offset)
	CPoint	obTargetPos = pobTarget->GetPosition() + pProjAttrs->m_TrackedEntityOffset;
	CDirection obDirection = (CDirection)(obTargetPos - obOrigin);
	obDirection.Normalise();

	Object_Projectile* pProjectile = CreateNewProjectileInDatabase( pProjAttrs );

	pProjectile->m_pobSharedAttributes = pProjAttrs;

	//Do the extra (entity-based) setup.
	pProjectile->m_ProjectileData.pAttacker = 0;
	pProjectile->m_ProjectileData.pOriginator = 0;
	pProjectile->m_ProjectileData.pTarget = pobTarget;
	pProjectile->m_ProjectileData.vPosition = obOrigin;
	pProjectile->m_ProjectileData.vDirection = obDirection;
	pProjectile->m_ProjectileData.ProjectileProperties = pProjAttrs->m_Properties;
	pProjectile->m_ProjectileData.ProjectileAttackData = pProjAttrs->m_AttackData;
	pProjectile->m_ProjectileData.ProjectileAttackDataAOE = pProjAttrs->m_AOEAttackData;
	pProjectile->m_ProjectileData.AreaOfEffectRadius = pProjAttrs->m_AreaOfEffectRadius;
	pProjectile->m_ProjectileData.ProjectileCameraDef = pProjAttrs->m_CameraDef;
	//pProjectile->m_ProjectileData.bFirstPersonAim = false;
	pProjectile->m_ProjectileData.bDestroyOnImpact = bDestroyOnImpact;
	pProjectile->m_ProjectileData.vTrackedEntityOffset = pProjAttrs->m_TrackedEntityOffset;
	pProjectile->m_ProjectileData.fTrackingSpeed = pProjAttrs->m_TrackingSpeed;
	// Projectile audio control
	pProjectile->m_ProjectileData.LoopingSoundCue = pProjAttrs->m_LoopingSoundCue;
	pProjectile->m_ProjectileData.LoopingSoundBank = pProjAttrs->m_LoopingSoundBank;
	pProjectile->m_ProjectileData.LoopingSoundRange = pProjAttrs->m_LoopingSoundRange;
	pProjectile->m_ProjectileData.PassbySoundCue = pProjAttrs->m_PassbySoundCue;
	pProjectile->m_ProjectileData.PassbySoundBank = pProjAttrs->m_PassbySoundBank;
	pProjectile->m_ProjectileData.PassbySoundRange = pProjAttrs->m_PassbySoundRange;
	pProjectile->m_ProjectileData.ImpactSoundCue = pProjAttrs->m_ImpactSoundCue;
	pProjectile->m_ProjectileData.ImpactSoundBank = pProjAttrs->m_ImpactSoundBank;
	pProjectile->m_ProjectileData.FireSoundCue = pProjAttrs->m_FireSoundCue;
	pProjectile->m_ProjectileData.FireSoundBank = pProjAttrs->m_FireSoundBank;

	switch ( pProjectile->m_eProjectileType )
	{
		case PROJECTILE_TYPE_BALLISTA_BOLT:
		case PROJECTILE_TYPE_CROSSBOW_BOLT:
			pProjectile->ConstructCrossbowObject();
			break;

		case PROJECTILE_TYPE_BAZOOKA_ROCKET:
			pProjectile->ConstructBazookaObject();
			break;
		case PROJECTILE_TYPE_AGEN_SWORD:
			pProjectile->ConstructAGenSwordObject();
			break;
		case PROJECTILE_TYPE_KING_LIGHTNINGBALL:
			pProjectile->ConstructKingLightningBallObject();
			break;
		case PROJECTILE_TYPE_KING_WINGATTACKCROW:
			pProjectile->ConstructKingWingAttackCrowObject();
			break;
		case PROJECTILE_TYPE_CANNON_BALL:
			pProjectile->ConstructCannonballObject();
			break;

		default:
			ntAssert_p( false, ("Unknown projectile type") );
			break;
	}
	
	return pProjectile;
}


//--------------------------------------------------
//!
//!	Object_Projectile::SpawnAimedProjectile
//!	Spawns an aimed projectile in the direction of the target
//!
//--------------------------------------------------
Object_Projectile* Object_Projectile::SpawnAimedProjectile(const Projectile_Attributes* const pProjAttrs, const CPoint& obOrigin, const CPoint& obTargetPos, bool bDestroyOnImpact)
{
	ntAssert( pProjAttrs );

	// Generate direction towards target entity
	CDirection obDirection = (CDirection)(obTargetPos - obOrigin);
	obDirection.Normalise();

	Object_Projectile* pProjectile = CreateNewProjectileInDatabase( pProjAttrs );

	pProjectile->m_pobSharedAttributes = pProjAttrs;

	//Do the extra (entity-based) setup.
	pProjectile->m_ProjectileData.pAttacker = 0;
	pProjectile->m_ProjectileData.pOriginator = 0;
	pProjectile->m_ProjectileData.pTarget = 0;
	pProjectile->m_ProjectileData.vPosition = obOrigin;
	pProjectile->m_ProjectileData.vDirection = obDirection;
	pProjectile->m_ProjectileData.ProjectileProperties = pProjAttrs->m_Properties;
	pProjectile->m_ProjectileData.ProjectileAttackData = pProjAttrs->m_AttackData;
	pProjectile->m_ProjectileData.ProjectileAttackDataAOE = pProjAttrs->m_AOEAttackData;
	pProjectile->m_ProjectileData.AreaOfEffectRadius = pProjAttrs->m_AreaOfEffectRadius;
	pProjectile->m_ProjectileData.ProjectileCameraDef = pProjAttrs->m_CameraDef;
	//pProjectile->m_ProjectileData.bFirstPersonAim = false;
	pProjectile->m_ProjectileData.bDestroyOnImpact = bDestroyOnImpact;
	pProjectile->m_ProjectileData.vTrackedEntityOffset = pProjAttrs->m_TrackedEntityOffset;
	pProjectile->m_ProjectileData.fTrackingSpeed = pProjAttrs->m_TrackingSpeed;
	// Projectile audio control
	pProjectile->m_ProjectileData.LoopingSoundCue = pProjAttrs->m_LoopingSoundCue;
	pProjectile->m_ProjectileData.LoopingSoundBank = pProjAttrs->m_LoopingSoundBank;
	pProjectile->m_ProjectileData.LoopingSoundRange = pProjAttrs->m_LoopingSoundRange;
	pProjectile->m_ProjectileData.PassbySoundCue = pProjAttrs->m_PassbySoundCue;
	pProjectile->m_ProjectileData.PassbySoundBank = pProjAttrs->m_PassbySoundBank;
	pProjectile->m_ProjectileData.PassbySoundRange = pProjAttrs->m_PassbySoundRange;
	pProjectile->m_ProjectileData.ImpactSoundCue = pProjAttrs->m_ImpactSoundCue;
	pProjectile->m_ProjectileData.ImpactSoundBank = pProjAttrs->m_ImpactSoundBank;
	pProjectile->m_ProjectileData.FireSoundCue = pProjAttrs->m_FireSoundCue;
	pProjectile->m_ProjectileData.FireSoundBank = pProjAttrs->m_FireSoundBank;

	switch ( pProjectile->m_eProjectileType )
	{
		case PROJECTILE_TYPE_BALLISTA_BOLT:
		case PROJECTILE_TYPE_CROSSBOW_BOLT:
			pProjectile->ConstructCrossbowObject();
			break;

		case PROJECTILE_TYPE_BAZOOKA_ROCKET:
			pProjectile->ConstructBazookaObject();
			break;

		case PROJECTILE_TYPE_AGEN_SWORD:
			pProjectile->ConstructAGenSwordObject();
			break;

		case PROJECTILE_TYPE_KING_LIGHTNINGBALL:
			pProjectile->ConstructKingLightningBallObject();
			break;

		case PROJECTILE_TYPE_KING_WINGATTACKCROW:
			pProjectile->ConstructKingWingAttackCrowObject();
			break;

		default:
			ntAssert_p( false, ("Unknown projectile type") );
			break;
	}

	return pProjectile;
}

//--------------------------------------------------
//!
//!	Object_Projectile::CrossbowOnSpawn
//! Spawn function for the crossbow bolt
//!
//--------------------------------------------------
void Object_Projectile::AGenSwordOnSpawn( void )
{
	ntAssert( m_pobSharedAttributes );	

	uint iID;
	for ( PFXIter obIt = m_pobSharedAttributes->m_aobPfxOnSpawnAttached.begin(); 
		obIt != m_pobSharedAttributes->m_aobPfxOnSpawnAttached.end(); obIt++ )
	{
		iID = FXHelper::Pfx_CreateAttached(*obIt, CHashedString(this->GetName().c_str()), "ROOT");
		m_aiPfxID.push_back(iID);
	}

	for ( PFXIter obIt = m_pobSharedAttributes->m_aobPfxOnSpawnStatic.begin(); 
		obIt != m_pobSharedAttributes->m_aobPfxOnSpawnStatic.end(); obIt++ )
	{
		iID = FXHelper::Pfx_CreateStatic(*obIt, this, "ROOT");
		m_aiPfxID.push_back(iID);
	}

	//Play world sound.
	ProjectileFireAudio();
}

//--------------------------------------------------
//!
//!	Object_Projectile::CrossbowOnSpawn
//! Spawn function for the crossbow bolt
//!
//--------------------------------------------------
void Object_Projectile::CrossbowOnSpawn( void )
{
	ntAssert( m_pobSharedAttributes );	

	uint iID;
	for ( PFXIter obIt = m_pobSharedAttributes->m_aobPfxOnSpawnAttached.begin(); 
		obIt != m_pobSharedAttributes->m_aobPfxOnSpawnAttached.end(); obIt++ )
	{
		iID = FXHelper::Pfx_CreateAttached(*obIt, CHashedString(this->GetName().c_str()), "ROOT");
		m_aiPfxID.push_back(iID);
	}

	for ( PFXIter obIt = m_pobSharedAttributes->m_aobPfxOnSpawnStatic.begin(); 
		obIt != m_pobSharedAttributes->m_aobPfxOnSpawnStatic.end(); obIt++ )
	{
		iID = FXHelper::Pfx_CreateStatic(*obIt, this, "ROOT");
		m_aiPfxID.push_back(iID);
	}

	//Play world sound.
	// AudioHelper::PlaySound("misc_sb", "crossbowfire", this);
	ProjectileFireAudio();

	//Queue up a destroy message to kill this projectile off after 25 seconds if it's still going (shot into the sky perhaps?).
	Message msgOnDestroy(msg_think_ondestroyprojectile);
	GetMessageHandler()->QueueMessageDelayed(msgOnDestroy, 25.0f);
}


//--------------------------------------------------
//!
//!	Object_Projectile::CrossbowIgniteEffects
//! Sets the on fire effects for the crossbow bolt
//!
//--------------------------------------------------
void Object_Projectile::CrossbowIgniteEffects( void )
{
	// Disable normal trail
	FXHelper::Pfx_Destroy(m_CrossbowPfxIDs.m_PfxTrailID, false);
	m_CrossbowPfxIDs.m_PfxTrailID = 0;

	// Spawn on fire effects
	m_CrossbowPfxIDs.m_PfxOnFireFlameID = FXHelper::Pfx_CreateAttached("PS_Crossbow_BoltTrail_Fire_PSystemSimpleDef", GetName().c_str(), "ROOT");
	m_CrossbowPfxIDs.m_PfxOnFireSmokeID = FXHelper::Pfx_CreateAttached("PS_Crossbow_BoltTrail_Smoke_PSystemSimpleDef", GetName().c_str(), "ROOT");
}

void Object_Projectile::KingLightningBallOnSpawn( bool bLarge )
{
	//Spawn different effects depending on if this is a heavy lightning ball or not.
	if(!bLarge)
	{
		m_KingLightningBallPfxIDs.m_PfxTrailID = FXHelper::Pfx_CreateAttached("KingLightningBall", GetName().c_str(), "ROOT");
	}
	else
	{
		m_KingLightningBallPfxIDs.m_PfxTrailID = FXHelper::Pfx_CreateAttached("KingLightningBallLarge", GetName().c_str(), "ROOT");
		m_KingLightningBallPfxIDs.m_PfxEmbersID = FXHelper::Pfx_CreateAttached("KingTrailParticleEmbers", GetName().c_str(), "ROOT");
	}

	//For now we want to hide the visible clump so the effect is all we see.
	Hide();
}


void Object_Projectile::KingWingAttackCrowOnSpawn( void )
{
	//Spawn the main cloudly trail effect for the crow.
	m_KingCrowPfxIDs.m_PfxTrailID = FXHelper::Pfx_CreateAttached("KingCrowTrail", GetName().c_str(), "ROOT");
}


//--------------------------------------------------
//!
//!	Object_Projectile::CannonBallOnCountered
//! Function for when the cannonball strike gets countered by who it's hit
//! Empty at the moment, only did this for AGenSword, but added these for completeness
//!
//--------------------------------------------------
void Object_Projectile::CannonBallOnCountered( CEntity* pobCounterer )
{
	UNUSED( pobCounterer );
}

//--------------------------------------------------
//!
//!	Object_Projectile::BazookaOnCountered
//! Function for when the bazooka strike gets countered by who it's hit
//! Empty at the moment, only did this for AGenSword, but added these for completeness
//!
//--------------------------------------------------
void Object_Projectile::BazookaOnCountered( CEntity* pobCounterer )
{
	UNUSED( pobCounterer );
}

//--------------------------------------------------
//!
//!	Object_Projectile::CrossbowOnCountered
//! Function for when the crossbow bolt strike gets countered by who it's hit
//! Empty at the moment, only did this for AGenSword, but added these for completeness
//!
//--------------------------------------------------
void Object_Projectile::CrossbowOnCountered( CEntity* pobCounterer )
{
	UNUSED( pobCounterer );
}

//--------------------------------------------------
//!
//!	Object_Projectile::AGenSwordOnCountered
//! Function for when the crossbow bolt strike gets countered by who it's hit
//! Empty at the moment, only did this for AGenSword, but added these for completeness
//!
//--------------------------------------------------
void Object_Projectile::AGenSwordOnCountered( CEntity* pobCounterer )
{
	ntError( m_ProjectileData.pAttacker->IsBoss() );
	if ( m_ProjectileData.pAttacker->IsBoss() )
	{
		AGenSwordBringBackStraight();
		AerialGeneral* pobAGen = (AerialGeneral*)m_ProjectileData.pAttacker;
		pobAGen->NotifyBoomerangSwordCountered(pobCounterer);
	}
}


void Object_Projectile::KingLightningBallOnCountered( CEntity* pobCounterer )
{
	ntError_p( m_ProjectileData.pAttacker->IsBoss(), ("This projectile should only have come from the king!") );
//	ntPrintf("KingLightningBallOnCountered() called\n");
	if( m_ProjectileData.pAttacker->IsBoss() )
	{
		//Remove collision between this projectile and the counterer (so that they don't re-collide after the counter).
		if(GetInteractionComponent())
		{
			GetInteractionComponent()->ExcludeCollisionWith(pobCounterer);
		}

		KingLightningBallBringBack();
	}
}


void Object_Projectile::KingLightningBallOnMissedCounter( CEntity* pobCounterer )
{
//	ntPrintf("KingLightningBallOnMissedCounter... time to explode!\n");
	//If this was a counterable projectile (which it should have been to get to this point), then the initial attack data
	//was probably set to put the player into RT_DEFLECT. For those ones, then, we use the Area-Of-Effect attack data to
	//generate the "failed to counter" strike (which can be used to just KO the player).
	if(m_bIsCounterable)
	{
		if(m_ProjectileData.ProjectileAttackDataAOE != "")
		{
			CombatHelper::Combat_GenerateStrike(this, m_ProjectileData.pAttacker, pobCounterer, m_ProjectileData.ProjectileAttackDataAOE);
		}
	}

	//Now destroy the lightning ball.
	KingLightningBallDestroy();
}

void Object_Projectile::AGenSwordOnMissedCounter( CEntity* pobCounterer )
{
	if (m_bAGenSwordBringBackOnImpact)
		AGenSwordBringBack();
}


//--------------------------------------------------
//!
//!	Object_Projectile::CrossbowOnImpact
//! Impact function for the crossbow bolt
//!
//--------------------------------------------------
void Object_Projectile::CrossbowOnImpact(const Message& msg)
{
	CEntity* pCollidee = msg.GetEnt("Collidee"); 
	CPoint obCollisionPoint( msg.GetFloat("pX"), msg.GetFloat("pY"), msg.GetFloat("pZ") );
	int hitArea = msg.GetInt("RagdollMaterial");	

        // Spawn some blood if the entity was an enemy
	if ( pCollidee && pCollidee->IsCharacter() && !pCollidee->ToCharacter()->IsDead() )
	{
		// check head shots
		if (pCollidee->GetAttackComponent() && ( pCollidee->GetAttackComponent()->GetAttackDefinition()->m_bOnlyHeadShots || pCollidee->GetAttackComponent()->GetCanHeadshotThisEntity() ) )
		{
			if (hitArea != Physics::RAGDOLL_HEAD_MATERIAL)
			{
				// just destroy the arrow
				EXTERNALLY_SET_STATE(PROJECTILE_FSM, DESTROY);
				return;
			}		
			else
			{
				CMessageSender::SendEmptyMessage( CHashedString("msg_headshot"), pCollidee->GetMessageHandler() );
			}
		}

		CDirection obDirection = GetMatrix().GetZAxis();
		obDirection.Normalise();
		obDirection *= 0.25f;

		CMatrix obPosMatrix;
		obPosMatrix.SetIdentity();
		obPosMatrix.SetTranslation( GetPosition() - obDirection );

		void* pBloodSprayDef = ObjectDatabase::Get().GetPointerFromName<void*>(CHashedString("Blood_00_Spray_NoOffset"));
		void* pBloodMistDef = ObjectDatabase::Get().GetPointerFromName<void*>(CHashedString("Blood_00_Mist_NoOffset"));

		if ( pBloodSprayDef )
		{
			PSystemUtils::ConstructParticleEffect( pBloodSprayDef, obPosMatrix );
		}

		if ( pBloodMistDef )
		{
			PSystemUtils::ConstructParticleEffect( pBloodMistDef, obPosMatrix );
		}
	}

	// Kill particle effects
	for ( UIntIter obIt = m_aiPfxID.begin(); obIt != m_aiPfxID.end(); obIt++ )
	{
		FXHelper::Pfx_Destroy( *obIt, false );
	}
	m_aiPfxID.clear();

	// Do the impact camera
	CoolCamera* pCam = CamMan::GetPrimaryView()->GetCoolCam(m_iCameraHandle);
	ntAssert(!pCam || pCam->IsType(CT_AFTERTOUCH));
	if( pCollidee && pCam && pCam->IsType(CT_AFTERTOUCH) )
	{
		((CoolCam_AfterTouch*)pCam)->LookAt(pCollidee, (pCollidee&&pCollidee->IsEnemy()) ? CoolCam_AfterTouch::HT_ENEMY : CoolCam_AfterTouch::HT_NORMAL);
	}

	// Play a sound.	
	// Notify collision effect handler
	uint32_t matID = msg.GetInt("PhysicsMaterial");
	Physics::psPhysicsMaterial * physMat = Physics::PhysicsMaterialTable::Get().GetMaterialFromId(matID);

	// Projectile can now be reparented (penetrate into target) OR bounce off... 
	// check the penetrability to decide this... 
	ProjectileProperties* pobProperties = ObjectDatabase::Get().GetPointerFromName<ProjectileProperties*>(m_ProjectileData.ProjectileProperties);
	bool penetrate = physMat && pobProperties ? pobProperties->m_fPenetrability > (1.0f - physMat->GetPenetrability()) : true;			

	if (GetPhysicsSystem()->GetCollisionEffectHandler())
	{
		CollisionEffectHandler::CollisionEffectHandlerEvent obEvent;
		obEvent.m_eType = msg.GetBool("HasRicocheted") || !penetrate ? CollisionEffectManager::BOUNCE : CollisionEffectManager::CRASH;
		obEvent.m_fRelevantVelocity = -msg.GetFloat("ProjVel");
		obEvent.m_obCollisionPosition = obCollisionPoint;
		obEvent.m_pobOther = pCollidee ? pCollidee->GetPhysicsSystem()->GetCollisionEffectHandler() : NULL;

		if (physMat)
			obEvent.m_uiCustomMaterial2 = physMat->GetEffectMaterialBit();

		GetPhysicsSystem()->GetCollisionEffectHandler()->ProcessEvent(obEvent);
	}

	//Generate an AI sound that allows AIs to investigate (Dario)
	CAIHearingMan::Get().TriggerSound((CEntity*)this, m_ProjectileData.SoundRadius, m_ProjectileData.SoundVolume, m_ProjectileData.IsVolumeConstant);

	// Inform AIs in the vecinity if an AI was hit by KAI
	if( pCollidee )
	{
		CAINavigationSystemMan::Get().TriggerKaiHitAI(pCollidee);
	}
	
	// Make this bolt uncapable of triggering any more diving
	CAINavigationSystemMan::Get().RemoveBoltFromDivingList(this);

	//Parent the bolt to whatever it hits.
	ntAssert((pCollidee != 0) && "Collided with nothing (Collidee parameter on message is NULL)");
	
	// Reparent the projectile - if the function returns false, then the projectile couldn't be reparented
	if (!msg.GetBool("HasRicocheted"))
	{
		// Reparent the projectile - if the function returns false, then the projectile couldn't be reparented
		if (pCollidee && penetrate)
			ReparentProjectile( pCollidee, obCollisionPoint );
		else
		{
			// bounce off projectile
			Physics::ProjectileLG *projectileLG = (Physics::ProjectileLG *) GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::PROJECTILE_LG);

			// arrow is now inside the hit object. We need it out of penetration, because
			// we want to switch to rigid body representation. Take it out -> move it back. 
			Transform * trans = GetHierarchy()->GetRootTransform();		
			CMatrix mat = trans->GetWorldMatrix();
			CDirection dir = projectileLG->GetLinearVelocity();
			dir.Normalise(); 
			mat.SetTranslation(mat.GetTranslation() - dir * 0.25f); // take it back by 20cm... this value can be also 
			                                                        // exatly calculated from shape, but this is 
																	// sufficient for a moment
			trans->SetLocalMatrixFromWorldMatrix(mat);

			// start rigid body stop projectile movement
			Physics::SingleRigidLG *rigidBodyLG = (Physics::SingleRigidLG *) GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::SINGLE_RIGID_BODY_LG);
			rigidBodyLG->Activate(true);
			rigidBodyLG->SetLinearVelocity(projectileLG->GetLinearVelocity()); 

			projectileLG->Deactivate(); 

			// Remove any controllers still on the arrow
			if( GetMovement() )
			{
				GetMovement()->ClearControllers();
			}

			return;
		}

		//Send a projectile collision message and/or combat strike to the collidee.
		if (pCollidee && !ntStr::IsNull(m_ProjectileData.ProjectileAttackData))
		{		
			if(pCollidee->GetMessageHandler())
			{
				Message msgProjCol(msg_projcol);
				msgProjCol.SetString("Type", "bolt");	//TODO: This should come from the table set-up in CreateCrossbowBolt.
				msgProjCol.SetEnt("Projectile", this);

				pCollidee->GetMessageHandler()->QueueMessage(msgProjCol);
			}

			//		Send( {Msg="msg_projcol", Type=this.attrib.ProjectileType, Projectile=this}, message.Collidee )
			// choose correct attack data according to ragdoll material...   
			bool hitFromFront = GetMatrix().GetZAxis().Dot(pCollidee->GetMatrix().GetZAxis()) < 0;

			bool bFullyCharged = (GetCharge() >= 1.0f);		
			CHashedString obDefaultAttackData = bFullyCharged && !m_ProjectileData.ProjectileAttackDataFullyCharged.IsNull() ? m_ProjectileData.ProjectileAttackDataFullyCharged : m_ProjectileData.ProjectileAttackData;

			CHashedString hitAreasDataName = bFullyCharged ? ( 
				hitFromFront ? m_ProjectileData.ProjectilePoweredFrontHitAreas : 
			m_ProjectileData.ProjectilePoweredBackHitAreas) :
			(hitFromFront ? m_ProjectileData.ProjectileNonPoweredFrontHitAreas : 
			m_ProjectileData.ProjectileNonPoweredBackHitAreas);						

			CHashedString attackData;
			if ( ntStr::IsNull(hitAreasDataName) )
			{
				attackData = obDefaultAttackData;
			}
			else
			{
				HitAreas_Attributes * pobHitAreas = ObjectDatabase::Get().GetPointerFromName< HitAreas_Attributes* >( hitAreasDataName );

				switch (hitArea)
				{
				case Physics::RAGDOLL_HEAD_MATERIAL : 
					{
						attackData = pobHitAreas->m_Head_AttackData;
						break;
					}
				case Physics::RAGDOLL_SPINE_00_MATERIAL :
					{
						attackData = pobHitAreas->m_Spine_AttackData;
						break;
					}
				case Physics::RAGDOLL_PELVIS_MATERIAL :
					{
						attackData = pobHitAreas->m_Pelvis_AttackData;
						break;
					}
				case Physics::RAGDOLL_L_ARM_MATERIAL :
					{
						attackData = pobHitAreas->m_L_UpperArm_AttackData;
						break;
					}
				case Physics::RAGDOLL_L_ELBOW_MATERIAL :
					{
						attackData = pobHitAreas->m_L_ForeArm_AttackData;
						break;
					}
				case Physics::RAGDOLL_R_ARM_MATERIAL :
					{
						attackData = pobHitAreas->m_R_UpperArm_AttackData;
						break;
					}
				case Physics::RAGDOLL_R_ELBOW_MATERIAL :
					{
						attackData = pobHitAreas->m_R_ForeArm_AttackData;
						break;
					}
				case Physics::RAGDOLL_L_LEG_MATERIAL :
					{
						attackData = pobHitAreas->m_L_Thigh_AttackData;
						break;
					}
				case Physics::RAGDOLL_L_KNEE_MATERIAL :
					{
						attackData = pobHitAreas->m_L_Calf_AttackData;
						break;
					}
				case Physics::RAGDOLL_R_LEG_MATERIAL :
					{
						attackData = pobHitAreas->m_R_Thigh_AttackData;
						break;
					}
				case Physics::RAGDOLL_R_KNEE_MATERIAL :
					{
						attackData = pobHitAreas->m_R_Calf_AttackData;
						break;
					}
				default:
					attackData = obDefaultAttackData;
				}
			}

			if (ntStr::IsNull(attackData))
				attackData = obDefaultAttackData;

			CombatHelper::Combat_GenerateStrike(this, m_ProjectileData.pAttacker, pCollidee, attackData, hitArea);

		}

		// Remove any controllers still on the arrow
		if( GetMovement() )
		{
			GetMovement()->ClearControllers();
		}

		//Should the bolt be destroyed when hitting a target (only applies for crossbow bolts at the moment)
		if( !penetrate || m_ProjectileData.bDestroyOnImpact )
		{
			EXTERNALLY_SET_STATE(PROJECTILE_FSM, DESTROY);
		}
		else
		{
			//Otherwise, go inactive instead.
			EXTERNALLY_SET_STATE(PROJECTILE_FSM, INACTIVE);
		}
	}
}
//--------------------------------------------------
//!
//!	Object_Projectile::AGenSwordOnImpact
//!
//--------------------------------------------------
void Object_Projectile::AGenSwordOnImpact( CEntity* pCollidee, const CPoint& obCollisionPoint )
{
	// Kill particle effects
	for ( UIntIter obIt = m_aiPfxID.begin(); obIt != m_aiPfxID.end(); obIt++ )
	{
		FXHelper::Pfx_Destroy( *obIt, false );
	}
	m_aiPfxID.clear();

	FXHelper::Pfx_Destroy(m_AGenSwordPfxIDs.m_PfxTrailID, false);
	m_AGenSwordPfxIDs.m_PfxTrailID = 0;

	// impact particle effects
	uint iID;
	for ( PFXIter obIt = m_pobSharedAttributes->m_aobPfxOnImpactAttached.begin(); 
		obIt != m_pobSharedAttributes->m_aobPfxOnImpactAttached.end(); obIt++ )
	{
		iID = FXHelper::Pfx_CreateAttached(*obIt, CHashedString(this->GetName().c_str()), "ROOT");
		m_aiPfxID.push_back(iID);
	}

	for ( PFXIter obIt = m_pobSharedAttributes->m_aobPfxOnImpactStatic.begin(); 
		obIt != m_pobSharedAttributes->m_aobPfxOnImpactStatic.end(); obIt++ )
	{
		iID = FXHelper::Pfx_CreateStatic(*obIt, this, "ROOT");
		m_aiPfxID.push_back(iID);
	}

	//Parent the bolt to whatever it hits.
	ntAssert((pCollidee != 0) && "Collided with nothing (Collidee parameter on message is NULL)");
	
	// Reparent the projectile - if the function returns false, then the projectile couldn't be reparented
	if (pCollidee->IsStatic() && m_bStickIntoStaticCollision)
	{
		ReparentProjectile( pCollidee, obCollisionPoint );
		EXTERNALLY_SET_STATE(PROJECTILE_FSM, INACTIVE);
	}
	else
	{
		// Reverse it's direction
		if (m_bAGenSwordBringBackOnImpact)
		{
		m_ProjectileData.vDirection *= -1;
		Physics::ProjectileLG* pobLG = (Physics::ProjectileLG*)GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::PROJECTILE_LG);
		pobLG->SetThrustDirection(m_ProjectileData.vDirection);
	}
	}

	if( !m_bGenerateNoStrikes && m_ProjectileData.ProjectileAttackData != "" && m_ProjectileData.pAttacker != pCollidee)
	{
		if (m_bAGenSwordBringBackOnImpact)
			//AGenSwordBringBack();
			AGenSwordFreeze();
		CombatHelper::Combat_GenerateStrike(this, m_ProjectileData.pAttacker, pCollidee, m_ProjectileData.ProjectileAttackData);
		// Do only one strike
		m_bGenerateNoStrikes = true;
	}
}

void Object_Projectile::AGenSwordBringBack()
{
	// Takes care of straightness
	m_ProjectileData.pTarget = m_ProjectileData.pOriginator;
	CPoint obSwordOffsetAGenPosition( m_ProjectileData.pTarget->GetPosition() );
	obSwordOffsetAGenPosition.Y() += 1.0f;
	m_ProjectileData.vDirection = CDirection( obSwordOffsetAGenPosition - GetPosition() );
	m_ProjectileData.vDirection.Normalise();
	Physics::ProjectileLG* pobLG = (Physics::ProjectileLG*)GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::PROJECTILE_LG);
	float fMagnitude = pobLG->GetLinearVelocity().Length();
	pobLG->SetThrustDirection(m_ProjectileData.vDirection);
	pobLG->SetLinearVelocity(m_ProjectileData.vDirection * fMagnitude);
	// Takes care of lerpness
	pobLG->SetFrozen(false);
	pobLG->Reset(&m_ProjectileData);
	if (!pobLG->IsMoving())
		pobLG->SetMoving(true);
}

void Object_Projectile::AGenSwordBringBackStraight()
{
	// Takes care of straightness
	m_ProjectileData.pTarget = m_ProjectileData.pOriginator;
	m_ProjectileData.vDirection = CDirection( m_ProjectileData.pTarget->GetPosition() - GetPosition() );
	m_ProjectileData.vDirection.Normalise();
	Physics::ProjectileLG* pobLG = (Physics::ProjectileLG*)GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::PROJECTILE_LG);
	float fMagnitude = pobLG->GetLinearVelocity().Length();
	pobLG->SetThrustDirection(m_ProjectileData.vDirection);
	pobLG->SetLinearVelocity(m_ProjectileData.vDirection * fMagnitude);
	// Takes care of lerpness
	pobLG->SetStraightVectorLerp(true);
	pobLG->SetFrozen(false);
	pobLG->Reset(&m_ProjectileData);
	if (!pobLG->IsMoving())
		pobLG->SetMoving(true);
}

void Object_Projectile::AGenSwordFreeze()
{
	Physics::ProjectileLG* pobLG = (Physics::ProjectileLG*)GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::PROJECTILE_LG);
	pobLG->SetFrozen(true);
}

bool Object_Projectile::AGenSwordIsFrozen()
{
	Physics::ProjectileLG* pobLG = (Physics::ProjectileLG*)GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::PROJECTILE_LG);
	return pobLG->GetFrozen();
}

bool Object_Projectile::AGenSwordHasMissedTarget()
{
	Physics::ProjectileLG* pobLG = (Physics::ProjectileLG*)GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::PROJECTILE_LG);
	return pobLG->HasMissedTarget(90.0f);
}


//--------------------------------------------------
//!
//!	Object_Projectile::KingLightningBallOnImpact
//!
//--------------------------------------------------
void Object_Projectile::KingLightningBallOnImpact( CEntity* pCollidee, const CPoint& obCollisionPoint )
{
	//Parent the bolt to whatever it hits.
	ntAssert((pCollidee != 0) && "Collided with nothing (Collidee parameter on message is NULL)");

	//If this is hitting the player, and the projectile is counterable, then generate a strike and allow them the chance to
	//counter. If they fail to counter then we allow the message (from the attack component) to handle destroying the projectile
	//and creating a KO strike for the player.
	if(pCollidee->IsPlayer())
	{
		CombatHelper::Combat_GenerateStrike(this, m_ProjectileData.pAttacker, pCollidee, m_ProjectileData.ProjectileAttackData);
		//Stop the projectile on-impact so that it doesn't pass through the player and collide with the floor during the counter-window.
		Physics::ProjectileLG* pobLG = (Physics::ProjectileLG*)GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::PROJECTILE_LG);
		if(pobLG->IsMoving())
			pobLG->SetMoving(false);
		pobLG->SetLinearVelocity(CDirection(0.0f, 0.0f, 0.0f));

		if(!m_bIsCounterable)	//LightningAttack2 projectile hits player
		{
			//If this is an uncounterable projectile, then destruct now (no chance to counter).
			EXTERNALLY_SET_STATE(PROJECTILE_FSM, DESTROY);
		}
	}
	//If it's hitting something other then the player (likely the floor) then strike the player with ProjectileAttackDataAOE if within radius.
	else
	{
//		ntPrintf("King lightning ball collided with an object that wasn't the player... or this lightning ball wasn't counterable, exploding...\n");
		//If it's not the player, check if the player is within a certain range and if so, generate a strike. Also destruct either way!
		if( m_ProjectileData.pAttacker != pCollidee )	//Don't do this upon hitting the king, as it fizzles out instead.
		{
			CEntity* pobPlayer = CEntityManager::Get().GetPlayer();
			ntError_p(pobPlayer, ("KingLightningBallOnImpact: Failed to retrieve player pointer from entity manager"));
			if(pobPlayer)
			{
				//If the player is within a certain radius of this point, we want to generate an area-of-effect strike from the projectile.
				CPoint obProjectilePosition = GetPosition();
				CPoint obPlayerPosition = pobPlayer->GetPosition();
				CDirection obBetween = CDirection(obProjectilePosition - obPlayerPosition);
				float fDistSquared = obBetween.LengthSquared();
				float fRadiusSquared = m_ProjectileData.AreaOfEffectRadius * m_ProjectileData.AreaOfEffectRadius;
				if(fDistSquared <= fRadiusSquared)
				{
					if(!ntStr::IsNull(m_ProjectileData.ProjectileAttackDataAOE))
					{
						CombatHelper::Combat_GenerateStrike(this, m_ProjectileData.pAttacker, pobPlayer, m_ProjectileData.ProjectileAttackDataAOE);
					}
				}
			}
			EXTERNALLY_SET_STATE(PROJECTILE_FSM, DESTROY);
		}
	}
}


void Object_Projectile::KingLightningBallBringBack()
{
	m_ProjectileData.pTarget = m_ProjectileData.pOriginator;
	m_ProjectileData.vDirection = CDirection( m_ProjectileData.pOriginator->GetPosition() - GetPosition() );
	m_ProjectileData.vDirection.Normalise();
	Physics::ProjectileLG* pobLG = (Physics::ProjectileLG*)GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::PROJECTILE_LG);
	if (!pobLG->IsMoving())
	{
		pobLG->SetMoving(true);
	}
	//We send the lightning back to the king at a different speed (slower!)
//	pobLG->SetLinearVelocity(pobLG->GetLinearVelocity().Length() * m_ProjectileData.vDirection);
	pobLG->SetLinearVelocity(25.0f * m_ProjectileData.vDirection);	//Send it back at a speed of 25.

//	float fImpactVelocity = pobLG->GetLinearVelocity().Length();
//	ntPrintf("KingLightningBallBringBack() called... impact at speed %f ... should now have redirected the lightning ball at the king at speed 25.0f\n",
//		fImpactVelocity);

	//Reset the tracking component so that it now tracks the king.
	GetPhysicsSystem()->Lua_Projectile_EnableEntityTracking(m_ProjectileData.pTarget, 0.0f , 1.0f, 0.0f, m_ProjectileData.fTrackingSpeed);
}



//--------------------------------------------------
//!
//!	Object_Projectile::KingWingAttackCrowOnImpact
//!
//--------------------------------------------------
void Object_Projectile::KingWingAttackCrowOnImpact( CEntity* pCollidee, const CPoint& obCollisionPoint )
{
	ntAssert((pCollidee != 0) && "Collided with nothing (Collidee parameter on message is NULL)");

	//Just send a strike if it's the player. If it hits anything else we kill the crow projectile off.

	//If this is hitting the player then generate a strike.
	if(pCollidee->IsPlayer())
	{
		//If it's the player, generate a strike.
		CombatHelper::Combat_GenerateStrike(this, m_ProjectileData.pAttacker, pCollidee, m_ProjectileData.ProjectileAttackData);
		//Also keep it moving.
		Physics::ProjectileLG* pobLG = (Physics::ProjectileLG*)GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::PROJECTILE_LG);
		if(!pobLG->IsMoving())
		{
			pobLG->SetMoving(true);
		}
		pobLG->SetLinearVelocity(pobLG->GetLinearVelocity().Length() * m_ProjectileData.vDirection);

		//Play impact sound
		ProjectileImpactAudio(this);

		//Set to destroyed either way.
		EXTERNALLY_SET_STATE(PROJECTILE_FSM, DESTROY);
	}

	if(pCollidee->IsProjectile())
	{
		//If it's a projectile, keep it moving (so it doesn't hang mid-air if two collide!)
		Physics::ProjectileLG* pobLG = (Physics::ProjectileLG*)GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::PROJECTILE_LG);
		if(!pobLG->IsMoving())
		{
			pobLG->SetMoving(true);
		}
		pobLG->SetLinearVelocity(pobLG->GetLinearVelocity().Length() * m_ProjectileData.vDirection);
	}
	else
	{
		//For everything else, destroy it immediately.
		EXTERNALLY_SET_STATE(PROJECTILE_FSM, DESTROY);
	}
}



//------------------------------------------------------------------------------------------
//!  public  CrossbowOnStrike
//!
//!  @param [in, out]  pStriker CEntity *    
//!
//!
//!  @remarks <TODO: insert remarks here>
//!
//!  @author GavB @date 31/08/2006
//------------------------------------------------------------------------------------------
void Object_Projectile::CrossbowOnStrike( CEntity* pStriker )
{
	// What to do on being struck. We could apply an impulse force to the projectile and 
	// send it off flying in another direction...

	if( GetPhysicsSystem() )
	{
		Physics::ProjectileLG* lg = (Physics::ProjectileLG*) GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::PROJECTILE_LG);
		ntAssert(lg);

		// Create a cheap directional heading for the projectile
		CDirection dirImpluseForce = (GetPosition() ^ pStriker->GetPosition());

		// Normalise the direction
		dirImpluseForce.Normalise();

		// Set the magnitude of the force
		dirImpluseForce *= lg->GetLinearVelocity().Length() * lg->GetMass() * 2.0f;

		// apply the force.
		lg->ApplyLocalisedLinearImpulse(dirImpluseForce, CVector(GetPosition()));
	}

	// As the striker entity has effectively taken ownership of the projectile then
	// allow the projectile to collide with the entity that fired it. As the host entity
	// originally excluded itself from the damage the projectile would do.
	// 
	GetInteractionComponent()->AllowCollisionWith( this );

}


//--------------------------------------------------
//!
//!	Object_Projectile::CrossbowOnDestroy
//! Destroy function for the crossbow bolt
//!
//--------------------------------------------------
void Object_Projectile::CrossbowOnDestroy( void )
{
	// Kill particle effects
	for ( UIntIter obIt = m_aiPfxID.begin(); obIt != m_aiPfxID.end(); obIt++ )
	{
		FXHelper::Pfx_Destroy( *obIt, false );
	}
	m_aiPfxID.clear();
}

//--------------------------------------------------
//!
//!	Object_Projectile::AGenSwordOnDestroy
//!
//--------------------------------------------------
void Object_Projectile::AGenSwordOnDestroy( void )
{
	// Kill particle effects
	for ( UIntIter obIt = m_aiPfxID.begin(); obIt != m_aiPfxID.end(); obIt++ )
	{
		FXHelper::Pfx_Destroy( *obIt, false );
	}
	m_aiPfxID.clear();

	uint iID;
	for ( PFXIter obIt = m_pobSharedAttributes->m_aobPfxOnDestroyStatic.begin(); 
		obIt != m_pobSharedAttributes->m_aobPfxOnDestroyStatic.end(); obIt++ )
	{
		iID = FXHelper::Pfx_CreateStatic(*obIt, this, "ROOT");
		m_aiPfxID.push_back(iID);
	}

	FXHelper::Pfx_Destroy(m_AGenSwordPfxIDs.m_PfxTrailID, false);
	m_AGenSwordPfxIDs.m_PfxTrailID = 0;
}

void Object_Projectile::KingLightningBallOnDestroy( void )
{
//	ntPrintf("KingLightningBallOnDestroy... KABOOM\n");
	//Kill off our trail effects.
	FXHelper::Pfx_Destroy(m_KingLightningBallPfxIDs.m_PfxTrailID, false);
	m_KingLightningBallPfxIDs.m_PfxTrailID = 0;
	FXHelper::Pfx_Destroy(m_KingLightningBallPfxIDs.m_PfxEmbersID, false);
	m_KingLightningBallPfxIDs.m_PfxEmbersID = 0;

	//We want the effect to play on-destruction by default, but we can flag this off for if the bolt hits the king
	//as we'll be playing a different effect then.
	if(m_bEffectOnDestroy)
	{
		FXHelper::Pfx_CreateStatic("Initial_boom", this, "ROOT");
	}

	//Play impact sound
	ProjectileImpactAudio(this);
}

void Object_Projectile::KingWingAttackCrowOnDestroy( void )
{
	//Kill off our trail effects.
	FXHelper::Pfx_Destroy(m_KingCrowPfxIDs.m_PfxTrailID, false);
	m_KingCrowPfxIDs.m_PfxTrailID = 0;

	//Create a non-attached static destroy effect.
	FXHelper::Pfx_CreateStatic("KingCrowDestroy", this, "ROOT");
}

//--------------------------------------------------
//!
//!	Object_Projectile::CannonBallOnSpawn
//! Spawn function for the crossbow bolt
//!
//--------------------------------------------------
void Object_Projectile::CannonBallOnSpawn( void )
{
	ntAssert( m_pobSharedAttributes );	

	uint iID;
	for ( PFXIter obIt = m_pobSharedAttributes->m_aobPfxOnSpawnAttached.begin(); 
		obIt != m_pobSharedAttributes->m_aobPfxOnSpawnAttached.end(); obIt++ )
	{
		iID = FXHelper::Pfx_CreateAttached(*obIt, CHashedString(this->GetName().c_str()), "ROOT");
		m_aiPfxID.push_back(iID);
	}

	for ( PFXIter obIt = m_pobSharedAttributes->m_aobPfxOnSpawnStatic.begin(); 
		obIt != m_pobSharedAttributes->m_aobPfxOnSpawnStatic.end(); obIt++ )
	{
		iID = FXHelper::Pfx_CreateStatic(*obIt, this, "ROOT");
		m_aiPfxID.push_back(iID);
	}

	ProjectileFireAudio();

	//Queue up a destroy message to kill this projectile off after 25 seconds if it's still going (shot into the sky perhaps?).
	Message msgOnDestroy(msg_think_ondestroyprojectile);
	GetMessageHandler()->QueueMessageDelayed(msgOnDestroy, 25.0f);
}


//--------------------------------------------------
//!
//!	Object_Projectile::CannonBallOnImpact
//! Impact function for the crossbow bolt
//!
//--------------------------------------------------
void Object_Projectile::CannonBallOnImpact( CEntity* pCollidee )
{
	// Kill particle effects
	for ( UIntIter obIt = m_aiPfxID.begin(); obIt != m_aiPfxID.end(); obIt++ )
	{
		FXHelper::Pfx_Destroy( *obIt, false );
	}
	m_aiPfxID.clear();

	ntAssert( m_pobSharedAttributes );	

	// impact particle effects
	uint iID;
	for ( PFXIter obIt = m_pobSharedAttributes->m_aobPfxOnImpactAttached.begin(); 
		obIt != m_pobSharedAttributes->m_aobPfxOnImpactAttached.end(); obIt++ )
	{
		iID = FXHelper::Pfx_CreateAttached(*obIt, CHashedString(this->GetName().c_str()), "ROOT");
		m_aiPfxID.push_back(iID);
	}

	for ( PFXIter obIt = m_pobSharedAttributes->m_aobPfxOnImpactStatic.begin(); 
		obIt != m_pobSharedAttributes->m_aobPfxOnImpactStatic.end(); obIt++ )
	{
		iID = FXHelper::Pfx_CreateStatic(*obIt, this, "ROOT");
		m_aiPfxID.push_back(iID);
	}

	//Parent the bolt to whatever it hits.
	ntAssert((pCollidee != 0) && "Collided with nothing (Collidee parameter on message is NULL)");
	
	// If the projectile has hit some background, then don't stick - bounce!
	//if( pCollidee->GetEntType() == CEntity::EntType_Static )
	//	return;

	//Send a projectile collision message and/or combat strike to the collidee.
	if( m_ProjectileData.ProjectileAttackData != "")
	{
		// Send a message to the collidee about the collision.
		if(pCollidee->GetMessageHandler())
		{
			Message msgProjCol(msg_projcol);
			msgProjCol.SetString("Type", "bolt");
			msgProjCol.SetEnt("Projectile", this);

			pCollidee->GetMessageHandler()->QueueMessage(msgProjCol);
		}

		// Apply a damage to the entity
		CombatHelper::Combat_GenerateStrike(this, m_ProjectileData.pAttacker, pCollidee, m_ProjectileData.ProjectileAttackData);
	}

	
		EXTERNALLY_SET_STATE(PROJECTILE_FSM, DESTROY);
	}


//--------------------------------------------------
//!
//!	Object_Projectile::CannonBallOnDestroy
//! Destroy function for the crossbow bolt
//!
//--------------------------------------------------
void Object_Projectile::CannonBallOnDestroy( void )
{
	// Kill particle effects
	for ( UIntIter obIt = m_aiPfxID.begin(); obIt != m_aiPfxID.end(); obIt++ )
	{
		FXHelper::Pfx_Destroy( *obIt, false );
	}
	m_aiPfxID.clear();

	// Params copied from from the bazooka
	if ( m_pobPhysicsSystem->Lua_Projectile_GetStateTime() >= 0.5f)
	{

		ntAssert( m_pobSharedAttributes );	

		uint iID;
		for ( PFXIter obIt = m_pobSharedAttributes->m_aobPfxOnDestroyStatic.begin(); 
			obIt != m_pobSharedAttributes->m_aobPfxOnDestroyStatic.end(); obIt++ )
		{
			iID = FXHelper::Pfx_CreateStatic(*obIt, this, "ROOT");
			m_aiPfxID.push_back(iID);
		}

		Physics::CExplosionParams params;
		params.m_fPush = 400.0f;
		params.m_fPushDropoff = 10.0f;
		params.m_fRadius = 8.0f;
		params.m_pobOriginator = m_ProjectileData.pAttacker;
		params.m_obPosition = GetPosition();
		m_pobPhysicsSystem->Lua_AltExplosion( params );

		// Notify army manager of cannonball explosion - PS3 only!
		#ifdef PLATFORM_PS3
			ArmyManager::Get().ExplodeBazookaShot( GetPosition() );
		#endif

		// Camera shake
		CPoint ptPos = GetPosition();
		CamMan::Get().Shake(2.f, 1.f, 3.f, &ptPos, 50.f*50.f);
	}

	//DeactivateAllStates, allow havok to take control of the object again.
	GetPhysicsSystem()->Deactivate();	
}

//--------------------------------------------------
//!
//!	Object_Projectile::AGenSwordDestroy
//!
//--------------------------------------------------
void Object_Projectile::AGenSwordDestroy( void )
{
	EXTERNALLY_SET_STATE(PROJECTILE_FSM, DESTROY);
	Message msgOnRemoveFromWorld(msg_think_onremoveobject);
	GetMessageHandler()->QueueMessage(msgOnRemoveFromWorld);
}

void Object_Projectile::KingLightningBallDestroy( void )
{
//	ntPrintf("KingLightningBallDestroy()...\n");
	EXTERNALLY_SET_STATE(PROJECTILE_FSM, DESTROY);
	Message msgOnRemoveFromWorld(msg_think_onremoveobject);
	GetMessageHandler()->QueueMessage(msgOnRemoveFromWorld);
}

void Object_Projectile::KingWingAttackCrowDestroy( void )
{
	EXTERNALLY_SET_STATE(PROJECTILE_FSM, DESTROY);
	Message msgOnRemoveFromWorld(msg_think_onremoveobject);
	GetMessageHandler()->QueueMessage(msgOnRemoveFromWorld);
}


void Object_Projectile::KingWingAttackCrowHitRanged( void )
{
	EXTERNALLY_SET_STATE(PROJECTILE_FSM, DESTROY);
	Message msgOnRemoveFromWorld(msg_think_onremoveobject);
	GetMessageHandler()->QueueMessage(msgOnRemoveFromWorld);
}

//--------------------------------------------------
//!
//!	Object_Projectile::BazookaOnSpawn
//! Spawn function for the bazooka rocket
//!
//--------------------------------------------------
void Object_Projectile::BazookaOnSpawn( void )
{
	// Particle effects (not the dummy rocket)
	if ( m_iProjectileIndex != iNumBazookaRockets )
	{
		BazookaProjectileProperties* pobBazookaProperties = ObjectDatabase::Get().GetPointerFromName<BazookaProjectileProperties*>(m_ProjectileData.ProjectileProperties);
		ntAssert( pobBazookaProperties );
		
		// If this is a LOD bazooka then start with the normal trail and ignore the ignition phase
		if ( pobBazookaProperties->m_bUseLODEffects )
		{
			m_BazookaPfxIDs.m_PfxFlameID = FXHelper::Pfx_CreateAttached("Clan_Cannon_Flame_PSystemSimpleDef", CHashedString(this->GetName().c_str()), "ROOT");
			m_BazookaPfxIDs.m_PfxSmokeID = FXHelper::Pfx_CreateAttached("Clan_Cannon_Smoke_Trail_PSystemSimpleDef", CHashedString(this->GetName().c_str()), "ROOT");
		}
		else
		{
			// Normal, non-LOD effects, and queue ignition message
		m_BazookaPfxIDs.m_PfxIgnitionID = FXHelper::Pfx_CreateAttached("RocketFuse_Def", CHashedString(this->GetName().c_str()), "ROOT");
		m_BazookaPfxIDs.m_PfxSeederID = 0;
		m_BazookaPfxIDs.m_PfxFlameID = FXHelper::Pfx_CreateAttached("RocketSparkler_Def", CHashedString(this->GetName().c_str()), "ROOT");
		m_BazookaPfxIDs.m_PfxSmokeID = 0;

		// Queue ignition message
		Message msgIgnition(msg_think_bazookaignition);
		GetMessageHandler()->QueueMessageDelayed(msgIgnition, 0.5f);
	}
	}

	//Queue up a destroy message to kill this projectile off after 25 seconds if it's still going (shot into the sky perhaps?).
	Message msgOnDestroy(msg_think_ondestroyprojectile);
	GetMessageHandler()->QueueMessageDelayed(msgOnDestroy, 25.0f);
}


//--------------------------------------------------
//!
//!	Object_Projectile::BazookaIgnition
//! Ignition function for the bazooka rocket
//!
//--------------------------------------------------
void Object_Projectile::BazookaIgnition( void )
{
	// Particle effects
	m_BazookaPfxIDs.m_PfxIgnitionID = FXHelper::Pfx_CreateAttached("RocketIgnite_Def", CHashedString(this->GetName().c_str()), "ROOT");
	m_BazookaPfxIDs.m_PfxSeederID = FXHelper::Pfx_CreateAttached("RocketSparkSeeder_Def", CHashedString(this->GetName().c_str()), "ROOT");
	m_BazookaPfxIDs.m_PfxSmokeID = FXHelper::Pfx_CreateAttached("RocketTrailSeeder_Def", CHashedString(this->GetName().c_str()), "ROOT");
}


//--------------------------------------------------
//!
//!	Object_Projectile::BazookaOnImpact
//! Impact function for the bazooka rocket
//!
//--------------------------------------------------
void Object_Projectile::BazookaOnImpact( CEntity* pCollidee )
{
	if ( m_iProjectileIndex != iNumBazookaRockets )
	{
		if ( m_pobPhysicsSystem->Lua_Projectile_GetStateTime() >= 0.5f)
		{
			if ( m_ProjectileData.ProjectileAttackData != "" )
			{
				if(pCollidee && pCollidee->GetMessageHandler())
				{
					Message ProjColMessage(msg_projcol);
					ProjColMessage.SetString("Type", "bazooka");
					ProjColMessage.SetEnt("Projectile", this);
					pCollidee->GetMessageHandler()->QueueMessage(ProjColMessage);

					CombatHelper::Combat_GenerateStrike(this, m_ProjectileData.pAttacker, pCollidee, m_ProjectileData.ProjectileAttackData);
				}
			}
		}
	}

	// Bazookas always get destroyed, as they explode
	EXTERNALLY_SET_STATE(PROJECTILE_FSM, DESTROY);
}

//--------------------------------------------------
//!
//!	Object_Projectile::BazookaOnStrike
//! When a bazooka is struck it can mean only one thing - a kick ass
//! hero causing carnage.
//!
//--------------------------------------------------
void Object_Projectile::BazookaOnStrike( CEntity* pStriker )
{
	// On a strike a special destroy is required. Pass the striker (which should be the hero)
	// to the destroy function and that'll change the behaviour of the bazooka destruction
	BazookaOnDestroy(pStriker);
}


//--------------------------------------------------
//!
//!	Object_Projectile::BazookaOnDestroy
//! Destroy function for the bazooka rocket
//!
//! pHeroDeflect: A special state where by the bazooka destroy was invoked 
//! by the hero anf therefore she shouldn't be damaged in the explosion.
//--------------------------------------------------

void Object_Projectile::BazookaOnDestroy( CEntity* pHeroDeflect )
{
	if ( m_iProjectileIndex != iNumBazookaRockets )
	{
		// Kill particle effects
		FXHelper::Pfx_Destroy( m_BazookaPfxIDs.m_PfxFlameID, false );
		m_BazookaPfxIDs.m_PfxFlameID = 0;
		FXHelper::Pfx_Destroy( m_BazookaPfxIDs.m_PfxIgnitionID, false );
		m_BazookaPfxIDs.m_PfxIgnitionID = 0;
		FXHelper::Pfx_Destroy( m_BazookaPfxIDs.m_PfxSeederID, false );
		m_BazookaPfxIDs.m_PfxSeederID = 0;
		FXHelper::Pfx_Destroy( m_BazookaPfxIDs.m_PfxSmokeID, false );
		m_BazookaPfxIDs.m_PfxSmokeID = 0;

		bool bUseLODExplosion = pHeroDeflect == NULL ? false : true;

		// If this bazooka has hit something close then use the LOD effect
		if ( m_pobPhysicsSystem->Lua_Projectile_GetStateTime() < 0.5f)
		{
			bUseLODExplosion = true;
		}

		// If this bazooka is marked as using LOD effects, then set flag
		BazookaProjectileProperties* pobBazookaProperties = ObjectDatabase::Get().GetPointerFromName<BazookaProjectileProperties*>(m_ProjectileData.ProjectileProperties);
		ntAssert( pobBazookaProperties );
		if ( pobBazookaProperties->m_bUseLODEffects )
		{
			bUseLODExplosion = true;
		}

			if ( !m_bAlreadyExploded )
			{
			if ( !bUseLODExplosion )
			{
				// Normal, non LOD effects
				FXHelper::Pfx_CreateStatic("RocketExplode_Def", this, "ROOT");
				FXHelper::Pfx_CreateStatic("RocketExplode_ComplexSeeder_Def", this, "ROOT");
				FXHelper::Pfx_CreateStatic("DirtWake_02", this, "ROOT");
			FXHelper::Pfx_CreateStatic("Initial_boom", this, "ROOT");
				FXHelper::Pfx_CreateStatic("frag_Def", this, "ROOT");
			}
			else
			{
				// Use LOD explosion effects instead
				FXHelper::Pfx_CreateStatic("Clan_Cannon_Initial_Boom_PSystemSimpleDef", this, "ROOT");
				FXHelper::Pfx_CreateStatic("Clan_Cannon_Fragmentation_PSystemSimpleDef", this, "ROOT");
				FXHelper::Pfx_CreateStatic("Clan_Cannon_Dust_PSystemSimpleDef", this, "ROOT");
				FXHelper::Pfx_CreateStatic("Clan_Cannon_Smoke_PSystemComplexDef", this, "ROOT");
			}

			Physics::CExplosionParams params;
			params.m_fPush = 400.0f;
			params.m_fPushDropoff = 10.0f;
			params.m_fRadius = 8.0f;
			params.m_pobOriginator = pHeroDeflect ? pHeroDeflect : m_ProjectileData.pAttacker;
			params.m_obPosition = GetPosition();
			m_pobPhysicsSystem->Lua_AltExplosion( params );

				// Notify army manager of bazooka explosion - PS3 only!
				#ifdef PLATFORM_PS3
					ArmyManager::Get().ExplodeBazookaShot( GetPosition() );
				#endif

			// Trigger impact sound
			//AudioHelper::PlaySound(m_ProjectileData.ImpactSoundBank.GetString(),m_ProjectileData.ImpactSoundCue.GetString(),this);

			// Camera shake
			CPoint ptPos = GetPosition();
			CamMan::Get().Shake(2.f, 1.f, 3.f, &ptPos, 50.f*50.f);

		// Play impact sound
		ProjectileImpactAudio(this);

				// Don't explode again
				m_bAlreadyExploded = true;
			}
		}
	}


//--------------------------------------------------
//!
//!	Object_Projectile::BazookaGetDropOffTime
//! Gets the bazooka drop off time
//!
//--------------------------------------------------
float Object_Projectile::BazookaGetDropOffTime( void )
{
	Physics::ProjectileLG* pobLG = (Physics::ProjectileLG*)m_pobPhysicsSystem->GetFirstGroupByType(Physics::LogicGroup::PROJECTILE_LG);
	ntAssert( pobLG );

	return pobLG->GetFallTime();
}


//--------------------------------------------------
//!
//!	Object_Projectile::TGSDestroyAllProjectiles
//! TGS Hack to destroy all projectiles
//!
//--------------------------------------------------
void Object_Projectile::TGSDestroyAllProjectiles( void )
{
	Message destroyMsg(msg_think_ondestroyprojectile);

	// If it's a bazooka, send the message to the children
	if ( m_eProjectileType == PROJECTILE_TYPE_BAZOOKA_ROCKET )
	{
		// Send messages to children
		for ( int i = 0; i < iNumBazookaRockets; i++ )
		{
			if ( !BazookaChildren[i].IsNull() )
			{
				Object_Projectile* pobChild = (Object_Projectile*)CEntityManager::Get().FindEntity( BazookaChildren[i] );

				if ( pobChild )
				{
					pobChild->GetMessageHandler()->QueueMessage( destroyMsg );
				}
			}
		}
	}

	// ... and to myself
	GetMessageHandler()->QueueMessage( destroyMsg );
}


//--------------------------------------------------
//!
//!	Object_Projectile::BazookaDoAftertouchDrop
//! If the user was in aftertouch and exited after
//! the threshold time, then the bazooka rockets
//! move their drop off times forward to this point
//!
//--------------------------------------------------
void Object_Projectile::BazookaDoAftertouchDrop( float fEarliestDropOffTime )
{
	// Get the projectile lg of this object
	Physics::ProjectileLG* pobLG = (Physics::ProjectileLG*)m_pobPhysicsSystem->GetFirstGroupByType(Physics::LogicGroup::PROJECTILE_LG);
	ntAssert( pobLG );

	BazookaProjectileProperties* pobBazookaProperties = ObjectDatabase::Get().GetPointerFromName<BazookaProjectileProperties*>(m_ProjectileData.ProjectileProperties);
	ntAssert( pobBazookaProperties );

	float fProjectileStateTime = pobLG->GetStateTime();
	
	// Check if the drop should occur
	float fDropThresholdTime = pobBazookaProperties->m_fAftertouchDropThreshold;
	float fAftertouchHoldTime = 0.15f;

	if ( fProjectileStateTime < ( fAftertouchHoldTime + fDropThresholdTime ) )
	{
		return;
	}
    
	float fCurrentFallTime = pobLG->GetFallTime();

	// If the rocket isn't dropping already
	if ( fProjectileStateTime < fCurrentFallTime )
	{
		// Move the drop time forward by subtracting the state time
		float fNewFallTime = fCurrentFallTime - (fEarliestDropOffTime - fProjectileStateTime);

		// Make sure it's not less than the current state time or the drop acceleration will be instantly increased
		if ( fNewFallTime < fProjectileStateTime )
		{
			fNewFallTime = fProjectileStateTime;
		}

		// Set the new drop off time on the projectile logic group
		pobLG->SetFallTime( fNewFallTime );
	}
}


//--------------------------------------------------
//!
//!	Object_Projectile::DummyBazookaOnSpawn()
//! Spawn event function for the dummy rocket
//!
//--------------------------------------------------
void Object_Projectile::DummyBazookaOnSpawn( void )
{
	//Queue up a destroy message to kill this projectile off after 6.8 seconds.
	Message msgOnDestroy(msg_think_ondestroyprojectile);
	GetMessageHandler()->QueueMessageDelayed(msgOnDestroy, 6.8f);
}


//--------------------------------------------------
//!
//!	Object_Projectile::DummyBazookaOnImpact()
//! Impact event function for the dummy rocket
//!
//--------------------------------------------------
void Object_Projectile::DummyBazookaOnImpact( void )
{
	EXTERNALLY_SET_STATE(PROJECTILE_FSM, DESTROY);
}

//--------------------------------------------------
//!
//!	Object_Projectile::CreateNewProjectileInDatabase()
//! Creates a new projectile in the object database
//! Also sets some of the standard common data
//!
//--------------------------------------------------
Object_Projectile* Object_Projectile::CreateNewProjectileInDatabase( const Projectile_Attributes* const pProjAttrs )
{
	ntAssert(pProjAttrs);

	char acStringType[32];
	switch( pProjAttrs->m_eType )
	{
		case PROJECTILE_TYPE_BAZOOKA_ROCKET:
			sprintf( acStringType, "bazooka" );
			break;

		case PROJECTILE_TYPE_CROSSBOW_BOLT:
			sprintf( acStringType, "crossbow" );
			break;

		case PROJECTILE_TYPE_BALLISTA_BOLT:
			sprintf( acStringType, "ballista" );
			break;
		case PROJECTILE_TYPE_AGEN_SWORD:
			sprintf( acStringType, "agensword" );
			break;
		case PROJECTILE_TYPE_KING_LIGHTNINGBALL:
			sprintf( acStringType, "kinglightningball" );
			break;
		case PROJECTILE_TYPE_KING_WINGATTACKCROW:
			sprintf( acStringType, "kingwingattackcrow" );
			break;
		case PROJECTILE_TYPE_CANNON_BALL:
			sprintf( acStringType, "cannonball" );
			break;

		default:
			ntAssert_p( false, ("Unknown projectile type somehow") );
			sprintf( acStringType, "unknown" );
			break;
	}

    // Generate a name for this projectile	
	char name[64] = {0};
	sprintf(name, "%s%d", acStringType, g_iProjectileID);
	g_iProjectileID++;

	// Create the projectile in the object database
	DataObject* pDO = ObjectDatabase::Get().ConstructObject("Object_Projectile", name, GameGUID(), 0, true, false);

	Object_Projectile* pProjectile = (Object_Projectile*)pDO->GetBasePtr();
	ntAssert(pProjectile != NULL);

	pProjectile->SetAttributeTable(LuaAttributeTable::Create());
	pProjectile->GetAttributeTable()->SetDataObject(pDO);
	LuaAttributeTable* pAttributes = pProjectile->GetAttributeTable();
	
	// Name
	pAttributes->SetAttribute("Name", name);

	// Clump
	pAttributes->SetAttribute("Clump", pProjAttrs->m_Clump.c_str());

	// Type
	pProjectile->m_eProjectileType = pProjAttrs->m_eType;
	pAttributes->SetAttribute("ProjectileType", acStringType);

	// Description
	pAttributes->SetAttribute("Description", "projectile");
	pProjectile->m_pobSharedAttributes = pProjAttrs;

	ObjectDatabase::Get().DoPostLoadDefaults(pDO);
	return pProjectile;
}


//--------------------------------------------------
//!
//!	Object_Projectile::ConstructCrossbowObject()
//! Creates the object (entity storage, clump creation, etc).
//!
//--------------------------------------------------
void Object_Projectile::ConstructCrossbowObject()
{
	//Handle creation of the clump for the projectile (originally from Lua_ConstructProjectile).
	ProjectileProperties* pobProperties = ObjectDatabase::Get().GetPointerFromName<ProjectileProperties*>(m_ProjectileData.ProjectileProperties);
	ntAssert(pobProperties);

	CPoint		obPosition		=	m_ProjectileData.vPosition;
	CDirection	obDirection		=	m_ProjectileData.vDirection;

	//Create a group
	Physics::ProjectileLG* lg = Physics::ProjectileLG::Construct(this, pobProperties, obPosition, obDirection);
	Physics::SingleRigidLG* srlg = NT_NEW Physics::SingleRigidLG( GetName(), this); 
	srlg->Load();
	
	//Createa a system
	if(GetPhysicsSystem() == 0)
	{
		Physics::System* system = NT_NEW_CHUNK(Mem::MC_ENTITY) Physics::System(this, GetName());
		SetPhysicsSystem(system);
	}
	//Add the group
	GetPhysicsSystem()->AddGroup(lg);
	GetPhysicsSystem()->AddGroup(srlg);
	lg->Activate();
	//Construction done here.		

	//If we have an 'attacker' (the person presumably holding the weapon that fired this projectile) then exclude collision
	//with that entity (no-one can shoot themselves in the foot this way).
	if(m_ProjectileData.pAttacker != 0)
	{
		GetInteractionComponent()->ExcludeCollisionWith(m_ProjectileData.pAttacker);
	}

	if((m_ProjectileData.pTarget != 0) && (m_ProjectileData.fTrackingSpeed > 0.0f))
	{
		//Tell this projectile to track with the offset specified.
		//(Commented out after being reported as a 'bug' for crossbowmen having aftertouch).
		CPoint vOffset = m_ProjectileData.vTrackedEntityOffset;
		GetPhysicsSystem()->Lua_Projectile_EnableEntityTracking(m_ProjectileData.pTarget, vOffset.X(), vOffset.Y(), vOffset.Z(),
			m_ProjectileData.fTrackingSpeed);
	}

	// Call spawn function here
	CrossbowOnSpawn();
}

//--------------------------------------------------
//!
//!	Object_Projectile::ConstructAGenSwordObject()
//! Creates the object (entity storage, clump creation, etc).
//!
//--------------------------------------------------
void Object_Projectile::ConstructAGenSwordObject()
{
	//Handle creation of the clump for the projectile (originally from Lua_ConstructProjectile).
	ProjectileProperties* pobProperties = ObjectDatabase::Get().GetPointerFromName<ProjectileProperties*>(m_ProjectileData.ProjectileProperties);
	ntAssert(pobProperties);

	CPoint		obPosition		=	m_ProjectileData.vPosition;
	CDirection	obDirection		=	m_ProjectileData.vDirection;

	//Create a group
	Physics::ProjectileLG* lg = Physics::ProjectileLG::Construct(this, pobProperties, obPosition, obDirection, 0.0f, 0.0f, &m_ProjectileData);
	//Createa a system
	if(GetPhysicsSystem() == 0)
	{
		Physics::System* system = NT_NEW_CHUNK(Mem::MC_ENTITY) Physics::System(this, GetName());
		SetPhysicsSystem(system);
	}
	//Add the group
	GetPhysicsSystem()->AddGroup(lg);
	lg->Activate();
	//Construction done here.		

	//If we have an 'attacker' (the person presumably holding the weapon that fired this projectile) then exclude collision
	//with that entity (no-one can shoot themselves in the foot this way).
	if(m_ProjectileData.pAttacker != 0)
	{
		GetInteractionComponent()->ExcludeCollisionWith(m_ProjectileData.pAttacker);
	}

	if(m_ProjectileData.pTarget != 0)
	{
		//Tell this projectile to track with the offset specified.
		//(Commented out after being reported as a 'bug' for crossbowmen having aftertouch).
		CPoint vOffset = m_ProjectileData.vTrackedEntityOffset;
		GetPhysicsSystem()->Lua_Projectile_EnableEntityTracking(m_ProjectileData.pTarget, vOffset.X(), vOffset.Y(), vOffset.Z(), 0.3f);
	}

	// Call spawn function here
	AGenSwordOnSpawn();
}




//--------------------------------------------------
//!
//!	Object_Projectile::ConstructKingLightningBallObject()
//! Creates the object (entity storage, clump creation, etc).
//!
//--------------------------------------------------
void Object_Projectile::ConstructKingLightningBallObject(bool bLarge)
{
	//Handle creation of the clump for the projectile (originally from Lua_ConstructProjectile).
	ProjectileProperties* pobProperties = ObjectDatabase::Get().GetPointerFromName<ProjectileProperties*>(m_ProjectileData.ProjectileProperties);
	ntAssert(pobProperties);

	CPoint		obPosition		=	m_ProjectileData.vPosition;
	CDirection	obDirection		=	m_ProjectileData.vDirection;

	//Create a group
	Physics::ProjectileLG* lg = Physics::ProjectileLG::Construct(this, pobProperties, obPosition, obDirection);
	//Create a system
	if(GetPhysicsSystem() == 0)
	{
		Physics::System* system = NT_NEW_CHUNK(Mem::MC_ENTITY) Physics::System(this, GetName());
		SetPhysicsSystem(system);
	}
	//Add the group
	GetPhysicsSystem()->AddGroup(lg);
	lg->Activate();
	//Construction done here.		

	//If we have an 'attacker' (the person presumably holding the weapon that fired this projectile) then exclude collision
	//with that entity (no-one can shoot themselves in the foot this way).
	if(m_ProjectileData.pAttacker != 0)
	{
		GetInteractionComponent()->ExcludeCollisionWith(m_ProjectileData.pAttacker);
	}

	if(m_ProjectileData.pTarget != 0)
	{
		//Tell this projectile to track with the offset and tracking-speed specified.
		CPoint vOffset = m_ProjectileData.vTrackedEntityOffset;
		GetPhysicsSystem()->Lua_Projectile_EnableEntityTracking(m_ProjectileData.pTarget, vOffset.X(), vOffset.Y(), vOffset.Z(), m_ProjectileData.fTrackingSpeed);
	}

	// Call spawn function here
	KingLightningBallOnSpawn(bLarge);
}


//--------------------------------------------------
//!
//!	Object_Projectile::ConstructKingWingAttackCrowObject()
//! Creates the object (entity storage, clump creation, etc).
//!
//--------------------------------------------------
void Object_Projectile::ConstructKingWingAttackCrowObject()
{
	//Handle creation of the clump for the projectile (originally from Lua_ConstructProjectile).
	ProjectileProperties* pobProperties = ObjectDatabase::Get().GetPointerFromName<ProjectileProperties*>(m_ProjectileData.ProjectileProperties);
	ntAssert(pobProperties);

	CPoint		obPosition		=	m_ProjectileData.vPosition;
	CDirection	obDirection		=	m_ProjectileData.vDirection;

	//Create a group
	Physics::ProjectileLG* lg = Physics::ProjectileLG::Construct(this, pobProperties, obPosition, obDirection);
	//Createa a system
	if(GetPhysicsSystem() == 0)
	{
		Physics::System* system = NT_NEW_CHUNK(Mem::MC_ENTITY) Physics::System(this, GetName());
		SetPhysicsSystem(system);
	}
	//Add the group
	GetPhysicsSystem()->AddGroup(lg);
	lg->Activate();
	//Construction done here.		

	//If we have an 'attacker' (the person presumably holding the weapon that fired this projectile) then exclude collision
	//with that entity (no-one can shoot themselves in the foot this way).
	if(m_ProjectileData.pAttacker != 0)
	{
		GetInteractionComponent()->ExcludeCollisionWith(m_ProjectileData.pAttacker);
	}

	if(m_ProjectileData.pTarget != 0)
	{
		//Tell this projectile to track with the offset specified.
		//(Commented out after being reported as a 'bug' for crossbowmen having aftertouch).
		CPoint vOffset = m_ProjectileData.vTrackedEntityOffset;
		GetPhysicsSystem()->Lua_Projectile_EnableEntityTracking(m_ProjectileData.pTarget, vOffset.X(), vOffset.Y(), vOffset.Z(), m_ProjectileData.fTrackingSpeed);
	}

	// Call spawn function here
	KingWingAttackCrowOnSpawn();
}





//--------------------------------------------------
//!
//!	Object_Projectile::ConstructBazookaObject
//! Constructs the bazooka (entity storage, clump creation, etc).
//!
//--------------------------------------------------
void Object_Projectile::ConstructBazookaObject()
{
	//Handle creation of the clump for the projectile (originally from Lua_ConstructProjectile).
	BazookaProjectileProperties* pobProperties = ObjectDatabase::Get().GetPointerFromName<BazookaProjectileProperties*>(m_ProjectileData.ProjectileProperties);
	ntAssert(pobProperties);

	// Setup the gravity drop off if required
	float fFallTime = 0.0f;
	float fFallAcceleration = 0.0f;

	if ( m_iProjectileIndex >= 0 && m_iProjectileIndex < iNumBazookaRockets )
	{
		fFallAcceleration = pobProperties->m_fDropOffAcceleration;
		
		int j = 0;
		ntstd::List<float>::const_iterator iter;
		ntstd::List<float>::const_iterator enditer = pobProperties->m_DropOffTimeList.end();

		for ( iter = pobProperties->m_DropOffTimeList.begin(); iter != enditer; iter++ )
		{
			if ( j == m_iProjectileIndex )
			{
				fFallTime = (*iter);
				break;
			}
			j++;
		}
	}
	else
	{
		// Must be the dummy rocket
		fFallAcceleration = pobProperties->m_fDropOffAcceleration;
		ntAssert( pobProperties->m_DropOffTimeList.size() >= 1 );

		ntstd::List<float>::const_iterator LeadDropOffTimeIter = pobProperties->m_DropOffTimeList.begin();
		fFallTime = (*LeadDropOffTimeIter);
	}

	CPoint		obPosition	= m_ProjectileData.vPosition;
	CDirection	obDirection = m_ProjectileData.vDirection;

	//Create a group
	Physics::ProjectileLG* lg = Physics::ProjectileLG::Construct(this, pobProperties, obPosition, obDirection, fFallTime, fFallAcceleration );	//Createa a system
	if(GetPhysicsSystem() == 0)
	{
		Physics::System* system = NT_NEW_CHUNK(Mem::MC_ENTITY) Physics::System(this, GetName());
		SetPhysicsSystem(system);
	}
	//Add the group
	GetPhysicsSystem()->AddGroup(lg);
	lg->Activate();
	//Construction done here.		

	//If we have an 'attacker' (the person presumably holding the weapon that fired this projectile) then exclude collision
	//with that entity (no-one can shoot themselves in the foot this way).
	if(m_ProjectileData.pAttacker != 0)
	{
		GetInteractionComponent()->ExcludeCollisionWith(m_ProjectileData.pAttacker);
	}

	if(m_ProjectileData.pTarget != 0)
	{
		//Tell this projectile to track with the specified offset from the target's root.
		CPoint vOffset = m_ProjectileData.vTrackedEntityOffset;
		GetPhysicsSystem()->Lua_Projectile_EnableEntityTracking(m_ProjectileData.pTarget, vOffset.X(), vOffset.Y(), vOffset.Z());
	}

	// Call spawn function here.
	BazookaOnSpawn();
}

//--------------------------------------------------
//!
//!	Object_Projectile::ConstructCannonballObject()
//! Creates the object (entity storage, clump creation, etc).
//!
//--------------------------------------------------
void Object_Projectile::ConstructCannonballObject()
{
	//Handle creation of the clump for the projectile (originally from Lua_ConstructProjectile).
	ProjectileProperties* pobProperties = ObjectDatabase::Get().GetPointerFromName<ProjectileProperties*>(m_ProjectileData.ProjectileProperties);
	ntAssert(pobProperties);

	CPoint		obPosition		=	m_ProjectileData.vPosition;
	CDirection	obDirection		=	m_ProjectileData.vDirection;

	//Create a group
	Physics::ProjectileLG* lg = Physics::ProjectileLG::Construct(this, pobProperties, obPosition, obDirection );

	// 
	lg->StopMovingOnCollision( false );

	//Create a physics system
	if(GetPhysicsSystem() == 0)
	{
		Physics::System* system = NT_NEW_CHUNK(Mem::MC_ENTITY) Physics::System(this, GetName());
		SetPhysicsSystem(system);
	}
	//Add the group
	GetPhysicsSystem()->AddGroup(lg);
	lg->Activate();
	//Construction done here.		

	//If we have an 'attacker' (the person presumably holding the weapon that fired this projectile) then exclude collision
	//with that entity (no-one can shoot themselves in the foot this way).
	if(m_ProjectileData.pAttacker != 0)
	{
		GetInteractionComponent()->ExcludeCollisionWith(m_ProjectileData.pAttacker);
	}

	if(m_ProjectileData.pTarget != 0)
	{
		//Tell this projectile to track with the offset specified.
		//(Commented out after being reported as a 'bug' for crossbowmen having aftertouch).
		CPoint vOffset = m_ProjectileData.vTrackedEntityOffset;
		GetPhysicsSystem()->Lua_Projectile_EnableEntityTracking(m_ProjectileData.pTarget, vOffset.X(), vOffset.Y(), vOffset.Z(), 0.3f);
	}

	// Call spawn function here
	CannonBallOnSpawn();
}



//------------------------------------------------------------------------------------------
//!  protected  ReparentProjectile
//!
//!
//!  @remarks <TODO: insert remarks here>
//!
//!  @author GavB @date 27/07/2006
//------------------------------------------------------------------------------------------
void Object_Projectile::ReparentProjectile( CEntity* pobTarget, const CPoint& obCollisionPoint)
{
	ntAssert(pobTarget);
	ntAssert(pobTarget->GetHierarchy());

	ntAssert(GetHierarchy());
	ntAssert(GetPhysicsSystem());

	Transform* pobSelfTransform = GetHierarchy()->GetRootTransform();
	Transform* pobTargetTransform = NULL;

	// Apply a force
	if(pobTarget->GetPhysicsSystem())
	{
		Physics::ProjectileLG* lg = (Physics::ProjectileLG*)GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::PROJECTILE_LG);
		ntAssert(lg);

		CDirection obForce(lg->GetLinearVelocity());
		obForce*=lg->GetMass();

		pobTarget->GetPhysicsSystem()->ApplyLocalisedLinearImpulse(obForce, CVector(obCollisionPoint));
	}

	// 
	if(pobTarget->IsCharacter()) // Parenting to a character
	{
		// Sanity check
		ntError( pobTarget->GetAttackComponent()->GetAttackDefinition() && "There should at least be an attack def" );
		
		// Clear movement controllers and disable collision strike handler
		if(GetMovement())
		{
			GetMovement()->ClearControllers();
		}

		GetPhysicsSystem()->GetCollisionStrikeHandler()->Disable();

		pobTargetTransform = pobTarget->GetHierarchy()->GetCharacterBoneTransform(CHARACTER_BONE_PELVIS);

		float fBestDist=FLT_MAX;

		// JML - Only consider real bones, there are transforms on ragdolls which don't correspond to visible bones.
		//       This can lead to very strange issues of weird swords under some (rare) circumstances.
		CHARACTER_BONE_ID eBestBone=CHARACTER_BONE_PELVIS;
		for(CHARACTER_BONE_ID eBone=CHARACTER_BONE_PELVIS; eBone<CHARACTER_BONE_R_KNEE; eBone = CHARACTER_BONE_ID(int(eBone)+1))
		{
			Transform* pTrans = pobTarget->GetHierarchy()->GetCharacterBoneTransform(eBone);
			
			if(!pTrans)
				continue;

			CDirection dDiff = pTrans->GetWorldTranslation() ^ obCollisionPoint;
			float fDist = dDiff.LengthSquared();
			if(fDist < fBestDist)
			{
				eBestBone = eBone;
				fBestDist = fDist;
				pobTargetTransform = pTrans;
			}
		}								

		if (!pobTargetTransform) // If for some bizarre reason we don't get a valid transform, revert back to using the root transform
		{
			pobTargetTransform=pobTarget->GetHierarchy()->GetRootTransform();
			eBestBone = CHARACTER_BONE_PELVIS;
		}

			// Calculate the new translation								
			CPoint obNewTranslation(0.0f,0.0f,0.0f);
			CMatrix obLocalMatrix(pobSelfTransform->GetLocalMatrix());
			obLocalMatrix*=pobTargetTransform->GetWorldMatrix().GetAffineInverse(); // Combine with the affine inverse of the parent
			pobSelfTransform->SetLocalMatrix(obLocalMatrix);

			//pobSelfTransform->SetLocalTranslation(obNewTranslation);

			// Debug info
			if (g_ShellOptions->m_bDebugArcher)
			{
				switch (eBestBone)
				{
					case CHARACTER_BONE_ROOT: ntPrintf("Projectile parent to root\n"); break; 
					case CHARACTER_BONE_PELVIS: ntPrintf("Projectile parent to pelvis\n"); break; 
					case CHARACTER_BONE_SPINE_00: ntPrintf("Projectile parent to spine_00\n"); break; 
					case CHARACTER_BONE_SPINE_01: ntPrintf("Projectile parent to spine_01\n"); break; 
					case CHARACTER_BONE_SPINE_02: ntPrintf("Projectile parent to spine_02\n"); break; 
					case CHARACTER_BONE_NECK: ntPrintf("Projectile parent to neck\n"); break; 
					case CHARACTER_BONE_HEAD: ntPrintf("Projectile parent to head\n"); break; 
					case CHARACTER_BONE_HIPS: ntPrintf("Projectile parent to hips\n"); break; 
					case CHARACTER_BONE_L_SHOULDER: ntPrintf("Projectile parent to l_shoulder\n"); break; 
					case CHARACTER_BONE_L_ARM: ntPrintf("Projectile parent to l_arm\n"); break; 
					case CHARACTER_BONE_L_ELBOW: ntPrintf("Projectile parent to l_elbow\n"); break; 
					case CHARACTER_BONE_L_WRIST: ntPrintf("Projectile parent to l_wrist\n"); break; 
					case CHARACTER_BONE_L_WEAPON: ntPrintf("Projectile parent to l_weapon\n"); break; 
					case CHARACTER_BONE_L_LEG: ntPrintf("Projectile parent to l_leg\n"); break; 
					case CHARACTER_BONE_L_KNEE: ntPrintf("Projectile parent to l_knee\n"); break; 
					case CHARACTER_BONE_R_SHOULDER: ntPrintf("Projectile parent to r_shoulder\n"); break; 
					case CHARACTER_BONE_R_ARM: ntPrintf("Projectile parent to r_arm\n"); break; 
					case CHARACTER_BONE_R_ELBOW: ntPrintf("Projectile parent to r_elbow\n"); break; 
					case CHARACTER_BONE_R_WRIST: ntPrintf("Projectile parent to r_wrist\n"); break; 
					case CHARACTER_BONE_R_WEAPON: ntPrintf("Projectile parent to r_weapon\n"); break; 
					case CHARACTER_BONE_R_LEG: ntPrintf("Projectile parent to r_leg\n"); break; 
					case CHARACTER_BONE_R_KNEE: ntPrintf("Projectile parent to r_knee\n"); break; 
					default: break;
				}
			}
		}
	else // Parenting to a world object
	{
		pobTargetTransform = pobTarget->GetHierarchy()->GetRootTransform();

		GetHierarchy()->GetRootTransform()->SetLocalTranslation(obCollisionPoint); // Set the position of the projectile to the collision point

		CMatrix obLocalMatrix=pobSelfTransform->GetLocalMatrix();
		obLocalMatrix*=pobTargetTransform->GetWorldMatrix().GetAffineInverse(); // Combine with the affine inverse of the parent
		pobSelfTransform->SetLocalMatrix(obLocalMatrix);
	}

	// Reparent this entity to the root of the target entity

	SetParentEntity(pobTarget);
	pobSelfTransform->RemoveFromParent();
	pobTargetTransform->AddChild(pobSelfTransform);
}

//------------------------------------------------------------------------------------------
//	Object_Projectile::UpdateProjectileAudio
//!	Starts, stops and maintains sound effects associated with projectile movement and passbys.
//!	Should be called whenever this entity is active and updating.
//------------------------------------------------------------------------------------------
void Object_Projectile::UpdateProjectileAudio(void)
{
	// Check if projectile audio enabled
	if (!m_bProjectileAudio)
		return;

	// Calculate projectile proximity to listener
	CPoint projPos = GetPosition();
	CPoint listenerPos = AudioSystem::Get().GetListenerPosition();
	CDirection projToListener = projPos^listenerPos;
	float fProximitySqr = projToListener.LengthSquared();

	// Start/stop loop as req'd
	if (!m_ProjectileData.LoopingSoundBank.IsNull() && !m_ProjectileData.LoopingSoundCue.IsNull())
	{
		// Test if looping sound currently playing
		if (0 == m_uiLoopingSoundId)
		{
			// Start looping sound when projectile within audible range (zero or negative range indicates ALWAYS audible)
			if (m_ProjectileData.LoopingSoundRange <= 0.0f
				|| fProximitySqr <= m_ProjectileData.LoopingSoundRange*m_ProjectileData.LoopingSoundRange)
			{
				// Start sound
				if (AudioSystem::Get().Sound_Prepare(m_uiLoopingSoundId, ntStr::GetString(m_ProjectileData.LoopingSoundBank), ntStr::GetString(m_ProjectileData.LoopingSoundCue)))
				{
					AudioSystem::Get().Sound_SetPosition(m_uiLoopingSoundId, projPos);

					if (m_iLoopingSoundArg >= 0)
					{
						float fMin = 0.0f;
						float fMax = 0.0f;
						if (AudioSystem::Get().Sound_GetParameterRange(m_uiLoopingSoundId, "rocketnumber", fMin, fMax))
						{
							int iArg = m_iLoopingSoundArg%(int)fMax;
							AudioSystem::Get().Sound_SetParameterValue(m_uiLoopingSoundId, "rocketnumber", (float)iArg);
						}
					}

					AudioSystem::Get().Sound_Play(m_uiLoopingSoundId);
				}
				else
				{
					// Ensure identifier remains invalid (possible for a valid handle to be supplied even on sound prep failure)
					m_uiLoopingSoundId = 0;
				}
			}
		}
		// Stop looping sound beyond audible range
		else if (m_ProjectileData.LoopingSoundRange > 0.0f
			&& fProximitySqr > m_ProjectileData.LoopingSoundRange*m_ProjectileData.LoopingSoundRange)
		{
			// Stop sound
			AudioSystem::Get().Sound_Stop(m_uiLoopingSoundId);
			m_uiLoopingSoundId = 0;
		}
		// Update looping sound position
		else
		{
			AudioSystem::Get().Sound_SetPosition(m_uiLoopingSoundId, projPos);
		}
	}

	// Trigger passby sound as req'd (no passby for player's own projectiles)
	if (CEntityManager::Get().GetPlayer() != m_ProjectileData.pAttacker
		&& !m_ProjectileData.PassbySoundBank.IsNull()
		&& !m_ProjectileData.PassbySoundCue.IsNull())
	{
		// Test if passby sound not currently playing
		if (0 == m_uiPassbySoundId)
		{
			// Trigger passby sound when projectile within audible range (zero or negative range indicates NEVER audible)
			if (m_ProjectileData.PassbySoundRange > 0.0f
				&& fProximitySqr <= m_ProjectileData.PassbySoundRange*m_ProjectileData.PassbySoundRange)
			{
				// Trigger sound
				if (AudioSystem::Get().Sound_Prepare(m_uiPassbySoundId, ntStr::GetString(m_ProjectileData.PassbySoundBank), ntStr::GetString(m_ProjectileData.PassbySoundCue)))
				{
					AudioSystem::Get().Sound_Play(m_uiPassbySoundId);
					AudioSystem::Get().Sound_SetPosition(m_uiPassbySoundId, projPos);
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


//------------------------------------------------------------------------------------------
//	Object_Projectile::EnableProjectileAudio
//!	Projectile flight audio is enabled by default. Disabling ensures all sounds associated
//!	with projectile movement are stopped. Projectile audio should be disabled whenever this
//!	entity deactivates, is destroyed, etc.
//!	@param bEnable	Use true to enable projectile audio, false to disable.
//!	@note Only audio associated with projectile flight (looping and passby sounds) is
//!	enabled/disabled.
//------------------------------------------------------------------------------------------
void Object_Projectile::EnableProjectileAudio(bool bEnable)
{
	m_bProjectileAudio = bEnable;

	// If diabling ensure stopped.
	if (!m_bProjectileAudio)
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
}


//------------------------------------------------------------------------------------------
//	Object_Projectile::ProjectileImpactAudio
//!	Triggers any audio associated with projectile impact.
//!	@param pobCollidee	Entity projectile has impacted.
//!	@note Disables projectile flight audio.
//------------------------------------------------------------------------------------------
void Object_Projectile::ProjectileImpactAudio(CEntity* pobCollidee)
{
	if (m_bProjectileAudio)
	{
		// Projectile flight audio control
		EnableProjectileAudio(false);

		if (!pobCollidee || m_ProjectileData.ImpactSoundBank.IsNull() || m_ProjectileData.ImpactSoundCue.IsNull())
			return;

		AudioHelper::PlaySound(m_ProjectileData.ImpactSoundBank.GetString(), m_ProjectileData.ImpactSoundCue.GetString(), this);
	}
}


//------------------------------------------------------------------------------------------
//	Object_Projectile::ProjectileFireAudio
//!	Triggers any audio associated with projectile fire/launch.
//------------------------------------------------------------------------------------------
void Object_Projectile::ProjectileFireAudio()
{
	if (m_ProjectileData.FireSoundBank.IsNull() || m_ProjectileData.FireSoundCue.IsNull())
		return;

	AudioHelper::PlaySound(ntStr::GetString(m_ProjectileData.FireSoundBank), ntStr::GetString(m_ProjectileData.FireSoundCue), this);
}


//------------------------------------------------------------------------------------------
//!
//!	Object_Projectile::Destroy
//!	Destroy this projectile
//!
//------------------------------------------------------------------------------------------
void Object_Projectile::Destroy()
{
	EXTERNALLY_SET_STATE(PROJECTILE_FSM, DESTROY);
}
