//--------------------------------------------------
//!
//!	\file texture.h
//!	The main texture class
//!
//--------------------------------------------------

#ifndef GFX_TEXTURE_H
#define GFX_TEXTURE_H

#if !defined( FW_STD_INTRUSIVE_PTR_H )
#include <Fw/FwStd/FwStdIntrusivePtr.h>
#endif

#if !defined( GFX_GFXFORMAT_H )
#include "gfx/gfxformat.h"
#endif

//! what type of texture are we? (vast majority will be 2D)
enum TEXTURE_TYPE
{
	TT_2D_TEXTURE,
	TT_3D_TEXTURE,
	TT_CUBE_TEXTURE,
};

#if defined( PLATFORM_PC )
#	include "gfx/texture_pc.h"
#elif defined( PLATFORM_PS3 )
#	include "gfx/texture_ps3.h"
#endif

class Texture : public FwStd::IntrusivePtrCountedBase< Texture >
{
public:
	friend class TextureManager;
	friend class SurfaceManager;
	friend class SurfaceManagerPlatform;
	friend class ToolsTextureManager;

	typedef TexturePlatform::CreationStruct CreationStruct; 
	typedef	FwStd::IntrusivePtr<Texture>	Ptr;

	//! use when you don't want a texture
	enum NONE_ENUM{ NONE };

	//! width accessor
	uint32_t GetWidth() const
	{ 
		return m_iWidth; 
	}
	//! height accessor
	uint32_t GetHeight() const
	{ 
		return m_iHeight; 
	}
	//! depth accessor
	uint32_t GetDepth() const
	{ 
		ntAssert_p( GetType() == TT_3D_TEXTURE, ("Depth is only valid for 3D textures") );
		return m_iDepth; 
	}
	//! number of mip levels this texture has
	uint32_t GetMipCount() const
	{
		return m_iMipCount;
	}

	//! returns a generic format you may get a platform specific marker
	GFXFORMAT GetFormat() const;

	//! returns what type we are
	TEXTURE_TYPE GetType() const;

	//! CPU Lock, allow access to the bits.
	void* CPULock2D( uint32_t& pitch );
	void* CPULock3D( uint32_t& rowPitch, uint32_t& slicePitch );
	void* CPULockCube( int iFaceID, uint32_t& pitch );

	//! CPU unlock, tell the system you finished fiddling or reading
	void CPUUnlock2D();
	void CPUUnlock3D();
	void CPUUnlockCube( int iFaceID );

	//! platform dependent stuff
	TexturePlatform m_Platform; //!< platform specific 

	//! dtor
	~Texture();

	//! debug method to help estimate our VRAM usage
	inline uint32_t CalculateVRAMFootprint()
	{
		uint32_t iResult = GFXFormat::CalculateVRAMFootprint(	GetFormat(),
																m_iWidth,
																m_iHeight,
																m_iMipCount );
		
		if ( GetType() == TT_3D_TEXTURE )
			iResult *= m_iDepth;
		else if ( GetType() == TT_CUBE_TEXTURE )
			iResult *= 6;

		return iResult;
	}

	//! debug method to help estimate our VRAM usage
	static uint32_t GetTextureDiskSize( const char* pTexName );

#ifdef TRACK_GFX_MEM
	uint32_t m_iDiskSize;	//!< Only valid for loaded textures
#endif

private:
	uint32_t m_iWidth;		//!< width of the top level
	uint32_t m_iHeight;		//!< height of the top level
	uint32_t m_iDepth;		//!< depth; only valid in a 3D texture
	uint32_t m_iMipCount;	//!< mip count
};

//-----------------------------------------------------
//!
//!	Uncached texture that lives in main memory
//!	Loads a texture directly into main mem, and does not cache it.
//!	This is used for Large textures that we donot want in VRAM, such as
//!	the loading screen, title screens and GUI.
//! You MUST release these as soon as youre done with them.
//!
//-----------------------------------------------------
class TextureXDRAM
{
public:
	TextureXDRAM( const char* pName );
	~TextureXDRAM();

	const Texture::Ptr& GetTexture() const { return m_pTexturePtr; }

private:
	uintptr_t		m_pTextureMem;
	Texture::Ptr	m_pTexturePtr;
};

#endif // end GFX_TEXTURE_H
