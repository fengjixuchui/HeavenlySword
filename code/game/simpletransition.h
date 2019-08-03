//------------------------------------------------------------------------------------------
//!
//!	\file simpletransition.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_SIMPLETRANSITION_H
#define	_SIMPLETRANSITION_H

// Necessary includes
#include "movementcontrollerinterface.h"

//------------------------------------------------------------------------------------------
//!
//!	SimpleTimedTransitionDef
//!	Used to play an animation and move
//!
//------------------------------------------------------------------------------------------
class SimpleTimedTransitionDef : public MovementControllerDef
{
public:

	// Construction
	SimpleTimedTransitionDef( void );
	virtual ~SimpleTimedTransitionDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW SimpleTimedTransitionDef( *this ); }

	// Members
	bool		m_bApplyGravity;
	bool		m_bLooping;
	CHashedString	m_obAnimationName;
	float		m_fTime;
	float		m_fExtraMovementSpeed;
};

//------------------------------------------------------------------------------------------
//!
//!	SimpleTimedTransition
//!	Used to play an animation and move
//!
//------------------------------------------------------------------------------------------
class SimpleTimedTransition : public MovementController
{
public:

	// Construction destruction
	SimpleTimedTransition( CMovement* pobMovement, const SimpleTimedTransitionDef& obDefinition );
	virtual ~SimpleTimedTransition( void );

	// The major functionality of any movement constroller
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );

protected:

	// A COPY of our defintion
	SimpleTimedTransitionDef m_obDefinition;

	// A pointer to our single animation
	CAnimationPtr m_obSingleAnimation;

	float m_fTimeInTransition;
};

//------------------------------------------------------------------------------------------
//!
//!	SimpleTransitionDef
//!	A simple structure that needs to be filled out to create a simple transition.  Can be
//! predefined in xml if one wishes.
//!
//------------------------------------------------------------------------------------------
class SimpleTransitionDef : public MovementControllerDef
{
public:

	// Construction
	SimpleTransitionDef( void );
	virtual ~SimpleTransitionDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW SimpleTransitionDef( *this ); }

	// Members
	bool		m_bApplyGravity;
	bool		m_bLooping;
	CHashedString	m_obAnimationName;

	float       m_fSpeed;
	float		m_fTimeOffsetPercentage;
};

//------------------------------------------------------------------------------------------
//!
//!	SimpleTransition
//!	Used to play a non looping animation on more compolex entities with a movement component
//!
//------------------------------------------------------------------------------------------
class SimpleTransition : public MovementController
{
public:

	// Construction destruction
	SimpleTransition( CMovement* pobMovement, const SimpleTransitionDef& obDefinition );
	virtual ~SimpleTransition( void );

	// The major functionality of any movement constroller
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );

protected:

	// A COPY of our defintion
	SimpleTransitionDef m_obDefinition;

	// A pointer to our single animation
	CAnimationPtr m_obSingleAnimation;
};


#endif // _SIMPLETRANSITION_H
