
//------------------------------------------------------------------------------------------
//!
//!	\file movementcontrollerinterface.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "game/movementcontrollerinterface.h"
#include "game/movement.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "game/entityinfo.h"
#include "objectdatabase/dataobject.h"
#include "physics/advancedcharactercontroller.h"
#include "physics/system.h"

// Make some space and initialise this
const float MovementControllerUtilities::m_fGravity = -9.81f;

/***************************************************************************************************
*	
*	FUNCTION		MovementControllerUtilities::RotationAboutX
*
*	DESCRIPTION		Static helper class for the movement code
*
***************************************************************************************************/
float MovementControllerUtilities::RotationAboutX( const CDirection& obRotateFrom, const CDirection& obRotateTo )
{
	// Remove any X translation on the two input vectors
	CDirection obTempRotateFrom( 0.0f, obRotateFrom.Y(), obRotateFrom.Z() );
	CDirection obTempRotateTo( 0.0f, obRotateTo.Y(), obRotateTo.Z() );

	// Make sure that the two directions are normalised
	obTempRotateFrom.Normalise();
	obTempRotateTo.Normalise();

	// Build a quat from information - and get the axis and angle
	float fAngle;
	CDirection obAxis;
	CQuat obQuat( obTempRotateFrom, obTempRotateTo );
	obQuat.GetAxisAndAngle( obAxis, fAngle );

	// If the returned axis is negative, flip the angle
	if ( obAxis.X() < 0.0f )
		fAngle = -fAngle;

	// Give them the angle
	return fAngle;
}


/***************************************************************************************************
*	
*	FUNCTION		MovementControllerUtilities::RotationAboutY
*
*	DESCRIPTION		Static helper class for the movement code
*
***************************************************************************************************/
float MovementControllerUtilities::RotationAboutY( const CDirection& obRotateFrom, const CDirection& obRotateTo )
{
	// Remove any Y translation on the two input vectors
	CDirection obTempRotateFrom( obRotateFrom.X(), 0.0f, obRotateFrom.Z() );
	CDirection obTempRotateTo( obRotateTo.X(), 0.0f, obRotateTo.Z() );

	// Make sure that the two directions are normalised
	obTempRotateFrom.Normalise();
	obTempRotateTo.Normalise();

	// Build a quat from information - and get the axis and angle
	float fAngle;
	CDirection obAxis;
	CQuat obQuat( obTempRotateFrom, obTempRotateTo );
	obQuat.GetAxisAndAngle( obAxis, fAngle );

	// If the returned axis is negative, flip the angle
	if ( obAxis.Y() < 0.0f )
		fAngle = -fAngle;

	// Give them the angle
	return fAngle;
}


/***************************************************************************************************
*	
*	FUNCTION		MovementControllerUtilities::RotationAboutZ
*
*	DESCRIPTION		Static helper class for the movement code
*
***************************************************************************************************/
float MovementControllerUtilities::RotationAboutZ( const CDirection& obRotateFrom, const CDirection& obRotateTo )
{
	// Remove any Y translation on the two input vectors
	CDirection obTempRotateFrom( obRotateFrom.X(), obRotateFrom.Y(), 0.0f );
	CDirection obTempRotateTo( obRotateTo.X(), obRotateTo.Y(), 0.0f );

	// Make sure that the two directions are normalised
	obTempRotateFrom.Normalise();
	obTempRotateTo.Normalise();

	// Build a quat from information - and get the axis and angle
	float fAngle;
	CDirection obAxis;
	CQuat obQuat( obTempRotateFrom, obTempRotateTo );
	obQuat.GetAxisAndAngle( obAxis, fAngle );

	// If the returned axis is negative, flip the angle
	if ( obAxis.Z() < 0.0f )
		fAngle = -fAngle;

	// Give them the angle
	return fAngle;
}


/***************************************************************************************************
*
*	FUNCTION		MovementControllerUtilities::GetTurnSpeedToFace
*
*	DESCRIPTION		calculate the required turn speed to face in this direction	
*
***************************************************************************************************/
float MovementControllerUtilities::GetTurnSpeedToFace( const CDirection& obCurr, const CDirection& obDest, float fTimeStep )
{
	CQuat obRotationDelta( obCurr, obDest );
	
	CDirection obAxis; float fAngle;
	obRotationDelta.GetAxisAndAngle( obAxis, fAngle );

	// Need to check for 180 changes, in this case the axis can be the local X not Y
	if( obAxis.Y() < 0.0f )
		fAngle = -fAngle;

	return ( fAngle / fTimeStep );
}


/***************************************************************************************************
*
*	FUNCTION		MovementControllerUtilities::GetAnimationDirection
*
*	DESCRIPTION		Describes an animation direction that is required based on a rotation about
*					the Y axis.
*
***************************************************************************************************/
ANIM_DIRECTION MovementControllerUtilities::GetAnimationDirection( float fRotationAboutY )
{
	// Set up our return value
	ANIM_DIRECTION eDirection = AD_FRONT;

	// For the right hand side
	if ( fRotationAboutY < 0.0f )
	{
		if ( fRotationAboutY < ( -3.0f * PI / 4.0f ) )	
			eDirection = AD_BACKRIGHT;

		else if ( fRotationAboutY < -PI / 4.0f )		
			eDirection = AD_RIGHT;

		else											
			eDirection = AD_FRONT;
	}

	// For the left hand side
	else
	{
		if ( fRotationAboutY > ( 3.0f * PI / 4.0f ) )	
			eDirection = AD_BACK_LEFT;

		else if ( fRotationAboutY > PI / 4.0f )			
			eDirection = AD_LEFT;

		else											
			eDirection = AD_FRONT;
	}

	// Give it to them
	return eDirection;
}


/***************************************************************************************************
*
*	FUNCTION		MovementControllerUtilities::GetYRotation
*
*	DESCRIPTION		This gets a Y rotation from a Quat - to be used on Quats you know only to rotate 
*					about Y.  Hokey.
*
***************************************************************************************************/
float MovementControllerUtilities::GetYRotation( const CQuat& obRotation )
{
	// Set up a return value
	float fRotation = 0.0f;

	// Get the axis and angle from the quat
	CDirection obAxis( CONSTRUCT_CLEAR );
	obRotation.GetAxisAndAngle( obAxis, fRotation );

	// Flip the rotation angle if there is a negative Y element
	if ( obAxis.Y() < 0.0f ) 
		fRotation *= -1.0f;

	// Give it to them
	return fRotation;
}

// All the functionality below for MovementControllerDef is debug 
// functionality only for the movement system
#ifndef _RELEASE

/***************************************************************************************************
*	
*	FUNCTION		MovementControllerDef::MovementControllerDef
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/
MovementControllerDef::MovementControllerDef( void )
:	m_pcInstanceName( 0 ),
	m_pcTypeName( 0 )
{
}


/***************************************************************************************************
*	
*	FUNCTION		MovementControllerDef::~MovementControllerDef
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/
MovementControllerDef::~MovementControllerDef( void )
{
	// Make sure that the debug strings are cleaned up - may be NULL
	NT_DELETE( m_pcInstanceName );
	NT_DELETE( m_pcTypeName );
}


/***************************************************************************************************
*	
*	FUNCTION		MovementControllerDef::MovementControllerDef
*
*	DESCRIPTION		Copy constructor
*
***************************************************************************************************/
MovementControllerDef::MovementControllerDef( const MovementControllerDef& obRef )
:	m_pcInstanceName( 0 ),
	m_pcTypeName( 0 )
{
	// If the other guy has an instance name then we need to copy it
	if ( obRef.m_pcInstanceName )
	{
		// Take a copy of the name
		char* pcTempName = NT_NEW char[ strlen( obRef.m_pcInstanceName ) + 1 ];
		strcpy( pcTempName, obRef.m_pcInstanceName );
		m_pcInstanceName = pcTempName;
	}

	// If the other guy has a type name then we need to copy it
	if ( obRef.m_pcTypeName )
	{
		// Take a copy of the name
		char* pcTempName = NT_NEW char[ strlen( obRef.m_pcTypeName ) + 1 ];
		strcpy( pcTempName, obRef.m_pcTypeName );
		m_pcTypeName = pcTempName;
	}
}


/***************************************************************************************************
*	
*	FUNCTION		MovementControllerDef::operator=
*
*	DESCRIPTION		Assignment
*
***************************************************************************************************/
const MovementControllerDef& MovementControllerDef::operator=( const MovementControllerDef& obRef )
{
	// If we have an instance name we need to clean it up
	if ( m_pcInstanceName )
	{
		NT_DELETE( m_pcInstanceName );
		m_pcInstanceName = 0;
	}

	// If the other guy has an instance name then we need to copy it
	if ( obRef.m_pcInstanceName )
	{
		// Take a copy of the name
		char* pcTempName = NT_NEW char[ strlen( obRef.m_pcInstanceName ) + 1 ];
		strcpy( pcTempName, obRef.m_pcInstanceName );
		m_pcInstanceName = pcTempName;
	}

	// If we have an type name we need to clean it up
	if ( m_pcTypeName )
	{
		NT_DELETE( m_pcTypeName );
		m_pcTypeName = 0;
	}

		// If the other guy has a type name then we need to copy it
	if ( obRef.m_pcTypeName )
	{
		// Take a copy of the name
		char* pcTempName = NT_NEW char[ strlen( obRef.m_pcTypeName ) + 1 ];
		strcpy( pcTempName, obRef.m_pcTypeName );
		m_pcTypeName = pcTempName;
	}

	// Pass back a reference to us
	return *this;
}


/***************************************************************************************************
*	
*	FUNCTION		MovementControllerDef::PostConstruct
*
*	DESCRIPTION		To be called after serialisation - not called on items that are created 'on the
*					fly'.
*
***************************************************************************************************/
void MovementControllerDef::PostConstruct( void )
{
	// Make sure we can get the data we want
	if ( !ntStr::IsNull(ObjectDatabase::Get().GetNameFromPointer( this )) )
	{
		// Take a copy of the name from the data file
		char* pcTempInstanceName = NT_NEW char[ strlen( ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer( this ) )) + 1 ];
		strcpy( pcTempInstanceName, ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer( this )) );
		m_pcInstanceName = pcTempInstanceName;
	}

	// Make sure we can get the data we want
	if ( !ntStr::IsNull(ObjectDatabase::Get().GetInterfaceNameFromPointer( this )) )
	{
		// Set up the type name from the interface
		char* pcTempTypeName = NT_NEW char[ strlen( ntStr::GetString(ObjectDatabase::Get().GetInterfaceNameFromPointer( this ) )) + 1 ];
		strcpy( pcTempTypeName, ntStr::GetString(ObjectDatabase::Get().GetInterfaceNameFromPointer( this )) );
		m_pcTypeName = pcTempTypeName;
	}
}


/***************************************************************************************************
*	
*	FUNCTION		MovementControllerDef::SetDebugNames
*
*	DESCRIPTION		To be called on definitions created on the fly.  Takes an instance name and 
*					finds the type name itself.
*
***************************************************************************************************/
void MovementControllerDef::SetDebugNames( const char* pcInstanceName, const char* pcTypeName ) const
{
	// If we have a name we need to clean it up
	if ( m_pcInstanceName )
	{
		NT_DELETE( m_pcInstanceName );
		m_pcInstanceName = 0;
	}

	// Take a copy of the name 
	char* pcTempInstanceName = NT_NEW char[ strlen( pcInstanceName ) + 1 ];
	strcpy( pcTempInstanceName, pcInstanceName );
	m_pcInstanceName = pcTempInstanceName;

	// If we have been given a type name use it
	if ( pcTypeName )
	{
		// Clear stuff up if we have a name already
		if ( m_pcTypeName )
		{
			NT_DELETE( m_pcTypeName );
			m_pcTypeName = 0;
		}

		// Take a copy of the name 
		char* pcTempTypeName = NT_NEW char[ strlen( pcTypeName ) + 1 ];
		strcpy( pcTempTypeName, pcTypeName );
		m_pcTypeName = pcTempTypeName;
	}

	// Set up the type name from the interface if we haven't already - give it a shot
	if ( ( !m_pcTypeName ) && ( ObjectDatabase::Get().GetInterfaceNameFromPointer( this ) ) )
	{
		char* pcTempTypeName = NT_NEW char[ strlen( ObjectDatabase::Get().GetInterfaceNameFromPointer( this ) ) + 1 ];
		strcpy( pcTempTypeName, ObjectDatabase::Get().GetInterfaceNameFromPointer( this ) );
		m_pcTypeName = pcTempTypeName;
	}
}

#endif


/***************************************************************************************************
*	
*	FUNCTION		MovementController::MovementController
*
*	DESCRIPTION		
*
***************************************************************************************************/
MovementController::MovementController( CMovement* pobMovement )
:	m_pobMovement( pobMovement ), 
	m_pobAnimator( pobMovement->GetAnimatorP() ),
	m_bFirstFrame( true ),
	m_fBlendWeight( 1.0f )
{

#ifndef _RELEASE
	m_pobDefinition = 0;
#endif

}


/***************************************************************************************************
*	
*	FUNCTION		MovementController::~MovementController
*
*	DESCRIPTION		
*
***************************************************************************************************/
MovementController::~MovementController( void ) 
{
}



/***************************************************************************************************
*
*	FUNCTION		MovementController::ApplyGravity
*
*	DESCRIPTION		A helper function that can be used to apply procedural gravity to anything that 
*					wants it
*
***************************************************************************************************/
void MovementController::ApplyGravity(	bool bApplyGravity )
{
	// Gravity setting
	Physics::AdvancedCharacterController* pobCharacter = 0;
	if ( m_pobMovement->GetPhysicsSystem() )
		pobCharacter = (Physics::AdvancedCharacterController*) m_pobMovement->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
	if( pobCharacter )
		pobCharacter->SetApplyCharacterControllerGravity(bApplyGravity);
}


