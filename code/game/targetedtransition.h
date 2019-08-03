//------------------------------------------------------------------------------------------
//!
//!	\file targetedtransition.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_TARGETEDTRANSITION_H
#define	_TARGETEDTRANSITION_H

// Necessary includes
#include "movementcontrollerinterface.h"

// Forward declarations
class CMovement;
class CEntity;

//------------------------------------------------------------------------------------------
//!
//!	TargetedTransitionToPointDef
//!
//------------------------------------------------------------------------------------------
class TargetedTransitionToPointDef : public MovementControllerDef
{
public:

	// Construction
	TargetedTransitionToPointDef( void );
	virtual ~TargetedTransitionToPointDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW TargetedTransitionToPointDef( *this ); }

	// What we need to define the movement
	CHashedString	m_obAnimationName;
	bool			m_bApplyGravity;
	CPoint			m_obPoint;
	float			m_fExtraSpeed;
	bool			m_bTravellingBackwards;
	float			m_fRadius;
}; 

//------------------------------------------------------------------------------------------
//!
//!	ZAxisAlignTargetedTransition
//!
//------------------------------------------------------------------------------------------
class TargetedTransitionToPoint : public MovementController
{
public:

	// Construction destruction
	TargetedTransitionToPoint( CMovement* pobMovement, const TargetedTransitionToPointDef& obDefinition );
	virtual ~TargetedTransitionToPoint( void );

	// The main beef, this moves the character about
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );
protected:

	// A COPY of our defintion
	TargetedTransitionToPointDef m_obDefinition;

	// A pointer to our single animation
	CAnimationPtr m_obSingleAnimation;

	CPoint m_obStartPoint;
};



//------------------------------------------------------------------------------------------
//!
//!	TargetedTransitionToPointManualMoveDef
//!
//! This transition manually moves an entity with SetPosition(), so is only advised for
//! objects that currently have their physics disabled and don't need to worry about
//! collision etc. It was programmed for use with the Demon's leave/return transitions.
//!
//------------------------------------------------------------------------------------------
class TargetedTransitionToPointManualMoveDef : public MovementControllerDef
{
public:

	// Construction
	TargetedTransitionToPointManualMoveDef( void );
	virtual ~TargetedTransitionToPointManualMoveDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW TargetedTransitionToPointManualMoveDef( *this ); }

	// What we need to define the movement
	CHashedString	m_obAnimationName;
	CPoint			m_obPoint;
	CDirection		m_obRotationToFix;
//	CQuat			m_obRotationToFix;
	float			m_fSpeed;
}; 


//------------------------------------------------------------------------------------------
//!
//!	TargetedTransitionToPointManualMove
//!
//! This transition manually moves an entity with SetPosition(), so is only advised for
//! objects that currently have their physics disabled and don't need to worry about
//! collision etc. It was programmed for use with the Demon's leave/return transitions.
//!
//------------------------------------------------------------------------------------------
class TargetedTransitionToPointManualMove : public MovementController
{
public:

	// Construction destruction
	TargetedTransitionToPointManualMove( CMovement* pobMovement, const TargetedTransitionToPointManualMoveDef& obDefinition );
	virtual ~TargetedTransitionToPointManualMove( void );

	// The main beef, this moves the character about
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );
protected:

	// A COPY of our defintion
	TargetedTransitionToPointManualMoveDef m_obDefinition;

	// A pointer to our single animation
	CAnimationPtr m_obSingleAnimation;

	//Store the direction we want to head in so that we can 'tween' along our vector.
	CDirection m_obDirection;
	//Store a flag for whether it's finished. Once the NotifyMovementDone() feedback has been sent this MUST stop, or
	//it could provide a further offset to an entity that considers itself "done". This could cause problems if the
	//entity has just been parented, as it will affect it's local translation/rotation around the offset.
	bool m_bFinished;
};



//------------------------------------------------------------------------------------------
//!
//!	ZAxisAlignTargetedTransitionDef
//!
//------------------------------------------------------------------------------------------
class ZAxisAlignTargetedTransitionDef : public MovementControllerDef
{
public:

	// Construction
	ZAxisAlignTargetedTransitionDef( void );
	virtual ~ZAxisAlignTargetedTransitionDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW ZAxisAlignTargetedTransitionDef( *this ); }

	// What we need to define the movement
	CHashedString	m_obAnimationName;
	bool			m_bApplyGravity;
	CDirection		m_obAlignZTo;
	CEntity*		m_pobEntityAlignZTowards;
	CPoint			m_obScaleToCoverDistance;
}; 

//------------------------------------------------------------------------------------------
//!
//!	ZAxisAlignTargetedTransition
//!
//------------------------------------------------------------------------------------------
class ZAxisAlignTargetedTransition : public MovementController
{
public:

	// Construction destruction
	ZAxisAlignTargetedTransition( CMovement* pobMovement, const ZAxisAlignTargetedTransitionDef& obDefinition );
	virtual ~ZAxisAlignTargetedTransition( void );

	// The main beef, this moves the character about
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );
protected:

	// A COPY of our defintion
	ZAxisAlignTargetedTransitionDef m_obDefinition;

	// A pointer to our single animation
	CAnimationPtr m_obSingleAnimation;

	float m_fYawNeeded;
	CDirection m_obStartZ, m_obDesiredZ;
	float m_fDistanceScalar;
};


//------------------------------------------------------------------------------------------
//!
//!	StandardTargetedTransitionDef
//!	Define the standard targeted movement
//!
//------------------------------------------------------------------------------------------
class StandardTargetedTransitionDef : public MovementControllerDef
{
public:

	// Construction
	StandardTargetedTransitionDef( void );
	virtual ~StandardTargetedTransitionDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW StandardTargetedTransitionDef( *this ); }

	// What we need to define the movement
	float			m_fMaximumTargetingRadius;
	bool			m_bReversed;
	bool			m_bTrackTarget;
	float			m_fMaximumRotationSpeed;
	CHashedString		m_obAnimationName;
	float			m_fMovementDuration;
	bool			m_bApplyGravity;
	bool			m_bLooping;
	bool			m_bStopWhenOnGround;
}; 

//------------------------------------------------------------------------------------------
//!
//!	StandardTargetedTransition
//!	The most general of target transition controllers - all the data is taken from the an 
//!	external system - all the movement is taken from the given animiser
//!
//!	This is a usable instance of what that says just above.
//!
//------------------------------------------------------------------------------------------
class StandardTargetedTransition : public MovementController
{
public:

	// Construction destruction
	StandardTargetedTransition( CMovement* pobMovement, const StandardTargetedTransitionDef& obDefinition );
	virtual ~StandardTargetedTransition( void );

	// The main beef, this moves the character about
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );


protected:

	// A COPY of our defintion
	StandardTargetedTransitionDef m_obDefinition;

	// A pointer to our single animation
	CAnimationPtr m_obSingleAnimation;

	// Movement time remaing
	float m_fMovementDurationRemaining;

	// Transition time
	float m_fTransitionTime;

	// Zero distance reached
	bool m_bZeroDistanceReached;
};


//------------------------------------------------------------------------------------------
//!
//!	ScaledTargetedTransitionDef
//!	Define the scaled targeted movement
//!
//------------------------------------------------------------------------------------------
class ScaledTargetedTransitionDef : public MovementControllerDef
{
public:

	// Construction
	ScaledTargetedTransitionDef( void );
	virtual ~ScaledTargetedTransitionDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW ScaledTargetedTransitionDef( *this ); }

	// What we need to define the movement
	CHashedString		m_obAnimationName;
	float			m_fMovementOffset;
	float			m_fMaxRange;
	float			m_fMovementDuration;
	bool			m_bApplyGravity;
	float			m_fScaleToTime;
	bool			m_bTrackAfterScale;
	bool			m_b3DScaling;
	bool			m_bScaleDown;
	bool			m_bNoDirectionCorrectionScaling;
	CAnimationPtr	m_pobAnimation;
	bool			m_bNoRotateIfTargetBehind;
}; 

//------------------------------------------------------------------------------------------
//!
//!	CScaledTargetedTransition
//!	This transition type scales the per frame root translation to get where we want to go.  
//!	For example if the animation would naturally provide 3 meters of forward movement and 
//!	we wanted to travel 3.6 meters we would scale the root offset each frame by 1.2
//!
//------------------------------------------------------------------------------------------
class ScaledTargetedTransition : public MovementController
{
public:

	// Construction destruction
	ScaledTargetedTransition( CMovement* pobMovement, const ScaledTargetedTransitionDef& obDefinition );
	virtual ~ScaledTargetedTransition( void );

	// The main beef, this moves the character about
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );

protected:

	// A COPY of our defintion
	ScaledTargetedTransitionDef m_obDefinition;

	// A pointer to our single animation
	CAnimationPtr m_obSingleAnimation;

	// Movement time remaing
	float m_fMovementDurationRemaining;

	// Transition time
	float m_fTransitionTime;

	// Movement that the animation will give us over it's lifetime
	float m_fAnimationRotation;
	float m_fAnimationXOffset;
	float m_fAnimationZOffset;

	// Our initial angle on the first frame
	CMatrix m_obInitialPosition;	

	// How much procedural twist have we provided so far
	float m_fSumOfProceduralTwist;
	
	// Our constant scaling factor for the animation
	CDirection m_obAnimRootScalar;

	// How far will out animation take us
	CPoint m_obAnimationDistance;

	// Was the scaled distance trucated?
	bool m_fScaleClipped;

	// How much is the animation scaled by
	float m_fAnimationTimeScalar;

	// Track how far we have twisted after the first frame
	float m_fAdditionalProceduralTwist;
};


//------------------------------------------------------------------------------------------
//!
//!	BlockTargetedTransitionDef
//!	Define the block targeted movement
//!
//------------------------------------------------------------------------------------------
class BlockTargetedTransitionDef : public MovementControllerDef
{
public:

	// Construction
	BlockTargetedTransitionDef( void );
	virtual ~BlockTargetedTransitionDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW BlockTargetedTransitionDef( *this ); }

	// What do we need to define the movement
	bool		m_bStayAligned;
	CHashedString	m_obAnimationName;
	float		m_fMovementOffset;
	float		m_fAngleToTargetTime;
	float		m_fAngleToTarget;
	float		m_fMovementDuration;
	bool		m_bApplyGravity;
	bool		m_bSyncInitialHeight;
	bool		m_bTurnToFaceTarget;

}; 

//------------------------------------------------------------------------------------------
//!
//!	CBlockTargetedTransition
//!	Deals with a players transition into a block
//!
//------------------------------------------------------------------------------------------
class BlockTargetedTransition : public MovementController
{
public:

	// Construction destruction
	BlockTargetedTransition( CMovement* pobMovement, const BlockTargetedTransitionDef& obDefinition );
	virtual ~BlockTargetedTransition( void );

	// The main beef, this moves the character about
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );


protected:

	// A COPY of our defintion
	BlockTargetedTransitionDef m_obDefinition;

	// A pointer to our single animation
	CAnimationPtr m_obSingleAnimation;

	// Movement time remaing
	float m_fMovementDurationRemaining;

	// A monitor the timing of the alignment phase
	float m_fProceduralMovementTime;

};


//------------------------------------------------------------------------------------------
//!
//!	KOAftertouchTransitionDef
//!	Define the movement for KO aftertouch
//!
//------------------------------------------------------------------------------------------
class KOAftertouchTransitionDef : public MovementControllerDef
{
public:

	// Construction
	KOAftertouchTransitionDef( void );
	virtual ~KOAftertouchTransitionDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW KOAftertouchTransitionDef( *this ); }

	// What do we need to define the movement
	CHashedString		m_obAnimationName;
	float			m_fAngleToTarget;
	float			m_fMovementDuration;
	const CEntity*	m_pobAttacker;

}; 

//------------------------------------------------------------------------------------------
//!
//!	KOAftertouchTransition
//!	Deals with a player that has been KOd and we want to apply aftertouch to
//!
//------------------------------------------------------------------------------------------
class KOAftertouchTransition : public MovementController
{
public:

	// Construction destruction
	KOAftertouchTransition( CMovement* pobMovement, const KOAftertouchTransitionDef& obDefinition );
	virtual ~KOAftertouchTransition( void );

	// The main beef, this moves the character about
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );


protected:

	// A COPY of our defintion
	KOAftertouchTransitionDef m_obDefinition;

	// A pointer to our single animation
	CAnimationPtr m_obSingleAnimation;

	// Movement time remaing
	float m_fMovementDurationRemaining;

	float m_fYawDelta;
};

//------------------------------------------------------------------------------------------
//!
//!	FallAftertouchTransitionDef
//!	Define the movement for Fall aftertouch
//!
//------------------------------------------------------------------------------------------
class FallAftertouchTransitionDef : public MovementControllerDef
{
public:

	// Construction
	FallAftertouchTransitionDef( void );
	virtual ~FallAftertouchTransitionDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW FallAftertouchTransitionDef( *this ); }

	// What do we need to define the movement
	CHashedString		m_obAnimationName;
	float			m_fMaxHorizontalVelocity;
	float			m_fHorizontalAccelFactor;
	float			m_fMaxVerticalVelocity;
	float			m_fVerticalAccel;
	CEntity*		m_pobParentEntity;
	CEntity*		m_pobControllingEntity;
}; 

//------------------------------------------------------------------------------------------
//!
//!	FallAftertouchTransition
//!	Deals with a player that is falling and we want to apply aftertouch to
//!
//------------------------------------------------------------------------------------------
class FallAftertouchTransition : public MovementController
{
public:

	// Construction destruction
	FallAftertouchTransition( CMovement* pobMovement, const FallAftertouchTransitionDef& obDefinition );
	virtual ~FallAftertouchTransition( void );

	// The main beef, this moves the character about
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );


protected:

	// A COPY of our defintion
	FallAftertouchTransitionDef m_obDefinition;

	// A pointer to our single animation
	CAnimationPtr m_obSingleAnimation;

	float m_fXVelocity;
	float m_fYVelocity;
	
	CEntity*		m_pobParentEntity;
	CEntity*		m_pobControllingEntity;

	// Some stuff we save on the first frame
	CDirection m_obVerticalVelocity;
	CDirection m_obVerticalAcceleration;
	CDirection m_obHorizontalVelocity;
};

#endif //_TARGETEDTRANSITION_H
