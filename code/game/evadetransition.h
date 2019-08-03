//------------------------------------------------------------------------------------------
//!
//!	\file evadetransition.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_EVADETRANSITION_H
#define	_EVADETRANSITION_H

// Necessary includes
#include "movementcontrollerinterface.h"

// Forward declarations
class CMovement;

//------------------------------------------------------------------------------------------
//!
//!	EvadeTransitionDef
//!	The available parameters for describing an evade movement
//!
//------------------------------------------------------------------------------------------
class EvadeTransitionDef : public MovementControllerDef
{
public:

	// Construction
	EvadeTransitionDef( void );
	virtual ~EvadeTransitionDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW EvadeTransitionDef( *this ); }

	// What we need to define the movement
	CHashedString	m_obEvadeAnim;
	float		m_fAvoidanceRadius;
	float		m_fMovementDuration;
	CDirection	m_obDirection;
	bool		m_bDoTargetedEvade;
	
	float m_fMaximumRotationSpeed;
	float m_fMaximumTargetingRadius;
}; 


//------------------------------------------------------------------------------------------
//!
//!	EvadeTransition
//!	Controls the full movement of an evade
//!
//------------------------------------------------------------------------------------------
class EvadeTransition : public MovementController
{
public:

	// Construction destruction
	EvadeTransition( CMovement* pobMovement, const EvadeTransitionDef& obDefinition );
	virtual ~EvadeTransition( void );

	// The main beef, this moves the character about
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );


protected:

	// A COPY of our defintion
	EvadeTransitionDef m_obDefinition;

	// A pointer to our single animation
	CAnimationPtr m_obSingleAnimation;

	bool m_bZeroDistanceReached;
};

#endif // _EVADETRANSITION_H
