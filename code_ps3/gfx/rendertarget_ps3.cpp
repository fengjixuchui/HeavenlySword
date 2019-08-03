//--------------------------------------------------
//!
//!	\file gfx/rendertarget_pc.cpp
//! This is the PC version of the rendertarget
//!
//--------------------------------------------------

#include "gfx/rendertarget.h"
#include "gfx/surfacemanager.h"

//-----------------------------------------------------
//!
//! No need for an explicit destructor, when m_Platform 
//! goes out of scope, our handle will be released.
//!
//-----------------------------------------------------
RenderTarget::~RenderTarget() {}

//-----------------------------------------------------
//!
//! 
//!
//-----------------------------------------------------
GFXFORMAT RenderTarget::GetFormat() const 
{
	return ConvertGCFORMATToGFXFORMAT( m_Platform.GetGCFormat() );
}

//-----------------------------------------------------
//!
//! 
//!
//-----------------------------------------------------
GFXAAMODE RenderTarget::GetAAMode() const 
{
	return ConvertGCAAMODEToGFXAAMODE( m_Platform.GetGCAAMode() );
}


//--------------------------------------------------
//!
//!< Get the texture interface asserts we are in a valid state to do so
//!
//--------------------------------------------------

Texture::Ptr RenderTarget::GetTexture()
{
	return SurfaceManager::Get().CreateTexture( Texture::CreationStruct(m_Platform.GetRenderBuffer(), true) );
}

//--------------------------------------------------
//!
//!< Save this out
//!
//--------------------------------------------------
void RenderTarget::SaveToDisk( const char* pcFileName )
{
	GetTexture()->m_Platform.SaveToDisk( pcFileName );
}
