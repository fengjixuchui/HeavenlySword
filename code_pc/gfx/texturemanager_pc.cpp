/***************************************************************************************************
*
*	$Header:: /game/texuremanager.cpp 8     15/08/03 13:06 Giles                                   $
*
*
*
*	CHANGES
*
*	8/5/2003	SimonB	Created
*
***************************************************************************************************/

#include "gfx/texturemanager.h"
#include "gfx/renderer.h"
#include "gfx/hardwarecaps.h"
#include "gfx/texturereader.h"
#include "gfx/surfacemanager.h"
#include "gfx/graphicsdevice.h"
#include "core/file.h"
#include "core/debug.h"
#include "gfx/dxerror_pc.h"

#include <ddraw.h> // for debugg dumping only

#ifdef USE_TEXTURE_LOGGING
/***************************************************************************************************
*
*	FUNCTION		TextureManager::LogTextureLoad
*
*	DESCRIPTION		Record problems with this texture to a log, return true if invalid
*
***************************************************************************************************/
bool	TextureManager::LogTextureLoad( void*, size_t, const char* pTexName )
{
	D3DXIMAGE_INFO info;
	D3DXGetImageInfoFromFile( pTexName, &info );

	// format validation
	GFXFORMAT fmt = ConvertD3DFORMATToGFXFORMAT( info.Format );

	if (CRendererSettings::bLogTextureInfo)
	{
		ntAssert( File::Exists( pTexName ) );
		File file( pTexName, File::FT_READ | File::FT_BINARY );

		size_t headerSize = 4 + sizeof( DDSURFACEDESC2 );
		uint32_t fileSize = (uint32_t)(file.GetFileSize() - headerSize);
		uint32_t estSize = GFXFormat::CalculateVRAMFootprint( fmt, info.Width, info.Height, info.MipLevels );
		uint32_t textureSize = ntstd::Max( estSize, fileSize );

		Debug::Printf( Debug::DCU_TEXTURE, "\nINFO: Loading: %s\n", pTexName );
		Debug::Printf( Debug::DCU_TEXTURE, "INFO: Width: %d. Height: %d. Mips: %d\n", info.Width, info.Height, info.MipLevels );
		
		uint32_t sizeKB = (textureSize + 1023) / 1024;
		float totalMB = _R(Renderer::ms_iDiskTex + textureSize) / (1024.f*1024.f);
		Debug::Printf( Debug::DCU_TEXTURE, "INFO: Format: %s. Size: %dKb. (Total: %.2f Mb).\n", GFXFormat::GetGFXFormatString( fmt ), sizeKB, totalMB );
	}

	bool bInvalid = false;
	
	bool bValidatePow2 = true;
	if (strstr( pTexName, "_npow2" ))
		bValidatePow2 = false;

	bool bValidateMips = true;
	if (strstr( pTexName, "_nomip" ))
		bValidateMips = false;

	if ((bValidateMips) && ((info.Width>1) || (info.Height>1)) && (info.MipLevels <= 1))
	{
		Debug::Printf( Debug::DCU_TEXTURE, "ERROR: TEX_SIZE: Texture %s does not have correct mip levels. Please generate mip chain or use _nomip in name.\n",pTexName);
		bInvalid = true;
	}

	// dimension validation
	if (info.Width > 4096)
	{
		Debug::Printf( Debug::DCU_TEXTURE, "ERROR: TEX_SIZE: Texture %s is too wide (%d pixels). Please downsize it.\n",pTexName, info.Width);
		bInvalid = true;
	}

	if (info.Height > 4096)
	{
		Debug::Printf( Debug::DCU_TEXTURE, "ERROR: TEX_SIZE: Texture %s is too high (%d pixels). Please downsize it.\n",pTexName, info.Height);
		bInvalid = true;
	}

	if (!Util::IsPow2(info.Width) && bValidatePow2)
	{
		Debug::Printf( Debug::DCU_TEXTURE, "ERROR: TEX_SIZE: Texture %s is not a power of 2 in width (%d pixels). Please fit to best power of two.\n",pTexName, info.Width);
		bInvalid = true;
	}

	if (!Util::IsPow2(info.Height) && bValidatePow2)
	{
		Debug::Printf( Debug::DCU_TEXTURE, "ERROR: TEX_SIZE: Texture %s is not a power of 2 in width (%d pixels). Please fit to best power of two.\n",pTexName, info.Height);
		bInvalid = true;
	}

	switch(fmt)
	{
	case GF_RGB8:
		{
			Debug::Printf( Debug::DCU_TEXTURE, "ERROR: TEX_FMT: Texture %s uses an unsupported texture format (24bit RGB8). Please change.\n",pTexName);
			bInvalid = true;
		}
		break;

	case GF_UNKNOWN:
		{
			Debug::Printf( Debug::DCU_TEXTURE, "ERROR: TEX_FMT: Texture %s uses an unsupported texture format. Please change.\n",pTexName);
			bInvalid = true;
		}
		break;
	}

	// name descriptor / format validation
	if (strstr( pTexName, "_colour_mono" ) || strstr( pTexName, "_colour_alpha" ))
	{
		if (fmt != GF_DXT5)
		{
			Debug::Printf( Debug::DCU_TEXTURE, "ERROR: TEX_FMT: Texture %s should be DXT5 compressed. Please change.\n",pTexName);
			bInvalid = bInvalid ? bInvalid : CRendererSettings::bCheckTextureNameValid;
		}
	}
	else if (strstr( pTexName, "_colour" ))
	{
		if (fmt != GF_DXT1)
		{
			Debug::Printf( Debug::DCU_TEXTURE, "ERROR: TEX_FMT: Texture %s should be DXT1 compressed. Please change.\n",pTexName);
			bInvalid = bInvalid ? bInvalid : CRendererSettings::bCheckTextureNameValid;
		}
	}
	else if (strstr( pTexName, "_normal_mono" ))
	{
		if (fmt != GF_DXT5)
		{
			Debug::Printf( Debug::DCU_TEXTURE, "ERROR: TEX_FMT: Normal+Parallax map %s should be DXT5 compressed. Please change.\n",pTexName);
			bInvalid = bInvalid ? bInvalid : CRendererSettings::bCheckTextureNameValid;
		}
	}
	else if (strstr( pTexName, "_normal_dxt1" ))
	{
		if (fmt != GF_DXT1)
		{
			Debug::Printf( Debug::DCU_TEXTURE, "ERROR: TEX_FMT: Normal_dxt1 map %s should be DXT1 compressed. Please change.\n",pTexName);
			bInvalid = bInvalid ? bInvalid : CRendererSettings::bCheckTextureNameValid;
		}
	}
	else if (strstr( pTexName, "_normal" ) )
	{
		if (fmt != GF_ARGB8)
		{
			Debug::Printf( Debug::DCU_TEXTURE, "ERROR: TEX_FMT: Normal map %s should be ARGB8 format. Please change.\n",pTexName);
			bInvalid = bInvalid ? bInvalid : CRendererSettings::bCheckTextureNameValid;
		}
	}
	else if (strstr( pTexName, "_mono" ))
	{
		if (fmt != GF_L8)
		{
			Debug::Printf( Debug::DCU_TEXTURE, "ERROR: TEX_FMT: Texture %s should be L8 format. Please change.\n",pTexName);
			bInvalid = bInvalid ? bInvalid : CRendererSettings::bCheckTextureNameValid;
		}
	}
	else if (strstr( pTexName, "_alpha" ))
	{
		if (fmt != GF_A8)
		{
			Debug::Printf( Debug::DCU_TEXTURE, "ERROR: TEX_FMT: Texture %s should be A8 format. Please change.\n",pTexName);
			bInvalid = bInvalid ? bInvalid : CRendererSettings::bCheckTextureNameValid;
		}
	}
	else
	{
		Debug::Printf( Debug::DCU_TEXTURE, "WARNING: TEX_USE: Cannot identify usage of texture %s. Is is named correctly?\n",pTexName);
		bInvalid = bInvalid ? bInvalid : CRendererSettings::bCheckTextureNameValid;
	}

	return bInvalid;
}
#endif _RELEASE

/***************************************************************************************************
*
*	FUNCTION		TextureManager::DebugLoadTexture
*
*	DESCRIPTION		Loads an uncached texture, to help simulate behaviour of XDDR textures
*
***************************************************************************************************/
Texture::Ptr TextureManager::DebugLoadTexture( char const* pName )
{
	char aFileName[MAX_PATH];
	TextureManager::MakePlatformTextureName( pName, aFileName );

	Util::SetToPlatformResources();

	if ( File::Exists( aFileName ) == false )
	{
		Debug::Printf( Debug::DCU_TEXTURE, "\n!!! ERROR: Missing texture: %s. Using red missing texture instead!!!\n\n",aFileName);
		Util::SetToNeutralResources();
		return TextureManager::Get().GetMissingTexture();
	}

	// see if there's anything wrong with this texture
#ifdef USE_TEXTURE_LOGGING
	if (CRendererSettings::bLogTextureErrors || CRendererSettings::bReplaceInvalidTextures)
	{
		uint8_t *pReadResult = NULL;
		File dataFile;
		LoadFile_Chunk( aFileName, File::FT_READ | File::FT_BINARY, Mem::MC_GFX, dataFile, &pReadResult );

		if (TextureManager::LogTextureLoad( (void *)pReadResult, dataFile.GetFileSize(), aFileName ) && CRendererSettings::bReplaceInvalidTextures)
		{
			Debug::Printf( Debug::DCU_TEXTURE, "\n!!! ERROR: Replacing texture: %s. Using shiny green texture instead. |||\n\n", aFileName );
		
			NT_DELETE_ARRAY_CHUNK( Mem::MC_GFX, pReadResult );
			Util::SetToNeutralResources();

			return TextureManager::Get().GetInvalidTexture();
		}

		NT_DELETE_ARRAY_CHUNK( Mem::MC_GFX, pReadResult );
	}
#endif

	// retrieve image info
	D3DXIMAGE_INFO imageInfo;
	HRESULT hr = D3DXGetImageInfoFromFile( aFileName, &imageInfo );

	// passed all debug switches, load the real texture
	Texture* pTex = NT_NEW_CHUNK( Mem::MC_GFX ) Texture;

	pTex->m_iWidth				= imageInfo.Width;
	pTex->m_iHeight				= imageInfo.Height;
	pTex->m_iMipCount			= imageInfo.MipLevels;
	pTex->m_Platform.m_eFormat	= imageInfo.Format;
	pTex->m_Platform.m_ePool		= D3DPOOL_MANAGED;
	pTex->m_Platform.m_bImplicit	= false;
	pTex->m_Platform.m_pThis		= pTex;

	switch (imageInfo.ResourceType)
	{
	case D3DRTYPE_TEXTURE:
		{
			pTex->m_Platform.m_eType = TT_2D_TEXTURE;
			hr = D3DXCreateTextureFromFile( GetD3DDevice(), aFileName, &pTex->m_Platform.m_p2DTexture );
		}
		break;

	case D3DRTYPE_VOLUMETEXTURE:
		{
			pTex->m_Platform.m_eType = TT_3D_TEXTURE;
			hr = D3DXCreateVolumeTextureFromFile( GetD3DDevice(), aFileName, &pTex->m_Platform.m_p3DTexture );
		}
		break;

	case D3DRTYPE_CUBETEXTURE:
		{
			pTex->m_Platform.m_eType = TT_CUBE_TEXTURE;
			hr = D3DXCreateCubeTextureFromFile( GetD3DDevice(), aFileName, &pTex->m_Platform.m_pCubeTexture );
		}
		break;
	}

	user_error_p( SUCCEEDED( hr ), ( __FUNCTION__": can't load texture \"%s\". D3D's Reason %s : %s \n", pName, ::DXGetErrorString9(hr), ::DXGetErrorDescription9(hr) ) );

	Util::SetToNeutralResources();

	// add it to the cache
	return Texture::Ptr( pTex );
}

/***************************************************************************************************
*
*	FUNCTION		TextureManager::LoadTexture_FromData
*
*	DESCRIPTION		Loads a texture from a raw block of DDS data, using a cache
*					key that corresponds to the hashed result of MakePlatformTextureName()
*					to insert texture into the cache.
*
*					Returns cached result if already loaded
*
***************************************************************************************************/
Texture::Ptr TextureManager::LoadTexture_FromData( uint32_t cacheKey, void* pTextureData, size_t fileSize, const char* pDebugTag, bool bForceLoad )
{
	Texture::Ptr result = FindFromCache( cacheKey );
	if (result)
		return result;
	
	// passed an invalid pointer, this probably means the source file didnt exist.
	// register an error texture instead.
	if (pTextureData == NULL)
	{
		Debug::Printf( Debug::DCU_TEXTURE, "\n!!! ERROR: Missing texture: %s. Using red missing texture instead!!!\n\n",pDebugTag);

		// pretend we've found it by adding to the cache
		m_errorCache[ cacheKey ] = m_pMissingTexture;
		return m_pMissingTexture;
	}

	// check source file validity (we use as we're being lazy)
	ntError_p( File::Exists( pDebugTag ), ("Texture file (%s) MUST exist at this point", pDebugTag));

#ifdef USE_TEXTURE_LOGGING
	// see if there's anything wrong with this texture
	if ((CRendererSettings::bLogTextureErrors || CRendererSettings::bReplaceInvalidTextures) && (bForceLoad == false))
	{
		if (LogTextureLoad( pTextureData, fileSize, pDebugTag ) && CRendererSettings::bReplaceInvalidTextures)
		{
			Debug::Printf( Debug::DCU_TEXTURE, "\n!!! ERROR: Replacing texture: %s. Using shiny green texture instead. |||\n\n",pDebugTag);

			// pretend we've found it by adding to the cache
			m_errorCache[ cacheKey ] = m_pInvalidTexture;
			return m_pInvalidTexture;
		}	
	}
#endif

	// see if texture loading is disabled
	if ((!CRendererSettings::bUseTextures) && (!bForceLoad))
	{
		// pretend we've found it by adding to the cache
		m_errorCache[ cacheKey ] = m_pWhiteTexture;
		return m_pWhiteTexture;
	}

	// retrieve image info
	D3DXIMAGE_INFO imageInfo;
	HRESULT hr = D3DXGetImageInfoFromFileInMemory( pTextureData, fileSize, &imageInfo );

	// passed all debug switches, load the real texture
	Texture* pTex = NT_NEW_CHUNK( Mem::MC_GFX ) Texture;

	pTex->m_iWidth				= imageInfo.Width;
	pTex->m_iHeight				= imageInfo.Height;
	pTex->m_iMipCount			= imageInfo.MipLevels;
	pTex->m_Platform.m_eFormat	= imageInfo.Format;
	pTex->m_Platform.m_ePool		= D3DPOOL_MANAGED;
	pTex->m_Platform.m_bImplicit	= false;
	pTex->m_Platform.m_pThis		= pTex;

	switch (imageInfo.ResourceType)
	{
	case D3DRTYPE_TEXTURE:
		{
			pTex->m_Platform.m_eType = TT_2D_TEXTURE;
			hr = D3DXCreateTextureFromFileInMemory( GetD3DDevice(), pTextureData, fileSize, &pTex->m_Platform.m_p2DTexture );
		}
		break;

	case D3DRTYPE_VOLUMETEXTURE:
		{
			pTex->m_Platform.m_eType = TT_3D_TEXTURE;
			hr = D3DXCreateVolumeTextureFromFileInMemory( GetD3DDevice(), pTextureData, fileSize, &pTex->m_Platform.m_p3DTexture );
		}
		break;

	case D3DRTYPE_CUBETEXTURE:
		{
			pTex->m_Platform.m_eType = TT_CUBE_TEXTURE;
			hr = D3DXCreateCubeTextureFromFileInMemory( GetD3DDevice(), pTextureData, fileSize, &pTex->m_Platform.m_pCubeTexture );
		}
		break;
	}

	user_error_p( SUCCEEDED( hr ), ( __FUNCTION__": can't load texture \"%s\". D3D's Reason %s : %s \n", pDebugTag, ::DXGetErrorString9(hr), ::DXGetErrorDescription9(hr) ) );

	// add it to the cache
	result = Texture::Ptr( pTex );
	m_cache[ cacheKey ] = result;

#ifdef TRACK_GFX_MEM
	pTex->m_iDiskSize = Texture::GetTextureDiskSize( pDebugTag );
	TRACK_GFX_ALLOC_DISK_TEX( pTex );	
#endif

	return result;
}
