/***************************************************************************************************
*
*	$Header:: /game/renderstates.h 13    7/07/03 11:05 Simonb                                      $
*
*	Holds coarsened render states for use with shaders, techniques and materials.
*
*	CHANGES
*
*	05/03/2003	Simon	Created
*
***************************************************************************************************/

#ifndef GFX_RENDERSTATES_H
#define GFX_RENDERSTATES_H

#ifdef PLATFORM_PC

//! Texture address mode.
enum TEXTUREADDRESS_TYPE
{
	TEXTUREADDRESS_UNKNOWN		= 0, 

	TEXTUREADDRESS_CLAMPALL		= (D3DTADDRESS_CLAMP << 16) | (D3DTADDRESS_CLAMP << 8) | D3DTADDRESS_CLAMP, 
	TEXTUREADDRESS_WRAPALL		= (D3DTADDRESS_WRAP << 16) | (D3DTADDRESS_WRAP << 8) | D3DTADDRESS_WRAP, 
	TEXTUREADDRESS_BORDERALL	= (D3DTADDRESS_BORDER << 16) | (D3DTADDRESS_BORDER << 8) | D3DTADDRESS_BORDER, 
};

//! Magnification, minification and mip filter and the texture sign mode.
enum TEXTUREFILTER_TYPE
{
	TEXTUREFILTER_UNKNOWN		= 0, 

	TEXTUREFILTER_POINT			= (D3DTEXF_POINT << 16) | (D3DTEXF_POINT << 8) | D3DTEXF_POINT, 
	TEXTUREFILTER_BILINEAR		= (D3DTEXF_LINEAR << 16) | (D3DTEXF_LINEAR << 8) | D3DTEXF_POINT, 
	TEXTUREFILTER_TRILINEAR		= (D3DTEXF_LINEAR << 16) | (D3DTEXF_LINEAR << 8) | D3DTEXF_LINEAR, 

	TEXTUREFILTER_sRGB_CONVERT	= ( 1 << 24 ), 
};

#elif PLATFORM_PS3

//! Texture address mode.
enum TEXTUREADDRESS_TYPE
{
	TEXTUREADDRESS_UNKNOWN		= 0, 

	TEXTUREADDRESS_CLAMPALL,
	TEXTUREADDRESS_WRAPALL,
	TEXTUREADDRESS_BORDERALL,

	TEXTUREADDRESS_WCC,	// wrap clamp clamp
};

//! Magnification, minification and mip filter and the texture sign mode.
enum TEXTUREFILTER_TYPE
{
	TEXTUREFILTER_UNKNOWN		= 0, 

	TEXTUREFILTER_POINT,
	TEXTUREFILTER_BILINEAR,
	TEXTUREFILTER_TRILINEAR,
};

#endif

//! Frame buffer blend mode type. The default is GFX_BLENDMODE_OVERWRITE.
enum GFX_BLENDMODE_TYPE
{
	GFX_BLENDMODE_OVERWRITE	= 0,		//!< The default state for materials; writes straight through to the frame buffer.
		
	GFX_BLENDMODE_DISABLED,				//!< No colour buffer writes are performed.
	GFX_BLENDMODE_LERP,					//!< Lerp with the destination value based on the source alpha channel.
	GFX_BLENDMODE_ADD,					//!< Adds all channels.
	GFX_BLENDMODE_ADD_SRCALPHA,			//!< Adds all channels, modulated by src alpha.
	GFX_BLENDMODE_SUB,					//!< Subs all channels.
	GFX_BLENDMODE_SUB_SRCALPHA,			//!< Subs all channels, modulated by src alpha.

	GFX_BLENDMODE_NORMAL = GFX_BLENDMODE_OVERWRITE,
};

//! Z-buffer mode type. The default is GFX_ZMODE_LESSEQUAL.
enum GFX_ZMODE_TYPE
{
	GFX_ZMODE_LESSEQUAL = 0,		//!< Compare function is <=, the results are written through.

	GFX_ZMODE_DISABLED, 			//!< All z-buffer reads and writes are disabled.
	GFX_ZMODE_LESSEQUAL_READONLY,	//!< Compare function is <=, the results are NOT written through.

	GFX_ZMODE_NORMAL = GFX_ZMODE_LESSEQUAL,
};

//! Cull mode type. The default is GFX_CULLMODE_NORMAL.
enum GFX_CULLMODE_TYPE
{
	GFX_CULLMODE_EXPLICIT_CCW = 0,	//!< Explicit counter-clockwise culling.
	GFX_CULLMODE_EXPLICIT_CW,		//!< Explicit clockwise culling.
	GFX_CULLMODE_NONE,				//!< No culling.

	GFX_CULLMODE_NORMAL,			//!< Normal winding order (this is context-specific).
	GFX_CULLMODE_REVERSED,			//!< Reverse winding order (this is context-specific).
};

//! Alphatest mode type. The default is GFX_CULLMODE_NORMAL.
enum GFX_ALPHA_TEST_MODE
{
	GFX_ALPHATEST_NONE = 0,			//!< Alpha testing disabled

	GFX_ALPHATEST_EQUAL,			//!< if (a == ref) then kill pixel
	GFX_ALPHATEST_NOTEQUAL,			//!< if (a != ref) then kill pixel

	GFX_ALPHATEST_LESS,				//!< if (a < ref) then kill pixel
	GFX_ALPHATEST_LESSEQUAL,		//!< if (a <= ref) then kill pixel

	GFX_ALPHATEST_GREATER,			//!< if (a > ref) then kill pixel
	GFX_ALPHATEST_GREATEREQUAL,		//!< if (a >= ref) then kill pixel

	GFX_ALPHATEST_ALWAYS,			//!< Always kill pixel
};

//! Rendering fill mode type
enum GFX_FILL_MODE
{
	GFX_FILL_SOLID,					//!< Solid fill
	GFX_FILL_POINT,					//!< Points at each vert
	GFX_FILL_WIREFRAME,				//!< wireframe for prim edges
};


#endif // ndef GFX_RENDERSTATES_H
