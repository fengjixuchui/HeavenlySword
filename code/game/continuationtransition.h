//------------------------------------------------------------------------------------------
//!
//!	\file continuationtransition.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_CONTINUATIONTRANSITION_H
#define	_CONTINUATIONTRANSITION_H

// Necessary includes
#include "movementcontrollerinterface.h"

#include "relativetransitions.h"

#include "game/entity.h"

// For ragdoll floored blendings
#include "physics/config.h"
#include "physics/havokincludes.h"
//#include <hkserialize/packfile/hkPackfileReader.h>
#include <hkmath/hkMath.h>
#include <hkbase/htl/hkArray.h>
#include <hkbase/memory/hkLocalArray.h>


//------------------------------------------------------------------------------------------
//!
//!	ContinuationTransitionDef
//!	Describes transition that will continue with the movement provided by the transition it 
//!	is blended over - until it hits something - currently only the floor.
//!
//------------------------------------------------------------------------------------------
class ContinuationTransitionDef : public MovementControllerDef
{
public:

	// Construction
	ContinuationTransitionDef( void );
	virtual ~ContinuationTransitionDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW ContinuationTransitionDef( *this ); }

	// Members
	bool		m_bLooping;
	bool		m_bEndOnGround;
	bool		m_bVerticalVelocity;
	bool		m_bVerticalAcceleration;
	bool		m_bHorizontalVelocity;
	bool		m_bHorizontalAcceleration;
	bool		m_bApplyGravity;
	CHashedString	m_obAnimationName;
};


//------------------------------------------------------------------------------------------
//!
//!	ContinuationTransition
//!	A transition that will continue with the movement provided by the transition it is
//!	blended over - until it hits something - currently only the floor, or ends the given 
//!	anim.
//!
//------------------------------------------------------------------------------------------
class ContinuationTransition : public MovementController
{
public:

	// Construction destruction
	ContinuationTransition( CMovement* pobMovement, const ContinuationTransitionDef& obDefinition );
	virtual ~ContinuationTransition( void );

	// The major functionality of any movement constroller
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );

protected:

	// A COPY of our defintion
	ContinuationTransitionDef m_obDefinition;

	// A pointer to our single animation
	CAnimationPtr m_obSingleAnimation;

	// Some stuff we save on the first frame
	CDirection m_obVerticalVelocity;
	CDirection m_obVerticalAcceleration;
	CDirection m_obHorizontalVelocity;
	CDirection m_obHorizontalAcceleration;

	// Did we have to crop the vertical acceleration
	bool m_bVerticalAccelerationCropped;

	float m_fTimeInTransition;
};

//------------------------------------------------------------------------------------------
//!
//!	RagdollContinuationTransitionDef
//!	Describes a ragdoll continuation transition
//!
//------------------------------------------------------------------------------------------
class RagdollContinuationTransitionDef  : public MovementControllerDef
{
public:
	// Construction
	RagdollContinuationTransitionDef( void ) {};
	virtual ~RagdollContinuationTransitionDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const;

};

//------------------------------------------------------------------------------------------
//!
//!	RagdollContinuationTransition
//!	A transition that will take care of a ragdoll, and it's safe transition to another state
//! if and when it's needed.
//!
//------------------------------------------------------------------------------------------
class RagdollContinuationTransition : public MovementController
{
public:

	// Construction destruction
	RagdollContinuationTransition( CMovement* pobMovement, const RagdollContinuationTransitionDef& obDef );
	virtual ~RagdollContinuationTransition( void ) {};

	// The major functionality of any movement constroller
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );

private:
	RagdollContinuationTransitionDef m_obDef;
	float m_fTimeInRagdoll, m_fTimeToEnd;
};

//------------------------------------------------------------------------------------------
//!
//!	RagdollFlooredTransitionDef
//!	Describes a ragdoll floored transition, only useful piece of info is the anim to play
//!
//------------------------------------------------------------------------------------------
class RagdollFlooredTransitionDef  : public MovementControllerDef
{
public:
	// Construction
	RagdollFlooredTransitionDef( void ) {};
	virtual ~RagdollFlooredTransitionDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const;

	CHashedString m_obFlooredAnimationName;
};

//------------------------------------------------------------------------------------------
//!
//!	RagdollFlooredTransitionDef
//!	A ragdoll floored transition, will play an anim (which will get animated physically by
//! by the ragdoll), and will physically rotate the ragdoll face up or face down according
//! to reaction zone, then pin it to the floor by force. This stops it moving around too much.
//!
//------------------------------------------------------------------------------------------
class RagdollFlooredTransition : public MovementController
{
public:

	// Construction destruction
	RagdollFlooredTransition( CMovement* pobMovement, const RagdollFlooredTransitionDef& obDef );
	virtual ~RagdollFlooredTransition( void );

	// The major functionality of any movement constroller
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );

private:
	RagdollFlooredTransitionDef m_obDef;
	CAnimationPtr m_obFlooredAnimation;
	CMatrix m_obTargetOrientation;
	hkArray<hkQsTransform> m_aobFinalRagdollAnimatedPose, m_aobNewBlendedAnimatedPose;
};

//------------------------------------------------------------------------------------------
//!
//!	SuperStyleSafetyTransitionDef
//!
//------------------------------------------------------------------------------------------
class SuperStyleSafetyTransitionDef  : public MovementControllerDef
{
public:
	// Construction
	SuperStyleSafetyTransitionDef( void )
		: m_obSafePoint(),
		m_pobNotifyThis( 0 ),
		m_pobTransitionAnimation( 0 ),
		m_pobRelativeTransform( 0 ),
		m_bRotate( true ),
		m_bTranslate( true ),
		m_bNotifyAtEnd( true )
	{};
	virtual ~SuperStyleSafetyTransitionDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const;

	// Where we're heading
	CPoint m_obSafePoint;
	// Who we need to tell
	CAttackComponent* m_pobNotifyThis;
	// Anim
	CAnimationPtr m_pobTransitionAnimation;
	// If we're animating, do it relative to this transform, which we will update
	Transform* m_pobRelativeTransform;
	// We can use combinations of these to do translations and rotations according to the def
	bool m_bRotate, m_bTranslate;
	// If this is not the end controller then this should be false
	bool m_bNotifyAtEnd;
};

//------------------------------------------------------------------------------------------
//!
//!	SuperStyleSafetyTransition
//!	A transition that will move into a safe area for a 
//! superstyle to take place.
//!
//------------------------------------------------------------------------------------------
class SuperStyleSafetyTransition : public MovementController
{
public:

	// Construction destruction
	SuperStyleSafetyTransition( CMovement* pobMovement, const SuperStyleSafetyTransitionDef& obDef );
	virtual ~SuperStyleSafetyTransition( void );

	// The major functionality of any movement constroller
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );

private:
	SuperStyleSafetyTransitionDef m_obDef;
	float m_fTimeInTransition;
	float m_fMovementSpeed;
	CPoint m_obStartPoint;
	CDirection m_obLastTravelVector;
	bool m_bDirectionFlipped;
	// How much we need to scale the anim
	float m_fScalarX, m_fScalarY, m_fScalarZ;
	// Start and end orientations
	CDirection m_obRelativeTransitionStartX, m_obRelativeTransitionStartY, m_obRelativeTransitionStartZ;
	CDirection m_obRelativeTransitionEndX, m_obRelativeTransitionEndY, m_obRelativeTransitionEndZ;
	float m_fYawNeeded;
};

//------------------------------------------------------------------------------------------
//!
//!	InputFollowingTransitionDef
//!
//------------------------------------------------------------------------------------------
class InputFollowingTransitionDef  : public MovementControllerDef
{
public:
	// Construction
	InputFollowingTransitionDef( void );
	virtual ~InputFollowingTransitionDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const;

	float m_fMaxRotationPerSecond;
	bool m_bApplyGravity;
	CHashedString m_obAnimName;
	CHashedString m_obLeanLeftAnimName, m_obLeanRightAnimName;
	float m_fExtraSpeed, m_fMaxSpeedChange, m_fSlowForTurnScalar, m_fMaxDirectionChange;
};

//------------------------------------------------------------------------------------------
//!
//!	InputFollowingTransition
//!
//------------------------------------------------------------------------------------------
class InputFollowingTransition : public MovementController
{
public:

	// Construction destruction
	InputFollowingTransition( CMovement* pobMovement, const InputFollowingTransitionDef& obDef );
	virtual ~InputFollowingTransition( void );

	// The major functionality of any movement constroller
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );

protected:
	// A COPY of our defintion
	InputFollowingTransitionDef m_obDefinition;

	// A pointer to our single animation
	CAnimationPtr m_obSingleAnimation;
	CAnimationPtr m_obLeftAnimation, m_obRightAnimation;

	float m_fLastYaw, m_fLastSpeed, m_fLastTimeStep;
};

#endif // _CONTINUATIONTRANSITION_H
