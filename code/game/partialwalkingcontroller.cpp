//------------------------------------------------------------------------------------------
//!
//!	\file partialwalkingcontroller.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "partialwalkingcontroller.h"
#include "objectdatabase/dataobject.h"
#include "anim/animator.h"
#include "anim/animation.h"
#include "core/exportstruct_anim.h"

// Interfaces
START_STD_INTERFACE(SimplePartialAnimSet)
	PUBLISH_VAR_AS( m_obSingleAnimation, SingleAnimation )
END_STD_INTERFACE

START_STD_INTERFACE(SimplePartialWalkRunDef)
	PUBLISH_PTR_AS( m_pobAnimSet, AnimSet )
	PUBLISH_VAR_AS( m_fDeadZoneMaximum, DeadZoneMaximum )
	PUBLISH_VAR_AS( m_fWalkZoneAnalogue, WalkZoneAnalogue )
	PUBLISH_VAR_AS( m_fWalkZonePlateau, WalkZonePlateau )
	PUBLISH_VAR_AS( m_fMaxAwareDistance, MaxAwareDistance )
	PUBLISH_VAR_AS( m_fMaxTurnSpeed, MaxTurnSpeed )
	PUBLISH_VAR_AS( m_fRunAnimMaxSpeed, RunAnimMaxSpeed )
	PUBLISH_VAR_AS( m_fRunAccelerationTime, RunAccelerationTime )
	PUBLISH_PTR_AS( m_pobSimplePartialAnimSet, SimplePartialAnimSet )
END_STD_INTERFACE

START_STD_INTERFACE(Partial2AnimSet)
	PUBLISH_VAR_AS( m_obStanding, Standing )
	PUBLISH_VAR_AS( m_obStandToSlowWalk, StandToSlowWalk )
	PUBLISH_VAR_AS( m_obStandToSlowWalkLeft, StandToSlowWalkLeft )
	PUBLISH_VAR_AS( m_obStandToSlowWalkRight, StandToSlowWalkRight )
	PUBLISH_VAR_AS( m_obStandToWalk, StandToWalk )
	PUBLISH_VAR_AS( m_obStandToWalkLeft, StandToWalkLeft )
	PUBLISH_VAR_AS( m_obStandToWalkRight, StandToWalkRight )
	PUBLISH_VAR_AS( m_obStandToAccel, StandToAccel )
	PUBLISH_VAR_AS( m_obStandToAccelLeft, StandToAccelLeft )
	PUBLISH_VAR_AS( m_obStandToAccelRight, StandToAccelRight )
	PUBLISH_VAR_AS( m_obSlowWalkToStandLeft, SlowWalkToStandLeft )
	PUBLISH_VAR_AS( m_obSlowWalkToStandRight, SlowWalkToStandRight )
	PUBLISH_VAR_AS( m_obWalkToStandLeft, WalkToStandLeft )
	PUBLISH_VAR_AS( m_obWalkToStandRight, WalkToStandRight )
	PUBLISH_VAR_AS( m_obRunToStandLeft, RunToStandLeft )
	PUBLISH_VAR_AS( m_obRunToStandRight, RunToStandRight )
	PUBLISH_VAR_AS( m_obSlowWalk, SlowWalk )
	PUBLISH_VAR_AS( m_obWalk, Walk )
	PUBLISH_VAR_AS( m_obRun, Run )
	PUBLISH_VAR_AS( m_obAccelRun, AccelRun )
	PUBLISH_VAR_AS( m_obFullTurnRight, FullTurnRight )
	PUBLISH_VAR_AS( m_obFullTurnLeft, FullTurnLeft )
END_STD_INTERFACE

START_STD_INTERFACE(Partial2WalkRunDef)
	PUBLISH_PTR_AS( m_pobAnimSet, AnimSet )
	PUBLISH_VAR_AS( m_fDeadZoneMaximum, DeadZoneMaximum )
	PUBLISH_VAR_AS( m_fWalkZoneAnalogue, WalkZoneAnalogue )
	PUBLISH_VAR_AS( m_fWalkZonePlateau, WalkZonePlateau )
	PUBLISH_VAR_AS( m_fMaxAwareDistance, MaxAwareDistance )
	PUBLISH_VAR_AS( m_fMaxTurnSpeed, MaxTurnSpeed )
	PUBLISH_VAR_AS( m_fRunAnimMaxSpeed, RunAnimMaxSpeed )
	PUBLISH_VAR_AS( m_fRunAccelerationTime, RunAccelerationTime )
	PUBLISH_PTR_AS( m_pobPartial2AnimSet, Partial2AnimSet )
END_STD_INTERFACE

START_STD_INTERFACE(Partial3AnimSet)
	PUBLISH_VAR_AS( m_obStanding, Standing )
	PUBLISH_VAR_AS( m_obStandToWalk, m_obStandToWalk )
	PUBLISH_VAR_AS( m_obStandToAccel, m_obStandToAccel )
	PUBLISH_VAR_AS( m_obStandToAccelLeft, m_obStandToAccelLeft )
	PUBLISH_VAR_AS( m_obStandToAccelRight, m_obStandToAccelRight )
	PUBLISH_VAR_AS( m_obWalkToStandLeft, m_obWalkToStandLeft )
	PUBLISH_VAR_AS( m_obWalkToStandRight, m_obWalkToStandRight )
	PUBLISH_VAR_AS( m_obRunToStandLeft, m_obRunToStandLeft )
	PUBLISH_VAR_AS( m_obRunToStandRight, m_obRunToStandRight )
	PUBLISH_VAR_AS( m_obWalk, m_obWalk )
	PUBLISH_VAR_AS( m_obRun, m_obRun )
	PUBLISH_VAR_AS( m_obAccelRun, m_obAccelRun )
	PUBLISH_VAR_AS( m_obFullTurnRight, m_obFullTurnRight )
	PUBLISH_VAR_AS( m_obFullTurnLeft, m_obFullTurnLeft )
END_STD_INTERFACE

START_STD_INTERFACE(Partial3WalkRunDef)
	PUBLISH_PTR_AS( m_pobAnimSet, AnimSet )
	PUBLISH_VAR_AS( m_fDeadZoneMaximum, DeadZoneMaximum )
	PUBLISH_VAR_AS( m_fWalkZoneAnalogue, WalkZoneAnalogue )
	PUBLISH_VAR_AS( m_fWalkZonePlateau, WalkZonePlateau )
	PUBLISH_VAR_AS( m_fMaxAwareDistance, MaxAwareDistance )
	PUBLISH_VAR_AS( m_fMaxTurnSpeed, MaxTurnSpeed )
	PUBLISH_VAR_AS( m_fRunAnimMaxSpeed, RunAnimMaxSpeed )
	PUBLISH_VAR_AS( m_fRunAccelerationTime, RunAccelerationTime )
	PUBLISH_PTR_AS( m_pobPartial2AnimSet, Partial2AnimSet )
	PUBLISH_PTR_AS( m_pobPartial3AnimSet, Partial3AnimSet )
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
//!
//!	SimplePartialWalkRunDef::SimplePartialWalkRunDef
//!	Construction
//!
//------------------------------------------------------------------------------------------
SimplePartialWalkRunDef::SimplePartialWalkRunDef( void )
:	m_pobSimplePartialAnimSet( 0 )
{
}


//------------------------------------------------------------------------------------------
//!
//!	SimplePartialWalkRunDef::CreateInstance
//!	Create an instance of what we describe
//!
//------------------------------------------------------------------------------------------
MovementController* SimplePartialWalkRunDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) SimplePartialWalkRun( pobMovement, *this );
}


//------------------------------------------------------------------------------------------
//!
//!	SimplePartialWalkRun::SimplePartialWalkRun
//!	Construction
//!
//------------------------------------------------------------------------------------------
SimplePartialWalkRun::SimplePartialWalkRun( CMovement* pobMovement, const SimplePartialWalkRunDef& obDefinition )
:	WalkRunController( pobMovement, obDefinition ),
	m_obSimplePartialDefinition( obDefinition )
{
	// Check that we have the bare minimum of animations to make this work
	if ( m_obSimplePartialDefinition.m_pobSimplePartialAnimSet->m_obSingleAnimation.IsNull() )
	{
		user_warn_p( 0, ( "The simple partial animation controller doesn't have a valid animation" ) );
	}

	// Otherwise go ahead and set up our animation
	else
	{
		m_obSingleAnimation = CreateAnimation( m_obSimplePartialDefinition.m_pobSimplePartialAnimSet->m_obSingleAnimation, ANIMF_LOOPING );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	SimplePartialWalkRun::SimplePartialWalkRun
//!	Destruction
//!
//------------------------------------------------------------------------------------------
SimplePartialWalkRun::~SimplePartialWalkRun( void )
{
	// If we have passed the first frame we need to remove our animation
	if ( !m_bFirstFrame )
		DeactivateAnimations( &m_obSingleAnimation, 1 );
}


//------------------------------------------------------------------------------------------
//!
//!	SimplePartialWalkRun::Update
//!	
//!
//------------------------------------------------------------------------------------------
bool SimplePartialWalkRun::Update(	float						fTimeStep, 
								const CMovementInput&		obMovementInput,
								const CMovementStateRef&	obCurrentMovementState,
								CMovementState&				obPredictedMovementState )
{
	// Add our animation if we are on the first frame
	if ( m_bFirstFrame )
		ActivateAnimations( &m_obSingleAnimation, 1 );

	// Call the base functionality 
	WalkRunController::Update( fTimeStep, obMovementInput, obCurrentMovementState, obPredictedMovementState );

	// Set the blend weight on our animation
	m_obSingleAnimation->SetBlendWeight( m_fBlendWeight );

	// We never finish
	return false;
}


//------------------------------------------------------------------------------------------
//!
//!	Partial2WalkRunDef::Partial2WalkRunDef
//!	Construction
//!
//------------------------------------------------------------------------------------------
Partial2WalkRunDef::Partial2WalkRunDef( void )
:	m_pobPartial2AnimSet( 0 )
{
}


//------------------------------------------------------------------------------------------
//!
//!	Partial2WalkRunDef::CreateInstance
//!	Create an instance of what we describe
//!
//------------------------------------------------------------------------------------------
MovementController* Partial2WalkRunDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) Partial2WalkRun( pobMovement, *this );
}


//------------------------------------------------------------------------------------------
//!
//!	Partial2WalkRun::Partial2WalkRun
//!	Construction
//!
//------------------------------------------------------------------------------------------
Partial2WalkRun::Partial2WalkRun( CMovement* pobMovement, const Partial2WalkRunDef& obDefinition )
:	WalkRunController( pobMovement, obDefinition ),
	m_obPartial2Definition( obDefinition ),
	m_bPartial2MinimumSet( false )
{
	// Check that we have the bare minimum of animations to make this work
	if ( !m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding.IsNull() )
	{
		m_bPartial2MinimumSet = true;
	}
	else
	{
		user_warn_p( 0, ( "" ) );
	}

	// If we are capable of providing the animations
	if ( m_bPartial2MinimumSet )
	{
		// Create our idle animations - this is the only animation that we must have!
		m_aobPartial2IdleAnims[IA_STAND] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_LOOPING );

		// Rotate on spot uses the standard idle animation
		m_aobPartial2RotateOnSpotAnims[RSA_IDLE] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_LOOPING );

		// Create our aware animations if necessary
		if ( m_bCanBeAware )
		{
			m_aobPartial2AwareAnims[AA_MIN]			= CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_LOOPING );
			m_aobPartial2AwareAnims[AA_MAX]			= CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_LOOPING );
			m_aobPartial2ToAwareAnims[TAA_START]	= CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, 0 );
			m_aobPartial2FromAwareAnims[FAA_FROM]	= CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, 0 );
		}

		// Create our starting animations
		if ( m_bCanSlowWalk )
		{
			if ( !m_obPartial2Definition.m_pobPartial2AnimSet->m_obStandToSlowWalk.IsNull() )
				m_aobPartial2StartingAnims[STA_TOSLOWWALK] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStandToSlowWalk, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
			else
				m_aobPartial2StartingAnims[STA_TOSLOWWALK] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );

			if ( !m_obPartial2Definition.m_pobPartial2AnimSet->m_obStandToSlowWalkLeft.IsNull() )
			{
				m_aobPartial2StartingAnims[STA_TOSLOWWALKLEFT]		= CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStandToSlowWalkLeft, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET  );
				m_aobPartial2StartingAnims[STA_TOSLOWWALKBACKLEFT]	= CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStandToSlowWalkLeft, ANIMF_PHASE_LINKED );
			}
			else
			{
				m_aobPartial2StartingAnims[STA_TOSLOWWALKLEFT]		= CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET  );
				m_aobPartial2StartingAnims[STA_TOSLOWWALKBACKLEFT]	= CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_PHASE_LINKED );
			}

			if ( !m_obPartial2Definition.m_pobPartial2AnimSet->m_obStandToSlowWalkRight.IsNull() )
			{
				m_aobPartial2StartingAnims[STA_TOSLOWWALKRIGHT]		= CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStandToSlowWalkRight, ANIMF_PHASE_LINKED);
				m_aobPartial2StartingAnims[STA_TOSLOWWALKBACKRIGHT]	= CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStandToSlowWalkRight, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
			}
			else
			{
				m_aobPartial2StartingAnims[STA_TOSLOWWALKRIGHT]		= CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_PHASE_LINKED);
				m_aobPartial2StartingAnims[STA_TOSLOWWALKBACKRIGHT]	= CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
			}
		}

		if ( m_bCanWalk )
		{
			if ( !m_obPartial2Definition.m_pobPartial2AnimSet->m_obStandToWalk.IsNull() )
				m_aobPartial2StartingAnims[STA_TOWALK] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStandToWalk, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
			else
				m_aobPartial2StartingAnims[STA_TOWALK] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );

			if ( !m_obPartial2Definition.m_pobPartial2AnimSet->m_obStandToWalkLeft.IsNull() )
			{
				m_aobPartial2StartingAnims[STA_TOWALKLEFT]		= CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStandToWalkLeft, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET  );
				m_aobPartial2StartingAnims[STA_TOWALKBACKLEFT]	= CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStandToWalkLeft, ANIMF_PHASE_LINKED );
			}
			else
			{
				m_aobPartial2StartingAnims[STA_TOWALKLEFT]		= CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET  );
				m_aobPartial2StartingAnims[STA_TOWALKBACKLEFT]	= CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_PHASE_LINKED );
			}

			if ( !m_obPartial2Definition.m_pobPartial2AnimSet->m_obStandToWalkRight.IsNull() )
			{
				m_aobPartial2StartingAnims[STA_TOWALKRIGHT]		= CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStandToWalkRight, ANIMF_PHASE_LINKED);
				m_aobPartial2StartingAnims[STA_TOWALKBACKRIGHT]	= CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStandToWalkRight, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
			}
			else
			{
				m_aobPartial2StartingAnims[STA_TOWALKRIGHT]		= CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_PHASE_LINKED);
				m_aobPartial2StartingAnims[STA_TOWALKBACKRIGHT]	= CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
			}
		}

		if ( m_bCanRun )
		{
			if ( !m_obPartial2Definition.m_pobPartial2AnimSet->m_obStandToAccel.IsNull() )
				m_aobPartial2StartingAnims[STA_TOACCEL] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStandToAccel, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
			else
				m_aobPartial2StartingAnims[STA_TOACCEL] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );

			if ( !m_obPartial2Definition.m_pobPartial2AnimSet->m_obStandToAccelLeft.IsNull() )
			{
				m_aobPartial2StartingAnims[STA_TOACCELLEFT]		= CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStandToAccelLeft, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
				m_aobPartial2StartingAnims[STA_TOACCELBACKLEFT]	= CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStandToAccelLeft, ANIMF_PHASE_LINKED );
			}
			else
			{
				m_aobPartial2StartingAnims[STA_TOACCELLEFT]		= CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
				m_aobPartial2StartingAnims[STA_TOACCELBACKLEFT]	= CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_PHASE_LINKED );
			}

			if ( !m_obPartial2Definition.m_pobPartial2AnimSet->m_obStandToAccelRight.IsNull() )
			{
				m_aobPartial2StartingAnims[STA_TOACCELRIGHT]		= CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStandToAccelRight, ANIMF_PHASE_LINKED );
				m_aobPartial2StartingAnims[STA_TOACCELBACKRIGHT]	= CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStandToAccelRight, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
			}
			else
			{
				m_aobPartial2StartingAnims[STA_TOACCELRIGHT]		= CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_PHASE_LINKED );
				m_aobPartial2StartingAnims[STA_TOACCELBACKRIGHT]	= CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
			}
		}

		// Create our stopping animations
		if ( m_bCanSlowWalk )
		{
			if ( !m_obPartial2Definition.m_pobPartial2AnimSet->m_obSlowWalkToStandLeft.IsNull() )
				m_aobPartial2StoppingAnims[SPA_FROMSLOWWALKLEFT] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obSlowWalkToStandLeft, ANIMF_PHASE_LINKED );
			else
				m_aobPartial2StoppingAnims[SPA_FROMSLOWWALKLEFT] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_PHASE_LINKED );

			if ( !m_obPartial2Definition.m_pobPartial2AnimSet->m_obSlowWalkToStandRight.IsNull() )
				m_aobPartial2StoppingAnims[SPA_FROMSLOWWALKRIGHT] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obSlowWalkToStandRight, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
			else
				m_aobPartial2StoppingAnims[SPA_FROMSLOWWALKRIGHT] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
		}

		if ( m_bCanWalk )
		{
			if ( !m_obPartial2Definition.m_pobPartial2AnimSet->m_obWalkToStandLeft.IsNull() )
				m_aobPartial2StoppingAnims[SPA_FROMWALKLEFT] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obWalkToStandLeft, ANIMF_PHASE_LINKED );
			else
				m_aobPartial2StoppingAnims[SPA_FROMWALKLEFT] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_PHASE_LINKED );

			if ( !m_obPartial2Definition.m_pobPartial2AnimSet->m_obWalkToStandRight.IsNull() )
				m_aobPartial2StoppingAnims[SPA_FROMWALKRIGHT] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obWalkToStandRight, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
			else
				m_aobPartial2StoppingAnims[SPA_FROMWALKRIGHT] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
		}

		if ( m_bCanRun )
		{
			if ( !m_obPartial2Definition.m_pobPartial2AnimSet->m_obRunToStandLeft.IsNull() )
				m_aobPartial2StoppingAnims[SPA_FROMRUNLEFT] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obRunToStandLeft, ANIMF_PHASE_LINKED );
			else
				m_aobPartial2StoppingAnims[SPA_FROMRUNLEFT] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_PHASE_LINKED );

			if ( !m_obPartial2Definition.m_pobPartial2AnimSet->m_obRunToStandRight.IsNull() )
				m_aobPartial2StoppingAnims[SPA_FROMRUNRIGHT] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obRunToStandRight, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
			else
				m_aobPartial2StoppingAnims[SPA_FROMRUNRIGHT] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
		}

		// Create our moving animations
		if ( m_bCanSlowWalk )
		{
			if ( !m_obPartial2Definition.m_pobPartial2AnimSet->m_obWalk.IsNull() )
			{
				m_aobPartial2MovingAnims[MA_SLOWWALK] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obWalk, ANIMF_LOOPING|ANIMF_PHASE_LINKED );

				if ( m_bCanTurn )
				{
					m_aobPartial2MovingAnims[MA_SLOWWALKLEFT] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obWalk, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
					m_aobPartial2MovingAnims[MA_SLOWWALKRIGHT] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obWalk, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
				}
			}
			else
			{
				m_aobPartial2MovingAnims[MA_SLOWWALK] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
				
				if ( m_bCanTurn )
				{
					m_aobPartial2MovingAnims[MA_SLOWWALKLEFT] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
					m_aobPartial2MovingAnims[MA_SLOWWALKRIGHT] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
				}
			}
		}

		if ( m_bCanWalk )
		{
			if ( !m_obPartial2Definition.m_pobPartial2AnimSet->m_obWalk.IsNull() )
			{
				m_aobPartial2MovingAnims[MA_WALK] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obWalk, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
								
				if ( m_bCanTurn )
				{
					m_aobPartial2MovingAnims[MA_WALKLEFT] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obWalk, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
					m_aobPartial2MovingAnims[MA_WALKRIGHT] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obWalk, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
				}
			}
			else
			{
				m_aobPartial2MovingAnims[MA_WALK] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
								
				if ( m_bCanTurn )
				{
					m_aobPartial2MovingAnims[MA_WALKLEFT] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
					m_aobPartial2MovingAnims[MA_WALKRIGHT] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
				}
			}
		}

		if ( m_bCanRun )
		{
			if ( !m_obPartial2Definition.m_pobPartial2AnimSet->m_obRun.IsNull() )
			{
				m_aobPartial2MovingAnims[MA_RUN] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obRun, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
								
				if ( m_bCanTurn )
				{
					m_aobPartial2MovingAnims[MA_RUNLEFT] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obRun, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
					m_aobPartial2MovingAnims[MA_RUNRIGHT] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obRun, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
				}
			}
			else
			{
				m_aobPartial2MovingAnims[MA_RUN] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
								
				if ( m_bCanTurn )
				{
					m_aobPartial2MovingAnims[MA_RUNLEFT] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
					m_aobPartial2MovingAnims[MA_RUNRIGHT] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
				}
			}
		}

		// Create our accelerating animations
		if ( m_bCanAccelerate )
		{
			if ( !m_obPartial2Definition.m_pobPartial2AnimSet->m_obAccelRun.IsNull() )
			{
				m_aobPartial2MovingAnims[MA_ACCELRUN] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obAccelRun, ANIMF_LOOPING|ANIMF_PHASE_LINKED );

				if ( m_bCanTurn )
				{
					m_aobPartial2MovingAnims[MA_ACCELRUNLEFT] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obAccelRun, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
					m_aobPartial2MovingAnims[MA_ACCELRUNRIGHT] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obAccelRun, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
				}
			}
			else
			{
				m_aobPartial2MovingAnims[MA_ACCELRUN] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_LOOPING|ANIMF_PHASE_LINKED );

				if ( m_bCanTurn )
				{
					m_aobPartial2MovingAnims[MA_ACCELRUNLEFT] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
					m_aobPartial2MovingAnims[MA_ACCELRUNRIGHT] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
				}
			}
		}

		// Create our fast turn animations
		if ( m_bCanFastTurn )
		{
			if ( !m_obPartial2Definition.m_pobPartial2AnimSet->m_obFullTurnLeft.IsNull() )
				m_aobPartial2TurningAnims[TUA_LEFT] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obFullTurnLeft, ANIMF_PHASE_LINKED );
			else
				m_aobPartial2TurningAnims[TUA_LEFT] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_PHASE_LINKED );

			if ( !m_obPartial2Definition.m_pobPartial2AnimSet->m_obFullTurnRight.IsNull() )
				m_aobPartial2TurningAnims[TUA_RIGHT] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obFullTurnRight, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
			else
				m_aobPartial2TurningAnims[TUA_RIGHT] = CreateAnimation( m_obPartial2Definition.m_pobPartial2AnimSet->m_obStanding, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
		}
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial2WalkRun::Partial2WalkRun
//!	Destruction
//!
//------------------------------------------------------------------------------------------
Partial2WalkRun::~Partial2WalkRun( void )
{
	// If we are capable of providing the animations
	if ( m_bPartial2MinimumSet )
	{
		// Ensure all our animations are removed
		if ( m_fStateWeights[MS_IDLE] > 0.0f )
			WalkRunController::ClearIdle( m_aobPartial2IdleAnims );

		if ( m_fStateWeights[MS_STARTING] > 0.0f )
			WalkRunController::ClearStarting( m_aobPartial2StartingAnims );

		if ( m_fStateWeights[MS_TO_AWARE] > 0.0f )
			WalkRunController::ClearToAware( m_aobPartial2ToAwareAnims );

		if ( m_fStateWeights[MS_AWARE] > 0.0f )
			WalkRunController::ClearAware( m_aobPartial2AwareAnims );

		if ( m_fStateWeights[MS_FROM_AWARE] > 0.0f )
			WalkRunController::ClearFromAware( m_aobPartial2FromAwareAnims );

		if ( m_fStateWeights[MS_ROTATE_ON_SPOT] > 0.0f )
			WalkRunController::ClearRotateOnSpot( m_aobPartial2RotateOnSpotAnims );
			
		if ( m_fStateWeights[MS_STOPPING] > 0.0f )
			WalkRunController::ClearStopping( m_aobPartial2StoppingAnims );
		
		if ( m_fStateWeights[MS_MOVING] > 0.0f )
			WalkRunController::ClearMoving( m_aobPartial2MovingAnims );

		if ( m_fStateWeights[MS_TURNING] > 0.0f )
			WalkRunController::ClearTurning( m_aobPartial2TurningAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial2WalkRun::Update
//!	
//!
//------------------------------------------------------------------------------------------
bool Partial2WalkRun::Update(	float						fTimeStep, 
								const CMovementInput&		obMovementInput,
								const CMovementStateRef&	obCurrentMovementState,
								CMovementState&				obPredictedMovementState )
{
	// Call the base functionality first
	WalkRunController::Update( fTimeStep, obMovementInput, obCurrentMovementState, obPredictedMovementState );

	// If we are capable of providing the animations
	if ( m_bPartial2MinimumSet )
	{
		// Recall the animation blending code with our own animation set
		UpdateAnimationBlendWeights( fTimeStep, m_aobPartial2IdleAnims,
												m_aobPartial2AwareAnims,
												m_aobPartial2ToAwareAnims,
												m_aobPartial2FromAwareAnims,
												m_aobPartial2RotateOnSpotAnims,
												m_aobPartial2StartingAnims,
												m_aobPartial2StoppingAnims,
												m_aobPartial2MovingAnims,
												m_aobPartial2TurningAnims );
	}

	// We never finish
	return false;
}


//------------------------------------------------------------------------------------------
//!
//!	Partial2WalkRun::StartIdle
//!	
//!
//------------------------------------------------------------------------------------------
void Partial2WalkRun::StartIdle( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobIdleAnims )
{
	// Call the base functionality first
	WalkRunController::StartIdle( obCurrentMovementState, aobIdleAnims );

	// If we are capable of providing the animations...
	if ( m_bPartial2MinimumSet )
	{
		// ...then call it again with our own anims
		WalkRunController::StartIdle( obCurrentMovementState, m_aobPartial2IdleAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial2WalkRun::ClearIdle
//!	
//!
//------------------------------------------------------------------------------------------
void Partial2WalkRun::ClearIdle( CAnimationPtr* aobIdleAnims )
{
	// Call the base functionality first
	WalkRunController::ClearIdle( aobIdleAnims );

	// If we are capable of providing the animations...
	if ( m_bPartial2MinimumSet )
	{
		// ...then call it again with our own anims
		WalkRunController::ClearIdle( m_aobPartial2IdleAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial2WalkRun::StartAware
//!	
//!
//------------------------------------------------------------------------------------------
void Partial2WalkRun::StartAware( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobAwareAnims )
{
	// Call the base functionality first
	WalkRunController::StartAware( obCurrentMovementState, aobAwareAnims );

	// If we are capable of providing the animations...
	if ( m_bPartial2MinimumSet )
	{
		// ...then call it again with our own anims
		WalkRunController::StartAware( obCurrentMovementState, m_aobPartial2AwareAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial2WalkRun::ClearAware
//!	
//!
//------------------------------------------------------------------------------------------
void Partial2WalkRun::ClearAware( CAnimationPtr* aobAwareAnims )
{
	// Call the base functionality first
	WalkRunController::ClearAware( aobAwareAnims );

	// If we are capable of providing the animations...
	if ( m_bPartial2MinimumSet )
	{
		// ...then call it again with our own anims
		WalkRunController::ClearAware( m_aobPartial2AwareAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial2WalkRun::StartToAware
//!	
//!
//------------------------------------------------------------------------------------------
void Partial2WalkRun::StartToAware( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobToAwareAnims )
{
	// Call the base functionality first
	WalkRunController::StartToAware( obCurrentMovementState, aobToAwareAnims );

	// If we are capable of providing the animations...
	if ( m_bPartial2MinimumSet )
	{
		// ...then call it again with our own anims
		WalkRunController::StartToAware( obCurrentMovementState, m_aobPartial2ToAwareAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial2WalkRun::ClearToAware
//!	
//!
//------------------------------------------------------------------------------------------
void Partial2WalkRun::ClearToAware( CAnimationPtr* aobToAwareAnims )
{
	// Call the base functionality first
	WalkRunController::ClearToAware( aobToAwareAnims );

	// If we are capable of providing the animations...
	if ( m_bPartial2MinimumSet )
	{
		// ...then call it again with our own anims
		WalkRunController::ClearToAware( m_aobPartial2ToAwareAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial2WalkRun::StartFromAware
//!	
//!
//------------------------------------------------------------------------------------------
void Partial2WalkRun::StartFromAware( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobFromAwareAnims )
{
	// Call the base functionality first
	WalkRunController::StartFromAware( obCurrentMovementState, aobFromAwareAnims );

	// If we are capable of providing the animations...
	if ( m_bPartial2MinimumSet )
	{
		// ...then call it again with our own anims
		WalkRunController::StartFromAware( obCurrentMovementState, m_aobPartial2FromAwareAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial2WalkRun::ClearFromAware
//!	
//!
//------------------------------------------------------------------------------------------
void Partial2WalkRun::ClearFromAware( CAnimationPtr* aobFromAwareAnims )
{
	// Call the base functionality first
	WalkRunController::ClearFromAware( aobFromAwareAnims );

	// If we are capable of providing the animations...
	if ( m_bPartial2MinimumSet )
	{
		// ...then call it again with our own anims
		WalkRunController::ClearFromAware( m_aobPartial2FromAwareAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial2WalkRun::StartRotateOnSpot
//!	
//!
//------------------------------------------------------------------------------------------
void Partial2WalkRun::StartRotateOnSpot( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobRotateOnSpotAnims )
{
	// Call the base functionality first
	WalkRunController::StartRotateOnSpot( obCurrentMovementState, aobRotateOnSpotAnims );

	// If we are capable of providing the animations...
	if ( m_bPartial2MinimumSet )
	{
		// ...then call it again with our own anims
		// TODO
		WalkRunController::StartRotateOnSpot( obCurrentMovementState, m_aobPartial2FromAwareAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial2WalkRun::ClearRotateOnSpot
//!	
//!
//------------------------------------------------------------------------------------------
void Partial2WalkRun::ClearRotateOnSpot( CAnimationPtr* aobRotateOnSpotAnims )
{
	// Call the base functionality first
	WalkRunController::ClearRotateOnSpot( aobRotateOnSpotAnims );

	// If we are capable of providing the animations...
	if ( m_bPartial2MinimumSet )
	{
		// ...then call it again with our own anims
		// TODO
		WalkRunController::ClearFromAware( m_aobPartial2FromAwareAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial2WalkRun::StartStarting
//!	
//!
//------------------------------------------------------------------------------------------
void Partial2WalkRun::StartStarting( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobStartingAnims )
{
	// Call the base functionality first
	WalkRunController::StartStarting( obCurrentMovementState, aobStartingAnims );

	// If we are capable of providing the animations...
	if ( m_bPartial2MinimumSet )
	{
		// ...then call it again with our own anims
		WalkRunController::StartStarting( obCurrentMovementState, m_aobPartial2StartingAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial2WalkRun::ClearStarting
//!	
//!
//------------------------------------------------------------------------------------------
void Partial2WalkRun::ClearStarting( CAnimationPtr* aobStartingAnims )
{
	// Call the base functionality first
	WalkRunController::ClearStarting( aobStartingAnims);

	// If we are capable of providing the animations...
	if ( m_bPartial2MinimumSet )
	{
		// ...then call it again with our own anims
		WalkRunController::ClearStarting( m_aobPartial2StartingAnims);
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial2WalkRun::StartStopping
//!	
//!
//------------------------------------------------------------------------------------------
void Partial2WalkRun::StartStopping( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobStoppingAnims )
{
	// Call the base functionality first
	WalkRunController::StartStopping( obCurrentMovementState, aobStoppingAnims );

	// If we are capable of providing the animations...
	if ( m_bPartial2MinimumSet )
	{
		// ...then call it again with our own anims
		WalkRunController::StartStopping( obCurrentMovementState, m_aobPartial2StoppingAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial2WalkRun::ClearStopping
//!	
//!
//------------------------------------------------------------------------------------------
void Partial2WalkRun::ClearStopping( CAnimationPtr* aobStoppingAnims )
{
	// Call the base functionality first
	WalkRunController::ClearStopping( aobStoppingAnims );

	// If we are capable of providing the animations...
	if ( m_bPartial2MinimumSet )
	{
		// ...then call it again with our own anims
		WalkRunController::ClearStopping( m_aobPartial2StoppingAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial2WalkRun::StartMoving
//!	
//!
//------------------------------------------------------------------------------------------
void Partial2WalkRun::StartMoving( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobMovingAnims )
{
	// Call the base functionality first
	WalkRunController::StartMoving( obCurrentMovementState, aobMovingAnims );

	// If we are capable of providing the animations...
	if ( m_bPartial2MinimumSet )
	{
		// ...then call it again with our own anims
		WalkRunController::StartMoving( obCurrentMovementState, m_aobPartial2MovingAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial2WalkRun::ClearMoving
//!	
//!
//------------------------------------------------------------------------------------------
void Partial2WalkRun::ClearMoving( CAnimationPtr* aobMovingAnims )
{
	// Call the base functionality first
	WalkRunController::ClearMoving( aobMovingAnims );

	// If we are capable of providing the animations...
	if ( m_bPartial2MinimumSet )
	{
		// ...then call it again with our own anims
		WalkRunController::ClearMoving( m_aobPartial2MovingAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial2WalkRun::StartTurning
//!	
//!
//------------------------------------------------------------------------------------------
void Partial2WalkRun::StartTurning( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobTurningAnims )
{
	// Call the base functionality first
	WalkRunController::StartTurning( obCurrentMovementState, aobTurningAnims );

	// If we are capable of providing the animations...
	if ( m_bPartial2MinimumSet )
	{
		// ...then call it again with our own anims
		WalkRunController::StartTurning( obCurrentMovementState, m_aobPartial2TurningAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial2WalkRun::ClearTurning
//!	
//!
//------------------------------------------------------------------------------------------
void Partial2WalkRun::ClearTurning( CAnimationPtr* aobTurningAnims )
{
	// Call the base functionality first
	WalkRunController::ClearTurning( aobTurningAnims );

	// If we are capable of providing the animations...
	if ( m_bPartial2MinimumSet )
	{
		// ...then call it again with our own anims
		WalkRunController::ClearTurning( m_aobPartial2TurningAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial3WalkRunDef::Partial3WalkRunDef
//!	Construction
//!
//------------------------------------------------------------------------------------------
Partial3WalkRunDef::Partial3WalkRunDef( void )
:	m_pobPartial3AnimSet( 0 )
{
}


//------------------------------------------------------------------------------------------
//!
//!	Partial3WalkRunDef::CreateInstance
//!	Create an instance of what we describe
//!
//------------------------------------------------------------------------------------------
MovementController* Partial3WalkRunDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) Partial3WalkRun( pobMovement, *this );
}


//------------------------------------------------------------------------------------------
//!
//!	Partial3WalkRun::Partial3WalkRun
//!	Construction
//!
//------------------------------------------------------------------------------------------
Partial3WalkRun::Partial3WalkRun( CMovement* pobMovement, const Partial3WalkRunDef& obDefinition )
:	Partial2WalkRun( pobMovement, obDefinition ),
	m_obPartial3Definition( obDefinition ),
	m_bPartial3MinimumSet( false )
{
	// Check that we have the bare minimum of animations to make this work
	if ( !m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding.IsNull() )
	{
		m_bPartial3MinimumSet = true;
	}
	else
	{
		user_warn_p( 0, ( "" ) );
	}

	// If we are capable of providing the animations
	if ( m_bPartial3MinimumSet )
	{
		// Create our idle animations - this is the only animation that we must have!
		m_aobPartial3IdleAnims[IA_STAND] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_LOOPING );

		// Rotate on spot uses the standard idle animation
		m_aobPartial3RotateOnSpotAnims[RSA_IDLE] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_LOOPING );

		// Create our aware animations if necessary
		if ( m_bCanBeAware )
		{
			m_aobPartial3AwareAnims[AA_MIN]			= CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_LOOPING );
			m_aobPartial3AwareAnims[AA_MAX]			= CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_LOOPING );
			m_aobPartial3ToAwareAnims[TAA_START]	= CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, 0 );
			m_aobPartial3FromAwareAnims[FAA_FROM]	= CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, 0 );
		}

		// Create our starting animations
		if ( m_bCanSlowWalk )
		{
			if ( !m_obPartial3Definition.m_pobPartial3AnimSet->m_obStandToWalk.IsNull() )
			{
				m_aobPartial3StartingAnims[STA_TOSLOWWALK]			= CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStandToWalk, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
				m_aobPartial3StartingAnims[STA_TOSLOWWALKLEFT]		= CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStandToWalk, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET  );
				m_aobPartial3StartingAnims[STA_TOSLOWWALKBACKLEFT]	= CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStandToWalk, ANIMF_PHASE_LINKED );
				m_aobPartial3StartingAnims[STA_TOSLOWWALKRIGHT]		= CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStandToWalk, ANIMF_PHASE_LINKED);
				m_aobPartial3StartingAnims[STA_TOSLOWWALKBACKRIGHT]	= CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStandToWalk, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
			}
			else
			{
				m_aobPartial3StartingAnims[STA_TOSLOWWALK]			= CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
				m_aobPartial3StartingAnims[STA_TOSLOWWALKLEFT]		= CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET  );
				m_aobPartial3StartingAnims[STA_TOSLOWWALKBACKLEFT]	= CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_PHASE_LINKED );
				m_aobPartial3StartingAnims[STA_TOSLOWWALKRIGHT]		= CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_PHASE_LINKED);
				m_aobPartial3StartingAnims[STA_TOSLOWWALKBACKRIGHT]	= CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
			}
		}

		if ( m_bCanWalk )
		{
			if ( !m_obPartial3Definition.m_pobPartial3AnimSet->m_obStandToWalk.IsNull() )
			{
				m_aobPartial3StartingAnims[STA_TOWALK]			= CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStandToWalk, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
				m_aobPartial3StartingAnims[STA_TOWALKLEFT]		= CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStandToWalk, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET  );
				m_aobPartial3StartingAnims[STA_TOWALKBACKLEFT]	= CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStandToWalk, ANIMF_PHASE_LINKED );
				m_aobPartial3StartingAnims[STA_TOWALKRIGHT]		= CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStandToWalk, ANIMF_PHASE_LINKED);
				m_aobPartial3StartingAnims[STA_TOWALKBACKRIGHT]	= CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStandToWalk, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
			}
			else
			{
				m_aobPartial3StartingAnims[STA_TOWALK]			= CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
				m_aobPartial3StartingAnims[STA_TOWALKLEFT]		= CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET  );
				m_aobPartial3StartingAnims[STA_TOWALKBACKLEFT]	= CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_PHASE_LINKED );
				m_aobPartial3StartingAnims[STA_TOWALKRIGHT]		= CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_PHASE_LINKED);
				m_aobPartial3StartingAnims[STA_TOWALKBACKRIGHT]	= CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
			}
		}

		if ( m_bCanRun )
		{
			if ( !m_obPartial3Definition.m_pobPartial3AnimSet->m_obStandToAccel.IsNull() )
				m_aobPartial3StartingAnims[STA_TOACCEL] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStandToAccel, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
			else
				m_aobPartial3StartingAnims[STA_TOACCEL] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );

			if ( !m_obPartial3Definition.m_pobPartial3AnimSet->m_obStandToAccelLeft.IsNull() )
			{
				m_aobPartial3StartingAnims[STA_TOACCELLEFT]		= CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStandToAccelLeft, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
				m_aobPartial3StartingAnims[STA_TOACCELBACKLEFT]	= CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStandToAccelLeft, ANIMF_PHASE_LINKED );
			}
			else
			{
				m_aobPartial3StartingAnims[STA_TOACCELLEFT]		= CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
				m_aobPartial3StartingAnims[STA_TOACCELBACKLEFT]	= CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_PHASE_LINKED );
			}

			if ( !m_obPartial3Definition.m_pobPartial3AnimSet->m_obStandToAccelRight.IsNull() )
			{
				m_aobPartial3StartingAnims[STA_TOACCELRIGHT]		= CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStandToAccelRight, ANIMF_PHASE_LINKED );
				m_aobPartial3StartingAnims[STA_TOACCELBACKRIGHT]	= CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStandToAccelRight, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
			}
			else
			{
				m_aobPartial3StartingAnims[STA_TOACCELRIGHT]			= CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_PHASE_LINKED );
				m_aobPartial3StartingAnims[STA_TOACCELBACKRIGHT]	= CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
			}
		}

		// Create our stopping animations
		if ( m_bCanSlowWalk )
		{
			if ( !m_obPartial3Definition.m_pobPartial3AnimSet->m_obWalkToStandLeft.IsNull() )
				m_aobPartial3StoppingAnims[SPA_FROMSLOWWALKLEFT] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obWalkToStandLeft, ANIMF_PHASE_LINKED );
			else
				m_aobPartial3StoppingAnims[SPA_FROMSLOWWALKLEFT] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_PHASE_LINKED );

			if ( !m_obPartial3Definition.m_pobPartial3AnimSet->m_obWalkToStandRight.IsNull() )
				m_aobPartial3StoppingAnims[SPA_FROMSLOWWALKRIGHT] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obWalkToStandRight, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
			else
				m_aobPartial3StoppingAnims[SPA_FROMSLOWWALKRIGHT] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
		}

		if ( m_bCanWalk )
		{
			if ( !m_obPartial3Definition.m_pobPartial3AnimSet->m_obWalkToStandLeft.IsNull() )
				m_aobPartial3StoppingAnims[SPA_FROMWALKLEFT] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obWalkToStandLeft, ANIMF_PHASE_LINKED );
			else
				m_aobPartial3StoppingAnims[SPA_FROMWALKLEFT] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_PHASE_LINKED );

			if ( !m_obPartial3Definition.m_pobPartial3AnimSet->m_obWalkToStandRight.IsNull() )
				m_aobPartial3StoppingAnims[SPA_FROMWALKRIGHT] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obWalkToStandRight, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
			else
				m_aobPartial3StoppingAnims[SPA_FROMWALKRIGHT] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
		}

		if ( m_bCanRun )
		{
			if ( !m_obPartial3Definition.m_pobPartial3AnimSet->m_obRunToStandLeft.IsNull() )
				m_aobPartial3StoppingAnims[SPA_FROMRUNLEFT] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obRunToStandLeft, ANIMF_PHASE_LINKED );
			else
				m_aobPartial3StoppingAnims[SPA_FROMRUNLEFT] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_PHASE_LINKED );

			if ( !m_obPartial3Definition.m_pobPartial3AnimSet->m_obRunToStandRight.IsNull() )
				m_aobPartial3StoppingAnims[SPA_FROMRUNRIGHT] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obRunToStandRight, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
			else
				m_aobPartial3StoppingAnims[SPA_FROMRUNRIGHT] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
		}

		// Create our moving animations
		if ( m_bCanSlowWalk )
		{
			if ( !m_obPartial3Definition.m_pobPartial3AnimSet->m_obWalk.IsNull() )
			{
				m_aobPartial3MovingAnims[MA_SLOWWALK] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obWalk, ANIMF_LOOPING|ANIMF_PHASE_LINKED );

				if ( m_bCanTurn )
				{
					m_aobPartial3MovingAnims[MA_SLOWWALKLEFT] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obWalk, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
					m_aobPartial3MovingAnims[MA_SLOWWALKRIGHT] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obWalk, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
				}
			}
			else
			{
				m_aobPartial3MovingAnims[MA_SLOWWALK] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
				
				if ( m_bCanTurn )
				{
					m_aobPartial3MovingAnims[MA_SLOWWALKLEFT] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
					m_aobPartial3MovingAnims[MA_SLOWWALKRIGHT] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
				}
			}
		}

		if ( m_bCanWalk )
		{
			if ( !m_obPartial3Definition.m_pobPartial3AnimSet->m_obWalk.IsNull() )
			{
				m_aobPartial3MovingAnims[MA_WALK] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obWalk, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
								
				if ( m_bCanTurn )
				{
					m_aobPartial3MovingAnims[MA_WALKLEFT] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obWalk, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
					m_aobPartial3MovingAnims[MA_WALKRIGHT] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obWalk, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
				}
			}
			else
			{
				m_aobPartial3MovingAnims[MA_WALK] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
								
				if ( m_bCanTurn )
				{
					m_aobPartial3MovingAnims[MA_WALKLEFT] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
					m_aobPartial3MovingAnims[MA_WALKRIGHT] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
				}
			}
		}

		if ( m_bCanRun )
		{
			if ( !m_obPartial3Definition.m_pobPartial3AnimSet->m_obRun.IsNull() )
			{
				m_aobPartial3MovingAnims[MA_RUN] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obRun, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
								
				if ( m_bCanTurn )
				{
					m_aobPartial3MovingAnims[MA_RUNLEFT] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obRun, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
					m_aobPartial3MovingAnims[MA_RUNRIGHT] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obRun, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
				}
			}
			else
			{
				m_aobPartial3MovingAnims[MA_RUN] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
								
				if ( m_bCanTurn )
				{
					m_aobPartial3MovingAnims[MA_RUNLEFT] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
					m_aobPartial3MovingAnims[MA_RUNRIGHT] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
				}
			}
		}

		// Create our accelerating animations
		if ( m_bCanAccelerate )
		{
			if ( !m_obPartial3Definition.m_pobPartial3AnimSet->m_obAccelRun.IsNull() )
			{
				m_aobPartial3MovingAnims[MA_ACCELRUN] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obAccelRun, ANIMF_LOOPING|ANIMF_PHASE_LINKED );

				if ( m_bCanTurn )
				{
					m_aobPartial3MovingAnims[MA_ACCELRUNLEFT] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obAccelRun, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
					m_aobPartial3MovingAnims[MA_ACCELRUNRIGHT] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obAccelRun, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
				}
			}
			else
			{
				m_aobPartial3MovingAnims[MA_ACCELRUN] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_LOOPING|ANIMF_PHASE_LINKED );

				if ( m_bCanTurn )
				{
					m_aobPartial3MovingAnims[MA_ACCELRUNLEFT] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
					m_aobPartial3MovingAnims[MA_ACCELRUNRIGHT] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
				}
			}
		}

		// Create our fast turn animations
		if ( m_bCanFastTurn )
		{
			if ( !m_obPartial3Definition.m_pobPartial3AnimSet->m_obFullTurnLeft.IsNull() )
				m_aobPartial3TurningAnims[TUA_LEFT] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obFullTurnLeft, ANIMF_PHASE_LINKED );
			else
				m_aobPartial3TurningAnims[TUA_LEFT] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_PHASE_LINKED );

			if ( !m_obPartial3Definition.m_pobPartial3AnimSet->m_obFullTurnRight.IsNull() )
				m_aobPartial3TurningAnims[TUA_RIGHT] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obFullTurnRight, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
			else
				m_aobPartial3TurningAnims[TUA_RIGHT] = CreateAnimation( m_obPartial3Definition.m_pobPartial3AnimSet->m_obStanding, ANIMF_PHASE_LINKED|ANIMF_PHASE_OFFSET );
		}
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial3WalkRun::Partial3WalkRun
//!	Destruction
//!
//------------------------------------------------------------------------------------------
Partial3WalkRun::~Partial3WalkRun( void )
{
	// If we are capable of providing the animations...
	if ( m_bPartial3MinimumSet )
	{
		// Ensure all our animations are removed
		if ( m_fStateWeights[MS_IDLE] > 0.0f )
			WalkRunController::ClearIdle( m_aobPartial3IdleAnims );

		if ( m_fStateWeights[MS_STARTING] > 0.0f )
			WalkRunController::ClearStarting( m_aobPartial3StartingAnims );

		if ( m_fStateWeights[MS_TO_AWARE] > 0.0f )
			WalkRunController::ClearToAware( m_aobPartial3ToAwareAnims );

		if ( m_fStateWeights[MS_AWARE] > 0.0f )
			WalkRunController::ClearAware( m_aobPartial3AwareAnims );

		if ( m_fStateWeights[MS_FROM_AWARE] > 0.0f )
			WalkRunController::ClearFromAware( m_aobPartial3FromAwareAnims );
			
		if ( m_fStateWeights[MS_ROTATE_ON_SPOT] > 0.0f )
			WalkRunController::ClearRotateOnSpot( m_aobPartial3RotateOnSpotAnims );

		if ( m_fStateWeights[MS_STOPPING] > 0.0f )
			WalkRunController::ClearStopping( m_aobPartial3StoppingAnims );
		
		if ( m_fStateWeights[MS_MOVING] > 0.0f )
			WalkRunController::ClearMoving( m_aobPartial3MovingAnims );

		if ( m_fStateWeights[MS_TURNING] > 0.0f )
			WalkRunController::ClearTurning( m_aobPartial3TurningAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial3WalkRun::Update
//!	
//!
//------------------------------------------------------------------------------------------
bool Partial3WalkRun::Update(	float						fTimeStep, 
								const CMovementInput&		obMovementInput,
								const CMovementStateRef&	obCurrentMovementState,
								CMovementState&				obPredictedMovementState )
{
	// Call the base functionality first
	Partial2WalkRun::Update( fTimeStep, obMovementInput, obCurrentMovementState, obPredictedMovementState );

	// If we are capable of providing the animations...
	if ( m_bPartial3MinimumSet )
	{
		// Recall the animation blending code with our own animation set
		UpdateAnimationBlendWeights( fTimeStep, m_aobPartial3IdleAnims,
												m_aobPartial3AwareAnims,
												m_aobPartial3ToAwareAnims,
												m_aobPartial3FromAwareAnims,
												m_aobPartial3RotateOnSpotAnims,
												m_aobPartial3StartingAnims,
												m_aobPartial3StoppingAnims,
												m_aobPartial3MovingAnims,
												m_aobPartial3TurningAnims );
	}

	// We never finish
	return false;
}


//------------------------------------------------------------------------------------------
//!
//!	Partial3WalkRun::StartIdle
//!	
//!
//------------------------------------------------------------------------------------------
void Partial3WalkRun::StartIdle( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobIdleAnims )
{
	// Call the base functionality first
	Partial2WalkRun::StartIdle( obCurrentMovementState, aobIdleAnims );

	// If we are capable of providing the animations...
	if ( m_bPartial3MinimumSet )
	{
		// ...then call the low level functionality with our own anims
		WalkRunController::StartIdle( obCurrentMovementState, m_aobPartial3IdleAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial3WalkRun::ClearIdle
//!	
//!
//------------------------------------------------------------------------------------------
void Partial3WalkRun::ClearIdle( CAnimationPtr* aobIdleAnims )
{
	// Call the base functionality first
	Partial2WalkRun::ClearIdle( aobIdleAnims );

	// If we are capable of providing the animations...
	if ( m_bPartial3MinimumSet )
	{
		// ...then call the low level functionality with our own anims
		WalkRunController::ClearIdle( m_aobPartial3IdleAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial3WalkRun::StartAware
//!	
//!
//------------------------------------------------------------------------------------------
void Partial3WalkRun::StartAware( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobAwareAnims )
{
	// Call the base functionality first
	Partial2WalkRun::StartAware( obCurrentMovementState, aobAwareAnims );

	// If we are capable of providing the animations...
	if ( m_bPartial3MinimumSet )
	{
		// ...then call the low level functionality with our own anims
		WalkRunController::StartAware( obCurrentMovementState, m_aobPartial2AwareAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial3WalkRun::ClearAware
//!	
//!
//------------------------------------------------------------------------------------------
void Partial3WalkRun::ClearAware( CAnimationPtr* aobAwareAnims )
{
	// Call the base functionality first
	Partial2WalkRun::ClearAware( aobAwareAnims );

	// If we are capable of providing the animations...
	if ( m_bPartial3MinimumSet )
	{
		// ...then call the low level functionality with our own anims
		WalkRunController::ClearAware( m_aobPartial3AwareAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial3WalkRun::StartToAware
//!	
//!
//------------------------------------------------------------------------------------------
void Partial3WalkRun::StartToAware( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobToAwareAnims )
{
	// Call the base functionality first
	Partial2WalkRun::StartToAware( obCurrentMovementState, aobToAwareAnims );

	// If we are capable of providing the animations...
	if ( m_bPartial3MinimumSet )
	{
		// ...then call the low level functionality with our own anims
		WalkRunController::StartToAware( obCurrentMovementState, m_aobPartial3ToAwareAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial3WalkRun::ClearToAware
//!	
//!
//------------------------------------------------------------------------------------------
void Partial3WalkRun::ClearToAware( CAnimationPtr* aobToAwareAnims )
{
	// Call the base functionality first
	Partial2WalkRun::ClearToAware( aobToAwareAnims );

	// If we are capable of providing the animations...
	if ( m_bPartial3MinimumSet )
	{
		// ...then call the low level functionality with our own anims
		WalkRunController::ClearToAware( m_aobPartial3ToAwareAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial3WalkRun::StartFromAware
//!	
//!
//------------------------------------------------------------------------------------------
void Partial3WalkRun::StartFromAware( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobFromAwareAnims )
{
	// Call the base functionality first
	Partial2WalkRun::StartFromAware( obCurrentMovementState, aobFromAwareAnims );

	// If we are capable of providing the animations...
	if ( m_bPartial3MinimumSet )
	{
		// ...then call the low level functionality with our own anims
		WalkRunController::StartFromAware( obCurrentMovementState, m_aobPartial3FromAwareAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial3WalkRun::ClearFromAware
//!	
//!
//------------------------------------------------------------------------------------------
void Partial3WalkRun::ClearFromAware( CAnimationPtr* aobFromAwareAnims )
{
	// Call the base functionality first
	Partial2WalkRun::ClearFromAware( aobFromAwareAnims );

	// If we are capable of providing the animations...
	if ( m_bPartial3MinimumSet )
	{
		// ...then call the low level functionality with our own anims
		WalkRunController::ClearFromAware( m_aobPartial3FromAwareAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial3WalkRun::StartRotateOnSpot
//!	
//!
//------------------------------------------------------------------------------------------
void Partial3WalkRun::StartRotateOnSpot( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobRotateOnSpotAnims )
{
	// Call the base functionality first
	Partial2WalkRun::StartRotateOnSpot( obCurrentMovementState, aobRotateOnSpotAnims );

	// If we are capable of providing the animations...
	if ( m_bPartial3MinimumSet )
	{
		// ...then call it again with our own anims
		// TODO
		WalkRunController::StartRotateOnSpot( obCurrentMovementState, m_aobPartial3FromAwareAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial3WalkRun::ClearRotateOnSpot
//!	
//!
//------------------------------------------------------------------------------------------
void Partial3WalkRun::ClearRotateOnSpot( CAnimationPtr* aobRotateOnSpotAnims )
{
	// Call the base functionality first
	Partial2WalkRun::ClearRotateOnSpot( aobRotateOnSpotAnims );

	// If we are capable of providing the animations...
	if ( m_bPartial3MinimumSet )
	{
		// ...then call it again with our own anims
		// TODO
		WalkRunController::ClearFromAware( m_aobPartial3FromAwareAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial3WalkRun::StartStarting
//!	
//!
//------------------------------------------------------------------------------------------
void Partial3WalkRun::StartStarting( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobStartingAnims )
{
	// Call the base functionality first
	Partial2WalkRun::StartStarting( obCurrentMovementState, aobStartingAnims );

	// If we are capable of providing the animations...
	if ( m_bPartial3MinimumSet )
	{
		// ...then call the low level functionality with our own anims
		WalkRunController::StartStarting( obCurrentMovementState, m_aobPartial3StartingAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial3WalkRun::ClearStarting
//!	
//!
//------------------------------------------------------------------------------------------
void Partial3WalkRun::ClearStarting( CAnimationPtr* aobStartingAnims )
{
	// Call the base functionality first
	Partial2WalkRun::ClearStarting( aobStartingAnims );

	// If we are capable of providing the animations...
	if ( m_bPartial3MinimumSet )
	{
		// ...then call the low level functionality with our own anims
		WalkRunController::ClearStarting( m_aobPartial3StartingAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial3WalkRun::StartStopping
//!	
//!
//------------------------------------------------------------------------------------------
void Partial3WalkRun::StartStopping( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobStoppingAnims )
{
	// Call the base functionality first
	Partial2WalkRun::StartStopping( obCurrentMovementState, aobStoppingAnims );

	// If we are capable of providing the animations...
	if ( m_bPartial3MinimumSet )
	{
		// ...then call the low level functionality with our own anims
		WalkRunController::StartStopping( obCurrentMovementState, m_aobPartial3StoppingAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial3WalkRun::ClearStopping
//!	
//!
//------------------------------------------------------------------------------------------
void Partial3WalkRun::ClearStopping( CAnimationPtr* aobStoppingAnims )
{
	// Call the base functionality first
	Partial2WalkRun::ClearStopping( aobStoppingAnims );

	// If we are capable of providing the animations...
	if ( m_bPartial3MinimumSet )
	{
		// ...then call the low level functionality with our own anims
		WalkRunController::ClearStopping( m_aobPartial3StoppingAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial3WalkRun::StartMoving
//!	
//!
//------------------------------------------------------------------------------------------
void Partial3WalkRun::StartMoving( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobMovingAnims )
{
	// Call the base functionality first
	Partial2WalkRun::StartMoving( obCurrentMovementState, aobMovingAnims );

	// If we are capable of providing the animations...
	if ( m_bPartial3MinimumSet )
	{
		// ...then call the low level functionality with our own anims
		WalkRunController::StartMoving( obCurrentMovementState, m_aobPartial3MovingAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial3WalkRun::ClearMoving
//!	
//!
//------------------------------------------------------------------------------------------
void Partial3WalkRun::ClearMoving( CAnimationPtr* aobMovingAnims )
{
	// Call the base functionality first
	Partial2WalkRun::ClearMoving( aobMovingAnims );

	// If we are capable of providing the animations...
	if ( m_bPartial3MinimumSet )
	{
		// ...then call the low level functionality with our own anims
		WalkRunController::ClearMoving( m_aobPartial3MovingAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial3WalkRun::StartTurning
//!	
//!
//------------------------------------------------------------------------------------------
void Partial3WalkRun::StartTurning( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobTurningAnims )
{
	// Call the base functionality first
	Partial2WalkRun::StartTurning( obCurrentMovementState, aobTurningAnims );

	// If we are capable of providing the animations...
	if ( m_bPartial3MinimumSet )
	{
		// ...then call the low level functionality with our own anims
		WalkRunController::StartTurning( obCurrentMovementState, m_aobPartial3TurningAnims );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Partial3WalkRun::ClearTurning
//!	
//!
//------------------------------------------------------------------------------------------
void Partial3WalkRun::ClearTurning( CAnimationPtr* aobTurningAnims )
{
	// Call the base functionality first
	Partial2WalkRun::ClearTurning( aobTurningAnims );

	// If we are capable of providing the animations...
	if ( m_bPartial3MinimumSet )
	{
		// ...then call the low level functionality with our own anims
		WalkRunController::ClearTurning( m_aobPartial3TurningAnims );
	}
}


