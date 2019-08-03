/***************************************************************************************************
*
*	DESCRIPTION		A unit that allows a user to pick a game value from the available range
*
*	NOTES			
*
***************************************************************************************************/

#ifndef	_GUIUNITTOGGLE_H
#define	_GUIUNITTOGGLE_H

// Includes
#include "guiunit.h"

/***************************************************************************************************
*
*	CLASS			CGuiUnitToggle
*
*	DESCRIPTION		
*
***************************************************************************************************/

class CGuiUnitToggle : public CGuiUnit
{
public:

protected:

	//! Construction Destruction - derive if you want one
	CGuiUnitToggle( void );
	virtual ~CGuiUnitToggle( void );

	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessEnd( void );

};

#endif // _GUIUNITTOGGLE_H
