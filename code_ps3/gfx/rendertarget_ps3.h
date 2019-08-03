#if !defined(GFX_RENDERTARGET_PS3_H)
#define GFX_RENDERTARGET_PS3_H
//-----------------------------------------------------
//!
//!	\file gfx\rendertarget_ps3.h
//! platform specific bit for PS3 of render target
//!
//-----------------------------------------------------

#include <Gc/GcRenderBuffer.h>

class RenderTarget;

//-----------------------------------------------------
//!
//! The Ps3 specific bits of a render target
//!
//-----------------------------------------------------
class RenderTargetPlatform
{
public:

	//! pointer back to container
	RenderTarget*	pThis;

	friend class RenderTarget;
	friend class SurfaceManager;
	friend class SurfaceManagerPlatform;

	struct CreationStruct
	{
	friend class SurfaceManager;

		//!< Create potentialled cached render target / renderable texture
		CreationStruct( uint32_t width, uint32_t height, 
				GFXFORMAT eformat, GFXAAMODE aamode = GAA_MULTISAMPLE_NONE ) :
			iWidth( width ),
			iHeight( height ),
			eFormat( eformat ),
			eAAMode( aamode ),
			pUserMemory( NULL ),
			bCacheable( true ),
			bForceTexture( false ),
			bMainMemory( false ){}

		//!< Create non cached handle from existing render buffer (such as Gc back buffer)
		CreationStruct( GcRenderBufferHandle hHandle ) :
			hSrcHandle( hHandle ),
			pUserMemory( NULL ),
			bCacheable( false ),
			bForceTexture( false ),
			bMainMemory( false ){}

		//!< Create non cached handle from existing VRAM
		CreationStruct( void* pMem, uint32_t iMemPitch, int32_t width, uint32_t height, 
				GFXFORMAT eformat, GFXAAMODE aamode = GAA_MULTISAMPLE_NONE, bool bXDRMemory = false  ) :
			iWidth( width ),
			iHeight( height ),
			eFormat( eformat ),
			eAAMode( aamode ),
			pUserMemory( pMem ),
			iUserMemPitch( iMemPitch ),
			bCacheable( false ),
			bForceTexture( false ),
			bMainMemory( bXDRMemory ){}

	private:
		uint32_t	iWidth;					//!< width of render target
		uint32_t	iHeight;				//!< height of render target
		GFXFORMAT	eFormat;				//!< Neutral format of the render target
		GFXAAMODE	eAAMode;				//!< AA mode on this render target
		GcRenderBufferHandle hSrcHandle;	//!< Create non cached handle from existing render buffer (such as Gc back buffer)
		void*		pUserMemory;			//!< Create non cached handle from existing VRAM
		uint32_t	iUserMemPitch;
		bool		bCacheable;				//!< is this render target cachable when released?
		bool		bForceTexture;			//!< is this render target cachable when released?
		bool		bMainMemory;			//!< does this render target live in main memory?
	};

	//! Get Gc format
	Gc::BufferFormat GetGCFormat() const
	{
		return m_hRenderBuffer->GetFormat();
	}

	//! Get Gc AA mode
	Gc::MultisampleMode GetGCAAMode() const
	{
		return m_hRenderBuffer->GetMultisampleMode();
	}

	//! Get the render target interface
	GcRenderBufferHandle GetRenderBuffer()
	{
		return m_hRenderBuffer;
	}

private:
	GcRenderBufferHandle	m_hRenderBuffer;
	bool					m_bFromHandle;
	bool					m_bCacheable;
};

#endif //GFX_RENDERTARGET_PS3_H
