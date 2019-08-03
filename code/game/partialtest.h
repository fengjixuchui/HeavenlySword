//------------------------------------------------------------------------------------------
//!
//!	\file partialtest.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_PARTIALTEST_H
#define	_PARTIALTEST_H

// Necessary includes
#include "movementcontrollerinterface.h"

// Forward declarations
class CMovement;

//------------------------------------------------------------------------------------------
//!
//!	PartialTestDef
//!	The available parameters for testing partial animation functionality
//!
//------------------------------------------------------------------------------------------
class PartialTestDef : public MovementControllerDef
{
public:

	// Construction
	PartialTestDef( void );
	virtual ~PartialTestDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW PartialTestDef( *this ); }

	// What we need to define the movement
	CHashedString	m_obFullAnimation;
	CHashedString	m_obPartialAnimation;
	int			m_iPartialPriority;
}; 


//------------------------------------------------------------------------------------------
//!
//!	PartialTest
//!	A test controller for partial animation functionality
//!	
//!	This has been set up in order to investigate how the partial animation functionality we
//!	have works since it seems to be lost knowledge.  After implementing this test i have
//!	confirmed the following.
//!	
//!	We need to set the priority of partial animations to a higher level than the ones we want 
//!	to animate over.
//!	The blend weights need to sum to > 1 for partial animations to work - that is how the
//!	priority system works - the higher priorities steal potential blend weight from those
//!	of lower priority.
//!	The exporter does not currently have the capability to export character animations with
//!	limited channels - i had to hack a .txtanim file in order to get this going.
//!	
//------------------------------------------------------------------------------------------
class PartialTest : public MovementController
{
public:

	// Construction destruction
	PartialTest( CMovement* pobMovement, const PartialTestDef& obDefinition );
	virtual ~PartialTest( void );

	// The main beef, this moves the character about
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );


protected:

	// A COPY of our defintion
	PartialTestDef m_obDefinition;

	// A pointer to our animations
	CAnimationPtr m_obFullAnimation;
	CAnimationPtr m_obPartialAnimation;
};

#endif // _PARTIALTEST_H
