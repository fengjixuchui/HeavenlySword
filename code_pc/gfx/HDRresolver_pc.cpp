//--------------------------------------------------
//!
//!	\file HDRresolver.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "gfx/HDRresolver.h"
#include "gfx/shader.h"
#include "gfx/renderer.h"
#include "gfx/lenseffects.h"
#include "gfx/rendercontext.h"
#include "gfx/graphicsdevice.h"

//--------------------------------------------------
//!
//!	HDRResolver_impl
//! Shifts scene from a HDR back buffer to a tone mapped X8R8G8B8 one.
//!
//--------------------------------------------------
class HDRResolver_impl
{
public:
	HDRResolver_impl( void );
	~HDRResolver_impl( void );

	void ResolveHDRtoLinear( RenderTarget::Ptr& pBackBuffer, const Texture::Ptr& pLens, const Texture::Ptr& pKeyLuminance );
	void ResolveHDRtoLinear( RenderTarget::Ptr& pBackBuffer, const Texture::Ptr& pLens, float fKeyLuminance );

private:
	Shader* m_pVertexShader;
	Shader* m_pTonemapPS_normal;
	Shader* m_pTonemapPS_cpuexp;
	CVertexDeclaration m_pDecl;
};

//--------------------------------------------------
//!
//!	HDRResolver_impl
//! construct
//!
//--------------------------------------------------
HDRResolver_impl::HDRResolver_impl( void )
{
	m_pVertexShader		= ShaderManager::Get().FindShader( "lighting_combiner_vs" );
	m_pTonemapPS_normal	= ShaderManager::Get().FindShader( "final_tonemapping_ps" );

	// create a pixel shader from an alternative file
	DebugShader* pNewPS = NT_NEW_CHUNK( Mem::MC_GFX ) DebugShader;
	pNewPS->SetFILEFunction( SHADERTYPE_PIXEL, "shaders\\exposure_overide.hlsl", "final_tonemapping_ps", "ps_2_0" );	
	m_pTonemapPS_cpuexp = pNewPS;

	ntAssert( m_pVertexShader && m_pTonemapPS_normal && m_pTonemapPS_cpuexp );

	D3DVERTEXELEMENT9 stDecl[] = 
	{
		{ 0, 0,					D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0 }, 
		D3DDECL_END()
	};
	m_pDecl = CVertexDeclarationManager::Get().GetDeclaration( stDecl );
}

//--------------------------------------------------
//!
//!	HDRResolver_impl
//! cleanup
//!
//--------------------------------------------------
HDRResolver_impl::~HDRResolver_impl( void )
{
	NT_DELETE_CHUNK( Mem::MC_GFX, m_pTonemapPS_cpuexp );
}

//--------------------------------------------------
//!
//!	HDRResolver_impl::ResolveHDRtoLinear
//! moves back buffer from HDR to linear space
//! two methods provided due to our exposure problems
//!
//--------------------------------------------------
void HDRResolver_impl::ResolveHDRtoLinear( RenderTarget::Ptr& pBackBuffer, const Texture::Ptr& pLens, const Texture::Ptr& pKeyLuminance )
{
	Renderer::Get().m_Platform.SetVertexDeclaration( m_pDecl );
	Renderer::Get().SetVertexShader( m_pVertexShader );
	Renderer::Get().SetPixelShader( m_pTonemapPS_normal );

	static float const afScreenTri[] = 
	{
		-1.0f,	 1.0f,
		 3.0f,	 1.0f, 
		-1.0f,	-3.0f
	};

	Renderer::Get().SetTexture( 0, pBackBuffer->GetTexture() );
	Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );
	
	if (CRendererSettings::GetAAMode() == AAM_NONE)
		Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_POINT );
	else
		Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_BILINEAR );

	Renderer::Get().SetTexture( 1, pLens );
	Renderer::Get().SetSamplerAddressMode( 1, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerFilterMode( 1, TEXTUREFILTER_BILINEAR );

	Renderer::Get().SetTexture( 2, pKeyLuminance );
	Renderer::Get().SetSamplerAddressMode( 2, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerFilterMode( 2, TEXTUREFILTER_POINT );

	Renderer::Get().SetPixelShaderConstant( 0, CVector( RenderingContext::Get()->m_exposureSettings.m_fKeyValueMapping, 0.0f, 0.0f, 0.0f ) );

	float const fWhiteRcpSq = 1.0f/(	RenderingContext::Get()->m_exposureSettings.m_fLuminanceBurnout*
										RenderingContext::Get()->m_exposureSettings.m_fLuminanceBurnout );
	float const fGammaRcp = 1.0f/( CRendererSettings::bEnableGammaCorrection ? 2.2f : 1.0f );

	Renderer::Get().SetPixelShaderConstant( 1, CVector( fWhiteRcpSq, 0.0f, 0.0f, 0.0f ) );
	Renderer::Get().SetPixelShaderConstant( 2, CVector( fGammaRcp, 0.0f, 0.0f, 0.0f ) );
	Renderer::Get().SetPixelShaderConstant( 3, RenderingContext::Get()->m_postProcessingMatrix );
	Renderer::Get().SetPixelShaderConstant( 7, CVector( CStaticLensConfig::GetEffectPower(), 0.0f, 0.0f, 0.0f ) );

	GetD3DDevice()->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 1, afScreenTri, 2*sizeof( float ) );

	// cleanup
	//-------------------------------------------------------------
	Renderer::Get().SetTexture( 0, Texture::NONE );
	Renderer::Get().SetTexture( 1, Texture::NONE );
	Renderer::Get().SetTexture( 2, Texture::NONE );
}

void HDRResolver_impl::ResolveHDRtoLinear( RenderTarget::Ptr& pBackBuffer, const Texture::Ptr& pLens, float fKeyLuminance )
{
	Renderer::Get().m_Platform.SetVertexDeclaration( m_pDecl );
	Renderer::Get().SetVertexShader( m_pVertexShader );
	Renderer::Get().SetPixelShader( m_pTonemapPS_cpuexp );

	static float const afScreenTri[] = 
	{
		-1.0f,	 1.0f,
		 3.0f,	 1.0f, 
		-1.0f,	-3.0f
	};

	Renderer::Get().SetTexture( 0, pBackBuffer->GetTexture() );
	Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );
	
	if (CRendererSettings::GetAAMode() == AAM_NONE)
		Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_POINT );
	else
		Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_BILINEAR );

	Renderer::Get().SetTexture( 1, pLens );
	Renderer::Get().SetSamplerAddressMode( 1, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerFilterMode( 1, TEXTUREFILTER_BILINEAR );

	Renderer::Get().SetPixelShaderConstant( 0, CVector( RenderingContext::Get()->m_exposureSettings.m_fKeyValueMapping / fKeyLuminance, 0.0f, 0.0f, 0.0f ) );

	float const fWhiteRcpSq = 1.0f/(	RenderingContext::Get()->m_exposureSettings.m_fLuminanceBurnout*
										RenderingContext::Get()->m_exposureSettings.m_fLuminanceBurnout );
	float const fGammaRcp = 1.0f/( CRendererSettings::bEnableGammaCorrection ? 2.2f : 1.0f );

	Renderer::Get().SetPixelShaderConstant( 1, CVector( fWhiteRcpSq, 0.0f, 0.0f, 0.0f ) );
	Renderer::Get().SetPixelShaderConstant( 3, RenderingContext::Get()->m_postProcessingMatrix );
	Renderer::Get().SetPixelShaderConstant( 2, CVector( fGammaRcp, 0.0f, 0.0f, 0.0f ) );
	Renderer::Get().SetPixelShaderConstant( 7, CVector( CStaticLensConfig::GetEffectPower(), 0.0f, 0.0f, 0.0f ) );

	GetD3DDevice()->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 1, afScreenTri, 2*sizeof( float ) );

	// cleanup
	//-------------------------------------------------------------
	Renderer::Get().SetTexture( 0, Texture::NONE );
	Renderer::Get().SetTexture( 1, Texture::NONE );
	Renderer::Get().SetTexture( 2, Texture::NONE );
}

//--------------------------------------------------
//!
//!	HDRResolver
//! Public interface to our GPU based exposure
//!
//--------------------------------------------------
HDRResolver::HDRResolver()
{
	m_pImpl = NT_NEW_CHUNK( Mem::MC_GFX ) HDRResolver_impl;
}
HDRResolver::~HDRResolver()
{
	NT_DELETE_CHUNK( Mem::MC_GFX, m_pImpl );
}
void HDRResolver::ResolveHDRtoLinear( RenderTarget::Ptr& pBackBuffer, const Texture::Ptr& pLens, const Texture::Ptr& pKeyLuminance )
{
	m_pImpl->ResolveHDRtoLinear( pBackBuffer, pLens, pKeyLuminance );
}
void HDRResolver::ResolveHDRtoLinear( RenderTarget::Ptr& pBackBuffer, const Texture::Ptr& pLens, float fKeyLuminance )
{
	m_pImpl->ResolveHDRtoLinear( pBackBuffer, pLens, fKeyLuminance );
}


