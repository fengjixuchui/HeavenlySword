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
*	DESCRIPTION		Skinned GUI element for displaying image static
*
***************************************************************************************************/

class CGuiE3SkinImageStatic : public CGuiUnitStatic
{
public:

	// Construction Destruction
	CGuiE3SkinImageStatic( void );
	virtual ~CGuiE3SkinImageStatic( void );

	// Flags to modify image fading
	// Public so CStringUtil can do its stuff
	enum
	{	
		FADE_IN				= ( 1 << 0 ),
		FADE_OUT			= ( 1 << 1 ),
		FULL_SCREEN			= ( 1 << 2 ),
	};

protected:

	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessEnd( void );

	//! For setting up specific attributes

	void	SetStateEnter( void );
	void	SetStateExit( void );

	void	UpdateEnter( void );
	void	UpdateExit( void );

	bool	Render( void );

	float			m_fHeight;
	float			m_fWidth;

	float			m_fAlpha;

	int				m_iImageFlags;

	const char*		m_pcImageName;

	ScreenSprite	m_obImage;

	CGuiTimer		m_obFadeTime;

	TextureXDRAM*	m_pobFullScreenTexture;
};

#endif // _E3GUISKINIMAGESTATIC_H
