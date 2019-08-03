//--------------------------------------------------
//!
//!	\file game/entityinteractablecatapult.cpp
//!	Definition of the catapult entity object
//!
//--------------------------------------------------

#include "objectdatabase/dataobject.h"
#include "messagehandler.h"
#include "movement.h"
#include "anim/animator.h"
#include "game/interactioncomponent.h"
#include "game/simpletransition.h"
#include "game/entityprojectile.h"

// Debug includes 
#include "core/OSDDisplay.h"

#include "game/entitycatapult.h"
#include "game/entitycatapultrock.h"

#include "camera/camutils.h"

#include "Physics/world.h"
#include "Physics/system.h"
#include "Physics/animatedlg.h"

// TGS Hack
/*#include "hud/hudmanager.h"
#include "hud/objectivemanager.h"*/


#define PROX_THRESHOLD (3.0f)
#define STOP_THRESHOLD (1.5f)
#define WARNING_TIME (20.0f)

void ForceLinkFunctionCatapult()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionCatapult() !ATTN!\n");
};

static void AnimatedToDynamic(CEntity* me)
{
	// Reparent to world
	me->Lua_ReparentToWorld();	

	// Place under physics
	Physics::AnimatedLG* lg = static_cast< Physics::AnimatedLG* > ( me->GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::ANIMATED_LG) );
	if(lg && lg->IsActive())
	{
		lg->MakeDynamicOnUpdate();
	}
};

START_CHUNKED_INTERFACE(Att_User, Mem::MC_ENTITY)
	PUBLISH_VAR_AS(	m_obAnimationMoveLoop,	AnimationMoveLoop)
	PUBLISH_VAR_AS(	m_obAnimationIdleLoop,	AnimationIdleLoop)
	PUBLISH_VAR_AS(	m_obAnimationFire,		AnimationFire)
	PUBLISH_VAR_AS(	m_obAnimationReload,	AnimationReload)
	PUBLISH_VAR_AS(	m_obTransform,			TransformName)
	PUBLISH_VAR_AS(	m_obTranslation,		Translation)
END_STD_INTERFACE

START_CHUNKED_INTERFACE(Att_Catapult, Mem::MC_ENTITY)
	PUBLISH_VAR_AS ( m_bAIAvoid,				AIAvoid)					// Sets if the AI will try to avoid this object
	PUBLISH_VAR_AS ( m_fAIAvoidRadius,			AIAvoidRadius)				// Sets the radius that the AI will try to avoid the object by
	PUBLISH_VAR_AS ( m_obMoveAnim,				AnimationMove)
	PUBLISH_VAR_AS ( m_fMaxRotationPerSecond,	MaxHeadingPerSecond)
	PUBLISH_VAR_AS ( m_fMaxNormalPerSecond,		MaxNormalPerSecond)
	PUBLISH_VAR_AS ( m_fMaxAcceleration,		MaxAcceleration)
	PUBLISH_VAR_WITH_DEFAULT_AS ( m_fMaxHeightPerSecond, 0.5f, MaxHeightPerSecond )
	PUBLISH_PTR_CONTAINER_AS ( m_aobUserDefList,				UserList)
	PUBLISH_PTR_CONTAINER_AS ( m_aobAmmoDefList,				AmmoList)
	PUBLISH_CONTAINER_AS ( m_aobVolleyTimeList,				VolleyTimeList)
	PUBLISH_VAR_AS ( m_strBreakySection1,					BreakySectionClump1)
	PUBLISH_VAR_AS ( m_strBreakySection2,					BreakySectionClump2)
	PUBLISH_VAR_AS ( m_strBreakySection3,					BreakySectionClump3)
	PUBLISH_VAR_AS ( m_strBreakySection4,					BreakySectionClump4)
	PUBLISH_VAR_AS ( m_strBreakySection5,					BreakySectionClump5)
	PUBLISH_VAR_AS ( m_strBreakySection6,					BreakySectionClump6)
	PUBLISH_VAR_WITH_DEFAULT_AS( m_iStrikesForBreakable, 3, StrikesForBreakable)
	PUBLISH_PTR_AS(	m_pobAmmoAtts,				AmmoAttributes)	
END_STD_INTERFACE

//--------------------------------------------------
//!
//--------------------------------------------------
START_CHUNKED_INTERFACE(Object_Catapult, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(Interactable)
	COPY_INTERFACE_FROM(Interactable)
	OVERRIDE_DEFAULT(DefaultDynamics, "Animated")

	PUBLISH_VAR_AS(m_Description, Description)
	PUBLISH_VAR_AS(m_AnimationContainer, AnimationContainer)
	PUBLISH_PTR_WITH_DEFAULT_AS( m_pobSharedAttributes, SharedAttributes, Att_Simple_Catapult)
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fSpeed, 1.0f, Speed)

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )
	DECLARE_POSTPOSTCONSTRUCT_CALLBACK(OnPostPostConstruct)
END_STD_INTERFACE


//--------------------------------------------------
//!
//--------------------------------------------------
START_CHUNKED_INTERFACE(Catapult_BreakySection, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(Interactable)
	COPY_INTERFACE_FROM(Interactable)

	OVERRIDE_DEFAULT(DefaultDynamics, "Animated")

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )
	DECLARE_POSTPOSTCONSTRUCT_CALLBACK(OnPostPostConstruct)
END_STD_INTERFACE


//--------------------------------------------------
//!
//! Catapult State Machine
//!
//--------------------------------------------------
STATEMACHINE(INTERACTABLE_CATAPULT_FSM, Object_Catapult)
	INTERACTABLE_CATAPULT_FSM(bool bActive)
	{
		if (bActive)
			SET_INITIAL_STATE(IDLE);
		else 
			SET_INITIAL_STATE(DEFAULT);
	}

	STATE(DEFAULT)
		BEGIN_EVENTS
			ON_ENTER
				ME->m_pOther = 0;
			END_EVENT(true)

			EVENT(msg_running_action)
			EVENT(msg_action)
			{
				Att_Catapult* pCatapultAtts = ME->GetSharedAttributes();
				ntAssert(pCatapultAtts);

				ME->m_pOther = msg.GetEnt("Other")->ToCharacter();
				ntError(!ME->m_pOther || ME->m_pOther->IsCharacter());

				// Player won't activate a catapult
				if ( ME->m_pOther->IsPlayer() )
				{
					// ... but they might trigger an OnAction event
					if ( ME->m_bUsePointActive )
					{
						ME->GetMessageHandler()->ProcessEvent("OnAction");
					}

					Message msgExitState(msg_exitstate);
					msgExitState.SetEnt( CHashedString(HASH_STRING_SENDER), ME);
					ME->m_pOther->GetMessageHandler()->QueueMessage(msgExitState);
					SET_STATE(DEFAULT);

					END_EVENT(true)
				}

				ME->m_aobUserList.push_back( ME->m_pOther );
				ME->m_pOther = 0;

				if ( ME->m_aobUserList.size() >= pCatapultAtts->m_aobUserDefList.size() )
				{
					// Only move on once we have everyone
					SET_STATE(IDLE);	
				}
			}
			END_EVENT(true)

			EVENT(msg_activate)
				// Force to active
				SET_STATE(IDLE);
			END_EVENT(true)


			//If we receive this message then go to destructable state, and forward on the explode-message.
			EVENT(msg_explode_catapult)
			{
				ntPrintf("msg_explode_catapult recieved in non-breakable catapult, GOING BREAKABLE\n");
				//Put ourselves into DESTRUCTABLE state and forward-on the msg_explode_catapult message.
				Message obExplodeMessage(msg_explode_catapult);
				ME->GetMessageHandler()->QueueMessage(obExplodeMessage);
				SET_STATE(DESTRUCTABLE);
			}
			END_EVENT(true)


			// Overide the global messages in inactive state
			IGNORE_EVENT(msg_fire)
			IGNORE_EVENT(msg_reload)
			IGNORE_EVENT(msg_move)
			IGNORE_EVENT(msg_fire_volley)
			IGNORE_EVENT(msg_destroy)


		END_EVENTS
	END_STATE

	STATE(DESTROYED)
		BEGIN_EVENTS
			ON_ENTER
				// Idle setup so it will grind to halt
				ME->m_aobMoveList.clear();
				ME->IdleSetup();

				ME->GetMessageHandler()->ProcessEvent( "OnDestroy" );
				ME->GetInteractionComponent()->Lua_SetInteractionPriority( NONE );
			END_EVENT(true)

			EVENT( msg_object_strike )
			{
				if( msg.IsInt(1) )
				{
					ME->SignalBreakyExplosion( msg.GetEnt("Sender"), msg.GetInt(1) );
				}
			}
			END_EVENT(true)

			// Overide the global messages in inactive state
			IGNORE_EVENT(msg_running_action)
			IGNORE_EVENT(msg_action)
			IGNORE_EVENT(msg_fire)
			IGNORE_EVENT(msg_reload)
			IGNORE_EVENT(msg_move)
			IGNORE_EVENT(msg_fire_volley)
			IGNORE_EVENT(msg_destroy)
		END_EVENTS
	END_STATE

// ------------------------------------------------ //
	STATE(IDLE)
		BEGIN_EVENTS
			ON_ENTER
			{
				ME->m_bMoving = false;

				ME->IdleSetup();
			}
			END_EVENT(true)

			EVENT(msg_move)
			{
				if ( msg.IsNumber("Speed") )
					ME->m_fSpeed = msg.GetFloat("Speed");

				CPoint* pobPoint = ME->GetPoint(msg);
				if (pobPoint)
				{
					ME->m_aobMoveList.push_back( pobPoint );
					SET_STATE(MOVE);
				}
			}
			END_EVENT(true)

			//If we receive this message then go to destructable state, and forward on the explode-message.
			EVENT(msg_explode_catapult)
			{
				ntPrintf("msg_explode_catapult recieved in non-breakable catapult, GOING BREAKABLE\n");
				//Put ourselves into DESTRUCTABLE state and forward-on the msg_explode_catapult message.
				Message obExplodeMessage(msg_explode_catapult);
				ME->GetMessageHandler()->QueueMessage(obExplodeMessage);
				SET_STATE(DESTRUCTABLE);
			}
			END_EVENT(true)


		END_EVENTS
	END_STATE

// ------------------------------------------------ //
	STATE(MOVE)
		BEGIN_EVENTS
			ON_ENTER
			{
				ME->m_bMoving = true;
				
				ME->MoveSetup();
			}
			END_EVENT(true)

			ON_UPDATE
			{
				ME->UpdateInput( );

				if ( ME->m_aobMoveList.empty() )
					SET_STATE (IDLE);
			}
			END_EVENT(true)

			EVENT(msg_stop)
				ME->m_aobMoveList.clear();
				SET_STATE (IDLE);
			END_EVENT(true)

			EVENT(msg_move)
			{
				if ( msg.IsNumber("Speed") )
					ME->m_fSpeed = msg.GetFloat("Speed");

				CPoint* pobPoint = ME->GetPoint(msg);
				if (pobPoint)
				{
					ME->m_aobMoveList.push_back( pobPoint );
				}
			}
			END_EVENT(true)

			//If we receive this message then go to destructable state, and forward on the explode-message.
			EVENT(msg_explode_catapult)
			{
				ntPrintf("msg_explode_catapult recieved in non-breakable catapult, GOING BREAKABLE\n");
				//Put ourselves into DESTRUCTABLE state and forward-on the msg_explode_catapult message.
				Message obExplodeMessage(msg_explode_catapult);
				ME->GetMessageHandler()->QueueMessage(obExplodeMessage);
				SET_STATE(DESTRUCTABLE_MOVE);
			}
			END_EVENT(true)


		END_EVENTS
	END_STATE

	// Destructable equivelent of Idle state
	STATE(DESTRUCTABLE)
		BEGIN_EVENTS
			ON_ENTER
			{
				// Setup on first enter
				ME->BreakableSetup();

				ME->IdleSetup();

				ME->m_bMoving = false;
			}
			END_EVENT(true)

			EVENT( msg_object_strike )
			{
				if( msg.IsInt(1) )
				{
					ME->SignalBreakyExplosion( msg.GetEnt("Sender"), msg.GetInt(1) );
				}
			}
			END_EVENT(true)

			EVENT(msg_explode_catapult)
				{
				ntPrintf("msg_explode_catapult recieved while breakable, EXPLODING ALL\n");
				ME->ExplodeWholeCatapult();
				SET_STATE(DESTROYED);	//No more interaction for this catapult.
				}
			END_EVENT(true)

			EVENT(msg_move)
				{
				if ( msg.IsNumber("Speed") )
					ME->m_fSpeed = msg.GetFloat("Speed");

				CPoint* pobPoint = ME->GetPoint(msg);
				if (pobPoint)
				{
					ME->m_aobMoveList.push_back( pobPoint );
					SET_STATE(DESTRUCTABLE_MOVE);
				}
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

	// Destructable equivelent of Move state
	STATE(DESTRUCTABLE_MOVE)
		BEGIN_EVENTS
			ON_ENTER
				{
				// Setup on first enter
				ME->BreakableSetup();

				// If not allready moving (we could come here from the moving state)
				if (!ME->m_bMoving)
				{
					ME->MoveSetup();
				}

				ME->m_bMoving = true;
			}
			END_EVENT(true)

			ON_UPDATE
					{
				ME->UpdateInput( );

				if ( ME->m_aobMoveList.empty() )
					SET_STATE (DESTRUCTABLE);
					}
			END_EVENT(true)

			EVENT(msg_stop)
				ME->m_aobMoveList.clear();
				SET_STATE (DESTRUCTABLE);
			END_EVENT(true)

			EVENT(msg_move)
			{
				if ( msg.IsNumber("Speed") )
					ME->m_fSpeed = msg.GetFloat("Speed");

				CPoint* pobPoint = ME->GetPoint(msg);
				if (pobPoint)
				{
					ME->m_aobMoveList.push_back( pobPoint );
				}
			}
			END_EVENT(true)

			EVENT( msg_object_strike )
			{
				if( msg.IsInt(1) )
				{
					ME->SignalBreakyExplosion( msg.GetEnt("Sender"), msg.GetInt(1) );
				}
			}
			END_EVENT(true)

			EVENT(msg_explode_catapult)
			{
				ntPrintf("msg_explode_catapult recieved while breakable, EXPLODING ALL\n");
				ME->ExplodeWholeCatapult();
				SET_STATE(DESTROYED);	//No more interaction for this catapult.
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE // DESTRUCTABLE_MOVE


// ---------------------------------------------------------------- //
	BEGIN_EVENTS
		EVENT(msg_running_action)
		EVENT(msg_action)
		{
			ME->m_pOther = msg.GetEnt("Other")->ToCharacter();
			ntError(!ME->m_pOther || ME->m_pOther->IsCharacter());

			// Player can fire OnAction by "using" the catapult
			// Could be used to fire NS, assuming we get dynamic parentable NS
			if ( ME->m_pOther->IsPlayer() && ME->m_bUsePointActive )
			{
				// Process any OnAction events
				ME->GetMessageHandler()->ProcessEvent("OnAction");
			}
			
			// Default state will handle AIs that should use the catapult
			// so let everyone know to exit their interacting state
			Message msgExitState(msg_exitstate);
			msgExitState.SetEnt( CHashedString(HASH_STRING_SENDER), ME);
			ME->m_pOther->GetMessageHandler()->QueueMessage(msgExitState);
		}
		END_EVENT(true)

		EVENT( msg_activate_usepoint )
			ME->m_bUsePointActive = true;
			ME->GetInteractionComponent()->Lua_SetInteractionPriority( USE );
		END_EVENT(true)

		EVENT( msg_deactivate_usepoint )
			ME->m_bUsePointActive = false;
			ME->GetInteractionComponent()->Lua_SetInteractionPriority( NONE );
		END_EVENT(true)

		EVENT( msg_fire )
		{
			u_int iArmID=0;

			// See if msg specified an arm
			if (! ME->GetArmID(msg, iArmID) )
			{
				// No Id in message - use default
				iArmID = ME->m_iNextArmFire;
				++ME->m_iNextArmFire;
				if (ME->m_iNextArmFire >= ME->m_aobArmStateList.size() )
					ME->m_iNextArmFire = 0;
			}
			
			// See if msg specified a target
			CPoint* pobPoint = ME->GetPoint(msg);
			if (pobPoint)
			{
				// Push to front so we go for a requested target first
				ME->m_aobTargetList.push_front( pobPoint );
			}

			// Good to go
			ME->FireProjectile(iArmID);
		}
		END_EVENT(true)

		EVENT( msg_fire_volley )
		{
			Att_Catapult* pCatapultAtts = ME->GetSharedAttributes();
			ntAssert(pCatapultAtts);
			
			int i = 0;
			for ( VolleyTimeList::iterator obIt = pCatapultAtts->m_aobVolleyTimeList.begin();
					obIt != pCatapultAtts->m_aobVolleyTimeList.end(); ++obIt )
			{
				Message msgFire(msg_fire);
				msgFire.SetEnt(CHashedString(HASH_STRING_SENDER), ME);
				msgFire.SetInt("ArmID", i);
				ME->GetMessageHandler()->QueueMessageDelayed(msgFire, (*obIt) );
				i++;
			}
		}
		END_EVENT(true)

		EVENT( msg_reset )
		{
			u_int iArmID=0;

			// See if msg specified an arm
			if (! ME->GetArmID(msg, iArmID) )
			{
				// No Id in message - use default
				iArmID = ME->m_iNextArmReset;
				++ME->m_iNextArmReset;
				if (ME->m_iNextArmReset >= ME->m_aobArmStateList.size() )
					ME->m_iNextArmReset = 0;
			}

			// Good to go
			ME->ResetArm(iArmID);
		}
		END_EVENT(true)

		EVENT( msg_reload )
		{
			u_int iArmID=0;

			// See if msg specified an arm
			if ( ME->GetArmID(msg, iArmID) )
			{
				// Good to go
				ME->ReloadProjectile(iArmID);
			}
			// ... Otherwise I'm felling generous and will reload all the arms
			else
			{
				for ( u_int i = 0; i < ME->m_aobArmStateList.size(); i++)
					ME->ReloadProjectile( i );
			}
		}
		END_EVENT(true)

		EVENT( msg_target )
		{
			// See if msg specified a target, we certainly expect it to
			CPoint* pobPoint = ME->GetPoint(msg);
			if (pobPoint)
			{
				ME->m_aobTargetList.push_back( pobPoint );
			}
		}
		END_EVENT(true)

		EVENT(msg_destroy)
			SET_STATE (DESTROYED);
		END_EVENT(true)

		//After being hit a couple of times the catapults turn into explodable ones.
		EVENT(msg_projcol)
		{
			Att_Catapult* pCatapultAtts = ME->GetSharedAttributes();
			ntAssert(pCatapultAtts);
			
			//Fudge to stop cannonballs from ricochet.
			CEntity* pProjectileEnt = msg.IsPtr("Projectile") ? msg.GetEnt("Projectile") : 0;
			CEntity* pAttacker = 0;
			if(pProjectileEnt && pProjectileEnt->IsProjectile())
			{
				int iCurrNumProjCollisions = ME->m_iProjCollisions;
				Object_Projectile* pProjectile = (Object_Projectile*)pProjectileEnt;
				pAttacker = pProjectile->m_ProjectileData.pAttacker;
				if(pProjectile->m_eProjectileType == PROJECTILE_TYPE_BAZOOKA_ROCKET)
				{
					ME->m_iProjCollisions++;
					ntPrintf("Catapult hit by rocket, has a total of %d hits now\n", ME->m_iProjCollisions);
				}
				if(pProjectile->m_eProjectileType == PROJECTILE_TYPE_CANNON_BALL)
				{
					if( pProjectile && pProjectile->GetMessageHandler() )
					{
						Message obCannonBall( msg_ricochet );

						// This will cause the cannon ball to explode
						obCannonBall.AddParam( 0.0f );

						// 
						pProjectile->GetMessageHandler()->Receive( obCannonBall );
					}
					ME->m_iProjCollisions++;
					ntPrintf("Catapult hit by cannon-ball, has a total of %d hits now\n", ME->m_iProjCollisions);
				}

				//If we've had a few projectile collisions from either cannonballs or rockets, then go to breakable.
				if((iCurrNumProjCollisions < pCatapultAtts->m_iStrikesForBreakable) 
						&& (ME->m_iProjCollisions >= pCatapultAtts->m_iStrikesForBreakable))
				{
					ntPrintf("GOING BREAKABLE\n");
					if ( ME->m_bMoving )
						SET_STATE(DESTRUCTABLE_MOVE);
					else
						SET_STATE(DESTRUCTABLE);
				}
			}

			//TGS Hack
			//If this was shot by the player, notify them of anything specific (such as "Catapults are too heavily armoured" etc).
			/*if(pAttacker == CEntityManager::Get().GetPlayer())
			{
				// Check we have an objective manager
				if ( !CHud::Exists() || !CHud::Get().GetCombatHUDElements() || !CHud::Get().GetCombatHUDElements()->m_pobObjectiveManager )
					END_EVENT(true);
				
				// Add the objective
				if ( ME->m_fWarningTime < ( CTimer::Get().GetGameTime() - WARNING_TIME ) )
				{
					CHud::Get().GetCombatHUDElements()->m_pobObjectiveManager->AddObjective("TGS_OBJECTIVE_2");
					ME->m_fWarningTime = (float)CTimer::Get().GetGameTime();
				}
			}*/
		}
		END_EVENT(true)

	END_EVENTS
END_STATEMACHINE


//--------------------------------------------------
//!
//! Animated Catapult_BreakySection State Machine
//!
//--------------------------------------------------
STATEMACHINE(CATAPULT_BREAKY_FSM, Catapult_BreakySection)
	CATAPULT_BREAKY_FSM()
	{
		SET_INITIAL_STATE(DEFAULT);
	}

	STATE(DEFAULT)
		BEGIN_EVENTS
			ON_ENTER
			{
			}
			END_EVENT(true)

			ON_UPDATE
			{
			}
			END_EVENT(true)

			//Do we want to handle splash-damage the same? Probably not or one hit will break all the bits off!
			//Because it sticks around for a while it will then also break off the pieces that have just been activated, so no good.
//			EVENT(msg_blast_damage)
			EVENT(msg_object_strike)
			EVENT(msg_projcol)
			{
				// Pass a message to the parent catapult telling it of the collision
				Message obMsg( msg_object_strike );

				// Set the sender of the message.
				obMsg.SetEnt( "Sender", ME );

				// Add the section ID to the message. 
				ntPrintf("Catapult %s Section %d sending msg_object_strike message\n", ME->GetName().c_str(), ME->BreakySectionId());
				obMsg.AddParam( ME->BreakySectionId() );

				// Pass the message to the parent ent
				ME->ParentCatapult()->GetMessageHandler()->Receive( obMsg );


				// Send a message back to the object that hit the catapult to destroy itself (cannon ball hack)
				CEntity* pCannon = msg.IsPtr("Projectile") ? msg.GetEnt("Projectile") : 0;
				
				if( pCannon && pCannon->GetMessageHandler() )
				{
					Message obCannonBall( msg_ricochet );

					// This will cause the cannon ball to explode
					obCannonBall.AddParam( 0.0f );

					// 
					pCannon->GetMessageHandler()->Receive( obCannonBall );
				}
			}
			END_EVENT(true)

			EVENT(msg_destroy)
			{
				// Build the animation to play
				char acAnimName[64];
				sprintf( acAnimName, "AnimCatapultExplode00%d", ME->BreakySectionId() + 1 );
				ntPrintf("Playing anim %s on breaky section %d\n", acAnimName, ME->BreakySectionId());

				// Create a simple movement to play the animation on
				SimpleTransitionDef obDef;
				obDef.m_bApplyGravity			= false;
				obDef.m_bLooping				= false;
				obDef.m_obAnimationName			= CHashedString(acAnimName);
				obDef.m_fSpeed					= 1.0f;		
				obDef.m_fTimeOffsetPercentage	= 0.0f;

				// Start the animtion.				
				ME->GetMovement()->BringInNewController( obDef, CMovement::DMM_STANDARD, 0.0f );

				// Request that once the animation is complete a 'msg_movementdone' message is sent. 
				Message msg( msg_movementdone );
				ME->GetMovement()->SetCompletionMessage( msg );
				ME->GetMovement()->SetCompletionCallback(AnimatedToDynamic, ME);

				// Place the entity into a destroyed state
				SET_STATE(DESTROYED);
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(DESTROYED)
		BEGIN_EVENTS
			ON_ENTER
			{
			}
			END_EVENT(true)

			EVENT(msg_movementdone)
			{
				/*// done inside the callback now... 
				// Reparent to world
				ME->Lua_ReparentToWorld();

				// Place under physics
				Physics::AnimatedLG* lg = static_cast< Physics::AnimatedLG* > ( ME->GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::ANIMATED_LG) );

				if(lg && lg->IsActive())
				{
					lg->MakeDynamic();
				}*/
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE
END_STATEMACHINE //CATAPULT_BREAKY_FSM


//--------------------------------------------------
//!
//!	Object_Catapult::Object_Catapult()
//!	Default constructor
//!
//--------------------------------------------------
Object_Catapult::Object_Catapult()
:	m_pOther ( 0 )
,	m_iNextArmReset ( 0 )
,	m_iNextArmFire ( 0 )
,	m_bDoReload ( false )
,	m_fSpeed ( 1.0f )
,	m_bMoving( false )
//,	m_fWarningTime (0.0f)
,	m_bCannonCatapult( false )
,	m_bDoneBreak( false )
{
	m_eInteractableType = EntTypeInteractable_Object_Catapult;

	m_eType = EntType_Interactable;

	m_obMovementInput.m_obMoveDirection = m_obMovementInput.m_obFacingDirection = CDirection(0.0f,0.0f,0.0f);
	m_obMovementInput.m_fMoveSpeed = 0.0f;
	m_iProjCollisions = 0;

	// The first few sections that can be destroyed are...
	m_BreakySectionMask = SECTION_LEFT | SECTION_RIGHT | SECTION_TOP | SECTION_TAIL | SECTION_CORE;

	// Clear out the section currenly destroyed
	m_BreakySectionDestroyed = 0;
}

//--------------------------------------------------
//!
//!	Object_Catapult::OnPostConstruct()
//!	Post Construct
//!
//--------------------------------------------------
void Object_Catapult::OnPostConstruct()
{
	Interactable::OnPostConstruct();
	
	// Create components
	InstallMessageHandler();
	InstallDynamics();
	InstallAudioChannel();
	
	// requires a hierarchy
	if( !GetHierarchy() )
		InstallHierarchy();
	if( !m_AnimationContainer.IsNull())
	{
		m_pobAnimator = CAnimator::Create( static_cast< EntityAnimSet * >( this ), m_pobHierarchy, this );

		// Pass on the animator to the EntityAnimSet.
		EntityAnimSet::InstallAnimator( m_pobAnimator, m_AnimationContainer );
	}

	//m_pobSharedAttributes = ObjectDatabase::Get().GetPointerFromName<Att_Catapult*>(m_obSharedAttributesName);
	ntAssert_p(m_pobSharedAttributes, ("Catapult: SharedAttributes have not been set\n") );

	if(!GetMovement()) // Create a movement system if one doesn't already exist
	{
		CMovement* pobMovement = NT_NEW CMovement(this, GetAnimator(), GetPhysicsSystem());
		SetMovement(pobMovement);
	}
}

//--------------------------------------------------
//!
//!	Object_Catapult::OnLevelStart()
//!	Called for each ent on level startup
//!
//--------------------------------------------------
void Object_Catapult::OnLevelStart()
{
	// all moved to level start as this code accesses animations

	// Setup our very special catapult movement controller
	CatapultControllerDef obMoveCycle;
	obMoveCycle.SetDebugNames("Catapult Move Loop","CatapultControllerDef");
	obMoveCycle.m_bApplyGravity = true;
	obMoveCycle.m_obMoveAnimName = m_pobSharedAttributes->m_obMoveAnim;	
	obMoveCycle.m_fMaxRotationPerSecond = m_pobSharedAttributes->m_fMaxRotationPerSecond * DEG_TO_RAD_VALUE;	
	obMoveCycle.m_fMaxNormalPerSecond = m_pobSharedAttributes->m_fMaxNormalPerSecond * DEG_TO_RAD_VALUE;	
	obMoveCycle.m_fMaxHeightPerSecond = m_pobSharedAttributes->m_fMaxHeightPerSecond;	
	obMoveCycle.m_pobHost = this;
	obMoveCycle.m_fMaxAcceleration = m_pobSharedAttributes->m_fMaxAcceleration;

	for (AmmoDefIter obIt = m_pobSharedAttributes->m_aobAmmoDefList.begin(); obIt != m_pobSharedAttributes->m_aobAmmoDefList.end(); ++obIt )
	{
		obMoveCycle.m_obFireAnimNameList.push_back( (*obIt)->m_obFireAnim );
		obMoveCycle.m_obResetAnimNameList.push_back( (*obIt)->m_obReloadAnim );
		m_aobArmStateList.push_back( AS_EMPTY );
		m_aobRockList.push_back( 0 );
	}
	obMoveCycle.m_pobStateList = &m_aobArmStateList;

	GetMovement()->BringInNewController( obMoveCycle, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );
	
	m_fMoveOnProximityThresholdSqu = PROX_THRESHOLD * PROX_THRESHOLD;		// FIX ME Expose to welder
	m_fStopProximityThresholdSqu = STOP_THRESHOLD * STOP_THRESHOLD;			// FIX ME Expose to welder

	m_bDoReload = false;

	user_warn_p( m_pobSharedAttributes->m_aobVolleyTimeList.size() == m_pobSharedAttributes->m_aobAmmoDefList.size(), ("Catapult: VolleyTimeList differs in length from AmmoList, this may cause unexpected results\n") );


	// Assume use point is active to begin with - change with msg_deactivate_usepoint
	m_bUsePointActive = true;
	GetInteractionComponent()->Lua_SetInteractionPriority( USE );

//BREAKYBITS - Begin
	//Hide this clump before adding the children, all the visible clumps will be breakable ones.
//	Hide();	//No longer required.

	// If this is a cannon catapult, then build up each of the sections.
	// Create an array of bit clump names. 
	const ntstd::String* apClumpNames[CATAPULT_BREAKY_COUNT] = {&m_pobSharedAttributes->m_strBreakySection1,
																&m_pobSharedAttributes->m_strBreakySection2,
																&m_pobSharedAttributes->m_strBreakySection3,
																&m_pobSharedAttributes->m_strBreakySection4,
																&m_pobSharedAttributes->m_strBreakySection5,
																&m_pobSharedAttributes->m_strBreakySection6 };


	for( int iSectionID = 0; iSectionID < CATAPULT_BREAKY_COUNT; ++iSectionID )
	{
		// In the beginning there might not be all the clumps
		if( !apClumpNames[iSectionID] || !apClumpNames[iSectionID]->size() )
			continue;

		// Name for the description of the item
		static const char* s_pcDescription = "catapult_breaky";

		// Temperary name for the catapult section
		char acName[128];

		// Generate a name for the section
		sprintf(acName, "%s_Breaky_%d", GetName().c_str(), iSectionID );

		// Sanity check the entity name
		ntAssert( strlen(acName) < sizeof(acName) && "Name for the catapult is too long, please shorten it" );
		
		// Create it in Database
		DataObject* pDO = ObjectDatabase::Get().ConstructObject("Catapult_BreakySection", acName, GameGUID(), 0, true, false);

		// Obtain the pointer to the object. 
		Catapult_BreakySection* pNewBreaky = (Catapult_BreakySection*) pDO->GetBasePtr();
		ntError_p( pNewBreaky != NULL, ("Error: couldn't allocate a breaky section") );

		if(pNewBreaky)
		{
			// Give it an attribute table and get a pointer to it
			pNewBreaky->SetAttributeTable(LuaAttributeTable::Create());
			pNewBreaky->GetAttributeTable()->SetDataObject(pDO);
			LuaAttributeTable* pAttributes = pNewBreaky->GetAttributeTable();

			// Set the basics for the entity in this good old fasioned way
			pAttributes->SetAttribute("Name",				acName);
			pAttributes->SetAttribute("Clump",				apClumpNames[iSectionID]->c_str());
			pAttributes->SetAttribute("Description",		s_pcDescription);
			pAttributes->SetAttribute("DefaultDynamics",	"Animated");
			pAttributes->SetInteger  ("SectorBits",			GetMappedAreaInfo() );

			// Record the breaky section the entity is. 
			pNewBreaky->BreakySectionId( iSectionID );

			// Pass a pointer that is type safe to the breaky section. 
			pNewBreaky->ParentCatapult( this );

			//Set the animation container for the breaky section
			pNewBreaky->SetAnimationContainer( m_AnimationContainer );

			// Perform the post load construction on the entity
			ObjectDatabase::Get().DoPostLoadDefaults(pDO);

			// Parent to this main base
			pNewBreaky->SetParentEntity( this );

			// Get our root transform, and the parent root transform
			Transform* pOurTransform = pNewBreaky->GetHierarchy()->GetRootTransform();
			Transform* pParentTransform = GetHierarchy()->GetRootTransform();

			// Now deal explicitly with the transforms
			pOurTransform->RemoveFromParent();

			// Reparent this
			pParentTransform->AddChild(pOurTransform);

			// Save the breaky section in the catapult
			m_apobBreakyBits[ iSectionID ] = pNewBreaky;

			//Make sure the breaky sections don't attempt to collide with the main catapult (even when un-parented to explode)
			if(pNewBreaky->GetInteractionComponent())
			{
				pNewBreaky->GetInteractionComponent()->ExcludeCollisionWith(this);
			}

			//The new pieces are invisible and inactive until the catapult goes to DESTRUCTABLE state too.
			pNewBreaky->Hide();
			if(pNewBreaky->GetPhysicsSystem())
			{
				pNewBreaky->GetPhysicsSystem()->Deactivate();
			}
		}
	}

	// Make it to that entities don't interact with each other. 
	for( int iSectionID = 0; iSectionID < CATAPULT_BREAKY_COUNT; ++iSectionID )
	{
		// The base breaky section.
		Catapult_BreakySection* pBreakyBreaky = m_apobBreakyBits[ iSectionID ];

		// All the other breaky sections, make sure they don't interact with the breakybreaky
		for( int iExtSectionID = 0; iExtSectionID < CATAPULT_BREAKY_COUNT; ++iExtSectionID )
		{
			if( iExtSectionID == iSectionID )
				continue;

			pBreakyBreaky->GetInteractionComponent()->ExcludeCollisionWith( m_apobBreakyBits[ iExtSectionID ] );
		}
	}
	//BREAKYBITS - End

	// If no users then we can be active imediatly
	bool bActive = (m_pobSharedAttributes->m_aobUserDefList.size() == 0 );

	// Create and attach the statemachine. Must be done AFTER anim containers fixed up by area system
	// i.e. after XML serialisation. OR this shouldnt play an animation
	INTERACTABLE_CATAPULT_FSM* pFSM = NT_NEW INTERACTABLE_CATAPULT_FSM(bActive);
	ATTACH_FSM(pFSM);
}

//--------------------------------------------------
//!
//!	Object_Catapult::OnPostPostConstruct()
//!	Post Post Construct
//!
//--------------------------------------------------
void Object_Catapult::OnPostPostConstruct()
{
	// Ground collision appears not to be avaliable OnPostConstruct.
	CPoint obPosition = GetPosition();
	CPoint obGround(CONSTRUCT_CLEAR);
	CDirection obNormal(CONSTRUCT_CLEAR);
	GroundInfo( obPosition, obGround, obNormal );
	Lua_SetLocalTransform(obGround.X(), obGround.Y(), obGround.Z(), 0.0f, 0.0f, 0.0f);

	//m_fWarningTime = ( (float)CTimer::Get().GetGameTime() - WARNING_TIME );
}

//--------------------------------------------------
//!
//!	Object_Catapult::~Object_Catapult()
//!	Default destructor
//!
//--------------------------------------------------
Object_Catapult::~Object_Catapult()
{
	GetPhysicsSystem()->Lua_RemoveChildEntities();

	//ntPrintf("Object_Catapult::~Object_Catapult\n");
	for (PointIter obIt = m_aobMoveList.begin(); obIt != m_aobMoveList.end(); ++obIt )
	{
		DeletePoint( *obIt );
	}
	m_aobMoveList.clear();

	for (PointIter obIt = m_aobTargetList.begin(); obIt != m_aobTargetList.end(); ++obIt )
	{
		DeletePoint( *obIt );
	}
	m_aobTargetList.clear();

	m_aobRockList.clear();
}

//--------------------------------------------------
//!
//!	Object_Catapult::ReloadProjectile()
//!	Reload the catapults ammo
//!
//--------------------------------------------------
bool Object_Catapult::ReloadProjectile( u_int iArmId )
{
	Att_Catapult* pCatapultAtts = GetSharedAttributes();
	ntAssert(pCatapultAtts);
	ntAssert(iArmId < pCatapultAtts->m_aobAmmoDefList.size() );

	// xml provides a ntstd::list so we have no random access, have to do a manual find
	u_int i = 0;
	for (AmmoDefIter obIt = pCatapultAtts->m_aobAmmoDefList.begin(); obIt != pCatapultAtts->m_aobAmmoDefList.end(); ++obIt )
	{
		if ( i == iArmId )
		{
			if( (!m_aobRockList[i]) && (m_aobArmStateList[i] == AS_EMPTY) )
			{
				m_aobArmStateList[i] = AS_READY;
				m_aobRockList[i] = Object_Catapult_Rock::ConstructCatapultRockObject( this, (*obIt), pCatapultAtts->m_pobAmmoAtts );
				return true;
			}
			else
				return false;
		}
		++i;
	}
	return false;
}

//--------------------------------------------------
//!
//!	Object_Catapult::ResetArm( int iArmId )
//!	Reset the catapults arm
//!
//--------------------------------------------------
bool Object_Catapult::ResetArm( u_int iArmId )
{
	Att_Catapult* pCatapultAtts = GetSharedAttributes();
	ntAssert(pCatapultAtts);
	ntAssert(iArmId < pCatapultAtts->m_aobAmmoDefList.size() );

	// xml provides a ntstd::list so we have no random access, have to do a manual find
	u_int i = 0;
	for (AmmoDefIter obIt = pCatapultAtts->m_aobAmmoDefList.begin(); obIt != pCatapultAtts->m_aobAmmoDefList.end(); ++obIt )
	{
		if ( i == iArmId )
		{
			if ( m_aobArmStateList[i] != AS_STOP )
				return false;
			
			// Flag for movement controller to start animation
			m_aobArmStateList[i] = AS_RESET;

			// Set a message for the rock respawn
			Message msgRespawn(msg_reload);
			msgRespawn.SetEnt(CHashedString(HASH_STRING_SENDER), (CEntity*)this);
			msgRespawn.SetInt("ArmID", (int)iArmId);
			this->GetMessageHandler()->QueueMessageDelayed(msgRespawn, (*obIt)->m_fRespawnTime );

			return true;
		}
		++i;
	}
	return false;
}

//--------------------------------------------------
//!
//!	Object_Catapult::FireProjectile( int iArmId )
//!	Fire the catapults ammo
//!
//--------------------------------------------------
bool Object_Catapult::FireProjectile( u_int iArmId )
{
	Att_Catapult* pCatapultAtts = GetSharedAttributes();
	ntAssert(pCatapultAtts);
	ntAssert(iArmId < pCatapultAtts->m_aobAmmoDefList.size() );

	// xml provides a ntstd::list so we have no random access, have to do a manual find
	u_int i = 0;
	for (AmmoDefIter obIt = pCatapultAtts->m_aobAmmoDefList.begin(); obIt != pCatapultAtts->m_aobAmmoDefList.end(); ++obIt )
	{
		if ( i == iArmId)
		{
			if ( m_aobArmStateList[i] != AS_READY )
				return false;
			
			// Flag for movement controller to start animation
			m_aobArmStateList[i] = AS_FIRE;

			CEntity* pobRock = m_aobRockList[i];

			if (pobRock)
			{
				CPoint obTarget(CONSTRUCT_CLEAR);

				// Have we a target on our list?
				if( !m_aobTargetList.empty() )
				{
					obTarget = *m_aobTargetList.front();
					DeletePoint( m_aobTargetList.front() );
					m_aobTargetList.pop_front();
				}
				// ... Otherwise use the default
				else
				{
					float ax,ay,az;	
					CCamUtil::EulerFromMat_XYZ(GetMatrix(),ax,ay,az);	
					CMatrix obRot;
					CCamUtil::MatrixFromEuler_XYZ(obRot,0.0f,ay,0.0f);		

					// This arms default target rotated by catapults heading
					obTarget = GetPosition() + (*obIt)->m_obTarget * obRot;	

					// Target on ground please
					CPoint obGround(CONSTRUCT_CLEAR);
					CDirection obNormal;
					GroundInfo( obTarget, obGround, obNormal );
					obTarget = obGround;
				}

				// Fire at wil
				Message msgFire(msg_fire);
				msgFire.SetEnt(CHashedString(HASH_STRING_OTHER), (CEntity*)this);
				msgFire.SetFloat("X", obTarget.X());
				msgFire.SetFloat("Y", obTarget.Y());
				msgFire.SetFloat("Z", obTarget.Z());	
				pobRock->GetMessageHandler()->QueueMessage(msgFire);

				// Set a message for the arm to auto reset
				Message msgReset(msg_reset);
				msgReset.SetEnt(CHashedString(HASH_STRING_SENDER), (CEntity*)this);
				msgReset.SetInt("ArmID", (int)iArmId);
				this->GetMessageHandler()->QueueMessageDelayed(msgReset, (*obIt)->m_fAutoResetTime );

				m_aobRockList[i] = 0;
			}
			return true;
		}
		++i;
	}
	return false;
}

//--------------------------------------------------
//!
//!	Object_Catapult::CreatePoint()
//!	Object_Catapult::DeletePoint()
//!	Helpers for safe point lists
//!
//--------------------------------------------------
CPoint* Object_Catapult::CreatePoint( CPoint* pobPoint )
{
	CPoint* pobNewPoint =  NT_NEW_CHUNK( Mem::MC_ENTITY ) CPoint (*pobPoint);
	return pobNewPoint;
}

CPoint* Object_Catapult::CreatePoint( CPoint& obPoint )
{
	CPoint* pobNewPoint = NT_NEW_CHUNK( Mem::MC_ENTITY ) CPoint ( obPoint );
	return pobNewPoint;
}

void Object_Catapult::DeletePoint( CPoint* pobPoint )
{
	NT_DELETE_CHUNK( Mem::MC_ENTITY, pobPoint );
}


//--------------------------------------------------
//!
//!	Object_Catapult::GetPoint()
//!	Helpers to get move to and target locations
//!
//--------------------------------------------------
CPoint* Object_Catapult::GetPoint( const Message msg )
{
	CPoint obPoint(CONSTRUCT_CLEAR);

	bool bPointSpecified = false;

	//Retrieve world position from message parameters.
	if ( msg.IsNumber("X") || msg.IsNumber("Y") || msg.IsNumber("Z") )
	{
		if ( msg.IsNumber("X") )
			obPoint.X() = msg.GetFloat("X");
		if ( msg.IsNumber("Y") )
			obPoint.Y() = msg.GetFloat("Y");
		if ( msg.IsNumber("Z") )
			obPoint.Z() = msg.GetFloat("Z");

		bPointSpecified = true;
	}
	//Retrieve world delta position from message parameters.
	else if ( msg.IsNumber("DeltaX") || msg.IsNumber("DeltaY") || msg.IsNumber("DeltaZ") )
	{
		obPoint = GetPosition();
		if ( msg.IsNumber("DeltaX") )
			obPoint.X() += msg.GetFloat("DeltaX");
		if ( msg.IsNumber("DeltaY") )
			obPoint.Y() += msg.GetFloat("DeltaY");
		if ( msg.IsNumber("DeltaZ") )
			obPoint.Z() += msg.GetFloat("DeltaZ");

		bPointSpecified = true;
	}
	//Retrieve catapult relative position from message parameters.
	else if ( msg.IsNumber("RelX") || msg.IsNumber("RelY") || msg.IsNumber("RelZ") )
	{
		//obPoint = GetPosition();
		if ( msg.IsNumber("RelX") )
			obPoint.X() = msg.GetFloat("RelX");
		if ( msg.IsNumber("RelY") )
			obPoint.Y() = msg.GetFloat("RelY");
		if ( msg.IsNumber("RelZ") )
			obPoint.Z() = msg.GetFloat("RelZ");

		float ax,ay,az;	
		CCamUtil::EulerFromMat_XYZ(GetMatrix(),ax,ay,az);	
		CMatrix obRot;
		CCamUtil::MatrixFromEuler_XYZ(obRot,0.0f,ay,0.0f);		

		// Rotate in Y axis only
		obPoint = GetPosition() + obPoint * obRot;

		bPointSpecified = true;
	}
	else if ( msg.IsHash("Locator") )
	{
		CHashedString obName = msg.GetHashedString("Locator");
		CEntity* pobLocator = ObjectDatabase::Get().GetPointerFromName<CEntity*>( obName );
		if (pobLocator)
		{
			obPoint = pobLocator->GetPosition();
		}
		else
		{
			user_warn_msg(( "Catapult: Unable to find locator %s" , ntStr::GetString( obName ) ));
		}

		bPointSpecified = true;
	}
	else if ( msg.IsPtr("Entity") )
	{
		CEntity* pobEnt = msg.GetEnt("Entity");
		obPoint = pobEnt->GetPosition();

		bPointSpecified = true;
	}

	CPoint obGround(CONSTRUCT_CLEAR);
	CDirection obNormal;
	GroundInfo( obPoint, obGround, obNormal );

	return bPointSpecified ? CreatePoint( obGround ) : 0;
}


//------------------------------------------------------------------------------------------
//!
//!	Object_Catapult::GroundInfo( CPoint& obCoords, CPoint& obGround, CDirection& obNormal )
//!	Retreive information about the ground beneith our feet
//!
//------------------------------------------------------------------------------------------
void Object_Catapult::GroundInfo( CPoint& obCoords, CPoint& obGround, CDirection& obNormal )
{
	CPoint start( obCoords.X(), 1000.0f, obCoords.Z());
	CPoint end( obCoords.X(), -1000.0f, obCoords.Z());

	Physics::TRACE_LINE_QUERY query;

	Physics::RaycastCollisionFlag flag;
	flag.base = 0;
	flag.flags.i_am = Physics::STATIC_ENVIRONMENT_BIT;
	flag.flags.i_collide_with = ( Physics::LARGE_INTERACTABLE_BIT );

	//g_VisualDebug->RenderLine(start, end, DC_GREEN);
	
	if ( Physics::CPhysicsWorld::Get().TraceLine( start, end, NULL, query, flag ) )
	{
		obGround = query.obIntersect;
		obNormal = query.obNormal; 
		//g_VisualDebug->RenderLine(obGround, obGround + obNormal * 100.0f, DC_BLUE);
	} else
	{
		obGround = CPoint(CONSTRUCT_CLEAR);
		obNormal = CDirection(CONSTRUCT_CLEAR);
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Object_Catapult::UpdateInput( )
//!	Per frame update to the movement input
//!
//------------------------------------------------------------------------------------------
void Object_Catapult::UpdateInput( void )
{
	CDirection obVector( *m_aobMoveList.front() - GetPosition() );

	obVector.Y() = 0; // In plane for the time being

	if ( !m_aobMoveList.empty() )
	{
		// Close enough to move onto next point
		if ( (m_aobMoveList.size() > 1) && (obVector.LengthSquared() < m_fMoveOnProximityThresholdSqu) )
		{
			DeletePoint( m_aobMoveList.front() );
			m_aobMoveList.pop_front();
		}
		// Last point, close enough to stop
		else if (obVector.LengthSquared() < m_fStopProximityThresholdSqu)
		{
			DeletePoint( m_aobMoveList.front() );
			m_aobMoveList.pop_front();
		}
	}

	// Update for this frames info
	if ( !m_aobMoveList.empty() )
	{
		obVector =  CDirection( *m_aobMoveList.front() - GetPosition() );
		obVector.Y() = 0;
		obVector.Normalise();
		GetMovementInput()->m_obMoveDirection = obVector;
		GetMovementInput()->m_fMoveSpeed = m_fSpeed;

		
		CPoint obPosition = GetPosition();
		CPoint obGround(CONSTRUCT_CLEAR);
		CDirection obNormal(CONSTRUCT_CLEAR);
		GroundInfo( obPosition, obGround, obNormal );
		
		GetMovementInput()->m_obMoveDirectionAlt = CDirection( obGround - obPosition );

		GetMovementInput()->m_obFacingDirection = obNormal;

#ifdef _DEBUG
		PointIter obIt = m_aobMoveList.begin();
		g_VisualDebug->RenderPoint(**obIt,10.0f,DC_GREEN);
		g_VisualDebug->RenderArc(CMatrix (CQuat(CONSTRUCT_CLEAR), **obIt), PROX_THRESHOLD, TWO_PI, DC_GREEN);
		g_VisualDebug->RenderArc(CMatrix (CQuat(CONSTRUCT_CLEAR), **obIt), STOP_THRESHOLD, TWO_PI, DC_GREEN);
		for ( ++obIt; obIt !=  m_aobMoveList.end(); ++obIt )
		{
			g_VisualDebug->RenderPoint(**obIt,10.0f,DC_RED);
			g_VisualDebug->RenderArc(CMatrix (CQuat(CONSTRUCT_CLEAR), **obIt), PROX_THRESHOLD, TWO_PI, DC_RED);	
			g_VisualDebug->RenderArc(CMatrix (CQuat(CONSTRUCT_CLEAR), **obIt), STOP_THRESHOLD, TWO_PI, DC_RED);	
		}
#endif // _DEBUG
	}
	else
	{
		GetMovementInput()->m_obMoveDirection = GetMatrix().GetZAxis();
		GetMovementInput()->m_fMoveSpeed = 0.0f;
		GetMovementInput()->m_obMoveDirectionAlt = CDirection(CONSTRUCT_CLEAR);
		GetMovementInput()->m_obFacingDirection = GetMatrix().GetYAxis();

		// Process any OnComplete events
		GetMessageHandler()->ProcessEvent("OnComplete");
	}
}

//--------------------------------------------------
//!
//!	Object_Catapult::GetArmID( int& iArmId )
//!	Helper to get armId
//!
//--------------------------------------------------
bool Object_Catapult::GetArmID( const Message msg, u_int& iArmId )
{
	if ( msg.IsNumber("ArmID") )
	{
		iArmId = (u_int)msg.GetInt("ArmID");
		
		// In valid range?
		if ( iArmId >= 0 && iArmId < m_aobArmStateList.size() )
			return true;
	}

	return false;
}


//------------------------------------------------------------------------------------------
//!  public  SignalBreakyExplosion
//!
//!  @param [in, out]  pBreaky CEntity *  
//!  @param [in]       iSectionID int     
//!
//!  @author GavB @date 16/10/2006
//------------------------------------------------------------------------------------------
void Object_Catapult::SignalBreakyExplosion( CEntity* pBreaky, u_int uiSectionID )
{
	ntPrintf("Breaky Section Mask is currently %d\n", m_BreakySectionMask);
	u_int uiSectionMask = 1 << uiSectionID;

	if( uiSectionMask & m_BreakySectionMask )
	{
		// Based on the section now breaking, set other sections that can be broken. 
		switch( uiSectionMask )
		{
			case SECTION_LEFT:
				break;

			case SECTION_BASE:
				break;

			case SECTION_RIGHT:
				break;

			case SECTION_TOP:
				break;

			case SECTION_CORE:
				{
					// Disable catapult
					EXTERNALLY_SET_STATE( INTERACTABLE_CATAPULT_FSM, DESTROYED );

					// Ensure any attached rocks fall
					for ( ntstd::Vector<CEntity*>::iterator obIt = m_aobRockList.begin();
						obIt != m_aobRockList.end(); obIt++ )
					{
						CEntity* pobRock = *obIt;

						if (pobRock)
						{
							Message obMsg( msg_detach );
							obMsg.SetEnt( "Sender", this );
							pobRock->GetMessageHandler()->Receive( obMsg );
						}
					}

					// Hit the core, remove other surviving sections
					// Leave the base so catapult can grind to a halt	
					// And the core as we know its just been destroyed
					for(int i = 0 ; i < CATAPULT_BREAKY_COUNT ; i++)
					{
						int iBreak = (1 << i);

						if ( ( SECTION_CORE == iBreak ) || ( SECTION_BASE == iBreak ) )
							continue;

						if(!(m_BreakySectionDestroyed & iBreak))
						{
							CEntity* pThisBreakyBit = m_apobBreakyBits[i];
							if(pThisBreakyBit && pThisBreakyBit->GetMessageHandler())
							{
								Message obDestroyMessage(msg_destroy);
								obDestroyMessage.SetEnt("Sender", this);
								pThisBreakyBit->GetMessageHandler()->Receive(obDestroyMessage);
							}
						}
					}
				}
				break;

			case SECTION_TAIL:
				break;

			default:
				ntAssert( false && "Oi! Please see gavb about this\n" );
				break;
		}


		// Mark the section that's just been destroyed, assuming that message is sent. 
		m_BreakySectionDestroyed |= uiSectionMask;

		// Pass a message to the parent catapult telling it of the collision
		Message obMsg( msg_destroy );

		// Set the sender of the message.
		obMsg.SetEnt( "Sender", this );

		// Pass the message to the parent ent
		pBreaky->GetMessageHandler()->Receive( obMsg );

		// Enable the destruction of different sections
		/*if( !(m_BreakySectionMask & SECTION_CORE)  &&
			( (m_BreakySectionDestroyed & SECTION_LEFT) || 
			(m_BreakySectionDestroyed & SECTION_RIGHT) || 
			(m_BreakySectionDestroyed & SECTION_TOP) ||
			(m_BreakySectionDestroyed & SECTION_TAIL) ) )
		{
			m_BreakySectionMask |= SECTION_CORE;
		}*/

		if( (m_BreakySectionDestroyed & SECTION_CORE) && !(m_BreakySectionMask & SECTION_BASE) )
		{
			m_BreakySectionMask |= SECTION_BASE;
		}

		ntPrintf("Breaky Section Mask is now %d\n", m_BreakySectionMask);
	}
}


void Object_Catapult::ExplodeWholeCatapult()
{
	int iNumBrokenBits = 0;
	//Break every section that hasn't already been broken off.
//	for(int i = 0 ; i < CATAPULT_BREAKY_COUNT ; i++)
	for(int i = 0 ; i < CATAPULT_BREAKY_COUNT && iNumBrokenBits < 2 ; i++)	//Temporary, exploding them all kills havok memory on PS3.
	{
		if(!(m_BreakySectionDestroyed & (1 << i)))
		{
			CEntity* pThisBreakyBit = m_apobBreakyBits[i];
			if(pThisBreakyBit && pThisBreakyBit->GetMessageHandler())
			{
				Message obDestroyMessage(msg_destroy);
				obDestroyMessage.SetEnt("Sender", this);
				pThisBreakyBit->GetMessageHandler()->Receive(obDestroyMessage);
			}
		}
	}


	//Mark our m_BreakySectionDestroyed as being all segments destroyed.
	m_BreakySectionDestroyed = SECTION_LEFT | SECTION_BASE | SECTION_RIGHT | SECTION_TOP | SECTION_CORE | SECTION_TAIL;
}

void Object_Catapult::MoveSetup ( void )
{
	Att_Catapult* pCatapultAtts = GetSharedAttributes();
	ntAssert(pCatapultAtts);
	
	UserDefIter defIt = pCatapultAtts->m_aobUserDefList.begin();
	for (CharacterIter obIt = m_aobUserList.begin(); obIt != m_aobUserList.end(); ++obIt )
	{
		ntAssert(*obIt);
		ntAssert(*defIt);
		ntAssert( defIt != pCatapultAtts->m_aobUserDefList.end() );
		
		GetInteractionComponent()->ExcludeCollisionWith( *obIt );
		ntAssert( ( *obIt )->GetMovement() );
		CPoint obRot(CONSTRUCT_CLEAR);
		( *obIt )->GetMovement()->Lua_AltStartLinkedMovement((*defIt)->m_obAnimationMoveLoop, this, (*defIt)->m_obTransform, (*defIt)->m_obTranslation, obRot );
		defIt++;
	}	
	
	// Reset input for new movement
	GetMovementInput()->m_fMoveSpeed = m_fSpeed;
	GetMovementInput()->m_obMoveDirection = GetMatrix().GetZAxis();
	GetMovementInput()->m_obMoveDirectionAlt = CDirection(CONSTRUCT_CLEAR);	
	GetMovementInput()->m_obFacingDirection = GetMatrix().GetYAxis();
}

void Object_Catapult::IdleSetup ( void )
{
	m_bDoReload = false;
	Att_Catapult* pCatapultAtts = GetSharedAttributes();
	ntAssert(pCatapultAtts);

	UserDefIter defIt = pCatapultAtts->m_aobUserDefList.begin();
	for (CharacterIter obIt = m_aobUserList.begin(); obIt != m_aobUserList.end(); ++obIt )
	{
		ntAssert(*obIt);
		ntAssert(*defIt);
		ntAssert( defIt != pCatapultAtts->m_aobUserDefList.end() );
		
		GetInteractionComponent()->ExcludeCollisionWith( *obIt );
		ntAssert( ( *obIt )->GetMovement() );
		CPoint obRot(CONSTRUCT_CLEAR);
		( *obIt )->GetMovement()->Lua_AltStartLinkedMovement((*defIt)->m_obAnimationIdleLoop, this, (*defIt)->m_obTransform, (*defIt)->m_obTranslation, obRot );
		defIt++;
	}

	// Set input for stationary
	GetMovementInput()->m_fMoveSpeed = 0.0f;
	GetMovementInput()->m_obMoveDirection = GetMatrix().GetZAxis();
	GetMovementInput()->m_obMoveDirectionAlt = CDirection(CONSTRUCT_CLEAR);	
	GetMovementInput()->m_obFacingDirection = GetMatrix().GetYAxis();
}

void Object_Catapult::BreakableSetup()
{
	// Only do setup the frist time we move to a breakable state
	if (m_bDoneBreak)
		return;

	ntPrintf("Entering DESTRUCTABLE state... switch clumps here\n");

	//Hide and deactivate the main catapult.
	Hide();

	// Ensure any attached rocks don't hide.
	for ( ntstd::Vector<CEntity*>::iterator obIt = m_aobRockList.begin();
		obIt != m_aobRockList.end(); obIt++ )
	{
		CEntity* pobRock = *obIt;
		if (pobRock)
		{
			pobRock->Show();
		}
	}

	if(GetInteractionComponent())
	{
		GetInteractionComponent()->Lua_SetInteractionPriority(NONE);
	}
	if(GetPhysicsSystem())
	{
		GetPhysicsSystem()->Deactivate();
	}

	//Activate and show all of the breakable segments.
	for(int i = 0 ; i < CATAPULT_BREAKY_COUNT ; i++)
	{
		Catapult_BreakySection* pBreakySection = m_apobBreakyBits[i];
		if(pBreakySection)
		{
			pBreakySection->Show();
			pBreakySection->GetPhysicsSystem()->Activate();
		}
	}

	m_bDoneBreak = true;
}

void Object_Catapult::AllowCollisionWith(CEntity* pobRock, bool bAllow)
{
	// Main catapult clump
	if ( bAllow )
		pobRock->GetInteractionComponent()->Lua_AllowCollisionWith( this );
	else
		pobRock->GetInteractionComponent()->ExcludeCollisionWith( this );

	// Breaky clumps
	for(int i = 0 ; i < CATAPULT_BREAKY_COUNT ; i++)
	{
		CEntity* pThisBreakyBit = m_apobBreakyBits[i];

		if ( bAllow )
			pobRock->GetInteractionComponent()->Lua_AllowCollisionWith( pThisBreakyBit );
		else
			pobRock->GetInteractionComponent()->ExcludeCollisionWith( pThisBreakyBit );
	}
}

//------------------------------------------------------------------------------------------
//!  public constructor  Catapult_BreakySection
//!
//!
//!  @author GavB @date 13/10/2006
//------------------------------------------------------------------------------------------
Catapult_BreakySection::Catapult_BreakySection()
{
	m_eInteractableType = EntTypeInteractable_Object_Catapult_Breaky;
	m_eType				= EntType_Interactable;
}

//------------------------------------------------------------------------------------------
//!  public destructor  ~Catapult_BreakySection
//!
//!
//!  @author GavB @date 13/10/2006
//------------------------------------------------------------------------------------------
Catapult_BreakySection::~Catapult_BreakySection()
{
}

//------------------------------------------------------------------------------------------
//!  public  OnPostConstruct
//!
//!  @author GavB @date 13/10/2006
//------------------------------------------------------------------------------------------
void Catapult_BreakySection::OnPostConstruct()
{
	//Be sure to use Interactable::OnPostConstruct here, not CEntity::OnPostConstruct, as Interactable::OnPostConstruct
	//sets m_ConstructinScript to 0. If you just call CEntity::OnPostConstruct then the default construction script is
	//Static_Construct which will create a duplicate of all of the physics and dump it at the origin.
	Interactable::OnPostConstruct();

	InstallMessageHandler();
	InstallDynamics();
	
	// requires a hierarchy
	if( !GetHierarchy() )
		InstallHierarchy();

	// Animations?
	if( !m_AnimationContainer.IsNull() )
	{
		m_pobAnimator = CAnimator::Create( static_cast< EntityAnimSet * >( this ), m_pobHierarchy, this );

		// Pass on the animator to the EntityAnimSet.
		EntityAnimSet::InstallAnimator( m_pobAnimator, m_AnimationContainer );
	}

	// Create a movement system if one doesn't already exist
	if(!GetMovement()) 
	{
		CMovement* pobMovement = NT_NEW CMovement(this, GetAnimator(), GetPhysicsSystem());
		SetMovement(pobMovement);
	}
}

//--------------------------------------------------
//!
//!	Catapult_BreakySection::OnLevelStart()
//!	Called for each ent on level startup
//!
//--------------------------------------------------
void Catapult_BreakySection::OnLevelStart()
{
	// Create and attach the statemachine. Must be done AFTER anim containers fixed up by area system
	// i.e. after XML serialisation. OR this shouldnt play an animation
	CATAPULT_BREAKY_FSM* pFSM = NT_NEW CATAPULT_BREAKY_FSM();
	ATTACH_FSM(pFSM);
}

//------------------------------------------------------------------------------------------
//!  public  OnPostPostConstruct
//!
//!  @author GavB @date 13/10/2006
//------------------------------------------------------------------------------------------
void Catapult_BreakySection::OnPostPostConstruct()
{
	CEntity::OnPostPostConstruct();

}
