/***************************************************************************************************
*
*	DESCRIPTION
*
*	NOTES
*
***************************************************************************************************/

#include "gfx/lenseffects.h"
#include "gfx/texturemanager.h"
#include "gfx/renderer.h"
#include "gfx/hardwarecaps.h"
#include "gfx/levellighting.h"
#include "gfx/rendercontext.h"
#include "gfx/surfacemanager.h"
#include "gfx/graphicsdevice.h"

//--------------------------------------------------
//!
//!	LensArtifacts
//! find shaders etc
//!
//--------------------------------------------------
LensArtifacts::LensArtifacts( void )
{
	m_captureVS = DebugShaderCache::Get().LoadShader( "passthrough_pos_tex_vp.sho" );
	
	m_capturePS_normal = DebugShaderCache::Get().LoadShader( "lens_capture_fp.sho" );
	m_ghostPass1PS_normal = DebugShaderCache::Get().LoadShader( "lens_ghostpass1_fp.sho" );
	m_ghostPass2PS_normal = DebugShaderCache::Get().LoadShader( "lens_ghostpass2_fp.sho" );

	GcStreamField	simpleQuadDesc[] = 
	{
		GcStreamField( FwHashedString( "IN.position" ), 0, Gc::kFloat, 2 ),	// 2 * 4 bytes
		GcStreamField( FwHashedString( "IN.texcoord" ), 8, Gc::kFloat, 2 ),	// 2 * 4 bytes
	};

	float const simpleQuad[] = 
	{
		-1.0f,  1.0f,	0.0f,	0.0f,		// 0
		 1.0f,  1.0f,	1.0f,	0.0f,		// 1
		 1.0f, -1.0f,	1.0f,	1.0f,		// 2
		-1.0f, -1.0f,	0.0f,	1.0f,		// 3
	};

	m_hSimpleQuadData = RendererPlatform::CreateVertexStream( 4, sizeof( float ) * 4, 2, simpleQuadDesc );
	m_hSimpleQuadData->Write( simpleQuad );

	// load the ghost vignette texture
	m_pVignetteTexture = TextureManager::Get().LoadTexture_Neutral( "lensghost_mono.dds" ); 

	CStaticLensConfig::LevelReset();
}

//--------------------------------------------------
//!
//!	LensArtifacts::Generate
//! Generate the lens effect. Has two versions,
//! depending on the exposure algorithm used
//!
//--------------------------------------------------
RenderTarget::Ptr LensArtifacts::Generate( const Texture::Ptr& pKeyLuminance, const Texture::Ptr& pSource )
{
	ntAssert_p( LevelLighting::Exists(), ("WTF? Level lighting does not exist...") );
	ntAssert_p( LevelLighting::Get().GetGhostSetting() != 0, ("Level lighting does not have a ghost lens definition") );

	const CGhostLensEffectSettings* pDef = LevelLighting::Get().GetGhostSetting();

//	GFXFORMAT surfaceFormat = Renderer::Get().GetHDRFormat();
//	if ( HardwareCapabilities::Get().IsValidRenderTargetFormat(GF_ABGR16F) )
//		surfaceFormat = GF_ABGR16F;

	// currently we use this for fastest possible, but slightly banded lensing.
	// like you can tell...
	GFXFORMAT surfaceFormat = GF_ARGB8;

	// get the source dimensions
	// we now use a fixed size due to our multiple viewport rendering.
	int iGlowWidth = 320;
	int iGlowHeight = 180;

#ifdef _PROFILING
	if (CRendererSettings::bProfileLowFillrate)
	{
		iGlowWidth = (int)(_R(iGlowWidth)*0.01f);
		iGlowHeight = (int)(_R(iGlowHeight)*0.01f);
	}
	else if (CRendererSettings::bUseLowRezMode)
	{
		iGlowWidth = iGlowWidth>>1;
		iGlowHeight = iGlowHeight>>1;
	}
#endif

	// allocate rendertarget and setp
	RenderTarget::Ptr pobResult = SurfaceManager::Get().CreateRenderTarget( iGlowWidth, iGlowHeight, surfaceFormat );
	Renderer::Get().m_targetCache.SetColourTarget( pobResult );

	// early out if disabled 
	if( !CRendererSettings::bEnableLensEffects )
	{
		if (surfaceFormat == GF_ABGR16F)
			Renderer::Get().FastFloatClear( Gc::kColourBufferBit, 1.0f, 0 );
		else
			Renderer::Get().Clear( Gc::kColourBufferBit, 0, 1.0f, 0 );

		return pobResult;
	}

	// otherwise capture lores scaled buffer
	Renderer::Get().SetVertexShader( m_captureVS );

	Renderer::Get().m_Platform.SetStream( m_hSimpleQuadData );

	Renderer::Get().SetTexture( 0, pSource );
	Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_POINT );

	float fLinearScale = RenderingContext::Get()->m_exposureSettings.m_fKeyValueMapping;
	static CVector const obOne( 1.0f, 1.0f, 1.0f, 1.0f );

	CVector obOffset = ( 4.0f*GetMin()/fLinearScale )*obOne;
	CVector obScale = ( 0.25f*fLinearScale/( GetMax() - GetMin() ) )*obOne;
	
	Renderer::Get().SetTexture( 1, pKeyLuminance );
	Renderer::Get().SetSamplerAddressMode( 1, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerFilterMode( 1, TEXTUREFILTER_POINT );

	float fXOffset = 0.25f/_R( iGlowWidth );
	float fYOffset = 0.25f/_R( iGlowHeight );
	CVector obFilterOffsets( fXOffset, fYOffset, -fXOffset, -fYOffset );

	m_capturePS_normal->SetPSConstantByName( "offset", obOffset );
	m_capturePS_normal->SetPSConstantByName( "scale", obScale );
	m_capturePS_normal->SetPSConstantByName( "filter_offsets", obFilterOffsets );

	Renderer::Get().SetPixelShader( m_capturePS_normal );
	Renderer::Get().m_Platform.DrawPrimitives( Gc::kQuads, 0, 4 );
	Renderer::Get().m_Platform.ClearStreams();

	Renderer::Get().SetTexture( 0, Texture::NONE );
	Renderer::Get().SetTexture( 1, Texture::NONE );

	// ghosting does its own post process gaussian so reduce the normal gaussian to componsate
	static int iGhostGaussLevel = 4;

	int iGausLevel = GetGausianLevels();
	if( CRendererSettings::bEnableLensGhost )
	{
		iGausLevel = GetGausianLevels() - iGhostGaussLevel;
		iGausLevel = ntstd::Max( iGausLevel, 1 );
	}

	// gaussian blur
	m_blurHelper.RecursiveBlur( pobResult, GF_ARGB8, CVector( 1.0f, 1.0f, 1.0f, 1.0f ), 0, GetGausianLevels() );

	if( CRendererSettings::bEnableLensGhost )
	{
		// ghost screen flip & vignettes
		int iGhostWidth = iGlowWidth;
		int iGhostHeight = iGlowHeight;

		// pass 1 makes a vignetted and scaled combination of hi and lo res version of the screen
		RenderTarget::Ptr pobGhostTemp = SurfaceManager::Get().CreateRenderTarget( iGhostWidth, iGhostHeight, surfaceFormat );
		Renderer::Get().m_targetCache.SetColourTarget( pobGhostTemp );

		Renderer::Get().SetVertexShader( m_captureVS );

		Renderer::Get().m_Platform.SetStream( m_hSimpleQuadData );

		// texture 0 = hires screen (un-scaled range)
		Renderer::Get().SetTexture( 0, pSource );
		Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );
		Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_POINT );

		// texture 1 = filter screen (scaled range)
		// bilinear used as 16 bit integer filtering is supported
		Renderer::Get().SetTexture( 1, pobResult->GetTexture() );
		Renderer::Get().SetSamplerAddressMode( 1, TEXTUREADDRESS_CLAMPALL );
		Renderer::Get().SetSamplerFilterMode( 1, TEXTUREFILTER_BILINEAR );
		// texture 2 = vignette texture
		Renderer::Get().SetTexture( 2, m_pVignetteTexture );
		Renderer::Get().SetSamplerAddressMode( 2, TEXTUREADDRESS_CLAMPALL );
		Renderer::Get().SetSamplerFilterMode( 2, TEXTUREFILTER_BILINEAR );

		Renderer::Get().SetTexture( 3, pKeyLuminance );
		Renderer::Get().SetSamplerAddressMode( 3, TEXTUREADDRESS_CLAMPALL );
		Renderer::Get().SetSamplerFilterMode( 3, TEXTUREFILTER_POINT );

		// same as previous pass (to get correct scales)
		m_ghostPass1PS_normal->SetPSConstantByName( "offset", obOffset );
		m_ghostPass1PS_normal->SetPSConstantByName( "scale", obScale );
		m_ghostPass1PS_normal->SetPSConstantByName( "filter_offsets", obFilterOffsets );

		// ghost scales
		CVector obScalePass1( pDef->m_fGhostScaleSharp, pDef->m_fGhostScaleBlurred, 0, 0 );
		m_ghostPass1PS_normal->SetPSConstantByName( "ghost_scale", obScalePass1 );		

		Renderer::Get().SetPixelShader( m_ghostPass1PS_normal );

		Renderer::Get().m_Platform.DrawPrimitives( Gc::kQuads, 0, 4 );
		Renderer::Get().m_Platform.ClearStreams();

		Renderer::Get().SetTexture( 3, Texture::NONE );

		// pass 2 used pass 1 8 times modulating and scaling to produce a model of the internal reflection
		// then combines with the gaussian blurred buffer

		RenderTarget::Ptr pobCombineTemp = SurfaceManager::Get().CreateRenderTarget( iGhostWidth, iGhostHeight, surfaceFormat );

		
		// render into rendertarget
		Renderer::Get().m_targetCache.SetColourTarget( pobCombineTemp );
		Renderer::Get().SetVertexShader( m_captureVS );

		Renderer::Get().m_Platform.SetStream( m_hSimpleQuadData );

		// texture 0 = ghost buffer
		Renderer::Get().SetTexture( 0, pobGhostTemp->GetTexture() );
		Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );
		Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_BILINEAR );
		
		// texture 1 = gaussian buffer
		Renderer::Get().SetTexture( 1, pobResult->GetTexture() );
		Renderer::Get().SetSamplerAddressMode( 1, TEXTUREADDRESS_CLAMPALL );
		Renderer::Get().SetSamplerFilterMode( 1, TEXTUREFILTER_BILINEAR );
		
		// texture 2 = vignette texture
		Renderer::Get().SetTexture( 2, m_pVignetteTexture );
		Renderer::Get().SetSamplerAddressMode( 2, TEXTUREADDRESS_CLAMPALL );
		Renderer::Get().SetSamplerFilterMode( 2, TEXTUREFILTER_BILINEAR );

		// colour modulate of the final ghost before adding the gaussian in
		m_ghostPass2PS_normal->SetPSConstantByName( "accum_scale", pDef->m_obGhostFinalTint );

		CVector obGhostScale[2];
		obGhostScale[0] = CVector(	pDef->m_fGhostScale[0], pDef->m_fGhostScale[1], 
									pDef->m_fGhostScale[2], pDef->m_fGhostScale[3] );
		obGhostScale[1] = CVector(	pDef->m_fGhostScale[4], pDef->m_fGhostScale[5], 
									pDef->m_fGhostScale[6], pDef->m_fGhostScale[7] );
		// ghost scales and modulate(8 samples)
		m_ghostPass2PS_normal->SetPSConstantByName( "ghost_scale", obGhostScale, 2 );
		m_ghostPass2PS_normal->SetPSConstantByName( "ghost_modulate", pDef->m_obGhostModulate, 8 );

		Renderer::Get().SetPixelShader( m_ghostPass2PS_normal );
		Renderer::Get().m_Platform.DrawPrimitives( Gc::kQuads, 0, 4 );
		Renderer::Get().m_Platform.ClearStreams();

		// gaussian blur
		SurfaceManager::Get().ReleaseRenderTarget( pobGhostTemp );
		SurfaceManager::Get().ReleaseRenderTarget( pobResult );
		pobResult = pobCombineTemp;

		CVector	weight( 1.0f, 1.0f, 1.0f, 1.0f );
		m_blurHelper.RecursiveBlur( pobResult, surfaceFormat,
									weight, 0, iGhostGaussLevel );

		Renderer::Get().SetTexture( 0, Texture::NONE );
		Renderer::Get().SetTexture( 1, Texture::NONE );
		Renderer::Get().SetTexture( 2, Texture::NONE );
	}

	return pobResult;
}

RenderTarget::Ptr LensArtifacts::Generate( float, const Texture::Ptr& )
{
	ntError_p(0,("CPU Exposure is not supported on PS3"));
	
	RenderTarget::Ptr empty;
	return empty;
}
