//------------------------------------------------------------------------------------------
//!
//!	\file game/entityhero.cpp
//!	Definition of the Hero entity object
//!
//------------------------------------------------------------------------------------------

#include "game/entityhero.h"

#include "game/entitymanager.h"
#include "game/inputcomponent.h"
#include "objectdatabase/dataobject.h"
#include "core/timer.h"

#include "game/attacks.h"				// Attack component
#include "game/awareness.h"				// Awareness component
#include "Physics/system.h"				// Dynamics
#include "movement.h"					// Movement
#include "JAMNET/netman.h"				// Net component
#include "game/aimcontroller.h"			// Aiming component
#include "game/interactioncomponent.h"	// Interaction component
#include "game/messagehandler.h"		// Message
#include "game/entityrangedweapon.h"	// Ranged Weapon
#include "game/chatterboxman.h"
#include "camera/camman.h"
#include "camera/camview.h"
#include "anim/animator.h"
#include "hair/haircollision.h"
#include "fsm.h"
#include "hair/forcefielditem.h"

#include "game/interactioncomponent.h"
#include "game/combatstyle.h"
#include "hud/hudmanager.h"
#include "hud/failurehud.h"

// For tasty aerialgeneral hack
#include "game/entityaerialgeneral.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include "Physics/advancedcharactercontroller.h"
#endif

/*void ForceLinkFunction24()
{
	ntPrintf("!ATTN! Calling ForceLinkFunction24() !ATTN!\n");
}*/

#ifdef _RELEASE
#	define LOG(pcText)
#else
#	define LOG(pcText) ntPrintf("%s\n", pcText)
#endif

 
//------------------------------------------------------------------------------------------
// Hero Lua Interface
//------------------------------------------------------------------------------------------
LUA_EXPOSED_START_INHERITED(Hero, Player)
	// Heavenly Sword Control
	LUA_EXPOSED_METHOD(LeftSword_Power, LeftSword_Power, "", "", "")
	LUA_EXPOSED_METHOD(RightSword_Power, RightSword_Power, "", "", "")
	LUA_EXPOSED_METHOD(LeftSword_Range, LeftSword_Range, "", "", "")
	LUA_EXPOSED_METHOD(RightSword_Range, RightSword_Range, "", "", "")
	LUA_EXPOSED_METHOD(LeftSword_Technique, LeftSword_Technique, "", "", "")
	LUA_EXPOSED_METHOD(RightSword_Technique, RightSword_Technique, "", "", "")
	LUA_EXPOSED_METHOD(LeftSword_Basic, LeftSword_Basic, "", "", "")
	LUA_EXPOSED_METHOD(LeftSword_Away, LeftSword_Away, "", "", "")
	LUA_EXPOSED_METHOD(RightSword_Away, RightSword_Away, "", "", "")
	LUA_EXPOSED_METHOD(BasicSword_Away, BasicSword_Away, "", "", "")

	// Lifeclock
	LUA_EXPOSED_METHOD(GetLifeclockTimeRemaining, GetLifeclockTimeRemaining, "", "", "")
LUA_EXPOSED_END(Player)


//------------------------------------------------------------------------------------------
// Hero XML Interface
//------------------------------------------------------------------------------------------
START_STD_INTERFACE(Hero)
	COPY_INTERFACE_FROM(Player)
	DEFINE_INTERFACE_INHERITANCE(Player)

	OVERRIDE_DEFAULT(ConstructionScript, "")		// FIX ME Not sure about this
	OVERRIDE_DEFAULT(DestructionScript, "")
	OVERRIDE_DEFAULT(InitialSystemState, "")
	//OVERRIDE_DEFAULT(Description, "player")

	PUBLISH_VAR_WITH_DEFAULT_AS( m_bInstantRefill,		false,	LCInstantRefill )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fRequiredHeldTime,	5.0f,	RequiredHeldTime )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fRefillRate,			5.0f,	LCRefillRate )	

	DECLARE_POSTCONSTRUCT_CALLBACK(OnPostConstruct)
	DECLARE_POSTPOSTCONSTRUCT_CALLBACK(OnPostPostConstruct)
END_STD_INTERFACE

//--------------------------------------------------
//!
//! Hero State Machine - was playerstate.lua
//!
//--------------------------------------------------
STATEMACHINE(HERO_FSM, Hero)

//-------------- Setup --------------//
	HERO_FSM(CHashedString& obInitState)
	{
		// TODO: Move this to an enumeration
		if			( obInitState == CHashedString("Player_DefaultState") )				SET_INITIAL_STATE( PLAYER_DEFAULTSTATE );
		else if		( obInitState == CHashedString("Player_CombatState") )				SET_INITIAL_STATE( PLAYER_COMBATSTATE );
		else if		( obInitState == CHashedString("Player_ReactState") )				SET_INITIAL_STATE( PLAYER_REACTSTATE );
		else if		( obInitState == CHashedString("Player_KOAftertouchControlState") )	SET_INITIAL_STATE( PLAYER_KOAFTERTOUCHCONTROLSTATE );
		else if		( obInitState == CHashedString("Player_DeadState") )				SET_INITIAL_STATE( PLAYER_DEADSTATE );
		else if		( obInitState == CHashedString("Player_ExternalControlState") )		SET_INITIAL_STATE( PLAYER_EXTERNALCONTROLSTATE );
		else if		( obInitState == CHashedString("Player_InteractingState") )			SET_INITIAL_STATE( PLAYER_INTERACTINGSTATE );
		else if		( obInitState == CHashedString("Player_RagdollDefaultState") )		SET_INITIAL_STATE( PLAYER_RAGDOLLDEFAULTSTATE );
		else if		( obInitState == CHashedString("Player_RagdollMovingState") )		SET_INITIAL_STATE( PLAYER_RAGDOLLMOVINGSTATE );
		else if		( obInitState == CHashedString("Player_PurgatoryState") )			SET_INITIAL_STATE( PLAYER_PURGATORYSTATE );
		else		ntError_p(0, ("Unrecognised playerstate %s in HERO_FSM\n", ntStr::GetString(obInitState) ) );

	}

//-------------- Global msg --------------//
	BEGIN_EVENTS
		EVENT(msg_combat_aerialended)
		{
			LOG("msg_combat_aerialended");
			ME->HelperGetLuaFunc("ReStartMusic")();
		}
		END_EVENT(true)

		EVENT(msg_fire_complete)		
			ME->m_bWaitingForFireComplete = false;
		END_EVENT(true)

		EVENT(msg_reload_complete)		
			ME->m_bWaitingForReloadComplete = false;
		END_EVENT(true)

		ON_UPDATE
			ME->UpdateHealth( CTimer::Get().GetGameTimeChange() );
		END_EVENT(true)
	
	END_EVENTS


//-------------- Hero states --------------//
	STATE(PLAYER_DEFAULTSTATE)
		BEGIN_EVENTS
			ON_ENTER
				//ntPrintf("Player: State PLAYER_DEFAULTSTATE ON_ENTER\n" );
				// Set the correct movement controller on the character
				ME->SetMovementController();

				ME->GetAttackComponent()->AllowSwitchStance();
				ME->GetPhysicsSystem()->Lua_SetHoldingCapsule(false);	
			END_EVENT(true)

			EVENT(msg_external_control_start)
				SET_STATE( PLAYER_EXTERNALCONTROLSTATE );
			END_EVENT(true)

			EVENT(msg_combat_recovered)
				ME->GetAttackComponent()->CompleteRecovery();
			END_EVENT(true)

			EVENT(msg_combat_impaled)
				SET_STATE( PLAYER_IMPALEDSTATE );
			END_EVENT(true)

			EVENT(msg_combat_struck)
				SET_STATE( PLAYER_REACTSTATE );
			END_EVENT(true)

			EVENT(msg_buttonattack)
			EVENT(msg_buttongrab)
			EVENT(msg_buttondodge)
				if ( ME->GetAttackComponent()->StartNewAttack() )
					SET_STATE( PLAYER_COMBATSTATE );
			END_EVENT(true)

			EVENT(msg_buttonaction)
				// Check for combat action
				if ( ME->GetAttackComponent()->StartNewAttack() )
				{
					SET_STATE( PLAYER_COMBATSTATE );
					END_EVENT(true)
				}
				// Make sure this player is not already manipulating something
				if ( ME->GetInteractionTarget() )
					END_EVENT(true)
				
				ME->SetInteractionTarget(ME->GetAwarenessComponent()->FindInteractionTarget(CUsePoint::ICT_Hero));

				// Did we get a target
				if ( ME->GetInteractionTarget() )
				{
					// new behaviour for the here - he/she controls to move point. This is transitional - once it has been tested for a couple-cases, it
					// can be generalised and ye olde behaviour done away with.
					if (ME->GetInteractionTargetUsePoint() && 
						ME->GetInteractionTargetUsePoint()->CharacterControlsToMoveToPoint())
					{
						ME->GetAttackComponent()->DisallowSwitchStance();
						SET_STATE( PLAYER_MOVINGTOSTATE );
					}
					else
					{
						Message msgAction(ME->GetInputComponent()->IsDirectionHeld() ? msg_running_action : msg_action);

						msgAction.SetEnt(CHashedString(HASH_STRING_OTHER), (CEntity*)ME);
						msgAction.SetEnt(CHashedString(HASH_STRING_SENDER), (CEntity*)ME);
						ME->GetInteractionTarget()->GetMessageHandler()->QueueMessage(msgAction);

						ME->GetAttackComponent()->DisallowSwitchStance();
						SET_STATE( PLAYER_INTERACTINGSTATE );
					}
				}

				// playerstate.lua has  --Check for a running pickup target
				// and					--Check for a pickup target
				// which are block commented	
			END_EVENT(true)

			EVENT(msg_forceequiprangedweapon)
			{
				ntAssert( msg.GetEnt("RangedWeapon") );
				CEntity* pobWeapon = msg.GetEnt("RangedWeapon");

				// Set the interaction target to the weapon
				ME->SetInteractionTarget( pobWeapon );
				
				// Put hero straight into interacting state
				SET_STATE(PLAYER_INTERACTINGSTATE);
			}
			END_EVENT(true)

			EVENT(msg_combat_stanceswitch)
			EVENT(msg_aware_gained_lockon)
			EVENT(msg_walk_run_stopped)
			EVENT(msg_aware_lost_lockon)
				ME->SetMovementController();
			END_EVENT(true)

			EVENT(msg_button_special)
				ME->GetAttackComponent()->StartSpecial();
			END_EVENT(true)

			EVENT(msg_combat_killed)
				SET_STATE( PLAYER_DEADSTATE );
			END_EVENT(true)

		END_EVENTS
	END_STATE // PLAYER_DEFAULTSTATE

	STATE(PLAYER_COMBATSTATE)
		BEGIN_EVENTS
			ON_ENTER
				//ntPrintf("Player: State PLAYER_COMBATSTATE ON_ENTER\n" );
				ntAssert(1);
			END_EVENT(true)

			EVENT(msg_external_control_start)
				SET_STATE( PLAYER_EXTERNALCONTROLSTATE );
			END_EVENT(true)

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

			EVENT(msg_combat_breakout)
				SET_STATE( PLAYER_DEFAULTSTATE );
			END_EVENT(true)

			EVENT(msg_combat_recovered)
				ME->GetAttackComponent()->CompleteRecovery();
				ME->GetAttackComponent()->SetMovementFromCombatPoseFlag(true);
				SET_STATE( PLAYER_DEFAULTSTATE );
			END_EVENT(true)

			EVENT(msg_buttonattack)
			EVENT(msg_buttongrab)
			EVENT(msg_buttonaction)
			EVENT(msg_buttondodge)
				ME->GetAttackComponent()->SelectNextAttack();
			END_EVENT(true)

			EVENT(msg_combat_struck)
				SET_STATE( PLAYER_REACTSTATE );
			END_EVENT(true)

			EVENT(msg_button_special)
				ME->GetAttackComponent()->StartSpecial();
			END_EVENT(true)

			EVENT(msg_rangefastsweep)  //--received when in range stance and there is a fast attack, message defined in data/sound/hero_animevents
			{
				NinjaLua::LuaObject obObj;
				ME->GetPhysicsSystem()->Lua_ParamAttack( obObj );
			}
			END_EVENT(true)
	
		END_EVENTS
	END_STATE // PLAYER_COMBATSTATE

	STATE(PLAYER_REACTSTATE)
		BEGIN_EVENTS
			ON_ENTER
				//ntPrintf("Player: State PLAYER_REACTSTATE ON_ENTER\n" );
				ME->GetAttackComponent()->Lua_SendRecoilMessage();
				ME->GetPhysicsSystem()->Lua_SetHoldingCapsule(false);
			END_EVENT(true)

			//DGF - this is heinous but quick
			EVENT(msg_aerialgeneralhack_forceherotointeracting)
				// As this should only be used in agen fight, we will have the doppelganger manager
				// Otherwise we want it to crash so we can see what the hell else is using it
				DoppelgangerManager::Get().GetMaster()->NotifyPlayerInteracting(true);

				ME->SetInteractionTarget(msg.GetEnt("Sword"));
				{
					Message msgAtRest(msg_atrest);
					ME->GetInteractionTarget()->GetMessageHandler()->QueueMessage(msgAtRest); // Will force an attached sword into default
					Message msgAction(msg_action);
					msgAction.SetEnt(CHashedString(HASH_STRING_OTHER), (CEntity*)ME);
					msgAction.SetEnt(CHashedString(HASH_STRING_SENDER), (CEntity*)ME);
					ME->GetInteractionTarget()->GetMessageHandler()->QueueMessage(msgAction);
					ME->GetAttackComponent()->DisallowSwitchStance();
					SET_STATE( PLAYER_INTERACTINGSTATE );
				}
			END_EVENT(true)

			EVENT(msg_external_control_start)
				SET_STATE( PLAYER_EXTERNALCONTROLSTATE );
			END_EVENT(true)

			EVENT(msg_combat_impaled)
				SET_STATE( PLAYER_IMPALEDSTATE );
			END_EVENT(true)

			EVENT(msg_combat_killed)
				SET_STATE( PLAYER_DEADSTATE );
			END_EVENT(true)

			EVENT(msg_combat_breakout)
				SET_STATE( PLAYER_DEFAULTSTATE );
			END_EVENT(true)

			EVENT(msg_combat_recovered)
				ME->GetAttackComponent()->CompleteRecovery();
				SET_STATE( PLAYER_DEFAULTSTATE );
			END_EVENT(true)

			EVENT(msg_combatsyncdreactionend)
				ME->GetAttackComponent()->EndSyncdReaction();
			END_EVENT(true)

			EVENT(msg_combat_countered)
				SET_STATE( PLAYER_COMBATSTATE );
			END_EVENT(true)

			EVENT(msg_combat_floored)
				ME->HelperGetLuaFunc( CHashedString("GameStatistics"), CHashedString("Event") )( CHashedString("PlayerKO") );
				CChatterBoxMan::Get().Trigger("PlayerKO", ME);

				ME->GetAttackComponent()->StartFlooredState();
			END_EVENT(true)
		END_EVENTS
	END_STATE // PLAYER_REACTSTATE

	STATE(PLAYER_KOAFTERTOUCHCONTROLSTATE) // FIX ME - commented in lua version
		BEGIN_EVENTS
			ON_ENTER
				//ntPrintf("Player: State PLAYER_KOAFTERTOUCHCONTROLSTATE ON_ENTER\n" );
				ME->m_bCombatRecovered = false;

				if ( ! ME->GetInteractionTarget() )
				{
					LOG("Error in Player_State_KOAftertouch: m_pobInteractionTarget is null!!");
					SET_STATE( PLAYER_DEFAULTSTATE );
				}
			END_EVENT(true)

			EVENT(msg_combat_struck)
			{
				Message msgInterrupt(msg_interrupt);	
				msgInterrupt.SetEnt( CHashedString(HASH_STRING_SENDER), ME);
				ME->GetInteractionTarget()->GetMessageHandler()->QueueMessage(msgInterrupt);
				ME->SetInteractionTarget(0);
				SET_STATE( PLAYER_REACTSTATE );
			}
			END_EVENT(true)
				
			EVENT(msg_exitstate)
				//ntPrintf("Player: State PLAYER_KOAFTERTOUCHCONTROLSTATE msg_exitstate\n" );
				ME->SetInteractionTarget(0);

				if ( ME->m_bCombatRecovered )
					SET_STATE( PLAYER_DEFAULTSTATE );
				else
					SET_STATE( PLAYER_COMBATSTATE );
			END_EVENT(true)

			EVENT(msg_combat_recovered)
				ME->GetAttackComponent()->CompleteRecovery();
				ME->m_bCombatRecovered = true;
			END_EVENT(true)

			EVENT(msg_external_control_start)
			{
				Message msgInterrupt(msg_interrupt);	
				msgInterrupt.SetEnt( CHashedString(HASH_STRING_SENDER), ME);
				ME->GetInteractionTarget()->GetMessageHandler()->QueueMessage(msgInterrupt);
				ME->SetInteractionTarget(0);

				SET_STATE( PLAYER_EXTERNALCONTROLSTATE );
			}
			END_EVENT(true)

			EVENT(msg_release_attack)
			{
				Message msgAttOff(msg_attack_off);	
				msgAttOff.SetEnt(CHashedString(HASH_STRING_SENDER), ME);
				ME->GetInteractionTarget()->GetMessageHandler()->QueueMessage(msgAttOff);

				ME->SetInteractionTarget(0);

				if ( ME->m_bCombatRecovered )
					SET_STATE( PLAYER_DEFAULTSTATE );
				else
					SET_STATE( PLAYER_COMBATSTATE );
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE // PLAYER_KOAFTERTOUCHCONTROLSTATE

	STATE(PLAYER_DEADSTATE)
		BEGIN_EVENTS
			ON_ENTER
				//ntPrintf("Player: State PLAYER_DEADSTATE ON_ENTER\n" );

				// Attempt to notify the HUD
				if ( CHud::Exists() && CHud::Get().GetFailureHud() )
				{
					CHud::Get().GetFailureHud()->NotifyFailure("FAILURE_KILLED");
				}

				CLuaGlobal::CallLuaFunc("OnHeroDeath");

				// Drain lifeclock (incase killed by means other than LC)
				if ( ME->HasHeavenlySword() && StyleManager::Exists() 
					&& StyleManager::Get().GetLifeClock() )
				{
					StyleManager::Get().GetLifeClock()->SetTotalInSeconds( 0.0 );
				}

				// Drop anything we are holding
				ME->DropWeapons();
			
				// Destroy any projectiles that may be parented to this object
				ME->GetPhysicsSystem()->Lua_RemoveChildEntities();

				// Make the character into a corpse
				ME->GetMovement()->ClearControllers();
				ME->GetMovement()->SetEnabled( false );
				ME->GetAnimator()->Disable();
				ME->GetPhysicsSystem()->Deactivate();
				ME->GetAttackComponent()->MakeDead();
				ME->Lua_MakeDead();

				// We will assume the character is moving when they enter ragdoll state
				SET_STATE( PLAYER_RAGDOLLMOVINGSTATE );
			END_EVENT(true)
		END_EVENTS
	END_STATE // PLAYER_DEADSTATE

	STATE(PLAYER_IMPALEDSTATE)
		BEGIN_EVENTS
			ON_ENTER
				// Drop anything we are holding
				ME->DropWeapons();
			
				// Destroy any projectiles that may be parented to this object
				ME->GetPhysicsSystem()->Lua_RemoveChildEntities();

				ME->GetAttackComponent()->MakeDead();
				ME->Lua_MakeDead();
			END_EVENT(true)

			EVENT(msg_activateragdoll)
				SET_STATE( PLAYER_RAGDOLLDEFAULTSTATE );
			END_EVENT(true)
		END_EVENTS
	END_STATE // PLAYER_IMPALEDSTATE

	STATE(PLAYER_EXTERNALCONTROLSTATE)
		BEGIN_EVENTS
			ON_ENTER
				ME->GetAttackComponent()->SetDisabled(true);
				ME->GetMovement()->SetInputDisabled(true);

				ME->GetPhysicsSystem()->Lua_SetCharacterControllerDoMovementAbsolutely(true);
			END_EVENT(true)

			EVENT(msg_external_control_end)
			{
				bool bEnableTGSHack = false;

				// TGS HACK TGSHACK!
				if( ME->GetAttackComponent()->AI_Access_GetState() == CS_FLOORED )			
					bEnableTGSHack = true;
				// TGS HACK TGSHACK!


				ME->GetMovement()->SetInputDisabled(false);
				ME->GetAttackComponent()->SetDisabled(false);
				ME->GetPhysicsSystem()->Lua_SetCharacterControllerDoMovementAbsolutely(false);

				// Tell the player to check her stance
				ME->GetAttackComponent()->AffirmStanceNextUpdate();


				// In order to get the hero to come out of her floored state at the 2nd or 3rd
				// failed section of the TGS demo the following was done...

				// TGS HACK TGSHACK!
				if( bEnableTGSHack )	
				{
					ME->LeftSword_Away();
					ME->RightSword_Away();
					ME->GetAttackComponent()->StartFlooredState();
					SET_STATE( PLAYER_TGS_HACK_STATE );
					return true;
				}
				// TGS HACK TGSHACK!


				SET_STATE( PLAYER_DEFAULTSTATE );
			}
			END_EVENT(true)

			EVENT(msg_combat_killed)
				SET_STATE( PLAYER_DEADSTATE );
			END_EVENT(true)
		END_EVENTS
	END_STATE // PLAYER_EXTERNALCONTROLSTATE


	// TGS HACK TGSHACK!
	STATE(PLAYER_TGS_HACK_STATE)
		BEGIN_EVENTS
			// Wait for the hero to finish her get up anim.
			EVENT(msg_combat_recovered)
				// Place the hero back into her standing state. 
				ME->GetAttackComponent()->CompleteRecovery();
				SET_STATE( PLAYER_DEFAULTSTATE );
			END_EVENT(true)
		END_EVENTS
	END_STATE // PLAYER_TGS_HACK_STATE
	// TGS HACK TGSHACK!

	STATE(PLAYER_INTERACTINGSTATE)
		BEGIN_EVENTS
			ON_ENTER
				//ntPrintf("Player: State PLAYER_INTERACTINGSTATE ON_ENTER\n" );
				ME->GetAttackComponent()->DisallowSwitchStance();

				ME->SetExitOnMovementDone(false);
					
				{
					Physics::AdvancedCharacterController* pobCharacterState = (Physics::AdvancedCharacterController*) ME->GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);
					pobCharacterState->FootIKEnabled( false );
				}
				
				if (! ME->GetInteractionTarget() )
				{
					LOG("Error in Player_State_Interacting: m_pobInteractionTarget is null!!");
					SET_STATE( PLAYER_DEFAULTSTATE );
				}
			END_EVENT(true)

			ON_EXIT
				// Hacktastic quick fix for AGen special - DGF
				if (DoppelgangerManager::Exists())
					DoppelgangerManager::Get().GetMaster()->NotifyPlayerInteracting(false);

				//ntPrintf("Player: State PLAYER_INTERACTINGSTATE ON_EXIT\n" );
				ME->GetAttackComponent()->AllowSwitchStance();
				{
					Physics::AdvancedCharacterController* pobCharacterState = (Physics::AdvancedCharacterController*) ME->GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);
					pobCharacterState->FootIKEnabled( true );
				}
			END_EVENT(true)

			EVENT(msg_combat_struck)
			{
				//ntPrintf("Player: State PLAYER_INTERACTINGSTATE msg_combat_struck\n" );
				Message msgInterrupt(msg_interrupt);	
				msgInterrupt.SetEnt(CHashedString(HASH_STRING_SENDER), (CEntity*)ME);
				ME->GetInteractionTarget()->GetMessageHandler()->QueueMessage(msgInterrupt);

				// JML Removing this...  Calling RemoveAllCoolCameras here is bad, I don't know what camera you're trying
				// to remove here but if you really need it removed here then please just remove the cameras you need.
				//CamMan::GetPrimaryView()->RemoveAllCoolCameras();
				ME->SetInteractionTarget(0);
				SET_STATE( PLAYER_REACTSTATE );
			}
			END_EVENT(true)
				
			EVENT(msg_exitstate)
				/*ntPrintf("Player: State PLAYER_INTERACTINGSTATE msg_exitstate\n" );
				{
					CEntity* pSender = msg.GetEnt("Sender");
					if (pSender)
					{
						ntPrintf("Player: msg_exitstate Sender %s\n", pSender->GetName().c_str() );
					}
				}*/

				// FIX ME note in Lua --CHECKME
				ME->SetInteractionTarget(0);
				ME->GetPhysicsSystem()->Lua_CharacterController_SetRagdollCollidable(true);
				SET_STATE( PLAYER_DEFAULTSTATE );
			END_EVENT(true)

			EVENT(msg_movementdone) // FIX ME note in Lua --CHECKME
				//ntPrintf("Player: State PLAYER_INTERACTINGSTATE msg_movementdone\n" );
				if ( ME->ExitOnMovementDone() )
				{
					// JML Removing this...  Calling RemoveAllCoolCameras here is bad, I don't know what camera you're trying
					// to remove here but if you really need it removed here then please just remove the cameras you need.
					CamMan::GetPrimaryView()->RemoveAllCoolCameras();
					ME->SetInteractionTarget(0);
					SET_STATE( PLAYER_DEFAULTSTATE );
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
				//ntPrintf("Player: State PLAYER_INTERACTINGSTATE msg_external_control_start\n" );
				Message msgInterrupt(msg_interrupt);	
				msgInterrupt.SetEnt(CHashedString(HASH_STRING_SENDER), ME);
				ME->GetInteractionTarget()->GetMessageHandler()->QueueMessage(msgInterrupt);

				// JML Removing this...  Calling RemoveAllCoolCameras here is bad, I don't know what camera you're trying
				// to remove here but if you really need it removed here then please just remove the cameras you need.
				CamMan::GetPrimaryView()->RemoveAllCoolCameras();
				ME->SetInteractionTarget(0);

				SET_STATE( PLAYER_EXTERNALCONTROLSTATE );
			}
			END_EVENT(true)

			// Note: This stuff should probably be in the interaction component at some point
			//EVENT(msg_aim_on)					ME->ForwardInteractionMsg(msg_aim_on);			END_EVENT(true)
			EVENT(msg_buttonattack)				ME->ForwardInteractionMsg(msg_attack_on);		END_EVENT(true)
			EVENT(msg_buttongrab)				ME->ForwardInteractionMsg(msg_grab_on);			END_EVENT(true)
			EVENT(msg_buttonaction)			
				// Hacktastic quick fix for AGen special - DGF
				if (DoppelgangerManager::Exists())		
				{
					DoppelgangerManager::Get().GetMaster()->NotifyPlayerInteractionAction();
				}

				ME->ForwardInteractionMsg(msg_action_on);		
			END_EVENT(true)
			EVENT(msg_combat_fire)				ME->ForwardInteractionMsg(msg_attack_on);		END_EVENT(true)
			EVENT(msg_button_power)				ME->ForwardInteractionMsg(msg_power_on);		END_EVENT(true)
			
			EVENT(msg_button_range)				ME->ForwardInteractionMsg(msg_range_on);		END_EVENT(true)
			EVENT(msg_release_attack)			ME->ForwardInteractionMsg(msg_attack_off);		END_EVENT(true)
			EVENT(msg_release_grab)				ME->ForwardInteractionMsg(msg_grab_off);		END_EVENT(true)
			EVENT(msg_release_action)			ME->ForwardInteractionMsg(msg_action_off);		END_EVENT(true)
			EVENT(msg_release_power)			ME->ForwardInteractionMsg(msg_power_off);		END_EVENT(true)
			EVENT(msg_release_range)			ME->ForwardInteractionMsg(msg_range_off);		END_EVENT(true)

			EVENT(msg_reload_complete)			ME->ForwardInteractionMsg(msg_reload_complete);	END_EVENT(true)

			


			// 
			EVENT(msg_weaponpickup_success)	
				//ntPrintf("Player: State PLAYER_INTERACTINGSTATE msg_weaponpickup_success\n" );
				SET_STATE( PLAYER_INTERACTINGSTATE_RANGED );
			END_EVENT(true)

			EVENT(msg_aftertouch)
				//ntPrintf("Player: State PLAYER_INTERACTINGSTATE msg_aftertouch\n" );
				ME->AfterTouchState(true);
			END_EVENT(true)

			EVENT(msg_aftertouch_done)
				//ntPrintf("Player: State PLAYER_INTERACTINGSTATE msg_aftertouch_done\n" );
				ME->AfterTouchState(false);
			END_EVENT(true)

		END_EVENTS

		//-----------------------------------------------------
		// 
		//-----------------------------------------------------

		// Another special substate that shows the hero as aiming... It would so much easier to 
		// complete this if there was a state for each type of weapon - the code relies too much on 
		// specluative assumptions on states that the hero might not even need.
		STATE( PLAYER_INTERACTINGSTATE_RANGED_AIMING )
			BEGIN_EVENTS

				ON_ENTER
					ME->ResetRotationAngles();
					ME->RequestFire( false );
					ME->RequestReload( false );
					ME->ForwardInteractionMsg(msg_aim_on);
				END_EVENT(true)

				ON_EXIT
					ME->ResetToIdleState();
					if( ME->m_bWaitingForReloadComplete )
						ME->ForwardInteractionMsg(msg_reload_complete);
				END_EVENT(true)

				// Update 
				ON_UPDATE
				{
					ME->m_bFireRequested	= (ME->GetInputComponent()->GetVHeld() & (AB_ATTACK_FAST | AB_ATTACK_MEDIUM | AB_ACTION)) && ME->m_bWaitingForReloadComplete;

					// Waiting for the fire timer?
					bool bWaitingForFireDelay = ME->m_bFireRequested && ME->m_fFireRequestTimer > 0.0f;
					ME->m_fFireRequestTimer -= CTimer::Get().GetSystemTimeChange();

					// Try to trigger another fire request. 
					if( bWaitingForFireDelay && ME->m_fFireRequestTimer <= 0.0f )
					{
						Message msg( msg_buttonaction );
						ME->GetMessageHandler()->Receive( msg );
					}

					ME->UpdateAiming( CTimer::Get().GetSystemTimeChange() );
					ME->UpdateHealth( CTimer::Get().GetGameTimeChange() );
				}
				END_EVENT(true)

				EVENT(msg_release_range)
					ME->ForwardInteractionMsg(msg_aim_off);
				END_EVENT(true)

				EVENT(msg_control_end)	
					SET_STATE( PLAYER_INTERACTINGSTATE_RANGED );
				END_EVENT(true)

				EVENT(msg_buttonaction)
				EVENT(msg_buttonattack)				
				EVENT(msg_combat_fire)				
				{
					if( !(ME->RequestReload() || ME->RequestFire() || ME->RequestedFire() || ME->AfterTouchState() || ME->m_bWaitingForReloadComplete || ME->m_bWaitingForFireComplete || ME->m_fFireRequestTimer > 0.0f ) )
					{
						// Make sure the player can't fire another shot for at least 5 secs. This value will change during the course
						// of the state machine depending the weapon type being used by the hero.
						ME->m_fFireRequestTimer = 5.0f;
						ME->RequestFire( true );
						ME->ForwardInteractionMsg(msg_attack_on);
					}
				}
				END_EVENT(true)

				EVENT(msg_fire)					
					ME->m_bWaitingForFireComplete = true;
				END_EVENT(true)

				EVENT(msg_fire_complete)		
					ME->m_bWaitingForFireComplete = false;
				END_EVENT(true)

				EVENT(msg_reload)				
					ME->RequestReload(true);	
					ME->m_bWaitingForReloadComplete = true;
				END_EVENT(true)
				
				EVENT(msg_reload_complete)		
				{
					ME->ForwardInteractionMsg(msg_reload_complete);
					ME->m_bWaitingForReloadComplete = false;

					static const float fTimeTillNextFireWindow = 1.0f / 10.0f;

					// Now that the reload has completed, don't allow another shot to be fired for a least 
					ME->m_fFireRequestTimer = fTimeTillNextFireWindow;

					// If there is a pending fire request - then that should be handled...
					if( ME->m_bFireRequested )
					{
						ME->m_bFireRequested = false;
						Message msg( msg_combat_fire );

						// Don't get the message straight away - wait until twice the next minimum fire window. 
						ME->GetMessageHandler()->Receive( msg, fTimeTillNextFireWindow * 2.0f );
					}
				}
				END_EVENT(true)

			END_EVENTS
		END_STATE // PLAYER_INTERACTINGSTATE_RANGED_AIMING

		STATE( PLAYER_INTERACTINGSTATE_RANGED )
			BEGIN_EVENTS

				ON_ENTER
					ME->ResetToIdleState();
					ME->m_fFireRequestTimer = 0.0f;
				END_EVENT(true)

				ON_EXIT
					ME->ResetToIdleState();
					ME->AfterTouchState(false);
					ME->RequestedFire(false);
					ME->m_bWaitingForFireComplete = false;
				END_EVENT(true)

				EVENT(msg_button_range)

					// For the given weapon, can the hero enter a 3rd person state? .. Todo

					// If the hero is processing a fire or reload - don't all the hero into a range stance
					/*
					if( ME->RequestReload() || ME->RequestFire() || ME->RequestedFire() || ME->AfterTouchState()  || ME->m_bWaitingForReloadComplete || ME->m_bWaitingForFireComplete || ME->m_fFireRequestTimer > 0.0f  )
					{
						SET_STATE( PLAYER_INTERACTINGSTATE_RANGED_AIM_WAIT );
					}
					else
					{
						SET_STATE( PLAYER_INTERACTINGSTATE_RANGED_AIMING );
					}
					*/
				END_EVENT(true)

				EVENT(msg_release_range)
					//ME->ForwardInteractionMsg(msg_aim_off);
				END_EVENT(true)

				// Update 
				ON_UPDATE
				{
					ME->m_bFireRequested	= (ME->GetInputComponent()->GetVHeld() & (AB_ATTACK_FAST | AB_ATTACK_MEDIUM | AB_ACTION)) && ME->m_bWaitingForReloadComplete;

					// Waiting for the fire timer?
					bool bWaitingForFireDelay = ME->m_bFireRequested && ME->m_fFireRequestTimer > 0.0f;
					ME->m_fFireRequestTimer -= CTimer::Get().GetSystemTimeChange();

					// Try to trigger another fire request. 
					if( bWaitingForFireDelay && ME->m_fFireRequestTimer <= 0.0f )
					{
						Message msg( msg_buttonaction );
						ME->GetMessageHandler()->Receive( msg );
					}
				}
				END_EVENT(true)

				EVENT(msg_buttonaction)
				EVENT(msg_buttonattack)				
				EVENT(msg_combat_fire)				
				{
					bool bRequestReload = ME->RequestReload();
					bool bRequestFire = ME->RequestFire();
					bool bRequestedFire = ME->RequestedFire();
					bool bAftertouchState = ME->AfterTouchState();
					bool bWaitingForReloadComplete = ME->m_bWaitingForReloadComplete;
					bool bWaitingForFireComplete = ME->m_bWaitingForFireComplete;
					float fFireRequestTimer = ME->m_fFireRequestTimer;

					if( !(bRequestReload || bRequestFire || bRequestedFire || bAftertouchState || bWaitingForReloadComplete || bWaitingForFireComplete || fFireRequestTimer > 0.0f ) )
					{
						// Make sure the player can't fire another shot for at least 5 secs. This value will change during the course
						// of the state machine depending the weapon type being used by the hero.
						ME->m_fFireRequestTimer = 5.0f;
						ME->RequestFire(true);
						ME->ForwardInteractionMsg(msg_attack_on);		
					}
				}
				END_EVENT(true)

				EVENT(msg_fire)					
					ME->RequestFire(true);		
					ME->m_bWaitingForFireComplete = true;
				END_EVENT(true)

				EVENT(msg_fire_complete)		
					ME->RequestFire(false);		
					ME->m_bWaitingForFireComplete = false;
					ME->m_fFireRequestTimer = 0.1f;
				END_EVENT(true)

				EVENT(msg_reload)				
					ME->m_fFireRequestTimer = 5.0f;
					ME->RequestReload(true);	
					ME->RequestFire(false);		
					ME->m_bWaitingForFireComplete = false;
					ME->m_bWaitingForReloadComplete = true;
				END_EVENT(true)

				EVENT(msg_reload_complete)		
					ME->RequestReload(false);	
					ME->m_bWaitingForReloadComplete = false;
					ME->m_fFireRequestTimer = 0.1f;
				END_EVENT(true)

			END_EVENTS
			
			STATE( PLAYER_INTERACTINGSTATE_RANGED_AIM_WAIT )
				BEGIN_EVENTS
					ON_UPDATE
						ME->m_fFireRequestTimer -= CTimer::Get().GetSystemTimeChange();

						if( !(ME->RequestReload() || ME->AfterTouchState() || ME->m_bWaitingForReloadComplete || ME->m_bWaitingForFireComplete || ME->m_fFireRequestTimer > 0.0f ) )
						{
							SET_STATE( PLAYER_INTERACTINGSTATE_RANGED_AIMING );
						}
					END_EVENT(true)

					EVENT(msg_release_range)
						POP_STATE();
					END_EVENT(true)

					EVENT(msg_weaponpickup_success)			
					END_EVENT(true)

				END_EVENTS
			END_STATE // PLAYER_INTERACTINGSTATE_RANGED_AIM_WAIT
		END_STATE // PLAYER_INTERACTINGSTATE_RANGED
	END_STATE // PLAYER_INTERACTINGSTATE

//-------------------- New Move to State ----------//

	STATE(PLAYER_MOVINGTOSTATE)
		BEGIN_EVENTS
			ON_ENTER
			{
				//ntPrintf("Player: State PLAYER_MOVINGTOSTATE ON_ENTER\n" );

				ME->GetAttackComponent()->DisallowSwitchStance();
				if (! ME->GetInteractionTarget() )
				{
					LOG("Error in Player_MoveToState: m_pobInteractionTarget is null!!");
					SET_STATE( PLAYER_DEFAULTSTATE );
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

				// JML Removing this...  Calling RemoveAllCoolCameras here is bad, I don't know what camera you're trying
				// to remove here but if you really need it removed here then please just remove the cameras you need.
				CamMan::GetPrimaryView()->RemoveAllCoolCameras();
				ME->SetInteractionTarget(0);
				SET_STATE( PLAYER_REACTSTATE );
			}
			END_EVENT(true)

			EVENT(msg_exitstate)
				// FIX ME note in Lua --CHECKME
				ME->SetInteractionTarget(0);
				ME->GetPhysicsSystem()->Lua_CharacterController_SetRagdollCollidable(true);
				SET_STATE( PLAYER_DEFAULTSTATE );
			END_EVENT(true)

			EVENT(msg_walk_moveto_done) 
			{
				Message msgActionSpecific(msg_action_specific);	
				msgActionSpecific.SetEnt(CHashedString(HASH_STRING_OTHER), (CEntity*)ME);
				msgActionSpecific.SetEnt(CHashedString(HASH_STRING_SENDER), (CEntity*)ME);
				ME->GetInteractionTarget()->GetMessageHandler()->QueueMessage(msgActionSpecific);
				SET_STATE( PLAYER_INTERACTINGSTATE );
			}
			END_EVENT(true)

			EVENT(msg_run_moveto_done) 
			{
				Message msgActionSpecificRun(msg_action_specific_run);	
				msgActionSpecificRun.SetEnt(CHashedString(HASH_STRING_OTHER), (CEntity*)ME);
				msgActionSpecificRun.SetEnt(CHashedString(HASH_STRING_SENDER), (CEntity*)ME);
				ME->GetInteractionTarget()->GetMessageHandler()->QueueMessage(msgActionSpecificRun);
				SET_STATE( PLAYER_INTERACTINGSTATE );
			}
			END_EVENT(true)

			EVENT(msg_external_control_start)
			{
				Message msgInterrupt(msg_interrupt);	
				msgInterrupt.SetEnt(CHashedString(HASH_STRING_SENDER), ME);
				ME->GetInteractionTarget()->GetMessageHandler()->QueueMessage(msgInterrupt);

				// JML Removing this...  Calling RemoveAllCoolCameras here is bad, I don't know what camera you're trying
				// to remove here but if you really need it removed here then please just remove the cameras you need.
				CamMan::GetPrimaryView()->RemoveAllCoolCameras();
				ME->SetInteractionTarget(0);

				SET_STATE( PLAYER_EXTERNALCONTROLSTATE );
			}
			END_EVENT(true)

			// Note: This stuff should probably be in the interaction component at some point
			EVENT(msg_aim_on)			ME->ForwardInteractionMsg(msg_aim_on);		END_EVENT(true)
			EVENT(msg_buttonattack)		ME->ForwardInteractionMsg(msg_attack_on);	END_EVENT(true)
			EVENT(msg_buttongrab)		ME->ForwardInteractionMsg(msg_grab_on);		END_EVENT(true)
			EVENT(msg_buttonaction)		ME->ForwardInteractionMsg(msg_action_on);	END_EVENT(true)
			EVENT(msg_button_power)		ME->ForwardInteractionMsg(msg_power_on);	END_EVENT(true)
			EVENT(msg_button_range)		ME->ForwardInteractionMsg(msg_range_on);	END_EVENT(true)
			EVENT(msg_release_attack)	ME->ForwardInteractionMsg(msg_attack_off);	END_EVENT(true)
			EVENT(msg_release_grab)		ME->ForwardInteractionMsg(msg_grab_off);	END_EVENT(true)
			EVENT(msg_aim_off)			ME->ForwardInteractionMsg(msg_aim_off);		END_EVENT(true)
			EVENT(msg_release_action)	ME->ForwardInteractionMsg(msg_action_off);	END_EVENT(true)
			EVENT(msg_release_power)	ME->ForwardInteractionMsg(msg_power_off);	END_EVENT(true)
			EVENT(msg_release_range)	ME->ForwardInteractionMsg(msg_range_off);	END_EVENT(true)
		END_EVENTS
	END_STATE // PLAYER_MOVINGTOSTATE


//-------------------- Ragdoll --------------------//

	STATE(PLAYER_RAGDOLLDEFAULTSTATE)
		BEGIN_EVENTS
			ON_ENTER
				//ntPrintf("Player: State PLAYER_RAGDOLLDEFAULTSTATE ON_ENTER\n" );
				ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);
			END_EVENT(true)

//			EVENT(Kill)
//				ME->Hide();
//				ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);
//			END_EVENT(true)

			EVENT(msg_action)
				//SET_STATE( PLAYER_RAGDOLLMOVETOSTATE ); // State not defined in lua
			END_EVENT(true)

			EVENT(msg_running_action)
				//SET_STATE( PLAYER_RAGDOLLRUNTOSTATE ); // State not defined in lua
			END_EVENT(true)

			EVENT(msg_external_control_start)
				SET_STATE( PLAYER_EXTERNALCONTROLSTATE );
			END_EVENT(true)

		END_EVENTS
	END_STATE // PLAYER_RAGDOLLDEFAULTSTATE

	STATE(PLAYER_RAGDOLLMOVINGSTATE)
		BEGIN_EVENTS
			ON_ENTER
				//ntPrintf("Player: State PLAYER_RAGDOLLMOVINGSTATE ON_ENTER\n" );
				ME->GetPhysicsSystem()->Lua_Ragdoll_CheckAtRest();
			END_EVENT(true)

			EVENT(msg_collision)
			{
				float fProjVel = msg.GetFloat("ProjVel");
				if ( fProjVel > -4.0 && fProjVel < 4.0 )
					END_EVENT(true)
				CEntity* pCollidee = msg.GetEnt("Collidee");

				if ( pCollidee && !pCollidee->IsPlayer() )
					ME->GetAttackComponent()->Lua_GenerateStrike((CEntity*)ME, pCollidee, "atk_corpse_strike");
			}
			END_EVENT(true)
				
			EVENT(msg_atrest)
				SET_STATE( PLAYER_RAGDOLLDEFAULTSTATE );
			END_EVENT(true)
				
			EVENT(msg_external_control_start)
				SET_STATE( PLAYER_EXTERNALCONTROLSTATE );
			END_EVENT(true)
		END_EVENTS
	END_STATE // PLAYER_RAGDOLLMOVINGSTATE

	STATE(PLAYER_PURGATORYSTATE)
		BEGIN_EVENTS
			ON_ENTER
				ntPrintf("Player: State PLAYER_PURGATORYSTATE ON_ENTER\n" );
				ME->GetPhysicsSystem()->Lua_SetCharacterControllerDoMovementAbsolutely(true);
			END_EVENT(true)
		END_EVENTS
	END_STATE // PLAYER_PURGATORYSTATE
		
END_STATEMACHINE //HERO_FSM


//------------------------------------------------------------------------------------------
//!
//!	Hero::Hero()
//!	Default constructor
//!
//------------------------------------------------------------------------------------------
Hero::Hero()
:	m_bCombatRecovered(false),
	m_iSetMovementFrame(0),
	m_eHeavenlySwordMode(AWAY)
{
	m_pBasicSword		= 0;
	m_pBigSword			= 0;
	m_pRanged_LHandle	= 0;
	m_pRanged_LBlade	= 0;
	m_pRanged_RHandle	= 0;
	m_pRanged_RBlade	= 0;
	m_pobCollisionSword	= 0;

	m_bWaitingForFireComplete		= false;	
	m_bWaitingForReloadComplete		= false;
	m_bFireRequested				= false;
	m_fFireRequestTimer				= 0.0f;

	m_fPartStyle = 0.0f;

	ATTACH_LUA_INTERFACE(Hero);
}

//------------------------------------------------------------------------------------------
//!
//!	Hero::~Hero()
//!	Default destructor
//!
//------------------------------------------------------------------------------------------
Hero::~Hero()
{

}

//------------------------------------------------------------------------------------------
//!
//!	Hero::OnPostConstruct()
//!	Post construction
//!	FIX ME A large amount of this function can be moved into the base classes
//! as more characters are moved from lua to C++
//------------------------------------------------------------------------------------------
void Hero::OnPostConstruct()
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
	AimingComponent* pobAimingComponent = NT_NEW_CHUNK(Mem::MC_ENTITY) AimingComponent(this,60.0f,60.0f,-25.0f,25.0f);
	SetAimingComponent(pobAimingComponent);

	// This is used for interaction targeting
	GetInteractionComponent()->SetEntityHeight(1.5f);

	if(m_obInitialSystemState == "" || ntStr::IsNull(m_obInitialSystemState) )
		m_obInitialSystemState = CHashedString("Player_DefaultState");
	
	// Set up the heavenly swords correctly
	m_bHasHeavenlySword = !m_pBasicSword;
	if(m_bHasHeavenlySword) // This should be an enum...
	{
		LeftSword_Technique();
		RightSword_Technique();
	}
	else
	{
		// We should probably have a new hero type for the player with the basic swords
		LeftSword_Basic();
	}

	// Double check movment controllers
	if (ntStr::IsNull (m_obWalkingController) )
	{
		m_obRangeStrafeController = CHashedString("RangeStrafe");
		m_obPowerStrafeController = CHashedString("PowerStrafe");
		m_obSpeedStrafeController = CHashedString("SpeedStrafe");
		m_obRangeWalkingController = CHashedString("HeroRangeWalkRun");
		m_obPowerWalkingController = CHashedString("HeroPowerWalkRun");
		m_obSpeedWalkingController = CHashedString("HeroSpeedWalkRun");
		m_obWalkingController = CHashedString("HeroWalkRun");
	}

	// Hero should register combat log for the style system
	if ( StyleManager::Exists() )
	{
		ntAssert( StyleManager::Get().GetHitCounter() );
		GetAttackComponent()->RegisterCombatEventLog( StyleManager::Get().GetHitCounter()->GetCombatEventLog() );
	}
}

//--------------------------------------------------
//!
//!	Hero::OnLevelStart()
//!	Called for each ent on level startup
//!
//--------------------------------------------------
void Hero::OnLevelStart()
{
	// Create and attach the statemachine. Must be done AFTER anim containers fixed up by area system
	// i.e. after XML serialisation. OR this shouldnt play an animation
	HERO_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) HERO_FSM(m_obInitialSystemState);
	ATTACH_FSM(pFSM);
}

//------------------------------------------------------------------------------------------
//!
//!	Hero::OnPostConstruct()
//!	Post construction
//!
//------------------------------------------------------------------------------------------
void Hero::OnPostPostConstruct()
{
	// Call the base postpost construction on the entity first
	Player::OnPostPostConstruct();
}


//------------------------------------------------------------------------------------------
//!
//!	Hero::Show
//!	Show the Hero and appropriate weapons
//!
//------------------------------------------------------------------------------------------
void Hero::Show()
{
	CEntity::Show();

	// Now make sure only the correct weapons are visible
	if(!m_bHasHeavenlySword)
		return;

	switch(m_eHeavenlySwordMode)
	{
	case POWER:
		RightSword_Power();
		LeftSword_Power();
		break;
	case RANGE:
		RightSword_Range();
		LeftSword_Range();
		break;
	case TECHNIQUE:
		RightSword_Technique();
		LeftSword_Technique();
		break;
	case AWAY:
		RightSword_Away();
		LeftSword_Away();
		break;
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Hero::SetMovementController()
//!	SetMovement controller
//! FIX ME - probably shared by both player characters
//!
//------------------------------------------------------------------------------------------
void Hero::SetMovementController()
{
	// This should only be called once per frame
	if ( m_iSetMovementFrame == CTimer::Get().GetSystemTicks() ) 
		return;

	m_iSetMovementFrame = CTimer::Get().GetSystemTicks();

	if (!GetAwarenessComponent())
		return;

	if ( GetAwarenessComponent()->GetCurrentAutoLockonP() )
	{
		if ( GetAttackComponent()->MoveInRangeStance() && !ntStr::IsNull(m_obRangeStrafeController) )
		{
			GetMovement()->Lua_StartMovementFromXMLDef( m_obRangeStrafeController );
		}
		else if ( GetAttackComponent()->MoveInPowerStance() && !ntStr::IsNull(m_obPowerStrafeController) )
		{
			GetMovement()->Lua_StartMovementFromXMLDef( m_obPowerStrafeController );
		}
		else if ( !ntStr::IsNull(m_obSpeedStrafeController) )
		{
			GetMovement()->Lua_StartMovementFromXMLDef( m_obSpeedStrafeController );
		}
	}
	else
	{
		if ( GetAttackComponent()->MoveInRangeStance() && !ntStr::IsNull(m_obRangeWalkingController) )
		{
			GetMovement()->Lua_StartMovementFromXMLDef( m_obRangeWalkingController );
		}
		else if ( GetAttackComponent()->MoveInPowerStance() && !ntStr::IsNull(m_obPowerWalkingController) )
		{
			GetMovement()->Lua_StartMovementFromXMLDef( m_obPowerWalkingController );
		}
		else if ( GetAwarenessComponent()->IsAwareOfEnemies() && !ntStr::IsNull(m_obSpeedWalkingController) )
		{
			GetMovement()->Lua_StartMovementFromXMLDef( m_obSpeedWalkingController );
		}
		else if ( !ntStr::IsNull(m_obWalkingController) )
		{
			GetMovement()->Lua_StartMovementFromXMLDef( m_obWalkingController );
		}
	}
}


//------------------------------------------------------------------------------------------
//!	Hero::ForwardInteractionMsg()
//!	forward msg when in interacting state
//! FIX ME - probably shared by all characters
//------------------------------------------------------------------------------------------
void Hero::ForwardInteractionMsg(MessageID obMsg)
{
	Message msgFwd(obMsg);
	msgFwd.SetEnt(CHashedString(HASH_STRING_SENDER), (CEntity*)this);
	GetInteractionTarget()->GetMessageHandler()->QueueMessage(msgFwd);
}


//------------------------------------------------------------------------------------------
//!
//!	Hero::DeactivateWeapon
//!	
//! 
//------------------------------------------------------------------------------------------
void Hero::DeactivateWeapon(CEntity* pWeapon)
{
	if(!pWeapon)
		return;

	pWeapon->Hide();

	if(pWeapon->GetPhysicsSystem())
	{
		Physics::LogicGroup* lg = pWeapon->GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::SINGLE_RIGID_BODY_LG);
		if(lg)
			lg->Deactivate();
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Hero::LeftSword_Power
//!	
//! 
//------------------------------------------------------------------------------------------
void Hero::LeftSword_Power()
{
	if(!m_bHasHeavenlySword)
		return;

	m_eHeavenlySwordMode = POWER;

	// some swords off
	DeactivateWeapon(m_pLeftWeapon);
	DeactivateWeapon(m_pRanged_LHandle);
	DeactivateWeapon(m_pRanged_LBlade);

	// Remove any sword collision for the hair
	if (m_pobCollisionSword)
	{
		m_pobCollisionSword->Remove();
	}

}


//------------------------------------------------------------------------------------------
//!
//!	Hero::RighSword_Power
//!	
//! 
//------------------------------------------------------------------------------------------
void Hero::RightSword_Power()
{
	if(!m_bHasHeavenlySword)
		return;

	m_eHeavenlySwordMode = POWER;

	// some swords off
	DeactivateWeapon(m_pRightWeapon);
	DeactivateWeapon(m_pRanged_RHandle);
	DeactivateWeapon(m_pRanged_RBlade);

	// Big sword on
	m_pBigSword->Lua_Reparent(this, "r_weapon");
	if(!GetRenderableComponent()->IsSuspended())
	{
		m_pBigSword->Show();
	}

	// Remove any sword collision for the hair
	if (m_pobCollisionSword)
	{
		m_pobCollisionSword->Remove();
	}

	// Activate particles on collision with this sword
	m_pBigSword->GetPhysicsSystem()->Lua_ActivateParticleOnContact();
}


//------------------------------------------------------------------------------------------
//!
//!	Hero::LeftSword_Range
//!	
//! 
//------------------------------------------------------------------------------------------
void Hero::LeftSword_Range()
{
	if(!m_bHasHeavenlySword)
		return;

	m_eHeavenlySwordMode = RANGE;

	// some swords off
	DeactivateWeapon(m_pLeftWeapon);

	// Ranged sword on
	m_pRanged_LHandle->Lua_Reparent(this, "l_weapon");
	m_pRanged_LHandle->Lua_SetLocalTransform(0.f,0.f,0.f, 0.f,0.f,0.f);
	if(!GetRenderableComponent()->IsSuspended())
	{
		m_pRanged_LHandle->Show();
	}

	m_pRanged_LBlade->Lua_Reparent(this, "l_end");
	m_pRanged_LBlade->Lua_SetLocalTransform(0.f,0.f,0.f, -30.f,23.f,-15.f);
	if(!GetRenderableComponent()->IsSuspended())
	{
		m_pRanged_LBlade->Show();
	}

	// Remove any sword collision for the hair
	if (m_pobCollisionSword)
	{
		m_pobCollisionSword->Remove();
	}

	// Activate particles on collision with this sword
	m_pRanged_LBlade->GetPhysicsSystem()->Lua_ActivateParticleOnContact();
}


//------------------------------------------------------------------------------------------
//!
//!	Hero::RightSword_Range
//!	
//! 
//------------------------------------------------------------------------------------------
void Hero::RightSword_Range()
{
	if(!m_bHasHeavenlySword)
		return;

	m_eHeavenlySwordMode = RANGE;

	// some swords off
	DeactivateWeapon(m_pRightWeapon);
	DeactivateWeapon(m_pBigSword);

	// Ranged sword on
	m_pRanged_RHandle->Lua_Reparent(this, "r_weapon");
	m_pRanged_RHandle->Lua_SetLocalTransform(0.f,0.f,0.f, 0.f,0.f,0.f);
	if(!GetRenderableComponent()->IsSuspended())
	{
		m_pRanged_RHandle->Show();
	}

	m_pRanged_RBlade->Lua_Reparent(this, "r_end");
	m_pRanged_RBlade->Lua_SetLocalTransform(0.f,0.f,0.f, 0.f,0.f,0.f);
	if(!GetRenderableComponent()->IsSuspended())
	{
		m_pRanged_RBlade->Show();
	}

	// Remove any sword collision for the hair
	if (m_pobCollisionSword)
	{
		m_pobCollisionSword->Remove();
	}

	// Activate particles on collision with this sword
	m_pRanged_RBlade->GetPhysicsSystem()->Lua_ActivateParticleOnContact();
}


//------------------------------------------------------------------------------------------
//!
//!	Hero::LeftSword_Technique
//!	
//! 
//------------------------------------------------------------------------------------------
void Hero::LeftSword_Technique()
{
	if(!m_bHasHeavenlySword || !m_pLeftWeapon)
		return;

	m_eHeavenlySwordMode = TECHNIQUE;

	// some swords off
	DeactivateWeapon(m_pRanged_LHandle);
	DeactivateWeapon(m_pRanged_LBlade);

	// Remove any sword collision for the hair
	if(m_pobCollisionSword)
	{
		m_pobCollisionSword->Remove();
	}

	// Technique sword on
	m_pLeftWeapon->Lua_Reparent(this, "l_weapon");
	m_pLeftWeapon->Lua_SetLocalTransform(0.f,0.f,0.f, 0.f,0.f,0.f);
	if(!GetRenderableComponent()->IsSuspended())
	{
		m_pLeftWeapon->Show();
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Hero::RightSword_Technique
//!	
//! 
//------------------------------------------------------------------------------------------
void Hero::RightSword_Technique()
{
	if(!m_bHasHeavenlySword || !m_pRightWeapon)
		return;

	m_eHeavenlySwordMode = TECHNIQUE;

	// some swords off
	DeactivateWeapon(m_pRanged_RHandle);
	DeactivateWeapon(m_pRanged_RBlade);
	DeactivateWeapon(m_pBigSword);

	// Remove any sword collision for the hair
	if (m_pobCollisionSword)
	{
		m_pobCollisionSword->Remove();
	}

	// Technique sword on
	m_pRightWeapon->Lua_Reparent(this, "r_weapon");
	m_pRightWeapon->Lua_SetLocalTransform(0.f,0.f,0.f, 0.f,0.f,0.f);
	if(!GetRenderableComponent()->IsSuspended())
	{
		m_pRightWeapon->Show();
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Hero::LeftSword_Basic
//!	
//! 
//------------------------------------------------------------------------------------------
void Hero::LeftSword_Basic()
{
	// Basic sword on
	m_pBasicSword->Lua_Reparent(this, "r_weapon");
	m_pBasicSword->Lua_SetLocalTransform(0.f,0.34f,0.f, -90.f,-102.f,0.f);
	if(!GetRenderableComponent()->IsSuspended())
	{
		m_pBasicSword->Show();
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Hero::LeftSword_Away
//!	
//! 
//------------------------------------------------------------------------------------------
void Hero::LeftSword_Away()
{
	if(!m_bHasHeavenlySword || !m_pLeftWeapon)
		return;

	m_eHeavenlySwordMode = AWAY;

	// swords off
	DeactivateWeapon(m_pRanged_LHandle);
	DeactivateWeapon(m_pRanged_LBlade);	
	
	// Technique sword in sheath
	m_pLeftWeapon->Lua_Reparent(this, m_sLeftSheathedTransform);
	m_pLeftWeapon->Lua_SetLocalTransform(m_ptLeftSheathedPosition.X(), m_ptLeftSheathedPosition.Y(), m_ptLeftSheathedPosition.Z(),
										 m_ptLeftSheathedYPR.X(), m_ptLeftSheathedYPR.Y(), m_ptLeftSheathedYPR.Z());

	if(!GetRenderableComponent()->IsSuspended())
	{
		m_pLeftWeapon->Show();
	}

	// Deactivate collision volumes
	Physics::LogicGroup* lg = m_pLeftWeapon->GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::SINGLE_RIGID_BODY_LG);
	if(lg)
		lg->Deactivate();

	// Remove any sword collision for the hair
	if (m_pobCollisionSword)
	{
		m_pobCollisionSword->Add();
	}

}


//------------------------------------------------------------------------------------------
//!
//!	Hero::RightSword_Away
//!	
//! 
//------------------------------------------------------------------------------------------
void Hero::RightSword_Away()
{
	if(!m_bHasHeavenlySword || !m_pRightWeapon)
		return;

	m_eHeavenlySwordMode = AWAY;

	// swords off
	DeactivateWeapon(m_pBigSword);
	DeactivateWeapon(m_pRanged_RHandle);
	DeactivateWeapon(m_pRanged_RBlade);	
	
	// Technique sword in sheath
	m_pRightWeapon->Lua_Reparent(this, m_sRightSheathedTransform);
	m_pRightWeapon->Lua_SetLocalTransform(m_ptRightSheathedPosition.X(), m_ptRightSheathedPosition.Y(), m_ptRightSheathedPosition.Z(),
										 m_ptRightSheathedYPR.X(), m_ptRightSheathedYPR.Y(), m_ptRightSheathedYPR.Z());

	if(!GetRenderableComponent()->IsSuspended())
	{
		m_pRightWeapon->Show();
	}

	// Deactivate collision volumes
	Physics::LogicGroup* lg = m_pRightWeapon->GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::SINGLE_RIGID_BODY_LG);
	if(lg)
		lg->Deactivate();

	// Remove any sword collision for the hair
	if (m_pobCollisionSword)
	{
		m_pobCollisionSword->Add();
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Hero::BasicSword_Away
//!	
//! 
//------------------------------------------------------------------------------------------
void Hero::BasicSword_Away()
{
	if(m_bHasHeavenlySword || !m_pBasicSword)
		return;

	// Basic sword in sheath
	m_pBasicSword->Lua_Reparent(this, m_sBasicSheathedTransform);
	m_pBasicSword->Lua_SetLocalTransform(m_ptBasicSheathedPosition.X(), m_ptBasicSheathedPosition.Y(), m_ptBasicSheathedPosition.Z(),
										 m_ptBasicSheathedYPR.X(), m_ptBasicSheathedYPR.Y(), m_ptBasicSheathedYPR.Z());

	if(!GetRenderableComponent()->IsSuspended())
	{
		m_pBasicSword->Show();
	}

	// Deactivate collision volumes
	if(m_pBasicSword->GetPhysicsSystem())
	{
	Physics::LogicGroup* lg = m_pBasicSword->GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::SINGLE_RIGID_BODY_LG);
	if(lg)
		lg->Deactivate();
	}

	// Remove any sword collision for the hair
	if(m_pobCollisionSword)
	{
		m_pobCollisionSword->Add();
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Hero::ChangeHealth
//!	
//! 
//------------------------------------------------------------------------------------------
void Hero::ChangeHealth( float fDelta, const char* pcReason )
{ 
	UNUSED( pcReason );

	// m_bUseHeroHealth = true, will put the health system on to the heroine with HS
	if ( !g_ShellOptions->m_bUseHeroHealth && StyleManager::Exists() && HasHeavenlySword() )
	{
		// Register heath loss style event
	}
	else
	{
		Player::ChangeHealth( fDelta, pcReason ); 
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Hero::RechargeLC
//!	
//! 
//------------------------------------------------------------------------------------------
void Hero::RechargeLC( float fHeldTime, float fTimeStep )
{
	// If LC charging from holding the grab is disabled then return now
	if (!g_ShellOptions->m_bLCCharge)
		return;

	int iStyle = StyleManager::Get().GetHitCounter()->GetStylePoints();

	// Been holding grab long enough
	if ( iStyle > 0 && fHeldTime > m_fRequiredHeldTime && StyleManager::Exists() && !StyleManager::Get().IsPrologMode() )
	{	
		StyleManager::Get().GetHitCounter()->ForceActive( true );

		if ( m_bInstantRefill )
		{
			StyleManager::Get().GetHitCounter()->SetStylePoints( 0 );
			StyleManager::Get().GetLifeClock()->IncrementLifeClock( (float) iStyle );
		}
		else
		{
			// Switch off life clock while we manually fill it so it does not look odd
			StyleManager::Get().GetLifeClock()->SetActive( false );

			float fStyle = fTimeStep * m_fRefillRate;
			fStyle += m_fPartStyle;

			// Any whole style points?
			if ( fStyle > 1.0f )
			{
				iStyle = (int)floor(fStyle);
				m_fPartStyle = fStyle - (float)iStyle;

				StyleManager::Get().GetHitCounter()->AddStylePoints( - iStyle );
				StyleManager::Get().GetLifeClock()->IncrementLifeClock( (float) iStyle );
			}
			else
			// hold onto the remainer for next time
			{
				m_fPartStyle = fStyle;
			}
		}
	}
	else
	{
		StyleManager::Get().GetLifeClock()->SetActive( true );
		StyleManager::Get().GetHitCounter()->ForceActive( false );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Hero::GetLifeclockTimeRemaining
//!	
//! 
//------------------------------------------------------------------------------------------
float Hero::GetLifeclockTimeRemaining()
{
	return (float)StyleManager::Get().GetLifeClock()->GetTotalInSeconds();
}
