#ifndef GFX_HARDWARECAPS_PC_H_
#define GFX_HARDWARECAPS_PC_H_

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
	HardwareCapabilities();

	const D3DCAPS9&	GetCAPS() { return m_CAPS; }

	// SupportsVertexShader3
	bool SupportsVertexShader3() const
	{
#ifdef CAN_BE_OVERIDDEN
		if( CRendererSettings::bForceShaderModel2 )
			return false;
#endif 
		return (m_CAPS.VertexShaderVersion & 0x0000ffff) >= 0x00000300;
	}

	// SupportsPixelShader2
	bool SupportsPixelShader2() const
	{
		return (m_CAPS.PixelShaderVersion & 0x0000ffff) >= 0x00000200;
	}

	// SupportsPixelShader3
	bool SupportsPixelShader3() const
	{
#ifdef CAN_BE_OVERIDDEN
		if( CRendererSettings::bForceShaderModel2 )
			return false;
#endif 
		return (m_CAPS.PixelShaderVersion & 0x0000ffff) >= 0x00000300;
	}

	//! helper to tell if we have both version 3 across the board (there is a least one hardware part that doesn't...)
	bool SupportsShaderModel3() const
	{
		return SupportsVertexShader3() && SupportsPixelShader3();
	}

	bool SupportsHardwareShadowMaps() const;

	bool IsValidRenderTargetFormat( D3DFORMAT type );
	bool IsValidTextureFormat( D3DFORMAT eType );
	bool IsValidDepthFormat( D3DFORMAT eType );
	bool IsValidAlphaBlendRTFormat( D3DFORMAT type );

	// compatablility inlines
	inline bool IsValidRenderTargetFormat( GFXFORMAT type )
	{
		return IsValidRenderTargetFormat( ConvertGFXFORMATToD3DFORMAT(type) );
	}

	inline bool IsValidTextureFormat( GFXFORMAT type )
	{
		return IsValidTextureFormat( ConvertGFXFORMATToD3DFORMAT(type) );
	}

	inline bool IsValidDepthFormat( GFXFORMAT type )
	{
		return IsValidDepthFormat( ConvertGFXFORMATToD3DFORMAT(type) );
	}

	inline bool IsValidAlphaBlendRTFormat( GFXFORMAT type )
	{
		return IsValidAlphaBlendRTFormat( ConvertGFXFORMATToD3DFORMAT(type) );
	}

private:

	// see DX9 help->DirectX Graphics->Reference->Structures->D3DCAPS9
	D3DCAPS9	m_CAPS;

	ntstd::Set<D3DFORMAT>	m_validRT;
	ntstd::Set<D3DFORMAT>	m_invalidRT;
	
	ntstd::Set<D3DFORMAT>	m_validTex;
	ntstd::Set<D3DFORMAT>	m_invalidTex;
	
	ntstd::Set<D3DFORMAT>	m_validDF;
	ntstd::Set<D3DFORMAT>	m_invalidDF;

	ntstd::Set<D3DFORMAT>	m_validART;
	ntstd::Set<D3DFORMAT>	m_invalidART;

	mutable enum TRI_STATE
	{
		TS_UNKNOWN = -1,
		TS_FALSE = 0,
		TS_TRUE = 1,
	} m_supportsShadowMapHardware;
};


#endif //GFX_HARDWARECAPS_PC_H_
