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
#include "gfx/surfacemanager.h"
#include "gfx/texturereader.h"

/***************************************************************************************************
*
*	FUNCTION		TextureManager::ctor
*
*	DESCRIPTION		
*
***************************************************************************************************/
TextureManager::TextureManager()
{
	m_pMissingTexture = CreateProcedural2DTexture( 2, 2, GF_ABGR16F, CVector( 100.0f, 0.0f, 0.0f, 1.0f ) );
	ntAssert_p( m_pMissingTexture, ("Failed to create missing texture") );

	if (CRendererSettings::bLogTextureErrors || CRendererSettings::bReplaceInvalidTextures)
	{
		m_pInvalidTexture = CreateProcedural2DTexture( 2, 2, GF_ABGR16F, CVector( 0.0f, 100.0f, 0.0f, 1.0f ) );
		ntAssert_p( m_pInvalidTexture, ("Failed to create invalid texture") );
	}

	if (CRendererSettings::bUseTextures == false)
	{
		m_pWhiteTexture = CreateProcedural2DTexture( 2, 2, GF_ARGB8, CVector( 1.0f, 1.0f, 1.0f, 1.0f ) );
		ntAssert_p( m_pWhiteTexture, ("Failed to create white texture") );
	}
}

/***************************************************************************************************
*
*	FUNCTION		TextureManager::~CTextureManager
*
*	DESCRIPTION		Unloads all managed textures from the list.
*
***************************************************************************************************/
TextureManager::~TextureManager()
{
	Clear();

	if (m_pMissingTexture)
		SurfaceManager::Get().ReleaseTexture( m_pMissingTexture );
	
	if (m_pInvalidTexture)
		SurfaceManager::Get().ReleaseTexture( m_pInvalidTexture );
	
	if (m_pWhiteTexture)
		SurfaceManager::Get().ReleaseTexture( m_pWhiteTexture );
}

/***************************************************************************************************
*
*	FUNCTION		TextureManager::CreateProcedural2DTexture
*
*	DESCRIPTION		generic method for creating defualt textures
*
***************************************************************************************************/
Texture::Ptr TextureManager::CreateProcedural2DTexture( uint32_t iWidth, uint32_t iHeight, GFXFORMAT fmt, const CVector& colour )
{
	// create our texture
	Texture::Ptr result = SurfaceManager::Get().CreateTexture( iWidth, iWidth, fmt );

	// lock it and fill it in with our colour values
	uint32_t pitch;
	uint8_t* pStart = (uint8_t*)result->CPULock2D(pitch);

	for (uint32_t i = 0; i < iHeight; i++)
	{
		TextureReader it( pStart, fmt );
		for ( uint32_t j = 0; j < iWidth; j++, it.Next() )
			it.Set( colour );

		pStart += pitch;
	}

	return result;
}

/***************************************************************************************************
*
*	FUNCTION		TextureManager::Clear
*
*	DESCRIPTION		Unloads all managed textures from the list.
*
***************************************************************************************************/
void TextureManager::Clear()
{
#ifdef TRACK_GFX_MEM
	for (	textureCache::iterator it = m_cache.begin(); 
			it != m_cache.end(); ++it )
	{
		TRACK_GFX_FREE_DISK_TEX( it->second );
	}
#endif

	m_cache.clear();
	m_errorCache.clear();
}

/***************************************************************************************************
*
*	FUNCTION		TextureManager::LoadTexture_Neutral
*
*	DESCRIPTION		Loads a 2D texture from the cache, or off disk if it's not found.
*
***************************************************************************************************/
Texture::Ptr TextureManager::LoadTexture_Neutral( const char* pNeutralName, bool bForceLoad)
{
	char pPlatformName[MAX_PATH];
	MakePlatformTextureName(pNeutralName,pPlatformName);
	return LoadTexture_Platform(pPlatformName, bForceLoad);
}

/***************************************************************************************************
*
*	FUNCTION		TextureManager::LoadTexture_Platform
*
*	DESCRIPTION		Loads a texture from the cache, or off disk if it's not found.
*
***************************************************************************************************/
Texture::Ptr TextureManager::LoadTexture_Platform( char const* pPlatformName, bool bForceLoad )
{
	CHashedString name(pPlatformName);
	
	// check the cache first
	Texture::Ptr result = FindFromCache( name.GetValue() );
	if (result)
		return result;

#ifdef PLATFORM_PC
	Util::SetToPlatformResources();
#endif

	// nope proceed with load
	uint8_t *pReadResult = NULL;
	size_t fileSize = 0;
	if (File::Exists( pPlatformName ))
	{
		File dataFile;
		LoadFile_Chunk( pPlatformName, File::FT_READ | File::FT_BINARY, Mem::MC_GFX, dataFile, &pReadResult );
		fileSize = dataFile.GetFileSize();
	}

	result = LoadTexture_FromData( name.GetValue(), (void *)pReadResult, fileSize, pPlatformName, bForceLoad );

	// free temp buffers and return result
	if (pReadResult)
	{
		NT_DELETE_ARRAY_CHUNK( Mem::MC_GFX, pReadResult );
	}

#ifdef PLATFORM_PC
	Util::SetToNeutralResources();
#endif

	return result;
}

/***************************************************************************************************
*
*	FUNCTION		TextureManager::UnloadTexture_Neutral
*
*	DESCRIPTION		Unload a texture from the cache. Return true if it was genuinely released
*
***************************************************************************************************/
bool TextureManager::UnloadTexture_Neutral( char const* pNeutralName )
{
	char pPlatformName[MAX_PATH];
	MakePlatformTextureName(pNeutralName,pPlatformName);
	return UnloadTexture_Platform(pPlatformName);
}

/***************************************************************************************************
*
*	FUNCTION		TextureManager::UnloadTexture_Platform
*
*	DESCRIPTION		Unload a texture from the cache. Return true if it was genuinely released
*
***************************************************************************************************/
bool TextureManager::UnloadTexture_Platform( char const* pPlatformName )
{
	CHashedString name( pPlatformName );

	// check normal cache
	textureCache::iterator tex = m_cache.find( name.GetValue() );
	if (tex != m_cache.end())
	{
		TRACK_GFX_FREE_DISK_TEX( tex->second );
		m_cache.erase( tex );
		return true;
	}

	// check error cache
	tex = m_errorCache.find( name.GetValue() );
	if (tex != m_errorCache.end())
	{
		m_errorCache.erase( tex );
		return true;
	}

	return false;
}

/***************************************************************************************************
*
*	FUNCTION		TextureManager::Loaded_Neutral
*
*	DESCRIPTION		Looks to see if a 2D texture exists, in the cache
*
***************************************************************************************************/
bool TextureManager::Loaded_Neutral( char const* pNeutralName )
{
	char pPlatformName[MAX_PATH];
	MakePlatformTextureName(pNeutralName,pPlatformName);
	return Loaded_Platform(pPlatformName);
}

/***************************************************************************************************
*
*	FUNCTION		TextureManager::Loaded_Platform
*
*	DESCRIPTION		Looks to see if a 2D texture exists, in the cache
*
***************************************************************************************************/
bool TextureManager::Loaded_Platform( char const* pPlatformName )
{
	CHashedString name( pPlatformName );
	return Loaded_Key( name.GetValue() );
}

/***************************************************************************************************
*
*	FUNCTION		TextureManager::Loaded_Key
*
*	DESCRIPTION		Looks to see if a texture exists in the cache
*
***************************************************************************************************/
bool TextureManager::Loaded_Key( uint32_t cacheKey )
{
	// check normal cache
	textureCache::iterator tex = m_cache.find( cacheKey );
	if (tex != m_cache.end())
		return true;

	// check error cache
	tex = m_errorCache.find( cacheKey );
	if (tex != m_errorCache.end())
		return true;

	return false;
}

/***************************************************************************************************
*
*	FUNCTION		TextureManager::FindFromCache
*
*	DESCRIPTION		Looks to see if a texture exists in the cache
*
***************************************************************************************************/
Texture::Ptr TextureManager::FindFromCache( uint32_t cacheKey )
{
	// check normal cache
	textureCache::iterator tex = m_cache.find( cacheKey );
	if (tex != m_cache.end())
		return tex->second;

	// check error cache
	tex = m_errorCache.find( cacheKey );
	if (tex != m_errorCache.end())
		return tex->second;

	return Texture::Ptr();
}

//--------------------------------------------------
//!
//!	TextureManager::MakePlatformTextureName
//!	Transform initial name into platform specific
//! file name.
//!
//--------------------------------------------------
void	TextureManager::MakePlatformTextureName( const char* pNeutralName, char* pPlatformName )
{
#ifndef _RELEASE
	// check for some very special case errors
	ntError_p( pNeutralName[0] != '\\', ("Texture path %s has a leading backslash, please remove.", pNeutralName) );
	ntError_p( pNeutralName[0] != '/', ("Texture path %s has a leading forwardslash, please remove.", pNeutralName) );
#endif

	// stick on data directory and .dds file extension
	char pTemp[MAX_PATH];
	strcpy( pTemp, TEXTURE_ROOT_PATH );
	strcat( pTemp, pNeutralName );

	if (CRendererSettings::bUseGTFTextures)
		strcpy( strstr( pTemp, "." ), ".gtf" );
	else
		strcpy( strstr( pTemp, "." ), ".dds" );

	// appends file system root and content directory
	Util::GetFiosFilePath_Platform( pTemp, pPlatformName );
}

/***************************************************************************************************
*
*	FUNCTION		TextureManager::Exists_Neutral
*
*	DESCRIPTION		Looks to see if a 2D texture exists, in the cache or on disk
*
***************************************************************************************************/
bool TextureManager::Exists_Neutral( char const* pNeutralName )
{
	char pPlatformName[MAX_PATH];
	MakePlatformTextureName(pNeutralName,pPlatformName);
	return Exists_Platform(pPlatformName);
}

/***************************************************************************************************
*
*	FUNCTION		TextureManager::Exists_Platform
*
*	DESCRIPTION		Looks to see if a 2D texture exists, in the cache or on disk
*
***************************************************************************************************/
bool TextureManager::Exists_Platform( char const* pPlatformName )
{
	if ( Loaded_Platform(pPlatformName) )
		return true;

	Util::SetToPlatformResources();

	// is the DDS present
	if( File::Exists( pPlatformName ) )
	{
		Util::SetToNeutralResources();
		return true;
	}

	Util::SetToNeutralResources();
	return false;
}
