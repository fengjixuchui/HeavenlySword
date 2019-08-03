//------------------------------------------------------------------------------------------
//!
//!	\file targetedtransition.cpp
//!
//------------------------------------------------------------------------------------------

#include "Physics/config.h"

// Necessary includes
#include "targetedtransition.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "game/entityinfo.h"
#include "anim/animator.h"
#include "input/inputhardware.h"
#include "objectdatabase/dataobject.h"
#include "movement.h"
#include "core/visualdebugger.h"
#include "core/exportstruct_anim.h"

#include "camera/camview.h"
#include "camera/camman.h"
#include "inputcomponent.h" // KO aftertouch control
#include "game/messagehandler.h" // Fall Aftertouch controll

#include "camera/camutils.h"

// TO REMOVE
#include "attacks.h"
#include "Physics/system.h"
#include "physics/world.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include "Physics/advancedcharactercontroller.h"
#endif

START_STD_INTERFACE	( ZAxisAlignTargetedTransitionDef )
	ISTRING		( ZAxisAlignTargetedTransitionDef, AnimationName )
	IBOOL		( ZAxisAlignTargetedTransitionDef, ApplyGravity )
	IPOINT		( ZAxisAlignTargetedTransitionDef, AlignZTo )
END_STD_INTERFACE

START_STD_INTERFACE	( StandardTargetedTransitionDef )
	IFLOAT		( StandardTargetedTransitionDef, MaximumTargetingRadius )
	IBOOL		( StandardTargetedTransitionDef, Reversed )
	IBOOL		( StandardTargetedTransitionDef, TrackTarget )
	IFLOAT		( StandardTargetedTransitionDef, MaximumRotationSpeed )
	ISTRING		( StandardTargetedTransitionDef, AnimationName )
	IFLOAT		( StandardTargetedTransitionDef, MovementDuration )
	IBOOL		( StandardTargetedTransitionDef, ApplyGravity )
	IBOOL		( StandardTargetedTransitionDef, Looping )
	IBOOL		( StandardTargetedTransitionDEf, StopWhenOnGround )
#ifndef _RELEASE
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
#endif
END_STD_INTERFACE

START_STD_INTERFACE	( ScaledTargetedTransitionDef )
	ISTRING		( ScaledTargetedTransitionDef, AnimationName )
	IFLOAT		( ScaledTargetedTransitionDef, MovementOffset )
	IFLOAT		( ScaledTargetedTransitionDef, MaxRange )
	IFLOAT		( ScaledTargetedTransitionDef, MovementDuration )
	IBOOL		( ScaledTargetedTransitionDef, ApplyGravity )
	IFLOAT		( ScaledTargetedTransitionDef, ScaleToTime )
	IBOOL		( ScaledTargetedTransitionDef, TrackAfterScale )
	IBOOL		( ScaledTargetedTransitionDef, 3DScaling )
	IBOOL		( ScaledTargetedTransitionDef, ScaleDown )
#ifndef _RELEASE
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
#endif
END_STD_INTERFACE

START_STD_INTERFACE	( BlockTargetedTransitionDef )
	IBOOL		( BlockTargetedTransitionDef, StayAligned )
	ISTRING		( BlockTargetedTransitionDef, AnimationName )
	IFLOAT		( BlockTargetedTransitionDef, MovementOffset )
	IFLOAT		( BlockTargetedTransitionDef, AngleToTargetTime )
	IFLOAT		( BlockTargetedTransitionDef, AngleToTarget )
	IFLOAT		( BlockTargetedTransitionDef, MovementDuration )
	IBOOL		( BlockTargetedTransitionDef, ApplyGravity )
	IBOOL		( BlockTargetedTransitionDef, SyncInitialHeight )
#ifndef _RELEASE
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
#endif
END_STD_INTERFACE

//------------------------------------------------------------------------------------------
//!
//!	StandardTargetedTransitionDef::StandardTargetedTransitionDef
//!	Construction
//!
//------------------------------------------------------------------------------------------
StandardTargetedTransitionDef::StandardTargetedTransitionDef( void )
:	m_fMaximumTargetingRadius( 0.0f ),
	m_bReversed( false ),
	m_bTrackTarget( false ),
	m_fMaximumRotationSpeed( 0.0f ),
	m_obAnimationName(),
	m_fMovementDuration( 0.0f ),
	m_bApplyGravity( false ),
	m_bLooping( false ),
	m_bStopWhenOnGround( false )
{
}


//------------------------------------------------------------------------------------------
//!
//!	StandardTargetedTransitionDef::CreateInstance
//!	To create an instance of what we describe
//!
//------------------------------------------------------------------------------------------
MovementController* StandardTargetedTransitionDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) StandardTargetedTransition( pobMovement, *this );
}


//------------------------------------------------------------------------------------------
//!
//!	StandardTargetedTransition::StandardTargetedTransition
//!	Construction
//!
//------------------------------------------------------------------------------------------
StandardTargetedTransition::StandardTargetedTransition(	CMovement*								pobMovement, 
														const StandardTargetedTransitionDef&	obDefinition )

:	MovementController( pobMovement ),
	m_obDefinition( obDefinition ),
	m_obSingleAnimation(),
	m_fMovementDurationRemaining( 0.0f ),
	m_fTransitionTime( 0.0f ),
	m_bZeroDistanceReached( false )
{
	// Register our definition for debugging purposes
	InternalRegisterDefinition( m_obDefinition );

	// Create our animation and add it to the animator
	m_obSingleAnimation = m_pobAnimator->CreateAnimation( obDefinition.m_obAnimationName );
	m_obSingleAnimation->SetBlendWeight( 0.0f );

	// Need to set a looping animation flag?
	if ( m_obDefinition.m_bLooping )
		m_obSingleAnimation->SetFlagBits( ANIMF_LOOPING | ANIMF_LOCOMOTING | ANIMF_INHIBIT_AUTO_DESTRUCT );
	else
		m_obSingleAnimation->SetFlagBits( ANIMF_LOCOMOTING | ANIMF_INHIBIT_AUTO_DESTRUCT );

	// We count down the movement duration
	m_fMovementDurationRemaining = m_obDefinition.m_fMovementDuration;

	// If the movement duration is zero we need to set it to the length of the animation we have created
	if ( m_obDefinition.m_fMovementDuration == 0.0f )
		m_obDefinition.m_fMovementDuration = m_obSingleAnimation->GetDuration();
}


//------------------------------------------------------------------------------------------
//!
//!	StandardTargetedTransition::~StandardTargetedTransition
//!	Destruction
//!
//------------------------------------------------------------------------------------------
StandardTargetedTransition::~StandardTargetedTransition( void )
{
	// Only remove the animation if we have done an update
	if ( !m_bFirstFrame && m_obSingleAnimation->IsActive() )
		m_pobAnimator->RemoveAnimation( m_obSingleAnimation );
}


//------------------------------------------------------------------------------------------
//!
//!	StandardTargetedTransition::Update
//!	
//!
//------------------------------------------------------------------------------------------
bool StandardTargetedTransition::Update(	float						fTimeStep, 
											const CMovementInput&		obMovementInput,
											const CMovementStateRef&	obCurrentMovementState,
											CMovementState&				obPredictedMovementState )
{
	// Add the animation and set the speed on our first update
	if ( m_bFirstFrame )
	{
		obPredictedMovementState.m_fProceduralYaw = 0.0;
		obPredictedMovementState.m_obProceduralRootDelta.Clear();

		m_pobAnimator->AddAnimation( m_obSingleAnimation );

		if ( ( !m_obDefinition.m_bLooping ) && ( m_fMovementDurationRemaining != 0.0f ) )
			m_obSingleAnimation->SetTimeRemaining( m_fMovementDurationRemaining );
	}

	// Update the time in this transition
	m_fTransitionTime += fTimeStep;

	// Set the animation weight
	m_obSingleAnimation->SetBlendWeight( m_fBlendWeight );

	// Check to see if we have been handed a target to aim at
	if ( obMovementInput.m_bTargetPointSet && ( !m_bZeroDistanceReached ) && ( m_obDefinition.m_bTrackTarget ) )
	{
		// Angular velocity - Create a direction describing where we are currently pointing
		CDirection obCurrentDirection = obCurrentMovementState.m_obRootMatrix.GetZAxis();
		obCurrentDirection.Y() = 0.0f;

		// Create a direction describing where we want to point
		CPoint obLockonPosition = obMovementInput.m_obTargetPoint;
		CDirection obPredictedDirection( obLockonPosition - obCurrentMovementState.m_obRootMatrix.GetTranslation() );
		obPredictedDirection.Y() = 0.0f;

		// This may be scripted so that we remain facing away from the target
		if ( m_obDefinition.m_bReversed )
			obPredictedDirection *= -1.0f;

		// Check to see if we have got very close to the target
		m_bZeroDistanceReached = ( obPredictedDirection.Length() < m_obDefinition.m_fMaximumTargetingRadius );

		// Get the angle that we need to turn through to point in the correct direction
		float fTurningAngle = MovementControllerUtilities::RotationAboutY( obCurrentDirection, obPredictedDirection );

		// If this is not the first frame then we may need to limit the amount of turn that can be achieved,
		// for example when tracking a moving character, it can look silly during time slowdown
		if ( !m_bFirstFrame )
		{
			if ( fTurningAngle > ( m_obDefinition.m_fMaximumRotationSpeed * DEG_TO_RAD_VALUE * fTimeStep ) )
				fTurningAngle = ( m_obDefinition.m_fMaximumRotationSpeed * DEG_TO_RAD_VALUE * fTimeStep );

			if ( -fTurningAngle > ( m_obDefinition.m_fMaximumRotationSpeed * DEG_TO_RAD_VALUE * fTimeStep ) )
					fTurningAngle = -( m_obDefinition.m_fMaximumRotationSpeed * DEG_TO_RAD_VALUE * fTimeStep );
		}

		// Set the procedural root rotation
		if ( !m_bZeroDistanceReached )
			obPredictedMovementState.m_fProceduralYaw = fTurningAngle;

		// Clear the procedural root movement
		obPredictedMovementState.m_obProceduralRootDelta.Clear();
	}
	else
	{
		// If we have no target, clear the remainders we are simply playing the animation
		obPredictedMovementState.m_fProceduralYaw = 0.0f;
		obPredictedMovementState.m_obProceduralRootDelta.Clear();
	}

	// Apply gravity if requested
	ApplyGravity(m_obDefinition.m_bApplyGravity);

	// If we have completed the attack without interuption then drop out to our return controller
	if ( ( !m_bFirstFrame ) && ( m_fTransitionTime >= m_obDefinition.m_fMovementDuration ) && !m_obDefinition.m_bLooping )
	{
		// We have completed our movement
		return true;
	}

	// If we have been asked to stop this movement when we hit the ground
	if ( ( !m_bFirstFrame ) && ( m_obDefinition.m_bStopWhenOnGround ) && ( obCurrentMovementState.m_bOnGround ) )
	{
		// We have completed our movement
		return true;
	}

	// Remove the first frame flag if it is set
	if ( m_bFirstFrame )
		m_bFirstFrame = false;

	// This return mechanism needs to be used to move to the next controller in future
	return false;
}


//------------------------------------------------------------------------------------------
//!
//!	ScaledTargetedTransitionDef::ScaledTargetedTransitionDef
//!	Construction
//!
//------------------------------------------------------------------------------------------
ScaledTargetedTransitionDef::ScaledTargetedTransitionDef( void )
:	m_obAnimationName(),
	m_fMovementOffset( 0.0f ),
	m_fMaxRange( 0.0f ),
	m_fMovementDuration( 0.0f ),
	m_bApplyGravity( false ),
	m_fScaleToTime( 0.0f ),
	m_bTrackAfterScale( true ),
	m_b3DScaling( false ),
	m_bScaleDown( false ),
	m_bNoDirectionCorrectionScaling( false ),
	m_bNoRotateIfTargetBehind( false )
{
}


//------------------------------------------------------------------------------------------
//!
//!	ScaledTargetedTransitionDef::CreateInstance
//!	To create an instance of what we describe
//!
//------------------------------------------------------------------------------------------
MovementController* ScaledTargetedTransitionDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) ScaledTargetedTransition( pobMovement, *this );
}


//------------------------------------------------------------------------------------------
//!
//!	ScaledTargetedTransition::ScaledTargetedTransition
//!	Construction
//!
//------------------------------------------------------------------------------------------
ScaledTargetedTransition::ScaledTargetedTransition(	CMovement*							pobMovement, 
													const ScaledTargetedTransitionDef&	obDefinition )

:	MovementController( pobMovement ),
	m_obDefinition( obDefinition ),
	m_obSingleAnimation(),
	m_fMovementDurationRemaining( 0.0f ),
	m_fTransitionTime( 0.0f ),
	m_fAnimationRotation( 0.0f ),
	m_fAnimationXOffset( 0.0f ),
	m_fAnimationZOffset( 0.0f ),
	m_obInitialPosition( CONSTRUCT_IDENTITY ),
	m_fSumOfProceduralTwist( 0.0f ),
	m_obAnimRootScalar( CONSTRUCT_CLEAR ),
	m_obAnimationDistance( CONSTRUCT_CLEAR ),
	m_fScaleClipped( false ),
	m_fAnimationTimeScalar( 0.0f ),
	m_fAdditionalProceduralTwist( 0.0f )
{
	// Register our definition for debugging purposes
	InternalRegisterDefinition( m_obDefinition );

	// Create our animation and add it to the animator
	if (m_obDefinition.m_pobAnimation.GetPtr())
	{
		m_obSingleAnimation = m_obDefinition.m_pobAnimation;
	}
	else
	{
		m_obSingleAnimation = m_pobAnimator->CreateAnimation( obDefinition.m_obAnimationName );
	}

	m_obSingleAnimation->SetBlendWeight( 0.0f );
	m_obSingleAnimation->SetFlagBits( ANIMF_LOCOMOTING | ANIMF_INHIBIT_AUTO_DESTRUCT );

	// We count down the movement duration
	m_fMovementDurationRemaining = m_obDefinition.m_fMovementDuration;

	if ( m_fMovementDurationRemaining == 0.0f )
		m_fMovementDurationRemaining = m_obSingleAnimation->GetDuration();

	// By how much is the animation being scaled
	m_fAnimationTimeScalar = m_fMovementDurationRemaining / m_obSingleAnimation->GetDuration();
}


//------------------------------------------------------------------------------------------
//!
//!	ScaledTargetedTransition::~ScaledTargetedTransition
//!	Destruction
//!
//------------------------------------------------------------------------------------------
ScaledTargetedTransition::~ScaledTargetedTransition( void )
{
	// Only remove the animation if we have done an update
	if ( !m_bFirstFrame && m_obSingleAnimation->IsActive() )
		m_pobAnimator->RemoveAnimation( m_obSingleAnimation );
}


//------------------------------------------------------------------------------------------
//!
//!	ScaledTargetedTransition::Update
//!	This transition takes information from an external system.  We have are given a target 
//! location, a duration and an animiser.  What we do is scale the root movement to give us 
//! the coreect amount of movment and procedurally turn the character so we end up in the 
//! correct position.
//!
//------------------------------------------------------------------------------------------
bool ScaledTargetedTransition::Update(	float						fTimeStep, 
										const CMovementInput&		obMovementInput,
										const CMovementStateRef&	obCurrentMovementState,
										CMovementState&				obPredictedMovementState )
{
	// Add the animation and set the speed on our first update
	if ( m_bFirstFrame )
	{
		m_pobAnimator->AddAnimation( m_obSingleAnimation );
		m_obSingleAnimation->SetTimeRemaining( m_fMovementDurationRemaining );

		/*
		// PRINT OUT SOME DEBUG INFORMATION ABOUT THIS ATTACK
		ntPrintf( "AttackTime: %0.2f, StrikeTime: %0.2f, EndZ: %0.2f, StrikeZ: %0.2f \n", 
				m_obDefinition.m_fMovementDuration,
				m_obDefinition.m_fScaleToTime * m_fAnimationTimeScalar,
				m_obSingleAnimation->GetRootEndTranslation().Z(),
				m_obSingleAnimation->GetRootTranslationAtTime( m_obDefinition.m_fScaleToTime ).Z() );
				*/
	}

	// Save our initial facing direction and position
	if ( m_bFirstFrame )
	{
		m_obInitialPosition = obCurrentMovementState.m_obRootMatrix;
	}

	// Update the time in this transition
	m_fTransitionTime += fTimeStep;

	// Set the animation weight
	m_obSingleAnimation->SetBlendWeight( m_fBlendWeight );

	// This will be our target location if we have a valid target
	CPoint obCurrentRelativePosition( CONSTRUCT_CLEAR );
	CPoint obInitialRelativePosition( CONSTRUCT_CLEAR );

	// Check to see if we have something to head at
	if ( obMovementInput.m_bTargetPointSet )
	{
		// Find the position of our target relative to our own
		obCurrentRelativePosition = ( obMovementInput.m_obTargetPoint - obCurrentMovementState.m_obRootMatrix.GetTranslation() );

		//g_VisualDebug->RenderLine( obCurrentMovementState.m_obRootMatrix.GetTranslation(), obCurrentMovementState.m_obRootMatrix.GetTranslation() + obCurrentRelativePosition, DC_RED );

		// Find the position of our target relative to our own
		obInitialRelativePosition = ( obMovementInput.m_obTargetPoint - m_obInitialPosition.GetTranslation() );

		// The code in this section deals with the distance scaling of the characters movement
		{
			// If we are within the times given for scaling then we do some further calculations
			if ( m_fTransitionTime < ( m_obDefinition.m_fScaleToTime * m_fAnimationTimeScalar ) || m_obDefinition.m_bTrackAfterScale ) 
			{
				if ( m_bFirstFrame )
				{
					// Find the total movement that the animation will give us.
					m_fAnimationRotation =	MovementControllerUtilities::GetYRotation
											(
												m_obSingleAnimation->GetRootRotationAtTime(	m_obDefinition.m_fScaleToTime,
																							m_pobAnimator != NULL ? m_pobAnimator->GetHierarchy() : NULL )
											);

					// Find the total distance moved by the animation
					m_obAnimationDistance = m_obSingleAnimation->GetRootTranslationAtTime( m_obDefinition.m_fScaleToTime, m_pobAnimator != NULL ? m_pobAnimator->GetHierarchy() : NULL );
				}

				// If we are doing this in 3D then the scaling is more complex
				if ( m_obDefinition.m_b3DScaling )
				{
					ntError( obMovementInput.m_bTargetPointSet );

					// Calculate a scalar for the animation - what is the total distance to move - Multiply the point
					// and not the direction - direction multiplication doesn't do the transform
					CDirection obDistanceToMove = CDirection( obMovementInput.m_obTargetPoint * m_obInitialPosition.GetAffineInverse() );
					obDistanceToMove.Z() -= m_obDefinition.m_fMovementOffset;

					// Scale in the x axis - checking for divide by zeros
					if ( ( m_obAnimationDistance.X() > EPSILON ) || ( m_obAnimationDistance.X() < -EPSILON ) )
						m_obAnimRootScalar.X() = obDistanceToMove.X() / m_obAnimationDistance.X();
					else
						m_obAnimRootScalar.X() = 1.0f;

					// Scale in the y axis - checking for divide by zeros
					if ( ( m_obAnimationDistance.Y() > EPSILON ) || ( m_obAnimationDistance.Y() < -EPSILON ) )
						m_obAnimRootScalar.Y() = obDistanceToMove.Y() / m_obAnimationDistance.Y();
					else
						m_obAnimRootScalar.Y() = 1.0f;

					// Scale in the z axis - checking for divide by zeros
					if ( ( m_obAnimationDistance.Z() > EPSILON ) || ( m_obAnimationDistance.Z() < -EPSILON ) )
						m_obAnimRootScalar.Z() = obDistanceToMove.Z() / m_obAnimationDistance.Z();
					else
						m_obAnimRootScalar.Z() = 1.0f;
				}

				// If we are assuming 2D movement then we need to check bounds
				else
				{
					// Calculate a scalar for the animation - what is the total distance to move
					float fDistanceToMove = obInitialRelativePosition.Length() - m_obDefinition.m_fMovementOffset;

					// The possible range of this movement may be restricted - don't scale backwards
					if ( ( m_obDefinition.m_fMaxRange > 0.0f ) && ( fDistanceToMove > 0.0f ) )
					{	
						// If we have a valid restriction crop the movement if necessary
						if ( fDistanceToMove > m_obDefinition.m_fMaxRange )
						{
							// NumberOfTimesDesignChanged = 5;
							fDistanceToMove = m_obDefinition.m_fMaxRange;
						}

						// So the scaling factor is...
						if ( ( m_obAnimationDistance.Z() > EPSILON ) || ( m_obAnimationDistance.Z() < -EPSILON ) )
							m_obAnimRootScalar = CDirection( 1.0f, 1.0f, ( fDistanceToMove / m_obAnimationDistance.Z() ) );
						else
							m_obAnimRootScalar = CDirection( 1.0f, 1.0f, 1.0f );

						// If we have a ludicrous scalar then we cap it 
						if ( m_obAnimRootScalar.Z() > 4.0f )
						{
							m_obAnimRootScalar.Z() = 4.0f;
							m_fScaleClipped = true;
						}
					}

					// If no maximum distance was set then we don't scale at all
					else
						m_obAnimRootScalar = CDirection( 1.0f, 1.0f, 1.0f );
				}
			}
			
			// If we are outside of the allowed scaling time then the scalar is unity
			else
				m_obAnimRootScalar = CDirection( 1.0f, 1.0f, 1.0f );
		}
		// End distance scaling section

		// The code in this section deals with the orientation of the character
		{
			// Stop targeting after the scale-to time - need to try this out
			if ( m_fTransitionTime < ( m_obDefinition.m_fScaleToTime * m_fAnimationTimeScalar ) || m_obDefinition.m_bTrackAfterScale )
			{
				// This section of code deals with any procedural twisting that is required	- it also ensures the movement of the character is along a straight vector while they're twisting			
				// How far left over?
				CDirection obToTarget( obCurrentRelativePosition );
				if (!m_obDefinition.m_b3DScaling)
					obToTarget.Y() = 0.0f;
				obToTarget.Normalise();

				float fAngle = MovementControllerUtilities::RotationAboutY(	obCurrentMovementState.m_obFacing, obToTarget );
				if (!(fabs(fAngle) > 90.0f * DEG_TO_RAD_VALUE && m_obDefinition.m_bNoRotateIfTargetBehind))
				{
					// How much time left over?
					float fRemainingTime = ( m_obDefinition.m_fScaleToTime * m_fAnimationTimeScalar ) - m_fTransitionTime;
					// How fast do we need to go?
					float fRequiredRotationSpeed = fAngle / fRemainingTime;
					// Do it
					// But if we're tracking after our scale time, we just do the necessary yaw instantly cos the remaining time is gonna be meaningless
					if ( m_obDefinition.m_bTrackAfterScale && fRemainingTime <= 0.0f )
						obPredictedMovementState.m_fProceduralYaw = fAngle;	
					// Otherwise rotate as fast as we need to
					else if ( fRemainingTime > 0.0f )
						obPredictedMovementState.m_fProceduralYaw = fRequiredRotationSpeed * fTimeStep;
					// Or if there's no time left, do none at all (shouldn't ever get here logically though)
					else
						obPredictedMovementState.m_fProceduralYaw = 0.0f;
				}
				
				float fDistanceToMove = obInitialRelativePosition.Length() - m_obDefinition.m_fMovementOffset;
				if (( m_obDefinition.m_fMaxRange > 0.0f ) && ( fDistanceToMove > 0.0f ) && fDistanceToMove < m_obDefinition.m_fMaxRange && !m_obDefinition.m_bNoDirectionCorrectionScaling)
				{
					// Clear out anim root movement so we can do it ourselves
					obPredictedMovementState.m_obRootDeltaScalar.Clear();

					// Do some root movement ourselves
					float fRequiredMovementSpeed = fDistanceToMove / (m_obDefinition.m_fScaleToTime * m_fAnimationTimeScalar); //CDirection( obCurrentRelativePosition ).Length() / fRemainingTime;
					CPoint obMovementLastFrame = m_obSingleAnimation->GetRootTranslationDelta();
					float fApproxCurrentMovementSpeed = obMovementLastFrame.Length() / fTimeStep;
					float fScalar = 0.0f;
					if ( fApproxCurrentMovementSpeed != 0.0f && fabs( fApproxCurrentMovementSpeed ) > EPSILON )
						fScalar = fRequiredMovementSpeed / fApproxCurrentMovementSpeed;
					obPredictedMovementState.m_obProceduralRootDelta = obToTarget;
					obPredictedMovementState.m_obProceduralRootDelta.Normalise();
					obPredictedMovementState.m_obProceduralRootDelta *= fApproxCurrentMovementSpeed * fScalar * fTimeStep;
				}
			}
		}
		// End rotation section

		// Some debugging information for this movement controller
#ifndef _GOLD_MASTER
		if ( CInputHardware::Get().GetContext() == INPUT_CONTEXT_COMBAT )
		{
			// If it is Sai that has built stuff then tell him if scale clipping occurs
			if ( m_fScaleClipped )
				g_VisualDebug->Printf2D(	100.0f, 100.0f, 0xffff0000, 0, "Movement Scale Exceeded, MaxRange: %0.2f, StrikeDistance: %0.2f",
												m_obDefinition.m_fMaxRange,
												m_obAnimationDistance.Z() );
		}
#endif
	}

	// If we have no target information we will use the movement input for direction only
	else 
	{
		// I'm  sorry about the next line - they made me do it
		if ( m_bFirstFrame && ( obMovementInput.m_fMoveSpeed > 0.5f ) )
		{
			// Find the total movement that the animation will give us.
			m_fAnimationRotation = MovementControllerUtilities::GetYRotation( m_obSingleAnimation->GetRootEndRotation() );
			m_fAnimationZOffset = m_obSingleAnimation->GetRootEndTranslation().Z();
			m_fAnimationXOffset = m_obSingleAnimation->GetRootEndTranslation().X();
			
			// Find the angle between our current facing direction and the movement input
			float fFullTurn = MovementControllerUtilities::RotationAboutY(	obCurrentMovementState.m_obRootMatrix.GetZAxis(), obMovementInput.m_obMoveDirection );

			// Take away the remaining turn provided by the animation
			fFullTurn -= m_fAnimationRotation;

			// Provide a procedural rotation
			obPredictedMovementState.m_fProceduralYaw = fFullTurn;
		}

		// No procedural twist after the first frame
		else
		{
			// Clear out any rotation if we don't have a target
			obPredictedMovementState.m_fProceduralYaw = 0.0f;
		}

		// If the maximum range is less than the animated distance we scale
		if ( ( m_obDefinition.m_bScaleDown ) && ( m_obDefinition.m_fMaxRange < m_obSingleAnimation->GetRootEndTranslation().Z() ) )
			obPredictedMovementState.m_obRootDeltaScalar.Z() = m_obDefinition.m_fMaxRange / ( m_obSingleAnimation->GetRootEndTranslation().Z() );
	}

	// Apply gravity if requested
	ApplyGravity(m_obDefinition.m_bApplyGravity);

	// If we have completed the attack without interuption then drop out to our return controller
	if ( ( !m_bFirstFrame ) && ( m_fTransitionTime >= m_obDefinition.m_fMovementDuration ) )
	{
		// We have completed our movement
		return true;
	}

	// Remove the first frame flag if it is set
	if ( m_bFirstFrame )
		m_bFirstFrame = false;

	// This return mechanism needs to be used to move to the next controller in future
	return false;
}


//------------------------------------------------------------------------------------------
//!
//!	BlockTargetedTransitionDef::BlockTargetedTransitionDef
//!	Construction
//!
//------------------------------------------------------------------------------------------
BlockTargetedTransitionDef::BlockTargetedTransitionDef( void )
:	m_bStayAligned( false ),
	m_obAnimationName(),
	m_fMovementOffset( 0.0f ),
	m_fAngleToTargetTime( 0.0f ),
	m_fAngleToTarget( 0.0f ),
	m_fMovementDuration( 0.0f ),
	m_bApplyGravity( false ),
	m_bSyncInitialHeight( false ),
	m_bTurnToFaceTarget( true )
{
}


//------------------------------------------------------------------------------------------
//!
//!	BlockTargetedTransitionDef::CreateInstance
//!	To create an instance of what we describe
//!
//------------------------------------------------------------------------------------------
MovementController* BlockTargetedTransitionDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) BlockTargetedTransition( pobMovement, *this );
}


//------------------------------------------------------------------------------------------
//!
//!	BlockTargetedTransition::BlockTargetedTransition
//!	Construction
//!
//------------------------------------------------------------------------------------------
BlockTargetedTransition::BlockTargetedTransition(	CMovement*							pobMovement, 
													const BlockTargetedTransitionDef&	obDefinition )

:	MovementController( pobMovement ),
	m_obDefinition( obDefinition ),
	m_obSingleAnimation(),
	m_fMovementDurationRemaining( 0.0f ),
	m_fProceduralMovementTime( 0.0f )
{
	// Register our definition for debugging purposes
	InternalRegisterDefinition( m_obDefinition );

	// Create our animation and add it to the animator
	m_obSingleAnimation = m_pobAnimator->CreateAnimation( obDefinition.m_obAnimationName );
	m_obSingleAnimation->SetBlendWeight( 0.0f );
	m_obSingleAnimation->SetFlagBits( ANIMF_LOCOMOTING | ANIMF_INHIBIT_AUTO_DESTRUCT );

	// We count down the movement duration
	m_fMovementDurationRemaining = m_obDefinition.m_fMovementDuration;
	if ( m_fMovementDurationRemaining == 0.0f )
		m_fMovementDurationRemaining = m_obSingleAnimation->GetDuration();
}


//------------------------------------------------------------------------------------------
//!
//!	BlockTargetedTransition::~BlockTargetedTransition
//!	Destruction
//!
//------------------------------------------------------------------------------------------
BlockTargetedTransition::~BlockTargetedTransition( void )
{
	// Only remove the animation if we have done an update
	if ( !m_bFirstFrame && m_obSingleAnimation->IsActive() )
		m_pobAnimator->RemoveAnimation( m_obSingleAnimation );
}


//------------------------------------------------------------------------------------------
//!
//!	BlockTargetedTransition::Update
//!	
//!
//------------------------------------------------------------------------------------------
bool BlockTargetedTransition::Update(	float						fTimeStep, 
										const CMovementInput&		obMovementInput,
										const CMovementStateRef&	obCurrentMovementState,
										CMovementState&				obPredictedMovementState )
{
	// Add the animation and set the speed on our first update
	if ( m_bFirstFrame )
	{
		m_pobAnimator->AddAnimation( m_obSingleAnimation );
		m_obSingleAnimation->SetTimeRemaining( m_fMovementDurationRemaining );
	}

	// Decrease the remaining duration
	m_fMovementDurationRemaining -= fTimeStep;
	if ( m_fMovementDurationRemaining < fTimeStep )
		m_fMovementDurationRemaining = fTimeStep;

	// Set the animation weight
	m_obSingleAnimation->SetBlendWeight( m_fBlendWeight );

// #define _DEBUG_BLOCK_MOVEMENT
#ifdef _DEBUG_BLOCK_MOVEMENT
	if ( m_bFirstFrame )
	{
		// Lets see what sort of movement the animation gives us
		ntPrintf( "X Animation Translation: %f. \n", m_obSingleAnimation->GetRootEndTranslation().X() ) );
		ntPrintf( "Z Animation Translation: %f. \n", m_obSingleAnimation->GetRootEndTranslation().Z() );
	}
#endif

	// Check to see if we have been handed a target to aim at
	if ( obMovementInput.m_bTargetPointSet && m_obDefinition.m_bTurnToFaceTarget )
	{
		// Increment the time taken to do the alignment
		m_fProceduralMovementTime += fTimeStep;

		// This code deals with the rotation of the character
		{
			// Create an idealised direction describing where we are facing
			CDirection obCurrentDirection = obCurrentMovementState.m_obRootMatrix.GetZAxis();
			obCurrentDirection.Y() = 0.0f;

			// Create an idealised direction defining where the opponent is relative to ourselves
			CPoint obOpponentPosition = obMovementInput.m_obTargetPoint;
			CDirection obOpponentDirection( obOpponentPosition - obCurrentMovementState.m_obRootMatrix.GetTranslation() );
			obOpponentDirection.Y() = 0.0f;

			// Find the angle between ourselves and our opponent
			float fOpponentAngle = MovementControllerUtilities::RotationAboutY( obCurrentDirection, obOpponentDirection );

			// Find the angle we need to move through to get to the requested angle
			float fAngleToMoveThrough = m_obDefinition.m_fAngleToTarget + fOpponentAngle;

			// Check the bounds
			if ( fAngleToMoveThrough >= PI ) fAngleToMoveThrough = -( TWO_PI - fmodf( fAngleToMoveThrough, TWO_PI ) );
			if ( fAngleToMoveThrough < -PI ) fAngleToMoveThrough = ( TWO_PI + fmodf( fAngleToMoveThrough, TWO_PI ) );

			// Find how much of the movement we should make in this frame
			if ( m_fProceduralMovementTime < m_obDefinition.m_fAngleToTargetTime )
			{
				fAngleToMoveThrough *= ( fTimeStep / ( m_obDefinition.m_fAngleToTargetTime - m_fProceduralMovementTime ) );
			}

			// Set the angle to the procedural rotation parameter
			if ( ( m_fProceduralMovementTime < m_obDefinition.m_fAngleToTargetTime ) || m_bFirstFrame || m_obDefinition.m_bStayAligned )
			{
				obPredictedMovementState.m_fProceduralYaw = fAngleToMoveThrough;
			}
			else
				obPredictedMovementState.m_fProceduralYaw = 0.0f;
		}

		// This code deals with the movement of the characater	
		if ( ( m_fProceduralMovementTime < m_obDefinition.m_fAngleToTargetTime ) || m_bFirstFrame )
		{
			// Find the direction of us from the target
			CPoint obRelativePosition( obCurrentMovementState.m_obRootMatrix.GetTranslation() - obMovementInput.m_obTargetPoint );

			// Convert the shunt vector into character space
			CDirection obCharacterShunt( obRelativePosition );

			// Set the length of the shunt to unity
			obCharacterShunt.Normalise();

			// This first frame addition allows us to snap a character into position if there is no clear time period
			if ( m_bFirstFrame && ( m_obDefinition.m_fAngleToTargetTime == 0 ) )
			{
				// The shunt is performed wholey in this frame
				obCharacterShunt *= m_obDefinition.m_fMovementOffset;
			}
			else
			{
				// Multiply the length by the distance requested - divide by the time frame
				obCharacterShunt *= ( ( m_obDefinition.m_fMovementOffset * fTimeStep ) / m_obDefinition.m_fAngleToTargetTime );
			}

			// Set this value as the procedural movement parameter
			obPredictedMovementState.m_obProceduralRootDelta = obCharacterShunt;

			// If this is the first frame we may need to do some vertical alignment
			if ( ( m_bFirstFrame ) && ( m_obDefinition.m_bSyncInitialHeight ) )
			{
				float fVerticalDistanceToCentrePoint = obMovementInput.m_obTargetPoint.Y() - obCurrentMovementState.m_obPosition.Y();
				obPredictedMovementState.m_obProceduralRootDelta.Y() += fVerticalDistanceToCentrePoint;
			}

#ifdef _DEBUG_BLOCK_MOVEMENT
			// Print out the procedural movement details
			ntPrintf( "X Procedural Translation: %f. \n", obCharacterShunt.X() );
			ntPrintf( "Z Procedural Translation: %f. \n", obCharacterShunt.Z() );
			ntPrintf( "Movement Offset: %f. \n", m_obDefinition.m_fMovementOffset );
#endif
		}

		else
		{
			// After the initial alignment time we leave the animation to do its thing
			obPredictedMovementState.m_obProceduralRootDelta.Clear();
		}
	}

	// If we do not have a target entity there is no procedural work to do.
	else
	{
		// If we have no target, clear the remainders we are simply playing the animation
		obPredictedMovementState.m_fProceduralYaw = 0.0f;
		obPredictedMovementState.m_obProceduralRootDelta.Clear();
	}

	// Gravity setting
	ApplyGravity(m_obDefinition.m_bApplyGravity);

	// If we have completed the attack without interuption then drop out to our return controller
	if ( ( !m_bFirstFrame ) && ( m_fMovementDurationRemaining <= fTimeStep ) )
	{
		// We have completed our movement
		return true;
	}

	// Remove the first frame flag if it is set
	if ( m_bFirstFrame )
		m_bFirstFrame = false;

	// This return mechanism needs to be used to move to the next controller in future
	return false;
}


//------------------------------------------------------------------------------------------
//!
//!	KOAftertouchTransitionDef::KOAftertouchTransitionDef
//!	Construction
//!
//------------------------------------------------------------------------------------------
KOAftertouchTransitionDef::KOAftertouchTransitionDef( void )
:	m_obAnimationName(),
	m_fAngleToTarget( 0.0f ),
	m_fMovementDuration( 0.0f ),
	m_pobAttacker( 0 )
{
}


//------------------------------------------------------------------------------------------
//!
//!	KOAftertouchTransitionDef::CreateInstance
//!	To create an instance of what we describe
//!
//------------------------------------------------------------------------------------------
MovementController* KOAftertouchTransitionDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) KOAftertouchTransition( pobMovement, *this );
}


//------------------------------------------------------------------------------------------
//!
//!	KOAftertouchTransition::KOAftertouchTransition
//!	Construction
//!
//------------------------------------------------------------------------------------------
KOAftertouchTransition::KOAftertouchTransition(	CMovement*							pobMovement, 
												const KOAftertouchTransitionDef&	obDefinition )

:	MovementController( pobMovement ),
	m_obDefinition( obDefinition ),
	m_obSingleAnimation(),
	m_fMovementDurationRemaining( 0.0f ),
	m_fYawDelta( 0.0f )
{
	// Register our definition for debugging purposes
	InternalRegisterDefinition( m_obDefinition );

	// Create our animation and add it to the animator
	m_obSingleAnimation = m_pobAnimator->CreateAnimation( obDefinition.m_obAnimationName );
	m_obSingleAnimation->SetBlendWeight( 0.0f );
	m_obSingleAnimation->SetFlagBits( ANIMF_LOCOMOTING | ANIMF_INHIBIT_AUTO_DESTRUCT );

	// We count down the movement duration
	m_fMovementDurationRemaining = m_obDefinition.m_fMovementDuration;
	if ( m_fMovementDurationRemaining == 0.0f )
		m_fMovementDurationRemaining = m_obSingleAnimation->GetDuration();
}


//------------------------------------------------------------------------------------------
//!
//!	KOAftertouchTransition::~KOAftertouchTransition
//!	Destruction
//!
//------------------------------------------------------------------------------------------
KOAftertouchTransition::~KOAftertouchTransition( void )
{
	// Only remove the animation if we have done an update
	if ( !m_bFirstFrame && m_obSingleAnimation->IsActive() )
		m_pobAnimator->RemoveAnimation( m_obSingleAnimation );
}


//------------------------------------------------------------------------------------------
//!
//!	KOAftertouchTransition::Update
//!	
//!
//------------------------------------------------------------------------------------------
bool KOAftertouchTransition::Update(	float						fTimeStep, 
										const CMovementInput&		obMovementInput,
										const CMovementStateRef&	obCurrentMovementState,
										CMovementState&				obPredictedMovementState )
{
	// Add the animation and set the speed on our first update
	if ( m_bFirstFrame )
	{
		m_pobAnimator->AddAnimation( m_obSingleAnimation );
		m_obSingleAnimation->SetTimeRemaining( m_fMovementDurationRemaining );
	}

	// Decrease the remaining duration
	m_fMovementDurationRemaining -= fTimeStep;
	if ( m_fMovementDurationRemaining < fTimeStep )
		m_fMovementDurationRemaining = fTimeStep;

	// Set the animation weight
	m_obSingleAnimation->SetBlendWeight( m_fBlendWeight );

	// Check to see if we have been handed a target to aim at
	if ( obMovementInput.m_bTargetPointSet )
	{
		// This code deals with the rotation of the character
		if ( m_bFirstFrame )
		{
			// Create an idealised direction describing where we are facing
			CDirection obCurrentDirection = obCurrentMovementState.m_obRootMatrix.GetZAxis();
			obCurrentDirection.Y() = 0.0f;

			// Create an idealised direction defining where the opponent is relative to ourselves
			CPoint obOpponentPosition = obMovementInput.m_obTargetPoint;
			CDirection obOpponentDirection( obOpponentPosition - obCurrentMovementState.m_obRootMatrix.GetTranslation() );
			obOpponentDirection.Y() = 0.0f;

			// Find the angle between ourselves and our opponent
			float fOpponentAngle = MovementControllerUtilities::RotationAboutY( obCurrentDirection, obOpponentDirection );

			// Find the angle we need to move through to get to the requested angle
			float fAngleToMoveThrough = m_obDefinition.m_fAngleToTarget + fOpponentAngle;

			// Check the bounds
			if ( fAngleToMoveThrough >= PI ) fAngleToMoveThrough = -( TWO_PI - fmodf( fAngleToMoveThrough, TWO_PI ) );
			if ( fAngleToMoveThrough < -PI ) fAngleToMoveThrough = ( TWO_PI + fmodf( fAngleToMoveThrough, TWO_PI ) );

			// Set the angle to the procedural rotation parameter
			obPredictedMovementState.m_fProceduralYaw = fAngleToMoveThrough;
		}

		else
		{
			// After the initial alignment time we leave the animation to do its thing
			obPredictedMovementState.m_obProceduralRootDelta.Clear();
		}
	}

	// If we do not have a target entity there is no procedural work to do.
	else
	{
		// If we have no target, clear the remainders we are simply playing the animation
		obPredictedMovementState.m_fProceduralYaw = 0.0f;
		obPredictedMovementState.m_obProceduralRootDelta.Clear();
	}

	// Aftertouch control

	const float fMIN_PAD_MAGNITUDE = 0.1f;
	const float fYAW_SPEED = 180.0f * DEG_TO_RAD_VALUE;
	const float fYAW_ADJUST_FACTOR = 0.5f;

	if (m_pobMovement->GetParentEntity()->GetAttackComponent()->InKOAftertouch())
	{
		if (m_obDefinition.m_pobAttacker && m_obDefinition.m_pobAttacker->GetInputComponent())
		{
			const CInputComponent* obMovementInput=m_obDefinition.m_pobAttacker->GetInputComponent();
			const CDirection obPadDirection=obMovementInput->GetInputDir();
			const float fPadMagnitude=obMovementInput->GetInputSpeed();

			CDirection obInputDirection = obMovementInput->GetInputDir() * CamMan::Get().GetPrimaryView()->GetCurrMatrix().GetAffineInverse();

			float fDesiredDelta;

			if (fPadMagnitude<fMIN_PAD_MAGNITUDE)
			{
				fDesiredDelta=0.0f;
			}
			else
			{
				fDesiredDelta = obInputDirection.X() * fPadMagnitude;

				if (fDesiredDelta>1.0f)
					fDesiredDelta=1.0f;

				if (fDesiredDelta<-1.0f)
					fDesiredDelta=-1.0f;

				fDesiredDelta *= fYAW_SPEED;
			}

			m_fYawDelta += ((fDesiredDelta-m_fYawDelta) * fYAW_ADJUST_FACTOR);

			obPredictedMovementState.m_fProceduralYaw=m_fYawDelta * fTimeStep;

			//ntPrintf("YawDelta=%f\n",m_fYawDelta * RAD_TO_DEG_VALUE);
		}
	}

	// If we have completed the attack without interuption then drop out to our return controller
	if ( ( !m_bFirstFrame ) && ( m_fMovementDurationRemaining <= fTimeStep ) )
	{
		// We have completed our movement
		return true;
	}

	// Remove the first frame flag if it is set
	if ( m_bFirstFrame )
		m_bFirstFrame = false;

	// This return mechanism needs to be used to move to the next controller in future
	return false;
}

//------------------------------------------------------------------------------------------
//!
//!	FallAftertouchTransitionDef::FallAftertouchTransitionDef
//!	Construction
//!
//------------------------------------------------------------------------------------------
FallAftertouchTransitionDef::FallAftertouchTransitionDef( void )
:	m_obAnimationName(),
	m_fMaxHorizontalVelocity( 0.0f ),
	m_fHorizontalAccelFactor( 0.0f),
	m_fMaxVerticalVelocity( 0.0f ),
	m_fVerticalAccel( 0.0f),
	m_pobParentEntity( 0 ),
	m_pobControllingEntity( 0 )
{
}


//------------------------------------------------------------------------------------------
//!
//!	FallAftertouchTransitionDef::CreateInstance
//!	To create an instance of what we describe
//!
//------------------------------------------------------------------------------------------
MovementController* FallAftertouchTransitionDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) FallAftertouchTransition( pobMovement, *this );
}


//------------------------------------------------------------------------------------------
//!
//!	FallAftertouchTransition::FallAftertouchTransition
//!	Construction
//!
//------------------------------------------------------------------------------------------
FallAftertouchTransition::FallAftertouchTransition(	CMovement*							pobMovement, 
												const FallAftertouchTransitionDef&	obDefinition )

:	MovementController( pobMovement ),
	m_obDefinition( obDefinition ),
	m_obSingleAnimation(),
	m_fXVelocity(0.0f),
	m_fYVelocity(0.0f),
	m_pobParentEntity( obDefinition.m_pobParentEntity ),
	m_pobControllingEntity( obDefinition.m_pobControllingEntity ),
	m_obVerticalVelocity( CONSTRUCT_CLEAR ),
	m_obVerticalAcceleration( CONSTRUCT_CLEAR ),
	m_obHorizontalVelocity( CONSTRUCT_CLEAR )
{
	// Register our definition for debugging purposes
	InternalRegisterDefinition( m_obDefinition );

	// Create our animation and add it to the animator
	m_obSingleAnimation = m_pobAnimator->CreateAnimation( obDefinition.m_obAnimationName );
	m_obSingleAnimation->SetBlendWeight( 1.0f );
	m_obSingleAnimation->SetFlagBits( ANIMF_LOCOMOTING | ANIMF_INHIBIT_AUTO_DESTRUCT | ANIMF_LOOPING);
	m_obSingleAnimation->SetSpeed( 1.0f );
}


//------------------------------------------------------------------------------------------
//!
//!	FallAftertouchTransition::~FallAftertouchTransition
//!	Destruction
//!
//------------------------------------------------------------------------------------------
FallAftertouchTransition::~FallAftertouchTransition( void )
{
	// Only remove the animation if we have done an update
	if ( !m_bFirstFrame && m_obSingleAnimation->IsActive() )
		m_pobAnimator->RemoveAnimation( m_obSingleAnimation );
}


//------------------------------------------------------------------------------------------
//!
//!	FallAftertouchTransition::Update
//!	
//!
//------------------------------------------------------------------------------------------
bool FallAftertouchTransition::Update(	float						fTimeStep, 
										const CMovementInput&		obMovementInput,
										const CMovementStateRef&	obCurrentMovementState,
										CMovementState&				obPredictedMovementState )
{
	// If we are on the first frame...
	if ( m_bFirstFrame )
	{
		// Add the animation to the animator
		m_pobAnimator->AddAnimation( m_obSingleAnimation );
		m_obSingleAnimation->SetSpeed( 1.0f ); 

		// Clear the flag
		m_bFirstFrame = false;
	}

	m_obSingleAnimation->SetBlendWeight( m_fBlendWeight );

	// We always kill all animation movement
	obPredictedMovementState.m_obRootDeltaScalar = CDirection( CONSTRUCT_CLEAR );

	// Aftertouch control

	const float fMIN_PAD_MAGNITUDE = 0.1f;

	const CInputComponent* pobInput=m_pobControllingEntity->GetInputComponent();

	float fDesiredXVelocity = 0.0f;
	float fDesiredZVelocity = 0.0f;

	float fPadMagnitude=pobInput->GetInputSpeed();

	if (fPadMagnitude >= fMIN_PAD_MAGNITUDE)
	{
		CDirection obInputDirection = pobInput->GetInputDir();
		
		// Scale the pad magnitude - ie dead spot
		if (fPadMagnitude >= fMIN_PAD_MAGNITUDE)
		{
			fPadMagnitude = (fPadMagnitude - fMIN_PAD_MAGNITUDE) / (1.0f - fMIN_PAD_MAGNITUDE);
		}
		else
		{
			fPadMagnitude = 0.0f;
		}

		// ----- X Velocity control -----

		fDesiredXVelocity = obInputDirection.X() * fPadMagnitude * m_obDefinition.m_fMaxHorizontalVelocity;

		/*if (fDesiredXVelocity>fMAX_VELOCITY)
			fDesiredXVelocity=fMAX_VELOCITY;

		if (fDesiredXVelocity<-fMAX_VELOCITY)
			fDesiredXVelocity=-fMAX_VELOCITY;*/

		// ----- Z Velocity control -----

		fDesiredZVelocity = obInputDirection.Z() * fPadMagnitude * m_obDefinition.m_fMaxHorizontalVelocity;

		/*if (fDesiredZVelocity>fMAX_VELOCITY)
			fDesiredZVelocity=fMAX_VELOCITY;

		if (fDesiredZVelocity<-fMAX_VELOCITY)
			fDesiredZVelocity=-fMAX_VELOCITY;*/

		/*const CMatrix& obWorldMatrix=m_pobParentEntity->GetMatrix();

		const CPoint& obWorldTranslation(obWorldMatrix.GetTranslation());

		CPoint obPadDir(	obWorldTranslation.X()+obInputDirection.X(),
							obWorldTranslation.Y()+obInputDirection.Y(),
							obWorldTranslation.Z()+obInputDirection.Z());

		g_VisualDebug->RenderLine(obWorldTranslation,obPadDir,0xffff0000);*/
	}

	// Set up the previous velocity values in different planes - vertical
	CDirection obLastVerticalVelocity = CDirection(	0.0f, 
													obCurrentMovementState.m_obLastRequestedVelocity.Y(), 
													0.0f );

	// ...horizontal
	CDirection obLastHorizontalVelocity = CDirection(	obCurrentMovementState.m_obLastRequestedVelocity.X(), 
														0.0f, 
														obCurrentMovementState.m_obLastRequestedVelocity.Z() );
	
	// Desired velocity delta
	CDirection obDesiredVelocityDelta = CDirection(	fDesiredXVelocity, 
															0.0f, 
															fDesiredZVelocity );

	//ntPrintf("Desired velocity dir %f, %f, %f\n", obDesiredVelocityDelta.X(), obDesiredVelocityDelta.Y(), obDesiredVelocityDelta.Z() );
	
	obDesiredVelocityDelta -= obLastHorizontalVelocity;
	obDesiredVelocityDelta *= m_obDefinition.m_fHorizontalAccelFactor;
	
	// Vertical accerleation
	m_obVerticalAcceleration = CDirection(	0.0f, m_obDefinition.m_fVerticalAccel,	0.0f );
	
	// Vertical velocity
	m_obVerticalVelocity =  obLastVerticalVelocity + ( m_obVerticalAcceleration * fTimeStep );

	// Cap vertical velocity at maximum value
	if (m_obVerticalVelocity.Y() > m_obDefinition.m_fMaxVerticalVelocity)
		m_obVerticalVelocity.Y() = m_obDefinition.m_fMaxVerticalVelocity;
	else if (m_obVerticalVelocity.Y() < -m_obDefinition.m_fMaxVerticalVelocity)
		m_obVerticalVelocity.Y() = -m_obDefinition.m_fMaxVerticalVelocity;

	obPredictedMovementState.m_obProceduralRootDelta += ( m_obVerticalVelocity ) * fTimeStep;
	
	// ...horizontal acceleration (due to aftertouch)
	obPredictedMovementState.m_obProceduralRootDelta += ( obLastHorizontalVelocity + ( obDesiredVelocityDelta * fTimeStep ) ) * fTimeStep;

	// Stop when we hit the ground 
	if ( obCurrentMovementState.m_bOnGround  )
	{
		//ntPrintf("FallAftertouchTransition - Entity hit ground\n");
		// We have completed our movement
		return true;
	}

	// This return mechanism needs to be used to move to the next controller in future
	return false;
}

//------------------------------------------------------------------------------------------
//!
//!	ZAxisAlignTargetedTransitionDef::ZAxisAlignTargetedTransitionDef
//!	Construction
//!
//------------------------------------------------------------------------------------------
ZAxisAlignTargetedTransitionDef::ZAxisAlignTargetedTransitionDef( void )
:	m_obAnimationName(),
	m_bApplyGravity( false ),
	m_obAlignZTo( CONSTRUCT_CLEAR ),
	m_pobEntityAlignZTowards( 0 ),
	m_obScaleToCoverDistance( 0.0f, 0.0f, 0.0f )
{
}


//------------------------------------------------------------------------------------------
//!
//!	ZAxisAlignTargetedTransitionDef::CreateInstance
//!	To create an instance of what we describe
//!
//------------------------------------------------------------------------------------------
MovementController* ZAxisAlignTargetedTransitionDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) ZAxisAlignTargetedTransition( pobMovement, *this );
}


//------------------------------------------------------------------------------------------
//!
//!	ZAxisAlignTargetedTransition::ZAxisAlignTargetedTransition
//!	Construction
//!
//------------------------------------------------------------------------------------------
ZAxisAlignTargetedTransition::ZAxisAlignTargetedTransition(	CMovement*								pobMovement, 
														const ZAxisAlignTargetedTransitionDef&	obDefinition )

:	MovementController( pobMovement ),
	m_obDefinition( obDefinition ),
	m_obSingleAnimation(),
	m_fYawNeeded( 0.0f ),
	m_obStartZ(),
	m_obDesiredZ()
{
	// Register our definition for debugging purposes
	InternalRegisterDefinition( m_obDefinition );

	// Create our animation and add it to the animator
	m_obSingleAnimation = m_pobAnimator->CreateAnimation( obDefinition.m_obAnimationName );
	m_obSingleAnimation->SetBlendWeight( 1.0f );

	m_obSingleAnimation->SetFlagBits( ANIMF_LOCOMOTING | ANIMF_INHIBIT_AUTO_DESTRUCT );
}


//------------------------------------------------------------------------------------------
//!
//!	ZAxisAlignTargetedTransition::~ZAxisAlignTargetedTransition
//!	Destruction
//!
//------------------------------------------------------------------------------------------
ZAxisAlignTargetedTransition::~ZAxisAlignTargetedTransition( void )
{
	// Only remove the animation if we have done an update
	if ( !m_bFirstFrame && m_obSingleAnimation->IsActive() )
		m_pobAnimator->RemoveAnimation( m_obSingleAnimation );
}


//------------------------------------------------------------------------------------------
//!
//!	ZAxisAlignTargetedTransition::Update
//!	
//!
//------------------------------------------------------------------------------------------
bool ZAxisAlignTargetedTransition::Update(	float						fTimeStep, 
											const CMovementInput&		obMovementInput,
											const CMovementStateRef&	obCurrentMovementState,
											CMovementState&				obPredictedMovementState )
{
	// Add the animation and set the speed on our first update
	if ( m_bFirstFrame )
	{
		obPredictedMovementState.m_fProceduralYaw = 0.0;
		obPredictedMovementState.m_obProceduralRootDelta.Clear();

		m_pobAnimator->AddAnimation( m_obSingleAnimation );
		m_obSingleAnimation->SetTimeRemaining( m_obSingleAnimation->GetDuration() );

		// See how much total movement we need during this animation
		m_obStartZ = obCurrentMovementState.m_obFacing;

		m_obDesiredZ = m_obDefinition.m_obAlignZTo;	
		m_fYawNeeded = MovementControllerUtilities::RotationAboutY( m_obStartZ, m_obDesiredZ );

		m_bFirstFrame = false;

		// Get scalars
		if (m_obDefinition.m_obScaleToCoverDistance.Length() > 0.0f)
		{
			float fAnim = m_obSingleAnimation->GetRootEndTranslation().Length();
			if (fAnim > 0.0f)
			{
				float fDesired = m_obDefinition.m_obScaleToCoverDistance.Length();
				m_fDistanceScalar = fDesired / fAnim;
			}
			else 
			{
				m_fDistanceScalar = 1.0f;
			}
		}
		else 
		{
			m_fDistanceScalar = 1.0f;
		}
	}

	// We do no movement in this controller
	obPredictedMovementState.m_obProceduralRootDelta.Clear();
	obPredictedMovementState.m_obRootDeltaScalar.Clear();
	if (m_fDistanceScalar != 1.0f)
	{
		obPredictedMovementState.m_obRootDeltaScalar.X() = m_fDistanceScalar;
		obPredictedMovementState.m_obRootDeltaScalar.Y() = m_fDistanceScalar;
		obPredictedMovementState.m_obRootDeltaScalar.Z() = m_fDistanceScalar;
	}

	// Smooth rotation according to time left of animation
	float fAmount = CMaths::SmoothStep( m_obSingleAnimation->GetTime() / m_obSingleAnimation->GetDuration() );
	if (m_obDefinition.m_pobEntityAlignZTowards)
	{
		m_obDesiredZ = CDirection( m_obDefinition.m_pobEntityAlignZTowards->GetPosition() - obCurrentMovementState.m_obPosition );
		m_obDesiredZ.Normalise();
		m_fYawNeeded = MovementControllerUtilities::RotationAboutY( m_obStartZ, m_obDesiredZ );
	}
	float fYawNeeded = m_fYawNeeded * fAmount;
	CMatrix obRotation( CONSTRUCT_IDENTITY );
	obRotation.SetFromAxisAndAngle(CDirection(0.0,1.0,0.0),fYawNeeded);	
	CDirection obNextZ = m_obStartZ * obRotation;
	fYawNeeded = MovementControllerUtilities::RotationAboutY( obCurrentMovementState.m_obFacing, obNextZ );
	obPredictedMovementState.m_fProceduralYaw = fYawNeeded;

	// Set the animation weight
	m_obSingleAnimation->SetBlendWeight( m_fBlendWeight );

	// Apply gravity if requested
	ApplyGravity(m_obDefinition.m_bApplyGravity);

	// If we have completed without interuption then drop out to our return controller
	if ( m_obSingleAnimation->GetTime() >= m_obSingleAnimation->GetDuration() )
	{
		// We have completed our movement
		return true;
	}		

	// This return mechanism needs to be used to move to the next controller in future
	return false;
}

//------------------------------------------------------------------------------------------
//!
//!	TargetedTransitionToPointDef::TargetedTransitionToPointDef
//!	Construction
//!
//------------------------------------------------------------------------------------------
TargetedTransitionToPointDef::TargetedTransitionToPointDef( void )
:	m_obAnimationName(),
	m_bApplyGravity( false ),
	m_obPoint( CONSTRUCT_CLEAR ),
	m_fExtraSpeed( 0.0f ),
	m_bTravellingBackwards( false ),
	m_fRadius( 0.0f )
{
}


//------------------------------------------------------------------------------------------
//!
//!	TargetedTransitionToPointDef::CreateInstance
//!	To create an instance of what we describe
//!
//------------------------------------------------------------------------------------------
MovementController* TargetedTransitionToPointDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) TargetedTransitionToPoint( pobMovement, *this );
}


//------------------------------------------------------------------------------------------
//!
//!	TargetedTransitionToPoint::TargetedTransitionToPoint
//!	Construction
//!
//------------------------------------------------------------------------------------------
TargetedTransitionToPoint::TargetedTransitionToPoint(	CMovement*								pobMovement, 
														const TargetedTransitionToPointDef&	obDefinition )

:	MovementController( pobMovement ),
	m_obDefinition( obDefinition ),
	m_obSingleAnimation()
{
	// Register our definition for debugging purposes
	InternalRegisterDefinition( m_obDefinition );

	// Create our animation and add it to the animator
	m_obSingleAnimation = m_pobAnimator->CreateAnimation( obDefinition.m_obAnimationName );
	m_obSingleAnimation->SetBlendWeight( 1.0f );

	// Assume we're looping an anim because the whole point of this controller is to continue towards a point till we get there
	m_obSingleAnimation->SetFlagBits( ANIMF_LOOPING | ANIMF_LOCOMOTING | ANIMF_INHIBIT_AUTO_DESTRUCT );
}


//------------------------------------------------------------------------------------------
//!
//!	TargetedTransitionToPoint::~TargetedTransitionToPoint
//!	Destruction
//!
//------------------------------------------------------------------------------------------
TargetedTransitionToPoint::~TargetedTransitionToPoint( void )
{
	// Only remove the animation if we have done an update
	if ( !m_bFirstFrame && m_obSingleAnimation->IsActive() )
		m_pobAnimator->RemoveAnimation( m_obSingleAnimation );
}


//------------------------------------------------------------------------------------------
//!
//!	TargetedTransitionToPoint::Update
//!	
//!
//------------------------------------------------------------------------------------------
bool TargetedTransitionToPoint::Update(	float						fTimeStep, 
										const CMovementInput&		obMovementInput,
										const CMovementStateRef&	obCurrentMovementState,
										CMovementState&				obPredictedMovementState )
{
	// Add the animation and set the speed on our first update
	if ( m_bFirstFrame )
	{
		obPredictedMovementState.m_fProceduralYaw = 0.0;
		obPredictedMovementState.m_obProceduralRootDelta.Clear();

		m_pobAnimator->AddAnimation( m_obSingleAnimation );
		m_obSingleAnimation->SetTimeRemaining( m_obSingleAnimation->GetDuration() );

		m_obStartPoint = obCurrentMovementState.m_obPosition;

		m_bFirstFrame = false;
	}

#ifndef _GOLD_MASTER
	g_VisualDebug->RenderPoint(m_obDefinition.m_obPoint,10.0f,DC_RED);
#endif

	// See how much total movement we need during this animation
	CDirection obCurrentZ( obCurrentMovementState.m_obFacing );
	CDirection obDesiredZ(CONSTRUCT_CLEAR);
	if(m_obDefinition.m_bTravellingBackwards)
	{
		obDesiredZ = CDirection( obCurrentMovementState.m_obPosition - m_obDefinition.m_obPoint);
	}
	else
	{
		obDesiredZ = CDirection( m_obDefinition.m_obPoint - obCurrentMovementState.m_obPosition );
	}
	float fLengthSquared = obDesiredZ.LengthSquared();
	obDesiredZ.Y() = 0.0f;
	obDesiredZ.Normalise();
	obPredictedMovementState.m_fProceduralYaw = MovementControllerUtilities::RotationAboutY( obCurrentZ, obDesiredZ ); // This function normalises them itself
	obPredictedMovementState.m_obProceduralRootDelta = obDesiredZ * m_obDefinition.m_fExtraSpeed * fTimeStep;

	CDirection obFromStart(CONSTRUCT_CLEAR);
	if(m_obDefinition.m_bTravellingBackwards)
	{
		obFromStart = CDirection( m_obStartPoint - m_obDefinition.m_obPoint );
	}
	else
	{
		obFromStart = CDirection( m_obDefinition.m_obPoint - m_obStartPoint );
	}
	obFromStart.Normalise();
	if ( obDesiredZ.Dot(obFromStart) < 0 )
	{
		// We have completed our movement
		return true;
	}

	// Set the animation weight
	m_obSingleAnimation->SetBlendWeight( m_fBlendWeight );

	// Apply gravity if requested
	ApplyGravity(m_obDefinition.m_bApplyGravity);

	// If we're within the radius of where we want to be, drop out.
	// This is more useful than the below check for animations with a large procedural root delta, where you can potentially go
	// past the target-point (which results in a snap-around to target the point from the other direction). E.G. King Retreat Anim.
	if ( m_obDefinition.m_fRadius != 0.0f )
	{
		if(fLengthSquared < (float)(m_obDefinition.m_fRadius * m_obDefinition.m_fRadius))
		{
			//We're within the radius, so we've completed our movement.
			return true;
		}
	}
	// If we've got to where we want to be, drop out
	if ( fLengthSquared < obPredictedMovementState.m_obProceduralRootDelta.LengthSquared() * 5.0f )
	{
		// We have completed our movement
		return true;
	}		

	// This return mechanism needs to be used to move to the next controller in future
	return false;
}









//------------------------------------------------------------------------------------------
//!
//!	TargetedTransitionToPointManualMoveDef::TargetedTransitionToPointManualMoveDef
//!	Construction
//!
//------------------------------------------------------------------------------------------
TargetedTransitionToPointManualMoveDef::TargetedTransitionToPointManualMoveDef( void )
:	m_obAnimationName(),
	m_obPoint( CONSTRUCT_CLEAR ),
	m_fSpeed( 1.0f )
{
}


//------------------------------------------------------------------------------------------
//!
//!	TargetedTransitionToPointManualMoveDef::CreateInstance
//!	To create an instance of what we describe
//!
//------------------------------------------------------------------------------------------
MovementController* TargetedTransitionToPointManualMoveDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) TargetedTransitionToPointManualMove( pobMovement, *this );
}




//------------------------------------------------------------------------------------------
//!
//!	TargetedTransitionToPoint::TargetedTransitionToPoint
//!	Construction
//!
//------------------------------------------------------------------------------------------
TargetedTransitionToPointManualMove::TargetedTransitionToPointManualMove(CMovement* pobMovement, 
																		 const TargetedTransitionToPointManualMoveDef&	obDefinition )

:	MovementController( pobMovement ),
	m_obDefinition( obDefinition ),
	m_obSingleAnimation()
{
	// Register our definition for debugging purposes
	InternalRegisterDefinition( m_obDefinition );

	// Create our animation and add it to the animator
	m_obSingleAnimation = m_pobAnimator->CreateAnimation( obDefinition.m_obAnimationName );
	m_obSingleAnimation->SetBlendWeight( 1.0f );

	// Assume we're looping an anim because the whole point of this controller is to continue towards a point till we get there
	m_obSingleAnimation->SetFlagBits( ANIMF_LOOPING | ANIMF_LOCOMOTING | ANIMF_INHIBIT_AUTO_DESTRUCT );
	m_bFinished = false;
}


//------------------------------------------------------------------------------------------
//!
//!	TargetedTransitionToPointManualMove::~TargetedTransitionToPointManualMove
//!	Destruction
//!
//------------------------------------------------------------------------------------------
TargetedTransitionToPointManualMove::~TargetedTransitionToPointManualMove( void )
{
	// Only remove the animation if we have done an update
	if ( !m_bFirstFrame && m_obSingleAnimation->IsActive() )
		m_pobAnimator->RemoveAnimation( m_obSingleAnimation );
}


//------------------------------------------------------------------------------------------
//!
//!	TargetedTransitionToPoint::Update
//!	
//!
//------------------------------------------------------------------------------------------
bool TargetedTransitionToPointManualMove::Update( float fTimeStep, const CMovementInput& obMovementInput,
												 const CMovementStateRef& obCurrentMovementState,
												 CMovementState& obPredictedMovementState )
{
	if(m_bFinished)
	{
		return true;
	}

	if(m_bFirstFrame)
	{

		obPredictedMovementState.m_fProceduralYaw = 0.0f;
		obPredictedMovementState.m_obProceduralRootDelta.Clear();

		m_pobAnimator->AddAnimation(m_obSingleAnimation);
		m_obSingleAnimation->SetTimeRemaining(m_obSingleAnimation->GetDuration());

		m_bFirstFrame = false;
		m_bFinished = false;
	}

#ifndef _GOLD_MASTER
	g_VisualDebug->RenderPoint(m_obDefinition.m_obPoint, 10.0f, DC_RED);
#endif

	//See how much total movement we need during this animation.
	m_obDirection = CDirection(m_obDefinition.m_obPoint - obCurrentMovementState.m_obPosition);
	float fLengthSquared = m_obDirection.LengthSquared();
	m_obDirection.Normalise();

//Here
	obPredictedMovementState.m_obProceduralRootDelta = m_obDirection * m_obDefinition.m_fSpeed * fTimeStep;
	//Force fixed rotation.
	obPredictedMovementState.m_fProceduralPitch = m_obDefinition.m_obRotationToFix.X();
	obPredictedMovementState.m_fProceduralYaw = m_obDefinition.m_obRotationToFix.Y();
	obPredictedMovementState.m_fProceduralRoll = m_obDefinition.m_obRotationToFix.Z();
	obPredictedMovementState.m_bApplyExplicitRotations = true;

	//Set the animation weight.
	m_obSingleAnimation->SetBlendWeight(m_fBlendWeight);

	// Apply gravity never.
	ApplyGravity(false);

	//If we've got to where we want to be, drop out.
	if(fLengthSquared < obPredictedMovementState.m_obProceduralRootDelta.LengthSquared() * 5.0f)
	{
		//We have completed our movement.
		m_bFinished = true;
		return true;
	}
//Here

/*
	CPoint obNewPosition(CONSTRUCT_CLEAR);
	bool bDone = false;
	float fDistanceThisFrameSqrd = (float)((m_obDefinition.m_fSpeed * fTimeStep) * (m_obDefinition.m_fSpeed * fTimeStep));
	if(fLengthSquared < fDistanceThisFrameSqrd)
	{
		obNewPosition = m_obDefinition.m_obPoint;
		bDone = true;
	}
	else
	{
		obNewPosition = (CPoint)(obCurrentMovementState.m_obPosition + CPoint(m_obDirection * m_obDefinition.m_fSpeed * fTimeStep));
	}

	//Cast away constness so we can set position and rotation.
	CEntity* pobEntity = const_cast<CEntity*>(m_pobMovement->GetParentEntity());
	pobEntity->SetPosition(obNewPosition);
	pobEntity->SetRotation(m_obDefinition.m_obRotationToFix);

	//Set the animation weight.
	m_obSingleAnimation->SetBlendWeight(m_fBlendWeight);

	// Apply gravity never.
	ApplyGravity(false);

	if(bDone == true)
	{
		//We're home.
		m_bFinished = true;
		return true;
	}
*/
	//This return mechanism needs to be used to move to the next controller in future
	return false;
}
