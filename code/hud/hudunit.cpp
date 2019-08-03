/***************************************************************************************************
*
*	DESCRIPTION		The basic unit on which all other HUD items are based
*
*	NOTES			
*
***************************************************************************************************/

// Includes
#include "hudunit.h"
#include "hudmanager.h"

// Interfaces

void ForceLinkFunctionHUDUnit()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionHUDUnit() !ATTN!\n");
}
/***************************************************************************************************
*
*	FUNCTION		CHudUnit::CHudUnit
*
*	DESCRIPTION		Construction/Destruction
*
***************************************************************************************************/

CHudUnitDef::CHudUnitDef( void )
{
}

CHudUnitDef::~CHudUnitDef( void )
{
}

/***************************************************************************************************
*
*	FUNCTION		CHudUnit::CHudUnit
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CHudUnit::CHudUnit( void )
:	m_eUnitState ( STATE_ACTIVE )
,	m_bRemoveOnExit ( false )
{
}


/***************************************************************************************************
*
*	FUNCTION		CHudUnit::~CHudUnit
*
*	DESCRIPTION		Desruction
*
***************************************************************************************************/

CHudUnit::~CHudUnit( void )
{
}


/***************************************************************************************************
*
*	FUNCTION		CHudUnit::Update
*
*	DESCRIPTION		Returns true whilst the unit is still active.
*
***************************************************************************************************/

bool CHudUnit::Update( float fTimestep )
{
	// Now do the state based update - this should probably be scripted
	switch ( m_eUnitState )
	{
	case STATE_ENTER:		UpdateEnter( fTimestep );		break;
	case STATE_INACTIVE:	UpdateInactive( fTimestep );	break;
	case STATE_ACTIVE:		UpdateActive( fTimestep );		break;
	case STATE_EXIT:		UpdateExit( fTimestep );		break;
	default:				ntAssert( 0 );					break;
	}

	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CHudUnit::Render
*
*	DESCRIPTION		Returns true when OK.
*
***************************************************************************************************/

bool CHudUnit::Render( void )
{
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CHudUnit::Initialise
*
*	DESCRIPTION		Returns true when OK.
*
***************************************************************************************************/

bool CHudUnit::Initialise( void )
{
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CHudUnit::SetStateEnter
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CHudUnit::SetStateEnter( void )
{
	// Update the flag
	m_eUnitState = STATE_ENTER;
}


/***************************************************************************************************
*
*	FUNCTION		CHudUnit::SetStateExit
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CHudUnit::SetStateExit( void )
{	
	// Set the state flag
	m_eUnitState = STATE_EXIT;
}


/***************************************************************************************************
*
*	FUNCTION		CHudUnit::SetStateActive
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CHudUnit::SetStateActive( void )
{
	// Set the state flag
	m_eUnitState = STATE_ACTIVE;
}

/***************************************************************************************************
*
*	FUNCTION		CHudUnit::SetStateInactive
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CHudUnit::SetStateInactive( void )
{
	// Set the state flag
	m_eUnitState = STATE_INACTIVE;
}

/***************************************************************************************************
*
*	FUNCTION		CHudUnit::BeginExit
*
*	DESCRIPTION		Kicks off the exit state for the unit.
*
***************************************************************************************************/
bool CHudUnit::BeginExit( bool bForce )
{	
	// Don't respond unless we are in a steady state
    if ( ( !bForce ) && ( ( m_eUnitState == STATE_ENTER ) || ( m_eUnitState == STATE_EXIT ) ) ) return false;

	// Go straight to the exit state
	SetStateExit();

	// We have successfully changed state
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CHudUnit::BeginEnter
*
*	DESCRIPTION		Kicks off the enter state for the unit.
*
***************************************************************************************************/
bool CHudUnit::BeginEnter( bool bForce )
{	
	// Don't respond unless we are in a steady state
    if ( ( !bForce ) && ( ( m_eUnitState == STATE_ENTER ) || ( m_eUnitState == STATE_EXIT ) ) ) return false;

	// Go straight to the exit state
	SetStateEnter();

	// We have successfully changed state
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CHudUnit::UpdateEnter
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CHudUnit::UpdateEnter( float fTimestep )
{
	UNUSED ( fTimestep );
}

/***************************************************************************************************
*
*	FUNCTION		CHudUnit::UpdateExit
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CHudUnit::UpdateExit( float fTimestep )
{
	UNUSED ( fTimestep );
}

/***************************************************************************************************
*
*	FUNCTION		CHudUnit::UpdateActive
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CHudUnit::UpdateActive( float fTimestep )
{
	UNUSED ( fTimestep );
}

/***************************************************************************************************
*
*	FUNCTION		CHudUnit::UpdateInactive
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CHudUnit::UpdateInactive( float fTimestep )
{
	UNUSED ( fTimestep );
}
