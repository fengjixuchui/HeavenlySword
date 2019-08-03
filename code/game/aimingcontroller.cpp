//------------------------------------------------------------------------------------------
//!
//!	\file AimController.cpp
//!
//------------------------------------------------------------------------------------------


/////////////////////////////////
// Required Includes
/////////////////////////////////
#include "aimcontroller.h"
#include "movement.h"
#include "camera/camman.h"
#include "objectdatabase/dataobject.h"
#include "anim/animator.h"
#include "core/exportstruct_anim.h"
#include "game/entitymanager.h"
#include "game/query.h"
#include "camera/camerainterface.h"
#include "camera/camview.h"
#include "camera/coolcam_chaseaimcombo.h"
#include "camera/coolcam_aim.h"
#include "game/luaglobal.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "game/entityarcher.h"
#include "game/messagehandler.h"
#include "game/inputcomponent.h"
#include "Physics/world.h" // Raycast stuff
#include "core/visualdebugger.h"
#include "game/messagehandler.h"
#include "game/playeroptions.h"
#include "gfx/renderer.h"
#include "gfx/rendercontext.h"
#include "effect/effect_manager.h"
#include "game/entityprojectile.h"
#include "physics/projectilelg.h"

START_STD_INTERFACE(AimingControllerDef)
	PUBLISH_VAR_AS(m_obLeftUpAnimation,			LeftUpAnimation)
	PUBLISH_VAR_AS(m_obLeftMidAnimation,		LeftMidAnimation)
	PUBLISH_VAR_AS(m_obLeftDownAnimation,		LeftDownAnimation)
	PUBLISH_VAR_AS(m_obCentreUpAnimation,		CentreUpAnimation)
	PUBLISH_VAR_AS(m_obCenterMidAnimation,		CenterMidAnimation)
	PUBLISH_VAR_AS(m_obCenterDownAnimation,		CenterDownAnimation)
	PUBLISH_VAR_AS(m_obRightUpAnimation,		RightUpAnimation)
	PUBLISH_VAR_AS(m_obRightMidAnimation,		RightMidAnimation)
	PUBLISH_VAR_AS(m_obRightDownAnimation,		RightDownAnimation)
	PUBLISH_VAR_AS(m_obUpFireAnimation,			UpFireAnimation)
	PUBLISH_VAR_AS(m_obCentreFireAnimation,		CentreFireAnimation)
	PUBLISH_VAR_AS(m_obDownFireAnimation,		DownFireAnimation)
	PUBLISH_VAR_AS(m_obHopAnimation,			HopAnimation)
	PUBLISH_VAR_AS(m_fConstMagnitudePower,		ConstMagnitudePower)
	PUBLISH_VAR_AS(m_fConstMagnitudeModifier,	ConstMagnitudeModifier)
	PUBLISH_VAR_AS(m_fYawMinSpeed,				YawMinSpeed)
	PUBLISH_VAR_AS(m_fYawMaxSpeed,				YawMaxSpeed)
	PUBLISH_VAR_AS(m_fYawAcceleration,			YawAcceleration)
	PUBLISH_VAR_AS(m_fConstYawBlendFactor,		ConstYawBlendFactor)
	PUBLISH_VAR_AS(m_fConstYawDeacceleration,	ConstYawDeacceleration)
	PUBLISH_VAR_AS(m_fConstYawMinimumDelta,		ConstYawMinimumDelta)
	PUBLISH_VAR_AS(m_fPitchMinSpeed,			PitchMinSpeed)
	PUBLISH_VAR_AS(m_fPitchMaxSpeed,			PitchMaxSpeed)
	PUBLISH_VAR_AS(m_fPitchAcceleration,		PitchAcceleration)
	PUBLISH_VAR_AS(m_fSoftLockSpeedModifier,	SoftLockSpeedModifier)
	PUBLISH_VAR_AS(m_fPadInputScreenSpaceScaleX,PadInputScreenSpaceScaleX)
	PUBLISH_VAR_AS(m_fPadInputScreenSpaceScaleY,PadInputScreenSpaceScaleY)
	PUBLISH_VAR_AS(m_fCamMovementThreshold,		CamMovementThreshold)
	PUBLISH_VAR_AS(m_fPadAccelerationThreshold,	PadAccelerationThreshold)
	PUBLISH_VAR_AS(m_fPadThreshold,				PadThreshold)
	PUBLISH_VAR_AS(m_fCamRaySmootherScalar,		CamRaySmootherScalar)
	PUBLISH_VAR_AS(m_fCamRayStartOffset,		CamRayStartOffset)
	PUBLISH_VAR_AS(m_fAimingYawScalar,			AimingYawScalar)
	PUBLISH_VAR_AS(m_fAimingPitchScalar,		AimingPitchScalar)
	PUBLISH_VAR_AS(m_fMovingYawScalar,			MovingYawScalar)
	PUBLISH_VAR_AS(m_WeaponName,				WeaponName)
	PUBLISH_VAR_AS(m_hWeaponAimingDirection,	WeaponAimingDirectionJoint)
	PUBLISH_VAR_AS(m_obFiringPopoutWindow,		FiringPopoutWindow)
	PUBLISH_VAR_AS(m_fAKRecoil,					AKRecoilAmplidude)
	PUBLISH_VAR_AS(m_obFiringAKRecoilWindow,	AKRecoilWindow)
	PUBLISH_VAR_AS(m_fAKRecoilModifier,			AKRecoilModifier)
	PUBLISH_VAR_AS(m_fAKRecoilModifierFilter,	AKRecoilModifierFilter)
	PUBLISH_VAR_AS(m_fBeamUpOffset,				BeamUpOffset)
	PUBLISH_PTR_AS(m_pLaserEffect,				LaserEffect)
	PUBLISH_VAR_AS(m_fChargedFOV,				ChargedFOV_Degs)
	PUBLISH_VAR_AS(m_fChargedFOVTime,			ChargedFOVTime)
	PUBLISH_VAR_AS(m_bUseGravity,				UseGravity)
	PUBLISH_VAR_AS(m_fInputDelay,				InputDelay)
END_STD_INTERFACE

START_STD_INTERFACE(AimingLaserEffectDef)
	PUBLISH_VAR_AS(m_fDotSize,					DotSize)
	PUBLISH_VAR_AS(m_fDotLuminosity,			DotLuminosity)
	PUBLISH_VAR_AS(m_fCameraAdjust,				CameraAdjust)
	PUBLISH_VAR_AS(m_fDotLuminosityFallOff,		DotLuminosityFallOff)
	PUBLISH_VAR_AS(m_obDotColour,				DotColour)
	PUBLISH_VAR_AS(m_obDotTexture,				DotTexture)

END_STD_INTERFACE

START_STD_INTERFACE(AimingLaunchControllerDef)
	PUBLISH_VAR_AS(m_obCentreUpAnimation,	CentreUpAnimation)
	PUBLISH_VAR_AS(m_obCenterMidAnimation,	CenterMidAnimation)
	PUBLISH_VAR_AS(m_obCenterDownAnimation,	CenterDownAnimation)
	PUBLISH_VAR_AS(m_fYawSpeedMultiplier,	YawSpeedMultiplier)
	PUBLISH_VAR_AS(m_fPitchSpeedMultiplier,	PitchSpeedMultiplier)
	PUBLISH_VAR_AS(m_fPadThreshold,			PadThreshold)
	PUBLISH_VAR_AS(m_fAccelerationFactor,	AccelerationFactor)
	PUBLISH_VAR_AS(m_fDeaccelerationFactor,	DeaccelerationFactor)
	PUBLISH_VAR_AS(m_bAllowMovement,		AllowMovement)
END_STD_INTERFACE


START_STD_INTERFACE(AimingWeaponControllerDef)
	PUBLISH_VAR_AS(m_obIdle,			Idle)
	PUBLISH_VAR_AS(m_obArm1ChargeUp,	Arm1ChargeUp)
	PUBLISH_VAR_AS(m_obArm2ChargeUp,	Arm2ChargeUp)
	PUBLISH_VAR_AS(m_obArm1Charged,		Arm1Charged)
	PUBLISH_VAR_AS(m_obArm2Charged,		Arm2Charged)
	PUBLISH_VAR_AS(m_obArm1Firing,		Arm1Firing)
	PUBLISH_VAR_AS(m_obArm2Firing,		Arm2Firing)
END_STD_INTERFACE

//------------------------------------------------------------------------------------------
//!
//!	AimControllerDef::AimControllerDef
//!	Construction of a definition for a turret controller.
//!
//------------------------------------------------------------------------------------------
AimingControllerDef::AimingControllerDef(void) :
	m_fYawMinSpeed(90.0f),
	m_fYawMaxSpeed(180.0f),
	m_fYawAcceleration(45.0f),
	m_fPitchMinSpeed(45.0f),
	m_fPitchMaxSpeed(90.0f),
	m_fPitchAcceleration(45.0f),
	m_fSoftLockSpeedModifier(1.0f),
	m_fCamMovementThreshold(0.85f),
	m_fPadThreshold(0.05f),
	m_fPadAccelerationThreshold(0.99f),
	m_fPadInputScreenSpaceScaleX(0.3f),
	m_fPadInputScreenSpaceScaleY(0.3f),
	m_fCamRaySmootherScalar(1.0f),
	m_fCamRayStartOffset(1.0f),
	m_fAimingYawScalar(1.0f),
	m_fAimingPitchScalar(1.0f),
	m_fMovingYawScalar(1.0f),
	m_fAKRecoil(0.0f),
	m_fAKRecoilModifier(0.0f),
	m_fAKRecoilModifierFilter(2.0f),
	m_fConstYawBlendFactor(0.1f),
	m_fConstMagnitudePower(16.0f),
	m_fConstMagnitudeModifier(0.15f),
	m_fConstYawDeacceleration(0.75f),
	m_fConstYawMinimumDelta(0.001f),
	m_fBeamUpOffset(0.08f),
	m_fChargedFOV( 30.0f ),
	m_fChargedFOVTime( 0.2f ),
	m_bUseGravity(true),
	m_fInputDelay(0.2f)
{
}


//------------------------------------------------------------------------------------------
//!
//!	AimControllerDef::CreateInstance
//!
//!
//------------------------------------------------------------------------------------------
MovementController* AimingControllerDef::CreateInstance(CMovement* pobMovement) const
{
	// Null out the weapon entity pointer. 
	CEntity* pWeapon = 0;

	// Attempt to find the weapon name. 
	if( m_WeaponName.length() )
	{
		ntstd::String strFullWeaponName = pobMovement->GetParentEntity()->GetName() + m_WeaponName;
		pWeapon = CEntityManager::Get().FindEntity( strFullWeaponName.c_str() );
	}

	// Return the instance of the new controller
	return NT_NEW_CHUNK(Mem::MC_ENTITY) AimingController(pobMovement, this, pWeapon);
}


//------------------------------------------------------------------------------------------
//!
//!	AimingController::AimingController
//!	Construction
//!
//------------------------------------------------------------------------------------------
AimingController::AimingController (CMovement* pMovement, const AimingControllerDef* pDefinition, const CEntity* pWeapon ) :
	MovementController(pMovement),
	m_pobAimingControllerDef(pDefinition),
	m_fYawBlend(0.0f),
	m_fYawAcc(0.0f),
	m_fPitchAcc(0.0f),
	m_fPitch(0.0f),
	m_StateTime(0.0f),
	m_ReloadTime(0.0f),
	m_FireTime(-1.0f),
	m_bMagDirection(false),
	m_dirRequiredFacing( CONSTRUCT_CLEAR ),
	m_ptIntersect( CONSTRUCT_CLEAR ),
	m_fRecoilModifier( 0.0f ),
	m_pWeaponEnt( pWeapon ),
	m_bShotCharged(false)
{
	ntAssert(pMovement);
	ntAssert(pDefinition);

	const float fAnimSpeed = 1.0f;
	const int iAnimFlags = ANIMF_LOCOMOTING | ANIMF_INHIBIT_AUTO_DESTRUCT | ANIMF_LOOPING;

	// Set up all the animations - NEVER add them to the animator in the contstructor
	m_obAnims[ LeftUpAnimation ]		= m_pobAnimator->CreateAnimation(pDefinition->m_obLeftUpAnimation);
	m_obAnims[ LeftMidAnimation ]		= m_pobAnimator->CreateAnimation(pDefinition->m_obLeftMidAnimation);
	m_obAnims[ LeftDownAnimation ]		= m_pobAnimator->CreateAnimation(pDefinition->m_obLeftDownAnimation);
	m_obAnims[ CentreUpAnimation ]		= m_pobAnimator->CreateAnimation(pDefinition->m_obCentreUpAnimation);
	m_obAnims[ CenterMidAnimation ]		= m_pobAnimator->CreateAnimation(pDefinition->m_obCenterMidAnimation);
	m_obAnims[ CenterDownAnimation ]	= m_pobAnimator->CreateAnimation(pDefinition->m_obCenterDownAnimation);
	m_obAnims[ RightUpAnimation ]		= m_pobAnimator->CreateAnimation(pDefinition->m_obRightUpAnimation);
	m_obAnims[ RightMidAnimation ]		= m_pobAnimator->CreateAnimation(pDefinition->m_obRightMidAnimation);
	m_obAnims[ RightDownAnimation ]		= m_pobAnimator->CreateAnimation(pDefinition->m_obRightDownAnimation);

	if( !pDefinition->m_obUpFireAnimation.IsNull() && 
		!pDefinition->m_obCentreFireAnimation.IsNull() &&
		!pDefinition->m_obDownFireAnimation.IsNull() )
	{
		// Mark the controller as having firing animations
		m_bHasFiringAnims = true;

		m_obAnims[ UpFireAnimation ]		= m_pobAnimator->CreateAnimation(pDefinition->m_obUpFireAnimation);
		m_obAnims[ CentreFireAnimation ]	= m_pobAnimator->CreateAnimation(pDefinition->m_obCentreFireAnimation);
		m_obAnims[ DownFireAnimation ]		= m_pobAnimator->CreateAnimation(pDefinition->m_obDownFireAnimation);
	}
	else
	{
		m_bHasFiringAnims = false;
	}

	// Initialise the animations.
	for( int iAnimIndex = 0; iAnimIndex < ANIM_COUNT; ++iAnimIndex )
	{
		if( m_obAnims[ iAnimIndex ] )
		{
			m_obAnims[ iAnimIndex ]->SetSpeed(fAnimSpeed);
			m_obAnims[ iAnimIndex ]->SetBlendWeight(0.0f);
			m_obAnims[ iAnimIndex ]->SetFlagBits(iAnimFlags);
		}
	}

	// Is this entity the archer?
	m_pPlayer = pMovement->GetParentEntity()->IsPlayer() ? (Player*) pMovement->GetParentEntity()->ToPlayer() : 0;

	if (!pDefinition->m_obHopAnimation.IsNull())
	{
		m_obAnims[ BobAnimation ] = m_pobAnimator->CreateAnimation(pDefinition->m_obHopAnimation);
		m_obAnims[ BobAnimation ]->SetSpeed(0.0f);
		m_obAnims[ BobAnimation ]->SetBlendWeight(0.0f);
		m_obAnims[ BobAnimation ]->SetFlagBits(ANIMF_LOOPING | ANIMF_INHIBIT_AUTO_DESTRUCT);
	}

	CEntity* pobParentEntity=(CEntity*)pMovement->GetParentEntity();
	
	m_pobAimingComponent=(AimingComponent*)pobParentEntity->GetAimingComponent();
	ntAssert(m_pobAimingComponent);

	// If the entity has move a distance of X .. from the last time the aiming controller was used then 
	// reset the angles. 
	if( (m_pobAimingComponent->GetResetAnglePoint() - pobParentEntity->GetPosition()).LengthSquared() > 0.5f )
	{
		m_pobAimingComponent->ResetAngles();
		m_pobAimingComponent->SetResetAnglePoint( pobParentEntity->GetPosition() );
	}

	ntAssert( CamMan::GetPrimaryView() );
	m_matInitCamMatrix = CamMan::GetPrimaryView()->GetCurrMatrix();
	//m_pobAimingComponent->SetFromParent();

	// Clear out the laser pointer. 
	m_pLaser = 0;

	// If defined, a laser effect, then set it up
	if( pDefinition->m_pLaserEffect )
	{
		m_pLaser = NT_NEW_CHUNK ( Mem::MC_EFFECTS ) AimingLaserEffect( pDefinition->m_pLaserEffect );
	}
 
	// Projectile properties, just temporary hack for GameShare at 26.11.2006. 
	// Always choose crossbow.
	Projectile_Attributes* pobProjectileAttributes = ObjectDatabase::Get().GetPointerFromName<Projectile_Attributes*>(CHashedString("Att_ArcherRangedProjectile_CrossbowBolt"));
	if (pobProjectileAttributes)
	{	
		m_pobProjectileProperties = ObjectDatabase::Get().GetPointerFromName<ProjectileProperties*>(pobProjectileAttributes->m_Properties);
	}
	else
	{
		m_pobProjectileProperties = NULL;
	}

	ApplyGravity( pDefinition->m_bUseGravity );
}


//------------------------------------------------------------------------------------------
//!
//!	AimingController::~AimingController
//!	Destruction
//!
//------------------------------------------------------------------------------------------
AimingController::~AimingController ()
{
	if ( !m_bFirstFrame )
	{
		// Remove the animations.
		for( int iAnimIndex = 0; iAnimIndex < ANIM_COUNT; ++iAnimIndex )
		{
			if( m_obAnims[ iAnimIndex ] )
			{
				m_pobAnimator->RemoveAnimation(m_obAnims[ iAnimIndex ]);
			}
		}
	}

	// Remove the effect
	if( m_pLaser )
	{
		EffectManager::Get().KillEffectWhenReady( m_pLaser->GetEffectID() );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	AimingController::AddAnimations
//!	Adds the animations to the animator - should always be done on the first frame update
//!
//------------------------------------------------------------------------------------------
void AimingController::AddAnimations( void )
{
	// Add all our animations to the animator
	for( int iAnimIndex = 0; iAnimIndex < ANIM_COUNT; ++iAnimIndex )
	{
		if( m_obAnims[ iAnimIndex ] )
		{
			m_pobAnimator->AddAnimation(m_obAnims[ iAnimIndex ]);
		}
	}
}


//------------------------------------------------------------------------------------------
//!
//!	AimingController::Update
//!	
//!
//------------------------------------------------------------------------------------------
bool AimingController::Update(float fTimeDelta, const CMovementInput& input, const CMovementStateRef& /*currentState*/, CMovementState& predictedState)
{
	static const float PAD_READ_SMOOTH_TIME = 1.0f;
	static const float STAN_CAM_INPUT_TH	= 0.3f;
	static const float STAN_CAM_CUT_TIME	= 0.04f;

	// If this entity isn't the active player - reset the angles and don't update the rest of the aiming code
	if( CEntityManager::Get().GetPlayer() != m_pPlayer )
	{
		m_pobAimingComponent->ResetAngles();
		
		// Reset the yaw value.
		predictedState.m_fProceduralYaw = 0.0f;

		return true;
	}

	// If firing the weapon - then don't update the controller
	if( m_pPlayer && m_pPlayer->GetState() == Player::ARC_1ST_FIRING )
	{
		return true;
	}

	// Obtain a base for an aiming matrix.
	CMatrix matAiming = m_pWeaponEnt ? m_pWeaponEnt->GetMatrix() : m_pobMovement->GetParentEntity()->GetMatrix();

	// Add the necessary animations to the aniamtor
	if ( m_bFirstFrame )
	{
		AddAnimations();
		m_PadReadFudge = 0.0f;

		// If the pad movement is still pressed, perhaps the pad input should be blended in.
		if( input.m_fMoveSpeed > 0.8f )
			m_PadReadFudge = PAD_READ_SMOOTH_TIME;
	}

	// Grab the camview pointer. 
	const CamView* pCamView	= CamMan::Get().GetPrimaryView();
	ntError( pCamView && "Making sure there is a camera" );

	CoolCam_ChaseAimCombo*	pCACCam = (CoolCam_ChaseAimCombo*) pCamView->FindCoolCam(CT_CHASEAIMCOMBO);
	CoolCam_Aim*			pCam	= pCACCam ? pCACCam->GetAimCam() : (CoolCam_Aim*)pCamView->FindCoolCam(CT_AIM);

	// From the current view get the camera matrix. 
	CMatrix matWorldCam = pCamView->GetCurrMatrix();

	// Make the 
	if( pCam && pCam->GetCamDef().m_bUpsideDownCam )
	{
		matWorldCam.SetXAxis( matWorldCam.GetXAxis() * -1.f );
		matWorldCam.SetYAxis( matWorldCam.GetYAxis() * -1.f );
	}

	if( m_pPlayer )
	{
		if ( m_bHasFiringAnims && m_pPlayer->RequestFire() ) 
		{ 
			// The duration of the firing animation is used to determine whether the animation is in the pop out window,
			// which controls the rate of fire.
			const float fFiringAnimLength = m_obAnims[CentreFireAnimation]->GetDuration();

			// If not playing a firing animation, or if in a popout window...
			if( m_FireTime < 0.0f || m_pobAimingControllerDef->m_obFiringPopoutWindow.IsSet( m_FireTime, fFiringAnimLength ) )
			{
				// Send a message to the state machine on the archer asking for a change into a firing state. 
				ntAssert( m_pobMovement ); ntAssert( m_pobMovement->GetParentEntity() ); ntAssert( m_pobMovement->GetParentEntity()->GetMessageHandler() );
				
				Message message(msg_combat_attack);
				message.SetEnt("Sender", m_pPlayer );
				m_pPlayer->GetMessageHandler()->QueueMessage(message);

				// If there is a weapon defined too, send it the message. 
				// The message is sent here to reduce the delay between the fire press and
				// the launch of the projectile. It might be the case that the projectile
				// will also get sent from here too if this proves too slow. 
				if( m_pWeaponEnt )
				{
					message.SetFloat( "X", matAiming.GetZAxis().X() );
					message.SetFloat( "Y", matAiming.GetZAxis().Y() );
					message.SetFloat( "Z", matAiming.GetZAxis().Z() );

					// If the entity is the arhcer - then pass the firing charge. 
					if( m_pPlayer->IsArcher() )
					{
						message.SetFloat( "Charge", m_pPlayer->ToArcher()->GetFiringCharge() );
					}
					

					m_pWeaponEnt->GetMessageHandler()->QueueMessage(message);
				}


				// Increase the number of arrows fired. 
				m_pPlayer->IncFiredCount();

				//
				m_obAnims[UpFireAnimation]->SetTime(0.0f);
				m_obAnims[CentreFireAnimation]->SetTime(0.0f);
				m_obAnims[DownFireAnimation ]->SetTime(0.0f);

				// 
				if( m_FireTime > 0.0f )
				{
					m_fRecoilModifier += m_pobAimingControllerDef->m_fAKRecoilModifier;
				}

				// 
				m_FireTime = 0.0f;
			}
		}
		else if( m_StateTime < STAN_CAM_CUT_TIME )
		{
			if( input.m_fMoveSpeed > STAN_CAM_INPUT_TH )
			{
				m_dirRequiredFacing = input.m_obMoveDirection;
			}

			m_bFirstFrame	= false;
			m_StateTime		+= fTimeDelta;

			if( m_StateTime >= STAN_CAM_CUT_TIME )
			{	
				CMessageSender::SendEmptyMessage( "msg_cut_the_stan_cam", m_pobMovement->GetParentEntity()->GetMessageHandler() );

				if( m_dirRequiredFacing.LengthSquared() > EPSILON )
				{
					predictedState.m_fProceduralYaw = MovementControllerUtilities::RotationAboutY( m_pobMovement->GetParentEntity()->GetMatrix().GetZAxis(), m_dirRequiredFacing );
				}
			}

			return true;
		}
	}

	//const float fYawBlendFactor = m_pobAimingControllerDef->m_fConstYawBlendFactor; //0.1f

	float fYawPadMagnitude		= 0.0f;
	float fPitchPadMagnitude	= 0.0f;

	if( !m_bFirstFrame )
	{
		CDirection dir = input.m_obMoveDirection * matWorldCam.GetAffineInverse();

		float fInputMag = input.m_fMoveSpeed;

		if( fInputMag < 0.5f )
			m_PadReadFudge = 0.0f;

		// Calc the input speed phased in
		float fInputSpeed = fInputMag * (1.0f - CMaths::SmoothStep( m_PadReadFudge / PAD_READ_SMOOTH_TIME ));

		// decrement the count and clamp it to zero
		m_PadReadFudge		= m_PadReadFudge - fTimeDelta;
		m_PadReadFudge		= m_PadReadFudge < 0.0f ? 0.0f : m_PadReadFudge;

		// If firing the weapon
		if( m_FireTime >= 0.0f )
		{
			// The duration of the firing animation is used to determine whether the ak recoil should be applied
			const float fFiringAnimLength = m_obAnims[CentreFireAnimation]->GetDuration();

			// If in the AK Recoil window.. 
			if( m_pobAimingControllerDef->m_obFiringAKRecoilWindow.IsSet( m_FireTime, fFiringAnimLength ) )
			{
				// Make the input a little sleepy
				dir.X() = fInputSpeed < 0.4f ? 0.0f : dir.X() / 10.0f;
				dir.Z() = fInputSpeed < 0.4f ? 0.0f : dir.Z() / 10.0f;

				// Adjust the directional input with the recoil
				dir.Z() += m_pobAimingControllerDef->m_fAKRecoil * m_fRecoilModifier;

				// Normalise the output. 
				dir.Normalise();

				// If the input speed isn't set, then add our own value
				fInputSpeed = fInputSpeed < 0.4f ? 1.0f : fInputSpeed;
			}
		}

		fPitchPadMagnitude	= dir.Z() * fInputSpeed;
		fYawPadMagnitude	= dir.X() * fInputSpeed;
	}

	// If there is a recoil modifier set, then apply a basic filter to remove it.
	if( m_fRecoilModifier > 0.0f )
	{
		m_fRecoilModifier /= m_pobAimingControllerDef->m_fAKRecoilModifierFilter;
	}

	if( fabs(fPitchPadMagnitude) < EPSILON )
		fPitchPadMagnitude = 0.0f;

	if( fabs(fYawPadMagnitude) < EPSILON )
		fYawPadMagnitude = 0.0f;

	if( fabs(fPitchPadMagnitude) > 0.97f )
		fPitchPadMagnitude = fPitchPadMagnitude < 0.0f ? -1.0f : 1.0f;

	if( fabs(fYawPadMagnitude) > 0.97f )
		fYawPadMagnitude = fYawPadMagnitude < 0.0f ? -1.0f : 1.0f;

	// Calculate the magnitude of the pad input
	float fPadMagnitudeRaw	= fsqrtf( (fYawPadMagnitude*fYawPadMagnitude)+(fPitchPadMagnitude*fPitchPadMagnitude) );
	float fPadMagnitude		= fPadMagnitudeRaw;

	if	( CPlayerOptions::Get().GetInvertY() )	// cam-dwhite
		fPitchPadMagnitude = -fPitchPadMagnitude;

	if (fPadMagnitude >= m_pobAimingControllerDef->m_fPadThreshold)
	{
		fPadMagnitude		= clamp( fPadMagnitude, 0.0f, 1.0f );
		fYawPadMagnitude	= clamp( fYawPadMagnitude, -1.0f, 1.0f );
		fPitchPadMagnitude	= clamp( fPitchPadMagnitude, -1.0f, 1.0f );

		// Clamp the magitude of the pad to a grid. 
		fPadMagnitude		= fPadMagnitude - fmodf( fPadMagnitude, m_pobAimingControllerDef->m_fPadThreshold );
	}
	else 
	{
		// Ignore pad input below specified threshold
		fPadMagnitude		= 0.0f;
	}

	fYawPadMagnitude	= fYawPadMagnitude > 0.0f ? CMaths::SmoothStep(fYawPadMagnitude) : -CMaths::SmoothStep(-fYawPadMagnitude) ;
	fPitchPadMagnitude	= fPitchPadMagnitude > 0.0f ? CMaths::SmoothStep(fPitchPadMagnitude) : -CMaths::SmoothStep(-fPitchPadMagnitude) ;

	// Test values. 
	const float fScaleWidth		= m_pobAimingControllerDef->m_fPadInputScreenSpaceScaleX;
	const float fScaleHeight	= m_pobAimingControllerDef->m_fPadInputScreenSpaceScaleY;
	const float fScaleMag		= fsqrtf( (fScaleWidth * fScaleWidth) + (fScaleHeight * fScaleHeight));

	// It shouldn't clip - but just to be on the safe side... 
	fYawPadMagnitude	= clamp( fYawPadMagnitude * fScaleWidth,	-fScaleWidth,	fScaleWidth );
	fPitchPadMagnitude	= clamp( fPitchPadMagnitude * fScaleHeight,	-fScaleHeight,	fScaleHeight );
	fPadMagnitude		= clamp( fPadMagnitude * fScaleMag,			0.0f,			fScaleMag );

	// World Intersection point
	CPoint	ptIntersect( CONSTRUCT_CLEAR ); 
	CPoint	ptWeaponIntersect( CONSTRUCT_CLEAR ); 
	
	static const float sc_RayCastLength = 200.0f;

	// Get the joint
	int	iIdx = m_pWeaponEnt->GetHierarchy()->GetTransformIndex(m_pobAimingControllerDef->m_hWeaponAimingDirection);

	// If the joint is available...
	if( iIdx >= 0 )
	{
		// Obtain the transform from the entity and use that as a base for the aiming direction.
		const Transform* pAimingTransform = m_pWeaponEnt->GetHierarchy()->GetTransform(iIdx);

		// Set the matrix to use. 
		matAiming = pAimingTransform->GetWorldMatrix();
	}

	// If there is camera and a weapon assigned to the controller
	if( pCam && m_pWeaponEnt )
	{
		// Place the screen space directional input into the camera space. 
		float fVPHeight = pCamView->GetZNear() * tanf( pCamView->GetFOVAngle() * 0.5f ) * 2.0f;
		float fVPWidth = fVPHeight * pCamView->GetAspectRatio();

		// Generate a ray based on the camera view through the 2d point of the user input
		CDirection	dirLocalRay = CDirection( fVPWidth * fYawPadMagnitude, fVPHeight * fPitchPadMagnitude, pCamView->GetZNear() );
		dirLocalRay.Normalise();

		// Create a world ray from the direction created above - this will allow a projection of a
		// point through the world to hit any obsticles
		CDirection	dirWorldRay = dirLocalRay * matWorldCam;
		CPoint		ptRayStart	= matWorldCam.GetTranslation() + (matWorldCam.GetZAxis() * m_pobAimingControllerDef->m_fCamRayStartOffset);
		CPoint		ptRayEnd	= ptRayStart + (dirWorldRay * sc_RayCastLength);

		Physics::RaycastCollisionFlag obFlag; obFlag.base = 0;
		obFlag.flags.i_am           =	Physics::PROJECTILE_RAYCAST_BIT;
		obFlag.flags.i_collide_with =	Physics::LARGE_INTERACTABLE_BIT         |
										Physics::SMALL_INTERACTABLE_BIT         |
										Physics::RAGDOLL_BIT;

		if (!m_pobProjectileProperties || m_pobProjectileProperties->m_CollideWithCharacters != ProjectileProperties::NOT_COLLIDE)
		{
			obFlag.flags.i_collide_with |= Physics::CHARACTER_CONTROLLER_ENEMY_BIT;
		}

		Physics::TRACE_LINE_QUERY stQuery;
		ProjectilePropertiesRaycastFilter filter(m_pobProjectileProperties); 

		bool ret;
		if (m_pobProjectileProperties && m_pobProjectileProperties->m_CollideWithCharacters == ProjectileProperties::COLLIDE_EXACT)
			ret = Physics::CPhysicsWorld::Get().TraceLineExactCharacters(ptRayStart, ptRayEnd, 0.5f, stQuery, obFlag, &filter);
		else
			ret = Physics::CPhysicsWorld::Get().TraceLine(ptRayStart, ptRayEnd, NULL, stQuery, obFlag, &filter);

        if (ret)
		{
			ptIntersect = stQuery.obIntersect;
		}
		else
		{
			ptIntersect = ptRayEnd;
		}


		m_ptIntersect = m_ptIntersect + ((ptIntersect - m_ptIntersect) * fTimeDelta * m_pobAimingControllerDef->m_fCamRaySmootherScalar );

		// Create a test point of impact from the view of the weapon
		CPoint ptWeaponRayStart = matAiming.GetTranslation() + matAiming.GetZAxis();
		ptWeaponIntersect = ptWeaponRayStart + (matAiming.GetZAxis() * sc_RayCastLength);

		// Offset up for the beam 
		CDirection dirWeaponBeamOffset = matAiming.GetYAxis() * m_pobAimingControllerDef->m_fBeamUpOffset;

		if (m_pobProjectileProperties && m_pobProjectileProperties->m_CollideWithCharacters == ProjectileProperties::COLLIDE_EXACT)
			ret = Physics::CPhysicsWorld::Get().TraceLineExactCharacters(ptWeaponRayStart, ptWeaponIntersect, 0.5f, stQuery, obFlag, &filter);
		else
			ret = Physics::CPhysicsWorld::Get().TraceLine(ptWeaponRayStart, ptWeaponIntersect, NULL, stQuery, obFlag, &filter);

        if (ret)
		{
			ptWeaponIntersect = stQuery.obIntersect;

			if( m_pLaser )
			{
				m_pLaser->SetLaserDotPoint( m_ptIntersect );
			}
		}
		else
		{
			//g_VisualDebug->RenderLine( ptWeaponRayStart + dirWeaponBeamOffset, m_ptIntersect + dirWeaponBeamOffset, DC_WHITE, 0);
		}		
	}

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
	float fFireBlendWeight	= 1.0f;

	float fYawBlend = fabsf(m_fYawBlend);

	if (fYawBlend>1.0f)
		fYawBlend=1.0f;

	float fAimingPitch	= m_pobAimingComponent->GetPitch();
	bool  bAimRight		= m_fYawBlend < 0.0f;

	// Only perform the following code if, 1, there is camera; 2, the data asked to, 3 the ent isn't firing
	if( m_pWeaponEnt && m_StateTime > m_pobAimingControllerDef->m_fInputDelay )
	{
		CDirection dirCurrent		= matAiming.GetZAxis();
		CDirection dirRequired		= CDirection(m_ptIntersect - matAiming.GetTranslation());
		dirRequired.Normalise();

		float fDeltaYAngle	= MovementControllerUtilities::RotationAboutY( dirCurrent, dirRequired );
		float fPitch		= asin(dirRequired.Y()) - asin(dirCurrent.Y());
		
		float fOOTime = (1.0f / fTimeDelta);
		float fYawFudge = fOOTime < ( m_pobAimingControllerDef->m_fAimingYawScalar * 1.6f) ? (fOOTime * 0.1f) / m_pobAimingControllerDef->m_fAimingYawScalar : 1.0f;
		float fPitchFudge = fOOTime < ( m_pobAimingControllerDef->m_fAimingPitchScalar * 1.6f) ? (fOOTime * 0.1f) / m_pobAimingControllerDef->m_fAimingPitchScalar : 1.0f;

		// Apply the angle difference with a little smoothing. 
		m_fYawAcc	-= fDeltaYAngle * fTimeDelta * m_pobAimingControllerDef->m_fAimingYawScalar * fYawFudge;
		m_fPitchAcc += fPitch * fTimeDelta * m_pobAimingControllerDef->m_fAimingPitchScalar * fPitchFudge;

		// 
		m_fPitchAcc		= clamp( m_fPitchAcc, m_pobAimingComponent->GetMinPitch(), m_pobAimingComponent->GetMaxPitch() );
		//m_fYawAcc		= clamp( m_fPitchAcc, m_pobAimingComponent->GetMinYaw(), m_pobAimingComponent->GetMaxYaw() );

		// Don't allow any values larger than 0.1
		fYawBlend		= m_fYawAcc;
		fAimingPitch	= m_fPitchAcc;
		bAimRight		= fYawBlend > 0.0f;
	}

	// Make sure the yaw blend value is positive
	fYawBlend = fabs(fYawBlend);

	// Yaw control - Direct control of the players yaw...
	
	// Prevent the anim from modifying the translation
	predictedState.m_obRootDeltaScalar.Clear(); 

	// Prevent the anim from modifying the rotation
	predictedState.m_fRootRotationDeltaScalar	= 0.0f; 

	if( fPadMagnitudeRaw > m_pobAimingControllerDef->m_fCamMovementThreshold )
	{
		// Create a scalar on which to modify the 
		float fScaler = clamp( fPadMagnitudeRaw - m_pobAimingControllerDef->m_fCamMovementThreshold, 0.01f, 1.0f );

		// This line was copied from the old version of this function, it's gives designers some power to modifiy the controlling of the yaw.. 
		fScaler		= powf( fScaler, m_pobAimingControllerDef->m_fConstMagnitudePower ) + m_pobAimingControllerDef->m_fConstMagnitudeModifier * fScaler;
		fScaler		= fScaler * fTimeDelta * m_pobAimingControllerDef->m_fMovingYawScalar;

		// Set the new yaw value
		predictedState.m_fProceduralYaw = m_pobAimingComponent->AdjustYaw(fYawPadMagnitude, fScaler);

		// Adjust the PITCH
		m_pobAimingComponent->AdjustPitch(fPitchPadMagnitude, fTimeDelta);
		
		// Only adjust the pitch if there is a camera
		if(pCam)
		{
			// Set the aiming pitch of the camera, we don't bother with yaw since it is derived from the character yaw
			pCam->SetPitch(-m_pobAimingComponent->GetPitch()); 
		}
	}

	float fInverseYawBlend		= 1.0f - fYawBlend;
	float fInversePitchBlend	= 0.0f;
	float fPitchBlend			= 0.0f;

	if( fAimingPitch < 0.0f ) // Looking down
	{
		fPitchBlend			= fabsf( fAimingPitch / m_pobAimingComponent->GetMinPitch() );
		fPitchBlend			= clamp( fPitchBlend, -1.0f, 1.0f );
		fInversePitchBlend	= 1.0f - fPitchBlend;

		fCentreDownBlend	= fPitchBlend * fInverseYawBlend;
		fCentreMidBlend		= fInversePitchBlend * fInverseYawBlend;

		if ( bAimRight ) // Looking right
		{
			fRightDownBlend	= fPitchBlend * fYawBlend;
			fRightMidBlend	= fInversePitchBlend * fYawBlend;
		}
		else // Looking left
		{
			fLeftDownBlend	= fPitchBlend * fYawBlend;
			fLeftMidBlend	= fInversePitchBlend * fYawBlend;
		}
	}
	else // Looking up
	{
		fPitchBlend			= fabsf( fAimingPitch / m_pobAimingComponent->GetMaxPitch() );
		fPitchBlend			= clamp( fPitchBlend, -1.0f, 1.0f );
		fInversePitchBlend	= 1.0f - fPitchBlend;

		fCentreUpBlend	= fPitchBlend * fInverseYawBlend;
		fCentreMidBlend	= fInversePitchBlend * fInverseYawBlend;

		if ( bAimRight ) // Looking right
		{
			fRightUpBlend	= fPitchBlend * fYawBlend;
			fRightMidBlend	= fInversePitchBlend * fYawBlend;
		}
		else // Looking left
		{
			fLeftUpBlend	= fPitchBlend * fYawBlend;
			fLeftMidBlend	= fInversePitchBlend * fYawBlend;
		}
	}

	if( m_bHasFiringAnims && m_FireTime >= 0.0f )
	{
		static const float FIRING_PHASE_IN = 0.25f;
		float fFireBlend = 0.0f;

		if( m_FireTime >= m_obAnims[ UpFireAnimation ]->GetDuration() )
		{
			m_FireTime = -1.0f;
		}
		else if( m_FireTime <= FIRING_PHASE_IN )
		{
			fFireBlend = CMaths::SmoothStep( m_FireTime / FIRING_PHASE_IN );
		}
		else
		{
			fFireBlend = 1.0f - CMaths::SmoothStep( (m_FireTime - FIRING_PHASE_IN) / m_obAnims[UpFireAnimation]->GetDuration() );
		}

		m_obAnims[ UpFireAnimation ]->SetBlendWeight( fFireBlend * fCentreUpBlend );
		m_obAnims[ CentreFireAnimation ]->SetBlendWeight( fFireBlend * fCentreMidBlend );
		m_obAnims[ DownFireAnimation ]->SetBlendWeight( fFireBlend * fCentreDownBlend );

		fFireBlendWeight = 1.0f - fFireBlend;
		m_FireTime += fTimeDelta;
	}

	m_obAnims[LeftUpAnimation]->SetBlendWeight(fLeftUpBlend);
	m_obAnims[LeftMidAnimation]->SetBlendWeight(fLeftMidBlend);
	m_obAnims[LeftDownAnimation]->SetBlendWeight(fLeftDownBlend);
	m_obAnims[CentreUpAnimation]->SetBlendWeight(fCentreUpBlend * fFireBlendWeight);
	m_obAnims[CenterMidAnimation]->SetBlendWeight(fCentreMidBlend * fFireBlendWeight);
	m_obAnims[CenterDownAnimation]->SetBlendWeight(fCentreDownBlend * fFireBlendWeight);
	m_obAnims[RightUpAnimation]->SetBlendWeight(fRightUpBlend);
	m_obAnims[RightMidAnimation]->SetBlendWeight(fRightMidBlend);
	m_obAnims[RightDownAnimation]->SetBlendWeight(fRightDownBlend);

	// If the shot is now all charged up, then change the FOV
	if( pCam && !m_bShotCharged && m_pPlayer->IsArcher() && m_pPlayer->ToArcher()->SuperShot() )
	{
		pCam->SetFoV( m_pobAimingControllerDef->m_fChargedFOV, m_pobAimingControllerDef->m_fChargedFOVTime );
		m_bShotCharged = true;
	}
	else if( pCam && m_bShotCharged && m_pPlayer->IsArcher() && !m_pPlayer->ToArcher()->SuperShot() )
	{
		pCam->RestoreFoV( m_pobAimingControllerDef->m_fChargedFOVTime );
		m_bShotCharged = false;
	}


	if (m_obAnims[BobAnimation])
	{
		//m_obBobAnimation->SetSpeed(fYawBlend);
		//m_obBobAnimation->SetBlendWeight(fYawBlend);
	}

	m_StateTime += fTimeDelta;
	m_bFirstFrame = false;

	return true;
}



//------------------------------------------------------------------------------------------
//!
//!	AimingLaunchControllerDef::AimingLaunchControllerDef
//!	Construction
//!
//------------------------------------------------------------------------------------------
AimingLaunchControllerDef::AimingLaunchControllerDef( void ) :
	m_fYawSpeedMultiplier(1.0f),
	m_fPitchSpeedMultiplier(1.0f),
	m_fPadThreshold(0.05f),
	m_fAccelerationFactor(0.025f),
	m_fDeaccelerationFactor(0.75f),
	m_bAllowMovement(false)
{
}


//------------------------------------------------------------------------------------------
//!
//!	AimingLaunchControllerDef::CreateInstance
//!	
//!
//------------------------------------------------------------------------------------------
MovementController* AimingLaunchControllerDef::CreateInstance(CMovement* pobMovement) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) AimingLaunchController(pobMovement, this);
}


//------------------------------------------------------------------------------------------
//!
//!	AimingLaunchController::AimingLaunchController
//!	Construction
//!
//------------------------------------------------------------------------------------------
AimingLaunchController::AimingLaunchController (CMovement* pMovement, const AimingLaunchControllerDef* pDefinition) :
	MovementController(pMovement)
{
	ntAssert(pMovement);
	ntAssert(pDefinition);

	m_obCentreUpAnimation = m_pobAnimator->CreateAnimation( pDefinition->m_obCentreUpAnimation );
	m_obCentreUpAnimation->SetFlagBits( ANIMF_LOCOMOTING | ANIMF_INHIBIT_AUTO_DESTRUCT );
	m_obCentreUpAnimation->SetBlendWeight( 0.0f );
	
	m_obCenterMidAnimation = m_pobAnimator->CreateAnimation( pDefinition->m_obCenterMidAnimation );
	m_obCenterMidAnimation->SetFlagBits( ANIMF_LOCOMOTING | ANIMF_INHIBIT_AUTO_DESTRUCT );
	m_obCenterMidAnimation->SetBlendWeight( 0.0f );
	
	m_obCenterDownAnimation = m_pobAnimator->CreateAnimation( pDefinition->m_obCenterDownAnimation );
	m_obCenterDownAnimation->SetFlagBits( ANIMF_LOCOMOTING | ANIMF_INHIBIT_AUTO_DESTRUCT );
	m_obCenterDownAnimation->SetBlendWeight( 0.0f );

	CEntity* pobParentEntity=(CEntity*)pMovement->GetParentEntity();

	AimingComponent* pobAimingComponent=(AimingComponent*)pobParentEntity->GetAimingComponent();
	ntAssert(pobAimingComponent);

	float fPitchBlend=fabsf(pobAimingComponent->GetPitch()/pobAimingComponent->GetMaxPitch());

	if(pobAimingComponent->GetPitch()<0.0f) // Looking down
	{
		m_fCentreDownBlend=fPitchBlend;
		m_fCentreMidBlend=1.0f-fPitchBlend;
		m_fCentreUpBlend=0.0f;
	}
	else
	{
		m_fCentreUpBlend=fPitchBlend;
		m_fCentreMidBlend=1.0f-fPitchBlend;
		m_fCentreDownBlend=0.0f;
	}
}


//------------------------------------------------------------------------------------------
//!
//!	AimingLaunchController::AimingLaunchController
//!	Construction
//!
//------------------------------------------------------------------------------------------
AimingLaunchController::~AimingLaunchController ()
{
	if ( !m_bFirstFrame )
	{
		m_pobAnimator->RemoveAnimation(m_obCentreUpAnimation);
		m_pobAnimator->RemoveAnimation(m_obCenterMidAnimation);
		m_pobAnimator->RemoveAnimation(m_obCenterDownAnimation);
	}
}


//------------------------------------------------------------------------------------------
//!
//!	AimingLaunchController::Update
//!	
//!
//------------------------------------------------------------------------------------------
bool AimingLaunchController::Update(float fTimeDelta, const CMovementInput& /*input*/, const CMovementStateRef& /*currentState*/, CMovementState& predictedState)
{
	// Only add the animations to the animator on the first update
	if ( m_bFirstFrame )
	{
		m_pobAnimator->AddAnimation( m_obCentreUpAnimation );
		m_pobAnimator->AddAnimation( m_obCenterMidAnimation );
		m_pobAnimator->AddAnimation( m_obCenterDownAnimation );
		m_bFirstFrame = false;
	}

	m_obCentreUpAnimation->SetBlendWeight( m_fCentreUpBlend * m_fBlendWeight );
	m_obCenterMidAnimation->SetBlendWeight( m_fCentreMidBlend * m_fBlendWeight );
	m_obCenterDownAnimation->SetBlendWeight( m_fCentreDownBlend * m_fBlendWeight );

	predictedState.m_obRootDeltaScalar.Clear(); // Prevent the anim from modifying the translation
	predictedState.m_fRootRotationDeltaScalar=0.0f; // Prevent the anim from modifying the rotation

	if ( m_obCentreUpAnimation->GetTime() > ( m_obCentreUpAnimation->GetDuration() - fTimeDelta ) &&
		 m_obCenterMidAnimation->GetTime() > ( m_obCenterMidAnimation->GetDuration() - fTimeDelta ) &&
		 m_obCenterDownAnimation->GetTime() > ( m_obCenterDownAnimation->GetDuration() - fTimeDelta ) )
	{
		return true;
	}

	return false;
}


//------------------------------------------------------------------------------------------
//!
//!	AimingComponent::AimingComponent
//!	Construction
//!
//------------------------------------------------------------------------------------------
AimingComponent::AimingComponent (CEntity* pobParentEntity,float fPitchSpeed,float fYawSpeed,float fMinPitch,float fMaxPitch) :
	CAnonymousEntComponent("AimingComponent"),
	m_pobEntity(pobParentEntity),
	m_fPitch(0.0f),
	m_fYaw(0.0f),
	m_fPitchSpeed(fPitchSpeed * DEG_TO_RAD_VALUE),
	m_fYawSpeed(fYawSpeed * DEG_TO_RAD_VALUE),
	m_fDefaultMinPitch(fMinPitch * DEG_TO_RAD_VALUE),
	m_fDefaultMaxPitch(fMaxPitch * DEG_TO_RAD_VALUE),
	m_fDefaultMinYaw(-TWO_PI),
	m_fDefaultMaxYaw(TWO_PI),
	m_fMinPitch(fMinPitch * DEG_TO_RAD_VALUE),
	m_fMaxPitch(fMaxPitch * DEG_TO_RAD_VALUE),
	m_fMinYaw(-TWO_PI),
	m_fMaxYaw(TWO_PI),
	m_ptResetAnglePoint(CONSTRUCT_CLEAR)
{

}


//------------------------------------------------------------------------------------------
//!
//!	AimingComponent::SetFromParent
//!	
//!
//------------------------------------------------------------------------------------------
void AimingComponent::SetFromParent ()
{
	// Derive the yaw from the parent entity world matrix

	const CMatrix& obWorldMatrix=m_pobEntity->GetMatrix();

	m_fYaw		= fatan2f(obWorldMatrix.GetZAxis().X(), obWorldMatrix.GetZAxis().Z());
	m_fYaw		= fmodf( m_fYaw, PI );
	m_fPitch	= 0.0f;
}

//------------------------------------------------------------------------------------------
//!
//!	AimingComponent::AdjustPitch
//!	Move up/down - return the pitch adjustment in radians
//!
//------------------------------------------------------------------------------------------
float AimingComponent::AdjustPitch (float fDelta,float fTimeStep)
{
	float fPitchStep = fDelta * fTimeStep;
	float fOrigPitch = m_fPitch;

	m_fPitch += fPitchStep;
	m_fPitch = fmodf( m_fPitch, TWO_PI );
	m_fPitch = clamp( m_fPitch, m_fMinPitch, m_fMaxPitch );

	fPitchStep = m_fPitch - fOrigPitch;

	return fPitchStep;
}


//------------------------------------------------------------------------------------------
//!
//!	AimingComponent::AdjustYaw
//!	
//!
//------------------------------------------------------------------------------------------
float AimingComponent::AdjustYaw (float fDelta,float fTimeStep)
{
	float fYawStep = fDelta * fTimeStep;
	float fOrigYaw = m_fYaw;

	m_fYaw += fYawStep;
	m_fYaw = fmodf( m_fYaw, TWO_PI );
	m_fYaw = clamp( m_fYaw, m_fMinYaw, m_fMaxYaw );

	float fNewYawStep = m_fYaw - fOrigYaw;

	// If the yaw steps don't both point in the same direction, then activate the wrap trap.
	if( (fNewYawStep >= 0.0f) ^ (fYawStep >= 0.0f) )
		fNewYawStep = fNewYawStep + ((fYawStep >= 0.0f) ? TWO_PI : -TWO_PI);

	// Return the new yaw step
	return fNewYawStep;
}

//------------------------------------------------------------------------------------------
//!
//!	AimingComponent::SetAimRange
//!	
//! Parameters in degrees from Z axis - as I belive this will be the best definition for 
//! the designers to visulise
//!
//------------------------------------------------------------------------------------------
void AimingComponent::SetAimRange ( float fMinYaw, float fMaxYaw, float fMinPitch, float fMaxPitch)
{
	ntAssert( fMinYaw < fMaxYaw );
	ntAssert( fMinPitch < fMaxPitch );

	m_fMinPitch = fMinPitch * DEG_TO_RAD_VALUE;
	m_fMaxPitch	= fMaxPitch * DEG_TO_RAD_VALUE;
	m_fMinYaw   = fMinYaw   * DEG_TO_RAD_VALUE; 
	m_fMaxYaw   = fMaxYaw   * DEG_TO_RAD_VALUE;	

	// Just incase the current values are outside of the new range
	m_fYaw = 0.0f;
	//m_fPitch = 0.0f;
}

//------------------------------------------------------------------------------------------
//!
//!	AimingComponent::ResetAimRange
//!	
//!
//------------------------------------------------------------------------------------------
void AimingComponent::ResetAimRange ( )
{
	m_fMinPitch = m_fDefaultMinPitch;
	m_fMaxPitch	= m_fDefaultMaxPitch;
	m_fMinYaw   = m_fDefaultMinYaw;
	m_fMaxYaw   = m_fDefaultMaxYaw;	
}

//------------------------------------------------------------------------------------------
//!  public constructor  AimingWeaponControllerDef
//!
//!  @author GavB @date 17/10/2006
//------------------------------------------------------------------------------------------
AimingWeaponControllerDef::AimingWeaponControllerDef(void)
{
	m_pPlayer = 0;
}

//------------------------------------------------------------------------------------------
//!  public virtual constant  CreateInstance
//!
//!  @param [in, out]  pobMovement CMovement *    
//!
//!  @return MovementController * 
//!
//!  @author GavB @date 17/10/2006
//------------------------------------------------------------------------------------------
MovementController* AimingWeaponControllerDef::CreateInstance(CMovement* pobMovement) const
{
	// Return the instance of the new controller
	return NT_NEW_CHUNK(Mem::MC_ENTITY) AimingWeaponController(pobMovement, this);
}

//------------------------------------------------------------------------------------------
//!
//!	AimingController::AimingController
//!	Construction
//!
//------------------------------------------------------------------------------------------
AimingWeaponController::AimingWeaponController (CMovement* pMovement, const AimingWeaponControllerDef* pDefinition ) :
	MovementController(pMovement)
{
	ntAssert(pMovement);
	ntAssert(pDefinition);

	const float fAnimSpeed = 1.0f;
	const int	iAnimFlags = ANIMF_LOCOMOTING | ANIMF_INHIBIT_AUTO_DESTRUCT;

	// Set up all the animations - NEVER add them to the animator in the contstructor
	m_obAnims[ IDLE ]				= m_pobAnimator->CreateAnimation(pDefinition->m_obIdle);
	m_obAnims[ ARM_1_CHARGE_UP ]	= m_pobAnimator->CreateAnimation(pDefinition->m_obArm1ChargeUp);
	m_obAnims[ ARM_2_CHARGE_UP ]	= m_pobAnimator->CreateAnimation(pDefinition->m_obArm2ChargeUp);
	m_obAnims[ ARM_1_CHARGED ]		= m_pobAnimator->CreateAnimation(pDefinition->m_obArm1Charged);
	m_obAnims[ ARM_2_CHARGED ]		= m_pobAnimator->CreateAnimation(pDefinition->m_obArm2Charged);
	m_obAnims[ ARM_1_FIRED ]		= m_pobAnimator->CreateAnimation(pDefinition->m_obArm1Firing);
	m_obAnims[ ARM_2_FIRED ]		= m_pobAnimator->CreateAnimation(pDefinition->m_obArm2Firing);


	// Initialise the animations.
	for( int iAnimIndex = 0; iAnimIndex < ANIM_COUNT; ++iAnimIndex )
	{
		if( m_obAnims[ iAnimIndex ] )
		{
			m_obAnims[ iAnimIndex ]->SetSpeed(fAnimSpeed);
			m_obAnims[ iAnimIndex ]->SetBlendWeight(0.0f);
			m_obAnims[ iAnimIndex ]->SetFlagBits(iAnimFlags);
		}
	}

	// Mark the charged state as looping. 
	m_obAnims[ ARM_1_CHARGED ]->SetFlagBits(iAnimFlags | ANIMF_LOOPING );
	m_obAnims[ ARM_2_CHARGED ]->SetFlagBits(iAnimFlags | ANIMF_LOOPING );

	// Set the player
	m_pPlayer = pDefinition->m_pPlayer;

}


//------------------------------------------------------------------------------------------
//!  public virtual destructor  ~AimingWeaponController
//!
//!  @author GavB @date 17/10/2006
//------------------------------------------------------------------------------------------
AimingWeaponController::~AimingWeaponController ()
{
	if ( !m_bFirstFrame )
	{
		// Remove the animations.
		for( int iAnimIndex = 0; iAnimIndex < ANIM_SET_COUNT; ++iAnimIndex )
		{
			if( m_obActiveAnims[ iAnimIndex ] )
			{
				m_pobAnimator->RemoveAnimation( m_obActiveAnims[ iAnimIndex ] );
			}
		}
	}
}

//------------------------------------------------------------------------------------------
//!  public virtual  Update
//!
//!  @param [in]       fTimeDelta float     
//!  @param [in]       input const CMovementInput &    
//!  @param [in]       currentState const CMovementStateRef &    
//!  @param [in, out]  predictedState CMovementState &    
//!
//!  @return bool 
//!
//!  @author GavB @date 17/10/2006
//------------------------------------------------------------------------------------------
bool AimingWeaponController::Update(float fTimeDelta, const CMovementInput&, const CMovementStateRef&, CMovementState&)
{
	if( m_pPlayer && m_pPlayer->IsPlayer() )
	{
		const Player* pPlayer = m_pPlayer->ToPlayer();

		// Update the archers specialised weapon controller
		if( pPlayer->IsArcher() )
		{
			UpdateArcher( fTimeDelta, pPlayer->ToArcher() );
		}
	}

	m_bFirstFrame = false;

	return true;
}

//------------------------------------------------------------------------------------------
//!  public  UpdateArcher
//!
//!
//!  @author GavB @date 05/12/2006
//------------------------------------------------------------------------------------------
void AimingWeaponController::UpdateArcher( float fTimeStep, const Archer* pArcher )
{
	// Add the necessary animations to the aniamtor
	if ( m_bFirstFrame )
	{
		m_obActiveAnims[ BASE_ANIM ]		= m_obAnims[ IDLE ];
		m_obActiveAnims[ PARTIAL_ANIM_1 ]	= m_obAnims[ ARM_1_CHARGE_UP ];
		m_obActiveAnims[ PARTIAL_ANIM_2 ]	= m_obAnims[ ARM_2_CHARGE_UP ];

		// Add the animations to the animator
		if( m_obActiveAnims[ BASE_ANIM ] )
			m_pobAnimator->AddAnimation( m_obActiveAnims[ BASE_ANIM ] );

		if( m_obActiveAnims[ PARTIAL_ANIM_1 ] )
			m_pobAnimator->AddAnimation( m_obActiveAnims[ PARTIAL_ANIM_1 ] );
		
		if( m_obActiveAnims[ PARTIAL_ANIM_2 ] )
			m_pobAnimator->AddAnimation( m_obActiveAnims[ PARTIAL_ANIM_2 ] );
	}

	// Set the blend weights for all the anims.. 
	m_obActiveAnims[ BASE_ANIM ]->SetBlendWeight( m_fBlendWeight );
	m_obActiveAnims[ PARTIAL_ANIM_1 ]->SetBlendWeight( m_fBlendWeight );
	m_obActiveAnims[ PARTIAL_ANIM_2 ]->SetBlendWeight( m_fBlendWeight );


	// Update each of the xbow arms
	for( int iIndex = 0; iIndex < XBOW_ARM_COUNT; ++iIndex )
	{
		// Cache a copy of what the charge used to be. 
		float fOldCharge = m_aobArms[iIndex].m_fChargeState;

		// Get the current charge on the xbow arm
		float fCharge = pArcher->GetXbowArmCharges( (u_int) iIndex );

		// Is the xbow state in the can't fire state?
		if( ((fOldCharge < 0.0f) &&( fCharge >= 0.0f)) || m_bFirstFrame )
		{
			// Remove the old animation... 
			m_pobAnimator->RemoveAnimation( m_obActiveAnims[ PARTIAL_ANIM_1 + iIndex ] );

			// Start the charge up animation for the current arm on the xbow. 
			m_obActiveAnims[ PARTIAL_ANIM_1 + iIndex ] = m_obAnims[ ARM_1_CHARGE_UP + iIndex ];
			m_obActiveAnims[ PARTIAL_ANIM_1 + iIndex ]->SetBlendWeight( m_fBlendWeight );

			m_pobAnimator->AddAnimation( m_obActiveAnims[ PARTIAL_ANIM_1 + iIndex ] );
		}
		else if( ( fCharge >= 0.0f ) && ( fCharge < 1.0f ) )
		{
			// Set the percentage through the animation
			m_obActiveAnims[ PARTIAL_ANIM_1 + iIndex ]->SetPercentage( fCharge );
		}

		// If the xbow is now fully charged... 
		else if( ( fOldCharge < 1.0f ) && ( fCharge > 1.0f ) )
		{
			// Remove the old animation... 
			m_pobAnimator->RemoveAnimation( m_obActiveAnims[ PARTIAL_ANIM_1 + iIndex ] );

			// Start the charged animation for the current arm on the xbow. 
			m_obActiveAnims[ PARTIAL_ANIM_1 + iIndex ] = m_obAnims[ ARM_1_CHARGED + iIndex ];
			m_obActiveAnims[ PARTIAL_ANIM_1 + iIndex ]->SetBlendWeight( m_fBlendWeight );

			m_pobAnimator->AddAnimation( m_obActiveAnims[ PARTIAL_ANIM_1 + iIndex ] );
		}

		// If the xbow is fired a shot... 
		else if( ( fOldCharge > 0.0f ) && ( fCharge < 0.0f ) )
		{
			// Remove the old animation... 
			m_pobAnimator->RemoveAnimation( m_obActiveAnims[ PARTIAL_ANIM_1 + iIndex ] );

			// Start the charged animation for the current arm on the xbow. 
			m_obActiveAnims[ PARTIAL_ANIM_1 + iIndex ] = m_obAnims[ ARM_1_FIRED + iIndex ];
			m_obActiveAnims[ PARTIAL_ANIM_1 + iIndex ]->SetBlendWeight( m_fBlendWeight );

			m_pobAnimator->AddAnimation( m_obActiveAnims[ PARTIAL_ANIM_1 + iIndex ] );
		}
		else if(  fCharge < 0.0f  )
		{

		}

		// Record the charged state
		m_aobArms[iIndex].m_fChargeState = fCharge;

		// Increase the counter signaling whether the xbow can shoot another shot. 
		//iCanShoot += m_aobArms[iIndex].m_fChargeState >= 0.0f ? 1 : 0;

		// Debug
#ifndef _GOLD_MASTER
		g_VisualDebug->Printf2D( 10.0f, 10.0f + ((float)iIndex * 14.0f), DC_WHITE, 0, "Charge: %f", m_aobArms[iIndex].m_fChargeState );
#endif
	}


}

//------------------------------------------------------------------------------------------
//!  public constructor  AimingLaserEffectDef
//!
//!
//!  @author GavB @date 20/11/2006
//------------------------------------------------------------------------------------------
AimingLaserEffectDef::AimingLaserEffectDef()
{
	m_fDotSize					= 0.05f;
	m_fDotLuminosity			= 1.0f;
	m_fCameraAdjust				= 1.0f / 20.0f;
	m_fDotLuminosityFallOff		= 1.0f / 20.0f;
	m_obDotColour				= CVector(CONSTRUCT_CLEAR);
	//m_obDotTexture		= "awesome.dds";
}

//------------------------------------------------------------------------------------------
//!  public constructor  AimingLaserEffect
//!
//!  @param [in]        const AimingLaserEffectDef * 
//!
//!
//!  @author GavB @date 20/11/2006
//------------------------------------------------------------------------------------------
AimingLaserEffect::AimingLaserEffect( const AimingLaserEffectDef* pLaserDef ) :
	m_pDef ( pLaserDef )
{
	// By default 
	m_EnableLaserDot = false;

	// Set the texture
	m_DotSprite.SetTexture( pLaserDef->m_obDotTexture.c_str() );
	m_DotSprite.SetSizeAll( pLaserDef->m_fDotSize );
	//m_DotSprite.SetUV( 0.0f, 0.0f, 1.0f, 1.0f );

	// Add the effect and obtain the ID
	m_EffectID = EffectManager::Get().AddEffect( this );
}

//------------------------------------------------------------------------------------------
//!  public virtual destructor  ~AimingLaserEffect
//!
//!
//!  @author GavB @date 21/11/2006
//------------------------------------------------------------------------------------------
AimingLaserEffect::~AimingLaserEffect(void)
{
}

//------------------------------------------------------------------------------------------
//!  public virtual  UpdateEffect
//!
//!  @return bool 
//!
//!  @author GavB @date 20/11/2006
//------------------------------------------------------------------------------------------
bool AimingLaserEffect::UpdateEffect()
{
	return false;
}

//------------------------------------------------------------------------------------------
//!  public virtual  RenderEffect
//!
//!
//!  @author GavB @date 20/11/2006
//------------------------------------------------------------------------------------------
void AimingLaserEffect::RenderEffect()
{

	if( m_EnableLaserDot )
	{
		Renderer::Get().SetBlendMode( GFX_BLENDMODE_ADD );

		// Distance from the dot to the camera
		float fDist = (RenderingContext::Get()->m_pViewCamera->GetViewTransform()->GetWorldMatrix().GetTranslation() - m_ptDot).Length();

		// Adjust the position of the dot
		m_ptDot += RenderingContext::Get()->m_pViewCamera->GetViewTransform()->GetWorldMatrix().GetZAxis() * m_pDef->m_fCameraAdjust;

		// Set the position of the dot. 
		m_DotSprite.SetPosition( m_ptDot );

		// Adjust the size of the laser dot based on the difference in Z distance. 
		m_DotSprite.SetSizeAll( m_pDef->m_fDotSize + (fDist * m_pDef->m_fDotLuminosityFallOff) );
		
		// Create a color
		CVector	vColour = m_pDef->m_obDotColour * m_pDef->m_fDotLuminosity;
		vColour.W() = 1.0f;
		
		// Set the colour
		m_DotSprite.SetColourAll( vColour );

		// Render that sucker...
		m_DotSprite.Render();

		// Don't render the laser anymore. The owner will have to ask for the render again by setting the 
		// position of the dot. 
		m_EnableLaserDot = false;
	}

	Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );
}

