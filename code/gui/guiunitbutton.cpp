/***************************************************************************************************
*
*	DESCRIPTION		A screen unit that represents a specific choice available to the user
*
*	NOTES			
*
***************************************************************************************************/

// Includes
#include "guiunitbutton.h"

/***************************************************************************************************
*
*	FUNCTION		CGuiUnitButton::CGuiUnitButton
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CGuiUnitButton::CGuiUnitButton( void )
{
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnitButton::~CGuiUnitButton
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CGuiUnitButton::~CGuiUnitButton( void )
{
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnitButton::ProcessAttribute
*
*	DESCRIPTION		Passes attribute values on to specific handlers based on their title.
*
***************************************************************************************************/

bool CGuiUnitButton::ProcessAttribute( const char* pcTitle, const char* pcValue )
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
*	FUNCTION		CGuiUnitButton::ProcessEnd
*
*	DESCRIPTION		This is called when the closing tag on an XML element is parsed.  Hence at this
*					stage we know we have all the information that will be set by a script.  This
*					function is used to set up any information that requireds two or more attributes
*					to avoid any attribute ordering issues.
*
***************************************************************************************************/

bool CGuiUnitButton::ProcessEnd( void )
{
	// Call the base class first
	CGuiUnit::ProcessEnd();

	return true;
}

