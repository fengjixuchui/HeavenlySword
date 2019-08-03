//------------------------------------------------------------------------------------------
//!
//!	\file moviehelpers.h
//!
//------------------------------------------------------------------------------------------

#ifndef	MOVIEHELPERS_H_
#define	MOVIEHELPERS_H_

#include "gfx/texture.h"

struct	BINK;
struct	MovieTextureSet;
class	MovieInstance;

namespace Movie
{
	namespace Helpers
	{
		//
		//	Allocate our own frame-buffers/textures for bink to write into.
		//
		void			CreateTextureSet		( BINK *bink, MovieTextureSet **texture_set );

		//
		//	Destroy the frame-buffers we allocated with CreateFrameBuffers.
		//
		void			DestroyTextureSet		( MovieTextureSet *texture_set );

		//
		//	Flushes the cache on the main-processor.
		//
		void			FlushCPUCacheForTextures( MovieTextureSet *texture_set );

		Texture::Ptr	GetCurrentYTexture		( const MovieInstance *movie );
		Texture::Ptr	GetCurrentcRTexture		( const MovieInstance *movie );
		Texture::Ptr	GetCurrentcBTexture		( const MovieInstance *movie );
		Texture::Ptr	GetCurrentAlphaTexture	( const MovieInstance *movie );
	}
}

#endif // !MOVIEHELPERS_H_

