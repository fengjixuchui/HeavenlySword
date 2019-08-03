//------------------------------------------------------------------------------------------
//!
//!	\file relativetransitions.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_RELATIVETRANSITION_H
#define	_RELATIVETRANSITION_H

// Necessary includes
#include "movementcontrollerinterface.h"
#include "game/entity.h"
#include "game/entity.inl"

class Transform;

//------------------------------------------------------------------------------------------
//!
//!	SimpleRelativeTransitionDef
//!	The definition of simple transition to control a single relative animation.
//!
//------------------------------------------------------------------------------------------
class SimpleRelativeTransitionDef : public MovementControllerDef
{
public:

	// Construction
	SimpleRelativeTransitionDef( void );
	virtual ~SimpleRelativeTransitionDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW SimpleRelativeTransitionDef( *this ); }

	// Members
	CAnimationPtr	m_pobAnimation;
	Transform*		m_pobRelativeTransform;
	float			m_fMovementDuration;
	bool			m_bOwnsTransform;
	bool			m_bLooping;
	bool			m_bNeedsToResynch;
	const CEntity*	m_pobResynchToEntity;
	bool			m_bInteractWithSyncdTransform;
	float			m_fMaxRotationPerSecond;
	CDirection		m_obMovementSpeed;
	bool			m_bReverseInteractiveStickInput;
	float			m_fCollisionCheckDistance;
	float			m_fInteractiveCollisionCheckStartHeight;
	int				m_iCollisionCheckHeightCount;
	float			m_fCollisionCheckHeightInterval;
};


//------------------------------------------------------------------------------------------
//!
//!	SimpleRelativeTransition
//!	A simple transition to control a single relative animation.
//!
//------------------------------------------------------------------------------------------
class SimpleRelativeTransition : public MovementController
{
public:

	// Construction destruction
	SimpleRelativeTransition( CMovement* pobMovement, const SimpleRelativeTransitionDef& obDefinition );
	virtual ~SimpleRelativeTransition( void );

	// The major functionality of any movement constroller
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );

protected:

	// A COPY of our defintion
	SimpleRelativeTransitionDef m_obDefinition;
};


#endif // _RELATIVETRANSITION_H
