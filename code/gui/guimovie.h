//-------------------------------------------------------------------------------------------------
//!
//!	DESCRIPTION		A usable screen unit that contains movie playback functionality
//!
//!	NOTES			
//!
//-------------------------------------------------------------------------------------------------

#ifndef _GUIMOVIE_H
#define _GUIMOVIE_H

// Includes
#include "guimovie.h"
#include "movies/movieinstance.h"

//-------------------------------------------------------------------------------------------------
//!
//!	CLASS			CGuiMovie
//!
//!	DESCRIPTION		Class to play a movie in the gui.
//!
//-------------------------------------------------------------------------------------------------
class CGuiMovie
{
public:

	CGuiMovie();
	~CGuiMovie();

	void Render( void );
	bool IsPlaying();
	bool Start( const char* pcFileName );
	bool Stop( void );

	bool SetPosition( float fPixelX, float fPixelY );
	bool SetSize( float fPixelWidth, float fPixelHeight );

private:

	MovieID m_ID;
};

#endif // _GUIMOVIE_H
