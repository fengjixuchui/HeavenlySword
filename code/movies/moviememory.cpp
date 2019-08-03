//------------------------------------------------------------------------------------------
//!
//!	\file moviememory.cpp
//!
//------------------------------------------------------------------------------------------

#include "movies/moviememory.h"

void *Movie::Alloc( uint32_t num_bytes )
{
	return (void *)NT_MEMALIGN_CHUNK( Movie::MemChunk, num_bytes, Movie::Alignment );
}

void Movie::Free( void *mem_ptr )
{
	NT_FREE_CHUNK( Movie::MemChunk, (uintptr_t)mem_ptr );
}

namespace
{
	static const uint32_t	NumPixelsInFrame			= ( 1280 * 720 ) + 2*( 640 * 360 );		// 1 full size 720p luminance buffer and 2 half-size chromiance buffers. 1 byte-per-pixel each.
	static const uint32_t	NumFramesToAlloc			= 2;
	static const uint32_t	MoviesGlobalMemorySize		= NumPixelsInFrame * NumFramesToAlloc;
	static uint8_t *		g_MoviesGlobalMemory		= NULL;
	static bool				g_MovieGlobalMemoryInUse	= false;
}

void Movie::MemoryCreate()
{
	ntError_p( g_MoviesGlobalMemory == NULL, ("Movie global memory has already been initialised.") );

	g_MoviesGlobalMemory = (uint8_t *)NT_MEMALIGN_CHUNK( Mem::MC_RSX_MAIN_USER, MoviesGlobalMemorySize, 16 );
	ntError_p( g_MoviesGlobalMemory != NULL, ("Failed to allocate movies global memory.") );
}

void *Movie::GetFrameMemory()
{
	ntError_p( !g_MovieGlobalMemoryInUse, ("Movie global memory pool already in use! Do not create movies while another one is playing!") );
	g_MovieGlobalMemoryInUse = true;
	return g_MoviesGlobalMemory;
}

void Movie::ReleaseFrameMemory()
{
	ntError_p( g_MovieGlobalMemoryInUse, ("Releasing global movies memory when it hasn't been requested.") );
	g_MovieGlobalMemoryInUse = false;
}

uint32_t Movie::GetFrameMemorySize()
{
	return MoviesGlobalMemorySize;
}

uint32_t Movie::GetMaxNumFrames()
{
	return NumFramesToAlloc;
}

void Movie::MemoryDestroy()
{
	ntError_p( g_MoviesGlobalMemory != NULL, ("Movie global memory has not yet been initialised.") );

	NT_FREE_CHUNK( Mem::MC_RSX_MAIN_USER, (uintptr_t)g_MoviesGlobalMemory );
	g_MoviesGlobalMemory = NULL;
}

bool Movie::MovieMemoryInUse( void )
{
	return g_MovieGlobalMemoryInUse;
}













