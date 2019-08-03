//------------------------------------------------------------------------------------------
//!
//!	\file game/entityarcher.cpp
//!	Definition of the Archer entity object
//!
//------------------------------------------------------------------------------------------

//#include "anim/animator.h"
#include "camera/camman.h"
#include "camera/camview.h"
#include "core/timer.h"
#include "core/visualdebugger.h"
#include "lua/ninjalua.h"
#include "physics/projectilelg.h"

#include "game/archer3rdpcontroller.h"
#include "game/archermovementparams.h"
#include "game/entityarcher.h"
#include "game/entitymanager.h"
#include "game/chatterboxman.h"
#include "game/inputcomponent.h"
#include "game/entityprojectile.h"
#include "game/luaglobal.h"
#include "game/attacks.h"				// Attack component
#include "game/awareness.h"				// Awareness component
#include "game/movement.h"				// Movement
#include "game/aimcontroller.h"			// Aiming component
#include "game/interactioncomponent.h"	// Interaction component
#include "game/messagehandler.h"		// Message
#include "game/fsm.h"
#include "game/query.h"

#include "hud/hudmanager.h"
#include "hud/failurehud.h"
#include "game/combatstyle.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include "Physics/advancedcharactercontroller.h"
#endif

//#include "hair/haircollision.h"
//#include "hair/forcefielditem.h"

#include "objectdatabase/dataobject.h"
#include "Physics/system.h"				// Dynamics
//#include "JAMNET/netman.h"				// Net component

#ifdef _RELEASE
#	define LOG(pcText)
#else
#	define LOG(pcText)
//#	define LOG(pcText) ntPrintf("%s\n", pcText)
#endif

//#define ARCHER_DEBUG_RENDER		

// Interesting global defining the distance the archer can get to a vaulting object before she'll
// drop on to all fours. 
float Archer::m_sCrouchActivatingRange = 2.0f;

//------------------------------------------------------------------------------------------
// Archer Lua Interface
//------------------------------------------------------------------------------------------
LUA_EXPOSED_START_INHERITED(Archer, Player)
	
	LUA_EXPOSED_METHOD(CanCrouch,						Lua_CanCrouch,	"", "", "" )
	LUA_EXPOSED_METHOD(AlwaysCrouch,					Lua_AlwaysCrouch,	"", "", "" )
LUA_EXPOSED_END(Player)


//------------------------------------------------------------------------------------------
// Archer XML Interface
//------------------------------------------------------------------------------------------
START_STD_INTERFACE(Archer)
	COPY_INTERFACE_FROM(Player)
	DEFINE_INTERFACE_INHERITANCE(Player)

	OVERRIDE_DEFAULT(ConstructionScript, "")		// FIX ME Not sure about this
	OVERRIDE_DEFAULT(DestructionScript, "")
	OVERRIDE_DEFAULT(InitialSystemState, "DefaultState")
	OVERRIDE_DEFAULT(WalkingController, "ArcherWalkRun")

	PUBLISH_VAR_AS( m_fCrouchVolumeHeight, CrouchVolumeHeight )
	PUBLISH_VAR_AS( m_fCrouchVolumeHeight2, CrouchVolumeHeight2 )

	DECLARE_POSTCONSTRUCT_CALLBACK(OnPostConstruct)
	DECLARE_POSTPOSTCONSTRUCT_CALLBACK(OnPostPostConstruct)
END_STD_INTERFACE

//------------------------------------------------------------------------------------------
// Archer XML Settings
//------------------------------------------------------------------------------------------
START_STD_INTERFACE(ArcherSettings)

	PUBLISH_VAR_AS( m_MainSoundBank,			MainSoundBank )
	PUBLISH_VAR_AS( m_ChargeUpSFX,				ChargeUpSFX )
	PUBLISH_VAR_AS( m_ChargedSFX,				ChargedSFX )

	PUBLISH_VAR_AS( m_fReloadTime,				ReloadTime )
	PUBLISH_VAR_AS( m_fChargeUpTime,			ChargeUpTime )
	PUBLISH_VAR_AS( m_fDenyControlUntilCharge,	DenyControlUntilCharge )
END_STD_INTERFACE


//--------------------------------------------------
//!
//! Archer State Machine - was archerstate.lua
//!
//--------------------------------------------------
STATEMACHINE(ARCHER_FSM, Archer)

//-------------- Setup --------------//
	ARCHER_FSM(CHashedString& sState)
	{
		// TODO: Move this to an enumeration
		if			(sState == CHashedString("DefaultState"))			SET_INITIAL_STATE(DefaultState);
		else if		(sState == CHashedString("ReactState"))				SET_INITIAL_STATE(ReactState);
		else if		(sState == CHashedString("DeadState"))				SET_INITIAL_STATE(DeadState);
		else if		(sState == CHashedString("ExternalControlState"))	SET_INITIAL_STATE(ExternalControlState);
		else if		(sState == CHashedString("InteractingState"))		SET_INITIAL_STATE(InteractingState);
		else		
		{
			ntPrintf("Unrecognised playerstate %s in ARCHER_FSM\n", ntStr::GetString(sState));
			SET_INITIAL_STATE(DefaultState);
		}
	}

	void JumpToState(CHashedString& sState)
	{
		// TODO: Move this to an enumeration
		if			(sState == CHashedString("DefaultState"))			SetState(DefaultState::GetInstance());
		else if		(sState == CHashedString("ReactState"))				SetState(ReactState::GetInstance());
		else if		(sState == CHashedString("DeadState"))				SetState(DeadState::GetInstance());
		else if		(sState == CHashedString("ExternalControlState"))	SetState(ExternalControlState::GetInstance());
		else if		(sState == CHashedString("InteractingState"))		SetState(InteractingState::GetInstance());
		else		
		{
			ntPrintf("Unrecognised playerstate %s in ARCHER_FSM\n", ntStr::GetString(sState));
			SetState(DefaultState::GetInstance());
		}
	}

//-------------- Global msg --------------//
	BEGIN_EVENTS
		EVENT(msg_combat_recovered)
		{
			LOG("msg_combat_recovered:received");
			// Tell the combat system the recovery is over
			ME->GetAttackComponent()->CompleteRecovery();
			SET_STATE(DefaultState);
		}
		END_EVENT(true)

		ON_UPDATE
			ME->UpdateHealth( CTimer::Get().GetGameTimeChange() );
		END_EVENT(true)

		EVENT(msg_combat_killed)
			SET_STATE( DeadState );
		END_EVENT(true)

	END_EVENTS


	//-------------- Archer states --------------//
	STATE(DefaultState)
		BEGIN_EVENTS
			ON_ENTER
				// No Interaction Target
				ME->SetInteractionTarget(0);


				{
					// Name of the walking controller
					CHashedString hsDefName = ME->GetWalkingController();

					// Get a transition that will do its best to put us in the best movement state
					MovementControllerDef* pobDefintion = ObjectDatabase::Get().GetPointerFromName<MovementControllerDef*>(hsDefName);
					user_error_p(pobDefintion, ("Error! Movement definition %s not found\n",ntStr::GetString(hsDefName)));

					if( ME->AddChainedController() )
					{
						// Push an instance of the controller on to our movement component
						ME->GetMovement()->AddChainedController(*pobDefintion, CMovement::DMM_STANDARD, ME->WalkRunBlendInTime() );
					}
					else
					{
						// Push an instance of the controller on to our movement component
						ME->GetMovement()->BringInNewController(*pobDefintion, CMovement::DMM_STANDARD, ME->WalkRunBlendInTime() );
					}

					// Restore the blend in time if required
					ME->WalkRunBlendInTime( CMovement::MOVEMENT_BLEND );

					// Restoe the flag whether it was used or not
					ME->AddChainedController( false );
				}

			END_EVENT(true)

			// Update the old archer component
			ON_UPDATE
				ME->UpdateAiming( CTimer::Get().GetSystemTimeChange() );
				ME->UpdateHealth( CTimer::Get().GetGameTimeChange() );

				if( ME->GetInputComponent() && (ME->GetInputComponent()->GetVPressed() | ME->GetInputComponent()->GetVHeld()) & (1 << AB_ACTION) )
				{
					ME->SetVaultingSafe();

					if(ME->VaultObjectIfAppropriate())
					{
						ME->LogEvent(AE_VAULT);
						SET_STATE(VaultingState);
						END_EVENT(true)
					}

					ME->ClearVaultingSafe();
				}

			END_EVENT(true)

			EVENT(msg_combat_struck)
				SET_STATE( ReactState );
			END_EVENT(true)

			EVENT(msg_external_control_start)
				SET_STATE( ExternalControlState );
			END_EVENT(true)

			EVENT(msg_combat_recovered)
				ME->GetAttackComponent()->CompleteRecovery();
			END_EVENT(true)

			EVENT(msg_combat_impaled)
				LOG("archer - msg_combat_impaled");
			END_EVENT(true)

			EVENT(msg_buttonattack)
			EVENT(msg_buttongrab)

			END_EVENT(true)

			EVENT(msg_buttondodge)
				if ( ME->GetAttackComponent()->StartNewAttack() )
					SET_STATE( CombatState );
			END_EVENT(true)

			EVENT(msg_buttonaction)
	
				// Make sure this player is not already manipulating something
				if ( ME->GetInteractionTarget() )
					END_EVENT(true)
				
				ME->SetInteractionTarget(ME->GetAwarenessComponent()->FindInteractionTarget(CUsePoint::ICT_Archer));
				
				// Did we get a target
				if ( ME->GetInteractionTarget() )
				{
						// new behaviour for the here - he/she controls to move point. This is transitional - once it has been tested for a couple-cases, it
					// can be generalised and ye olde behaviour done away with.
					if (ME->GetInteractionTargetUsePoint() && 
						ME->GetInteractionTargetUsePoint()->CharacterControlsToMoveToPoint())
					{
						ME->GetAttackComponent()->DisallowSwitchStance();
						SET_STATE( MovingToState );
					}
					else
					{
						Message msgAction(ME->GetInputComponent()->IsDirectionHeld() ? msg_running_action : msg_action);
	
						msgAction.SetEnt(CHashedString(HASH_STRING_OTHER), (CEntity*)ME);
						msgAction.SetEnt(CHashedString(HASH_STRING_SENDER), (CEntity*)ME);
						ME->GetInteractionTarget()->GetMessageHandler()->QueueMessage(msgAction);
	
						ME->GetAttackComponent()->DisallowSwitchStance();
						SET_STATE( InteractingState );
					}
				}

				// playerstate.lua has  --Check for a running pickup target
				// and					--Check for a pickup target
				// which are block commented	
			END_EVENT(true)

			EVENT(msg_release_power)
				LOG( "archer - msg_release_power" );
			END_EVENT(true)

			
			EVENT(msg_button_range)
				ME->ShootyMode( Archer::DEFAULT );
				SET_STATE(AimingState);
			END_EVENT(true)
			
			EVENT(msg_turret_point)
				ME->SetInteractionTarget(msg.GetEnt(CHashedString("Sender")));
				ME->ShootyMode( Archer::TURRET );
				SET_STATE(AimingState);
			END_EVENT(true)

			EVENT(msg_turret_cablecar)
				ME->SetInteractionTarget(msg.GetEnt("Sender"));
				ME->ShootyMode( Archer::TURRET_CALBECAR );
				SET_STATE(AimingState);
			END_EVENT(true)
			
			EVENT(msg_return_state)
				ME->SetReturnState(msg.GetHashedString("State"));
			END_EVENT(true)

		END_EVENTS
	END_STATE // DefaultState

	STATE(CombatState)
		BEGIN_EVENTS
			ON_ENTER
				//ntPrintf("Player: State PLAYER_COMBATSTATE ON_ENTER\n" );
				ntAssert(1);
			END_EVENT(true)

			EVENT(msg_external_control_start)
				SET_STATE( ExternalControlState );
			END_EVENT(true)

			/*
			EVENT(msg_combat_enemy_ko)
			{
				//ntPrintf("Player: State PLAYER_COMBATSTATE msg_combat_enemy_ko\n" );
				ME->SetInteractionTarget(msg.GetEnt(CHashedString(HASH_STRING_SENDER)));
				Message msgKO(msg_ko_aftertouch);	
				msgKO.SetEnt(CHashedString(HASH_STRING_SENDER), ME);
				ME->GetInteractionTarget()->GetMessageHandler()->QueueMessage(msgKO);
				SET_STATE( PLAYER_KOAFTERTOUCHCONTROLSTATE );
			}
			END_EVENT(true)
			*/

			EVENT(msg_combat_breakout)
				SET_STATE( DefaultState );
			END_EVENT(true)

			EVENT(msg_combat_recovered)
				ME->GetAttackComponent()->CompleteRecovery();
				ME->GetAttackComponent()->SetMovementFromCombatPoseFlag(true);
				SET_STATE( DefaultState );
			END_EVENT(true)

			EVENT(msg_buttongrab)
			EVENT(msg_buttondodge)
				ME->GetAttackComponent()->SelectNextAttack();
			END_EVENT(true)

			EVENT(msg_combat_struck)
				SET_STATE( ReactState );
			END_EVENT(true)

		/*
			EVENT(msg_button_special)
				ME->GetAttackComponent()->StartSpecial();
			END_EVENT(true)
		*/

		END_EVENTS
	END_STATE // PLAYER_COMBATSTATE

	STATE(VaultingState)
		BEGIN_EVENTS
			EVENT(msg_vault_completed)
				SET_STATE(DefaultState);
			END_EVENT(true)

			ON_ENTER
				ME->SetVaultingSafe();
				ME->UseSmallCapsule(true);
				ME->UseFootIK( false );
				ME->GetAttackComponent()->SetDisabled( true );
			END_EVENT(true)

			ON_EXIT
				ME->UseSmallCapsule(false);
				ME->VaultFinished();				
				ME->ResetToIdleState();
				ME->ClearVaultingSafe();
				ME->UseFootIK( true );
				ME->GetAttackComponent()->SetDisabled( false );
			END_EVENT(true)

			// Update the old archer component
			ON_UPDATE
				ME->UpdateAiming( CTimer::Get().GetSystemTimeChange() );
				ME->UpdateHealth( CTimer::Get().GetGameTimeChange() );
			END_EVENT(true)

			EVENT(msg_combat_struck)
				SET_STATE( ReactState );
			END_EVENT(true)
				
			EVENT(msg_external_control_start)
				SET_STATE(WaitForVaultComplete);
			END_EVENT(true)

			//-----------------------------------------------------
			// 1st person mode handling
			//-----------------------------------------------------
			EVENT(msg_button_range)
				SET_STATE(Enter1stPersonOnEnd);
			END_EVENT(true)

		END_EVENTS


		//-----------------------------------------------------
		// Wait for Vault sub-state
		//-----------------------------------------------------
		STATE(WaitForVaultComplete)
			BEGIN_EVENTS
				EVENT(msg_vault_completed)
					SET_STATE(ExternalControlState);
				END_EVENT(true)
			END_EVENTS
		END_STATE // WaitForVaultComplete

		//-----------------------------------------------------
		// 1st person mode sub-state
		//-----------------------------------------------------
		STATE(Enter1stPersonOnEnd)
			BEGIN_EVENTS
				EVENT(msg_vault_completed)
					ME->ShootyMode( Archer::DEFAULT );
					SET_STATE(AimingState);
				END_EVENT(true)
				
				EVENT(msg_release_range)
					POP_STATE();
				END_EVENT(true)
			END_EVENTS
		END_STATE //Enter1stPersonOnEnd

	END_STATE // VaultingState
	
	STATE(AimingState)
		BEGIN_EVENTS
			ON_ENTER
			{
				ME->Set1stPersonState();
				ME->RequestFire( false );
				ME->RequestReload( false );
				ME->AfterTouchState(false);
				
				Message message(msg_combat_state);
				ME->GetRangedWeapon()->GetMessageHandler()->QueueMessage(message);

				// Place the pad in to a deadzone and remember the current deadzone mode
				ME->EnableAimingPadDeadZone();

				// 
				switch( ME->ShootyMode() )
				{
					case Archer::DEFAULT:
						ME->LogEvent(AE_FIRSTPERSON);
						ME->GetMovement()->Lua_StartMovementFromXMLDef( "ArcherXbow_Aim_Movement" );
						break;

					case Archer::TURRET:
						ME->LogEvent(AE_MOUNT_TURRET);
						ME->GetMovement()->Lua_StartMovementFromXMLDef( "TurretPointAim_Movement" );
						ME->SetAimCamID(CamMan::Get().GetPrimaryView()->ActivateAimCam(ME, "TurretPointAimCamProps"));
						ME->GetPhysicsSystem()->Lua_SetCharacterControllerDoMovementAbsolutely(true);
						break;

					case Archer::TURRET_CALBECAR:
						ME->LogEvent(AE_MOUNT_TURRET);
						ME->GetMovement()->Lua_StartMovementFromXMLDef( "CableCarTurretMovement" );
						ME->SetAimCamID(CamMan::Get().GetPrimaryView()->ActivateAimCam(ME, "CableCarTurretCamProps"));
						break;
				};

			}
			END_EVENT(true)

			ON_EXIT
			{
				// Place the archer back in to an idle state. 
				ME->ResetToIdleState();
				
				// Restore the pad back to the system used before the aiming started
				ME->RestoreAimingPadDeadZone();

				// Send a message to the weapon that we've now finished the combat state. 
				Message message(msg_combat_exit);
				message.SetEnt( CHashedString(HASH_STRING_SENDER), ME);
				ME->GetRangedWeapon()->GetMessageHandler()->QueueMessage(message);

				// FIX ME - PLEASE DO NOT USE REMOVE ALL COOLCAMERAS! GRRRRRRRRRRRR!
				CamMan::GetPrimaryView()->RemoveAllCoolCameras(); 

				// kill the looping sound
				ME->GetEntityAudioChannel()->Stop(CHANNEL_ACTION_B);
				ME->GetEntityAudioChannel()->Stop(CHANNEL_ACTION_C);

				// Clear out the stored charges. 
				ME->ClearXbowArmCharges();

				// 
				switch( ME->ShootyMode() )
				{
					case Archer::DEFAULT:
						break;

					case Archer::TURRET:
					case Archer::TURRET_CALBECAR:
						// Log the dismount
						ME->LogEvent(AE_DISMOUNT_TURRET);

						// Reset the interaction target
						ME->SetInteractionTarget(0);
						break;
				};
			}
			END_EVENT(true)

			// Update the old archer component
			ON_UPDATE
			{
				float fCurrentCharge1 = ME->GetXbowArmCharge( 0 );
				float fCurrentCharge2 = ME->GetXbowArmCharge( 1 );
				
				ME->UpdateAiming( CTimer::Get().GetSystemTimeChange() );
				ME->UpdateHealth( CTimer::Get().GetGameTimeChange() );

				if( ME->GetSettings() )
				{
					// Has the charge up started?
					if( fCurrentCharge1 == 0.0f && ME->GetXbowArmCharge( 0 ) > 0.0f )
					{
						// ... If so, start the charge up sound
						ME->GetEntityAudioChannel()->Play(CHANNEL_ACTION_B, ME->GetSettings()->m_MainSoundBank, ME->GetSettings()->m_ChargeUpSFX);
					}
					else if( ME->GetXbowArmCharge( 0 ) >= 1.0f && fCurrentCharge1 < 1.0f )
					{
						// if the shot is at full charge - then play a shot charged sound
						ME->GetEntityAudioChannel()->Play(CHANNEL_ACTION_B, ME->GetSettings()->m_MainSoundBank, ME->GetSettings()->m_ChargedSFX);
					}

					// Has the charge up started?
					if( fCurrentCharge2 == 0.0f && ME->GetXbowArmCharge( 1 ) > 0.0f )
					{
						// ... If so, start the charge up sound
						ME->GetEntityAudioChannel()->Play(CHANNEL_ACTION_C, ME->GetSettings()->m_MainSoundBank, ME->GetSettings()->m_ChargeUpSFX);
					}
					else if( ME->GetXbowArmCharge( 1 ) >= 1.0f && fCurrentCharge2 < 1.0f )
					{
						// if the shot is at full charge - then play a shot charged sound
						ME->GetEntityAudioChannel()->Play(CHANNEL_ACTION_C, ME->GetSettings()->m_MainSoundBank, ME->GetSettings()->m_ChargedSFX);
					}
				}
			}
			END_EVENT(true)

			EVENT(msg_combat_recovered)
				ME->RequestFire( false );
				ME->RequestReload( false );
				ME->AfterTouchState(false);
				ME->GetAttackComponent()->CompleteRecovery();
				ME->Set1stPersonState();

				// Based on the shooting style.. 
				switch( ME->ShootyMode() )
				{
					case Archer::DEFAULT:
						ME->GetMovement()->Lua_StartMovementFromXMLDef( "CrossbowAim_Movement" );
						break;

					case Archer::TURRET:
						ME->GetMovement()->Lua_StartMovementFromXMLDef( "TurretPointAim_Movement" );
						break;

					case Archer::TURRET_CALBECAR:
						ME->GetMovement()->Lua_StartMovementFromXMLDef( "CableCarTurretMovement" );
						break;
				};
			END_EVENT(true)

			EVENT(msg_combat_struck)
				// If struck and in a KO reaction - go into a react state, else, just ignore it
				SET_STATE(ReactState);
			END_EVENT(true)

			EVENT(msg_external_control_start)
				SET_STATE(ExternalControlState);
			END_EVENT(true)
			
			EVENT(msg_cut_the_stan_cam)

				// Only process if the current shooty mode is default. 
				if( ME->ShootyMode() == Archer::DEFAULT )
				{
					ME->SetAimCamID(CamMan::Get().GetPrimaryView()->ActivateAimCam(ME, "FirstPersonAimCamProps"));
				}

			END_EVENT(true)

			// Override the dodge so in first person evades don't work.
			EVENT(msg_buttondodge)
			END_EVENT(true)

			// If asked to exit the current state...
			EVENT(msg_exitstate)
				ME->GetPhysicsSystem()->Lua_CharacterController_SetRagdollCollidable(true);
				SET_STATE(DefaultState);
			END_EVENT(true)

			EVENT(msg_release_range)
			{
				// Only perform the release range code if in a normal turrent mode. 
				if( ME->ShootyMode() == Archer::DEFAULT )
				{
					// Only if not trying to run. 
					if( ME->GetInputComponent()->GetInputSpeed() < 0.2f )
					{
						// As the archer is leaving her 1st person state in a controlled manor, then 
						// play an animation to return her to normal idle
						ME->AddChainedController( true );

						// 
						bool bCrouchRequired = ME->IsCrouching();
						const char* pcIdleMovement = bCrouchRequired ? "ArcherBackToCrouchIdle" : "ArcherBackToStdIdle";

						// Get a transition that will do its best to put us in the best movement state
						MovementControllerDef* pobDefintion = ObjectDatabase::Get().GetPointerFromName<MovementControllerDef*>(pcIdleMovement);
						user_error_p(pobDefintion, ("Error! Movement definition %s not found\n", pcIdleMovement));

						// Push an instance of the controller on to our movement component
						ME->GetMovement()->BringInNewController(*pobDefintion, CMovement::DMM_STANDARD, 0.0f );
					}

					// When coming out of the aiming state - don't allow for any blend in. 
					// This produces a much cleaner pop out. 
					ME->WalkRunBlendInTime( 0.0f );
					SET_STATE(DefaultState);
				}
			}
			END_EVENT(true)

			EVENT(msg_combat_attack)
			END_EVENT(true)
			
			EVENT(msg_combat_firing_xbow)
				// Kill the sound
				ME->GetEntityAudioChannel()->Stop(CHANNEL_ACTION_B);
				ME->GetEntityAudioChannel()->Stop(CHANNEL_ACTION_C);

				ME->LogEvent(AE_FIRSTPERSON_FIRE);
				//LOG("**************playing fire sound *****************");
				ME->GetEntityAudioChannel()->Play(CHANNEL_ACTION, "char_sb", "arch_xbow_trigger");

				SET_STATE(Firing);
			END_EVENT(true)
		END_EVENTS

		//-----------------------------------------------------
		// Firing sub-state
		//-----------------------------------------------------
		STATE(Firing)
			BEGIN_EVENTS
				ON_ENTER
				{
					ME->AfterTouchEnabled( false );

					Message message(msg_enter_aftertouchcheck);
					ME->GetMessageHandler()->QueueMessageDelayed(message, ME->GetAfterTouchStartDelay());
					//
				}
				END_EVENT(true)
		
				ON_EXIT
				{
					ME->GetAttackComponent()->SetDisabled( false );

					Message message(msg_combat_endaftertouch);
					message.SetUnnamedParams();

					if( msg.IsPtr("target") )
						message.AddParam(msg.GetEnt("target"));

					ME->GetRangedWeapon()->GetMessageHandler()->QueueMessage(message);
					
					if( ME->AfterTouchEnabled() )
					{
						ME->GetMovement()->SetEnabled( true );
						ME->AfterTouchEnabled( false );
					}
				}
				END_EVENT(true)
		
				// Update the old archer component
				ON_UPDATE
					ME->UpdateAiming( CTimer::Get().GetSystemTimeChange() );
					ME->UpdateHealth( CTimer::Get().GetGameTimeChange() );
				END_EVENT(true)

				EVENT(msg_enter_aftertouchcheck)
					
					if( ME->Lua_IsAttackHeld( ME->GetAfterTouchStartDelay() ))
					{
						ME->GetAttackComponent()->SetDisabled( true );
						ME->LogEvent(AE_AFTERTOUCH);
						Message message(msg_combat_startaftertouch);
						message.SetUnnamedParams();
						message.AddParam(msg.GetEnt("target"));
						ME->GetRangedWeapon()->GetMessageHandler()->QueueMessage(message);
						ME->GetMovement()->SetEnabled( false );
						ME->AfterTouchEnabled( true );
					}

				END_EVENT(true)
				
				EVENT(msg_aftertouch_done)
					POP_STATE();
				END_EVENT(true)
				
    			EVENT(msg_release_attack)
    				POP_STATE();
    			END_EVENT(true)
			END_EVENTS
		END_STATE // Firing
	END_STATE //Aiming_1st

	STATE(ReactState)
		BEGIN_EVENTS
			ON_ENTER
				ME->GetAttackComponent()->Lua_SendRecoilMessage();
				ME->GetPhysicsSystem()->Lua_SetHoldingCapsule(false);
			END_EVENT(true)

			// Update the old archer component
			ON_UPDATE
				ME->UpdateAiming( CTimer::Get().GetSystemTimeChange() );
				ME->UpdateHealth( CTimer::Get().GetGameTimeChange() );
			END_EVENT(true)

			EVENT(msg_external_control_start)
				SET_STATE(ExternalControlState);
			END_EVENT(true)

			EVENT(msg_combat_impaled)
				//SET_STATE( PLAYER_IMPALEDSTATE );
			END_EVENT(true)

			EVENT(msg_combat_breakout)
				SET_STATE( DefaultState );
			END_EVENT(true)

			EVENT(msg_combat_recovered)
				ME->GetAttackComponent()->CompleteRecovery();
				SET_STATE( DefaultState );
			END_EVENT(true)

			EVENT(msg_combatsyncdreactionend)
				ME->GetAttackComponent()->EndSyncdReaction();
			END_EVENT(true)

			EVENT(msg_combat_countered)
				SET_STATE( DefaultState );
			END_EVENT(true)

			EVENT(msg_combat_floored)
				ME->HelperGetLuaFunc( CHashedString("GameStatistics"), CHashedString("Event") )( CHashedString("PlayerKO") );
				CChatterBoxMan::Get().Trigger("PlayerKO", ME);

				ME->GetAttackComponent()->StartFlooredState();
			END_EVENT(true)

			//-----------------------------------------------------
			// 1st person mode handling
			//-----------------------------------------------------
			EVENT(msg_button_range)
				SET_STATE(Enter1stPersonOnEnd);
			END_EVENT(true)

		END_EVENTS

		//-----------------------------------------------------
		// 1st person mode sub-state
		//-----------------------------------------------------
		STATE(Enter1stPersonOnEnd)
			BEGIN_EVENTS
				EVENT(msg_vault_completed)
					ME->ShootyMode( Archer::DEFAULT );
					SET_STATE(AimingState);
				END_EVENT(true)
				
				EVENT(msg_release_range)
					POP_STATE();
				END_EVENT(true)

				EVENT(msg_combat_breakout)
					ME->ShootyMode( Archer::DEFAULT );
					SET_STATE(AimingState);
				END_EVENT(true)

				EVENT(msg_combat_recovered)
					ME->GetAttackComponent()->CompleteRecovery();
					ME->ShootyMode( Archer::DEFAULT );
					SET_STATE(AimingState);
				END_EVENT(true)
			END_EVENTS
		END_STATE //Enter1stPersonOnEnd
	END_STATE // ReactState

	STATE(ExternalControlState)
		BEGIN_EVENTS
			ON_ENTER
				ME->GetAttackComponent()->SetTargetingDisabled(true);
				ME->GetMovement()->SetInputDisabled(true);

				ME->GetPhysicsSystem()->Lua_SetCharacterControllerDoMovementAbsolutely(true);
			END_EVENT(true)

			ON_EXIT
				ME->SetReturnState("DefaultState");
			END_EVENT(true)

			// Update the old archer component
			ON_UPDATE
				ME->UpdateAiming( CTimer::Get().GetSystemTimeChange() );
				ME->UpdateHealth( CTimer::Get().GetGameTimeChange() );
			END_EVENT(true)


			EVENT(msg_external_control_end)
			{
				ME->GetAttackComponent()->SetTargetingDisabled(false);
				ME->GetMovement()->SetInputDisabled(false);

				ME->GetPhysicsSystem()->Lua_SetCharacterControllerDoMovementAbsolutely(false);
				CHashedString sState = ME->GetReturnState();
				((ARCHER_FSM&)FSM).JumpToState(sState);
			}
			END_EVENT(true)

			EVENT(msg_turret_point)
				ME->SetInteractionTarget(msg.GetEnt("Sender"));
				ME->ShootyMode( Archer::TURRET );
				SET_STATE(AimingState);
			END_EVENT(true)

			EVENT(msg_turret_cablecar)
				ME->SetInteractionTarget(msg.GetEnt("Sender"));
				ME->ShootyMode( Archer::TURRET_CALBECAR );
				SET_STATE(AimingState);
			END_EVENT(true)

			EVENT(msg_return_state)
				ME->SetReturnState(msg.GetHashedString("State"));
			END_EVENT(true)

		END_EVENTS
	END_STATE // ExternalControlState

	STATE(DeadState)
		BEGIN_EVENTS
			ON_ENTER
				// Attempt to notify the HUD
				if ( CHud::Exists() && CHud::Get().GetFailureHud() )
				{
					CHud::Get().GetFailureHud()->NotifyFailure("FAILURE_KILLED");
				}

				CLuaGlobal::CallLuaFunc("OnArcherDeath");
			END_EVENT(true)
		END_EVENTS
	END_STATE // DeadState

//-------------------- New Move to State ----------//

	STATE(MovingToState)
		BEGIN_EVENTS
			ON_ENTER
			{
				ME->GetAttackComponent()->DisallowSwitchStance();
				if (! ME->GetInteractionTarget() )
				{
					LOG("Error in Archer_MoveToState: m_pobInteractionTarget is null!!");
					SET_STATE( DefaultState );
				}

				const bool bWantsRun = ME->GetInputComponent()->IsDirectionHeld();
				if (ME->SetOffNavigationToUsePoint(Character::CT_Player, bWantsRun))
				{
					ME->GetMovement()->Lua_AltSetMovementCompleteMessage( bWantsRun? "msg_run_moveto_done":"msg_walk_moveto_done", ME );
					ME->GetInteractionTarget()->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);
				}
				else
				{
					// gets here if SetOffNavigationToUsePoint fails for some reason
					// shouldn't really get here, in other words
					Message msgInterrupt(bWantsRun? msg_run_moveto_done: msg_walk_moveto_done);	
					msgInterrupt.SetEnt(CHashedString(HASH_STRING_SENDER), ME);
					ME->GetMessageHandler()->QueueMessage(msgInterrupt);
				}
				
			}
			END_EVENT(true)

			EVENT(msg_combat_struck)
			{
				Message msgInterrupt(msg_interrupt);	
				msgInterrupt.SetEnt(CHashedString(HASH_STRING_SENDER), (CEntity*)ME);
				ME->GetInteractionTarget()->GetMessageHandler()->QueueMessage(msgInterrupt);

				CamMan::GetPrimaryView()->RemoveAllCoolCameras();
				ME->SetInteractionTarget(0);
				SET_STATE( ReactState );
			}
			END_EVENT(true)

			EVENT(msg_exitstate)
				// FIX ME note in Lua --CHECKME
				ME->SetInteractionTarget(0);
				ME->GetPhysicsSystem()->Lua_CharacterController_SetRagdollCollidable(true);
				SET_STATE( DefaultState );
			END_EVENT(true)

			EVENT(msg_walk_moveto_done) 
			{
				Message msgActionSpecific(msg_action_specific);	
				msgActionSpecific.SetEnt(CHashedString(HASH_STRING_OTHER), (CEntity*)ME);
				msgActionSpecific.SetEnt(CHashedString(HASH_STRING_SENDER), (CEntity*)ME);
				ME->GetInteractionTarget()->GetMessageHandler()->QueueMessage(msgActionSpecific);
				SET_STATE( InteractingState );
			}
			END_EVENT(true)

			EVENT(msg_run_moveto_done) 
			{
				Message msgActionSpecificRun(msg_action_specific_run);	
				msgActionSpecificRun.SetEnt(CHashedString(HASH_STRING_OTHER), (CEntity*)ME);
				msgActionSpecificRun.SetEnt(CHashedString(HASH_STRING_SENDER), (CEntity*)ME);
				ME->GetInteractionTarget()->GetMessageHandler()->QueueMessage(msgActionSpecificRun);
				SET_STATE( InteractingState );
			}
			END_EVENT(true)

			EVENT(msg_external_control_start)
			{
				Message msgInterrupt(msg_interrupt);	
				msgInterrupt.SetEnt(CHashedString(HASH_STRING_SENDER), ME);
				ME->GetInteractionTarget()->GetMessageHandler()->QueueMessage(msgInterrupt);

				CamMan::GetPrimaryView()->RemoveAllCoolCameras();
				ME->SetInteractionTarget(0);

				SET_STATE( ExternalControlState );
			}
			END_EVENT(true)

			// Note: This stuff should probably be in the interaction component at some point
			EVENT(msg_aim_on)			ME->ForwardInteractionMsg(msg_aim_on);			END_EVENT(true)
			EVENT(msg_buttonattack)		ME->ForwardInteractionMsg(msg_attack_on);		END_EVENT(true)
			EVENT(msg_buttongrab)		ME->ForwardInteractionMsg(msg_grab_on);			END_EVENT(true)
			EVENT(msg_buttonaction)		ME->ForwardInteractionMsg(msg_action_on);		END_EVENT(true)
			EVENT(msg_button_power)		ME->ForwardInteractionMsg(msg_power_on);		END_EVENT(true)
			EVENT(msg_button_range)		ME->ForwardInteractionMsg(msg_range_on);		END_EVENT(true)
			EVENT(msg_release_attack)	ME->ForwardInteractionMsg(msg_attack_off);		END_EVENT(true)
			EVENT(msg_release_grab)		ME->ForwardInteractionMsg(msg_grab_off);		END_EVENT(true)
			EVENT(msg_aim_off)			ME->ForwardInteractionMsg(msg_aim_off);			END_EVENT(true)
			EVENT(msg_release_action)	ME->ForwardInteractionMsg(msg_action_off);		END_EVENT(true)
			EVENT(msg_release_power)	ME->ForwardInteractionMsg(msg_power_off);		END_EVENT(true)
			EVENT(msg_release_range)	ME->ForwardInteractionMsg(msg_range_off);		END_EVENT(true)

			EVENT(msg_turret_point)
				ME->SetInteractionTarget(msg.GetEnt("Sender"));
				ME->ShootyMode( Archer::TURRET );
				SET_STATE(AimingState);
			END_EVENT(true)
			
			EVENT(msg_turret_cablecar)
				ME->SetInteractionTarget(msg.GetEnt("Sender"));
				ME->ShootyMode( Archer::TURRET_CALBECAR );
				SET_STATE(AimingState);
			END_EVENT(true)

			EVENT(msg_return_state)
				ME->SetReturnState(msg.GetHashedString("State"));
			END_EVENT(true)

		END_EVENTS
	END_STATE // MoveToState

	STATE(InteractingState)
		BEGIN_EVENTS
			ON_ENTER
				ME->SetExitOnMovementDone(false);
					
				if (! ME->GetInteractionTarget() )
				{
					LOG("Error in Player_State_Interacting: m_pobInteractionTarget is null!!");
					SET_STATE( DefaultState );
				}
			END_EVENT(true)

			// Update the old archer component
			ON_UPDATE
				ME->UpdateAiming( CTimer::Get().GetSystemTimeChange() );
				ME->UpdateHealth( CTimer::Get().GetGameTimeChange() );
			END_EVENT(true)


			EVENT(msg_combat_struck)
			{
				Message msgInterrupt(msg_interrupt);	
				msgInterrupt.SetEnt(CHashedString(HASH_STRING_SENDER), (CEntity*)ME);
				ME->GetInteractionTarget()->GetMessageHandler()->QueueMessage(msgInterrupt);

				CamMan::GetPrimaryView()->RemoveAllCoolCameras();
				ME->SetInteractionTarget(0);
				SET_STATE( ReactState );
			}
			END_EVENT(true)
				
			EVENT(msg_exitstate)
				// FIX ME note in Lua --CHECKME
				ME->SetInteractionTarget(0);
				ME->GetPhysicsSystem()->Lua_CharacterController_SetRagdollCollidable(true);
				SET_STATE( DefaultState );
			END_EVENT(true)

			EVENT(msg_movementdone) // FIX ME note in Lua --CHECKME
				if ( ME->ExitOnMovementDone() )
				{
					CamMan::GetPrimaryView()->RemoveAllCoolCameras();
					ME->SetInteractionTarget(0);
					SET_STATE( DefaultState );
				}
				else
				{
					Message msgInterrupt(msg_movementdone);	
					msgInterrupt.SetEnt(CHashedString(HASH_STRING_SENDER), (CEntity*)ME);
					ME->GetInteractionTarget()->GetMessageHandler()->QueueMessage(msgInterrupt);
				}
			END_EVENT(true)

			EVENT(msg_external_control_start)
			{
				Message msgInterrupt(msg_interrupt);	
				msgInterrupt.SetEnt(CHashedString(HASH_STRING_SENDER), ME);
				ME->GetInteractionTarget()->GetMessageHandler()->QueueMessage(msgInterrupt);

				CamMan::GetPrimaryView()->RemoveAllCoolCameras();
				ME->SetInteractionTarget(0);

				SET_STATE( ExternalControlState );
			}
			END_EVENT(true)

			// Note: This stuff should probably be in the interaction component at some point
			EVENT(msg_aim_on)			ME->ForwardInteractionMsg(msg_aim_on);		END_EVENT(true)
			EVENT(msg_buttonattack)		ME->ForwardInteractionMsg(msg_attack_on);		END_EVENT(true)
			EVENT(msg_buttongrab)		ME->ForwardInteractionMsg(msg_grab_on);		END_EVENT(true)
			EVENT(msg_buttonaction)		ME->ForwardInteractionMsg(msg_action_on);		END_EVENT(true)
			EVENT(msg_button_power)		ME->ForwardInteractionMsg(msg_power_on);		END_EVENT(true)
			EVENT(msg_button_range)		ME->ForwardInteractionMsg(msg_range_on);		END_EVENT(true)
			EVENT(msg_release_attack)	ME->ForwardInteractionMsg(msg_attack_off);	END_EVENT(true)
			EVENT(msg_release_grab)		ME->ForwardInteractionMsg(msg_grab_off);		END_EVENT(true)
			EVENT(msg_aim_off)			ME->ForwardInteractionMsg(msg_aim_off);		END_EVENT(true)
			EVENT(msg_release_action)	ME->ForwardInteractionMsg(msg_action_off);	END_EVENT(true)
			EVENT(msg_release_power)	ME->ForwardInteractionMsg(msg_power_off);		END_EVENT(true)
			EVENT(msg_release_range)	ME->ForwardInteractionMsg(msg_range_off);		END_EVENT(true)


			EVENT(msg_turret_point)
				ME->SetInteractionTarget(msg.GetEnt("Sender"));
				ME->ShootyMode( Archer::TURRET );
				SET_STATE(AimingState);
			END_EVENT(true)

			EVENT(msg_turret_cablecar)
				ME->SetInteractionTarget(msg.GetEnt("Sender"));
				ME->ShootyMode( Archer::TURRET_CALBECAR );
				SET_STATE(AimingState);
			END_EVENT(true)

			
			EVENT(msg_return_state)
				ME->SetReturnState(msg.GetHashedString("State"));
			END_EVENT(true)
		END_EVENTS
	END_STATE // InteractingState
END_STATEMACHINE //HERO_FSM

//------------------------------------------------------------------------------------------
//!
//!	Archer::ForwardInteractionMsg()
//!	forward msg when in interacting state
//! FIX ME - probably shared by all characters
//------------------------------------------------------------------------------------------
void Archer::ForwardInteractionMsg(MessageID id)
{
	Message msgFwd(id);
	msgFwd.SetEnt(CHashedString(HASH_STRING_SENDER), (CEntity*)this);
	GetInteractionTarget()->GetMessageHandler()->QueueMessage(msgFwd);
}

//------------------------------------------------------------------------------------------
//!
//!	Archer::Lua_LogEvent
//!
//------------------------------------------------------------------------------------------
void Archer::LogEvent(ArcherEventType eArcherEvent) const
{
	Archer* pSelf = const_cast<Archer*>(this);
	pSelf->m_obEventLogManager.AddEvent(eArcherEvent, 0, 0);
}

//------------------------------------------------------------------------------------------
//!  public constant  Lua_CreateBolt
//!
//!
//!  @remarks Creates a crossbow bolt for the archers xbow
//!
//!  @author GavB @date 26/07/2006
//------------------------------------------------------------------------------------------
CEntity* Archer::Lua_CreateBolt(CEntity* pXbow, CEntity* pTarget, const CDirection& dirShot, float fCharge) const
{
	Archer* pSelf = const_cast<Archer*>(this);

	// 
	DataObject* pDataObject = ObjectDatabase::Get().GetDataObjectFromName( CHashedString( "Att_ArcherRangedProjectile_CrossbowBolt" ) );
	ntAssert( pDataObject && "This is only a temp solution" );

	// 
	if( CHashedString( pDataObject->GetClassName() ) == CHashedString("Projectile_Attributes" ) )
	{
		Projectile_Attributes* pProjectile = (Projectile_Attributes*) pDataObject->GetBasePtr();
		CEntity* pNewBolt = Object_Projectile::CreateCrossbowBolt(pXbow, (CEntity*)pSelf, pProjectile, pTarget, false, &dirShot, fCharge );

		// Find the physics system attached to the bolt
		Physics::ProjectileLG* pBoltPhysics = (Physics::ProjectileLG*) pNewBolt->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::PROJECTILE_LG );

		// If there is a bolt then it's possible to apply the amount of charge to the initial speed
		if( pBoltPhysics )
		{
			static const float CHARGE_SPEED_SCALAR_SHOULD_BE_IN_XML = 10.0f;
			pBoltPhysics->AddToInitialSpeed( fCharge * CHARGE_SPEED_SCALAR_SHOULD_BE_IN_XML );
		}

		// 
		return pNewBolt;
	}

	return 0;
}


//------------------------------------------------------------------------------------------
//!
//!	Archer::Archer()
//!	Default constructor
//!
//------------------------------------------------------------------------------------------
Archer::Archer()
:	m_iAimCamID(-1),
	m_obEventLogManager( ),
	m_iLastVaultAnimPlayed(0),
	m_bVaultingSafe(false),
	m_bCanCrouch(true),
	m_bAlwaysCrouch(false),
	m_bAfterTouchEnabled(false),
	m_bCurrentlyCrouching(false),
	m_fCrouchVolumeHeight(0.6f),
	m_fCrouchVolumeHeight2(0.15f),
	m_fWalkRunBlendInTime(CMovement::MOVEMENT_BLEND),
	m_bAddNewController( false )
{
	m_bArcher = true;
	m_obEventLogManager.SetParent( this );
	ATTACH_LUA_INTERFACE(Archer);
	
	// Clear out the charges on the xbow arms. 
	m_afXbowArmCharges[0]	= 0.0f;
	m_afXbowArmCharges[1]	= 0.0f;

	// Set the firing charge to a negative number - other systems peek at this value 
	// for a normalised range to indicate the xbow firing. 
	m_fFiringCharge			= -1.0f;
}

//------------------------------------------------------------------------------------------
//!
//!	Archer::~Archer()
//!	Default destructor
//!
//------------------------------------------------------------------------------------------
Archer::~Archer()
{

}

//------------------------------------------------------------------------------------------
//!
//!	Archer::OnPostConstruct()
//!	Post construction
//!	FIX ME A large amount of this function can be moved into the base classes
//! as more characters are moved from lua to C++
//------------------------------------------------------------------------------------------
void Archer::OnPostConstruct()
{
	Player::OnPostConstruct();

	// FIX ME - move to player base
	PAD_NUMBER ePad = PAD_0;
	switch ((int)m_obAttributeTable->GetNumber( CHashedString(HASH_STRING_PAD) ))
	{
	case 0:			ePad = PAD_0;		break;
	case 1:			ePad = PAD_1;		break;
	case 2:			ePad = PAD_2;		break;
	case 3:			ePad = PAD_3;		break;
	default:		ntAssert(0);		break;
	} // switch ((int)m_obAttributeTable->GetNumber("Pad"))
	
	SetInputComponent(NT_NEW_CHUNK(Mem::MC_ENTITY) CInputComponent(this, ePad) );

	// FIX ME - Note this will be specific for the character
	// Player specific components - FIX ME magic numbers ick
	AimingComponent* pobAimingComponent = NT_NEW_CHUNK(Mem::MC_ENTITY) AimingComponent(this,60.0f,60.0f,-20.0f,20.0f);
	SetAimingComponent(pobAimingComponent);

	// This is used for interaction targeting
	GetInteractionComponent()->SetEntityHeight(1.5f);

	if(m_obInitialSystemState == "" || ntStr::IsNull(m_obInitialSystemState) )
		m_obInitialSystemState = CHashedString("DefaultState");
	
	// Double check movment controllers
	if ( m_obWalkingController.IsNull() )
	{
		m_obWalkingController = CHashedString("ArcherWalkRun");
	}

	// Double check movment controllers
	if ( m_obCrouchedController.IsNull() )
	{ 
		m_obCrouchedController = CHashedString("CrouchedArcherWalkRun");
	}

	// Hero should register combat log for the style system
	if ( StyleManager::Exists() )
	{
		ntAssert( StyleManager::Get().GetArcherHitCounter() );
		GetAttackComponent()->RegisterCombatEventLog( StyleManager::Get().GetArcherHitCounter()->GetCombatEventLog() );
	}

	// Pick up the default settings for the archer
	m_pSettings = ObjectDatabase::Get().GetPointerFromName<ArcherSettings*>("ArcherDefaultSettings");
}

//--------------------------------------------------
//!
//!	Archer::OnLevelStart()
//!	Called for each ent on level startup
//!
//--------------------------------------------------
void Archer::OnLevelStart()
{
	// Create and attach the statemachine. Must be done AFTER anim containers fixed up by area system
	// i.e. after XML serialisation. OR this shouldnt play an animation
	ARCHER_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) ARCHER_FSM(m_obInitialSystemState);
	ATTACH_FSM(pFSM);
}

//------------------------------------------------------------------------------------------
//!
//!	Archer::OnPostConstruct()
//!	Post construction
//!
//------------------------------------------------------------------------------------------
void Archer::OnPostPostConstruct()
{
	// Call the base postpost construction on the entity first
	Player::OnPostPostConstruct();
}


//------------------------------------------------------------------------------------------
//!
//! @function		Archer::VaultFinished
//!
//------------------------------------------------------------------------------------------
void Archer::VaultFinished()
{
	if ( m_State == ARC_VAULTING )
	{
		m_State = ARC_IDLE;
	}
}

//------------------------------------------------------------------------------------------
//!  public  GetTargetList
//!
//!  @param [in]       rRefMatrix const CMatrix &    
//!  @param [in, out]  obDestList ntstd::List<CEntity *> &    
//!
//!
//!  @author GavB @date 20/06/2006
//------------------------------------------------------------------------------------------
void ThirdPersonTargeting::GetTargetList( const CMatrix& rmatRef, ntstd::List<CEntity*>& obEntList ) const
{
	// For each segement, find all the entities that lay within its bounds.
	for( ntstd::List<ThirdPersonTargetingSegment*>::const_iterator obIt( m_SegmentList.begin() ); obIt != m_SegmentList.end(); ++obIt )
	{
		// Cache a pointer to the segment
		const ThirdPersonTargetingSegment* pSegment = *obIt;
		
#ifdef ARCHER_DEBUG_RENDER
		// Render the arc
		g_VisualDebug->RenderArc(	rmatRef,
									pSegment->m_fRadius, 
									pSegment->m_fAngle * DEG_TO_RAD_VALUE, 
									pSegment->m_vDebugColour.GetNTColor() );
#endif // ARCHER_DEBUG_RENDER

		// Create a query to obtain the entities.. 
		CEntityQuery obQuery;

		// The entities should at least be alive. 
		CEQCHealthLTE obHealth(0.0f);
		obQuery.AddUnClause( obHealth );

		// Create a segment volume to check within
		CEQCProximitySegment obSeg;
		obSeg.SetRadius( pSegment->m_fRadius );
		obSeg.SetAngle( pSegment->m_fAngle * DEG_TO_RAD_VALUE );

		/// Copy the matrix. 
		CMatrix matSeg = rmatRef;

		// Add the matrix and set the clause
		obSeg.SetMatrix( matSeg );
		obQuery.AddClause( obSeg );

		// Make sure that we're only going for entities that can targeted by the player. 
		CEQCIsTargetableByPlayer obPlayerTargetable;
		obQuery.AddClause( obPlayerTargetable );

		// Ranged weapons shouldn't target entities across the other side of the plaent
		CEQCIsEntityInArea obAreaCheck( AreaManager::Get().GetCurrActiveArea() );
		obQuery.AddClause( obAreaCheck );

		
		// Find all the AI entities that match our needs
		CEntityManager::Get().FindEntitiesByType( obQuery, CEntity::EntType_AI );

		// Add entities into the list
		obEntList.assign( obQuery.GetResults().begin(), obQuery.GetResults().end() );
	}
}


//------------------------------------------------------------------------------------------
//!  public constant  IsActiveTargetValid
//!
//!  @param [in]       rmatRef const CMatrix &    
//!  @param [in]       pActiveEnt const CEntity *    
//!
//!  @return bool 
//!
//!  @author GavB @date 20/06/2006
//------------------------------------------------------------------------------------------
bool ThirdPersonTargeting::IsActiveTargetValid( const CMatrix& rmatRef, const CEntity* pActiveEnt ) const
{
	// For each segement, find all the entities that lay within its bounds.
	for( ntstd::List<ThirdPersonTargetingSegment*>::const_iterator obIt( m_ActiveTargetSegmentList.begin() ); obIt != m_ActiveTargetSegmentList.end(); ++obIt )
	{
		// Cache a pointer to the segment
		const ThirdPersonTargetingSegment* pSegment = *obIt;
		
#ifdef ARCHER_DEBUG_RENDER
		// Render the arc
		g_VisualDebug->RenderArc(	rmatRef, 
									pSegment->m_fRadius, 
									pSegment->m_fAngle * DEG_TO_RAD_VALUE, 
									pSegment->m_vDebugColour.GetNTColor() );
#endif // ARCHER_DEBUG_RENDER

		// Create a query to obtain the entities.. 
		CEntityQuery obQuery;

		// Create a segment volume to check within
		CEQCProximitySegment obSeg;
		obSeg.SetRadius( pSegment->m_fRadius );
		obSeg.SetAngle( pSegment->m_fAngle * DEG_TO_RAD_VALUE );

		/// Copy the matrix. 
		CMatrix matSeg = rmatRef;

		// Add the matrix and set the clause
		obSeg.SetMatrix( matSeg );
		obQuery.AddClause( obSeg );

		// Make sure that we're only going for entities that can targeted by the player. 
		CEQCIsTargetableByPlayer obPlayerTargetable;
		obQuery.AddClause( obPlayerTargetable );

		CEQCIsThis obActiveEntCheck( pActiveEnt );
		obQuery.AddClause( obActiveEntCheck );

		// Find all the AI entities that match our needs
		CEntityManager::Get().FindEntitiesByType( obQuery, CEntity::EntType_AI );

		// Add entities into the list
		if( obQuery.GetResults().size() )
			return true;
	}


	return false;
}

//------------------------------------------------------------------------------------------
//!  public virtual  ModifyPhysicsHoldingShape
//!	
//!  @param [in, out]  rptA CPoint &		Top part to the capsule
//!  @param [in, out]  rptB CPoint &		Bottom part to the capsule
//!  @param [in, out]  fRadius float &		Radius of the capsule
//!
//!
//!  @remarks <TODO: insert remarks here>
//!
//!  @author GavB @date 25/09/2006
//------------------------------------------------------------------------------------------
void Archer::ModifyPhysicsHoldingShape( CPoint& rptA, CPoint& rptB, float& fRadius )
{
	// These "magic" values should really be taken from data... But due to the 
	rptA	= CPoint( 0.0f, -m_fCrouchVolumeHeight2, 0.0f );
	rptB	= CPoint( 0.0f, -m_fCrouchVolumeHeight, 0.0f );
	fRadius = 0.2f;
}

//------------------------------------------------------------------------------------------
//!  public constant  UseSmallCapsule
//!
//!  @param [in]        bool    
//!
//!  @remarks 
//!
//!  @author GavB @date 25/09/2006
//------------------------------------------------------------------------------------------
void Archer::UseSmallCapsule(bool bState) const
{
	Physics::AdvancedCharacterController* pobCharacterState = (Physics::AdvancedCharacterController*) GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);

	if(pobCharacterState)
		pobCharacterState->SetCharacterControllerHoldingCapsule( bState );
}

//------------------------------------------------------------------------------------------
//!  public constant  UseFootIK
//!
//!  @param [in]        bool    
//!
//!
//!  @remarks 
//!
//!  @author GavB @date 03/10/2006
//------------------------------------------------------------------------------------------
void Archer::UseFootIK(bool bState) const
{
	Physics::AdvancedCharacterController* pobCharacterState = (Physics::AdvancedCharacterController*) GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);

	if(pobCharacterState)
		pobCharacterState->FootIKEnabled( bState );
}

	
//------------------------------------------------------------------------------------------
//!  public virtual  Update1stFiring
//!
//!  @param [in]        float TimeStep
//!
//!  
//!
//!  @author GavB @date 05/12/2006
//------------------------------------------------------------------------------------------
void Archer::Update1stFiring( float fTimeStep )
{
	// Update the base 1st person core code. 
	Player::Update1stFiring( fTimeStep );

	// Update the charge given to the xbow arm	
	m_afXbowArmCharges[0] += m_afXbowArmCharges[0] < 0.0f ? fTimeStep : fTimeStep / m_pSettings->m_fChargeUpTime;
	m_afXbowArmCharges[1] += m_afXbowArmCharges[1] < 0.0f ? fTimeStep : fTimeStep / m_pSettings->m_fChargeUpTime;

	// Check if there is a fire requesst, then...
	if( m_bFireRequest )
	{
		// Can the archers xbow handle the request
		bool bSuper = SuperShot();

		if( bSuper ) 
		{
			// Save the charge for the xbow arm that'll be firing. 
			m_fFiringCharge = 1.0f;

			// Clear the arm charge back down to XML defined variable. 
			m_afXbowArmCharges[0] = m_afXbowArmCharges[1] = -m_pSettings->m_fReloadTime;
		}
		
		else 
		{
			int iBestArm = m_afXbowArmCharges[0] > m_afXbowArmCharges[1] ? 0 : 1;

			// Only process the fire request if there enough charge to fire. 
			if( m_afXbowArmCharges[iBestArm] > m_pSettings->m_fDenyControlUntilCharge )
			{
				// Save the charge for the xbow arm that'll be firing and make sure the power never reaches 1.0f. 
				m_fFiringCharge = clamp( m_afXbowArmCharges[iBestArm], 0.0f, 0.9f );

				// Clear the arm charge back down to XML defined variable. 
				m_afXbowArmCharges[iBestArm] = -m_pSettings->m_fReloadTime;
			}
			else
			{
				// Clear the fire request as it's now being processed. 
				m_bFireRequest = false;
			}
		}
	}
}

