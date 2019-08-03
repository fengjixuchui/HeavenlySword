//------------------------------------------------------------------------------------------
//!
//!	\file game/entityplayer.h
//!	Definition of the Player entity object
//!
//------------------------------------------------------------------------------------------

#ifndef	_ENTITY_PLAYER_H
#define	_ENTITY_PLAYER_H

#include "input/inputhardware.h"
#include "game/entitycharacter.h"

class ThirdPersonAttackState;
class ThirdPersonAttackTransition;
class Archer;
class Hero;

//------------------------------------------------------------------------------------------
//!
//! Class Player.
//! Player entity type
//!
//------------------------------------------------------------------------------------------
class Player : public Character
{
	// Declare dataobject interface
	HAS_INTERFACE(Player)

public:
	// Create an exposed enum list
	enum STATE
	{
		ARC_VAULTING,
		ARC_IDLE,
		ARC_TRANSITION,
		ARC_AIMING,
		ARC_FIRING,
		ARC_1ST_AIMING,
		ARC_1ST_FIRING
	};


public:
	Player();
	~Player();

	void OnPostPostConstruct();

	// Set Check the Primary Playable Player
	void SetPrimaryPlayer();
	void IsPrimaryPlayer();

	// Components
	COMPONENT_ACCESS( CInputComponent,		GetInputComponent,			m_pobInputComponent );


	// Input Methods
	void	SetInputComponent( CInputComponent* pobInput )	{ m_pobInputComponent = pobInput; };

	// Methods for the third person aiming and shooting...

	// Indicate when a trasition has started.
	void ThirdPersonAttackTransitionStarted( const ThirdPersonAttackTransition* );
	void SetThirdPersonAimingState	(const ThirdPersonAttackState* pState) { m_State = m_State == ARC_IDLE ? ARC_AIMING : m_State; m_CurrentState = pState; }

	// Is there a request to shoot? This should only be accessible by the archer 3rd person controller. 
	void	RequestFire				(bool bValue)	{ m_bFireRequestQueued = bValue; }
	void	RequestReload			(bool bValue)	{ m_bReloadRequest = bValue; }

	bool	RequestFire				()	const { return m_bFireRequest; }
	bool    RequestedFire			()	const { return m_bFireRequestQueued; }
	bool	RequestReload			()	const { return m_bReloadRequest; }
	void	FireRequestCompleted	()	const { m_bFireRequestCompleted = true; }
	u_int	GetFiredCount			()	const { return m_FiredCount; }
	void	IncFiredCount			()	const { ++m_FiredCount; m_bFireRequest = false; }
	void	ResetFiredCount			()	const { m_FiredCount = 0; m_bReloadRequest = false; }

	// Queue a requested firing of the current weapon
	void	RequestedFire(bool bValue)	{ m_bFireRequestQueued = bValue; }

	// Disable fire feedback
	void	DisableFireFeedback		(bool bValue) const { m_bDisableFireFeedback = bValue; }
	bool	DisableFireFeedback		() const { return m_bDisableFireFeedback; }

	// Aftertouch handling
	void	AfterTouchState			(bool bValue)	{ m_AfterTouchState = bValue; }
	bool	AfterTouchState			() const		{ return m_AfterTouchState; }

	// Return the current target locked on to
	CEntity*		GetLockOnTarget				()	const { return m_pLockOnTarget; }

	// Return the current rotation angle
	float			GetCurrentRotationAngle		()	const { return m_CurrentAngleSmoothed; }

	// Get the last rotation angle
	float			GetLastRotationAngle		()	const { return m_LastAngleSmoothed; }

	// Allow the angles to get reset externally
	void			ResetRotationAngles			()  { 	m_CurrentAngle = m_LastAngle = m_CurrentAngleSmoothed = m_LastAngleSmoothed = 0.0f; }


	// Is the player type the archer?
	bool			IsArcher					()	const	{ return m_bArcher; }
	Archer*			ToArcher					()			{ ntError( IsArcher() && "Typecast not valid" ); return (Archer*) this; }
	const Archer*	ToArcher					()	const	{ ntError( IsArcher() && "Typecast not valid" ); return (const Archer*) this; }

	bool			IsHero						()	const	{ return !m_bArcher; }
	Hero*			ToHero						()			{ ntError( IsHero() && "Typecast not valid" ); return (Hero*) this; }
	const Hero*		ToHero						()	const	{ ntError( IsHero() && "Typecast not valid" ); return (const Hero*) this; }

	// Return the current state of the archer. 
	STATE			GetState					()	const	{ return m_State; }

	// Reset the shooting system back to an idle state
	void		ResetToIdleState				() { m_State = ARC_IDLE; m_StateTime = 0.0f; m_Mag1Time = 0.0f; }

	/// Common update function 
	void			UpdateAiming				( float );
	void			UpdateHealth				( float );

	virtual void ChangeHealth( float fDelta, const char* pcReason );

	// Allow the player to adjust the dead zone handling of the pad input
	void			EnableAimingPadDeadZone		( void );
	void			RestoreAimingPadDeadZone	( void );


protected:

	// Updates for various player states
	void 			UpdateIdle				( float );
	void 			UpdateAimingAndFiring	( float );
	void 			UpdateTransition		( float );
	void 			UpdateFiring			( float );
	virtual void 	Update1stFiring			( float );


	bool			ProcessTransition		( const ThirdPersonAttackState* pState );
	static	void	MovementCompletedCB		( CEntity* );
	static	void	MovementInterruptedCB	( CEntity* );
	void			MovementCompleted		( void );
	void			MovementInterrupted		( void );


protected:
	// Description of the entity
	ntstd::String m_obDescription;

	// Needs comment !!MB_ENT
	CHashedString	m_obSceneElementDef;

	// Initial system state
	CHashedString	m_obInitialSystemState;

	// Walking controller
	CHashedString	m_obWalkingController;

	// These can be moved onto the Hero class
	CHashedString	m_obSpeedWalkingController; // Speed walking controller
	CHashedString	m_obPowerWalkingController; // Power walking controller
	CHashedString	m_obRangeWalkingController; // Range walking controller
	CHashedString	m_obSpeedStrafeController;  // Speed strafe controller
	CHashedString	m_obPowerStrafeController;  // Power strafe controller
	CHashedString	m_obRangeStrafeController;  // Range strafe controller

	// Components
	CInputComponent*	m_pobInputComponent;

	// Pad identifier
	uint32_t	m_uiPad;

	// 
	bool			m_bArcher;

	// Current state of the archer
	STATE			m_State;

	// Time in the above state
	float			m_StateTime;


	// delay before the fired shot is placed into aftertouch
	float			m_fAfterTouchStartDelay;

	// Time the pad has been held in a full direction.
	float			m_Mag1Time;

	// Last rotation angle / used to limit the angluar velocity
	float			m_CurrentAngle;
	float			m_LastAngle;
	float			m_CurrentAngleSmoothed;
	float			m_LastAngleSmoothed;

	// Lock on target
	CEntity*		m_pLockOnTarget;

	// Are the directional inputs of the pad being read?
	bool			m_bReadingPadInput;

	// The number of items fired since the last reload
	mutable u_int	m_FiredCount;

	// Raised when a request to shoot is made
	mutable bool	m_bFireRequest;

	// Raised when a request to reload is made
	mutable bool	m_bReloadRequest;

	// Raised when the firing request has been completed
	mutable bool	m_bFireRequestCompleted;

	// Raised perhaps when a fire is requested but the entity is currently reloading. 
	mutable bool	m_bFireRequestQueued;

	// Disallow allow parts of the system to give feedback
	mutable bool	m_bDisableFireFeedback;

	// The amount of charge in the shot fired. 
	mutable float	m_fShotCharge;

	// Record the time when the last shot was fired. 
	double	m_dTimeSinceLastShot;
	
	// Time firing delay
	float	m_fTimeFiringDelay;

	// Should be true if the game is in an after touch state
	bool			m_AfterTouchState;

	// This is a special bool used during aiming states of the hero. Here's the scenario...
	bool			m_bBreakWaitState;

	// 
	const ThirdPersonAttackState*		m_CurrentState;
	const ThirdPersonAttackTransition*	m_CurrentTransition;

	// Recharging health parameters
	bool			m_bInstantRefill;			// Should the health refill instantly (renderable will handle blending)?  
												// Or recharge over time?
	float			m_fDamageCooldownTime;		// Time after damage when health can refill
	float			m_fRefillRate;				// Rate to recharge at
	float			m_fCurrDamageTime;			// Current damage cooldown time

	// 
	PAD_DEADZONE_MODE	m_eDeadZoneMode;
};

LV_DECLARE_USERDATA(Player);

#endif //_ENTITY_PLAYER_H
