//------------------------------------------------------------------------------------------
//!
//!	\file moviehelpers_pc.cpp
//!
//!			These are just dummy functions for the pc build.
//!
//------------------------------------------------------------------------------------------

#include "movies/moviehelpers.h"

void Movie::Helpers::CreateTextureSet( BINK *bink, MovieTextureSet **texture_set )
{
	UNUSED( bink );
	UNUSED( texture_set );
	ntError( bink == NULL );
	ntError( texture_set != NULL );
	ntError( *texture_set == NULL );
}

void Movie::Helpers::DestroyTextureSet( MovieTextureSet *texture_set )
{
	UNUSED( texture_set );
	ntError( texture_set != NULL );
}

void Movie::Helpers::FlushCPUCacheForTextures( MovieTextureSet *texture_set )
{
	UNUSED( texture_set );
}

Texture::Ptr Movie::Helpers::GetCurrentYTexture( const MovieInstance *movie )
{
	UNUSED( movie );
	return Texture::Ptr();
}

Texture::Ptr Movie::Helpers::GetCurrentcRTexture( const MovieInstance *movie )
{
	UNUSED( movie );
	return Texture::Ptr();
}

Texture::Ptr Movie::Helpers::GetCurrentcBTexture( const MovieInstance *movie )
{
	UNUSED( movie );
	return Texture::Ptr();
}

Texture::Ptr Movie::Helpers::GetCurrentAlphaTexture( const MovieInstance *movie )
{
	UNUSED( movie );
	return Texture::Ptr();
}




