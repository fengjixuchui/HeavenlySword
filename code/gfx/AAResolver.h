//--------------------------------------------------
//!
//!	\file aaresolver.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef GFX_AA_RESOLVER_H
#define GFX_AA_RESOLVER_H

#ifndef GFX_RENDERTARGET_H
#include "gfx/rendertarget.h"
#endif

#ifndef GFX_RENDERERSETTINGS_H
#include "gfx/renderersettings.h"
#endif

class AAResolver_impl;

//--------------------------------------------------
//!
//!	AAResolver
//! resolve antialiased buffers (multisampling, sumpersampling?)
//!
//--------------------------------------------------
class AAResolver
{
public:
	AAResolver( void );
	~AAResolver( void );

	void Resolve ( const RenderTarget::Ptr& pSourceBuffer, const AAResolveMode eAAResMode );

private:
	AAResolver_impl* m_pImpl;
};

#endif // GFX_AA_RESOLVER_H
