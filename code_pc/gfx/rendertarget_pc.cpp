//--------------------------------------------------
//!
//!	\file gfx/rendertarget_pc.cpp
//! This is the PC version of the rendertarget
//!
//--------------------------------------------------

#include "gfx/rendertarget.h"
#include "gfx/dxerror_pc.h"
#include "gfx/surfacemanager.h"
#include "gfx/graphicsdevice.h"

//--------------------------------------------------
//!
//! destroy, free any allocated memory
//!
//--------------------------------------------------
RenderTarget::~RenderTarget()
{
	if ( !m_Platform.m_bImplicit )
		m_Platform.m_pRtSurface->Release();

	// note: two releases here, to match the additional D3Dref
	// caused by calling GetSurfaceLevel on IDirect3DTexture9
	if ( m_Platform.m_pTexture )
		m_Platform.m_pTexture->Release();
}

//--------------------------------------------------
//!
//! returns a generic format you may get a platform specific marker
//!
//--------------------------------------------------
GFXFORMAT RenderTarget::GetFormat() const 
{
	return ConvertD3DFORMATToGFXFORMAT( m_Platform.GetDXFormat() );
}

//-----------------------------------------------------
//!
//! 
//!
//-----------------------------------------------------
GFXAAMODE RenderTarget::GetAAMode() const 
{
	// we dont support MSAA on pc
	return GAA_MULTISAMPLE_NONE;
}

//--------------------------------------------------
//!
//!< Get the texture interface asserts we are in a valid state to do so
//!
//--------------------------------------------------

Texture::Ptr RenderTarget::GetTexture()
{
	ntAssert_p( m_Platform.m_bAlwaysDiscarded == false, ("Render Target has no associated texture") );

	// implicit textures don't have to be released but can be safely
	return SurfaceManager::Get().CreateTexture( Texture::CreationStruct(m_Platform.m_pTexture, true) );
}

//--------------------------------------------------
//!
//!< Get the surface interface asserts we are in a valid state to do so
//!
//--------------------------------------------------
Surface::Ptr RenderTarget::GetSurface()
{
	// implicit textures don't have to be released but can be safely
	return SurfaceManager::Get().CreateSurface( Surface::CreationStruct(m_Platform.m_pRtSurface, true) );
}

//--------------------------------------------------
//!
//!< Save this out
//!
//--------------------------------------------------
void RenderTarget::SaveToDisk( const char* pcFileName )
{
	// create scratch surface
	Surface::CreationStruct	createParams( GetWidth(), GetHeight(), m_Platform.GetDXFormat(), D3DPOOL_SYSTEMMEM );
	createParams.bCacheable = false;

	// resolve RT to main memory and save to disk
	Surface::Ptr pTemp = SurfaceManager::Get().CreateSurface( createParams );
	GetD3DDevice()->GetRenderTargetData( m_Platform.GetSurface(), pTemp->m_Platform.GetSurface() );
	pTemp->m_Platform.SaveToDisk( pcFileName, D3DXIFF_DDS );

	// release scratch ram
	SurfaceManager::Get().ReleaseSurface( pTemp );
}

