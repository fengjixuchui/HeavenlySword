#if !defined(GFX_GFXFORMAT_PC_H)
#define GFX_GFXFORMAT_PC_H

//-----------------------------------------------------
//!
//!	\file gfx\gfxformat_pc.h
//! platform specific gfx formats (PC Version)
//!
//-----------------------------------------------------

//! Takes a GFX_FORMAT and returns a D3DFORMAT you can pump into Direct3D.
inline D3DFORMAT ConvertGFXFORMATToD3DFORMAT( GFXFORMAT eFormat )
{
	ntAssert( eFormat != GF_UNKNOWN );

	//! table lookup from our format to d3dformat
	static D3DFORMAT GFXFORMATToD3DFORMATMap[] = 
	{
		D3DFMT_R8G8B8,			// GF_RGB8
		D3DFMT_A8R8G8B8,		// GF_ARGB8
		D3DFMT_X8R8G8B8,		// GF_XRGB8
		D3DFMT_UNKNOWN,			// GF_BGRA8
		
		D3DFMT_L8,				// GF_L8
		D3DFMT_A8,				// GF_A8
		D3DFMT_A8L8,			// GF_L8A8
		
		D3DFMT_A16B16G16R16F,	// GF_ABGR16F
		D3DFMT_A32B32G32R32F,	// GF_ABGR32F
		D3DFMT_R16F,			// GF_R16F
		D3DFMT_R32F,			// GF_R32F

		D3DFMT_D24S8,			// GF_D24S8

		D3DFMT_DXT1,			// GF_DXT1
		D3DFMT_DXT3,			// GF_DXT3
		D3DFMT_DXT5,			// GF_DXT5

		D3DFMT_V8U8,
	};

	return GFXFORMATToD3DFORMATMap[ eFormat ];
}

//! Takes a Direct3D can returns a cross-platform code (or 0 for platform specific).
inline GFXFORMAT ConvertD3DFORMATToGFXFORMAT( D3DFORMAT eFormat )
{
	switch( eFormat )
	{
	case D3DFMT_R8G8B8:		return GF_RGB8;
	case D3DFMT_A8R8G8B8:	return GF_ARGB8;
	case D3DFMT_X8R8G8B8:	return GF_XRGB8;
	case D3DFMT_L8:			return GF_L8;
	case D3DFMT_A8:			return GF_A8;
	case D3DFMT_A8L8:		return GF_L8A8;

	case D3DFMT_A16B16G16R16F:	return GF_ABGR16F;
	case D3DFMT_A32B32G32R32F:	return GF_ABGR32F;
	case D3DFMT_R16F:			return GF_R16F;
	case D3DFMT_R32F:			return GF_R32F;

	case D3DFMT_D24S8:		return GF_D24S8;

	case D3DFMT_DXT1:		return GF_DXT1;
	case D3DFMT_DXT3:		return GF_DXT3;
	case D3DFMT_DXT5:		return GF_DXT5;
	case D3DFMT_V8U8:		return GF_V8U8;

	default:				return GF_UNKNOWN;
	};
}

//! Takes a PRIMTYPE and returns a D3DPRIMITIVETYPE you can pump into Direct3D.
inline D3DPRIMITIVETYPE ConvertPRIMTYPEToD3DPRIM( PRIMTYPE eFormat )
{
	ntAssert( eFormat != PT_UNKNOWN );

	static D3DPRIMITIVETYPE PRIMTYPEToD3DPRIMMap[] = 
	{
		D3DPT_POINTLIST,		// PT_POINTLIST
		
		D3DPT_LINELIST,			// PT_LINELIST
		D3DPT_FORCE_DWORD,		// PT_LINELOOP		// UNSUPPORTED!
		D3DPT_LINESTRIP,		// PT_LINESTRIP
		
		D3DPT_TRIANGLELIST,		// PT_TRIANGLELIST
		D3DPT_TRIANGLESTRIP,	// PT_TRIANGLESTRIP
		D3DPT_TRIANGLEFAN,		// PT_TRIANGLEFAN
		
		D3DPT_FORCE_DWORD,		// PT_QUADLIST		// UNSUPPORTED!
		D3DPT_FORCE_DWORD,		// PT_QUADSTRIP		// UNSUPPORTED!
		D3DPT_FORCE_DWORD,		// PT_POLYGON		// UNSUPPORTED!
	};
	return PRIMTYPEToD3DPRIMMap[ eFormat ];
}

//! Takes a D3DPRIMITIVETYPE can returns a cross-platform code
inline PRIMTYPE ConvertD3DPRIMToPRIMTYPE( D3DPRIMITIVETYPE eFormat )
{
	switch( eFormat )
	{
	case D3DPT_POINTLIST:		return PT_POINTLIST;		
	case D3DPT_LINELIST:		return PT_LINELIST;	
	case D3DPT_LINESTRIP:		return PT_LINESTRIP;
	case D3DPT_TRIANGLELIST:	return PT_TRIANGLELIST;
	case D3DPT_TRIANGLESTRIP:	return PT_TRIANGLESTRIP;
	case D3DPT_TRIANGLEFAN:		return PT_TRIANGLEFAN;
	default:					return PT_UNKNOWN;
	};
}

#endif // end GFX_GFXFORMAT_PC_H