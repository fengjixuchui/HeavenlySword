//------------------------------------------------------------------------------------------
//!
//!	\file partialwalkingcontroller.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_PARTIALWALKINGCONTROLLER_H
#define	_PARTIALWALKINGCONTROLLER_H

// Necessary includes
#include "movementcontrollerinterface.h"
#include "walkingcontroller.h"

// Forward declarations
class CMovement;

//------------------------------------------------------------------------------------------
//!
//!	SimplePartialAnimSet
//!	Holds a few animations to be played over the top of a walkruncontroller
//!
//------------------------------------------------------------------------------------------
class SimplePartialAnimSet
{
public:

	// This interface is exposed
	HAS_INTERFACE( SimplePartialAnimSet );

	// Standing
	CHashedString	m_obSingleAnimation;
};


//------------------------------------------------------------------------------------------
//!
//!	SimplePartialWalkRunDef
//!	A controller which augments the walk run controller with a few animations - intended for
//!	use with interactive objects.
//!
//------------------------------------------------------------------------------------------
class SimplePartialWalkRunDef : public WalkRunControllerDef 
{
public:

	// This interface is exposed
	HAS_INTERFACE( SimplePartialWalkRunDef );

	// Construction 
	SimplePartialWalkRunDef( void );
	virtual ~SimplePartialWalkRunDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW SimplePartialWalkRunDef( *this ); }

	// The set of animations to use
	SimplePartialAnimSet* m_pobSimplePartialAnimSet;
};


//------------------------------------------------------------------------------------------
//!
//!	SimplePartialWalkRun
//!	The main controller for the standard movement
//!
//------------------------------------------------------------------------------------------
class SimplePartialWalkRun : public WalkRunController
{
public:

	// Construction
	SimplePartialWalkRun( CMovement* pobMovement, const SimplePartialWalkRunDef& obDefinition );

	// Destruction
	virtual ~SimplePartialWalkRun( void );

	// The main update details
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );

	// Disable one-off partial animations for this derived controller 
	virtual bool IsPlayingPartialAnim() const { return false; };
	virtual bool CanPlayPartialAnim() const	{ return false; };
	virtual void PlayPartialAnim( const CHashedString& m_pobPartialAnim, float fFadeIn, float fFadeOut ) { UNUSED( m_pobPartialAnim ); UNUSED( fFadeIn ); UNUSED( fFadeOut ); };

protected:

	// A copy of our definition
	SimplePartialWalkRunDef m_obSimplePartialDefinition;

	// Our animations that map over the base controller
	CAnimationPtr m_obSingleAnimation;
};


//------------------------------------------------------------------------------------------
//!
//!	Partial2AnimSet
//!	Holds a set of partial animations to play over the top of a walk run controller
//!
//------------------------------------------------------------------------------------------
class Partial2AnimSet
{
public:

	// This interface is exposed
	HAS_INTERFACE( Partial2AnimSet );

	// Standing
	CHashedString	m_obStanding;

	// Starting
	CHashedString	m_obStandToSlowWalk;
	CHashedString	m_obStandToSlowWalkLeft;
	CHashedString	m_obStandToSlowWalkRight;

	CHashedString	m_obStandToWalk;
	CHashedString	m_obStandToWalkLeft;
	CHashedString	m_obStandToWalkRight;

	CHashedString	m_obStandToAccel;
	CHashedString	m_obStandToAccelLeft;
	CHashedString	m_obStandToAccelRight;

	// Stopping
	CHashedString	m_obSlowWalkToStandLeft;
	CHashedString	m_obSlowWalkToStandRight;

	CHashedString	m_obWalkToStandLeft;
	CHashedString	m_obWalkToStandRight;

	CHashedString	m_obRunToStandLeft;
	CHashedString	m_obRunToStandRight;

	// General Walking
	CHashedString	m_obSlowWalk;
	CHashedString	m_obWalk;
	CHashedString	m_obRun;
	CHashedString	m_obAccelRun;

	// Quick Turning
	CHashedString	m_obFullTurnRight;
	CHashedString	m_obFullTurnLeft;
};

//------------------------------------------------------------------------------------------
//!
//!	Partial2WalkRunDef
//!	Walk/run animations with an extra layer of animations for carrying objects
//!
//------------------------------------------------------------------------------------------
class Partial2WalkRunDef : public WalkRunControllerDef 
{
public:

	// This interface is exposed
	HAS_INTERFACE( Partial2WalkRunDef );

	// Construction 
	Partial2WalkRunDef( void );
	virtual ~Partial2WalkRunDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW Partial2WalkRunDef( *this ); }

	// The set of animations to use
	Partial2AnimSet* m_pobPartial2AnimSet;
};


//------------------------------------------------------------------------------------------
//!
//!	Partial2WalkRun
//!	
//!
//------------------------------------------------------------------------------------------
class Partial2WalkRun : public WalkRunController
{
public:

	// Construction
	Partial2WalkRun( CMovement* pobMovement, const Partial2WalkRunDef& obDefinition );

	// Destruction
	virtual ~Partial2WalkRun( void );

	// The main update details
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );

	// Disable one-off partial animations for this derived controller 
	virtual bool IsPlayingPartialAnim() const { return false; };
	virtual bool CanPlayPartialAnim() const	{ return false; };
	virtual void PlayPartialAnim( const CHashedString& m_pobPartialAnim, float fFadeIn, float fFadeOut ) { UNUSED( m_pobPartialAnim ); UNUSED( fFadeIn ); UNUSED( fFadeOut ); };

protected:

	// A copy of our definition
	Partial2WalkRunDef m_obPartial2Definition;

	// Have we got all of the aniamtions we need?
	bool m_bPartial2MinimumSet;

	// Our animations that map over the base controller
	CAnimationPtr m_aobPartial2IdleAnims[IA_COUNT];
	CAnimationPtr m_aobPartial2AwareAnims[AA_COUNT];
	CAnimationPtr m_aobPartial2ToAwareAnims[TAA_COUNT];
	CAnimationPtr m_aobPartial2FromAwareAnims[FAA_COUNT];
	CAnimationPtr m_aobPartial2RotateOnSpotAnims[RSA_COUNT];
	CAnimationPtr m_aobPartial2StartingAnims[STA_COUNT];
	CAnimationPtr m_aobPartial2StoppingAnims[SPA_COUNT];
	CAnimationPtr m_aobPartial2MovingAnims[MA_COUNT];
	CAnimationPtr m_aobPartial2TurningAnims[TUA_COUNT];

	//------------------------------------------------------------------------------------------
	// Add to the idle state

	// Start and Update the idle state
	virtual void StartIdle( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobIdleAnims );
	virtual void ClearIdle( CAnimationPtr* aobIdleAnims );

	//------------------------------------------------------------------------------------------
	// Add to the awareness state

	// Start and Update the aware state
	virtual void StartAware( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobAwareAnims );
	virtual void ClearAware( CAnimationPtr* aobAwareAnims );

	//------------------------------------------------------------------------------------------
	// Add to the to-awareness state

	// Start and Update the to-aware state
	virtual void StartToAware( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobToAwareAnims );
	virtual void ClearToAware( CAnimationPtr* aobToAwareAnims );

	//------------------------------------------------------------------------------------------
	// Add to the from-awareness state

	// Start and Update the from-aware state
	virtual void StartFromAware( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobFromAwareAnims );
	virtual void ClearFromAware( CAnimationPtr* aobFromAwareAnims );

	//------------------------------------------------------------------------------------------
	// Add to the rotate on spot state

	// Start and clear the rotate on spot state
	virtual void StartRotateOnSpot( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobRotateOnSpotAnims );
	virtual void ClearRotateOnSpot( CAnimationPtr* aobRotateOnSpotAnims );

	//------------------------------------------------------------------------------------------
	// Add to the starting state

	// Start and update the starting state
	virtual void StartStarting( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobStartingAnims );
	virtual void ClearStarting( CAnimationPtr* aobStartingAnims );

	//------------------------------------------------------------------------------------------
	// Add to the stopping state

	// Start and update the stopping state
	virtual void StartStopping( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobStoppingAnims );
	virtual void ClearStopping( CAnimationPtr* aobStoppingAnims );

	//------------------------------------------------------------------------------------------
	// Add to the general moving state

	// Start and update the moving state
	virtual void StartMoving( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobMovingAnims );
	virtual void ClearMoving( CAnimationPtr* aobMovingAnims );

	//------------------------------------------------------------------------------------------
	// Add to the turning state

	// Start and update the moving state
	virtual void StartTurning( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobTuringAnims );
	virtual void ClearTurning( CAnimationPtr* aobTurningAnims );
};


//------------------------------------------------------------------------------------------
//!
//!	Partial3AnimSet
//!	Holds a secondary set of partial animations to play over the top of a walk run controller
//!
//------------------------------------------------------------------------------------------
class Partial3AnimSet
{
public:

	// This interface is exposed
	HAS_INTERFACE( Partial3AnimSet );

	// Standing
	CHashedString	m_obStanding;

	// Starting
	CHashedString	m_obStandToWalk;

	CHashedString	m_obStandToAccel;
	CHashedString	m_obStandToAccelLeft;
	CHashedString	m_obStandToAccelRight;

	// Stopping
	CHashedString	m_obWalkToStandLeft;
	CHashedString	m_obWalkToStandRight;

	CHashedString	m_obRunToStandLeft;
	CHashedString	m_obRunToStandRight;

	// General Walking
	CHashedString	m_obWalk;
	CHashedString	m_obRun;
	CHashedString	m_obAccelRun;

	// Quick Turning
	CHashedString	m_obFullTurnRight;
	CHashedString	m_obFullTurnLeft;
};


//------------------------------------------------------------------------------------------
//!
//!	Partial3WalkRunDef
//!	Walk/run animations with two extra layers of animations for carrying objects
//!
//------------------------------------------------------------------------------------------
class Partial3WalkRunDef : public Partial2WalkRunDef 
{
public:

	// This interface is exposed
	HAS_INTERFACE( Partial3WalkRunDef );

	// Construction 
	Partial3WalkRunDef( void );
	virtual ~Partial3WalkRunDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW Partial3WalkRunDef( *this ); }

	// The set of animations to use
	Partial3AnimSet* m_pobPartial3AnimSet;
};


//------------------------------------------------------------------------------------------
//!
//!	Partial3WalkRun
//!	The main controller for the standard movement
//!
//------------------------------------------------------------------------------------------
class Partial3WalkRun : public Partial2WalkRun
{
public:

	// Construction
	Partial3WalkRun( CMovement* pobMovement, const Partial3WalkRunDef& obDefinition );

	// Destruction
	virtual ~Partial3WalkRun( void );

	// The main update details
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );

protected:

	// A copy of our definition
	Partial3WalkRunDef m_obPartial3Definition;

	// Have we got all of the aniamtions we need?
	bool m_bPartial3MinimumSet;

	// Our animations that map over the base controller
	CAnimationPtr m_aobPartial3IdleAnims[IA_COUNT];
	CAnimationPtr m_aobPartial3AwareAnims[AA_COUNT];
	CAnimationPtr m_aobPartial3ToAwareAnims[TAA_COUNT];
	CAnimationPtr m_aobPartial3FromAwareAnims[FAA_COUNT];
	CAnimationPtr m_aobPartial3RotateOnSpotAnims[RSA_COUNT];
	CAnimationPtr m_aobPartial3StartingAnims[STA_COUNT];
	CAnimationPtr m_aobPartial3StoppingAnims[SPA_COUNT];
	CAnimationPtr m_aobPartial3MovingAnims[MA_COUNT];
	CAnimationPtr m_aobPartial3TurningAnims[TUA_COUNT];

	//------------------------------------------------------------------------------------------
	// Add to the idle state

	// Start and Update the idle state
	virtual void StartIdle( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobIdleAnims );
	virtual void ClearIdle( CAnimationPtr* aobIdleAnims );

	//------------------------------------------------------------------------------------------
	// Add to the awareness state

	// Start and Update the aware state
	virtual void StartAware( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobAwareAnims );
	virtual void ClearAware( CAnimationPtr* aobAwareAnims );

	//------------------------------------------------------------------------------------------
	// Add to the to-awareness state

	// Start and Update the to-aware state
	virtual void StartToAware( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobToAwareAnims );
	virtual void ClearToAware( CAnimationPtr* aobToAwareAnims );

	//------------------------------------------------------------------------------------------
	// Add to the from-awareness state

	// Start and Update the from-aware state
	virtual void StartFromAware( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobFRomAwareAnims );
	virtual void ClearFromAware( CAnimationPtr* aobFromAwareAnims );

	//------------------------------------------------------------------------------------------
	// Add to the rotate on spot state

	// Start and clear the rotate on spot state
	virtual void StartRotateOnSpot( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobRotateOnSpotAnims );
	virtual void ClearRotateOnSpot( CAnimationPtr* aobRotateOnSpotAnims );

	//------------------------------------------------------------------------------------------
	// Add to the starting state

	// Start and update the starting state
	virtual void StartStarting( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobStartingAnims );
	virtual void ClearStarting( CAnimationPtr* aobStartingAnims );

	//------------------------------------------------------------------------------------------
	// Add to the stopping state

	// Start and update the stopping state
	virtual void StartStopping( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobStoppingAnims );
	virtual void ClearStopping( CAnimationPtr* aobStoppingAnims );

	//------------------------------------------------------------------------------------------
	// Add to the general moving state

	// Start and update the moving state
	virtual void StartMoving( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobMovingAnims );
	virtual void ClearMoving( CAnimationPtr* aobMovingAnims );

	//------------------------------------------------------------------------------------------
	// Add to the turning state

	// Start and update the moving state
	virtual void StartTurning( const CMovementStateRef& obCurrentMovementState, CAnimationPtr* aobTurningAnims );
	virtual void ClearTurning( CAnimationPtr* aobTurningAnims );
};


#endif // _PARTIALWALKINGCONTROLLER_H
