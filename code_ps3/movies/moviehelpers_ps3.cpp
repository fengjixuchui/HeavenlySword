//------------------------------------------------------------------------------------------
//!
//!	\file moviehelpers_ps3.cpp
//!
//------------------------------------------------------------------------------------------

#include "movies/moviehelpers.h"
#include "movies/moviememory.h"
#include "movies/movietexture.h"
#include "movies/movieinstance.h"

#include "gfx/texture.h"
#include "gfx/rendertarget.h"
#include "gfx/surfacemanager_ps3.h"

#include "3rdparty/bink/bink.h"

void Movie::Helpers::CreateTextureSet( BINK *bink, MovieTextureSet **pp_texture_set )
{
	ntError_p( pp_texture_set != NULL, ("We need a pointer to a valid object - this function just fills in the object's fields.") );
	ntError( *pp_texture_set == NULL );
	*pp_texture_set = NT_NEW_CHUNK( Movie::MemChunk ) MovieTextureSet;

	MovieTextureSet *texture_set = *pp_texture_set;

	// Get the buffer info from bink...
	BINKFRAMEBUFFERS *buffers = &( texture_set->m_BinkBuffers );
	BinkGetFrameBuffersInfo( bink, buffers );

	// Work out the total memory size of the buffers.
	uint32_t ywidth		= ( buffers->Frames[ 0 ].YPlane.Allocate ) ? buffers->YABufferWidth : 0;
	uint32_t awidth		= ( buffers->Frames[ 0 ].APlane.Allocate ) ? buffers->YABufferWidth : 0;
	uint32_t crcbwidth	= ( buffers->Frames[ 0 ].cRPlane.Allocate ) ? buffers->cRcBBufferWidth : 0;
	uint32_t yaheight	= buffers->YABufferHeight;
	uint32_t crcbheight	= buffers->cRcBBufferHeight;

	uint32_t ysize		= ROUND_POW2( ywidth * yaheight, 128 );
	uint32_t asize		= ROUND_POW2( awidth * yaheight, 128 );
	uint32_t crcbsize	= ROUND_POW2( crcbwidth * crcbheight, 128 );

	uint32_t Aoffset	= ysize;
	uint32_t cRoffset	= ysize + asize;
	uint32_t cBoffset	= ysize + asize + crcbsize;

	ntError( ywidth == awidth || ywidth == 0 || awidth == 0 );
	uint32_t YApitch	= ywidth | awidth; // they will be the same, or one will be zero
	uint32_t cRcBpitch	= crcbwidth;

	texture_set->m_FrameSize = ysize + asize + 2 * crcbsize;

	// Allocate a big enough pool of memory to fit all the buffers into. This must come from XDDR host ram.
	ntError_p( buffers->TotalFrames >= 0 && buffers->TotalFrames <= (int32_t)Movie::GetMaxNumFrames(), ("Too many frames required for this movie! Memory will be overwritten") );
	uint8_t *tex_buffer = (uint8_t *)Movie::GetFrameMemory();
	texture_set->m_BufferMemory = (void *)tex_buffer;

	//
	//	Now create the textures.
	//
	for ( int32_t i=0;i<buffers->TotalFrames;i++ )
	{
		// Luminance plane.
		if ( buffers->Frames[ i ].YPlane.Allocate )
		{
			buffers->Frames[ i ].YPlane.BufferPitch = YApitch;
			buffers->Frames[ i ].YPlane.Buffer = tex_buffer;

			texture_set->m_Textures[ i ].m_Y = SurfaceManagerPlatform::TextureFromMainMem(	buffers->YABufferWidth,
																							buffers->YABufferHeight,
																							YApitch,
																							Gc::kTexFormatL8,
																							tex_buffer );
		}

		// cR plane (first chromiance axis).
		if ( buffers->Frames[ i ].cRPlane.Allocate )
		{
			buffers->Frames[ i ].cRPlane.BufferPitch = cRcBpitch;
			buffers->Frames[ i ].cRPlane.Buffer = tex_buffer + cRoffset;

			texture_set->m_Textures[ i ].m_cR = SurfaceManagerPlatform::TextureFromMainMem(	buffers->cRcBBufferWidth,
																							buffers->cRcBBufferHeight,
																							cRcBpitch,
																							Gc::kTexFormatL8,
																							tex_buffer + cRoffset );
		}

		// cB plane (second chromiance axis).
		if ( buffers->Frames[ i ].cBPlane.Allocate )
		{
			buffers->Frames[ i ].cBPlane.BufferPitch = cRcBpitch;
			buffers->Frames[ i ].cBPlane.Buffer = tex_buffer + cBoffset;

			texture_set->m_Textures[ i ].m_cB = SurfaceManagerPlatform::TextureFromMainMem(	buffers->cRcBBufferWidth,
																							buffers->cRcBBufferHeight,
																							cRcBpitch,
																							Gc::kTexFormatL8,
																							tex_buffer + cBoffset );
		}

		// Alpha plane.
		if ( buffers->Frames[ i ].APlane.Allocate )
		{
			buffers->Frames[ i ].APlane.Buffer = tex_buffer + Aoffset;
			buffers->Frames[ i ].APlane.BufferPitch = YApitch;

			texture_set->m_Textures[ i ].m_Alpha = SurfaceManagerPlatform::TextureFromMainMem(	buffers->YABufferWidth,
																								buffers->YABufferHeight,
																								YApitch,
																								Gc::kTexFormatL8,
																								tex_buffer + Aoffset );
		}

		tex_buffer += texture_set->m_FrameSize;
	}

	// Tell bink to use these buffers.
	BinkRegisterFrameBuffers( bink, buffers );
}

void Movie::Helpers::DestroyTextureSet( MovieTextureSet *texture_set )
{
	ntError_p( texture_set != NULL, ("We need a pointer to a valid object - this function just fills in the object's fields.") );

	BINKFRAMEBUFFERS *buffers = &( texture_set->m_BinkBuffers );
	for ( int32_t i=0;i<buffers->TotalFrames;i++ )
	{
		buffers->Frames[ i ].YPlane.Buffer	= NULL;
		buffers->Frames[ i ].cRPlane.Buffer	= NULL;
		buffers->Frames[ i ].cBPlane.Buffer	= NULL;
		buffers->Frames[ i ].APlane.Buffer	= NULL;

		texture_set->m_Textures[ i ].m_Y	= Texture::Ptr();
		texture_set->m_Textures[ i ].m_cR	= Texture::Ptr();
		texture_set->m_Textures[ i ].m_cB	= Texture::Ptr();
		texture_set->m_Textures[ i ].m_Alpha= Texture::Ptr();
	}

	Movie::ReleaseFrameMemory();
	texture_set->m_BufferMemory = NULL;

	NT_DELETE_CHUNK( Movie::MemChunk, texture_set );
}

void Movie::Helpers::FlushCPUCacheForTextures( MovieTextureSet *texture_set )
{
	ntError( texture_set != NULL );

	void *ptr = static_cast< uint8_t * >( texture_set->m_BufferMemory ) + texture_set->m_FrameSize*texture_set->m_BinkBuffers.FrameNum;

#	ifndef _RELEASE
	{
		uint32_t pitch = 0;
		ntError( (uintptr_t)ptr == (uintptr_t)( texture_set->m_Textures[ texture_set->m_BinkBuffers.FrameNum ].m_Y->CPULock2D( pitch ) ) );
		ntError( pitch = texture_set->m_BinkBuffers.Frames[ texture_set->m_BinkBuffers.FrameNum ].YPlane.BufferPitch );
		UNUSED( pitch );
	}
#	endif
	
	uint32_t size = texture_set->m_FrameSize;

	for ( uint32_t i=0;i<size;i+=128 )
	{
		asm  ("dcbst     0,%0" : : "r" (ptr));
		ptr = ( (uint8_t *)ptr ) + 128;
	}
}

Texture::Ptr Movie::Helpers::GetCurrentYTexture( const MovieInstance *movie )
{
	const MovieTextureSet *tex_set = movie->GetTextureSet();
	ntError( tex_set != NULL );

	ntError( tex_set->m_BinkBuffers.FrameNum >= 0 && tex_set->m_BinkBuffers.FrameNum < BINKMAXFRAMEBUFFERS );
	return tex_set->m_Textures[ tex_set->m_BinkBuffers.FrameNum ].m_Y;
}

Texture::Ptr Movie::Helpers::GetCurrentcRTexture( const MovieInstance *movie )
{
	const MovieTextureSet *tex_set = movie->GetTextureSet();
	ntError( tex_set != NULL );

	ntError( tex_set->m_BinkBuffers.FrameNum >= 0 && tex_set->m_BinkBuffers.FrameNum < BINKMAXFRAMEBUFFERS );
	return tex_set->m_Textures[ tex_set->m_BinkBuffers.FrameNum ].m_cR;
}

Texture::Ptr Movie::Helpers::GetCurrentcBTexture( const MovieInstance *movie )
{
	const MovieTextureSet *tex_set = movie->GetTextureSet();
	ntError( tex_set != NULL );

	ntError( tex_set->m_BinkBuffers.FrameNum >= 0 && tex_set->m_BinkBuffers.FrameNum < BINKMAXFRAMEBUFFERS );
	return tex_set->m_Textures[ tex_set->m_BinkBuffers.FrameNum ].m_cB;
}

Texture::Ptr Movie::Helpers::GetCurrentAlphaTexture( const MovieInstance *movie )
{
	const MovieTextureSet *tex_set = movie->GetTextureSet();
	ntError( tex_set != NULL );

	ntError( tex_set->m_BinkBuffers.FrameNum >= 0 && tex_set->m_BinkBuffers.FrameNum < BINKMAXFRAMEBUFFERS );
	return tex_set->m_Textures[ tex_set->m_BinkBuffers.FrameNum ].m_Alpha;
}


