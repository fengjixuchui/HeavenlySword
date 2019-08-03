//------------------------------------------------------------------------------------------
//!
//!	\file moviescripts.cpp
//!
//------------------------------------------------------------------------------------------

#include "movies/movieplayer.h"
#include "movies/movieinstance.h"
#include "game/entity.h"
#include "game/shellmain.h"

#include "gfx/renderer.h"

#include "lua/ninjalua.h"

namespace MovieScripts
{
	//
	//	Creates and starts playback of a movie.
	//	Returns the ID of the movie - use in any subsequent calls to StopMovie, etc...
	//	filename		:	filename+NON-EXTENDED-path of <movie>.bik file. e.g. "movies/my_movie.bik".
	//	script_entity	:	Only required if you want to receive "msg_movie_finished".
	//	pip_debug_id	:	The debug-id of the pip view you want to render the movie into, 0=main view.
	//	flags			:	Any combination of flags - see MovieInstance creation flags.
	//
	static MovieID PlayMovie( const char *filename, CEntity *script_entity, int32_t pip_debug_id, int32_t flags )
	{
		// Pause the game-code if we're running a full-screen movie.
		if ( pip_debug_id == 0 )
		{
			ShellMain::Get().RequestPauseToggle( ShellMain::PM_INTERNAL, true );

			// also, we need to put the present interval back to VBLANK
			Renderer::Get().RequestPresentMode( Renderer::PM_VBLANK );
		}

		MovieInstancePtr movie = MovieInstance::Create( filename, pip_debug_id, flags );
		movie->SetScriptEntity( script_entity );

		MoviePlayer::Get().AddMovie( movie );

		return MoviePlayer::Get().GetMovieIDFromPtr( movie );
	}

	//
	//	Stops playback of the movie - if nothing else is holding onto the movie
	//	then this will also delete the movie.
	//	id				:	MovieID returned from PlayMovie.
	//
	static void StopMovie( MovieID id )
	{
		MovieInstancePtr movie = MoviePlayer::Get().GetMoviePtrFromID( id );
		if ( movie != NULL )
		{
			MoviePlayer::Get().RemoveMovie( movie );
		}
	}

	//
	//	Returns true if the movie is still playing, false if it's finished or if the
	//	movie pointer could not be found.
	//	id				:	MovieID returned from PlayMovie.
	//
	static bool IsMoviePlaying( MovieID id )
	{
		MovieInstancePtr movie = MoviePlayer::Get().GetMoviePtrFromID( id );
		if ( movie != NULL )
		{
			return MoviePlayer::Get().IsPlaying( movie );
		}

		return false;
	}

	//
	//	Sets the size of the movie in normalised coordinates.
	//	x				:	width of the movie, 1=full-viewport width.
	//	y				:	height of the movie, 1=full-viewport height.
	//
	static void SetMovieSize( MovieID id, float x, float y )
	{
		MovieInstancePtr movie = MoviePlayer::Get().GetMoviePtrFromID( id );
		if ( movie != NULL )
		{
			movie->SetSize( CDirection( x, y, 0.0f ) );
		}
	}

	//
	//	Sets the position of the centre of the movie in normalised coordinates.
	//	x				:	x-pos of the movie, 0=left edge, 0.5=middle, 1=right edge.
	//	y				:	y-pos of the movie, 0=top edge, 0.5=middle, 1=bottom edge.
	//
	static void SetMoviePos( MovieID id, float x, float y )
	{
		MovieInstancePtr movie = MoviePlayer::Get().GetMoviePtrFromID( id );
		if ( movie != NULL )
		{
			movie->SetPosition( CPoint( x, y, 0.0f ) );
		}
	}

	void Register()
	{
		NinjaLua::LuaObject globals = CLuaGlobal::Get().State().GetGlobals();

		globals.Set( "Movies", NinjaLua::LuaObject::CreateTable( CLuaGlobal::Get().State() ) );

		globals[ "Movies" ].Register( "Play",		MovieScripts::PlayMovie );
		globals[ "Movies" ].Register( "Stop",		MovieScripts::StopMovie );
		globals[ "Movies" ].Register( "IsPlaying",	MovieScripts::IsMoviePlaying );
		globals[ "Movies" ].Register( "SetSize",	MovieScripts::SetMovieSize );
		globals[ "Movies" ].Register( "SetPos",	MovieScripts::SetMoviePos );
	}
}



