//------------------------------------------------------------------------------------------
//!
//!	\file movieinstance.h
//!
//------------------------------------------------------------------------------------------

#ifndef	MOVIEINSTANCE_H_
#define	MOVIEINSTANCE_H_

class	CEntity;
struct	BINK;
struct	MovieTextureSet;
class	MoviePlayer;
class	MovieInstance;
class MovieSprite;

typedef IntrusiveRefCountPtr< MovieInstance >	MovieInstancePtr;
typedef uint32_t MovieID;

#include "movies/moviehelpers.h"

//
//	Object to hold a movie clip - create using MoviePlayer::Get().CreateMovie(...).
//	IMPORTANT!	MovieInstances are ref-counted - either call AddRef/Release correctly
//				(COM-stylee) or always use a MovieInstancePtr.
//
class MovieInstance
{
	public:
		//
		//	Parameter accessors.
		//
								// Set the centre position of the movie on the screen, in normalised
								// coordinates. (0,0)=top-left, (1,1)=bottom-right.
		inline void				SetPosition		( float x, float y );
		inline void				SetPosition		( const CPoint &pos );			// z component ignored.

								// Get the normalised coordinate position of the centre of the movie.
		inline CPoint			GetPosition		()						const;

								// Set the length(x) and height(y) of the movie, in normalised coordinates.
		inline void				SetSize			( float x, float y );
		inline void				SetSize			( const CDirection &size );		// z component ignored.

								// Get the normalised coordinate size of the movie.
		inline CDirection		GetSize			()						const;

								// Return the flags.
		inline uint32_t			GetFlags		()						const;

								// Return the filename of the movie this object corresponds to.
		inline const char *		GetMovieName	()						const;

								// Return the texture set for this movie-instance.
		inline const MovieTextureSet *GetTextureSet	()					const;

		inline bool				IsMainViewport	()						const;

		inline const CEntity *	GetScriptEntity	()						const;
		inline CEntity *		GetScriptEntity	();

		inline void				SetScriptEntity	( CEntity *script_entity );

		MovieSprite *			GetMovieSpritePtr	()					const;
		void					SetMovieSpritePtr	( MovieSprite *pobMovieSprite );

		bool					IsFadingOut		()						const;
		void					SetFadingOut	();

	public:
		//
		//	Create a movie.
		//
		enum	// Creation flags.
		{
			MOVIE_LOOPING		=	( 1 << 0 ),
			MOVIE_NO_SOUND		=	( 1 << 1 )
		};
		static MovieInstancePtr	Create			( const char *filename, int32_t pip_debug_id, uint32_t flags = 0 );

	public:
		//
		//	Ref-counting functionality.
		//
								// Decrement the ref-count and, if zero, delete the animation.
		inline void				Release			()						const;

								// Increment the reference count by one.
		inline void				AddRef			()						const		{ ++m_RefCount; }

		inline int32_t			GetRefCount		()						const		{ return m_RefCount; }

	private:
		//
		//	Internal helper functions.
		//
		void					InitMixBinVolumes	( bool stereo );

	private:
		//
		//	Ctors, dtor.
		//
		MovieInstance( const char *filename, int32_t pip_debug_id, uint32_t flags );
		~MovieInstance();

	private:
		//
		//	Prevent copying/assignment.
		//
		MovieInstance( const MovieInstance & )				NOT_IMPLEMENTED;
		MovieInstance &operator = ( const MovieInstance & )	NOT_IMPLEMENTED;

	private:
		//
		//	Aggregated members.
		//
		friend class		MoviePlayer;

		CPoint				m_Position;
		CDirection			m_Size;

		CEntity *			m_ScriptEntity;

		BINK *				m_Bink;
		MovieTextureSet *	m_TextureSet;

		char				m_Filename[ MAX_PATH ];

		uint32_t			m_Flags;

		int32_t				m_PIPDebugID;
		MovieID				m_MovieID;

		mutable int32_t		m_RefCount;					// Our ref-count.

		bool				m_FirstFrame;
		bool				m_IsFadingOut;

		MovieSprite *		m_pobMovieSprite;

		static MovieID		m_MasterMovieID;
};

//								 //
// ***** Inlined functions ***** //
//								 //
inline void MovieInstance::SetPosition( float x, float y )
{
	m_Position = CPoint( x, y, 0.0f );
}

inline void MovieInstance::SetPosition( const CPoint &pos )
{
	ntError_p( fabs( pos.Z() ) < 0.0001f, ("MovieInstance::SetPosition: Z component should be zero!") );
	m_Position = pos;
}

inline CPoint MovieInstance::GetPosition() const
{
	return m_Position;
}

inline void MovieInstance::SetSize( float x, float y )
{
	m_Size = CDirection( x, y, 0.0f );
}

inline void MovieInstance::SetSize( const CDirection &size )
{
	ntError_p( fabs( size.Z() ) < 0.0001f, ("MovieInstance::SetSize: Z component should be zero!") );
	m_Size = size;
}

inline CDirection MovieInstance::GetSize() const
{
	return m_Size;
}

inline uint32_t MovieInstance::GetFlags() const
{
	return m_Flags;
}

inline const char *MovieInstance::GetMovieName() const
{
	return m_Filename;
}

inline const MovieTextureSet *MovieInstance::GetTextureSet() const
{
	return m_TextureSet;
}

inline const CEntity *MovieInstance::GetScriptEntity() const
{
	return m_ScriptEntity;
}

inline CEntity *MovieInstance::GetScriptEntity()
{
	return m_ScriptEntity;
}

inline void MovieInstance::SetScriptEntity( CEntity *script_entity )
{
	m_ScriptEntity = script_entity;
}

inline void MovieInstance::Release() const
{
	ntError_p( m_RefCount > 0, ("This ref-count is <= 0, why isn't the object deleted already?") );
	if ( --m_RefCount == 0 )
	{
		NT_DELETE_CHUNK( Movie::MemChunk, this );
	}
}

inline bool MovieInstance::IsMainViewport() const
{
	return m_PIPDebugID == 0;
}

inline MovieSprite* MovieInstance::GetMovieSpritePtr( void ) const
{
	return m_pobMovieSprite;
}

inline void MovieInstance::SetMovieSpritePtr( MovieSprite* pobMovieSprite )
{
	m_pobMovieSprite = pobMovieSprite;
}

inline bool MovieInstance::IsFadingOut() const
{
	return m_IsFadingOut;
}

inline void MovieInstance::SetFadingOut()
{
	m_IsFadingOut = true;
}

#endif // !MOVIEINSTANCE_H_



