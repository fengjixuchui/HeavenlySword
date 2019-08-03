//------------------------------------------------------------------------------------------
//!
//!	\file turretcontroller.cpp
//!
//------------------------------------------------------------------------------------------


/////////////////////////////////
// Required Includes
/////////////////////////////////
#include "game/movement.h"
#include "game/turretcontroller.h"
#include "core/exportstruct_anim.h"
#include "anim/animator.h"
#include "anim/hierarchy.h"
#include "camera/camutils.h"
#include "camera/camman.h"
#include "camera/camview.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "objectdatabase/dataobject.h"
#include "game/messagehandler.h"
#include "Physics/config.h"
#include "Physics/collisionbitfield.h"
#include "game/randmanager.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include "Physics/advancedcharactercontroller.h"
#endif

#include "Physics/system.h"
#include "Physics/world.h"

#include "camera/coolcam_aim.h"

#include "game/aicomponent.h"
#include "ai/ainavigationsystem/aimovement.h"
#include "game/entityinteractableturretweapon.h"

#include "game/playeroptions.h"

#include "core/visualdebugger.h" 


START_STD_INTERFACE(TurretControllerDef)
	PUBLISH_CONTAINER_AS		(m_aobYawTransformName,				YawTransformName)
	PUBLISH_CONTAINER_AS		(m_aobPitchTransformName,			PitchTransformName)
	PUBLISH_CONTAINER_AS		(m_aobCombinedTransformName,		CombinedTransformName)
	PUBLISH_VAR_AS	(m_obIdleAnimation,					IdleAnimation)

	PUBLISH_VAR_AS	(m_obRecoilUpAnimation,				RecoilUpAnimation)
	PUBLISH_VAR_AS	(m_obRecoilMidAnimation,				RecoilMidAnimation)
	PUBLISH_VAR_AS	(m_obRecoilDownAnimation,				RecoilDownAnimation)

	PUBLISH_VAR_AS	(m_obPullLeftUpAnimation,			PullLeftUpAnimation)
	PUBLISH_VAR_AS	(m_obPullLeftMidAnimation,			PullLeftMidAnimation)
	PUBLISH_VAR_AS	(m_obPullLeftDownAnimation,			PullLeftDownAnimation)
	
	PUBLISH_VAR_AS	(m_obPullRightUpAnimation,			PullRightUpAnimation)	
	PUBLISH_VAR_AS	(m_obPullRightMidAnimation,			PullRightMidAnimation)	
	PUBLISH_VAR_AS	(m_obPullRightDownAnimation,		PullRightDownAnimation)	

	PUBLISH_VAR_AS	(m_obPullCentreUpAnimation,			PullCentreUpAnimation)	
	PUBLISH_VAR_AS	(m_obPullCentreDownAnimation,		PullCentreDownAnimation)


	PUBLISH_VAR_AS				(m_ptTranslationOffset,				TranslationOffset)
	PUBLISH_VAR_AS				(m_fMaxUpPitch,						MaxUpPitch)
	PUBLISH_VAR_AS				(m_fMaxDownPitch,					MaxDownPitch)
	PUBLISH_VAR_AS				(m_fYawMinSpeed,					YawMinSpeed)
	PUBLISH_VAR_AS				(m_fYawMaxSpeed,					YawMaxSpeed)
	PUBLISH_VAR_AS				(m_fYawAcceleration,				YawAcceleration)
	PUBLISH_VAR_AS				(m_fPitchMinSpeed,					PitchMinSpeed)
	PUBLISH_VAR_AS				(m_fPitchMaxSpeed,					PitchMaxSpeed)
	PUBLISH_VAR_AS				(m_fPitchAcceleration,				PitchAcceleration)
	PUBLISH_VAR_AS				(m_fSoftLockSpeedModifier,			SoftLockSpeedModifier)
	PUBLISH_VAR_AS				(m_fPadAccelerationThreshold,		PadAccelerationThreshold)
	PUBLISH_VAR_AS				(m_fPadThreshold,					PadThreshold)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_fRecoilAngle,		0.0f,		RecoilAngle)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_fRecoilTime,			0.0f,		RecoilTime)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_fSettleAngle,		0.0f,		SettleAngle)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_fSettleTime,			0.0f,		SettleTime)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_fDampedTime,			0.0f,		DampedTime)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_fDampedAngle,		0.0f,		DampedAngle)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_fDampedFrequency,	0.0f,		DampedFrequency)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_fMaxAimLossAngle,	0.0f,		MaxAimLossAngle)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_fUserLeanDelta,		0.0f,		UserLeanDelta)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_fUserRecoilTime,		0.0f,		UserRecoilTime)
END_STD_INTERFACE

//------------------------------------------------------------------------------------------
//!
//!	TurretControllerDef::TurretControllerDef
//!	Construction of a definition for a turret controller.
//!
//------------------------------------------------------------------------------------------
TurretControllerDef::TurretControllerDef() 
:	m_fMaxYaw ( 0 )
,	m_fYawMinSpeed(90.0f)
,	m_fYawMaxSpeed(180.0f)
,	m_fYawAcceleration(45.0f)
,	m_fPitchMinSpeed(45.0f)
,	m_fPitchMaxSpeed(90.0f)
,	m_fPitchAcceleration(45.0f)
,	m_fSoftLockSpeedModifier(1.0f)
,	m_fPadThreshold(0.05f)
,	m_fPadAccelerationThreshold(0.99f)
,	m_bAIControlled(false)
,	m_fRecoilAngle(0.0f)
,	m_fRecoilTime(0.0f)
,	m_fMaxAimLossAngle(0.0f)
,	m_fSettleAngle(0.0f)
,	m_fSettleTime(0.0f)
,	m_fDampedTime(0.0f)
,	m_fDampedAngle(0.0f)
,	m_fDampedFrequency(0.0f)
{
	m_pVehicle = 0;
}


//------------------------------------------------------------------------------------------
//!
//!	TurretControllerDef::CreateInstance
//!	Create a turret controller from a definition.
//!
//------------------------------------------------------------------------------------------
MovementController* TurretControllerDef::CreateInstance(CMovement* pMovement) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) TurretController(pMovement, *this);
}

//------------------------------------------------------------------------------------------
//!
//!	TurretController::TurretController
//!	Construction.
//!
//------------------------------------------------------------------------------------------
TurretController::TurretController(CMovement* pMovement, const TurretControllerDef& rDef) :
	MovementController(pMovement),
	m_obDefinition( rDef ),
	m_fYawSpeed(0.0f),
	m_fPitchSpeed(0.0f),
	m_fYawDelta(0.0f),
	m_fPitchDelta(0.0f),
	m_fCurrRecoilTime(0.0f),
	m_fCurrRecoilAngle ( 0.0f ),
	m_fCurrDampAngle(0.0f),
	m_fAimLossAngle( 0.0f ),
	m_fCurrAimLossAngle(0.0f),
	m_fAcheivedRecoilAngle(0.0f),
	m_fUserLean(0.0f),
	m_fCurrUserRecoilTime(0.0f)
{
	ntAssert(pMovement);

	m_pVehicle			= (Interactable_TurretWeapon*)rDef.m_pVehicle;
	m_pDrivingSeat		= m_pVehicle->GetHierarchy()->GetTransform(rDef.m_sDrivingSeat);

	// Register ourself so procedural changes can be done post animation
	m_pVehicle->RegisterTurretController( this );

	// Remember turret root so we can re-apply it after the animator has nuked it
	m_pRoot = m_pVehicle->GetHierarchy()->GetRootTransform();
	m_obInitalRoot = m_pRoot->GetLocalMatrix();

	// Yaw transforms
	for ( ntstd::List<CHashedString>::const_iterator obIt = rDef.m_aobYawTransformName.begin();
			obIt != rDef.m_aobYawTransformName.end(); obIt++ )
	{
		if ( m_pVehicle->GetHierarchy()->DoesTransformExist( *obIt ) )
			m_apYawTransform.push_back( m_pVehicle->GetHierarchy()->GetTransform( *obIt ) );
	}

	// Pitch transforms
	for ( ntstd::List<CHashedString>::const_iterator obIt = rDef.m_aobPitchTransformName.begin();
		obIt != rDef.m_aobPitchTransformName.end(); obIt++ )
	{
		if ( m_pVehicle->GetHierarchy()->DoesTransformExist( *obIt ) )
			m_apPitchTransform.push_back( m_pVehicle->GetHierarchy()->GetTransform( *obIt ) );
	}

	// Combined transforms
	for ( ntstd::List<CHashedString>::const_iterator obIt = rDef.m_aobCombinedTransformName.begin();
		obIt != rDef.m_aobCombinedTransformName.end(); obIt++ )
	{
		if ( m_pVehicle->GetHierarchy()->DoesTransformExist( *obIt ) )
			m_apCombinedTransform.push_back( m_pVehicle->GetHierarchy()->GetTransform( *obIt ) );
	}

	m_fPitch=0.0f; // Reset the pitch angle

	// Figure out the current yaw angle
	if ( m_apYawTransform.size() > 0 )
	{
		const CDirection& obZAxis = m_apYawTransform.front()->GetLocalMatrix().GetZAxis();
		m_fYaw = fatan2f(obZAxis.X(), obZAxis.Z()); // Get the Y angle of the turret support
	}
	else if ( m_apCombinedTransform.size() > 0 )
	{
		const CDirection& obZAxis = m_apCombinedTransform.front()->GetLocalMatrix().GetZAxis();
		m_fYaw = fatan2f(obZAxis.X(), obZAxis.Z()); // Get the Y angle of the turret support
	}
	else
	{
		ntError_p(0, ("TurretController %s without transform defined for yaw\n", ntStr::GetString( rDef.GetInstanceName() )));
	}

	// For AI Controlled Cannons we need to know the initial Yaw and Pitch of the Turret
	CDirection CannonHeading = m_pDrivingSeat->GetWorldMatrix().GetZAxis();
	CDirection CannonSideDir = m_pDrivingSeat->GetWorldMatrix().GetXAxis();
	CDirection CannonPitchDir= m_pDrivingSeat->GetWorldMatrix().GetYAxis();
	float fSafeYDir		=	CannonPitchDir.Y() > 0.999f		? 1.0f : 
							CannonPitchDir.Y() < -0.999f	? -1.0f : CannonPitchDir.Y();

	float fSafeZDir		=	CannonHeading.Z() > 0.999f	? 1.0f :
							CannonHeading.Z() < -0.999f	? -1.0f : CannonHeading.Z();
	m_fInitialTurretYaw		= (CannonSideDir.X() < 0.0f)	? -facosf(fSafeZDir) : facosf(fSafeZDir);
	m_fInitialTurretPitch	= (CannonPitchDir.Z() < 0.0f)	? -facosf(fSafeYDir) : facosf(fSafeYDir);

	// Player animations
	if (!m_obDefinition.m_obIdleAnimation.IsNull())
	{
		m_pobIdleAnim = m_pobAnimator->CreateAnimation( m_obDefinition.m_obIdleAnimation );
		m_pobIdleAnim->SetBlendWeight( 0.0f );
		m_pobIdleAnim->SetFlagBits( ANIMF_LOOPING | ANIMF_INHIBIT_AUTO_DESTRUCT );
		m_pobAnimator->AddAnimation( m_pobIdleAnim );
	}

	if (!m_obDefinition.m_obRecoilUpAnimation.IsNull())
	{
		m_pobRecoilUpAnim = m_pobAnimator->CreateAnimation( m_obDefinition.m_obRecoilUpAnimation );
		m_pobRecoilUpAnim->SetBlendWeight( 0.0f );
		m_pobRecoilUpAnim->SetFlagBits( ANIMF_INHIBIT_AUTO_DESTRUCT );
		m_pobAnimator->AddAnimation( m_pobRecoilUpAnim );
	}

	if (!m_obDefinition.m_obRecoilMidAnimation.IsNull())
	{
		m_pobRecoilMidAnim = m_pobAnimator->CreateAnimation( m_obDefinition.m_obRecoilMidAnimation );
		m_pobRecoilMidAnim->SetBlendWeight( 0.0f );
		m_pobRecoilMidAnim->SetFlagBits( ANIMF_INHIBIT_AUTO_DESTRUCT );
		m_pobAnimator->AddAnimation( m_pobRecoilMidAnim );
	}

	if (!m_obDefinition.m_obRecoilDownAnimation.IsNull())
	{
		m_pobRecoilDownAnim = m_pobAnimator->CreateAnimation( m_obDefinition.m_obRecoilDownAnimation );
		m_pobRecoilDownAnim->SetBlendWeight( 0.0f );
		m_pobRecoilDownAnim->SetFlagBits( ANIMF_INHIBIT_AUTO_DESTRUCT );
		m_pobAnimator->AddAnimation( m_pobRecoilDownAnim );
	}

	if (!m_obDefinition.m_obPullLeftUpAnimation.IsNull())
	{
		m_pobPullLeftUpAnim = m_pobAnimator->CreateAnimation( m_obDefinition.m_obPullLeftUpAnimation );
		m_pobPullLeftUpAnim->SetBlendWeight( 0.0f );
		m_pobPullLeftUpAnim->SetFlagBits( ANIMF_LOOPING | ANIMF_INHIBIT_AUTO_DESTRUCT );
		m_pobAnimator->AddAnimation( m_pobPullLeftUpAnim );
	}

	if (!m_obDefinition.m_obPullLeftMidAnimation.IsNull())
	{
		m_pobPullLeftMidAnim = m_pobAnimator->CreateAnimation( m_obDefinition.m_obPullLeftMidAnimation );
		m_pobPullLeftMidAnim->SetBlendWeight( 0.0f );
		m_pobPullLeftMidAnim->SetFlagBits( ANIMF_LOOPING | ANIMF_INHIBIT_AUTO_DESTRUCT );
		m_pobAnimator->AddAnimation( m_pobPullLeftMidAnim );
	}

	if (!m_obDefinition.m_obPullLeftDownAnimation.IsNull())
	{
		m_pobPullLeftDownAnim = m_pobAnimator->CreateAnimation( m_obDefinition.m_obPullLeftDownAnimation );
		m_pobPullLeftDownAnim->SetBlendWeight( 0.0f );
		m_pobPullLeftDownAnim->SetFlagBits( ANIMF_LOOPING | ANIMF_INHIBIT_AUTO_DESTRUCT );
		m_pobAnimator->AddAnimation( m_pobPullLeftDownAnim );
	}

	if (!m_obDefinition.m_obPullRightUpAnimation.IsNull())
	{
		m_pobPullRightUpAnim = m_pobAnimator->CreateAnimation( m_obDefinition.m_obPullRightUpAnimation );
		m_pobPullRightUpAnim->SetBlendWeight( 0.0f );
		m_pobPullRightUpAnim->SetFlagBits( ANIMF_LOOPING | ANIMF_INHIBIT_AUTO_DESTRUCT );
		m_pobAnimator->AddAnimation( m_pobPullRightUpAnim );
	}

	if (!m_obDefinition.m_obPullRightMidAnimation.IsNull())
	{
		m_pobPullRightMidAnim = m_pobAnimator->CreateAnimation( m_obDefinition.m_obPullRightMidAnimation );
		m_pobPullRightMidAnim->SetBlendWeight( 0.0f );
		m_pobPullRightMidAnim->SetFlagBits( ANIMF_LOOPING | ANIMF_INHIBIT_AUTO_DESTRUCT );
		m_pobAnimator->AddAnimation( m_pobPullRightMidAnim );
	}

	if (!m_obDefinition.m_obPullRightDownAnimation.IsNull())
	{
		m_pobPullRightDownAnim = m_pobAnimator->CreateAnimation( m_obDefinition.m_obPullRightDownAnimation );
		m_pobPullRightDownAnim->SetBlendWeight( 0.0f );
		m_pobPullRightDownAnim->SetFlagBits( ANIMF_LOOPING | ANIMF_INHIBIT_AUTO_DESTRUCT );
		m_pobAnimator->AddAnimation( m_pobPullRightDownAnim );
	}

	if (!m_obDefinition.m_obPullCentreUpAnimation.IsNull())
	{
		m_pobPullCentreUpAnim = m_pobAnimator->CreateAnimation( m_obDefinition.m_obPullCentreUpAnimation );
		m_pobPullCentreUpAnim->SetBlendWeight( 0.0f );
		m_pobPullCentreUpAnim->SetFlagBits( ANIMF_LOOPING | ANIMF_INHIBIT_AUTO_DESTRUCT );
		m_pobAnimator->AddAnimation( m_pobPullCentreUpAnim );
	}

	if (!m_obDefinition.m_obPullCentreDownAnimation.IsNull())
	{
		m_pobPullCentreDownAnim = m_pobAnimator->CreateAnimation( m_obDefinition.m_obPullCentreDownAnimation );
		m_pobPullCentreDownAnim->SetBlendWeight( 0.0f );
		m_pobPullCentreDownAnim->SetFlagBits( ANIMF_LOOPING | ANIMF_INHIBIT_AUTO_DESTRUCT );
		m_pobAnimator->AddAnimation( m_pobPullCentreDownAnim );
	}	
}

//------------------------------------------------------------------------------------------
//!
//!	TurretController::~TurretController
//!	Destruction.
//!
//------------------------------------------------------------------------------------------
TurretController::~TurretController()
{
	// About to go out of scope, deregister with our object
	m_pVehicle->RegisterTurretController( 0 );

	// Remove the animations from the animator if it has been added
	if ( !m_bFirstFrame )
	{
		if ( !m_obDefinition.m_obIdleAnimation.IsNull() && m_pobIdleAnim->IsActive() )
			m_pobAnimator->RemoveAnimation( m_pobIdleAnim );

		if ( !m_obDefinition.m_obRecoilUpAnimation.IsNull() && m_pobRecoilUpAnim->IsActive() )
			m_pobAnimator->RemoveAnimation( m_pobRecoilUpAnim );

		if ( !m_obDefinition.m_obRecoilMidAnimation.IsNull() && m_pobRecoilMidAnim->IsActive() )
			m_pobAnimator->RemoveAnimation( m_pobRecoilMidAnim );

		if ( !m_obDefinition.m_obRecoilDownAnimation.IsNull() && m_pobRecoilDownAnim->IsActive() )
			m_pobAnimator->RemoveAnimation( m_pobRecoilDownAnim );

		if ( !m_obDefinition.m_obPullLeftUpAnimation.IsNull() && m_pobPullLeftUpAnim->IsActive() )
			m_pobAnimator->RemoveAnimation( m_pobPullLeftUpAnim );

		if ( !m_obDefinition.m_obPullLeftMidAnimation.IsNull() && m_pobPullLeftMidAnim->IsActive() )
			m_pobAnimator->RemoveAnimation( m_pobPullLeftMidAnim );

		if ( !m_obDefinition.m_obPullLeftDownAnimation.IsNull() && m_pobPullLeftDownAnim->IsActive() )
			m_pobAnimator->RemoveAnimation( m_pobPullLeftDownAnim );

		if ( !m_obDefinition.m_obPullRightUpAnimation.IsNull() && m_pobPullRightUpAnim->IsActive() )
			m_pobAnimator->RemoveAnimation( m_pobPullRightUpAnim );

		if ( !m_obDefinition.m_obPullRightMidAnimation.IsNull() && m_pobPullRightMidAnim->IsActive() )
			m_pobAnimator->RemoveAnimation( m_pobPullRightMidAnim );

		if ( !m_obDefinition.m_obPullRightDownAnimation.IsNull() && m_pobPullRightDownAnim->IsActive() )
			m_pobAnimator->RemoveAnimation( m_pobPullRightDownAnim );

		if ( !m_obDefinition.m_obPullCentreUpAnimation.IsNull() && m_pobPullCentreUpAnim->IsActive() )
			m_pobAnimator->RemoveAnimation( m_pobPullCentreUpAnim );

		if ( !m_obDefinition.m_obPullCentreDownAnimation.IsNull() && m_pobPullCentreDownAnim->IsActive() )
			m_pobAnimator->RemoveAnimation( m_pobPullCentreDownAnim );
	}
}

//------------------------------------------------------------------------------------------
//!
//!	TurretController::Update
//!
//!	Rotate the turret as requested.
//! This would of course be a easier if I could just get the angle of the pad input here!
//!
//!	N.B.	Having to use this bAIControlled flag is a bit sucky, but unfortunately necessary.
//!			This is because the movement controller def is owned by the vehicle and not the
//!			character using it, and the standard human player controller is dependent on the
//!			camera.
//------------------------------------------------------------------------------------------

bool TurretController::Update(float fTimeDelta, const CMovementInput& input, 
							  const CMovementStateRef& currentState, CMovementState& predictedState)
{
	bool bReturn = m_obDefinition.m_bAIControlled	? AIUpdate( fTimeDelta, input, currentState, predictedState ) 
													: PlayerUpdate( fTimeDelta, input, currentState, predictedState );

	if( m_bFirstFrame )
	{
		const CMatrix& obTargetWorldMatrix	= m_obDefinition.m_pVehicle->GetMatrix();

		// Calculate the translation delta
		CDirection	dirTranslationOffset	= CDirection(m_obDefinition.m_ptTranslationOffset) * obTargetWorldMatrix;
		CPoint		ptNewPosition			= obTargetWorldMatrix.GetTranslation() + dirTranslationOffset;

		predictedState.m_obProceduralRootDelta += ptNewPosition ^ currentState.m_obPosition;

		// No longer the first frame
		m_bFirstFrame = false;
	}

	return bReturn;
}

bool TurretController::PlayerUpdate(float fTimeDelta, const CMovementInput& input, 
							  const CMovementStateRef& currentState, CMovementState& predictedState)
{
	const CMatrix& obCameraMatrix=CamMan::Get().GetPrimaryView()->GetCurrMatrix();

	// ---- Softlock -----

	float fSoftLockSpeedMultiplier=1.0f;
	
	if (m_obDefinition.m_fSoftLockSpeedModifier > 0.0f && m_obDefinition.m_fSoftLockSpeedModifier < 1.0f) 
	{
		// This is a raycast to check for softlock
		CDirection obForward=CDirection(0.0f,0.0f,50.0f) * obCameraMatrix;

		CPoint obEnd=obCameraMatrix.GetTranslation() + obForward;

		Physics::RaycastCollisionFlag obFlag; obFlag.base = 0;
		obFlag.flags.i_am = Physics::COLLISION_ENVIRONMENT_BIT;
		obFlag.flags.i_collide_with = Physics::CHARACTER_CONTROLLER_ENEMY_BIT | Physics::LARGE_INTERACTABLE_BIT;

		Physics::TRACE_LINE_QUERY stQuery;

		if (Physics::CPhysicsWorld::Get().TraceLine(obCameraMatrix.GetTranslation(),obEnd,NULL,stQuery,obFlag))
		{
			if (stQuery.pobEntity && stQuery.pobEntity->IsEnemy())
				fSoftLockSpeedMultiplier=m_obDefinition.m_fSoftLockSpeedModifier;
		}
	}

	CDirection dir = input.m_obMoveDirection * obCameraMatrix.GetAffineInverse();

	float fYawPadMagnitude = dir.X() * input.m_fMoveSpeed;
	float fPitchPadMagnitude = dir.Z() * input.m_fMoveSpeed;
	float fPadMagnitude=fsqrtf((fYawPadMagnitude*fYawPadMagnitude)+(fPitchPadMagnitude*fPitchPadMagnitude));

	if	( CPlayerOptions::Get().GetInvertY() == false )	// cam-dwhite
		fPitchPadMagnitude=-fPitchPadMagnitude;

	// Beyond dead zone
	if (fPadMagnitude>=m_obDefinition.m_fPadThreshold)
	{
		fYawPadMagnitude=clamp( fYawPadMagnitude, -1.0f, 1.0f );

		fPitchPadMagnitude=clamp( fPitchPadMagnitude, -1.0f, 1.0f );

		// Rescale the magnitude so that it covers a range from 0.0 to 1.0
		if (fPadMagnitude>1.0f)
			fPadMagnitude=1.0f;

		fPadMagnitude-=m_obDefinition.m_fPadThreshold;
		fPadMagnitude/=(1.0f-m_obDefinition.m_fPadThreshold);
	}
	else // Ignore pad input below specified threshold
	{
		fPadMagnitude=0.0f;
	}

	// Have an input
	if (fPadMagnitude>0.0f)
	{
		// y = x^16 + 0.15x
		float fMagnitudeModifier=fPadMagnitude*fPadMagnitude*fPadMagnitude*fPadMagnitude;
		fMagnitudeModifier=fMagnitudeModifier*fMagnitudeModifier*fMagnitudeModifier*fMagnitudeModifier;
		fMagnitudeModifier+=0.15f*fPadMagnitude;

		if (fMagnitudeModifier>1.0f)
			fMagnitudeModifier=1.0f;

		// Speed increses
		if (fPadMagnitude>m_obDefinition.m_fPadAccelerationThreshold)
		{
			m_fYawSpeed+=m_obDefinition.m_fYawAcceleration * fTimeDelta;
			
			if (m_fYawSpeed>m_obDefinition.m_fYawMaxSpeed)
				m_fYawSpeed=m_obDefinition.m_fYawMaxSpeed;
			
			m_fPitchSpeed+=m_obDefinition.m_fPitchAcceleration * fTimeDelta;

			if (m_fPitchSpeed>m_obDefinition.m_fPitchMaxSpeed)
				m_fPitchSpeed=m_obDefinition.m_fPitchMaxSpeed;

			//g_VisualDebug->Printf2D(20.0f, 10.0f,  DC_GREEN, 0, "MagnitudeModifier %f - Speed inc", fMagnitudeModifier );
		}
		else // Speed is damped
		{
			m_fYawSpeed-=m_obDefinition.m_fYawAcceleration * fTimeDelta;
			
			if (m_fYawSpeed<m_obDefinition.m_fYawMinSpeed)
				m_fYawSpeed=m_obDefinition.m_fYawMinSpeed;
			
			m_fPitchSpeed-=m_obDefinition.m_fPitchAcceleration * fTimeDelta;

			if (m_fPitchSpeed<m_obDefinition.m_fPitchMinSpeed)
				m_fPitchSpeed=m_obDefinition.m_fPitchMinSpeed;

			//g_VisualDebug->Printf2D(20.0f, 10.0f,  DC_GREEN, 0, "MagnitudeModifier %f - Speed damp", fMagnitudeModifier );
		}

		m_fYawDelta=fYawPadMagnitude * fMagnitudeModifier * m_fYawSpeed * DEG_TO_RAD_VALUE; // * fSoftLockSpeedMultiplier;
		m_fPitchDelta=fPitchPadMagnitude * fMagnitudeModifier * m_fPitchSpeed * DEG_TO_RAD_VALUE;

		if ( m_fYawDelta > 0.1f )
			m_fUserLean += m_obDefinition.m_fUserLeanDelta * fTimeDelta;
		else if ( m_fYawDelta < -0.1f )
			m_fUserLean -= m_obDefinition.m_fUserLeanDelta * fTimeDelta;

		m_fUserLean = clamp ( m_fUserLean, -1.0f, 1.0f);

	}
	else
	// No longer have an input
	{
		// Decelerate the yaw speed
		if ( m_fYawSpeed > 0.0f )
		{
			m_fYawSpeed-=m_obDefinition.m_fYawAcceleration * fTimeDelta;

			if (m_fYawSpeed < 0.0f )
				m_fYawSpeed = 0.0f;
		}		

		m_fPitchSpeed=m_obDefinition.m_fPitchMinSpeed;

		const float fYawDeaccelerationFactor = 0.75f;
		const float fMinimumDelta = 0.001f;

		if (m_fYawDelta!=0.0f)
		{
			float fYawChange=-m_fYawDelta*fYawDeaccelerationFactor;

			if (fabsf(fYawChange)<fMinimumDelta)
				m_fYawDelta=0.0f;
			else
				m_fYawDelta+=fYawChange;
		}

		if (m_fPitchDelta!=0.0f)
		{
			float fPitchChange=-m_fPitchDelta*fYawDeaccelerationFactor;

			if (fabsf(fPitchChange)<fMinimumDelta)
				m_fPitchDelta=0.0f;
			else
				m_fPitchDelta+=fPitchChange;
		}

		if ( m_fUserLean > 0.0f )
		{
			m_fUserLean -= m_obDefinition.m_fUserLeanDelta * fTimeDelta;
			m_fUserLean = clamp ( m_fUserLean, 0.0f, 1.0f);
		}
		else if ( m_fUserLean < 0.0f )
		{
			m_fUserLean += m_obDefinition.m_fUserLeanDelta * fTimeDelta;
			m_fUserLean = clamp ( m_fUserLean, -1.0f, 0.0f);
		}
	}

	// Update the pitch/yaw
	m_fPitch+=m_fPitchDelta * fTimeDelta;
	m_fYaw+=m_fYawDelta * fTimeDelta;

	m_fPitch=clamp( m_fPitch, (m_obDefinition.m_fMaxDownPitch * DEG_TO_RAD_VALUE), (m_obDefinition.m_fMaxUpPitch * DEG_TO_RAD_VALUE) );

	if ( m_obDefinition.m_fMaxYaw > 0.0f )
	{
		float fMaxYawRad = m_obDefinition.m_fMaxYaw * DEG_TO_RAD_VALUE;

		if ( m_fYaw > fMaxYawRad )
			m_fYaw =  fMaxYawRad;
		else if ( m_fYaw < -fMaxYawRad )
			m_fYaw = -fMaxYawRad;
	}
	
	return TurretUpdate(fTimeDelta, input, currentState, predictedState);
}

// Update function for AIs

bool TurretController::AIUpdate(
	float fTimeDelta,
	const CMovementInput& input,
    const CMovementStateRef& currentState,
	CMovementState& predictedState
)
{
	ntAssert( m_obDefinition.m_bAIControlled );
	ntAssert_p(input.m_pAIUser->IsAI(),("TurretController::AIUpdate. Entity [%s] is not an AI but it is using a cannon.",ntStr::GetString(input.m_pAIUser->GetName())));
	
	//if (m_bFirstFrame)
	//{
	//	// Calculate the initial Yaw
	//	const CDirection& obZAxis = m_pYawTransform->GetLocalMatrix().GetZAxis();
	//	m_fYaw = fatan2f(obZAxis.X(), obZAxis.Z()); // Get the Y angle of the turret support

	//}

	CAIComponent* pAIComp = input.m_pAIUser->ToAI()->GetAIComponent();
	CAIMovement* pMov = pAIComp->GetCAIMovement();
	pMov->SetCannonFacingTarget(false);

	bool bYawAligned = false;
	bool bPitchAligned = false;

	m_fPitchDelta	= 1.0f;//0.5f*(m_pobControllerDef->m_fPitchMaxSpeed + m_pobControllerDef->m_fPitchMinSpeed);
	m_fYawDelta		= 1.0f;//0.5f*(m_pobControllerDef->m_fYawMaxSpeed + m_pobControllerDef->m_fYawMinSpeed);
	
	// Adjust the Yaw command accounting for the initial Yaw.
	float fActualYawCommand		= input.m_fYaw - m_fInitialTurretYaw;
	float fActualPitchCommand	= input.m_fPitch - m_fInitialTurretPitch;

	float fDiffYaw		= fabsf(fActualYawCommand - m_fYaw);
	float fDiffPitch	= fabsf(fActualPitchCommand - m_fPitch);

	// Calculate YAW movement
	if (fDiffYaw < 0.05f)
	{
		m_fYaw = fActualYawCommand;
		bYawAligned = true;
	}
	else
	{
		// Dario : If the turret is suppose to be able to turn 360 degrees, then due to the Euler angles we need to check 
		// the relative positions between turret and target in order to take the shortest turning direction.
		// This is called on a per-frame basis.
		
		if ( fActualYawCommand < m_fYaw )
			m_fYawDelta = -m_fYawDelta;
		
		m_fYaw+=m_fYawDelta * fTimeDelta;

		//if (m_fYaw > PI )
		//	m_fYaw = -PI+(m_fYaw-PI);
		//else if (m_fYaw < -PI )
		//	m_fYaw = PI+(PI+m_fYaw);
	}

	// Calculate PITCH movement
	if (fDiffPitch < 0.05f)
	{
		m_fPitch = fActualPitchCommand;
		bPitchAligned = true;
	}
	else
	{
		if ( fActualPitchCommand < m_fPitch )
			m_fPitchDelta  = -m_fPitchDelta;

		m_fPitch+=m_fPitchDelta * fTimeDelta;
	}

	if (bYawAligned && bPitchAligned)
	{
		pMov->SetCannonFacingTarget(true);
	}

	return TurretUpdate(fTimeDelta, input, currentState, predictedState);
}

bool TurretController::TurretUpdate(float fTimeDelta, const CMovementInput& input,
						const CMovementStateRef& currentState, CMovementState& predictedState)
{
	//g_VisualDebug->Printf2D(20.0f, 30.0f,  DC_GREEN, 0, "Pitch %f, Yaw %f, Recoil %f, Damp %f", m_fPitch, m_fYaw, m_fCurrRecoilAngle, m_fCurrDampAngle );
	//g_VisualDebug->Printf2D(20.0f, 50.0f,  DC_GREEN, 0, "Yaw Delta %f, Yaw Speed %f, Lean %f", m_fYawDelta, m_fYawSpeed, m_fUserLean);	

	// Centre blend balance
	float fLeftUpBlend		= 0.0f;
	float fLeftMidBlend		= 0.0f;
	float fLeftDownBlend	= 0.0f;
	float fCentreUpBlend	= 0.0f;
	float fCentreMidBlend	= 1.0f;
	float fCentreDownBlend	= 0.0f;
	float fRightUpBlend		= 0.0f;
	float fRightMidBlend	= 0.0f;
	float fRightDownBlend	= 0.0f;
	float fFireBlendWeight	= 0.0f;

	float fPitchBlend = 0.0f;
	float fInversePitchBlend = 0.0f;

	float fYawBlend = abs(m_fUserLean);
	float fInverseYawBlend = 1.0f - fYawBlend;

	float fCurrPitch = m_fPitch - m_fCurrRecoilAngle + m_fCurrDampAngle;

	if ( fCurrPitch < 0.0f )
	{
		fPitchBlend = fCurrPitch/(m_obDefinition.m_fMaxDownPitch* DEG_TO_RAD_VALUE);
		fPitchBlend = clamp ( fPitchBlend, 0.0f, 1.0f);
		fInversePitchBlend = 1.0f - fPitchBlend;

		fCentreDownBlend	= fPitchBlend * fInverseYawBlend;
		fCentreMidBlend		= fInversePitchBlend * fInverseYawBlend;

		if ( m_fUserLean < 0.0f ) // Right
		{
			fLeftDownBlend	= fPitchBlend * fYawBlend;
			fLeftMidBlend	= fInversePitchBlend * fYawBlend;
		}
		else	// Left
		{
			fRightDownBlend	= fPitchBlend * fYawBlend;
			fRightMidBlend	= fInversePitchBlend * fYawBlend;
		}
	}
	else 
	{
		fPitchBlend = fCurrPitch/(m_obDefinition.m_fMaxUpPitch* DEG_TO_RAD_VALUE);		
		fPitchBlend = clamp ( fPitchBlend, 0.0f, 1.0f);
		fInversePitchBlend = 1.0f - fPitchBlend;

		fCentreUpBlend	= fPitchBlend * fInverseYawBlend;
		fCentreMidBlend		= fInversePitchBlend * fInverseYawBlend;

		if ( m_fUserLean < 0.0f ) // Right
		{
			fLeftUpBlend	= fPitchBlend * fYawBlend;
			fLeftMidBlend	= fInversePitchBlend * fYawBlend;
		}
		else	// Left
		{
			fRightUpBlend	= fPitchBlend * fYawBlend;
			fRightMidBlend	= fInversePitchBlend * fYawBlend;
		}
	}	

	m_pobPullLeftUpAnim->SetBlendWeight( fLeftUpBlend );
	m_pobPullLeftMidAnim->SetBlendWeight( fLeftMidBlend );
	m_pobPullLeftDownAnim->SetBlendWeight( fLeftDownBlend );

	m_pobPullRightUpAnim->SetBlendWeight( fRightUpBlend );
	m_pobPullRightMidAnim->SetBlendWeight( fRightMidBlend );
	m_pobPullRightDownAnim->SetBlendWeight( fRightDownBlend );

	m_pobPullCentreUpAnim->SetBlendWeight( fCentreUpBlend );
	m_pobIdleAnim->SetBlendWeight( fCentreMidBlend );
	m_pobPullCentreDownAnim->SetBlendWeight( fCentreDownBlend );

	if ( m_fCurrUserRecoilTime > 0.0f )
	{
		/*static const float FIRING_PHASE_IN = 0.25f;
		float fFireBlend = 0.0f;

		if( m_fCurrUserRecoilTime <= FIRING_PHASE_IN )
		{
			fFireBlend = CMaths::SmoothStep( m_fCurrUserRecoilTime / FIRING_PHASE_IN );
		}
		else
		{
			float fDuration = m_obDefinition.m_fUserRecoilTime;
		
			if ( fDuration == 0.0f) 
				fDuration = m_pobRecoilMidAnim->GetDuration();
			fFireBlend = 1.0f - CMaths::SmoothStep( (m_fCurrUserRecoilTime - FIRING_PHASE_IN) / fDuration );
		}*/

		fFireBlendWeight = 1.0f;// - fFireBlend;
		m_fCurrUserRecoilTime -= fTimeDelta;

		m_pobRecoilUpAnim->SetBlendWeight( fFireBlendWeight * fCentreUpBlend );
		m_pobRecoilMidAnim->SetBlendWeight( fFireBlendWeight * fCentreMidBlend );
		m_pobRecoilDownAnim->SetBlendWeight( fFireBlendWeight * fCentreDownBlend );		
	}
	
	/*g_VisualDebug->Printf2D(20.0f, 100.0f,  DC_RED, 0, "%f  %f  %f", fLeftUpBlend, fCentreUpBlend, fRightUpBlend);
	g_VisualDebug->Printf2D(20.0f, 130.0f,  DC_RED, 0, "%f  %f  %f", fLeftMidBlend, fCentreMidBlend, fRightMidBlend);
	g_VisualDebug->Printf2D(20.0f, 160.0f,  DC_RED, 0, "%f  %f  %f", fLeftDownBlend, fCentreDownBlend, fRightDownBlend);
	g_VisualDebug->Printf2D(20.0f, 190.0f,  DC_RED, 0, "                    %f", fFireBlendWeight);	*/
	
	// Calculate what the position of the player should be
	{
		Transform* pTransform = m_pDrivingSeat;

		const CMatrix& obTargetWorldMatrix=pTransform->GetWorldMatrix();

		// Calculate the translation delta
		CDirection obTranslationOffset=CDirection(m_obDefinition.m_ptTranslationOffset) * obTargetWorldMatrix;

		CPoint obNewPosition(obTargetWorldMatrix.GetTranslation());
		obNewPosition+=obTranslationOffset;

		predictedState.m_obProceduralRootDelta=CDirection(obNewPosition-currentState.m_obPosition);		
		
		// Calculate the rotation delta
		
		CQuat obTargetRotation(obTargetWorldMatrix);

		//CQuat obNewRotation=obTargetRotation * m_obDefinition.m_obRotationOffset;

		//CMatrix obNewMatrix(obNewRotation);
		CMatrix obNewMatrix(obTargetRotation);

		float ax,ay,az;
		
		CCamUtil::EulerFromMat_XYZ(obNewMatrix,ax,ay,az);

		predictedState.m_fProceduralPitch=ax;
		predictedState.m_fProceduralYaw=ay;
		predictedState.m_fProceduralRoll=az;
		predictedState.m_bApplyExplicitRotations = true;
	}

	// Update procedural animation (recoil)
	switch ( m_pVehicle->m_eTurretState )
	{
	// Set up for recoil
	case TS_FIRE:
		{
			// Get set to recoil
			m_fCurrRecoilTime = m_obDefinition.m_fRecoilTime;
			m_pVehicle->m_eTurretState = TS_RECOIL;

			// Setup an amount to loose yaw aim by
			if ( m_obDefinition.m_fMaxAimLossAngle > 0.0f )
			{
				float fMaxYawRad = m_obDefinition.m_fMaxYaw * DEG_TO_RAD_VALUE;
				m_fAimLossAngle = m_obDefinition.m_fMaxAimLossAngle * (grandf(2.0f) - 1.0f);
		
				if ( m_obDefinition.m_fMaxYaw > 0.0f )
				{
					float fAimLossAngleRad = m_fAimLossAngle* DEG_TO_RAD_VALUE;
					if ( (m_fYaw + fAimLossAngleRad) > fMaxYawRad || (m_fYaw + fAimLossAngleRad) < -fMaxYawRad )
					{
						m_fAimLossAngle = -m_fAimLossAngle;
					}
				}
			}
			m_fCurrAimLossAngle = 0.0f;


			float fSpeed = 1.0f;
			if ( m_obDefinition.m_fUserRecoilTime != 0.0f )
			{
				fSpeed = m_pobRecoilMidAnim->GetDuration() / m_obDefinition.m_fUserRecoilTime ;
				m_fCurrUserRecoilTime = m_obDefinition.m_fUserRecoilTime;
			}
			else
				m_fCurrUserRecoilTime = m_pobRecoilMidAnim->GetDuration();

			m_pobRecoilUpAnim->SetSpeed( fSpeed );
			m_pobRecoilUpAnim->SetTime(0.0f);
			m_pobRecoilMidAnim->SetSpeed( fSpeed );
			m_pobRecoilMidAnim->SetTime(0.0f);
			m_pobRecoilDownAnim->SetSpeed( fSpeed );
			m_pobRecoilDownAnim->SetTime(0.0f);
			

			break;
		}

	// Doing recoil
	case TS_RECOIL:
		m_fCurrRecoilTime -= fTimeDelta;

		m_fCurrRecoilAngle = CMaths::SmoothStep( 1.0f - m_fCurrRecoilTime / m_obDefinition.m_fRecoilTime ) * m_obDefinition.m_fRecoilAngle * DEG_TO_RAD_VALUE;
		m_fCurrAimLossAngle = CMaths::SmoothStep( 1.0f - m_fCurrRecoilTime / m_obDefinition.m_fRecoilTime ) * m_fAimLossAngle * DEG_TO_RAD_VALUE;
		
		// Done - setup simple settle move
		if ( m_fCurrRecoilTime < 0.0f )
		{
			m_pVehicle->m_eTurretState = TS_SETTLE;
			m_fCurrRecoilTime = m_obDefinition.m_fSettleTime;
			m_fAcheivedRecoilAngle = m_fCurrRecoilAngle * RAD_TO_DEG_VALUE;

			m_fYaw += m_fAimLossAngle * DEG_TO_RAD_VALUE;
			m_fCurrAimLossAngle = 0.0f;
		}
		break;

	// Simple settle movment
	case TS_SETTLE:
		m_fCurrRecoilTime -= fTimeDelta;

		if ( m_fPitch - m_obDefinition.m_fSettleAngle * DEG_TO_RAD_VALUE  < m_obDefinition.m_fMaxDownPitch * DEG_TO_RAD_VALUE )
		{
			m_fCurrRecoilAngle = (m_fAcheivedRecoilAngle - CMaths::SmoothStep( 1.0f - m_fCurrRecoilTime / m_obDefinition.m_fSettleTime ) * m_fAcheivedRecoilAngle ) * DEG_TO_RAD_VALUE;
		}
		else
		{
			m_fCurrRecoilAngle = (m_fAcheivedRecoilAngle - CMaths::SmoothStep( 1.0f - m_fCurrRecoilTime / m_obDefinition.m_fSettleTime ) * (m_obDefinition.m_fRecoilAngle - m_obDefinition.m_fSettleAngle) ) * DEG_TO_RAD_VALUE;
		}

		m_fCurrDampAngle = CMaths::SmoothStep( 1.0f - m_fCurrRecoilTime / m_obDefinition.m_fSettleTime ) * m_obDefinition.m_fDampedAngle * DEG_TO_RAD_VALUE;

		// Done - setup a fake damped motion
		if ( m_fCurrRecoilTime < 0.0f )
		{
			m_fAcheivedRecoilAngle = m_fCurrDampAngle;
			
			m_fPitch -= m_fCurrRecoilAngle; 
			m_fCurrRecoilAngle = 0.0f;

			m_pVehicle->m_eTurretState = TS_DAMPED;
			m_fCurrRecoilTime = m_obDefinition.m_fDampedTime;
		}
		break;

	// A fake damped motion
	case TS_DAMPED:
	{
		m_fCurrRecoilTime -= fTimeDelta;

		float fNormalisedTime = m_fCurrRecoilTime / m_obDefinition.m_fDampedTime;

		// SHM = Amp * cos ( 2 * pi * time * freq )
		float fSHM = m_fAcheivedRecoilAngle * fcosf ( TWO_PI * fNormalisedTime *  m_obDefinition.m_fDampedFrequency);

		// damping = t^2
		m_fCurrDampAngle = fSHM * fNormalisedTime * fNormalisedTime;
	
		// Done - put in a normal state
		if ( m_fCurrRecoilTime < 0.0f )
		{
			m_pVehicle->m_eTurretState = TS_INACTIVE;
			m_fCurrRecoilTime = 0.0f;

			m_fAcheivedRecoilAngle = m_fCurrDampAngle = 0.0f;
			m_fCurrDampAngle = 0.0f;
		}
		break;
	}

	case TS_INACTIVE:
		break;

	default:
		ntAssert(0);
		break;
	} // switch ( m_pVehicle->m_eTurretState )*/


	// FIX ME: I'm not sure this is correct as true removes and cleans up most movement controllers
	return true;
}

void TurretController::PostAnimatorUpdate ()
{
	// Re apply root transform
	m_pRoot->SetLocalMatrix( m_obInitalRoot );

	// Apply yaw rotation to the turret 
	CMatrix obYawMatrix;
	CCamUtil::MatrixFromEuler_XYZ(obYawMatrix,0.0f,m_fYaw+m_fCurrAimLossAngle,0.0f);
	for ( ntstd::Vector<Transform*>::iterator obIt = m_apYawTransform.begin();
		obIt != m_apYawTransform.end(); obIt++ )
	{
		obYawMatrix.SetTranslation( (*obIt)->GetLocalTranslation() );
		(*obIt)->SetLocalMatrix(obYawMatrix);
	}
	
	// Apply pitch rotation to the turret 
	CMatrix obPitchMatrix;
	CCamUtil::MatrixFromEuler_XYZ(obPitchMatrix,m_fPitch-m_fCurrRecoilAngle+m_fCurrDampAngle,0.0f,0.0f);
	for ( ntstd::Vector<Transform*>::iterator obIt = m_apPitchTransform.begin();
		obIt != m_apPitchTransform.end(); obIt++ )
	{
		obPitchMatrix.SetTranslation( (*obIt)->GetLocalTranslation() );
		(*obIt)->SetLocalMatrix(obPitchMatrix);
	}

	// Apply pitch & yaw rotation to the turret 
	CMatrix obCombinedMatrix;
	CCamUtil::MatrixFromEuler_XYZ(obCombinedMatrix,m_fPitch-m_fCurrRecoilAngle+m_fCurrDampAngle,m_fYaw+m_fCurrAimLossAngle,0.0f);
	for ( ntstd::Vector<Transform*>::iterator obIt = m_apCombinedTransform.begin();
		obIt != m_apCombinedTransform.end(); obIt++ )
	{
		obCombinedMatrix.SetTranslation( (*obIt)->GetLocalTranslation() );
		(*obIt)->SetLocalMatrix(obCombinedMatrix);
	}
}

