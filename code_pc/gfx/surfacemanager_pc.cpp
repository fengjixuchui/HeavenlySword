//------------------------------------------------------------
//!
//! \file gfx\surfacemanager_pc.cpp
//! PC implementation of the surface manager/allocator
//!
//------------------------------------------------------------

#include "gfx/surfacemanager.h"
#include "gfx/dxerror_pc.h"
#include "gfx/graphicsdevice.h"
#include "gfx/renderer.h"
#include "gfx/hardwarecaps.h"

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
//! CreateTexture, this is the simple cross platform version
//!
//------------------------------------------------------------
Texture::Ptr SurfaceManager::CreateTexture( uint32_t iWidth, uint32_t iHeight, GFXFORMAT format )
{
	D3DFORMAT d3dFmt = ConvertGFXFORMATToD3DFORMAT( format );
	ntAssert_p( d3dFmt != D3DFMT_UNKNOWN, ("Unknown Texture format") );
	return CreateTexture( Texture::CreationStruct(iWidth, iHeight, d3dFmt, 1) );
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
	// we allocate render targets (and just render targets) from POOL_DEFAULT
	D3DPOOL ePool = D3DPOOL_MANAGED;
	uint32_t iUsage = 0;

	// special case, just convert into our texture class
	if( creationStruct.bFromTexture )
	{
		D3DSURFACE_DESC stDesc;
		dxerror( creationStruct.pTexture->GetLevelDesc( 0, &stDesc ) );

		// note, allocations of types derived from IntrusivePtrCountedBase<> MUST use FW_NEW()
		// to match the chunk they will be released from
		Texture::Ptr pTexture				=		Texture::Ptr( FW_NEW Texture );

		pTexture->m_iWidth					=		stDesc.Width;
		pTexture->m_iHeight					=		stDesc.Height;
		pTexture->m_iMipCount				=		creationStruct.pTexture->GetLevelCount();
		pTexture->m_Platform.m_eFormat		=		stDesc.Format;
		pTexture->m_Platform.m_p2DTexture	=		creationStruct.pTexture;
		pTexture->m_Platform.m_ePool		=		stDesc.Pool;
		pTexture->m_Platform.m_eType		=		TT_2D_TEXTURE;
		pTexture->m_Platform.m_pThis		=		pTexture.Get();

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

	// find a texture of these dimensions and format from the cache
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
				( (*obIt)->GetMipCount()			== creationStruct.iMipLevels ) &&
				( (*obIt)->m_Platform.GetDXFormat()	== creationStruct.eFormat) &&
				( (*obIt)->m_Platform.GetPool()		== ePool )
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
	
	// note, allocations of types derived from IntrusivePtrCountedBase<> MUST use FW_NEW()
	// to match the chunk they will be released from
	Texture::Ptr pTexture = Texture::Ptr( FW_NEW Texture );

	switch( creationStruct.eType )
	{
	case TT_2D_TEXTURE:
		{
			dxerror_p( 
					GetD3DDevice()->CreateTexture( 
					creationStruct.iWidth, creationStruct.iHeight, creationStruct.iMipLevels, iUsage, creationStruct.eFormat, ePool, 
					&pTexture->m_Platform.m_p2DTexture, 0 ),
					( "failed to create texture" ) );
		}
		break;

	case TT_3D_TEXTURE:
		{
			dxerror_p( 
					GetD3DDevice()->CreateVolumeTexture(
					creationStruct.iWidth, creationStruct.iHeight, creationStruct.iDepth, creationStruct.iMipLevels, iUsage, creationStruct.eFormat, ePool, 
					&pTexture->m_Platform.m_p3DTexture, 0 ),
					( "failed to create texture" ) );			
		}
		break;

	case TT_CUBE_TEXTURE:
		{
			dxerror_p( 
					GetD3DDevice()->CreateCubeTexture(
					creationStruct.iWidth, creationStruct.iMipLevels, iUsage, creationStruct.eFormat, ePool, 
					&pTexture->m_Platform.m_pCubeTexture, 0 ),
					( "failed to create texture" ) );	
		}
		break;
	}

	pTexture->m_iWidth				= creationStruct.iWidth;
	pTexture->m_iHeight				= creationStruct.iHeight;
	pTexture->m_iDepth				= creationStruct.iDepth;
	pTexture->m_iMipCount			= creationStruct.iMipLevels;
	pTexture->m_Platform.m_eFormat	= creationStruct.eFormat;
	pTexture->m_Platform.m_ePool	= ePool;
	pTexture->m_Platform.m_bImplicit = false;
	pTexture->m_Platform.m_bCacheable = creationStruct.bCacheable;
	pTexture->m_Platform.m_pThis	= pTexture.Get();
	pTexture->m_Platform.m_eType	= creationStruct.eType;

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
//! CreateSurface, this is the simple cross platform version
//!
//------------------------------------------------------------
Surface::Ptr SurfaceManager::CreateSurface( uint32_t iWidth, uint32_t iHeight, GFXFORMAT format )
{
	D3DFORMAT d3dFmt = ConvertGFXFORMATToD3DFORMAT( format );
	ntAssert_p( d3dFmt != D3DFMT_UNKNOWN, ("Unknown Texture format") );
	return CreateSurface( Surface::CreationStruct(iWidth, iHeight, d3dFmt) );
}

//------------------------------------------------------------
//!
//! Create Surface takes a creation struct and produces
//! a Surface::Ptr, searches the free Surface cache for an exact
//! match.
//!
//------------------------------------------------------------
Surface::Ptr SurfaceManager::CreateSurface( const Surface::CreationStruct& creationStruct )
{
	// special case, just convert into our surface class. 
	if( creationStruct.bFromSurface )
	{
		D3DSURFACE_DESC stDesc;
		dxerror( creationStruct.pSurface->GetDesc( &stDesc ) );

		// note, allocations of types derived from IntrusivePtrCountedBase<> MUST use FW_NEW()
		// to match the chunk they will be released from
		Surface::Ptr pSurface			=		Surface::Ptr( FW_NEW Surface );

		pSurface->m_iWidth				=		stDesc.Width;
		pSurface->m_iHeight				=		stDesc.Height;
		pSurface->m_Platform.m_eFormat	=		stDesc.Format;
		pSurface->m_Platform.m_pSurface =		creationStruct.pSurface;
		pSurface->m_Platform.m_ePool	=		stDesc.Pool;
		if( creationStruct.bImplicit )
		{
			pSurface->m_Platform.m_bImplicit =	true;
			pSurface->m_Platform.m_bCacheable =	false;
		}
		else
		{
			pSurface->m_Platform.m_bImplicit =	false;
			pSurface->m_Platform.m_bCacheable =	creationStruct.bCacheable;

			// push it into the used list
			m_Platform.m_UsedSurfaceList.push_back( pSurface );
		}


		return pSurface;
	}

	if (creationStruct.bCacheable)
	{
		// find a texture of these dimensions and format
		for(	SurfaceManagerPlatform::SurfaceList::iterator obIt = m_Platform.m_FreeSurfaceList.begin(); 
				obIt != m_Platform.m_FreeSurfaceList.end(); 
				++obIt )
		{
			if	(
				( (*obIt)->GetWidth()				== creationStruct.iWidth ) &&
				( (*obIt)->GetHeight()				== creationStruct.iHeight ) && 
				( (*obIt)->m_Platform.GetDXFormat()	== creationStruct.eFormat ) &&
				( (*obIt)->m_Platform.GetPool()		== creationStruct.ePool )
				)
			{
				// get the surface pointer
				Surface::Ptr pSurface = (*obIt);
				// push it into the used list
				m_Platform.m_UsedSurfaceList.push_back( pSurface );
				// and remove it from the free list
				m_Platform.m_FreeSurfaceList.erase( obIt );

				return pSurface;
			}
		}
	}

	// create one
	IDirect3DSurface9* pD3DSurface;
	switch( creationStruct.eFormat )
	{
	case D3DFMT_D15S1:
	case D3DFMT_D16:
	case D3DFMT_D16_LOCKABLE:
	case D3DFMT_D24FS8:
	case D3DFMT_D24S8:
	case D3DFMT_D24X4S4:
	case D3DFMT_D24X8:
	case D3DFMT_D32:
	case D3DFMT_D32F_LOCKABLE:
		// under Dx9 depth stencil are always POOL_DEFAULT, so we don't pass in ePool
		dxerror_p( 
			GraphicsDevice::Get().m_Platform.Get()->CreateDepthStencilSurface( 
			creationStruct.iWidth, creationStruct.iHeight, creationStruct.eFormat, D3DMULTISAMPLE_NONE, 0, TRUE, 
			&pD3DSurface, 0 ),
			( "failed to create surface" ) );
		break;
	default:
		dxerror_p( 
			GraphicsDevice::Get().m_Platform.Get()->CreateOffscreenPlainSurface( 
			creationStruct.iWidth, creationStruct.iHeight, creationStruct.eFormat, creationStruct.ePool, 
			&pD3DSurface, 0 ),
			( "failed to create texture" ) );
		break;
	}

	// note, allocations of types derived from IntrusivePtrCountedBase<> MUST use FW_NEW()
	// to match the chunk they will be released from
	Surface::Ptr pSurface			= Surface::Ptr( FW_NEW Surface );

	pSurface->m_iWidth				= creationStruct.iWidth;
	pSurface->m_iHeight				= creationStruct.iHeight;
	pSurface->m_Platform.m_eFormat	= creationStruct.eFormat;
	pSurface->m_Platform.m_ePool	= creationStruct.ePool;
	pSurface->m_Platform.m_pSurface = pD3DSurface;
	pSurface->m_Platform.m_bImplicit =	false;
	pSurface->m_Platform.m_bCacheable =	creationStruct.bCacheable;

	// push it into the used list
	m_Platform.m_UsedSurfaceList.push_back( pSurface );

	TRACK_GFX_ALLOC_RT( pSurface );

	return pSurface;

}

//------------------------------------------------------------
//!
//! Release a surface
//!
//------------------------------------------------------------
void SurfaceManager::ReleaseSurface( Surface::Ptr pSurface )
{
	if( !pSurface->m_Platform.m_bImplicit )
	{
		// find this surface in the used surface list
		SurfaceManagerPlatform::SurfaceList::iterator surfIt = ntstd::find( m_Platform.m_UsedSurfaceList.begin(), m_Platform.m_UsedSurfaceList.end(), pSurface );
		ntError_p( surfIt != m_Platform.m_UsedSurfaceList.end(), ("Surface: Width %i Height %i released but not currently in use\n", pSurface->GetWidth(), pSurface->GetHeight() ) );

		if ( surfIt == m_Platform.m_UsedSurfaceList.end() )
		{
			surfIt = ntstd::find( m_Platform.m_FreeSurfaceList.begin(), m_Platform.m_FreeSurfaceList.end(), pSurface );

			if ( surfIt != m_Platform.m_FreeSurfaceList.end() )
			{
				ntError_p( 0, ("pSurface: Width %i Height %i released twice\n", pSurface->GetWidth(), pSurface->GetHeight() ) );
			}
			else
			{
				ntError_p( 0, ("pSurface: Width %i Height %i released but not currently in use\n", pSurface->GetWidth(), pSurface->GetHeight() ) );
			}
		}

		// should we cache this texture or just remove it entirely?
		if( pSurface->m_Platform.m_bCacheable )
		{
			// remove it from the used list
			m_Platform.m_UsedSurfaceList.erase( surfIt );
			// and add  it to the free list
			m_Platform.m_FreeSurfaceList.push_back( pSurface );

		} else
		{
			// remove it from the used list
			m_Platform.m_UsedSurfaceList.erase( surfIt );

			TRACK_GFX_FREE_RT(pSurface);
		}
	}
}

//------------------------------------------------------------
//!
//! CreateRenderTarget, this is the simple cross platform version
//!
//------------------------------------------------------------
RenderTarget::Ptr SurfaceManager::CreateRenderTarget( uint32_t iWidth, uint32_t iHeight, GFXFORMAT format, bool bTexturable, GFXAAMODE aamode  )
{
	D3DFORMAT d3dFmt = ConvertGFXFORMATToD3DFORMAT( format );
	ntAssert_p( d3dFmt != D3DFMT_UNKNOWN, ("Unknown Texture format") );
	return CreateRenderTarget( RenderTarget::CreationStruct(iWidth, iHeight, d3dFmt, !bTexturable, aamode) );
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
	// we allocate render targets are from POOL_DEFAULT
	D3DPOOL ePool = D3DPOOL_DEFAULT;	 
	uint32_t iUsage = D3DUSAGE_RENDERTARGET;

	// special case, just convert into our render target class
	// a render target thats created from a surface is a always discard and can never be turned into a texture
	if( creationStruct.bFromSurface )
	{
		D3DSURFACE_DESC stDesc;
		creationStruct.pSurface->GetDesc( &stDesc );
		ntError_p( stDesc.Pool == D3DPOOL_DEFAULT, ("Render targets must be D3DPOOL_DEFAULT") );
		ntError_p( stDesc.Type == D3DRTYPE_SURFACE, ("Render target must be plain old surfaces via this construction method") );

		// note, allocations of types derived from IntrusivePtrCountedBase<> MUST use FW_NEW()
		// to match the chunk they will be released from
		RenderTarget::Ptr pRenderTarget = RenderTarget::Ptr( FW_NEW RenderTarget );

		pRenderTarget->m_iWidth							=		stDesc.Width;
		pRenderTarget->m_iHeight						=		stDesc.Height;
		pRenderTarget->m_Platform.m_eFormat				=		stDesc.Format;
		pRenderTarget->m_Platform.m_bAlwaysDiscarded	=		true;
		pRenderTarget->m_Platform.m_pTexture			=		0;
		pRenderTarget->m_Platform.m_pRtSurface			=		creationStruct.pSurface;

		if( creationStruct.bImplicit )
		{
			pRenderTarget->m_Platform.m_bImplicit = true;
			pRenderTarget->m_Platform.m_bCacheable = false;
		}
		else
		{
			pRenderTarget->m_Platform.m_bImplicit = false;
			pRenderTarget->m_Platform.m_bCacheable = creationStruct.bCacheable;

			// push it into the used list
			m_Platform.m_UsedRenderTargetList.push_back( pRenderTarget );
		}

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
				( (*obIt)->GetWidth()						== creationStruct.iWidth) &&
				( (*obIt)->GetHeight()						== creationStruct.iHeight) && 
				( (*obIt)->m_Platform.GetDXFormat()			== creationStruct.eFormat) &&
				( (*obIt)->m_Platform.IsAlwaysDiscarded()	== creationStruct.bDiscard) &&
				( (*obIt)->m_Platform.IsD3DDynamic()		== creationStruct.bDynamic) 
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

	// create one
	IDirect3DSurface9* pD3DSurface = 0;
	IDirect3DTexture9* pD3DTexture = 0;

	// discards are created via CreateRenderTarget and can be multi-sampled (TODO)
	if( creationStruct.bDiscard )
	{
		dxerror_p( 
				GraphicsDevice::Get().m_Platform.Get()->CreateRenderTarget( 
					creationStruct.iWidth, creationStruct.iHeight, creationStruct.eFormat, D3DMULTISAMPLE_NONE, 0, 0,
					&pD3DSurface, 0 ),
				( "failed to create render target" ) );
	} else
	{
		// factor in the dynamic flag
		iUsage |= (creationStruct.bDynamic) ? D3DUSAGE_DYNAMIC : 0;

		if (creationStruct.bHardwareSMap)
		{
			ntError_p( HardwareCapabilities::Get().SupportsHardwareShadowMaps(), ("Cannot create hardware shadow map") );
			
			// reset the usage flags
			iUsage = D3DUSAGE_DEPTHSTENCIL;
		}

		// if true textureable render-target they are created via CreateTexture
		HRESULT res = GraphicsDevice::Get().m_Platform.Get()->CreateTexture( 
				creationStruct.iWidth, creationStruct.iHeight, 1, iUsage, creationStruct.eFormat, ePool, 
				&pD3DTexture, 0 );

		if ( FAILED(res) )																		\
		{
			ntAssert_p(0,("failed to create texture"));
		}

		// get the surface associated with the texture
		dxerror_p( pD3DTexture->GetSurfaceLevel(0, &pD3DSurface ), ("Unable to get surface from render-target texture") );
	}

	// note, allocations of types derived from IntrusivePtrCountedBase<> MUST use FW_NEW()
	// to match the chunk they will be released from
	RenderTarget::Ptr pRenderTarget			= RenderTarget::Ptr( FW_NEW RenderTarget );

	pRenderTarget->m_iWidth					= creationStruct.iWidth;
	pRenderTarget->m_iHeight				= creationStruct.iHeight;
	pRenderTarget->m_Platform.m_eFormat		= creationStruct.eFormat;
	pRenderTarget->m_Platform.m_bDynamic	= creationStruct.bDiscard;
	pRenderTarget->m_Platform.m_bAlwaysDiscarded = false;
	pRenderTarget->m_Platform.m_pTexture	= pD3DTexture;
	pRenderTarget->m_Platform.m_pRtSurface	= pD3DSurface;
	pRenderTarget->m_Platform.m_bImplicit	= false;
	pRenderTarget->m_Platform.m_bCacheable	= creationStruct.bCacheable;

	// push it into the used list
	m_Platform.m_UsedRenderTargetList.push_back( pRenderTarget );

	TRACK_GFX_ALLOC_RT( pRenderTarget );

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
	if( !pRenderTarget->m_Platform.m_bImplicit )
	{
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
}
//------------------------------------------------------------
//!
//! Clear any caches
//!
//------------------------------------------------------------
void SurfaceManager::ClearCache( bool bTexture, bool bSurface, bool bRenderTarget)
{
	if( bTexture )
	{
		m_Platform.m_FreeTextureList.clear();
	}

	if( bSurface )
	{
		m_Platform.m_FreeSurfaceList.clear();
	}

	if( bRenderTarget )
	{
		m_Platform.m_FreeRenderTargetList.clear();
	}

}
