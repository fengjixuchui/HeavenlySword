//------------------------------------------------------------------------------------------
//!
//!	\file strafecontroller.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_STRAFECONTROLLER_H
#define	_STRAFECONTROLLER_H

// Necessary includes
#include "anim/animation.h"
#include "movementcontrollerinterface.h"

// Forward declarations
class CMovementState;

//------------------------------------------------------------------------------------------
//!
//!	StrafeAnimSet
//!	Container to hold all of the animations required to make a StrafeController work
//!
//------------------------------------------------------------------------------------------
class StrafeAnimSet
{
public:

	// This interface is exposed
	HAS_INTERFACE( StrafeAnimSet );

	// Construction
	StrafeAnimSet();

	// Animations - standing
	CHashedString m_obStanding;
	CHashedString m_obLookLeft;
	CHashedString m_obLookRight;

	// Animations - turning
	CHashedString m_obTurnLeft;
	CHashedString m_obTurnRight;

	// Animations
	CHashedString m_obMoveForwards;
	CHashedString m_obMoveBackwards;
	CHashedString m_obMoveLeft;
	CHashedString m_obMoveRight;

	// blinking
	CHashedString	m_obBlink;

	// Turning Attributes
	float m_fTurnPoint;
	float m_fTurnBlend;
};

//------------------------------------------------------------------------------------------
//!
//!	StrafeControllerDef
//!	All the details needed to define strafing movement
//!
//------------------------------------------------------------------------------------------
class StrafeControllerDef : public MovementControllerDef
{
public:

	// Construction 
	StrafeControllerDef( void );
	virtual ~StrafeControllerDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW StrafeControllerDef( *this ); }

	// The animation set to use
	const StrafeAnimSet* m_pobAnimSet;
	
	// Speed up to apply, added for AI characters but really needs fixing in the anims... JML
	float m_fSpeed;

	// How sticky do we want it to be - minimum input speed
	float m_fInputThreshold;

	// For people that float
	bool m_bApplyGravity;

	// blink every N seconds +/- a random factor
	float m_fBlinkInterval;
};

//------------------------------------------------------------------------------------------
//!
//!	StrafeController
//!	Strafing Movement - facing direction and movement direction are different
//!
//!	We have three internal states in this controller.  Each calculates the weight of their
//! animations as a percentage of one.  Then from above we blend in and out the states.
//!
//------------------------------------------------------------------------------------------
class StrafeController : public MovementController
{
public:

	// Construction
	StrafeController( CMovement* pobMovement, const StrafeControllerDef& obDefinition );

	// Destruction
	virtual ~StrafeController();

	// The main update details
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );

protected:

	// The possible strafe states
	enum STRAFE_STATES
	{
		SS_LOOKING,
		SS_TURNING,
		SS_MOVING,

		SS_COUNT
	};
	
	// The animations we use to turn
	enum TURNING_ANIMATIONS	
	{
		TA_TURN_LEFT,
		TA_TURN_RIGHT,

		TA_COUNT
	};

	// The animations we use to look
	enum LOOKING_ANIMATIONS	
	{
		LA_LOOK_LEFT,
		LA_LOOK_RIGHT,

		LA_COUNT
	};

	// The animations we use to move
	enum MOVING_ANIMATIONS	
	{
		MA_FORWARDS,
		MA_BACKWARDS,
		MA_LEFT,
		MA_RIGHT,

		MA_COUNT
	};

	// Create an animation
	CAnimationPtr CreateAnimation( const CHashedString& obAnimationName, int iVariableFlags );

	// For adding and removing animations to the animator
	void InitialiseAnimations( void );
	void ActivateAnimations( CAnimationPtr* aAnimations, int iNumberOfAnims );
	void DeactivateAnimations( CAnimationPtr* aAnimations, int iNumberOfAnims );

	// Calculate the animation weights 
	void CalculateLookWeights( float fRequiredTurn, float fTimeStep );
	void CalculateTurnWeights( float fTimeStep );
	void CalculateMoveWeights( const CMatrix& obRootMatrix, const CDirection& obMoveDirection, float fMoveSpeed, float fTimeStep );

	// State helping functions
	bool CheckTurnStart( float fRequiredTurn );
	bool CheckEndTurn( float fTimeStep );
	void UpdateStateAndStateWeights( float fTimeStep, float fInputSpeed, float fRequiredTurn );

	// A COPY of our defintion
	StrafeControllerDef m_obDefinition;

	// Our standing animation
	CAnimationPtr m_obStandingAnimation;

	// Our turning animations
	CAnimationPtr	m_aobTurningAnims[TA_COUNT];
	float			m_afTurningWeights[TA_COUNT];

	// Our looking animations
	CAnimationPtr	m_aobLookingAnims[LA_COUNT];
	float			m_afLookingWeights[LA_COUNT];

	// Our moving animations
	CAnimationPtr	m_aobMovingAnims[MA_COUNT];
	float			m_afMovingWeights[MA_COUNT];

	// Our current state weights
	STRAFE_STATES	m_eStrafeState;
	float			m_afStrafeWeights[SS_COUNT];	

	// Other details we need
	bool			m_bTurnLeft;

	// What are the capabilities of this controller - can we lock on or just strafe
	bool			m_bCanLockOn;
	bool			m_bCanMove;

	// We need to keep this value around if it is not updated
	CDirection m_obRequiredFaceDirection;

	//------------------------------------------------------------------------------------------
	// Blinking stuff
	bool			m_bCanBlink;
	float			m_fTimeSinceLastBlink;
	CAnimationPtr	m_pobBlinkAnim;
	void UpdateBlinking( float fTimeStep );

};


#endif // _STRAFECONTROLLER_H
