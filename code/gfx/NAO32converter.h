//--------------------------------------------------
//!
//!	\file NAO32converter.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef GFX_NAO32_CONVERTER_H
#define GFX_NAO32_CONVERTER_H

#ifndef GFX_RENDERTARGET_H
#include "gfx/rendertarget.h"
#endif

class NAO32converter_impl;

//--------------------------------------------------------------------------------------------------------
//!
//!	NA032converter
//! Simple converter from custom log YUV colour space to RGB
//!
//--------------------------------------------------------------------------------------------------------
class NAO32converter
{
public:
	NAO32converter( void );
	~NAO32converter( void );

	void Convert ( const Texture::Ptr& toBeConverted );

private:
	NAO32converter_impl* m_pImpl;
};

#endif // GFX_NAO32_CONVERTER_H
