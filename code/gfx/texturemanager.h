#if !defined(GFX_TEXTUREMANAGER_H)
#define GFX_TEXTUREMANAGER_H

#ifndef GFX_TEXTURE_H
#include "gfx/texture.h"
#endif

#define TEXTURE_ROOT_PATH	"data\\"

#if !defined( _RELEASE )
#define USE_TEXTURE_LOGGING
#endif

/***************************************************************************************************
*
*	CLASS			TextureManager
*
*	DESCRIPTION		Manages the texture cache, and VRAM resources.
*					Can load them explicitly, or have raw file data passed to them from the area
*					resource system.
*
***************************************************************************************************/
class TextureManager : public Singleton<TextureManager>
{
public:
	TextureManager();

	//! Unloads all textures.
	~TextureManager();

	//! Unloads all textures.
	void Clear();

	//! Make a platform specific texture name
	//! the hash of this 
	static void	MakePlatformTextureName( const char* pNeutralName, char* pPlatformName );

	//! Test if a texture exists or not
	bool Exists_Neutral( char const* pNeutralName );
	bool Exists_Platform( char const* pPlatformName );

	//! Test if a texture is loaded already
	bool Loaded_Neutral( char const* pNeutralName );
	bool Loaded_Platform( char const* pPlatformName );
	bool Loaded_Key( uint32_t cacheKey );

	//! Loads a texture either from the cache or from disk.
	Texture::Ptr LoadTexture_Neutral( char const* pNeutralName, bool bForceLoad = false );
	Texture::Ptr LoadTexture_Platform( char const* pPlatformName, bool bForceLoad = false );

	//! Loads a texture from a raw block of DDS data, using a cache
	//! key that corresponds to the hashed result of MakePlatformTextureName()
	Texture::Ptr LoadTexture_FromData(	uint32_t cacheKey,
										void* pTextureData, 
										size_t fileSize,
										const char* pDebugTag,
										bool bForceLoad = false );

	//! Unloads a texture from the cache. Return true if it was genuinely released.
	bool UnloadTexture_Neutral( char const* pNeutralName );
	bool UnloadTexture_Platform( char const* pPlatformName );
	bool UnloadTexture_Key( uint32_t cacheKey );

	static Texture::Ptr	CreateProcedural2DTexture( uint32_t iWidth, uint32_t iHeight, GFXFORMAT fmt, const CVector& colour );

#ifdef PLATFORM_PC
	// this is for an uncached load to help emulate XDDR textures. DO NOT USE!!!!!!!
	Texture::Ptr DebugLoadTexture( char const* pName );
#endif

	// some debug texture checking, only works for DDS files
	static bool		LogTextureLoad( void* pTextureData, size_t fileSize, const char* pTexName );

	// get error textures
	Texture::Ptr	GetMissingTexture() const { return m_pMissingTexture; }
	Texture::Ptr	GetInvalidTexture() const { return m_pInvalidTexture; }
	Texture::Ptr	GetWhiteTexture() const { return m_pWhiteTexture; }

private:
	// cache entry names are always lowercase, platform specific
	Texture::Ptr	FindFromCache( uint32_t cacheKey );

	Texture::Ptr	m_pMissingTexture;
	Texture::Ptr	m_pInvalidTexture;
	Texture::Ptr	m_pWhiteTexture;

	// simple cache that uses a CHashedString of the texture name as the key
	typedef	ntstd::Map<uint32_t,Texture::Ptr> textureCache;
	textureCache	m_cache;
	textureCache	m_errorCache;
};

#endif // ndef _TEXTUREMANAGER_H
