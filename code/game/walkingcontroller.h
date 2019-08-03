//------------------------------------------------------------------------------------------
//!
//!	\file walkingcontroller.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_WALKINGCONTROLLER_H
#define	_WALKINGCONTROLLER_H

// Necessary includes
#include "movementcontrollerinterface.h"

// Forward declarations
class CMovementState;

//------------------------------------------------------------------------------------------
//!
//!	WalkingAnimSet
//!	Container to hold all of the animations required to make a WalkingController work
//!
//------------------------------------------------------------------------------------------
class WalkingAnimSet
{
public:

	// This interface is exposed
	HAS_INTERFACE( WalkingAnimSet );

	// Standing
	CHashedString	m_obStanding;

	// Start Aware
	CHashedString	m_obToAware;

	// Aware
	CHashedString	m_obAwareMin;
	CHashedString	m_obAwareMax;

	// End Aware
	CHashedString	m_obFromAware;

	// Starting
	CHashedString	m_obStandToSlowWalk;
	CHashedString	m_obStandToSlowWalkLeft;
	CHashedString	m_obStandToSlowWalkRight;
	CHashedString	m_obStandToSlowWalkBackLeft;
	CHashedString	m_obStandToSlowWalkBackRight;

	CHashedString	m_obStandToWalk;
	CHashedString	m_obStandToWalkLeft;
	CHashedString	m_obStandToWalkRight;
	CHashedString	m_obStandToWalkBackLeft;
	CHashedString	m_obStandToWalkBackRight;

	CHashedString	m_obStandToAccel;
	CHashedString	m_obStandToAccelLeft;
	CHashedString	m_obStandToAccelRight;
	CHashedString	m_obStandToAccelBackLeft;
	CHashedString	m_obStandToAccelBackRight;

	// Stopping
	CHashedString	m_obSlowWalkToStandLeft;
	CHashedString	m_obSlowWalkToStandRight;

	CHashedString	m_obWalkToStandLeft;
	CHashedString	m_obWalkToStandRight;

	CHashedString	m_obRunToStandLeft;
	CHashedString	m_obRunToStandRight;

	// General Walking
	CHashedString	m_obSlowWalk;
	CHashedString	m_obSlowWalkLeft;
	CHashedString	m_obSlowWalkRight;

	CHashedString	m_obWalk;
	CHashedString	m_obWalkLeft;
	CHashedString	m_obWalkRight;

	CHashedString	m_obRun;
	CHashedString	m_obRunLeft;
	CHashedString	m_obRunRight;

	CHashedString	m_obAccelRun;
	CHashedString	m_obAccelRunLeft;
	CHashedString	m_obAccelRunRight;

	// Quick Turning
	CHashedString	m_obFullTurnRight;
	CHashedString	m_obFullTurnLeft;


	// blinking
	CHashedString	m_obBlink;
};

//------------------------------------------------------------------------------------------
//!
//!	WalkRunControllerDef
//!	All the details needed to define walk/run movement
//!
//------------------------------------------------------------------------------------------
class WalkRunControllerDef : public MovementControllerDef
{
public:

	// Construction 
	WalkRunControllerDef( void );
	virtual ~WalkRunControllerDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW WalkRunControllerDef( *this ); }

	// The set of animations to use
	WalkingAnimSet* m_pobAnimSet;

	// Describe our movement zones
	float m_fDeadZoneMaximum; 
	float m_fWalkZoneAnalogue; 
	float m_fWalkZonePlateau;  
	float m_fRunZoneAnalogue;

	// For blending in awareness stuff
	float m_fMaxAwareDistance;

	// How fast can we turn
	float m_fMaxTurnSpeed;

	// How much can we multiply the run anim speed by
	float m_fRunAnimMinSpeed;
	float m_fRunAnimMaxSpeed;
	float m_fRunAccelerationTime;

	// blink every N seconds +/- a random factor
	float m_fBlinkInterval;
};


//------------------------------------------------------------------------------------------
//!
//!	WalkRunController
//!	The main controller for the standard movement
//!
//!	This code passes around pointers to arrays of animations at the moment.  This may look
//!	unecessary but it means that we can overlay partial anim sets over the top of this 
//!	controller without additional control code - just a few points at which to make sure
//!	we can duplicate processes carried out on all the animations themselves.
//!
//------------------------------------------------------------------------------------------
class WalkRunController : public MovementController
{
public:

	static const float fAI_ROTATE_SPEED;

	// Construction
	WalkRunController( CMovement* pobMovement, const WalkRunControllerDef& obDefinition );

	// Destruction
	virtual ~WalkRunController();

	// The main update details
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );

protected:

	// These are the sub states available within this controller
	enum MOVE_STATE
	{
		MS_IDLE,
		MS_TO_AWARE,
		MS_AWARE,
		MS_FROM_AWARE,
		MS_ROTATE_ON_SPOT,
		MS_STARTING,
		MS_STOPPING,
		MS_MOVING,
		MS_TURNING,

		MS_COUNT
	};

	// The animations we have for standing
	enum IDLE_ANIMATIONS	
	{
		IA_STAND,

		IA_COUNT
	};

	// The Animations for starting aware
	enum TO_AWARE_ANIMATIONS	
	{
		TAA_START,

		TAA_COUNT
	};

	// The animations for aware
	enum AWARE_ANIMATIONS	
	{
		AA_MIN,
		AA_MAX,

		AA_COUNT
	};

	// The animations for moving from aware
	enum FROM_AWARE_ANIMATIONS	
	{
		FAA_FROM,

		FAA_COUNT
	};

	// The animations for rotating on the spot
	enum ROTATE_ON_SPOT_ANIMATIONS
	{
		RSA_IDLE,

		RSA_COUNT
	};

	// The animations we can use for starting
	enum STARTING_ANIMATIONS
	{
		STA_TOSLOWWALK,
		STA_TOSLOWWALKLEFT,
		STA_TOSLOWWALKRIGHT,
		STA_TOSLOWWALKBACKLEFT,
		STA_TOSLOWWALKBACKRIGHT,
		STA_TOWALK,
		STA_TOWALKLEFT,
		STA_TOWALKRIGHT,
		STA_TOWALKBACKLEFT,
		STA_TOWALKBACKRIGHT,
		STA_TOACCEL,
		STA_TOACCELLEFT,
		STA_TOACCELRIGHT,
		STA_TOACCELBACKLEFT,
		STA_TOACCELBACKRIGHT,

		STA_COUNT
	};

	// The animations we can use for stopping
	enum STOPPING_ANIMATIONS
	{
		SPA_FROMSLOWWALKLEFT,
		SPA_FROMSLOWWALKRIGHT,
		SPA_FROMWALKLEFT,
		SPA_FROMWALKRIGHT,
		SPA_FROMRUNLEFT,
		SPA_FROMRUNRIGHT,

		SPA_COUNT
	};

	// The animations we use for the general movement
	enum MOVING_ANIMATIONS	
	{
		MA_SLOWWALK,
		MA_SLOWWALKLEFT,
		MA_SLOWWALKRIGHT,
		MA_WALK,
		MA_WALKLEFT,
		MA_WALKRIGHT,
		MA_RUN,
		MA_RUNLEFT,
		MA_RUNRIGHT,
		MA_ACCELRUN,
		MA_ACCELRUNLEFT,
		MA_ACCELRUNRIGHT,

		MA_COUNT
	};

	// The animations to use for a quick turn
	enum TURNING_ANIMATIONS 
	{ 
		TUA_LEFT, 
		TUA_RIGHT,
		
		TUA_COUNT 
	};

	// A COPY of our defintion
	WalkRunControllerDef m_obDefinition;

	//------------------------------------------------------------------------------------------
	// General detail tracking for the whole controller

	// Choose the first internal state when we are pushed on to the controller stack
	void InitialiseState( const CMovementStateRef& obCurrentMovementState );

	// Process the input data and build more useful parameters
	void ProcessInputParameters( const CMovementInput& obMovementInput );

	// Update the relative state blend weights
	void UpdateStateWeights( float fTimeDelta );

	// Make a capped change to a value
	float SetCappedValue( float fCurrentValue, float fNewValue, float fAllowedChange ); 

	// Update the animation blend weights
	void UpdateAnimationBlendWeights(	float			fTimeDelta,
										CAnimationPtr*	aobIdleAnims,
										CAnimationPtr*	aobAwareAnims,
										CAnimationPtr*	aobToAwareAnims,
										CAnimationPtr*	aobFromAwareAnims,
										CAnimationPtr*  aobRotateOnSpotAnims,
										CAnimationPtr*	aobStartingAnims,
										CAnimationPtr*	aobStoppingAnims,
										CAnimationPtr*	aobMovingAnims,
										CAnimationPtr*	aobTurningAnims );

	// Create an animation
	CAnimationPtr CreateAnimation( const CHashedString& obAnimationName, int iVariableFlags );

	// For adding and removing animations to the animator
	void ActivateAnimations( CAnimationPtr* aAnimations, int iNumberOfAnims );
	void DeactivateAnimations( CAnimationPtr* aAnimations, int iNumberOfAnims );

	// To find the optimum place to start stopping or turning
	bool JustPassedPhaseBoundary( void ) const;

	// Can the character turn in their current state?
	bool CanTurn( void ) const;

	// Should we be turning using values generated from animations
	bool UseAnimatedTurn( void ) const;

	// Tweak the speed of the run animation
	void UpdateRunSpeed(float fTimeDelta);

	// Has the controller been initialised
	bool m_bInitialised;

	// How much should we turn this frame - used by some states
	float m_fAnimTurnAmount;
	float m_fLastAnimTurnAmount;

	// What state are we currently in?
	MOVE_STATE m_eMovementState;

	// To monitor our relative blend weights
	float m_fStateWeights[MS_COUNT];

	// Some flags to allow the controller to use reduced animation sets
	bool m_bCanBeAware;
	bool m_bCanRotateOnSpot;
	bool m_bCanSlowWalk;
	bool m_bCanWalk;
	bool m_bCanRun;
	bool m_bCanTurn;
	bool m_bCanAccelerate;
	bool m_bCanFastTurn;
	
	// Processed information for controller behaviour
	CPoint		m_obTargetPoint;
	CDirection	m_obLastRequestedDirection;
	bool		m_bTargetPointSet;
	float		m_fRequestedDirectionChange;
	float		m_fCurrentSpeed;
	float		m_fTurnAmount;
	float		m_fRequestedSpeed;
	float		m_fPreviousPhase;
	float		m_fBlendInTime;

	// Constant values that are used to decide on blending
	float m_fSlowWalkSpeed;
	float m_fWalkSpeed;
	float m_fRunSpeed;

	// Time spent in a particular state
	float m_fStateTime;

	//------------------------------------------------------------------------------------------
	// Blinking stuff
	bool			m_bCanBlink;
	float			m_fTimeSinceLastBlink;
	CAnimationPtr	m_pobBlinkAnim;

	void UpdateBlinking( float fTimeStep );

	//------------------------------------------------------------------------------------------
	// Details for the idle state

	// Start and Update the idle state
	virtual void StartIdle( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobIdleAnims );
	void UpdateIdle( float fTimeDelta, const CMovementStateRef& obCurrentMovementState );
	virtual void ClearIdle( CAnimationPtr* aobIdleAnims );

	// Our idle animations
	CAnimationPtr m_aobIdleAnims[IA_COUNT];

	//------------------------------------------------------------------------------------------
	// Details for the awareness state

	// Start and Update the aware state
	virtual void StartAware( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobAwareAnims );
	void UpdateAware( float fTimeDelta, const CMovementStateRef& obCurrentMovementState );
	virtual void ClearAware( CAnimationPtr* aobAwareAnims );

	// Our idle animations
	CAnimationPtr m_aobAwareAnims[AA_COUNT];

	// How 'aware' are we
	float m_fAwarenessWeight;

	//------------------------------------------------------------------------------------------
	// Details for the to-awareness state

	// Start and Update the to-aware state
	virtual void StartToAware( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobToAwareAnims );
	void UpdateToAware( float fTimeDelta, const CMovementStateRef& obCurrentMovementState );
	virtual void ClearToAware( CAnimationPtr* aobToAwareAnims );

	// Our idle animations
	CAnimationPtr m_aobToAwareAnims[TAA_COUNT];

	//------------------------------------------------------------------------------------------
	// Details for the from-awareness state

	// Start and Update the from-aware state
	virtual void StartFromAware( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobFRomAwareAnims );
	void UpdateFromAware( float fTimeDelta, const CMovementStateRef& obCurrentMovementState );
	virtual void ClearFromAware( CAnimationPtr* aobFromAwareAnims );

	// Our idle animations
	CAnimationPtr m_aobFromAwareAnims[FAA_COUNT];

	//------------------------------------------------------------------------------------------
	// Details for the rotate on spot state

	virtual void StartRotateOnSpot( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobRotateOnSpotAnims );
	void UpdateRotateOnSpot( float fTimeDelta, const CMovementStateRef& obCurrentMovementState );
	virtual void ClearRotateOnSpot( CAnimationPtr* aobRotateOnSpotAnims );

	// Our rotate on spot animations
	CAnimationPtr m_aobRotateOnSpotAnims[RSA_COUNT];

	// HACK: AI's only.  Has passed in a rotate on speed.
	bool m_bAIRequestedRotation;
	
	//------------------------------------------------------------------------------------------
	// Details for the starting state

	// Start and update the starting state
	virtual void StartStarting( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobStartingAnims );
	void UpdateStarting( float fTimeDelta, const CMovementStateRef& obCurrentMovementState );
	virtual void ClearStarting( CAnimationPtr* aobStartingAnims );

	// Calculate the animation blend weights for when we are starting
	void CalculateStartBlendWeights( float fTimeDelta );

	// Calculate the animation turn values whilst starting
	void CalculateStartTurnAmount( float fTimeDelta );

	// Our starting animations
	CAnimationPtr m_aobStartingAnims[STA_COUNT];

	// We don't blend start animations - we choose two
	STARTING_ANIMATIONS m_eChosenStartSlowWalk;
	STARTING_ANIMATIONS m_eChosenStartWalk;
	STARTING_ANIMATIONS m_eChosenStartRun;

	// We need to know whether we have moved to a run
	float m_fRunStartWeight;
	float m_fWalkStartWeight;
	float m_fSlowWalkStartWeight;

	// What direction were we facing in at the start of this state
	CDirection m_obStartFacingDirection;

	//------------------------------------------------------------------------------------------
	// Details for the stopping state

	// Start and update the stopping state
	virtual void StartStopping( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobStoppingAnims );
	void UpdateStopping( float fTimeDelta, const CMovementStateRef& obCurrentMovementState );
	virtual void ClearStopping( CAnimationPtr* aobStoppingAnims );

	// Our stopping animations
	CAnimationPtr m_aobStoppingAnims[SPA_COUNT];

	// We don't blend stop animations - we choose one
	STOPPING_ANIMATIONS m_eChosenStop;

	//------------------------------------------------------------------------------------------
	// Details for the general moving state

	// Start and update the moving state
	virtual void StartMoving( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobMovingAnims );
	void UpdateMoving( float fTimeDelta, const CMovementStateRef& obCurrentMovementState );
	virtual void ClearMoving( CAnimationPtr* aobMovingAnims );

	// Calculate the acceleration appearance
	float CalculateAccelerationAppearance( float fTimeStep );

	// Values that we need to track and smooth
	float	m_fTurningAppearance;
	float	m_fAccelerationAppearance;

	// Do we need to do a quick turn
	bool m_bQuickTurn;

	// How long has our turn been clamped for
	float m_fTurnClampDuration;

	// Our moving animations
	CAnimationPtr m_aobMovingAnims[MA_COUNT];

	//------------------------------------------------------------------------------------------
	// Details for the turning state

	// Start and update the moving state
	virtual void StartTurning( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobTurningAnims );
	void UpdateTurning( float fTimeDelta, const CMovementStateRef& obCurrentMovementState );
	virtual void ClearTurning( CAnimationPtr* aobTurningAnims );

	// Our turning animations
	CAnimationPtr m_aobTurningAnims[TUA_COUNT];

	// We don't blend stop animations - we choose one
	TURNING_ANIMATIONS m_eChosenTurn;
};


#endif //_WALKINGCONTROLLER_H
