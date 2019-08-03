//------------------------------------------------------------------------------------------
//!
//!	\file movementcontrollerinterface.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_MOVEMENTCONTROLLERINTERFACE_H
#define	_MOVEMENTCONTROLLERINTERFACE_H

#include "editable/enumlist.h"
#include "anim/animation.h"

class CAnimator;
class CMovement;
class CMovementInput;
class CMovementState;
class CMovementStateRef;
class MovementController;

/***************************************************************************************************
*	
*	CLASS			MovementControllerUtilities
*
*	DESCRIPTION		Static helper class for the movement code
*
***************************************************************************************************/

class MovementControllerUtilities
{
public:

	// Find the rotation between two arbitrary directions about a particular world axis
	static float	RotationAboutX( const CDirection& obRotateFrom, const CDirection& obRotateTo );
	static float	RotationAboutY( const CDirection& obRotateFrom, const CDirection& obRotateTo );
	static float	RotationAboutZ( const CDirection& obRotateFrom, const CDirection& obRotateTo );

	// This will give an angular velocity required to make the turn
	static float	GetTurnSpeedToFace( const CDirection& obCurr, const CDirection& obDest, float fTimeStep );

	// Based on a rotation about Y this will give you an animation direction best to use
	static ANIM_DIRECTION	GetAnimationDirection( float fRotationAboutY );

	// This gets a Y rotation from a Quat - to be used on Quats you know only to rotate about Y
	static float			GetYRotation( const CQuat& obRotation );

	// Useful constants
	static const float m_fGravity;
};


//------------------------------------------------------------------------------------------
//!
//!	MovementControllerDef
//!	A simple abstract class to define a simple 'factory' interface to movement controller
//! definitions.  Allows movement controllers to be generated directly from XML definitions.
//!
//------------------------------------------------------------------------------------------
class MovementControllerDef
{
public:

	// A pure virtual call to be overridden by specific movement controller defintions
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const = 0;

	// We must be able to copy movement Defs - so we can have chained controllers etc
	virtual MovementControllerDef* Clone( void ) const = 0;

	// All the details below provide debug functionality for the movement system
		
#ifndef _RELEASE

	// Construction destruction
	MovementControllerDef( void );
	virtual ~MovementControllerDef( void );

	// Copy constructor
	MovementControllerDef( const MovementControllerDef& obRef );

	// Assignment
	const MovementControllerDef& operator=( const MovementControllerDef& obRef );

	// To be called after serialisation
	void PostConstruct( void );

	// Set up the names of this definition for debug purposes - will set the type name
	void SetDebugNames( const char* pcInstanceName, const char* pcTypeName = 0 ) const;

	// Get the name of this controller for debug purposes - may return NULL
	const char* GetInstanceName( void ) const { return m_pcInstanceName; }

	// The name of the type of this controller for debug purposes - may return NULL
	const char* GetTypeName( void ) const { return m_pcTypeName; }

protected:

	// For debugging purposes we save the instance and type name
	mutable const char* m_pcInstanceName;
	mutable const char* m_pcTypeName;

#else

	virtual ~MovementControllerDef( void ) {};

	// Stub functions for release
	void SetDebugNames( const char*, const char* ) const {}

#endif

};


/***************************************************************************************************
*	
*	CLASS			MovementController
*
*	DESCRIPTION		The base instance of a movement controller
*
***************************************************************************************************/

class MovementController
{
public:

	// Destruction
	virtual ~MovementController( void );

	// The major functionality of any movement constroller
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState ) = 0;

	// Set animation speed - needed when you need more control over blending
	void SetTotalWeight( float fBlendWeight ) { m_fBlendWeight = fBlendWeight; }

	// The below is debug functionality for the movement system
#ifndef _RELEASE

	// Get the instance name from the def
	const char* GetInstanceName( void ) const 
	{	if ( m_pobDefinition ) 
			return m_pobDefinition->GetInstanceName();
		else
			return 0; }

	// Get the type name of the def
	const char* GetTypeName( void ) const
	{	if ( m_pobDefinition ) 
			return m_pobDefinition->GetTypeName();
		else
			return 0; }
	
	// Register the a definition here so we can have a single point of access for debug
	void InternalRegisterDefinition( const MovementControllerDef& obDefinition ) 
	{	m_pobDefinition = &obDefinition; 
		if ( !GetTypeName() && !GetInstanceName() )
			m_pobDefinition->SetDebugNames( "UnNamed" ); }

	// Our pointer to a definition for debug output
	const MovementControllerDef* m_pobDefinition;

#else

	// Stubs for release versions
	void InternalRegisterDefinition( const MovementControllerDef& ) {}

#endif

protected:

	// Construction
	MovementController( CMovement* pobMovement );

	void ApplyGravity(bool bGravity);

	// A pointer to the last controller we came from so we can clean it up
	CMovement*		m_pobMovement;
	CAnimator*		m_pobAnimator;

	// We need to know whether we are updating on the first frame or not
	bool			m_bFirstFrame;

	// Deal with the wieght of this controller, i would like to make the wieght of a controller
	// invisible to it at some point, blending should be dealt with outside the controller
	float			m_fBlendWeight;
};


#endif //_MOVEMENTCONTROLLERINTERFACE_H
