//--------------------------------------------------
//!
//!	\file game/entityinteractabletraverser.cpp
//!	Definition of the Interactable Traverser entity object
//!
//--------------------------------------------------

#include "objectdatabase/dataobject.h"
#include "game/luaattrtable.h"
#include "game/entitymanager.h"
#include "game/simpletransition.h"
#include "Physics/system.h"
#include "physics/collisionbitfield.h"
#include "messagehandler.h"

#include "interactioncomponent.h"
#include "movement.h"
#include "anim/animator.h"
//#include "core/exportstruct_anim.h"
//#include "game/combathelper.h"
#include "camera/camman.h"
#include "camera/camview.h"
#include "camera/camutils.h"

#include "game/entityinteractableturretpoint.h"
#include "game/entitybindings.h"
#include "game/aimcontroller.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#	include "Physics/advancedcharactercontroller.h"
#endif

void ForceLinkFunctionTurretPoint()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionTurretPoint() !ATTN!\n");
}


START_CHUNKED_INTERFACE(Interactable_Turret_Point, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(Interactable)
	COPY_INTERFACE_FROM(Interactable)

	PUBLISH_VAR_AS(m_Description, Description)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_InitialState, "Active", InitialState)
	PUBLISH_VAR_AS(m_AnimationContainer, AnimationContainer)

	PUBLISH_VAR_AS(	m_EnterAnimation, EnterAnimation)
	PUBLISH_VAR_AS(	m_ExitAnimation, ExitAnimation )
	PUBLISH_VAR_AS(	m_FallAnimation, FallAnimation )
	PUBLISH_VAR_AS(	m_CollapseAnimation, CollapseAnimation )

	PUBLISH_VAR_WITH_DEFAULT_AS( m_obMountPoint, CPoint(0.0f, 0.0f, 0.0f), MountPosition )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_obSpritePoint, CPoint(0.0f, 0.0f, 0.0f), SpritePosition )

	PUBLISH_VAR_WITH_DEFAULT_AS( m_fMinPitch, -25.0f, MinPitch )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fMaxPitch,  25.0f, MaxPitch )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fMinYaw, -360.0f, MinYaw )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fMaxYaw,  360.0f, MaxYaw )

	PUBLISH_VAR_WITH_DEFAULT_AS( m_AttachToParent, 0, AttachToParent )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_AttachToParentTransform, CHashedString("ROOT"), AttachToParentTransform )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_ptAttachToParentOffset, CPoint(0.0f, 0.0f, 0.0f), AttachToParentOffset )

	PUBLISH_VAR_WITH_DEFAULT_AS( m_bRelativeAnimations, true, RelativeAnimations )
	

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )
END_STD_INTERFACE


//--------------------------------------------------
//!
//! Interactable Traverser State Machine
//!
//--------------------------------------------------
STATEMACHINE(INTERACTABLE_TURRET_POINT_FSM, Interactable_Turret_Point)
	INTERACTABLE_TURRET_POINT_FSM(bool bActive)
	{
		if(bActive)
			SET_INITIAL_STATE(ACTIVE);
		else
			SET_INITIAL_STATE(DEACTIVE);
	}

	STATE(DEACTIVE)
		BEGIN_EVENTS
			ON_ENTER
				ME->m_pOther = 0;
				ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);
			END_EVENT(true)

			EVENT(Activate)
				SET_STATE(ACTIVE);
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(ACTIVE)
		BEGIN_EVENTS
			ON_ENTER
				ME->m_pOther = 0;
				ME->GetInteractionComponent()->Lua_SetInteractionPriority(USE);
			END_EVENT(true)

			EVENT(msg_running_action)
			EVENT(msg_action)
				ME->m_pOther = (Character*) (msg.IsPtr("Other") ? msg.GetEnt("Other") : msg.GetEnt("Sender"));
				ntError(!ME->m_pOther || ME->m_pOther->IsCharacter());
				SET_STATE(MOUNTING);
			END_EVENT(true)

			EVENT(Deactivate)
				SET_STATE(DEACTIVE);
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(MOUNTING)
		BEGIN_EVENTS
			ON_ENTER
				ME->GetInteractionComponent()->Lua_SetInteractionPriority( NONE );

				// If there is a valid parent to attach the entity to...
				if( !ME->m_AttachToParent.IsNull() )
				{
					// Find that entity
					CEntity* pMountMe = CEntityManager::Get().FindEntity( ME->m_AttachToParent );

					// and if valid
					if( pMountMe )
					{
						// Disable collision
						ME->m_pOther->GetPhysicsSystem()->Lua_Character_SetCollidable( false );
						pMountMe->GetInteractionComponent()->ExcludeCollisionWith( ME->m_pOther );

						// Attach the entity to the parent. 
						ME->m_pOther->Lua_Reparent( pMountMe, ME->m_AttachToParentTransform );

						// With the given offset. 
/*
						ME->m_pOther->Lua_SetLocalTransform(ME->m_ptAttachToParentOffset.X(),
															ME->m_ptAttachToParentOffset.Y(),
															ME->m_ptAttachToParentOffset.Z(),
															0.0f, 0.0f, 0.0f );
*/
					}  
				} 
	
				// Do we have mount animation?
				if ( !ntStr::IsNull(ME->m_EnterAnimation) )
				{
					ME->GetInteractionComponent()->ExcludeCollisionWith( ME->m_pOther );
					ME->m_pOther->GetPhysicsSystem()->Lua_Character_SetCollidable( false );
					
					// Movement must be soft, or they don't change position after Relative Movement
					if( ME->m_bRelativeAnimations )
					{
						ME->m_pOther->GetMovement()->Lua_StartRelativeMovement( ME, ME->m_EnterAnimation, false, true );
					}
					else
					{
						SimpleTransitionDef obDef;

						obDef.m_bApplyGravity			= false;
						obDef.m_bLooping				= false;
						obDef.m_obAnimationName			= ME->m_EnterAnimation;
						obDef.m_fSpeed					= 1.0f;
						obDef.m_fTimeOffsetPercentage	= 0.0f;
						
						ME->m_pOther->GetMovement()->BringInNewController( obDef, CMovement::DMM_STANDARD, 0.0f );
					}

					ME->m_pOther->GetMovement()->Lua_AltSetMovementCompleteMessage( "msg_movementdone", ME->m_pOther );
				}
				// Or any action events?
				else if ( ME->GetMessageHandler()->ProcessEvent("OnAction") )
				{
					Message msg(msg_return_state);
					msg.SetEnt( CHashedString(HASH_STRING_SENDER), ME);
					msg.SetString( CHashedString("State"), CHashedString("Archer_Aiming_TurretPoint") );
						
					ME->m_pOther->GetMessageHandler()->QueueMessage(msg);
				}
				else
				{
					// Or just move straight on
					SET_STATE(MOUNTED);
				}

			END_EVENT(true)

			EVENT(msg_ninja_sequence_success)
			EVENT(msg_success)
				SET_STATE(MOUNTED);
			END_EVENT(true)

			EVENT(msg_ninja_sequence_fail)
			EVENT(msg_fail)
				SET_STATE(ACTIVE);
			END_EVENT(true)

			EVENT(msg_movementdone)
			{
				ME->GetInteractionComponent()->AllowCollisionWith( ME->m_pOther );
				ME->m_pOther->GetPhysicsSystem()->Lua_Character_SetCollidable( false );

				SET_STATE(MOUNTED);
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE  // MOUNTING

	STATE(MOUNTED)
		BEGIN_EVENTS
			ON_ENTER
			{
				Physics::AdvancedCharacterController* pobAdvCC = (Physics::AdvancedCharacterController*) ME->m_pOther->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
				if (pobAdvCC)
				{
					pobAdvCC->SetApplyCharacterControllerGravity( false );
				}

				AimingComponent* pobAimComponent = (AimingComponent*)ME->m_pOther->GetAimingComponent();
				pobAimComponent->SetAimRange( ME->m_fMinYaw, ME->m_fMaxYaw, ME->m_fMinPitch, ME->m_fMaxPitch );

				//Transform* pobTransform = ME->m_pOther->GetHierarchy()->GetRootTransform();
				/*Transform* pobObjTransform = ME->GetHierarchy()->GetRootTransform();

				CPoint Pos = pobObjTransform->GetWorldTranslation();

				// rotate offset into objects space
				CQuat obRot = ME->GetRotation();	
				CMatrix obMatrix(CONSTRUCT_IDENTITY);
				obMatrix.SetFromQuat(obRot);

				Pos += ME->m_obMountPoint * obMatrix;

				//Pos += ME->m_obMountPoint;
					
				CPoint delta = ME->m_pOther->GetHierarchy()->GetRootTransform()->GetWorldTranslation() - Pos;
				if ( delta.LengthSquared() > EPSILON )
					ME->m_pOther->SetPosition( Pos);*/
				
				Message msg( msg_turret_cablecar );
				msg.SetEnt( CHashedString(HASH_STRING_SENDER), ME);
					
				ME->m_pOther->GetMessageHandler()->QueueMessage(msg);
			}
			END_EVENT(true)

			EVENT(msg_grab_on)
				SET_STATE(DISMOUNTING);
			END_EVENT(true)

			EVENT(msg_interrupt)
				SET_STATE(FALL);
			END_EVENT(true)
		END_EVENTS
	END_STATE // MOUNTED

	STATE(DISMOUNTING)
		BEGIN_EVENTS
			ON_ENTER
			{
				Message msgExitState(msg_exitstate);
				msgExitState.SetEnt( CHashedString(HASH_STRING_SENDER), ME);
				ME->m_pOther->GetMessageHandler()->QueueMessage(msgExitState);

				Physics::AdvancedCharacterController* pobAdvCC = (Physics::AdvancedCharacterController*) ME->m_pOther->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
				if (pobAdvCC)
				{
					pobAdvCC->SetApplyCharacterControllerGravity( true );
				}

				AimingComponent* pobAimComponent = (AimingComponent*)ME->m_pOther->GetAimingComponent();
				pobAimComponent->ResetAimRange();

				// Do we have dismount animation?
				if ( !ntStr::IsNull(ME->m_ExitAnimation) )
				{
					ME->GetInteractionComponent()->ExcludeCollisionWith( ME->m_pOther );
					ME->m_pOther->GetPhysicsSystem()->Lua_Character_SetCollidable( false );
					
					// Movement must be soft, or they don't change position after Relative Movement
					ME->m_pOther->GetMovement()->Lua_StartRelativeMovement( ME, ME->m_EnterAnimation, false, true );

					ME->m_pOther->GetMovement()->Lua_AltSetMovementCompleteMessage( "msg_movementdone", ME->m_pOther );
				}
				// Or any action events?
				else if (! ME->GetMessageHandler()->ProcessEvent("OnComplete") )
				{
					// Or just move straight on
					SET_STATE(ACTIVE);
				}
			}
			END_EVENT(true)

			EVENT(msg_ninja_sequence_success)
			EVENT(msg_success)
			{
				// Clear controllers and exit state for player
				Message msgExitState(msg_exitstate);
				msgExitState.SetEnt( CHashedString(HASH_STRING_SENDER), ME);
				ME->m_pOther->GetMessageHandler()->QueueMessage(msgExitState);

				SET_STATE(ACTIVE);
			}
			END_EVENT(true)

			EVENT(msg_ninja_sequence_fail)
			EVENT(msg_fail)
				SET_STATE(MOUNTED);
			END_EVENT(true)

			EVENT(msg_movementdone)
			{
				// Clear controllers and exit state for player
				Message msgExitState(msg_exitstate);
				msgExitState.SetEnt( CHashedString(HASH_STRING_SENDER), ME);
				ME->m_pOther->GetMessageHandler()->QueueMessage(msgExitState);

				ME->GetInteractionComponent()->AllowCollisionWith( ME->m_pOther );
				ME->m_pOther->GetPhysicsSystem()->Lua_Character_SetCollidable( false );

				SET_STATE(ACTIVE);
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE  // DISMOUNTING

	STATE(FALL)
		BEGIN_EVENTS
			ON_ENTER
			{
				Message msgExitState(msg_exitstate);
				msgExitState.SetEnt( CHashedString(HASH_STRING_SENDER), ME);
				ME->m_pOther->GetMessageHandler()->QueueMessage(msgExitState);

				Physics::AdvancedCharacterController* pobAdvCC = (Physics::AdvancedCharacterController*) ME->m_pOther->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
				if (pobAdvCC)
				{
					pobAdvCC->SetApplyCharacterControllerGravity( true );
				}

				// Do we have a fall animation?
				if ( !ntStr::IsNull(ME->m_FallAnimation) )
				{
					ME->GetInteractionComponent()->ExcludeCollisionWith( ME->m_pOther );
					ME->m_pOther->GetPhysicsSystem()->Lua_Character_SetCollidable( false );
					
					// Movement must be soft, or they don't change position after Relative Movement
					ME->m_pOther->GetMovement()->Lua_StartRelativeMovement( ME, ME->m_FallAnimation, false, true );

					ME->m_pOther->GetMovement()->Lua_AltSetMovementCompleteMessage( "msg_movementdone", ME->m_pOther );
				}
				// Or any action events?
				else if (! ME->GetMessageHandler()->ProcessEvent("OnFail") )
				{
					// Or just move straight on
					SET_STATE(ACTIVE);
				}
			}
			END_EVENT(true)

			EVENT(msg_ninja_sequence_success)
			EVENT(msg_success)
			{
				// Clear controllers and exit state for player
				Message msgExitState(msg_exitstate);
				msgExitState.SetEnt( CHashedString(HASH_STRING_SENDER), ME);
				ME->m_pOther->GetMessageHandler()->QueueMessage(msgExitState);

				SET_STATE(ACTIVE);
			}
			END_EVENT(true)

			EVENT(msg_ninja_sequence_fail)
			EVENT(msg_fail)
				SET_STATE(MOUNTED);
			END_EVENT(true)

			EVENT(msg_movementdone)
			{
				// Clear controllers and exit state for player
				Message msgExitState(msg_exitstate);
				msgExitState.SetEnt( CHashedString(HASH_STRING_SENDER), ME);
				ME->m_pOther->GetMessageHandler()->QueueMessage(msgExitState);

				ME->GetInteractionComponent()->AllowCollisionWith( ME->m_pOther );
				ME->m_pOther->GetPhysicsSystem()->Lua_Character_SetCollidable( false );

				SET_STATE(ACTIVE);
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE  // FALL

END_STATEMACHINE // INTERACTABLE_TURRET_POINT_FSM


//--------------------------------------------------
//!
//!	Interactable_Turret_Point::Interactable_Turret_Point()
//!	Default constructor
//!
//--------------------------------------------------
Interactable_Turret_Point::Interactable_Turret_Point()
{
	m_iCamID = -1;
	m_eType 			=	EntType_Interactable;
	m_eInteractableType =	EntTypeInteractable_Turret_Point;
}

//--------------------------------------------------
//!
//!	Interactable_Turret_Point::OnPostConstruct()
//!	Post Construct
//!
//--------------------------------------------------
void Interactable_Turret_Point::OnPostConstruct()
{
	Interactable::OnPostConstruct();

		// requires a hierarchy
	if( !GetHierarchy() )
		InstallHierarchy();

	// Create components
	InstallMessageHandler();

	if( GetHierarchy() )
		InstallDynamics();

	if ( ! m_AnimationContainer.IsNull() )
		InstallAnimator(m_AnimationContainer);

	bool bActive = false;

	// State machine initial state
	if (m_InitialState == "Active")
		bActive = true;

	// Create and attach the statemachine
	INTERACTABLE_TURRET_POINT_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) INTERACTABLE_TURRET_POINT_FSM(bActive);
	ATTACH_FSM(pFSM);
}

//--------------------------------------------------
//!
//!	Interactable_Turret_Point::~Interactable_Turret_Point()
//!	Default destructor
//!
//--------------------------------------------------
Interactable_Turret_Point::~Interactable_Turret_Point()
{
}
