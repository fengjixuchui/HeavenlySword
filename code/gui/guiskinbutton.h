/***************************************************************************************************
*
*	DESCRIPTION		A temporary skin of the basic user interface button unit
*
*	NOTES			Just for testing out the basic structural functionality of the GUI
*
***************************************************************************************************/

#ifndef _GUISKINBUTTON_H
#define _GUISKINBUTTON_H

// Includes
#include "guiunitbutton.h"

// Forward Declarations
class CString;

/***************************************************************************************************
*
*	CLASS			CGuiSkinTestButton
*
*	DESCRIPTION		Test skin for developing the abstract screen elements.
*
***************************************************************************************************/

class CGuiSkinTestButton : public CGuiUnitButton
{
public:

	// Construction Destruction
	CGuiSkinTestButton( void );
	virtual ~CGuiSkinTestButton( void );

protected:

	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessEnd( void );

	//! For setting up specific attributes

	CString*		m_pobString;
	const char*		m_pcStringTextID;


	void	UpdateFocusIn( void );
	void	UpdateFocusOut( void );
};

#endif // _GUISKINBUTTON_H
