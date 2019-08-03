//------------------------------------------------------------
//!
//! \file gfx\surfacemanager_pc.h
//! PC specific bits of the surfacemanager
//!
//------------------------------------------------------------


#if !defined( GFX_SURFACEMANAGER_PS3_H )
#define GFX_SURFACEMANAGER_PS3_H

//------------------------------------------------------------
//!
//!
//------------------------------------------------------------
class SurfaceManagerPlatform
{
public:

	// for loading screens / video codecs and the like.
	static Texture::Ptr TextureFromMainMem(	uint32_t iWidth, uint32_t iHeight, uint32_t iPitch, 
											Gc::TexFormat format,
											void* pTextureAddress );

	static Texture::Ptr TextureFromMainMem(	GcTextureHandle	hHandle,
											void* pTextureAddress );

	typedef ntstd::List< Texture::Ptr, Mem::MC_GFX >			TextureList;			//!< List of ref counted texture pointers
	typedef ntstd::List< RenderTarget::Ptr, Mem::MC_GFX >	RenderTargetList;		//!< List of ref counted render target pointers

	TextureList m_FreeTextureList;										//!< List of free unused textures
	TextureList m_UsedTextureList;										//!< List of current in use textures

	RenderTargetList m_FreeRenderTargetList;							//!< List of free unused render targets
	RenderTargetList m_UsedRenderTargetList;							//!< List of used render targets
};

#endif // GFX_SURFACEMANAGER_PS3_H
