//!----------------------------------------------------------------------------------------------
//!
//!	\file game/entityai.cpp
//!	Definition of the AI entity object
//!
//!----------------------------------------------------------------------------------------------

#include "game/entityai.h"

#include "ai/aiformationcomponent.h"
#include "ai/ainavgraphmanager.h"
#include "ai/airepulsion.h"
#include "game/fsm.h"
#include "game/aicomponent.h"
#include "game/attacks.h"
#include "game/messagehandler.h"
#include "game/awareness.h"
#include "game/movement.h"
#include "game/interactioncomponent.h"
#include "game/inputcomponent.h"
#include "game/combathelper.h"
#include "game/chatterboxman.h"
#include "game/movement.h"
#include "game/aftertouchcontroller.h"
#include "game/renderablecomponent.h"
#include "game/entityspawnpoint.h"
#include "game/weapons.h"
#include "game/armycopymovementcontroller.h"
#include "camera/camman.h"
#include "camera/camview.h"
#include "anim/animator.h"
#include "Physics/world.h"
#include "Physics/system.h"
#include "Physics/collisionbitfield.h"
#include "objectdatabase/dataobject.h"
#include "core/visualdebugger.h"

#if defined( PLATFORM_PS3 )
#	include "army/armyrenderable.h"
#endif

void ForceLinkFunction19()
{
	ntPrintf("!ATTN! Calling ForceLinkFunction19() !ATTN!\n");
}

//!----------------------------------------------------------------------------------------------
//! AI XML Interface
//!----------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE(AI, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(Character)
	COPY_INTERFACE_FROM(Character)

	OVERRIDE_DEFAULT(ConstructionScript, "AI_Construct")
	OVERRIDE_DEFAULT(Description, "ai")
	OVERRIDE_DEFAULT(SceneElementDef, "DefaultAISceneElement")

	PUBLISH_VAR_WITH_DEFAULT_AS(m_sAIDefinition, "HeroAIComponentDef", AIDefinition)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_AttackSkill, 0, AttackSkill)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_BlockSkill, 0, BlockSkill)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fAttackSkillAdjust, 30.0f, AttackSkillAdjust)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fBlockSkillAdjust, 50.0f, BlockSkillAdjust)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fVisionUpdateRate, 0.0f, VisionUpdateRate)

	PUBLISH_VAR_AS(m_sInitialSystemState, InitialSystemState)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_sController,	"AIController_Test", Controller)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obInitialPatrolRoute, "", InitialPatrolRoute)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bIsEnemy,	false, IsEnemy)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bAttackAI,false, AttackAI)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_iChatGroupID,	0, ChatGroupID)
	
	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )
DECLARE_POSTPOSTCONSTRUCT_CALLBACK( OnPostPostConstruct )
END_STD_INTERFACE

//!----------------------------------------------------------------------------------------------
//! AIFactScoreWeights Interface
//!----------------------------------------------------------------------------------------------
START_STD_INTERFACE	(AIFactScoreWeights)
	PUBLISH_VAR_AS(m_FactDistanceWeight,				FactDistanceWeight )
	PUBLISH_VAR_AS(m_FactAgeWeight,						FactAgeWeight )				
	PUBLISH_VAR_AS(m_FactEntAttackedWeight,				FactEntAttackedWeight )
	PUBLISH_VAR_AS(m_InnerRangeWeight,					InnerRangeWeight )			
	PUBLISH_VAR_AS(m_CloseRangeWeight,					CloseRangeWeight )				
	PUBLISH_VAR_AS(m_LineOfSightRangeWeight,			LineOfSightRangeWeight )
	PUBLISH_VAR_AS(m_MainRangeWeight,					MainRangeWeight )	
	PUBLISH_VAR_AS(m_ShootRangeWeight,					ShootRangeWeight )				
	PUBLISH_VAR_AS(m_OtherPersonsProblemRangeWeight,	OtherPersonsProblemRangeWeight )
END_STD_INTERFACE


//!----------------------------------------------------------------------------------------------
//! AI Lua Interface
//!----------------------------------------------------------------------------------------------
LUA_EXPOSED_START_INHERITED(AI, Character)
	LUA_EXPOSED_METHOD_GET(AI,			GetAIComponent_nonconst,			"AI Functionality")

	LUA_EXPOSED_METHOD(GetVisionUpdateRate, GetVisionUpdateRate, "", "", "")
	LUA_EXPOSED_METHOD(SetVisionUpdateRate, SetVisionUpdateRate, "", "", "")


	// Expose the methods for contorlling invisible entities
	LUA_EXPOSED_METHOD(AddInvisibleEntity,			AddInvisibleEntity, "", "", "")
	LUA_EXPOSED_METHOD(RemoveInvisibleEntity,		RemoveInvisibleEntity, "", "", "")
	LUA_EXPOSED_METHOD(FlushInvisibleEntityList,	FlushInvisibleEntityList, "", "", "")

	// Exposed methods for assigning visible score weights. 
	LUA_EXPOSED_METHOD(DefaultVisibleScoreWeight,	DefaultVisibleScoreWeight, "", "", "")
	LUA_EXPOSED_METHOD(SetVisibleScoreWeight,		SetVisibleScoreWeight, "", "", "")

LUA_EXPOSED_END(AI)


//!----------------------------------------------------------------------------------------------
//!
//! Enemy AI State Machine - was aistate.lua
//!
//!----------------------------------------------------------------------------------------------
STATEMACHINE(AI_FSM, AI)

	AI_FSM(CHashedString& obInitState)
	{
		if(obInitState == "AI_DefaultState")				SET_INITIAL_STATE(DEFAULTSTATE);
		else if(obInitState == "AI_ExternalControlState")	SET_INITIAL_STATE(EXTERNALCONTROLSTATE);
		else ntError_p(0, ("Unrecognised state %s in AI_FSM\n", ntStr::GetString(obInitState)));		
	}

	//!----------------------------------------------------------------------------------------------
	//! DEFAULT STATE
	//!----------------------------------------------------------------------------------------------
	STATE(DEFAULTSTATE)
		BEGIN_EVENTS
			ON_ENTER
				ME->SetInteractionTarget(0);				// Reset the interaction target
				//ntPrintf("DEFAULTSTATE!");
				ME->GetAIComponent()->RefreshController();	// Set the correct movement controller
				ME->GetAIComponent()->SetDisabled(false);	// Enable the AI
			END_EVENT(true)

			ON_EXIT
				ME->GetAIComponent()->SetDisabled(true);		
			END_EVENT(true)

			ON_UPDATE
			END_EVENT(true)

			EVENT(msg_external_control_start)
				SET_STATE(EXTERNALCONTROLSTATE);
			END_EVENT(true)

			EVENT(msg_combat_impaled)
				SET_STATE(IMPALEDDEADSTATE);
			END_EVENT(true)
			
			EVENT(msg_combat_killed) 
				SET_STATE(DEADSTATE);
			END_EVENT(true)
			
			EVENT(msg_combat_struck) 
				SET_STATE(REACTSTATE);
			END_EVENT(true)

			EVENT(msg_buttonattack)
			EVENT(msg_buttongrab)
				if(ME->GetAttackComponent() && ME->GetAttackComponent()->StartNewAttack())
				{
					SET_STATE(COMBATSTATE);
				}
			END_EVENT(true)

			EVENT(msg_interact) // Force an interaction with a target entity (assumes the entity is available for interaction)
			{
				static CHashedString sTarget("target");
				Message msgAction(msg_action);
				msgAction.SetEnt(CHashedString(HASH_STRING_OTHER),	(CEntity*)ME);

				msg.GetEnt(sTarget)->GetMessageHandler()->QueueMessage(msgAction);
				
				ME->GetAttackComponent()->DisallowSwitchStance();
				ME->SetInteractionTarget(msg.GetEnt("target"));
				SET_STATE(INTERACTINGSTATE);
			}
			END_EVENT(true)

			EVENT(msg_buttonaction) //set a substate according to what the current targets are		
			{
				if(ME->GetInteractionTarget()) // Make sure this player isn't already manipulating something
					return true;
				
				CEntity* pTarget = ME->GetAwarenessComponent()->Lua_FindInteractionTarget( ME->IsEnemy() ? CUsePoint::ICT_Enemy : CUsePoint::ICT_Ally );
				
				if(pTarget)
				{
					if(ME->GetAIComponent()->GetMovementMagnitude() > 0.3f) // Can we do something with this magic number please? - JML
					{
						Message msgRunningAction(msg_running_action);
						msgRunningAction.SetEnt(CHashedString(HASH_STRING_OTHER),	(CEntity*)ME);
						pTarget->GetMessageHandler()->QueueMessage(msgRunningAction);
					}
					else
					{
						Message msgAction(msg_action);
						msgAction.SetEnt(CHashedString(HASH_STRING_OTHER),	(CEntity*)ME);

						pTarget->GetMessageHandler()->QueueMessage(msgAction);
					}
					
					ME->GetAttackComponent()->DisallowSwitchStance();
					ME->SetInteractionTarget(msg.GetEnt("target"));
					SET_STATE(INTERACTINGSTATE);
				}
			}
			END_EVENT(true)

			EVENT(msg_interactnamed)
			{
				CEntity* pInteractionTarget = ME->GetInteractionTarget();
				if(pInteractionTarget) // Make sure this player ISN'T already manipulating something
					return true;
		
				WEAPON_PICKUP_TYPE eType = (WEAPON_PICKUP_TYPE)msg.GetInt("Type");
				RANGED_WEAPON_TYPE eRWType = RWT_NONE;

				if(eType == WPT_NONE)
				{
					return true;
				}

				if(eType == WPT_CROSSBOW) { eRWType = RWT_CROSSBOW; }
				else if(eType == WPT_BAZOOKA) { eRWType = RWT_BAZOOKA; }
				else
				{
					ntPrintf("ERROR: msg_interactnamed called with an unspecified weapon-pickup-type\n");
					return true;
				}
				
				CEntity* pTarget = ME->GetAwarenessComponent()->FindRangedWeaponInteractionTarget(eRWType);

				
				if(!pTarget)
					return true;
				
				if((ME->GetInputComponent() != NULL) && (ME->GetInputComponent()->IsDirectionHeld()))
				{
					Message message(msg_running_action);
					message.SetEnt(CHashedString(HASH_STRING_OTHER), ME);
					pTarget->GetMessageHandler()->QueueMessage(message);
				}
				else
				{
					Message message(msg_action);
					message.SetEnt(CHashedString(HASH_STRING_OTHER), ME);
					pTarget->GetMessageHandler()->QueueMessage(message);
				}
				
				ME->GetAttackComponent()->DisallowSwitchStance();
				ME->SetInteractionTarget(pTarget);
				SET_STATE(INTERACTINGWAITSTATE);
			}
			END_EVENT(true);

			EVENT(msg_ai_attack) 
			EVENT(msg_buttondodge) 
				SET_STATE(COMBATSTATE);
			END_EVENT(true)
			
			EVENT(msg_button_special) 
				ME->GetAttackComponent()->StartSpecial();
			END_EVENT(true)
		END_EVENTS
	END_STATE


	//!----------------------------------------------------------------------------------------------
	//! EXTERNAL CONTROL STATE
	//!----------------------------------------------------------------------------------------------
	STATE(EXTERNALCONTROLSTATE) 
		BEGIN_EVENTS
			ON_ENTER
				ME->GetAIComponent()->SetExternalControlState(true);
				ME->GetAIComponent()->SetDisabled( true );
				ME->GetAttackComponent()->SetTargetingDisabled(true);
				ME->GetPhysicsSystem()->Lua_SetCharacterControllerDoMovementAbsolutely(true);
			END_EVENT(true)

			ON_EXIT
				ME->GetPhysicsSystem()->Lua_SetCharacterControllerDoMovementAbsolutely(false);
				ME->GetAIComponent()->SetDisabled( false );
				ME->GetAIComponent()->SetExternalControlState(false);
				ME->GetAttackComponent()->SetTargetingDisabled(false);
			END_EVENT(true)
			
			EVENT(msg_external_control_end)
				if(ME->GetRangedWeapon())
				{
					SET_STATE(INTERACTINGSTATE);
				}
				else
				{
					SET_STATE(DEFAULTSTATE);
				}
			END_EVENT(true)
			
			EVENT(msg_combat_killed)
				SET_STATE(DEADSTATE);
			END_EVENT(true)
		END_EVENTS
	END_STATE


	//!----------------------------------------------------------------------------------------------
	//! COMBAT STATE
	//!----------------------------------------------------------------------------------------------
	STATE(COMBATSTATE) 
		BEGIN_EVENTS
			ON_ENTER
				// Enable the AI for the entity
				ME->GetAIComponent()->SetDisabled(false);
				ME->GetAIComponent()->SetRecovering(true);
			END_EVENT(true)
			
			ON_EXIT
				// Disable the AI for the entity
				ME->GetAIComponent()->SetDisabled(true);
			END_EVENT(true)
			
			EVENT(msg_combat_killed) 
				ME->GetAttackComponent()->CompleteRecovery();
				ME->GetAIComponent()->SetRecovering(false);
				SET_STATE(DEADSTATE) ;
			END_EVENT(true)
			
			EVENT(msg_combat_breakout)
				if(ME->GetRangedWeapon())
				{
					SET_STATE(INTERACTINGSTATE);
				}
				else
				{
					SET_STATE(DEFAULTSTATE);
				}
			END_EVENT(true)
												
			EVENT(msg_combat_recovered) 
				ME->GetAttackComponent()->CompleteRecovery();
				ME->GetAIComponent()->SetRecovering(false);
				if(ME->GetRangedWeapon())
				{
					SET_STATE(INTERACTINGSTATE);
				}
				else
				{
					SET_STATE(DEFAULTSTATE);
				}
			END_EVENT(true)
												
			EVENT(msg_buttonattack)
				ME->GetAttackComponent()->SelectNextAttack();
			END_EVENT(true)
												
			EVENT(msg_buttondodge)
			EVENT(msg_buttonaction)
				ME->GetAttackComponent()->SelectNextAttack();
			END_EVENT(true)
												
			EVENT(msg_combat_struck) 
				SET_STATE(REACTSTATE) ;
			END_EVENT(true)
												
			EVENT(msg_button_special) 
				ME->GetAttackComponent()->StartSpecial() ;
			END_EVENT(true)
			
			EVENT(msg_external_control_start)
				SET_STATE(EXTERNALCONTROLSTATE);
			END_EVENT(true)
			
			EVENT(msg_combat_impaled)
				SET_STATE(IMPALEDDEADSTATE);
			END_EVENT(true)
		END_EVENTS
	END_STATE

	//!----------------------------------------------------------------------------------------------
	//! REACT STATE
	//!----------------------------------------------------------------------------------------------
	STATE(REACTSTATE)
		BEGIN_EVENTS
			ON_ENTER
				ME->GetAttackComponent()->Lua_SendRecoilMessage();
				ME->GetAIComponent()->SetDisabled(false);
				ME->GetAIComponent()->SetRecovering(true);
				CChatterBoxMan::Get().StopParticipantAudio(ME);
			END_EVENT(true)

			ON_EXIT
				// Disable the AI for the entity
				ME->GetAIComponent()->SetDisabled(true);
			END_EVENT(true)

			EVENT(msg_combat_impaled)
				SET_STATE(IMPALEDSTATE);
			END_EVENT(true)
			
			EVENT(msg_combat_killed)
				SET_STATE(DEADSTATE);
			END_EVENT(true)
			
			EVENT(msg_combat_breakout)
				if(ME->GetRangedWeapon())
				{
					SET_STATE(INTERACTINGSTATE);
				}
				else
				{
					SET_STATE(DEFAULTSTATE);
				}
			END_EVENT(true)
			
			EVENT(msg_combat_recovered)
				// Tell the combat to end recovery before moveing to default
				ME->GetAttackComponent()->CompleteRecovery();
				ME->GetAIComponent()->SetRecovering(false);
				
				if(ME->GetRangedWeapon())
				{
					SET_STATE(INTERACTINGSTATE);
				}
				else
				{
					SET_STATE(DEFAULTSTATE);
				}
			END_EVENT(true)
			
			EVENT(msg_combatsyncdreactionend)
				ME->GetAttackComponent()->EndSyncdReaction();
			END_EVENT(true)
															
			EVENT(msg_combat_countered)
				SET_STATE(COMBATSTATE);
			END_EVENT(true)
			
			EVENT(msg_combat_floored)
				////  Tell the combat system that the character has hit the floor
				ntPrintf("msg_combat_floored");
				ME->GetAttackComponent()->StartFlooredState();
				
				ME->HelperGetLuaFunc(CHashedString("GameStatistics"), CHashedString("Event"))(CHashedString("EnemyKO"));
				CChatterBoxMan::Get().Trigger("EnemyKO", ME);
			END_EVENT(true)
			
			EVENT(msg_combat_ragdollfloored)
				ME->HelperGetLuaFunc(CHashedString("GameStatistics"), CHashedString("Event"))(CHashedString("EnemyKO"));
				CChatterBoxMan::Get().Trigger("EnemyKO", ME);
			END_EVENT(true)
															
			EVENT(msg_combat_rise_wait)
				ME->GetAIComponent()->Lua_MakeConscious();
			END_EVENT(true)
			
			EVENT(msg_ko_aftertouch)
				ME->SetOtherCharacter((Character*)msg.GetEnt(CHashedString("Sender")));
				SET_STATE(KOAFTERTOUCH);
			END_EVENT(true)

			EVENT(msg_external_control_start)
				SET_STATE(EXTERNALCONTROLSTATE);
			END_EVENT(true)

			EVENT(msg_dropweapons)
				ME->DropWeapons();
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(KOAFTERTOUCH)
		BEGIN_EVENTS
			ON_ENTER
				ntPrintf("*** KO Aftertouch -> Enter ***")	;	
				// sounds on
				//aftertouchStartAudio()

				// SCEE_SWRIGHT: Using meaty obj cam def as there is no ragdoll one.
				//ME->m_iCameraHandle = CamMan::Get().GetPrimaryView()->ActivateAfterTouchCoolCam(ME, "RagdollCoolCamAfterTouchDef"); 
				ME->m_iCameraHandle = CamMan::Get().GetPrimaryView()->ActivateAfterTouchCoolCam(ME,"MeatyObjCoolCamAfterTouchDef"); 
				ME->GetAttackComponent()->SetKOAftertouch(true); // Enable aftertouch control on this AI
				
				ME->GetPhysicsSystem()->Lua_SendMessageOnCollision(true);
			END_EVENT(true)
			
			ON_EXIT
				if(ME->GetOtherCharacter())// Ensure we only do this once
				{
					ntPrintf("*** KO Aftertouch -> Exit ***\n");
					//ntPrintf("AI %s: State KOAFTERTOUCH ON_EXIT\n", ME->GetName().c_str() );


					// sounds off
					//aftertouchEndAudio()

					Message message(msg_exitstate);
					message.SetEnt( CHashedString(HASH_STRING_SENDER), ME);

					// Removing the 5 second delay on this message, if it really is needed then 
					// could try puting the delay in the player's KO aftertouch control state - T McK
					ME->GetOtherCharacter()->GetMessageHandler()->QueueMessage(message); //Player can return to their default state
	
					ME->GetAttackComponent()->SetKOAftertouch(false);
					CamMan::Get().GetPrimaryView()->RemoveCoolCamera( ME->m_iCameraHandle ); 
					
					ME->SetOtherCharacter(0); // Player is no longer interacting
					
					ME->GetPhysicsSystem()->Lua_SendMessageOnCollision(false);
				}
			END_EVENT(true)
			
			EVENT(msg_obj_collision)
				CamMan::Get().GetPrimaryView()->AfterTouchCoolCamLookAt(ME, 0);
			END_EVENT(true)

			EVENT(msg_interrupt) // Player is no longer 
				ME->GetAttackComponent()->SetKOAftertouch(false);
				ME->SetOtherCharacter(0);
				//aftertouchEndAudio()
			END_EVENT(true)

			EVENT(msg_combat_impaled)
				SET_STATE(IMPALEDSTATE);
			END_EVENT(true)
			
			EVENT(msg_combat_killed)
				SET_STATE(DEADSTATE);
			END_EVENT(true)
			
			EVENT(msg_combat_breakout)
			
	//GAVBEGIN - Go to interacting-state instead of default if holding a ranged weapon (so we can fire that weapon with msg_buttonaction to holder).
				if(ME->GetRangedWeapon())
				{
					SET_STATE(INTERACTINGSTATE);
				}
				else
				{
					SET_STATE(DEFAULTSTATE);
				}
	//GAVEND
			END_EVENT(true)
			
			EVENT(msg_combat_recovered)
				// Tell the combat to end recovery before moveing to default
				ME->GetAttackComponent()->CompleteRecovery();
				ME->GetAIComponent()->SetRecovering(false);
				
	//GAVBEGIN - Go to interacting-state instead of default if holding a ranged weapon (so we can fire that weapon with msg_buttonaction to holder).
				if(ME->GetRangedWeapon())
				{
					SET_STATE(INTERACTINGSTATE);
				}
				else
				{
					SET_STATE(DEFAULTSTATE);
				}
	//GAVEND
			END_EVENT(true)
			
			EVENT(msg_combatsyncdreactionend)
				ME->GetAttackComponent()->EndSyncdReaction();
			END_EVENT(true)
															
			EVENT(msg_combat_countered)
				SET_STATE(COMBATSTATE);
			END_EVENT(true)
			
			EVENT(msg_combat_floored)
				ntPrintf("msg_combat_floored");
				// Tell the combat system that the character has hit the floor
				ME->GetAttackComponent()->StartFlooredState();
				
				ME->HelperGetLuaFunc(CHashedString("GameStatistics"), CHashedString("Event"))(CHashedString("EnemyKO"));
				CChatterBoxMan::Get().Trigger("EnemyKO", ME);
			END_EVENT(true)
			
			EVENT(msg_combat_ragdollfloored)
				// Tell the combat system that the character has hit the floor
				ME->HelperGetLuaFunc(CHashedString("GameStatistics"), CHashedString("Event"))(CHashedString("EnemyKO"));
				CChatterBoxMan::Get().Trigger("EnemyKO", ME);
			END_EVENT(true)
															
			EVENT(msg_combat_rise_wait)
				ME->GetAIComponent()->Lua_MakeConscious();
			END_EVENT(true)
			
			EVENT(msg_attack_off) // We assume the player has already exited ko aftertouch state
				//aftertouchEndAudio()

				ME->GetAttackComponent()->SetKOAftertouch(false);
				CamMan::Get().GetPrimaryView()->RemoveCoolCamera( ME->m_iCameraHandle ); 
				ME->SetOtherCharacter(0); // Player is no longer interacting
			END_EVENT(true)

			EVENT(msg_external_control_start)
				SET_STATE(EXTERNALCONTROLSTATE);
			END_EVENT(true)
			
			EVENT(msg_ko_aftertouch)
			{
				ntPrintf("AI in ko aftertouch state has received msg_ko_aftertouch");
			
				// If the player attempts to KO a character thats already in KO aftertouch state, then tell the player to exit their KO aftertouch state immediately
				
				Message message(msg_exitstate);
				message.SetEnt( CHashedString(HASH_STRING_SENDER), ME);
				msg.GetEnt("Sender")->GetMessageHandler()->QueueMessage(message);
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE

	//!----------------------------------------------------------------------------------------------
	//! DISABLED STATE
	//!----------------------------------------------------------------------------------------------
	STATE(DISABLEDSTATE)
		BEGIN_EVENTS
			ON_ENTER
				ME->HelperGetLuaFunc(CHashedString("GameStatistics"), CHashedString("Event"))(CHashedString("EnemyDeath"));
				CChatterBoxMan::Get().Trigger("EnemyDeath", ME);
				CChatterBoxMan::Get().RemoveParticipant(ME); // - Chatterbox now handles this internally
				ME->GetMessageHandler()->ProcessEvent( "OnDeath" );
				
				ME->Hide();
				
				// Make the character into a corpse
				ME->GetMovement()->ClearControllers();
				ME->GetMovement()->SetEnabled(false);
				ME->GetAnimator()->Disable();
				ME->GetPhysicsSystem()->Deactivate();
				ME->GetAttackComponent()->MakeDead();
				ME->Lua_MakeDead();
			END_EVENT(true)
			
			EVENT(msg_external_control_start)
				SET_STATE(EXTERNALCONTROLSTATE);
			END_EVENT(true)
		END_EVENTS
	END_STATE

	// Death from being impaled by spear, etc
	// Objects like the spear have already turned the character to ragdoll, disabled their animator etc on their own accord in order for ragdoll behaviour to work correctly.
	//!----------------------------------------------------------------------------------------------
	//! IMPALED DEAD STATE
	//!----------------------------------------------------------------------------------------------
	STATE(IMPALEDDEADSTATE)
		BEGIN_EVENTS
			ON_ENTER

				ME->HelperGetLuaFunc(CHashedString("GameStatistics"), CHashedString("Event"))(CHashedString("EnemyDeath"));
				CChatterBoxMan::Get().StopParticipantAudio(ME);	
				CChatterBoxMan::Get().Trigger("EnemyDeath", ME);
				CChatterBoxMan::Get().RemoveParticipant(ME); // - Chatterbox now handles this internally
				ME->GetMessageHandler()->ProcessEvent( "OnDeath" );
			
				// Character drops anything they might be holding								
				ME->DropWeapons();
				
				ME->GetAttackComponent()->MakeDead();
				ME->Lua_MakeDead();
				
				SET_STATE(RAGDOLLINACTIVATESTATE); // We will assume the character is moving when they enter ragdoll state;
			END_EVENT(true)

			EVENT(msg_external_control_start)
				SET_STATE(EXTERNALCONTROLSTATE);
			END_EVENT(true)
		END_EVENTS
	END_STATE
	
	//!----------------------------------------------------------------------------------------------
	//! DEAD STATE
	//!----------------------------------------------------------------------------------------------
	STATE(DEADSTATE) 
		BEGIN_EVENTS
			ON_ENTER
				ME->HelperGetLuaFunc(CHashedString("GameStatistics"), CHashedString("Event"))(CHashedString("EnemyDeath"));
				CChatterBoxMan::Get().StopParticipantAudio(ME);
				CChatterBoxMan::Get().Trigger("EnemyDeath", ME);
				CChatterBoxMan::Get().RemoveParticipant(ME); //- Chatterbox now handles this internally
				ME->GetMessageHandler()->ProcessEvent( "OnDeath" );
			
				// Character drops anything they might be holding								
				ME->DropWeapons();
			
				// JML Commented out the below for aftertouch impact cams
				//ME->GetPhysicsSystem()->Lua_RemoveChildEntities(); // Destroy any projectiles that may be parented to this object
			
				// Make the character into a corpse
				ME->GetMovement()->ClearControllers();
				//ME->GetMovement()->SetEnabled(false);
				ME->GetAnimator()->Disable();
				//ME->GetPhysicsSystem()->Deactivate()
				ME->GetPhysicsSystem()->RegisterCollisionEffectFilterDef("RagdollPhysicsSoundDef");
				ME->GetAttackComponent()->MakeDead();
				ME->Lua_MakeDead();
				
				if (ME->GetAttackComponent()->GetCannotDieIntoRagdoll())
					SET_STATE(DEADWITHOUTRAGDOLLSTATE);
				else
				SET_STATE(RAGDOLLMOVINGSTATE); // We will assume the character is moving when they enter ragdoll state;
			END_EVENT(true)

			ON_UPDATE
			END_EVENT(true)

			//EVENT(msg_combat_killed)
			//	bool bUberKill;
			//	bUberKill = true;
			//	printf("Double Killing AI\n");
			//END_EVENT(true)

			EVENT(msg_external_control_start)
				SET_STATE(EXTERNALCONTROLSTATE);
			END_EVENT(true)
		END_EVENTS
	END_STATE

	//!----------------------------------------------------------------------------------------------
	//! DEAD STATE 2 (!?)
	//!----------------------------------------------------------------------------------------------
	STATE(AI_DeadState2)
		BEGIN_EVENTS
			ON_ENTER
				// Do we really need this state? I think it was just Mus doing stuff for a ragdoll demo yonks ago
				ntError(0);

				ME->HelperGetLuaFunc(CHashedString("GameStatistics"), CHashedString("Event"))(CHashedString("EnemyDeath"));
				CChatterBoxMan::Get().Trigger("EnemyDeath", ME);
				CChatterBoxMan::Get().RemoveParticipant(ME); //- Chatterbox now handles this internally
				ME->GetMessageHandler()->ProcessEvent( "OnDeath" );
			
				// Character drops anything they might be holding								
				ME->DropWeapons();
			
				// JML Commented out the below for aftertouch impact cams
				//ME->GetPhysicsSystem()->Lua_RemoveChildEntities(); // Destroy any projectiles that may be parented to this object
			
				// Make the character into a corpse
				ME->GetMovement()->ClearControllers();
				// ME->GetMovement()->SetEnabled(false);
				// ME->GetAnimator()->Disable();
				ME->GetPhysicsSystem()->Deactivate();
				ME->GetPhysicsSystem()->Lua_ActivateState("Ragdoll");
				ME->GetPhysicsSystem()->RegisterCollisionEffectFilterDef("RagdollPhysicsSoundDef");
				ME->GetAttackComponent()->MakeDead();
				ME->Lua_MakeDead();

				ME->GetAIComponent()->ActivateSingleAnim("Shieldman_Agony") ;
				ME->GetMovement()->SetCompletionMessage("msg_movementdone"); //defined below
				//ME->GetPhysicsSystem()->Ragdoll_Zombie();
				//SET_STATE(RAGDOLLMOVINGSTATE);
			END_EVENT(true)

			EVENT(msg_movementdone)
			{
				int iCount = msg.GetInt("Count");
				if(iCount++ >= 5)
				{
					//ME->GetPhysicsSystem()->Ragdoll_Dead();
					SET_STATE(RAGDOLLMOVINGSTATE);
				}
				else
				{
					ME->GetAIComponent()->ActivateSingleAnim("Shieldman_Agony") ;
					Message msg(msg_movementdone);
					msg.SetInt(CHashedString("Count"),			iCount);
					ME->GetMovement()->SetCompletionMessage(msg);
				}
			}
			END_EVENT(true)
			
			ON_UPDATE
			END_EVENT(true)

			EVENT(msg_external_control_start)
				SET_STATE(EXTERNALCONTROLSTATE);
			END_EVENT(true)
		END_EVENTS
	END_STATE
	
	//!----------------------------------------------------------------------------------------------
	//! IMPALED STATE
	//!----------------------------------------------------------------------------------------------
	STATE(IMPALEDSTATE) 
		BEGIN_EVENTS
			ON_ENTER
				// Character drops anything they might be holding
				ME->DropWeapons();
			
				ME->GetAttackComponent()->MakeDead();
				ME->Lua_MakeDead();
			END_EVENT(true)
			
			EVENT(msg_activateragdoll)
				SET_STATE(RAGDOLLINACTIVATESTATE);
			END_EVENT(true)
			
			EVENT(msg_external_control_start)
				SET_STATE(EXTERNALCONTROLSTATE);
			END_EVENT(true)
		END_EVENTS
	END_STATE
	
	//!----------------------------------------------------------------------------------------------
	//! INTERACTING WAIT STATE
	//!----------------------------------------------------------------------------------------------
	//Using msg_interactnamed to pick-up a weapon, put in a mandatory wait-period to allow the pickup to complete before
	//going into interacting state (so that the attrib.rangedweapon value will be accurate)
	//If either the timer runs out, or a msg_weaponpickup_success message is processed, we can move onto interacting-state.
	STATE(INTERACTINGWAITSTATE) 
		BEGIN_EVENTS
			ON_ENTER
			{
				Message message(msg_waiting_done);
				ME->GetMessageHandler()->QueueMessageDelayed(message, 7.f); //This is probably a bit cautious.
			}
			END_EVENT(true)

			EVENT(msg_weaponpickup_success)
				SET_STATE(INTERACTINGSTATE);
			END_EVENT(true)

			EVENT(msg_waiting_done)
				SET_STATE(INTERACTINGSTATE);
			END_EVENT(true)
		
			EVENT(msg_combat_killed)
				SET_STATE(DEADSTATE);
			END_EVENT(true)
		
			EVENT(msg_combat_impaled)
				SET_STATE(IMPALEDDEADSTATE);
			END_EVENT(true)
			
			EVENT(msg_movementdone)
			{
				Message message(msg_movementdone);
				ME->GetInteractionTarget()->GetMessageHandler()->QueueMessage(message);	//No delay on this one (or it'll freeze till msg_waiting_done!)
			}
			END_EVENT(true)

			EVENT(msg_external_control_start)
				SET_STATE(EXTERNALCONTROLSTATE);
			END_EVENT(true)

		END_EVENTS
	END_STATE


	//!----------------------------------------------------------------------------------------------
	//! INTERACTING STATE
	//!----------------------------------------------------------------------------------------------
	STATE(INTERACTINGSTATE)
		BEGIN_EVENTS
			ON_ENTER
				if (ME->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER ))
				{
					((Physics::AdvancedCharacterController*) ME->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER ))->SwitchCharacterControllerType(Physics::CharacterController::FULL_CONTROLLER);
				}

				if(ME->GetRangedWeapon())
				{
					ntPrintf("!!!!!Entering interacting state with ranged weapon\n");
					ME->GetAIComponent()->SetDisabled(false);
				}
			
				ME->SetExitOnMovementDone(false);
			
				if(!ME->GetInteractionTarget())
				{
					ntPrintf("Error in AI_InteractingState: InteractionTarget is nil!!\n");
					SET_STATE(DEFAULTSTATE);
					return true;
				}

				//Perform this check AFTER the above check for interaction target...
				if (((Interactable*)(ME->GetInteractionTarget()))->GetInteractableType()==Interactable::EntTypeInteractable_TurretWeapon)
				{
					ME->GetAIComponent()->SetDisabled(false);
				}
				
		END_EVENT(true)

			ON_EXIT
				if (ME->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER ))
				{
					((Physics::AdvancedCharacterController*) ME->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER ))->SwitchCharacterControllerType(Physics::CharacterController::FULL_CONTROLLER);
				}

				ME->GetAIComponent()->SetDisabled(true);
			END_EVENT(true)

			EVENT(msg_goto_defaultstate)
				SET_STATE(DEFAULTSTATE);
			END_EVENT(true);

			EVENT(msg_combat_killed)
				SET_STATE(DEADSTATE);
			END_EVENT(true)

			EVENT(msg_combat_impaled)
				SET_STATE(IMPALEDDEADSTATE);
			END_EVENT(true)
		
			EVENT(msg_combat_struck)
			{
				if(ME->GetInteractionTarget() && ME->GetInteractionTarget()->GetMessageHandler())
				{
					ME->GetInteractionTarget()->GetMessageHandler()->ReceiveMsg<msg_interrupt>();
				}

				// don't clear our interaction-target if we're holding a ranged-weapon (it should stay as that weapon instead).
				if(ME->GetRangedWeapon())
					ME->SetInteractionTarget(ME->GetRangedWeapon());
				else
					ME->SetInteractionTarget(0);
				
				SET_STATE(REACTSTATE);
			}
			END_EVENT(true)
											
			EVENT(msg_exitstate)
				// don't clear our interaction-target if we're holding a ranged-weapon (it should stay as that weapon instead).
				if(ME->GetRangedWeapon())
					ME->SetInteractionTarget(ME->GetRangedWeapon());
				else
					ME->SetInteractionTarget(0);

				SET_STATE(DEFAULTSTATE);
			END_EVENT(true)
		
			EVENT(msg_movementdone)
			
				if(ME->ExitOnMovementDone())
				{
					// It could be that there is special stuff to perform when leaving the 
					// interaction state
					
					
					// If the entity is a crossbow man...	
					/*if(ME->IsType("crossbowman"))
					{
						// We'll assume that the dude is dropping his crossbow and pulling out his sword.
						// The easiest way to do this is turn the entity in to a fodder charactor

						// Mark the entity as fodder and remove the crossbowman type					
						ME->GetEntityInfo()->AddDescription("fodder");
						ME->GetEntityInfo()->RemoveDescription("crossbowman");
						
						// remove all the crossbowman anims from the entity
						//Animator_RemoveAnimsContainer( "CrossbowmanAnimContainer" )
						//Animator_AddAnimsFromContainer( "FodderAnimContainer" )
						
						// Apply a new AI def to the entity. changes the anims set used, default pose, etc...
						ME->GetAIComponent()->ApplyNewAIDef( "EasyFodderAIDef" )
						
						// Create the sword on the entity
						Sword_Create()
									
						// The crossbow dropped can be picked up by the player... 
						// Cripple the weapon to a limited ammount of ammo, and enable
						// player crossbow attributes. 								
						GetInteractionTarget()->GetAttributeTable()->SetInt("Ammo", 5);
						GetInteractionTarget()->GetAttributeTable()->SetAttribute("SharedAttributes", "Att_RangedWeapon_Crossbow");
						//local objectAttrib = this.attrib.InteractionTarget.attrib
						//objectAttrib.Ammo = 5
						//objectAttrib.SharedAttributes = Att_RangedWeapon_Crossbow
					}*/
									
					ME->SetInteractionTarget(0);
					SET_STATE(DEFAULTSTATE);
				}
				else
				{
					CEntity* pMe = (CEntity*)ME;
					if(pMe->IsCharacter())
					{
						Character* pCharMe = pMe->ToCharacter();

						CEntity* pInteractionTarget = pCharMe->GetInteractionTarget();
						if(pInteractionTarget && pInteractionTarget->GetMessageHandler())
						{
							pInteractionTarget->GetMessageHandler()->ReceiveMsg<msg_movementdone>();
						}
					}
				}
			END_EVENT(true)
		
			EVENT(msg_buttonattack)
			{
				if(ME->GetInteractionTarget() && ME->GetInteractionTarget()->GetMessageHandler())
				{
					ME->GetInteractionTarget()->GetMessageHandler()->ReceiveMsg<msg_attack_on>();
				}
			}
			END_EVENT(true)
			
			EVENT(msg_buttongrab)
			{
				if(ME->GetInteractionTarget() && ME->GetInteractionTarget()->GetMessageHandler())
				{
					ME->GetInteractionTarget()->GetMessageHandler()->ReceiveMsg<msg_grab_on>();
				}
			}
			END_EVENT(true)	
			
			EVENT(msg_buttonaction)
			{
				if(ME->GetInteractionTarget() && ME->GetInteractionTarget()->GetMessageHandler())
				{
					ME->GetInteractionTarget()->GetMessageHandler()->ReceiveMsg<msg_action_on>();
				}
			}
			END_EVENT(true)
			
			EVENT(msg_button_power)
			{
				if(ME->GetInteractionTarget() && ME->GetInteractionTarget()->GetMessageHandler())
				{
					ME->GetInteractionTarget()->GetMessageHandler()->ReceiveMsg<msg_power_on>();
				}
			}
			END_EVENT(true)
											
			EVENT(msg_button_range)
			{
				if(ME->GetInteractionTarget() && ME->GetInteractionTarget()->GetMessageHandler())
				{
					ME->GetInteractionTarget()->GetMessageHandler()->ReceiveMsg<msg_range_on>();
				}
			}
			END_EVENT(true)
											
			EVENT(msg_release_attack)
			{
				if(ME->GetInteractionTarget() && ME->GetInteractionTarget()->GetMessageHandler())
				{
					ME->GetInteractionTarget()->GetMessageHandler()->ReceiveMsg<msg_attack_off>();
				}
			}
			END_EVENT(true)
											
			EVENT(msg_release_grab)
			{
				if(ME->GetInteractionTarget() && ME->GetInteractionTarget()->GetMessageHandler())
				{
					ME->GetInteractionTarget()->GetMessageHandler()->ReceiveMsg<msg_grab_off>();
				}
			}
			END_EVENT(true)	
											
			EVENT(msg_release_action)
			{
				if(ME->GetInteractionTarget() && ME->GetInteractionTarget()->GetMessageHandler())
				{
					ME->GetInteractionTarget()->GetMessageHandler()->ReceiveMsg<msg_action_off>();
				}
			}
			END_EVENT(true)
											
			EVENT(msg_release_power)
			{
				if(ME->GetInteractionTarget() && ME->GetInteractionTarget()->GetMessageHandler())
				{
					ME->GetInteractionTarget()->GetMessageHandler()->ReceiveMsg<msg_power_off>();
				}
			}
			END_EVENT(true)
											
			EVENT(msg_release_range)
			{
				if(ME->GetInteractionTarget() && ME->GetInteractionTarget()->GetMessageHandler())
				{
					ME->GetInteractionTarget()->GetMessageHandler()->ReceiveMsg<msg_range_off>();
				}
			}
			END_EVENT(true)
											
			EVENT(msg_external_control_start)
				SET_STATE(EXTERNALCONTROLSTATE);
			END_EVENT(true)

			EVENT(msg_interact)
			{
				// AI is trying to interact with something, despite being in the interacting state.
				// But if we are interacting with our ranged weapon, holster it and then use the object
				if ( ME->GetInteractionTarget() == ME->GetRangedWeapon() )
				{
					static CHashedString sTarget("target");
					Message msgAction(msg_action);
					msgAction.SetEnt(CHashedString(HASH_STRING_OTHER), (CEntity*)ME);

					msg.GetEnt(sTarget)->GetMessageHandler()->QueueMessage(msgAction);
					
					ME->GetAttackComponent()->DisallowSwitchStance();
					ME->SetInteractionTarget(msg.GetEnt("target"));
					
					// HOLSTER OUR WEAPON
					SET_STATE(HOLSTERINGRANGEDWEAPONSTATE);
				}
			}
			END_EVENT(true)
		END_EVENTS

		//!------------------------------------------------------------------------------------------
		//! SUBSTATE: HOLSTERING RANGED WEAPON STATE
		//!------------------------------------------------------------------------------------------
		STATE(HOLSTERINGRANGEDWEAPONSTATE)
			BEGIN_EVENTS
				ON_ENTER
				{
					ntAssert( ME->GetRangedWeapon() );

					//ntPrintf("*** AI HOLSTERING WEAPON ***\n");
					
					// Attach ranged weapon to head for comedy value
					// ... and to encourage animations to get made of course.
					ME->GetRangedWeapon()->Lua_Reparent(ME, "head");

					ME->GetAIComponent()->SetDisabled( false );

					// Set this flag to false each time, so only subsequent messages are taken into account
					ME->SetUnholsterGotoState( UNHOLSTER_GOTO_STATE_INTERACT );

					// Go into the holstered interacting sub state
					SET_STATE(HOLSTEREDRANGEDWEAPONSTATE);
				}
				END_EVENT(true)
			END_EVENTS
		END_STATE	// HOLSTERING RANGED WEAPON STATE

		//!------------------------------------------------------------------------------------------
		//! SUBSTATE: HOLSTERED RANGED WEAPON STATE
		//!------------------------------------------------------------------------------------------
		STATE(HOLSTEREDRANGEDWEAPONSTATE)
			BEGIN_EVENTS
				ON_ENTER
				{
				}
				END_EVENT(true)

				EVENT(msg_goto_defaultstate)
				{
					// Unholster then go to default state
					ME->SetUnholsterGotoState( UNHOLSTER_GOTO_STATE_DEFAULT );
					SET_STATE(UNHOLSTERINGRANGEDWEAPONSTATE);
				}
				END_EVENT(true);

				EVENT(msg_combat_struck)
				{
					ME->GetInteractionTarget()->GetMessageHandler()->ReceiveMsg<msg_interrupt>();	

					ME->SetInteractionTarget(ME->GetRangedWeapon());
					
					ME->SetUnholsterGotoState( UNHOLSTER_GOTO_STATE_REACT );
					SET_STATE(UNHOLSTERINGRANGEDWEAPONSTATE);
				}
				END_EVENT(true)

				EVENT(msg_exitstate)
				{
					// don't clear our interaction-target if we're holding a ranged-weapon (it should stay as that weapon instead).
					if(ME->GetRangedWeapon())
						ME->SetInteractionTarget(ME->GetRangedWeapon());
					else
						ME->SetInteractionTarget(0);

					ME->SetUnholsterGotoState( UNHOLSTER_GOTO_STATE_DEFAULT );
					SET_STATE(UNHOLSTERINGRANGEDWEAPONSTATE);
				}
				END_EVENT(true)

				EVENT(msg_movementdone)
				{
					if(ME->ExitOnMovementDone())
					{
						// We want to exit the ladder interaction, but then go back into the
						// interaction with our ranged weapon
						ME->SetInteractionTarget(ME->GetRangedWeapon());
						ME->SetUnholsterGotoState( UNHOLSTER_GOTO_STATE_INTERACT );
						SET_STATE(UNHOLSTERINGRANGEDWEAPONSTATE);
					}
					else
					{
						ME->GetInteractionTarget()->GetMessageHandler()->ReceiveMsg<msg_movementdone>();

						ME->SetInteractionTarget(ME->GetRangedWeapon());
						ME->SetUnholsterGotoState( UNHOLSTER_GOTO_STATE_INTERACT );
						SET_STATE(UNHOLSTERINGRANGEDWEAPONSTATE);
					}
				}
				END_EVENT(true)

			END_EVENTS
		END_STATE

		//!------------------------------------------------------------------------------------------
		//! SUBSTATE: UNHOLSTERING RANGED WEAPON STATE
		//!------------------------------------------------------------------------------------------
		STATE(UNHOLSTERINGRANGEDWEAPONSTATE)
			BEGIN_EVENTS
				ON_ENTER
				{
					ntAssert( ME->GetRangedWeapon() );

					//ntPrintf("*** AI UNHOLSTERING WEAPON ***\n");

					ME->GetRangedWeapon()->Lua_Reparent(ME, "r_weapon");

					ME->GetAIComponent()->SetDisabled(false);

					// Which state do we goto afterwards
					switch ( ME->GetUnholsterGotoState() )
					{
						case UNHOLSTER_GOTO_STATE_INTERACT:
							ntPrintf("UNHOLSTER - GOTO INTERACT\n");
							SET_STATE(INTERACTINGSTATE);
							break;

						case UNHOLSTER_GOTO_STATE_DEFAULT:
							ntPrintf("UNHOLSTER - GOTO DEFAULT\n");
							SET_STATE(DEFAULTSTATE);
							break;

						case UNHOLSTER_GOTO_STATE_REACT:
							ntPrintf("UNHOLSTER - GOTO REACT\n");
							SET_STATE(REACTSTATE);
							break;

						default:
							ntAssert_p(0, ("Unknown unholster goto state") );
							break;
					}
				}
				END_EVENT(true)
			END_EVENTS
		END_STATE	// UNHOLSTERING RANGED WEAPON STATE
	END_STATE
	
	STATE(DEADWITHOUTRAGDOLLSTATE)
		BEGIN_EVENTS
			ON_ENTER
				user_warn_p( false, ("***Ran out of ragdolls! Leaving in an animated state! ***\n"));
				ME->GetPhysicsSystem()->Deactivate();
			END_EVENT(true)
					
			EVENT(msg_external_control_start)
				SET_STATE(EXTERNALCONTROLSTATE);
			END_EVENT(true)
		END_EVENTS
	END_STATE
					
					
	/////////////////////////////////////////////////////////////////////////////////////////////////
	// RAGDOLL
	/////////////////////////////////////////////////////////////////////////////////////////////////

	//!----------------------------------------------------------------------------------------------
	//! RAGDOLL INACTIVATE STATE
	//!----------------------------------------------------------------------------------------------
	STATE(RAGDOLLINACTIVATESTATE)
	END_STATE

					
	//!----------------------------------------------------------------------------------------------
	//! RAGDOLL DEFAULT STATE
	//!----------------------------------------------------------------------------------------------
	STATE(RAGDOLLDEFAULTSTATE) 
		BEGIN_EVENTS
			ON_ENTER
			{
			    // Make sure the ragdoll can get picked up.
				if ( g_ShellOptions->CanPickupRagdolls() )
				{
					ME->GetInteractionComponent()->SetInteractionType(PICKUP);
				}
				else
				{
					ME->GetInteractionComponent()->SetInteractionType(NONE);
				}
				
				// Cancel any collision strike, from throws.
				ME->GetPhysicsSystem()->Lua_CollisionStrike(false);
				
				// If we were involved in a ragdoll throw, we would have been made exempt from cleanup, so now we're at rest, take away that exemption
				ME->GetPhysicsSystem()->Lua_Ragdoll_SetExemptFromCleanup(false);
				ME->SetOtherCharacter(0); // Sanity check.
				
				// Cancel any flexibility adjustments.
				ME->GetPhysicsSystem()->Lua_Ragdoll_SetBeingHeld(false);

#if defined( PLATFORM_PS3 )
				// Army demo. 
				if(ME->IsSpawned() && ME->GetArmyRenderable() )
				{
					// The chances are that an atrest message got the entity in to this state, but to check that was the case
					// ask the physics system again to send an at rest message
					Physics::AdvancedCharacterController* pobCharacter = (Physics::AdvancedCharacterController*) ME->GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);
					if(pobCharacter)
						pobCharacter->SetSendRagdollMessageOnAtRest("msg_atrest");
				}
#endif
			}

			END_EVENT(true)

			EVENT(msg_action)
			EVENT(msg_action_specific)
				ME->SetOtherCharacter((Character*)msg.GetEnt("Other"));
				SET_STATE(RAGDOLLMOVETOSTATE);
			END_EVENT(true)
			
			EVENT(msg_running_action)
			EVENT(msg_action_specific_run)
				ME->SetOtherCharacter((Character*)msg.GetEnt("Other"));
				SET_STATE(RAGDOLLRUNTOSTATE);
			END_EVENT(true)
			
			EVENT(msg_external_control_start)
				SET_STATE(EXTERNALCONTROLSTATE);
			END_EVENT(true)

			EVENT(msg_atrest)
#if defined( PLATFORM_PS3 )
				// TGS - if the entity is an AI and off-screen, then a better entity is likely to be around. 
				if(ME->IsDead() && ME->IsSpawned() && ME->GetArmyRenderable() )
				{
					SET_STATE(REMOVEFROMWORLD);
				}
#endif

			END_EVENT(true)

			ON_UPDATE
			END_EVENT(true)
			
			ON_EXIT
				// Stop the ragdoll getting picked up.
				ME->GetInteractionComponent()->SetInteractionType(NONE);
			END_EVENT(true)
		END_EVENTS
	END_STATE
	
				
	//!----------------------------------------------------------------------------------------------
	//! DEFAULT STATE
	//!----------------------------------------------------------------------------------------------
	STATE(RAGDOLLMOVETOSTATE)
		BEGIN_EVENTS
			ON_ENTER
				// Player moves into position before starting the pickup
				ME->GetOtherCharacter()->GetMovement()->Lua_StartMoveToTransition("hero_obj_goto", ME, 1, 1);
				ME->GetOtherCharacter()->GetMovement()->SetCompletionMessage( "msg_movementdone" );
				
				// Stop this ragdoll being picked up by anyone else or removed from the world.
				ME->GetInteractionComponent()->SetInteractionType(NONE);
				ME->GetPhysicsSystem()->Lua_Ragdoll_SetExemptFromCleanup(true);
			END_EVENT(true)
			
			EVENT(msg_interrupt)
				SET_STATE(RAGDOLLINTERRUPTSTATE);
			END_EVENT(true)
			
			EVENT(msg_movementdone)
				SET_STATE(RAGDOLLPICKUPSTATE);
			END_EVENT(true)
			
			EVENT(msg_external_control_start)
				SET_STATE(EXTERNALCONTROLSTATE);
			END_EVENT(true)
		END_EVENTS
	END_STATE
	
				
	//!----------------------------------------------------------------------------------------------
	//! DEFAULT STATE
	//!----------------------------------------------------------------------------------------------
	STATE(RAGDOLLRUNTOSTATE)
		BEGIN_EVENTS
			ON_ENTER
				// Player moves into position before starting the pickup
				ME->GetOtherCharacter()->GetMovement()->Lua_StartMoveToTransition("hero_obj_meaty_run_goto", ME, 1, 1);
				ME->GetOtherCharacter()->GetMovement()->SetCompletionMessage( "msg_movementdone" );
				
				// Stop this ragdoll being picked up by anyone else or removed from the world.
				ME->GetInteractionComponent()->SetInteractionType(NONE);
				ME->GetPhysicsSystem()->Lua_Ragdoll_SetExemptFromCleanup(true);
			END_EVENT(true)
			
			EVENT(msg_interrupt)
				SET_STATE(RAGDOLLINTERRUPTSTATE);
			END_EVENT(true)
			
			EVENT(msg_movementdone)
				SET_STATE(RAGDOLLRUNPICKUPSTATE);
			END_EVENT(true)
			
			EVENT(msg_external_control_start)
				SET_STATE(EXTERNALCONTROLSTATE);
			END_EVENT(true)
		END_EVENTS
	END_STATE
	
				
	//!----------------------------------------------------------------------------------------------
	//! DEFAULT STATE
	//!----------------------------------------------------------------------------------------------
	STATE(RAGDOLLPICKUPSTATE) 
		BEGIN_EVENTS
			ON_ENTER
				// Player performs the pickup animation.
				ME->GetOtherCharacter()->GetMovement()->Lua_AltStartFacingMovement("hero_obj_meaty_pickup",360,1,0,0,0);
				ME->GetOtherCharacter()->GetMovement()->SetCompletionMessage("msg_movementdone");

				// Disable collision between the player and the ragdoll.
				ME->GetOtherCharacter()->GetInteractionComponent()->ExcludeCollisionWith(ME); 
				ME->GetInteractionComponent()->ExcludeCollisionWith(ME->GetOtherCharacter()); 
				ME->GetOtherCharacter()->GetPhysicsSystem()->Lua_CharacterController_SetRagdollCollidable(false);

				// Switch the ragdoll to transform tracking (animated) mode, so that parts of the body are synchronised to the player's
				// movements.
				ME->GetPhysicsSystem()->Lua_Ragdoll_SetMotionType(4);
				ME->GetPhysicsSystem()->Lua_Ragdoll_SetAnimatedBones(CHARACTER_BONE_PELVIS); 
				
				// SCEE_SWright - Removing this, as it causes the ragdoll constraints to be violated and hence
				// jitter about like mad. See the function AdvancedRagdoll:::SetRagdollHeld for the alternate solution
				// of modifying the waist constraint.
				//ME->GetPhysicsSystem()->Lua_Ragdoll_AddAnimatedBone(CHARACTER_BONE_SPINE_00);
				
				// Reparent the ragdoll to the right weapon transform on the character
				ME->Lua_SetIdentity();
				ME->Lua_Reparent(ME->GetOtherCharacter(), "r_weapon"); 
				
				// Stop the ragdoll going fully dynamic or being removed from the world.
				ME->GetPhysicsSystem()->Lua_Ragdoll_SetTurnDynamicOnContact(false);
				ME->GetPhysicsSystem()->Lua_Ragdoll_SetExemptFromCleanup(true);
				
				// Adjust the ragdoll's flexibility to be more stable.
				ME->GetPhysicsSystem()->Lua_Ragdoll_SetBeingHeld(true);
			END_EVENT(true)
			
			EVENT(msg_interrupt)
				SET_STATE(RAGDOLLINTERRUPTSTATE);
			END_EVENT(true)
											
			EVENT(msg_movementdone)
				SET_STATE(RAGDOLLHELDSTATE);
			END_EVENT(true)
			
			EVENT(msg_external_control_start)
				SET_STATE(EXTERNALCONTROLSTATE);
			END_EVENT(true)
		END_EVENTS
	END_STATE
	
				
	//!----------------------------------------------------------------------------------------------
	//! DEFAULT STATE
	//!----------------------------------------------------------------------------------------------
	STATE(RAGDOLLRUNPICKUPSTATE) 
		BEGIN_EVENTS
			ON_ENTER
				// Player performs the pickup animation.
				ME->GetOtherCharacter()->GetMovement()->Lua_AltStartFacingMovement("hero_obj_meaty_run_pickup",360,1,0,0,0);
				ME->GetOtherCharacter()->GetMovement()->SetCompletionMessage("msg_movementdone");	

				// Disable collision between the player and the ragdoll.
				ME->GetOtherCharacter()->GetInteractionComponent()->ExcludeCollisionWith(ME);
				ME->GetInteractionComponent()->ExcludeCollisionWith(ME->GetOtherCharacter()); 
				ME->GetOtherCharacter()->GetPhysicsSystem()->Lua_CharacterController_SetRagdollCollidable(false);

				// Switch the ragdoll to transform tracking (animated) mode, so that parts of the body are synchronised to the player's
				// movements.
				ME->GetPhysicsSystem()->Lua_Ragdoll_SetMotionType(4);
				ME->GetPhysicsSystem()->Lua_Ragdoll_SetAnimatedBones(CHARACTER_BONE_PELVIS); 
				
				// SCEE_SWright - Removing this, as it causes the ragdoll constraints to be violated and hence
				// jitter about like mad. See the function AdvancedRagdoll:::SetRagdollHeld for the alternate solution
				// of modifying the waist constraint.
				//ME->GetPhysicsSystem()->Lua_Ragdoll_AddAnimatedBone(CHARACTER_BONE_SPINE_00);
				
				// Reparent the ragdoll to the right weapon transform on the character
				ME->Lua_SetIdentity();
				ME->Lua_Reparent(ME->GetOtherCharacter(), "r_weapon"); 
				
				// Stop the ragdoll going fully dynamic or being removed from the world.
				ME->GetPhysicsSystem()->Lua_Ragdoll_SetTurnDynamicOnContact(false);
				ME->GetPhysicsSystem()->Lua_Ragdoll_SetExemptFromCleanup(true);
				
				// Adjust the ragdoll's flexibility to be more stable.
				ME->GetPhysicsSystem()->Lua_Ragdoll_SetBeingHeld(true);
			END_EVENT(true)
			
			EVENT(msg_interrupt)
				SET_STATE(RAGDOLLINTERRUPTSTATE);
			END_EVENT(true)
											
			EVENT(msg_movementdone)
				SET_STATE(RAGDOLLHELDSTATE)																												;
			END_EVENT(true)
			
			EVENT(msg_external_control_start)
				SET_STATE(EXTERNALCONTROLSTATE);
			END_EVENT(true)
		END_EVENTS
	END_STATE
	
				
	//!----------------------------------------------------------------------------------------------
	//! DEFAULT STATE
	//!----------------------------------------------------------------------------------------------
	STATE(RAGDOLLHELDSTATE) 
		BEGIN_EVENTS
			ON_ENTER
				// Chase cam is activated
				CamMan::Get().GetPrimaryView()->ActivateChaseAimCam(ME->GetOtherCharacter(), "RagdollChaseCamProps", "RagdollAimCamProps");

				// Player begins their holding movement, 
				ME->GetOtherCharacter()->GetMovement()->Lua_StartMovementFromXMLDef("MovementWithRagdoll"); 
				ME->GetOtherCharacter()->GetPhysicsSystem()->Lua_SetHoldingCapsule(true);
				
				// Aiming mode starts deactivated.
				ME->GetOtherCharacter()->Lua_ResetAimingComponent();
				ME->m_bRagdollAiming = false;
			END_EVENT(true)
			
			
			//!----------------------------------------------------------------------------------------------
			// Throw/Drop
			//!----------------------------------------------------------------------------------------------
			EVENT(msg_power_on)
			EVENT(msg_attack_on)
			EVENT(msg_action_on)
				SET_STATE(RAGDOLLTHROWSTATE);
			END_EVENT(true)
			
			EVENT(msg_grab_on)
				SET_STATE(RAGDOLLDROPSTATE);
			END_EVENT(true)

			
			//!----------------------------------------------------------------------------------------------
			// Events switching into and out of aiming mode
			//!----------------------------------------------------------------------------------------------
			EVENT(msg_aim_on)
				// Player begins their aiming movement
				ME->GetOtherCharacter()->GetMovement()->Lua_StartMovementFromXMLDef("RagdollAim_Movement");
				ME->GetOtherCharacter()->Lua_ResetAimingComponent();
					
				// Set aiming flag
				ME->m_bRagdollAiming = true;
			END_EVENT(true)
		
			EVENT(msg_aim_off)
				// Player ends aiming movement.
				ME->GetOtherCharacter()->GetMovement()->Lua_StartMovementFromXMLDef("MovementWithRagdoll"); 
				ME->GetOtherCharacter()->Lua_ResetAimingComponent();
				
				// Reset aiming flag
				ME->m_bRagdollAiming = false;
			END_EVENT(true)
			
			
			EVENT(msg_interrupt)
				SET_STATE(RAGDOLLINTERRUPTSTATE);
			END_EVENT(true)
			
			EVENT(msg_external_control_start)
				SET_STATE(EXTERNALCONTROLSTATE);
			END_EVENT(true)
		END_EVENTS
	END_STATE
	
				
				
	//!----------------------------------------------------------------------------------------------
	//! DEFAULT STATE
	//!----------------------------------------------------------------------------------------------
	STATE(RAGDOLLTHROWSTATE) 
		BEGIN_EVENTS
			// Any longer than this and the ragdolls will get launched to late in the animation and go styaight into the ground.
			// Any shorter and the throws will become too sensitive to aftertouch.
			#define THROW_DELAY	( 0.25f ) 
			
			ON_ENTER
			{
				// Start the player throw anim
				if(ME->m_bRagdollAiming)
				{
					ME->GetOtherCharacter()->GetMovement()->Lua_StartMovementFromXMLDef("RagdollThrow_Movement");
					ME->GetOtherCharacter()->GetMovement()->SetCompletionMessage( "msg_movementdone" );
				}
				else
				{
					ME->GetOtherCharacter()->GetMovement()->Lua_AltStartTargetedFacingMovement("hero_obj_meaty_throw",360.0f, 1.0f, 0.0f, 0.1f); 
					ME->GetOtherCharacter()->GetMovement()->SetCompletionMessage( "msg_movementdone" );
				}   		

				// Ensure collision strike handler is enabled. 
				ME->GetPhysicsSystem()->Lua_CollisionStrike(true); 
				
				// Queue up the actual throw event for the ragdoll.
				Message message(msg_think_onthrow);
				ME->GetMessageHandler()->QueueMessageDelayed(message, THROW_DELAY);
			}
			END_EVENT(true)

			
			EVENT(msg_think_onthrow)
			{
				// Reparent the ragdoll back to the world.
				ME->Lua_ReparentToWorld(); 
				
				// Decide whether aftertouch effects are allowed... if button held for long enough.
				bool use_aftertouch = (ME->GetOtherCharacter()->Lua_IsPowerHeld(THROW_DELAY) || 
									   ME->GetOtherCharacter()->Lua_IsAttackHeld(THROW_DELAY) ||
									   ME->GetOtherCharacter()->Lua_IsActionHeld(THROW_DELAY));
				
				// Turn the ragdoll fully dynamic, in case not already.
				ME->GetPhysicsSystem()->Lua_Ragdoll_SetMotionType(0);
				
				// Bloody SetMotionType() turns off the collision strike handler, so re-enable it!
				ME->GetPhysicsSystem()->Lua_CollisionStrike(true); 
				
				// Perform the throw.
				ME->Ragdoll_StartAimedThrow(ME->GetOtherCharacter(),"RagdollAftertouchControlParameters",use_aftertouch);
				ME->GetMovement()->SetCompletionMessage( "msg_throw_end" );
				
				// Cancel any flexibility adjustments.
				ME->GetPhysicsSystem()->Lua_Ragdoll_SetBeingHeld(false);
				
				// Start aftertouch effects.
				if(use_aftertouch)
				{
					SET_STATE(RAGDOLLAFTERTOUCHSTATE);
				}
			}
			END_EVENT(true)
											
			
			EVENT(msg_movementdone)
			{
				// Reset chase cam 
				CamMan::Get().GetPrimaryView()->RemoveCoolCamera( ME->m_iCameraHandle ); 
				
				// Clear any special controllers.
				ME->GetMovement()->ClearControllers();
				
				// Make sure the ragdoll generates an AtRest message
				ME->GetPhysicsSystem()->Lua_Ragdoll_CheckAtRest(); 
					
				// End the interaction, restoring collision with other entity.
				if ( ME->GetOtherCharacter() )
				{
					Message message(msg_exitstate);
					message.SetEnt( CHashedString(HASH_STRING_SENDER), ME);
					ME->GetOtherCharacter()->GetMessageHandler()->QueueMessage(message);
					
					ME->GetInteractionComponent()->AllowCollisionWith(ME->GetOtherCharacter());
					ME->GetOtherCharacter()->GetInteractionComponent()->AllowCollisionWith(ME); 
					ME->SetOtherCharacter(0);
				}
				SET_STATE(RAGDOLLMOVINGSTATE);
			}
			END_EVENT(true)
			
			
			EVENT(msg_interrupt)
				SET_STATE(RAGDOLLINTERRUPTSTATE);
			END_EVENT(true)
			
			EVENT(msg_atrest)			
				SET_STATE(RAGDOLLDEFAULTSTATE);
			END_EVENT(true)
		END_EVENTS
	END_STATE

				
				
	//!----------------------------------------------------------------------------------------------
	//! DEFAULT STATE
	//!----------------------------------------------------------------------------------------------
	STATE(RAGDOLLAFTERTOUCHSTATE)
		BEGIN_EVENTS
			ON_ENTER
			{
				// Send a message when we've hit something 'msg_obj_collision'
				ME->GetPhysicsSystem()->Lua_SendMessageOnCollision( true );
				
				// Activate aftertouch camera.
				ME->m_iCameraHandle = CamMan::Get().GetPrimaryView()->ActivateAfterTouchCoolCam(ME,"MeatyObjCoolCamAfterTouchDef"); 
				
				// After touch control stuff.
				ME->GetPhysicsSystem()->Lua_Ragdoll_Twitch();
			}
			END_EVENT(true)
			
			
			EVENT(msg_obj_collision)
			{
				// Adjust cool collision camera.
				CamMan::Get().GetPrimaryView()->AfterTouchCoolCamLookAt(ME, 0);
				
				// No more messages please. 
				ME->GetPhysicsSystem()->Lua_SendMessageOnCollision( false );
				
				// Delay the end of the aftertouch state. 
				Message message(msg_power_off);
				ME->GetMessageHandler()->QueueMessageDelayed(message, 1.5f);
			}
			
			
			EVENT(msg_action_off)
			EVENT(msg_attack_off)
			EVENT(msg_power_off)
			{
				// Reset aftertouch cam 
				CamMan::Get().GetPrimaryView()->RemoveCoolCamera( ME->m_iCameraHandle );
				
				// Clear any special controllers.
				ME->GetMovement()->ClearControllers();
				
				// Make sure the rigid body generates an at-rest message, before reverting to default state.
				ME->GetPhysicsSystem()->Lua_Ragdoll_CheckAtRest(); 
				
				// End the interaction, restoring collision with the other entity.
				if(ME->GetOtherCharacter()) 
				{
					Message message(msg_exitstate);
					message.SetEnt( CHashedString(HASH_STRING_SENDER), ME);
					ME->GetOtherCharacter()->GetMessageHandler()->QueueMessage(message);
					
					ME->GetInteractionComponent()->AllowCollisionWith(ME->GetOtherCharacter());
					ME->GetOtherCharacter()->GetInteractionComponent()->AllowCollisionWith(ME); 
					ME->SetOtherCharacter(0);
				}				
				SET_STATE(RAGDOLLMOVINGSTATE);
			}
			END_EVENT(true)
			
			
			EVENT(msg_interrupt)
				SET_STATE(RAGDOLLINTERRUPTSTATE);
			END_EVENT(true)
			
			EVENT(msg_atrest)
				SET_STATE(RAGDOLLDEFAULTSTATE);
			END_EVENT(true)

			ON_EXIT
				ME->GetPhysicsSystem()->Lua_SendMessageOnCollision( false );
			END_EVENT(true)
		END_EVENTS
	END_STATE
	
				
				
	//!----------------------------------------------------------------------------------------------
	//! DEFAULT STATE
	//!----------------------------------------------------------------------------------------------
	STATE(RAGDOLLDROPSTATE) 
		BEGIN_EVENTS
			ON_ENTER
			{
				// Reset chase cam 
				CamMan::Get().GetPrimaryView()->RemoveCoolCamera( ME->m_iCameraHandle );
				
				// Perform the drop movement.
				ME->GetOtherCharacter()->GetMovement()->Lua_AltStartFacingMovement("hero_obj_meaty_drop",360,1,0,0,0); 
				ME->GetOtherCharacter()->GetMovement()->SetCompletionMessage( "msg_movementdone" );
				ME->GetOtherCharacter()->SetExitOnMovementDone(true); 
								
				// Inform the other character they are dropping us.
				Message message(msg_dropragdoll);
				ME->GetMessageHandler()->QueueMessageDelayed(message, 0.1f);
			}
			END_EVENT(true)

			
			EVENT(msg_dropragdoll)
			{
				// Reparent the ragdoll back to the world.
				ME->Lua_ReparentToWorld();
				
				// Turn the ragdoll fully dynamic again.
				ME->GetPhysicsSystem()->Lua_Ragdoll_SetMotionType(0); 

				// Apply a drop velocity relative to the player's current direction.
				CDirection velocity(-1.f,0.f,0.f);
				velocity = velocity * ME->GetOtherCharacter()->GetMatrix();
				ME->GetPhysicsSystem()->SetLinearVelocity(velocity);
				
				// When safely clear, end the interaction.
				Message message(msg_end_ragdollhold);
				ME->GetMessageHandler()->QueueMessageDelayed(message, 1.0f);
			}
			END_EVENT(true)
			

			EVENT(msg_end_ragdollhold)
			{
				// Make sure the rigid body generates an at-rest message 
				ME->GetPhysicsSystem()->Lua_Ragdoll_CheckAtRest(); 
			
				// Cancel any flexibility adjustments.
				ME->GetPhysicsSystem()->Lua_Ragdoll_SetBeingHeld(false);
				
				// End the interaction. Restore collision with other entity.
				if ( ME->GetOtherCharacter() )
				{
					ME->GetInteractionComponent()->AllowCollisionWith(ME->GetOtherCharacter());
					ME->GetOtherCharacter()->GetInteractionComponent()->AllowCollisionWith(ME); 
					ME->GetOtherCharacter()->GetPhysicsSystem()->Lua_CharacterController_SetRagdollCollidable(true);
					ME->SetOtherCharacter(0);
				}
			}
			END_EVENT(true)
			
			EVENT(msg_atrest)
				SET_STATE(RAGDOLLDEFAULTSTATE);
			END_EVENT(true)
		END_EVENTS
	END_STATE
	
				
				
	//!----------------------------------------------------------------------------------------------
	//! DEFAULT STATE
	//!----------------------------------------------------------------------------------------------
	STATE(RAGDOLLINTERRUPTSTATE) 
		BEGIN_EVENTS
			ON_ENTER
				// Make sure ragdoll is fully dynamimc and parented to the world (not other entities).
				ME->Lua_ReparentToWorld();
				ME->GetPhysicsSystem()->Lua_Ragdoll_SetMotionType(0); // This also disables collision strike handler.

				// Disassociate from other characters and restore collisions.
				if(ME->GetOtherCharacter())
				{
					ME->GetInteractionComponent()->AllowCollisionWith(ME->GetOtherCharacter());
					ME->GetOtherCharacter()->GetInteractionComponent()->AllowCollisionWith(ME); 
					ME->GetOtherCharacter()->GetPhysicsSystem()->Lua_CharacterController_SetRagdollCollidable(true);
					ME->SetOtherCharacter(0);
				}
				
				// Be sure to send at-rest messages.
				ME->GetPhysicsSystem()->Lua_Ragdoll_CheckAtRest();
			END_EVENT(true)
			
			EVENT(msg_atrest)
				SET_STATE(RAGDOLLDEFAULTSTATE);
			END_EVENT(true)
			
			EVENT(msg_external_control_start)
				SET_STATE(EXTERNALCONTROLSTATE);
			END_EVENT(true)
			
		END_EVENTS
	END_STATE
	
				
				
	//!----------------------------------------------------------------------------------------------
	//! DEFAULT STATE
	//!----------------------------------------------------------------------------------------------
	STATE(RAGDOLLMOVINGSTATE) 
		BEGIN_EVENTS
			ON_ENTER
				// Be sure to send at-rest messages.
				ME->GetPhysicsSystem()->Lua_Ragdoll_CheckAtRest();
				ME->SetOtherCharacter(0); // Sanity check.
			END_EVENT(true)
	
			EVENT(msg_collision)
				// Make sure the ragdoll is travelling above the threshold velocity
				if(msg.GetFloat("ProjVel") > -4.0 && msg.GetFloat("ProjVel") < 4.0) 
				{
					return true;
				}

				// Try and send a combat strike to our target
				if(msg.GetEnt("Collidee") && !msg.GetEnt("Collidee")->IsPlayer())
				{
					CombatHelper::Combat_GenerateStrike(ME, ME, msg.GetEnt("Collidee"), "atk_corpse_strike"); 
				}
			END_EVENT(true)
			
			EVENT(msg_atrest)
				SET_STATE(RAGDOLLDEFAULTSTATE);
			END_EVENT(true)
			
			EVENT(msg_external_control_start)
				SET_STATE(EXTERNALCONTROLSTATE);
			END_EVENT(true)
		END_EVENTS
	END_STATE
	
				
				
	//!----------------------------------------------------------------------------------------------
	//! REMOVE FROM WORLD STATE
	//!----------------------------------------------------------------------------------------------
	STATE(REMOVEFROMWORLD)
		BEGIN_EVENTS	
			ON_UPDATE
			{
				// Try to remove the entity from the world. Should the entity not remove itself from the world 
				// then process it again next frame.
				if( ME->CanRemoveFromWorld() )
				{
					ME->RemoveFromWorld();
				}
			}
			END_EVENT(true)

			// Handle recovery messages.
			EVENT(msg_combat_recovered) 
				ME->GetAttackComponent()->CompleteRecovery();
				ME->GetAIComponent()->SetRecovering(false);
			END_EVENT(true)

		END_EVENTS
	END_STATE


	//!----------------------------------------------------------------------------------------------
	//! Global Handlers
	//!----------------------------------------------------------------------------------------------
	//BEGIN_EVENTS
	//END_EVENTS

END_STATEMACHINE //AI_FSM


//!----------------------------------------------------------------------------------------------
//!
//!	AI::AI()
//!	Default constructor
//!
//!----------------------------------------------------------------------------------------------
AI::AI()
:	m_pobAIComponent(0),
	m_pSpawnPool(0),
	m_iHasBeenInUnsafePos(0), m_iChatGroupID(0)
{
	m_eType = EntType_AI;
	m_eCharacterType = Character::CT_AI;

	// Ranged Weapon Holstering
	m_eUnholsterGotoState = UNHOLSTER_GOTO_STATE_INTERACT;

	// Set the default visible score weights. 
	DefaultVisibleScoreWeight();

	m_pArmyRenderable = 0;

	m_iCameraHandle = 0;

	ATTACH_LUA_INTERFACE(AI);
}

//!----------------------------------------------------------------------------------------------
//!
//!	AI::~AI()
//!	Default destructor
//!
//!----------------------------------------------------------------------------------------------
AI::~AI()
{
#if defined( PLATFORM_PS3 )
	if( m_pArmyRenderable )
	{
		m_pArmyRenderable->TurnFromRealDude( false );
		m_pArmyRenderable = 0;
	}
#endif

	// If we have an AI component, destroy it
	NT_DELETE_CHUNK( Mem::MC_ENTITY, m_pobAIComponent);
	m_pobAIComponent = 0;
}


//!----------------------------------------------------------------------------------------------
//!
//!	AI::OnPostConstruct
//!	Post Construction
//!
//!----------------------------------------------------------------------------------------------
void AI::OnPostConstruct()
{
	// Drop any construction script since we no longer want one...
	m_ConstructionScript = 0;

	// Character PostConstruction...
	Character::OnPostConstruct();

	// Get our AI definition
	CAIComponentDef* pDef = ObjectDatabase::Get().GetPointerFromName<CAIComponentDef*>(m_sAIDefinition);
	if(!pDef)
	{
		user_error_p(0, ("AI Component Definition: %s does not exist. The game will not stop.", ntStr::GetString(m_sAIDefinition)) );
		return;
	}

	// Check that the component is null
	ntError( m_pobAIComponent == 0 && "m_pobAIComponent is not false..." )

	// Oh, what wonderful things the Scarecrow could do......If He Only Had a........
	m_pobAIComponent = NT_NEW_CHUNK ( Mem::MC_ENTITY ) CAIComponent(this, pDef);
	m_pobAIComponent->Constructed();

	// (DARIO) : This is moved to  ::OnPostPostCOnstruct()

	//// Set up the AI controller lua script...
	//if(!ntStr::IsNull(m_sController))
	//{
	//	static NinjaLua::LuaObject luaNil(CLuaGlobal::Get().State());
	//	static CHashedString sFunc("MakeAIController");
	//	HelperGetLuaFunc(sFunc)(m_sController, luaNil, GetAttrib());
	//}

	if(!m_pobAIComponent)
	{
		ntPrintf("AI %s has no ai component\n", GetName().c_str());
		return;
	}

	//! // This section from AI_State.Setup
	//By default all entities AI are disabled, and states that are known to work will have them enabled. 
	m_pobAIComponent->SetDisabled(true);
	
	// TODO: Change this to an enumeration type instead
	if ( m_sInitialSystemState == "" || ntStr::IsNull(m_sInitialSystemState) )
		m_sInitialSystemState = "AI_DefaultState";
	
	// Create and attach the statemachine
	AI_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) AI_FSM(m_sInitialSystemState);
	ATTACH_FSM(pFSM);
}


//!----------------------------------------------------------------------------------------------
//!
//!	AI::OnPostPostConstruct
//!	Post Post Construction
//!
//!----------------------------------------------------------------------------------------------
void AI::OnPostPostConstruct()
{
	CEntity::OnPostPostConstruct();
}

//--------------------------------------------------
//!
//!	AI::OnLevelStart()
//!	Called for each ent on level startup
//!
//--------------------------------------------------
void AI::OnLevelStart()
{
	// Set up the AI controller lua script...
	// have moved to here as AI controllers require animations to be present,

	// Must be done AFTER anim containers fixed up by area system
	// i.e. after XML serialisation. OR this shouldnt play an animation

	if(!ntStr::IsNull(m_sController))
	{
		static NinjaLua::LuaObject luaNil(CLuaGlobal::Get().State());
		static CHashedString sFunc("MakeAIController");
		HelperGetLuaFunc(sFunc)(m_sController, luaNil, GetAttrib());
	}
}

//!----------------------------------------------------------------------------------------------
//!
//!	AI::Ragdoll_StartAimedThrow
//!	
//!
//!----------------------------------------------------------------------------------------------
void AI::Ragdoll_StartAimedThrow(CEntity* pController, CHashedString sParams, bool bAftertouch)
{
	if ( !bAftertouch )
	{
		// Get the ragdoll's torso.
		Physics::AdvancedCharacterController* pobCharacter = (Physics::AdvancedCharacterController*)GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);
		ntAssert( pobCharacter && pobCharacter->GetAdvancedRagdoll() );
		Physics::AdvancedRagdoll* pobRagdoll = pobCharacter->GetAdvancedRagdoll();
		hkRigidBody *pobTorso = pobRagdoll->GetRagdollBone( pobRagdoll->GetBoneIndexFromFlag( Physics::AdvancedRagdoll::SPINE ) );
		
		// Apply an impulse to the ragdoll along the player's facing direction.
		CDirection obImpulse = pController->GetMatrix().GetZAxis(); 
		float fImpulseStrength = 100.0f / obImpulse.Length();
	   
		obImpulse *= fImpulseStrength;
		hkVector4 obhkImpulse(obImpulse.X(), obImpulse.Y(), obImpulse.Z() );
		pobTorso->applyLinearImpulse( obhkImpulse );
	}
	else
	{
		// Set up a raycast in the entity's facing direction.
	    CDirection obFaceDir( pController->GetMatrix().GetZAxis() );
		obFaceDir.Normalise();
		CPoint ptStart = GetPosition() + obFaceDir;
		CPoint ptEnd   = ptStart + ( obFaceDir * 50.0f );
	
		// Raycast collision filter.
		Physics::RaycastCollisionFlag obFlag; obFlag.base = 0;
		obFlag.flags.i_am = Physics::COLLISION_ENVIRONMENT_BIT;
		obFlag.flags.i_collide_with = (	Physics::CHARACTER_CONTROLLER_ENEMY_BIT		|
										//Physics::RAGDOLL_BIT						| // Don't want to collide with ourselves!
										Physics::SMALL_INTERACTABLE_BIT				|
										Physics::LARGE_INTERACTABLE_BIT				);

		// Perform raycast query.
		Physics::TRACE_LINE_QUERY stQuery;
		RagdollThrownControllerDef obDef;
		CDirection dNewVelocity;

		if (Physics::CPhysicsWorld::Get().TraceLine(ptStart,ptEnd,this,stQuery,obFlag))
		{
			dNewVelocity = CDirection( stQuery.obIntersect - GetPosition() ); 
		}
		else
		{
			dNewVelocity = CDirection( ptEnd - GetPosition() );
		}
	
		// Add a bit of upwards angle to the throw.
		dNewVelocity.Normalise();
		dNewVelocity.Y() = 0.1f;
		dNewVelocity.Normalise();

		// Set the linear velocity for the throw.
		dNewVelocity *= 20.0f; 
		obDef.m_obLinearVelocity = dNewVelocity;

		// Set aftertouch controller.
		obDef.m_pobControllingEntity = pController;

		// Get aftertouch control parameters.
		obDef.m_pobParameters=ObjectDatabase::Get().GetPointerFromName<AftertouchControlParameters*>(sParams);
		ntAssert( obDef.m_pobParameters );
	
		// Activate new controller for thrown ragdoll.
		GetMovement()->BringInNewController(obDef, CMovement::DMM_STANDARD, 0.f);
	}
}



//!----------------------------------------------------------------------------------------------
//!
//!	AI::Respawn
//!	This AI needs to be brought back to life, re-equipped, reset and respawned
//!
//!----------------------------------------------------------------------------------------------
bool AI::Respawn(const ntstd::Vector<CEntity*>& vecWeapons)
{
	// Unpause it
	//---------------------------------------------
	Pause(false, true);

	// Add all it's renderables
	//---------------------------------------------
	Show();

	// Ressurect it
	//---------------------------------------------
	SetDead(false);
	GetAttackComponent()->MakeUndead();
	GetPhysicsSystem()->UnfixRagdoll();
	GetMovement()->ClearControllers();
	GetAnimator()->Enable();
	GetMessageHandler()->ClearMessageQueues();
	GetAIComponent()->Reset();
	SetKillInNextFrame(false);

	// Reboot it's FSM
	//---------------------------------------------
	//ntPrintf("Spawn AI (%s)\n", GetName().c_str());
	//Debug::AlwaysOutputString("Spawn AI\n");
	if(m_sInitialSystemState == "AI_ExternalControlState")
	{
		EXTERNALLY_SET_STATE(AI_FSM, EXTERNALCONTROLSTATE);
	}
	else
	{
		EXTERNALLY_SET_STATE(AI_FSM, DEFAULTSTATE);
	}

	Message msg(State_Enter);
	m_pFSM->ProcessMessage(msg);

	// Lua Controllers handle this.
	//---------------------------------------------
	Message msgSpawned(msg_spawned);
	GetMessageHandler()->Receive(msgSpawned);

	// Assign and ready weapons
	//---------------------------------------------
	if(m_pWeaponsDef)
	{
		if(!m_pWeaponsDef->CreateWeapons(this, vecWeapons))
		{
// TGS HACK
#if defined( PLATFORM_PS3 )
			if(GetArmyRenderable())
			{
				return true;
			}
#endif
			GetSpawnPool()->DeSpawn(this);
			return false;

		}
	}

#if defined( PLATFORM_PS3 )
	// If this is an army dude - then call a function when the ragdoll is at rest. 
	if( GetArmyRenderable() )
	{
		Physics::AdvancedCharacterController* pobCharacter = (Physics::AdvancedCharacterController*) GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);
		
		if(pobCharacter)
		{
			pobCharacter->SetSendRagdollMessageOnAtRest("msg_atrest");
		}
	}
#endif

	return true;
}

//------------------------------------------------------------------------------------------
//!  public  DefaultVisibleScoreWeight
//!
//!
//!  @remarks 
//!
//!  @author GavB @date 06/09/2006
//------------------------------------------------------------------------------------------
void AI::DefaultVisibleScoreWeight( void )
{
	// Static default weights.
	static AIFactScoreWeights s_AIFactScoreDefaults;

	// Assign the defaults
	m_pVisibleScoreWeights = &s_AIFactScoreDefaults;
}

//------------------------------------------------------------------------------------------
//!  public  SetVisibleScoreWeight
//!
//!  @param [in]       rXMLObjectName const CHashedString &    <TODO: insert parameter description here>
//!
//!
//!  @remarks Assigns XML defined visible scoring weights to the entity
//!
//!  @author GavB @date 06/09/2006
//------------------------------------------------------------------------------------------
void AI::SetVisibleScoreWeight	( CHashedString obXMLObjectName )
{
	// Find the object from the database
	const DataObject* pDataObject = ObjectDatabase::Get().GetDataObjectFromName( obXMLObjectName );

	// Santy check
	ntAssert( strcmp( pDataObject->GetClassName(), "AIFactScoreWeights" ) == 0 );

	// Cast the pointer
	m_pVisibleScoreWeights = (AIFactScoreWeights*) pDataObject->GetBasePtr();
}


//------------------------------------------------------------------------------------------
//!  public constructor  AIFactScoreWeights
//!
//!
//!  @remarks Just a normal constructor setting default values of its members
//!
//!  @author GavB @date 06/09/2006
//------------------------------------------------------------------------------------------
AIFactScoreWeights::AIFactScoreWeights()
{
	m_FactDistanceWeight				= 1.0f;
	m_FactAgeWeight						= 1.0f;
	m_FactEntAttackedWeight				= 1.0f;
	m_InnerRangeWeight					= 0.7f;
	m_CloseRangeWeight					= 0.8f;
	m_LineOfSightRangeWeight			= 0.9f;
	m_MainRangeWeight					= 1.0f;
	m_ShootRangeWeight					= 2.0f;
	m_OtherPersonsProblemRangeWeight	= 3.0f;
}


//!----------------------------------------------------------------------------------------------
//!
//!	AI::SetArmyRenderable
//!	
//!
//!----------------------------------------------------------------------------------------------
void AI::SetArmyRenderable(ArmyRenderable* pRenderable)
{
#if defined( PLATFORM_PS3 )

	ntError( (bool)m_pArmyRenderable ^ (bool) pRenderable );
	
	// Assign the renderable
	m_pArmyRenderable = pRenderable;

	// Handle the zero case
	if( !pRenderable )
		return;

	ntError(GetMovement());

	// Bring over the animations
	ArmyCopyMovementControllerDef MovementDef;
	const CAnimator* pArmyAnimator = pRenderable->GetAnimator();
	int              iAnims        = pArmyAnimator->GetNumAnimations();

	for(int i = 0; i < iAnims; i++)
	{
		CAnimationPtr pAnim = pArmyAnimator->GetAnimation(i);

		if(pAnim && pAnim->IsActive())
		{
			MovementDef.AddAnimation(pAnim);
		}
	}

	GetMovement()->ClearControllers();
	GetMovement()->BringInNewController(MovementDef, CMovement::DMM_STANDARD, 0.f);
#endif
}

//------------------------------------------------------------------------------------------
//!  public virtual  CanRemoveFromWorld
//!
//!  @return bool <TODO: insert return value description here>
//!
//!  @remarks <TODO: insert remarks here>
//!
//!  @author GavB @date 19/09/2006
//------------------------------------------------------------------------------------------
bool AI::CanRemoveFromWorld(void)
{
	return Character::CanRemoveFromWorld();
}


//------------------------------------------------------------------------------------------
//!  public virtual  RemoveFromWorld
//!
//!  @return bool <TODO: insert return value description here>
//!
//!  @remarks <TODO: insert remarks here>
//!
//!  @author GavB @date 19/09/2006
//------------------------------------------------------------------------------------------
bool AI::RemoveFromWorld(bool bSafeRemove)
{
	//Debug::AlwaysOutputString("RemoveFromWorld\n");

	// If told not to destroy/deactivate the entity now
	if( bSafeRemove && !IsSpawned() )
		return Character::RemoveFromWorld( bSafeRemove );

	/// If the entity is in the middle of formation combat - then remove the entity
	/// any current attacks and then remove it from the formation.
	const CAIComponent* pAIComp = GetAIComponent();
	
	// Make sure the LUA FSM is all cleared up - meaning all the behaviours are removed. 
	pAIComp->ResetLuaFSM();

	// If in a formation - then remove the entity
	if( pAIComp->GetAIFormationComponent() )
		pAIComp->GetAIFormationComponent()->Remove();

	if( !IsSpawned() )
	{
		return Character::RemoveFromWorld( false );
	}
	else
	{

#if		defined( PLATFORM_PS3 )
//		ntError( GetArmyRenderable() );
		if( GetArmyRenderable() )
		{
			ntError( GetArmyRenderable()->GetRealDudeEntity() == this );
			GetArmyRenderable()->TurnFromRealDude( IsDead() );
			SetArmyRenderable( 0 );
		}
#		endif

		GetSpawnPool()->DeSpawn(this);

		return Character::RemoveFromWorld( false );
	}

	
}

