//------------------------------------------------------------------------------------------
//!
//!	\file archer3rdpController.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "game/archer3rdpController.h"
#include "game/archermovementparams.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "game/entityarcher.h"
#include "game/messagehandler.h"
#include "game/query.h"
#include "game/entitymanager.h"
#include "game/simpletransition.h"
#include "movement.h"
#include "anim/animator.h"
#include "core/exportstruct_anim.h"
#include "objectdatabase/dataobject.h"
#include "core/visualdebugger.h"
#include "game/eventlog.h"


START_STD_INTERFACE	(ThirdPersonAimControllerTransDef)
	PUBLISH_PTR_AS( m_pTransition, StartTransition )
END_STD_INTERFACE

START_STD_INTERFACE	( ThirdPersonAimControllerDef )
	PUBLISH_PTR_AS( m_pCurrentState, EntryState )
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
//!
//!	ThirdPersonAimControllerTransDef::ThirdPersonAimControllerTransDef
//!	Construction
//!
//------------------------------------------------------------------------------------------
ThirdPersonAimControllerTransDef::ThirdPersonAimControllerTransDef( void )
{
	// Set the default animation speed
	m_pTransition = 0;
}


//------------------------------------------------------------------------------------------
//!
//!	ThirdPersonAimControllerTransDef::CreateInstance
//!	To create an instance of what we describe
//!
//------------------------------------------------------------------------------------------
MovementController* ThirdPersonAimControllerTransDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) ThirdPersonAimControllerTrans( pobMovement, *this );
}


//------------------------------------------------------------------------------------------
//!
//!	ThirdPersonAimController::ThirdPersonAimControllerTrans
//!	Construction
//!
//------------------------------------------------------------------------------------------
ThirdPersonAimControllerTrans::ThirdPersonAimControllerTrans( CMovement* pobMovement, const ThirdPersonAimControllerTransDef& obDefinition )
:	MovementController( pobMovement ),
	m_obDefinition( obDefinition ),
	m_Fired( false ),
	m_RequestFire( false )
{
	// Register our definition for debugging purposes
	InternalRegisterDefinition( m_obDefinition );

	ThirdPersonAttackTransition* pTrans = m_obDefinition.m_pTransition;

	// Create our animation and add it to the animator
	m_obSingleAnimation = m_pobAnimator->CreateAnimation( CHashedString(pTrans->m_TransitionAnim) );
	m_obSingleAnimation->SetBlendWeight( 0.0f );
	m_obSingleAnimation->SetFlagBits( ANIMF_LOCOMOTING | ANIMF_INHIBIT_AUTO_DESTRUCT );
	m_obSingleAnimation->SetSpeed( pTrans->m_TransitionAnimSpeed );

	// Apply gravity if required
	ApplyGravity( true );

	((CEntity*)pobMovement->GetParentEntity())->ToPlayer()->ThirdPersonAttackTransitionStarted( m_obDefinition.m_pTransition );
}


//------------------------------------------------------------------------------------------
//!
//!	ThirdPersonAimControllerTrans::~ThirdPersonAimControllerTrans
//!	Destruction
//!
//------------------------------------------------------------------------------------------
ThirdPersonAimControllerTrans::~ThirdPersonAimControllerTrans( void )
{
	// Remove the animation from the animator if it has been added
	if ( !m_bFirstFrame )
		m_pobAnimator->RemoveAnimation( m_obSingleAnimation );
}


//------------------------------------------------------------------------------------------
//!
//!	ThirdPersonAimController::Update
//!	Simply move our way through the single animation
//!
//------------------------------------------------------------------------------------------
bool ThirdPersonAimControllerTrans::Update(	float						/*fTimeStep*/, 
											const CMovementInput&		/*obMovementInput*/,
											const CMovementStateRef&	/*obCurrentMovementState*/,
											CMovementState&				/*obPredictedMovementState*/ )
{
	// Sanity Check.
	ntAssert( m_obDefinition.m_pTransition );

	// Set the weight on our animation and update it
	m_obSingleAnimation->SetBlendWeight( m_fBlendWeight );

	if ( m_bFirstFrame )
	{
		// Add the single animation to the animator
		m_pobAnimator->AddAnimation( m_obSingleAnimation );

		m_bFirstFrame = false;
	}

	// If we have reached the end of our movement - indicate to the movement component
	if ( m_obSingleAnimation->GetTime() >= m_obSingleAnimation->GetDuration() )
	{
		return true;
	}

	// If we have reached the end of our movement - indicate to the movement component
	if ( m_obDefinition.m_pTransition->m_EarlyOutTime > 0.0f && m_obSingleAnimation->GetTime() >= m_obDefinition.m_pTransition->m_EarlyOutTime )
	{
		return true;
	}

	// If we are still here then we are still going
	return false;
}



//------------------------------------------------------------------------------------------
//!
//!	ThirdPersonAimControllerDef::ThirdPersonAimControllerDef
//!	Construction
//!
//------------------------------------------------------------------------------------------
ThirdPersonAimControllerDef::ThirdPersonAimControllerDef( void )
{
}


//------------------------------------------------------------------------------------------
//!
//!	ThirdPersonAimControllerDef::CreateInstance
//!	To create an instance of what we describe
//!
//------------------------------------------------------------------------------------------
MovementController* ThirdPersonAimControllerDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) ThirdPersonAimController( pobMovement, *this );
}


//------------------------------------------------------------------------------------------
//!
//!	ThirdPersonAimController::ThirdPersonAimController
//!	Construction
//!
//------------------------------------------------------------------------------------------
ThirdPersonAimController::ThirdPersonAimController( CMovement* pobMovement, const ThirdPersonAimControllerDef& obDefinition )
:	MovementController( pobMovement ),
	m_obDefinition( obDefinition ),
	m_StateTime( 0.0f ),
	m_FireTime( 0.0f )
{
	// Sanity check for a pointer setup. 
	ntAssert( obDefinition.m_pCurrentState );

	// Register our definition for debugging purposes
	InternalRegisterDefinition( m_obDefinition );

	// Set the animations to play during the course of the controller
	m_obAnims[IDLE_FRONT]		= m_pobAnimator->CreateAnimation( obDefinition.m_pCurrentState->m_IdleAnim );
	m_obAnims[IDLE_FRONT_UP]	= m_pobAnimator->CreateAnimation( obDefinition.m_pCurrentState->m_IdleUpAnim );
	m_obAnims[IDLE_LEFT]		= m_pobAnimator->CreateAnimation( obDefinition.m_pCurrentState->m_IdleLeftAim );
	m_obAnims[IDLE_LEFT_UP]		= m_pobAnimator->CreateAnimation( obDefinition.m_pCurrentState->m_IdleLeftUpAim );
	m_obAnims[IDLE_RIGHT]		= m_pobAnimator->CreateAnimation( obDefinition.m_pCurrentState->m_IdleRightAim );
	m_obAnims[IDLE_RIGHT_UP]	= m_pobAnimator->CreateAnimation( obDefinition.m_pCurrentState->m_IdleRightUpAim );
	m_obAnims[FIRE_FRONT]		= m_pobAnimator->CreateAnimation( obDefinition.m_pCurrentState->m_FireAnim );
	m_obAnims[FIRE_FRONT_UP]	= m_pobAnimator->CreateAnimation( obDefinition.m_pCurrentState->m_FireUpAnim );
	m_obAnims[FIRE_LEFT]		= m_pobAnimator->CreateAnimation( obDefinition.m_pCurrentState->m_FireLeftAim );
	m_obAnims[FIRE_LEFT_UP]		= m_pobAnimator->CreateAnimation( obDefinition.m_pCurrentState->m_FireLeftUpAim );
	m_obAnims[FIRE_RIGHT]		= m_pobAnimator->CreateAnimation( obDefinition.m_pCurrentState->m_FireRightAim );
	m_obAnims[FIRE_RIGHT_UP]	= m_pobAnimator->CreateAnimation( obDefinition.m_pCurrentState->m_FireRightUpAim );

	// Initialise common variables between all the animations, this should save some code space. 
	for( int iLoop = 0; iLoop < ANIMS_MAX; ++iLoop )
	{
		m_obAnims[iLoop]->SetFlagBits( ANIMF_LOCOMOTING | ANIMF_INHIBIT_AUTO_DESTRUCT | ANIMF_LOOPING );
		m_obAnims[iLoop]->SetBlendWeight( 0.0f );
		m_obAnims[iLoop]->SetSpeed( 1.0f );
	}

	// Mark the front idle animation weight to 100%
	m_obAnims[IDLE_FRONT]->SetBlendWeight( 1.0f );

	// Set up the reload anim
	m_ReloadAnim = m_pobAnimator->CreateAnimation( obDefinition.m_pCurrentState->m_ReloadAnim );
	m_ReloadAnim->SetFlagBits( ANIMF_LOCOMOTING | ANIMF_INHIBIT_AUTO_DESTRUCT );
	m_ReloadAnim->SetBlendWeight( 0.0f );
	m_ReloadAnim->SetSpeed( 1.0f );

	// Apply gravity if required
	ApplyGravity( true );


	// Cache a pointer to the parent entity
	CEntity* pEntity = (CEntity*) m_pobMovement->GetParentEntity();

	// Type safe pointer cast with the above checks. 
	Player* pPlayer = pEntity->ToPlayer();

	// Put the player into an aiming state
	pPlayer->SetThirdPersonAimingState( obDefinition.m_pCurrentState );

	// 
	CEntity* pRangedWeapon = pPlayer->GetRangedWeapon();

	// 
	if( pRangedWeapon && pRangedWeapon->GetMovement() && !obDefinition.m_pCurrentState->m_WeaponIdle.IsNull() )
	{
		SimpleTransitionDef obDef;
		
		obDef.m_bApplyGravity			= false;
		obDef.m_bLooping				= true;
		obDef.m_obAnimationName			= obDefinition.m_pCurrentState->m_WeaponIdle;
		obDef.m_fSpeed					= 1.0f;
		obDef.m_fTimeOffsetPercentage	= 0.0f;

		pRangedWeapon->GetMovement()->BringInNewController( obDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );
	}

}


//------------------------------------------------------------------------------------------
//!
//!	ThirdPersonAimController::~ThirdPersonAimController
//!	Destruction
//!
//------------------------------------------------------------------------------------------
ThirdPersonAimController::~ThirdPersonAimController( void )
{
	// Remove the animation from the animator if it has been added
	if ( !m_bFirstFrame  )
	{
		// Create our animation and add it to the animator
		for( int iLoop = 0; iLoop < ANIMS_MAX; ++iLoop )
		{
			if( m_obAnims[iLoop]->IsActive() )
			{
				m_pobAnimator->RemoveAnimation( m_obAnims[iLoop] );
			}
		}
	}

	// 
	if( m_ReloadAnim->IsActive() )
	{
		// Cache a pointer to the parent entity
		const CEntity* pEntity = m_pobMovement->GetParentEntity();

		// Sanity checks.
		ntAssert( pEntity && "Movement controller without a parent entity?");
		ntAssert( pEntity->IsPlayer() && "This controller can only run on a player");

		// Type safe pointer cast with the above checks. 
		const Player* pPlayer = static_cast<const Player*>(pEntity);

		// Reset the fire count
		pPlayer->ResetFiredCount();
		m_pobAnimator->RemoveAnimation( m_ReloadAnim );
	}
}


//-------------------------------------------------------------------------------------------
//!
//!	ThirdPersonAimController::Update
//!	Simply move our way through the single animation
//!
//------------------------------------------------------------------------------------------
bool ThirdPersonAimController::Update(	float fTimeStep, 
										const CMovementInput&		/*obMovementInput*/,
										const CMovementStateRef&	/*obCurrentMovementState*/,
										CMovementState&				/*obPredictedMovementState*/ )
{
	// Cache a pointer to the parent entity
	const CEntity* pEntity = m_pobMovement->GetParentEntity();

	// Sanity checks.
	ntAssert( pEntity && "Movement controller without a parent entity?");
	ntAssert( pEntity->IsPlayer() && "This controller can only run on a player");

	// Type safe pointer cast with the above checks. 
	const Player* pPlayer = pEntity->ToPlayer();

	// 
	float fBlendWeight = m_FireTime ? 0.0f : m_fBlendWeight;
	float fFireBlendWeight = m_FireTime ? m_fBlendWeight : 0.0f;
	float fReloadBlendWeight = 0.0f;

	// Allow the archer to blend in and out of her reload animations. 
	if( m_ReloadAnim->IsActive() )
	{
		float fReloadPhase = m_ReloadAnim->GetTime() / m_ReloadAnim->GetDuration();
		static const float PHASE1 = 0.3f;

		if( fReloadPhase < PHASE1 )
		{
			fReloadBlendWeight	= CMaths::SmoothStep( fReloadPhase / PHASE1 );
		}
		else
		{
			fReloadBlendWeight	= 1.0f - CMaths::SmoothStep( (fReloadPhase - PHASE1) / (1.0f - PHASE1) );
		}

		fBlendWeight		= 1.0f - fReloadBlendWeight;
		fFireBlendWeight	= 0.0f;
	}

	// Sanity check
	ntAssert( m_obDefinition.m_pCurrentState );

	// If this is the first frame, then perform some once only initialisation
	if ( m_bFirstFrame )
	{
		m_bFirstFrame = false;

		// Add the animations to the animattor on the first update of the controller
		m_pobAnimator->AddAnimation( m_obAnims[IDLE_FRONT] );
		m_pobAnimator->AddAnimation( m_obAnims[IDLE_FRONT_UP] );
		m_pobAnimator->AddAnimation( m_obAnims[IDLE_LEFT] );
		m_pobAnimator->AddAnimation( m_obAnims[IDLE_LEFT_UP] );
		m_pobAnimator->AddAnimation( m_obAnims[IDLE_RIGHT] );
		m_pobAnimator->AddAnimation( m_obAnims[IDLE_RIGHT_UP] );
	}

	CPoint ptPlayer = pPlayer->GetPosition();

	// Get the angle from
	float fAngle = pPlayer->GetCurrentRotationAngle();

	// Create a temperary vertical angle variable
	//float fVAngle = obMovementInput.m_obMoveDirectionAlt.Z() * m_obDefinition.m_pCurrentState->m_VerticalAngleLimit;

	// Clip the angle then normalise it
	float fVWeight = 0.0f; // min( fVAngle, m_obDefinition.m_pCurrentState->m_VerticalAngleLimit ) / m_obDefinition.m_pCurrentState->m_VerticalAngleLimit;

	// Calculate a directional blend weight. 
	float fDirectionBlend = fAngle / m_obDefinition.m_pCurrentState->m_AngleRange;

	// If the value is positive, then turn left, else, turn right
	if( fDirectionBlend > 0.0f )
	{
		m_obAnims[IDLE_FRONT]->SetBlendWeight( (1.0f - fDirectionBlend) * fBlendWeight * (1.0f - fVWeight) );
		m_obAnims[IDLE_LEFT]->SetBlendWeight( fDirectionBlend * fBlendWeight * (1.0f - fVWeight) );
		m_obAnims[IDLE_RIGHT]->SetBlendWeight( 0.0f * fBlendWeight );
		m_obAnims[IDLE_FRONT_UP]->SetBlendWeight( (1.0f - fDirectionBlend) * fVWeight * fBlendWeight );
		m_obAnims[IDLE_LEFT_UP]->SetBlendWeight( fDirectionBlend * fVWeight * fBlendWeight );
		m_obAnims[IDLE_RIGHT_UP]->SetBlendWeight( 0.0f * fBlendWeight );

		if( fFireBlendWeight > 0.0f )
		{
			m_obAnims[FIRE_FRONT]->SetBlendWeight( (1.0f - fDirectionBlend) * fFireBlendWeight * (1.0f - fVWeight) );
			m_obAnims[FIRE_LEFT]->SetBlendWeight( fDirectionBlend * fFireBlendWeight * (1.0f - fVWeight) );
			m_obAnims[FIRE_RIGHT]->SetBlendWeight( 0.0f * fBlendWeight );
			m_obAnims[FIRE_FRONT_UP]->SetBlendWeight( (1.0f - fDirectionBlend) * fVWeight * fFireBlendWeight );
			m_obAnims[FIRE_LEFT_UP]->SetBlendWeight( fDirectionBlend * fVWeight * fFireBlendWeight );
			m_obAnims[FIRE_RIGHT_UP]->SetBlendWeight( 0.0f * fFireBlendWeight );
		}
	}
	else
	{
		m_obAnims[IDLE_FRONT]->SetBlendWeight( (1.0f + fDirectionBlend) * fBlendWeight * (1.0f - fVWeight) );
		m_obAnims[IDLE_LEFT]->SetBlendWeight( 0.0f * fBlendWeight);
		m_obAnims[IDLE_RIGHT]->SetBlendWeight( (-fDirectionBlend) * fBlendWeight * (1.0f - fVWeight) );
		m_obAnims[IDLE_FRONT_UP]->SetBlendWeight( (1.0f + fDirectionBlend) * fVWeight * fBlendWeight );
		m_obAnims[IDLE_LEFT_UP]->SetBlendWeight( 0.0f * fBlendWeight);
		m_obAnims[IDLE_RIGHT_UP]->SetBlendWeight( (-fDirectionBlend) * fVWeight * fBlendWeight);

		if( fFireBlendWeight > 0.0f )
		{
			m_obAnims[FIRE_FRONT]->SetBlendWeight( (1.0f + fDirectionBlend) * fFireBlendWeight * (1.0f - fVWeight) );
			m_obAnims[FIRE_LEFT]->SetBlendWeight( 0.0f * fFireBlendWeight);
			m_obAnims[FIRE_RIGHT]->SetBlendWeight( (-fDirectionBlend) * fFireBlendWeight * (1.0f - fVWeight) );
			m_obAnims[FIRE_FRONT_UP]->SetBlendWeight( (1.0f + fDirectionBlend) * fVWeight * fFireBlendWeight );
			m_obAnims[FIRE_LEFT_UP]->SetBlendWeight( 0.0f * fFireBlendWeight);
			m_obAnims[FIRE_RIGHT_UP]->SetBlendWeight( (-fDirectionBlend) * fVWeight * fFireBlendWeight);
		}
	}

	// Update the active time for this state. 
	m_StateTime += fTimeStep;

	// If the firing timer is not zero, then there's some fire action happening. 
	if( m_FireTime != 0.0f )
	{
		float fFireDuration = m_obAnims[FIRE_FRONT]->GetDuration();
		
		if( m_FireTime >= fFireDuration )
		{
			m_pobAnimator->RemoveAnimation( m_obAnims[FIRE_FRONT] );
			m_pobAnimator->RemoveAnimation( m_obAnims[FIRE_LEFT] );
			m_pobAnimator->RemoveAnimation( m_obAnims[FIRE_RIGHT] );
			m_pobAnimator->RemoveAnimation( m_obAnims[FIRE_FRONT_UP] );
			m_pobAnimator->RemoveAnimation( m_obAnims[FIRE_LEFT_UP] );
			m_pobAnimator->RemoveAnimation( m_obAnims[FIRE_RIGHT_UP] );

			m_FireTime = 0.0f;
			pPlayer->FireRequestCompleted();

			if( !pPlayer->DisableFireFeedback() )
			{
				// Send a fire complete msg
				Message obMsg(msg_movementdone);
				obMsg.SetEnt("Sender", pPlayer);
				pPlayer->GetMessageHandler()->Receive( obMsg );
			}

			pPlayer->DisableFireFeedback(false);
		}
		else
		{
			m_FireTime += fTimeStep;
		}
	}

	// Is there a request for reload?
	if( (pPlayer->RequestReload() && !m_obAnims[FIRE_FRONT]->IsActive()) || m_ReloadAnim->IsActive() )
	{
		m_ReloadAnim->SetBlendWeight( fReloadBlendWeight );

		// If the animation hasn't started, then kick it off
		if( !m_ReloadAnim->IsActive() )
		{
			if( pPlayer->IsArcher() )
			{
				pPlayer->ToArcher()->LogEvent(AE_RELOAD); 
			}

			m_pobAnimator->AddAnimation( m_ReloadAnim );

			// Use the assigned fire animation for the weapon 
			CEntity* pRangedWeapon = pPlayer->GetRangedWeapon();

			// Only reload the weapon if there is an animation and a weapon 
			if( pRangedWeapon && pRangedWeapon->GetMovement() && !m_obDefinition.m_pCurrentState->m_WeaponReload.IsNull() )
			{
				SimpleTransitionDef obDef;
				
				obDef.m_bApplyGravity			= false;
				obDef.m_bLooping				= false;
				obDef.m_obAnimationName			= m_obDefinition.m_pCurrentState->m_WeaponReload;
				obDef.m_fSpeed					= 1.0f;
				obDef.m_fTimeOffsetPercentage	= 0.0f;

				pRangedWeapon->GetMovement()->BringInNewController( obDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );

				// Once the reload animation has completed put the weapon back into an idle state
				SimpleTransitionDef obDefChained;
				
				obDefChained.m_bApplyGravity			= false;
				obDefChained.m_bLooping					= true;
				obDefChained.m_obAnimationName			= m_obDefinition.m_pCurrentState->m_WeaponIdle;
				obDefChained.m_fSpeed					= 1.0f;
				obDefChained.m_fTimeOffsetPercentage	= 0.0f;

				pRangedWeapon->GetMovement()->AddChainedController( obDefChained, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );
			}

		}
		// wait until the animtion is completed. 
		else if( m_ReloadAnim->GetTime() >= m_ReloadAnim->GetDuration())
		{
			pPlayer->ResetFiredCount();
			m_ReloadAnim->SetTime( 0.0f );
			m_ReloadAnim->SetBlendWeight( 0.0f );
			m_pobAnimator->RemoveAnimation( m_ReloadAnim );

			// Send a fire complete msg
			Message obMsg(msg_reload_complete);
			obMsg.SetEnt("Sender", pPlayer);
			pPlayer->GetMessageHandler()->Receive( obMsg, 0.5f );
			pPlayer->DisableFireFeedback( false );
		}
	}
	// Is there a fired request is pending. 
	else if( pPlayer->RequestFire() && !m_ReloadAnim->IsActive() && m_FireTime <= 0.0f )
	{
		// Increase the number of arrows fired. 
		pPlayer->IncFiredCount();

		// Send a fire bow message!
		Message obMsg(msg_combat_fire);
		obMsg.SetEnt("Sender", pPlayer);
		obMsg.SetEnt("target", pPlayer->GetLockOnTarget());

		pPlayer->GetMessageHandler()->Receive( obMsg );

		if( m_obAnims[FIRE_FRONT]->IsActive() )
		{
			m_obAnims[FIRE_FRONT]->SetTime( 0.0f );
			m_obAnims[FIRE_FRONT_UP]->SetTime( 0.0f );
			m_obAnims[FIRE_LEFT]->SetTime( 0.0f );
			m_obAnims[FIRE_LEFT_UP]->SetTime( 0.0f );
			m_obAnims[FIRE_RIGHT]->SetTime( 0.0f );
			m_obAnims[FIRE_RIGHT_UP]->SetTime( 0.0f );
		}
		else
		{
			m_pobAnimator->AddAnimation( m_obAnims[FIRE_FRONT] );	
			m_pobAnimator->AddAnimation( m_obAnims[FIRE_FRONT_UP] );
			m_pobAnimator->AddAnimation( m_obAnims[FIRE_LEFT] );
			m_pobAnimator->AddAnimation( m_obAnims[FIRE_LEFT_UP] );
			m_pobAnimator->AddAnimation( m_obAnims[FIRE_RIGHT] );
			m_pobAnimator->AddAnimation( m_obAnims[FIRE_RIGHT_UP] );
		}


		// Use the assigned fire animation for the weapon 
		CEntity* pRangedWeapon = pPlayer->GetRangedWeapon();

		// Only fire the weapon if there is an animation and a weapon 
		if( pRangedWeapon && pRangedWeapon->GetMovement() && !m_obDefinition.m_pCurrentState->m_WeaponFire.IsNull() )
		{
			SimpleTransitionDef obDef;
			
			obDef.m_bApplyGravity			= false;
			obDef.m_bLooping				= false;
			obDef.m_obAnimationName			= m_obDefinition.m_pCurrentState->m_WeaponFire;
			obDef.m_fSpeed					= 1.0f;
			obDef.m_fTimeOffsetPercentage	= 0.0f;

			pRangedWeapon->GetMovement()->BringInNewController( obDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );


			// Once the fire animation has completed put the weapon back into an idle state
			SimpleTransitionDef obDefChained;
			
			obDefChained.m_bApplyGravity			= false;
			obDefChained.m_bLooping					= true;
			obDefChained.m_obAnimationName			= m_obDefinition.m_pCurrentState->m_WeaponIdle;
			obDefChained.m_fSpeed					= 1.0f;
			obDefChained.m_fTimeOffsetPercentage	= 0.0f;

			pRangedWeapon->GetMovement()->AddChainedController( obDefChained, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );
		}

		m_FireTime = 0.0f;
		m_FireTime += fTimeStep;
	}

	/*
	if( CHashedString( g_ShellOptions->m_dbgStartLevel ) == CHashedString("kits_den/kits_den") )
	{
		CEntityQuery obQuery;
		CEQCHealthLTE obHealth(0.0f);
		obQuery.AddUnClause( obHealth );

		CEntityManager::Get().FindEntitiesByType( obQuery, CEntity::EntType_AI );

		// 
		if( obQuery.GetResults().size() < 15 )
		{
			while( obQuery.GetResults().size() < 25 )
			{
				NinjaLua::LuaObject table;
				table.AssignNewTable( CLuaGlobal::Get().State() );

				static int EntityNumber = 1;
				char strNumber[64];
				sprintf( strNumber, "TestEntity%d", EntityNumber++ );

				table.Set( "Name"				 , strNumber );
				table.Set( "Class"				 , "EnemyAI" );
				table.Set( "IsEnemy"             , 1 );
				table.Set( "ConstructionScript"  , "AI_Construct" );
				table.Set( "Description"         , "enemy,shieldsman" );
				table.Set( "AIDefinition"        , "ShieldsmanAIDef" );
				table.Set( "AIDifficulty"        , 0.5f );
				table.Set( "AIAggression"        , 0.5f );
				table.Set( "AIType"              , "shieldsman" );
				table.Set( "SceneElementDef"     , "DefaultAISceneElement" );
				table.Set( "StateSystem"         , "AI_State" );
				table.Set( "Controller"          , "Formation_Controller" );
				table.Set( "InitialState"        , "CombatTestState" );
				table.Set( "DestructionScript"   , "" );
				table.Set( "Position"            , "10,0.5,0" );
				table.Set( "Orientation"         , "0,0,0,1" );
				table.Set( "ParentEntity"        , "" );
				table.Set( "ParentTransform"     , "ROOT" );
				table.Set( "Clump"               , "entities/characters/shieldman/shieldman.clump" );
				table.Set( "DefaultDynamics"     , "" );
				table.Set( "HeldPosition"        , "0,0,0" );
				table.Set( "HeldOrientation"     , "0,0,0,1" );
				table.Set( "HoldingPosition"     , "0,0,0" );
				table.Set( "HoldingOrientation"  , "0,0,0,1" );
				table.Set( "WeaponConstruction"  , "Sword_Shield_Create" );
				table.Set( "CombatDefinition"    , "shieldman_AttackDefinition" );
				table.Set( "AwarenessDefinition" , "shieldman_AttackTargetingData" );
				table.Set( "AnimationContainer"  , "ShieldmanAnimContainer" );
				table.Set( "AIControlled"        , 0 );
				table.Set( "CollisionHeight"     , 1.6f );
				table.Set( "CollisionRadius"     , 0.35f );
				table.Set( "Health"              , 10 );
				table.Set( "HairConstruction"    , "" );
				table.Set( "CastShadows"         , 1 );
				table.Set( "RecieveShadows"      , 1 );
				table.Set( "DisableRender"       , 0 );
				table.Set( "SectorBits"          , -1 );

				extern void CreateEntityFromLuaAttributeTable( LuaAttributeTable* );
				LuaAttributeTable attrib_table( table ); 
				
				CreateEntityFromLuaAttributeTable( &attrib_table );

				obQuery.GetResults().clear();
				CEntityManager::Get().FindEntitiesByType( obQuery, CEntity::EntType_AI );
			}
		}
	}
	*/

	// If we are still here then we are still going
	return false;
}


