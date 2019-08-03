//--------------------------------------------------
//!
//!	\file depthoffield.h
//!	Depth of field functionality
//!
//--------------------------------------------------

#include "gfx/depthoffield.h"
#include "gfx/renderer.h"
#include "gfx/graphicsdevice.h"
#include "gfx/hardwarecaps.h"
#include "gfx/surfacemanager.h"
#include "gfx/rendercontext.h"
#include "input/inputhardware.h"		// for debug control only

//--------------------------------------------------
//!
//!	DepthOfField::ctor
//!
//--------------------------------------------------
DepthOfField::DepthOfField()
{
	if (!HardwareCapabilities::Get().SupportsPixelShader3())
		return;

	// create vertex shaders
	//-------------------------------------
	m_simpleVS = DebugShaderCache::Get().LoadShader( "passthrough_pos_tex_vp.sho" );
	m_simple2VS = DebugShaderCache::Get().LoadShader( "passthrough_pos_tex2_vp.sho" );

	// create new pixel shaders
	//-------------------------------------
	m_debugCapturePS = DebugShaderCache::Get().LoadShader( "dof_alpha2rgb_fp.sho" );
	m_debugRenderPS = DebugShaderCache::Get().LoadShader( "simpletexture2_lerp_fp.sho" );
	m_pokeAlphaChannelPS = DebugShaderCache::Get().LoadShader( "dof_setalpha_fp.sho" );
	m_downSamplePS = DebugShaderCache::Get().LoadShader( "simpletexture_fp.sho" );
	m_depthOfFieldPS = DebugShaderCache::Get().LoadShader( "dof_generate_fp.sho" );

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
}

//--------------------------------------------------
//!
//!	DepthOfField::GetDebugDepthOfField
//!
//--------------------------------------------------
RenderTarget::Ptr DepthOfField::GetDebugDepthOfField( RenderTarget::Ptr source )
{
	// allocate a result texture
	RenderTarget::Ptr result = 
		SurfaceManager::Get().CreateRenderTarget( source->GetWidth(), source->GetHeight(), GF_ARGB8 );

	Renderer::Get().m_targetCache.SetColourTarget( result );

	// do our stuff
	Renderer::Get().SetVertexShader( m_simpleVS );
	Renderer::Get().SetPixelShader( m_debugCapturePS );

	Renderer::Get().m_Platform.SetStream( m_hSimpleQuadData );

	Renderer::Get().SetTexture( 0, source->GetTexture() );
	Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_BILINEAR );
	
	Renderer::Get().m_Platform.DrawPrimitives( Gc::kQuads, 0, 4 );
	Renderer::Get().m_Platform.ClearStreams();

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
		!viewport.m_DOFSettings.m_bApplyDepthOfField)
		return;

	RenderTarget::Ptr nonMSAAcolour = Renderer::Get().m_targetCache.GetPrimaryColourTarget();
	Renderer::Get().m_targetCache.SetColourTarget( nonMSAAcolour );

	Renderer::Get().SetVertexShader( m_simple2VS );

	Renderer::Get().m_Platform.SetStream( m_hSimpleQuadData );

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
	m_debugRenderPS->SetPSConstantByName( "lerpVal", lerp );
	Renderer::Get().SetPixelShader( m_debugRenderPS );
	
	Renderer::Get().m_Platform.DrawPrimitives( Gc::kQuads, 0, 4 );
	Renderer::Get().m_Platform.ClearStreams();

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
void DepthOfField::ApplyDepthOfField(	RenderTarget::Ptr& source,
										RenderTarget::Ptr& floatZ,
										RenderTarget::Ptr& scratchMem )
{
	PIPView& viewport = Renderer::Get().m_pPIPManager->GetCurrentView();
	if (!viewport.m_DOFSettings.m_bApplyDepthOfField)
		return;

	// First generate our 0->1 DOF value, stick it in the
	// texture's alpha channel, so the depth can be gaussian'd down
	//------------------------------------------------------------

	// note: as we're faking a result using scratchMemory, the caller
	// MUST be certain he's providing enough memory to store the result

	RenderTarget::CreationStruct intermediateParams(
		scratchMem->m_Platform.GetRenderBuffer()->GetDataAddress(),
		scratchMem->m_Platform.GetRenderBuffer()->GetPitch(),
		source->GetWidth(), source->GetHeight(), source->GetFormat() );

	RenderTarget::Ptr intermediate = SurfaceManager::Get().CreateRenderTarget( intermediateParams );

	Renderer::Get().m_targetCache.SetColourTarget( intermediate );
	Renderer::Get().SetVertexShader( m_simpleVS );
	
	m_pokeAlphaChannelPS->SetPSConstantByName(
		"depthOfFieldParams", RenderingContext::Get()->m_depthOfFieldParams );

	Renderer::Get().SetPixelShader( m_pokeAlphaChannelPS );

	Renderer::Get().m_Platform.SetStream( m_hSimpleQuadData );

	Renderer::Get().SetTexture( 0, source->GetTexture() );
	Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_POINT );

	Renderer::Get().SetTexture( 1, floatZ->GetTexture() );
	Renderer::Get().SetSamplerAddressMode( 1, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerFilterMode( 1, TEXTUREFILTER_POINT );

	Renderer::Get().m_Platform.DrawPrimitives( Gc::kQuads, 0, 4 );
	Renderer::Get().m_Platform.ClearStreams();

	// Now it's valid to grab a debug copy of it
	//------------------------------------------------------------
	if (CRendererSettings::bGetDebugDOF)
		m_debugAlphaChannel = GetDebugDepthOfField( intermediate );

	// get the source dimensions
	u_int iFullWidth = intermediate->GetWidth();
	u_int iFullHeight = intermediate->GetHeight();

	// we now use a fixed size due to our multiple viewport rendering.
	u_int iMiniWidth = 320;
	u_int iMiniHeight = 180;

	// downsample the intermediate to a 1/16th
	//------------------------------------------------
	RenderTarget::Ptr miniBuffer = 
		SurfaceManager::Get().CreateRenderTarget( iMiniWidth, iMiniHeight, intermediate->GetFormat() );

	Renderer::Get().m_targetCache.SetColourTarget( miniBuffer );

	Renderer::Get().SetVertexShader( m_simpleVS );
	Renderer::Get().SetPixelShader( m_downSamplePS );

	Renderer::Get().m_Platform.SetStream( m_hSimpleQuadData );

	Renderer::Get().SetTexture( 0, intermediate->GetTexture() );
	Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_BILINEAR );

	Renderer::Get().m_Platform.DrawPrimitives( Gc::kQuads, 0, 4 );
	Renderer::Get().m_Platform.ClearStreams();

	Renderer::Get().SetTexture( 0, Texture::NONE );

	// now we gausian blur our mini buffer
	//------------------------------------------------
	// ATTN! its important that miniBuffer does not vary in size, (by PIP for instance)
	// or we will rapidly run out of VRAM. Hence the fixed size of iMiniWidth / iMiniHieght
	m_blurHelper.RecursiveBlur( miniBuffer, 
								intermediate->GetFormat(),
								CVector( 1.0f, 1.0f, 1.0f, 1.0f ), 0, 2 );

	if (CRendererSettings::bGetDebugDOF)
		m_debugAlphaChannelBlur = GetDebugDepthOfField( miniBuffer );

	// now render the full screen result texture
	//------------------------------------------------
	Renderer::Get().m_targetCache.SetColourTarget( source );
	Renderer::Get().SetVertexShader( m_simpleVS );
	Renderer::Get().m_Platform.SetStream( m_hSimpleQuadData );

	Renderer::Get().SetTexture( 0, intermediate->GetTexture() );
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

	m_depthOfFieldPS->SetPSConstantByName( "pixelSizeHigh", pixelSizeHigh );
	m_depthOfFieldPS->SetPSConstantByName( "pixelSizeLow", pixelSizeLow );
	m_depthOfFieldPS->SetPSConstantByName( "maxCoC", maxCoC );
	
	Renderer::Get().SetPixelShader( m_depthOfFieldPS );
	
	Renderer::Get().m_Platform.DrawPrimitives( Gc::kQuads, 0, 4 );
	Renderer::Get().m_Platform.ClearStreams();

	Renderer::Get().SetTexture( 0, Texture::NONE );
	Renderer::Get().SetTexture( 1, Texture::NONE );

	// we only deallocate the mini buffer, all others must be handled by the 
	// caller. Note: the result is now in 'source'
	SurfaceManager::Get().ReleaseRenderTarget( miniBuffer );
}

