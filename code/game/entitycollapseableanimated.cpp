//--------------------------------------------------
//!
//!	\file game/entityCollapsableanimated.cpp
//!	Definition of the animated collapse-able entity object
//!
//--------------------------------------------------


//#define DEBUG_COLLAPSABLE	//Uncomment for ntPrintf debug outputs.

#include "objectdatabase/dataobject.h"
#include "game/luaattrtable.h"
#include "Physics/system.h"
#include "physics/collisionbitfield.h"
#include "game/movement.h"
#include "anim/animator.h"
#include "core/exportstruct_anim.h"
#include "messagehandler.h"
#include "effect/fxhelper.h"
#include "audio/audiohelper.h"
#include "game/renderablecomponent.h"
#include "game/attacks.h"
#include "simpletransition.h"

#include "game/entitycollapseableanimated.h"
#include "Physics/singlerigidlg.h"
#include "Physics/animatedlg.h"

// Components needed
#include "game/interactioncomponent.h"

#define ACTION_1 0
#define ACTION_2 1
#define ACTION_3 2
#define ACTION_4 3
#define ACTION_5 4

void ForceLinkFunctionColapsableAnimated()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionCollapsableAnimated() !ATTN!\n");
}

static void AnimatedToDynamic(CEntity* me)
{
	// Place under physics
	Physics::AnimatedLG* lg = static_cast< Physics::AnimatedLG* > ( me->GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::ANIMATED_LG) );
	if(lg && lg->IsActive())
	{
		Collapsable_Animated * collapsable = static_cast<Collapsable_Animated *>(me);
		lg->MakeDynamicOnUpdate(collapsable->m_obSmashDirection.LengthSquared() > 0.0f ? &(collapsable->m_obSmashDirection): 0);		
	}
};

START_CHUNKED_INTERFACE(Att_Collapsable, Mem::MC_ENTITY)
	PUBLISH_VAR_AS(	m_bVulnerable_SpeedAtk, Vulnerable_SpeedAtk)
	PUBLISH_VAR_AS(	m_bVulnerable_RangeAtk, Vulnerable_RangeAtk)
	PUBLISH_VAR_AS(	m_bVulnerable_PowerAtk, Vulnerable_PowerAtk)
	PUBLISH_VAR_AS(	m_bVulnerable_Lvl1_PowerAtk, Vulnerable_Lvl1_PowerAtk)
	PUBLISH_VAR_AS(	m_bVulnerable_Lvl2_PowerAtk, Vulnerable_Lvl2_PowerAtk)
	PUBLISH_VAR_AS(	m_bVulnerable_Lvl3_PowerAtk, Vulnerable_Lvl3_PowerAtk)
	PUBLISH_VAR_AS(	m_bVulnerable_Bolt, Vulnerable_Bolt)
	PUBLISH_VAR_AS(	m_bVulnerable_Rocket, Vulnerable_Rocket)
	PUBLISH_VAR_AS(	m_bVulnerable_ObjectStrike, Vulnerable_ObjectStrike)
END_STD_INTERFACE

START_STD_INTERFACE(Collapsable_Animated)
	DEFINE_INTERFACE_INHERITANCE(CEntity)
	COPY_INTERFACE_FROM(CEntity)

	OVERRIDE_DEFAULT(DefaultDynamics, "Animated")

	PUBLISH_VAR_AS(m_Description, Description)
	PUBLISH_VAR_AS(m_AnimationContainer, AnimationContainer)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bCollapsable, false, Collapsable)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bAnimMessagesOnly, false, ScriptedOnly)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_iMaxHits, 0, HitCount)
	// Vunerabilities here
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bVulnerable_SpeedAtk, false, Vulnerable_SpeedAtk)
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bVulnerable_RangeAtk, false, Vulnerable_RangeAtk)
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bVulnerable_PowerAtk, false, Vulnerable_PowerAtk)
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bVulnerable_Lvl1_PowerAtk, false, Vulnerable_Lvl1_PowerAtk)
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bVulnerable_Lvl2_PowerAtk, false, Vulnerable_Lvl2_PowerAtk)
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bVulnerable_Lvl3_PowerAtk, false, Vulnerable_Lvl3_PowerAtk)
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bVulnerable_Bolt, false, Vulnerable_Bolt)
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bVulnerable_Rocket, false, Vulnerable_Rocket)
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bVulnerable_ObjectStrike, false, Vulnerable_ObjectStrike)
	PUBLISH_VAR_WITH_DEFAULT_AS( m_obStaticClumpName, NULL, StaticClumpName )

	PUBLISH_PTR_AS( m_pobPresetVulnerability, PresetVulnerability)

	PUBLISH_VAR_AS(m_AnimCollapse, AnimCollapse)
	PUBLISH_VAR_AS(m_AnimAction[0], AnimAction1)
	PUBLISH_VAR_AS(m_AnimAction[1], AnimAction2)
	PUBLISH_VAR_AS(m_AnimAction[2], AnimAction3)
	PUBLISH_VAR_AS(m_AnimAction[3], AnimAction4)
	PUBLISH_VAR_AS(m_AnimAction[4], AnimAction5)
	PUBLISH_VAR_AS(m_SfxCollapse, SfxCollapse)
	PUBLISH_VAR_AS(m_PfxCollapse, PfxCollapse)

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )	
END_STD_INTERFACE


START_STD_INTERFACE(Collapsable_StaticPart)
	DEFINE_INTERFACE_INHERITANCE(CEntity)
	COPY_INTERFACE_FROM(CEntity)

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )
END_STD_INTERFACE

//--------------------------------------------------
//!
//! Animated Collapse-Able-Entity State Machine
//!
//--------------------------------------------------
STATEMACHINE(COLLAPSABLE_ANIMATED_FSM, Collapsable_Animated)
	COLLAPSABLE_ANIMATED_FSM()
	{
		SET_INITIAL_STATE(DEFAULT);
	}

	STATE(DEFAULT)
		BEGIN_EVENTS
			ON_ENTER
			{
				ME->m_iHits = 0;
#ifdef DEBUG_COLLAPSABLE
				ntPrintf("Value of anim-messages only is %d\n", ME->m_bAnimMessagesOnly);
#endif
				if( !ntStr::IsNull(ME->m_AnimCollapse) )
				{
#ifdef DEBUG_COLLAPSABLE 
					ntPrintf("Has anim collapse message\n");
#endif
					ME->m_bHasCollapseAnim = true;
				}

				// Set a collision msg callback
				ME->GetPhysicsSystem()->GetCollisionCallbackHandler()->SetCollisionCallback(5.0f);
			}
			END_EVENT(true)

//Animation-Event-Based-Messages (for scripted level sequences etc).

			EVENT(msg_action)
			{
				ME->Action( ME->m_iAction++ );
			}
			END_EVENT(true)

			EVENT(msg_action1)
			{
				ME->Action( ACTION_1 );
			}
			END_EVENT(true)

			EVENT(msg_action2)
			{
				ME->Action( ACTION_2 );
			}
			END_EVENT(true)

			EVENT(msg_action3)
			{
				ME->Action( ACTION_3 );
			}
			END_EVENT(true)

			EVENT(msg_action4)
			{
				ME->Action( ACTION_4 );
			}
			END_EVENT(true)

			EVENT(msg_action5)
			{
				ME->Action( ACTION_5 );
			}
			END_EVENT(true)

			EVENT(msg_smash)
			{
#ifdef DEBUG_COLLAPSABLE 
				ntPrintf("**** Msg_Smash recieved ****\n");
#endif
				//Don't play animation here, it's played (if applicable) on entering the destroyed state.
				ME->GetMovement()->ClearControllers();
				if( (ME->GetAnimator()) && (ME->GetAnimator()->IsPlayingAnimation() == false) )
				{
					if (msg.IsFloat("smashDirX") && msg.IsFloat("smashDirY") && msg.IsFloat("smashDirZ"))
					{
						ME->m_obSmashDirection = CDirection( msg.GetFloat("smashDirX"), msg.GetFloat("smashDirY"), msg.GetFloat("smashDirZ") );
					}

					SET_STATE(DESTROYED);
				}
			}
			END_EVENT(true)


//Manual Hit-Based Messages (for standard Collapsables).

			EVENT(msg_sword_strike)
			{
#ifdef DEBUG_COLLAPSABLE 
				ntPrintf("msg_sword_strike recieved\n");
#endif
				CEntity* pobAttacker = msg.GetEnt(CHashedString(HASH_STRING_OTHER));
				// If we do anims are not playing an anim are not anims only, or don't do anims
				if( ME->CanCollapse() )
				{
					//ME->GetMessageHandler()->ProcessEvent("OnHit");

					switch ( pobAttacker->GetAttackComponent()->CurrentStance() )
					{
					case ST_POWER:
						if ( ((ME->m_pobPresetVulnerability) && (ME->m_pobPresetVulnerability->m_bVulnerable_PowerAtk))
								|| ((!ME->m_pobPresetVulnerability) && (ME->m_bVulnerable_PowerAtk)) )
							SET_STATE(DESTROYED);
						break;

					case ST_RANGE:
						if ( ((ME->m_pobPresetVulnerability) && (ME->m_pobPresetVulnerability->m_bVulnerable_RangeAtk))
								|| ((!ME->m_pobPresetVulnerability) && (ME->m_bVulnerable_RangeAtk)) )
							SET_STATE(DESTROYED);
						break;

					case ST_SPEED:
						if ( ((ME->m_pobPresetVulnerability) && (ME->m_pobPresetVulnerability->m_bVulnerable_SpeedAtk))
								|| ((!ME->m_pobPresetVulnerability) && (ME->m_bVulnerable_SpeedAtk)) )
							SET_STATE(DESTROYED);
						break;

					case ST_COUNT:
						break;

					} // switch ( pobAttacker->GetAttackComponent()->CurrentStance() )
						
				}
			}
			END_EVENT(true);

			EVENT(msg_collision)
			{
				if( ME->CanCollapse() )
				{
					if ( ((ME->m_pobPresetVulnerability) && (ME->m_pobPresetVulnerability->m_bVulnerable_ObjectStrike))
							|| ((!ME->m_pobPresetVulnerability) && (ME->m_bVulnerable_ObjectStrike)) )					
						SET_STATE(DESTROYED);
				}
			}
			END_EVENT(true);

			EVENT(msg_blast_damage)
			{
				if( ME->CanCollapse() )
				{
					if ( ((ME->m_pobPresetVulnerability) && (ME->m_pobPresetVulnerability->m_bVulnerable_Rocket))
							|| ((!ME->m_pobPresetVulnerability) && (ME->m_bVulnerable_Rocket)) )					
						SET_STATE(DESTROYED);
				}
			}
			END_EVENT(true);

			EVENT(msg_object_strike)
			EVENT(msg_projcol)
			{
				if( ME->CanCollapse() )
				{
					if ( ((ME->m_pobPresetVulnerability) && (ME->m_pobPresetVulnerability->m_bVulnerable_Bolt))
							|| ((!ME->m_pobPresetVulnerability) && (ME->m_bVulnerable_Bolt)) )					
						SET_STATE(DESTROYED);
				}
			}
			END_EVENT(true);
			

#ifdef DEBUG_COLLAPSABLE 
			EVENT(State_Update)
			{
				//Just so that it doesn't turn up on our default: list.
			}
			END_EVENT(true);
//		END_EVENTS

//Replace END_EVENTS above with this to get info about messages recieved but not handled.
			default:
				ntPrintf("Message recieved was %d %s\n", msg.GetID(), msg.GetHashedString("Msg").GetDebugString());
				return false;
			}
		}
#else
		END_EVENTS
#endif
	END_STATE

	//Destroyed state.
	STATE(DESTROYED)
		BEGIN_EVENTS
			ON_ENTER
			{
				//We can't animate the physics in KEYFRAMED_STOPPED, so make sure we're just in normal KEYFRAMED motion-type.
				Physics::AnimatedLG* lg = (Physics::AnimatedLG*)ME->GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::ANIMATED_LG);		
				if (lg/* && lg->GetMotionType() == Physics::HS_MOTION_KEYFRAMED_STOPPED*/)
				{
					lg->SetMotionType(Physics::HS_MOTION_KEYFRAMED);
				}
				
				ME->Show();
				if(ME->m_pobStaticPart)
				{
					ME->m_pobStaticPart->Hide();
				}

				//If we have a collapse animation, play that then let msg_animdone event handle destroying the object.
				if(ME->m_bHasCollapseAnim)
				{			
					SimpleTransitionDef obDef;
					obDef.m_obAnimationName = ME->m_AnimCollapse;
					obDef.SetDebugNames(ntStr::GetString(obDef.m_obAnimationName), "SimpleTransitionDef");
					obDef.m_bApplyGravity = false;
					obDef.m_bLooping = false;
					obDef.m_fSpeed = 1.0f;					

					//Push the controller onto our movement component.
					ME->GetMovement()->BringInNewController(obDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND);				
					ME->GetMovement()->SetCompletionCallback(AnimatedToDynamic, ME);
					ME->GetMovement()->SetCompletionMessage("msg_animdone", ME);
				}
				//Otherwise, just destroy it immediately.
				else
				{
					ME->Destroy();
					ME->GetMessageHandler()->ProcessEvent("OnDestroy");
					ME->GetMessageHandler()->ProcessEvent("OnCollapse");
				}
			}
			END_EVENT(true)

			//Process msg_animdone message.
			EVENT(msg_animdone)
			{
#ifdef DEBUG_COLLAPSABLE 
				ntPrintf("msg_animdone recieved\n");
#endif
				ME->Destroy();
				ME->GetMessageHandler()->ProcessEvent("OnDestroy");
				ME->GetMessageHandler()->ProcessEvent("OnCollapse");
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE

END_STATEMACHINE //COLLAPSABLE_ANIMATED_FSM


//--------------------------------------------------
//!
//!	Collapsable_Animated::Collapsable_Animated()
//!	Default constructor
//!
//--------------------------------------------------
Collapsable_Animated::Collapsable_Animated()
:	m_iHits ( 0 )
,	m_iAction ( 0 )
,	m_bAnimMessagesOnly ( false )	//By default we can collapse these objects with attacks.
,	m_bHasCollapseAnim ( false )	//By default there is no animation to play before collapsing.
,	m_bHasStaticPart ( false )
,	m_pobPresetVulnerability ( 0 )
,	m_pobStaticPart ( 0 )
,	m_obSmashDirection ( CDirection( CONSTRUCT_CLEAR ) )
{
	m_eType = EntType_Object;
}

//--------------------------------------------------
//!
//!	Collapsable_Animated::OnPostConstruct()
//!	Post Construct
//!
//--------------------------------------------------
void Collapsable_Animated::OnPostConstruct()
{
	CEntity::OnPostConstruct();

	//Add Components.
	InstallMessageHandler();
	if ( !ntStr::IsNull( m_AnimationContainer ) )
		InstallAnimator(m_AnimationContainer);
	InstallDynamics();

	Physics::AnimatedLG* lg = (Physics::AnimatedLG*)GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::ANIMATED_LG);
	if(lg)
	{
		lg->Activate(true);
		lg->SetMotionType(Physics::HS_MOTION_KEYFRAMED_STOPPED);
	}

	//Create our static clump IF we want one.
	if(!ntStr::IsNull(m_obStaticClumpName))
	{

		// Name for the description of the item
		static const char* s_pcDescription = "Collapsable_StaticPart";

		// Temperary name for the catapult section
		char acName[128];

		// Generate a name for the section
		sprintf(acName, "%s_StaticPart", GetName().c_str());

		DataObject* pDO = ObjectDatabase::Get().ConstructObject("Collapsable_StaticPart", acName, GameGUID(), 0, true, false);

		//Obtain the pointer to the object.
		Collapsable_StaticPart* pStaticPart = (Collapsable_StaticPart*)pDO->GetBasePtr();
		ntError_p(pStaticPart != NULL, ("Error: Could not allocate static-part"));

		if(pStaticPart)
		{
			//Give it an attribute table and get a pointer to it.
			pStaticPart->SetAttributeTable(LuaAttributeTable::Create());
			pStaticPart->GetAttributeTable()->SetDataObject(pDO);
			LuaAttributeTable* pAttributes = pStaticPart->GetAttributeTable();

			//Set the basics for the entity.
			pAttributes->SetAttribute("Name", acName);
			pAttributes->SetAttribute("Clump", ntStr::GetString(m_obStaticClumpName));
			pAttributes->SetAttribute("Description", s_pcDescription);
			pAttributes->SetAttribute("DefaultDynamics", "Animated");
			pAttributes->SetInteger("SectorBits", GetMappedAreaInfo());
			CPoint obPosition = GetPosition();
			CQuat obRotation = GetRotation();
			char acPosition[64] = {0};
			sprintf(acPosition, "%f,%f,%f", obPosition.X(), obPosition.Y(), obPosition.Z());
			pAttributes->SetAttribute("Position", acPosition);
			char acRotation[64] = {0};
			sprintf(acRotation, "%f,%f,%f,%f", obRotation.X(), obRotation.Y(), obRotation.Z(), obRotation.W());
			pAttributes->SetAttribute("Orientation", acRotation);

			//The static part needs to know that this object is it's parent so that it can pass back a "turn-active" message.
			pStaticPart->SetParent(this);

			//Perform the post-load construction on the entity.
			ObjectDatabase::Get().DoPostLoadDefaults(pDO);

			//Make sure the static part doesn't attempt to collide with the destructable part.
			if(pStaticPart->GetInteractionComponent())
			{
				pStaticPart->GetInteractionComponent()->ExcludeCollisionWith(this);
			}

			//Store this static part.
			m_pobStaticPart = pStaticPart;
			m_bHasStaticPart = true;

			//We've successfully made a static part, so we can hide and deactivate ourself for now, until the static part
			//lets us know it's time to become active again.
			Hide();
			//We want to show the static-part though (just been hidden as it was a child node during Hide() call above).
			pStaticPart->Show();
		}
	}

	if(!GetMovement()) // Create a movement system if one doesn't already exist
	{
		CMovement* pobMovement = NT_NEW_CHUNK(Mem::MC_ENTITY) CMovement(this, GetAnimator(), GetPhysicsSystem());
		pobMovement->ClearControllers();
		SetMovement(pobMovement);
	}

	// Create and attach the statemachine
	COLLAPSABLE_ANIMATED_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) COLLAPSABLE_ANIMATED_FSM();
	ATTACH_FSM(pFSM);
}

//--------------------------------------------------
//!
//!	Collapsable_Animated::~Collapsable_Animated()
//!	Default destructor
//!
//--------------------------------------------------
Collapsable_Animated::~Collapsable_Animated()
{
}

//--------------------------------------------------
//!
//!	void Collapsable_Animated::Destroy()
//!	Handles "breaking" the object and letting physics take over.
//!
//--------------------------------------------------
void Collapsable_Animated::Destroy()
{
	//Make non-interactive.
	GetInteractionComponent()->Lua_SetInteractionPriority(NONE);
	GetPhysicsSystem()->Lua_RemoveChildEntities();

	if(m_bCollapsable)	//We want the hierarchy to turn into rigid bodies.
	{
		Physics::AnimatedLG* lg = (Physics::AnimatedLG*)GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::ANIMATED_LG);
		
		if(lg && lg->IsActive() && !lg->IsDynamic())
		{
			if (m_obSmashDirection.LengthSquared() > 0.0f)
			{
				lg->MakeDynamic(&m_obSmashDirection);
			}
			else
			{
				lg->MakeDynamic();
			}
		}
	}

	if(!ntStr::IsNull( m_PfxCollapse ) )
	{
		FXHelper::Pfx_CreateStatic(m_PfxCollapse, this, "ROOT");	//Spawn destruction particle effect.
	}

	if(!ntStr::IsNull( m_SfxCollapse ) )
	{
		//AudioHelper::PlaySound(ntStr::GetString(m_SfxCollapse));
	}
}

//--------------------------------------------------
//!
//!	void Collapsable_Animated::Action(int iAction)
//!	Handles action on the object.
//!
//--------------------------------------------------
void Collapsable_Animated::Action(int iAction)
{
	ntAssert ( iAction < MAX_ACTIONS );

	// if object is in keyframed stopped state make it keyframes

	if( !ntStr::IsNull( m_AnimAction[iAction] ) )
	{
#ifdef DEBUG_COLLAPSABLE 
		ntPrintf("msg_action%i received, Playing AnimAction1 animation [%s]\n", iAction+1, ntStr::GetString( m_AnimAction[iAction] ));
#endif

		SimpleTransitionDef obDef;
		obDef.m_obAnimationName = m_AnimAction[iAction];
		obDef.SetDebugNames(ntStr::GetString(obDef.m_obAnimationName), "SimpleTransitionDef");
		obDef.m_bApplyGravity = false;
		obDef.m_bLooping = false;
		obDef.m_fSpeed = 1.0f;					

		//Push the controller onto our movement component.
		GetMovement()->BringInNewController(obDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND);				
	}
#ifdef DEBUG_COLLAPSABLE 
	else
	{
		ntPrintf("WARNING: msg_action%i recieved, but collapse-able entity has no AnimAction%i animation\n", iAction+1, iAction+1);
	}
#endif
}

//--------------------------------------------------
//!
//!	bool Collapsable_Animated::CanCollapse( void )
//!	Collect our can collapse checks in one place
//!
//--------------------------------------------------
bool Collapsable_Animated::CanCollapse( void )
{
	// If we do anims are not playing an anim are not anims only
	GetMovement()->ClearControllers();
	if( (GetAnimator()) && (GetAnimator()->IsPlayingAnimation() == false) && (!m_bAnimMessagesOnly))
		return true;
	// Or if we don't do anims
	else if (! GetAnimator() )
		return true;
	else
		return false;
}


//--------------------------------------------------
//!
//!	Collapsable_StaticPart::Collapsable_StaticPart()
//!	Default constructor
//!
//--------------------------------------------------
Collapsable_StaticPart::Collapsable_StaticPart()
{
	m_eType = EntType_Object;

	m_pParentCollapsable = 0;
}

//--------------------------------------------------
//!
//!	Collapsable_Animated::OnPostConstruct()
//!	Post Construct
//!
//--------------------------------------------------
void Collapsable_StaticPart::OnPostConstruct()
{
	CEntity::OnPostConstruct();
}

//--------------------------------------------------
//!
//!	Collapsable_StaticPart::~Collapsable_StaticPart()
//!	Default destructor
//!
//--------------------------------------------------
Collapsable_StaticPart::~Collapsable_StaticPart()
{
}
