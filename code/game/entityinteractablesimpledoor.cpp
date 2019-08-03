//--------------------------------------------------
//!
//!	\file game/entityinteractablerigidbody.cpp
//!	Definition of the Interactable Rigid Body entity object
//!
//--------------------------------------------------


#include "objectdatabase/dataobject.h"
#include "game/luaattrtable.h"
#include "core/timer.h"
#include "Physics/system.h"
#include "physics/collisionbitfield.h"
#include "messagehandler.h"

#include "movement.h"
#include "simpletransition.h"
#include "anim/animator.h"
#include "core/exportstruct_anim.h"


#include "game/entityinteractablesimpledoor.h"


void ForceLinkFunctionSimpleDoor()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionSimpleDoor() !ATTN!\n");
}


START_CHUNKED_INTERFACE(Interactable_SimpleDoor, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(CEntity)
	COPY_INTERFACE_FROM(CEntity)

	OVERRIDE_DEFAULT(Clump, "Resources\\Objects\\Walkways\\PlaceholderDoor\\PlaceholderDoor.clump")
	OVERRIDE_DEFAULT(DefaultDynamics, "Animated")

	PUBLISH_VAR_AS(m_Description, Description)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_AnimationContainer, "SimpleDoorAnimContainer", AnimationContainer)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_OpenAnimation, "DoorOpen", AnimOpen)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_CloseAnimation, "DoorClose", AnimClose)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bOpenOnStart, "false", OpenOnStart)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bAIAvoid, "false", AIAvoid)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bAutoClose, "false", AutoClose)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fSlamSpeed, 2.f, SlamSpeed)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fHoldTime, 1.f, HoldTime)

	DECLARE_POSTCONSTRUCT_CALLBACK(OnPostConstruct)
END_STD_INTERFACE


//--------------------------------------------------
//!
//! Interactable Simple Door State Machine
//!
//--------------------------------------------------
STATEMACHINE(INTERACTABLE_SIMPLEDOOR_FSM, Interactable_SimpleDoor)
	INTERACTABLE_SIMPLEDOOR_FSM(bool bOpen)
	{
		if(bOpen)
			SET_INITIAL_STATE(OPEN);
		else
			SET_INITIAL_STATE(CLOSED);
	}

//////////////////////////////////////////////////////////
// Hot fix until OnAwake (or similar) is added to CEntity
	BEGIN_EVENTS
		ON_UPDATE
			if(!ME->m_bOnAwakeSetupDone)
			{
				ME->OnAwake();
				ME->m_bOnAwakeSetupDone = true;
			}
		END_EVENT(true)
	END_EVENTS
//////////////////////////////////////////////////////////

	STATE(CLOSED)
		BEGIN_EVENTS

			EVENT(Door_Open)	
				ME->GetMovement()->ClearControllers();
				SET_STATE(OPENING);
			END_EVENT(true)

			EVENT(InUse)	
				ME->GetMovement()->ClearControllers();
				ME->m_pSyncEnt = msg.GetEnt(CHashedString(HASH_STRING_SENDER));
				ntAssert(ME->m_pSyncEnt);

				SET_STATE(SYNCED_OPENING);
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(OPENING)
		BEGIN_EVENTS
			ON_ENTER
				if(!ntStr::IsNull(ME->m_OpenAnimation))
				{
					ME->GetMovement()->Lua_AltStartSimpleMovement(ME->m_OpenAnimation, false, false, false);
					ME->GetMovement()->Lua_AltSetMovementCompleteMessage("msg_animdone", ME);
				}	
			END_EVENT(true)

			EVENT(msg_animdone)
				if(ME->m_bAutoClose)
					SET_STATE(CLOSING);
				else
				{
					// Process any OnOpen events
					ME->GetMessageHandler()->ProcessEvent("OnOpen");

					SET_STATE(OPEN);
				}
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(OPEN)
		BEGIN_EVENTS
			EVENT(Door_Close)
				ME->GetMovement()->ClearControllers();
				SET_STATE(CLOSING);
			END_EVENT(true)

			EVENT(Door_Slam)
				ME->GetMovement()->ClearControllers();
				ME->m_bSlamming = true;
				SET_STATE(CLOSING);
			END_EVENT(true)

			EVENT(InUse)
				ME->GetMovement()->ClearControllers();
				ME->m_pSyncEnt = msg.GetEnt(CHashedString(HASH_STRING_SENDER));
				ntAssert(ME->m_pSyncEnt);

				SET_STATE(SYNCED_CLOSING);
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(CLOSING)
		BEGIN_EVENTS
			ON_ENTER
				if(!ntStr::IsNull(ME->m_CloseAnimation))
				{
					SimpleTransitionDef obDef;
					obDef.m_obAnimationName = ME->m_CloseAnimation;
					obDef.SetDebugNames(ntStr::GetString(obDef.m_obAnimationName), "SimpleTransitionDef");
					obDef.m_bApplyGravity = false;
					obDef.m_bLooping = false;
					obDef.m_fSpeed = ME->m_bSlamming ? ME->GetSlamSpeed() : 1.f;
					ME->m_bSlamming = false;

					//Push the controller onto our movement component.
					ME->GetMovement()->BringInNewController(obDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND);
					ME->GetMovement()->SetCompletionMessage("msg_animdone", ME);
				}	
			END_EVENT(true)
				
			EVENT(msg_animdone)
				// Process any OnClose events
				ME->GetMessageHandler()->ProcessEvent("OnClose");

				SET_STATE(CLOSED);
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(FALLING)
		BEGIN_EVENTS
			ON_ENTER
				ME->m_obCoordinationParams.m_fSpeed = -1.0f;
			END_EVENT(true)
				
			ON_UPDATE
				if(ME->m_obCoordinationParams.m_fNormalisedTime <= 0.0f)
				{
					// Process any OnClose events
					ME->GetMessageHandler()->ProcessEvent("OnClose");	

					ME->GetMovement()->ClearControllers();
					SET_STATE(CLOSED);
				}
			END_EVENT(true)

			EVENT(msg_animdone)
				SET_STATE(CLOSED);
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(SYNCED_OPENING)
		BEGIN_EVENTS
			ON_ENTER
				if(!ntStr::IsNull(ME->m_OpenAnimation))
				{
					ME->m_bMovedOffStart = false;
					ME->m_obCoordinationParams.m_fSpeed = 0.0f;
					ME->m_obCoordinationParams.m_fNormalisedTime = ME->m_fPosition = 0.0f;
					ME->GetMovement()->Lua_StartCoordinatedMovement(ME->m_OpenAnimation, false, false, &ME->m_obCoordinationParams);
				}		
			END_EVENT(true)

			ON_UPDATE
			{
				Interactable_Switch_Trigger* pSwitch = (Interactable_Switch_Trigger*)(ME->m_pSyncEnt);
				ME->m_fPosition = pSwitch->GetStateValFloat();
				ME->m_obCoordinationParams.m_fNormalisedTime = ME->m_fPosition;

				// Are we done yet?
				if(ME->m_fPosition >= 1.0f)
				{
					if(! ME->m_bAutoClose)
					{
						// Process any OnOpen events
						ME->GetMessageHandler()->ProcessEvent("OnOpen");	

						SET_STATE(OPEN);
					}
					else
					{
						ME->m_fHoldingTime += CTimer::Get().GetGameTimeChange();
						if(ME->m_fHoldingTime > ME->GetHoldTime())
						{
							// leave controller on so door can fall
							ME->m_fHoldingTime = 0.f;
							SET_STATE(FALLING);
						}
					}
				}    // Did we fall all the way closed again?
				else if(! ME->m_bMovedOffStart && ME->m_fPosition > 0.0f)
				{
					ME->m_bMovedOffStart = true;
				}
				else if(ME->m_bMovedOffStart && ME->m_fPosition <= 0.0f)
				{
					// Process any OnClose events
					ME->GetMessageHandler()->ProcessEvent("OnClose");					

					ME->GetMovement()->ClearControllers();
					SET_STATE(CLOSED);
				}
			}
			END_EVENT(true)

			EVENT(Door_Close)	
				SET_STATE(FALLING);
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(SYNCED_CLOSING)
		BEGIN_EVENTS
			ON_ENTER
				if(!ntStr::IsNull(ME->m_CloseAnimation))
				{
					ME->m_bMovedOffStart = false;
					
					ME->m_obCoordinationParams.m_fSpeed = 0.0f;
					ME->m_obCoordinationParams.m_fNormalisedTime = ME->m_fPosition = 0.0f;
					ME->GetMovement()->Lua_StartCoordinatedMovement(ME->m_CloseAnimation, false, false, &ME->m_obCoordinationParams);
				}		
			END_EVENT(true)

			ON_UPDATE
			{
				Interactable_Switch_Trigger* pSwitch = (Interactable_Switch_Trigger*)(ME->m_pSyncEnt);
				ME->m_fPosition = pSwitch->GetStateValFloat();
				ME->m_obCoordinationParams.m_fNormalisedTime = ME->m_fPosition;

				// Are we done yet
				if(ME->m_fPosition >= 1.0f)
				{
					// Process any OnClose events
					ME->GetMessageHandler()->ProcessEvent("OnClose");	

					SET_STATE(CLOSED);
				}

				// Did we get all the way open again
				if(! ME->m_bMovedOffStart && ME->m_fPosition > 0.0f)
					ME->m_bMovedOffStart = true;

				if(ME->m_bMovedOffStart && ME->m_fPosition <= 0.0f)
				{
					// Process any OnOpen events
					ME->GetMessageHandler()->ProcessEvent("OnOpen");	
					
					ME->GetMovement()->ClearControllers();
					SET_STATE(OPEN);
				}
			}
			END_EVENT(true)

			EVENT(Door_Open)	
				SET_STATE(OPENING);
			END_EVENT(true)
		END_EVENTS
	END_STATE

END_STATEMACHINE


//--------------------------------------------------
//!
//!	Interactable_SimpleDoor::Interactable_SimpleDoor()
//!	Default constructor
//!
//--------------------------------------------------
Interactable_SimpleDoor::Interactable_SimpleDoor()
:	m_pSyncEnt(0),
	m_fHoldingTime(0.f),
	m_bSlamming(false),
	m_bOnAwakeSetupDone(false),
	m_fHoldTime(1.f),
	m_fSlamSpeed(2.f)
	
{
	// You don't interact directly with doors so maybe they should be static?
	m_eType = EntType_Static;
}


//--------------------------------------------------
//!
//!	Interactable_SimpleDoor::OnPostConstruct()
//!	Post Construct
//!
//--------------------------------------------------
void Interactable_SimpleDoor::OnPostConstruct()
{
	CEntity::OnPostConstruct();
	
	// Create components
	InstallMessageHandler();
	InstallDynamics();
	InstallAudioChannel();
	
#ifdef _DEBUG
	if(!GetPhysicsSystem())
		ntPrintf("%s(%d): ### PHYSICS ERROR - door %s without .ps.xml file for clump %s will not have collision\n", __FILE__, __LINE__, GetName().c_str(), GetClumpString().c_str());	
#endif
	

	// requires a hierarchy
	if(!GetHierarchy())
		InstallHierarchy();
	if(! m_AnimationContainer.IsNull())
	{
		InstallAnimator(m_AnimationContainer);
	}

	user_error_p( GetAnimator() != 0, ("SimpleDoor %s doesn't have an anim container\n", GetName().c_str()) );


	if(!GetMovement()) // Create a movement system if one doesn't already exist
	{
		CMovement* pobMovement = NT_NEW_CHUNK(Mem::MC_ENTITY) CMovement(this, GetAnimator(), GetPhysicsSystem());
		pobMovement->ClearControllers();
		SetMovement(pobMovement);
	}

}

//--------------------------------------------------
//!
//!	Interactable_SimpleDoor::OnLevelStart()
//!	Called for each ent on level startup
//!
//--------------------------------------------------
void Interactable_SimpleDoor::OnLevelStart()
{
	// Create and attach the statemachine. Must be done AFTER anim containers fixed up by area system
	// i.e. after XML serialisation. OR this shouldnt play an animation
	INTERACTABLE_SIMPLEDOOR_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) INTERACTABLE_SIMPLEDOOR_FSM(m_bOpenOnStart);
	ATTACH_FSM(pFSM);
}

//--------------------------------------------------
//!
//!	Interactable_SimpleDoor::~Interactable_SimpleDoor()
//!	Default destructor
//!
//--------------------------------------------------
Interactable_SimpleDoor::~Interactable_SimpleDoor()
{
}


//--------------------------------------------------
//!
//!	Interactable_SimpleDoor::OnAwake()
//!	Further setup that must be done once resources are loaded
//!	Currently a hot fix for door dodgyness, intergrated into the 
//! CEntity OnAwake (or similar) functionality when it is ready.  
//! May require function prototype to change to fit the virtualisation.
//!
//--------------------------------------------------
void Interactable_SimpleDoor::OnAwake(void)
{
	user_error_p( GetAnimator() != 0, ("SimpleDoor %s doesn't have an anim container, yet is being awoken\n", GetName().c_str()) );
	if(m_bOpenOnStart)
	{
		m_fPosition=0.0f;
		if(!ntStr::IsNull(m_OpenAnimation))
		{
			m_obCoordinationParams.m_fNormalisedTime = 1.0f; // To the end of the anim
			GetMovement()->Lua_StartCoordinatedMovement(m_OpenAnimation, false, false, &m_obCoordinationParams);

#ifdef _DEBUG
			CAnimator* pobAnimator = GetAnimator();	
			CAnimationPtr obNewAnim = pobAnimator->CreateAnimation(m_OpenAnimation);

			CPoint obDelta(obNewAnim->GetRootEndTranslation());
			if(obDelta.LengthSquared() < EPSILON)
			{
				ntPrintf("SimpleDoor %s without root delta on open animation. Is this intentional?\n", GetName().c_str());	
			}
#endif
		}
	}
	else
	{
		m_fPosition=0.0f;
		if(!ntStr::IsNull(m_CloseAnimation))
		{
			CAnimator* pobAnimator = GetAnimator();	
			CAnimationPtr obNewAnim = pobAnimator->CreateAnimation(m_CloseAnimation);

			CPoint obDelta(obNewAnim->GetRootEndTranslation());
			
			// NB need to undo the root delta here as doors are placed in closed position
			// This is the assumption we make
			if(obDelta.LengthSquared() > EPSILON)
			{
				obDelta *= -1.0f;
				SetPosition(obDelta * GetMatrix());
			}
#ifdef _DEBUG
			else 
			{
				ntPrintf("SimpleDoor %s without root delta on close animation. Is this intentional?\n", GetName().c_str());	
			}
#endif

			m_obCoordinationParams.m_fNormalisedTime = 1.0f; // To the end of the anim
			GetMovement()->Lua_StartCoordinatedMovement(m_CloseAnimation, false, false, &m_obCoordinationParams);
		}
	}
}

