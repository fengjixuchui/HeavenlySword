//------------------------------------------------------------------------------------------
//!
//!	\file continuationtransition.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "continuationtransition.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "movement.h"
#include "anim/animator.h"
#include "core/exportstruct_anim.h"
#include "objectdatabase/dataobject.h"

#include "game/attacks.h"
#include "physics/advancedcharactercontroller.h"
#include "physics/system.h"
#include "game/randmanager.h"

#include "core/visualdebugger.h"

#include "core/maths.h"
#include "core/OSDDisplay.h"

#include "game/aicomponent.h"

#include "game/messagehandler.h"

// For floored blendings
#include "physics/hierarchy_tools.h"

START_STD_INTERFACE	(ContinuationTransitionDef)
	IBOOL		(ContinuationTransitionDef, Looping)
	IBOOL		(ContinuationTransitionDef, EndOnGround)
	IBOOL		(ContinuationTransitionDef, VerticalVelocity)
	IBOOL		(ContinuationTransitionDef, VerticalAcceleration)
	IBOOL		(ContinuationTransitionDef, HorizontalVelocity)
	IBOOL		(ContinuationTransitionDef, HorizontalAcceleration)
	ISTRING		(ContinuationTransitionDef, AnimationName)
#ifndef _RELEASE
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
#endif
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
//!
//!	InputFollowingTransitionDef::InputFollowingTransitionDef
//!	Construction
//!
//------------------------------------------------------------------------------------------
InputFollowingTransitionDef::InputFollowingTransitionDef( void )
:	m_fMaxRotationPerSecond( 90.0f * DEG_TO_RAD_VALUE ),
	m_bApplyGravity( true ),
	m_obAnimName(),
	m_fExtraSpeed( 0.0f ),
	m_fMaxSpeedChange( 0.15f ),
	m_fSlowForTurnScalar( 0.6f ),
	m_fMaxDirectionChange( 45.0f * DEG_TO_RAD_VALUE )
{
}


//------------------------------------------------------------------------------------------
//!
//!	InputFollowingTransitionDef::CreateInstance
//!	To create an instance of what we describe
//!
//------------------------------------------------------------------------------------------
MovementController* InputFollowingTransitionDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) InputFollowingTransition( pobMovement, *this );
}


//------------------------------------------------------------------------------------------
//!
//!	InputFollowingTransitionDef::Clone
//!	Clone me
//!
//------------------------------------------------------------------------------------------
MovementControllerDef* InputFollowingTransitionDef::Clone( void ) const 
{ 
	return NT_NEW_CHUNK(Mem::MC_ENTITY) InputFollowingTransitionDef( *this ); 
}


//------------------------------------------------------------------------------------------
//!
//!	InputFollowingTransition::InputFollowingTransition
//!	Construction
//!
//------------------------------------------------------------------------------------------
InputFollowingTransition::InputFollowingTransition( CMovement* pobMovement, const InputFollowingTransitionDef& obDefinition )
:	MovementController( pobMovement ),
	m_obDefinition( obDefinition ),
	m_obSingleAnimation()	
{
	// Register our definition for debugging purposes
	InternalRegisterDefinition( m_obDefinition );

	// Set up the animation flags
	int iFlags = ANIMF_LOCOMOTING | ANIMF_INHIBIT_AUTO_DESTRUCT | ANIMF_LOOPING;

	// Create our animation and add it to the animator
	m_obSingleAnimation = m_pobAnimator->CreateAnimation( obDefinition.m_obAnimName );
	m_obSingleAnimation->SetBlendWeight( m_fBlendWeight );
	m_obSingleAnimation->SetFlagBits( iFlags );

	m_obLeftAnimation = m_pobAnimator->CreateAnimation( obDefinition.m_obLeanLeftAnimName );
	m_obLeftAnimation->SetBlendWeight( 0.0f );
	m_obLeftAnimation->SetFlagBits( iFlags );

	m_obRightAnimation = m_pobAnimator->CreateAnimation( obDefinition.m_obLeanRightAnimName );
	m_obRightAnimation->SetBlendWeight( 0.0f );
	m_obRightAnimation->SetFlagBits( iFlags );

	m_fLastYaw = m_fLastSpeed = 0.0f;
}


//------------------------------------------------------------------------------------------
//!
//!	InputFollowingTransition::~InputFollowingTransition
//!	Destruction
//!
//------------------------------------------------------------------------------------------
InputFollowingTransition::~InputFollowingTransition( void )
{
	// Remove the animation from the animator if it has been added
	if ( !m_bFirstFrame && m_obSingleAnimation->IsActive() )
		m_pobAnimator->RemoveAnimation( m_obSingleAnimation );
}


//------------------------------------------------------------------------------------------
//!
//!	InputFollowingTransition::Update
//!	Simply move our way through the single animation
//!
//------------------------------------------------------------------------------------------
bool InputFollowingTransition::Update(	float						fTimeStep, 
										const CMovementInput&		obMovementInput,
										const CMovementStateRef&	obCurrentMovementState,
										CMovementState&				obPredictedMovementState )
{
	if (m_bFirstFrame)
	{
		// Add the animation to the animator
		m_pobAnimator->AddAnimation( m_obSingleAnimation );
		if (m_obLeftAnimation)
			m_pobAnimator->AddAnimation( m_obLeftAnimation );
		if (m_obRightAnimation)
			m_pobAnimator->AddAnimation( m_obRightAnimation );

		m_fLastTimeStep = fTimeStep;
	}

	m_obSingleAnimation->SetBlendWeight( m_fBlendWeight );

	// Turning
	obPredictedMovementState.m_fProceduralYaw = MovementControllerUtilities::RotationAboutY( obCurrentMovementState.m_obFacing, obMovementInput.m_obMoveDirection );
	// Adjust scalar for angle we need to turn, cos we want to slow down quite a lot if we need to do a full 180
	float fTurnNeededScalar = 1.0f;
	if (m_obDefinition.m_fSlowForTurnScalar > 0.0f)
	{
		if (fabs(obPredictedMovementState.m_fProceduralYaw) > PI)
			fTurnNeededScalar = 0.0f;
		else
			fTurnNeededScalar = 1.0f - (fabs(obPredictedMovementState.m_fProceduralYaw) / PI );
		fTurnNeededScalar *= fTurnNeededScalar * fTurnNeededScalar; // Cubed so we slow for less harsh angles too
		//ntPrintf("%f\n",fTurnNeededScalar);
	}

	if (m_obLeftAnimation && obPredictedMovementState.m_fProceduralYaw < 0.0)
	{
		m_obLeftAnimation->SetBlendWeight( ntstd::Clamp( fabs(obPredictedMovementState.m_fProceduralYaw) / (m_obDefinition.m_fMaxRotationPerSecond * fTimeStep), 0.0f, 1.0f) );
		m_obRightAnimation->SetBlendWeight( 0.0f );
	}
	else if (m_obRightAnimation)
	{
		m_obRightAnimation->SetBlendWeight( ntstd::Clamp( fabs(obPredictedMovementState.m_fProceduralYaw) / (m_obDefinition.m_fMaxRotationPerSecond * fTimeStep), 0.0f, 1.0f) );
		m_obLeftAnimation->SetBlendWeight( 0.0f );
	}

	// Check if angle change is too much, clamp it to max
	if ( fabs(obPredictedMovementState.m_fProceduralYaw) > m_obDefinition.m_fMaxRotationPerSecond * fTimeStep )
	{
		obPredictedMovementState.m_fProceduralYaw < 0.0f ? obPredictedMovementState.m_fProceduralYaw = -m_obDefinition.m_fMaxRotationPerSecond * fTimeStep : obPredictedMovementState.m_fProceduralYaw = m_obDefinition.m_fMaxRotationPerSecond * fTimeStep;
	}
	// Check if angle change is too much relative to what it was before, smooth it's increase/decrease rate
	if (fabs(obPredictedMovementState.m_fProceduralYaw - m_fLastYaw) > m_obDefinition.m_fMaxDirectionChange * fTimeStep)
	{
		if (obPredictedMovementState.m_fProceduralYaw < m_fLastYaw) // Left
		{
			obPredictedMovementState.m_fProceduralYaw = m_fLastYaw - m_obDefinition.m_fMaxDirectionChange * fTimeStep;
		}
		else // Right
		{
			obPredictedMovementState.m_fProceduralYaw = m_fLastYaw + m_obDefinition.m_fMaxDirectionChange * fTimeStep;
		}
	}
	
	// Moving
	obPredictedMovementState.m_obProceduralRootDelta = obCurrentMovementState.m_obFacing * m_obDefinition.m_fExtraSpeed * obMovementInput.m_fMoveSpeed * fTimeStep;
	obPredictedMovementState.m_obProceduralRootDelta *= fTurnNeededScalar; // Apply scalar from turn amount
	float fLength = obPredictedMovementState.m_obProceduralRootDelta.Length();
	// Check if speed change is too much, smooth it's increase/decrease rate 
	if (fabs(fLength - m_fLastSpeed) > m_obDefinition.m_fMaxSpeedChange * fTimeStep /*&& m_fLastTimeStep * 0.5f > fTimeStep*/)
	{		
		if (fLength < m_fLastSpeed) // Decellerating
		{
			fLength = m_fLastSpeed - m_obDefinition.m_fMaxSpeedChange * fTimeStep;
		}
		else // Accellerating
		{
			fLength = m_fLastSpeed + m_obDefinition.m_fMaxSpeedChange * fTimeStep;
		}

		obPredictedMovementState.m_obProceduralRootDelta.Normalise();
		obPredictedMovementState.m_obProceduralRootDelta *= fLength;
	}

	// Record this frames history
	m_fLastYaw = obPredictedMovementState.m_fProceduralYaw;
	m_fLastSpeed = obPredictedMovementState.m_obProceduralRootDelta.Length();
	m_fLastTimeStep = fTimeStep;

	m_bFirstFrame = false;
	return false;
}


//------------------------------------------------------------------------------------------
//!
//!	ContinuationTransitionDef::ContinuationTransitionDef
//!	Construction
//!
//------------------------------------------------------------------------------------------
ContinuationTransitionDef::ContinuationTransitionDef( void )
:	m_bLooping( false ),
	m_bEndOnGround( false ),
	m_bVerticalVelocity( false ),
	m_bVerticalAcceleration( false ),
	m_bHorizontalVelocity( false ),
	m_bHorizontalAcceleration( false ),
	m_bApplyGravity( false ),
	m_obAnimationName()
{
}

//------------------------------------------------------------------------------------------
//!
//!	ContinuationTransitionDef::CreateInstance
//!	To create an instance of what we describe
//!
//------------------------------------------------------------------------------------------
MovementController* ContinuationTransitionDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) ContinuationTransition( pobMovement, *this );
}


//------------------------------------------------------------------------------------------
//!
//!	ContinuationTransition::ContinuationTransition
//!	Construction
//!
//------------------------------------------------------------------------------------------
ContinuationTransition::ContinuationTransition( CMovement* pobMovement, const ContinuationTransitionDef& obDefinition )
:	MovementController( pobMovement ),
	m_obDefinition( obDefinition ),
	m_obSingleAnimation(),
	m_obVerticalVelocity( CONSTRUCT_CLEAR ),
	m_obVerticalAcceleration( CONSTRUCT_CLEAR ),
	m_obHorizontalVelocity( CONSTRUCT_CLEAR ),
	m_obHorizontalAcceleration( CONSTRUCT_CLEAR ),
	m_bVerticalAccelerationCropped( false )
{
	// Register our definition for debugging purposes
	InternalRegisterDefinition( m_obDefinition );

	// Set up the animation flags
	int iFlags = ANIMF_LOCOMOTING | ANIMF_INHIBIT_AUTO_DESTRUCT;
	if ( m_obDefinition.m_bLooping == true )
		iFlags |= ANIMF_LOOPING;

	// Create our animation and add it to the animator
	m_obSingleAnimation = m_pobAnimator->CreateAnimation( obDefinition.m_obAnimationName );
	m_obSingleAnimation->SetBlendWeight( 0.0f );
	m_obSingleAnimation->SetFlagBits( iFlags );

	m_fTimeInTransition = 0.0f;
}


//------------------------------------------------------------------------------------------
//!
//!	ContinuationTransition::~ContinuationTransition
//!	Destruction
//!
//------------------------------------------------------------------------------------------
ContinuationTransition::~ContinuationTransition( void )
{
	// Remove the animation from the animator if it has been added
	if ( !m_bFirstFrame && m_obSingleAnimation->IsActive() )
		m_pobAnimator->RemoveAnimation( m_obSingleAnimation );
}


//------------------------------------------------------------------------------------------
//!
//!	ContinuationTransition::Update
//!	Simply move our way through the single animation
//!
//------------------------------------------------------------------------------------------
bool ContinuationTransition::Update(	float						fTimeStep, 
										const CMovementInput&		/* obMovementInput */,
										const CMovementStateRef&	obCurrentMovementState,
										CMovementState&				obPredictedMovementState )
{
	static const float s_fMaxAcceleration = -9.8f * 2.0f;
	static const float s_fMinAcceleration = -9.8f * 1.0f;

	// If we are on the first frame...
	if ( m_bFirstFrame )
	{
		// Add the animation to the animator
		m_pobAnimator->AddAnimation( m_obSingleAnimation );

		// Capture the information we are going to use - current vertical velocity
		m_obVerticalVelocity = CDirection(	0.0f, 
											obCurrentMovementState.m_obLastRequestedVelocity.Y(), 
											0.0f );

		// Current vertical acceleration
		m_obVerticalAcceleration = CDirection(	0.0f, 
												obCurrentMovementState.m_obLastRequestedAcceleration.Y(), 
												0.0f );

		// QUICK FIX - we may need to crop the vertical acceleration
		if ( m_obVerticalAcceleration.Y() >= s_fMinAcceleration )
		{
			m_obVerticalAcceleration.Y() = s_fMinAcceleration;
			m_bVerticalAccelerationCropped = true;
		}

		if ( m_obVerticalAcceleration.Y() < s_fMaxAcceleration  )
			m_obVerticalAcceleration.Y() = s_fMaxAcceleration;

		// Current horizontal velocity
		m_obHorizontalVelocity = CDirection(	obCurrentMovementState.m_obLastRequestedVelocity.X(), 
												0.0f, 
												obCurrentMovementState.m_obLastRequestedVelocity.Z() );

		// Current horizontal acceleration
		m_obHorizontalAcceleration = CDirection(	obCurrentMovementState.m_obLastRequestedAcceleration.X(), 
													0.0f, 
													obCurrentMovementState.m_obLastRequestedAcceleration.Z() );

		// Clear the flag
		m_bFirstFrame = false;

		/*m_obHorizontalVelocity.Clear();
		m_obHorizontalAcceleration.Clear();
		m_obVerticalVelocity.Clear();
		m_obVerticalAcceleration.Clear();*/
	}

	m_fTimeInTransition += fTimeStep;

	// QUICK FIX - we may need to crop the vertical acceleration
	if ( m_bVerticalAccelerationCropped && m_obVerticalAcceleration.Y() > s_fMaxAcceleration )
	{
		m_obVerticalAcceleration.Y() += s_fMaxAcceleration * fTimeStep;
	}

	// We always kill all animation movement
	obPredictedMovementState.m_obRootDeltaScalar = CDirection( CONSTRUCT_CLEAR );

	// Set the weight on our animation and update it
	m_obSingleAnimation->SetBlendWeight( m_fBlendWeight );

	// Set up the previous velocity values in different planes - vertical
	CDirection obLastVerticalVelocity = CDirection(	0.0f, 
													obCurrentMovementState.m_obLastRequestedVelocity.Y(), 
													0.0f );

	// ...horizontal
	CDirection obLastHorizontalVelocity = CDirection(	obCurrentMovementState.m_obLastRequestedVelocity.X(), 
														0.0f, 
														obCurrentMovementState.m_obLastRequestedVelocity.Z() );
	
	if ( m_obDefinition.m_bVerticalAcceleration )
		obPredictedMovementState.m_obProceduralRootDelta += ( obLastVerticalVelocity + ( m_obVerticalAcceleration * fTimeStep ) ) * fTimeStep;
	else if ( m_obDefinition.m_bVerticalVelocity )
		obPredictedMovementState.m_obProceduralRootDelta += ( m_obVerticalVelocity * fTimeStep );
	else
		obPredictedMovementState.m_obProceduralRootDelta.Y() = 0.0f;

	// ...horizontal
	if ( m_obDefinition.m_bHorizontalAcceleration )
		obPredictedMovementState.m_obProceduralRootDelta += ( obLastHorizontalVelocity + ( m_obHorizontalAcceleration * fTimeStep ) ) * fTimeStep;
	else if ( m_obDefinition.m_bHorizontalVelocity )
		obPredictedMovementState.m_obProceduralRootDelta += ( m_obHorizontalVelocity * fTimeStep );
	else
	{
		obPredictedMovementState.m_obProceduralRootDelta.X() = 0.0f;
		obPredictedMovementState.m_obProceduralRootDelta.Z() = 0.0f;
	}

	// When we are finished indicate that to the movement component
	if ( m_obDefinition.m_bLooping == false )
	{
		if ( m_obSingleAnimation->GetTime() > ( m_obSingleAnimation->GetDuration() - fTimeStep ) )
			return true;
	}

	// If we have been asked to stop when we hit the ground - do so
	if ( ( m_obDefinition.m_bEndOnGround ) && ( obCurrentMovementState.m_bOnGround ) )
	{
		// We have completed our movement
		return true;
	}

	// Gravity setting
	Physics::AdvancedCharacterController* pobCharacter = 0;
	if ( m_pobMovement->GetPhysicsSystem() )
		pobCharacter = (Physics::AdvancedCharacterController*) m_pobMovement->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
	if( pobCharacter )
		pobCharacter->SetApplyCharacterControllerGravity(m_obDefinition.m_bApplyGravity);

	// If we are still here then we are still going
	return false;
}


//------------------------------------------------------------------------------------------
//!
//!	RagdollContinuationTransitionDef::CreateInstance
//!	Create me
//!
//------------------------------------------------------------------------------------------
MovementController* RagdollContinuationTransitionDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) RagdollContinuationTransition( pobMovement, *this );
}

//------------------------------------------------------------------------------------------
//!
//!	RagdollContinuationTransitionDef::Clone
//!	Clone me
//!
//------------------------------------------------------------------------------------------
MovementControllerDef* RagdollContinuationTransitionDef::Clone( void ) const 
{ 
	return NT_NEW_CHUNK(Mem::MC_ENTITY) RagdollContinuationTransitionDef( *this ); 
}

//------------------------------------------------------------------------------------------
//!
//!	RagdollContinuationTransition::RagdollContinuationTransition
//!	Construct me
//!
//------------------------------------------------------------------------------------------
RagdollContinuationTransition::RagdollContinuationTransition( CMovement* pobMovement, const RagdollContinuationTransitionDef& obDef )
: MovementController( pobMovement ),
m_obDef( obDef )
{	
	InternalRegisterDefinition( m_obDef );
};

//------------------------------------------------------------------------------------------
//!
//!	RagdollContinuationTransition::Update
//!	Update me, waiting for the right time to goto floored state if neccessary
//!
//------------------------------------------------------------------------------------------
bool RagdollContinuationTransition::Update(	float						fTimeStep, 
							const CMovementInput&		/*obMovementInput*/,
							const CMovementStateRef&	/*obCurrentMovementState*/,
							CMovementState&				/*obPredictedMovementState*/ )
{
	if (m_bFirstFrame)
	{
		m_bFirstFrame = !m_bFirstFrame;	
		m_fTimeInRagdoll = 0.0f; // Keep track of how long we've been ragdolled
		m_fTimeToEnd = 3.0f + (grandf(2.0f)-1.0f); // And set the maximum amount of time we want to be ragdolled
		m_pobAnimator->RemoveAllAnimations(); // Stop all anims
		
		CAttackComponent* pobAttack = const_cast< CAttackComponent* >(m_pobMovement->GetParentEntity()->GetAttackComponent());			
		// If we're mid super safety transition then we get the attacker and notify them I've ragdolled
		if (pobAttack && pobAttack->IsInSuperStyleSafetyTransition() && pobAttack->AI_Access_GetState() == CS_HELD)
		{
			pobAttack->GetSuperStyleSafetyAttacker()->GetAttackComponent()->NotifyHeldVictimRagdolled();
		}
		// If we were HELD, we need to tell our attacker that we've ragdolled so they can stop looking silly by attacking thin air
		else if (pobAttack && (pobAttack->AI_Access_GetState() == CS_HELD || pobAttack->AI_Access_GetState() == CS_DYING))
		{
			// Breaking const to save resorting to lua
			pobAttack = const_cast< CAttackComponent* >(m_pobMovement->GetParentEntity()->GetAttackComponent()->GetStruckStrike()->GetOriginatorP()->GetAttackComponent());
			pobAttack->NotifyHeldVictimRagdolled();
		}
	}

	// Update time
	m_fTimeInRagdoll += fTimeStep;

	// Do nothing except wait for rest, or the maximum time
	Physics::AdvancedCharacterController* pobCharacterState = (Physics::AdvancedCharacterController*) m_pobMovement->GetParentEntity()->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
	if (pobCharacterState && !m_pobMovement->GetParentEntity()->ToCharacter()->IsDead())
	{
		// Check the movement status of the ragdoll (cumulative velocity squared of each rigid body), but it must be in ragdoll for a while.
		if (( pobCharacterState->GetAdvancedRagdoll()->GetMotionStatus() < sqr( 1.0f ) && m_fTimeInRagdoll > 0.1f)
			|| m_fTimeInRagdoll > m_fTimeToEnd )
		{
			// Stop all movement on all bones
			CDirection obZero(0.0,0.0,0.0);
			pobCharacterState->SetRagdollLinearVelocity(obZero);

			// Breaking const to save resorting to lua
			CAttackComponent* pobAttack = const_cast< CAttackComponent* >(m_pobMovement->GetParentEntity()->GetAttackComponent());
			
			// Only do this if we're KO'd or HELD - we'll stay in this controller indefinately if we're DEAD
			if (pobAttack && (pobAttack->AI_Access_GetState() == CS_KO || pobAttack->AI_Access_GetState() == CS_HELD))
			{
				// Update if I'm face up or down when I've stopped moving
				Physics::AdvancedCharacterController* pobCharacterState = (Physics::AdvancedCharacterController*) m_pobMovement->GetParentEntity()->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
				if (pobCharacterState)
				{
					Transform* pobPelvis = m_pobMovement->GetParentEntity()->GetHierarchy()->GetRootTransform()->GetFirstChild();
					if (pobPelvis->GetWorldMatrix().GetZAxis().Y() < -0.5f)
						pobAttack->SetStruckReactionZone(RZ_BACK);
					else
						pobAttack->SetStruckReactionZone(RZ_FRONT);
				}

				// So we don't do an impact animation, notify attack component we did a physical impact
				pobAttack->NotifyWasFullyRagdolled();
				// Start floored state
				pobAttack->StartFlooredState();
				// This movement controller is done
				return true;
			}
		}
	}

	return false;
};

//------------------------------------------------------------------------------------------
//!
//!	RagdollFlooredTransitionDef::CreateInstance
//!	Create me
//!
//------------------------------------------------------------------------------------------
MovementController* RagdollFlooredTransitionDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) RagdollFlooredTransition( pobMovement, *this );
}

//------------------------------------------------------------------------------------------
//!
//!	RagdollFlooredTransitionDef::CreateInstance
//!	Clone me
//!
//------------------------------------------------------------------------------------------
MovementControllerDef* RagdollFlooredTransitionDef::Clone( void ) const 
{ 
	return NT_NEW_CHUNK(Mem::MC_ENTITY) RagdollFlooredTransitionDef( *this ); 
}

//------------------------------------------------------------------------------------------
//!
//!	RagdollFlooredTransition::RagdollFlooredTransition
//!	Construct me
//!
//------------------------------------------------------------------------------------------
RagdollFlooredTransition::RagdollFlooredTransition( CMovement* pobMovement, const RagdollFlooredTransitionDef& obDef )
: MovementController( pobMovement ),
m_obDef( obDef )
{	
	InternalRegisterDefinition( m_obDef );

	// Create our animation and add it to the animator
	m_obFlooredAnimation = m_pobAnimator->CreateAnimation( m_obDef.m_obFlooredAnimationName );
	m_obFlooredAnimation->SetBlendWeight( 1.0f );

	int iNumTransforms = Physics::HierarchyTools::NumberOfTransform( m_pobMovement->GetParentEntity()->GetHierarchy(), m_pobMovement->GetParentEntity()->GetHierarchy()->GetRootTransform()->GetFirstChild() );
	m_aobFinalRagdollAnimatedPose.setSize(iNumTransforms);
	m_aobNewBlendedAnimatedPose.setSize(iNumTransforms);

	m_obFlooredAnimation->SetFlagBits( ANIMF_LOOPING | ANIMF_LOCOMOTING | ANIMF_INHIBIT_AUTO_DESTRUCT );
};

//------------------------------------------------------------------------------------------
//!
//!	RagdollFlooredTransition::~RagdollFlooredTransition
//!	Destruct me
//!
//------------------------------------------------------------------------------------------
RagdollFlooredTransition::~RagdollFlooredTransition()
{
	// Only remove the animation if we have done an update
	if ( !m_bFirstFrame )
		m_pobAnimator->RemoveAnimation( m_obFlooredAnimation );
}

//------------------------------------------------------------------------------------------
//!
//!	RagdollFlooredTransition::Update
//!	Update my orientation so when the next proper animation starts, I'm as close as I possibly
//! can be to the traditional root position and rotation. And pin me to the ground when I'm in
//! the right position/orientation to stop me bouncing myself all over the place while animating.
//!
//------------------------------------------------------------------------------------------
bool RagdollFlooredTransition::Update(	float						fTimeStep, 
							const CMovementInput&		/*obMovementInput*/,
							const CMovementStateRef&	/*obCurrentMovementState*/,
							CMovementState&				/*obPredictedMovementState*/ )
{
	if (m_bFirstFrame)
	{
		// Get the state of the animated hierarchy as it stands right now before any anim
		Physics::HierarchyTools::GetLocalPoseRecursive(m_aobFinalRagdollAnimatedPose, m_pobMovement->GetParentEntity()->GetHierarchy(), m_pobMovement->GetParentEntity()->GetHierarchy()->GetRootTransform()->GetFirstChild());
		
		Physics::AdvancedCharacterController* pobCharacter = 0;
		if ( m_pobMovement->GetPhysicsSystem() )
			pobCharacter = (Physics::AdvancedCharacterController*) m_pobMovement->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
		if (pobCharacter)
		{
			CMatrix obFinalRagdollRoot = pobCharacter->GetAdvancedRagdoll()->GetWorldMatrixBeforeUpdate();			
			pobCharacter->StartBlendFromRagdollToAnimation(&m_aobFinalRagdollAnimatedPose,obFinalRagdollRoot);
		}
		
		// Kick off the animation
		m_pobAnimator->AddAnimation( m_obFlooredAnimation );
		m_obFlooredAnimation->SetTimeRemaining( m_obFlooredAnimation->GetDuration() );

		m_bFirstFrame = !m_bFirstFrame;	
	} 

	// Gravity setting
	Physics::AdvancedCharacterController* pobCharacter = 0;
	if ( m_pobMovement->GetPhysicsSystem() )
		pobCharacter = (Physics::AdvancedCharacterController*) m_pobMovement->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
	if( pobCharacter )
		pobCharacter->SetApplyCharacterControllerGravity(true);

	m_obFlooredAnimation->SetBlendWeight( 1.0f );
	
	// We never need to return true from this because the attackcomponent will force this off the movement controller when it's ready to recover
	return false;
};

//------------------------------------------------------------------------------------------
//!
//!	SuperStyleSafetyTransitionDef::CreateInstance
//!	Create me
//!
//------------------------------------------------------------------------------------------
MovementController* SuperStyleSafetyTransitionDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) SuperStyleSafetyTransition( pobMovement, *this );
}

//------------------------------------------------------------------------------------------
//!
//!	SuperStyleSafetyTransitionDef::CreateInstance
//!	Clone me
//!
//------------------------------------------------------------------------------------------
MovementControllerDef* SuperStyleSafetyTransitionDef::Clone( void ) const 
{ 
	return NT_NEW_CHUNK(Mem::MC_ENTITY) SuperStyleSafetyTransitionDef( *this );
}

//------------------------------------------------------------------------------------------
//!
//!	SuperStyleSafetyTransition::SuperStyleSafetyTransition
//!	Construct me
//!
//------------------------------------------------------------------------------------------
SuperStyleSafetyTransition::SuperStyleSafetyTransition( CMovement* pobMovement, const SuperStyleSafetyTransitionDef& obDef )
: MovementController( pobMovement ),
m_obDef( obDef )
{	
	InternalRegisterDefinition( m_obDef );

	m_fTimeInTransition = 0.0f;
	m_fMovementSpeed = 5.0f;
	
	m_bDirectionFlipped = false;

	if (m_obDef.m_pobTransitionAnimation)
	{
		m_obDef.m_pobTransitionAnimation->SetBlendWeight( 1.0f );
		m_obDef.m_pobTransitionAnimation->SetFlagBits( ANIMF_RELATIVE_MOVEMENT | ANIMF_INHIBIT_AUTO_DESTRUCT );
	}
};

//------------------------------------------------------------------------------------------
//!
//!	SuperStyleSafetyTransition::~SuperStyleSafetyTransition
//!	Destruct me
//!
//------------------------------------------------------------------------------------------
SuperStyleSafetyTransition::~SuperStyleSafetyTransition()
{
	if (m_obDef.m_pobTransitionAnimation && m_obDef.m_pobTransitionAnimation->IsActive())
	{
		m_pobAnimator->RemoveAnimation(m_obDef.m_pobTransitionAnimation);
	}
}

//------------------------------------------------------------------------------------------
//!
//!	SuperStyleSafetyTransition::Update
//!	Update, moving to a safe point. If we've got an animation and a relative transform, use them
//! otherwise just slide us.
//!
//------------------------------------------------------------------------------------------
bool SuperStyleSafetyTransition::Update(	float						fTimeStep, 
							const CMovementInput&		/*obMovementInput*/,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState )
{
	// Keep track of time
	m_fTimeInTransition += fTimeStep;

	// If we don't have an anim to play we'll be doing all movement ourselves
	if (!m_obDef.m_pobTransitionAnimation)
	{
		if (m_bFirstFrame)
		{
			// Make a note of where we started
			m_obStartPoint = obCurrentMovementState.m_obPosition;	
		}

		// Get a vector from where we are to where we want to be
		CDirection obTravelVector = CDirection(m_obDef.m_obSafePoint - obCurrentMovementState.m_obPosition);
		obTravelVector.Y() = 0.0f;
		float fDistanceSquared = obTravelVector.LengthSquared();
		// Normalise it and use it to move ourselves at a set velocity
		obTravelVector.Normalise();
		obTravelVector *= m_fMovementSpeed;
		obTravelVector *= fTimeStep;

		// Check our current travel vector with our last travel vector to check for flippy juddering
		if (!m_bFirstFrame) // Only do it after one frame so we have a valid LastTravelVector value
		{
			CDirection obNormTravel(obTravelVector);
			CDirection obNormLastTravel(m_obLastTravelVector);
			obNormTravel.Normalise();
			obNormLastTravel.Normalise();
			
			float fAngle = acos(obNormTravel.Dot(obNormLastTravel));
			
			if (fAngle > HALF_PI && fDistanceSquared < 0.25f*0.25f)
			{
				m_bDirectionFlipped = true;
			}
		}	
		
		if (!m_bDirectionFlipped) // Only add more movement if it's not gonna add to flipping issues
			obPredictedMovementState.m_obProceduralRootDelta = obTravelVector;
		else
			obPredictedMovementState.m_obProceduralRootDelta.Clear();

		// If we're within a reasonable distance, or we're flipping in our direction, then stop
		if (m_bDirectionFlipped || fDistanceSquared < 0.1f*0.1f)
		{
			// Finish ourselves off...
			if (!m_bDirectionFlipped) // Only if it's not gonna cause more flipping though
				obPredictedMovementState.m_obProceduralRootDelta = CDirection(m_obDef.m_obSafePoint - obCurrentMovementState.m_obPosition);
			else
				obPredictedMovementState.m_obProceduralRootDelta.Clear();
			
			// Notify the attackers attack component that we're done
			m_obDef.m_pobNotifyThis->NotifyDoneSuperStyleSafetyTransition(m_pobMovement->GetParentEntity());
		
			// Be done.
			return true;
		}

		m_obLastTravelVector = obTravelVector;
	}
	else // We're animating and using a relative transform, in which case we orient the Z of our relative transform to the travel vector, and scale the total anim translation so it travels in the direction of the safe area
	{
		// Get the angles we need to rotate and scales we need to apply in the first frame
		if (m_bFirstFrame)
		{
			// Get the anim going
			m_pobAnimator->RemoveAllAnimations();
			m_obDef.m_pobTransitionAnimation->m_pobRelativeTransform = m_obDef.m_pobRelativeTransform;
			m_pobAnimator->AddAnimation( m_obDef.m_pobTransitionAnimation );
			m_obDef.m_pobTransitionAnimation->SetTimeRemaining( m_obDef.m_pobTransitionAnimation->GetDuration() );

			// Get a vector from where we are to where we want to be
			m_obDef.m_obSafePoint.Y() = m_obDef.m_pobRelativeTransform->GetLocalTranslation().Y(); // Shouldn't move up/down
			CDirection obTravelVector = CDirection(m_obDef.m_obSafePoint - m_obDef.m_pobRelativeTransform->GetLocalTranslation());
			float fTravelDistance = obTravelVector.Length();
			obTravelVector.Normalise();

			// Make a note of where we started
			m_obStartPoint = m_obDef.m_pobRelativeTransform->GetLocalTranslation();	
			CMatrix obRelativeTransformMatrix = m_obDef.m_pobRelativeTransform->GetLocalMatrix();
			m_obRelativeTransitionStartX = obRelativeTransformMatrix.GetXAxis();
			m_obRelativeTransitionStartY = obRelativeTransformMatrix.GetYAxis();
			m_obRelativeTransitionStartZ = obRelativeTransformMatrix.GetZAxis();

			// Calculate a new set of axes that represents the safe end state of the relative transform
			m_fYawNeeded = MovementControllerUtilities::RotationAboutY( m_obRelativeTransitionStartZ, obTravelVector );
			// Rotate the current axes by that much, and store them for lerping
			CMatrix obRotation( CONSTRUCT_IDENTITY );
			obRotation.SetFromAxisAndAngle(CDirection(0.0,1.0,0.0),m_fYawNeeded);	
			m_obRelativeTransitionEndX = obRelativeTransformMatrix.GetXAxis() * obRotation;
			m_obRelativeTransitionEndY = obRelativeTransformMatrix.GetYAxis() * obRotation;
			m_obRelativeTransitionEndZ = obRelativeTransformMatrix.GetZAxis() * obRotation;

			// Get the amount of movement this anim will provide
			CPoint obEndTranslation = m_obDef.m_pobTransitionAnimation->GetRootEndTranslation();
			float fAnimDistance = obEndTranslation.Length();
			// Use it to scale the distance we travel
			m_fScalarX = m_fScalarY = m_fScalarZ = fTravelDistance/fAnimDistance;
		}

		// Keep setting root scalar so animator doesn't go back to 1.0f
		obPredictedMovementState.m_obRootDeltaScalar.X() = m_fScalarX;
		obPredictedMovementState.m_obRootDeltaScalar.Y() = m_fScalarY;
		obPredictedMovementState.m_obRootDeltaScalar.Z() = m_fScalarZ;

		// Make sure the anim is playing at full whack
		m_obDef.m_pobTransitionAnimation->SetBlendWeight( 1.0f );

		// Only one of the 2 concurrent safety transition controllers (there's one on the attacker and one on the victim)
		// should update the relative transform. I'm arbitrarily picking the attacker, because I don't trust the AIs
		if (m_pobMovement->GetParentEntity()->GetAttackComponent()->AI_Access_GetState() == CS_ATTACKING)
		{
			// Get some value to use to smooth out movement
			float fLerpAmount = m_obDef.m_pobTransitionAnimation->GetTime()/m_obDef.m_pobTransitionAnimation->GetDuration();
			fLerpAmount = CMaths::SmoothStep(fLerpAmount); // Schmoooodth

			// Get the matrix of the relative transform
			CMatrix obRelativeTransformMatrix = m_obDef.m_pobRelativeTransform->GetLocalMatrix();
			// Shift it gradually towards safe point and orientation - using a lerp so in future we can weight the original and target in some custom way for individual anims
			if (m_obDef.m_bTranslate)
			{
				obRelativeTransformMatrix.SetTranslation(CPoint::Lerp(m_obStartPoint, m_obDef.m_obSafePoint, fLerpAmount));	
			}
			if (m_obDef.m_bRotate)
			{
				float fYawNeeded = m_fYawNeeded * fLerpAmount;

				CMatrix obRotation( CONSTRUCT_IDENTITY );
				obRotation.SetFromAxisAndAngle(CDirection(0.0,1.0,0.0),fYawNeeded);	

				CDirection obNewX = m_obRelativeTransitionStartX * obRotation;
				obNewX.Normalise();
				CDirection obNewY = m_obRelativeTransitionStartY * obRotation;
				obNewY.Normalise();
				CDirection obNewZ = m_obRelativeTransitionStartZ * obRotation;
				obNewZ.Normalise();

				obRelativeTransformMatrix.SetXAxis(obNewX);
				obRelativeTransformMatrix.SetYAxis(obNewY);
				obRelativeTransformMatrix.SetZAxis(obNewZ);
			}
			
			// Set the matrix to our relative transition
			m_obDef.m_pobRelativeTransform->SetLocalMatrix(obRelativeTransformMatrix);

			// Debug render
			#ifdef _DEBUG
			g_VisualDebug->RenderAxis(m_obDef.m_pobRelativeTransform->GetLocalMatrix(),2.5f);
			g_VisualDebug->RenderPoint(m_obDef.m_obSafePoint,10.0f,DC_YELLOW);
			#endif
		}

		// We done?
		if ( m_obDef.m_pobTransitionAnimation->GetTime() >= m_obDef.m_pobTransitionAnimation->GetDuration() )
		{		
			// Get the matrix of the relative transform
			CMatrix obRelativeTransformMatrix = m_obDef.m_pobRelativeTransform->GetLocalMatrix();
			// Set final values
			if (m_obDef.m_bTranslate)
			{
				obRelativeTransformMatrix.SetTranslation(m_obDef.m_obSafePoint);	
			}
			if (m_obDef.m_bRotate)
			{
				obRelativeTransformMatrix.SetXAxis(m_obRelativeTransitionEndX);
				obRelativeTransformMatrix.SetYAxis(m_obRelativeTransitionEndY);
				obRelativeTransformMatrix.SetZAxis(m_obRelativeTransitionEndZ);
			}
			
			// Set the matrix to our relative transition
			m_obDef.m_pobRelativeTransform->SetLocalMatrix(obRelativeTransformMatrix);

			// Notify the attackers attack component that we're done
			if (m_obDef.m_bNotifyAtEnd)
				m_obDef.m_pobNotifyThis->NotifyDoneSuperStyleSafetyTransition(m_pobMovement->GetParentEntity());
	

			// Be done.
			return true;
		}
	}
	
	if (m_fTimeInTransition > 5.0f)
	{
		// We've been here for a long time, stopping
		ntPrintf("SuperStyleSafetyTransition: Bailing because of time out.");
		
		if (!m_pobMovement)
		{
			ntPrintf("SuperStyleSafetyTransition: No movement component pointer while bailing!");
		} 
		else if (!m_pobMovement->GetParentEntity())
		{
			ntPrintf("SuperStyleSafetyTransition: No parent entity pointer while bailing!");
		}
		else if (m_pobMovement->GetParentEntity()->GetAttackComponent())
		{
			ntPrintf("SuperStyleSafetyTransition: No attack component while bailing!");
		}

		if ( m_pobMovement && m_pobMovement->GetParentEntity() && m_pobMovement->GetParentEntity()->GetAttackComponent())
		{
			// Notify the attackers attack component that we're done
			CAttackComponent* pobAttack = const_cast< CAttackComponent* >(m_pobMovement->GetParentEntity()->GetAttackComponent());			
			if (pobAttack)
				pobAttack->ForceDoneSuperStyleSafetyTransition();
		}
		
		return true;
	}
	
	if (m_bFirstFrame)
	{
		m_bFirstFrame = !m_bFirstFrame;
	}

	return false;
};
