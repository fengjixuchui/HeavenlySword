/***************************************************************************************************
*
*	DESCRIPTION		A temporary skin of the basic user interface static unit
*
*	NOTES			Just for testing out the basic structural functionality of the GUI
*
***************************************************************************************************/

#ifndef _E3GUISKINIMAGESTATIC_H
#define _E3GUISKINIMAGESTATIC_H

// Includes
#include "guiunitstatic.h"

#include "effect/screensprite.h"

/***************************************************************************************************
*
*	CLASS			CGuiE3SkinImageStatic
*
*	DESCRIPTION		Skinned GUI element for displaying static
*
***************************************************************************************************/

class CGuiE3SkinStatic : public CGuiUnitStatic
{
public:

	// Construction Destruction
	CGuiE3SkinStatic( void );
	virtual	~CGuiE3SkinStatic( void );

	bool			Render( void );

protected:

	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessEnd( void );

	const char*		m_pcStringText;

};

#endif // _E3GUISKINIMAGESTATIC_H
