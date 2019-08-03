//--------------------------------------------------
//!
//!	\file texturexdram.cpp
//!	specialised texture class
//!
//--------------------------------------------------

#include "gfx/texture.h"
#include "gfx/texturemanager.h"
#include "gfx/surfacemanager.h"
#include "gfx/renderer.h"

#include <Gp/GpGtf/GpGtf.h>

/***************************************************************************************************
*
*	FUNCTION		TextureManager::LoadUnchachedTexture
*
*	DESCRIPTION		Loads a texture directly into main mem, and does not cache it.
*
*	NOTES			This is used for Large textures that we donot want in VRAM, such as
*					the loading screen, title screens and GUI.
*					You MUST release these as soon as youre done with them.
*
*	RESTRICTIONS	These textures must conform to the same restrictions as
*					SurfaceManagerPlatform::TextureFromMainMem()
*
***************************************************************************************************/
TextureXDRAM::TextureXDRAM( char const* pName ) :
	m_pTextureMem(0)
{
	char aFileName[MAX_PATH];
	TextureManager::MakePlatformTextureName( pName, aFileName );

	if ( File::Exists( aFileName ) == false )
	{
		Debug::Printf( Debug::DCU_TEXTURE, "\n!!! ERROR: Missing texture: %s. Using red missing texture instead!!!\n\n",aFileName);
		m_pTexturePtr = TextureManager::Get().GetMissingTexture();
		return;
	}

	if (CRendererSettings::bUseGTFTextures)
	{
		// see if texture loading is disabled
		if (CRendererSettings::bUseTextures == false)
		{
			m_pTexturePtr = TextureManager::Get().GetWhiteTexture();
			return;
		}

		// GTF LOADER
		//------------------------------------------------------------------------------
	
		// retrieve texture header (assume and check for only 1 texture in the GTF)
		File dataFile( aFileName, File::FT_READ | File::FT_BINARY );

		struct GTFHeader
		{
			GpGtfResource	m_gtf;
			GpGtfAttribute	m_tex;
		};

		GTFHeader header;
		dataFile.Read( &header, sizeof(GTFHeader) );

		ntError_p( header.m_gtf.m_numTextures == 1, ("Must only use GTF's with one texture inside") );

		// allocate a chunck of RSX main ram and blat the texture directly into that.
		ntError_p( header.m_tex.m_textureSize != 0, ("Cannot create a zero sized texture :%s", aFileName ) );
		m_pTextureMem = NT_MEMALIGN_CHUNK( Mem::MC_RSX_MAIN_USER, header.m_tex.m_textureSize, Gc::kTextureAlignment );

		dataFile.Seek( header.m_tex.m_offsetToTex, File::SEEK_FROM_START );
		dataFile.Read( (void*)m_pTextureMem, header.m_tex.m_textureSize );
	
		int base = header.m_tex.m_tex.format & 0x1F;
		int muxswizzle = header.m_tex.m_tex.remap;

		// handle shitty SCEI tool
		if (muxswizzle == 0)
		{
			// tool has failed to generate muxswizzle bits, see if we can fake
			// whole format based on base.

			switch (base)
			{
			case Ice::Render::kBtfDxt1:						
			case Ice::Render::kBtfDxt3:
			case Ice::Render::kBtfDxt5:
				muxswizzle = 0xAAE4;
				break;
			}
		}
		else if ( base == Ice::Render::kBtfGb88 )	
		{
			// our l8 a8 texture seem to have 0xAAA4. great!
			muxswizzle = 0xAAAB;
		}

		Gc::TexFormat format = ( Gc::TexFormat )( ( base << 24 ) | muxswizzle );

		// see icetextureformats.h for explanation of this magic bit (I don't like it either)
		if( ( Gc::TexFormat )( format | 0x00010000 ) == Gc::kTexFormatL16A16 )
			format = Gc::kTexFormatL16A16;
		else if( ( ( format >> 24 ) & 0x1f ) == Ice::Render::kBtfR32f )
			format = Gc::kTexFormatR32F;

		m_pTexturePtr = SurfaceManagerPlatform::TextureFromMainMem( header.m_tex.m_tex.width, 
																	header.m_tex.m_tex.height,
																	header.m_tex.m_tex.pitch,
																	format, 
																	(void*)m_pTextureMem );
	}
	else
	{
		// REGULAR DDS LOADER
		//------------------------------------------------------------------------------

		char aFileName[MAX_PATH];
		TextureManager::MakePlatformTextureName( pName, aFileName );

		// check source file validity (we use as we're being lazy)
		ntError_p( File::Exists( aFileName ), ("Texture file (%s) MUST exist at this point", pName));

#ifdef USE_TEXTURE_LOGGING
		// see if there's anything wrong with this texture
		if (CRendererSettings::bLogTextureErrors || CRendererSettings::bReplaceInvalidTextures)
		{
			uint8_t *pReadResult = NULL;
			File dataFile;
			LoadFile_Chunk( aFileName, File::FT_READ | File::FT_BINARY, Mem::MC_GFX, dataFile, &pReadResult );

			if (TextureManager::LogTextureLoad( (void *)pReadResult, dataFile.GetFileSize(), aFileName ) && CRendererSettings::bReplaceInvalidTextures)
			{
				Debug::Printf( Debug::DCU_TEXTURE, "\n!!! ERROR: Replacing texture: %s. Using shiny green texture instead. |||\n\n", aFileName );
				m_pTexturePtr = TextureManager::Get().GetInvalidTexture();
				NT_DELETE_ARRAY_CHUNK( Mem::MC_GFX, (uint8_t *)pReadResult );
				return;
			}
			NT_DELETE_ARRAY_CHUNK( Mem::MC_GFX, pReadResult );
		}
#endif

		// see if texture loading is disabled
		if (CRendererSettings::bUseTextures == false)
		{
			m_pTexturePtr = TextureManager::Get().GetWhiteTexture();
			return;
		}

		bool bSuccess = true;
		DDS::DDS_HEADER header;
		bSuccess = TexturePlatform::RetrieveTextureHeader( aFileName, header );

		ntError_p( bSuccess, ("Unable to retrieve texture header for %d.", aFileName) );
		
		// validate some basic assumptions.
		ntError_p( header.dwDepth == 0, ("TextureXDRAM %d is incorrect depth (%d)", aFileName,header.dwDepth) );

		// retrieve the format
		Gc::TexFormat format = TexturePlatform::GetTextureFormat( header );
		ntError_p( format != 0, ("TextureXDRAM %d has un recognised format", aFileName ) );

		uint32_t iAllocSize = 0;
		uint32_t iPitch = 0;

		if( format == Gc::kTexFormatDXT1 || format == Gc::kTexFormatDXT3 || format == Gc::kTexFormatDXT5 )
		{
			// work in blocks
			uint bytesPerBlock = 16*Gc::GetBitsPerPixel( format )/8;
			uint blockWidth = ( header.dwWidth + 3 )/4;
			uint blockHeight = ( header.dwHeight + 3 )/4;

			iPitch = bytesPerBlock * blockWidth;
			iAllocSize = iPitch * blockHeight;
		}
		else
		{
			iPitch = header.dwWidth * Gc::GetBytesPerPixel( format );
			iAllocSize = header.dwWidth * header.dwHeight * Gc::GetBytesPerPixel( format );
		}

		// allocate a chunck of RSX main ram and blat the texture directly into that.
		ntError_p( iAllocSize != 0, ("Cannot create a zero sized texture :%s", aFileName ) );
		m_pTextureMem = NT_MEMALIGN_CHUNK( Mem::MC_RSX_MAIN_USER, iAllocSize, Gc::kTextureAlignment );

		File texFile(aFileName, File::FT_READ | File::FT_BINARY );
		ntError_p( texFile.IsValid(), ("Unable to open texture: %s", aFileName) );

		texFile.Seek( sizeof(DDS::DDS_HEADER), File::SEEK_FROM_START );
		texFile.Read( (void*)m_pTextureMem, iAllocSize );

		// now create a user mode GcTexture handle based off our main memory.
		m_pTexturePtr = SurfaceManagerPlatform::TextureFromMainMem( header.dwWidth, header.dwHeight, iPitch, format, (void*)m_pTextureMem );
	}
}

TextureXDRAM::~TextureXDRAM()
{
	if (m_pTextureMem)
	{
		NT_FREE_CHUNK( Mem::MC_RSX_MAIN_USER, m_pTextureMem );
	}
}
