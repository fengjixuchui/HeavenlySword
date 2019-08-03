/***************************************************************************************************
*
*	DESCRIPTION		A usable screen unit that contains movie playback functionality
*
*	NOTES			
*
***************************************************************************************************/

#ifndef _E3GUISKINMOVIE_H
#define _E3GUISKINMOVIE_H

// Includes
#include "guiunitstatic.h"
#include "movies/movieplayer.h"
#include "guimovie.h"


/***************************************************************************************************
*
*	CLASS			CGuiSkinMovie
*
*	DESCRIPTION		Skinned GUI element for displaying image static
*
***************************************************************************************************/

class CGuiSkinMovie : public CGuiUnit
{
	typedef CGuiUnit super;

public:

	// Construction Destruction
	CGuiSkinMovie( void );
	virtual ~CGuiSkinMovie( void );

protected:

	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessEnd( void );
	virtual void	UpdateIdle( void );
	virtual bool	AnyAction( int iPads );

	virtual bool	Render( void );

private:

	typedef enum eGuiMovieState
	{
		GM_IDLE = 0,
		GM_READY,
		GM_PLAYING
	} GuiMovieState;

	float m_fPixelWidth;
	float m_fPixelHeight;
	float m_fPixelX;
	float m_fPixelY;
	GuiMovieState m_State;
	CGuiMovie* m_pobGuiMovie;
	const char* m_pcFileName;
	bool m_bMovieEndMoveBack;
};

#endif // _E3GUISKINMOVIE_H
