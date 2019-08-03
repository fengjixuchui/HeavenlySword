//------------------------------------------------------------------------------------------
//!
//!	\file catapultcontroller.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "catapultcontroller.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "movement.h"
#include "anim/animator.h"
#include "core/exportstruct_anim.h"
#include "objectdatabase/dataobject.h"
#include "camera/camutils.h"

#include "core/visualdebugger.h"
#include "core/OSDDisplay.h"


////////////////////////////
//------------------------------------------------------------------------------------------
//!
//!	VehicleControllerDef::VehicleControllerDef
//!	Construction
//!
//------------------------------------------------------------------------------------------
VehicleControllerDef::VehicleControllerDef( void )
:	m_fMaxRotationPerSecond( 90.0f * DEG_TO_RAD_VALUE )
,	m_bApplyGravity( true )
,	m_obMoveAnimName()
,	m_fMaxNormalPerSecond( 1.0f * DEG_TO_RAD_VALUE )
,	m_fMaxAcceleration ( 0.2f )
{
}


//------------------------------------------------------------------------------------------
//!
//!	VehicleControllerDef::CreateInstance
//!	To create an instance of what we describe
//!
//------------------------------------------------------------------------------------------
MovementController* VehicleControllerDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) VehicleController( pobMovement, *this );
}


//------------------------------------------------------------------------------------------
//!
//!	VehicleControllerDef::Clone
//!	Clone me
//!
//------------------------------------------------------------------------------------------
MovementControllerDef* VehicleControllerDef::Clone( void ) const 
{ 
	return NT_NEW_CHUNK(Mem::MC_ENTITY) VehicleControllerDef( *this ); 
}


//------------------------------------------------------------------------------------------
//!
//!	VehicleController::VehicleController
//!	Construction
//!
//------------------------------------------------------------------------------------------
VehicleController::VehicleController( CMovement* pobMovement, const VehicleControllerDef& obDefinition )
:	MovementController( pobMovement )
,	m_obDefinition( obDefinition )
,	m_obMoveAnimation()
,	m_obLastHeading( CONSTRUCT_CLEAR )
,	m_fLastSpeed( 0.0f )
,	m_obLastRot( CONSTRUCT_IDENTITY )
,	m_obAimRot( CONSTRUCT_IDENTITY )
,	m_obLastNormal ( CONSTRUCT_CLEAR )
,	m_obAimNormal ( CONSTRUCT_CLEAR )
{
	// Register our definition for debugging purposes
	InternalRegisterDefinition( m_obDefinition );

	// Create our animation and add it to the animator
	// Move animation is full
	m_obMoveAnimation = m_pobAnimator->CreateAnimation( obDefinition.m_obMoveAnimName );
	m_obMoveAnimation->SetBlendWeight( m_fBlendWeight );
	m_obMoveAnimation->SetFlagBits( ANIMF_LOCOMOTING | ANIMF_INHIBIT_AUTO_DESTRUCT | ANIMF_LOOPING );

	m_obLastHeading = obDefinition.m_pobHost->GetMatrix().GetZAxis();
	m_obLastNormal = obDefinition.m_pobHost->GetMatrix().GetYAxis();
}


//------------------------------------------------------------------------------------------
//!
//!	VehicleController::~VehicleController
//!	Destruction
//!
//------------------------------------------------------------------------------------------
VehicleController::~VehicleController( void )
{
	// Remove the animation from the animator if it has been added
	if ( !m_bFirstFrame && m_obMoveAnimation->IsActive() )
		m_pobAnimator->RemoveAnimation( m_obMoveAnimation );
}


//------------------------------------------------------------------------------------------
//!
//!	VehicleController::Update
//!
//------------------------------------------------------------------------------------------
bool VehicleController::Update(	float						fTimeStep, 
										const CMovementInput&		obMovementInput,
										const CMovementStateRef&	obCurrentMovementState,
										CMovementState&				obPredictedMovementState )
{
	UNUSED (obCurrentMovementState);

	if (m_bFirstFrame)
	{
		// Move animation is on all the time
		m_pobAnimator->AddAnimation( m_obMoveAnimation );			

		m_bFirstFrame = false;
	}

	// Change in speed needed
	float fSpeedDelta = obMovementInput.m_fMoveSpeed - m_fLastSpeed;

	// Change in speed we can do
	if ( fabs(fSpeedDelta) > m_obDefinition.m_fMaxAcceleration * fTimeStep )
	{
		if (fSpeedDelta < 0.0f)
			fSpeedDelta = -m_obDefinition.m_fMaxAcceleration * fTimeStep;
		else
			fSpeedDelta =  m_obDefinition.m_fMaxAcceleration * fTimeStep;
	}
	float fIdealSpeed = m_fLastSpeed + fSpeedDelta;

	m_obMoveAnimation->SetBlendWeight( m_fBlendWeight );
	m_obMoveAnimation->SetSpeed( fIdealSpeed );


	CDirection obRequiredHeading = obMovementInput.m_obMoveDirection;

	// Change in heading we need to do
	float fProceduralYaw = MovementControllerUtilities::RotationAboutY( m_obLastHeading, obRequiredHeading );
	
	// Change in heading we can do
	if ( fabs(fProceduralYaw) > m_obDefinition.m_fMaxRotationPerSecond * fTimeStep )
	{
		if (fProceduralYaw < 0.0f)
			fProceduralYaw = -m_obDefinition.m_fMaxRotationPerSecond * fTimeStep;
		else
			fProceduralYaw =  m_obDefinition.m_fMaxRotationPerSecond * fTimeStep;
	}

	// Heading this frame
	CDirection obIdealHeading = m_obLastHeading * CMatrix(CQuat( CVecMath::GetYAxis(), fProceduralYaw ));

	// I had tried to slerp between last rotation and next rotation quaternion over m_fCurrentSlerpTime
	// But this did not take path following heading into account.  Adding this would make the slerping extreemly
	// messy IMHO. Instead I'm attempting to lerp the normal and heading vectors then construct an appropriate 
	// local matirx from those. T McK

	// Normal we need
	CDirection obRequiredNormal = obMovementInput.m_obFacingDirection;

	// Change in normal to get there
	CQuat obRotationDelta( m_obLastNormal, obRequiredNormal );
	
	CDirection obAxis; 
	float fAngle;
	obRotationDelta.GetAxisAndAngle( obAxis, fAngle );

	// Change in normal we can do this frame
	if ( fabs(fAngle) > m_obDefinition.m_fMaxNormalPerSecond * fTimeStep )
	{
		if (fAngle < 0.0f)
			fAngle = -m_obDefinition.m_fMaxNormalPerSecond * fTimeStep;
		else
			fAngle =  m_obDefinition.m_fMaxNormalPerSecond * fTimeStep;
	}

	// Normal for this frame
	CDirection obIdealNormal = m_obLastNormal * CMatrix(CQuat( obAxis, fAngle ));

	CDirection obPerp = obIdealHeading.Cross( CDirection(0.0f,1.0f,0.0f) );
	CDirection obRealHeading = obIdealNormal.Cross ( obPerp );

	CDirection obIdealPerp = obRealHeading.Cross( obIdealNormal );

	CMatrix obMat;
	obMat.SetXAxis(obIdealPerp);
	obMat.SetYAxis(obIdealNormal);
	obMat.SetZAxis(obRealHeading);

	float ax,ay,az;	
	CCamUtil::EulerFromMat_XYZ(obMat,ax,ay,az);			

	obPredictedMovementState.m_fProceduralPitch=ax;
	obPredictedMovementState.m_fProceduralYaw=ay;
	obPredictedMovementState.m_fProceduralRoll=az;
	obPredictedMovementState.m_bApplyExplicitRotations = true;

	// Change in height we can do
	float fProceduralHeight = obMovementInput.m_obMoveDirectionAlt.Y();

	if ( fabs(fProceduralHeight) > m_obDefinition.m_fMaxHeightPerSecond * fTimeStep )
	{
		if (fProceduralHeight < 0.0f)
			fProceduralHeight = -m_obDefinition.m_fMaxHeightPerSecond * fTimeStep;
		else
			fProceduralHeight =  m_obDefinition.m_fMaxHeightPerSecond * fTimeStep;
	}

	// Correct the height
	obPredictedMovementState.m_obProceduralRootDelta.Y() += fProceduralHeight;

#ifdef _DEBUG
	g_VisualDebug->RenderLine(obCurrentMovementState.m_obPosition, obCurrentMovementState.m_obPosition + obIdealPerp*20.0f, DC_RED);
	g_VisualDebug->RenderLine(obCurrentMovementState.m_obPosition, obCurrentMovementState.m_obPosition + obIdealNormal*20.0f, DC_GREEN);
	g_VisualDebug->RenderLine(obCurrentMovementState.m_obPosition, obCurrentMovementState.m_obPosition + obRealHeading*20.0f, DC_BLUE);

	g_VisualDebug->RenderLine(obCurrentMovementState.m_obPosition, obCurrentMovementState.m_obPosition + obRequiredNormal*20.0f, DC_YELLOW);
	g_VisualDebug->RenderLine(obCurrentMovementState.m_obPosition, obCurrentMovementState.m_obPosition + obRequiredHeading*20.0f, DC_YELLOW);
	//g_VisualDebug->RenderLine(obCurrentMovementState.m_obPosition, obCurrentMovementState.m_obPosition + obAxis*20.0f, DC_WHITE);
#endif // _DEBUG

	m_obLastHeading = obIdealHeading;
	m_obLastNormal = obIdealNormal;
	m_fLastSpeed = fIdealSpeed;

	return false;
}
/////////////////////////

//------------------------------------------------------------------------------------------
//!
//!	CatapultControllerDef::CatapultControllerDef
//!	Construction
//!
//------------------------------------------------------------------------------------------
CatapultControllerDef::CatapultControllerDef( void )
:	VehicleControllerDef()
{
}


//------------------------------------------------------------------------------------------
//!
//!	CatapultControllerDef::CreateInstance
//!	To create an instance of what we describe
//!
//------------------------------------------------------------------------------------------
MovementController* CatapultControllerDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) CatapultController( pobMovement, *this );
}


//------------------------------------------------------------------------------------------
//!
//!	CatapultControllerDef::Clone
//!	Clone me
//!
//------------------------------------------------------------------------------------------
MovementControllerDef* CatapultControllerDef::Clone( void ) const 
{ 
	return NT_NEW_CHUNK(Mem::MC_ENTITY) CatapultControllerDef( *this ); 
}


//------------------------------------------------------------------------------------------
//!
//!	CatapultController::CatapultController
//!	Construction
//!
//------------------------------------------------------------------------------------------
CatapultController::CatapultController( CMovement* pobMovement, const CatapultControllerDef& obDefinition )
:	VehicleController( pobMovement, obDefinition )
,	m_obCatapultDefinition( obDefinition )
{
	// Partial Fire animations
	for (HashIter obIt = m_obCatapultDefinition.m_obFireAnimNameList.begin(); obIt != m_obCatapultDefinition.m_obFireAnimNameList.end(); ++obIt )
	{
		CAnimationPtr pobAnimation;
		pobAnimation = m_pobAnimator->CreateAnimation( *obIt );
		pobAnimation->SetBlendWeight( m_fBlendWeight );
		pobAnimation->SetFlagBits( ANIMF_INHIBIT_AUTO_DESTRUCT );
		m_aobFireAnimList.push_back( pobAnimation );
	}

	// Partial Reset animations
	for (HashIter obIt = m_obCatapultDefinition.m_obResetAnimNameList.begin(); obIt != m_obCatapultDefinition.m_obResetAnimNameList.end(); ++obIt )
	{
		CAnimationPtr pobAnimation;
		pobAnimation = m_pobAnimator->CreateAnimation( *obIt );
		pobAnimation->SetBlendWeight( m_fBlendWeight );
		pobAnimation->SetFlagBits( ANIMF_INHIBIT_AUTO_DESTRUCT );
		m_aobResetAnimList.push_back( pobAnimation );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	CatapultController::~CatapultController
//!	Destruction
//!
//------------------------------------------------------------------------------------------
CatapultController::~CatapultController( void )
{
	// Remove the animation from the animator if it has been added
	if ( !m_bFirstFrame )
	{
		// Partial Fire animations
		for (AnimIter obIt = m_aobFireAnimList.begin(); obIt != m_aobFireAnimList.end(); ++obIt )
		{
			if ( (*obIt)->IsActive() )
				m_pobAnimator->RemoveAnimation( (*obIt) );
		}

		// Partial Reset animations
		for (AnimIter obIt = m_aobResetAnimList.begin(); obIt != m_aobResetAnimList.end(); ++obIt )
		{
			if ( (*obIt)->IsActive() )
				m_pobAnimator->RemoveAnimation( (*obIt) );
		}
	}
}


//------------------------------------------------------------------------------------------
//!
//!	CatapultController::Update
//!
//------------------------------------------------------------------------------------------
bool CatapultController::Update(	float						fTimeStep, 
										const CMovementInput&		obMovementInput,
										const CMovementStateRef&	obCurrentMovementState,
										CMovementState&				obPredictedMovementState )
{
	// Update the base class
	VehicleController::Update(fTimeStep, obMovementInput, obCurrentMovementState, obPredictedMovementState);

	// Update Partial animations
	u_int iPos = 0;
	for ( StateIter stateIt = m_obCatapultDefinition.m_pobStateList->begin(); stateIt != m_obCatapultDefinition.m_pobStateList->end(); ++stateIt )
	{
		ntAssert (iPos <= m_obCatapultDefinition.m_pobStateList->size() );
		switch ( *stateIt )
		{
		case AS_FIRE:
			m_pobAnimator->AddAnimation( m_aobFireAnimList[iPos] );
			if ( m_aobResetAnimList[iPos]->IsActive() )
				m_pobAnimator->RemoveAnimation( m_aobResetAnimList[iPos] );
			( *stateIt ) = AS_FIRING;
			// Fall through

		case AS_FIRING:
			m_aobFireAnimList[iPos]->SetBlendWeight( m_fBlendWeight );
			m_aobFireAnimList[iPos]->SetSpeed( 1.0f );

			if ( m_aobFireAnimList[iPos]->GetTime() >= m_aobFireAnimList[iPos]->GetDuration() )
			{
				( *stateIt ) = AS_STOP;
			}
			break;

		case AS_STOP:
			m_aobFireAnimList[iPos]->SetBlendWeight( m_fBlendWeight );
			break;

		case AS_RESET:
			m_pobAnimator->AddAnimation( m_aobResetAnimList[iPos] );
			if ( m_aobFireAnimList[iPos]->IsActive() )
				m_pobAnimator->RemoveAnimation( m_aobFireAnimList[iPos] );
			( *stateIt ) = AS_RESETING;
			// Fall through

		case AS_RESETING:
			m_aobResetAnimList[iPos]->SetBlendWeight( m_fBlendWeight );
			m_aobResetAnimList[iPos]->SetSpeed( 1.0f );

			if ( m_aobResetAnimList[iPos]->GetTime() >= m_aobResetAnimList[iPos]->GetDuration() )
			{
				( *stateIt ) = AS_EMPTY;
				if ( m_aobResetAnimList[iPos]->IsActive() )
					m_pobAnimator->RemoveAnimation( m_aobResetAnimList[iPos] );
			}
			break;

		case AS_READY:
		case AS_EMPTY:
			m_aobResetAnimList[iPos]->SetBlendWeight( m_fBlendWeight );
			m_aobResetAnimList[iPos]->SetSpeed( 1.0f );
			break;

		default:
			ntAssert(0);
			break;
		} // switch ( *StateIter )

		++iPos;
	}

	return false;
}
