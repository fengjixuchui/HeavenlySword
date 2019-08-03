/***************************************************************************************************
*
*	DESCRIPTION		Animated widget to show that the game is still loading/reloading
*
*	NOTES			
*
***************************************************************************************************/

#ifndef _E3GUISKINLOADPROGRESS_H
#define _E3GUISKINLOADPROGRESS_H

// Includes
#include "guiunitstatic.h"

#include "effect/screensprite.h"

/***************************************************************************************************
*
*	CLASS			CGuiE3SkinLoadProgress
*
*	DESCRIPTION		Skinned GUI element for indicating load in progress
*
***************************************************************************************************/

class CGuiE3SkinLoadProgress : public CGuiUnitStatic
{
public:

	// Construction Destruction
	CGuiE3SkinLoadProgress( void );
	virtual ~CGuiE3SkinLoadProgress( void );

protected:

	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessEnd( void );

	bool	Render( void );
	bool	Update( void );

	float	m_fHeight;
	float	m_fWidth;

	const char*		m_pcImageName;

	ScreenSprite m_obImage;
};

#endif // _E3GUISKINLOADPROGRESS_H
