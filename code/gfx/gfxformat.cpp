//-----------------------------------------------------
//!
//!	\file gfx\gfxformat.h
//! This contains the cross platform graphics formats
//! There will be lots more per platform but these will
//! be platform specific and as such you will have to 
//! use platform specific interfaces to create them
//!
//-----------------------------------------------------

#include "gfx/gfxformat.h"

namespace GFXFormat
{

const char* GetGFXFormatString( GFXFORMAT type )
{
	switch( type )
	{
	case GF_RGB8:		return "GF_RGB8";
	case GF_ARGB8:		return "GF_ARGB8";
	case GF_XRGB8:		return "GF_XRGB8";
	case GF_BGRA8:		return "GF_BGRA8";
	case GF_L8:			return "GF_L8";
	case GF_A8:			return "GF_A8";
	case GF_L8A8:		return "GF_L8A8";
	case GF_V8U8:		return "GF_V8U8";
								
	case GF_ABGR16F:	return "GF_ABGR16F";
	case GF_ABGR32F:	return "GF_ABGR32F";
	case GF_R16F:		return "GF_R16F";
	case GF_R32F:		return "GF_R32F";
								
	case GF_D24S8:		return "GF_D24S8";
								
	case GF_DXT1:		return "GF_DXT1";
	case GF_DXT3:		return "GF_DXT3";
	case GF_DXT5:		return "GF_DXT5";
								
	case GF_UNKNOWN:	return "GF_UNKNOWN";
	}

	ntAssert_p( 0, ("Unrecognised graphics format :%d", type ) );
	return "";
}

//! debug method to help estimate our VRAM usage
uint32_t GetBitsPerPixel( GFXFORMAT type )
{
	switch( type )
	{
	case GF_RGB8:		return 32;
	case GF_ARGB8:		return 32;
	case GF_XRGB8:		return 32;
	case GF_BGRA8:		return 32;
	case GF_L8:			return 8;
	case GF_A8:			return 8;
	case GF_L8A8:		return 16;
	case GF_V8U8:		return 16;
							
	case GF_ABGR16F:	return 64;
	case GF_ABGR32F:	return 128;
	case GF_R16F:		return 16;
	case GF_R32F:		return 32;
								
	case GF_D24S8:		return 32;
								
	case GF_DXT1:		return 4;
	case GF_DXT3:		return 8;
	case GF_DXT5:		return 8;
				
	case GF_UNKNOWN:	return 0;
	}

	ntAssert_p( 0, ("Unrecognised graphics format :%d", type ) );
	return 0;
}

//! debug method to help estimate our VRAM usage
uint32_t CalculateVRAMFootprint( GFXFORMAT	type,
								uint32_t	width,
								uint32_t	height,
								uint32_t	mips )
{
	// we always have at least 1 mip level
	mips = ntstd::Max( mips, (uint32_t)1 );

	uint32_t iResult = 0;
	uint32_t iBitsPerPixel = GetBitsPerPixel(type);

	for (uint32_t i = 0; i < mips; i++)
	{
		iResult += iBitsPerPixel * width * height;
		width = ntstd::Max( (width >> 1), 1u );
		height = ntstd::Max( (height >> 1), 1u );
	}

	return (iResult >> 3);
}

}
