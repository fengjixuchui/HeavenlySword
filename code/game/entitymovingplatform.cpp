//--------------------------------------------------
//!
//!	\file game/entitymovingplatform.cpp
//!	Definition of the moving platform entity object
//!
//--------------------------------------------------


#include "objectdatabase/dataobject.h"
#include "game/luaattrtable.h"
#include "game/entitymanager.h"
#include "game/chatterboxman.h"
#include "Physics/system.h"
#include "Physics/animatedlg.h"
//#include "physics/collisionbitfield.h"
#include "Physics/singlerigidlg.h"
#include "audio/gameaudiocomponents.h"

#include "game/movement.h"
#include "anim/animator.h"
#include "core/exportstruct_anim.h"
#include "messagehandler.h"

#include "game/entitymovingplatform.h"

// Components needed
#include "game/interactioncomponent.h"

void ForceLinkFunction38()
{
	ntPrintf("!ATTN! Calling ForceLinkFunction38() !ATTN!\n");
}

static void AnimatedToDynamic(CEntity* me)
{
	// Place under physics
	Physics::AnimatedLG* lg = static_cast< Physics::AnimatedLG* > ( me->GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::ANIMATED_LG) );
	if(lg && lg->IsActive())
	{
		lg->MakeDynamicOnUpdate();
	}
};

START_CHUNKED_INTERFACE(Interactable_Moving_Platform, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(Interactable)
	COPY_INTERFACE_FROM(Interactable)

	PUBLISH_VAR_AS(m_Description, Description)
	PUBLISH_VAR_AS(m_AnimationContainer, AnimationContainer)
	PUBLISH_VAR_AS(m_EnterAnimation, PlayerEnterAnimation)
	PUBLISH_VAR_AS(m_ExitAnimation, PlayerExitAnimation)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_MoveToPositionIn, CVector(0.0f, 0.0f, 0.0f, 0.0f), MoveToPositionIn)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_MoveToRotationIn, CQuat(0.0f, 0.0f, 0.0f, 1.0f), MoveToRotationIn)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_MoveToPositionOut, CVector(0.0f, 0.0f, 0.0f, 0.0f), MoveToPositionOut)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_MoveToRotationOut, CQuat(0.0f, 0.0f, 0.0f, 1.0f), MoveToRotationOut)

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )	
END_STD_INTERFACE


// Cable car stuff..

START_CHUNKED_INTERFACE(Interactable_Breaky_CableCar_Panel, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(CEntity)
	COPY_INTERFACE_FROM(CEntity)
	
	PUBLISH_VAR_AS(m_hParentName, ParentName)
	PUBLISH_VAR_AS(m_AnimationContainer, AnimationContainer)
	PUBLISH_VAR_AS(m_BreakAwayAnimationName, BreakAwayAnimationName)

	PUBLISH_VAR_AS(m_PanelMesh1,	PanelState1)
	PUBLISH_VAR_AS(m_PanelMesh2,	PanelState2)
	PUBLISH_VAR_AS(m_PanelMesh3,	PanelState3)
	PUBLISH_VAR_AS(m_PanelMesh4,	PanelState4)
	PUBLISH_VAR_AS(m_PanelMesh4b,	PanelState4b)

	PUBLISH_VAR_AS(m_obSoundBank,		SoundBank)
	PUBLISH_VAR_AS(m_obSoundCueStage1,	SoundCueStage1)
	PUBLISH_VAR_AS(m_obSoundCueStage2,	SoundCueStage2)

	PUBLISH_VAR_AS(m_iInToState2,				HitInToState2)
	PUBLISH_VAR_AS(m_iInToState3,				HitInToState3)
	PUBLISH_VAR_AS(m_iInToState4,				HitInToState4)
	PUBLISH_VAR_AS(m_iPauseHitDetectionCount,	PauseHitDetection)

	PUBLISH_VAR_AS(m_bIsFront,					IsFront)

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )	
END_STD_INTERFACE


START_CHUNKED_INTERFACE(Interactable_Breaky_CableCar, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(Interactable_Moving_Platform)
	COPY_INTERFACE_FROM(Interactable_Moving_Platform)
	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )	

	PUBLISH_VAR_AS(m_iFrontThresholdToPausedState,	FrontThresholdToPausedState)
	PUBLISH_VAR_AS(m_iFrontThresholdOutPausedState,	FrontThresholdOutPausedState)
	PUBLISH_VAR_AS(m_iRearThresholdToPausedState,	RearThresholdToPausedState)
	PUBLISH_VAR_AS(m_iRearThresholdOutPausedState,	RearThresholdOutPausedState)
END_STD_INTERFACE


//--------------------------------------------------
//!
//! Interactable Moving-Platform State Machine
//!
//--------------------------------------------------
STATEMACHINE(MOVING_PLATFORM_FSM, Interactable_Moving_Platform)
	MOVING_PLATFORM_FSM()
	{
		SET_INITIAL_STATE(DEFAULT);
	}

	STATE(DEFAULT)
		BEGIN_EVENTS
		
			ON_ENTER
			{
				ntPrintf("##--##--## Default state for moving platform entered ##--##--##\n");
				ME->m_pOther = 0;
				ME->m_bActive = true;
				ME->GetInteractionComponent()->Lua_SetInteractionPriority(USE);
			}
			END_EVENT(true)

			EVENT(msg_moveforward)
			{
				ntPrintf("##--##--## msg_moveforward recieved ##--##--##\n");				

				//We shouldn't get a forward-anim message if we're already at the end!
				//This would cause the platform to "pop" to the start then play forwards.
				if(ME->m_bAtStart != true)
				{
					//For testing, leave this here, but maybe remove the assert later and just END_EVENT to ignore the message.
					ntPrintf("Warning: Asking the platform to play forward when it's not at the start, would cause pop to beginning and play\n");
					ntAssert(ME->m_bAtStart == true);
					END_EVENT(true);
				}

				if(ME->m_AnimationContainer != "")
				{
					ME->PlayAnim("PlatformMoveUp");
				}

				ME->m_bAtStartNext = false;
			}
			END_EVENT(true)

			EVENT(msg_movereverse)
			{
				ntPrintf("##--##--## msg_movereverse recieved ##--##--##\n");				

				//We shouldn't get a reverse-anim message if we're already at the beginning!
				//This would cause the platform to "pop" to the end then play backwards.
				if(ME->m_bAtStart == true)
				{
					//For testing, leave this here, but maybe remove the assert later and just END_EVENT to ignore the message.
					ntPrintf("Warning: Asking the platform to reverse when it's not at the end, would cause pop to end and reverse\n");
					ntAssert(ME->m_bAtStart != true);
					END_EVENT(true);
				}

				if(ME->m_AnimationContainer != "")
				{
					ME->PlayAnim("PlatformMoveDown");
				}

				ME->m_bAtStartNext = true;
			}
			END_EVENT(true)

			EVENT(msg_action)
			EVENT(msg_running_action)
			{
				//We ignore the action messages if the moving platform is inactive.
				//NOTE: msg_moveforward and msg_movereverse can still be sent to the platform via scripts even when it's inactive.
				if(ME->m_bActive == false)
				{
					ntPrintf("Moving platform is inactive, not using\n");
					END_EVENT(true);
				}

				ntPrintf("msg_action recieved\n");
				//Do nothing if we're currently in-motion.
				if(ME->GetAnimator()->IsPlayingAnimation())
				{
					ntPrintf("Mid-animation, not doing anything\n");
					END_EVENT(true);
				}

				//Ignore the message, the player is trying to exit the platform directly after getting in, but before it has moved.
				if(ME->m_bPlayerWait == true)
				{
					ntPrintf("Player forced to wait until platform has moved\n");
					END_EVENT(true);
				}

				//Handle if we're getting INTO the moving platform, or out of it using 'm_bPlayerInside'.
				ME->m_bPlayerInside = !ME->m_bPlayerInside;
				//Log a message for now so that we can check it's status at any point.
				if(ME->m_bPlayerInside == false)
				{
					ntPrintf("Jumping out\n");
				}
				else
				{
					ntPrintf("Jumping in\n");
				}

				//If the player is getting into the platform, force them to wait until after the platform has moved.
				//(Do this by not accepting any action messages on the platform to allow them to jump out).
				if(ME->m_bPlayerInside)
				{
					ME->m_bPlayerWait = true;
				}
				
				//Retrieve our actor.
				ME->m_pOther = (Character*)msg.GetEnt("Other");
				ntAssert(ME->m_pOther != 0 && ME->m_pOther->IsCharacter());	//It should be the entity that sent this message.
				Character* pUserEntity = ME->m_pOther;
				UNUSED(pUserEntity);
				CMovement* pUserMovement = ME->m_pOther->GetMovement();
				UNUSED(pUserMovement);

				//Play an animation on the player and then return them to normal state etc etc.
				if(ME->m_bPlayerInside == false)
				{
					//Get the user to play the exit animation.
					const char* pcAnimName = ntStr::GetString(ME->m_ExitAnimation);
					CPoint		obPosition(ME->m_MoveToPositionOut);
					CQuat		obRotation(ME->m_MoveToRotationOut);

					ME->m_pOther->GetMovement()->Lua_AltStartSnapToMovement(pcAnimName, ME, obPosition, obRotation);
					ME->m_pOther->GetMovement()->Lua_AltSetMovementCompleteMessage("msg_movementdone", ME);

					//Tell the player to automatically return to default state when their movement is complete.
					ME->m_pOther->SetExitOnMovementDone(true);

					//We don't want to collide with the platform's walls while we're getting out of it.
					//Collision is re-allowed after the player's entering-animation/snap-to-movement/rotation has finished.
					ME->m_pOther->GetInteractionComponent()->ExcludeCollisionWith(ME);
				}
				else
				{
					//Get the user to play the enter animation.
					const char* pcAnimName = ntStr::GetString(ME->m_EnterAnimation);
					CPoint		obPosition(ME->m_MoveToPositionIn);
					CQuat		obRotation(ME->m_MoveToRotationIn);

					ME->m_pOther->GetMovement()->Lua_AltStartSnapToMovement(pcAnimName, ME, obPosition, obRotation);
					ME->m_pOther->GetMovement()->Lua_AltSetMovementCompleteMessage("msg_movementdone", ME);
					
					//Tell the player to automatically return to default state when their movement is complete.
					ME->m_pOther->SetExitOnMovementDone(true);

					//We don't want to collide with the platform's walls while we're getting into it.
					//Collision is re-allowed after the player's entering-animation/snap-to-movement/rotation has finished.
					ME->m_pOther->GetInteractionComponent()->ExcludeCollisionWith(ME);
				}

				//Play our own animation depending on whether we're currently at the start or not.
				//Only do this if the player has just gotten INTO the moving platform.
				if(ME->m_bPlayerInside)
				{
					if(ME->m_bAtStart)
					{
						//We're at the start, play forward after player's animation has finished.
						Message MoveForward(msg_moveforward);
						ME->GetMessageHandler()->QueueMessageDelayed(MoveForward, 2.0f); //TODO: Appropriate delay here (length of player's anim or fixed?).
					}
					else
					{
						//We're at the end, play backwards after player's animation has finished.
						Message MoveReverse(msg_movereverse);
						ME->GetMessageHandler()->QueueMessageDelayed(MoveReverse, 2.0f); //TODO: Appropriate delay here (length of player's anim or fixed?).
					}
					//Set the value of m_bAtStartNext for where we're going to be at the end of the animation (start or end position)
					ME->m_bAtStartNext = !ME->m_bAtStart;
				}
			}
			END_EVENT(true)

			//Platform has finished moving.
			EVENT(msg_animdone)
			{
				ntPrintf("Moving platform completed animation (msg_animdone received)\n");
				//Update the flag for where we are.
				ME->m_bAtStart = ME->m_bAtStartNext;
				//We've finished our movement, so the player can jump out if they want.
				ME->m_bPlayerWait = false;

				//Notify m_pobNotifyScriptEntity entity with a set message it's waiting for.
				if(ME->m_bNotifyMovementDone && ME->m_pobNotifyScriptEntity && ME->m_pobNotifyScriptEntity->GetMessageHandler())
				{
					Message obMovementAnimDone(msg_platform_movementfinished);
					ME->m_pobNotifyScriptEntity->GetMessageHandler()->QueueMessage(obMovementAnimDone);
				}
			}
			END_EVENT(true)

			//Handle the msg_movementdone messages from the user after their transition in or out of the moving platform.
			EVENT(msg_movementdone)
			{
				//Return the user to their movement controller.
				ntPrintf("'msg_exitstate' sent to user\n");
				Message msgExitState(msg_exitstate);
				msgExitState.SetEnt( CHashedString(HASH_STRING_SENDER), ME);
				ME->m_pOther->GetMessageHandler()->QueueMessage(msgExitState);

				//We can collide again, hurrah.
				ME->m_pOther->GetInteractionComponent()->AllowCollisionWith(ME);

				//If we're getting out, unattach the user's transform from the moving platform and clear m_pOther.
				if(ME->m_bPlayerInside == false)
				{
					ntPrintf("The user is now out of the platform\n");
					ME->m_pOther = 0;
					//TODO: Unattach our player's transform here too.
				}
				else	//Otherwise, our player has just finished jumping in, so attach them.
				{
					ntPrintf("The user is now in the platform\n");
					//TODO: Attach our player's transform!
				}
			}
			END_EVENT(true)

			EVENT(State_Update)
			{
				//Just so that it doesn't turn up on our default: list.
				//TODO: Put the soft-transform code here?
			}
			END_EVENT(true);

			EVENT(msg_activate)
			{
				ntPrintf("msg_activate received, platform is active\n");
				ME->m_bActive = true;
			}
			END_EVENT(true);

			EVENT(msg_deactivate)
			{
				ntPrintf("msg_deactivate received, platform is inactive\n");
				ME->m_bActive = false;
			}
			END_EVENT(true);

			EVENT(msg_movingplatform_settoend)
			{
				ntPrintf("msg_movingplatform_settoend recieved, setting platform position to the end\n");
				if(ME->m_AnimationContainer != "")
				{
					ME->PlayAnim("PlatformMoveUp", 1.0f);
				}
				ME->m_bAtStart = false;	//Flag this so that we know to play the back anim next.
				ME->m_bAtStartNext = false;	//So that on msg_animdone it's still false.
			}
			END_EVENT(true);

			EVENT(msg_platform_notifyonmovecomplete)
			{
				CEntity* obEntityToNotify = msg.GetEnt("EntityToNotify");
				ME->m_pobNotifyScriptEntity = obEntityToNotify;
				if(ME->m_pobNotifyScriptEntity != NULL)
				{
					ntPrintf("Platform will now notify entity [%s] when all anims are finished\n", ME->m_pobNotifyScriptEntity->GetName().c_str());
					ME->m_bNotifyMovementDone = true;
				}
				else
				{
					ntPrintf("WARNING: Passed in entity to notify was NULL, so no entity will be notified when all anims are complete\n");
				}
			}
			END_EVENT(true);
		END_EVENTS
/*
			default:
				ntPrintf("Message recieved was %d %s\n", msg.GetID(), msg.GetHashedString("Msg").GetDebugString());
				return false;
			}
		}
*/
	END_STATE

END_STATEMACHINE //MOVING_PLATFORM_FSM


//--------------------------------------------------
//!
//!	Interactable_Moving_Platform::Interactable_Moving_Platform()
//!	Default constructor
//!
//--------------------------------------------------
Interactable_Moving_Platform::Interactable_Moving_Platform()
{
	m_eType = EntType_Interactable;
	m_eInteractableType = EntTypeInteractable_Moving_Platform;

	m_pSharedAttributes = 0;

	m_bAtStart = true;
	m_bAtStartNext = true;
	m_bPlayerInside = false;
	m_bPlayerWait = false;
	m_bActive = true;

	m_pOther = 0;

	m_bNotifyMovementDone = false;
	m_pobNotifyScriptEntity = 0;
}

//--------------------------------------------------
//!
//!	Interactable_Moving_Platform::OnPostConstruct()
//!	Post Construct
//!
//--------------------------------------------------
void Interactable_Moving_Platform::OnPostConstruct()
{
	Interactable::OnPostConstruct();

	//Add Components.
	InstallMessageHandler();
	InstallAnimator(m_AnimationContainer);

	// Create rigid body physics
	Lua_CreatePhysicsSystem();
	//Physics::LogicGroup* lg = (Physics::LogicGroup*)Physics::ClumpTools::ConstructRigidLGFromClump( this, 0);	
	Physics::SingleRigidLG* lg = NT_NEW Physics::SingleRigidLG( GetName(),  this);
	lg->Load();

	if(lg)	
	{
		// characters can be soft parented to platforms, so they must be perfectly synchronized with anims
		lg->SetHardKeyframing(true);

		// Add the group
		m_pobPhysicsSystem->AddGroup( (Physics::LogicGroup *) lg );
		lg->Activate();	
	}
	else
	{
		ntPrintf("%s(%d): ### PHYSICS ERROR - Logic group not created for entity %s with clump %s\n", __FILE__, __LINE__, GetName().c_str(), GetClumpString().c_str());		
	}

	m_pobPhysicsSystem->Lua_Rigid_SetKeyframedMotion(true);

	// Create and attach the statemachine
	MOVING_PLATFORM_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) MOVING_PLATFORM_FSM();
	ATTACH_FSM(pFSM);
}

//--------------------------------------------------
//!
//!	Interactable_Moving_Platform::~Interactable_Moving_Platform()
//!	Default destructor
//!
//--------------------------------------------------
Interactable_Moving_Platform::~Interactable_Moving_Platform()
{
	NT_DELETE(m_pSharedAttributes);
}


//--------------------------------------------------
//!
//!	void Interactable_Moving_Platform::PlayAnim(const char *anim)
//!	Play requested animation.
//!
//--------------------------------------------------
void Interactable_Moving_Platform::PlayAnim(CHashedString anim, float fPercentage)
{
	//Check we've been given an animation to play.
	if(ntStr::IsNull(anim))
	{
		return;
	}

	//Play the anim (torn from Anim_Play, including the rest of this comment block).
	CAnimationPtr obNewAnim = GetAnimator()->CreateAnimation(anim);
	if(obNewAnim != 0)
	{
		obNewAnim->SetSpeed(1.0f);

		int iFlags = 0;
/*
		//TODO: Decide if each of these flags needs to be set from the xml interface or not.
		//If not, can remove these constant conditionals.
		if(true)
		{
//			iFlags |= ANIMF_LOCOMOTING;
		}

		if(false)	//Make looping an option? or just remove this block.
		{
			iFlags |= ANIMF_LOOPING;
		}
*/
		obNewAnim->SetFlagBits(iFlags);
		GetAnimator()->AddAnimation(obNewAnim);
		if(fPercentage != 0.0f)
		{
			obNewAnim->SetPercentage(fPercentage);
		}
		GetAnimator()->GetAnimEventHandler().GenerateAnimDoneMessage(anim);
	}
	else
	{
		ntPrintf("Warning: Interactable_Moving_Platform::PlayAnim() - Entity %s has no anim called %s\n", ntStr::GetString(GetName()), ntStr::GetString(anim));
	}
}

//--------------------------------------------------
//!
//!	Interactable_Breaky_CableCar::Interactable_Breaky_CableCar()
//!	Default constructor
//!
//--------------------------------------------------
Interactable_Breaky_CableCar::Interactable_Breaky_CableCar()
{
	m_iFrontPanelHitCounter = 0;
	m_iRearPanelHitCounter = 0;
	m_iFrontThresholdToPausedState = -1;
	m_iFrontThresholdOutPausedState = -1;
	m_iRearThresholdToPausedState = -1;
	m_iRearThresholdOutPausedState = -1;
}

//--------------------------------------------------
//!
//!	Interactable_Breaky_CableCar::~Interactable_Breaky_CableCar()
//!	Default destructor
//!
//--------------------------------------------------
Interactable_Breaky_CableCar::~Interactable_Breaky_CableCar()
{
}

//--------------------------------------------------
//!
//!	Interactable_Breaky_CableCar::OnPostConstruct()
//!	Post Construct
//!
//--------------------------------------------------
void Interactable_Breaky_CableCar::OnPostConstruct()
{
	// Make sure the parent post construct is called... I always forget to add this. 
	Interactable_Moving_Platform::OnPostConstruct();
}


//--------------------------------------------------
//!
//!	Interactable_Breaky_CableCar::ChildPanelHit()
//!	Called whenever the child panels are hit. This
//! allows the cable car to control (to a small deg)
//! the flow of the gameplay
//!
//--------------------------------------------------
void Interactable_Breaky_CableCar::ChildPanelHit(bool bFront)
{
	int& riPanelHitCnt	= bFront ? m_iFrontPanelHitCounter : m_iRearPanelHitCounter;
	int iLowerValue		= bFront ? m_iFrontThresholdToPausedState : m_iRearThresholdToPausedState;
	int iUpperValue		= bFront ? m_iFrontThresholdOutPausedState: m_iRearThresholdOutPausedState;

	// Increase the number of hits for the panel section
	++riPanelHitCnt;

	// Sound some type of awareness?
	if( riPanelHitCnt == iUpperValue )
	{
		CChatterBoxMan::Get().Trigger("AI_BreakCover", this );
	}

	// Have we hit a threshold?
	if( iLowerValue == riPanelHitCnt || iUpperValue == riPanelHitCnt )
	{
		bool bInToPausaed = iLowerValue == riPanelHitCnt;

		// Itearate over each of the child
		for( ntstd::List<CEntity*>::const_iterator obIt = GetChildEntities().begin();
				obIt != GetChildEntities().end(); 
					++obIt )
		{
			const CEntity* pEnt = (*obIt);

			// Check that the type is safe. 
			if( pEnt->GetEntType() == CEntity::EntType_Interactable )
			{
				// Cast the entity type.
				Interactable_Breaky_CableCar_Panel* pBreaky = (Interactable_Breaky_CableCar_Panel*) pEnt;
				
				// Only break the matching position.
				if( pBreaky->m_bIsFront == bFront )
				{
					pBreaky->PauseHitDetection(bInToPausaed);
				}
			}
		}
	}
}

//--------------------------------------------------
//!
//! Animated Cablecar panel State Machine
//!
//--------------------------------------------------
STATEMACHINE(CABLECAR_PANEL_FSM, Interactable_Breaky_CableCar_Panel)
	CABLECAR_PANEL_FSM()
	{
		SET_INITIAL_STATE(DEFAULT);
	}

	STATE(DEFAULT)
		BEGIN_EVENTS
			ON_ENTER
			{
				Message msg( Activate );
				ME->GetMessageHandler()->Receive( msg );
			}
			END_EVENT(true)

			EVENT(Activate)
			{
				CEntity* pParent = CEntityManager::Get().FindEntity( ME->m_hParentName );
				ME->Lua_Reparent( pParent, "ROOT" );
				ME->GetRenderableComponent()->DisableAllRenderables();
				ME->GetRenderableComponent()->EnableAllByMeshName( ME->m_PanelMesh1, true );

				// Cache a local pointer for the cable car, just to make my life a little easier
				ME->m_pParentCableCar = (Interactable_Breaky_CableCar*) pParent;

				// sanity check the entity type
				ntError( ME->m_pParentCableCar->GetEntType() == CEntity::EntType_Interactable );
			}
			END_EVENT(true)
				
			EVENT(msg_projcol)
			
				ME->SignalHit();

				// Only process the hit if the breaky section isn't paused.
				if( !ME->m_bPauseHitDetection )
				{
					// Increase the hit count
					++ME->m_iHitCount;

					// Ready to enter the next state?
					if( ME->m_iHitCount >= ME->m_iInToState2 )
					{
						SET_STATE(BREAKY_1);
					}

					// Pause the hit detection
					if( ME->m_iHitCount == ME->m_iPauseHitDetectionCount && !ME->m_bNoToLocalPauseState )
					{
						ME->m_bPauseHitDetection = true;
					}
				}
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(BREAKY_1)
		BEGIN_EVENTS
			ON_ENTER
			{
				ME->GetEntityAudioChannel()->Play(CHANNEL_ACTION, ME->m_obSoundBank, ME->m_obSoundCueStage1);
				ME->GetRenderableComponent()->EnableAllByMeshName( ME->m_PanelMesh1, false );
				ME->GetRenderableComponent()->EnableAllByMeshName( ME->m_PanelMesh2, true );
			}
			END_EVENT(true)

			EVENT(msg_projcol)

				ME->SignalHit();

				// Only process the hit if the breaky section isn't paused.
				if( !ME->m_bPauseHitDetection )
				{
					// Increase the hit count
					++ME->m_iHitCount;

					// Ready to enter the next state?
					if( ME->m_iHitCount >= ME->m_iInToState3 )
					{
						SET_STATE(BREAKY_2);
					}

					// Pause the hit detection
					if( ME->m_iHitCount == ME->m_iPauseHitDetectionCount && !ME->m_bNoToLocalPauseState )
					{
						ME->m_bPauseHitDetection = true;
					}
				}
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(BREAKY_2)
		BEGIN_EVENTS
			ON_ENTER
			{
				ME->GetEntityAudioChannel()->Play(CHANNEL_ACTION, ME->m_obSoundBank, ME->m_obSoundCueStage2);
				ME->GetRenderableComponent()->EnableAllByMeshName( ME->m_PanelMesh2, false );
				ME->GetRenderableComponent()->EnableAllByMeshName( ME->m_PanelMesh3, true );
			}
			END_EVENT(true)

			EVENT(msg_projcol)

				ME->SignalHit();

				// Only process the hit if the breaky section isn't paused.
				if( !ME->m_bPauseHitDetection )
				{
					// Increase the hit count
					++ME->m_iHitCount;

					// Ready to enter the next state?
					if( ME->m_iHitCount >= ME->m_iInToState4 )
					{
						SET_STATE(BREAK_AWAY);
					}

					// Pause the hit detection
					if( ME->m_iHitCount == ME->m_iPauseHitDetectionCount && !ME->m_bNoToLocalPauseState )
					{
						ME->m_bPauseHitDetection = true;
					}
				}
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(BREAK_AWAY)
		BEGIN_EVENTS
			ON_ENTER
			{
				ME->GetPhysicsSystem()->Lua_RemoveChildEntities();
				ME->GetRenderableComponent()->EnableAllByMeshName( ME->m_PanelMesh3, false );
				ME->GetRenderableComponent()->EnableAllByMeshName( ME->m_PanelMesh4, true );
				ME->GetRenderableComponent()->EnableAllByMeshName( ME->m_PanelMesh4b, true );

				//Play the anim (torn from Anim_Play, including the rest of this comment block).
				CAnimationPtr obNewAnim = ME->GetAnimator()->CreateAnimation( ME->m_BreakAwayAnimationName );
				if(obNewAnim)
				{
					obNewAnim->SetSpeed( 1.0f );
					obNewAnim->SetFlagBits( ANIMF_LOCOMOTING );
					obNewAnim->SetPercentage( 0.0f );
					ME->GetAnimator()->AddAnimation( obNewAnim );
					ME->GetAnimator()->GetAnimEventHandler().GenerateAnimDoneMessageAndCallback(ME->m_BreakAwayAnimationName, 0, AnimatedToDynamic);
				}
			}
			END_EVENT(true)

			EVENT( msg_animdone )
			{
				const CEntity* pParentEnt = ME->GetParentEntity();

				// Reparent to world
				ME->Lua_ReparentToWorld();

				// Place under physics
				Physics::AnimatedLG* lg = (Physics::AnimatedLG*)ME->GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::ANIMATED_LG);
				if(lg && lg->IsActive())
				{
					CPoint obParentRootPos = pParentEnt->GetPosition();
					CDirection obInitialVelocity( ME->GetPosition() - obParentRootPos );
					obInitialVelocity.Normalise();
					obInitialVelocity *= 2.0f;					
					ME->GetPhysicsSystem()->Lua_AltSetLinearVelocity(obInitialVelocity);
				}
			}

		END_EVENTS
	END_STATE
END_STATEMACHINE //CABLECAR_PANEL_FSM


//--------------------------------------------------
//!
//!	Interactable_Breaky_CableCar_Panel::OnPostConstruct
//!
//--------------------------------------------------
Interactable_Breaky_CableCar_Panel::Interactable_Breaky_CableCar_Panel(void)
{
	m_eType = CEntity::EntType_Interactable;

	m_iInToState2 = 1; 
	m_iInToState3 = 2;
	m_iInToState4 = 3;

	// DonBy default - don't pause hit detection.
	m_bPauseHitDetection = false;

	// Allow the local paused state 
	m_bNoToLocalPauseState = false;

	// When the hit count reaches this value, then the hit detection is paused. 
	m_iPauseHitDetectionCount = -1;

	// Clear out the hit counter
	m_iHitCount = 0;

	// Is the this the front or the rearend
	m_bIsFront = false;
}

//--------------------------------------------------
//!
//!	Interactable_Breaky_CableCar_Panel::OnPostConstruct
//!
//--------------------------------------------------
void Interactable_Breaky_CableCar_Panel::OnPostConstruct()
{
	CEntity::OnPostConstruct();

	// Install components
	InstallMessageHandler();
	InstallDynamics();
	InstallAnimator(m_AnimationContainer);
	InstallAudioChannel();

	Physics::AnimatedLG* lg = (Physics::AnimatedLG*)GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::ANIMATED_LG);
	
	if(lg)
	{
		//lg->Activate(true);
	}

	// Create and attach the statemachine
	CABLECAR_PANEL_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) CABLECAR_PANEL_FSM();
	ATTACH_FSM(pFSM);
}

//--------------------------------------------------
//!
//!	Interactable_Breaky_CableCar_Panel::SignalHit
//!
//--------------------------------------------------
void Interactable_Breaky_CableCar_Panel::SignalHit(void)
{
	// Sanity check the valid state of the pointer
	ntAssert( m_pParentCableCar != NULL );

	// Signal the panel hit
	m_pParentCableCar->ChildPanelHit( m_bIsFront );
}
