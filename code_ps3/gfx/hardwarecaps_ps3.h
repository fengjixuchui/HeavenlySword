#ifndef GFX_HARDWARECAPS_PS3_H_
#define GFX_HARDWARECAPS_PS3_H_

#include "gfx/renderersettings.h"
#include "gfx/gfxformat.h"

#if !defined( _RELEASE ) || defined(_PROFILING)
#define CAN_BE_OVERIDDEN
#endif 

//--------------------------------------------------
//!
//!	HardwareCapabilities
//!
//--------------------------------------------------
class HardwareCapabilities : public Singleton<HardwareCapabilities>
{
public:
	// SupportsVertexShader3
	bool SupportsVertexShader3() const
	{
		return true;
	}

	// SupportsPixelShader2
	bool SupportsPixelShader2() const
	{
		return true;
	}

	// SupportsPixelShader3
	bool SupportsPixelShader3() const
	{
		return true;
	}

	//! helper to tell if we have both version 3 across the board (there is a least one hardware part that doesn't...)
	bool SupportsShaderModel3() const
	{
		return true;
	}

	bool SupportsHardwareShadowMaps() const;

	//--------------------------------------------------
	//!
	//! Can we render to this format?
	//!
	//--------------------------------------------------
	bool IsValidRenderTargetFormat( GFXFORMAT eType )
	{
		switch( eType )
		{
		case GF_ARGB8:
		case GF_ABGR16F:
		case GF_ABGR32F:
		case GF_R32F:
		case GF_D24S8:
			return true;
		default:
			return false;
		};
	}

	//--------------------------------------------------
	//!
	//! Can we texture from this format?
	//!
	//--------------------------------------------------
	bool IsValidTextureFormat( GFXFORMAT eType )
	{
		switch( eType )
		{
		case GF_ARGB8:
		case GF_BGRA8:
		case GF_L8:
		case GF_A8:
		case GF_L8A8:
		case GF_ABGR16F:
		case GF_ABGR32F:
		case GF_R32F:
		case GF_D24S8:
		case GF_DXT1:
		case GF_DXT3:
		case GF_DXT5:
			return true;
		default:
			return false;
		};
	}
	//--------------------------------------------------
	//!
	//! Can we use this a depth buffer?
	//!
	//--------------------------------------------------
	bool IsValidDepthFormat( GFXFORMAT eType )
	{
		switch( eType )
		{
		case GF_D24S8:
			return true;
		default:
			return false;
		};
	}
	//--------------------------------------------------
	//!
	//! Can we alphablend while this is a render target?
	//!
	//--------------------------------------------------
	bool IsValidAlphaBlendRTFormat( GFXFORMAT eType )
	{
		switch( eType )
		{
		case GF_ARGB8:
		case GF_ABGR16F:
			return true;
		default:
			return false;
		};
	}

private:
};

#endif //GFX_HARDWARECAPS_PS3_H_
