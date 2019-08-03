//------------------------------------------------------------------------------------------
//!
//!	\file FormationStrafeController.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "formationstrafecontroller.h"
#include "anim/animator.h"
#include "core/exportstruct_anim.h"
#include "objectdatabase/dataobject.h"
#include "core/OSDDisplay.h"
#include "movement.h"

START_STD_INTERFACE(FormationStrafeControllerDef)
	_IREFERENCE(AnimSet)
	_IFLOAT(InputThreshold)
#ifndef _RELEASE
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
#endif
END_STD_INTERFACE

START_STD_INTERFACE(FormationStrafeAnimSet)
	_ISTRING(Standing)
	_ISTRING(MoveForwards)
	_ISTRING(MoveBackwards)
	_ISTRING(MoveLeft)
	_ISTRING(MoveRight)
	_ISTRING(MoveForwardLeft)
	_ISTRING(MoveBackwardLeft)
	_ISTRING(MoveForwardRight)
	_ISTRING(MoveBackwardRight)
END_STD_INTERFACE

//------------------------------------------------------------------------------------------
//!
//!	FormationStrafeControllerDef::FormationStrafeControllerDef
//! Construction
//!	Attaches the XML Interface
//!
//------------------------------------------------------------------------------------------
FormationStrafeControllerDef::FormationStrafeControllerDef()
 : m_pobAnimSet(0), m_fInputThreshold(0.2f)
{
	
}


//------------------------------------------------------------------------------------------
//!
//!	FormationStrafeControllerDef::CreateInstance
//!	To create an instance of what we describe
//!
//------------------------------------------------------------------------------------------
MovementController* FormationStrafeControllerDef::CreateInstance(CMovement* pMovement) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) FormationStrafeController(pMovement, *this);
	OSD::Add(OSD::MOVEMENT, DC_WHITE, "Created FormationStrafeController");
}


//------------------------------------------------------------------------------------------
//!
//!	FormationStrafeController::FormationStrafeController
//! Construction
//!
//------------------------------------------------------------------------------------------
FormationStrafeController::FormationStrafeController(CMovement* pMovement, 
                                   const FormationStrafeControllerDef& def)
:	MovementController(pMovement),
	m_obDefinition(def),
	m_fRecheckTime(0.f),
	m_fMoveSpeed(-1.f)
{
	// Register our definition for debugging purposes
	InternalRegisterDefinition( m_obDefinition );

	// Create our animations
	m_aobMovingAnims[MA_STANDING]     = CreateAnimation(m_obDefinition.m_pobAnimSet->m_obStanding, ANIMF_LOOPING|ANIMF_PHASE_LINKED);
	m_aobMovingAnims[MA_FORWARD]      = CreateAnimation(m_obDefinition.m_pobAnimSet->m_obMoveForwards, ANIMF_LOOPING|ANIMF_PHASE_LINKED);
	m_aobMovingAnims[MA_BACK]         = CreateAnimation(m_obDefinition.m_pobAnimSet->m_obMoveBackwards, ANIMF_LOOPING|ANIMF_PHASE_LINKED);
	m_aobMovingAnims[MA_LEFT]         = CreateAnimation(m_obDefinition.m_pobAnimSet->m_obMoveLeft, ANIMF_LOOPING|ANIMF_PHASE_LINKED);
	m_aobMovingAnims[MA_RIGHT]        = CreateAnimation(m_obDefinition.m_pobAnimSet->m_obMoveRight, ANIMF_LOOPING|ANIMF_PHASE_LINKED);
	m_aobMovingAnims[MA_FORWARDLEFT]  = CreateAnimation(m_obDefinition.m_pobAnimSet->m_obMoveForwardLeft, ANIMF_LOOPING|ANIMF_PHASE_LINKED);
	m_aobMovingAnims[MA_BACKLEFT]     = CreateAnimation(m_obDefinition.m_pobAnimSet->m_obMoveBackwardLeft, ANIMF_LOOPING|ANIMF_PHASE_LINKED);
	m_aobMovingAnims[MA_FORWARDRIGHT] = CreateAnimation(m_obDefinition.m_pobAnimSet->m_obMoveForwardRight, ANIMF_LOOPING|ANIMF_PHASE_LINKED);
	m_aobMovingAnims[MA_BACKRIGHT]    = CreateAnimation(m_obDefinition.m_pobAnimSet->m_obMoveBackwardRight, ANIMF_LOOPING|ANIMF_PHASE_LINKED);

	// Make sure we have a sensible definition
	ntAssert((m_obDefinition.m_fInputThreshold < 1.0f) && (m_obDefinition.m_fInputThreshold >= 0.0f));
}


//------------------------------------------------------------------------------------------
//!
//!	FormationStrafeController::~FormationStrafeController
//! Destruction
//!
//------------------------------------------------------------------------------------------
FormationStrafeController::~FormationStrafeController()
{
	// Deactivate all the animations
	if ( !m_bFirstFrame )
		DeactivateAnimations( m_aobMovingAnims, MA_COUNT );
}


//------------------------------------------------------------------------------------------
//!
//!	FormationStrafeController::CreateAnimation
//! Creates an animation from a name and sets the correct flags
//!
//------------------------------------------------------------------------------------------
CAnimationPtr FormationStrafeController::CreateAnimation(const CHashedString& obAnimationName, int iVariableFlags)
{
	// Create the animation
	CAnimationPtr obReturnAnim = m_pobAnimator->CreateAnimation(obAnimationName);

	// Zero the initial blend weight
	obReturnAnim->SetBlendWeight(0.0f);

	// Set up the flags - including the item we passed in
	obReturnAnim->SetFlags(iVariableFlags|ANIMF_LOCOMOTING|ANIMF_INHIBIT_AUTO_DESTRUCT);

	// Return a pointer to the animation
	return obReturnAnim;
}


//------------------------------------------------------------------------------------------
//!
//!	FormationStrafeController::ActivateAnimations
//! Activate a sub-states set of animations
//!
//------------------------------------------------------------------------------------------
void FormationStrafeController::ActivateAnimations(CAnimationPtr* aAnimations, int iNumberOfAnims)
{
	// Add to the animator
	for (int iAnimations = 0; iAnimations < iNumberOfAnims; ++iAnimations)
		m_pobAnimator->AddAnimation(aAnimations[iAnimations]);

	// Zero the blend weights
	for (int iAnimations = 0; iAnimations < iNumberOfAnims; ++iAnimations)
		aAnimations[iAnimations]->SetBlendWeight(0.0f);
}


//------------------------------------------------------------------------------------------
//!
//!	FormationStrafeController::DeactivateAnimations
//! Remove a sub-states set of animations from the animator
//!
//------------------------------------------------------------------------------------------
void FormationStrafeController::DeactivateAnimations(CAnimationPtr* aAnimations, int iNumberOfAnims)
{
	for (int iAnimations = 0; iAnimations < iNumberOfAnims; ++iAnimations)
		m_pobAnimator->RemoveAnimation(aAnimations[iAnimations]);
}


//------------------------------------------------------------------------------------------
//!
//!	FormationStrafeController::InitialiseAnimations
//! Adds the animations to the animator and initialises blend weights
//!
//------------------------------------------------------------------------------------------
void FormationStrafeController::InitialiseAnimations( void )
{
	// Add all the animations to the animator here for now
	ActivateAnimations(m_aobMovingAnims, MA_COUNT);

	m_afDesiredMovingWeights[MA_STANDING]     = m_afMovingWeights[MA_STANDING]     = 0.0f;
	m_afDesiredMovingWeights[MA_BACK]         = m_afMovingWeights[MA_BACK]         = 0.0f;
	m_afDesiredMovingWeights[MA_FORWARD]      = m_afMovingWeights[MA_FORWARD]      = 0.0f;
	m_afDesiredMovingWeights[MA_RIGHT]        = m_afMovingWeights[MA_RIGHT]        = 0.0f;
	m_afDesiredMovingWeights[MA_LEFT]         = m_afMovingWeights[MA_LEFT]         = 0.0f;
	m_afDesiredMovingWeights[MA_FORWARDLEFT]  = m_afMovingWeights[MA_FORWARDLEFT]  = 0.0f;
	m_afDesiredMovingWeights[MA_BACKLEFT]     = m_afMovingWeights[MA_BACKLEFT]     = 0.0f;
	m_afDesiredMovingWeights[MA_FORWARDRIGHT] = m_afMovingWeights[MA_FORWARDRIGHT] = 0.0f;
	m_afDesiredMovingWeights[MA_BACKRIGHT]    = m_afMovingWeights[MA_BACKRIGHT]    = 0.0f;
}


//------------------------------------------------------------------------------------------
//!
//!	FormationStrafeController::Update
//! The main movement controller functionality
//!
//------------------------------------------------------------------------------------------
bool FormationStrafeController::Update(float                    fTimeStep, 
                                       const CMovementInput&    obMovementInput,
                                       const CMovementStateRef&	obCurrentMovementState,
                                       CMovementState&          obPredictedMovementState)
{
	// Only add the animations to the animator on the first frame
	if ( m_bFirstFrame )
		InitialiseAnimations();

	// Ensure that the input we are running off is normalised
	CDirection obInputDirection(obMovementInput.m_obMoveDirection);
	obInputDirection.Normalise();

	// Ensure that we don't have a silly input speed
	float fInputSpeed = clamp(obMovementInput.m_fMoveSpeed, 0.0f, 1.0f);
	const float fLerpSpeed = clamp( ( 0.1f * ( fTimeStep / ( 1.0f / 30.0f ) ) ), 0.0f, 1.0f );
	m_fMoveSpeed = m_fMoveSpeed < 0.f ? fInputSpeed : CMaths::Lerp(m_fMoveSpeed, fInputSpeed, fLerpSpeed );

	// If we have a target set up some values
	float fRequiredTurn = 0.0f;
	if (obMovementInput.m_obFacingDirection.LengthSquared() > 0.0f)
	{
		// Find the required turn and claculate the weights
		CDirection obRequiredFaceDirection(obMovementInput.m_obFacingDirection);
		fRequiredTurn = MovementControllerUtilities::RotationAboutY(obCurrentMovementState.m_obFacing, obRequiredFaceDirection);
	}

	else if (obMovementInput.m_bTargetPointSet)
	{
		// Find the required turn
		CDirection obRequiredFaceDirection(CDirection(obMovementInput.m_obTargetPoint) - CDirection(obCurrentMovementState.m_obPosition));
		fRequiredTurn = MovementControllerUtilities::RotationAboutY(obCurrentMovementState.m_obFacing, obRequiredFaceDirection);
	}

	else {} // We don't need to do anything if we have no facing direction

	// Calculate the blend weights of our individual states
	if(m_aobMovingAnims[MA_FORWARD]->GetTime() >= m_fRecheckTime &&
	   m_aobMovingAnims[MA_FORWARD]->GetTime() <= m_fRecheckTime + fTimeStep)
	{
		CalculateMoveWeights(obCurrentMovementState.m_obRootMatrix, obInputDirection, m_fMoveSpeed);

		m_fRecheckTime = m_fRecheckTime < EPSILON ? m_aobMovingAnims[MA_FORWARD]->GetDuration() / 2.f : 0.f;

		OSD::Add(OSD::MOVEMENT, DC_WHITE, "RECHECK");
	}
	
	// A constant for our smoothness
	const float fMoveLerp = clamp( ( 0.08f * ( fTimeStep / ( 1.0f / 30.0f ) ) ), 0.0f, 1.0f );

	// Set the weights directly to our animations	
	m_afMovingWeights[MA_STANDING    ] = m_fBlendWeight * CMaths::Lerp(m_afMovingWeights[MA_STANDING    ], m_afDesiredMovingWeights[MA_STANDING    ], fMoveLerp);
	m_afMovingWeights[MA_FORWARD     ] = m_fBlendWeight * CMaths::Lerp(m_afMovingWeights[MA_FORWARD     ], m_afDesiredMovingWeights[MA_FORWARD     ], fMoveLerp);
	m_afMovingWeights[MA_BACK        ] = m_fBlendWeight * CMaths::Lerp(m_afMovingWeights[MA_BACK        ], m_afDesiredMovingWeights[MA_BACK        ], fMoveLerp);
	m_afMovingWeights[MA_LEFT        ] = m_fBlendWeight * CMaths::Lerp(m_afMovingWeights[MA_LEFT        ], m_afDesiredMovingWeights[MA_LEFT        ], fMoveLerp);
	m_afMovingWeights[MA_RIGHT       ] = m_fBlendWeight * CMaths::Lerp(m_afMovingWeights[MA_RIGHT       ], m_afDesiredMovingWeights[MA_RIGHT       ], fMoveLerp);
	m_afMovingWeights[MA_FORWARDLEFT ] = m_fBlendWeight * CMaths::Lerp(m_afMovingWeights[MA_FORWARDLEFT ], m_afDesiredMovingWeights[MA_FORWARDLEFT ], fMoveLerp);
	m_afMovingWeights[MA_BACKLEFT    ] = m_fBlendWeight * CMaths::Lerp(m_afMovingWeights[MA_BACKLEFT    ], m_afDesiredMovingWeights[MA_BACKLEFT    ], fMoveLerp);
	m_afMovingWeights[MA_FORWARDRIGHT] = m_fBlendWeight * CMaths::Lerp(m_afMovingWeights[MA_FORWARDRIGHT], m_afDesiredMovingWeights[MA_FORWARDRIGHT], fMoveLerp);
	m_afMovingWeights[MA_BACKRIGHT   ] = m_fBlendWeight * CMaths::Lerp(m_afMovingWeights[MA_BACKRIGHT   ], m_afDesiredMovingWeights[MA_BACKRIGHT   ], fMoveLerp);
	m_aobMovingAnims[MA_STANDING    ]->SetBlendWeight(m_fBlendWeight * CMaths::Lerp(m_afMovingWeights[MA_STANDING    ], m_afDesiredMovingWeights[MA_STANDING    ], fMoveLerp));
	m_aobMovingAnims[MA_FORWARD     ]->SetBlendWeight(m_fBlendWeight * CMaths::Lerp(m_afMovingWeights[MA_FORWARD     ], m_afDesiredMovingWeights[MA_FORWARD     ], fMoveLerp));
	m_aobMovingAnims[MA_BACK        ]->SetBlendWeight(m_fBlendWeight * CMaths::Lerp(m_afMovingWeights[MA_BACK        ], m_afDesiredMovingWeights[MA_BACK        ], fMoveLerp));
	m_aobMovingAnims[MA_LEFT        ]->SetBlendWeight(m_fBlendWeight * CMaths::Lerp(m_afMovingWeights[MA_LEFT        ], m_afDesiredMovingWeights[MA_LEFT        ], fMoveLerp));
	m_aobMovingAnims[MA_RIGHT       ]->SetBlendWeight(m_fBlendWeight * CMaths::Lerp(m_afMovingWeights[MA_RIGHT       ], m_afDesiredMovingWeights[MA_RIGHT       ], fMoveLerp));
	m_aobMovingAnims[MA_FORWARDLEFT ]->SetBlendWeight(m_fBlendWeight * CMaths::Lerp(m_afMovingWeights[MA_FORWARDLEFT ], m_afDesiredMovingWeights[MA_FORWARDLEFT ], fMoveLerp));
	m_aobMovingAnims[MA_BACKLEFT    ]->SetBlendWeight(m_fBlendWeight * CMaths::Lerp(m_afMovingWeights[MA_BACKLEFT    ], m_afDesiredMovingWeights[MA_BACKLEFT    ], fMoveLerp));
	m_aobMovingAnims[MA_FORWARDRIGHT]->SetBlendWeight(m_fBlendWeight * CMaths::Lerp(m_afMovingWeights[MA_FORWARDRIGHT], m_afDesiredMovingWeights[MA_FORWARDRIGHT], fMoveLerp));
	m_aobMovingAnims[MA_BACKRIGHT   ]->SetBlendWeight(m_fBlendWeight * CMaths::Lerp(m_afMovingWeights[MA_BACKRIGHT   ], m_afDesiredMovingWeights[MA_BACKRIGHT   ], fMoveLerp));

	// Set the speed too...
	//m_aobMovingAnims[MA_STANDING    ]->SetSpeed(m_fMoveSpeed);// * m_afMovingWeights[MA_STANDING]);
	m_aobMovingAnims[MA_FORWARD     ]->SetSpeed(m_fMoveSpeed);// * m_afMovingWeights[MA_FORWARD]);
	m_aobMovingAnims[MA_BACK        ]->SetSpeed(m_fMoveSpeed);// * m_afMovingWeights[MA_BACK]);
	m_aobMovingAnims[MA_LEFT        ]->SetSpeed(m_fMoveSpeed);// * m_afMovingWeights[MA_LEFT]);
	m_aobMovingAnims[MA_RIGHT       ]->SetSpeed(m_fMoveSpeed);// * m_afMovingWeights[MA_RIGHT]);
	m_aobMovingAnims[MA_FORWARDLEFT ]->SetSpeed(m_fMoveSpeed);// * m_afMovingWeights[MA_FORWARDLEFT]);
	m_aobMovingAnims[MA_BACKLEFT    ]->SetSpeed(m_fMoveSpeed);// * m_afMovingWeights[MA_BACKLEFT]);
	m_aobMovingAnims[MA_FORWARDRIGHT]->SetSpeed(m_fMoveSpeed);// * m_afMovingWeights[MA_FORWARDRIGHT]);
	m_aobMovingAnims[MA_BACKRIGHT   ]->SetSpeed(m_fMoveSpeed);// * m_afMovingWeights[MA_BACKRIGHT]);

	// If we are moving or not capable of turning, we need to set a procedural turn
	const float fTurnLerp = clamp( ( 0.2f * ( fTimeStep / ( 1.0f / 30.0f ) ) ), 0.0f, 1.0f );
	obPredictedMovementState.m_fProceduralYaw = CMaths::Lerp( 0.0f, fRequiredTurn, fTurnLerp );

	// If we are not performing an animated turn we need to make sure no animated rotation is coming through
	obPredictedMovementState.m_fRootRotationDeltaScalar = 0.0f;

	// Gravity setting
	ApplyGravity(true);

	// And we're done!
	m_bFirstFrame = false;


	// Some debug info
	/*g_VisualDebug->Printf2D(50.0f,  50.0f, 0xffffffff, 0, "Velocity: %.2f \n", m_fMoveSpeed);
	g_VisualDebug->Printf2D(50.0f,  75.0f, 0xffffffff, 0, "    S: %.2f\n", m_aobMovingAnims[MA_STANDING    ]->GetBlendWeight());
	g_VisualDebug->Printf2D(50.0f, 100.0f, 0xffffffff, 0, "    F: %.2f\n", m_aobMovingAnims[MA_FORWARD     ]->GetBlendWeight());
	g_VisualDebug->Printf2D(50.0f, 125.0f, 0xffffffff, 0, "   FL: %.2f\n", m_aobMovingAnims[MA_FORWARDLEFT ]->GetBlendWeight());
	g_VisualDebug->Printf2D(50.0f, 150.0f, 0xffffffff, 0, "    L: %.2f\n", m_aobMovingAnims[MA_LEFT        ]->GetBlendWeight());
	g_VisualDebug->Printf2D(50.0f, 175.0f, 0xffffffff, 0, "   BL: %.2f\n", m_aobMovingAnims[MA_BACKLEFT    ]->GetBlendWeight());
	g_VisualDebug->Printf2D(50.0f, 200.0f, 0xffffffff, 0, "    B: %.2f\n", m_aobMovingAnims[MA_BACK        ]->GetBlendWeight());
	g_VisualDebug->Printf2D(50.0f, 225.0f, 0xffffffff, 0, "   BR: %.2f\n", m_aobMovingAnims[MA_BACKRIGHT   ]->GetBlendWeight());
	g_VisualDebug->Printf2D(50.0f, 250.0f, 0xffffffff, 0, "    R: %.2f\n", m_aobMovingAnims[MA_RIGHT       ]->GetBlendWeight());
	g_VisualDebug->Printf2D(50.0f, 275.0f, 0xffffffff, 0, "   FR: %.2f\n", m_aobMovingAnims[MA_FORWARDRIGHT]->GetBlendWeight());///*///


	// This controller never finishes - push it off
	return false;
}

//------------------------------------------------------------------------------------------
//!
//!	FormationStrafeController::CalculateTurnWeights
//!
//------------------------------------------------------------------------------------------
void FormationStrafeController::CalculateMoveWeights(const CMatrix&    obRootMatrix, 
													 const CDirection& obMoveDirection, 
													 float             fMoveSpeed)
{
	// Calculate the movement components
	float fFowardVelocity   = clamp(fMoveSpeed * obRootMatrix.GetZAxis().Dot(obMoveDirection), -1.0f, 1.0f);
	float fSidewaysVelocity = clamp(fMoveSpeed * obRootMatrix.GetXAxis().Dot(obMoveDirection), -1.0f, 1.0f);

	float fTheta = atan2f(fFowardVelocity, fSidewaysVelocity);
	
	if(fMoveSpeed <= EPSILON)
	{
		m_afDesiredMovingWeights[MA_STANDING]     = 1.f;
		m_afDesiredMovingWeights[MA_BACK]         = 0.f;
		m_afDesiredMovingWeights[MA_BACKLEFT]     = 0.f;
		m_afDesiredMovingWeights[MA_BACKRIGHT]    = 0.f;
		m_afDesiredMovingWeights[MA_FORWARD]      = 0.f;
		m_afDesiredMovingWeights[MA_FORWARDLEFT]  = 0.f;
		m_afDesiredMovingWeights[MA_FORWARDRIGHT] = 0.f;
		m_afDesiredMovingWeights[MA_RIGHT]        = 0.f;
		m_afDesiredMovingWeights[MA_LEFT]         = 0.f;
	}
	else if(fTheta < -3.f*PI/4.f) // Split into eight arcs
	{
		// [[R - BR]]
		float fRatio = (fTheta+PI) / QUARTER_PI;
		if(fRatio > 1.f || fRatio < 0.f)
			ntPrintf("Bad Ratio 1\n");

		m_afDesiredMovingWeights[MA_STANDING]     = 0.f;
		m_afDesiredMovingWeights[MA_BACK]         = 0.f;
		m_afDesiredMovingWeights[MA_BACKLEFT]     = 0.f;
		m_afDesiredMovingWeights[MA_BACKRIGHT]    = fRatio;
		m_afDesiredMovingWeights[MA_FORWARD]      = 0.f;
		m_afDesiredMovingWeights[MA_FORWARDLEFT]  = 0.f;
		m_afDesiredMovingWeights[MA_FORWARDRIGHT] = 0.f;
		m_afDesiredMovingWeights[MA_RIGHT]        = 1.f-fRatio;
		m_afDesiredMovingWeights[MA_LEFT]         = 0.f;
	}
	else if(fTheta < -HALF_PI)
	{
		// [[BR - B]]
		float fRatio = (fTheta+3.f*PI/4.f) / QUARTER_PI;
		if(fRatio > 1.f || fRatio < 0.f)
			ntPrintf("Bad Ratio 2\n");

		m_afDesiredMovingWeights[MA_STANDING]     = 0.f;
		m_afDesiredMovingWeights[MA_BACK]         = 0.f;//fRatio;
		m_afDesiredMovingWeights[MA_BACKLEFT]     = 0.f;
		m_afDesiredMovingWeights[MA_BACKRIGHT]    = 1.f;//1.f-fRatio;
		m_afDesiredMovingWeights[MA_FORWARD]      = 0.f;
		m_afDesiredMovingWeights[MA_FORWARDLEFT]  = 0.f;
		m_afDesiredMovingWeights[MA_FORWARDRIGHT] = 0.f;
		m_afDesiredMovingWeights[MA_RIGHT]        = 0.f;
		m_afDesiredMovingWeights[MA_LEFT]         = 0.f;
	}
	else if(fTheta < -QUARTER_PI)
	{
		// [[B - BL]]
		float fRatio = (fTheta+HALF_PI) / QUARTER_PI;
		if(fRatio > 1.f || fRatio < 0.f)
			ntPrintf("Bad Ratio 3\n");

		m_afDesiredMovingWeights[MA_STANDING]     = 0.f;
		m_afDesiredMovingWeights[MA_BACK]         = 0.f;//1.f-fRatio;
		m_afDesiredMovingWeights[MA_BACKLEFT]     = 1.f;//fRatio;
		m_afDesiredMovingWeights[MA_BACKRIGHT]    = 0.f;
		m_afDesiredMovingWeights[MA_FORWARD]      = 0.f;
		m_afDesiredMovingWeights[MA_FORWARDLEFT]  = 0.f;
		m_afDesiredMovingWeights[MA_FORWARDRIGHT] = 0.f;
		m_afDesiredMovingWeights[MA_RIGHT]        = 0.f;
		m_afDesiredMovingWeights[MA_LEFT]         = 0.f;
	}
	else if(fTheta < 0.f)
	{
		// [[BL-L]]
		float fRatio = fTheta / -QUARTER_PI;
		if(fRatio > 1.f || fRatio < 0.f)
			ntPrintf("Bad Ratio 4\n");

		m_afDesiredMovingWeights[MA_STANDING]     = 0.f;
		m_afDesiredMovingWeights[MA_BACK]         = 0.f;
		m_afDesiredMovingWeights[MA_BACKLEFT]     = 1.f-fRatio;
		m_afDesiredMovingWeights[MA_BACKRIGHT]    = 0.f;
		m_afDesiredMovingWeights[MA_FORWARD]      = 0.f;
		m_afDesiredMovingWeights[MA_FORWARDLEFT]  = 0.f;
		m_afDesiredMovingWeights[MA_FORWARDRIGHT] = 0.f;
		m_afDesiredMovingWeights[MA_RIGHT]        = 0.f;
		m_afDesiredMovingWeights[MA_LEFT]         = fRatio;
	}
	else if(fTheta < QUARTER_PI)
	{
		// [[L-FL]]
		float fRatio = fTheta / QUARTER_PI;
		if(fRatio > 1.f || fRatio < 0.f)
			ntPrintf("Bad Ratio 5\n");

		m_afDesiredMovingWeights[MA_STANDING]     = 0.f;
		m_afDesiredMovingWeights[MA_BACK]         = 0.f;
		m_afDesiredMovingWeights[MA_BACKLEFT]     = 0.f;
		m_afDesiredMovingWeights[MA_BACKRIGHT]    = 0.f;
		m_afDesiredMovingWeights[MA_FORWARD]      = 0.f;
		m_afDesiredMovingWeights[MA_FORWARDLEFT]  = fRatio;
		m_afDesiredMovingWeights[MA_FORWARDRIGHT] = 0.f;
		m_afDesiredMovingWeights[MA_RIGHT]        = 0.f;
		m_afDesiredMovingWeights[MA_LEFT]         = 1.f-fRatio;
	}
	else if(fTheta < HALF_PI)
	{
		// [[F - FL]]
		float fRatio = (fTheta-QUARTER_PI) / QUARTER_PI;
		if(fRatio > 1.f || fRatio < 0.f)
			ntPrintf("Bad Ratio 6\n");

		m_afDesiredMovingWeights[MA_STANDING]     = 0.f;
		m_afDesiredMovingWeights[MA_BACK]         = 0.f;
		m_afDesiredMovingWeights[MA_BACKLEFT]     = 0.f;
		m_afDesiredMovingWeights[MA_BACKRIGHT]    = 0.f;
		m_afDesiredMovingWeights[MA_FORWARD]      = 0.f;//fRatio;
		m_afDesiredMovingWeights[MA_FORWARDLEFT]  = 1.f;//1.f-fRatio;
		m_afDesiredMovingWeights[MA_FORWARDRIGHT] = 0.f;
		m_afDesiredMovingWeights[MA_RIGHT]        = 0.f;
		m_afDesiredMovingWeights[MA_LEFT]         = 0.f;
	}
	else if(fTheta < 3.f*PI/4.f)
	{
		// [[FR-F]]
		float fRatio = (fTheta-HALF_PI) / QUARTER_PI;
		if(fRatio > 1.f || fRatio < 0.f)
			ntPrintf("Bad Ratio 7\n");

		m_afDesiredMovingWeights[MA_STANDING]     = 0.f;
		m_afDesiredMovingWeights[MA_BACK]         = 0.f;
		m_afDesiredMovingWeights[MA_BACKLEFT]     = 0.f;
		m_afDesiredMovingWeights[MA_BACKRIGHT]    = 0.f;
		m_afDesiredMovingWeights[MA_FORWARD]      = 0.f;//1.f-fRatio;
		m_afDesiredMovingWeights[MA_FORWARDLEFT]  = 0.f;
		m_afDesiredMovingWeights[MA_FORWARDRIGHT] = 1.f;//fRatio;
		m_afDesiredMovingWeights[MA_RIGHT]        = 0.f;
		m_afDesiredMovingWeights[MA_LEFT]         = 0.f;
	}
	else
	{
		// [[R - FR]]
		float fRatio = clamp((fTheta-3.f*PI/4.f) / QUARTER_PI, 0.f, 1.f); // Sometimes goes to 1.f + EPSILON
		if(fRatio > 1.f || fRatio < 0.f)
			ntPrintf("Bad Ratio 8\n");

		m_afDesiredMovingWeights[MA_STANDING]     = 0.f;
		m_afDesiredMovingWeights[MA_BACK]         = 0.f;
		m_afDesiredMovingWeights[MA_BACKLEFT]     = 0.f;
		m_afDesiredMovingWeights[MA_BACKRIGHT]    = 0.f;
		m_afDesiredMovingWeights[MA_FORWARD]      = 0.f;
		m_afDesiredMovingWeights[MA_FORWARDLEFT]  = 0.f;
		m_afDesiredMovingWeights[MA_FORWARDRIGHT] = 1.f-fRatio;
		m_afDesiredMovingWeights[MA_RIGHT]        = fRatio;
		m_afDesiredMovingWeights[MA_LEFT]         = 0.f;
	}
}
