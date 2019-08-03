#if !defined(GFX_SURFACE_PC_H)
#define GFX_SURFACE_PC_H
//-----------------------------------------------------
//!
//!	\file gfx\surface_pc.h
//! platform specific bit for PC of Surface
//!
//-----------------------------------------------------


// forward decl
class Surface;

//-----------------------------------------------------
//!
//! The PC specific bits of a surface
//!
//-----------------------------------------------------
class SurfacePlatform
{
public:
	friend class Surface;
	friend class SurfaceManager;
	friend class SurfaceManagerPlatform;

	//! platform specific render target creation paramater (This should expose as much as possible)
	struct CreationStruct
	{
		//! normal ctor for PC specific
		CreationStruct( uint32_t width, uint32_t height, D3DFORMAT eformat, D3DPOOL epool = D3DPOOL_DEFAULT) :
			bFromSurface( false ),
			bImplicit( false ),
			bCacheable( true ),
			iWidth( width ),
			iHeight( height ),
			eFormat( eformat ),
			ePool( epool )
		{
		}

		//! special ctor for getting surfaces from a texture
		CreationStruct( IDirect3DSurface9* psurf, bool implicit ) : 
			bFromSurface( true ),
			bImplicit( implicit ),
			bCacheable( !implicit ),
			pSurface( psurf )
		{
		}
		bool bFromSurface;				//!< Special case
		bool bImplicit;					//!< when converting from a existing surface is this an internal implicit surface that shouldn't be cached
		bool bCacheable;					//!< should this surface be cached when released? (aslong as its not bImplicit)
		IDirect3DSurface9* pSurface;	//!< Create a surface object from an already created surface
		uint32_t iWidth;				//!< width of surface
		uint32_t iHeight;				//!< height of surface
		D3DFORMAT eFormat;				//!< D3D format of the surfaec
		D3DPOOL ePool;					//!< which pool almost always D3DPOOL_DEFAULT
	};

	//!< Get the surface interface (unchecked for naughty people).
	IDirect3DSurface9* GetSurface()
	{
		return m_pSurface;
	}

	//! Get d3d format
	D3DFORMAT GetDXFormat() const
	{
		return m_eFormat;
	}

	//! get d3d pool 
	D3DPOOL GetPool() const
	{
		return m_ePool;
	}

	//! dump to disk
	void SaveToDisk( const char* pName, D3DXIMAGE_FILEFORMAT fmt, bool bAppendType = true, bool bInDataDir = false );

private:
	D3DFORMAT		m_eFormat;				//!< D3D format of surface
	IDirect3DSurface9* m_pSurface;			//!< The Surface
	D3DPOOL			m_ePool;				//!< Which memory pool this texture is from
	bool			m_bImplicit;			//!< Is the surface really something else in disguise
	bool			m_bCacheable;			//!< should this surface be cached when released? (aslong as its not bImplicit)
};

#endif //GFX_SURFACE_PC_H