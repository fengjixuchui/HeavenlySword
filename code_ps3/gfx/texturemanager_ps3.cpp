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
#include "gfx/texturereader.h"
#include "gfx/surfacemanager.h"

#include <Gp/GpGtf/GpGtf.h>

#ifdef USE_TEXTURE_LOGGING
/***************************************************************************************************
*
*	FUNCTION		TextureManager::LogTextureLoad
*
*	DESCRIPTION		Record problems with this texture to a log, return true if invalid
*
***************************************************************************************************/
bool	TextureManager::LogTextureLoad( void* pTextureData, size_t fileSize, const char* pDebugTag )
{
	DDS::DDS_HEADER fileHeader;
	TexturePlatform::RetrieveTextureHeader( pTextureData, fileHeader );

	// convert from our DDS header to a GFXFORMAT
	GFXFORMAT fmt = GF_UNKNOWN;
	if (fileHeader.ddspf.dwFlags & DDS::DDPF_FOURCC)
	{
		switch (fileHeader.ddspf.dwFourCC)
		{
		case DDS::FOURCC_DXT1:				fmt = GF_DXT1;		break;
		case DDS::FOURCC_DXT3:				fmt = GF_DXT3;		break;
		case DDS::FOURCC_DXT5:				fmt = GF_DXT5;		break;

		// this corresponds to DDS:FOURCC_A16R16G16B16 (16bit int, 4 channel), that is not supported by Gc
		case 36: break;

		// this corresponds to DDS:FOURCC_R16F (16bit float, 1 channel), that is not supported by Gc
		case 111: break;

		case DDS::FOURCC_A16R16G16B16F:		fmt = GF_ABGR16F;	break;
		case DDS::FOURCC_R32F:				fmt = GF_R32F;		break;
		case DDS::FOURCC_A32R32G32B32F:		fmt = GF_ABGR32F;	break;
		default:
			break;
		}
	}
	else
	{
		// 8 bit formats
		if (fileHeader.ddspf.dwRGBBitCount == 8)
		{
			if ((fileHeader.ddspf.dwFlags & DDS::DDPF_LUMINANCE) && (fileHeader.ddspf.dwRBitMask == 0xff))
				fmt = GF_L8;
			else if ((fileHeader.ddspf.dwFlags & DDS::DDPF_ALPHA) && (fileHeader.ddspf.dwABitMask == 0xff))
				fmt = GF_A8;
		}
		// 16 bit formats
		else if (fileHeader.ddspf.dwRGBBitCount == 16)
		{
			if	(
				(fileHeader.ddspf.dwFlags & (DDS::DDPF_LUMINANCE | DDS::DDPF_ALPHAPIXELS)) &&
				(fileHeader.ddspf.dwRBitMask == 0xff) &&
				(fileHeader.ddspf.dwABitMask == 0xff00)
				)
				fmt = GF_L8A8;
		}
		// 32 bit formats
		else if (fileHeader.ddspf.dwRGBBitCount == 32)
		{
			if	(
				(fileHeader.ddspf.dwFlags & (DDS::DDPF_RGB | DDS::DDPF_ALPHAPIXELS)) &&
				(fileHeader.ddspf.dwRBitMask == 0x00ff0000) &&
				(fileHeader.ddspf.dwGBitMask == 0x0000ff00) &&
				(fileHeader.ddspf.dwBBitMask == 0x000000ff) &&
				(fileHeader.ddspf.dwABitMask == 0xff000000)
				)
			{
				fmt = GF_BGRA8;
			}
		}
	}

	if (CRendererSettings::bLogTextureInfo)
	{
		size_t headerSize = sizeof( DDS::DDS_HEADER );
		uint32_t textureSize = (uint32_t)(fileSize - headerSize);

		Debug::Printf( Debug::DCU_TEXTURE, "\nINFO: Loading: %s\n", pDebugTag );
		Debug::Printf( Debug::DCU_TEXTURE, "INFO: Width: %d. Height: %d. Mips: %d\n", fileHeader.dwWidth, fileHeader.dwHeight, fileHeader.dwMipMapCount );
		
		uint32_t sizeKB = (textureSize + 1023) / 1024;
#		ifdef TRACK_GFX_MEM
		{
			float totalMB = _R(Renderer::ms_iDiskTex + textureSize) / (1024.f*1024.f);
			Debug::Printf( Debug::DCU_TEXTURE, "INFO: Format: %s. Size: %dKb. (Total: %.2f Mb).\n", GFXFormat::GetGFXFormatString( fmt ), sizeKB, totalMB );
		}
#		else
		{
			Debug::Printf( Debug::DCU_TEXTURE, "INFO: Format: %s. Size: %dKb. (Total: Unknown, gfx mem tracking disabled).\n", GFXFormat::GetGFXFormatString( fmt ), sizeKB );
		}
#		endif
	}

	bool bInvalid = false;
	bool bValidatePow2 = true;

	if (strstr( pDebugTag, "_npow2" ))
		bValidatePow2 = false;

	bool bValidateMips = true;
	if (strstr( pDebugTag, "_nomip" ))
		bValidateMips = false;

	if ((bValidateMips) && ((fileHeader.dwWidth>1) || (fileHeader.dwHeight>1)) && (fileHeader.dwMipMapCount <= 1))
	{
		Debug::Printf( Debug::DCU_TEXTURE, "ERROR: TEX_SIZE: Texture %s does not have correct mip levels. Please generate mip chain or use _nomip in name.\n",pDebugTag);
		bInvalid = true;
	}

	// dimension validation
	if (fileHeader.dwWidth > 4096)
	{
		Debug::Printf( Debug::DCU_TEXTURE, "ERROR: TEX_SIZE: Texture %s is too wide (%d pixels). Please downsize it.\n",pDebugTag, fileHeader.dwWidth);
		bInvalid = true;
	}

	if (fileHeader.dwHeight > 4096)
	{
		Debug::Printf( Debug::DCU_TEXTURE, "ERROR: TEX_SIZE: Texture %s is too high (%d pixels). Please downsize it.\n",pDebugTag, fileHeader.dwHeight);
		bInvalid = true;
	}

	if (!Util::IsPow2(fileHeader.dwWidth) && bValidatePow2)
	{
		Debug::Printf( Debug::DCU_TEXTURE, "ERROR: TEX_SIZE: Texture %s is not a power of 2 in width (%d pixels). Please fit to best power of two.\n",pDebugTag, fileHeader.dwWidth);
		bInvalid = true;
	}

	if (!Util::IsPow2(fileHeader.dwHeight) && bValidatePow2)
	{
		Debug::Printf( Debug::DCU_TEXTURE, "ERROR: TEX_SIZE: Texture %s is not a power of 2 in width (%d pixels). Please fit to best power of two.\n",pDebugTag, fileHeader.dwHeight);
		bInvalid = true;
	}

	if (fmt == GF_UNKNOWN)
	{
		Debug::Printf( Debug::DCU_TEXTURE, "ERROR: TEX_FMT: Texture %s uses an unsupported texture format. Please change.\n",pDebugTag);
		bInvalid = true;
	}

	// name descriptor / format validation
	if (strstr( pDebugTag, "_colour_mono" ) || strstr( pDebugTag, "_colour_alpha" ))
	{
		if (fmt != GF_DXT5)
		{
			Debug::Printf( Debug::DCU_TEXTURE, "ERROR: TEX_FMT: Texture %s should be DXT5 compressed. Please change.\n",pDebugTag);
			bInvalid = bInvalid ? bInvalid : CRendererSettings::bCheckTextureNameValid;
		}
	}
	else if (strstr( pDebugTag, "_colour" ))
	{
		if (fmt != GF_DXT1)
		{
			Debug::Printf( Debug::DCU_TEXTURE, "ERROR: TEX_FMT: Texture %s should be DXT1 compressed. Please change.\n",pDebugTag);
			bInvalid = bInvalid ? bInvalid : CRendererSettings::bCheckTextureNameValid;
		}
	}
	else if (strstr( pDebugTag, "_normal_mono" ))
	{
		if (fmt != GF_DXT5)
		{
			Debug::Printf( Debug::DCU_TEXTURE, "ERROR: TEX_FMT: Normal/Parallax map %s should be DXT5 compressed. Please change.\n",pDebugTag);
			bInvalid = bInvalid ? bInvalid : CRendererSettings::bCheckTextureNameValid;
		}
	}
	else if (strstr( pDebugTag, "_normal_dxt1" ))
	{
		if (fmt != GF_DXT1)
		{
			Debug::Printf( Debug::DCU_TEXTURE, "ERROR: TEX_FMT: Normal_dxt1 map %s should be DXT1 compressed. Please change.\n",pDebugTag);
			bInvalid = bInvalid ? bInvalid : CRendererSettings::bCheckTextureNameValid;
		}
	}
	else if (strstr( pDebugTag, "_normal" ) )
	{
		if (fmt != GF_L8A8)
		{
			Debug::Printf( Debug::DCU_TEXTURE, "ERROR: TEX_FMT: Normal map %s should be GF_L8A8 format. Please change.\n",pDebugTag);
			bInvalid = bInvalid ? bInvalid : CRendererSettings::bCheckTextureNameValid;
		}
	}
	else if (strstr( pDebugTag, "_mono" ))
	{
		if (fmt != GF_L8)
		{
			Debug::Printf( Debug::DCU_TEXTURE, "ERROR: TEX_FMT: Texture %s should be L8 format. Please change.\n",pDebugTag);
			bInvalid = bInvalid ? bInvalid : CRendererSettings::bCheckTextureNameValid;
		}
	}
	else if (strstr( pDebugTag, "_alpha" ))
	{
		if (fmt != GF_A8)
		{
			Debug::Printf( Debug::DCU_TEXTURE, "ERROR: TEX_FMT: Texture %s should be A8 format. Please change.\n",pDebugTag);
			bInvalid = bInvalid ? bInvalid : CRendererSettings::bCheckTextureNameValid;
		}
	}
	else
	{
		Debug::Printf( Debug::DCU_TEXTURE, "WARNING: TEX_USE: Cannot identify usage of texture %s. Is is named correctly?\n",pDebugTag);
		bInvalid = bInvalid ? bInvalid : CRendererSettings::bCheckTextureNameValid;
	}

	return bInvalid;
}
#endif

/***************************************************************************************************
*
*	FUNCTION		TextureManager::LoadTexture_FromData
*
*	DESCRIPTION		Loads a texture from a raw block of data, using a cache
*					key that corresponds to the hashed result of MakePlatformTextureName()
*					to insert texture into the cache.
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

	// see if there's anything wrong with this texture
#ifdef USE_TEXTURE_LOGGING
	if	(
		(CRendererSettings::bLogTextureErrors || CRendererSettings::bReplaceInvalidTextures) && 
		(bForceLoad == false) &&
		(CRendererSettings::bUseGTFTextures == false)
		)
	{
		if (LogTextureLoad( pTextureData, fileSize, pDebugTag ) && CRendererSettings::bReplaceInvalidTextures)
		{
			Debug::Printf( Debug::DCU_TEXTURE, "\n!!! ERROR: Replacing texture: %s. Using shiny green texture instead. |||\n\n", pDebugTag );

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

	// passed all debug switches, load the real texture
	Texture* pTex = NT_NEW_CHUNK(Mem::MC_GFX) Texture;

#ifdef TRACK_GFX_MEM
	uint32_t vramStart = GcKernel::GetFreeVram();
#endif

	if ( CRendererSettings::bUseGTFTextures )
		pTex->m_Platform.m_hTexture	= GpGtf::LoadTexture( pTextureData, fileSize );
	else
		pTex->m_Platform.m_hTexture	= GcTexture::Create( pTextureData, fileSize );

	// validate we got a senstible GcTexture format back
#ifndef _RELEASE
	GFXFORMAT fmt = ConvertGCFORMATToGFXFORMAT( pTex->m_Platform.GetGCFormat(), false );
	ntError_p( fmt != GF_UNKNOWN, ("Invalid texture %s.", pDebugTag) );
#endif

	pTex->m_iWidth					= pTex->m_Platform.m_hTexture->GetWidth();
	pTex->m_iHeight					= pTex->m_Platform.m_hTexture->GetHeight();
	pTex->m_iMipCount				= pTex->m_Platform.m_hTexture->GetMipLevelCount();
	pTex->m_Platform.m_pThis		= pTex;
	pTex->m_Platform.m_bImplicit	= false;

	// add it to the cache
	result = Texture::Ptr( pTex );
	m_cache[ cacheKey ] = result;

#ifdef TRACK_GFX_MEM
	pTex->m_iDiskSize = vramStart - GcKernel::GetFreeVram();
	TRACK_GFX_ALLOC_DISK_TEX( pTex );	
#endif

	return result;
}
