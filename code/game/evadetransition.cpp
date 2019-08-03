//------------------------------------------------------------------------------------------
//!
//!	\file evadetransition.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "evadetransition.h"
#include "objectdatabase/dataobject.h"
#include "anim/animator.h"
#include "core/exportstruct_anim.h"
#include "movement.h"
#include "Physics/advancedcharactercontroller.h"
#include "Physics/system.h"
#include "core/visualdebugger.h"

START_STD_INTERFACE	( EvadeTransitionDef )
		ISTRING	( EvadeTransitionDef, EvadeAnim )
		IFLOAT	( EvadeTransitionDef, AvoidanceRadius )
		IFLOAT	( EvadeTransitionDef, MovementDuration )
#ifndef _RELEASE
		DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
#endif
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
//!
//!	EvadeTransitionDef::EvadeTransitionDef
//!	Construction
//!
//------------------------------------------------------------------------------------------
EvadeTransitionDef::EvadeTransitionDef( void )
:	m_obEvadeAnim(),
	m_fAvoidanceRadius( 0.0f ),
	m_fMovementDuration( 0.0f ),
	m_obDirection( CONSTRUCT_CLEAR ),
	m_bDoTargetedEvade( false )
{
}


//------------------------------------------------------------------------------------------
//!
//!	EvadeTransitionDef::CreateInstance
//!	To create an instance of what we describe
//!
//------------------------------------------------------------------------------------------
MovementController* EvadeTransitionDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) EvadeTransition( pobMovement, *this );
}


//------------------------------------------------------------------------------------------
//!
//!	EvadeTransition::EvadeTransition
//!	Construction
//!
//------------------------------------------------------------------------------------------
EvadeTransition::EvadeTransition( CMovement* pobMovement, const EvadeTransitionDef& obDefinition )
:	MovementController( pobMovement ),
	m_obDefinition( obDefinition ),
	m_obSingleAnimation()
{
	// Register our definition for debugging purposes
	InternalRegisterDefinition( m_obDefinition );

	// Create our animation
	m_obSingleAnimation = m_pobAnimator->CreateAnimation( obDefinition.m_obEvadeAnim );
	m_obSingleAnimation->SetBlendWeight( 0.0f );
	m_obSingleAnimation->SetFlagBits( ANIMF_LOCOMOTING | ANIMF_INHIBIT_AUTO_DESTRUCT );

	m_bZeroDistanceReached = false;
}


//------------------------------------------------------------------------------------------
//!
//!	EvadeTransition::~EvadeTransition
//!	Destruction
//!
//------------------------------------------------------------------------------------------
EvadeTransition::~EvadeTransition( void )
{
	// Only remove the animation if we have done an update
	if ( !m_bFirstFrame && m_obSingleAnimation->IsActive() )
		m_pobAnimator->RemoveAnimation( m_obSingleAnimation );
}


//------------------------------------------------------------------------------------------
//!
//!	EvadeTransition::Update
//!
//------------------------------------------------------------------------------------------
bool EvadeTransition::Update(	float						fTimeStep, 
								const CMovementInput&		obMovementInput,
								const CMovementStateRef&	obCurrentMovementState,
								CMovementState&				obPredictedMovementState )
{
	// Add the animation and set the speed on our first update
	if ( m_bFirstFrame )
	{
		m_pobAnimator->AddAnimation( m_obSingleAnimation );
		m_obSingleAnimation->SetTimeRemaining( m_obDefinition.m_fMovementDuration );
	}

	// Set the blend weight of our single anim
	m_obSingleAnimation->SetBlendWeight( m_fBlendWeight );

	// On the first frame we need to set the direction for the whole movement
	if ( m_bFirstFrame )
	{
		float fAngle = MovementControllerUtilities::RotationAboutY( obCurrentMovementState.m_obFacing, m_obDefinition.m_obDirection );
		obPredictedMovementState.m_fProceduralYaw = fAngle;
	}

	// Check to see if we have been handed a target to aim at
	if ( !m_bFirstFrame && obMovementInput.m_bTargetPointSet && ( !m_bZeroDistanceReached ) && ( m_obDefinition.m_bDoTargetedEvade ) )
	{
		// Angular velocity - Create a direction describing where we are currently pointing
		CDirection obCurrentDirection = obCurrentMovementState.m_obRootMatrix.GetZAxis();
		obCurrentDirection.Y() = 0.0f;

		// Create a direction describing where we want to point
		CPoint obLockonPosition = obMovementInput.m_obTargetPoint;
		CDirection obPredictedDirection( obLockonPosition - obCurrentMovementState.m_obRootMatrix.GetTranslation() );
		obPredictedDirection.Y() = 0.0f;

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

	// Gravity setting
	Physics::AdvancedCharacterController* pobCharacter = 0;
	if ( m_pobMovement->GetPhysicsSystem() )
		pobCharacter = (Physics::AdvancedCharacterController*) m_pobMovement->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
	if( pobCharacter )
		pobCharacter->SetApplyCharacterControllerGravity(true);
	
	// If we have completed the attack without interuption then drop out to our return controller
	if ( ( !m_bFirstFrame ) && ( ( m_obSingleAnimation->GetTime() + fTimeStep ) > ( m_obSingleAnimation->GetDuration() ) ) )
	{
		// We have completed our movement
		return true;
	}

	// Remove the first frame flag if it is set
	if ( m_bFirstFrame )
		m_bFirstFrame = false;

	// Return false until we are finished
	return false;
}
