//------------------------------------------------------------------------------------------
//!
//!	\file walkingcontroller.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "walkingcontroller.h"
#include "movement.h"
#include "inputcomponent.h"
#include "anim/animator.h"
#include "core/exportstruct_anim.h"
#include "physics/world.h" // find out what we're walking on or into
#include "objectdatabase/dataobject.h"
#include "game/messagehandler.h"
#include "game/entity.h"
#include "game/entity.inl"

#include "game/attacks.h"

//For debugging
#include "core/osddisplay.h"
#include "anim/animloader.h"

START_STD_INTERFACE(WalkingAnimSet)
	PUBLISH_VAR_AS( m_obStanding, Standing )
	PUBLISH_VAR_AS( m_obToAware, ToAware )
	PUBLISH_VAR_AS( m_obAwareMin, AwareMin )
	PUBLISH_VAR_AS( m_obAwareMax, AwareMax )
	PUBLISH_VAR_AS( m_obFromAware, FromAware )
	PUBLISH_VAR_AS( m_obStandToSlowWalk, StandToSlowWalk )
	PUBLISH_VAR_AS( m_obStandToSlowWalkLeft, StandToSlowWalkLeft )
	PUBLISH_VAR_AS( m_obStandToSlowWalkRight, StandToSlowWalkRight )
	PUBLISH_VAR_AS( m_obStandToSlowWalkBackLeft, StandToSlowWalkBackLeft )
	PUBLISH_VAR_AS( m_obStandToSlowWalkBackRight, StandToSlowWalkBackRight )
	PUBLISH_VAR_AS( m_obStandToWalk, StandToWalk )
	PUBLISH_VAR_AS( m_obStandToWalkLeft, StandToWalkLeft )
	PUBLISH_VAR_AS( m_obStandToWalkRight, StandToWalkRight )
	PUBLISH_VAR_AS( m_obStandToWalkBackLeft, StandToWalkBackLeft )
	PUBLISH_VAR_AS( m_obStandToWalkBackRight, StandToWalkBackRight )
	PUBLISH_VAR_AS( m_obStandToAccel, StandToAccel )
	PUBLISH_VAR_AS( m_obStandToAccelLeft, StandToAccelLeft )
	PUBLISH_VAR_AS( m_obStandToAccelRight, StandToAccelRight )
	PUBLISH_VAR_AS( m_obStandToAccelBackLeft, StandToAccelBackLeft )
	PUBLISH_VAR_AS( m_obStandToAccelBackRight, StandToAccelBackRight )
	PUBLISH_VAR_AS( m_obSlowWalkToStandLeft, SlowWalkToStandLeft )
	PUBLISH_VAR_AS( m_obSlowWalkToStandRight, SlowWalkToStandRight )
	PUBLISH_VAR_AS( m_obWalkToStandLeft, WalkToStandLeft )
	PUBLISH_VAR_AS( m_obWalkToStandRight, WalkToStandRight )
	PUBLISH_VAR_AS( m_obRunToStandLeft, RunToStandLeft )
	PUBLISH_VAR_AS( m_obRunToStandRight, RunToStandRight )
	PUBLISH_VAR_AS( m_obSlowWalk, SlowWalk )
	PUBLISH_VAR_AS( m_obSlowWalkLeft, SlowWalkLeft )
	PUBLISH_VAR_AS( m_obSlowWalkRight, SlowWalkRight )
	PUBLISH_VAR_AS( m_obWalk, Walk )
	PUBLISH_VAR_AS( m_obWalkLeft, WalkLeft )
	PUBLISH_VAR_AS( m_obWalkRight, WalkRight )
	PUBLISH_VAR_AS( m_obRun, Run )
	PUBLISH_VAR_AS( m_obRunLeft, RunLeft )
	PUBLISH_VAR_AS( m_obRunRight, RunRight )
	PUBLISH_VAR_AS( m_obAccelRun, AccelRun )
	PUBLISH_VAR_AS( m_obAccelRunLeft, AccelRunLeft )
	PUBLISH_VAR_AS( m_obAccelRunRight, AccelRunRight )
	PUBLISH_VAR_AS( m_obFullTurnRight, FullTurnRight )
	PUBLISH_VAR_AS( m_obFullTurnLeft, FullTurnLeft )
	PUBLISH_VAR_AS( m_obBlink, Blink )
END_STD_INTERFACE

START_STD_INTERFACE	(WalkRunControllerDef)
	IREFERENCE	(WalkRunControllerDef, AnimSet)
	IFLOAT		(WalkRunControllerDef, DeadZoneMaximum)
	IFLOAT		(WalkRunControllerDef, WalkZoneAnalogue)
	IFLOAT		(WalkRunControllerDef, WalkZonePlateau)
	IFLOAT		(WalkRunControllerDef, RunZoneAnalogue)
	IFLOAT		(WalkRunControllerDef, MaxAwareDistance)
	IFLOAT		(WalkRunControllerDef, MaxTurnSpeed)
	IFLOAT		(WalkRunControllerDef, RunAnimMinSpeed)
	IFLOAT		(WalkRunControllerDef, RunAnimMaxSpeed)
	IFLOAT		(WalkRunControllerDef, RunAccelerationTime)
	IFLOAT		(WalkRunControllerDef, BlinkInterval)
#ifndef _RELEASE
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
#endif
END_STD_INTERFACE


const float WalkRunController::fAI_ROTATE_SPEED = 0.19f;

static const float BLINK_RANDOMNESS = 0.5f;
static const float BLINK_INTERVAL_MULTIPLIER = BLINK_RANDOMNESS / RAND_MAX;

//------------------------------------------------------------------------------------------
//!
//!	WalkRunControllerDef::WalkRunControllerDef
//! Construction
//!	Attaches the XML Interface
//!
//------------------------------------------------------------------------------------------
WalkRunControllerDef::WalkRunControllerDef( void )
:	m_pobAnimSet( 0 ),
	m_fDeadZoneMaximum( 0.2f ),  
	m_fWalkZoneAnalogue( 0.45f ),
	m_fWalkZonePlateau( 0.7f ),
	m_fRunZoneAnalogue(0.7f),
	m_fMaxAwareDistance( 15.0f ),
	m_fMaxTurnSpeed( 400.0f ),
	m_fRunAnimMinSpeed( 1.0f ),
	m_fRunAnimMaxSpeed( 1.0f ),
	m_fRunAccelerationTime( 0.5f ),
	m_fBlinkInterval( 3.5f )
{
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunControllerDef::CreateInstance
//!	To create an instance of what we describe
//!
//------------------------------------------------------------------------------------------
MovementController* WalkRunControllerDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) WalkRunController( pobMovement, *this );
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::WalkRunController
//! Construction
//!
//------------------------------------------------------------------------------------------
WalkRunController::WalkRunController(	CMovement*					pobMovement,
										const WalkRunControllerDef&	obDefinition )
:	MovementController( pobMovement ),
	m_obDefinition( obDefinition ),
	m_bInitialised( false ),
	m_fAnimTurnAmount( 0.0f ),
	m_fLastAnimTurnAmount( 0.0f ),
	m_eMovementState( MS_COUNT ),
	m_bCanBeAware( false ),
	m_bCanRotateOnSpot( false ),
	m_bCanSlowWalk( false ),
	m_bCanWalk( false ),
	m_bCanRun( false ),
	m_bCanTurn( false ),
	m_bCanAccelerate( false ),
	m_bCanFastTurn( false ),
	m_obTargetPoint( CONSTRUCT_CLEAR ),
	m_obLastRequestedDirection( CONSTRUCT_CLEAR ),
	m_bTargetPointSet( false ),
	m_fRequestedDirectionChange( 0.0f ),
	m_fCurrentSpeed( 0.0f ),
	m_fTurnAmount( 0.0f ),
	m_fRequestedSpeed( 0.0f ),
	m_fPreviousPhase( 0.0f ),
	m_fBlendInTime( CMovement::MOVEMENT_BLEND ),
	m_fSlowWalkSpeed( 0.0f ),
	m_fWalkSpeed( 0.0f ),
	m_fRunSpeed( 0.0f ),
	m_fStateTime( 0.0f ),
	m_bCanBlink( false ),
	m_fTimeSinceLastBlink( 0.0f ),
	m_fAwarenessWeight( 0.0f ),
	m_bAIRequestedRotation( false ),
	m_eChosenStartSlowWalk( STA_COUNT ),
	m_eChosenStartWalk( STA_COUNT ),
	m_eChosenStartRun( STA_COUNT ),
	m_fRunStartWeight( 0.0f ),
	m_fWalkStartWeight( 0.0f ),
	m_fSlowWalkStartWeight( 0.0f ),
	m_obStartFacingDirection( CONSTRUCT_CLEAR ),
	m_eChosenStop( SPA_COUNT ),
	m_fTurningAppearance(),
	m_fAccelerationAppearance(),
	m_bQuickTurn( false ),
	m_fTurnClampDuration( 0.0f ),
	m_eChosenTurn( TUA_COUNT )
{
	// Register our definition for debugging purposes
	InternalRegisterDefinition( m_obDefinition );

	if (m_obDefinition.m_fWalkZoneAnalogue < m_obDefinition.m_fDeadZoneMaximum)
		m_obDefinition.m_fWalkZoneAnalogue = m_obDefinition.m_fDeadZoneMaximum;

	if (m_obDefinition.m_fWalkZonePlateau < m_obDefinition.m_fWalkZoneAnalogue)
		m_obDefinition.m_fWalkZonePlateau = m_obDefinition.m_fWalkZoneAnalogue;

	if (m_obDefinition.m_fRunZoneAnalogue < m_obDefinition.m_fWalkZonePlateau)
		m_obDefinition.m_fRunZoneAnalogue = m_obDefinition.m_fWalkZonePlateau;

	// Initialise the state weights
	for ( int iState = 0; iState < MS_COUNT; ++iState )
		m_fStateWeights[iState] = 0.0f;

	// Here we are going to analyse the animations that the definition holds to see which
	// bits of functionality the walk run controller should be used with this anim set

	// Check the animation set required to be aware
	m_bCanBeAware = (	!m_obDefinition.m_pobAnimSet->m_obAwareMin.IsNull() &&
						!m_obDefinition.m_pobAnimSet->m_obAwareMax.IsNull() &&
						!m_obDefinition.m_pobAnimSet->m_obToAware.IsNull() &&
						!m_obDefinition.m_pobAnimSet->m_obFromAware.IsNull() );

	// TODO: MARTIN, Only AI's can rotate on spot, so use idle animation
	m_bCanRotateOnSpot = true;
	//m_bCanRotateOnSpot = (	!m_obDefinition.m_pobAnimSet->m_obStandToSlowWalkBackLeft.IsNull() &&
	//						!m_obDefinition.m_pobAnimSet->m_obStandToSlowWalkBackRight.IsNull() );

	// Check the basic animation set required to slow walk
	m_bCanSlowWalk = (	!m_obDefinition.m_pobAnimSet->m_obStandToSlowWalk.IsNull() &&
						!m_obDefinition.m_pobAnimSet->m_obStandToSlowWalkLeft.IsNull() &&
						!m_obDefinition.m_pobAnimSet->m_obStandToSlowWalkRight.IsNull() &&
						!m_obDefinition.m_pobAnimSet->m_obStandToSlowWalkBackLeft.IsNull() &&
						!m_obDefinition.m_pobAnimSet->m_obStandToSlowWalkBackRight.IsNull() &&
						!m_obDefinition.m_pobAnimSet->m_obSlowWalkToStandLeft.IsNull() &&
						!m_obDefinition.m_pobAnimSet->m_obSlowWalkToStandRight.IsNull() &&
						!m_obDefinition.m_pobAnimSet->m_obSlowWalk.IsNull() );
					
	// Check the basic animation set required for walking
	m_bCanWalk =	(	!m_obDefinition.m_pobAnimSet->m_obStandToWalk.IsNull() &&
						!m_obDefinition.m_pobAnimSet->m_obStandToWalkLeft.IsNull() &&
						!m_obDefinition.m_pobAnimSet->m_obStandToWalkRight.IsNull() &&
						!m_obDefinition.m_pobAnimSet->m_obStandToWalkBackLeft.IsNull() &&
						!m_obDefinition.m_pobAnimSet->m_obStandToWalkBackRight.IsNull() &&
						!m_obDefinition.m_pobAnimSet->m_obWalkToStandLeft.IsNull() &&
						!m_obDefinition.m_pobAnimSet->m_obWalkToStandRight.IsNull() &&
						!m_obDefinition.m_pobAnimSet->m_obWalk.IsNull() );

	// Check the basic animation set required for running
	m_bCanRun = (	!m_obDefinition.m_pobAnimSet->m_obStandToAccel.IsNull() &&
					!m_obDefinition.m_pobAnimSet->m_obStandToAccelLeft.IsNull() &&
					!m_obDefinition.m_pobAnimSet->m_obStandToAccelRight.IsNull() &&
					!m_obDefinition.m_pobAnimSet->m_obStandToAccelBackLeft.IsNull() &&
					!m_obDefinition.m_pobAnimSet->m_obStandToAccelBackRight.IsNull() &&
					!m_obDefinition.m_pobAnimSet->m_obRunToStandLeft.IsNull() &&
					!m_obDefinition.m_pobAnimSet->m_obRunToStandRight.IsNull() &&
					!m_obDefinition.m_pobAnimSet->m_obRun.IsNull() );

	// Check the basic animation set required to accelerate
	m_bCanAccelerate = (	m_bCanRun &&
							m_bCanWalk &&
							!m_obDefinition.m_pobAnimSet->m_obAccelRun.IsNull() &&
							!m_obDefinition.m_pobAnimSet->m_obAccelRunLeft.IsNull() &&
							!m_obDefinition.m_pobAnimSet->m_obAccelRunRight.IsNull() );
	
	// Check whether we can turn for all the speed set we have
	m_bCanTurn = (	( ( !m_bCanSlowWalk )
					  ||
					  (	!m_obDefinition.m_pobAnimSet->m_obSlowWalkLeft.IsNull() &&
						!m_obDefinition.m_pobAnimSet->m_obSlowWalkRight.IsNull() ) )
					&&
					( ( !m_bCanWalk )
					  ||
					  (	!m_obDefinition.m_pobAnimSet->m_obWalkLeft.IsNull() &&
						!m_obDefinition.m_pobAnimSet->m_obWalkRight.IsNull() ) )
					&&
					( ( !m_bCanRun )
					  ||
					  (	!m_obDefinition.m_pobAnimSet->m_obRunLeft.IsNull() &&
						!m_obDefinition.m_pobAnimSet->m_obRunRight.IsNull() ) ) );

	// Can we do a fast turn? - Currently not
	m_bCanFastTurn = (	!m_obDefinition.m_pobAnimSet->m_obFullTurnLeft.IsNull() &&
						!m_obDefinition.m_pobAnimSet->m_obFullTurnRight.IsNull() );


	m_bCanBlink = !m_obDefinition.m_pobAnimSet->m_obBlink.IsNull();
	if ( m_bCanBlink )
	{
		m_pobBlinkAnim = CreateAnimation( m_obDefinition.m_pobAnimSet->m_obBlink, 0 );
		m_pobAnimator->AddAnimation( m_pobBlinkAnim );
	}


	// OK - now lets check that we can at least do something!
	ntAssert( m_bCanSlowWalk || m_bCanWalk || m_bCanRun );

	// Create our idle animations - this is the only animation that we must have!
	m_aobIdleAnims[IA_STAND] = CreateAnimation( m_obDefinition.m_pobAnimSet->m_obStanding, ANIMF_LOCOMOTING|ANIMF_LOOPING );

	// Create our aware animations if necessary
	if ( m_bCanBeAware )
	{
		m_aobAwareAnims[AA_MIN]			= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obAwareMin, ANIMF_LOCOMOTING|ANIMF_LOOPING );
		m_aobAwareAnims[AA_MAX]			= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obAwareMax, ANIMF_LOCOMOTING|ANIMF_LOOPING );
		m_aobToAwareAnims[TAA_START]	= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obToAware, ANIMF_LOCOMOTING );
		m_aobFromAwareAnims[FAA_FROM]	= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obFromAware, ANIMF_LOCOMOTING );
	}

	// Create our rotate on spot animations if necessary
	if ( m_bCanRotateOnSpot )
	{
		// Use idle animation for now.  Add animations here for better rotate on spot movement.
		m_aobRotateOnSpotAnims[RSA_IDLE]	= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obStanding, ANIMF_LOCOMOTING|ANIMF_LOOPING );
	}

	// Create our starting animations
	if ( m_bCanSlowWalk )
	{
		m_aobStartingAnims[STA_TOSLOWWALK]				= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obStandToSlowWalk, ANIMF_LOCOMOTING|ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
		m_aobStartingAnims[STA_TOSLOWWALKLEFT]			= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obStandToSlowWalkLeft, ANIMF_LOCOMOTING|ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET  );
		m_aobStartingAnims[STA_TOSLOWWALKRIGHT]			= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obStandToSlowWalkRight, ANIMF_LOCOMOTING|ANIMF_PHASE_LINKED);
		m_aobStartingAnims[STA_TOSLOWWALKBACKLEFT]		= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obStandToSlowWalkBackLeft, ANIMF_LOCOMOTING|ANIMF_PHASE_LINKED );
		m_aobStartingAnims[STA_TOSLOWWALKBACKRIGHT]		= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obStandToSlowWalkBackRight, ANIMF_LOCOMOTING|ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
	}

	if ( m_bCanWalk )
	{
		m_aobStartingAnims[STA_TOWALK]				= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obStandToWalk, ANIMF_LOCOMOTING|ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
		m_aobStartingAnims[STA_TOWALKLEFT]			= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obStandToWalkLeft, ANIMF_LOCOMOTING|ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET  );
		m_aobStartingAnims[STA_TOWALKRIGHT]			= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obStandToWalkRight, ANIMF_LOCOMOTING|ANIMF_PHASE_LINKED);
		m_aobStartingAnims[STA_TOWALKBACKLEFT]		= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obStandToWalkBackLeft, ANIMF_LOCOMOTING|ANIMF_PHASE_LINKED );
		m_aobStartingAnims[STA_TOWALKBACKRIGHT]		= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obStandToWalkBackRight, ANIMF_LOCOMOTING|ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
	}

	if ( m_bCanRun )
	{
		m_aobStartingAnims[STA_TOACCEL]				= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obStandToAccel, ANIMF_LOCOMOTING|ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
		m_aobStartingAnims[STA_TOACCELLEFT]			= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obStandToAccelLeft, ANIMF_LOCOMOTING|ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
		m_aobStartingAnims[STA_TOACCELRIGHT]		= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obStandToAccelRight, ANIMF_LOCOMOTING|ANIMF_PHASE_LINKED );
		m_aobStartingAnims[STA_TOACCELBACKLEFT]		= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obStandToAccelBackLeft, ANIMF_LOCOMOTING|ANIMF_PHASE_LINKED );
		m_aobStartingAnims[STA_TOACCELBACKRIGHT]	= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obStandToAccelBackRight, ANIMF_LOCOMOTING|ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
	}

	// Create our stopping animations
	if ( m_bCanSlowWalk )
	{
		m_aobStoppingAnims[SPA_FROMSLOWWALKLEFT]	= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obSlowWalkToStandLeft, ANIMF_LOCOMOTING|ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
		m_aobStoppingAnims[SPA_FROMSLOWWALKRIGHT]	= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obSlowWalkToStandRight, ANIMF_LOCOMOTING|ANIMF_PHASE_LINKED );
	}

	if ( m_bCanWalk )
	{
		m_aobStoppingAnims[SPA_FROMWALKLEFT]	= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obWalkToStandLeft, ANIMF_LOCOMOTING|ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
		m_aobStoppingAnims[SPA_FROMWALKRIGHT]	= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obWalkToStandRight, ANIMF_LOCOMOTING|ANIMF_PHASE_LINKED );
	}

	if ( m_bCanRun )
	{
		m_aobStoppingAnims[SPA_FROMRUNLEFT]		= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obRunToStandLeft, ANIMF_LOCOMOTING|ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
		m_aobStoppingAnims[SPA_FROMRUNRIGHT]	= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obRunToStandRight, ANIMF_LOCOMOTING|ANIMF_PHASE_LINKED );
	}

	// Create our moving animations
	if ( m_bCanSlowWalk )
		m_aobMovingAnims[MA_SLOWWALK] = CreateAnimation( m_obDefinition.m_pobAnimSet->m_obSlowWalk, ANIMF_LOCOMOTING|ANIMF_LOOPING|ANIMF_PHASE_LINKED );

	if ( m_bCanWalk )
		m_aobMovingAnims[MA_WALK] = CreateAnimation( m_obDefinition.m_pobAnimSet->m_obWalk, ANIMF_LOCOMOTING|ANIMF_LOOPING|ANIMF_PHASE_LINKED );

	if ( m_bCanRun )
		m_aobMovingAnims[MA_RUN] = CreateAnimation( m_obDefinition.m_pobAnimSet->m_obRun, ANIMF_LOCOMOTING|ANIMF_LOOPING|ANIMF_PHASE_LINKED );

	// Create our turning animations
	if ( m_bCanSlowWalk && m_bCanTurn )
	{
		m_aobMovingAnims[MA_SLOWWALKLEFT]	= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obSlowWalkLeft, ANIMF_LOCOMOTING|ANIMF_LOOPING|ANIMF_PHASE_LINKED );
		m_aobMovingAnims[MA_SLOWWALKRIGHT]	= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obSlowWalkRight, ANIMF_LOCOMOTING|ANIMF_LOOPING|ANIMF_PHASE_LINKED );
	}
	
	if ( m_bCanWalk && m_bCanTurn )
	{
		m_aobMovingAnims[MA_WALKLEFT]		= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obWalkLeft, ANIMF_LOCOMOTING|ANIMF_LOOPING|ANIMF_PHASE_LINKED );
		m_aobMovingAnims[MA_WALKRIGHT]		= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obWalkRight, ANIMF_LOCOMOTING|ANIMF_LOOPING|ANIMF_PHASE_LINKED );
	}
	
	if ( m_bCanRun && m_bCanTurn )
	{
		m_aobMovingAnims[MA_RUNLEFT]		= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obRunLeft, ANIMF_LOCOMOTING|ANIMF_LOOPING|ANIMF_PHASE_LINKED );
		m_aobMovingAnims[MA_RUNRIGHT]		= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obRunRight, ANIMF_LOCOMOTING|ANIMF_LOOPING|ANIMF_PHASE_LINKED );
	}

	// Create our accelerating animations
	if ( m_bCanAccelerate )
	{
		m_aobMovingAnims[MA_ACCELRUN]		= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obAccelRun, ANIMF_LOCOMOTING|ANIMF_LOOPING|ANIMF_PHASE_LINKED );
		m_aobMovingAnims[MA_ACCELRUNLEFT]	= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obAccelRunLeft, ANIMF_LOCOMOTING|ANIMF_LOOPING|ANIMF_PHASE_LINKED );
		m_aobMovingAnims[MA_ACCELRUNRIGHT]	= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obAccelRunRight, ANIMF_LOCOMOTING|ANIMF_LOOPING|ANIMF_PHASE_LINKED );
	}

	// Create our fast turn animations
	if ( m_bCanFastTurn )
	{
		m_aobTurningAnims[TUA_LEFT]			= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obFullTurnLeft, ANIMF_LOCOMOTING|ANIMF_PHASE_LINKED );
		m_aobTurningAnims[TUA_RIGHT]		= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obFullTurnRight, ANIMF_LOCOMOTING|ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
	}

	// Precalculate our possible movement velocities
	if ( m_bCanSlowWalk )
		m_fSlowWalkSpeed = m_aobMovingAnims[MA_SLOWWALK]->GetRootEndTranslation().Z() / m_aobMovingAnims[MA_SLOWWALK]->GetDuration();

	if ( m_bCanWalk )
		m_fWalkSpeed = m_aobMovingAnims[MA_WALK]->GetRootEndTranslation().Z() / m_aobMovingAnims[MA_WALK]->GetDuration();

	if ( m_bCanRun )
		m_fRunSpeed = m_aobMovingAnims[MA_RUN]->GetRootEndTranslation().Z() / m_aobMovingAnims[MA_RUN]->GetDuration();

	// Report if animations are missing
#ifndef _RELEASE
	if (CAnimLoader::Get().IsErrorAnim(m_aobMovingAnims[MA_WALK]->GetAnimationHeader()))
	{
		ntPrintf("WalkRunController: WALK Animation Is Missing or Erroneous\n");
	}

	if (CAnimLoader::Get().IsErrorAnim(m_aobMovingAnims[MA_RUN]->GetAnimationHeader()))
	{
		ntPrintf("WalkRunController: RUN Animation Is Missing or Erroneous\n");
	}
#endif
	// For debugging
	// OSD::EnableChannel(OSD::MOVEMENT);
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::~WalkRunController
//! Destruction
//!
//------------------------------------------------------------------------------------------
WalkRunController::~WalkRunController( void )
{
	// Ensure all our animations are removed
	if ( m_fStateWeights[MS_IDLE] > 0.0f )
		ClearIdle( m_aobIdleAnims );

	if ( m_fStateWeights[MS_STARTING] > 0.0f )
		ClearStarting( m_aobStartingAnims );

	if ( m_fStateWeights[MS_TO_AWARE] > 0.0f )
		ClearToAware( m_aobToAwareAnims );

	if ( m_fStateWeights[MS_AWARE] > 0.0f )
		ClearAware( m_aobAwareAnims );

	if ( m_fStateWeights[MS_FROM_AWARE] > 0.0f )
		ClearFromAware( m_aobFromAwareAnims );
		
	if ( m_fStateWeights[MS_STOPPING] > 0.0f )
		ClearStopping( m_aobStoppingAnims );
	
	if ( m_fStateWeights[MS_MOVING] > 0.0f )
		ClearMoving( m_aobMovingAnims );

	if ( m_fStateWeights[MS_TURNING] > 0.0f )
		ClearTurning( m_aobTurningAnims );

	if ( m_bCanBlink && m_pobBlinkAnim )
		m_pobAnimator->RemoveAnimation( m_pobBlinkAnim );
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::CreateAnimation
//! Creates an animation from a name and sets the correct flags
//!
//------------------------------------------------------------------------------------------
CAnimationPtr WalkRunController::CreateAnimation( const CHashedString& obAnimationName, int iVariableFlags )
{
	// Create the animation
	CAnimationPtr obReturnAnim = m_pobAnimator->CreateAnimation( obAnimationName );

	// Zero the initial blend weight
	obReturnAnim->SetBlendWeight( 0.0f );

	// Set up the flags - including the item we passed in
	obReturnAnim->SetFlags( iVariableFlags|ANIMF_INHIBIT_AUTO_DESTRUCT );

	// Return a pointer to the animation
	return obReturnAnim;
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::ActivateAnimations
//! Activate a sub-states set of animations
//!
//------------------------------------------------------------------------------------------
void WalkRunController::ActivateAnimations( CAnimationPtr* aAnimations, int iNumberOfAnims )
{
	// Add to the animator
	for ( int iAnimations = 0; iAnimations < iNumberOfAnims; ++iAnimations )
	{
		if ( aAnimations[iAnimations] != NULL )
			m_pobAnimator->AddAnimation( aAnimations[iAnimations] );
	}

	// Zero the blend weights
	for ( int iAnimations = 0; iAnimations < iNumberOfAnims; ++iAnimations )
	{
		if ( aAnimations[iAnimations] != NULL )
			aAnimations[iAnimations]->SetBlendWeight( 0.0f );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::DeactivateAnimations
//! Remove a sub-states set of animations from the animator
//!
//------------------------------------------------------------------------------------------
void WalkRunController::DeactivateAnimations( CAnimationPtr* aAnimations, int iNumberOfAnims )
{
	for ( int iAnimations = 0; iAnimations < iNumberOfAnims; ++iAnimations )
	{
		if ( aAnimations[iAnimations] != NULL && aAnimations[iAnimations]->IsActive() )
			m_pobAnimator->RemoveAnimation( aAnimations[iAnimations] );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::StartIdle
//! Start the idle state
//!
//------------------------------------------------------------------------------------------
void WalkRunController::StartIdle( const CMovementStateRef& /* obCurrentMovementState */, CAnimationPtr* aobIdleAnims )
{
	// Reset the state time
	m_fStateTime = 0.0f;

	// Make sure our mechanisms are safe
	if ( m_fStateWeights[MS_IDLE] > 0.0f )
		ClearIdle( m_aobIdleAnims );

	// Set the state flag
	m_eMovementState = MS_IDLE;

	// Activate all our animations
	ActivateAnimations( aobIdleAnims, IA_COUNT );

		// Send a message out to anyone who may be interested - only after initialisation and for our base animations
	if ( ( m_bInitialised ) && ( aobIdleAnims == m_aobIdleAnims ) )
		CMessageSender::SendEmptyMessage( CHashedString(HASH_STRING_MSG_WALK_RUN_STOPPED), m_pobMovement->GetParentEntity()->GetMessageHandler() );

#ifndef _RELEASE
	// Some debug output
	OSD::Add( OSD::MOVEMENT, 0xffff8888, "Start Idle" );
#endif // _RELEASE
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::UpdateIdle
//! Update the idle state
//!
//------------------------------------------------------------------------------------------
void WalkRunController::UpdateIdle( float /* fTimeDelta */, const CMovementStateRef& obCurrentMovementState )
{
	// HACK for AI rotating on spot
	if ( m_pobMovement->GetParentEntity()->IsAI() && m_bAIRequestedRotation )
	{
		StartRotateOnSpot( obCurrentMovementState, m_aobRotateOnSpotAnims );
		return;
	}

	// Start moving as soon as we have a requested velocity
	if ( ( m_fRequestedSpeed > 0 )  )
	{
		StartStarting( obCurrentMovementState, m_aobStartingAnims );
	}

	// Otherwise if we have a target point and can respond
	else if ( m_bCanBeAware && m_bTargetPointSet )
		StartToAware( obCurrentMovementState, m_aobToAwareAnims );
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::ClearIdle
//! Clears up the idle sub-state
//!
//------------------------------------------------------------------------------------------
void WalkRunController::ClearIdle( CAnimationPtr* aobIdleAnims )
{
	DeactivateAnimations( aobIdleAnims, IA_COUNT );	
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::StartAware
//! 
//!
//------------------------------------------------------------------------------------------
void WalkRunController::StartAware( const CMovementStateRef& /* obCurrentMovementState */, CAnimationPtr* aobAwareAnims )
{
	// Reset the state time
	m_fStateTime = 0.0f;

	// Make sure our mechaisms are safe
	if ( m_fStateWeights[MS_AWARE] > 0.0f )
		ClearAware( m_aobAwareAnims );

	// Set the state flag
	m_eMovementState = MS_AWARE;

	// Activate all our animations
	ActivateAnimations( aobAwareAnims, AA_COUNT );

	// WARNING - Disabled message as it causes a new walk run controller to be added when we go into the aware state.
	// This stops the aware state from working properly.  So I have disabled this message.  Nothing else seems to
	// rely on this message anyway.  MB

	// Send a message out to anyone who may be interested - only after initialisation and for our base animations
	//if ( ( m_bInitialised ) && ( aobAwareAnims == m_aobAwareAnims ) )
		//CMessageSender::SendEmptyMessage( CHashedString(HASH_STRING_MSG_WALK_RUN_STOPPED), m_pobMovement->GetParentEntity()->GetMessageHandler() );

#ifndef _RELEASE
	// Some debug output
	OSD::Add( OSD::MOVEMENT, 0xffff8888, "Start Aware" );
#endif // _RELEASE
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::UpdateAware
//! 
//!
//------------------------------------------------------------------------------------------
void WalkRunController::UpdateAware( float /* fTimeDelta */, const CMovementStateRef& obCurrentMovementState )
{
	// Start moving as soon as we have a requested velocity
	if ( ( m_fRequestedSpeed > 0 ) || ( !m_bTargetPointSet ) )
		StartFromAware( obCurrentMovementState, m_aobFromAwareAnims );
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::ClearAware
//! 
//!
//------------------------------------------------------------------------------------------
void WalkRunController::ClearAware( CAnimationPtr* aobAwareAnims )
{
	DeactivateAnimations( aobAwareAnims, AA_COUNT );
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::StartToAware
//! 
//!
//------------------------------------------------------------------------------------------
void WalkRunController::StartToAware( const CMovementStateRef& /* obCurrentMovementState */, CAnimationPtr* aobToAwareAnims )
{
	// Reset the state time
	m_fStateTime = 0.0f;

	// Make sure our mechaisms are safe
	if ( m_fStateWeights[MS_TO_AWARE] > 0.0f )
		ClearToAware( m_aobToAwareAnims );

	// Set the state flag
	m_eMovementState = MS_TO_AWARE;

	// Clear the animation time and add it to the animator
	m_aobToAwareAnims[TAA_START]->SetTime( 0.0f );
	m_pobAnimator->AddAnimation( aobToAwareAnims[TAA_START] );

#ifndef _RELEASE
	// Some debug output
	OSD::Add( OSD::MOVEMENT, 0xffff8888, "Start To Aware" );
#endif // _RELEASE
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::UpdateToAware
//! 
//!
//------------------------------------------------------------------------------------------
void WalkRunController::UpdateToAware( float fTimeDelta, const CMovementStateRef& obCurrentMovementState )
{
	// When our transition animation has finished move onto the looping stuff
	if ( m_aobToAwareAnims[TAA_START]->GetTime() > ( m_aobToAwareAnims[TAA_START]->GetDuration() - fTimeDelta ) )
		StartAware( obCurrentMovementState, m_aobAwareAnims );
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::ClearToAware
//! 
//!
//------------------------------------------------------------------------------------------
void WalkRunController::ClearToAware( CAnimationPtr* aobToAwareAnims )
{
	DeactivateAnimations( aobToAwareAnims, TAA_COUNT );
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::StartFromAware
//! 
//!
//------------------------------------------------------------------------------------------
void WalkRunController::StartFromAware( const CMovementStateRef& /* obCurrentMovementState */, CAnimationPtr* aobFromAwareAnims )
{
	// Reset the state time
	m_fStateTime = 0.0f;

	// Make sure our mechaisms are safe
	if ( m_fStateWeights[MS_FROM_AWARE] > 0.0f )
		ClearFromAware( m_aobFromAwareAnims );

	// Set the state flag
	m_eMovementState = MS_FROM_AWARE;

	// Clear the animation time and add it to the animator
	m_aobFromAwareAnims[FAA_FROM]->SetTime( 0.0f );
	m_pobAnimator->AddAnimation( aobFromAwareAnims[FAA_FROM] );

#ifndef _RELEASE
	// Some debug output
	OSD::Add( OSD::MOVEMENT, 0xffff8888, "Start From Aware" );
#endif // _RELEASE
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::UpdateFromAware
//! 
//!
//------------------------------------------------------------------------------------------
void WalkRunController::UpdateFromAware( float fTimeDelta, const CMovementStateRef& obCurrentMovementState )
{
	// HACK for AI rotating on spot
	if ( m_pobMovement->GetParentEntity()->IsAI() && m_bAIRequestedRotation )
	{
		StartRotateOnSpot( obCurrentMovementState, m_aobRotateOnSpotAnims );
		return;
	}

	// If there is a requested speed go to starting
	if ( m_fRequestedSpeed > 0 ) 
		StartStarting( obCurrentMovementState, m_aobStartingAnims );

	// Otherwise if we manage to reach the end of our anim return to idle
	else if ( m_aobFromAwareAnims[FAA_FROM]->GetTime() > ( m_aobFromAwareAnims[FAA_FROM]->GetDuration() - fTimeDelta ) )
		StartIdle( obCurrentMovementState, m_aobIdleAnims );
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::ClearFromAware
//! 
//!
//------------------------------------------------------------------------------------------
void WalkRunController::ClearFromAware( CAnimationPtr* aobFromAwareAnims )
{
	DeactivateAnimations( aobFromAwareAnims, FAA_COUNT );
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::StartRotateOnSpot
//! 
//!
//------------------------------------------------------------------------------------------
void WalkRunController::StartRotateOnSpot( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobRotateOnSpotAnims )
{
	// Only AI's can use this code at the moment
	ntAssert( m_pobMovement->GetParentEntity()->IsAI() );

	// Reset the state time
	m_fStateTime = 0.0f;

	// Make sure our mechanisms are safe
	if ( m_fStateWeights[MS_ROTATE_ON_SPOT] > 0.0f )
		ClearRotateOnSpot( m_aobRotateOnSpotAnims );

	// Set the state flag
	m_eMovementState = MS_ROTATE_ON_SPOT;

	// Activate animations
	ActivateAnimations( aobRotateOnSpotAnims, RSA_COUNT );

	// Match the animation time of this idle animation with the one from the idle state
	// WARNING: We're assuming that we're using the same animations, but the code below is safer for future changes.
	float fIdleStandAnimTime = m_aobIdleAnims[IA_STAND]->GetTime();
	float fRotateIdleDuration = aobRotateOnSpotAnims[RSA_IDLE]->GetDuration();

	if ( fIdleStandAnimTime > fRotateIdleDuration )
	{
		aobRotateOnSpotAnims[RSA_IDLE]->SetTime( fRotateIdleDuration );
	}
	else
	{
		aobRotateOnSpotAnims[RSA_IDLE]->SetTime( fIdleStandAnimTime );
	}

#ifndef _RELEASE
	// Some debug output
	OSD::Add( OSD::MOVEMENT, 0xffff8888, "Start Rotate On Spot" );
#endif // _RELEASE
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::UpdateRotateOnSpot
//! 
//!
//------------------------------------------------------------------------------------------
void WalkRunController::UpdateRotateOnSpot( float fTimeDelta, const CMovementStateRef& obCurrentMovementState )
{
	// Only AI's can use this code at the moment
	ntAssert( m_pobMovement->GetParentEntity()->IsAI() );

	// Start moving as soon as we have a requested velocity and we don't want to rotate
	if ( m_fRequestedSpeed > 0.0f && !m_bAIRequestedRotation  )
	{
		StartStarting( obCurrentMovementState, m_aobStartingAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::ClearRotateOnSpot
//! 
//!
//------------------------------------------------------------------------------------------
void WalkRunController::ClearRotateOnSpot( CAnimationPtr* aobRotateOnSpotAnims )
{
	DeactivateAnimations( aobRotateOnSpotAnims, RSA_COUNT );
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::CalculateStartBlendWeights
//! Calculate the animation blend weights for when we are starting
//!
//------------------------------------------------------------------------------------------
void WalkRunController::CalculateStartBlendWeights( float fTimeDelta )
{
	// If we can only run then we must have a full blend for the run animation
	if ( !( m_bCanWalk || m_bCanSlowWalk ) )
	{
		m_fRunStartWeight = 1.0f;
		m_fWalkStartWeight = 0.0f;
		m_fSlowWalkStartWeight = 0.0f;
	}

	// Otherwise things are more complicated
	else
	{
		// If our input wants us to run
		if ( ( m_fRunStartWeight > 0.0f ) || ( ( m_fRequestedSpeed == m_fRunSpeed ) && ( m_bCanRun ) && ( m_fStateTime >= m_fBlendInTime ) ) )
		{
			// If this is the first time here
			if ( m_fRunStartWeight == 0.0f )
				m_aobStartingAnims[m_eChosenStartRun]->SetTime( 0.0f );

			// If we have started running blend in the run animation
			if ( ( m_fRunStartWeight < 1.0f ) )
			{
				// Add to the run weight
				m_fRunStartWeight += fTimeDelta / m_fBlendInTime;

				// Check the boundaries
				if ( m_fRunStartWeight > 1.0f )
					m_fRunStartWeight = 1.0f;
			}
		}

		// Now we know the run amount - find the slow walk amount
		if ( ( !m_bCanSlowWalk ) || ( m_fRequestedSpeed == 0.0f ) || ( m_fWalkSpeed <= m_fSlowWalkSpeed ) || ( m_fRunStartWeight > 0.0f ) || ( m_fRequestedSpeed > m_fWalkSpeed ) )
			m_fSlowWalkStartWeight = 0.0f;
		else
			m_fSlowWalkStartWeight = ( 1.0f - ( ( m_fRequestedSpeed - m_fSlowWalkSpeed ) / ( m_fWalkSpeed - m_fSlowWalkSpeed ) ) );

		// The walk start weight is simply what's left
		m_fWalkStartWeight = 1.0f - m_fRunStartWeight - m_fSlowWalkStartWeight;
	}
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::CalculateStartTurnAmount
//! Calculate the animation turn values whilst starting
//!
//------------------------------------------------------------------------------------------
void WalkRunController::CalculateStartTurnAmount( float fTimeDelta )
{
	// Store the last value
	m_fLastAnimTurnAmount = m_fAnimTurnAmount;

	// We'll need these for some calls 
	CDirection obTempAxis( CONSTRUCT_CLEAR );
	CQuat obTempRotation( CONSTRUCT_IDENTITY );

	// Get the current root rotation of each of the start animations
	float fRunTurnProgress = 0.0f;
	float fWalkTurnProgress = 0.0f;
	float fSlowWalkTurnProgress = 0.0f;

	// If we can run...
	if ( m_bCanRun )
		fRunTurnProgress = MovementControllerUtilities::GetYRotation( m_aobStartingAnims[m_eChosenStartRun]->GetRootRotationAtTime( m_aobStartingAnims[m_eChosenStartRun]->GetTime(), m_pobAnimator != NULL ? m_pobAnimator->GetHierarchy() : NULL ) );

	// If we can walk
	if ( m_bCanWalk )
		fWalkTurnProgress = MovementControllerUtilities::GetYRotation( m_aobStartingAnims[m_eChosenStartWalk]->GetRootRotationAtTime( m_aobStartingAnims[m_eChosenStartWalk]->GetTime(), m_pobAnimator != NULL ? m_pobAnimator->GetHierarchy() : NULL ) );

	// If we can slow walk
	if ( m_bCanSlowWalk )
		fSlowWalkTurnProgress = MovementControllerUtilities::GetYRotation( m_aobStartingAnims[m_eChosenStartSlowWalk]->GetRootRotationAtTime( m_aobStartingAnims[m_eChosenStartSlowWalk]->GetTime(), m_pobAnimator != NULL ? m_pobAnimator->GetHierarchy() : NULL ) );

	// Now take the weighted average of these values - assume the sum of weights is one
	m_fAnimTurnAmount =	( fSlowWalkTurnProgress * m_fSlowWalkStartWeight ) +
						( fWalkTurnProgress * m_fWalkStartWeight ) + 
						( fRunTurnProgress * m_fRunStartWeight );

	// Now how far will this animation turn us over the whole period and how long is the state - both approximate
	float fAnimTurn = 0.0f;
	float fStartDuration = 0.0f;

	// If slow walk is the dominant animation
	if ( ( m_bCanSlowWalk ) && ( m_fSlowWalkStartWeight > m_fWalkStartWeight ) && ( m_fSlowWalkStartWeight > fRunTurnProgress ) )
	{
		fAnimTurn = MovementControllerUtilities::GetYRotation( m_aobStartingAnims[m_eChosenStartSlowWalk]->GetRootEndRotation() );
		fStartDuration = m_aobStartingAnims[m_eChosenStartSlowWalk]->GetDuration();
	}

	// Otherwise if walk is the dominant animation
	else if ( ( m_bCanWalk ) && ( m_fWalkStartWeight > fRunTurnProgress ) )
	{
		fAnimTurn = MovementControllerUtilities::GetYRotation( m_aobStartingAnims[m_eChosenStartWalk]->GetRootEndRotation() );
		fStartDuration = m_aobStartingAnims[m_eChosenStartWalk]->GetDuration();
	}

	// Otherwise run must be the dominant animation
	else if ( m_bCanRun )
	{
		fAnimTurn = MovementControllerUtilities::GetYRotation( m_aobStartingAnims[m_eChosenStartRun]->GetRootEndRotation() );
		fStartDuration = m_aobStartingAnims[m_eChosenStartRun]->GetDuration();
	}

	// How far do we need to have turned over the course of this start state
	float fStartTurnAmount = MovementControllerUtilities::RotationAboutY( m_obStartFacingDirection, m_obLastRequestedDirection );

	// Add a small slice of the difference this frame
	if ( fStartDuration > 0.0f )
		m_fAnimTurnAmount += ( fStartTurnAmount - fAnimTurn ) * ( fTimeDelta / fStartDuration );
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::StartStarting
//! Start the starting sub state
//!
//------------------------------------------------------------------------------------------
void WalkRunController::StartStarting( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobStartingAnims )
{
	// Reset the state time
	m_fStateTime = 0.0f;

	// Make sure our mechaisms are safe
	if ( m_fStateWeights[MS_STARTING] > 0.0f )
		ClearStarting( m_aobStartingAnims );

	// Set the state flag
	m_eMovementState = MS_STARTING;

	// Reset our animation weights
	m_fRunStartWeight = 0.0f;
	m_fWalkStartWeight = 0.0f;
	m_fSlowWalkStartWeight = 0.0f;

	// Choose our animation based on direction/speed
	float fTurnAngle = MovementControllerUtilities::RotationAboutY( obCurrentMovementState.m_obFacing, m_obLastRequestedDirection );

	// Choose an angled animation from each of our speeds
	m_eChosenStartRun = static_cast<STARTING_ANIMATIONS>( STA_TOACCEL + MovementControllerUtilities::GetAnimationDirection( fTurnAngle ) );
	m_eChosenStartWalk = static_cast<STARTING_ANIMATIONS>( STA_TOWALK + MovementControllerUtilities::GetAnimationDirection( fTurnAngle ) );
	m_eChosenStartSlowWalk = static_cast<STARTING_ANIMATIONS>( STA_TOSLOWWALK + MovementControllerUtilities::GetAnimationDirection( fTurnAngle ) );

	// Acivate the animations we are going to use
	if ( m_bCanSlowWalk )
	{
		aobStartingAnims[m_eChosenStartSlowWalk]->SetTime( 0.0f );
		m_pobAnimator->AddAnimation( aobStartingAnims[m_eChosenStartSlowWalk] );
	}

	if ( m_bCanWalk )
	{
		aobStartingAnims[m_eChosenStartWalk]->SetTime( 0.0f );
		m_pobAnimator->AddAnimation( aobStartingAnims[m_eChosenStartWalk] );
	}

	if ( m_bCanRun )
	{
		aobStartingAnims[m_eChosenStartRun]->SetTime( 0.0f );
		m_pobAnimator->AddAnimation( aobStartingAnims[m_eChosenStartRun] );
	}

	// Sort out the blend wieghts
	CalculateStartBlendWeights( 0.0f );

	// Reset our turn amount
	m_fAnimTurnAmount = 0.0f;
	m_fLastAnimTurnAmount = 0.0f;

	// Save the current facing direction
	m_obStartFacingDirection = obCurrentMovementState.m_obFacing;

#ifndef _RELEASE
	// Some debug output
	OSD::Add( OSD::MOVEMENT, 0xffff8888, "Start Starting. Angle: %0.2f.", fTurnAngle * RAD_TO_DEG_VALUE );
#endif // _RELEASE
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::UpdateStarting
//! Update the the starting sub state
//!
//------------------------------------------------------------------------------------------
void WalkRunController::UpdateStarting( float fTimeDelta, const CMovementStateRef& obCurrentMovementState )
{
	// Update our info
	CalculateStartBlendWeights( fTimeDelta );
	CalculateStartTurnAmount( fTimeDelta );

	// If we are no longer requesting movement
	if ( ( JustPassedPhaseBoundary() ) && ( m_fRequestedSpeed == 0.0f ) )
	{
		StartStopping( obCurrentMovementState, m_aobStoppingAnims );
	}

	// Start blending out so we blend to the end of the animation
	else if ( ( m_fRequestedSpeed > 0.0f )
			  &&
		      ( ( ( m_fRunStartWeight > 0.0f ) && ( m_aobStartingAnims[m_eChosenStartRun]->GetTime() > ( m_aobStartingAnims[m_eChosenStartRun]->GetDuration() - m_fBlendInTime - fTimeDelta ) ) )
		        ||
		        ( ( m_fRunStartWeight == 0.0f ) && ( m_aobStartingAnims[m_eChosenStartWalk]->GetTime() > ( m_aobStartingAnims[m_eChosenStartWalk]->GetDuration() - m_fBlendInTime - fTimeDelta ) ) ) ) )
	{
		// If a speed is still being requested go into our main movement
		if ( m_fRequestedSpeed > 0.0f )
			StartMoving( obCurrentMovementState, m_aobMovingAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::ClearStarting
//! Clears up the starting sub-state
//!
//------------------------------------------------------------------------------------------
void WalkRunController::ClearStarting( CAnimationPtr* aobStartingAnims )
{
	if ( m_bCanRun )
	{
		if (aobStartingAnims[m_eChosenStartRun]->IsActive())
			m_pobAnimator->RemoveAnimation( aobStartingAnims[m_eChosenStartRun] );	
	}
	if ( m_bCanWalk )
	{
		if (aobStartingAnims[m_eChosenStartWalk]->IsActive())
			m_pobAnimator->RemoveAnimation( aobStartingAnims[m_eChosenStartWalk] );	
	}
	if ( m_bCanSlowWalk )
	{
		if (aobStartingAnims[m_eChosenStartSlowWalk]->IsActive())
			m_pobAnimator->RemoveAnimation( aobStartingAnims[m_eChosenStartSlowWalk] );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::StartStopping
//! Start the stopping sub state
//!
//------------------------------------------------------------------------------------------
void WalkRunController::StartStopping( const CMovementStateRef& /* obCurrentMovementState */, CAnimationPtr* aobStoppingAnims )
{
	// Reset the state time
	m_fStateTime = 0.0f;

	// Make sure our mechaisms are safe
	if ( m_fStateWeights[MS_STOPPING] > 0.0f )
		ClearStopping( m_aobStoppingAnims );

	// Set the state flag
	m_eMovementState = MS_STOPPING;

	// If we have just started up the speed values are unreliable
	bool bJustStarted = ( m_fStateWeights[MS_STARTING] > 0.0f );

	// Choose the animation we want
	if ( ( m_bCanRun ) && ( ( m_fCurrentSpeed > m_fWalkSpeed ) || ( bJustStarted && ( m_fRunStartWeight > 0.5f ) ) ) )
	{
		if ( m_pobAnimator->GetPhasePosition() > 0.5f )
			m_eChosenStop = SPA_FROMRUNLEFT;
		else
			m_eChosenStop = SPA_FROMRUNRIGHT;
	}
	else
	{
		if ( m_bCanSlowWalk )
		{
			if ( m_pobAnimator->GetPhasePosition() > 0.5f )
				m_eChosenStop = SPA_FROMSLOWWALKLEFT;
			else
				m_eChosenStop = SPA_FROMSLOWWALKRIGHT;
		}

		else
		{
			if ( m_pobAnimator->GetPhasePosition() > 0.5f )
				m_eChosenStop = SPA_FROMWALKLEFT;
			else
				m_eChosenStop = SPA_FROMWALKRIGHT;
		}
	}

	// Activate the single animation we are going to use - make sure the phase linking is set - it could have been
	// removed if the character has done a quick stop/start previously
	aobStoppingAnims[m_eChosenStop]->SetTime( 0.0f );
	aobStoppingAnims[m_eChosenStop]->SetFlagBits( ANIMF_PHASE_LINKED );
	m_pobAnimator->AddAnimation( aobStoppingAnims[m_eChosenStop] );

#ifndef _RELEASE
	// Some debug output
	OSD::Add( OSD::MOVEMENT, 0xffff8888, "Start Stopping" );
#endif // _RELEASE
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::UpdateStopping
//! Update the stopping sub state
//!
//------------------------------------------------------------------------------------------
void WalkRunController::UpdateStopping( float fTimeDelta, const CMovementStateRef& obCurrentMovementState )
{
	// Play our chosen animation throught to the end
	if ( m_aobStoppingAnims[m_eChosenStop]->GetTime() > ( m_aobStoppingAnims[m_eChosenStop]->GetDuration() - fTimeDelta ) )
	{
		if ( m_bCanBeAware && m_bTargetPointSet )
			StartToAware( obCurrentMovementState, m_aobToAwareAnims );
		else
			StartIdle( obCurrentMovementState, m_aobIdleAnims );
	}

	// If we have started requesting movement again
	if ( m_fStateTime > m_fBlendInTime )
	{
		// If movement is being requested again then start moving
		if ( m_fRequestedSpeed > 0.0f )
		{
			// Kill the phase linking on the stop animation so we don't link to the start
			m_aobStoppingAnims[m_eChosenStop]->ClearFlagBits( ANIMF_PHASE_LINKED );
				
			StartStarting( obCurrentMovementState, m_aobStartingAnims );
		}
	}
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::ClearStopping
//! Clears up the stopping sub-state
//!
//------------------------------------------------------------------------------------------
void WalkRunController::ClearStopping( CAnimationPtr* aobStoppingAnims )
{
	if (aobStoppingAnims[m_eChosenStop]->IsActive())
		m_pobAnimator->RemoveAnimation( aobStoppingAnims[m_eChosenStop] );	
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::StartMoving
//! Start the moving sub state
//!
//------------------------------------------------------------------------------------------
void WalkRunController::StartMoving( const CMovementStateRef& /* obCurrentMovementState */, CAnimationPtr* aobMovingAnims )
{
	// Reset the state time
	m_fStateTime = 0.0f;

	// Make sure our mechaisms are safe
	if ( m_fStateWeights[MS_MOVING] > 0.0f )
		ClearMoving( m_aobMovingAnims );

	// Set the state flag
	m_eMovementState = MS_MOVING;

	// Activate all our animations
	ActivateAnimations( aobMovingAnims, MA_COUNT );

	// Set the run animation speed to minimum allowed.
	if ( m_bCanRun )
		aobMovingAnims[MA_RUN]->SetSpeed(m_obDefinition.m_fRunAnimMinSpeed);

	// We don't want to turn yet
	m_bQuickTurn = false;

#ifndef _RELEASE
	// Some debug output
	OSD::Add( OSD::MOVEMENT, 0xffff8888, "Start Moving" );
#endif // _RELEASE
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::UpdateMoving
//! Update the moving sub state
//!
//------------------------------------------------------------------------------------------
void WalkRunController::UpdateMoving( float /* fTimeDelta */, const CMovementStateRef& obCurrentMovementState )
{
	// find the requested angle between our facing direction and the requested direction
	float fTurnAngle = MovementControllerUtilities::RotationAboutY( obCurrentMovementState.m_obFacing, m_obLastRequestedDirection );
	
	// If we are running and the angle is large request a quick turn
	if ( ( m_bCanFastTurn ) 
		 && 
		 ( m_fRequestedSpeed > m_fWalkSpeed ) 
		 && 
		 ( fabs( m_fCurrentSpeed - m_fRunSpeed ) < fabs( m_fCurrentSpeed - m_fWalkSpeed ) ) 
		 && 
		 ( fabsf( fTurnAngle ) > ( 0.8f * PI ) )
		 &&
		 ( m_fTurnClampDuration < 0.1f ) )
	{
		m_bQuickTurn = true;
	}

	// We can only pop out of this movement if we are at the best spot in an animation
	if ( JustPassedPhaseBoundary() )
	{
		// Turn if the time is right
		if ( m_bQuickTurn )
		{
			StartTurning( obCurrentMovementState, m_aobTurningAnims );
			m_bQuickTurn = false;
		}

		// If our requested speed is zero - stop
		else if ( m_fRequestedSpeed == 0 )
			StartStopping( obCurrentMovementState, m_aobStoppingAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::ClearMoving
//! Clears up the moving sub-state
//!
//------------------------------------------------------------------------------------------
void WalkRunController::ClearMoving( CAnimationPtr* aobMovingAnims )
{
	DeactivateAnimations( aobMovingAnims, MA_COUNT );	
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::StartTurning
//! Start the turning sub state
//!
//------------------------------------------------------------------------------------------
void WalkRunController::StartTurning( const CMovementStateRef& /* obCurrentMovementState */, CAnimationPtr* aobTurningAnims )
{
	// Reset the state time
	m_fStateTime = 0.0f;

	// Make sure our mechanisms are safe
	if ( m_fStateWeights[MS_TURNING] > 0.0f )
		ClearTurning( m_aobTurningAnims );

	// Set the state flag
	m_eMovementState = MS_TURNING;

	// Choose the animation we want
	if ( m_pobAnimator->GetPhasePosition() > 0.5f )
		m_eChosenTurn = TUA_RIGHT;
	else
		m_eChosenTurn = TUA_LEFT;

	// Activate the single animation we are going to use
	aobTurningAnims[m_eChosenTurn]->SetTime( 0.0f );
	m_pobAnimator->AddAnimation( aobTurningAnims[m_eChosenTurn] );

	// Reset our turn amount
	m_fAnimTurnAmount = 0.0f;
	m_fLastAnimTurnAmount = 0.0f;

#ifndef _RELEASE
	// Some debug output
	OSD::Add( OSD::MOVEMENT, 0xffff8888, "Start Turning" );
#endif // _RELEASE
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::UpdateTurning
//! Update the turning sub state
//!
//------------------------------------------------------------------------------------------
void WalkRunController::UpdateTurning( float fTimeDelta, const CMovementStateRef& obCurrentMovementState )
{
	// Start blending out so we blend to the end of the movement
	if ( m_aobTurningAnims[m_eChosenTurn]->GetTime() > ( m_aobTurningAnims[m_eChosenTurn]->GetDuration() - m_fBlendInTime - fTimeDelta ) )
	{
		// If a speed is still being requested go into our main movement
		if ( m_fRequestedSpeed > 0.0f )
			StartMoving( obCurrentMovementState, m_aobMovingAnims );

		// Otherwise we need to start stopping
		else
			StartStopping( obCurrentMovementState, m_aobStoppingAnims );
	}

	// Otherwise we need to calculate our turn amount for this frame
	else
	{
		// Store the last value
		m_fLastAnimTurnAmount = m_fAnimTurnAmount;

		// Get the root rotation from the animation
		CQuat obTurnProgress = m_aobTurningAnims[m_eChosenTurn]->GetRootRotationAtTime( m_aobTurningAnims[m_eChosenTurn]->GetTime(), m_pobAnimator != NULL ? m_pobAnimator->GetHierarchy() : NULL );

		// Get the axis and angle - we know that the animation only rotates around y
		CDirection obTempAxis( CONSTRUCT_CLEAR );
		obTurnProgress.GetAxisAndAngle( obTempAxis, m_fAnimTurnAmount );

		// Make sure that we have the right sign
		if ( obTempAxis.Y() < 0.0f )
			m_fAnimTurnAmount = -m_fAnimTurnAmount;
	}
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::ClearTurning
//! Clears up the turning sub-state
//!
//------------------------------------------------------------------------------------------
void WalkRunController::ClearTurning( CAnimationPtr* aobTurningAnims )
{
	m_pobAnimator->RemoveAnimation( aobTurningAnims[m_eChosenTurn] );		
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::UpdateStateWeights
//! Update the blend weights of the sub states
//!
//------------------------------------------------------------------------------------------
void WalkRunController::UpdateStateWeights( float fTimeDelta )
{
	// Set up a general blend time - should be fairly swift since we have tight control

	// Calculate the blend weight change
	float fBlendChange = fTimeDelta / m_fBlendInTime;

	// Increment the weight of the current state 
	m_fStateWeights[m_eMovementState] += fBlendChange;

	// Check the bounds
	if ( m_fStateWeights[m_eMovementState] > 1.0f )
		m_fStateWeights[m_eMovementState] = 1.0f;

	// Assume that we only have two states cross fading - nearly always true
	for ( int iState = 0; iState < MS_COUNT; ++iState )
	{
		// If another state has a weight...
		if ( ( iState != m_eMovementState ) && m_fStateWeights[iState] > 0.0f )
		{
			// Decrement it
			m_fStateWeights[iState] -= fBlendChange;

			// Check the bounds
			if ( m_fStateWeights[iState] <= 0.0f )
			{
				// Clip the weight 
				m_fStateWeights[iState] = 0.0f;

				// Clear out the animations
				switch( iState )
				{
				case MS_IDLE:			ClearIdle( m_aobIdleAnims );				break;
				case MS_TO_AWARE:		ClearToAware( m_aobToAwareAnims );			break;
				case MS_AWARE:			ClearAware( m_aobAwareAnims );				break;
				case MS_FROM_AWARE:		ClearFromAware( m_aobFromAwareAnims );		break;
				case MS_ROTATE_ON_SPOT: ClearRotateOnSpot( m_aobRotateOnSpotAnims );break;
				case MS_STARTING:		ClearStarting( m_aobStartingAnims );		break;
				case MS_STOPPING:		ClearStopping( m_aobStoppingAnims );		break;
				case MS_MOVING:			ClearMoving( m_aobMovingAnims );			break;
				case MS_TURNING:		ClearTurning( m_aobTurningAnims );			break;
				default:				ntAssert( 0 );								break;
				}
			}
		}
	}
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::InitialiseState
//! Choose the first internal state when we are pushed on to the controller stack. 
//!	Currently this is very simple but we may want to jump straight into start/stop/turn
//!	directly in the future.
//!
//------------------------------------------------------------------------------------------
void WalkRunController::InitialiseState( const CMovementStateRef& obCurrentMovementState )
{
	// If we have a requested input
	if ( m_fRequestedSpeed > 0.0f )
	{
		// Try and match the current move state
		if ( ( m_fCurrentSpeed <= m_fSlowWalkSpeed )
			||
			( ( !m_bCanSlowWalk ) && ( m_fCurrentSpeed <= m_fWalkSpeed ) ) )
		{
			// Should we be aware?
			if ( m_bCanBeAware && m_bTargetPointSet )
				StartAware( obCurrentMovementState, m_aobAwareAnims );
			else if (m_bCanBeAware && m_pobMovement->GetParentEntity()->GetAttackComponent()->GetMovementFromCombatPoseFlag())
			{
				StartFromAware( obCurrentMovementState, m_aobFromAwareAnims );

				CAttackComponent* pobAttack = const_cast< CAttackComponent* >(m_pobMovement->GetParentEntity()->GetAttackComponent());
				pobAttack->SetMovementFromCombatPoseFlag(false);
			}
			else
				StartIdle( obCurrentMovementState, m_aobIdleAnims );
		}

		// Otherwise we'll go with moving
		else
			StartMoving( obCurrentMovementState, m_aobMovingAnims );
	}

	// Otherwise if we have no input we default to static
	else
	{
		// Should we be aware?
		if ( m_bCanBeAware && m_bTargetPointSet )
			StartAware( obCurrentMovementState, m_aobAwareAnims );
		else if (m_bCanBeAware && m_pobMovement->GetParentEntity()->GetAttackComponent()->GetMovementFromCombatPoseFlag())
		{
			StartFromAware( obCurrentMovementState, m_aobFromAwareAnims );

			CAttackComponent* pobAttack = const_cast< CAttackComponent* >(m_pobMovement->GetParentEntity()->GetAttackComponent());
			pobAttack->SetMovementFromCombatPoseFlag(false);
		}
		else
			StartIdle( obCurrentMovementState, m_aobIdleAnims );
	}

	// Blend fully to the start state
	m_fStateWeights[m_eMovementState] = 1.0f;

	// We are initialised
	m_bInitialised = true;
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::ProcessInputParameters
//! Takes the requested directions and build parameters that are used to directly define the
//!	constroller behaviour.
//!
//------------------------------------------------------------------------------------------
void WalkRunController::ProcessInputParameters( const CMovementInput& obMovementInput )
{
	// Take the input speed...
	float fInputSpeed = obMovementInput.m_fMoveSpeed;

	// AI ONLY ROTATE ON SPOT HACK
	if ( m_pobMovement->GetParentEntity()->IsAI() )
	{
		if ( fInputSpeed < m_obDefinition.m_fDeadZoneMaximum && fInputSpeed > 0.18f )
			m_bAIRequestedRotation = true;
		else
			m_bAIRequestedRotation = false;
	}

	// Calculate how our requested input speed maps to a walking output speed
	if ( fInputSpeed < m_obDefinition.m_fDeadZoneMaximum )
		m_fRequestedSpeed = 0.0f;
	else if ( fInputSpeed <= m_obDefinition.m_fWalkZoneAnalogue )
		m_fRequestedSpeed = m_fSlowWalkSpeed + ( ( ( fInputSpeed - m_obDefinition.m_fDeadZoneMaximum ) / ( m_obDefinition.m_fWalkZoneAnalogue - m_obDefinition.m_fDeadZoneMaximum ) ) * ( m_fWalkSpeed - m_fSlowWalkSpeed ) );
	else if ( fInputSpeed <= m_obDefinition.m_fWalkZonePlateau )
		m_fRequestedSpeed = m_fWalkSpeed;
	else if (fInputSpeed <= m_obDefinition.m_fRunZoneAnalogue)
		m_fRequestedSpeed = m_fWalkSpeed + (((fInputSpeed - m_obDefinition.m_fWalkZonePlateau) / (m_obDefinition.m_fRunZoneAnalogue - m_obDefinition.m_fWalkZonePlateau)) * (m_fRunSpeed - m_fWalkSpeed));
	else
		m_fRequestedSpeed = m_fRunSpeed;

	// If the requested speed is greater than zero we update the last requested direction
	if ( m_fRequestedSpeed > 0.0f )
	{
		// We need to lag the direction change in some cases - otherwise we get bounce effects - find the change in requested direction
		float fChangeAngle = MovementControllerUtilities::RotationAboutY( m_obLastRequestedDirection, obMovementInput.m_obMoveDirection );
	
		// Take the goal direction directly - calculate the change in angle from the last frame
		m_fRequestedDirectionChange = fChangeAngle;

		// Store the last requested direction
		m_obLastRequestedDirection = obMovementInput.m_obMoveDirection;
	}

	// Hack for AI rotating on spot
	if ( m_bAIRequestedRotation )
		m_obLastRequestedDirection = obMovementInput.m_obMoveDirection;

	// Just take a straight copy of the stuff we need for awareness
	m_obTargetPoint = obMovementInput.m_obTargetPoint;
	m_bTargetPointSet = obMovementInput.m_bTargetPointSet;
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::Update
//! The main update of the movement controller
//!
//------------------------------------------------------------------------------------------
bool WalkRunController::Update(	float						fTimeStep, 
								const CMovementInput&		obMovementInput,
								const CMovementStateRef&	obCurrentMovementState,
								CMovementState&				obPredictedMovementState)
{
	// Interpret our data input
	ProcessInputParameters( obMovementInput );

	// Render some debug stuff so we can see what we are doing
	// g_VisualDebug->RenderLine( obCurrentMovementState.m_obPosition + CPoint( 0.0f, 0.5f, 0.0f ), obCurrentMovementState.m_obPosition + CPoint( obCurrentMovementState.m_obFacing ) + CPoint( 0.0f, 0.5f, 0.0f ), 0xffff0000 );
	// g_VisualDebug->RenderLine( obCurrentMovementState.m_obPosition + CPoint( 0.0f, 0.5f, 0.0f ), obCurrentMovementState.m_obPosition + CPoint( m_obLastRequestedDirection ) + CPoint( 0.0f, 0.5f, 0.0f ), 0xff0000ff );

	// Update our current state time
	m_fStateTime += fTimeStep;

	// If this is the first frame initialise our velocity from that requested by the last controller
	if( m_bFirstFrame )
	{
		// Get our current speed from the last requested velocity - since this movement controller
		// only moves in the facing direction we only take the component in that direction
		CDirection obVelocity = obCurrentMovementState.m_obLastRequestedVelocity;
		obVelocity.Y() = 0.0f;
		m_fCurrentSpeed = obVelocity.Dot( obCurrentMovementState.m_obFacing );
		// ntPrintf( "starting speed %f \n", m_fCurrentSpeed );

		// Check the bounds here
		if ( m_fCurrentSpeed > m_fRunSpeed )
			m_fCurrentSpeed = m_fRunSpeed;

		// We need to choose our starting state
		InitialiseState( obCurrentMovementState );
	}

	
	UpdateBlinking( fTimeStep );


	// Sort our state entrance and exit conditions
	switch( m_eMovementState )
	{
	case MS_IDLE:			UpdateIdle( fTimeStep, obCurrentMovementState );		break;
	case MS_TO_AWARE:		UpdateToAware( fTimeStep, obCurrentMovementState );		break;
	case MS_AWARE:			UpdateAware( fTimeStep, obCurrentMovementState );		break;
	case MS_FROM_AWARE:		UpdateFromAware( fTimeStep, obCurrentMovementState );	break;
	case MS_ROTATE_ON_SPOT:	UpdateRotateOnSpot( fTimeStep, obCurrentMovementState );	break;
	case MS_STARTING:		UpdateStarting( fTimeStep, obCurrentMovementState );	break;
	case MS_STOPPING:		UpdateStopping( fTimeStep, obCurrentMovementState );	break;
	case MS_MOVING:			UpdateMoving( fTimeStep, obCurrentMovementState );		break;
	case MS_TURNING:		UpdateTurning( fTimeStep, obCurrentMovementState );		break;
	default:				ntAssert( 0 );											break;
	}

	// Update the phase flag...
	m_fPreviousPhase = m_pobAnimator->GetPhasePosition();

	// Deal with the blending between state animations
	UpdateStateWeights( fTimeStep );

	// Calculate the requested turn amount
	float fRequestedTurnAmount = MovementControllerUtilities::RotationAboutY( obCurrentMovementState.m_obFacing, m_obLastRequestedDirection );

	// What is the maximum rotation i can make this frame?
	float fMaxRotation = m_obDefinition.m_fMaxTurnSpeed * DEG_TO_RAD_VALUE * fTimeStep;

	// We ensure that the change in direction is in the same direction as the change in pad input
	if ( ( m_fRequestedSpeed > 0.0f ) && ( fabsf( m_fRequestedDirectionChange ) > fMaxRotation ) && ( ( m_fRequestedDirectionChange * fRequestedTurnAmount ) < 0.0f ) )
		fRequestedTurnAmount = ( m_fRequestedDirectionChange > 0.0f ) ? fMaxRotation : -fMaxRotation;

	// Cap my turn amount accordingly
	float fNewTurnAmount = clamp( fRequestedTurnAmount, -fMaxRotation, fMaxRotation );

	// Update our clamped turn time if we cannot turn as much as we would like
	if ( fMaxRotation == fabsf( fNewTurnAmount ) )
		m_fTurnClampDuration += fTimeStep;
	else
		m_fTurnClampDuration = 0.0f;

	// Smooth the turn amount each frame - lerp 30% at the standard frame rate
	const float fLerpTurnAmount = clamp( ( 0.7f * ( fTimeStep / ( 1.0f / 30.0f ) ) ), 0.0f, 1.0f );
	m_fTurnAmount = CMaths::Lerp( m_fTurnAmount, fNewTurnAmount, fLerpTurnAmount );

	// Calculate the turn appearance - we use a squared curve
	if ( m_bCanTurn )
	{
		float fNewTurnAppearance = ( m_fTurnAmount / fMaxRotation ) * ( m_fTurnAmount / fMaxRotation );
		fNewTurnAppearance = ( m_fTurnAmount < 0.0f ) ? -fNewTurnAppearance : fNewTurnAppearance;
		const float fLerpTurnAppearance = clamp( ( 0.1f * ( fTimeStep / ( 1.0f / 30.0f ) ) ), 0.0f, 1.0f );
		m_fTurningAppearance = CMaths::Lerp( m_fTurningAppearance, fNewTurnAppearance, fLerpTurnAppearance );

		//if ( m_eMovementState == MS_MOVING )
			//ntPrintf( "Turn appearance - %f\n", m_fTurningAppearance );
	}

	// Calculate my new speed - change in speed so acceleration * fTimeDelta NOT SQUARED
	const float fLerpCurrentSpeed = clamp( (  0.2f * ( fTimeStep / ( 1.0f / 30.0f ) ) ), 0.0f, 1.0f );
	m_fCurrentSpeed = CMaths::Lerp( m_fCurrentSpeed, m_fRequestedSpeed, fLerpCurrentSpeed );

	// Calculate our acceleration apperarance
	if ( m_bCanAccelerate )
		m_fAccelerationAppearance = CalculateAccelerationAppearance( fTimeStep );

	// Calculate the awareness appearance if necessary
	if ( m_bCanBeAware && m_bTargetPointSet )
	{
		float fTargetDistance = ( m_obTargetPoint - obCurrentMovementState.m_obPosition ).Length();
		m_fAwarenessWeight = clamp( fTargetDistance / m_obDefinition.m_fMaxAwareDistance, 0.0f, 1.0f );
	}

	// Based on all we have calculated set the blend weights on all our animations
	UpdateAnimationBlendWeights( fTimeStep, m_aobIdleAnims,
											m_aobAwareAnims,
											m_aobToAwareAnims,
											m_aobFromAwareAnims,
											m_aobRotateOnSpotAnims,
											m_aobStartingAnims,
											m_aobStoppingAnims,
											m_aobMovingAnims,
											m_aobTurningAnims );
	
	// Set up the predicted root delta and rotation delta
	if ( UseAnimatedTurn() )
		obPredictedMovementState.m_fProceduralYaw = ( m_fAnimTurnAmount - m_fLastAnimTurnAmount );
	else if ( !CanTurn() )
		obPredictedMovementState.m_fProceduralYaw = 0.0f;
	else if ( m_pobMovement->GetParentEntity()->IsAI() && m_bAIRequestedRotation )
		obPredictedMovementState.m_fProceduralYaw = fNewTurnAmount;
	else
		obPredictedMovementState.m_fProceduralYaw = m_fTurnAmount;

	// Kill the animated root rotation
	obPredictedMovementState.m_fRootRotationDeltaScalar = 0.0f;

	// Always add gravity
	ApplyGravity( true );

	// And we're done!
	m_bFirstFrame = false;

	

	// Never indicate that we have finished
	return false;
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::UseAnimatedTurn
//! Should we be using turn values generated from aniamtions
//!
//------------------------------------------------------------------------------------------
bool WalkRunController::UseAnimatedTurn( void ) const
{
	return ( ( m_eMovementState == MS_TURNING ) || ( m_eMovementState == MS_STARTING ) );
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::CanTurn
//! Can the character turn based on their current state
//!
//------------------------------------------------------------------------------------------
bool WalkRunController::CanTurn( void ) const
{
	// Cannot turn if we are about to execute a quick turn
	if ( m_bQuickTurn )
		return false;
	
	// Cannot turn in any of the following states
	if ( ( m_eMovementState == MS_IDLE )
		 ||
		 ( m_eMovementState == MS_AWARE )
		 ||
		 ( m_eMovementState == MS_TO_AWARE )
		 ||
		 ( m_eMovementState == MS_FROM_AWARE ) )
	{
		 return false;
	}

	// Otherwise we can
	return true;
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::SetCappedValue
//! Make a capped change to a value - may be different signs
//!
//------------------------------------------------------------------------------------------
float WalkRunController::SetCappedValue( float fCurrentValue, float fNewValue, float fAllowedChange )
{
	// Find the desired change
	float fDesiredChange = fNewValue - fCurrentValue;

	if( fDesiredChange >= 0.0f )
		return fCurrentValue + min( fAllowedChange, fDesiredChange );
	else
		return fCurrentValue - min( fAllowedChange, fabsf( fDesiredChange ) );
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::CalculateAccelerationAppearance
//! Calculate the acceleration appearance - peaks half way between the full walk and run
//!
//------------------------------------------------------------------------------------------
float WalkRunController::CalculateAccelerationAppearance( float fTimeStep )
{
	// If we are currently in the walking gait - no acceleration
	if ( m_fCurrentSpeed <= m_fWalkSpeed )
		return 0.0f;

	// If the requested speed is lower than the current speed - no acceleration
	if ( m_fRequestedSpeed <= m_fCurrentSpeed )
		return 0.0f;

	// Calculate the walk / run mid point
	float fAccFallOffPoint = ( ( ( m_fRunSpeed - m_fWalkSpeed ) / 4.0f ) * 3.0f ) + m_fWalkSpeed;

	if( m_fCurrentSpeed < fAccFallOffPoint )
		return 1.0f;

	return 1.0f - CMaths::SmoothStep( (m_fCurrentSpeed - fAccFallOffPoint) / (m_fRunSpeed - fAccFallOffPoint) );

	/*
	return 1.0f;

	// How much do we lerp the acceleration by
	const float fLerpAcceleration = 1.0f; //clamp( ( 0.1f * ( fTimeStep / ( 1.0f / 30.0f ) ) ), 0.0f, 1.0f );

	// If we are passed the mid point
	if ( m_fCurrentSpeed > fWalkRunMidPoint )
		return CMaths::Lerp( m_fAccelerationAppearance, 1.0f - ( ( m_fCurrentSpeed - fWalkRunMidPoint ) / ( m_fRunSpeed - fWalkRunMidPoint ) ), fLerpAcceleration );

	// Otherwise...
	else
		return CMaths::Lerp( m_fAccelerationAppearance, ( ( m_fCurrentSpeed - m_fWalkSpeed ) / ( fWalkRunMidPoint - m_fWalkSpeed ) ), fLerpAcceleration );
	*/
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::UpdateRunSpeed
//! Speed up the run animation if it sits over a certain threshold
//!
//------------------------------------------------------------------------------------------
void WalkRunController::UpdateRunSpeed(float fTimeDelta )
{
	float fSpeedRange = m_obDefinition.m_fRunAnimMaxSpeed - m_obDefinition.m_fRunAnimMinSpeed;
	float fDesiredRunSpeed = ((m_fCurrentSpeed - m_fWalkSpeed) / (m_fRunSpeed - m_fWalkSpeed)) * fSpeedRange + m_obDefinition.m_fRunAnimMinSpeed;

	// What speed should we play the run animation at
	float fRunSpeed = m_aobMovingAnims[MA_RUN]->GetSpeed();

	if (m_obDefinition.m_fRunAccelerationTime == 0.0f)
	{
		fRunSpeed = fDesiredRunSpeed;
	}
	else
	{
		float fSpeedDelta = fSpeedRange * (fTimeDelta / m_obDefinition.m_fRunAccelerationTime);

		// Move current speed closer to its desired amount.
		if (fabs(fRunSpeed - fDesiredRunSpeed) <= fSpeedDelta)
			fRunSpeed = fDesiredRunSpeed;
		else if (fRunSpeed < fDesiredRunSpeed)
			fRunSpeed += fSpeedDelta;
		else
			fRunSpeed -= fSpeedDelta;
	}

	// Check the bounds
	fRunSpeed = clamp(fRunSpeed, m_obDefinition.m_fRunAnimMinSpeed, m_obDefinition.m_fRunAnimMaxSpeed);
	
	// Set the speed on the animation
	m_aobMovingAnims[MA_RUN]->SetSpeed( fRunSpeed );
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::UpdateAnimationBlendWeights
//! Set the blend weights on individual animations based on calculated parameters
//!
//------------------------------------------------------------------------------------------
void WalkRunController::UpdateAnimationBlendWeights(	float			fTimeDelta,
														CAnimationPtr*	aobIdleAnims,
														CAnimationPtr*	aobAwareAnims,
														CAnimationPtr*	aobToAwareAnims,
														CAnimationPtr*	aobFromAwareAnims,
														CAnimationPtr*  aobRotateOnSpotAnims,
														CAnimationPtr*	aobStartingAnims,
														CAnimationPtr*	aobStoppingAnims,
														CAnimationPtr*	aobMovingAnims,
														CAnimationPtr*	aobTurningAnims )
{
	// If we have some idle weight update the single animation
	if ( m_fStateWeights[MS_IDLE] > 0.0f )
		aobIdleAnims[IA_STAND]->SetBlendWeight( m_fBlendWeight * m_fStateWeights[MS_IDLE] );

	// If we have some 'to-aware' weight update the single animation
	if ( m_fStateWeights[MS_TO_AWARE] > 0.0f )
		aobToAwareAnims[TAA_START]->SetBlendWeight( m_fBlendWeight * m_fStateWeights[MS_TO_AWARE] );

	// If we have some awareness then set the blends there
	if ( m_fStateWeights[MS_AWARE] > 0.0f )
	{
		aobAwareAnims[AA_MIN]->SetBlendWeight( m_fBlendWeight * m_fStateWeights[MS_AWARE] * m_fAwarenessWeight );
		aobAwareAnims[AA_MAX]->SetBlendWeight( m_fBlendWeight * m_fStateWeights[MS_AWARE] * ( 1.0f - m_fAwarenessWeight ) );
	}

	// If we have some 'from-aware' weight update the single animation
	if ( m_fStateWeights[MS_FROM_AWARE] > 0.0f )
		aobFromAwareAnims[FAA_FROM]->SetBlendWeight( m_fBlendWeight * m_fStateWeights[MS_FROM_AWARE] );

	// If we have some 'rotate on spot' weight update the single animation
	if ( m_fStateWeights[MS_ROTATE_ON_SPOT] > 0.0f )
		aobRotateOnSpotAnims[RSA_IDLE]->SetBlendWeight( m_fBlendWeight * m_fStateWeights[MS_ROTATE_ON_SPOT] );

	// If we have some starting weight update the chosen animation
	if ( m_fStateWeights[MS_STARTING] > 0.0f )
	{
		// Set the blend weights
		if ( m_bCanRun )
			aobStartingAnims[m_eChosenStartRun]->SetBlendWeight( m_fBlendWeight * m_fStateWeights[MS_STARTING] * m_fRunStartWeight );
		if ( m_bCanWalk )
			aobStartingAnims[m_eChosenStartWalk]->SetBlendWeight( m_fBlendWeight * m_fStateWeights[MS_STARTING] * m_fWalkStartWeight );
		if ( m_bCanSlowWalk )
			aobStartingAnims[m_eChosenStartSlowWalk]->SetBlendWeight( m_fBlendWeight * m_fStateWeights[MS_STARTING] * m_fSlowWalkStartWeight );
	}

	// If we have some stopping weight update the chosen animation
	if ( m_fStateWeights[MS_STOPPING] > 0.0f )
		aobStoppingAnims[m_eChosenStop]->SetBlendWeight( m_fBlendWeight * m_fStateWeights[MS_STOPPING] );

	// If we have any overall weight for the moving animations
	if ( m_fStateWeights[MS_MOVING] > 0.0f )
	{
		// Check the quality of our blend parameters
		ntAssert(		(m_fCurrentSpeed <= m_fRunSpeed && m_bCanRun)
				||	(m_fCurrentSpeed <= m_fWalkSpeed && m_bCanWalk)
				||	(m_fCurrentSpeed <= m_fSlowWalkSpeed && m_bCanSlowWalk) );
		ntAssert( m_fAccelerationAppearance <= 1.0f );

		// Precalculate a few values - turning
		float fRightTurnAppearance = fabsf( clamp( m_fTurningAppearance, -1.0f, 0.0f ) );
		float fLeftTurnAppearance = clamp( m_fTurningAppearance, 0.0f, 1.0f );
		float fNoTurnAppearance = 1.0f - fabsf( m_fTurningAppearance );

		// Acceleration
		float fAccelerationAppearance = m_fAccelerationAppearance;
		float fNoAccelerationAppearance = 1.0f - fAccelerationAppearance;

		// Speed - a bit more complicated because we have three 'layers' rather than one 
		// Cope with possible divide by zeros with a nice nested ?? stuff - Grrrrrrrreat
		float fSlowWalkAppearance = ( m_fWalkSpeed <= m_fSlowWalkSpeed ) ? 0.0f : ( ( m_fCurrentSpeed <= m_fWalkSpeed ) ? ( 1.0f - ( ( m_fCurrentSpeed - m_fSlowWalkSpeed ) / ( m_fWalkSpeed - m_fSlowWalkSpeed ) ) ) : 0.0f );

		// If the current speed is less than the slowwalk speed we can't stop - we must do the slow walk
		if ( m_fCurrentSpeed < m_fSlowWalkSpeed )
			fSlowWalkAppearance = 1.0f;

		float fRunAppearance = ( m_fRunSpeed <= m_fWalkSpeed ) ? 0.0f : ( ( m_fCurrentSpeed >= m_fWalkSpeed ) ? ( ( m_fCurrentSpeed - m_fWalkSpeed ) / ( m_fRunSpeed - m_fWalkSpeed ) ) : 0.0f );
		float fWalkAppearance = 1.0f - fRunAppearance - fSlowWalkAppearance;

		// Calculate the total blend weight for the movement animations
		float fMovingBlendWeight = m_fBlendWeight * m_fStateWeights[MS_MOVING];

		// Set the blend weights on the moving animations
		if ( m_bCanSlowWalk )
			aobMovingAnims[MA_SLOWWALK]->SetBlendWeight( fMovingBlendWeight * fSlowWalkAppearance * fNoTurnAppearance * fNoAccelerationAppearance );

		if ( m_bCanSlowWalk && m_bCanTurn )
		{
			aobMovingAnims[MA_SLOWWALKLEFT]->SetBlendWeight( fMovingBlendWeight * fSlowWalkAppearance * fLeftTurnAppearance * fNoAccelerationAppearance );
			aobMovingAnims[MA_SLOWWALKRIGHT]->SetBlendWeight( fMovingBlendWeight * fSlowWalkAppearance * fRightTurnAppearance * fNoAccelerationAppearance );
		}

		if ( m_bCanWalk )
			aobMovingAnims[MA_WALK]->SetBlendWeight( fMovingBlendWeight * fWalkAppearance * fNoTurnAppearance * fNoAccelerationAppearance );

		if ( m_bCanWalk && m_bCanTurn )
		{
			aobMovingAnims[MA_WALKLEFT]->SetBlendWeight( fMovingBlendWeight * fWalkAppearance * fLeftTurnAppearance *	fNoAccelerationAppearance );
			aobMovingAnims[MA_WALKRIGHT]->SetBlendWeight( fMovingBlendWeight * fWalkAppearance * fRightTurnAppearance * fNoAccelerationAppearance );
		}

		if ( m_bCanRun )
		{
			// Calculate the weight of our run animation
			float fRunAnimationWeight = fRunAppearance * fNoTurnAppearance * fNoAccelerationAppearance;
			aobMovingAnims[MA_RUN]->SetBlendWeight( fMovingBlendWeight * fRunAnimationWeight );

			// Update the speed of this animation based on the weight
			UpdateRunSpeed(fTimeDelta);
		}

		if ( m_bCanRun && m_bCanTurn )
		{
			aobMovingAnims[MA_RUNLEFT]->SetBlendWeight( fMovingBlendWeight * fRunAppearance * fLeftTurnAppearance * fNoAccelerationAppearance );
			aobMovingAnims[MA_RUNRIGHT]->SetBlendWeight( fMovingBlendWeight * fRunAppearance * fRightTurnAppearance * fNoAccelerationAppearance );
		}

		if ( m_bCanAccelerate )
		{
			aobMovingAnims[MA_ACCELRUN]->SetBlendWeight( fMovingBlendWeight * 1.0f * fNoTurnAppearance * fAccelerationAppearance );
			aobMovingAnims[MA_ACCELRUNLEFT]->SetBlendWeight( fMovingBlendWeight * 1.0f * fLeftTurnAppearance * fAccelerationAppearance );
			aobMovingAnims[MA_ACCELRUNRIGHT]->SetBlendWeight( fMovingBlendWeight * 1.0f * fRightTurnAppearance * fAccelerationAppearance );
		}
	}

	// If we have some turning weight update the chosen animation
	if ( m_fStateWeights[MS_TURNING] > 0.0f )
		aobTurningAnims[m_eChosenTurn]->SetBlendWeight( m_fBlendWeight * m_fStateWeights[MS_TURNING] );
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::JustPassedPhaseBoundary
//! Helper to find whether we are at a good point to change sub state
//!
//------------------------------------------------------------------------------------------
bool WalkRunController::JustPassedPhaseBoundary( void ) const
{
	// Check if we have just passed the end or half way point of a phase
	if ( ( m_fPreviousPhase < 0.5f && m_pobAnimator->GetPhasePosition() > 0.5f )
		 ||
		 ( m_fPreviousPhase > m_pobAnimator->GetPhasePosition() ) )
	{
		return true;
	}

	// If we are here we haven't just moved into a new 'cycle'
	return false;
}


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController::UpdateBlinking
//! Guess what this does...
//!
//------------------------------------------------------------------------------------------
void WalkRunController::UpdateBlinking( float fTimeStep )
{
	if ( m_bCanBlink )
	{
		m_fTimeSinceLastBlink += fTimeStep;
		
		// force blend weight if still playing the blinking animation
		if ( m_pobBlinkAnim->GetTime() < m_pobBlinkAnim->GetDuration() )
		{
			m_pobBlinkAnim->SetBlendWeight( 1.0f );
		}
		else
		{
			// if not, make sure is set to zero
			m_pobBlinkAnim->SetBlendWeight( 0.0f );
			
			// if it's time to blink again, reset the anim so its weight can be forced back to one 
			// and reset the timer
			if ( m_fTimeSinceLastBlink > m_obDefinition.m_fBlinkInterval )
			{
				m_pobBlinkAnim->SetTime( 0.0f );
				m_fTimeSinceLastBlink = m_obDefinition.m_fBlinkInterval * rand() * BLINK_INTERVAL_MULTIPLIER;
			}
		}
	}
}


//void WalkRunController::UpdateBlinking( float fTimeStep )
//{
//	const CHashedString& obBlinkAnimName = m_obDefinition.m_pobAnimSet->m_obBlink;
//
//	// only update if we have a bliking anim set
//	if ( !obBlinkAnimName.IsNull() )
//	{
//		// always update time
//		m_fTimeSinceLastBlink += fTimeStep;
//
//		// check if it's time to blink. If so, add the animation if not still playing it from the last blink
//		if ( m_fTimeSinceLastBlink > m_obDefinition.m_fBlinkInterval && !m_pobAnimator->IsPlayingAnimation( obBlinkAnimName.GetHash() ) )
//		{
//			CAnimationPtr pobBlinkAnim =  m_pobAnimator->CreateAnimation( obBlinkAnimName );
//			pobBlinkAnim->SetBlendWeight( 1.0f );
//			m_pobAnimator->AddAnimation( pobBlinkAnim );
//
//			// finally, reset our timer and add a little randomness to it
//			m_fTimeSinceLastBlink = m_obDefinition.m_fBlinkInterval * rand() * BLINK_INTERVAL_MULTIPLIER;
//		}
//	}
//}
