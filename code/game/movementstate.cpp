/***************************************************************************************************
*
*       DESCRIPTION	
*
*       NOTES          
*
***************************************************************************************************/

// Necessary includes
#include "movementstate.h"

/***************************************************************************************************
*
*	FUNCTION		CMovementState::CMovementState
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CMovementState::CMovementState( void )
:	m_obProceduralRootDelta( CONSTRUCT_CLEAR ),				
	m_fProceduralYaw( 0.0f ),
	m_fProceduralPitch( 0.0f ),
	m_fProceduralRoll( 0.0f ),
	m_obRootDeltaScalar( 1.0f, 1.0f, 1.0f  ),						
	m_fRootRotationDeltaScalar( 1.0f ),
	m_bApplyExplicitRotations( false )
{
}


/***************************************************************************************************
*
*	FUNCTION		CMovementState::operator=
*
*	DESCRIPTION		Assignment
*
***************************************************************************************************/

CMovementState& CMovementState::operator = ( const CMovementState& obState )
{
	m_obProceduralRootDelta = obState.m_obProceduralRootDelta;
	m_fProceduralYaw = obState.m_fProceduralYaw;
	m_fProceduralPitch = obState.m_fProceduralPitch;
	m_fProceduralRoll = obState.m_fProceduralRoll;
	m_obRootDeltaScalar = obState.m_obRootDeltaScalar;
	m_fRootRotationDeltaScalar = obState.m_fRootRotationDeltaScalar;
	m_bApplyExplicitRotations = obState.m_bApplyExplicitRotations;

	return *this;
}


/***************************************************************************************************
*
*	FUNCTION		CMovementStateRef::CMovementStateRef
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CMovementStateRef::CMovementStateRef( void )
:	m_obLastRequestedVelocity( CONSTRUCT_CLEAR ),
	m_obLastRequestedAcceleration( CONSTRUCT_CLEAR ),
	m_obRootMatrix( CONSTRUCT_IDENTITY ),								
	m_obFacing( CVecMath::GetZAxis() ),									
	m_obOrientation( CONSTRUCT_IDENTITY ),							
	m_obPosition( CONSTRUCT_CLEAR ),																		
	m_bOnGround( false )
{
}


/***************************************************************************************************
*
*	FUNCTION		CMovementStateRef::operator=
*
*	DESCRIPTION		Assignment
*
***************************************************************************************************/

CMovementStateRef& CMovementStateRef::operator = ( const CMovementStateRef& obState )
{
	m_obLastRequestedVelocity = obState.m_obLastRequestedVelocity;
	m_obLastRequestedAcceleration = obState.m_obLastRequestedAcceleration;
	m_obRootMatrix = obState.m_obRootMatrix;
	m_obFacing = obState.m_obFacing;
	m_obOrientation = obState.m_obOrientation;
	m_obPosition = obState.m_obPosition;
	m_bOnGround = obState.m_bOnGround;

	return *this;
}


