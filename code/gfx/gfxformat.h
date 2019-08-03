#if !defined(GFX_GFXFORMAT_H)
#define GFX_GFXFORMAT_H
//-----------------------------------------------------
//!
//!	\file gfx\gfxformat.h
//! This contains the cross platform graphics formats
//! There will be lots more per platform but these will
//! be platform specific and as such you will have to 
//! use platform specific interfaces to create them
//!
//-----------------------------------------------------

//! formats supports as both render target and textures.
//! Platforms are allowed to 'fake' these but must be supported or the format will be removed from cross-platform section
enum GFXFORMAT
{
						//!<	size	chan	chan fmt	R	G	B	A	C struct				mem (little endian)
						//!<------------------------------------------------------------------------------------------
	GF_RGB8 = 0,		//!<	24bit	3		8bit uint	R	G	B	1	{ u_char b,g,r; }		(r<<16) | (g<<8) | b
	GF_ARGB8,			//!<	32bit	4		8bit uint	R	G	B	A	{ u_char b,g,r,a; }		(a<<24) | (r<<16) | (g<<8) | b
	GF_XRGB8,			//!<	32bit	3		8bit uint 	R	G	B	1	{ u_char b,g,r,x; }		(0xff<<24) | (r<<16) | (g<<8) | b
	GF_BGRA8,			//!<	32bit	3		8bit uint 	R	G	B	1	{ u_char a,r,g,b; }		(b<<24) | (g<<16) | (r<<8) | a
	GF_L8,				//!<	8bit	1		8bit uint	L	L	L	1	{ u_char l; }
	GF_A8,				//!<	8bit	1		8bit uint	0	0	0	A	{ u_char a; }
	GF_L8A8,			//!<	16bit	2		8bit uint	L	L	L	A	{ u_char l, a; }		(a<<8) | l

	GF_ABGR16F,			//!<	64bit	4		16bit s10e5	R	G	B	A	{ half r,g,b,a; }		(g<<16) | r, (a<<16) | b
	GF_ABGR32F,			//!<	128bit	4		32bit s23e8	R	G	B	A	{ float r,g,b,a; }		r, g, b, a
	GF_R16F,			//!<	16bit	1		16bit s10e5	R				{ half r; }
	GF_R32F,			//!<	32bit	1		32bit s23e8	R				{ float r; }

	GF_D24S8,			//!< 32bit, 24 bit integer stencil, 8 bit stencil render target only

	GF_DXT1,			//!< DXT1 texture format only 4 bits per texel
	GF_DXT3,			//!< DXT3 texture format only 8 bits per texel explicit alpha
	GF_DXT5,			//!< DXT5 texture format only 8 bits per texel interpolated alpha

	GF_V8U8,			//!< 16 bit per pixel format, to be used with high quality normal maps

	GF_UNKNOWN,
};

//! AA modes 
enum GFXAAMODE
{
	GAA_MULTISAMPLE_NONE = 0,
	GAA_MULTISAMPLE_4X,
	GAA_MULTISAMPLE_ORDERED_GRID_4X,

	GAA_UNKNOWN,
};


//! AA resolvers
enum GFXAARESOLVER
{
	GAARES_NONE = 0,
	GAARES_BILINEAR,
	GAARES_GAUSSIAN,

	GAARES_UNKNOWN,
};


enum PRIMTYPE
{
	PT_POINTLIST = 0,
	
	PT_LINELIST,
	PT_LINELOOP,
	PT_LINESTRIP,

	PT_TRIANGLELIST,
	PT_TRIANGLESTRIP,
	PT_TRIANGLEFAN,

	PT_QUADLIST,
	PT_QUADSTRIP,
	PT_POLYGON,

	PT_UNKNOWN,
};

#if defined(PLATFORM_PC)
#include "gfx/gfxformat_pc.h"
#elif defined( PLATFORM_PS3)
#include "gfx/gfxformat_ps3.h"
#endif

namespace GFXFormat
{
	// so we can validate filter modes
	inline bool Is32F( GFXFORMAT fmt )
	{
		if ((fmt == GF_R32F) || (fmt == GF_ABGR32F))
			return true;
		return false;
	}

	// for debug code
	const char* GetGFXFormatString( GFXFORMAT type );

	//! debug method to help estimate our VRAM usage
	uint32_t GetBitsPerPixel( GFXFORMAT type );

	//! debug method to help estimate our VRAM usage
	uint32_t CalculateVRAMFootprint( GFXFORMAT	type,
									uint32_t	width,
									uint32_t	height,
									uint32_t	mips = 1 );
};

#endif // GFX_GFXFORMAT_H
