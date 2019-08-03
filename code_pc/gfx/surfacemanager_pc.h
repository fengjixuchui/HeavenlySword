//------------------------------------------------------------
//!
//! \file gfx\surfacemanager_pc.h
//! PC specific bits of the surfacemanager
//!
//------------------------------------------------------------


#if !defined( GFX_SURFACEMANAGER_PC_H )
#define GFX_SURFACEMANAGER_PC_H

//------------------------------------------------------------
//!
//!
//------------------------------------------------------------
class SurfaceManagerPlatform
{
public:
	typedef ntstd::List< Texture::Ptr >			TextureList;			//!< List of ref counted texture pointers
	typedef ntstd::List< Surface::Ptr >			SurfaceList;			//!< List of ref counted surface pointers
	typedef ntstd::List< RenderTarget::Ptr >	RenderTargetList;		//!< List of ref counted render target pointers

	TextureList m_FreeTextureList;										//!< List of free unused textures
	TextureList m_UsedTextureList;										//!< List of current in use textures

	SurfaceList m_FreeSurfaceList;										//!< List of free unused independently created surfaces
	SurfaceList m_UsedSurfaceList;										//!< List of current in use surfaces (excluding implicits)

	RenderTargetList m_FreeRenderTargetList;							//!< List of free unused render targets
	RenderTargetList m_UsedRenderTargetList;							//!< List of used render targets

};

#endif