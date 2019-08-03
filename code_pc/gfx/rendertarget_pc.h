#if !defined(GFX_RENDERTARGET_PC_H)
#define GFX_RENDERTARGET_PC_H
//-----------------------------------------------------
//!
//!	\file gfx\rendertarget_pc.h
//! platform specific bit for PC of render target
//!
//-----------------------------------------------------


// forward decl
class RenderTarget;
class Texture;
//-----------------------------------------------------
//!
//! The PC specific bits of a render target
//!
//-----------------------------------------------------
class RenderTargetPlatform
{
public:
	friend class RenderTarget;
	friend class SurfaceManager;
	friend class SurfaceManagerPlatform;

	//! pointer back to container
//	RenderTarget*	pThis;

	//! platform specific render target creation paramater (This should expose as much as possible)
	struct CreationStruct
	{
		//! normal ctor for PC specific (no multisample yet, do we need to on PC?)
		CreationStruct( uint32_t width, uint32_t height, 
						D3DFORMAT eformat,
						bool discard,
						bool dynamic = false,
						bool hardwareSMap = false ) :
			bFromSurface( false ),
			bImplicit( false ),
			bCacheable( true ),
			iWidth( width ),
			iHeight( height ),
			eFormat( eformat ),
			bDiscard( discard ),
			bDynamic( dynamic ),
			bHardwareSMap( hardwareSMap )
		{
			if ( bHardwareSMap )
			{
				ntError_p( !bDiscard, ("Hardware shadow maps are always used as textures") );
				ntError_p( !bDynamic, ("Dynamic usage makes no sense here") );
			}
		}

		//! special ctor for swap chain surfaces
		CreationStruct( IDirect3DSurface9* psurf, bool implicit ) : 
			bFromSurface( true ),
			bImplicit( implicit ),
			bCacheable( !implicit ),
			pSurface( psurf )
		{
		}
		bool bFromSurface;				//!< Special case for swap chain render targets
		bool bImplicit;
		bool bCacheable;					//!< should this texture be cached when released? (aslong as its not bImplicit)
		IDirect3DSurface9* pSurface;	//!< Create a render target object from an already created surface
		uint32_t iWidth;				//!< width of render target
		uint32_t iHeight;				//!< height of render target
		D3DFORMAT eFormat;				//!< D3D format of the render target
		bool bDiscard;					//!< Never used as a texture, discarded when finished rendering to
		bool bDynamic;					//!< create this with the dyamic flag
		bool bHardwareSMap;				//!< create with the depth stencil flag
	};

	//! Get the texture interface.
	IDirect3DTexture9* GetTexture() { return m_pTexture; }

	//! Get the surface interface.
	IDirect3DSurface9* GetSurface() { return m_pRtSurface; }

	//! Get d3d format
	D3DFORMAT GetDXFormat() const { return m_eFormat; }

	//! Is this render always discard and never turned into a texture
	bool IsAlwaysDiscarded() const { return m_bAlwaysDiscarded; }

	//! was this surface created with the D3D dynamic flag
	bool IsD3DDynamic() const { return m_bDynamic; }

	//! so render targets based on this one can copy its caching strategy
	bool IsCacheable() const { return m_bCacheable; }

private:
	D3DFORMAT		m_eFormat;				//!< D3D format of render target
	bool			m_bAlwaysDiscarded;		//!< Never used as a texture, depth stencil and the real back buffer use this
	bool			m_bDynamic;				//!< Render target was created with the DYNAMIC flag
	IDirect3DTexture9* m_pTexture;			//!< backing texture can be null for discard type render target
	IDirect3DSurface9* m_pRtSurface;		//!< The Render target Surface
	bool			m_bImplicit;			//!< Is the render target something else in disguise
	bool			m_bCacheable;			//!< should this render target be cached when released? (aslong as its not bImplicit)
};

#endif //GFX_RENDERTARGET_PC_H