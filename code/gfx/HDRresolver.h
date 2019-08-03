//--------------------------------------------------
//!
//!	\file HDRresolver.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef GFX_HDR_RESOLVE_H
#define GFX_HDR_RESOLVE_H

#ifndef GFX_RENDERTARGET_H
#include "gfx/rendertarget.h"
#endif

class HDRResolver_impl;

//--------------------------------------------------
//!
//!	HDRResolver
//! Shifts scene from a HDR back buffer to a tone mapped X8R8G8B8 one.
//!
//--------------------------------------------------
class HDRResolver
{
public:
	HDRResolver( void );
	~HDRResolver( void );

	void ResolveHDRtoLinear( RenderTarget::Ptr& pBackBuffer, const Texture::Ptr& pLens, const Texture::Ptr& pKeyLuminance );
	void ResolveHDRtoLinear( RenderTarget::Ptr& pBackBuffer, const Texture::Ptr& pLens, float fKeyLuminance );

private:
	HDRResolver_impl* m_pImpl;
};

#endif // GFX_HDR_RESOLVE_H
