//----------------------------------------------------------------------------------------------------
//!
//!	\file game/entityinteractableturretweapon.cpp
//!	Definition of the Interactable Turret Weapon
//!
//----------------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------------
//! Includes
//----------------------------------------------------------------------------------------------------
#include "entityinteractableturretweapon.h"

#include "objectdatabase/dataobject.h"
#include "game/luaattrtable.h"

#include "game/interactioncomponent.h"
#include "game/messagehandler.h"
#include "game/entityprojectile.h"
#include "game/movement.h"
#include "anim/animator.h"
#include "camera/camman.h"
#include "camera/camview.h"
#include "camera/camutils.h"
#include "physics/system.h"
#include "core/timer.h"
#include "core/exportstruct_anim.h"
#include "game/interactiontransitions.h"
#include "game/simpletransition.h"


void ForceLinkFunctionTurretWeapon()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionTurretWeapon() !ATTN!\n");
}

//----------------------------------------------------------------------------------------------------
//!
//! Turret Weapon Interface
//!
//----------------------------------------------------------------------------------------------------
START_STD_INTERFACE(Att_Turret_Weapon)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_sAnimMount,          	"hero_intact_cannon_on",		AnimMount)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_sAnimDismount,       	"hero_intact_cannon_off",		AnimDismount)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_sAnimRecoil,				"",								RecoilAnimation)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_sAnimReload,				"",								ReloadAnimation)

	PUBLISH_VAR_WITH_DEFAULT_AS	(m_sMovementController, 	"CannonController",				MovementController)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_sDrivingSeat,        	"pivot",						DrivingSeat)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_sLaunchTransform,        "pivot",						LaunchTransform)
	PUBLISH_PTR_AS				(m_pProjectileAttribs,  									ProjectileAttribs)
	PUBLISH_PTR_AS				(m_pobAftertouchParams,										AftertouchParams)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_ptTranslationOffset, 	CPoint(0,1,0),					TranslationOffset)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_ptTurrentCam, 			CPoint(-1,1,-3),				TurrentCam)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_bCanDoAftertouch, 		true,							CanDoAftertouch)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_fShootDelay, 			0.0f,							ShootDelay)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_fReloadDelay, 			1.0f,							ReloadDelay)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_fReadyDelay, 			1.0f,							ReadyDelay)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_fRecoilTime, 			1.0f,							RecoilTime)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_fReloadTime, 			1.0f,							ReloadTime)
END_STD_INTERFACE


START_STD_INTERFACE(Interactable_TurretWeapon)
	DEFINE_INTERFACE_INHERITANCE(Interactable)
	COPY_INTERFACE_FROM(Interactable)
	DECLARE_POSTCONSTRUCT_CALLBACK(OnPostConstruct)

	PUBLISH_VAR_WITH_DEFAULT_AS	(m_AnimationContainer,		"CannonAnimContainer",			AnimationContainer)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_fMaxYaw, 				0.0f,							MaxYaw)

	PUBLISH_PTR_AS( m_pobSharedAttributes, SharedAttributes)
	
END_STD_INTERFACE


//----------------------------------------------------------------------------------------------------
//!
//! Turret Weapon State Machine
//!
//----------------------------------------------------------------------------------------------------
STATEMACHINE(INTERACTABLE_TURRETWEAPON_FSM, Interactable_TurretWeapon)
	INTERACTABLE_TURRETWEAPON_FSM()
	{
		SET_INITIAL_STATE(State_Default);
	}

	//------------------------------------------------------------------------------------------------
	// State_Default
	//------------------------------------------------------------------------------------------------
	STATE(State_Default)
		BEGIN_EVENTS
			ON_ENTER
				ME->GetInteractionComponent()->SetInteractionType(USE);
			END_EVENT(true)

			EVENT(msg_action)
			EVENT(msg_running_action)
			{
				ME->SetOperator((Character*)msg.GetEnt("Other"));
				SET_STATE(State_Mounting);
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE // State_Default

	//------------------------------------------------------------------------------------------------
	// State_Mounting
	//------------------------------------------------------------------------------------------------
	STATE(State_Mounting)
		BEGIN_EVENTS
			ON_ENTER
			{
				// Set the operator as being non-collidable
				ME->GetOperator()->GetPhysicsSystem()->Lua_Character_SetCollidable(false);

				// Begin movement onto the ballista
				CHashedString sHash(ME->GetDrivingSeat());
				Transform* pTransform = ME->GetHierarchy()->GetTransform(sHash);
				CPoint pos = pTransform->GetLocalTranslation();
				CQuat  rot(0.0f, 0.0f, 0.0f, 1.0f);
				ME->GetOperator()->GetMovement()->Lua_AltStartSnapToMovement(ME->GetMountAnim(), ME, pos, rot, false);
				ME->GetOperator()->GetMovement()->SetCompletionMessage("msg_movementdone");

				// No-one else can use the ballista while we are
				ME->GetInteractionComponent()->SetInteractionType(NONE);
			}
			END_EVENT(true)

			EVENT(msg_interrupt)
				SET_STATE(State_Interrupted);
			END_EVENT(true)

			EVENT(msg_movementdone)
			{
			  	// Get a transition that will do its best to put us in the best movement state
				TurretControllerDef* pDef = ObjectDatabase::Get().GetPointerFromName<TurretControllerDef*>( ME->GetMovementController() );
				ntError_p(pDef, ("Error! Movement definition %s not found\n", ntStr::GetString( ME->GetMovementController() )));

				TurretControllerDef obTempDef = *pDef;

				obTempDef.m_pVehicle = ME;
				obTempDef.m_sDrivingSeat = ME->GetDrivingSeat();
				obTempDef.m_bAIControlled = ME->GetOperator()->GetEntType() == CEntity::EntType_AI;
				obTempDef.m_fMaxYaw = ME->m_fMaxYaw;

				// Push an instance of the controller on to our movement component
				ME->GetOperator()->GetMovement()->BringInNewController( obTempDef, CMovement::DMM_STANDARD, 0.0f );

				if(ME->GetOperator()->GetEntType() != CEntity::EntType_AI)
				{
					//CamMan::Get().GetPrimaryView()
					ME->iCamID = CamMan::Get().GetPrimaryView()->ActivateTurretCamera(ME, ME->GetDrivingSeat(), CDirection(ME->GetSharedAttributes()->m_ptTurrentCam) ); 
				}

				ME->GetMessageHandler()->ProcessEvent("OnAction");
				SET_STATE(State_Mounted);
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE // State_Mounted

	//------------------------------------------------------------------------------------------------
	// State_Mounted
	//------------------------------------------------------------------------------------------------
	STATE(State_Mounted)
		BEGIN_EVENTS
			ON_ENTER
				//if this.attrib.ExitNextThink == true then
				//	this.attrib.ExitNextThink = false
				//	me:msg_grab_on()
				//end
			END_EVENT(true)

			EVENT(msg_interrupt)
				SET_STATE(State_Interrupted);
			END_EVENT(true)

			EVENT(msg_attack_on)
			EVENT(msg_action_on)
				if(!ME->IsExternallyDisabled())
				{
					SET_STATE(State_Shoot);
				}
			END_EVENT(true)

			EVENT(msg_exit_ballista)
			EVENT(msg_grab_on)
				SET_STATE(State_Dismounting);
			END_EVENT(true)
		END_EVENTS
	END_STATE // State_Mounted

	//------------------------------------------------------------------------------------------------
	// State_Dismounting
	//------------------------------------------------------------------------------------------------
	STATE(State_Dismounting)
		BEGIN_EVENTS
			ON_ENTER
			{
				ntAssert( ME->GetOperator() );

				// Re-enable collision with the operator
				ME->GetOperator()->GetPhysicsSystem()->Lua_Character_SetCollidable(true);

				// Remove the turret camera
				CamMan::Get().GetPrimaryView()->RemoveCoolCamera(ME->iCamID);
				ME->iCamID = -1;

				// Dewonkify our local transform
				CHashedString sHash(ME->GetDrivingSeat());
				Transform* pTransform = ME->GetHierarchy()->GetTransform(sHash);
				float ax,ay,az;
				CCamUtil::EulerFromMat_XYZ(pTransform->GetLocalMatrix(),ax,ay,az);
				CMatrix obMat;
				CCamUtil::MatrixFromEuler_XYZ( obMat, 0.f, ay * DEG_TO_RAD_VALUE, 0.f);
				obMat.SetTranslation(pTransform->GetLocalTranslation());
				pTransform->SetLocalMatrix( obMat );
				
				if( ME->GetPhysicsSystem() )
					ME->GetPhysicsSystem()->EntityRootTransformHasChanged();

				// Correct Player
				{
					CorrectiveMovementTransitionDef obDef;
					obDef.SetDebugNames( "", "CorrectiveTransitionDef" );
					obDef.m_obTargetPostion = pTransform->GetWorldTranslation();
					obDef.m_obTargetRotation = pTransform->GetWorldRotation();
					ME->GetOperator()->GetMovement()->BringInNewController( obDef, CMovement::DMM_STANDARD, 0.0f );
				}

				// Play the dismount animation
				{
					SimpleTransitionDef obDef;				
					obDef.m_obAnimationName = ME->GetDismountAnim();
					obDef.SetDebugNames(ntStr::GetString(ME->GetDismountAnim()), "SimpleTransitionDef");
					obDef.m_bLooping = false;
					obDef.m_bApplyGravity = false;
					ME->GetOperator()->GetMovement()->AddChainedController(obDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND);
					ME->GetOperator()->GetMovement()->SetCompletionMessage("msg_movementdone");
				}
			}
			END_EVENT(true)

			EVENT(msg_movementdone)
			{
				ME->GetOperator()->GetPhysicsSystem()->Lua_Character_SetCollidable(true);
				Message msg(msg_exitstate);
				msg.SetEnt("Sender", ME);
				ME->GetOperator()->GetMessageHandler()->QueueMessage(msg);
				SET_STATE(State_Default);
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE // State_Dismounting

	//------------------------------------------------------------------------------------------------
	// State_Shoot
	//------------------------------------------------------------------------------------------------
	STATE(State_Shoot)
		BEGIN_EVENTS
			ON_ENTER
			{
				Message obMsg(msg_fire);
				ME->GetMessageHandler()->QueueMessageDelayed(obMsg, ME->GetSharedAttributes()->m_fShootDelay);

				ME->m_bReloadRequested = true;
			}
			END_EVENT(true)

			EVENT(msg_fire)
			{
				ME->OnFire();
				SET_STATE(State_Reload);
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE	// State_Reload

	STATE(State_Reload)
		BEGIN_EVENTS
			EVENT(msg_think_onaftertouch)
				if( ME->GetOperator()->Lua_IsAttackHeld(0.2f) || ME->GetOperator()->Lua_IsActionHeld(0.2f) )
				{
					SET_STATE(State_Aftertouch);
				}
			END_EVENT(true)

			EVENT(msg_reload)
			{
				ME->OnReload();
			}	
			END_EVENT(true)

			EVENT(msg_reset)
				ME->m_bReloadRequested = false;
				SET_STATE(State_Mounted);
			END_EVENT(true)

			EVENT(msg_interrupt)
				SET_STATE(State_Interrupted);
			END_EVENT(true)
		END_EVENTS
	END_STATE // State_Reload

	//------------------------------------------------------------------------------------------------
	// State_Aftertouch
	//------------------------------------------------------------------------------------------------
	STATE(State_Aftertouch)
		BEGIN_EVENTS
			ON_ENTER
			{
				ntAssert( ME->m_pActiveCannonBall );
				ntAssert( ME->GetSharedAttributes() );
				ntAssert( ME->GetSharedAttributes()->m_pobAftertouchParams );
				ME->GetOperator()->GetMovement()->SetEnabled(false);

				ME->m_pActiveCannonBall->GetPhysicsSystem()->Lua_Projectile_EnableDualSteering(ME->m_pOther, ME->GetSharedAttributes()->m_pobAftertouchParams);

				// Tell the projectile to enter aftertouch state
				Message message(msg_aftertouch);
				message.SetEnt("Sender", ME);
				ME->m_pActiveCannonBall->GetMessageHandler()->QueueMessage(message);
			}
			END_EVENT(true)

			EVENT(msg_interrupt)
				// Re-enable movement on the operator
				ME->GetOperator()->GetMovement()->SetEnabled(true);
				// And disable projectile steering
				ME->m_pActiveCannonBall->GetPhysicsSystem()->Lua_Projectile_DisableSteering();;

				SET_STATE(State_Interrupted);
			END_EVENT(true)

			EVENT(msg_reload)
			{
				ME->OnReload();
			}	
			END_EVENT(true)

			EVENT(msg_reset)
				ME->m_bReloadRequested = false;
			END_EVENT(true)

			EVENT(msg_projectile_aftertouch_complete)
			EVENT(msg_action_off)
			EVENT(msg_attack_off)
			EVENT(msg_projectile_destroyed)
			{
				// Re-enable movement on the operator
				ME->GetOperator()->GetMovement()->SetEnabled(true);
				// And disable projectile steering
				ME->DisableProjectileSteering();
				
				// Disable the steering of item
				ME->m_pActiveCannonBall->GetPhysicsSystem()->Lua_Projectile_DisableSteering();;

				// Send word to end the aftertouch state. 
				Message message(msg_aftertouch_done);
				ME->m_pActiveCannonBall->GetMessageHandler()->QueueMessage(message);

				// Dewonkify our local transform // Why do we need to do this here?
				/*CHashedString sHash(ME->GetDrivingSeat());
				Transform*    pTransform = ME->GetHierarchy()->GetTransform(sHash);
				float ax,ay,az;
				CCamUtil::EulerFromMat_XYZ(pTransform->GetLocalMatrix(),ax,ay,az);
				CMatrix obMat;
				CCamUtil::MatrixFromEuler_XYZ( obMat, 0.f, ay * DEG_TO_RAD_VALUE, 0.f);
				obMat.SetTranslation(pTransform->GetLocalTranslation());
				pTransform->SetLocalMatrix( obMat );

				if( ME->GetPhysicsSystem() )
					ME->GetPhysicsSystem()->EntityRootTransformHasChanged();
				*/
				
				if ( ME->m_bReloadRequested )	
					SET_STATE(State_Reload);
				else
					SET_STATE(State_Mounted);
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE // State_Aftertouch

	//------------------------------------------------------------------------------------------------
	// State_Interrupted
	//------------------------------------------------------------------------------------------------
	STATE(State_Interrupted)
		BEGIN_EVENTS
			ON_ENTER
			{
				// Re-enable collision with the operator
				ME->GetOperator()->GetPhysicsSystem()->Lua_Character_SetCollidable(true);

				// Must remove all animations, or the cannon will be popped to origin by the partial anims
				CAnimator* pobAnimator = ME->GetAnimator();
				if (pobAnimator)
				{
					pobAnimator->RemoveAllAnimations();
				}
				
							// Remove the turret camera
				CamMan::Get().GetPrimaryView()->RemoveCoolCamera(ME->iCamID);
				ME->iCamID = -1;

				// Dewonkify our local transform
				CHashedString sHash(ME->GetDrivingSeat());
				Transform* pTransform = ME->GetHierarchy()->GetTransform(sHash);
				float ax,ay,az;
				CCamUtil::EulerFromMat_XYZ(pTransform->GetLocalMatrix(),ax,ay,az);
				CMatrix obMat;
				CCamUtil::MatrixFromEuler_XYZ( obMat, 0.f, ay * DEG_TO_RAD_VALUE, 0.f);
				obMat.SetTranslation(pTransform->GetLocalTranslation());
				pTransform->SetLocalMatrix( obMat );
				
				if( ME->GetPhysicsSystem() )
					ME->GetPhysicsSystem()->EntityRootTransformHasChanged();

				// Correct Player
				ntAssert( ME->GetOperator() );
				{
					CorrectiveMovementTransitionDef obDef;
					obDef.SetDebugNames( "", "CorrectiveTransitionDef" );
					obDef.m_obTargetPostion = pTransform->GetWorldTranslation();
					obDef.m_obTargetRotation = pTransform->GetWorldRotation();
					ME->GetOperator()->GetMovement()->BringInNewController( obDef, CMovement::DMM_STANDARD, 0.0f );
				}

				// Play the dismount animation
				{
					SimpleTransitionDef obDef;				
					obDef.m_obAnimationName = ME->GetDismountAnim();
					obDef.SetDebugNames(ntStr::GetString(ME->GetDismountAnim()), "SimpleTransitionDef");
					obDef.m_bLooping = false;
					obDef.m_bApplyGravity = false;
					ME->GetOperator()->GetMovement()->AddChainedController(obDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND);
					ME->GetOperator()->GetMovement()->SetCompletionMessage("msg_movementdone");
				}
			}
			END_EVENT(true)

			EVENT(msg_movementdone)
			{
				ME->GetOperator()->GetPhysicsSystem()->Lua_Character_SetCollidable(true);
				Message msg(msg_exitstate);
				msg.SetEnt("Sender", ME);
				ME->GetOperator()->GetMessageHandler()->QueueMessage(msg);
				SET_STATE(State_Default);
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE // State_Interrupted

END_STATEMACHINE


//----------------------------------------------------------------------------------------------------
//!
//!	Interactable_TurretWeapon::Interactable_TurretWeapon()
//!	Default constructor
//!
//----------------------------------------------------------------------------------------------------
Interactable_TurretWeapon::Interactable_TurretWeapon()
	:	iCamID(-1)
	,	m_pOther(0)
	,	m_bExternallyDisabled(false)
	,	m_bReloadRequested(false)
	,	m_pobTurretController(0)
	,	m_pobSharedAttributes(0)
{
	m_eType 				=	EntType_Interactable;
	m_eInteractableType		=	EntTypeInteractable_TurretWeapon;
	m_pobSharedAttributes	= 	0;
	m_pActiveCannonBall		=	0;
	m_eTurretState			=	TS_INACTIVE;
}

//----------------------------------------------------------------------------------------------------
//!
//!	Interactable_TurretWeapon::OnPostConstruct()
//!	Post Construct
//!
//----------------------------------------------------------------------------------------------------
void Interactable_TurretWeapon::OnPostConstruct()
{
	Interactable::OnPostConstruct();
	
	// Create components
	InstallMessageHandler();
	InstallDynamics();

	
	// Animations
	if( !m_AnimationContainer.IsNull() )
	{
		m_pobAnimator = CAnimator::Create( static_cast< EntityAnimSet * >( this ), m_pobHierarchy, this );

		// Pass on the animator to the EntityAnimSet.
		EntityAnimSet::InstallAnimator( m_pobAnimator, m_AnimationContainer );
	}
}

//--------------------------------------------------
//!
//!	Interactable_TurretWeapon::OnLevelStart()
//!	Called for each ent on level startup
//!
//--------------------------------------------------
void Interactable_TurretWeapon::OnLevelStart()
{
	// Create and attach the statemachine. Must be done AFTER anim containers fixed up by area system
	// i.e. after XML serialisation. OR this shouldnt play an animation
	INTERACTABLE_TURRETWEAPON_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) INTERACTABLE_TURRETWEAPON_FSM();
	ATTACH_FSM(pFSM);
}

//----------------------------------------------------------------------------------------------------
//!
//!	Interactable_TurretWeapon::~Interactable_TurretWeapon()
//!	
//!
//----------------------------------------------------------------------------------------------------
Interactable_TurretWeapon::~Interactable_TurretWeapon()
{
}

//----------------------------------------------------------------------------------------------------
//!
//!	Interactable_TurretWeapon::UpdateDerivedComponents()
//!	
//!
//----------------------------------------------------------------------------------------------------
void	Interactable_TurretWeapon::UpdateDerivedComponents( float )
{
	if ( m_pobTurretController )
		m_pobTurretController->PostAnimatorUpdate();
}

//----------------------------------------------------------------------------------------------------
//!
//!	Interactable_TurretWeapon::OnFire()
//!	
//!
//----------------------------------------------------------------------------------------------------
void Interactable_TurretWeapon::OnFire()
{
	ntAssert( GetSharedAttributes() );

	m_pActiveCannonBall = Object_Projectile::CreateCannonBall( this, GetProjectileAttribs() );

	// AT check
	if( CanDoAftertouch() )
	{
		Message obMsg(msg_think_onaftertouch);
		GetMessageHandler()->QueueMessageDelayed(obMsg, 0.2f);
	}

	Message obMsg(msg_reload);
	float fReloadTime = GetSharedAttributes()->m_fReloadDelay;

	if (!GetRecoilAnim().IsNull() )
	{
		CAnimator* pobAnimator = GetAnimator();
		ntAssert( pobAnimator );

		CAnimationPtr obNewAnim = pobAnimator->CreateAnimation( GetRecoilAnim() );

		if (obNewAnim!=0)
		{
			float fSpeed = 1.0f;
			if ( GetSharedAttributes()->m_fRecoilTime != 0.0f )
			{
				fSpeed = obNewAnim->GetDuration() / GetSharedAttributes()->m_fRecoilTime ;
			}
			obNewAnim->SetSpeed( fSpeed );
			obNewAnim->SetFlagBits( 0 );

			pobAnimator->AddAnimation( obNewAnim );

			// Just incase the requested reload delay is too short to accomodate the recoil anim
			if ( fReloadTime < GetSharedAttributes()->m_fRecoilTime)
				fReloadTime = GetSharedAttributes()->m_fRecoilTime;
		}
	}

	GetMessageHandler()->QueueMessageDelayed(obMsg, fReloadTime);

	m_eTurretState = TS_FIRE;

	// Camera shake
	CPoint ptPos = m_pActiveCannonBall->GetPosition();
	CamMan::Get().Shake(2.f, 1.f, 3.f, &ptPos, 50.f*50.f);
}

//----------------------------------------------------------------------------------------------------
//!
//!	Interactable_TurretWeapon::OnReload()
//!	
//!
//----------------------------------------------------------------------------------------------------
void Interactable_TurretWeapon::OnReload()
{
	ntAssert( GetSharedAttributes() );

	Message obMsg(msg_reset);
	float fReadyTime = GetSharedAttributes()->m_fReadyDelay;

	if (!GetReloadAnim().IsNull() )
	{
		CAnimator* pobAnimator = GetAnimator();
		ntAssert( pobAnimator );

		CAnimationPtr obNewAnim = pobAnimator->CreateAnimation( GetReloadAnim() );

		if (obNewAnim!=0)
		{
			float fSpeed = 1.0f;
			if ( GetSharedAttributes()->m_fReloadTime != 0.0f )
			{
				fSpeed = obNewAnim->GetDuration() / GetSharedAttributes()->m_fReloadTime ;
			}
			obNewAnim->SetSpeed( fSpeed );
			obNewAnim->SetFlagBits( 0 );

			pobAnimator->AddAnimation( obNewAnim );

			// Just incase the requested reload delay is too short to accomodate the recoil anim
			if ( fReadyTime < GetSharedAttributes()->m_fReloadTime )
				fReadyTime = GetSharedAttributes()->m_fReloadTime;
		}
	}

	GetMessageHandler()->QueueMessageDelayed(obMsg, fReadyTime);
}


