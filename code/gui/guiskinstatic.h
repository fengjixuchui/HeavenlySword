/***************************************************************************************************
*
*	DESCRIPTION		A temporary skin of the basic user interface static unit
*
*	NOTES			Just for testing out the basic structural functionality of the GUI
*
***************************************************************************************************/

#ifndef _GUISKINSTATIC_H
#define _GUISKINSTATIC_H

// Includes
#include "guiunitstatic.h"
#include "guiutil.h"

// Forward Declarations
class CString;

/***************************************************************************************************
*
*	CLASS			CGuiSkinTestStatic
*
*	DESCRIPTION		Test skin for developing the functionality of the abstract screen elements.
*
***************************************************************************************************/

class CGuiSkinTestStatic : public CGuiUnitStatic
{
public:

	// Construction Destruction
	CGuiSkinTestStatic( void );
	CGuiSkinTestStatic( const char* pcStringID, const char* pcFont, CPoint& obPos, float fDuration = 0.0f );
	virtual ~CGuiSkinTestStatic( void );

protected:

	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessEnd( void );

	bool	Render( void );

	virtual void SetStateIdle( void );
	virtual void UpdateIdle( void );

	//! For setting up specific attributes

	CString*		m_pobString;
	const char*		m_pcStringTextID;
	const char*		m_pcFontName;

	float m_fDuration;
	CGuiTimer m_obTimer;
};

#endif // _GUISKINSTATIC_H
