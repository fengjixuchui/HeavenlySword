//------------------------------------------------------------------------------------------
//!
//!	\file TurretController.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_TURRETCONTROLLER_INC
#define	_TURRETCONTROLLER_INC


/////////////////////////////////
// Required Includes
/////////////////////////////////
#include "movementcontrollerinterface.h"


/////////////////////////////////
// External Classes
/////////////////////////////////
class Transform;
class CEntity;
class Interactable_TurretWeapon;

//Enum to track turret states

enum TURRET_STATE {
	TS_INACTIVE,
	TS_FIRE,
	TS_RECOIL,
	TS_SETTLE,
	TS_DAMPED,
};

//------------------------------------------------------------------------------------------
//!
//! TurretControllerDef
//!	All the details needed to define movement
//!
//------------------------------------------------------------------------------------------
class TurretControllerDef : public MovementControllerDef
{
	// Declare dataobject interface
	HAS_INTERFACE(TurretControllerDef)

public:

	// Construction destruction
	TurretControllerDef( void );
	virtual ~TurretControllerDef( void ) {}

	// Create an instance of what we describe
	virtual MovementController* CreateInstance(CMovement* pobMovement) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW TurretControllerDef( *this ); }

	// Our vehicle
	CEntity*		m_pVehicle;
	CHashedString	m_sDrivingSeat;
	ntstd::List<CHashedString>	m_aobYawTransformName;		// Transform for movement
	ntstd::List<CHashedString>	m_aobPitchTransformName;		// Transform for movement
	ntstd::List<CHashedString>	m_aobCombinedTransformName;		// Transform for movement
	CHashedString	m_obIdleAnimation;			// Idle anim
	
	CHashedString	m_obPullLeftUpAnimation;		//
	CHashedString	m_obPullLeftMidAnimation;		//
	CHashedString	m_obPullLeftDownAnimation;		//
	CHashedString	m_obPullRightUpAnimation;		
	CHashedString	m_obPullRightMidAnimation;		
	CHashedString	m_obPullRightDownAnimation;		
	CHashedString	m_obPullCentreUpAnimation;		
	CHashedString	m_obPullCentreDownAnimation;		

	CHashedString	m_obRecoilUpAnimation;			// Recoil anim
	CHashedString	m_obRecoilMidAnimation;			// Recoil anim
	CHashedString	m_obRecoilDownAnimation;		// Recoil anim

	CPoint			m_ptTranslationOffset;
	
	// Movement Parameters
	float		m_fMaxUpPitch;
	float		m_fMaxDownPitch;

	float		m_fMaxYaw;

	float		m_fYawMinSpeed; // Min yaw speed at full tilt
	float		m_fYawMaxSpeed; // Max yaw speed at full tilt
	float		m_fYawAcceleration; // Yaw acceleration at full tilt

	float		m_fPitchMinSpeed; // Min pitch speed at full tilt
	float		m_fPitchMaxSpeed; // Max pitch speed at full tilt
	float		m_fPitchAcceleration; // Pitch acceleration at full tilt

	float		m_fSoftLockSpeedModifier; // Speed modifier when reticle moves over an enemy

	float 		m_fPadThreshold;
	float		m_fPadAccelerationThreshold;
	
	bool		m_bAIControlled;

	float		m_fRecoilAngle;
	float		m_fRecoilTime;

	float		m_fMaxAimLossAngle;

	float		m_fSettleAngle;
	float		m_fSettleTime;

	float		m_fDampedTime;
	float		m_fDampedAngle;
	float		m_fDampedFrequency;

	float		m_fUserLeanDelta;
	float		m_fUserRecoilTime;
};

//------------------------------------------------------------------------------------------
//!
//!	TurretController
//!	The main controller for the standard movement
//!
//------------------------------------------------------------------------------------------
class TurretController : public MovementController
{
public:
	TurretController(CMovement* pMovement, const TurretControllerDef& rDefinition);
	virtual ~TurretController();


	virtual bool Update(float fTimeDelta, const CMovementInput& input,
						const CMovementStateRef& currentState, CMovementState& predictedState);

	void PostAnimatorUpdate ();

protected:

	bool PlayerUpdate(float fTimeDelta, const CMovementInput& input,
						const CMovementStateRef& currentState, CMovementState& predictedState);

	bool AIUpdate(float fTimeDelta, const CMovementInput& input,
						const CMovementStateRef& currentState, CMovementState& predictedState);

	bool TurretUpdate(float fTimeDelta, const CMovementInput& input,
						const CMovementStateRef& currentState, CMovementState& predictedState);

	TurretControllerDef			m_obDefinition;
	Interactable_TurretWeapon*	m_pVehicle;
	Transform*					m_pDrivingSeat;
	Transform*					m_pRoot;
	ntstd::Vector<Transform*>	m_apYawTransform;
	ntstd::Vector<Transform*>	m_apPitchTransform;
	ntstd::Vector<Transform*>	m_apCombinedTransform;

	CAnimationPtr				m_pobIdleAnim;


	CAnimationPtr				m_pobPullLeftUpAnim;
	CAnimationPtr				m_pobPullLeftMidAnim;
	CAnimationPtr				m_pobPullLeftDownAnim;

	CAnimationPtr				m_pobPullRightUpAnim;
	CAnimationPtr				m_pobPullRightMidAnim;
	CAnimationPtr				m_pobPullRightDownAnim;

	CAnimationPtr				m_pobPullCentreUpAnim;
	CAnimationPtr				m_pobPullCentreDownAnim;

	CAnimationPtr				m_pobRecoilUpAnim;
	CAnimationPtr				m_pobRecoilMidAnim;
	CAnimationPtr				m_pobRecoilDownAnim;

	float	m_fYawSpeed;
	float	m_fPitchSpeed;

	float	m_fYawDelta;
	float	m_fPitchDelta;

	float	m_fPitch;
	float	m_fYaw;
	float	m_fInitialTurretYaw;
	float	m_fInitialTurretPitch;

	float	m_fCurrRecoilTime;
	float	m_fCurrRecoilAngle;
	float	m_fCurrDampAngle;

	float	m_fAimLossAngle;
	float	m_fCurrAimLossAngle;
	float	m_fAcheivedRecoilAngle;

	float	m_fUserLean;
	float	m_fCurrUserRecoilTime;

	CMatrix m_obInitalRoot;
};


#endif //_TURRETCONTROLLER_INC
