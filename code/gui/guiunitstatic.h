/***************************************************************************************************
*
*	DESCRIPTION		The most basic type of derivable screen units - it has no user input/response
*
*	NOTES			
*
***************************************************************************************************/

#ifndef	_GUIUNITSTATIC_H
#define	_GUIUNITSTATIC_H

// Includes
#include "guiunit.h"

/***************************************************************************************************
*
*	CLASS			CGuiUnitStatic
*
*	DESCRIPTION		
*
***************************************************************************************************/

class CGuiUnitStatic : public CGuiUnit
{
public:

protected:

	//! Construction Destruction - derive if you want one
	CGuiUnitStatic( void );
	virtual ~CGuiUnitStatic( void );

	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessEnd( void );

};

#endif // _GUIUNITSTATIC_H
