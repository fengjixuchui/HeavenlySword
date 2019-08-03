//------------------------------------------------------------------------------------------
//!
//!	\file AimController.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_AIMCONTROLLER_INC
#define	_AIMCONTROLLER_INC


/////////////////////////////////
// Required Includes
/////////////////////////////////
#include "movementcontrollerinterface.h"
#include "anonymouscomponent.h"
#include "editable/flipflop.h"
#include "effect/effect.h"
#include "effect/worldsprite.h"

/////////////////////////////////
// External Classes
/////////////////////////////////
class Transform;
class CEntity;
class Player;
class Archer;
class Character;
class AimingLaserEffectDef;
class AimingLaserEffect;
class ProjectileProperties;


//------------------------------------------------------------------------------------------
//!
//! AimingComponent
//!	Keeps track of a specific direction an entity might be aiming in - this is a direction
//! which is independant of the direction the parent entity is facing in.
//!
//------------------------------------------------------------------------------------------
class AimingComponent : public CAnonymousEntComponent
{
public:

	AimingComponent (CEntity* pobParentEntity,float fPitchSpeed,float fYawSpeed,float fMinPitch,float fMaxPitch);
	virtual ~AimingComponent(void) {}

	void SetFromParent ();

	float AdjustPitch (float fDelta,float fTimeStep); // Move up/down - return the pitch adjustment in radians
	float AdjustYaw (float fDelta,float fTimeStep); // Move left/right - return the yaw adjustment in radians

	float GetPitch () { return m_fPitch; }
	float GetYaw () { return m_fYaw; }
	float GetMinPitch () { return m_fMinPitch; }
	float GetMaxPitch () { return m_fMaxPitch; }
	float GetMinYaw () { return m_fMinYaw; }
	float GetMaxYaw () { return m_fMaxYaw; }

	void SetAimRange (float fMinYaw, float fMaxYaw, float fMinPitch, float fMaxPitch); 
	void ResetAimRange ( ); 
	void ResetAngles(void) { m_fPitch = 0.0f; m_fYaw = 0.0f; }

	const CPoint& GetResetAnglePoint(void) const { return m_ptResetAnglePoint; }
	void SetResetAnglePoint( const CPoint& rptSetValue ) { m_ptResetAnglePoint = rptSetValue; }

private:
	
	CEntity* m_pobEntity;

	float m_fPitch; // X axis rotation in radians
	float m_fYaw; // Y axis rotation in radians

	float m_fPitchSpeed;
	float m_fYawSpeed;

	// Default ranges so they can be reset
	const float m_fDefaultMinPitch;
	const float m_fDefaultMaxPitch;
	const float m_fDefaultMinYaw;
	const float m_fDefaultMaxYaw;

	// Currently active ranges, as these can be overriden by turret points
	float m_fMinPitch;
	float m_fMaxPitch;
	float m_fMinYaw;
	float m_fMaxYaw;

	// If the player moves X meters from this point when the aiming controller
	// is activated then the pitch and yaw angles should be reset. 
	CPoint	m_ptResetAnglePoint;
};


//------------------------------------------------------------------------------------------
//!
//! AimingControllerDef
//!	All the details needed to define movement
//!
//------------------------------------------------------------------------------------------
class AimingControllerDef : public MovementControllerDef
{
public:

	// Construction destruction
	AimingControllerDef();
	virtual ~AimingControllerDef() {}

	// Create an instance of what we describe
	virtual MovementController* CreateInstance(CMovement* pobMovement) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone() const { return NT_NEW AimingControllerDef(*this); }

	// ----- Serialised members -----

	CHashedString 	m_obLeftUpAnimation;		CHashedString 	m_obWeaponLeftUpAnimation;
	CHashedString 	m_obLeftMidAnimation;		CHashedString 	m_obWeaponLeftMidAnimation;
	CHashedString 	m_obLeftDownAnimation;		CHashedString 	m_obWeaponLeftDownAnimation;
												
	CHashedString 	m_obCentreUpAnimation;		CHashedString 	m_obWeaponCentreUpAnimation;
	CHashedString 	m_obCenterMidAnimation;		CHashedString 	m_obWeaponCenterMidAnimation;
	CHashedString 	m_obCenterDownAnimation;	CHashedString 	m_obWeaponCenterDownAnimation;
					
	CHashedString 	m_obRightUpAnimation;		CHashedString 	m_obWeaponRightUpAnimation;
	CHashedString 	m_obRightMidAnimation;		CHashedString 	m_obWeaponRightMidAnimation;
	CHashedString 	m_obRightDownAnimation;		CHashedString 	m_obWeaponRightDownAnimation;

	// Firing animations. 
	CHashedString 	m_obUpFireAnimation;		CHashedString 	m_obWeaponUpFireAnimation;
	CHashedString 	m_obCentreFireAnimation;	CHashedString 	m_obWeaponCentreFireAnimation;
	CHashedString 	m_obDownFireAnimation;		CHashedString 	m_obWeaponDownFireAnimation;

	CHashedString	m_obHopAnimation;

	float		m_fYawMinSpeed; // Min yaw speed at full tilt
	float		m_fYawMaxSpeed; // Max yaw speed at full tilt
	float		m_fYawAcceleration; // Yaw acceleration at full tilt

	float		m_fPitchMinSpeed; // Min pitch speed at full tilt
	float		m_fPitchMaxSpeed; // Max pitch speed at full tilt
	float		m_fPitchAcceleration; // Pitch acceleration at full tilt

	
	// Speed modifier when reticle moves over an enemy
	float		m_fSoftLockSpeedModifier; 

	//	Point at where the pad input translates to camera movement
	float		m_fCamMovementThreshold;

	float 		m_fPadThreshold;
	float		m_fPadAccelerationThreshold;

	// Adjust the mapping of the pad input to screen sapce
	float		m_fPadInputScreenSpaceScaleX;
	float		m_fPadInputScreenSpaceScaleY;

	// Adjust the rate of smoothing on the intersection point
	float		m_fCamRaySmootherScalar;

	// Offset the ray from the near plane
	float		m_fCamRayStartOffset;

	// Allow the rate of pitch and yaw values to be scaled.
	float		m_fAimingYawScalar;
	float		m_fAimingPitchScalar;

	// Allow the yaw movement values to be scaled.
	float		m_fMovingYawScalar;

	// 
	ntstd::String	m_WeaponName;

	// A while back it was assumed that the root of 
	CHashedString	m_hWeaponAimingDirection;

	// Window where the player can popout of the current firing state and start the firing state again (rate of fire)
	CFlipFlop		m_obFiringPopoutWindow;

	// AK47 style recoil accumalater
	float			m_fAKRecoil;

	// Window where the players control is poked with values that sumulate the recoil effect
	CFlipFlop		m_obFiringAKRecoilWindow;

	// Modifiers that adjust the AK47 style recoil accumaltion
	float			m_fAKRecoilModifier;
	float			m_fAKRecoilModifierFilter;

	//
	float		m_fConstYawBlendFactor;
	float		m_fConstMagnitudePower;
	float		m_fConstMagnitudeModifier;
	float		m_fConstYawDeacceleration;
	float		m_fConstYawMinimumDelta;

	// Offset on the y direction for the beam of light (Archer only)
	float		m_fBeamUpOffset;

	// Pointer to the structure for the laser dot. 
	AimingLaserEffectDef*  m_pLaserEffect;

	// Charged FOV params
	float		m_fChargedFOV;
	float		m_fChargedFOVTime;

	// 
	bool		m_bUseGravity;

	// delayed time before the input is applied to the character
	float		m_fInputDelay;
}; 


//!------------------------------------------------------------------------------
//!  AimingLaserEffectDef
//!  <TODO: insert class description here>
//!
//!
//!  @author GavB @date 20/11/2006
//!------------------------------------------------------------------------------
class AimingLaserEffectDef
{
public:
	AimingLaserEffectDef( void );

public:

// [ Laser dot params ]
	
	// Size of the dot at the nearest point
	float		m_fDotSize;

	// How bright the dot is at it's closest point
	float		m_fDotLuminosity;

	// Adjust the position of the dot relative to the camera direction - the aim is to prevent the 
	// dot clipping through items in the background
	float		m_fCameraAdjust;

	// The colour fall off rate until the dot isn't visible. 
	float		m_fDotLuminosityFallOff;

	// Colour of the dot
	CVector		m_obDotColour;

	// Name of the dot texture
	ntstd::String m_obDotTexture;
};


//!------------------------------------------------------------------------------
//!  AimingLaserEffect
//!  <TODO: insert class description here>
//!
//!  Base class Effect <TODO: insert base class description here>
//!
//!  @author GavB @date 20/11/2006
//!------------------------------------------------------------------------------
class AimingLaserEffect : public Effect
{
public:

	// Bog-standard constructor
	AimingLaserEffect(const AimingLaserEffectDef*);
	virtual ~AimingLaserEffect(void);

	// Set the point for the laser .
	void SetLaserDotPoint( const CPoint& rptLaserDot )
	{
		m_EnableLaserDot	= true;
		m_ptDot				= rptLaserDot;
	}

	// 
	virtual bool UpdateEffect();
	virtual void RenderEffect();

	// I'm just guessning these vaiables 
	virtual bool HighDynamicRange() const { return false; }
	virtual bool WaitingForResources()const { return false; }


	// Get the effect ID
	u_int GetEffectID() const { return m_EffectID; }

private:

	// Point to the laser dot. 
	CPoint	m_ptDot;

	// Pointer to the data that defines this instance.
	const AimingLaserEffectDef* m_pDef;

	// Enables rendering of the laser dot. 
	bool m_EnableLaserDot;

	// The ID of this effect
	u_int m_EffectID;

	// Sprite for the 'freaking' laser dot
	WorldSprite		m_DotSprite;
};


//------------------------------------------------------------------------------------------
//!
//! AimingController
//!	All the details needed to define movement
//!
//------------------------------------------------------------------------------------------
class AimingController : public MovementController
{
public:

	// Animations
	enum ANIMATIONS 
	{
		LeftUpAnimation,			LeftMidAnimation,			LeftDownAnimation,
		CentreUpAnimation,			CenterMidAnimation,			CenterDownAnimation,
		RightUpAnimation,			RightMidAnimation,			RightDownAnimation,
		UpFireAnimation,			CentreFireAnimation,		DownFireAnimation,
		BobAnimation,
		ChargeAnimation,			ChargedAnimation, 
		ANIM_COUNT
	};

public:
	AimingController(CMovement* pMovement, const AimingControllerDef* pDefinition, const CEntity* = 0);
	virtual ~AimingController();


	virtual bool Update(float fTimeDelta, const CMovementInput& input,
						const CMovementStateRef& currentState, CMovementState& predictedState);

protected:

	// Helper for sticking our animations onto the animator
	void AddAnimations( void );

	const AimingControllerDef*	m_pobAimingControllerDef;

	// Projectile properties, just temporary hack for GameShare at 26.11.2006. 
	// Later could be part of AimingControllerDef
	ProjectileProperties *      m_pobProjectileProperties; 

	AimingComponent*			m_pobAimingComponent;

	CAnimationPtr				m_obAnims[ANIM_COUNT];

	float						m_fYawBlend;

	float						m_fYawAcc;
	float						m_fPitchAcc;
	float						m_fPitch;

	float						m_StateTime;
	float						m_ReloadTime;
	float						m_FireTime;

	float						m_PadReadFudge;

	int							m_iCamID;

	// Save the direction of the magnitude, when the direction of the pad input changes, reset the acceleration.
	bool						m_bMagDirection;

	// Does the archer have firing animations.
	bool						m_bHasFiringAnims;

	// pointer the signifies the entity is a player
	const Player*				m_pPlayer;

	CMatrix						m_matInitCamMatrix;
	CDirection					m_dirRequiredFacing;

	CPoint						m_ptIntersect;

	// Scalar modifier for the recoil system
	float						m_fRecoilModifier;

	// Pointer to the weapon entity
	const CEntity*				m_pWeaponEnt;

	// Laser stuff.. 
	AimingLaserEffect*			m_pLaser;

	// If the shot is now fully charged.. 
	bool						m_bShotCharged;
};


//------------------------------------------------------------------------------------------
//!
//! AimingLaunchControllerDef
//!	All the details needed to define movement
//!
//------------------------------------------------------------------------------------------
class AimingLaunchControllerDef : public MovementControllerDef
{
public:

	// Construction destruction
	AimingLaunchControllerDef( void );
	virtual ~AimingLaunchControllerDef( void ) {}

	// Create an instance of what we describe
	virtual MovementController* CreateInstance(CMovement* pobMovement) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW AimingLaunchControllerDef( *this ); }

	// ----- Serialised members -----

	CHashedString 	m_obCentreUpAnimation;
	CHashedString 	m_obCenterMidAnimation;
	CHashedString 	m_obCenterDownAnimation;

	float 		m_fYawSpeedMultiplier;
	float 		m_fPitchSpeedMultiplier;
	float 		m_fPadThreshold;

	float 		m_fAccelerationFactor;
	float 		m_fDeaccelerationFactor;

	bool		m_bAllowMovement;
};



//------------------------------------------------------------------------------------------
//!
//! AimingLaunchController
//!	All the details needed to define movement
//!
//------------------------------------------------------------------------------------------
class AimingLaunchController : public MovementController
{
public:
	AimingLaunchController(CMovement* pMovement, const AimingLaunchControllerDef* pDefinition);
	virtual ~AimingLaunchController();


	virtual bool Update(float fTimeDelta, const CMovementInput& input,
						const CMovementStateRef& currentState, CMovementState& predictedState);

protected:

	CAnimationPtr 	m_obCentreUpAnimation;
	CAnimationPtr 	m_obCenterMidAnimation;
	CAnimationPtr 	m_obCenterDownAnimation;

	float			m_fCentreUpBlend;
	float			m_fCentreMidBlend;
	float			m_fCentreDownBlend;
};

//------------------------------------------------------------------------------------------
//!
//! AimingWeaponControllerDef
//!	All the details needed to define movement
//!
//------------------------------------------------------------------------------------------
class AimingWeaponControllerDef : public MovementControllerDef
{
public:
	// Construction destruction
	AimingWeaponControllerDef();
	virtual ~AimingWeaponControllerDef() {}

	// Create an instance of what we describe
	virtual MovementController* CreateInstance(CMovement* pobMovement) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone() const { return NT_NEW AimingWeaponControllerDef(*this); }

	// Idle for the xbow.
	CHashedString 	m_obIdle;

	// Charge up animations
	CHashedString 	m_obArm1ChargeUp, m_obArm2ChargeUp;		

	// Charged animations
	CHashedString 	m_obArm1Charged, m_obArm2Charged;		

	// Firing anims for each of the arms. 
	CHashedString 	m_obArm1Firing, m_obArm2Firing;

	// Pointer the entity holding this weapon
	const Character* m_pPlayer;
}; 


//------------------------------------------------------------------------------------------
//!
//! AimingWeaponController
//!	All the details needed to define movement
//!
//------------------------------------------------------------------------------------------
class AimingWeaponController : public MovementController
{
public:

	enum ANIMATIONS
	{
		// Idle animation for the xbow, really just there to set the corrent 
		// keys for the transform of the xbow relative to Kia
		IDLE,

		// Animations for each of the charge up states for the arms. 
		ARM_1_CHARGE_UP,
		ARM_2_CHARGE_UP,

		// Animations for each of the charged states for the arms. 
		ARM_1_CHARGED,
		ARM_2_CHARGED,

		// Animations for each of the fired states of the arms. 
		ARM_1_FIRED,
		ARM_2_FIRED,

		// Number of animations for the xbow (these'll all get loaded)
		ANIM_COUNT,
	};

	//
	enum ANIMATION_SET
	{
		// Holds the base orientation for the xbow. 
		BASE_ANIM,

		// There are two partial anims that can be run on the xbow at anyone time. These are for the upper and 
		// lower arms of the xbow. 
		PARTIAL_ANIM_1,
		PARTIAL_ANIM_2,

		ANIM_SET_COUNT,
	};

	// Define the arms on the xbow
	enum ARMS
	{
		XBOW_UPPER_ARM,
		XBOW_LOWER_ARM,

		XBOW_ARM_COUNT,
	};


	// As each arm has it's own state, it's best to keep the two seperate in mind
	//
	// Here defines the possible states of the xbow arm other various useful params
	// used during the state switching. 
	struct ArmState
	{
		// The charge variable, whilst being only a float value can hold state information
		// with the use of the negative state and the window of values it represents. 
		// 0.0 > 1.0 (Charging)	(firing possible)
		// 1.0 > (Charged ( time charged) ) (firing possible)
		// 0.0 < just fired, the xbow must wait until the charge value is greater than 0 to fire again (firing not possible)
		float	m_fChargeState;


		// Default construction for the arm state.
		ArmState( void ) : m_fChargeState( 0.0f ) {}
	};

public:

	// Constructor and destructor to the contorller (standard within the framework)
	AimingWeaponController(CMovement* pMovement, const AimingWeaponControllerDef* pDefinition);
	virtual ~AimingWeaponController();

	virtual bool Update( float						fTimeDelta, 
						 const CMovementInput&		input, 
						 const CMovementStateRef&	currentState, 
						 CMovementState&			predictedState);

	// Specific code for the archer character
	void UpdateArcher( float fTimeStep, const Archer* );


private:

	// Pointer the controller def. 
	const AimingWeaponControllerDef* m_pobAimingControllerDef;

	// Pointer the entity holding this weapon
	const Character* m_pPlayer;

	// All the aniamtions used by the xbow
	CAnimationPtr m_obAnims[ ANIM_COUNT ];

	// Array of animations used for the weapon
	CAnimationPtr m_obActiveAnims[ ANIM_SET_COUNT ];

	// Hold the state information for the xbow arms. 
	ArmState m_aobArms[ XBOW_ARM_COUNT ];
	
};



#endif //_AIMCONTROLLER_INC
