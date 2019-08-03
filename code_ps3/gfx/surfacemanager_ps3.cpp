//------------------------------------------------------------
//!
//! \file gfx\surfacemanager_pc.cpp
//! PC implementation of the surface manager/allocator
//!
//------------------------------------------------------------

#include "gfx/surfacemanager.h"
#include "gfx/graphicsdevice.h"
#include "gfx/hardwarecaps.h"
#include "gfx/renderer.h"

//------------------------------------------------------------
//!
//! ctor
//!
//------------------------------------------------------------
SurfaceManager::SurfaceManager()
{
}

//------------------------------------------------------------
//!
//! dtor
//!
//------------------------------------------------------------
SurfaceManager::~SurfaceManager()
{
//	ntAssert_p( m_Platform.m_UsedTextureList.empty(),		("not all textures returned to the surface manager") );
//	ntAssert_p( m_Platform.m_UsedSurfaceList.empty(),		("not all surfaces returned to the surface manager") );
//	ntAssert_p( m_Platform.m_UsedRenderTargetList.empty(),	("not all rendertargets returned to the surface manager") );

	ClearCache( true, true, true );
}

//------------------------------------------------------------
//!
//! Create a texture pointer from an arbitary (tho 16byte aligned)
//! Area of RSX addressable XDRAM. Useful with video decoders
//! loading screens and the like.
//! Assumptions:
//!		texture pitch == width * bytesPerPixel
//!		texture is linear rather than swizzled.
//!		texture has no mip chain
//!		texture is 2D
//! Requirements:
//!		XDRAM is addressable by RSX
//!		XDRAM address is 16byte aligned
//!
//------------------------------------------------------------
Texture::Ptr SurfaceManagerPlatform::TextureFromMainMem(	uint32_t iWidth,
															uint32_t iHeight,
															uint32_t iPitch,
															Gc::TexFormat format,
															void* pTextureAddress )
{
	// note, allocations of types derived from IntrusivePtrCountedBase<> MUST use FW_NEW()
	// to match the chunk they will be released from
	Texture::Ptr pTexture = Texture::Ptr( FW_NEW Texture );

	pTexture->m_Platform.m_hTexture = GcTexture::Create(
		Gc::kTexture2D, 1, iWidth, iHeight, 1, format,
		false, iPitch, Gc::kUserBuffer );

	pTexture->m_Platform.m_hTexture->SetDataAddress( pTextureAddress );

	pTexture->m_iWidth				= pTexture->m_Platform.m_hTexture->GetWidth();
	pTexture->m_iHeight				= pTexture->m_Platform.m_hTexture->GetHeight();
	pTexture->m_iMipCount			= pTexture->m_Platform.m_hTexture->GetMipLevelCount();
	pTexture->m_Platform.m_pThis	= pTexture.Get();
	pTexture->m_Platform.m_bImplicit = true;
	pTexture->m_Platform.m_bCacheable = false;

	return pTexture;
}

Texture::Ptr SurfaceManagerPlatform::TextureFromMainMem(	GcTextureHandle	hHandle,
															void* pTextureAddress )
{
	// note, allocations of types derived from IntrusivePtrCountedBase<> MUST use FW_NEW()
	// to match the chunk they will be released from
	Texture::Ptr pTexture = Texture::Ptr( FW_NEW Texture );

	pTexture->m_Platform.m_hTexture = hHandle;
	pTexture->m_Platform.m_hTexture->SetDataAddress( pTextureAddress );

	pTexture->m_iWidth				= pTexture->m_Platform.m_hTexture->GetWidth();
	pTexture->m_iHeight				= pTexture->m_Platform.m_hTexture->GetHeight();
	pTexture->m_iMipCount			= pTexture->m_Platform.m_hTexture->GetMipLevelCount();
	pTexture->m_Platform.m_pThis	= pTexture.Get();
	pTexture->m_Platform.m_bImplicit = true;
	pTexture->m_Platform.m_bCacheable = false;

	return pTexture;
}

//------------------------------------------------------------
//!
//! CreateTexture, this is the simple cross platform version
//!
//------------------------------------------------------------
Texture::Ptr SurfaceManager::CreateTexture( uint32_t iWidth, uint32_t iHeight, GFXFORMAT format )
{
	Gc::TexFormat fmt = ConvertGFXFORMATToGCFORMAT( format );
	return CreateTexture( Texture::CreationStruct(iWidth, iHeight, fmt, 1) );
}

//------------------------------------------------------------
//!
//! Create Texture takes a creation struct and produces
//! a Texture::Ptr, searches the free texture cache for an exact
//! match.
//!
//------------------------------------------------------------
Texture::Ptr SurfaceManager::CreateTexture( const Texture::CreationStruct& creationStruct )
{
	// special case, just convert into our texture class
	if( creationStruct.bFromRenderBuffer )
	{
		// note, allocations of types derived from IntrusivePtrCountedBase<> MUST use FW_NEW()
		// to match the chunk they will be released from
		Texture::Ptr pTexture			= Texture::Ptr( FW_NEW Texture );
		pTexture->m_Platform.m_hTexture = GcTexture::Create( creationStruct.hRenderBuffer );
		
		pTexture->m_iWidth				= pTexture->m_Platform.m_hTexture->GetWidth();
		pTexture->m_iHeight				= pTexture->m_Platform.m_hTexture->GetHeight();
		pTexture->m_iMipCount			= pTexture->m_Platform.m_hTexture->GetMipLevelCount();
		pTexture->m_Platform.m_pThis	= pTexture.Get();

		if( creationStruct.bImplicit )
		{
			pTexture->m_Platform.m_bImplicit = true;
			pTexture->m_Platform.m_bCacheable = false;
		}
		else
		{
			pTexture->m_Platform.m_bImplicit = false;
			pTexture->m_Platform.m_bCacheable = creationStruct.bCacheable;

			// push it into the used list
			m_Platform.m_UsedTextureList.push_back( pTexture );
		}

		return pTexture;
	}

	// find a texture of these dimensions and format
	if (creationStruct.bCacheable)
	{
		for(	SurfaceManagerPlatform::TextureList::iterator obIt = m_Platform.m_FreeTextureList.begin(); 
				obIt != m_Platform.m_FreeTextureList.end(); 
				++obIt )
		{
			if	(
				( (*obIt)->GetWidth()				== creationStruct.iWidth) &&
				( (*obIt)->GetHeight()				== creationStruct.iHeight) && 
				( (*obIt)->GetType()				== creationStruct.eType) && 
				( (*obIt)->m_Platform.GetGCFormat()	== creationStruct.eFormat) &&
				( (*obIt)->GetMipCount()			== creationStruct.iMipLevels )
				)
			{
				// get the texture pointer
				Texture::Ptr pTexture = (*obIt);
				// push it into the used list
				m_Platform.m_UsedTextureList.push_back( pTexture );
				// and remove it from the free list
				m_Platform.m_FreeTextureList.erase( obIt );

				return pTexture;
			}
		}
	}

	// create one

	// remember, pitch is in BYTES, 0 pitch means we want a swizzled texture.
	u_int iPitch = creationStruct.bSwizzled ? 0 : ((creationStruct.iWidth * Gc::GetBitsPerPixel( creationStruct.eFormat )) >> 3);

	Gc::TexType eType = Gc::kTexture2D;
	if (creationStruct.eType == TT_3D_TEXTURE)
		eType = Gc::kTexture3D;
	else if (creationStruct.eType == TT_CUBE_TEXTURE)
		eType = Gc::kTextureCube;

	// note, allocations of types derived from IntrusivePtrCountedBase<> MUST use FW_NEW()
	// to match the chunk they will be released from
	Texture::Ptr pTexture			= Texture::Ptr( FW_NEW Texture );

	pTexture->m_Platform.m_hTexture = GcTexture::Create(
		eType,
		creationStruct.iMipLevels,
		creationStruct.iWidth,
		creationStruct.iHeight,
		creationStruct.iDepth,
		creationStruct.eFormat,
		false, // unsure what signed signfies in Gc, this is unsupported at the moment
		iPitch,
		Gc::kStaticBuffer // we always use static buffers for the moment
	);

	pTexture->m_iWidth				= pTexture->m_Platform.m_hTexture->GetWidth();
	pTexture->m_iHeight				= pTexture->m_Platform.m_hTexture->GetHeight();
	pTexture->m_iDepth				= pTexture->m_Platform.m_hTexture->GetDepth();
	pTexture->m_iMipCount			= pTexture->m_Platform.m_hTexture->GetMipLevelCount();
	pTexture->m_Platform.m_pThis	= pTexture.Get();
	pTexture->m_Platform.m_bImplicit = false;
	pTexture->m_Platform.m_bCacheable = creationStruct.bCacheable;

	// push it into the used list
	m_Platform.m_UsedTextureList.push_back( pTexture );

	TRACK_GFX_ALLOC_PROC_TEX( pTexture );

	return pTexture;
}

//------------------------------------------------------------
//!
//! Create Texture takes a creation struct and produces
//! a Texture::Ptr, searches the free texture cache for an exact
//! match.
//!
//------------------------------------------------------------
void SurfaceManager::ReleaseTexture( Texture::Ptr pTexture )
{
	if( !pTexture->m_Platform.m_bImplicit )
	{
		// find this texture in the used texture list
		SurfaceManagerPlatform::TextureList::iterator texIt = ntstd::find( m_Platform.m_UsedTextureList.begin(), m_Platform.m_UsedTextureList.end(), pTexture );
	
		if ( texIt == m_Platform.m_UsedTextureList.end() )
		{
			texIt = ntstd::find( m_Platform.m_FreeTextureList.begin(), m_Platform.m_FreeTextureList.end(), pTexture );

			if ( texIt != m_Platform.m_FreeTextureList.end() )
			{
				ntError_p( 0, ("pTexture: Width %i Height %i released twice\n", pTexture->GetWidth(), pTexture->GetHeight() ) );
			}
			else
			{
				ntError_p( 0, ("pTexture: Width %i Height %i released but not currently in use\n", pTexture->GetWidth(), pTexture->GetHeight() ) );
			}
		}

		// should we cache this texture or just remove it entirely?
		if( pTexture->m_Platform.m_bCacheable )
		{
			// remove it from the used list
			m_Platform.m_UsedTextureList.erase( texIt );
			// and add  it to the free list
			m_Platform.m_FreeTextureList.push_back( pTexture );

		} else
		{
			// remove it from the used list
			m_Platform.m_UsedTextureList.erase( texIt );

			TRACK_GFX_FREE_PROC_TEX(pTexture);
		}
	}
}

//------------------------------------------------------------
//!
//! CreateRenderTarget, this is the simple cross platform version
//!
//------------------------------------------------------------
RenderTarget::Ptr SurfaceManager::CreateRenderTarget( uint32_t iWidth, uint32_t iHeight, GFXFORMAT format, bool bTexturable, GFXAAMODE aamode )
{
	return CreateRenderTarget( RenderTarget::CreationStruct(iWidth, iHeight, format, aamode ) );
}

//------------------------------------------------------------
//!
//! Create RenderTarget takes a creation struct and produces
//! a Texture::Ptr, searches the free texture cache for an exact
//! match.
//!
//------------------------------------------------------------
RenderTarget::Ptr SurfaceManager::CreateRenderTarget( const RenderTarget::CreationStruct& creationStruct )
{
	ntAssert_p( (creationStruct.bMainMemory != true) || (creationStruct.pUserMemory != NULL), ("Can't create a rendert target in main memory if some User Buffer for it has not been pre-allocated"));

	GcRenderBufferHandle hHandle;

	bool bDisableUserMemRT = false;
	bool bIsUserMemRT = false;

	if (creationStruct.pUserMemory)
	{
		if (bDisableUserMemRT)
		{
			bIsUserMemRT = true;
		}
		else
		{
			// select memory context
			Gc::MemoryContext memoryContext = Gc::kVideoMemory;
			if ( creationStruct.bMainMemory )
				memoryContext = Gc::kHostMemory;

			// we're creating from pre-exisiting memory, fake up a handle and return the one below
			hHandle = GcRenderBuffer::Create( 
				creationStruct.iWidth, 
				creationStruct.iHeight,
				ConvertGFXFORMATToGCBUFFERFORMAT( creationStruct.eFormat ), 
				ConvertGFXAAMODEToGCMULTISAMPLEMODE( creationStruct.eAAMode ), 
				Gc::kUserBuffer, NULL, memoryContext
			);

			hHandle->SetDataAddress( creationStruct.pUserMemory );
			hHandle->SetPitch( creationStruct.iUserMemPitch );
		}
	}
	else
	{
		hHandle = creationStruct.hSrcHandle;
	}

	if (hHandle)
	{
		// when creating from a handle we never cache, these should also never be released

		// note, allocations of types derived from IntrusivePtrCountedBase<> MUST use FW_NEW()
		// to match the chunk they will be released from
		RenderTarget::Ptr pRenderTarget	= RenderTarget::Ptr( FW_NEW RenderTarget );

		pRenderTarget->m_Platform.m_bFromHandle = true;
		pRenderTarget->m_Platform.m_bCacheable = false;
		pRenderTarget->m_Platform.m_hRenderBuffer = hHandle;

		pRenderTarget->m_iWidth		= hHandle->GetWidth();
		pRenderTarget->m_iHeight	= hHandle->GetHeight();

		return pRenderTarget;
	}

	// find a texture of these dimensions and format
	if (creationStruct.bCacheable)
	{
		for(	SurfaceManagerPlatform::RenderTargetList::iterator obIt = m_Platform.m_FreeRenderTargetList.begin(); 
				obIt != m_Platform.m_FreeRenderTargetList.end(); 
				++obIt )
		{
			if	(
				( (*obIt)->GetWidth()	== creationStruct.iWidth) &&
				( (*obIt)->GetHeight()	== creationStruct.iHeight) && 
				( (*obIt)->GetFormat()	== creationStruct.eFormat) &&
				( (*obIt)->GetAAMode()  == creationStruct.eAAMode)
				)
			{
				// get the texture pointer
				RenderTarget::Ptr pRenderTarget = (*obIt);
				
				// push it into the used list
				m_Platform.m_UsedRenderTargetList.push_back( pRenderTarget );
				// and remove it from the free list
				m_Platform.m_FreeRenderTargetList.erase( obIt );

				return pRenderTarget;
			}
		}
	}

	// note, allocations of types derived from IntrusivePtrCountedBase<> MUST use FW_NEW()
	// to match the chunk they will be released from
	RenderTarget::Ptr pRenderTarget	= RenderTarget::Ptr( FW_NEW RenderTarget );
	pRenderTarget->m_Platform.m_bFromHandle = false;

	uint32_t numBuffers = (m_Platform.m_UsedRenderTargetList.size() + m_Platform.m_FreeRenderTargetList.size());

	// currently we can only have 8 real render buffers (with tiling for colour and depth compression)
	// past the 8th we have to use normal texture, converted into render targets
	bool bUseTexture = ( numBuffers >= 7 ) ? true : false;
	if (creationStruct.bForceTexture)
		bUseTexture = true;

	if( bUseTexture )
	{
		int iWidth = creationStruct.iWidth;
		int iHeight = creationStruct.iHeight;

		// if we're supposed to be MSAA but not tiled (god only knows why you want to piss away
		// your performance) then we silently change the allocation size. The Gc texture
		// to render buffer conversion will ensure it has the 'correct' dimensions.
		if (creationStruct.eAAMode == GAA_MULTISAMPLE_4X || creationStruct.eAAMode == GAA_MULTISAMPLE_ORDERED_GRID_4X)
		{
			iWidth *= 2;
			iHeight *= 2;
		}

		Gc::TexFormat fmt = ConvertGFXFORMATToGCFORMAT( creationStruct.eFormat );
		u_int iPitch = (iWidth * Gc::GetBitsPerPixel( fmt )) >> 3;

		// render buffer must be 64 byte aligned
		iPitch = ROUND_POW2(iPitch,64);
		GcTextureHandle hTex = GcTexture::Create(	Gc::kTexture2D,
													1,
													iWidth,
													iHeight,
													1,
													fmt,
													false, // unsure what signed signfies in Gc, this is unsupported at the moment
													iPitch,
													Gc::kStaticBuffer // we always use static buffers for the moment
												);

		pRenderTarget->m_Platform.m_hRenderBuffer = GcRenderBuffer::Create( hTex->GetMipLevel(0) );

	} else
	{
		pRenderTarget->m_Platform.m_hRenderBuffer = GcRenderBuffer::Create(
			creationStruct.iWidth,
			creationStruct.iHeight,
			ConvertGFXFORMATToGCBUFFERFORMAT( creationStruct.eFormat ),
			ConvertGFXAAMODEToGCMULTISAMPLEMODE( creationStruct.eAAMode )
		);
	}

	pRenderTarget->m_iWidth					= creationStruct.iWidth;
	pRenderTarget->m_iHeight				= creationStruct.iHeight;
	pRenderTarget->m_Platform.m_bCacheable	= creationStruct.bCacheable;

	// push it into the used list
	if (bIsUserMemRT == false)
	{
		m_Platform.m_UsedRenderTargetList.push_back( pRenderTarget );
		TRACK_GFX_ALLOC_RT( pRenderTarget );
	}

	return pRenderTarget;

}

//------------------------------------------------------------
//!
//! Create Render Target takes a creation struct and produces
//! a Render Target::Ptr, searches the free texture cache for an exact
//! match.
//!
//------------------------------------------------------------
void SurfaceManager::ReleaseRenderTarget( RenderTarget::Ptr pRenderTarget )
{
	ntAssert_p( pRenderTarget->m_Platform.m_bFromHandle == false, ("Should not release this type of rendertarget"));

	// find this texture in the used texture list
	SurfaceManagerPlatform::RenderTargetList::iterator texIt = ntstd::find( m_Platform.m_UsedRenderTargetList.begin(), m_Platform.m_UsedRenderTargetList.end(), pRenderTarget );

	if ( texIt == m_Platform.m_UsedRenderTargetList.end() )
	{
		texIt = ntstd::find( m_Platform.m_FreeRenderTargetList.begin(), m_Platform.m_FreeRenderTargetList.end(), pRenderTarget );

		if ( texIt != m_Platform.m_FreeRenderTargetList.end() )
		{
			ntError_p( 0, ("pRenderTarget: Width %i Height %i released twice\n", pRenderTarget->GetWidth(), pRenderTarget->GetHeight() ) );
		}
		else
		{
			ntError_p( 0, ("pRenderTarget: Width %i Height %i released but not currently in use\n", pRenderTarget->GetWidth(), pRenderTarget->GetHeight() ) );
		}
	}

	// should we cache this texture or just remove it entirely?
	if( pRenderTarget->m_Platform.m_bCacheable )
	{
		// remove it from the used list
		m_Platform.m_UsedRenderTargetList.erase( texIt );
		// and add  it to the free list
		m_Platform.m_FreeRenderTargetList.push_back( pRenderTarget );

	} else
	{
		// remove it from the used list
		m_Platform.m_UsedRenderTargetList.erase( texIt );

		TRACK_GFX_FREE_RT( pRenderTarget );
	}
}
//------------------------------------------------------------
//!
//! Clear any caches
//!
//------------------------------------------------------------
void SurfaceManager::ClearCache( bool bTexture, bool, bool bRenderTarget)
{
	if( bTexture )
	{
		m_Platform.m_FreeTextureList.clear();
	}

	if( bRenderTarget )
	{
		m_Platform.m_FreeRenderTargetList.clear();
	}

}
