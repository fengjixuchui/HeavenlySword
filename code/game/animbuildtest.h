//------------------------------------------------------------------------------------------
//!
//!	\file animbuildtest.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_ANIMBUILDTEST_H
#define	_ANIMBUILDTEST_H

// Necessary includes
#include "movementcontrollerinterface.h"

// Forward declarations
class CMovement;
class CAnimationHeader;

//------------------------------------------------------------------------------------------
//!
//!	AnimBuildTestDef
//!	The available parameters for testing dynamically built animation headers
//!
//------------------------------------------------------------------------------------------
class AnimBuildTestDef : public MovementControllerDef
{
public:

	// Construction
	AnimBuildTestDef( void );
	virtual ~AnimBuildTestDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return FW_NEW AnimBuildTestDef( *this ); }
}; 


//------------------------------------------------------------------------------------------
//!
//!	AnimBuildTest
//!	A test controller for dynamically built animation headers
//!	
//------------------------------------------------------------------------------------------
class AnimBuildTest : public MovementController
{
public:

	// Construction destruction
	AnimBuildTest( CMovement* pobMovement, const AnimBuildTestDef& obDefinition );
	virtual ~AnimBuildTest( void );

	// The main beef, this moves the character about
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );


protected:

	// A COPY of our defintion
	AnimBuildTestDef m_obDefinition;

	// Pointer to the animation header that we will construct
	CAnimationHeader* m_popDynamicAnimationHeader;

	// A pointer to our full animation
	CAnimationPtr m_obDynamicAnimation;
};

#endif // _ANIMBUILDTEST_H
