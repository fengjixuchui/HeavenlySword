//------------------------------------------------------------------------------------------
//!
//!	\file movieplayer.cpp
//!
//------------------------------------------------------------------------------------------

#include "gui/guisubtitle.h"

#include "movies/movieplayer.h"

#include "movies/moviememory.h"
#include "movies/movieinstance.h"
#include "movies/moviehelpers.h"

#include "effect/screensprite.h"
#include "effect/effect_shims.h"
#include "effect/effect_manager.h"

#include "game/shellconfig.h"
#include "game/entity.h"
#include "game/messagehandler.h"
#include "game/shellmain.h"

#ifdef PLATFORM_PS3
#	include "3rdparty/bink/bink.h"
#endif

#include "audio/audiosystem.h"


// Declare function from moviescripts.cpp to Register our scripts.
// I see no need to put this in a separate header file as no-one
// apart from MoviePlayer should be calling this anyway.
namespace MovieScripts
{
	void Register();

	void OnFMVStartAudio();
	void OnFMVEndAudio();

	static const CHashedString OnFMVStartAudio_ScriptName( "OnFMVStartAudio" );
	static const CHashedString OnFMVEndAudio_ScriptName( "OnFMVEndAudio" );
}

#ifndef _GOLD_MASTER
	bool g_SkipCurrentMovies = false;
#endif

//------------------------------------------------------------------------------------------
//!
//!	Update the player's movies.
//!
//------------------------------------------------------------------------------------------
void MoviePlayer::Update()
{
#	ifdef PLATFORM_PS3
	{
		DecompressNextFrame();

		SkipFramesIfRequired();

		FlushCPUCache();

		WaitForFrameToComplete();

		DrawFrameIntoTexture();	// Actually, the textures are already good so this just blats to screen.
	}
#	endif
}

#ifdef PLATFORM_PS3
//------------------------------------------------------------------------------------------
//!
//!	Internal update stage functions.
//!
//------------------------------------------------------------------------------------------
void MoviePlayer::DecompressNextFrame()
{
	for (	MovieList::iterator it = m_Movies.begin();
			it != m_Movies.end();
			++it )
	{
		BinkDoFrame( ( *it )->m_Bink );
	}
}

void MoviePlayer::SkipFramesIfRequired()
{
	for (	MovieList::iterator it = m_Movies.begin();
			it != m_Movies.end();
			++it )
	{
		HBINK bink = ( *it )->m_Bink;
		while ( BinkShouldSkip( bink ) )
		{
			BinkNextFrame( bink );
			BinkDoFrame( bink );
		}
	}
}

void MoviePlayer::FlushCPUCache()
{
	for (	MovieList::iterator it = m_Movies.begin();
			it != m_Movies.end();
			++it )
	{
		Movie::Helpers::FlushCPUCacheForTextures( ( *it )->m_TextureSet );
	}
}

void MoviePlayer::WaitForFrameToComplete()
{
	for (	MovieList::iterator it = m_Movies.begin();
			it != m_Movies.end();
			++it )
	{
		while ( BinkWait( ( *it )->m_Bink ) )
		{
			// Do nothing for now...
		}
	}
}

void MoviePlayer::DrawFrameIntoTexture()
{
	for (	MovieList::iterator it = m_Movies.begin();
			it != m_Movies.end();
			++it )
	{
		// Draw frames into textures here...
		MovieInstancePtr movie = *it;
		if ( movie->IsFadingOut() )
		{
			// Don't display anything if we're just fading out music - the video should
			// just consist of black frames at this point.
			continue;
		}

		if ( movie->m_Bink->FrameNum == movie->m_Bink->Frames )
		{
			// Don't display anything if we're already finished the movie.
			continue;
		}

		Texture::Ptr Y_plane	= Movie::Helpers::GetCurrentYTexture( movie.GetPtr() );
		Texture::Ptr cR_plane	= Movie::Helpers::GetCurrentcRTexture( movie.GetPtr() );
		Texture::Ptr cB_plane	= Movie::Helpers::GetCurrentcBTexture( movie.GetPtr() );
		Texture::Ptr alpha_plane= Movie::Helpers::GetCurrentAlphaTexture( movie.GetPtr() );
		MovieSprite *pSprite	= NT_NEW MovieSprite( movie->m_PIPDebugID, Y_plane, cR_plane, cB_plane, alpha_plane );

		pSprite->SetPosition( movie->GetPosition() );

		pSprite->SetWidth( movie->GetSize().X() );
		pSprite->SetHeight( movie->GetSize().Y() );
		pSprite->SetColour( CVector( 1.0f, 1.0f, 1.0f, 1.0f ) );

		MovieSpriteShim* pShim = NT_NEW_CHUNK ( Mem::MC_EFFECTS ) MovieSpriteShim( pSprite );
		pShim->m_bHDR = false;
		pShim->m_bAlpha = ( alpha_plane != NULL );

		EffectManager::Get().AddEffect( pShim );

		movie.GetPtr()->SetMovieSpritePtr( pSprite );
	}
}
#endif

void MoviePlayer::AdvanceToNextFrame()
{
	for (	MovieList::iterator it = m_Movies.begin();
			it != m_Movies.end();  )
	{
		MovieInstancePtr movie = *it;
		if ( movie->m_FirstFrame )
		{
			movie->m_FirstFrame = false;
			++it;
			continue;						// We call this function *before* Update(), so on the
											// first frame we don't want to advance because we
											// haven't done anything yet.
		}

		// 'movie_finished' is true when the movie has got within the last 30 frames
		// of it's playback - the last one second is always black so that we can fade
		// down the music from the movie and fade up the background music. We don't
		// display these black frames.
		//
		// 'remove_movie' is true when the movie has actually run out of frames to
		// decode. It corresponds to when we remove the movie.
#		ifdef PLATFORM_PS3
			HBINK bink = movie->m_Bink;
			bool remove_movie = ( bink->FrameNum == bink->Frames );

			static const uint32_t NumSecondsToFadeOutOver = 2;
			uint32_t NumFramesToFadeOutOver = ( NumSecondsToFadeOutOver * bink->FrameRate ) / bink->FrameRateDiv;
			if ( bink->FrameRateDiv != 1 )
			{
				// If we have a fractional frame-rate then integer maths will round down.
				// We don't want to display a single black frame, so skip another one as
				// we obviously can't skip "fractional" frames.
				NumFramesToFadeOutOver++;
			}
			bool movie_finished = ( bink->FrameNum >= ( bink->Frames - NumFramesToFadeOutOver ) );

			if ( bink->Frames < 30 )
			{
				movie_finished = true;
			}

#			ifndef _GOLD_MASTER
			{
				if ( g_ShellOptions->m_SkipMovies )
				{
					movie_finished = true;
					remove_movie = true;
				}

				if ( g_SkipCurrentMovies )
				{
					movie_finished = true;
					remove_movie = true;
				}
			}
#			endif
#		else
			bool remove_movie = true;
			bool movie_finished = true;
#		endif

		// Is the video finished?
		if ( movie_finished )
		{
			// Yes, we're done... 
			if ( !( movie->GetFlags() & MovieInstance::MOVIE_LOOPING ) && !movie->IsFadingOut() )
			{
				// Tell the movie it's in a fading-out state - i.e. it shouldn't render any
				// more frames - only the audio is important now.
				movie->SetFadingOut();

				// If this movie was created with an associated entity
				// then we should send it a message to make sure it knows we've finished.
				if ( movie->m_ScriptEntity != NULL && movie->m_ScriptEntity->GetMessageHandler() != NULL )
				{
					CMessageHandler *message_handler = movie->m_ScriptEntity->GetMessageHandler();
					message_handler->QueueMessage( Message( msg_movie_finished ) );
				}

				// Call the lua function for changing the audio mix-levels.
				MovieScripts::OnFMVEndAudio();

				// If we have no full-screen movies left to finished playing then we should unpause
				// the internal pause flag.
				if ( !HasMainViewportMovie() )
				{
					ShellMain::Get().RequestPauseToggle( ShellMain::PM_INTERNAL, false );

					// also, we need to put the present interval back to normal
					Renderer::Get().RequestPresentMode( Renderer::PM_AUTO );
				}
			}
		}

		if ( remove_movie )
		{
			// Yes, we're done... 
			if ( !( movie->GetFlags() & MovieInstance::MOVIE_LOOPING ) )
			{
				// Not looping, so remove the movie from the player...
				it = m_Movies.erase( it );

				// Stop playing any subtitles
				CSubtitleMan::Get().Stop();

				// We need to go onto the next movie in the list - if we're not
				// looping then BinkNextFrame shouldn't be called on the last frame
				// as doing so causes a pause at the end as we seek back to the start.
				continue;
			}
		}

#		ifdef PLATFORM_PS3
			BinkNextFrame( bink );
#		endif

		++it;
	}

#	ifndef _GOLD_MASTER
	{
		g_SkipCurrentMovies = false;
	}
#	endif
}

//------------------------------------------------------------------------------------------
//!
//!	Conversion from/to MovieID and MovieInstancePtr.
//!
//------------------------------------------------------------------------------------------
MovieID MoviePlayer::GetMovieIDFromPtr( MovieInstancePtr movie ) const
{
	return movie->m_MovieID;
}

MovieInstancePtr MoviePlayer::GetMoviePtrFromID( MovieID id ) const
{
	for (	MovieList::iterator it = m_Movies.begin();
			it != m_Movies.end();
			++it )
	{
		MovieInstancePtr movie = *it;
		if ( movie->m_MovieID == id )
		{
			return movie;
		}
	}

	return MovieInstancePtr();	// Return a NULL pointer.
}

//------------------------------------------------------------------------------------------
//!
//!	Init, De-Init.
//!
//------------------------------------------------------------------------------------------
void MoviePlayer::Initialise()
{
	Movie::MemoryCreate();

#	ifdef PLATFORM_PS3
	{
		BinkSetMemory( Movie::Alloc, Movie::Free );

		int32_t num_speakers = AudioSystem::Get().GetSpeakerCount();
		BinkSoundUseLibAudio( num_speakers );

		EffectResourceMan::Get().RegisterResource( *DebugShaderCache::Get().LoadShader( "moviesprite_vp.sho" ) );
		EffectResourceMan::Get().RegisterResource( *DebugShaderCache::Get().LoadShader( "moviesprite_fp.sho" ) );
/*
		//
		//	Test code.
		//
		{
			File movie_file( "content_neutral/movies/movies.txt", File::FT_READ | File::FT_TEXT );
			char movie_name[ 512+6 ];
			movie_file.Read( movie_name, movie_file.GetFileSize() > 512 ? 512 : movie_file.GetFileSize() );
			char *extension = strstr( movie_name, "bik" );
			extension[ 6 ] = '\0';
			int32_t movie_height = atoi( extension + 3 );
			extension[ 3 ] = '\0';

			const float max_height = 720.0f;

			MovieInstancePtr movie = MovieInstance::Create( movie_name, 0, MovieInstance::MOVIE_LOOPING );
			movie->SetPosition( CPoint( 0.5f, 0.5f, 0.0f ) );
			movie->SetSize( CDirection( 1.0f, float( movie_height ) / max_height, 0.0f ) );
			MoviePlayer::Get().AddMovie( movie );
		}
//*/
	}
#	endif

	MovieScripts::Register();
}

/**
	Add a movie to the list and start playing it.

	@param movie Movie instance pointer.
*/
void MoviePlayer::AddMovie( MovieInstancePtr movie )
{
	m_Movies.push_back( movie );

	// Call the lua function for changing the audio mix-levels.
	MovieScripts::OnFMVStartAudio();

	// Start playing subtitles for this movie
	CSubtitleMan::Get().Play( Util::Upppercase( Util::NoExtension( Util::BaseName(movie->GetMovieName()) ) ) );
}

/**
	Remove a movie from the list.

	@param movie Movie instance pointer.
*/
void MoviePlayer::RemoveMovie( MovieInstancePtr movie )
{
	for (	MovieList::iterator it = m_Movies.begin();
			it != m_Movies.end();
			++it )
	{
		if ( *it == movie )
		{
			m_Movies.erase( it );
			return;
		}
	}

	// Stop playing any subtitles
	CSubtitleMan::Get().Stop();
}

bool MoviePlayer::HasMainViewportMovie() const
{
	for (	MovieList::iterator it = m_Movies.begin();
			it != m_Movies.end();
			++it )
	{
		MovieInstancePtr movie = *it;
		if ( movie->IsMainViewport() && !movie->IsFadingOut() )
		{
			return true;
		}
	}

	return false;
}

void MoviePlayer::DeInitialise()
{
	Movie::MemoryDestroy();
}

//------------------------------------------------------------------------------------------
//!
//!	Ctor, dtor.
//!
//------------------------------------------------------------------------------------------
MoviePlayer::MoviePlayer()
{}

MoviePlayer::~MoviePlayer()
{}

//------------------------------------------------------------------------------------------
//!
//!	Callback for skipping movies currently playing.
//!
//------------------------------------------------------------------------------------------
#ifndef _GOLD_MASTER
COMMAND_RESULT MoviePlayer::SkipCurrentMoviesCallback()
{
	g_SkipCurrentMovies = true;
	return CR_SUCCESS;
}
#endif

//------------------------------------------------------------------------------------------
//!
//!	Function for changing audio mix on FMV start.
//!
//------------------------------------------------------------------------------------------
void MovieScripts::OnFMVStartAudio()
{
	NinjaLua::LuaObject lua_obj = CLuaGlobal::Get().State().GetGlobals().Get<NinjaLua::LuaObject>( OnFMVStartAudio_ScriptName );
	if ( lua_obj.IsFunction() )
	{
		NinjaLua::LuaFunction func( lua_obj );
		func();
	}
}

//------------------------------------------------------------------------------------------
//!
//!	Function for changing audio mix on FMV end.
//!
//------------------------------------------------------------------------------------------
void MovieScripts::OnFMVEndAudio()
{
	NinjaLua::LuaObject lua_obj = CLuaGlobal::Get().State().GetGlobals().Get<NinjaLua::LuaObject>( OnFMVEndAudio_ScriptName );
	if ( lua_obj.IsFunction() )
	{
		NinjaLua::LuaFunction func( lua_obj );
		func();
	}
}













