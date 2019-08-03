//------------------------------------------------------------------------------------------
//!
//!	\file movietexture.h
//!
//------------------------------------------------------------------------------------------

#ifndef	MOVIETEXTURE_H_
#define	MOVIETEXTURE_H_

#include "gfx/texture.h"
#include "3rdparty/bink/bink.h"

struct MovieTexture
{
	Texture::Ptr	m_Y;
	Texture::Ptr	m_cR;
	Texture::Ptr	m_cB;
	Texture::Ptr	m_Alpha;
};

struct MovieTextureSet
{
	MovieTexture		m_Textures[ BINKMAXFRAMEBUFFERS ];
	BINKFRAMEBUFFERS	m_BinkBuffers;

	void *				m_BufferMemory;			// Big chunk of memory that our buffers come out of.
	uint32_t			m_FrameSize;			// Size, in bytes, of a MovieTexture set of textures (i.e. one frame).
												// The allocated size is m_BinkBuffers.TotalFrames * m_FrameSize.
};

#endif // !MOVIETEXTURE_H_

