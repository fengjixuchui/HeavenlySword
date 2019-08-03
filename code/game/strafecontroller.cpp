//------------------------------------------------------------------------------------------
//!
//!	\file strafecontroller.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "strafecontroller.h"
#include "anim/animator.h"
#include "core/exportstruct_anim.h"
#include "objectdatabase/dataobject.h"
#include "movement.h"
#include "entity.h"
#include "attacks.h"
#include "core/visualdebugger.h"

START_STD_INTERFACE	(StrafeControllerDef)
	IREFERENCE	(StrafeControllerDef, AnimSet)
	IFLOAT		(StrafeControllerDef, Speed)
	IFLOAT		(StrafeControllerDef, InputThreshold)
	IFLOAT		(StrafeControllerDef, BlinkInterval)
	PUBLISH_VAR_AS( m_bApplyGravity, ApplyGravity )

#ifndef _RELEASE
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
#endif
END_STD_INTERFACE

START_STD_INTERFACE( StrafeAnimSet )
	PUBLISH_VAR_AS( m_obStanding, Standing )
	PUBLISH_VAR_AS( m_obLookLeft, LookLeft )
	PUBLISH_VAR_AS( m_obLookRight, LookRight )
	PUBLISH_VAR_AS( m_obTurnLeft, TurnLeft )
	PUBLISH_VAR_AS( m_obTurnRight, TurnRight )
	PUBLISH_VAR_AS( m_obMoveForwards, MoveForwards )
	PUBLISH_VAR_AS( m_obMoveBackwards, MoveBackwards )
	PUBLISH_VAR_AS( m_obMoveLeft, MoveLeft )
	PUBLISH_VAR_AS( m_obMoveRight, MoveRight )
	PUBLISH_VAR_AS( m_fTurnPoint, TurnPoint )
	PUBLISH_VAR_AS( m_fTurnBlend, TurnBlend )
	PUBLISH_VAR_AS( m_obBlink, Blink )
END_STD_INTERFACE

// Our parameters - not exposed as yet
const static float s_fTurnPoint = 80.0f;
const static float s_fTurnBlend = 0.1f;

static const float BLINK_RANDOMNESS = 0.5f;
static const float BLINK_INTERVAL_MULTIPLIER = BLINK_RANDOMNESS / RAND_MAX;

//------------------------------------------------------------------------------------------
//!
//!	StrafeAnimSet::StrafeAnimSet
//! Construction
//!	Attaches the XML Interface
//!
//------------------------------------------------------------------------------------------
StrafeAnimSet::StrafeAnimSet( void )
:	m_fTurnPoint( s_fTurnPoint ),
	m_fTurnBlend( s_fTurnBlend )
{
}

//------------------------------------------------------------------------------------------
//!
//!	StrafeControllerDef::StrafeControllerDef
//! Construction
//!	Attaches the XML Interface
//!
//------------------------------------------------------------------------------------------
StrafeControllerDef::StrafeControllerDef( void )
:	m_pobAnimSet( 0 ),
	m_fSpeed( 1.0f ),
	m_fInputThreshold( 0.2f ),
	m_bApplyGravity( true )
{
	
}


//------------------------------------------------------------------------------------------
//!
//!	StrafeControllerDef::CreateInstance
//!	To create an instance of what we describe
//!
//------------------------------------------------------------------------------------------
MovementController* StrafeControllerDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) StrafeController( pobMovement, *this );
}


//------------------------------------------------------------------------------------------
//!
//!	StrafeController::StrafeController
//! Construction
//!
//------------------------------------------------------------------------------------------
StrafeController::StrafeController(	CMovement*					pobMovement, 
									const StrafeControllerDef&	obDefinition )
:	MovementController( pobMovement ),
	m_obDefinition( obDefinition ),
	m_obStandingAnimation(),
	m_bTurnLeft( false ),
	m_bCanLockOn( false ),
	m_bCanMove( false ),
	m_obRequiredFaceDirection( CONSTRUCT_CLEAR )
{
	// Register our definition for debugging purposes
	InternalRegisterDefinition( m_obDefinition );

	// Check the given animation set to see what capability this controller will have - lock on?
	m_bCanLockOn = (	!m_obDefinition.m_pobAnimSet->m_obLookLeft.IsNull() &&
						!m_obDefinition.m_pobAnimSet->m_obLookRight.IsNull() &&
						!m_obDefinition.m_pobAnimSet->m_obTurnLeft.IsNull() &&
						!m_obDefinition.m_pobAnimSet->m_obTurnRight.IsNull() );

	// Check the given animation set to see what capability this controller will have - movement?
	m_bCanMove = (	!m_obDefinition.m_pobAnimSet->m_obMoveForwards.IsNull() &&
					!m_obDefinition.m_pobAnimSet->m_obMoveBackwards.IsNull() &&
					!m_obDefinition.m_pobAnimSet->m_obMoveLeft.IsNull() &&
					!m_obDefinition.m_pobAnimSet->m_obMoveRight.IsNull() );

	// We must always have a standing animation
	ntAssert_p( !m_obDefinition.m_pobAnimSet->m_obStanding.IsNull(), ( "Standing animation always requuired for the strafe controller.\n" ) );

	// Create our animations
	m_obStandingAnimation				= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obStanding, ANIMF_LOOPING ); //|ANIMF_PHASE_LINKED );

	// If we have a full movement set then add the animations
	if ( m_bCanMove )
	{
		m_aobMovingAnims[MA_FORWARDS]	= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obMoveForwards, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
		m_aobMovingAnims[MA_BACKWARDS]	= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obMoveBackwards, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
		m_aobMovingAnims[MA_LEFT]		= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obMoveLeft, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
		m_aobMovingAnims[MA_RIGHT]		= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obMoveRight, ANIMF_LOOPING|ANIMF_PHASE_LINKED );
	}

	// If we can do the twisting and turning add the animations
	if ( m_bCanLockOn )
	{
		m_aobLookingAnims[LA_LOOK_LEFT]	= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obLookLeft, ANIMF_LOOPING ); //|ANIMF_PHASE_LINKED );
		m_aobLookingAnims[LA_LOOK_RIGHT]= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obLookRight, ANIMF_LOOPING ); //|ANIMF_PHASE_LINKED );
		m_aobTurningAnims[TA_TURN_LEFT]	= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obTurnLeft, 0 ); //ANIMF_PHASE_LINKED );
		m_aobTurningAnims[TA_TURN_RIGHT]= CreateAnimation( m_obDefinition.m_pobAnimSet->m_obTurnRight, 0 ); //ANIMF_PHASE_LINKED );
	}

	// If we can move we may want to tweak the animation speed
	if ( m_bCanMove )
	{
		m_aobMovingAnims[MA_FORWARDS]->SetSpeed( obDefinition.m_fSpeed );
		m_aobMovingAnims[MA_BACKWARDS]->SetSpeed( obDefinition.m_fSpeed );
		m_aobMovingAnims[MA_LEFT]->SetSpeed( obDefinition.m_fSpeed );
		m_aobMovingAnims[MA_RIGHT]->SetSpeed( obDefinition.m_fSpeed );
	}

	// Make sure we have a sensible definition
	ntAssert( ( m_obDefinition.m_fInputThreshold < 1.0f ) && ( m_obDefinition.m_fInputThreshold >= 0.0f ) );
}


//------------------------------------------------------------------------------------------
//!
//!	StrafeController::~StrafeController
//! Destruction
//!
//------------------------------------------------------------------------------------------
StrafeController::~StrafeController()
{
	// Deactivate all the animations
	if ( !m_bFirstFrame )
	DeactivateAnimations( &m_obStandingAnimation, 1 );

	if ( m_bCanMove && !m_bFirstFrame )
		DeactivateAnimations( m_aobMovingAnims, MA_COUNT );

	if ( m_bCanLockOn && !m_bFirstFrame )
	{
		DeactivateAnimations( m_aobTurningAnims, TA_COUNT );
		DeactivateAnimations( m_aobLookingAnims, LA_COUNT );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	StrafeController::CreateAnimation
//! Creates an animation from a name and sets the correct flags
//!
//------------------------------------------------------------------------------------------
CAnimationPtr StrafeController::CreateAnimation( const CHashedString& obAnimationName, int iVariableFlags )
{
	// Create the animation
	CAnimationPtr obReturnAnim = m_pobAnimator->CreateAnimation( obAnimationName );

	// Zero the initial blend weight
	obReturnAnim->SetBlendWeight( 0.0f );

	// Set up the flags - including the item we passed in
	obReturnAnim->SetFlags( iVariableFlags|ANIMF_LOCOMOTING|ANIMF_INHIBIT_AUTO_DESTRUCT );

	// Return a pointer to the animation
	return obReturnAnim;
}


//------------------------------------------------------------------------------------------
//!
//!	StrafeController::ActivateAnimations
//! Activate a sub-states set of animations
//!
//------------------------------------------------------------------------------------------
void StrafeController::ActivateAnimations( CAnimationPtr* aAnimations, int iNumberOfAnims )
{
	// Add to the animator
	for ( int iAnimations = 0; iAnimations < iNumberOfAnims; ++iAnimations )
		m_pobAnimator->AddAnimation( aAnimations[iAnimations] );

	// Zero the blend weights
	for ( int iAnimations = 0; iAnimations < iNumberOfAnims; ++iAnimations )
		aAnimations[iAnimations]->SetBlendWeight( 0.0f );
}


//------------------------------------------------------------------------------------------
//!
//!	StrafeController::DeactivateAnimations
//! Remove a sub-states set of animations from the animator
//!
//------------------------------------------------------------------------------------------
void StrafeController::DeactivateAnimations( CAnimationPtr* aAnimations, int iNumberOfAnims )
{
	for ( int iAnimations = 0; iAnimations < iNumberOfAnims; ++iAnimations )
	{
		if (aAnimations[iAnimations]->IsActive())
			m_pobAnimator->RemoveAnimation( aAnimations[iAnimations] );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	StrafeController::InitialiseAnimations
//! Add the animations to the animator, initialise weights
//!
//------------------------------------------------------------------------------------------
void StrafeController::InitialiseAnimations( void )
{
	// Add all the animations to the animator here for now
	ActivateAnimations( &m_obStandingAnimation, 1 );

	// Add the movement ones only if required
	if ( m_bCanMove )
		ActivateAnimations( m_aobMovingAnims, MA_COUNT );

	// Add the lockon ones if required
	if ( m_bCanLockOn )
	{
		ActivateAnimations( m_aobTurningAnims, TA_COUNT );
		ActivateAnimations( m_aobLookingAnims, LA_COUNT );
	}

	// Initialise all our weights
	if ( m_bCanLockOn )
	{
		m_afTurningWeights[TA_TURN_LEFT] = 0.0f;
		m_afTurningWeights[TA_TURN_RIGHT] = 0.0f;
		m_afLookingWeights[LA_LOOK_RIGHT] = 0.0f;
		m_afLookingWeights[LA_LOOK_LEFT] = 0.0f;
	}

	if ( m_bCanMove )
	{
		m_afMovingWeights[MA_BACKWARDS] = 0.0f;
		m_afMovingWeights[MA_FORWARDS] = 0.0f;
		m_afMovingWeights[MA_RIGHT] = 0.0f;
		m_afMovingWeights[MA_LEFT] = 0.0f;
	}

	m_afStrafeWeights[SS_MOVING] = 0.0f;
	m_afStrafeWeights[SS_TURNING] = 0.0f;
	m_afStrafeWeights[SS_LOOKING] = 0.0f;
}


//------------------------------------------------------------------------------------------
//!
//!	StrafeController::Update
//! The main movement controller functionality
//!
//------------------------------------------------------------------------------------------
bool StrafeController::Update(	float						fTimeStep, 
								const CMovementInput&		obMovementInput,
								const CMovementStateRef&	obCurrentMovementState,
								CMovementState&				obPredictedMovementState )
{
	// If this is the first frame we need to set up our state
	if ( m_bFirstFrame )
	{
		// Add the animations to the animator, initialise weights
		InitialiseAnimations();

		// Initialise state stuff here
		if ( ( m_bCanMove ) && ( obCurrentMovementState.m_obLastRequestedVelocity.LengthSquared() > 0.0f ) )
			m_eStrafeState = SS_MOVING;
		else
			m_eStrafeState = SS_LOOKING;
	}

	// Ensure that the input we are running off is normalised
	CDirection obInputDirection( obMovementInput.m_obMoveDirection );
	obInputDirection.Normalise();

	// Ensure that we don't have a silly input speed
	float fInputSpeed = clamp( obMovementInput.m_fMoveSpeed, 0.0f, 1.0f );

	// If we have a target set up some values
	float fRequiredTurn = 0.0f;
	if ( obMovementInput.m_obFacingDirection.LengthSquared() > 0.0f )
	{
		// Find the required turn and claculate the weights
		m_obRequiredFaceDirection = obMovementInput.m_obFacingDirection;
		fRequiredTurn = MovementControllerUtilities::RotationAboutY( obCurrentMovementState.m_obFacing, m_obRequiredFaceDirection );
	}

	else if ( obMovementInput.m_bTargetPointSet )
	{
		// Find the required turn
		m_obRequiredFaceDirection = ( CDirection( obMovementInput.m_obTargetPoint ) - CDirection( obCurrentMovementState.m_obPosition ) );
		fRequiredTurn = MovementControllerUtilities::RotationAboutY( obCurrentMovementState.m_obFacing, m_obRequiredFaceDirection );
	}

	else if ( ( !m_bCanMove ) && ( obMovementInput.m_fMoveSpeed > m_obDefinition.m_fInputThreshold ) )
	{
		m_obRequiredFaceDirection = obMovementInput.m_obMoveDirection;
		fRequiredTurn = MovementControllerUtilities::RotationAboutY( obCurrentMovementState.m_obFacing, m_obRequiredFaceDirection );
	}

	else {} // We don't need to do anything if we have no facing direction

	// Update the state type stuff
	UpdateStateAndStateWeights( fTimeStep, fInputSpeed, fRequiredTurn );
		
	// Calculate the blend weights of our individual states
	CalculateLookWeights( fRequiredTurn, fTimeStep );
	CalculateTurnWeights( fTimeStep );
	CalculateMoveWeights( obCurrentMovementState.m_obRootMatrix, obInputDirection, fInputSpeed, fTimeStep );
		
	// Set the weights directly to our animations	
	if ( m_bCanLockOn )
	{
		m_aobLookingAnims[LA_LOOK_LEFT]->SetBlendWeight( m_fBlendWeight * m_afStrafeWeights[SS_LOOKING] * m_afLookingWeights[LA_LOOK_LEFT] );	
		m_aobLookingAnims[LA_LOOK_RIGHT]->SetBlendWeight( m_fBlendWeight * m_afStrafeWeights[SS_LOOKING] * m_afLookingWeights[LA_LOOK_RIGHT] );	

		m_aobTurningAnims[TA_TURN_LEFT]->SetBlendWeight( m_fBlendWeight * m_afStrafeWeights[SS_TURNING] * m_afTurningWeights[TA_TURN_LEFT] );	
		m_aobTurningAnims[TA_TURN_RIGHT]->SetBlendWeight( m_fBlendWeight * m_afStrafeWeights[SS_TURNING] * m_afTurningWeights[TA_TURN_RIGHT] );  
	}

	if ( m_bCanMove )
	{
		m_aobMovingAnims[MA_FORWARDS]->SetBlendWeight( m_fBlendWeight * m_afStrafeWeights[SS_MOVING] * m_afMovingWeights[MA_FORWARDS] );		
		m_aobMovingAnims[MA_BACKWARDS]->SetBlendWeight( m_fBlendWeight * m_afStrafeWeights[SS_MOVING] * m_afMovingWeights[MA_BACKWARDS] );	
		m_aobMovingAnims[MA_LEFT]->SetBlendWeight( m_fBlendWeight * m_afStrafeWeights[SS_MOVING] * m_afMovingWeights[MA_LEFT] );				
		m_aobMovingAnims[MA_RIGHT]->SetBlendWeight( m_fBlendWeight * m_afStrafeWeights[SS_MOVING] * m_afMovingWeights[MA_RIGHT] );			
	}

	// Finally calculate our standing weight - just what is left of our total blend
	float fStandWeight = m_fBlendWeight;

	// Remove the weights of the movement animations if necessary
	if ( m_bCanMove )
		fStandWeight = fStandWeight - m_aobMovingAnims[MA_FORWARDS]->GetBlendWeight()
									- m_aobMovingAnims[MA_BACKWARDS]->GetBlendWeight()
									- m_aobMovingAnims[MA_LEFT]->GetBlendWeight()
									- m_aobMovingAnims[MA_RIGHT]->GetBlendWeight();


	// Take these other animations into account too if necessary
	if ( m_bCanLockOn )
		fStandWeight = fStandWeight	- m_aobLookingAnims[LA_LOOK_LEFT]->GetBlendWeight()
									- m_aobLookingAnims[LA_LOOK_RIGHT]->GetBlendWeight()
									- m_aobTurningAnims[TA_TURN_LEFT]->GetBlendWeight()
									- m_aobTurningAnims[TA_TURN_RIGHT]->GetBlendWeight();

	// Set it and clamp for rounding issues
	m_obStandingAnimation->SetBlendWeight( clamp( fStandWeight, 0.0f, 1.0f ) );	

	// If we are moving or not capable of turning, we need to set a procedural turn
	if ( ( m_eStrafeState == SS_MOVING ) || ( !m_bCanLockOn ) )
	{
		// How much do we lerp per frame - written like this to show the ideal amount at the ideal framerate
		const float fLerpRequiredTurn = clamp( 0.06f * ( fTimeStep / ( 1.0f / 30.0f ) ), 0.0f, 1.0f );
		obPredictedMovementState.m_fProceduralYaw = CMaths::Lerp( 0.0f, fRequiredTurn, fLerpRequiredTurn );
	}

	// If we are not performing an animated turn we need to make sure no animated rotation is coming through
	if ( m_eStrafeState != SS_TURNING )
		obPredictedMovementState.m_fRootRotationDeltaScalar = 0.0f;

	ApplyGravity(m_obDefinition.m_bApplyGravity);
	
	// And we're done!
	m_bFirstFrame = false;

	// This controller never finishes - push it off
	return false;
}


//------------------------------------------------------------------------------------------
//!
//!	StrafeController::UpdateStateAndStateWeights
//!
//------------------------------------------------------------------------------------------
void StrafeController::UpdateStateAndStateWeights( float fTimeStep, float fInputSpeed, float fRequiredTurn )
{
	// How much do we lerp per frame - written like this to show the ideal amount at the ideal framerate
	const float fStateLerp = clamp( 0.06f * ( fTimeStep / ( 1.0f / 30.0f ) ), 0.0f, 1.0f );

	// switch on the current state
	switch ( m_eStrafeState )
	{
	case SS_TURNING: 
		// Check whether we should finish a turn
		if ( CheckEndTurn( fTimeStep ) )
		{
			if ( !CheckTurnStart( fRequiredTurn ) )
				m_eStrafeState = SS_LOOKING;
		}
		break;

	case SS_MOVING: 
		if ( fInputSpeed < m_obDefinition.m_fInputThreshold )
			m_eStrafeState = SS_LOOKING;
		break;

	case SS_LOOKING:
		if ( ( m_bCanLockOn ) && ( CheckTurnStart( fRequiredTurn ) ) )
			m_eStrafeState = SS_TURNING;
		else if ( ( m_bCanMove ) && ( fInputSpeed >= m_obDefinition.m_fInputThreshold ) )
			m_eStrafeState = SS_MOVING;
		break;

	default:
		ntAssert( 0 );
		break;
	}

	// Update the state weights
	m_afStrafeWeights[SS_TURNING] = ( m_eStrafeState == SS_TURNING ) ?
									CMaths::Lerp( m_afStrafeWeights[SS_TURNING], 1.0f, fStateLerp )
									: CMaths::Lerp( m_afStrafeWeights[SS_TURNING], 0.0f, fStateLerp );

	m_afStrafeWeights[SS_MOVING] = ( m_eStrafeState == SS_MOVING ) ?
									CMaths::Lerp( m_afStrafeWeights[SS_MOVING], 1.0f, fStateLerp )
									: CMaths::Lerp( m_afStrafeWeights[SS_MOVING], 0.0f, fStateLerp );

	m_afStrafeWeights[SS_LOOKING] = ( m_eStrafeState == SS_LOOKING ) ?
									CMaths::Lerp( m_afStrafeWeights[SS_LOOKING], 1.0f, fStateLerp )
									: CMaths::Lerp( m_afStrafeWeights[SS_LOOKING], 0.0f, fStateLerp );
}


//------------------------------------------------------------------------------------------
//!
//!	StrafeController::CheckEndTurn
//!
//------------------------------------------------------------------------------------------
bool StrafeController::CheckEndTurn( float fTimeStep )
{
	// If we are currently turning left...
	if ( m_bTurnLeft )
	{
		// The weight is dependant on how far through the animation we are
		if ( ( m_aobTurningAnims[TA_TURN_LEFT]->GetTime() + fTimeStep ) >= m_aobTurningAnims[TA_TURN_LEFT]->GetDuration() )
			return true;
	}

	// ...otherwise if we are turning right
	else
	{
		// The weight is dependant on how far through the animation we are
		if ( ( m_aobTurningAnims[TA_TURN_RIGHT]->GetTime() + fTimeStep ) >= m_aobTurningAnims[TA_TURN_RIGHT]->GetDuration() )
			return true;
	}

	// Otherwise we are still going
	return false;
}


//------------------------------------------------------------------------------------------
//!
//!	StrafeController::CheckTurnStart
//!
//------------------------------------------------------------------------------------------
bool StrafeController::CheckTurnStart( float fRequiredTurn )
{
	// Check whether or not we should start turning right
	if ( fRequiredTurn > ( m_obDefinition.m_pobAnimSet->m_fTurnPoint * DEG_TO_RAD_VALUE ) )
	{
		// Note the direction of our turn
		m_bTurnLeft = true;

		if (m_eStrafeState == SS_TURNING)
		{
			// If we're already turning, need to remove and re-add the aimation 
			m_pobAnimator->RemoveAnimation(m_aobTurningAnims[TA_TURN_LEFT]);
			m_pobAnimator->AddAnimation(m_aobTurningAnims[TA_TURN_LEFT]);
		}
		else
		{
			// Reset the time for this animation
			m_aobTurningAnims[TA_TURN_LEFT]->SetTime( 0.0f );
		}

		// Indicate we wish to turn
		return true;
	}

	// ...or maybe start turning left
	else if ( fRequiredTurn < ( -m_obDefinition.m_pobAnimSet->m_fTurnPoint * DEG_TO_RAD_VALUE ) )
	{
		// Note the direction of our turn
		m_bTurnLeft = false;

		if (m_eStrafeState == SS_TURNING)
		{
			// If we're already turning, need to remove and re-add the aimation 
			m_pobAnimator->RemoveAnimation(m_aobTurningAnims[TA_TURN_RIGHT]);
			m_pobAnimator->AddAnimation(m_aobTurningAnims[TA_TURN_RIGHT]);
		}
		else
		{
			// Reset the time for this animation
			m_aobTurningAnims[TA_TURN_RIGHT]->SetTime( 0.0f );
		}

		// Indicate we wish to turn
		return true;
	}

	// If we are here we don't want to turn
	return false;
}


//------------------------------------------------------------------------------------------
//!
//!	StrafeController::CalculateLookWeights
//! Works out the weights for the animations used when the character's root is not moving
//!
//------------------------------------------------------------------------------------------
void StrafeController::CalculateLookWeights( float fRequiredTurn, float fTimeStep )
{
	// If we have the twisting capabiltiy then do this
	if ( m_bCanLockOn )
	{
		// How much do we lerp per frame - written like this to show the ideal amount at the ideal framerate
		const float fLookLerp = clamp( 0.13f * ( fTimeStep / ( 1.0f / 30.0f ) ), 0.0f, 1.0f );

		// Sort out the looking weights
		m_afLookingWeights[LA_LOOK_LEFT] = CMaths::Lerp( m_afLookingWeights[LA_LOOK_LEFT], clamp( fRequiredTurn / ( m_obDefinition.m_pobAnimSet->m_fTurnPoint * DEG_TO_RAD_VALUE ), 0.0f, 1.0f ), fLookLerp );
		m_afLookingWeights[LA_LOOK_RIGHT] = CMaths::Lerp( m_afLookingWeights[LA_LOOK_RIGHT], clamp( fRequiredTurn / ( m_obDefinition.m_pobAnimSet->m_fTurnPoint * DEG_TO_RAD_VALUE ), -1.0f, 0.0f ) * -1.0f, fLookLerp );
	}

	// Otherwise we can't look
	else
	{
		m_afLookingWeights[LA_LOOK_LEFT] = 0.0f;
		m_afLookingWeights[LA_LOOK_RIGHT] = 0.0f;
	}
}


//------------------------------------------------------------------------------------------
//!
//!	StrafeController::CalculateTurnWeights
//!
//------------------------------------------------------------------------------------------
void StrafeController::CalculateTurnWeights( float fTimeStep )
{
	// if we have the capability to turn
	if ( m_bCanLockOn )
	{
		// How much do we lerp per frame - written like this to show the ideal amount at the ideal framerate
		const float fTurnLerp = clamp( 0.16f * ( fTimeStep / ( 1.0f / 30.0f ) ), 0.0f, 1.0f );

		// If our main state is not turning - check to see whether we should
		if ( m_eStrafeState != SS_TURNING )
		{
			// Drop the blend weights of these animations
			m_afTurningWeights[TA_TURN_LEFT] = CMaths::Lerp( m_afTurningWeights[TA_TURN_LEFT], 0.0f, fTurnLerp );
			m_afTurningWeights[TA_TURN_RIGHT] = CMaths::Lerp( m_afTurningWeights[TA_TURN_RIGHT], 0.0f, fTurnLerp );
		}

		else
		{
			// Increment the blend weight of the relevant animation
			if ( m_bTurnLeft )
				m_afTurningWeights[TA_TURN_LEFT] = CMaths::Lerp( m_afTurningWeights[TA_TURN_LEFT], 1.0f, fTurnLerp );
			else
				m_afTurningWeights[TA_TURN_RIGHT] = CMaths::Lerp( m_afTurningWeights[TA_TURN_RIGHT], 1.0f, fTurnLerp );
		}
	}

	// Otherwise we can't turn
	else
	{
		m_afTurningWeights[TA_TURN_LEFT] = 0.0f;
		m_afTurningWeights[TA_TURN_RIGHT] = 0.0f;
	}
}


//------------------------------------------------------------------------------------------
//!
//!	StrafeController::CalculateMoveWeights
//!
//------------------------------------------------------------------------------------------
void StrafeController::CalculateMoveWeights(	const CMatrix&		obRootMatrix, 
												const CDirection&	obMoveDirection, 
												float				fMoveSpeed, 
												float				fTimeStep )
{
	// Make sure this is not bothered with if we cannot move
	if ( m_bCanMove )
	{
		// How much do we lerp per frame - written like this to show the ideal amount at the ideal framerate
		const float fMoveLerp = clamp( 0.16f * ( fTimeStep / ( 1.0f / 30.0f ) ), 0.0f, 1.0f );

		// Calculate the movement - forward velocity
		float fFowardVelocity = clamp( fMoveSpeed * obRootMatrix.GetZAxis().Dot( obMoveDirection ), -1.0f, 1.0f );

		// Sideways velocity
		float fSidewaysVelocity = clamp( fMoveSpeed * obRootMatrix.GetXAxis().Dot( obMoveDirection ), -1.0f, 1.0f );

		// Calulate the movement animation weights
		m_afMovingWeights[MA_FORWARDS] = CMaths::Lerp( m_afMovingWeights[MA_FORWARDS], clamp( fFowardVelocity, 0.0f, 1.0f ), fMoveLerp );
		m_afMovingWeights[MA_BACKWARDS] = CMaths::Lerp( m_afMovingWeights[MA_BACKWARDS], clamp( fFowardVelocity, -1.0f, 0.0f ) * -1.0f, fMoveLerp );
		m_afMovingWeights[MA_RIGHT] = CMaths::Lerp( m_afMovingWeights[MA_RIGHT], clamp( fSidewaysVelocity, -1.0f, 0.0f ) * -1.0f, fMoveLerp );
		m_afMovingWeights[MA_LEFT] = CMaths::Lerp( m_afMovingWeights[MA_LEFT], clamp( fSidewaysVelocity, 0.0f, 1.0f ), fMoveLerp );
	}
}

//------------------------------------------------------------------------------------------
//!
//!	StrafeController::UpdateBlinking
//! Guess what this does...
//!
//------------------------------------------------------------------------------------------
void StrafeController::UpdateBlinking( float fTimeStep )
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

