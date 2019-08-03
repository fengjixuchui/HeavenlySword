#if !defined(GFX_GFXFORMAT_PS3_H)
#define GFX_GFXFORMAT_PS3_H
//-----------------------------------------------------
//!
//!	\file gfx\gfxformat_ps3.h
//! platform specific gfx formats (PS3 Version)
//!
//-----------------------------------------------------

#include <Gc/Gc.h>

//! Takes a GFX_FORMAT and returns a Gc::TexFormat you can pump into Gc.
inline Gc::TexFormat ConvertGFXFORMATToGCFORMAT( GFXFORMAT eFormat )
{
	ntAssert( eFormat != GF_UNKNOWN );

	switch (eFormat)
	{
	case GF_ARGB8:		return Gc::kTexFormatARGB8;
	case GF_BGRA8:		return Gc::kTexFormatBGRA8;
	case GF_L8:			return Gc::kTexFormatL8;
	case GF_A8:			return Gc::kTexFormatA8;
	case GF_L8A8:		return Gc::kTexFormatL8A8;
	case GF_ABGR16F:	return Gc::kTexFormatRGBA16F;
	case GF_ABGR32F:	return Gc::kTexFormatRGBA32F;
	case GF_R32F:		return Gc::kTexFormatR32F;
	case GF_D24S8:		return Gc::kTexFormatD24X8;
	case GF_DXT1:		return Gc::kTexFormatDXT1;
	case GF_DXT3:		return Gc::kTexFormatDXT3;
	case GF_DXT5:		return Gc::kTexFormatDXT5;
	default:
		ntAssert_p( 0, ("Unsupported texture format type %d used!", eFormat ));
		return Gc::kTexFormatARGB8;
	};
}

inline Gc::BufferFormat ConvertGFXFORMATToGCBUFFERFORMAT( GFXFORMAT eFormat )
{
	ntAssert( eFormat != GF_UNKNOWN );
	
	switch( eFormat )
	{
	case GF_ARGB8:		return Gc::kBufferFormatARGB8;
	case GF_ABGR16F:	return Gc::kBufferFormatRGBA16F;
	case GF_ABGR32F:	return Gc::kBufferFormatRGBA32F;
	case GF_R32F:		return Gc::kBufferFormatR32F;
	case GF_D24S8:		return Gc::kBufferFormatD24S8;
	default:
		ntAssert_p( 0, ("GFXFORMAT %d is invalid as a render target buffer format on PS3", eFormat ) );
		return Gc::kBufferFormatARGB8;
	}
}

inline Gc::MultisampleMode ConvertGFXAAMODEToGCMULTISAMPLEMODE( GFXAAMODE eAAMode )
{
	ntAssert( eAAMode != GAA_UNKNOWN );

	switch( eAAMode )
	{
		case GAA_MULTISAMPLE_NONE:	return Gc::kMultisampleNone;
		case GAA_MULTISAMPLE_4X:	return Gc::kMultisample4xRotated;
		case GAA_MULTISAMPLE_ORDERED_GRID_4X:	return Gc::kMultisample4xOrdered;

		default:
		ntAssert_p( 0, ("GFXAAMODE %d is invalid as anti-aliasing mode on PS3", eAAMode ) );
		return Gc::kMultisampleNone;
	}
}


//! Takes a Gc::TexFormat can returns a cross-platform code (or 0 for platform specific).
inline GFXFORMAT ConvertGCFORMATToGFXFORMAT( Gc::TexFormat eFormat, bool bAssertOnInvalid = true )
{
	switch( eFormat )
	{
	case Gc::kTexFormatARGB8:		return GF_ARGB8;
	case Gc::kTexFormatBGRA8:		return GF_BGRA8;
	case Gc::kTexFormatL8:			return GF_L8;
	case Gc::kTexFormatA8:			return GF_A8;
	case Gc::kTexFormatL8A8:		return GF_L8A8;
	case Gc::kTexFormatRGBA16F:		return GF_ABGR16F;
	case Gc::kTexFormatRGBA32F:		return GF_ABGR32F;
	case Gc::kTexFormatR32F:		return GF_R32F;
	case Gc::kTexFormatD24X8:		return GF_D24S8;
	case Gc::kTexFormatDXT1:		return GF_DXT1;
	case Gc::kTexFormatDXT3:		return GF_DXT3;
	case Gc::kTexFormatDXT5:		return GF_DXT5;
	default:
		if (bAssertOnInvalid)
		{	
			ntAssert_p( 0, ("Gc::TexFormat %d does not have an equivalent GFXFORMAT", eFormat ) );
		}
		return GF_UNKNOWN;
	};
}

//! Takes a Gc::MultisampleMode can returns a cross-platform code (or 0 for platform specific).
inline GFXAAMODE ConvertGCAAMODEToGFXAAMODE( Gc::MultisampleMode eAAMode )
{
	switch( eAAMode )
	{
		case Gc::kMultisampleNone:			return GAA_MULTISAMPLE_NONE;
		case Gc::kMultisample4xRotated:		return GAA_MULTISAMPLE_4X;
		case Gc::kMultisample4xOrdered:		return GAA_MULTISAMPLE_ORDERED_GRID_4X;

		default:
			ntAssert_p( 0, ("Gc::MultisampleMode %d does not have an equivalent GFXAAMODE", eAAMode ) );
			return GAA_UNKNOWN;
	};
}

//! Takes a Gc::BufferFormat can returns a cross-platform code (or 0 for platform specific).
inline GFXFORMAT ConvertGCFORMATToGFXFORMAT( Gc::BufferFormat eFormat )
{
	switch( eFormat )
	{
	case Gc::kBufferFormatARGB8:		return GF_ARGB8;
	case Gc::kBufferFormatRGBA16F:		return GF_ABGR16F;
	case Gc::kBufferFormatRGBA32F:		return GF_ABGR32F;
	case Gc::kBufferFormatR32F:			return GF_R32F;
	case Gc::kBufferFormatD24S8:		return GF_D24S8;
	default:							return GF_UNKNOWN;
	};
}

//! Takes a PRIMTYPE and returns a Gc::PrimitiveType you can pump into Gc.
inline Gc::PrimitiveType ConvertPRIMTYPEToGCPRIM( PRIMTYPE eFormat )
{
	ntAssert( eFormat != PT_UNKNOWN );

	static Gc::PrimitiveType PRIMTYPEToGCPRIMMap[] = 
	{
		Gc::kPoints,		// PT_POINTLIST
		
		Gc::kLines,			// PT_LINELIST
		Gc::kLineLoop,		// PT_LINELOOP
		Gc::kLineStrip,		// PT_LINESTRIP

		Gc::kTriangles,		// PT_TRIANGLELIST
		Gc::kTriangleStrip,	// PT_TRIANGLESTRIP
		Gc::kTriangleFan,	// PT_TRIANGLEFAN

		Gc::kQuads,			// PT_QUADLIST
		Gc::kQuadStrip,		// PT_QUADSTRIP
		Gc::kPolygon,		// PT_POLYGON
	};
	return PRIMTYPEToGCPRIMMap[ eFormat ];
}

//! Takes a Gc::PrimitiveType can returns a cross-platform code
inline PRIMTYPE ConvertGCPRIMToPRIMTYPE( Gc::PrimitiveType eFormat )
{
	switch( eFormat )
	{
	case Gc::kPoints:			return PT_POINTLIST;		
	
	case Gc::kLines:			return PT_LINELIST;	
	case Gc::kLineLoop:			return PT_LINELOOP;	
	case Gc::kLineStrip:		return PT_LINESTRIP;

	case Gc::kTriangles:		return PT_TRIANGLELIST;
	case Gc::kTriangleStrip:	return PT_TRIANGLESTRIP;
	case Gc::kTriangleFan:		return PT_TRIANGLEFAN;

	case Gc::kQuads:			return PT_QUADLIST;
	case Gc::kQuadStrip:		return PT_QUADSTRIP;
	case Gc::kPolygon:			return PT_POLYGON;

	default:					return PT_UNKNOWN;
	};
}

namespace GFXFormat
{
	// for debug code
	inline const char* GetGcBufferFormatString( Gc::BufferFormat type )
	{
		switch( type )
		{
		case Gc::kBufferFormatRGB565:		return "Gc::kBufferFormatRGB565";
		case Gc::kBufferFormatARGB8:		return "Gc::kBufferFormatARGB8";
		case Gc::kBufferFormatB8:			return "Gc::kBufferFormatB8";
		case Gc::kBufferFormatGB8:			return "Gc::kBufferFormatGB8";
		case Gc::kBufferFormatRGBA16F:		return "Gc::kBufferFormatRGBA16F";
		case Gc::kBufferFormatRGBA32F:		return "Gc::kBufferFormatRGBA32F";
		case Gc::kBufferFormatR32F:			return "Gc::kBufferFormatR32F";
		case Gc::kBufferFormatD24S8:		return "Gc::kBufferFormatD24S8";
		case Gc::kBufferFormatD16S0:		return "Gc::kBufferFormatD16S0";
		}

		ntAssert_p( 0, ("Unrecognised graphics format :%d", type ) );
		return "";
	}

	inline const char* GetGcTextureFormatString( Gc::TexFormat type )
	{
		switch( type )
		{
		case Gc::kTexFormatL8:			return "Gc::kTexFormatL8";
		case Gc::kTexFormatL16:			return "Gc::kTexFormatL16";
		case Gc::kTexFormatA8:			return "Gc::kTexFormatA8";
		case Gc::kTexFormatA16:			return "Gc::kTexFormatA16";
		case Gc::kTexFormatL8A8:		return "Gc::kTexFormatL8A8";
		case Gc::kTexFormatL16A16:		return "Gc::kTexFormatL16A16";
		case Gc::kTexFormatRGB565:		return "Gc::kTexFormatRGB565";			
		case Gc::kTexFormatARGB8:		return "Gc::kTexFormatARGB8";
		case Gc::kTexFormatARGB4444:	return "Gc::kTexFormatARGB4444";
		case Gc::kTexFormatARGB1555:	return "Gc::kTexFormatARGB1555";
		case Gc::kTexFormatBGRA8:		return "Gc::kTexFormatBGRA8";
		case Gc::kTexFormatRGBA8:		return "Gc::kTexFormatRGBA8";
		case Gc::kTexFormatDXT1:		return "Gc::kTexFormatDXT1";
		case Gc::kTexFormatDXT3:		return "Gc::kTexFormatDXT3";
		case Gc::kTexFormatDXT5:		return "Gc::kTexFormatDXT5";
		case Gc::kTexFormatD24X8:		return "Gc::kTexFormatD24X8";
		case Gc::kTexFormatD16:			return "Gc::kTexFormatD16";		
		case Gc::kTexFormatR32F:		return "Gc::kTexFormatR32F";
		case Gc::kTexFormatRGBA16F:		return "Gc::kTexFormatRGBA16F";
		case Gc::kTexFormatRGBA32F:		return "Gc::kTexFormatRGBA32F";
		case Gc::kTexFormatGR16F:		return "Gc::kTexFormatGR16F";
		}

		ntAssert_p( 0, ("Unrecognised graphics format :%d", type ) );
		return "";
	}
};

#endif // end GFX_GFXFORMAT_PS3_H
