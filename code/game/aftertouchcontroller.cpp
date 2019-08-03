//------------------------------------------------------------------------------------------
//!
//!	\file AimController.cpp
//!
//------------------------------------------------------------------------------------------

#include "physics/havokincludes.h"
#include "Physics/config.h"
#include "Physics/system.h"
#include "Physics/singlerigidlg.h"
#include "Physics/spearlg.h"
#include "Physics/projectilelg.h"

//#include "maths.h"
#include "camera/coolcam_aftertouch.h"

/////////////////////////////////
// Required Includes
/////////////////////////////////
#include "aftertouchcontroller.h"
#include "camera/camman.h"
#include "camera/camview.h"
#include "camera/camutils.h"
#include "core/timer.h"
#include "objectdatabase/dataobject.h"
#include "anim/animator.h"
#include "game/entitymanager.h"
#include "camera/camerainterface.h"
#include "camera/coolcam_chaseaimcombo.h"
#include "game/luaglobal.h"
#include "game/entityinfo.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "game/movement.h"
#include "game/messagehandler.h"
#include "physics/world.h" // Raycast stuff
#include "inputcomponent.h"
#include "game/playeroptions.h"

#ifdef PLATFORM_PS3
#include "code_ps3/input/inputhardware_ps3.h"
#endif

/////////////////////////////////
// Debug Includes
/////////////////////////////////
//#include "core/visualdebugger.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include <hkvisualize/hkDebugDisplay.h>
#endif


START_STD_INTERFACE(AftertouchControlParameters)
	IFLOAT(AftertouchControlParameters,		MinPadMagnitude)
	IFLOAT(AftertouchControlParameters,		YawAdjustFactor)
	IFLOAT(AftertouchControlParameters,		PitchAdjustFactor)
	IFLOAT(AftertouchControlParameters,		MaxYawSpeed)
	IFLOAT(AftertouchControlParameters,		MaxPitchSpeed)
	IFLOAT(AftertouchControlParameters,		MaxUpwardsPitchAngle)
	IFLOAT(AftertouchControlParameters,		MaxDownwardsPitchAngle)
	IFLOAT(AftertouchControlParameters,		MaxRollAngle)
	IFLOAT(AftertouchControlParameters,		ControlFadeIn)
	IFLOAT(AftertouchControlParameters,		ControlHold)
	IFLOAT(AftertouchControlParameters,		ControlFadeOut)

	// PS3 Motion Sensor values
	IFLOAT(AftertouchControlParameters,		MotionSensorMinPadMagnitude)
	IFLOAT(AftertouchControlParameters,		MotionSensorMaxPadMagnitude)
	IFLOAT(AftertouchControlParameters,		MotionSensorYawAdjustFactor)
	IFLOAT(AftertouchControlParameters,		MotionSensorPitchAdjustFactor)
	IFLOAT(AftertouchControlParameters,		MotionSensorMaxYawSpeed)
	IFLOAT(AftertouchControlParameters,		MotionSensorMaxPitchSpeed)
	IFLOAT(AftertouchControlParameters,		MotionSensorMaxUpwardsPitchAngle)
	IFLOAT(AftertouchControlParameters,		MotionSensorMaxDownwardsPitchAngle)
	IFLOAT(AftertouchControlParameters,		MotionSensorMaxRollAngle)
	IFLOAT(AftertouchControlParameters,		MotionSensorControlFadeIn)
	IFLOAT(AftertouchControlParameters,		MotionSensorControlHold)
	IFLOAT(AftertouchControlParameters,		MotionSensorControlFadeOut)
END_STD_INTERFACE









//------------------------------------------------------------------------------------------------------------------------------------------------


AfterTouchControllerDef::AfterTouchControllerDef(void) :
	m_pobControllingEntity(0),
	m_pobParameters(0)
{
}

MovementController* AfterTouchControllerDef::CreateInstance(CMovement* pobMovement) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) AfterTouchController( pobMovement, this );
}



//------------------------------------------------------------------------------------------------------------------------------------------------



AfterTouchController::AfterTouchController (CMovement* pMovement, const AfterTouchControllerDef* pDefinition) :
	MovementController( pMovement ),
	m_pobControllingEntity( pDefinition->m_pobControllingEntity ),
	m_pobParameters( pDefinition->m_pobParameters ),
	m_fTime( 0.0f ),
	m_fYawDelta( 0.0f ),
	m_fPitchDelta( 0.0f )
{
	ntAssert(pMovement);
	ntAssert(m_pobControllingEntity);
	ntAssert(m_pobParameters);
}

AfterTouchController::~AfterTouchController ()
{
}

bool AfterTouchController::Update(float /*fTimeDelta*/, const CMovementInput& /*input*/, const CMovementStateRef& /*currentState*/, CMovementState& /*predictedState*/)
{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

	Physics::System* system = this->m_pobMovement->GetPhysicsSystem();

	if( 0 == system )
		return true;

	Physics::SingleRigidLG* srlg = (Physics::SingleRigidLG*) system->GetFirstGroupByType( Physics::LogicGroup::SINGLE_RIGID_BODY_LG );

	if (!srlg)
		return true;

	if( (srlg->GetLinearVelocity().Length() + srlg->GetAngularVelocity( ).Length()) < 0.1f )
		return true;

	// No aftertouch movement slowdown.  Independent of game slow down.
	const float fTimeDelta = CTimer::Get().GetSystemTimeChange();
	m_fTime += CTimer::Get().GetGameTimeChange();

	// Try to sync with the consistent stepping done by physics world  - DGF
	//const float fTimeDelta = fPHYSICS_WANTED_STEP_SPEED * CTimer::Get().GetGameTimeScalar(); //CTimer::Get().GetSystemTimeChange(); // Use absolute time, so that control doesnt feel different when time is slowed down

	const CInputComponent* pobInput=m_pobControllingEntity->GetInputComponent();
	const CDirection obPadDirection=pobInput->GetInputDir();
	float fPadMagnitude=pobInput->GetInputSpeed();

	CDirection obInputDirection=obPadDirection * CamMan::GetPrimaryView()->GetCurrMatrix().GetAffineInverse();

	// Do not apply aftertouch is object is heading towards camera!
	CDirection camZAxis = CamMan::GetPrimaryView()->GetCurrMatrix().GetZAxis();
	CDirection objDir = srlg->GetLinearVelocity();
	float fDotProduct	= camZAxis.X() * objDir.X() + camZAxis.Y() * objDir.Y() + camZAxis.Z() * objDir.Z();
	if (fDotProduct < 0.0f)
	{
		return true;
	}

	// Scale the pad magnitude
	
	if (fPadMagnitude >= m_pobParameters->m_fMinPadMagnitude)
	{
		fPadMagnitude = (fPadMagnitude - m_pobParameters->m_fMinPadMagnitude) / (1.0f - m_pobParameters->m_fMinPadMagnitude);
	}
	else
	{
		fPadMagnitude = 0.0f;
	}

	//ntPrintf("pad magnitude=%f\n",fPadMagnitude);

	// ----- Control multiplier -----

	float fControlMultiplier = 1.0f;

	if (m_fTime>(m_pobParameters->m_fControlFadeIn+m_pobParameters->m_fControlHold+m_pobParameters->m_fControlFadeOut)) // Control has expired
	{
		fControlMultiplier = 0.0f;
	}
	else if (m_fTime<m_pobParameters->m_fControlFadeIn)
	{
		fControlMultiplier = m_fTime/m_pobParameters->m_fControlFadeIn;
	}
	else if (m_fTime>(m_pobParameters->m_fControlFadeIn+m_pobParameters->m_fControlHold))
	{
		fControlMultiplier = 1.0f - ((m_fTime - m_pobParameters->m_fControlFadeIn - m_pobParameters->m_fControlHold) / m_pobParameters->m_fControlFadeOut);
	}

	fPadMagnitude *= fControlMultiplier;

	//ntPrintf("Control=%f\n",fControlMultiplier);

	// ----- Yaw control -----

	float fDesiredYawDelta = obInputDirection.X() * fPadMagnitude;

	if (fDesiredYawDelta>1.0f)
		fDesiredYawDelta=1.0f;

	if (fDesiredYawDelta<-1.0f)
		fDesiredYawDelta=-1.0f;

	fDesiredYawDelta *= m_pobParameters->m_fMaxYawSpeed * DEG_TO_RAD_VALUE;

	m_fYawDelta += (fDesiredYawDelta-m_fYawDelta) * m_pobParameters->m_fYawAdjustFactor;

	// ----- Pitch control -----

	float fDesiredPitchDelta = obInputDirection.Z() * fPadMagnitude;

	if (fDesiredPitchDelta>1.0f)
		fDesiredPitchDelta=1.0f;

	if (fDesiredPitchDelta<-1.0f)
		fDesiredPitchDelta=-1.0f;

	fDesiredPitchDelta *= m_pobParameters->m_fMaxPitchSpeed * DEG_TO_RAD_VALUE;

	m_fPitchDelta += (fDesiredPitchDelta-m_fPitchDelta) * m_pobParameters->m_fPitchAdjustFactor;


	// Calculate the velocity impulse to apply

	CDirection obNewLinearVelocity(srlg->GetLinearVelocity());

	float fSpeed=obNewLinearVelocity.Length();

	CMatrix obMatrix;

	CCamUtil::CreateFromPoints(obMatrix,CVecMath::GetZeroPoint(),CPoint(obNewLinearVelocity));
		
	const CDirection& obZAxis = obMatrix.GetZAxis();
	float fPhi = fatan2f(obZAxis.X(), obZAxis.Z());
	float fC = fsqrtf(obZAxis.X()*obZAxis.X() + obZAxis.Z()*obZAxis.Z());
	float fTheta = -fatan2f(obZAxis.Y(), fC);

	float fTurn = m_fYawDelta * fTimeDelta; // Use absolute time
	float fTilt = m_fPitchDelta * fTimeDelta; // Use absolute time

	if	( CPlayerOptions::Get().GetInvertY() == false )	// cam-dwhite
	{
		fTilt=-fTilt;
	}

	fPhi += fTurn;

	while(fPhi < 0)
		fPhi += TWO_PI;
	while(fPhi > TWO_PI)
		fPhi -= TWO_PI;

	fTheta += fTilt;

	while(fTheta < 0)
		fTheta += TWO_PI;
	while(fTheta > TWO_PI)
		fTheta -= TWO_PI;

	// Limit the pitch
	const float fUpwardsPitchAngleLimit = m_pobParameters->m_fMaxUpwardsPitchAngle * DEG_TO_RAD_VALUE;
	const float fDownwardsPitchAngleLimit = m_pobParameters->m_fMaxDownwardsPitchAngle * DEG_TO_RAD_VALUE;

	if (fTheta > PI)
	{
		if (fTheta < (TWO_PI-fUpwardsPitchAngleLimit))
		{
			fTheta=TWO_PI-fUpwardsPitchAngleLimit;
		}
	}
	else
	{
		if (fTheta > fDownwardsPitchAngleLimit)
		{
			fTheta=fDownwardsPitchAngleLimit;
		}
	}

	// Calculate new linear velocity
	CMatrix obNewMatrix;
	CCamUtil::MatrixFromEuler_XYZ(obNewMatrix,fTheta,fPhi,0.0f);
	CDirection obNewDirection(obNewMatrix.GetZAxis());
	obNewLinearVelocity=obNewDirection;
	obNewLinearVelocity*=fSpeed;

	obNewLinearVelocity-=srlg->GetLinearVelocity();
			
	obNewLinearVelocity *= srlg->GetRigidBody()->getMass();

	srlg->ApplyLinearImpulse(obNewLinearVelocity);

	// ----- Camera roll -----

	float fCameraRoll=(m_fYawDelta / m_pobParameters->m_fMaxYawSpeed) * m_pobParameters->m_fMaxRollAngle;

	int iCamID = CamMan::GetPrimaryView()->GetActiveCameraID();
	CoolCamera* pobCoolCam=(CoolCamera*)CamMan::GetPrimaryView()->GetCoolCam(iCamID);

	if (pobCoolCam && pobCoolCam->GetType()==CT_AFTERTOUCH)
	{
		CoolCam_AfterTouch* pobCam=(CoolCam_AfterTouch*)pobCoolCam;
		pobCam->SetRoll(fCameraRoll);
	}

#endif
	return false;
}


//------------------------------------------------------------------------------------------------------------------------------------------------


MotionSensorAfterTouchControllerDef::MotionSensorAfterTouchControllerDef(void) :
	m_pobControllingEntity(0),
	m_pobParameters(0)
{
}

MovementController* MotionSensorAfterTouchControllerDef::CreateInstance(CMovement* pobMovement) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) MotionSensorAfterTouchController( pobMovement, this );
}



//------------------------------------------------------------------------------------------------------------------------------------------------



MotionSensorAfterTouchController::MotionSensorAfterTouchController (CMovement* pMovement, const MotionSensorAfterTouchControllerDef* pDefinition) :
	MovementController( pMovement ),
	m_pobControllingEntity( pDefinition->m_pobControllingEntity ),
	m_pobParameters( pDefinition->m_pobParameters ),
	m_fTime( 0.0f ),
	m_fYawDelta( 0.0f ),
	m_fPitchDelta( 0.0f )
{
	ntAssert(pMovement);
	ntAssert(m_pobControllingEntity);
	ntAssert(m_pobParameters);
}

MotionSensorAfterTouchController::~MotionSensorAfterTouchController ()
{
}

bool MotionSensorAfterTouchController::Update(float /*fTimeDelta*/, const CMovementInput& /*input*/, const CMovementStateRef& /*currentState*/, CMovementState& /*predictedState*/)
{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

	Physics::System* system = this->m_pobMovement->GetPhysicsSystem();

	if( 0 == system )
		return true;

	Physics::SingleRigidLG* srlg = (Physics::SingleRigidLG*) system->GetFirstGroupByType( Physics::LogicGroup::SINGLE_RIGID_BODY_LG );

	if (!srlg)
		return true;

	if( (srlg->GetLinearVelocity().Length() + srlg->GetAngularVelocity( ).Length()) < 0.1f )
		return true;

	// No aftertouch movement slowdown.  Independent of game slow down.
	const float fTimeDelta = CTimer::Get().GetSystemTimeChange();
	m_fTime += CTimer::Get().GetGameTimeChange();

	// Do not apply aftertouch is object is heading towards camera!
	CDirection camZAxis = CamMan::GetPrimaryView()->GetCurrMatrix().GetZAxis();
	CDirection objDir = srlg->GetLinearVelocity();
	float fDotProduct	= camZAxis.X() * objDir.X() + camZAxis.Y() * objDir.Y() + camZAxis.Z() * objDir.Z();
	if (fDotProduct < 0.0f)
	{
		return true;
	}

	//ntPrintf("pad magnitude=%f\n",fPadMagnitude);

	// ----- Control multiplier -----

	float fControlMultiplier = 1.0f;

	if ( m_fTime > ( m_pobParameters->m_fMotionSensorControlFadeIn + m_pobParameters->m_fMotionSensorControlHold + m_pobParameters->m_fMotionSensorControlFadeOut ) ) // Control has expired
	{
		fControlMultiplier = 0.0f;
	}
	else if ( m_fTime < m_pobParameters->m_fMotionSensorControlFadeIn )
	{
		fControlMultiplier = m_fTime / m_pobParameters->m_fMotionSensorControlFadeIn;
	}
	else if ( m_fTime > ( m_pobParameters->m_fMotionSensorControlFadeIn + m_pobParameters->m_fMotionSensorControlHold ) )
	{
		fControlMultiplier = 1.0f - ( ( m_fTime - m_pobParameters->m_fMotionSensorControlFadeIn - m_pobParameters->m_fControlHold ) / m_pobParameters->m_fControlFadeOut );
	}

	// ----- Motion Sensor Input -----

	ntAssert(m_pobControllingEntity);

	CInputComponent* pobInputComponent = m_pobControllingEntity->GetInputComponent();
	ntAssert(pobInputComponent);

	float fMotionAccelX = pobInputComponent->GetSensorFilteredMag( PAD_SENSOR_ACCEL_X, PAD_SENSOR_FILTER_AVERAGE_5 );
	float fMotionAccelZ = pobInputComponent->GetSensorFilteredMag( PAD_SENSOR_ACCEL_Z, PAD_SENSOR_FILTER_AVERAGE_5 );

	// ----- Limit sensor values to maximum and scale to 0-1 range (motion contoller only) -----

	float fInputScale = 1.0f / m_pobParameters->m_fMotionSensorMaxPadMagnitude;

	// Limit X
	if ( fMotionAccelX >= m_pobParameters->m_fMotionSensorMaxPadMagnitude )
	{
		fMotionAccelX = m_pobParameters->m_fMotionSensorMaxPadMagnitude;
	}

	// Limit Z
	if ( fMotionAccelZ >= m_pobParameters->m_fMotionSensorMaxPadMagnitude )
	{
		fMotionAccelZ = m_pobParameters->m_fMotionSensorMaxPadMagnitude;
	}

	// Scale so that the max pad input is 1.0
	fMotionAccelX *= fInputScale;
	fMotionAccelZ *= fInputScale;

	// ----- Square curve for motion values (motion contoller only) -----

	// X
	if ( fMotionAccelX < 0.0f )
	{
		// Square it
		fMotionAccelX *= fMotionAccelX;

		// Make negative again
		fMotionAccelX = -fMotionAccelX;
	}
	else
	{
		// Square it
		fMotionAccelX *= fMotionAccelX;
	}

	// Z
	if ( fMotionAccelZ < 0.0f )
	{
		// Square it
		fMotionAccelZ *= fMotionAccelZ;

		// Make negative again
		fMotionAccelZ = -fMotionAccelZ;
	}
	else
	{
		// Square it
		fMotionAccelZ *= fMotionAccelZ;
	}

	// ----- Adjust Input values with control multiplier -----

	fMotionAccelX *= fControlMultiplier;
	fMotionAccelZ *= fControlMultiplier;

	// ----- Yaw control -----

	float fDesiredYawDelta = fMotionAccelX;

	if (fDesiredYawDelta>1.0f)
		fDesiredYawDelta=1.0f;

	if (fDesiredYawDelta<-1.0f)
		fDesiredYawDelta=-1.0f;

	fDesiredYawDelta *= m_pobParameters->m_fMotionSensorMaxYawSpeed * DEG_TO_RAD_VALUE;

	m_fYawDelta += ( fDesiredYawDelta - m_fYawDelta ) * m_pobParameters->m_fMotionSensorYawAdjustFactor;

	// ----- Pitch control -----

	float fDesiredPitchDelta = fMotionAccelZ;

	if (fDesiredPitchDelta>1.0f)
		fDesiredPitchDelta=1.0f;

	if (fDesiredPitchDelta<-1.0f)
		fDesiredPitchDelta=-1.0f;

	fDesiredPitchDelta *= m_pobParameters->m_fMotionSensorMaxPitchSpeed * DEG_TO_RAD_VALUE;

	m_fPitchDelta += ( fDesiredPitchDelta - m_fPitchDelta ) * m_pobParameters->m_fMotionSensorPitchAdjustFactor;

	// Calculate the velocity impulse to apply
	CDirection obNewLinearVelocity(srlg->GetLinearVelocity());

	float fSpeed=obNewLinearVelocity.Length();

	CMatrix obMatrix;

	CCamUtil::CreateFromPoints(obMatrix, CVecMath::GetZeroPoint(), CPoint(obNewLinearVelocity));
		
	const CDirection& obZAxis = obMatrix.GetZAxis();
	float fPhi = fatan2f(obZAxis.X(), obZAxis.Z());
	float fC = fsqrtf(obZAxis.X()*obZAxis.X() + obZAxis.Z()*obZAxis.Z());
	float fTheta = -fatan2f(obZAxis.Y(), fC);

	float fTurn = -m_fYawDelta * fTimeDelta; // Use absolute time
	float fTilt = -m_fPitchDelta * fTimeDelta; // Use absolute time

	if	( CPlayerOptions::Get().GetInvertY() == false )	// cam-dwhite
	{
		fTilt=-fTilt;
	}

	fPhi += fTurn;

	while(fPhi < 0)
		fPhi += TWO_PI;
	while(fPhi > TWO_PI)
		fPhi -= TWO_PI;

	fTheta += fTilt;

	while(fTheta < 0)
		fTheta += TWO_PI;
	while(fTheta > TWO_PI)
		fTheta -= TWO_PI;

	// Limit the pitch
	const float fUpwardsPitchAngleLimit = m_pobParameters->m_fMotionSensorMaxUpwardsPitchAngle * DEG_TO_RAD_VALUE;
	const float fDownwardsPitchAngleLimit = m_pobParameters->m_fMotionSensorMaxDownwardsPitchAngle * DEG_TO_RAD_VALUE;

	if (fTheta > PI)
	{
		if (fTheta < (TWO_PI - fUpwardsPitchAngleLimit))
		{
			fTheta = TWO_PI - fUpwardsPitchAngleLimit;
		}
	}
	else
	{
		if (fTheta > fDownwardsPitchAngleLimit)
		{
			fTheta = fDownwardsPitchAngleLimit;
		}
	}

	// Calculate new linear velocity
	CMatrix obNewMatrix;
	CCamUtil::MatrixFromEuler_XYZ(obNewMatrix,fTheta,fPhi,0.0f);
	CDirection obNewDirection(obNewMatrix.GetZAxis());
	obNewLinearVelocity=obNewDirection;
	obNewLinearVelocity*=fSpeed;

	obNewLinearVelocity-=srlg->GetLinearVelocity();
			
	obNewLinearVelocity *= srlg->GetRigidBody()->getMass();

	srlg->ApplyLinearImpulse(obNewLinearVelocity);

	// ----- Camera roll -----

	float fCameraRoll=(m_fYawDelta / m_pobParameters->m_fMaxYawSpeed) * m_pobParameters->m_fMotionSensorMaxRollAngle;

	int iCamID = CamMan::GetPrimaryView()->GetActiveCameraID();
	CoolCamera* pobCoolCam=(CoolCamera*)CamMan::GetPrimaryView()->GetCoolCam(iCamID);

	if (pobCoolCam && pobCoolCam->GetType()==CT_AFTERTOUCH)
	{
		CoolCam_AfterTouch* pobCam=(CoolCam_AfterTouch*)pobCoolCam;
		pobCam->SetRoll(fCameraRoll);
	}

#endif
	return false;
}


//------------------------------------------------------------------------------------------------------------------------------------------------

SpearAfterTouchControllerDef::SpearAfterTouchControllerDef(void) :
	m_pobControllingEntity(0),
	m_pobParameters(0)
{
}

MovementController* SpearAfterTouchControllerDef::CreateInstance(CMovement* pobMovement) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) SpearAfterTouchController( pobMovement, this );
}



//------------------------------------------------------------------------------------------------------------------------------------------------



SpearAfterTouchController::SpearAfterTouchController (CMovement* pMovement, const SpearAfterTouchControllerDef* pDefinition) :
	MovementController( pMovement ),
	m_pobControllingEntity( pDefinition->m_pobControllingEntity ),
	m_pobParameters( pDefinition->m_pobParameters ),
	m_fTime( 0.0f ),
	m_fYawDelta( 0.0f ),
	m_fPitchDelta( 0.0f )
{
	ntAssert(pMovement);
	ntAssert(m_pobControllingEntity);
	ntAssert(m_pobParameters);	
}

SpearAfterTouchController::~SpearAfterTouchController ()
{
}

bool SpearAfterTouchController::Update(float /*fTimeDelta*/, const CMovementInput& /*input*/, const CMovementStateRef& /*currentState*/, CMovementState& /*predictedState*/)
{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

	Physics::System* system = this->m_pobMovement->GetPhysicsSystem();

	if( 0 == system )
		return true;

	Physics::SpearLG* srlg = (Physics::SpearLG*) system->GetFirstGroupByType( Physics::LogicGroup::SPEAR_LG );

	if (!srlg)
		return true;

	const float fTimeDelta = CTimer::Get().GetSystemTimeChange();

	m_fTime += CTimer::Get().GetGameTimeChange();

	// Try to sync with the consistent stepping done by physics world  - DGF
	//const float fTimeDelta = fPHYSICS_WANTED_STEP_SPEED * CTimer::Get().GetGameTimeScalar(); //CTimer::Get().GetSystemTimeChange(); // Use absolute time, so that control doesnt feel different when time is slowed down

	const CInputComponent* pobInput=m_pobControllingEntity->GetInputComponent();
	const CDirection obPadDirection=pobInput->GetInputDir();
	float fPadMagnitude=pobInput->GetInputSpeed();

	CDirection obInputDirection=obPadDirection * CamMan::GetPrimaryView()->GetCurrMatrix().GetAffineInverse();

	// Scale the pad magnitude
	
	if (fPadMagnitude >= m_pobParameters->m_fMinPadMagnitude)
	{
		fPadMagnitude = (fPadMagnitude - m_pobParameters->m_fMinPadMagnitude) / (1.0f - m_pobParameters->m_fMinPadMagnitude);
	}
	else
	{
		fPadMagnitude = 0.0f;
	}

	//ntPrintf("pad magnitude=%f\n",fPadMagnitude);

	// ----- Control multiplier -----

	float fControlMultiplier = 1.0f;

	if (m_fTime>(m_pobParameters->m_fControlFadeIn+m_pobParameters->m_fControlHold+m_pobParameters->m_fControlFadeOut)) // Control has expired
	{
		fControlMultiplier = 0.0f;
	}
	else if (m_fTime<m_pobParameters->m_fControlFadeIn)
	{
		fControlMultiplier = m_fTime/m_pobParameters->m_fControlFadeIn;
	}
	else if (m_fTime>(m_pobParameters->m_fControlFadeIn+m_pobParameters->m_fControlHold))
	{
		fControlMultiplier = 1.0f - ((m_fTime - m_pobParameters->m_fControlFadeIn - m_pobParameters->m_fControlHold) / m_pobParameters->m_fControlFadeOut);
	}

	fPadMagnitude *= fControlMultiplier;

	//ntPrintf("Control=%f\n",fControlMultiplier);

	// ----- Yaw control -----

	float fDesiredYawDelta = obInputDirection.X() * fPadMagnitude;

	if (fDesiredYawDelta>1.0f)
		fDesiredYawDelta=1.0f;

	if (fDesiredYawDelta<-1.0f)
		fDesiredYawDelta=-1.0f;

	fDesiredYawDelta *= m_pobParameters->m_fMaxYawSpeed * DEG_TO_RAD_VALUE;

	m_fYawDelta += (fDesiredYawDelta-m_fYawDelta) * m_pobParameters->m_fYawAdjustFactor;

	// ----- Pitch control -----

	float fDesiredPitchDelta = obInputDirection.Z() * fPadMagnitude;

	if (fDesiredPitchDelta>1.0f)
		fDesiredPitchDelta=1.0f;

	if (fDesiredPitchDelta<-1.0f)
		fDesiredPitchDelta=-1.0f;

	fDesiredPitchDelta *= m_pobParameters->m_fMaxPitchSpeed * DEG_TO_RAD_VALUE;

	m_fPitchDelta += (fDesiredPitchDelta-m_fPitchDelta) * m_pobParameters->m_fPitchAdjustFactor;


	// Calculate the velocity impulse to apply

	CDirection obNewLinearVelocity(srlg->GetLinearVelocity());

	float fSpeed=obNewLinearVelocity.Length();

	CMatrix obMatrix;

	CCamUtil::CreateFromPoints(obMatrix,CVecMath::GetZeroPoint(),CPoint(obNewLinearVelocity));
		
	const CDirection& obZAxis = obMatrix.GetZAxis();
	float fPhi = fatan2f(obZAxis.X(), obZAxis.Z());
	float fC = fsqrtf(obZAxis.X()*obZAxis.X() + obZAxis.Z()*obZAxis.Z());
	float fTheta = -fatan2f(obZAxis.Y(), fC);

	float fTurn = m_fYawDelta * fTimeDelta; // Use absolute time
	float fTilt = m_fPitchDelta * fTimeDelta; // Use absolute time

	if	( CPlayerOptions::Get().GetInvertY() == false )	// cam-dwhite
	{
		fTilt=-fTilt;
	}

	fPhi += fTurn;

	while(fPhi < 0)
		fPhi += TWO_PI;
	while(fPhi > TWO_PI)
		fPhi -= TWO_PI;

	fTheta += fTilt;

	while(fTheta < 0)
		fTheta += TWO_PI;
	while(fTheta > TWO_PI)
		fTheta -= TWO_PI;

	// Limit the pitch
	const float fUpwardsPitchAngleLimit = m_pobParameters->m_fMaxUpwardsPitchAngle * DEG_TO_RAD_VALUE;
	const float fDownwardsPitchAngleLimit = m_pobParameters->m_fMaxDownwardsPitchAngle * DEG_TO_RAD_VALUE;

	if (fTheta > PI)
	{
		if (fTheta < (TWO_PI-fUpwardsPitchAngleLimit))
		{
			fTheta=TWO_PI-fUpwardsPitchAngleLimit;
		}
	}
	else
	{
		if (fTheta > fDownwardsPitchAngleLimit)
		{
			fTheta=fDownwardsPitchAngleLimit;
		}
	}

	// Calculate new linear velocity
	CMatrix obNewMatrix;
	CCamUtil::MatrixFromEuler_XYZ(obNewMatrix,fTheta,fPhi,0.0f);
	CDirection obNewDirection(obNewMatrix.GetZAxis());
	obNewLinearVelocity=obNewDirection;
	obNewLinearVelocity*=fSpeed;

	obNewLinearVelocity-=srlg->GetLinearVelocity();
			
	obNewLinearVelocity *= srlg->GetMass();

	srlg->ApplyLinearImpulse(obNewLinearVelocity);

	// ----- Camera roll -----

	float fCameraRoll=(m_fYawDelta / m_pobParameters->m_fMaxYawSpeed) * m_pobParameters->m_fMaxRollAngle;

	int iCamID = CamMan::GetPrimaryView()->GetActiveCameraID();
	CoolCamera* pobCoolCam=(CoolCamera*)CamMan::GetPrimaryView()->GetCoolCam(iCamID);

	if (pobCoolCam && pobCoolCam->GetType()==CT_AFTERTOUCH)
	{
		CoolCam_AfterTouch* pobCam=(CoolCam_AfterTouch*)pobCoolCam;
		pobCam->SetRoll(fCameraRoll);
	}

#endif
	return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------------


ProjectileAfterTouchControllerDef::ProjectileAfterTouchControllerDef(void) :
	m_pobControllingEntity( 0 )
{
}

MovementController* ProjectileAfterTouchControllerDef::CreateInstance(CMovement* pobMovement) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) ProjectileAfterTouchController( pobMovement, this );
}



//------------------------------------------------------------------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------------
//!  public constructor  ProjectileAfterTouchController
//!
//!  @param [in, out]  pMovement CMovement *    
//!  @param [in]       pDefinition const ProjectileAfterTouchControllerDef *    
//!
//!  @author GavB @date 07/11/2006
//------------------------------------------------------------------------------------------
ProjectileAfterTouchController::ProjectileAfterTouchController (CMovement* pMovement, const ProjectileAfterTouchControllerDef* pDefinition) :
	MovementController( pMovement ),
	m_pobControllingEntity( pDefinition->m_pobControllingEntity ),
	m_pobParameters( pDefinition->m_pobParameters ),
	m_AllowDualInput( pDefinition->m_bDualController )
{
	ntAssert(pMovement);
	ntAssert(m_pobParameters);
	ntAssert(m_pobControllingEntity);
	
	m_fControlTime=0.0f;
	m_fYawDelta = 0.0f;
	m_fPitchDelta = 0.0f;
}

//------------------------------------------------------------------------------------------
//!  public virtual destructor  ~ProjectileAfterTouchController
//!
//!
//!  @author GavB @date 07/11/2006
//------------------------------------------------------------------------------------------
ProjectileAfterTouchController::~ProjectileAfterTouchController ()
{
	m_pobControllingEntity = 0;
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
//!  @author GavB @date 07/11/2006
//------------------------------------------------------------------------------------------
bool ProjectileAfterTouchController::Update(float /*fTimeStep*/, const CMovementInput& /*input*/, const CMovementStateRef& /*currentState*/, CMovementState& predictedState)
{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

	Physics::System* pPhysicsSystem = m_pobMovement->GetPhysicsSystem();

	if( !pPhysicsSystem )
		return true;

	// No aftertouch movement slowdown.  Independent of game slow down.
	float fTimeStep = CTimer::Get().GetSystemTimeChange();

	// Try to sync with the consistent stepping done by physics world  - DGF
	//const float fTimeStep = fPHYSICS_WANTED_STEP_SPEED * CTimer::Get().GetGameTimeScalar(); //CTimer::Get().GetSystemTimeChange(); // Use absolute time, so that control doesnt feel different when time is slowed down

	Physics::ProjectileLG* srlg = (Physics::ProjectileLG*) pPhysicsSystem->GetFirstGroupByType( Physics::LogicGroup::PROJECTILE_LG );

	if( srlg )
	{
		if (srlg->GetLinearVelocity().LengthSquared()<EPSILON) // No point doing any aftertouch if the projectile is not moving!
			return false;

		m_fControlTime += CTimer::Get().GetGameTimeChange();

		// ----- Control multiplier -----

		float fControlMultiplier = 1.0f;

		if ( m_fControlTime > ( m_pobParameters->m_fControlFadeIn + m_pobParameters->m_fControlHold + m_pobParameters->m_fControlFadeOut ) ) // Control has expired
		{
			fControlMultiplier = 0.0f;

			if (( m_fControlTime - CTimer::Get().GetGameTimeChange() ) < ( m_pobParameters->m_fControlFadeIn + m_pobParameters->m_fControlHold + m_pobParameters->m_fControlFadeOut ))
			{
				int iCamID = CamMan::GetPrimaryView()->GetActiveCameraID();
				CoolCamera* pobCoolCam=(CoolCamera*)CamMan::GetPrimaryView()->GetCoolCam(iCamID);

				if (pobCoolCam && pobCoolCam->GetType()==CT_AFTERTOUCH)
				{
					CoolCam_AfterTouch* pobCam=(CoolCam_AfterTouch*)pobCoolCam;
					pobCam->SetRoll(0.0f);
				}

				CMessageSender::SendEmptyMessage(CHashedString(HASH_STRING_MSG_CONTROL_END),srlg->GetEntity()->GetMessageHandler());

				//ntPrintf("Projectile control has ended\n");
			}

			return false;
		}
		else if ( m_fControlTime < m_pobParameters->m_fControlFadeIn )
		{
			// Bring in control
			fControlMultiplier = m_fControlTime / m_pobParameters->m_fControlFadeIn;
		}
		else if ( m_fControlTime > ( m_pobParameters->m_fControlFadeIn + m_pobParameters->m_fControlHold ) )
		{
			// Fade out control
			fControlMultiplier = 1.0f - ((m_fControlTime - m_pobParameters->m_fControlFadeIn - m_pobParameters->m_fControlHold) / m_pobParameters->m_fControlFadeOut);
		}

		// ----- Pad Input -----

		float		fInputSpeed			= m_pobControllingEntity->GetInputComponent()->GetInputSpeed();
		float		fInputSpeed2		= m_pobControllingEntity->GetInputComponent()->GetInputSpeedAlt();
		CDirection	dirInputDirection	= m_pobControllingEntity->GetInputComponent()->GetInputDir();

		// Use the direcion input based on which has the largest directional magnitude
		if( fInputSpeed < fInputSpeed2 && m_AllowDualInput )
		{
			fInputSpeed = fInputSpeed2;
			dirInputDirection = m_pobControllingEntity->GetInputComponent()->GetInputDirAlt();
		}

		fInputSpeed *= fControlMultiplier;

		if (fInputSpeed > m_pobParameters->m_fMinPadMagnitude)
		{
			float fSpeed = srlg->GetLinearVelocity().Length();
		
			const CMatrix& obCameraMatrix=CamMan::Get().GetPrimaryView()->GetCurrMatrix();
			CDirection obInputDirection = dirInputDirection * obCameraMatrix.GetAffineInverse();

			CMatrix obMatrix;

			CCamUtil::CreateFromPoints(obMatrix,CVecMath::GetZeroPoint(),CPoint(srlg->GetLinearVelocity()));
			
			const CDirection& obZAxis = obMatrix.GetZAxis();
			float fPhi = fatan2f(obZAxis.X(), obZAxis.Z());
			float fC = fsqrtf(obZAxis.X()*obZAxis.X() + obZAxis.Z()*obZAxis.Z());
			float fTheta = -fatan2f(obZAxis.Y(), fC);

			// ----- Yaw control -----

			float fDesiredYawDelta = obInputDirection.X() * fInputSpeed;

			if (fDesiredYawDelta>1.0f)
				fDesiredYawDelta=1.0f;

			if (fDesiredYawDelta<-1.0f)
				fDesiredYawDelta=-1.0f;

			fDesiredYawDelta *= m_pobParameters->m_fMaxYawSpeed * DEG_TO_RAD_VALUE;

			m_fYawDelta += ( fDesiredYawDelta - m_fYawDelta ) * m_pobParameters->m_fYawAdjustFactor;

			// ----- Pitch control -----

			float fDesiredPitchDelta = obInputDirection.Z() * fInputSpeed;;

			if (fDesiredPitchDelta>1.0f)
				fDesiredPitchDelta=1.0f;

			if (fDesiredPitchDelta<-1.0f)
				fDesiredPitchDelta=-1.0f;

			fDesiredPitchDelta *= m_pobParameters->m_fMaxPitchSpeed * DEG_TO_RAD_VALUE;

			m_fPitchDelta += ( fDesiredPitchDelta - m_fPitchDelta ) * m_pobParameters->m_fPitchAdjustFactor;

			float fTurn = m_fYawDelta * fTimeStep;
			float fTilt = m_fPitchDelta * fTimeStep;

			// ----- Invert Y Control -----

			if	( CPlayerOptions::Get().GetInvertY() == false )	// cam-dwhite
			{
				fTilt=-fTilt;
			}

			// ----- Angle Range Limiting -----

			fPhi += fTurn;
			fPhi = fmodf( fPhi, TWO_PI );

			fTheta += fTilt;
			fTheta = fmodf( fTheta, TWO_PI );

			// ----- Pitch Limiting -----

			const float fUpwardsPitchAngleLimit = m_pobParameters->m_fMaxUpwardsPitchAngle * DEG_TO_RAD_VALUE;
			const float fDownwardsPitchAngleLimit = m_pobParameters->m_fMaxDownwardsPitchAngle * DEG_TO_RAD_VALUE;

			if (fTheta > PI)
			{
				if (fTheta < (TWO_PI-fUpwardsPitchAngleLimit))
				{
					fTheta=TWO_PI-fUpwardsPitchAngleLimit;
				}
			}
			else
			{
				if (fTheta > fDownwardsPitchAngleLimit)
				{
					fTheta=fDownwardsPitchAngleLimit;
				}
			}

			// ----- Camera roll -----

			float fCameraRoll = (m_fYawDelta / (m_pobParameters->m_fMaxYawSpeed * DEG_TO_RAD_VALUE)) * (m_pobParameters->m_fMaxRollAngle * DEG_TO_RAD_VALUE);

			CoolCamera* pobCoolCam = CamMan::GetPrimaryView()->FindCoolCam(CT_AFTERTOUCH);

			if (pobCoolCam && pobCoolCam->GetType()==CT_AFTERTOUCH)
			{
				CoolCam_AfterTouch* pobCam=(CoolCam_AfterTouch*)pobCoolCam;
				pobCam->SetRoll(fCameraRoll);
			}

			// ----- Projectile Velocity and Thrust Setting -----

			CMatrix obNewMatrix;

			CCamUtil::MatrixFromEuler_XYZ(obNewMatrix,fTheta,fPhi,0.0f);

			CDirection temp = obNewMatrix.GetZAxis();
			srlg->SetThrustDirection( temp );

			temp*=fSpeed;
			srlg->SetLinearVelocity( temp );
		}
		else
		{
			int iCamID = CamMan::GetPrimaryView()->GetActiveCameraID();
			CoolCamera* pobCoolCam=(CoolCamera*)CamMan::GetPrimaryView()->GetCoolCam(iCamID);

			if (pobCoolCam && pobCoolCam->GetType()==CT_AFTERTOUCH)
			{
				CoolCam_AfterTouch* pobCam=(CoolCam_AfterTouch*)pobCoolCam;
				pobCam->SetRoll(0.0f);
			}
		}
	}
	
#else
	UNUSED( fTimeStep );
#endif
	return false;
}


//------------------------------------------------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
//!  public constructor  MotionSensorProjectileAfterTouchControllerDef
//!
//!
//!  @author GavB @date 14/12/2006
//------------------------------------------------------------------------------------------
MotionSensorProjectileAfterTouchControllerDef::MotionSensorProjectileAfterTouchControllerDef(void) :
	m_pobControllingEntity( 0 )
{
}

//------------------------------------------------------------------------------------------
//!  public virtual constant  CreateInstance
//!
//!  @param [in, out]  pobMovement CMovement * 
//!
//!  @return MovementController * 
//!
//!  @author GavB @date 14/12/2006
//------------------------------------------------------------------------------------------
MovementController* MotionSensorProjectileAfterTouchControllerDef::CreateInstance(CMovement* pobMovement) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) MotionSensorProjectileAfterTouchController( pobMovement, this );
}



//------------------------------------------------------------------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------------
//!  public constructor  MotionSensorProjectileAfterTouchController
//!
//!  @param [in, out]  pMovement CMovement * 
//!  @param [in]       pDefinition const MotionSensorProjectileAfterTouchControllerDef * 
//!
//!
//!  @author GavB @date 14/12/2006
//------------------------------------------------------------------------------------------
MotionSensorProjectileAfterTouchController::MotionSensorProjectileAfterTouchController (CMovement* pMovement, const MotionSensorProjectileAfterTouchControllerDef* pDefinition) :
	MovementController( pMovement ),
	m_pobControllingEntity( pDefinition->m_pobControllingEntity ),
	m_pobParameters( pDefinition->m_pobParameters ),
	m_AllowDualInput( pDefinition->m_bDualController )
{
	ntAssert(pMovement);
	ntAssert(m_pobParameters);
	ntAssert(m_pobControllingEntity);
	
	m_fControlTime=0.0f;
	m_fYawDelta = 0.0f;
	m_fPitchDelta = 0.0f;
}

//------------------------------------------------------------------------------------------
//!  public virtual destructor  ~MotionSensorProjectileAfterTouchController
//!
//!
//!  @author GavB @date 14/12/2006
//------------------------------------------------------------------------------------------
MotionSensorProjectileAfterTouchController::~MotionSensorProjectileAfterTouchController ()
{
	m_pobControllingEntity = 0;
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
//!  @author GavB @date 14/12/2006
//------------------------------------------------------------------------------------------
bool MotionSensorProjectileAfterTouchController::Update(float /*fTimeStep*/, const CMovementInput& /*input*/, const CMovementStateRef& /*currentState*/, CMovementState& /*predictedState*/)
{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#ifdef PLATFORM_PS3

	Physics::System* system = this->m_pobMovement->GetPhysicsSystem();

	if( !system )
		return true;

	// No aftertouch movement slowdown.  Independent of game slow down.
	float fTimeStep = CTimer::Get().GetSystemTimeChange();

	// Try to sync with the consistent stepping done by physics world  - DGF
	//fTimeStep = fPHYSICS_WANTED_STEP_SPEED * CTimer::Get().GetGameTimeScalar(); //CTimer::Get().GetSystemTimeChange(); // Use absolute time, so that control doesnt feel different when time is slowed down

	Physics::ProjectileLG* srlg = (Physics::ProjectileLG*) system->GetFirstGroupByType( Physics::LogicGroup::PROJECTILE_LG );

	if( srlg )
	{
		if (srlg->GetLinearVelocity().LengthSquared()<EPSILON) // No point doing any aftertouch if the projectile is not moving!
			return false;

		m_fControlTime += CTimer::Get().GetGameTimeChange();

		// ----- Control multiplier -----

		float fControlMultiplier = 1.0f;

		if ( m_fControlTime > ( m_pobParameters->m_fMotionSensorControlFadeIn + m_pobParameters->m_fMotionSensorControlHold + m_pobParameters->m_fMotionSensorControlFadeOut ) ) // Control has expired
		{
			fControlMultiplier = 0.0f;

			if (( m_fControlTime - CTimer::Get().GetGameTimeChange() ) < ( m_pobParameters->m_fMotionSensorControlFadeIn + m_pobParameters->m_fMotionSensorControlHold + m_pobParameters->m_fMotionSensorControlFadeOut ))
			{
				int iCamID = CamMan::GetPrimaryView()->GetActiveCameraID();
				CoolCamera* pobCoolCam=(CoolCamera*)CamMan::GetPrimaryView()->GetCoolCam(iCamID);

				if (pobCoolCam && pobCoolCam->GetType()==CT_AFTERTOUCH)
				{
					CoolCam_AfterTouch* pobCam=(CoolCam_AfterTouch*)pobCoolCam;
					pobCam->SetRoll(0.0f);
				}

				CMessageSender::SendEmptyMessage(CHashedString(HASH_STRING_MSG_CONTROL_END),srlg->GetEntity()->GetMessageHandler());

				//ntPrintf("Projectile control has ended\n");
			}

			return false;
		}
		else if ( m_fControlTime < m_pobParameters->m_fMotionSensorControlFadeIn )
		{
			// Bring in control
			fControlMultiplier = m_fControlTime / m_pobParameters->m_fMotionSensorControlFadeIn;
		}
		else if ( m_fControlTime > ( m_pobParameters->m_fMotionSensorControlFadeIn + m_pobParameters->m_fMotionSensorControlHold ) )
		{
			// Fade out control
			fControlMultiplier = 1.0f - ((m_fControlTime - m_pobParameters->m_fMotionSensorControlFadeIn - m_pobParameters->m_fMotionSensorControlHold) / m_pobParameters->m_fMotionSensorControlFadeOut);
		}

		//ntPrintf("Time: %.4f, Step: %.4f, Multi: %.4f\n", m_fControlTime, fTimeStep, fControlMultiplier);

		// ----- Motion Sensor Input -----

		ntAssert(m_pobControllingEntity);

		CInputComponent* pobInputComponent = m_pobControllingEntity->GetInputComponent();
		ntAssert(pobInputComponent);

		float fMotionAccelX = pobInputComponent->GetSensorFilteredMag( PAD_SENSOR_ACCEL_X, PAD_SENSOR_FILTER_AVERAGE_5 );
		float fMotionAccelZ = pobInputComponent->GetSensorFilteredMag( PAD_SENSOR_ACCEL_Z, PAD_SENSOR_FILTER_AVERAGE_5 );

		// ----- Limit sensor values to maximum and scale to 0-1 range (motion contoller only) -----

		float fInputScale = 1.0f / m_pobParameters->m_fMotionSensorMaxPadMagnitude;

		// Limit X
		if ( fMotionAccelX >= m_pobParameters->m_fMotionSensorMaxPadMagnitude )
		{
			fMotionAccelX = m_pobParameters->m_fMotionSensorMaxPadMagnitude;
		}

		// Limit Z
		if ( fMotionAccelZ >= m_pobParameters->m_fMotionSensorMaxPadMagnitude )
		{
			fMotionAccelZ = m_pobParameters->m_fMotionSensorMaxPadMagnitude;
		}

		// Scale so that the max pad input is 1.0
		fMotionAccelX *= fInputScale;
		fMotionAccelZ *= fInputScale;

		// ----- Square curve for motion values (motion contoller only) -----

		// X
		if ( fMotionAccelX < 0.0f )
		{
			// Square it
			fMotionAccelX *= fMotionAccelX;

			// Make negative again
			fMotionAccelX = -fMotionAccelX;
		}
		else
		{
			// Square it
			fMotionAccelX *= fMotionAccelX;
		}

		// Z
		if ( fMotionAccelZ < 0.0f )
		{
			// Square it
			fMotionAccelZ *= fMotionAccelZ;

			// Make negative again
			fMotionAccelZ = -fMotionAccelZ;
		}
		else
		{
			// Square it
			fMotionAccelZ *= fMotionAccelZ;
		}

		// ----- Adjust Input values with control multiplier -----

		fMotionAccelX *= fControlMultiplier;
		fMotionAccelZ *= fControlMultiplier;

		// ----- Yaw control -----

		float fDesiredYawDelta = fMotionAccelX;

		if (fDesiredYawDelta>1.0f)
			fDesiredYawDelta=1.0f;

		if (fDesiredYawDelta<-1.0f)
			fDesiredYawDelta=-1.0f;

		fDesiredYawDelta *= m_pobParameters->m_fMotionSensorMaxYawSpeed * DEG_TO_RAD_VALUE;

		m_fYawDelta += ( fDesiredYawDelta - m_fYawDelta ) * m_pobParameters->m_fMotionSensorYawAdjustFactor;

		// ----- Pitch control -----

		float fDesiredPitchDelta = fMotionAccelZ;

		if (fDesiredPitchDelta>1.0f)
			fDesiredPitchDelta=1.0f;

		if (fDesiredPitchDelta<-1.0f)
			fDesiredPitchDelta=-1.0f;

		fDesiredPitchDelta *= m_pobParameters->m_fMotionSensorMaxPitchSpeed * DEG_TO_RAD_VALUE;

		m_fPitchDelta += ( fDesiredPitchDelta - m_fPitchDelta ) * m_pobParameters->m_fMotionSensorPitchAdjustFactor;

		if (fabs(m_fYawDelta) > m_pobParameters->m_fMotionSensorMinPadMagnitude || fabs(m_fPitchDelta) > m_pobParameters->m_fMotionSensorMinPadMagnitude)
		{
			float fSpeed = srlg->GetLinearVelocity().Length();

			CMatrix obMatrix;

			CCamUtil::CreateFromPoints(obMatrix,CVecMath::GetZeroPoint(),CPoint(srlg->GetLinearVelocity()));
			
			const CDirection& obZAxis = obMatrix.GetZAxis();
			float fPhi = fatan2f(obZAxis.X(), obZAxis.Z());
			float fC = fsqrtf(obZAxis.X()*obZAxis.X() + obZAxis.Z()*obZAxis.Z());
			float fTheta = -fatan2f(obZAxis.Y(), fC);

			float fTurn = -m_fYawDelta * fTimeStep;
			float fTilt = -m_fPitchDelta * fTimeStep;

			// ----- Invert Y Control -----

			/*
			if	( CPlayerOptions::Get().GetInvertY() == false )	// cam-dwhite
			{
				fTilt=-fTilt;
			}
			*/

			// ----- Angle Range Limiting -----

			fPhi += fTurn;

			while(fPhi < 0)
				fPhi += TWO_PI;
			while(fPhi > TWO_PI)
				fPhi -= TWO_PI;

			fTheta += fTilt;

			while(fTheta < 0)
				fTheta += TWO_PI;
			while(fTheta > TWO_PI)
				fTheta -= TWO_PI;

			// ----- Pitch Limiting -----

			const float fUpwardsPitchAngleLimit = m_pobParameters->m_fMotionSensorMaxUpwardsPitchAngle * DEG_TO_RAD_VALUE;
			const float fDownwardsPitchAngleLimit = m_pobParameters->m_fMotionSensorMaxDownwardsPitchAngle * DEG_TO_RAD_VALUE;

			if (fTheta > PI)
			{
				if (fTheta < (TWO_PI-fUpwardsPitchAngleLimit))
				{
					fTheta=TWO_PI-fUpwardsPitchAngleLimit;
				}
			}
			else
			{
				if (fTheta > fDownwardsPitchAngleLimit)
				{
					fTheta=fDownwardsPitchAngleLimit;
				}
			}

			// ----- Camera roll -----

			float fCameraRoll = (-m_fYawDelta / m_pobParameters->m_fMotionSensorMaxYawSpeed) * m_pobParameters->m_fMotionSensorMaxRollAngle;

			int iCamID = CamMan::GetPrimaryView()->GetActiveCameraID();
			CoolCamera* pobCoolCam=(CoolCamera*)CamMan::GetPrimaryView()->GetCoolCam(iCamID);

			if (pobCoolCam && pobCoolCam->GetType()==CT_AFTERTOUCH)
			{
				CoolCam_AfterTouch* pobCam=(CoolCam_AfterTouch*)pobCoolCam;
				pobCam->SetRoll(fCameraRoll);
			}

			// ----- Projectile Velocity and Thrust Setting -----

			CMatrix obNewMatrix;

			CCamUtil::MatrixFromEuler_XYZ(obNewMatrix,fTheta,fPhi,0.0f);

			CDirection temp = obNewMatrix.GetZAxis();
			srlg->SetThrustDirection( temp );

			temp*=fSpeed;
			srlg->SetLinearVelocity( temp );
		}
		else
		{
			// ----- Camera roll -----

			int iCamID = CamMan::GetPrimaryView()->GetActiveCameraID();
			CoolCamera* pobCoolCam=(CoolCamera*)CamMan::GetPrimaryView()->GetCoolCam(iCamID);

			if (pobCoolCam && pobCoolCam->GetType()==CT_AFTERTOUCH)
			{
				CoolCam_AfterTouch* pobCam=(CoolCam_AfterTouch*)pobCoolCam;
				pobCam->SetRoll(0.0f);
			}
		}
	}
#endif
#else
	UNUSED( fTimeStep );
#endif
	return false;
}


//------------------------------------------------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
//!
//! Constructor
//!	ProjectileTrackEntityControllerDef::ProjectileTrackEntityControllerDef
//!
//------------------------------------------------------------------------------------------
ProjectileTrackEntityControllerDef::ProjectileTrackEntityControllerDef(void) :
	m_pobTrackedEntity( 0 ), m_fTurnWeighting(0.8f)
{
}


//------------------------------------------------------------------------------------------
//!
//! Creates a new instance of a movement controller
//!	ProjectileTrackEntityControllerDef::CreateInstance
//!
//------------------------------------------------------------------------------------------
MovementController* ProjectileTrackEntityControllerDef::CreateInstance(CMovement* pobMovement) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) ProjectileTrackEntityController( pobMovement, this );
}



//------------------------------------------------------------------------------------------------------------------------------------------------



ProjectileTrackEntityController::ProjectileTrackEntityController (CMovement* pMovement, const ProjectileTrackEntityControllerDef* pDefinition) :
	MovementController( pMovement ),
	m_pobTrackedEntity( pDefinition->m_pobTrackedEntity ),
	m_obPosOffset(pDefinition->m_obPosOffset ),
	m_fTurnWeighting(pDefinition->m_fTurnWeighting)
{
	ntAssert(pMovement);
	ntAssert(m_pobTrackedEntity);
}

ProjectileTrackEntityController::~ProjectileTrackEntityController ()
{
	m_pobTrackedEntity = 0;
}

bool ProjectileTrackEntityController::Update(float fTimeStep, const CMovementInput& /*input*/, const CMovementStateRef& /*currentState*/, CMovementState& /*predictedState*/)
{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

	Physics::System* pPhysicsSystem = m_pobMovement->GetPhysicsSystem();

	if( NULL == pPhysicsSystem )
		return true;

	Physics::ProjectileLG* pProjectile = (Physics::ProjectileLG*) pPhysicsSystem->GetFirstGroupByType( Physics::LogicGroup::PROJECTILE_LG );

	if( pProjectile )
	{
		if (pProjectile->GetLinearVelocity().LengthSquared()<EPSILON) // No point doing any aftertouch if the projectile is not moving!
			return true;

		CPoint ptCurrentpos = m_pobMovement->GetParentEntity()->GetPosition();
		CPoint ptTrackedPosition = m_pobTrackedEntity->GetPosition() + m_obPosOffset;

		//g_VisualDebug->Printf2D( 10, 10, DC_WHITE, 0, "TrackedEntPos	x=%.3f y=%.3f z=%.3f\n", m_pobTrackedEntity->GetPosition().X(),m_pobTrackedEntity->GetPosition().Y(),m_pobTrackedEntity->GetPosition().Z());
		//g_VisualDebug->Printf2D( 10, 30, DC_WHITE, 0, "PosOffset		x=%.3f y=%.3f z=%.3f\n", m_obPosOffset.X(),m_obPosOffset.Y(),m_obPosOffset.Z());
		//g_VisualDebug->Printf2D( 10, 50, DC_WHITE, 0, "TrackedPos		x=%.3f y=%.3f z=%.3f\n", ptTrackedPosition.X(),ptTrackedPosition.Y(),ptTrackedPosition.Z());

		// Nasty aiming point
		//ptCurrentpos.Y() = ptTrackedPosition.Y();

		// Wrok out the direction to send the projectile
		CDirection	dirDirecion			= CDirection(ptTrackedPosition - ptCurrentpos);
		CDirection	dirCurrentVelocity	= pProjectile->GetLinearVelocity();
		
		float fVel							= dirCurrentVelocity.Length();
		CDirection	dirDirecionNorm			= dirDirecion / dirDirecion.Length();
		CDirection	dirCurrentVelocityNorm	= dirCurrentVelocity / fVel;
		float fDot							= dirDirecionNorm.Dot( dirCurrentVelocityNorm );

		if( fDot > 0.8f )
		{
//			static const float WEIGHTING = 0.8f;

//			CDirection	dirNewVelocity = dirCurrentVelocityNorm + (dirDirecion * WEIGHTING * fTimeStep);
			CDirection	dirNewVelocity = dirCurrentVelocityNorm + (dirDirecion * m_fTurnWeighting * fTimeStep);
			dirNewVelocity.Normalise();
			dirNewVelocity *= fVel;

			// Set the velocity
			pProjectile->SetLinearVelocity( dirNewVelocity );
		}
	}
	
#else
	UNUSED( fTimeStep );
#endif
	return false;
}




//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------



RagdollThrownControllerDef::RagdollThrownControllerDef(void) :
	m_pobControllingEntity( 0 ),
	m_pobParameters( 0 ),
	m_obLinearVelocity(CONSTRUCT_CLEAR)
{
}

MovementController* RagdollThrownControllerDef::CreateInstance(CMovement* pobMovement) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) RagdollThrownController( pobMovement, *this );
}

RagdollThrownController::RagdollThrownController(CMovement*	pobMovement, const RagdollThrownControllerDef&	obDefinition ) :
	MovementController( pobMovement ),
	m_pobRagdollLG( 0 ),
	m_pobParentEntity( 0 ),
	m_pobControllingEntity( 0 ),
	m_pobParameters( 0 ),
	m_obLinearVelocity(CONSTRUCT_CLEAR),
	m_fTime( 0.0f ),
	m_fGameTime( 0.0f ),
	m_fPitchDelta( 0.0f ),
	m_fYawDelta( 0.0f ),
	m_bFinished( false )
{
	//ntPrintf("RagdollThrownController - Begin\n");

	m_pobParentEntity=(CEntity*)m_pobMovement->GetParentEntity();
	m_pobControllingEntity=obDefinition.m_pobControllingEntity;
	m_pobParameters=obDefinition.m_pobParameters;
	m_obLinearVelocity=obDefinition.m_obLinearVelocity;
	m_fControlDuration = m_pobParameters ? ( m_pobParameters->m_fControlFadeIn+m_pobParameters->m_fControlHold+m_pobParameters->m_fControlFadeOut ) : 0.0f;

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

	// Put the character into an animated state
	Physics::AdvancedCharacterController* pobCharacter = (Physics::AdvancedCharacterController*) m_pobParentEntity->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );

	if (pobCharacter)
	{
		// Ragdoll should already be in animated transform tracking state from LUA setting
		//pobCharacter->SetRagdollAnimated(Physics::AdvancedRagdoll::PELVIS | Physics::AdvancedRagdoll::SPINE);
		pobCharacter->SetRagdollTurnDynamicOnContact(true);
		m_pobParentEntity->GetPhysicsSystem()->GetCollisionStrikeHandler()->Enable();
		m_pobRagdollLG = pobCharacter->GetAdvancedRagdoll(); // This is evil but for e3 it's hackily neccessary
	}

#endif // _PS3_RUN_WITHOUT_HAVOK_BUILD
}

RagdollThrownController::~RagdollThrownController( void )
{
	m_pobRagdollLG->SetAntiGravity(false);
	//m_pobParentEntity->GetPhysicsSystem()->DoHurtOnCollision( false );
	//ntPrintf("RagdollThrownController - End\n");

	// Put the character back into their dead state
//	Physics::AdvancedCharacterController* pobCharacter = (Physics::AdvancedCharacterController*) m_pobParentEntity->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
//	pobCharacter->GetAdvancedRagdoll()->SetState(Physics::AdvancedRagdoll::DEAD);
}

bool RagdollThrownController::Update (float /*fTimeStep*/, const CMovementInput& /*obMovementInput*/, const CMovementStateRef& /*obCurrentMovementState*/, CMovementState& /*obPredictedMovementState*/)
{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

	if (!m_pobRagdollLG)
	{
		ntError(0);
		m_bFinished=true;
		return true;
	}

	if (m_bFirstFrame)
	{
		m_pobRagdollLG->SetAntiGravity(true);
	}

	const float fTHROW_TIME = 1.0f;

	const float fTimeStep = CTimer::Get().GetSystemTimeChange();
	m_fTime += CTimer::Get().GetGameTimeChange();

	m_fGameTime += fTimeStep; // Game time

	//ntError(m_pobRagdollLG->GetExemptFromCleanup());

	if (m_fGameTime>=fTHROW_TIME || !m_pobParameters)
	{
		if (!m_bFinished)
		{
			m_pobParentEntity->GetPhysicsSystem()->GetCollisionStrikeHandler()->Enable();
			m_pobRagdollLG->SetAntiGravity(false);			
			/*CDirection obNewLinearVelocity = m_pobControllingEntity->GetMatrix().GetZAxis();
			obNewLinearVelocity *= m_obLinearVelocity.Length();*/
			m_pobRagdollLG->SetPelvisLinearVelocity(m_obLinearVelocity);

			m_bFinished=true;
		}

		return true;
	}

	float fTurn=0.0f;
	float fTilt=0.0f;

	if (m_pobControllingEntity)
	{
		const CInputComponent* pobInput=m_pobControllingEntity->GetInputComponent();
		const CDirection obPadDirection=pobInput->GetInputDir();
		float fPadMagnitude=pobInput->GetInputSpeed();

		CDirection obInputDirection=obPadDirection * CamMan::GetPrimaryView()->GetCurrMatrix().GetAffineInverse();

		// Scale the pad magnitude		
		if ( fPadMagnitude >= m_pobParameters->m_fMinPadMagnitude)
		{
			fPadMagnitude = (fPadMagnitude - m_pobParameters->m_fMinPadMagnitude) / (1.0f - m_pobParameters->m_fMinPadMagnitude);
		}
		else
		{
			fPadMagnitude = 0.0f;
		}

		// ----- Control multiplier -----
		float fControlMultiplier = 1.0f;

		if (m_fTime>=m_fControlDuration /*|| eRagdollState!=Physics::TRANSFORM_TRACKING_ANIMATED*/) // Control has expired
		{
			fControlMultiplier = 0.0f;
		}
		else if (m_fTime<m_pobParameters->m_fControlFadeIn)
		{
			fControlMultiplier = m_fTime/m_pobParameters->m_fControlFadeIn;
		}
		else if (m_fTime>(m_pobParameters->m_fControlFadeIn+m_pobParameters->m_fControlHold))
		{
			fControlMultiplier = 1.0f - ((m_fTime - m_pobParameters->m_fControlFadeIn - m_pobParameters->m_fControlHold) / m_pobParameters->m_fControlFadeOut);
		}

		fPadMagnitude *= fControlMultiplier;

		// ----- Yaw control -----

		float fDesiredYawDelta = obInputDirection.X() * fPadMagnitude;

		if (fDesiredYawDelta>1.0f)
			fDesiredYawDelta=1.0f;

		if (fDesiredYawDelta<-1.0f)
			fDesiredYawDelta=-1.0f;

		fDesiredYawDelta *= m_pobParameters->m_fMaxYawSpeed * DEG_TO_RAD_VALUE;
		m_fYawDelta += (fDesiredYawDelta-m_fYawDelta) * m_pobParameters->m_fYawAdjustFactor;

		// ----- Pitch control -----

		float fDesiredPitchDelta = obInputDirection.Z() * fPadMagnitude;

		if (fDesiredPitchDelta>1.0f)
			fDesiredPitchDelta=1.0f;

		if (fDesiredPitchDelta<-1.0f)
			fDesiredPitchDelta=-1.0f;

		fDesiredPitchDelta *= m_pobParameters->m_fMaxPitchSpeed * DEG_TO_RAD_VALUE;
		m_fPitchDelta += (fDesiredPitchDelta-m_fPitchDelta) * m_pobParameters->m_fPitchAdjustFactor;

		// Calculate the velocity impulse to apply
		CMatrix obMatrix;

		CCamUtil::CreateFromPoints(obMatrix,m_pobRagdollLG->GetPosition(),CPoint(m_pobRagdollLG->GetPosition()+m_pobRagdollLG->GetLinearVelocity()));
			
		const CDirection& obZAxis = obMatrix.GetZAxis();
		float fPhi = fatan2f(obZAxis.X(), obZAxis.Z());
		float fC = fsqrtf(obZAxis.X()*obZAxis.X() + obZAxis.Z()*obZAxis.Z());
		float fTheta = -fatan2f(obZAxis.Y(), fC);

		fTurn = m_fYawDelta * fTimeStep; // Use absolute time
		fTilt = m_fPitchDelta * fTimeStep; // Use absolute time

		if	( CPlayerOptions::Get().GetInvertY() == false )	// cam-dwhite
		{
			fTilt=-fTilt;
		}

		fPhi += fTurn;

		while(fPhi < 0)
			fPhi += TWO_PI;
		while(fPhi > TWO_PI)
			fPhi -= TWO_PI;

		fTheta += fTilt;

		while(fTheta < 0)
			fTheta += TWO_PI;
		while(fTheta > TWO_PI)
			fTheta -= TWO_PI;

		// Limit the pitch
		const float fUpwardsPitchAngleLimit = m_pobParameters->m_fMaxUpwardsPitchAngle * DEG_TO_RAD_VALUE;
		const float fDownwardsPitchAngleLimit = m_pobParameters->m_fMaxDownwardsPitchAngle * DEG_TO_RAD_VALUE;

		if (fTheta > PI)
		{
			if (fTheta < (TWO_PI-fUpwardsPitchAngleLimit))
			{
				fTheta=TWO_PI-fUpwardsPitchAngleLimit;
			}
		}
		else
		{
			if (fTheta > fDownwardsPitchAngleLimit)
			{
				fTheta=fDownwardsPitchAngleLimit;
			}
		}

		// Calculate new linear velocity
		// DGF - hacking it till it works, no more hierarchy updates, only velocity vector and rotation of that velocity vector
		CMatrix obNewMatrix;
		CCamUtil::MatrixFromEuler_XYZ(obNewMatrix,(fDesiredPitchDelta*0.1f)*(m_fGameTime*m_fGameTime),(fDesiredYawDelta*0.1f)*(m_fGameTime*m_fGameTime),0.0f);
		m_obLinearVelocity = m_obLinearVelocity * obNewMatrix;
		m_pobRagdollLG->SetPelvisLinearVelocity(m_obLinearVelocity);

		// ----- Camera roll -----

		float fCameraRoll= (m_fYawDelta / m_pobParameters->m_fMaxYawSpeed) * m_pobParameters->m_fMaxRollAngle;
		
		int iCamID = CamMan::GetPrimaryView()->GetActiveCameraID();
		CoolCamera* pobCoolCam=(CoolCamera*)CamMan::GetPrimaryView()->GetCoolCam(iCamID);

		if (pobCoolCam && pobCoolCam->GetType()==CT_AFTERTOUCH)
		{
			CoolCam_AfterTouch* pobCam=(CoolCam_AfterTouch*)pobCoolCam;
			pobCam->SetRoll(fCameraRoll);
		}
	}

	/*CMatrix obNewLocalMatrix(m_pobParentEntity->GetHierarchy()->GetRootTransform()->GetLocalMatrix());
    
	if (fTurn!=0.0f)
	{
		obNewLocalMatrix *= CMatrix(CQuat( CDirection(0.0f,1.0f,0.0f), fTurn ));
	}*/

	//CPoint obPosition(m_pobParentEntity->GetPosition());
	//obPosition+=obTranslationDelta;
	//obNewLocalMatrix.SetTranslation(obPosition);

	//m_pobParentEntity->GetHierarchy()->GetRootTransform()->SetLocalMatrix(obNewLocalMatrix);
	//m_pobParentEntity->GetPhysicsSystem()->EntityRootTransformHasChanged();


	if (m_bFirstFrame)
	{
		m_bFirstFrame=false;
	}

	return false;

#else

	UNUSED(fTimeStep);

	return false;

#endif // _PS3_RUN_WITHOUT_HAVOK_BUILD
}
