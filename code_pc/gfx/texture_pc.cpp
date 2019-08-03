//------------------------------------------------------------
//!
//! \file gfx\texture_pc.cpp
//! PC implementation of the texture
//!
//------------------------------------------------------------

#include "gfx/texture.h"
#include "gfx/texturemanager.h"
#include "gfx/dxerror_pc.h"
#include <ddraw.h> // for debugg dumping only

//--------------------------------------------------
//!
//! destroy, free any allocated memory
//!
//--------------------------------------------------
Texture::~Texture()
{
	if (!m_Platform.m_bImplicit)
	{
		switch( m_Platform.m_eType )
		{
		case TT_2D_TEXTURE:	
			m_Platform.m_p2DTexture->Release();
			break;

		case TT_3D_TEXTURE:	
			m_Platform.m_p3DTexture->Release();
			break;

		case TT_CUBE_TEXTURE:	
			m_Platform.m_pCubeTexture->Release();
			break;
		}
	}
}

GFXFORMAT Texture::GetFormat() const
{
	return ConvertD3DFORMATToGFXFORMAT( m_Platform.GetDXFormat() );
}

uint32_t Texture::GetTextureDiskSize( const char* pTexName )
{
	ntAssert( File::Exists( pTexName ) );
	File file( pTexName, File::FT_READ | File::FT_BINARY );

	size_t headerSize = 4 + sizeof( DDSURFACEDESC2 );
	return (uint32_t)(file.GetFileSize() - headerSize);
}

void* Texture::CPULock2D( uint32_t& pitch )
{
	ntAssert( m_Platform.m_eType == TT_2D_TEXTURE );

	D3DLOCKED_RECT rect;
	dxerror( m_Platform.Get2DTexture()->LockRect( 0, &rect, 0, 0 ) ); 
	pitch = rect.Pitch;
	return rect.pBits;
}

void Texture::CPUUnlock2D()
{
	ntAssert( m_Platform.m_eType == TT_2D_TEXTURE );
	dxerror( m_Platform.Get2DTexture()->UnlockRect(0) );
}

void* Texture::CPULock3D( uint32_t& rowPitch, uint32_t& slicePitch )
{
	ntAssert( m_Platform.m_eType == TT_3D_TEXTURE );

	D3DLOCKED_BOX box;
	dxerror( m_Platform.Get3DTexture()->LockBox( 0, &box, 0, 0 ) ); 
	rowPitch = box.RowPitch;
	slicePitch = box.SlicePitch;
	return box.pBits;
}

void Texture::CPUUnlock3D()
{
	ntAssert( m_Platform.m_eType == TT_3D_TEXTURE );
	dxerror( m_Platform.Get3DTexture()->UnlockBox(0) );
}

void* Texture::CPULockCube( int iFaceID, uint32_t& pitch )
{
	ntAssert( m_Platform.m_eType == TT_CUBE_TEXTURE );
	D3DCUBEMAP_FACES eFace = (D3DCUBEMAP_FACES)iFaceID;

	D3DLOCKED_RECT rect;
	dxerror( m_Platform.GetCubeTexture()->LockRect( eFace, 0, &rect, 0, 0 ) ); 
	pitch = rect.Pitch;
	return rect.pBits;
}

void Texture::CPUUnlockCube( int iFaceID )
{
	ntAssert( m_Platform.m_eType == TT_CUBE_TEXTURE );
	D3DCUBEMAP_FACES eFace = (D3DCUBEMAP_FACES)iFaceID;
	dxerror( m_Platform.GetCubeTexture()->UnlockRect(eFace,0) );
}

TEXTURE_TYPE Texture::GetType() const
{
	return m_Platform.m_eType;
}

IDirect3DSurface9* TexturePlatform::GetSurfaceLevel( uint32_t iMipLevel )
{
	ntAssert( m_eType == TT_2D_TEXTURE );
	IDirect3DSurface9* pSurface;	
	dxerror( Get2DTexture()->GetSurfaceLevel( iMipLevel, &pSurface ) );
	return pSurface;
}

IDirect3DVolume9* TexturePlatform::GetVolumeLevel( uint32_t iMipLevel )
{
	ntAssert( m_eType == TT_3D_TEXTURE );
	IDirect3DVolume9* pVolume;	
	dxerror( Get3DTexture()->GetVolumeLevel( iMipLevel, &pVolume ) );
	return pVolume;
}

IDirect3DSurface9* TexturePlatform::GetCubeSurfaceLevel( D3DCUBEMAP_FACES eFace, uint32_t iMipLevel )
{
	ntAssert( m_eType == TT_CUBE_TEXTURE );
	IDirect3DSurface9* pSurface;	
	dxerror( GetCubeTexture()->GetCubeMapSurface( eFace, iMipLevel, &pSurface ) );
	return pSurface;
}

void TexturePlatform::SaveToDisk( const char* pName, D3DXIMAGE_FILEFORMAT fmt, bool bAppendType, bool bInDataDir )
{
	ntAssert_p( m_eType == TT_2D_TEXTURE, ("Have only implemented saving 2D textures, sorry!"));

	char aName[MAX_PATH];

	if (bInDataDir)
	{
		Util::GetFiosFilePath( TEXTURE_ROOT_PATH, aName );
		strcat( aName, pName );
	}
	else
	{
		Util::GetFiosFilePath( pName, aName );
	}

	if (bAppendType)
	{
		switch( fmt )
		{
		case D3DXIFF_BMP:	strcat( aName, ".bmp" );	break;
		case D3DXIFF_JPG:	strcat( aName, ".jpg" );	break;
		case D3DXIFF_TGA:	strcat( aName, ".tga" );	break;
		case D3DXIFF_PNG:	strcat( aName, ".png" );	break;
		case D3DXIFF_DDS:	strcat( aName, ".dds" );	break;
		default:
			ntError_p(0, ("unrecognised texture format")); return;
		}
	}

	dxerror( D3DXSaveTextureToFile( aName, fmt, Get2DTexture(), 0 ) );
}

//--------------------------------------------------
//!
//! Fake XRAM texture for PC compatablility
//!
//--------------------------------------------------
TextureXDRAM::TextureXDRAM( char const* pName )
{
// sorry for this ifndef but I need this in order to not include texturemanager_pc.cpp in the texture converter project
// since this is a dummy class on the PC platform anyway I don't believe it's a big deal :) (Marco)
#ifndef TEXTURE_CONVERTER
	m_pTexturePtr = TextureManager::Get().DebugLoadTexture( pName );
#endif
}

TextureXDRAM::~TextureXDRAM()
{}

