/***************************************************************************************************
*
*	DESCRIPTION		A usable screen unit that sontains video playback functionality
*
*	NOTES			
*
***************************************************************************************************/

#ifndef _E3GUISKINVIDEO_H
#define _E3GUISKINVIDEO_H

// Includes
#include "guiunitstatic.h"

#include "effect/screensprite.h"

/***************************************************************************************************
*
*	CLASS			CGuiSkinVideo
*
*	DESCRIPTION		Skinned GUI element for displaying image static
*
***************************************************************************************************/

class CGuiSkinVideo : public CGuiUnitStatic
{
public:

	// Construction Destruction
	CGuiSkinVideo( void );
	virtual ~CGuiSkinVideo( void );

	// Flags to modify image fading
	// Public so CStringUtil can do its stuff
	/*enum
	{	
		FADE_IN				= ( 1 << 0 ),
		FADE_OUT			= ( 1 << 1 ),
		FULL_SCREEN			= ( 1 << 2 ),
	};*/

protected:

	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessEnd( void );

	//! For setting up specific attributes

	void	SetStateEnter( void );
	void	SetStateExit( void );
	void	SetStateIdle( void );

	void	UpdateEnter( void );
	void	UpdateExit( void );
	void	UpdateIdle( void );

	bool	Render( void );

	float			m_fHeight;
	float			m_fWidth;

	float			m_fAlpha;

	//int				m_iImageFlags;

	const char*		m_pcFileName;

	ScreenSprite	m_obVideo;

	CGuiTimer		m_obFadeTime;

	TextureXDRAM*	m_pobFullScreenTexture;
};

#endif // _E3GUISKINVIDEO_H
