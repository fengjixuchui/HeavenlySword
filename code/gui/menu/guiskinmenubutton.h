/***************************************************************************************************
*
*	DESCRIPTION		
*
*	NOTES			
*
***************************************************************************************************/

#ifndef _GUISKINMENUBUTTON_H
#define _GUISKINMENUBUTTON_H

// Includes
#include "gui/menu/guiskinmenutext.h"
#include "gui/guiaction.h"
#include "effect/screensprite.h"

// Forward Declarations

/***************************************************************************************************
*
*	CLASS			CGuiSkinMenuButton
*
*	DESCRIPTION		Button with inactive and active states
*
***************************************************************************************************/

class CGuiSkinMenuButton : public CGuiSkinMenuText
{
	typedef CGuiSkinMenuText super;
public:

	// Construction Destruction
	CGuiSkinMenuButton( void );
	virtual ~CGuiSkinMenuButton( void );

	// Called after ProcessEnd to perform additional processing after all
	// screen elements have been fully created.
	virtual void	PostProcessEnd( void );

protected:

	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessEnd( void );

	// system callbacks
	virtual bool	SelectAction( int iPads );
	virtual bool	StartAction( int iPads );
	virtual bool	Render();

	// us
	virtual void	ActivationChange() {}

	CGuiAction m_obAction;
};

/***************************************************************************************************
*
*	CLASS			CGuiSkinMenuBGButton
*
*	DESCRIPTION		Subclass which allows a background image
*
***************************************************************************************************/

class CGuiSkinMenuBGButton : public CGuiSkinMenuButton
{
	typedef CGuiSkinMenuButton super;
public:

	// Construction Destruction
	CGuiSkinMenuBGButton( void );
	virtual ~CGuiSkinMenuBGButton( void );

protected:
	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessEnd( void );

	// system callbacks
	virtual bool	Render();
    
	bool m_bHasTexture;
	ScreenSprite m_obBGImage;

	float m_fXPad;
	float m_fYPad;
};

#endif // _GUISKINMENUBUTTON_H
