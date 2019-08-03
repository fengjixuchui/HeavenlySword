/***************************************************************************************************
*
*	DESCRIPTION		A unit that allows a user to pick a game value from the available range
*
*	NOTES			
*
***************************************************************************************************/

// Includes
#include "guiunittoggle.h"

/***************************************************************************************************
*
*	FUNCTION		CGuiUnitToggle::CGuiUnitToggle
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CGuiUnitToggle::CGuiUnitToggle( void )
{
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnitToggle::~CGuiUnitToggle
*
*	DESCRIPTION		Destruction
*	
***************************************************************************************************/

CGuiUnitToggle::~CGuiUnitToggle( void )
{
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnitToggle::ProcessAttribute
*
*	DESCRIPTION		Passes attribute values on to specific handlers based on their title.
*
***************************************************************************************************/

bool CGuiUnitToggle::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( !CGuiUnit::ProcessAttribute( pcTitle, pcValue ) )
	{
		/*
		if ( strcmp( pcTitle, "something" ) == 0 )
		{
			return SetSomethingValue( pcValue );
		}

		else if ( strcmp( pcTitle, "somethingelse" ) == 0 )
		{
			return SetSomethingElseValue( pcValue );
		}
		*/

		return false;
	}

	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnitToggle::ProcessEnd
*
*	DESCRIPTION		This is called when the closing tag on an XML element is parsed.  Hence at this
*					stage we know we have all the information that will be set by a script.  This
*					function is used to set up any information that requireds two or more attributes
*					to avoid any attribute ordering issues.
*
***************************************************************************************************/

bool CGuiUnitToggle::ProcessEnd( void )
{
	// Call the base class first
	CGuiUnit::ProcessEnd();

	return true;
}
