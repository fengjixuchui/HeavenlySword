//------------------------------------------------------------------------------------------
//!
//!	\file AimController.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_AFTERTOUCHCONTROLLER_INC
#define	_AFTERTOUCHCONTROLLER_INC


/////////////////////////////////
// Required Includes
/////////////////////////////////
#include "movementcontrollerinterface.h"
#include "Physics/advancedcharactercontroller.h"
#include "anonymouscomponent.h"
#include "input/inputhardware.h"

/////////////////////////////////
// External Classes
/////////////////////////////////
class Transform;
class CEntity;


//------------------------------------------------------------------------------------------
//!
//! AftertouchControlParameters
//! General serialised control parameters for aftertouch controllers.
//!
//------------------------------------------------------------------------------------------
class AftertouchControlParameters
{
public:

	AftertouchControlParameters () :
		m_fMinPadMagnitude(0.1f),	
		m_fYawAdjustFactor(0.5f),
		m_fPitchAdjustFactor(0.25f),
		m_fMaxYawSpeed(45.0f),
		m_fMaxPitchSpeed(45.0f),
		m_fMaxUpwardsPitchAngle(25.0f),
		m_fMaxDownwardsPitchAngle(25.0f),
		m_fMaxRollAngle(25.0f),
		m_fControlFadeIn(2.0f),
		m_fControlHold(2.0f),
		m_fControlFadeOut(2.0f),
		m_fMotionSensorMinPadMagnitude(0.0f),
		m_fMotionSensorMaxPadMagnitude(0.25f),
		m_fMotionSensorYawAdjustFactor(0.5f),
		m_fMotionSensorPitchAdjustFactor(0.25f),
		m_fMotionSensorMaxYawSpeed(45.0f),
		m_fMotionSensorMaxPitchSpeed(45.0f),
		m_fMotionSensorMaxUpwardsPitchAngle(25.0f),
		m_fMotionSensorMaxDownwardsPitchAngle(25.0f),
		m_fMotionSensorMaxRollAngle(25.0f),
		m_fMotionSensorControlFadeIn(2.0f),
		m_fMotionSensorControlHold(2.0f),
		m_fMotionSensorControlFadeOut(2.0f)
	{
	}

	float m_fMinPadMagnitude;
	float m_fYawAdjustFactor;
	float m_fPitchAdjustFactor;
	float m_fMaxYawSpeed;
	float m_fMaxPitchSpeed;
	float m_fMaxUpwardsPitchAngle;
	float m_fMaxDownwardsPitchAngle;
	float m_fMaxRollAngle;

	float m_fControlFadeIn;
	float m_fControlHold;
	float m_fControlFadeOut;

	// PS3 Motion Sensor values for the alternative motion sensor aftertouch controller

	float m_fMotionSensorMinPadMagnitude;
	float m_fMotionSensorMaxPadMagnitude;
	float m_fMotionSensorYawAdjustFactor;
	float m_fMotionSensorPitchAdjustFactor;
	float m_fMotionSensorMaxYawSpeed;
	float m_fMotionSensorMaxPitchSpeed;
	float m_fMotionSensorMaxUpwardsPitchAngle;
	float m_fMotionSensorMaxDownwardsPitchAngle;
	float m_fMotionSensorMaxRollAngle;

	float m_fMotionSensorControlFadeIn;
	float m_fMotionSensorControlHold;
	float m_fMotionSensorControlFadeOut;
};


//------------------------------------------------------------------------------------------
//!
//! AfterTouchControllerDef
//!
//------------------------------------------------------------------------------------------
class AfterTouchControllerDef : public MovementControllerDef
{
public:

	// Construction destruction
	AfterTouchControllerDef();
	virtual ~AfterTouchControllerDef() {}

	// Create an instance of what we describe
	virtual MovementController* CreateInstance(CMovement* pobMovement) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone() const { return NT_NEW AfterTouchControllerDef(*this); }

	// ----- Serialised members -----

	CEntity* m_pobControllingEntity;

	AftertouchControlParameters* m_pobParameters;
}; 

//------------------------------------------------------------------------------------------
//!
//! AfterTouchController
//!	All the details needed to define movement
//!
//------------------------------------------------------------------------------------------
class AfterTouchController : public MovementController
{
public:
	AfterTouchController(CMovement* pMovement, const AfterTouchControllerDef* pDefinition);
	virtual ~AfterTouchController();


	virtual bool Update(float fTimeDelta, const CMovementInput& input,
						const CMovementStateRef& currentState, CMovementState& predictedState);

protected:

	CEntity*						m_pobControllingEntity;
	AftertouchControlParameters*	m_pobParameters;
	float							m_fTime;
	float							m_fYawDelta;
	float							m_fPitchDelta;
};


//------------------------------------------------------------------------------------------
//!
//! MotionSensorAfterTouchControllerDef - motion sensor version
//!
//------------------------------------------------------------------------------------------
class MotionSensorAfterTouchControllerDef : public MovementControllerDef
{
public:

	// Construction destruction
	MotionSensorAfterTouchControllerDef();
	virtual ~MotionSensorAfterTouchControllerDef() {}

	// Create an instance of what we describe
	virtual MovementController* CreateInstance(CMovement* pobMovement) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone() const { return NT_NEW MotionSensorAfterTouchControllerDef(*this); }

	// ----- Serialised members -----

	CEntity* m_pobControllingEntity;

	AftertouchControlParameters* m_pobParameters;
};


//------------------------------------------------------------------------------------------
//!
//! MotionSensorAfterTouchController
//!	All the details needed to define movement - motion sensor version
//!
//------------------------------------------------------------------------------------------
class MotionSensorAfterTouchController : public MovementController
{
public:
	MotionSensorAfterTouchController(CMovement* pMovement, const MotionSensorAfterTouchControllerDef* pDefinition);
	virtual ~MotionSensorAfterTouchController();


	virtual bool Update(float fTimeDelta, const CMovementInput& input,
						const CMovementStateRef& currentState, CMovementState& predictedState);

protected:

	CEntity*						m_pobControllingEntity;
	AftertouchControlParameters*	m_pobParameters;
	float							m_fTime;
	float							m_fYawDelta;
	float							m_fPitchDelta;
};


//------------------------------------------------------------------------------------------
//!
//! SpearAfterTouchControllerDef
//!
//------------------------------------------------------------------------------------------
class SpearAfterTouchControllerDef : public MovementControllerDef
{
public:

	// Construction destruction
	SpearAfterTouchControllerDef();
	virtual ~SpearAfterTouchControllerDef() {}

	// Create an instance of what we describe
	virtual MovementController* CreateInstance(CMovement* pobMovement) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone() const { return NT_NEW SpearAfterTouchControllerDef(*this); }

	// ----- Serialised members -----

	CEntity* m_pobControllingEntity;
	AftertouchControlParameters* m_pobParameters;
}; 

//------------------------------------------------------------------------------------------
//!
//! SpearAfterTouchController
//!	All the details needed to define movement
//!
//------------------------------------------------------------------------------------------
class SpearAfterTouchController : public MovementController
{
public:
	SpearAfterTouchController(CMovement* pMovement, const SpearAfterTouchControllerDef* pDefinition);
	virtual ~SpearAfterTouchController();


	virtual bool Update(float fTimeDelta, const CMovementInput& input,
						const CMovementStateRef& currentState, CMovementState& predictedState);

protected:

	CEntity*						m_pobControllingEntity;
	AftertouchControlParameters*	m_pobParameters;
	float							m_fTime;
	float							m_fYawDelta;
	float							m_fPitchDelta;
};









//------------------------------------------------------------------------------------------
//!
//! AfterTouchControllerDef
//!
//------------------------------------------------------------------------------------------
class ProjectileAfterTouchControllerDef : public MovementControllerDef
{
public:

	// Construction destruction
	ProjectileAfterTouchControllerDef();
	virtual ~ProjectileAfterTouchControllerDef() {}

	// Create an instance of what we describe
	virtual MovementController* CreateInstance(CMovement* pobMovement) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone() const { return NT_NEW ProjectileAfterTouchControllerDef(*this); }

	// ----- Serialised members -----

	CEntity*	m_pobControllingEntity;
	AftertouchControlParameters*	m_pobParameters;
	bool		m_bDualController;
}; 

//------------------------------------------------------------------------------------------
//!
//! AfterTouchController
//!	All the details needed to define movement
//!
//------------------------------------------------------------------------------------------
class ProjectileAfterTouchController : public MovementController
{
public:
	ProjectileAfterTouchController(CMovement* pMovement, const ProjectileAfterTouchControllerDef* pDefinition);
	virtual ~ProjectileAfterTouchController();


	virtual bool Update(float fTimeDelta, const CMovementInput& input,
						const CMovementStateRef& currentState, CMovementState& predictedState);

protected:

	CEntity*	m_pobControllingEntity;
	AftertouchControlParameters*	m_pobParameters;
	float		m_fControlTime;
	bool		m_AllowDualInput;

	float		m_fYawDelta;
	float		m_fPitchDelta;
};


//------------------------------------------------------------------------------------------
//!
//! MotionSensorProjectileAfterTouchControllerDef
//!
//------------------------------------------------------------------------------------------
class MotionSensorProjectileAfterTouchControllerDef : public MovementControllerDef
{
public:

	// Construction destruction
	MotionSensorProjectileAfterTouchControllerDef();
	virtual ~MotionSensorProjectileAfterTouchControllerDef() {}

	// Create an instance of what we describe
	virtual MovementController* CreateInstance(CMovement* pobMovement) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone() const { return NT_NEW MotionSensorProjectileAfterTouchControllerDef(*this); }

	// ----- Serialised members -----

	CEntity*	m_pobControllingEntity;
	AftertouchControlParameters*	m_pobParameters;
	bool		m_bDualController;
}; 


//------------------------------------------------------------------------------------------
//!
//! MotionSensorProjectileAfterTouchController
//!	All the details needed to define movement
//!
//------------------------------------------------------------------------------------------
class MotionSensorProjectileAfterTouchController : public MovementController
{
public:
	MotionSensorProjectileAfterTouchController(CMovement* pMovement, const MotionSensorProjectileAfterTouchControllerDef* pDefinition);
	virtual ~MotionSensorProjectileAfterTouchController();


	virtual bool Update(float fTimeDelta, const CMovementInput& input,
						const CMovementStateRef& currentState, CMovementState& predictedState);

protected:

	CEntity*	m_pobControllingEntity;
	AftertouchControlParameters*	m_pobParameters;
	float		m_fControlTime;
	bool		m_AllowDualInput;

	float		m_fYawDelta;
	float		m_fPitchDelta;

};


//------------------------------------------------------------------------------------------
//!
//! Projectile track entity controller def
//!
//------------------------------------------------------------------------------------------
class ProjectileTrackEntityControllerDef : public MovementControllerDef
{
public:

	// Construction destruction
	ProjectileTrackEntityControllerDef();
	virtual ~ProjectileTrackEntityControllerDef() {}

	// Create an instance of what we describe
	virtual MovementController* CreateInstance(CMovement* pobMovement) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone() const { return NT_NEW ProjectileTrackEntityControllerDef(*this); }

	// ----- Serialised members -----
	CEntity* m_pobTrackedEntity;
	CPoint		m_obPosOffset;
	float	m_fTurnWeighting;
}; 

//------------------------------------------------------------------------------------------
//!
//!	Projectile track entity controller
//!
//------------------------------------------------------------------------------------------
class ProjectileTrackEntityController : public MovementController
{
public:
	ProjectileTrackEntityController(CMovement* pMovement, const ProjectileTrackEntityControllerDef* pDefinition);
	virtual ~ProjectileTrackEntityController();

	virtual bool Update(float fTimeDelta, const CMovementInput& input, const CMovementStateRef& currentState, CMovementState& predictedState);

protected:

	CEntity*	m_pobTrackedEntity;
	CPoint		m_obPosOffset;
	float		m_fTurnWeighting;
};







//------------------------------------------------------------------------------------------
//!
//! RagdollThrownControllerDef
//!	All the details needed to define movement
//!
//------------------------------------------------------------------------------------------
class RagdollThrownControllerDef : public MovementControllerDef
{
public:

	// Construction destruction
	RagdollThrownControllerDef();
	virtual ~RagdollThrownControllerDef() {}

	// Create an instance of what we describe
	virtual MovementController* CreateInstance(CMovement* pobMovement) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone() const { return NT_NEW RagdollThrownControllerDef(*this); }

	// ----- Serialised members -----

	CEntity*						m_pobControllingEntity; // Entity thats controlling the thrown ragdoll
	AftertouchControlParameters*	m_pobParameters; // Aftertouch control parameters
	CDirection						m_obLinearVelocity; // Initial linear velocity of the ragdoll
};

class RagdollThrownController : public MovementController
{
public:
	RagdollThrownController(CMovement* pMovement, const RagdollThrownControllerDef& obDefinition);
	virtual ~RagdollThrownController();


	virtual bool Update(float fTimeDelta, const CMovementInput& input, const CMovementStateRef& currentState, CMovementState& predictedState);

protected:

	Physics::AdvancedRagdoll*		m_pobRagdollLG;
	CEntity*						m_pobParentEntity;
	CEntity*						m_pobControllingEntity;
	AftertouchControlParameters*	m_pobParameters;
	CDirection						m_obLinearVelocity;
	float							m_fTime;
	float							m_fGameTime;
	float							m_fControlDuration;
	float							m_fPitchDelta;
	float							m_fYawDelta;
	bool							m_bFinished;
};


















#endif //_AFTERTOUCHCONTROLLER_INC
