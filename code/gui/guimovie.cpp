//-------------------------------------------------------------------------------------------------
//!
//!	DESCRIPTION		Class to implement movie playback in the gui.
//!
//!	NOTES			
//!
//-------------------------------------------------------------------------------------------------

// Includes
#include "guimovie.h"
#include "movies/movieplayer.h"
#include "effect/moviesprite.h"
#include "guiutil.h"
#include "guimanager.h"
#include "movies/moviememory.h"

//-------------------------------------------------------------------------------------------------
//!
//! Constructor
//!
//-------------------------------------------------------------------------------------------------
CGuiMovie::CGuiMovie() :
	m_ID( 0 )
{
}

//-------------------------------------------------------------------------------------------------
//!
//! Destructor
//!
//-------------------------------------------------------------------------------------------------
CGuiMovie::~CGuiMovie()
{
	Stop();
}

//-------------------------------------------------------------------------------------------------
//!
//! CGuiMovie::Start
//!
//! Start movie playback.
//!
//-------------------------------------------------------------------------------------------------
bool CGuiMovie::Start( const char* pcFileName )
{
	UNUSED( pcFileName );

#ifdef PLATFORM_PS3

	// If movie memory is already do not allow this movie to be created.
	if ( Movie::MovieMemoryInUse() )
	{
		return false;
	}

	MovieInstancePtr movie = MoviePlayer::Get().GetMoviePtrFromID( m_ID );

	if ( NULL == movie )
	{
		// Create the movie
		movie = MovieInstance::Create( pcFileName, 0 );

		if ( NULL != movie )
		{
			// Default to center of screen at normal size.
			movie->SetPosition( CPoint( 0.5f, 0.5f, 0.0f ) );
			movie->SetSize( CDirection( 1.0f, 1.0f, 0.0f ) );

			MoviePlayer::Get().AddMovie( movie );

			m_ID = MoviePlayer::Get().GetMovieIDFromPtr( movie );

			return true;
		}
	}

#endif

	return false;
}

//-------------------------------------------------------------------------------------------------
//!
//! CGuiMovie::Stop
//!
//! Stop movie playback.
//!
//-------------------------------------------------------------------------------------------------
bool CGuiMovie::Stop( void )
{
#ifdef PLATFORM_PS3

	MovieInstancePtr movie = MoviePlayer::Get().GetMoviePtrFromID( m_ID );

	if ( NULL != movie )
	{
		if ( MoviePlayer::Get().IsPlaying( movie ) )
		{
			// Ultimately results in the movie memory being freed.
			MoviePlayer::Get().RemoveMovie( movie );

			return true;
		}
	}
#endif

	return false;
}

//-------------------------------------------------------------------------------------------------
//!
//! CGuiMovie::Render
//!
//-------------------------------------------------------------------------------------------------
void CGuiMovie::Render( void )
{
#ifdef PLATFORM_PS3

	MovieInstancePtr movie = MoviePlayer::Get().GetMoviePtrFromID( m_ID );

	if ( NULL != movie )
	{
		MovieInstance* pobMovieInstance = movie.GetPtr();
		
		if ( pobMovieInstance )
		{
			// Get pointer to the movie sprite and explicitly call render.
			MovieSprite* pobMovieSprite = pobMovieInstance->GetMovieSpritePtr();

			if ( pobMovieSprite )
			{
				pobMovieSprite->Render( true );
			}
		}
	}

#endif
}

//-------------------------------------------------------------------------------------------------
//!
//! CGuiMovie::IsPlaying
//!
//-------------------------------------------------------------------------------------------------
bool CGuiMovie::IsPlaying( void )
{
	bool bIsPlaying = false;

#ifdef PLATFORM_PS3

	MovieInstancePtr movie = MoviePlayer::Get().GetMoviePtrFromID( m_ID );

	if ( NULL != movie )
	{
		if ( MoviePlayer::Get().IsPlaying( movie ) )
		{
			bIsPlaying = true;
		}
	}
#endif

	return bIsPlaying;
}

//-------------------------------------------------------------------------------------------------
//!
//! CGuiMovie::SetPosition
//!
//! Set playback position in pixel coordinates.
//!
//-------------------------------------------------------------------------------------------------
bool CGuiMovie::SetPosition( float fPixelX, float fPixelY )
{
	UNUSED( fPixelX );
	UNUSED( fPixelY );

#ifdef PLATFORM_PS3

	MovieInstancePtr movie = MoviePlayer::Get().GetMoviePtrFromID( m_ID );

	if ( NULL != movie )
	{
		// Calc normalised position.
		float fScreenX = fPixelX/CGuiManager::Get().BBWidth();
		float fScreenY = fPixelY/CGuiManager::Get().BBHeight();
		
		movie->SetPosition( CPoint( fScreenX, fScreenY, 0.0f ) );
	
		return true;
	}

#endif

	return false;
}

//-------------------------------------------------------------------------------------------------
//!
//! CGuiMovie::SetSize
//!
//! Set playback size.
//!
//-------------------------------------------------------------------------------------------------
bool CGuiMovie::SetSize( float fPixelWidth, float fPixelHeight )
{
	UNUSED( fPixelWidth );
	UNUSED( fPixelHeight );

#ifdef PLATFORM_PS3

	MovieInstancePtr movie = MoviePlayer::Get().GetMoviePtrFromID( m_ID );

	if ( NULL != movie )
	{
		// Calc normalised position.
		float fWidth = fPixelWidth/CGuiManager::Get().BBWidth();
		float fHeight = fPixelHeight/CGuiManager::Get().BBHeight();

		movie->SetSize( fWidth, fHeight );
	
		return true;
	}

#endif

	return false;
}
