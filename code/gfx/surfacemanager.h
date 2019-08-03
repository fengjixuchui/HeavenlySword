//------------------------------------------------------------
//!
//! \file gfx\surfacemanager.h
//! Cross platform bits of the surfacemanager
//!
//------------------------------------------------------------
#if !defined( GFX_SURFACEMANAGER_H )
#define GFX_SURFACEMANAGER_H

#if !defined( GFX_GFXFORMAT_H )
#include "gfx/gfxformat.h"
#endif

#if !defined( GFX_SURFACE_H )
#include "gfx/surface.h"
#endif

#if !defined( GFX_RENDERTARGET_H )
#include "gfx/rendertarget.h"
#endif

#if !defined( GFX_TEXTURE_H )
#include "gfx/texture.h"
#endif

#if defined( PLATFORM_PC )
#include "gfx/surfacemanager_pc.h"
#elif defined( PLATFORM_PS3 )
#include "gfx/surfacemanager_ps3.h"
#endif


//------------------------------------------------------------
//!
//!
//------------------------------------------------------------
class SurfaceManager : public Singleton<SurfaceManager>
{
public:
	//! ctor.
	SurfaceManager();
	//! dtor.
	~SurfaceManager();

	//! cross platform non render target texture creation.
	Texture::Ptr CreateTexture( uint32_t iWidth, uint32_t iHeight, GFXFORMAT format );

	//! platform specific non render target texture creation.
	Texture::Ptr CreateTexture( const Texture::CreationStruct& creationStruct );

	//! release texture.
	void ReleaseTexture( Texture::Ptr pTexture );

#ifdef PLATFORM_PC
	Surface::Ptr CreateSurface( uint32_t iWidth, uint32_t iHeight, GFXFORMAT format );
	Surface::Ptr CreateSurface( const Surface::CreationStruct& creationStruct );
	void ReleaseSurface( Surface::Ptr pTexture );
#endif

	//! cross platform render target creation (optionally textureable).
	RenderTarget::Ptr CreateRenderTarget( uint32_t iWidth, uint32_t iHeight, GFXFORMAT format, bool bTexturable = true, GFXAAMODE aamode = GAA_MULTISAMPLE_NONE);

	//! platform specific render target creation.
	RenderTarget::Ptr CreateRenderTarget( const RenderTarget::CreationStruct& creationStruct );

	//! release render target.
	void ReleaseRenderTarget( RenderTarget::Ptr pTexture );

	//! clears any caches.
	void ClearCache( bool bTexture = true, bool bSurface = true, bool bRenderTarget = true);

	SurfaceManagerPlatform m_Platform;
};

#endif
