/***************************************************************************************************
*
*	DESCRIPTION
*
*	NOTES
*
***************************************************************************************************/

#include "gfx/lenseffects.h"
#include "gfx/shader.h"
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
	m_pCaptureVS = ShaderManager::Get().FindShader( "effects_resample_vs" );	
	m_pCapturePS_normal = ShaderManager::Get().FindShader( "effects_resample_lens_ps" );
	m_pGhostPass1PS_normal = ShaderManager::Get().FindShader( "effects_ghost_pass1_ps" );
	m_pGhostPass2PS_normal = ShaderManager::Get().FindShader( "effects_ghost_pass2_ps" );

	// debug create NT_NEW_CHUNK(Mem::MC_GFX) pixel shaders
	//-------------------------------------
	DebugShader* pNewPS = NT_NEW_CHUNK(Mem::MC_GFX) DebugShader;
	pNewPS->SetHLSLFunction(
		SHADERTYPE_PIXEL,

		// used by several shaders now so seperated into its own funciton
		"float4 effects_scale_and_filter_hdr( float2 texcoord, "
		"								uniform float4 offset, "
		"								uniform float4 scale, "
		"								uniform float4 filter_offsets, "
		"								uniform sampler2D source)"
		"{"
		"	float4 colour = tex2D( source, texcoord + filter_offsets.xy )"
		"		+ tex2D( source, texcoord + filter_offsets.xw )"
		"		+ tex2D( source, texcoord + filter_offsets.zy )"
		"		+ tex2D( source, texcoord + filter_offsets.zw );"
		""
		"	return ( colour - offset )*scale;"
		"}"
		" "
		// actual lens thingumy
		"float4 effects_resample_lens_ps( float2 texcoord : TEXCOORD0, "
		"								 uniform float4 offset : register(c0), "
		"								 uniform float4 scale : register(c1), "
		"								 uniform float4 filter_offsets : register(c2), "
		"								 uniform sampler2D source : register(s0) ) : COLOR0"
		"{	"
		"	return effects_scale_and_filter_hdr( texcoord, offset, scale, filter_offsets, source );"
		"}",

		"effects_resample_lens_ps",
		"ps.2.0"
	);

	m_pCapturePS_cpuexp = pNewPS;

	pNewPS = NT_NEW_CHUNK(Mem::MC_GFX) DebugShader;
	pNewPS->SetHLSLFunction(
		SHADERTYPE_PIXEL,

		// used by several shaders now so seperated into its own funciton
		"float4 effects_scale_and_filter_hdr( float2 texcoord, "
		"								uniform float4 offset, "
		"								uniform float4 scale, "
		"								uniform float4 filter_offsets, "
		"								uniform sampler2D source)"
		"{"
		"	float4 colour = tex2D( source, texcoord + filter_offsets.xy )"
		"		+ tex2D( source, texcoord + filter_offsets.xw )"
		"		+ tex2D( source, texcoord + filter_offsets.zy )"
		"		+ tex2D( source, texcoord + filter_offsets.zw );"
		""
		"	return ( colour - offset )*scale;"
		"}"
		" "
		"float4 effects_ghost_pass1_ps( float2 texcoord : TEXCOORD0, "
		"								uniform float4 offset : register(c0), "
		"								uniform float4 scale : register(c1), "
		"								uniform float4 filter_offsets : register(c2), "
		"								uniform float4 ghost_scale : register(c3),"
		"								uniform sampler2D hi_source : register(s0),"
		"								uniform sampler2D lo_source : register(s1),"
		"								uniform sampler2D vignette : register(s2) ) : COLOR0"
		"{"
		"	float2 hi_texcoord = (texcoord - 0.5) * ghost_scale.xx + 0.5;"
		"	float2 lo_texcoord = (texcoord - 0.5) * ghost_scale.yy + 0.5;	"
		" "
			// assuming the vignette texture blends to zero at the edges no need for border texture addressing
		"	float4 hi_sample = effects_scale_and_filter_hdr( hi_texcoord, offset, scale, filter_offsets, hi_source );"
		"	float4 lo_sample = tex2D( lo_source, lo_texcoord );"
		"	float4 hi_vig_sample = tex2D( vignette, hi_texcoord );"
		"	float4 lo_vig_sample = tex2D( vignette, lo_texcoord );"
		"	"
		"	return clamp( (hi_sample * hi_vig_sample) + (lo_sample * lo_vig_sample), 0.0f, 1.0f );"
		"	"
		"}",

		"effects_ghost_pass1_ps",
		"ps.2.0"
	);

	m_pGhostPass1PS_cpuexp = pNewPS;

	pNewPS = NT_NEW_CHUNK(Mem::MC_GFX) DebugShader;
	pNewPS->SetHLSLFunction(
		SHADERTYPE_PIXEL,

		"float4 effects_ghost_pass2_ps( float2 texcoord : TEXCOORD0, "
		"								uniform float4 accum_scale : register(c0),"
		"								uniform float4 ghost_scale[2] : register(c1), "
		"								uniform float4 ghost_modulate[8] : register(c10), "
		"								uniform sampler2D ghost_texture : register(s0),"
		"								uniform sampler2D gaussian_texture : register(s1),"
		"								uniform sampler2D vignette_texture : register(s2)"
		"								) : COLOR0"
		"{"
			// sample the ghost buffer 8 times at various scales
			// modulate and accumulate the ghost image
		"	float4 accum = 0;"
		"	for( int i=0;i < 2;i++)"
		"	{"
		"		float2 scaled_coord[4];"
		"		scaled_coord[0] = (texcoord - 0.5) * ghost_scale[i].xx + 0.5;"
		"		scaled_coord[1] = (texcoord - 0.5) * ghost_scale[i].yy + 0.5;"
		"		scaled_coord[2] = (texcoord - 0.5) * ghost_scale[i].zz + 0.5;"
		"		scaled_coord[3] = (texcoord - 0.5) * ghost_scale[i].ww + 0.5;"
		"		"
		"		accum += tex2D( ghost_texture, scaled_coord[0] ) * tex2D( vignette_texture, scaled_coord[0] ) * ghost_modulate[ (i*2)+0 ];"
		"		accum += tex2D( ghost_texture, scaled_coord[1] ) * tex2D( vignette_texture, scaled_coord[1] ) * ghost_modulate[ (i*2)+1 ];"
		"		accum += tex2D( ghost_texture, scaled_coord[2] ) * tex2D( vignette_texture, scaled_coord[2] ) * ghost_modulate[ (i*2)+2 ];"
		"		accum += tex2D( ghost_texture, scaled_coord[3] ) * tex2D( vignette_texture, scaled_coord[3] ) * ghost_modulate[ (i*2)+3 ];"
		"	}"
		"	"
			// scale and add to the gaussian
		"	float4 gaussian_sample = tex2D( gaussian_texture, texcoord );"
		"	accum = accum * accum_scale + gaussian_sample;"
		" "
		"	return clamp( accum, 0.0f, 1.0f );"
		"}",

		"effects_ghost_pass2_ps",
		"ps.2.0"
	);

	m_pGhostPass2PS_cpuexp = pNewPS;

	D3DVERTEXELEMENT9 stDecl[] = 
	{
		{ 0, 0,					D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0 }, 
		{ 0, 2*sizeof(float),	D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	0 }, 
		D3DDECL_END()
	};
	m_pCaptureDecl = CVertexDeclarationManager::Get().GetDeclaration( &stDecl[0] );

	// load the ghost vignette texture
	m_pVignetteTexture = TextureManager::Get().LoadTexture_Neutral( "lensghost_mono.tga" ); 

	CStaticLensConfig::LevelReset();
}

//--------------------------------------------------
//!
//!	LensArtifacts
//! cleanup
//!
//--------------------------------------------------
LensArtifacts::~LensArtifacts( void )
{
	NT_DELETE_CHUNK(Mem::MC_GFX,  m_pCapturePS_cpuexp );
	NT_DELETE_CHUNK(Mem::MC_GFX,  m_pGhostPass1PS_cpuexp );
	NT_DELETE_CHUNK(Mem::MC_GFX,  m_pGhostPass2PS_cpuexp );
}

//--------------------------------------------------
//!
//!	LensArtifacts::Generate
//! Generate the lens effect. Has two versions,
//! depending on the exposure algorithm used
//!
//--------------------------------------------------
RenderTarget::Ptr LensArtifacts::Generate( const Texture::Ptr& pKeyLuminance, const RenderTarget::Ptr& pSource )
{
	ntAssert_p( LevelLighting::Exists(), ("WTF? Level lighting does not exist...") );
	ntAssert_p( LevelLighting::Get().GetGhostSetting() != 0, ("Level lighting does not have a ghost lens definition") );

	const CGhostLensEffectSettings* pDef = LevelLighting::Get().GetGhostSetting();

	// get the source dimensions
	D3DFORMAT surfaceFormat = ConvertGFXFORMATToD3DFORMAT(Renderer::Get().GetHDRFormat());
	if ( HardwareCapabilities::Get().IsValidRenderTargetFormat(D3DFMT_A16B16G16R16) )
		surfaceFormat = D3DFMT_A16B16G16R16;

	int iGlowWidth = (int)(_R(pSource->GetWidth()) * 0.25f);
	int iGlowHeight = (int)(_R(pSource->GetHeight()) * 0.25f);
	bool bCacheable = pSource->m_Platform.IsCacheable();

	// allocate rendertarget and set
	RenderTarget::CreationStruct artifactParams( iGlowWidth, iGlowHeight, surfaceFormat, false );
	artifactParams.bCacheable = bCacheable;

	RenderTarget::Ptr pobResult = SurfaceManager::Get().CreateRenderTarget( artifactParams );
	Renderer::Get().m_targetCache.SetColourTarget( pobResult );

	// early out if disabled
	if( !CRendererSettings::bEnableLensEffects )
	{
		Renderer::Get().Clear( D3DCLEAR_TARGET, 0, 1.0f, 0 );
		return pobResult;
	}

	// otherwise capture lores scaled buffer
	Renderer::Get().SetVertexShader( m_pCaptureVS );
	Renderer::Get().m_Platform.SetVertexDeclaration( m_pCaptureDecl );
	Renderer::Get().SetPixelShader( m_pCapturePS_normal );

	static const float afVertices[] = 
	{
		-1.0f,  1.0f,		0.0f, 0.0f, 
		 3.0f,  1.0f,		2.0f, 0.0f, 
		-1.0f, -3.0f,		0.0f, 2.0f, 
	};

	Renderer::Get().SetTexture( 0, pSource->GetTexture() );
	Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_POINT );

	float fLinearScale = RenderingContext::Get()->m_exposureSettings.m_fKeyValueMapping;
	static CVector const obOne( 1.0f, 1.0f, 1.0f, 1.0f );

	CVector obOffset = ( 4.0f*GetMin()/fLinearScale )*obOne;
	CVector obScale = ( 0.25f*fLinearScale/( GetMax() - GetMin() ) )*obOne;
	
	Renderer::Get().SetTexture( 1, pKeyLuminance );
	Renderer::Get().SetSamplerAddressMode( 1, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerFilterMode( 1, TEXTUREFILTER_POINT );

	float fXOffset = 0.25f/_R( pSource->GetWidth() );
	float fYOffset = 0.25f/_R( pSource->GetHeight() );
	CVector obFilterOffsets( fXOffset, fYOffset, -fXOffset, -fYOffset );

	Renderer::Get().SetPixelShaderConstant( 0, obOffset );
	Renderer::Get().SetPixelShaderConstant( 1, obScale );
	Renderer::Get().SetPixelShaderConstant( 2, obFilterOffsets );
	
	GetD3DDevice()->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 1, &afVertices[0], 4*sizeof( float ) );

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
	m_blurHelper.RecursiveBlur( pobResult, D3DFMT_A8R8G8B8, CVector( 1.0f, 1.0f, 1.0f, 1.0f ), 0, GetGausianLevels() );

	if( CRendererSettings::bEnableLensGhost )
	{
		// ghost screen flip & vignettes
		int iGhostWidth = (int)(_R(pSource->GetWidth()) * 0.25f);
		int iGhostHeight = (int)(_R(pSource->GetHeight()) * 0.25f);

		// pass 1 makes a vignetted and scaled combination of hi and lo res version of the screen
		RenderTarget::CreationStruct ghostParams( iGhostWidth, iGhostHeight, surfaceFormat, false );
		ghostParams.bCacheable = bCacheable;

		RenderTarget::Ptr pobGhostTemp = SurfaceManager::Get().CreateRenderTarget( ghostParams );
		Renderer::Get().m_targetCache.SetColourTarget( pobGhostTemp );

		Renderer::Get().SetVertexShader( m_pCaptureVS );
		Renderer::Get().m_Platform.SetVertexDeclaration( m_pCaptureDecl );
		Renderer::Get().SetPixelShader( m_pGhostPass1PS_normal );

		// texture 0 = hires screen (un-scaled range)
		Renderer::Get().SetTexture( 0, pSource->GetTexture() );
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
		Renderer::Get().SetPixelShaderConstant( 0, obOffset );
		Renderer::Get().SetPixelShaderConstant( 1, obScale );
		Renderer::Get().SetPixelShaderConstant( 2, obFilterOffsets );

		// ghost scales
		CVector obScalePass1( pDef->m_fGhostScaleSharp, pDef->m_fGhostScaleBlurred, 0, 0 );
		Renderer::Get().SetPixelShaderConstant( 3, obScalePass1 );

		GetD3DDevice()->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 1, &afVertices[0], 4*sizeof( float ) );

		Renderer::Get().SetTexture( 3, Texture::NONE );

		// pass 2 used pass 1 8 times modulating and scaling to produce a model of the internal reflection
		// then combines with the gaussian blurred buffer
		RenderTarget::CreationStruct combineParams( iGhostWidth, iGhostHeight, surfaceFormat, false );
		combineParams.bCacheable = bCacheable;

		RenderTarget::Ptr pobCombineTemp = SurfaceManager::Get().CreateRenderTarget( combineParams );
		Renderer::Get().m_targetCache.SetColourTarget( pobCombineTemp );
		
		// render into rendertarget
		Renderer::Get().SetVertexShader( m_pCaptureVS );
		Renderer::Get().m_Platform.SetVertexDeclaration( m_pCaptureDecl );
		Renderer::Get().SetPixelShader( m_pGhostPass2PS_normal );

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
		Renderer::Get().SetPixelShaderConstant( 0, pDef->m_obGhostFinalTint );

		CVector obGhostScale[2];
		obGhostScale[0] = CVector(	pDef->m_fGhostScale[0], pDef->m_fGhostScale[1], 
									pDef->m_fGhostScale[2], pDef->m_fGhostScale[3] );
		obGhostScale[1] = CVector(	pDef->m_fGhostScale[4], pDef->m_fGhostScale[5], 
									pDef->m_fGhostScale[6], pDef->m_fGhostScale[7] );
		// ghost scales and modulate(8 samples)
		Renderer::Get().SetPixelShaderConstant( 1, obGhostScale, 2 );
		Renderer::Get().SetPixelShaderConstant( 3, pDef->m_obGhostModulate, 8 );

		GetD3DDevice()->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 1, &afVertices[0], 4*sizeof( float ) );

		// gaussian blur
		SurfaceManager::Get().ReleaseRenderTarget( pobGhostTemp );
		SurfaceManager::Get().ReleaseRenderTarget( pobResult );
		pobResult = pobCombineTemp;

		m_blurHelper.RecursiveBlur( pobResult, surfaceFormat, CVector( 1.0f, 1.0f, 1.0f, 1.0f ), 0, iGhostGaussLevel );

		Renderer::Get().SetTexture( 0, Texture::NONE );
		Renderer::Get().SetTexture( 1, Texture::NONE );
		Renderer::Get().SetTexture( 2, Texture::NONE );
	}

	return pobResult;
}

RenderTarget::Ptr LensArtifacts::Generate( float fKeyLuminance, const RenderTarget::Ptr& pSource )
{
	ntAssert_p( LevelLighting::Exists(), ("WTF? Level lighting does not exist...") );
	ntAssert_p( LevelLighting::Get().GetGhostSetting() != 0, ("Level lighting does not have a ghost lens definition") );

	const CGhostLensEffectSettings* pDef = LevelLighting::Get().GetGhostSetting();

	// get the source dimensions
	D3DFORMAT surfaceFormat = ConvertGFXFORMATToD3DFORMAT(Renderer::Get().GetHDRFormat());
	if ( HardwareCapabilities::Get().IsValidRenderTargetFormat(D3DFMT_A16B16G16R16) )
		surfaceFormat = D3DFMT_A16B16G16R16;

	int iGlowWidth = (int)(_R(pSource->GetWidth()) * 0.25f);
	int iGlowHeight = (int)(_R(pSource->GetHeight()) * 0.25f);
	bool bCacheable = pSource->m_Platform.IsCacheable();

	// allocate rendertarget and setp
	RenderTarget::CreationStruct artifactParams( iGlowWidth, iGlowHeight, surfaceFormat, false );
	artifactParams.bCacheable = bCacheable;

	RenderTarget::Ptr pobResult = SurfaceManager::Get().CreateRenderTarget( artifactParams );
	Renderer::Get().m_targetCache.SetColourTarget( pobResult );

	// early out if disabled
	if( !CRendererSettings::bEnableLensEffects )
	{
		Renderer::Get().Clear( D3DCLEAR_TARGET, 0, 1.0f, 0 );
		return pobResult;
	}

	// otherwise capture lores scaled buffer
	Renderer::Get().SetVertexShader( m_pCaptureVS );
	Renderer::Get().m_Platform.SetVertexDeclaration( m_pCaptureDecl );
	Renderer::Get().SetPixelShader( m_pCapturePS_cpuexp );

	static const float afVertices[] = 
	{
		-1.0f,  1.0f,		0.0f, 0.0f, 
		 3.0f,  1.0f,		2.0f, 0.0f, 
		-1.0f, -3.0f,		0.0f, 2.0f, 
	};

	Renderer::Get().SetTexture( 0, pSource->GetTexture() );
	Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_POINT );

	float fLinearScale = RenderingContext::Get()->m_exposureSettings.m_fKeyValueMapping;
	static CVector const obOne( 1.0f, 1.0f, 1.0f, 1.0f );

	CVector obOffset = ( 4.0f*GetMin()/fLinearScale )*obOne;
	CVector obScale = ( 0.25f*fLinearScale/( GetMax() - GetMin() ) )*obOne;
	
	obOffset = obOffset * fKeyLuminance;
	obScale = obScale * (1.0f / fKeyLuminance);

	float fXOffset = 0.25f/_R( pSource->GetWidth() );
	float fYOffset = 0.25f/_R( pSource->GetHeight() );
	CVector obFilterOffsets( fXOffset, fYOffset, -fXOffset, -fYOffset );

	Renderer::Get().SetPixelShaderConstant( 0, obOffset );
	Renderer::Get().SetPixelShaderConstant( 1, obScale );
	Renderer::Get().SetPixelShaderConstant( 2, obFilterOffsets );
	
	GetD3DDevice()->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 1, &afVertices[0], 4*sizeof( float ) );

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
	m_blurHelper.RecursiveBlur( pobResult, D3DFMT_A8R8G8B8, CVector( 1.0f, 1.0f, 1.0f, 1.0f ), 0, GetGausianLevels() );

	if( CRendererSettings::bEnableLensGhost )
	{
		// ghost screen flip & vignettes
		int iGhostWidth = (int)(_R(pSource->GetWidth()) * 0.5f);
		int iGhostHeight = (int)(_R(pSource->GetHeight()) * 0.5f);

		// pass 1 makes a vignetted and scaled combination of hi and lo res version of the screen
		RenderTarget::CreationStruct ghostParams( iGhostWidth, iGhostHeight, surfaceFormat, false );
		ghostParams.bCacheable = bCacheable;

		RenderTarget::Ptr pobGhostTemp = SurfaceManager::Get().CreateRenderTarget( ghostParams );
		Renderer::Get().m_targetCache.SetColourTarget( pobGhostTemp );

		Renderer::Get().SetVertexShader( m_pCaptureVS );
		Renderer::Get().m_Platform.SetVertexDeclaration( m_pCaptureDecl );
		Renderer::Get().SetPixelShader( m_pGhostPass1PS_cpuexp );

		// texture 0 = hires screen (un-scaled range)
		Renderer::Get().SetTexture( 0, pSource->GetTexture() );
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

		// same as previous pass (to get correct scales)
		Renderer::Get().SetPixelShaderConstant( 0, obOffset );
		Renderer::Get().SetPixelShaderConstant( 1, obScale );
		Renderer::Get().SetPixelShaderConstant( 2, obFilterOffsets );

		// ghost scales
		CVector obScalePass1( pDef->m_fGhostScaleSharp, pDef->m_fGhostScaleBlurred, 0, 0 );
		Renderer::Get().SetPixelShaderConstant( 3, obScalePass1 );

		GetD3DDevice()->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 1, &afVertices[0], 4*sizeof( float ) );

		Renderer::Get().SetTexture( 3, Texture::NONE );

		// pass 2 used pass 1 8 times modulating and scaling to produce a model of the internal reflection
		// then combines with the gaussian blurred buffer
		RenderTarget::CreationStruct combineParams( iGhostWidth, iGhostHeight, surfaceFormat, false );
		combineParams.bCacheable = bCacheable;

		RenderTarget::Ptr pobCombineTemp = SurfaceManager::Get().CreateRenderTarget( combineParams );
		Renderer::Get().m_targetCache.SetColourTarget( pobCombineTemp );
		
		// render into rendertarget
		Renderer::Get().SetVertexShader( m_pCaptureVS );
		Renderer::Get().m_Platform.SetVertexDeclaration( m_pCaptureDecl );
		Renderer::Get().SetPixelShader( m_pGhostPass2PS_cpuexp );

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
		Renderer::Get().SetPixelShaderConstant( 0, pDef->m_obGhostFinalTint );

		CVector obGhostScale[2];
		obGhostScale[0] = CVector(	pDef->m_fGhostScale[0], pDef->m_fGhostScale[1], 
									pDef->m_fGhostScale[2], pDef->m_fGhostScale[3] );
		obGhostScale[1] = CVector(	pDef->m_fGhostScale[4], pDef->m_fGhostScale[5], 
									pDef->m_fGhostScale[6], pDef->m_fGhostScale[7] );
		// ghost scales and modulate(8 samples)
		Renderer::Get().SetPixelShaderConstant( 1, obGhostScale, 2 );
		Renderer::Get().SetPixelShaderConstant( 10, &pDef->m_obGhostModulate[0], 8 );

		GetD3DDevice()->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 1, &afVertices[0], 4*sizeof( float ) );

		// gaussian blur
		SurfaceManager::Get().ReleaseRenderTarget( pobGhostTemp );
		SurfaceManager::Get().ReleaseRenderTarget( pobResult );
		pobResult = pobCombineTemp;

		m_blurHelper.RecursiveBlur( pobResult, surfaceFormat, CVector( 1.0f, 1.0f, 1.0f, 1.0f ), 0, iGhostGaussLevel );

		Renderer::Get().SetTexture( 0, Texture::NONE );
		Renderer::Get().SetTexture( 1, Texture::NONE );
		Renderer::Get().SetTexture( 2, Texture::NONE );
	}

	return pobResult;
}