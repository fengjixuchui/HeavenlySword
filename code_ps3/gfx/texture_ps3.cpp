//------------------------------------------------------------
//!
//! \file gfx\texture_pc.cpp
//! PC implementation of the texture
//!
//------------------------------------------------------------

#include "gfx/texture.h"
#include "gfx/texturemanager.h"

#include "core/profiling.h"
#include "core/cellfsfile_ps3.h"


//-----------------------------------------------------
//!
//! No need for an explicit destructor, when m_Platform 
//! goes out of scope, our handle will be released.
//!
//-----------------------------------------------------
Texture::~Texture() {}

//-----------------------------------------------------
//!
//! 
//!
//-----------------------------------------------------
GFXFORMAT Texture::GetFormat() const
{
	return ConvertGCFORMATToGFXFORMAT( m_Platform.GetGCFormat() );
}

uint32_t Texture::GetTextureDiskSize( const char* pTexName )
{
	ntAssert( File::Exists( pTexName ) );
	File file( pTexName, File::FT_READ | File::FT_BINARY );

	static size_t headerSize = sizeof( DDS::DDS_HEADER );
	return (uint32_t)(file.GetFileSize() - headerSize);
}

//-----------------------------------------------------
//!
//! In GC there is no such thing as locking a texture,
//! you just write straight into VRAM, so its your
//! responisbility to make sure youve finished writing
//! before it used....
//!
//-----------------------------------------------------
void* Texture::CPULock2D( uint32_t& pitch )
{
	pitch = m_Platform.m_hTexture->GetPitch();
	return m_Platform.m_hTexture->GetDataAddress();
}

void Texture::CPUUnlock2D() {}

//! Gc doesn't appear to have an equivelent to slicePitch... so this is proberly wrong at the moment
void* Texture::CPULock3D( uint32_t& pitch, uint32_t& slicePitch )
{
	pitch = m_Platform.m_hTexture->GetPitch();
	slicePitch = 0;
	return m_Platform.m_hTexture->GetDataAddress();
}

void Texture::CPUUnlock3D() {}

void* Texture::CPULockCube( int iFaceId, uint32_t& pitch )
{
	GcTextureHandle	face = m_Platform.m_hTexture->GetCubeFace( (Gc::TexCubeFace)iFaceId );
	pitch = face->GetPitch();
	return face->GetDataAddress();
}

void Texture::CPUUnlockCube( int iFaceId ) 
{
	UNUSED( iFaceId );
}

//-----------------------------------------------------
//!
//! what type of texture are we?
//!
//-----------------------------------------------------
TEXTURE_TYPE Texture::GetType() const
{
	switch (m_Platform.GetTexture()->GetType())
	{
	case Gc::kTexture2D:	return TT_2D_TEXTURE;
	case Gc::kTexture3D:	return TT_3D_TEXTURE;
	case Gc::kTextureCube:	return TT_CUBE_TEXTURE;
	default:
		ntAssert_p( 0, ("Unrecognised texture type: %d", m_Platform.GetTexture()->GetType() ));
		return TT_2D_TEXTURE;
	}
}

//-----------------------------------------------------
//!
//! RetrieveTextureHeader
//! Retrieve the texture header from a file so we can
//! see if the texture is okay to load or not.
//! Returns true if info retrieved.
//!
//-----------------------------------------------------
bool TexturePlatform::RetrieveTextureHeader( const char* pTexName, DDS::DDS_HEADER& header )
{
	if (File::Exists(pTexName))
	{
		File texFile(pTexName, File::FT_READ | File::FT_BINARY );
		DDS::DDS_HEADER littleEndian;
		texFile.Read( &littleEndian, sizeof( DDS::DDS_HEADER ) );
		RetrieveTextureHeader( &littleEndian, header);
		return true;
	}

	return false;
}

//-----------------------------------------------------
//!
//! RetrieveTextureHeader
//! Retrieve the texture header from a file so we can
//! see if the texture is okay to load or not.
//! Returns true if info retrieved.
//!
//-----------------------------------------------------
void TexturePlatform::RetrieveTextureHeader( void* pRawTexData, DDS::DDS_HEADER& header )
{
	ntAssert( pRawTexData );
	DDS::DDS_HEADER& littleEndian = *(DDS::DDS_HEADER*)pRawTexData;

	header.dwSize				= FwByteSwap( littleEndian.dwSize );
	header.dwFlags				= FwByteSwap( littleEndian.dwFlags );
	header.dwHeight				= FwByteSwap( littleEndian.dwHeight );
	header.dwWidth				= FwByteSwap( littleEndian.dwWidth );
	header.dwPitchOrLinearSize	= FwByteSwap( littleEndian.dwPitchOrLinearSize );
	header.dwDepth				= FwByteSwap( littleEndian.dwDepth );
	header.dwMipMapCount		= FwByteSwap( littleEndian.dwMipMapCount );
	header.dwCaps1				= FwByteSwap( littleEndian.dwCaps1 );
	header.dwCaps2				= FwByteSwap( littleEndian.dwCaps2 );

	header.ddspf.dwSize			= FwByteSwap( littleEndian.ddspf.dwSize );
	header.ddspf.dwFlags		= FwByteSwap( littleEndian.ddspf.dwFlags );
	header.ddspf.dwFourCC		= FwByteSwap( littleEndian.ddspf.dwFourCC );
	header.ddspf.dwRGBBitCount	= FwByteSwap( littleEndian.ddspf.dwRGBBitCount );
	header.ddspf.dwRBitMask		= FwByteSwap( littleEndian.ddspf.dwRBitMask );
	header.ddspf.dwGBitMask		= FwByteSwap( littleEndian.ddspf.dwGBitMask );
	header.ddspf.dwBBitMask		= FwByteSwap( littleEndian.ddspf.dwBBitMask );
	header.ddspf.dwABitMask		= FwByteSwap( littleEndian.ddspf.dwABitMask );
}

//-----------------------------------------------------
//!
//! GetTextureFormat
//! Convert the contents of a DDS header in to the 
//! corresponding Gc texture format
//!
//-----------------------------------------------------
Gc::TexFormat TexturePlatform::GetTextureFormat( const DDS::DDS_HEADER& header )
{
	const DDS::DDS_PIXELFORMAT& pixelformat( header.ddspf );

	if( pixelformat.dwFlags & DDS::DDPF_FOURCC ) 
	{
		switch( pixelformat.dwFourCC )
		{
		case DDS::FOURCC_DXT1:			return Gc::kTexFormatDXT1;
		case DDS::FOURCC_DXT3:			return Gc::kTexFormatDXT3;
		case DDS::FOURCC_DXT5:			return Gc::kTexFormatDXT5;
		case DDS::FOURCC_A16R16G16B16F:	return Gc::kTexFormatRGBA16F;
		case DDS::FOURCC_A32R32G32B32F:	return Gc::kTexFormatRGBA32F;
		case DDS::FOURCC_R32F:			return Gc::kTexFormatR32F;
		default:
			ntError_p(0,("Unsupported FOURCC code %d", pixelformat.dwFourCC));
			return ( Gc::TexFormat )0;
		}
	}
	else if ( pixelformat.dwRGBBitCount == 32 )
	{
		if( 		pixelformat.dwRBitMask == 0x0000ffff && 
					pixelformat.dwGBitMask == 0x00000000 && 
					pixelformat.dwBBitMask == 0x00000000 && 
					pixelformat.dwABitMask == 0xffff0000 )
			return Gc::kTexFormatL16A16;
		else if( 	pixelformat.dwRBitMask == 0x0000ff00 && 
					pixelformat.dwGBitMask == 0x00ff0000 && 
					pixelformat.dwBBitMask == 0xff000000 && 
					pixelformat.dwABitMask == 0x000000ff )
			return Gc::kTexFormatARGB8;
		else if( 	pixelformat.dwRBitMask == 0x00ff0000 && 
					pixelformat.dwGBitMask == 0x0000ff00 && 
					pixelformat.dwBBitMask == 0x000000ff && 
					pixelformat.dwABitMask == 0xff000000 )
			return Gc::kTexFormatBGRA8;
		else if( 	pixelformat.dwRBitMask == 0x000000ff && 
					pixelformat.dwGBitMask == 0x0000ff00 && 
					pixelformat.dwBBitMask == 0x00ff0000 &&
					pixelformat.dwABitMask == 0xff000000 )
			return Gc::kTexFormatRGBA8;
	}
	else if ( pixelformat.dwRGBBitCount == 16 )
	{
		if( 		pixelformat.dwRBitMask == 0x0000ffff &&
					pixelformat.dwGBitMask == 0x00000000 &&
					pixelformat.dwBBitMask == 0x00000000 &&
					pixelformat.dwABitMask == 0x00000000 )
			return Gc::kTexFormatL16;
		else if( 	pixelformat.dwRBitMask == 0x00000000 && 
					pixelformat.dwGBitMask == 0x00000000 &&
					pixelformat.dwBBitMask == 0x00000000 &&
					pixelformat.dwABitMask == 0x0000ffff )
			return Gc::kTexFormatA16;
		else if(	pixelformat.dwRBitMask == 0x000000ff &&
					pixelformat.dwGBitMask == 0x00000000 &&
					pixelformat.dwBBitMask == 0x00000000 &&
					pixelformat.dwABitMask == 0x0000ff00 )
			return Gc::kTexFormatL8A8;
	}
	else if ( pixelformat.dwRGBBitCount == 8 )
	{
		if( 		pixelformat.dwRBitMask == 0x000000ff &&
					pixelformat.dwGBitMask == 0x00000000 &&
					pixelformat.dwBBitMask == 0x00000000 &&
					pixelformat.dwABitMask == 0x00000000 )
			return Gc::kTexFormatL8;
		else if( 	pixelformat.dwRBitMask == 0x00000000 && 
					pixelformat.dwGBitMask == 0x00000000 &&
					pixelformat.dwBBitMask == 0x00000000 &&
					pixelformat.dwABitMask == 0x000000ff )
			return Gc::kTexFormatA8;
	}
	return ( Gc::TexFormat )0;
}

//-----------------------------------------------------
//!
//! dump the texture to disk.
//! ATTN! this fundamentally doesnt work as there is
//! no way of schyncronising with the GPU.
//! When we have access to ICE render calls we 
//! should be able to spin the cell till the GPU has
//! caught up to the last render call, at which point
//! the contents of this buffer will be valid.
//!
//-----------------------------------------------------
void TexturePlatform::SaveToDisk( const char* pName, bool bInDataDir )
{
	ntAssert_p( m_pThis->GetType() == TT_2D_TEXTURE, ("Have only implemented saving 2D textures, sorry!"));

	char aName[MAX_PATH];

	if (bInDataDir)
	{
		Util::GetFullGameDataFilePath( TEXTURE_ROOT_PATH, aName );
		strcat( aName, pName );
	}
	else
	{
		Util::GetFullGameDataFilePath( pName, aName );
	}

	// we're automagically going to block for 1 second before hand, as we have
	// no garuntee weve finished rendering when this is called.
	BLOCK_FOR_N_SECONDS( 0.1f );

	uint32_t iMipLevel = 0;

	void* pVRAM = GetTexture()->GetMipLevel(0)->GetDataAddress();

	uint32_t iMipWidth = ntstd::Max( m_pThis->GetWidth() >> iMipLevel, 1u );
	uint32_t iMipHeight = ntstd::Max( m_pThis->GetHeight() >> iMipLevel, 1u );

	Gc::TexFormat fmt = m_hTexture->GetFormat();
	
	// get the pitch(s)
	uint32_t iSourcePitch	= m_hTexture->GetPitch();
	uint32_t iLinearSize	= iMipWidth * Gc::GetBytesPerPixel( fmt );
	uint32_t iWritePitch	= ROUND_POW2( iLinearSize, 4 );

	// create a header for a single-mip 2D texture file
	DDS::DDS_HEADER header;
	memset( &header, 0, sizeof( DDS::DDS_HEADER ) );
	header.tag 					= 0x44445320;	// "DDS "
	header.dwSize 				= sizeof( DDS::DDS_HEADER ) - 4;
	header.dwFlags 				= DDS::DDSD_CAPS 
								| DDS::DDSD_PIXELFORMAT 
								| DDS::DDSD_WIDTH 
								| DDS::DDSD_HEIGHT 
								| DDS::DDSD_PITCH;

	header.dwHeight 			= iMipHeight;
	header.dwWidth 				= iMipWidth;
//	header.dwPitchOrLinearSize	= (m_hTexture->IsSwizzled() ? iLinearSize : iPitch)
	header.dwPitchOrLinearSize	= iWritePitch;
	header.dwCaps1				= DDS::DDSCAPS_TEXTURE;
		
	header.ddspf.dwSize	= sizeof( DDS::DDS_PIXELFORMAT );
		
	// we'll only implement the colour formats we can render to
	switch( fmt )
	{
	case Gc::kTexFormatARGB8:
	case Gc::kTexFormatD24X8:		
		header.ddspf.dwFlags = DDS::DDPF_ALPHAPIXELS | DDS::DDPF_RGB;
		header.ddspf.dwRGBBitCount = 32;
		header.ddspf.dwRBitMask = 0x00ff0000;
		header.ddspf.dwGBitMask = 0x0000ff00;
		header.ddspf.dwBBitMask = 0x000000ff;
		header.ddspf.dwABitMask = 0xff000000;
		break;

	case Gc::kTexFormatRGBA32F:
		header.ddspf.dwFlags = DDS::DDPF_FOURCC;
		header.ddspf.dwFourCC = DDS::FOURCC_A32R32G32B32F;
		break;

	case Gc::kTexFormatRGBA16F:
		header.ddspf.dwFlags = DDS::DDPF_FOURCC;
		header.ddspf.dwFourCC = DDS::FOURCC_A16R16G16B16F;
		break;


	case Gc::kTexFormatR32F:
		header.ddspf.dwFlags = DDS::DDPF_FOURCC;
		header.ddspf.dwFourCC = DDS::FOURCC_R32F;
		break;

	default:
		FW_ASSERT_MSG( false, ( "unknown texture format" ) );
	}

	// byte-swap the header for PC reading
	header.dwSize				= FwByteSwap( header.dwSize );
	header.dwFlags				= FwByteSwap( header.dwFlags );
	header.dwHeight				= FwByteSwap( header.dwHeight );
	header.dwWidth				= FwByteSwap( header.dwWidth );
	header.dwPitchOrLinearSize	= FwByteSwap( header.dwPitchOrLinearSize );
	header.dwDepth				= FwByteSwap( header.dwDepth );
	header.dwMipMapCount		= FwByteSwap( header.dwMipMapCount );
	header.dwCaps1				= FwByteSwap( header.dwCaps1 );
	header.dwCaps2				= FwByteSwap( header.dwCaps2 );

	header.ddspf.dwSize			= FwByteSwap( header.ddspf.dwSize );
	header.ddspf.dwFlags		= FwByteSwap( header.ddspf.dwFlags );
	header.ddspf.dwFourCC		= FwByteSwap( header.ddspf.dwFourCC );
	header.ddspf.dwRGBBitCount	= FwByteSwap( header.ddspf.dwRGBBitCount );
	header.ddspf.dwRBitMask		= FwByteSwap( header.ddspf.dwRBitMask );
	header.ddspf.dwGBitMask		= FwByteSwap( header.ddspf.dwGBitMask );
	header.ddspf.dwBBitMask		= FwByteSwap( header.ddspf.dwBBitMask );
	header.ddspf.dwABitMask		= FwByteSwap( header.ddspf.dwABitMask );

	// create the whole file in main RAM
	uint32_t headerSize = uint( sizeof( DDS::DDS_HEADER ) );
	uint32_t dataSize = iMipHeight * iWritePitch;

	// Changed this to allocate from MC_GFX as they apparently have memory to spare and the main chunk doesn't... [ARV].
	uint8_t* pStorage = NT_NEW_CHUNK( Mem::MC_GFX ) u8[ headerSize + dataSize ];
	uint8_t* pHeader = pStorage;
	uint8_t* pTextureData = pStorage + headerSize;

	FwMemcpy( pHeader, &header, headerSize );

	// copy out scanlines of iPitchOrLinearSize 
	uint8_t* pRead = (uint8_t*)pVRAM;
	uint8_t* pWrite = pTextureData;

	for (uint32_t i = 0; i < iMipHeight; i++)
	{
		FwMemcpy( pWrite, pRead, iWritePitch );
		pRead += iSourcePitch;
		pWrite += iWritePitch;
	}

	// byte-swap the data to little endian
	switch( fmt )
	{
	case Gc::kTexFormatARGB8:
	case Gc::kTexFormatD24X8:		
		{
			u32* pData = reinterpret_cast< u32* >( pTextureData );
			u32* pDataEnd = reinterpret_cast< u32* >( pTextureData + dataSize );
			while( pData < pDataEnd )
			{
				*pData = FwByteSwap( *pData );
				++pData;
			}
		}
		break;

	case Gc::kTexFormatRGBA16F:
		{
			u16* pData = reinterpret_cast< u16* >( pTextureData );
			u16* pDataEnd = reinterpret_cast< u16* >( pTextureData + dataSize );
			while( pData < pDataEnd )
			{
				*pData = FwByteSwap( *pData );
				++pData;
			}
		}
		break;

	case Gc::kTexFormatRGBA32F:
		{
			u32* pData = reinterpret_cast< u32* >( pTextureData );
			u32* pDataEnd = reinterpret_cast< u32* >( pTextureData + dataSize );
			while( pData < pDataEnd )
			{
				*pData = FwByteSwap( *pData );
				++pData;
			}
		}
		break;

	case Gc::kTexFormatR32F:
		{
			u32* pData = reinterpret_cast< u32* >( pTextureData );
			u32* pDataEnd = reinterpret_cast< u32* >( pTextureData + dataSize );
			while( pData < pDataEnd )
			{
				*pData = FwByteSwap( *pData );
				++pData;
			}
		}
		break;


	default:
		FW_ASSERT_MSG( false, ( "unknown texture format" ) );
	}

	// add file extension and open file
	strcat( aName, ".dds" );

	CellFsFile textureFile( aName, File::FT_WRITE | File::FT_BINARY );
	ntError_p( textureFile.IsValid(), ("Failed to create dump file: %s\n", aName ) );

	textureFile.Write( pStorage, headerSize + dataSize );

	NT_DELETE_ARRAY_CHUNK( Mem::MC_GFX, pStorage );
}
