//------------------------------------------------------------------------------------------
//!
//!	\file archerwalkruncontroller.cpp
//!
//------------------------------------------------------------------------------------------

#include "archerwalkruncontroller.h"

#include "ai/ainavigationsystem/ainavigsystemman.h"
#include "anim/animator.h"
#include "game/movement.h"
#include "game/inputcomponent.h"
#include "game/awareness.h"
#include "game/messagehandler.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "game/staticentity.h"
#include "game/entityarcher.h"

#include "core/exportstruct_anim.h"
#include "core/osddisplay.h"					// For debugging.

#include "objectdatabase/dataobject.h"

// For testing, it's nice to be able to make the Archer crouch at will.
#ifndef _RELEASE
//#	define PUT_CROUCH_ON_KEY
#	define CROUCH_KEY	( 1 << AB_PSTANCE )
#endif

//------------------------------------------------------------------------------------------
//!
//!	XML Interface for the Archer walk/run controller.
//!
//------------------------------------------------------------------------------------------
START_STD_INTERFACE				( ArcherWalkRunControllerDef )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_HasCrouchedAnimSet, false,	HasCrouchedAnimSet )
	IREFERENCE					( ArcherWalkRunControllerDef, 	AnimSet)
	IFLOAT						( ArcherWalkRunControllerDef, 	DeadZoneMaximum)
	IFLOAT						( ArcherWalkRunControllerDef, 	WalkZoneAnalogue)
	IFLOAT						( ArcherWalkRunControllerDef, 	WalkZonePlateau)
	IFLOAT						( ArcherWalkRunControllerDef, 	MaxAwareDistance)
	IFLOAT						( ArcherWalkRunControllerDef, 	MaxTurnSpeed)
	IFLOAT						( ArcherWalkRunControllerDef, 	RunAnimMaxSpeed)
	IFLOAT						( ArcherWalkRunControllerDef, 	RunAccelerationTime )
#	ifndef _RELEASE
		DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
#	endif
END_STD_INTERFACE

//------------------------------------------------------------------------------------------
//!
//!	Archer movement controller specific objects.
//!
//------------------------------------------------------------------------------------------
namespace ArcherMovement
{
	bool UpdateArcherMovementState( const CEntity *archer, CMovement *movement, bool currently_crouched );
}

//------------------------------------------------------------------------------------------
//!
//!	ArcherWalkRunControllerDef::CreateInstance
//!	To create an instance of what we describe
//!
//------------------------------------------------------------------------------------------
MovementController *ArcherWalkRunControllerDef::CreateInstance( CMovement *movement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) ArcherWalkRunController( movement, *this );
}

//------------------------------------------------------------------------------------------
//!
//!	ArcherWalkRunController::ArcherWalkRunController
//! Construction
//!
//------------------------------------------------------------------------------------------
ArcherWalkRunController::ArcherWalkRunController(	CMovement*							movement,
													const ArcherWalkRunControllerDef &	definition )
:	WalkRunController	( movement, definition )
,	m_HasCrouchedAnimSet( definition.m_HasCrouchedAnimSet )
,	m_OkToStartNewController( true )
,	m_bFirstFrame( true )
{
}

//------------------------------------------------------------------------------------------
//!
//!	ArcherWalkRunController::ArcherWalkRunController
//! Construction
//!
//------------------------------------------------------------------------------------------
ArcherWalkRunController::~ArcherWalkRunController()
{
	const CEntity* pEnt = m_pobMovement->GetParentEntity();
	ntAssert( pEnt && pEnt->IsPlayer() && pEnt->ToPlayer()->IsArcher() );

	const Player* pPlayer = pEnt->ToPlayer();
	UNUSED( pPlayer );
}

//------------------------------------------------------------------------------------------
//!
//!	bool ArcherWalkRunController::Update
//! Main update function.
//!
//------------------------------------------------------------------------------------------
bool ArcherWalkRunController::Update(	float						fTimeStep, 
										const CMovementInput&		obMovementInput,
										const CMovementStateRef&	obCurrentMovementState,
										CMovementState&				obPredictedMovementState)
{
	ntError( m_pobMovement != NULL );
	ntError( m_pobMovement->GetParentEntity() != NULL );

	// The archer is a free flowing character, when she enters her walk controller it is likely that
	// she'll want to keep her momentum
	if( m_bFirstFrame )
	{
		CMovementStateRef& obMovState = const_cast<CMovementStateRef&>(obCurrentMovementState);
		obMovState.m_obLastRequestedVelocity = obCurrentMovementState.m_obFacing * m_fRunSpeed;

		m_bFirstFrame = false;
	}

	if ( m_OkToStartNewController )
	{
		m_OkToStartNewController = ArcherMovement::UpdateArcherMovementState( m_pobMovement->GetParentEntity(), m_pobMovement, m_HasCrouchedAnimSet );
		return WalkRunController::Update( fTimeStep, obMovementInput, obCurrentMovementState, obPredictedMovementState );
	}

	WalkRunController::Update( fTimeStep, obMovementInput, obCurrentMovementState, obPredictedMovementState );
	return true;
}

//------------------------------------------------------------------------------------------
//!
//!	void ArcherMovement::UpdateArcherMovementState
//! Main update function.
//!
//------------------------------------------------------------------------------------------
bool ArcherMovement::UpdateArcherMovementState( const CEntity *archer, CMovement *movement, bool currently_crouched )
{
	using namespace ArcherMovement;

	ntError( archer->GetAwarenessComponent() != NULL );

	//
	bool can_crouch = false;

	const CEntity* pAwareOfEnemy = archer->GetAwarenessComponent()->IsAwareOfEnemies();

	const Archer* pArcher = archer->ToPlayer()->ToArcher();
	can_crouch = pArcher->CanCrouch() && (can_crouch || pArcher->AlwaysCrouch());

	if ( pAwareOfEnemy && (pAwareOfEnemy->GetPosition() - archer->GetPosition()).LengthSquared() > 25.0f && !pArcher->AlwaysCrouch() )
	{
		can_crouch = CAINavigationSystemMan::Get().CloseToVaultingVolume( archer->GetPosition(), Archer::m_sCrouchActivatingRange ) != NULL;
	}
	
	if ( can_crouch )
	{
		if ( !currently_crouched )
		{
			// We can crouch by this object and we're not currently doing so.... so start crouching.
			MovementControllerDef *crouched_walk_run_def = ObjectDatabase::Get().GetPointerFromName<MovementControllerDef*>( "CrouchedArcherWalkRun" );
			ntError( crouched_walk_run_def != NULL );
			movement->AddChainedController( *crouched_walk_run_def, CMovement::DMM_STANDARD, 0.25f );
			pArcher->UseSmallCapsule( true );
			//Flag on the archer that we're now going into crouched state.
			Archer* pArcherNonConst = const_cast<Archer*>(pArcher);
			pArcherNonConst->SetCrouching(true);
			return false;
		}
	}
	else
	{
		if ( currently_crouched )
		{
			// We're not near any objects that we can crouch by and we're currently crouching, so make us stand.
			MovementControllerDef *stand_walk_run_def = ObjectDatabase::Get().GetPointerFromName<MovementControllerDef*>( "ArcherWalkRun" );
			ntError( stand_walk_run_def != NULL );
			movement->AddChainedController( *stand_walk_run_def, CMovement::DMM_STANDARD, 0.25f );
			pArcher->UseSmallCapsule( false );
			//Flag on the archer that we're now coming out of crouched state.
			Archer* pArcherNonConst = const_cast<Archer*>(pArcher);
			pArcherNonConst->SetCrouching(false);
			return false;
		}
	}

	return true;
}


