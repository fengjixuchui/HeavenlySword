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

	void ResolveHDRtoLinear( RenderTarget::Ptr& pBackBuffer, const Texture::Ptr& pLens, const Texture::Ptr& pKeyLuminance );
	void ResolveHDRtoLinear( RenderTarget::Ptr& pBackBuffer, const Texture::Ptr& pLens, float fKeyLuminance );

private:
	DebugShader *	m_vertexShader;
	DebugShader	*	m_tonemapPS_normal;
	DebugShader	*	m_tonemapPS_cpuexp;
	VBHandle		m_hSimpleQuadData;
};

//--------------------------------------------------
//!
//!	HDRResolver_impl
//! construct
//!
//--------------------------------------------------
HDRResolver_impl::HDRResolver_impl( void )
{
	m_vertexShader = DebugShaderCache::Get().LoadShader( "hdrresolver_vp.sho" );
	m_tonemapPS_normal = DebugShaderCache::Get().LoadShader( "hdrresolver_normal_fp.sho" );
	m_tonemapPS_cpuexp = DebugShaderCache::Get().LoadShader( "hdrresolver_cpuexp_fp.sho" );

	// create screen quad data
	GcStreamField	simpleQuadDesc( FwHashedString( "IN.position" ), 0, Gc::kFloat, 2 ); 

	float const simpleQuad[] = 
	{
		-1.0f,  1.0f,
		 1.0f,  1.0f,
		 1.0f, -1.0f,
		-1.0f, -1.0f,
	};

	m_hSimpleQuadData = RendererPlatform::CreateVertexStream( 4, sizeof( float ) * 2, 1, &simpleQuadDesc );
	m_hSimpleQuadData->Write( simpleQuad );
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
	Renderer::Get().SetVertexShader( m_vertexShader );

	Renderer::Get().m_Platform.SetStream( m_hSimpleQuadData );

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

	m_tonemapPS_normal->SetPSConstantByName( "key_value_mapping",
		CVector( RenderingContext::Get()->m_exposureSettings.m_fKeyValueMapping, 0.0f, 0.0f, 0.0f ) );

	float const fWhiteRcpSq = 1.0f/(	RenderingContext::Get()->m_exposureSettings.m_fLuminanceBurnout*
										RenderingContext::Get()->m_exposureSettings.m_fLuminanceBurnout );

	m_tonemapPS_normal->SetPSConstantByName( "white_luminance_rcp_sq",
		CVector( fWhiteRcpSq, 0.0f, 0.0f, 0.0f ) );

	m_tonemapPS_normal->SetPSConstantByName( "image_transform",
		RenderingContext::Get()->m_postProcessingMatrix, 3 );

	m_tonemapPS_normal->SetPSConstantByName( "lens_effect_weight",
		CVector( CStaticLensConfig::GetEffectPower(), 0.0f, 0.0f, 0.0f ) );

	Renderer::Get().SetPixelShader( m_tonemapPS_normal );

	Renderer::Get().SetZBufferMode( GFX_ZMODE_DISABLED );

	GcKernel::Enable( Gc::kGammaCorrectedWrites );

	Renderer::Get().m_Platform.DrawPrimitives( Gc::kQuads, 0, 4 );
	Renderer::Get().m_Platform.ClearStreams();

	// cleanup
	//-------------------------------------------------------------
	Renderer::Get().SetTexture( 0, Texture::NONE );
	Renderer::Get().SetTexture( 1, Texture::NONE );
	Renderer::Get().SetTexture( 2, Texture::NONE );

	Renderer::Get().SetZBufferMode( GFX_ZMODE_NORMAL );
	GcKernel::Disable( Gc::kGammaCorrectedWrites );
}

void HDRResolver_impl::ResolveHDRtoLinear( RenderTarget::Ptr& pBackBuffer, const Texture::Ptr& pLens, float fKeyLuminance )
{
	Renderer::Get().SetVertexShader( m_vertexShader );

	Renderer::Get().m_Platform.SetStream( m_hSimpleQuadData );

	Renderer::Get().SetTexture( 0, pBackBuffer->GetTexture() );
	Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );
	
	if (CRendererSettings::GetAAMode() == AAM_NONE)
		Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_POINT );
	else
		Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_BILINEAR );

	Renderer::Get().SetTexture( 1, pLens );
	Renderer::Get().SetSamplerAddressMode( 1, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerFilterMode( 1, TEXTUREFILTER_BILINEAR );

	m_tonemapPS_cpuexp->SetPSConstantByName( "key_value_mapping",
		CVector( RenderingContext::Get()->m_exposureSettings.m_fKeyValueMapping, 0.0f, 0.0f, 0.0f ) );

	float const fWhiteRcpSq = 1.0f/(	RenderingContext::Get()->m_exposureSettings.m_fLuminanceBurnout*
										RenderingContext::Get()->m_exposureSettings.m_fLuminanceBurnout );
	float const fGammaRcp = 1.0f/( CRendererSettings::bEnableGammaCorrection ? 2.2f : 1.0f );

	m_tonemapPS_cpuexp->SetPSConstantByName( "white_luminance_rcp_sq",
		CVector( fWhiteRcpSq, 0.0f, 0.0f, 0.0f ) );

	m_tonemapPS_cpuexp->SetPSConstantByName( "gamma_rcp",
		CVector( fGammaRcp, 0.0f, 0.0f, 0.0f ) );

	m_tonemapPS_cpuexp->SetPSConstantByName( "image_transform",
		RenderingContext::Get()->m_postProcessingMatrix );

	m_tonemapPS_cpuexp->SetPSConstantByName( "lens_effect_weight",
		CVector( CStaticLensConfig::GetEffectPower(), 0.0f, 0.0f, 0.0f ) );

	Renderer::Get().SetPixelShader( m_tonemapPS_cpuexp );

	Renderer::Get().m_Platform.DrawPrimitives( Gc::kQuads, 0, 4 );
	Renderer::Get().m_Platform.ClearStreams();

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
	m_pImpl = NT_NEW_CHUNK(Mem::MC_GFX) HDRResolver_impl;
}
HDRResolver::~HDRResolver()
{
	NT_DELETE_CHUNK(Mem::MC_GFX, m_pImpl );
}
void HDRResolver::ResolveHDRtoLinear( RenderTarget::Ptr& pBackBuffer, const Texture::Ptr& pLens, const Texture::Ptr& pKeyLuminance )
{
	m_pImpl->ResolveHDRtoLinear( pBackBuffer, pLens, pKeyLuminance );
}
void HDRResolver::ResolveHDRtoLinear( RenderTarget::Ptr& pBackBuffer, const Texture::Ptr& pLens, float fKeyLuminance )
{
	m_pImpl->ResolveHDRtoLinear( pBackBuffer, pLens, fKeyLuminance );
}


