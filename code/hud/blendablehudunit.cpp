/***************************************************************************************************
*
*	DESCRIPTION		The basic unit on which all other HUD items are based
*
*	NOTES			
*
***************************************************************************************************/

// Includes
#include "blendablehudunit.h"
#include "hudmanager.h"
#include "objectdatabase/dataobject.h"

// Interfaces

START_STD_INTERFACE( CBlendableHudUnitDef )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fBlendInTime, 0.0f, BlendInTime )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fBlendOutTime, 0.0f, BlendOutTime )
END_STD_INTERFACE

void ForceLinkFunctionBlendableHUDUnit()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionBlendableHUDUnit() !ATTN!\n");
}
/***************************************************************************************************
*
*	FUNCTION		CBlendableHudUnit::CBlendableHudUnit
*
*	DESCRIPTION		Construction/Destruction
*
***************************************************************************************************/

CBlendableHudUnitDef::CBlendableHudUnitDef( void )
{
}

CBlendableHudUnitDef::~CBlendableHudUnitDef( void )
{
}

/***************************************************************************************************
*
*	FUNCTION		CBlendableHudUnitDef::CreateInstance
*
*	DESCRIPTION		Desruction
*
***************************************************************************************************/
CHudUnit* CBlendableHudUnitDef::CreateInstance( void ) const
{
	// Should only be creating instances of derived classes
	// Cannot leave pure virtual as I want base parameters exposed to xml
	ntAssert( 0 );
	return 0;
}

/***************************************************************************************************
*
*	FUNCTION		CBlendableHudUnit::CBlendableHudUnit
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CBlendableHudUnit::CBlendableHudUnit( CBlendableHudUnitDef* pobDef )
:	m_pobBlendableDef ( pobDef )
,	m_fOverallAlpha ( 0.0f )
{
	m_eUnitState = STATE_INACTIVE;
}


/***************************************************************************************************
*
*	FUNCTION		CBlendableHudUnit::~CBlendableHudUnit
*
*	DESCRIPTION		Desruction
*
***************************************************************************************************/
CBlendableHudUnit::~CBlendableHudUnit( void )
{
}

/***************************************************************************************************
*
*	FUNCTION		CBlendableHudUnit::Render
*
*	DESCRIPTION		Returns true when OK.
*
***************************************************************************************************/

bool CBlendableHudUnit::Render( void )
{
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CBlendableHudUnit::Initialise
*
*	DESCRIPTION		Returns true when OK.
*
***************************************************************************************************/

bool CBlendableHudUnit::Initialise( void )
{
	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CBlendableHudUnit::BeginExit
*
*	DESCRIPTION		Kicks off the exit state for the unit.
*
***************************************************************************************************/
bool CBlendableHudUnit::BeginExit( bool bForce )
{	
    if ( CHudUnit::BeginExit( bForce ) )
	{
		m_obBlendTimer.Set( m_pobBlendableDef->m_fBlendOutTime );
		return true;
	}
	
	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CBlendableHudUnit::BeginEnter
*
*	DESCRIPTION		Kicks off the enter state for the unit.
*
***************************************************************************************************/
bool CBlendableHudUnit::BeginEnter( bool bForce )
{	
    if ( CHudUnit::BeginEnter( bForce ) )
	{
		m_obBlendTimer.Set( m_pobBlendableDef->m_fBlendInTime );
		return true;
	}
	
	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CBlendableHudUnit::UpdateEnter
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CBlendableHudUnit::UpdateEnter( float fTimestep )
{
	UNUSED ( fTimestep );

	m_obBlendTimer.Update();

	m_fOverallAlpha = 1.0f - CMaths::SmoothStep( m_obBlendTimer.NormalisedTime() );

	if ( m_obBlendTimer.Passed() )
	{
		m_fOverallAlpha = 1.0f;
		SetStateActive();
	}
};

/***************************************************************************************************
*
*	FUNCTION		CBlendableHudUnit::UpdateExit
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CBlendableHudUnit::UpdateExit( float fTimestep )
{
	UNUSED ( fTimestep );

	m_obBlendTimer.Update();

	m_fOverallAlpha = CMaths::SmoothStep( m_obBlendTimer.NormalisedTime() );

	if ( m_obBlendTimer.Passed() )
	{
		m_fOverallAlpha = 0.0f;
		SetStateInactive();
	}
};

/***************************************************************************************************
*
*	FUNCTION		CBlendableHudUnit::UpdateActive
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CBlendableHudUnit::UpdateActive( float fTimestep )
{
	UNUSED ( fTimestep );
}

/***************************************************************************************************
*
*	FUNCTION		CBlendableHudUnit::UpdateInactive
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CBlendableHudUnit::UpdateInactive( float fTimestep )
{
	UNUSED ( fTimestep );
}
