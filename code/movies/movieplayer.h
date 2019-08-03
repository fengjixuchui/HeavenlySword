//------------------------------------------------------------------------------------------
//!
//!	\file movieplayer.h
//!
//------------------------------------------------------------------------------------------

#ifndef	MOVIEPLAYER_H_
#define	MOVIEPLAYER_H_

#include "movies/moviememory.h"
#include "game/command.h"

class	MovieInstance;
typedef IntrusiveRefCountPtr< MovieInstance > MovieInstancePtr;
class	MoviePlayerImpl;
typedef uint32_t MovieID;

//
//	Wrapper class for the movie-player.
//
class MoviePlayer : public Singleton< MoviePlayer >
{
	public:
		//
		//	Operations on movies.
		//
							// Add (and play) a movie.
		void				AddMovie			( MovieInstancePtr movie );

							// Remove a movie - NOTE: If you didn't specify the looping flag
							// then the movie will automatically be removed once it has finished.
		void				RemoveMovie			( MovieInstancePtr movie );

							// Returns true if the given movie is currently playing.
		inline bool			IsPlaying			( MovieInstancePtr movie );

	public:
		//
		//	Player operations.
		//
		inline bool			IsActive			()		const;
		bool				HasMainViewportMovie()		const;

	public:
		//
		//	Swap between ID and MovieInstancePtr. If ID is invalid, or movie
		//	has been deleted then the returned value when converting from an
		//	ID will be NULL.
		//
		//	NOTE:	These are only really used by the lua bind functions - if
		//			you need to keep hold of a movie then use MovieInstancePtr;
		//			it gives you far greater control over the lifetime of the
		//			object and should be quicker as you don't have to check
		//			for NULL pointers all the time.
		//
		MovieID				GetMovieIDFromPtr	( MovieInstancePtr movie )	const;
		MovieInstancePtr	GetMoviePtrFromID	( MovieID id )				const;

	public:
		//
		//	Update the player's movies.
		//
		void				Update				();
		void				AdvanceToNextFrame	();

	public:
		//
		//	Initialise, de-initialise. You must call these after
		//	construction/destruction of the singleton.
		//
		void				Initialise			();
		void				DeInitialise		();

	public:
		//
		//	Ability to skip all currently playing movies on a key press.
		//
#		ifndef _GOLD_MASTER
			COMMAND_RESULT	SkipCurrentMoviesCallback();
#		endif

	public:
		//
		//	Ctor, dtor.
		//
		MoviePlayer();
		~MoviePlayer();

	private:
		//
		//	Disable copying/assignment.
		//
		MoviePlayer( const MoviePlayer & )				NOT_IMPLEMENTED;
		MoviePlayer &operator = ( const MoviePlayer & )	NOT_IMPLEMENTED;

	private:
		//
		//	Internal helper functions - different stages of update().
		//
#		ifdef PLATFORM_PS3
			void		DecompressNextFrame			();
			void		SkipFramesIfRequired		();
			void		FlushCPUCache				();
			void		WaitForFrameToComplete		();
			void		DrawFrameIntoTexture		();
#		endif

	private:
		//
		//	Aggregated members.
		//
		typedef ntstd::List< MovieInstancePtr, Movie::MemChunk > MovieList;

		mutable MovieList	m_Movies;
};

inline bool MoviePlayer::IsPlaying( MovieInstancePtr movie )
{
	for (	MovieList::iterator it = m_Movies.begin();
			it != m_Movies.end();
			++it )
	{
		if ( *it == movie )
		{
			return true;
		}
	}

	return false;
}

inline bool MoviePlayer::IsActive() const
{
	return ( m_Movies.size() > 0 );
}

#endif // !MOVIEPLAYER_H_

