/***************************************************************************************************
*
*	DESCRIPTION		The most basic type of derivable screen units - it has no user input/response
*
*	NOTES			
*
***************************************************************************************************/

// Includes
#include "guiunitstatic.h"

/***************************************************************************************************
*
*	FUNCTION		CGuiUnitStatic::CGuiUnitStatic
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CGuiUnitStatic::CGuiUnitStatic( void )
{
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnitStatic::~CGuiUnitStatic
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CGuiUnitStatic::~CGuiUnitStatic( void )
{
}


/***************************************************************************************************
*
*	FUNCTION		CGuiUnitStatic::ProcessAttribute
*
*	DESCRIPTION		Passes attribute values on to specific handlers based on their title.
*
***************************************************************************************************/

bool CGuiUnitStatic::ProcessAttribute( const char* pcTitle, const char* pcValue )
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
*	FUNCTION		CGuiUnitStatic::ProcessEnd
*
*	DESCRIPTION		This is called when the closing tag on an XML element is parsed.  Hence at this
*					stage we know we have all the information that will be set by a script.  This
*					function is used to set up any information that requireds two or more attributes
*					to avoid any attribute ordering issues.
*
***************************************************************************************************/

bool CGuiUnitStatic::ProcessEnd( void )
{
	// Call the base class first
	CGuiUnit::ProcessEnd();

	return true;
}
