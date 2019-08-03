//------------------------------------------------------------------------------------------
//!
//!	\file interactiontransitions.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_INTERACTIONTRANSITIONS_H
#define	_INTERACTIONTRANSITIONS_H

// Necessary includes
#include "movementcontrollerinterface.h"

class CEntity;

//------------------------------------------------------------------------------------------
//!
//!	EmptyTransitionDef
//!	Creates an empty movement controller - not much to define
//!
//------------------------------------------------------------------------------------------
class EmptyTransitionDef : public MovementControllerDef
{
public:

	// Construction
	EmptyTransitionDef( void ) {}
	virtual ~EmptyTransitionDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW EmptyTransitionDef( *this ); }
};


//------------------------------------------------------------------------------------------
//!
//!	EmptyTransition
//!
//------------------------------------------------------------------------------------------
class EmptyTransition : public MovementController
{
public:

	// Construction destruction
	EmptyTransition( CMovement* pobMovement, const EmptyTransitionDef& ) : MovementController( pobMovement ) { }
	virtual ~EmptyTransition( void ) {}

	// The major functionality of any movement constroller
	virtual bool Update(	float						/*fTimeStep*/, 
							const CMovementInput&		/*obMovementInput*/,
							const CMovementStateRef&	/*obCurrentMovementState*/,
							CMovementState&				/*obPredictedMovementState*/ )
	{
		// This return mechanism needs to be used to move to the next controller in future
		return false;
	}
};


//------------------------------------------------------------------------------------------
//!
//!	MoveToTransitionDef
//!	A simple structure that needs to be filled out to create a simple transition.  Can be
//! predefined in xml if one wishes.
//!
//------------------------------------------------------------------------------------------
class MoveToTransitionDef : public MovementControllerDef
{
public:

	// Construction
	MoveToTransitionDef( void );
	virtual ~MoveToTransitionDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW MoveToTransitionDef( *this ); }

	// Members
	bool		m_bApplyGravity;
	CHashedString	m_obAnimationName;
	float		m_fDistance;
	float		m_fMaximumRotationSpeed;
	float		m_fAnimSpeed;

	CPoint		m_obOffset;

	CEntity*	m_pobTargetEntity;
};

//------------------------------------------------------------------------------------------
//!
//!	MoveToTransition
//!	Used to play a non looping animation on more compolex entities with a movement component
//!
//------------------------------------------------------------------------------------------
class MoveToTransition : public MovementController
{
public:

	// Construction destruction
	MoveToTransition( CMovement* pobMovement, const MoveToTransitionDef& obDefinition );
	virtual ~MoveToTransition( void );

	// The major functionality of any movement constroller
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );

protected:

	// A COPY of our defintion
	MoveToTransitionDef m_obDefinition;

	// A pointer to our single animation
	CAnimationPtr m_obSingleAnimation;


	float m_fAnimationDistance; // Find the total distance covered by the animation
	
	CPoint m_obAnimTranslation;
};



//------------------------------------------------------------------------------------------
//!
//!	FacingMoveToTransitionDef
//!	A simple structure that needs to be filled out to create a simple transition.  Can be
//! predefined in xml if one wishes.
//! Moves to a specified point, and moves to the correct facing angle.
//!
//------------------------------------------------------------------------------------------
class FacingMoveToTransitionDef : public MovementControllerDef
{
public:

	// Construction
	FacingMoveToTransitionDef( void );
	virtual ~FacingMoveToTransitionDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW FacingMoveToTransitionDef( *this ); }

	// Members
	bool			m_bApplyGravity;
	CHashedString	m_obAnimationName;
	float			m_fDistance;
	float			m_fMaximumRotationSpeed;
	float			m_fAnimSpeed;

	CPoint			m_obOffsetLS;
	CDirection		m_obFacingDirLS;

	CEntity*		m_pobTargetEntity;
};

//------------------------------------------------------------------------------------------
//!
//!	FacingMoveToTransition
//!	Used to play a non looping animation on more compolex entities with a movement component
//!
//------------------------------------------------------------------------------------------
class FacingMoveToTransition : public MovementController
{
public:

	// Construction destruction
	FacingMoveToTransition( CMovement* pobMovement, const FacingMoveToTransitionDef& obDefinition );
	virtual ~FacingMoveToTransition( void );

	// The major functionality of any movement controller
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );

protected:

	// A COPY of our defintion
	FacingMoveToTransitionDef m_obDefinition;

	// A pointer to our single animation
	CAnimationPtr m_obSingleAnimation;

	float m_fAnimationDistance; // Find the total distance covered by the animation
	
	CPoint m_obAnimTranslation;
};


//------------------------------------------------------------------------------------------
//!
//!	FacingTransitionDef
//!	A simple structure that needs to be filled out to create a simple transition.  Can be
//! predefined in xml if one wishes.
//!
//------------------------------------------------------------------------------------------
class FacingTransitionDef : public MovementControllerDef
{
public:

	// Construction
	FacingTransitionDef( void );
	virtual ~FacingTransitionDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW FacingTransitionDef( *this ); }

	// Members
	bool		m_bApplyGravity;
	float		m_fAngularSpeed; // Degrees rotation per second
	float		m_fEarlyOut; // Allows a transition to opt out slightly earlier (for blending)
    CHashedString	m_obAnimationName;
	float		m_fAnimSpeed;
	float		m_fAnimOffset;

	float		m_fStartTurnControl; // Time offset in the animation when they player can control the character
	float		m_fEndTurnControl; // The last time offset in the animation when the player can control the character

	float		m_fAlignToThrowTarget;


	// Return the length of the animation - this call only works once the animation has been added
	// to the movement contorller. In the world of random animations, it's not possible to
	// find the length of the current animation until the animation has been picked during the
	// contruction of the movement controller
	float		GetDuration() const { return m_fAnimLength; }

private:
	friend class FacingTransition;
	// This a read only
	mutable float m_fAnimLength;
};

//------------------------------------------------------------------------------------------
//!
//!	FacingTransition
//!	Used to play a non looping animation on more compolex entities with a movement component
//!
//------------------------------------------------------------------------------------------
class FacingTransition : public MovementController
{
public:

	// Construction destruction
	FacingTransition( CMovement* pobMovement, const FacingTransitionDef& obDefinition );
	virtual ~FacingTransition( void );

	// The major functionality of any movement constroller
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );

protected:

	// A COPY of our defintion
	FacingTransitionDef m_obDefinition;

	// A pointer to our single animation
	CAnimationPtr m_obSingleAnimation;

	float m_fPreviousAnimTime;
};



//------------------------------------------------------------------------------------------
//!
//!	SnapToTransitionDef
//!	A simple structure that needs to be filled out to create a simple transition.  Can be
//! predefined in xml if one wishes.
//!
//------------------------------------------------------------------------------------------
class SnapToTransitionDef : public MovementControllerDef
{
public:

	// Construction
	SnapToTransitionDef( void );
	virtual ~SnapToTransitionDef( void );

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW SnapToTransitionDef( *this ); }

	CEntity* m_pobTargetEntity;
	CHashedString m_sTransformName;

	// ----- Serialised members -----

	CHashedString	m_obAnimationName;

	CPoint m_obTranslationOffset;

	CQuat m_obRotationOffset;

	bool m_bApplyGravity;
};

//------------------------------------------------------------------------------------------
//!
//!	SnapToTransition
//!	Used to play a non looping animation on more compolex entities with a movement component
//!
//------------------------------------------------------------------------------------------
class SnapToTransition : public MovementController
{
public:

	// Construction destruction
	SnapToTransition( CMovement* pobMovement, const SnapToTransitionDef& obDefinition );
	virtual ~SnapToTransition( void );

	// The major functionality of any movement constroller
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );

protected:

	SnapToTransitionDef m_obDefinition;

	CAnimationPtr m_obSingleAnimation;
};








//------------------------------------------------------------------------------------------
//!
//!	LinkedMovementTransitionDef
//!	Plays an an animation on an entity whilst that entity is translated/rotated (only in Y) relative
//! to another entity.
//!
//------------------------------------------------------------------------------------------
class LinkedMovementTransitionDef : public MovementControllerDef
{
public:
	// Construction
	LinkedMovementTransitionDef( void ) : m_pobTargetEntity(0) {}

	virtual ~LinkedMovementTransitionDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW LinkedMovementTransitionDef( *this ); }

	CEntity*	m_pobTargetEntity;
	CHashedString	m_obTargetTransform;
	CHashedString	m_obAnimationName;
	CPoint		m_obTranslationOffset;
	CQuat		m_obRotationOffset;
};


//------------------------------------------------------------------------------------------
//!
//!	LinkedMovementTransition
//!	Plays an an animation on an entity whilst that entity is translated/rotated (only in Y) relative
//! to another entity.
//!
//------------------------------------------------------------------------------------------
class LinkedMovementTransition : public MovementController
{
public:

	// Construction destruction
	LinkedMovementTransition( CMovement* pobMovement, const LinkedMovementTransitionDef& obDefinition );
	virtual ~LinkedMovementTransition( void );

	// The major functionality of any movement constroller
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );

protected:

	LinkedMovementTransitionDef m_obDefinition;

	CAnimationPtr m_obSingleAnimation;
};

//------------------------------------------------------------------------------------------
//!
//!	FullyLinkedMovementTransitionDef
//!	Plays an an animation on an entity whilst that entity is translated/rotated (in all axis) relative
//! to another entity.  Needed by kite ninjas.
//!
//------------------------------------------------------------------------------------------
class FullyLinkedMovementTransitionDef : public MovementControllerDef
{
public:
	// Construction
	FullyLinkedMovementTransitionDef( void ) : m_pobTargetEntity(0) {}

	virtual ~FullyLinkedMovementTransitionDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW FullyLinkedMovementTransitionDef( *this ); }

	CEntity*	m_pobTargetEntity;
	CHashedString	m_obTargetTransform;
	CHashedString	m_obAnimationName;
	CPoint		m_obTranslationOffset;
	CQuat		m_obRotationOffset;
};

//------------------------------------------------------------------------------------------
//!
//!	FullyLinkedMovementTransition
//!	Plays an an animation on an entity whilst that entity is translated/rotated relative
//! to another entity.
//!
//------------------------------------------------------------------------------------------
class FullyLinkedMovementTransition : public MovementController
{
public:

	// Construction destruction
	FullyLinkedMovementTransition( CMovement* pobMovement, const FullyLinkedMovementTransitionDef& obDefinition );
	virtual ~FullyLinkedMovementTransition( void );

	// The major functionality of any movement constroller
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );

protected:

	FullyLinkedMovementTransitionDef m_obDefinition;

	CAnimationPtr m_obSingleAnimation;
};


//------------------------------------------------------------------------------------------
//!
//!	CorrectiveMovementTransitionDef
//!	Plays an an animation on an entity whilst that entity is translated/rotated (in all axis) relative
//! to another entity.  Needed by kite ninjas.
//!
//------------------------------------------------------------------------------------------
class CorrectiveMovementTransitionDef : public MovementControllerDef
{
public:
	// Construction
	CorrectiveMovementTransitionDef( void ) {}

	virtual ~CorrectiveMovementTransitionDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW CorrectiveMovementTransitionDef( *this ); }

	CPoint		m_obTargetPostion;
	CQuat		m_obTargetRotation;
};

//------------------------------------------------------------------------------------------
//!
//!	CorrectiveMovementTransition
//!	Plays an an animation on an entity whilst that entity is translated/rotated relative
//! to another entity.
//!
//------------------------------------------------------------------------------------------
class CorrectiveMovementTransition : public MovementController
{
public:

	// Construction destruction
	CorrectiveMovementTransition( CMovement* pobMovement, const CorrectiveMovementTransitionDef& obDefinition );
	virtual ~CorrectiveMovementTransition( void );

	// The major functionality of any movement constroller
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );

protected:

	CorrectiveMovementTransitionDef m_obDefinition;
};



//------------------------------------------------------------------------------------------
//!
//!	InteractiveAnimControllerDef
//!	NOTE: Used by gatehouse wheel - not needed anymore!
//!
//------------------------------------------------------------------------------------------
class InteractiveAnimControllerDef : public MovementControllerDef
{
public:

	InteractiveAnimControllerDef( void ) :
		m_pobControllingEntity(0),
		m_fAnimMaxSpeed(1.0f),
		m_fAnimSpeedAcceleration(1.0f),
        m_fAnimSpeedDeacceleration(2.5f),
		m_fButtonPressInterval(0.5f)
	{
	}

	virtual ~InteractiveAnimControllerDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW InteractiveAnimControllerDef( *this ); }

	// ----- Controller parameters -----

	CHashedString		m_obAnimationName;
	CEntity*		m_pobControllingEntity;
	float			m_fAnimMaxSpeed;
	float			m_fAnimSpeedAcceleration;
	float			m_fAnimSpeedDeacceleration;
	float			m_fButtonPressInterval;
};


//------------------------------------------------------------------------------------------
//!
//!	InteractiveAnimController
//!	NOTE: Used by gatehouse wheel - not needed anymore!
//!
//------------------------------------------------------------------------------------------
class InteractiveAnimController : public MovementController
{
public:

	// Construction destruction
	InteractiveAnimController( CMovement* pobMovement, const InteractiveAnimControllerDef& obDefinition );
	virtual ~InteractiveAnimController( void );

	// The major functionality of any movement constroller
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );

protected:
	
	CAnimationPtr					m_obSingleAnimation;
	InteractiveAnimControllerDef	m_obDefinition;
	float							m_fAnimSpeed;
	float							m_fAccelerationTime;
};

//------------------------------------------------------------------------------------------
//!
//!	LadderParameters
//!
//------------------------------------------------------------------------------------------
#include "game/interactableparams.h"
class LadderParameters : public Attr_Interactable
{
public:

	LadderParameters () :
		m_obMountTopPosition(CONSTRUCT_CLEAR),
		m_obMountTopRotation(CONSTRUCT_CLEAR),
		m_obMountBottomPosition(CONSTRUCT_CLEAR),
		m_obMountBottomRotation(CONSTRUCT_CLEAR),
		m_fMoveHeight(4.0f)
	{
	}

	CHashedString		m_obAnimMoveTo;
	CHashedString		m_obAnimRunTo;
	CHashedString		m_obAnimClimbUp;
	CHashedString		m_obAnimClimbDown;
	CHashedString		m_obAnimMountTop;
	CHashedString		m_obAnimDismountTop;
	CHashedString		m_obAnimMountBottom;
	CHashedString		m_obAnimDismountBottom;

	CPoint			m_obMountTopPosition;
	CQuat			m_obMountTopRotation;
	CPoint			m_obMountBottomPosition;
	CQuat			m_obMountBottomRotation;

	float			m_fMoveHeight;
	float			m_fDismountHeight;

};


//------------------------------------------------------------------------------------------
//!
//!	LadderControllerDef
//!
//------------------------------------------------------------------------------------------
class LadderControllerDef : public MovementControllerDef
{
public:

	LadderControllerDef( void ) :
		m_pobLadderParameters(0),
		m_pobLadderEntity(0)
	{
	}

	virtual ~LadderControllerDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW LadderControllerDef( *this ); }

	// ----- Controller parameters -----

	LadderParameters*	m_pobLadderParameters;

	CEntity*			m_pobLadderEntity;
};

//------------------------------------------------------------------------------------------
//!
//!	LadderController
//!
//------------------------------------------------------------------------------------------

class LadderController : public MovementController
{
public:

	// Construction destruction
	LadderController( CMovement* pobMovement, const LadderControllerDef& obDefinition );
	virtual ~LadderController( void );

	// The major functionality of any movement constroller
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );

protected:

	// A COPY of our defintion
	LadderControllerDef m_obDefinition;

	enum LADDER_STATE
	{
		MOUNT,
		CLIMB,
		DISMOUNT,
		FINISHED,
	};

	LADDER_STATE			m_eLadderState;

	CMatrix					m_obLadderWorldMatrix;

	float					m_fMoveHeight;
	float					m_fDismountHeight;

	float					m_fDismountScalar;

	CAnimationPtr 			m_obMountAnimation;
	CAnimationPtr 			m_obClimbAnimation;
	CAnimationPtr			m_obDismountAnimation;

	CPoint				m_obMountPosition;
	CQuat					m_obMountRotation;

	CEntity*				m_pobLadder;

	bool					m_bClimbingUp;
};




/* // Replaced by Button mash controller/object
// Parameters for a crank/door setup - this is on a per instance basis, not a per crank type!

class CrankSharedParameters
{
public:

	HAS_INTERFACE( CrankSharedParameters );

	CrankSharedParameters () :
		m_pobCrankEntity(0),
		m_pobCrankOperatorEntity(0),
		m_pobDoorEntity(0),
		m_fAnimTime(0.0f),
		m_fAnimSpeed(0.0f),
		m_fOpenTime(0.0f),
		m_fDoorSpeed(0.0f)
	{
	}

	// ----- Serialised parameters -----

	CHashedString 	m_obAnimCharacter;
	CHashedString 	m_obAnimCrank;

	CPoint 		m_obCharacterTranslationOffset;
	CPoint 		m_obCharacterRotationOffset;

	float 		m_fAnimAcceleration;
	float 		m_fAnimDeacceleration;

	float 		m_fDoorOpenHeight;
	float 		m_fDoorOpenSpeed;
	float 		m_fDoorCloseSpeed;
	bool		m_bDoorOpenOnStart;
	bool		m_bDoorLockedOnStart;

	// ----- Shared parameters -----

	// These essentially represent the state of the crank, allowing player and crank to be synchronised.

	CEntity* 	m_pobCrankEntity;
	CEntity* 	m_pobCrankOperatorEntity;
	CEntity* 	m_pobDoorEntity;

	float 		m_fAnimTime; // Time offset of the crank anim
	float 		m_fAnimSpeed; // The speed at which the crank anim is playing at
	float 		m_fOpenTime; // Time spent making this door open up

	float 		m_fDoorMinY;
	float 		m_fDoorMaxY;
	float 		m_fDoorSpeed;
	CPoint		m_obDoorPosition;
	bool		m_bDoorLocked;
};





// ---------- Crank user ----------

class CrankOperatorControllerDef : public MovementControllerDef
{
public:

	CrankOperatorControllerDef( void ) {}

	virtual ~CrankOperatorControllerDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW CrankOperatorControllerDef( *this ); }

	// ----- Controller parameters -----

	CHashedString m_obCrankParameters;
};

class CrankOperatorController : public MovementController
{
public:

	// Construction destruction
	CrankOperatorController( CMovement* pobMovement, const CrankOperatorControllerDef& obDefinition );
	virtual ~CrankOperatorController( void );

	// The major functionality of any movement constroller
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );

protected:

	CrankSharedParameters*	m_pobParameters;

	CAnimationPtr 			m_obSingleAnimation;
};

// ---------- Crank lever ----------

class CrankLeverControllerDef : public MovementControllerDef
{
public:

	CrankLeverControllerDef( void ) {}

	virtual ~CrankLeverControllerDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW CrankLeverControllerDef( *this ); }

	// ----- Controller parameters -----

	CHashedString m_obCrankParameters;
};

class CrankLeverController : public MovementController
{
public:

	// Construction destruction
	CrankLeverController( CMovement* pobMovement, const CrankLeverControllerDef& obDefinition );
	virtual ~CrankLeverController( void );

	// The major functionality of any movement constroller
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );

protected:

	CrankSharedParameters*	m_pobParameters;

	CAnimationPtr 			m_obSingleAnimation;
};

// ---------- Crank door ----------

class CrankDoorControllerDef : public MovementControllerDef
{
public:

	CrankDoorControllerDef( void ) {}

	virtual ~CrankDoorControllerDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW CrankDoorControllerDef( *this ); }

	// ----- Controller parameters -----

	CHashedString m_obCrankParameters;
};

class CrankDoorController : public MovementController
{
public:

	// Construction destruction
	CrankDoorController( CMovement* pobMovement, const CrankDoorControllerDef& obDefinition );
	virtual ~CrankDoorController( void ) {}

	// The major functionality of any movement constroller
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );

protected:

	CrankSharedParameters*	m_pobParameters;
};*/

//------------------------------------------------------------------------------------------
//!
//!	Button Mash Shared Parameters
//!
//------------------------------------------------------------------------------------------
class ButtonMashSharedParams
{
public:

	enum BUTTONMASHOBJECT_STATE
	{
		INACTIVE,	// Operator is using object, but buttonmash animation is at start and no button mashing is occuring
		BUTTONMASH,	// Operator is using object, and is button mashing!
		SUCCESS,	// All done.
		FAILED,		// Failed.
	};

	ButtonMashSharedParams () :
		m_obCharacterTranslationOffset( CONSTRUCT_CLEAR ),
		m_obCharacterRotationOffset( CONSTRUCT_CLEAR ),
		m_pObjectEntity(0),
		m_pOperatorEntity(0),
		m_fSecondsPerMashStart(1.0),
		m_fSecondsPerMashEnd(0.25),
		m_fMPSPveRateVariation(0.5),
		m_fMPSNveRateVariation(0.5),
		m_fRequiredMPS(1.0f),
		m_fCurrentInterpolant(0.0f),
		m_fCurrentMPS(1.0f),
		m_fMashPressTime(0.0f),
		m_fAnimSpeed(0.0f),
		m_fAnimTime(0.0f),
		m_fAnimAcceleration(0.0f),
		m_fAnimDurationUser(0.0f),
	 	m_fAnimDurationObject(0.0f),
		m_eButtonMashState(INACTIVE),
		m_iButtonMashRepeats( 1 ),
		m_bExitOnFail ( false )
	{
	}

	CPoint 	m_obCharacterTranslationOffset;
	CPoint 	m_obCharacterRotationOffset;

	CEntity*	m_pObjectEntity;
	CEntity*	m_pOperatorEntity;

	float	m_fSecondsPerMashStart;	// Required Seconds Per Mash At Start (1/MPS)
	float	m_fSecondsPerMashEnd;	// Required Seconds Per Mash At End (1/MPS)
	float	m_fMPSPveRateVariation;	// Variation rate of how fast you press it. (Should be > 0.0)
	float	m_fMPSNveRateVariation;	// Variation rate of how fast you press it. (Should be > 0.0)

	float	m_fRequiredMPS;		// Current Required Mashes Per Second (MPS)

	// Variables used throughout the button mashing
	float	m_fCurrentInterpolant;	// Current interpolant value
	float	m_fCurrentMPS;			// Current MPS
	float	m_fMashPressTime;		// Time since last button press

	// Variables for animation
	float	m_fAnimSpeed;
	float	m_fAnimTime;
	float 	m_fAnimAcceleration;

	float 	m_fAnimDurationUser;
	float 	m_fAnimDurationObject;

	BUTTONMASHOBJECT_STATE	m_eButtonMashState;

	CHashedString	m_obObjectButtonMashAnimName;
	CHashedString	m_obOperatorButtonMashAnimName;

	int m_iButtonMashRepeats;
	int m_iButtonMashCount;

	bool m_bExitOnFail;
};


//------------------------------------------------------------------------------------------
//!
//!	Button Mash Controller Def
//!
//------------------------------------------------------------------------------------------
class ButtonMashController;
class ButtonMashControllerDef : public MovementControllerDef
{
public:

	ButtonMashControllerDef( void ) {}

	virtual ~ButtonMashControllerDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW ButtonMashControllerDef( *this ); }

	// ----- Controller parameters -----

	ButtonMashSharedParams*	m_pSharedParams;
	
};

//------------------------------------------------------------------------------------------
//!
//!	Button Mash Controller
//!		MPS = Mashes per second.  Yes, that's the technical term.  Hertz is unacceptable.
//!
//------------------------------------------------------------------------------------------
class ButtonMashOperatorController;	// Forward declaration required
class ButtonMashController : public MovementController
{
public:

	ButtonMashController( CMovement* pobMovement, const ButtonMashControllerDef& obDefinition );
	virtual ~ButtonMashController( void );

	// The major functionality of any movement constroller
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );

private:

	ButtonMashSharedParams*	m_pSharedParams;
	
	CAnimationPtr 	m_obButtonMashAnim;

};


//------------------------------------------------------------------------------------------
//!
//!	Button Mash Operator Controller Def
//!
//------------------------------------------------------------------------------------------
class ButtonMashOperatorControllerDef : public MovementControllerDef
{
public:

	ButtonMashOperatorControllerDef( void ) {}

	virtual ~ButtonMashOperatorControllerDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW ButtonMashOperatorControllerDef( *this ); }

	// ----- Shared parameters -----

	ButtonMashSharedParams*	m_pSharedParams;

};


//------------------------------------------------------------------------------------------
//!
//!	Button Mash Operator Controller
//!		MPS = Mashes per second.  Yes, that's the technical term.  Hertz is unacceptable.
//!
//------------------------------------------------------------------------------------------
class ButtonMashOperatorController : public MovementController
{
public:

	// Construction destruction
	ButtonMashOperatorController( CMovement* pobMovement, const ButtonMashOperatorControllerDef& obDefinition );
	virtual ~ButtonMashOperatorController( void );

	// The major functionality of any movement constroller
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );

protected:
	
	ButtonMashSharedParams*	m_pSharedParams;

	CAnimationPtr 			m_obButtonMashAnim;

};

//------------------------------------------------------------------------------------------
//!
//!	CoordinatedTransitionDef
//!	A simple structure that needs to be filled out to create a coordinated transition.  
//! Can be predefined in xml if one wishes.
//!
//------------------------------------------------------------------------------------------
class CoordinationParams
{
public:
	float m_fSpeed;
	float m_fNormalisedTime;

	CoordinationParams() { m_fSpeed = m_fNormalisedTime = 0.0f; };
};

class CoordinatedTransitionDef : public MovementControllerDef
{
public:

	// Construction
	CoordinatedTransitionDef( void );
	virtual ~CoordinatedTransitionDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW CoordinatedTransitionDef( *this ); }

	// Members
	bool		m_bApplyGravity;
	CHashedString	m_obAnimationName;

	CoordinationParams* m_pobCoordinationParams;
};

//------------------------------------------------------------------------------------------
//!
//!	CoordinatedTransition
//!	Used to play an animation coordinated to another object
//!
//------------------------------------------------------------------------------------------
class CoordinatedTransition : public MovementController
{
public:

	// Construction destruction
	CoordinatedTransition( CMovement* pobMovement, const CoordinatedTransitionDef& obDefinition );
	virtual ~CoordinatedTransition( void );

	// The major functionality of any movement constroller
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );

protected:

	CPoint m_obPrevTrans;

	// A COPY of our defintion
	CoordinatedTransitionDef m_obDefinition;

	// A pointer to our single animation
	CAnimationPtr m_obSingleAnimation;
};

/* // Replaced by button mash
//------------------------------------------------------------------------------------------
//!
//!	CounterWeight Lever setup
//!	
//!
//------------------------------------------------------------------------------------------

class CounterWeightLeverSharedParameters
{
public:

	HAS_INTERFACE( CounterWeightLeverSharedParameters );

	CounterWeightLeverSharedParameters () :
		m_pobLeverEntity(0),
		m_pobLeverOperatorEntity(0),
		m_fAnimTime(0.0f),
		m_fAnimSpeed(0.0f),
		m_fOpenTime(0.0f),
		m_fRequiredButtonRate(1.0f),
		m_fButtonRate(0.0f),
		m_fButtonPressTime(0.0f),
		m_eLeverState(INACTIVE)
	{
	}

	// ----- Serialised parameters -----

	CHashedString 	m_obAnimCharacterPush;
	CHashedString 	m_obAnimLeverLift;
	CHashedString 	m_obAnimLeverFall;

	CPoint 		m_obCharacterTranslationOffset;
	CPoint 		m_obCharacterRotationOffset;

	float 		m_fAnimAcceleration;
	float 		m_fAnimDeacceleration;
	

	// ----- Shared parameters -----

	// These essentially represent the state of the crank, allowing player and crank to be synchronised.

	CEntity* 	m_pobLeverEntity;
	CEntity* 	m_pobLeverOperatorEntity;
	
	float 		m_fAnimTime;			// Time offset of the Lever anim
	float 		m_fAnimSpeed;			// The speed at which the lever anim is playing at	
	float		m_fOpenTime;			// Time since player input
	float		m_fRequiredButtonRate;  // Ideal time between button presses
	float		m_fButtonRate;			// Current time between button presses
	float		m_fButtonPressTime;		// Time since last button press

	
	enum COUNTERWEIGHTLEVER_STATE
	{
		INACTIVE,
		LIFT,
		FALL,
	};

	COUNTERWEIGHTLEVER_STATE			m_eLeverState;
};



// ---------- CounterWeight Lever user ----------

class CounterWeightLeverOperatorControllerDef : public MovementControllerDef
{
public:

	CounterWeightLeverOperatorControllerDef( void ) {}

	virtual ~CounterWeightLeverOperatorControllerDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW CounterWeightLeverOperatorControllerDef( *this ); }

	// ----- Shared parameters -----

	CHashedString m_obLeverParameters;
};

class CounterWeightLeverOperatorController : public MovementController
{
public:

	// Construction destruction
	CounterWeightLeverOperatorController( CMovement* pobMovement, const CounterWeightLeverOperatorControllerDef& obDefinition );
	virtual ~CounterWeightLeverOperatorController( void );

	// The major functionality of any movement constroller
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );

protected:

	CounterWeightLeverSharedParameters*	m_pobParameters;

	CAnimationPtr 			m_obPushAnimation;		// Player pushing lever anim

	float			m_fRecoverAnimTime;		// Local anim time, as player is not synced once lever falls
	float			m_fRecoverAnimSpeed;	// Local anim speed, as player is not synced once lever falls
};

// ---------- CounterWeight lever ----------

class CounterWeightLeverControllerDef : public MovementControllerDef
{
public:

	CounterWeightLeverControllerDef( void ) {}

	virtual ~CounterWeightLeverControllerDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW CounterWeightLeverControllerDef( *this ); }

	// ----- Shared parameters -----

	CHashedString m_obLeverParameters;
};

class CounterWeightLeverController : public MovementController
{
public:

	// Construction destruction
	CounterWeightLeverController( CMovement* pobMovement, const CounterWeightLeverControllerDef& obDefinition );
	virtual ~CounterWeightLeverController( void );

	// The major functionality of any movement constroller
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );

protected:

	CounterWeightLeverSharedParameters*	m_pobParameters;

	CAnimationPtr 			m_obLiftAnimation;
	CAnimationPtr 			m_obFallAnimation;

};
*/


/* // Replaced by collapsable object FSM
//------------------------------------------------------------------------------------------
//!
//!	AnimatedCollapseTransitionDef
//!	
//!
//------------------------------------------------------------------------------------------
class AnimatedCollapseTransitionDef : public MovementControllerDef
{
public:

	// Construction
	AnimatedCollapseTransitionDef( void );
	virtual ~AnimatedCollapseTransitionDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW AnimatedCollapseTransitionDef( *this ); }

	// Members
	CHashedString	m_obAnimationName;
};

//------------------------------------------------------------------------------------------
//!
//!	AnimatedCollapseTransition
//!	
//!
//------------------------------------------------------------------------------------------
class AnimatedCollapseTransition : public MovementController
{
public:

	// Construction destruction
	AnimatedCollapseTransition( CMovement* pobMovement, const AnimatedCollapseTransitionDef& obDefinition );
	virtual ~AnimatedCollapseTransition( void );

	// The major functionality of any movement constroller
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );

protected:

	// A pointer to our single animation
	CAnimationPtr m_obSingleAnimation;

	bool m_bFinished;
};*/


#endif // _INTERACTIONTRANSITIONS_H

