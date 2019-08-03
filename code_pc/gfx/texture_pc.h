//-----------------------------------------------------
//!
//!	\file gfx\texture_pc.h
//! platform specific bit for PC of texture
//!
//-----------------------------------------------------

#ifndef GFX_TEXTURE_PC_H
#define GFX_TEXTURE_PC_H

#include "gfxformat.h"

// forward decl
class Texture;

//-----------------------------------------------------
//!
//! The PC specific bits of a texture
//!
//-----------------------------------------------------
class TexturePlatform
{
public:
	friend class Texture;
	friend class TextureManager;
	friend class SurfaceManager;
	friend class SurfaceManagerPlatform;
	friend class ToolsTextureManager;

	struct CreationStruct
	{
		//! normal ctor for PC specific.
		CreationStruct( uint32_t width, uint32_t height, D3DFORMAT eformat, uint32_t miplevels) :
			bFromTexture( false ),
			bImplicit( false ),
			bCacheable( true ),
			iWidth( width ),
			iHeight( height ),
			iDepth( 1 ),
			eFormat( eformat ),
			iMipLevels( miplevels ),
			eType(TT_2D_TEXTURE) {}

		//! creation from a already existing IDirect3DTexture9.
		CreationStruct( IDirect3DTexture9* tex, bool implicit ) :
			bFromTexture( true ),
			bImplicit( implicit ),
			bCacheable( !implicit ),
			pTexture(tex),
			eType(TT_2D_TEXTURE) {}

		bool bFromTexture;				//!< Special case
		bool bImplicit;					//!< this texture is owned by another type of surface and shouldn't be cached
		bool bCacheable;					//!< should this texture be cached when released? (aslong as its not bImplicit)
		IDirect3DTexture9* pTexture;	//!< when converting from an existing texture this is the texture
		uint32_t iWidth;				//!< width of surface
		uint32_t iHeight;				//!< height of surface
		uint32_t iDepth;				//!< depth of surface (only used if we're a 3D texture)
		D3DFORMAT eFormat;				//!< D3D format of the surfaec
		uint32_t iMipLevels;			//!< number of mip levels
		TEXTURE_TYPE eType;				//!< what type of texture are we?
	};

	//!< Get the texture interface.
	IDirect3DBaseTexture9* GetTexture()
	{ 
		switch (m_eType)
		{
		case TT_2D_TEXTURE:		return m_p2DTexture;
		case TT_3D_TEXTURE:		return m_p3DTexture;
		case TT_CUBE_TEXTURE:	return m_pCubeTexture;
		default:
			ntError(0);
			return m_p2DTexture;
		}
	};

	IDirect3DTexture9* Get2DTexture()
	{ 
		ntAssert( m_eType == TT_2D_TEXTURE );
		return m_p2DTexture; 
	};

	IDirect3DVolumeTexture9* Get3DTexture()
	{
		ntAssert( m_eType == TT_3D_TEXTURE );
		return m_p3DTexture; 
	};

	IDirect3DCubeTexture9* GetCubeTexture()
	{ 
		ntAssert( m_eType == TT_CUBE_TEXTURE );
		return m_pCubeTexture; 
	};

	//!< Get a mip surface interface (unchecked for naughty people).
	IDirect3DSurface9*	GetSurfaceLevel( uint32_t iMipLevel );
	IDirect3DVolume9*	GetVolumeLevel( uint32_t iMipLevel );
	IDirect3DSurface9*	GetCubeSurfaceLevel( D3DCUBEMAP_FACES eFace, uint32_t iMipLevel );

	//! Get d3d format
	D3DFORMAT GetDXFormat() const { return m_eFormat; }

	//! get d3d pool 
	D3DPOOL GetPool() const { return m_ePool; }

	//! dump to disk
	void SaveToDisk( const char* pName, D3DXIMAGE_FILEFORMAT fmt, bool bAppendType = true, bool bInDataDir = false );

private:
	const Texture*			m_pThis;
	D3DFORMAT				m_eFormat;				//!< D3D format of texture

	// could union these if we care about space?
	IDirect3DTexture9*			m_p2DTexture;		//!< texture pointer used 99% of the time
	IDirect3DVolumeTexture9*	m_p3DTexture;		//!< texture pointer used when we're a 3D volume texture
	IDirect3DCubeTexture9*		m_pCubeTexture;		//!< texture pointer used when we're a cube map

	D3DPOOL					m_ePool;				//!< Which memory pool this texture is from
	bool					m_bImplicit;			//!< Texture is a temporary as its actually another type of surface in disguise
	bool					m_bCacheable;			//!< should this texture be cached when released? (aslong as its not bImplicit)
	TEXTURE_TYPE			m_eType;				//!< texture type
};

#endif // ndef _TEXTURE_H

