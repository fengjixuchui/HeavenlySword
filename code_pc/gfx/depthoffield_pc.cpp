//--------------------------------------------------
//!
//!	\file depthoffield.h
//!	Depth of field functionality
//!
//--------------------------------------------------

#include "gfx/depthoffield.h"
#include "gfx/shader.h"
#include "gfx/renderer.h"
#include "gfx/graphicsdevice.h"
#include "gfx/hardwarecaps.h"
#include "gfx/surfacemanager.h"
#include "gfx/pictureinpicture.h"
#include "input/inputhardware.h"		// for debug control only

// verts for rendering a screen covering tri
static const float g_afVertices[] = 
{
	-1.0f,  1.0f,		0.0f, 0.0f, 
	3.0f,  1.0f,		2.0f, 0.0f, 
	-1.0f, -3.0f,		0.0f, 2.0f, 
};
//--------------------------------------------------
//!
//!	DepthOfField::ctor
//!
//--------------------------------------------------
DepthOfField::DepthOfField()
{
	m_pDebugCapturePS = 0;
	m_pDebugRenderPS = 0;
	m_pDownSamplePS = 0;
	m_pDepthOfFieldPS = 0;

	if (!HardwareCapabilities::Get().SupportsPixelShader3())
		return;

	// create NT_NEW_CHUNK(Mem::MC_GFX) vertex shaders
	//-------------------------------------
	m_pSimpleVS = ShaderManager::Get().FindShader( "effects_resample_vs" );

	// create NT_NEW_CHUNK(Mem::MC_GFX) pixel shaders
	//-------------------------------------
	DebugShader* pNewPS;

	// transmit alpha to rgb
	pNewPS = NT_NEW_CHUNK(Mem::MC_GFX) DebugShader;
	pNewPS->SetHLSLFunction(
		SHADERTYPE_PIXEL,

		"float4 debug_dof( float2 texcoord : TEXCOORD0, "
		"					uniform sampler2D source : register(s0) ) : COLOR0"
		"{	"
		"	float4 colour = tex2D( source, texcoord );"
		"	float4 result = colour.wwww;"
		"	result.w = 1.0f;"
		"	return result;"
		"}",

		"debug_dof",
		"ps.2.0"
	);

	m_pDebugCapturePS = pNewPS;

	// debug pass through
	pNewPS = NT_NEW_CHUNK(Mem::MC_GFX) DebugShader;
	pNewPS->SetHLSLFunction(
		SHADERTYPE_PIXEL,

		"float4 debug_render( float2 texcoord : TEXCOORD0, "
		"					uniform float4 lerp_val : register(c0), "
		"					uniform sampler2D source0 : register(s0), "
		"					uniform sampler2D source1 : register(s1) ) : COLOR0"
		"{	"
		"	float4 col0 = tex2D( source0, texcoord ); "
		"	float4 col1 = tex2D( source1, texcoord ); "
		"	return lerp( col0, col1, lerp_val ); "
		"}",

		"debug_render",
		"ps.2.0"
	);

	m_pDebugRenderPS = pNewPS;

	// downsample
	pNewPS = NT_NEW_CHUNK(Mem::MC_GFX) DebugShader;
	pNewPS->SetHLSLFunction(
		SHADERTYPE_PIXEL,

		"float4 debug_render( float2 texcoord : TEXCOORD0, "
		"					uniform sampler2D source : register(s0) ) : COLOR0"
		"{	"
		"	return tex2D( source, texcoord );"
		"}",

		"debug_render",
		"ps.2.0"
	);

	m_pDownSamplePS = pNewPS;

	// dof shader
	pNewPS = NT_NEW_CHUNK(Mem::MC_GFX) DebugShader;
	pNewPS->SetHLSLFunction(
		SHADERTYPE_PIXEL,

		"float4 depthoffield( float2 texcoord : TEXCOORD0, "
		"					uniform float4 pixelSizeHigh : register(c0), "
		"					uniform float4 pixelSizeLow : register(c1), "
		"					uniform float4 maxCoC : register(c2), "
		"					uniform sampler2D source		: register(s0), "
		"					uniform sampler2D sourceblur	: register(s1) ) : COLOR0"
		"{	"
		
		// these are consts
		"	float2 poisson[8]; "
		"	poisson[0] = float2( 0.16, 0.86); "
		"	poisson[1] = float2( 0.70, 0.62); "
		"	poisson[2] = float2( 0.70, 0.20); "
		"	poisson[3] = float2( 0.70,-0.48); "
		"	poisson[4] = float2( 0.28,-0.84); "
		"	poisson[5] = float2(-0.26,-0.58); "
		"	poisson[6] = float2(-0.60, 0.00); "
		"	poisson[7] = float2(-0.76, 0.46); "
		
		// guts of shader
		"	float4	cOut; "
		"	float	discRadius, discRadiusLow, centerDepth; "

		"	cOut = tex2D( source, texcoord ); "	// fetch center tap
		"	centerDepth = cOut.a; "				// save its depth

		// convert depth into blur radius in pixels
		"	discRadius = abs (cOut.a * maxCoC.y - maxCoC.x); "
		"	discRadiusLow = abs (cOut.a * maxCoC.w - maxCoC.z); "
		"	cOut = 0; "

		"	for (int t = 0; t < 8; t++) "
		"	{ "
		// compute tap tecture coordinates
		"		float2	coordLow = texcoord + (pixelSizeLow.xy * poisson[t] * discRadiusLow); "
		"		float2	coordHigh = texcoord + (pixelSizeHigh.xy * poisson[t] * discRadius); "
		// fetch high res tap
		"		float4	tapLow = tex2D (sourceblur, coordLow); "
		"		float4	tapHigh = tex2D (source, coordHigh); "
		// mix low and hi rex taps based on blurriness
		"		float tapBlur = abs (tapHigh.a * 2.0 - 1.0); " // put blurrines into [0, 1]
		"		float4 tap = lerp (tapHigh, tapLow, tapBlur); "
		// "smart" blur ignores taps that are closer than the center tap and in focus 
		"		tap.a = (tap.a >= centerDepth) ? 1.0 : abs (tap.a * 2.0 - 1.0); "

		"		cOut.rgb += tap.rgb * tap.a; "	// accumulate
		"		cOut.a += tap.a; "
		"	} "

		"	return (cOut / cOut.a);"
		"}",

		"depthoffield",
		"ps_3_0"
	);

	m_pDepthOfFieldPS = pNewPS;

	// create the declarations
	//-------------------------------------
	D3DVERTEXELEMENT9 simpleDecl[] = 
	{
		{ 0, 0,					D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0 }, 
		{ 0, 2*sizeof( float ),	D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	0 }, 
		D3DDECL_END()
	};
	m_pSimpleDecl = CVertexDeclarationManager::Get().GetDeclaration( simpleDecl );
}

//--------------------------------------------------
//!
//!	DepthOfField::ctor
//!
//--------------------------------------------------
DepthOfField::~DepthOfField()
{
	if (m_pDebugCapturePS)
		NT_DELETE_CHUNK(Mem::MC_GFX,  m_pDebugCapturePS );

	if (m_pDebugRenderPS)
		NT_DELETE_CHUNK(Mem::MC_GFX,  m_pDebugRenderPS );

	if (m_pDownSamplePS)
		NT_DELETE_CHUNK(Mem::MC_GFX,  m_pDownSamplePS );

	if (m_pDepthOfFieldPS)
		NT_DELETE_CHUNK(Mem::MC_GFX,  m_pDepthOfFieldPS );
}

//--------------------------------------------------
//!
//!	DepthOfField::GetDebugDepthOfField
//! Note! doesnt restore old render targets
//!
//--------------------------------------------------
RenderTarget::Ptr DepthOfField::GetDebugDepthOfField( RenderTarget::Ptr source )
{
	// allocate a result texture
	RenderTarget::CreationStruct dofParams( source->GetWidth(), source->GetHeight(), D3DFMT_A8R8G8B8, false );
	dofParams.bCacheable = source->m_Platform.IsCacheable();

	RenderTarget::Ptr result = SurfaceManager::Get().CreateRenderTarget( dofParams );
	Renderer::Get().m_targetCache.SetColourTarget( result );

	// do our stuff
	Renderer::Get().SetVertexShader( m_pSimpleVS );
	Renderer::Get().m_Platform.SetVertexDeclaration( m_pSimpleDecl );
	Renderer::Get().SetPixelShader( m_pDebugCapturePS );

	Renderer::Get().SetTexture( 0, source->GetTexture() );
	Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_BILINEAR );
	
	GetD3DDevice()->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 1, &g_afVertices[0], 4*sizeof( float ) );

	Renderer::Get().SetTexture( 0, Texture::NONE );

	return result;
}

//--------------------------------------------------
//!
//!	DepthOfField::DebugDisplayDepthOfField
//! This function just renders this texture, fullscreen,
//! over whatever is in the current back buffer
//! it also assume responsibility for deallocing the
//! input texture
//!
//--------------------------------------------------
void DepthOfField::DebugDisplayDepthOfField()
{
	PIPView& viewport = Renderer::Get().m_pPIPManager->GetCurrentView();

	if (!CRendererSettings::bGetDebugDOF ||
		!viewport.m_DOFSettings.m_bApplyDepthOfField ||
		!HardwareCapabilities::Get().SupportsVertexShader3())
		return;

	RenderTarget::Ptr backBufferLDR = Renderer::Get().m_targetCache.GetPrimaryColourTarget();
	Renderer::Get().m_targetCache.SetColourTarget( backBufferLDR );

	Renderer::Get().SetVertexShader( m_pSimpleVS );
	Renderer::Get().m_Platform.SetVertexDeclaration( m_pSimpleDecl );
	Renderer::Get().SetPixelShader( m_pDebugRenderPS );

	Renderer::Get().SetTexture( 0, m_debugAlphaChannel->GetTexture() );
	Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_BILINEAR );

	Renderer::Get().SetTexture( 1, m_debugAlphaChannelBlur->GetTexture() );
	Renderer::Get().SetSamplerAddressMode( 1, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerFilterMode( 1, TEXTUREFILTER_BILINEAR );

	static float fLerp = 0.0f;

	if ( CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_KPAD_PLUS ) )
		fLerp += 0.1f;
	if ( CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_KPAD_MINUS ) )
		fLerp -= 0.1f;

	fLerp = ntstd::Clamp( fLerp, 0.0f, 1.0f );

	CVector lerp(fLerp,fLerp,fLerp,fLerp);
	Renderer::Get().SetPixelShaderConstant( 0, lerp );
	
	GetD3DDevice()->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 1, &g_afVertices[0], 4*sizeof( float ) );

	Renderer::Get().SetTexture( 0, Texture::NONE );
	Renderer::Get().SetTexture( 1, Texture::NONE );

	SurfaceManager::Get().ReleaseRenderTarget( m_debugAlphaChannel );
	SurfaceManager::Get().ReleaseRenderTarget( m_debugAlphaChannelBlur );
}

//--------------------------------------------------
//!
//!	DepthOfField::ApplyDepthOfField
//!
//--------------------------------------------------
void DepthOfField::ApplyDepthOfField( RenderTarget::Ptr& source )
{
	PIPView& viewport = Renderer::Get().m_pPIPManager->GetCurrentView();

	if (!viewport.m_DOFSettings.m_bApplyDepthOfField ||
		!HardwareCapabilities::Get().SupportsVertexShader3())
		return;

	if (CRendererSettings::bGetDebugDOF)
		m_debugAlphaChannel = GetDebugDepthOfField( source );

	u_int iFullWidth = source->GetWidth();
	u_int iFullHeight = source->GetHeight();

	u_int iMiniWidth = iFullWidth>>2;
	u_int iMiniHeight = iFullHeight>>2;

	// down sample the current back buffer to a 1/16th
	//------------------------------------------------
	RenderTarget::CreationStruct miniParams( iMiniWidth, iMiniHeight,  source->m_Platform.GetDXFormat(), false );
	miniParams.bCacheable = source->m_Platform.IsCacheable();

	RenderTarget::Ptr miniBuffer = SurfaceManager::Get().CreateRenderTarget( miniParams );
	Renderer::Get().m_targetCache.SetColourTarget( miniBuffer );

	Renderer::Get().SetVertexShader( m_pSimpleVS );
	Renderer::Get().m_Platform.SetVertexDeclaration( m_pSimpleDecl );
	Renderer::Get().SetPixelShader( m_pDownSamplePS );

	Renderer::Get().SetTexture( 0, source->GetTexture() );
	Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_BILINEAR );
	
	GetD3DDevice()->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 1, &g_afVertices[0], 4*sizeof( float ) );

	Renderer::Get().SetTexture( 0, Texture::NONE );

	// now we gausian blur our mini buffer
	//------------------------------------------------
	m_blurHelper.RecursiveBlur( miniBuffer, 
								source->m_Platform.GetDXFormat(),
								CVector( 1.0f, 1.0f, 1.0f, 1.0f ), 0, 2 );

	if (CRendererSettings::bGetDebugDOF)
		m_debugAlphaChannelBlur = GetDebugDepthOfField( miniBuffer );

	// now render the full screen result texture
	//------------------------------------------------
	RenderTarget::CreationStruct dofParams( source->GetWidth(), source->GetHeight(), source->m_Platform.GetDXFormat(), false );
	dofParams.bCacheable = source->m_Platform.IsCacheable();

	RenderTarget::Ptr result = SurfaceManager::Get().CreateRenderTarget( dofParams );
	Renderer::Get().m_targetCache.SetColourTarget( result );

	Renderer::Get().SetVertexShader( m_pSimpleVS );
	Renderer::Get().m_Platform.SetVertexDeclaration( m_pSimpleDecl );
	Renderer::Get().SetPixelShader( m_pDepthOfFieldPS );

	Renderer::Get().SetTexture( 0, source->GetTexture() );
	Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_BILINEAR );

	Renderer::Get().SetTexture( 1, miniBuffer->GetTexture() );
	Renderer::Get().SetSamplerAddressMode( 1, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerFilterMode( 1, TEXTUREFILTER_BILINEAR );

	CVector pixelSizeHigh( 1.0f / _R(iFullWidth), 1.0f / _R(iFullHeight), 0.0f, 0.0f );
	CVector pixelSizeLow( 1.0f / _R(iMiniWidth), 1.0f / _R(iMiniHeight), 0.0f, 0.0f );

	float fCircleOfConfusion = viewport.m_DOFSettings.m_circleOfConfusion;
	float fCircleOfConfusionLow = viewport.m_DOFSettings.m_circleOfConfusionLow;
	
	CVector maxCoC( fCircleOfConfusion, fCircleOfConfusion*2.0f,
					fCircleOfConfusionLow, fCircleOfConfusionLow*2.0f );

	Renderer::Get().SetPixelShaderConstant( 0, pixelSizeHigh );
	Renderer::Get().SetPixelShaderConstant( 1, pixelSizeLow );
	Renderer::Get().SetPixelShaderConstant( 2, maxCoC );
	
	GetD3DDevice()->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 1, &g_afVertices[0], 4*sizeof( float ) );

	Renderer::Get().SetTexture( 0, Texture::NONE );
	Renderer::Get().SetTexture( 1, Texture::NONE );

	// we must deallocate our input render target and pass the NT_NEW_CHUNK(Mem::MC_GFX) one back
	SurfaceManager::Get().ReleaseRenderTarget( source );
	SurfaceManager::Get().ReleaseRenderTarget( miniBuffer );
	source = result;
}

