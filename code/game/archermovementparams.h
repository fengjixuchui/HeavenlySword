//------------------------------------------------------------------------------------------
//!
//!	\file archermovementparams.h
//!
//------------------------------------------------------------------------------------------

#ifndef	ARCHERMOVEMENTPARAMS_H_
#define	ARCHERMOVEMENTPARAMS_H_

#include "editable/flipflop.h"

class CEntity;
class ThirdPersonAttackState;

//------------------------------------------------------------------------------------------
//!
//!	ThirdPersonTargetingSegment
//!
//------------------------------------------------------------------------------------------
class ThirdPersonTargetingSegment
{
public:
	ThirdPersonTargetingSegment(void);

	/// Base rotation about the entities root transform pointing down the z axis.
	float	m_fBaseRotation;

	// Angle of the segment
	float	m_fAngle;

	// radius extents of the segment. 
	float	m_fRadius;

	// segment height.
	float	m_fHeight;

	// 
	CVector	m_vDebugColour;
};

//------------------------------------------------------------------------------------------
//!
//!	ThirdPersonTargeting
//!
//------------------------------------------------------------------------------------------
class ThirdPersonTargeting
{
public:
	// Construction power. 
	ThirdPersonTargeting();

	// Obtain a list of entities from the targeting data.
	void GetTargetList( const CMatrix& rmatRef, ntstd::List<CEntity*>& obEntList ) const;

	// Is the active target still valid?
	bool IsActiveTargetValid( const CMatrix& rmatRef, const CEntity* ) const;

	// List of segements. 
	ntstd::List<ThirdPersonTargetingSegment*>	m_SegmentList;

	// List of targets for the active target. 
	ntstd::List<ThirdPersonTargetingSegment*>	m_ActiveTargetSegmentList;
};


//------------------------------------------------------------------------------------------
//!
//!	ThirdPersonAttackTransition
//!
//------------------------------------------------------------------------------------------
class ThirdPersonAttackTransition
{
public:
	ThirdPersonAttackTransition(void);

	// Transition Movement controller magnitude requirement
	float	m_InputMagnitude;

	// Angle of the movement controller relative to the facing direction of the archer
	float	m_HorizontalAngleOffset;

	// Tolerance of the angle that can be matched. 
	float	m_HorizontalAngleRange;

	// Name of the transition animation
	ntstd::String	m_TransitionAnim;

	// Speed the animation should play at
	float		m_TransitionAnimSpeed;

	// Time the animation should blend in to 100% blend
	float		m_BlendInTime;

	// If set, gives a point at which the animation ends itselfs
	float		m_EarlyOutTime;

	// If set, the number time in the variable is the amount of time the state must have run for 
	float		m_StateUpTimeCheck;

	// The state the transition will move into
	ThirdPersonAttackState* m_TransitionTo;
};

//------------------------------------------------------------------------------------------
//!
//!	ThirdPersonAttackState
//!
//------------------------------------------------------------------------------------------
class ThirdPersonAttackState
{
public:
	ThirdPersonAttackState(void);

	// The current idle anim, this will likely change...
	CHashedString								m_IdleAnim, m_IdleUpAnim;
	CHashedString								m_FireAnim, m_FireUpAnim;

	// Anims to blend to from the centre to the right of the character
	CHashedString								m_IdleRightAim, m_IdleRightUpAim;
	CHashedString								m_FireRightAim, m_FireRightUpAim;

	// Anims to blend to from the centre to the left of the character
	CHashedString								m_IdleLeftAim, m_IdleLeftUpAim;
	CHashedString								m_FireLeftAim, m_FireLeftUpAim;

	// Hashed string that is used to play a pop out animation
	CHashedString								m_PopoutAnim;

	// Animation used to reload the weapon
	CHashedString								m_ReloadAnim;

	// Weapon anims
	CHashedString								m_WeaponIdle, m_WeaponFire, m_WeaponReload;


	// The popout time for the fire animation - currently not used. 
	CFlipFlop									m_FirePopout;

	// Popout transition, it's possible to pop out of the this state after a given time, and transition to another state.
	// If it's not set, then the next character pose will be the replacement. 
	ThirdPersonAttackTransition*				m_PopoutTransition;

	// Time for the current state until it should popout
	float										m_PopoutTime;

	// Popout time if the magnitude of the input is greater than 1
	float										m_PopoutTimeMag1;

	// The extreme range of the left /right animstions
	float										m_AngleRange;

	// Limit the vertical angle
	float										m_VerticalAngleLimit;

	// Limit the rotation input to the following deg per second
	float										m_RotationSpeedLimit;

	// Lockon threshold is a value that will snap the archer to face a target for a given angle
	float										m_LockOnThreshold;

	// Transitions of the state.
	ntstd::List<ThirdPersonAttackTransition*>	m_Transitions;

	//
	const ThirdPersonTargeting*						m_Targeting;
};

//------------------------------------------------------------------------------------------
//!
//!	Exposed XML archer vaulting animations
//!
//------------------------------------------------------------------------------------------
class ArcherVaultParams
{
public:

	// Is the vaulting param given here enabled. 
	bool		m_bEnabled;

	// Name of the vault animation.
	CKeyString	m_VaultAnimName;				

	// Distance scaling window
	CFlipFlop	m_DistanceScaling;

	// Popout window for the next vaulting animation
	CFlipFlop	m_NextVaultPopout;

	// Popout window for the exiting the vaulting animation
	CFlipFlop	m_MovementPopout;

	// The minimum distance the jump will have to be to get this animation to play
	float		m_fJumpDistanceRequired;

	// The max distance this animation should attempt to jump
	float		m_fJumpDistanceLimit;

	// The time the animation has to blend out. 
	float		m_fBlendOutTime;

};

//------------------------------------------------------------------------------------------
//!
//!	Expose to XML some archer movement parameters related to crouching and vaulting.
//!
//------------------------------------------------------------------------------------------
class ArcherMovementParams
{
	public:
		//
		//	Ctor, dtor.
		//
		ArcherMovementParams		();
		~ArcherMovementParams		()		{}

	public:
		//
		//	Post-construction and value changed functions.
		//
		void		PostConstruct	();
		bool		ValueChanged	( CallBackParameter, CallBackParameter);
		void		DeleteCallback	();

	public:
		//
		//	Movement parameters.
		//
		float		m_VaultRayCastLength;			// How far should we ray-cast to test if we can vault?
		float		m_CosVaultRayAngle;				// Angle between the centre test-ray and the two outer test-rays for vault testing.
		float		m_VaultUnitHeight;				// Height of a one-unit object that the archer can vault.
		float		m_VaultRayCastZOffset;			// Adjust the starting point of the ray

		// Name of the high-vault animation.
		ntstd::List<ArcherVaultParams*>	m_HighVaultAnims;

		// Name of the low-vault animation.
		ntstd::List<ArcherVaultParams*>	m_LowVaultAnims;

		float		m_CrouchVolumeLength;			// Length of both of the capsule phantoms used for working out whether to crouch or not.
		float		m_CrouchVolumeRadius;			// Radius of both of the capsule phantoms used for working out whether to crouch or not.

		float		m_VaultCacheTimeOut;			// The maximum time that a vault button-press will stay cached before being removed.
};

#endif // !ARCHERMOVEMENTPARAMS_H_

