#if !defined(GFX_RENDERTARGET_H)
#define GFX_RENDERTARGET_H
//-----------------------------------------------------
//!
//!	\file gfx\rendertarget.h
//! A surface that can be rendered to, some render 
//! targets can also be textured from, this encapsulates
//! which state (render to or from) a surface is currently
//! in
//!
//----------------------------------------------------

#if !defined( FW_STD_INTRUSIVE_PTR_H )
#include <Fw/FwStd/FwStdIntrusivePtr.h>
#endif

#if !defined(GFX_GFXFORMAT_H)
#include "gfx/gfxformat.h"
#endif

#if defined( PLATFORM_PC )
#include "gfx/rendertarget_pc.h"
#elif defined( PLATFORM_PS3 )
#include "gfx/rendertarget_ps3.h"
#endif

#ifndef GFX_TEXTURE_H
#include "gfx/texture.h"
#endif

#ifndef GFX_SURFACE_H
#include "gfx/surface.h"
#endif

//-----------------------------------------------------
//!
//! A platform independent render target texture object
//! this represents both the target and its use as a 
//! texture after you have finished with it as a texture.
//! Some render target format can't be used as texture. 
//! Any attempt to set these into texture mode will fail.
//!
//! Any 'funny' business using it as both a texture and 
//! a target at the same time, is not cross-platform and
//! should use platform specific code if a particular 
//! platform supports it (currently none reliable)
//!
//-----------------------------------------------------
class RenderTarget : public FwStd::IntrusivePtrCountedBase< RenderTarget >
{
public:	
	friend class RenderTargetPlatform;
	friend class SurfaceManager;
	friend class SurfaceManagerPlatform;

	typedef RenderTargetPlatform::CreationStruct CreationStruct; 
	typedef	FwStd::IntrusivePtr<RenderTarget>	Ptr;

	//! use when you don't want a render target (clearing MRTs targets for example)
	enum NONE_ENUM{ NONE };

	//! the width of this render target
	uint32_t GetWidth() const
	{ 
		return m_iWidth; 
	}
	//! the height of this render target
	uint32_t GetHeight() const
	{ 
		return m_iHeight; 
	}
	//! returns a generic format you may get a platform specific marker
	GFXFORMAT GetFormat() const;

	//! returns aa mode
	GFXAAMODE GetAAMode() const;

	//! Gets the texture object
	Texture::Ptr GetTexture();

#ifdef PLATFORM_PC
	//! Gets the surface
	Surface::Ptr GetSurface();
#endif

	//! platform dependent stuff
	RenderTargetPlatform m_Platform;

	//! dtor
	~RenderTarget();

	//! debug dumping
	void SaveToDisk( const char* pcFileName );

	//! debug method to help estimate our VRAM usage
	inline uint32_t CalculateVRAMFootprint()
	{
		if (GetAAMode() == GAA_MULTISAMPLE_4X || GetAAMode() == GAA_MULTISAMPLE_ORDERED_GRID_4X)
			return GFXFormat::CalculateVRAMFootprint( GetFormat(), m_iWidth, m_iHeight ) * 4;
		else
			return GFXFormat::CalculateVRAMFootprint( GetFormat(), m_iWidth, m_iHeight );
	}

private:
	uint32_t m_iWidth;				//!< width of the render target
	uint32_t m_iHeight;				//!< height of the render target
};

#endif // end GFX_RENDERTARGET_H
