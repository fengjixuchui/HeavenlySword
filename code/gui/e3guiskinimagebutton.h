/***************************************************************************************************
*
*	DESCRIPTION		A skin of the basic user interface button unit
*
*	NOTES			Uses images as the background, for the time being will use debug text for the label
*
***************************************************************************************************/

#ifndef _E3GUISKINIMAGEBUTTON_H
#define _E3GUISKINIMAGEBUTTON_H

// Includes
#include "guiunitbutton.h"
#include "effect/screensprite.h"


// FIX ME TOM only using visualdebugger until 2D font ready
#include "core/visualdebugger.h"

// Forward Declarations


/***************************************************************************************************
*
*	CLASS			CGuiE3SkinImageButton
*
*	DESCRIPTION		Button with inactive and active images
*
***************************************************************************************************/

class CGuiE3SkinImageButton : public CGuiUnitButton
{
public:

	// Construction Destruction
	CGuiE3SkinImageButton( void );
	virtual ~CGuiE3SkinImageButton( void );

	virtual bool	SelectAction( int iPads );
	virtual bool	StartAction( int iPads );

	// Flags to modify button action
	// Public so CStringUtil can do its stuff
	enum
	{	
		MOVE_SCREEN_ON			= ( 1 << 0 ),
		MOVE_SCREEN_BACK		= ( 1 << 1 ),
		MOVE_SCREENTYPE_ON		= ( 1 << 2 ),
		MOVE_SCREENTYPE_BACK	= ( 1 << 3 ),
		RESUME_GAME				= ( 1 << 4 ),
		RELOAD_LEVEL			= ( 1 << 5 ),
		LOAD_LEVEL				= ( 1 << 6 ),
	};

protected:

	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	bool			ProcessAction( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessEnd( void );

	//! For setting up specific attributes

	void	SetStateEnter( void );
	void	SetStateExit( void );
	void	SetStateFocus( void );
	void	SetStateFocusIn( void );
	void	SetStateFocusOut( void );

	void	UpdateEnter( void );
	void	UpdateExit( void );
	void	UpdateFocus( void );
	void	UpdateFocusIn( void );
	void	UpdateFocusOut( void );
	
	bool	Render( void );	

	int				m_iActionFlags;
	const char*		m_pcActionParam;
	int				m_iActionParam;
	
	// Temporary label for the button
	const char*		m_pcStringText;

	float			m_fHeight;
	float			m_fWidth;

	const char*		m_pcImageName;
	const char*		m_pcImageOffName;
	const char*		m_pcBackgroundName;

	ScreenSprite	m_obImage;
	ScreenSprite	m_obImageOff;
	ScreenSprite	m_obBackground;

	float			m_fImageAlpha;
	float			m_fImageOffAlpha;
	float			m_fBackgroundAlpha;

	CGuiTimer		m_obFadeTime;
	bool			m_bFadeIn;
};

#endif // _GUISKINBUTTON_H
