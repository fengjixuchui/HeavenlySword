/***************************************************************************************************
*
*	DESCRIPTION		A screen unit that represents a specific choice available to the user
*
*	NOTES			
*
***************************************************************************************************/

#ifndef	_GUIUNITBUTTON_H
#define	_GUIUNITBUTTON_H

// Includes
#include "guiunit.h"

/***************************************************************************************************
*
*	CLASS			CGuiUnitButton
*
*	DESCRIPTION		
*
***************************************************************************************************/

class CGuiUnitButton : public CGuiUnit
{
public:
	
protected:

	//! Construction Destruction - derive if you want one
	CGuiUnitButton( void );
	virtual ~CGuiUnitButton( void );

	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessEnd( void );
	


};

#endif // _GUIUNITBUTTON_H
