#if !defined(GFX_TEXTURE_PS3_H)
#define GFX_TEXTURE_PS3_H
//-----------------------------------------------------
//!
//!	\file gfx\texture_ps3.h
//! platform specific bit for PS3 of texture
//!
//-----------------------------------------------------

#ifndef GC_TEXTURE_H
#include <Gc/GcTexture.h>
#endif

#ifndef GC_RENDER_BUFFER_H
#include <Gc/GcRenderBuffer.h>
#endif

#ifndef DDS_H
#include <Gc/Internal/dds.h>
#endif

class Texture;

//-----------------------------------------------------
//!
//! TODO
//!
//-----------------------------------------------------
class TexturePlatform
{
public:
	friend class Texture;
	friend class TextureManager;
	friend class SurfaceManager;
	friend class SurfaceManagerPlatform;

	struct CreationStruct
	{
		//! normal ctor for PS3 specific.
		CreationStruct( uint32_t width, uint32_t height, Gc::TexFormat eformat, uint32_t miplevels, bool swizzled = false ) :
			bFromRenderBuffer( false ),
			bImplicit( false ),
			bCacheable( true ),
			bSwizzled( swizzled ),
			iWidth( width ),
			iHeight( height ),
			iDepth( 1 ),
			eFormat( eformat ),
			iMipLevels( miplevels ),
			eType(TT_2D_TEXTURE)
		{
			if (bSwizzled)
			{
				ntAssert( FwIsPow2(iWidth) );
				ntAssert( FwIsPow2(iHeight) );
			}
		}

		//! creation from a already existing GcRenderBufferHandle.
		//! normally used in conjunction with render to texture
		CreationStruct( GcRenderBufferHandle hBuffer, bool implicit ) :
			bFromRenderBuffer( true ),
			bImplicit( implicit ),
			bCacheable( !implicit ),
			hRenderBuffer( hBuffer ),
			eType(TT_2D_TEXTURE) {}

		bool bFromRenderBuffer;				//!< Special case
		bool bImplicit;						//!< this texture is owned by another type of surface and shouldn't be cached
		bool bCacheable;						//!< should this texture be cached when released? (aslong as its not bImplicit)
		bool bSwizzled;						//!< is the texture swizzled?
		GcRenderBufferHandle hRenderBuffer;	//!< when converting from an existing surface this is the surface
		uint32_t iWidth;					//!< width of surface
		uint32_t iHeight;					//!< height of surface
		uint32_t iDepth;					//!< depth of surface (only used if we're a 3D texture)
		Gc::TexFormat eFormat;				//!< Gc format of the texture
		uint32_t iMipLevels;				//!< number of mip levels
		TEXTURE_TYPE eType;					//!< what type of texture are we?
	};

	//!< Get the texture interface.
	GcTextureHandle			GetTexture() { return m_hTexture; };
	const GcTextureHandle&	GetTexture() const { return m_hTexture; };

	Gc::TexFormat	GetGCFormat() const
	{
		return m_hTexture->GetFormat();
	}

	void SaveToDisk( const char* pName, bool bInDataDir = false );

	//!< Debug functionality to retrieve a dds header from a texture file
	static bool RetrieveTextureHeader( const char* pTexName, DDS::DDS_HEADER& header );	
	static void RetrieveTextureHeader( void* pRawTexData, DDS::DDS_HEADER& header );	

	//!< functionality to retrieve a Gc texture format from a DDS header
	static Gc::TexFormat GetTextureFormat( const DDS::DDS_HEADER& header );

private:
	const Texture*	m_pThis;
	GcTextureHandle	m_hTexture;
	bool			m_bImplicit; //!< Texture is a temporary as its actually another type of surface in disguise
	bool			m_bCacheable; //!< should this texture be cached when released? (aslong as its not bImplicit)
};

#endif // ndef GFX_TEXTURE_PS3_H

