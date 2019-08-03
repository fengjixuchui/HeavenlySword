/**
	@file radialmotionblur.cpp

	@author campf

	Radial motion blur effect for arrow-cam and missile-cam
*/

#include "gfx/radialmotionblur.h"
#include "gfx/renderer.h"
#include "gfx/graphicsdevice.h"
#include "gfx/hardwarecaps.h"
#include "gfx/surfacemanager.h"
#include "gfx/rendercontext.h"
#include "gfx/texturemanager.h"

// This factor control how far apart the motion blur images are
#define ADD 0.03f

/**
	Constructor for motion blur effect
*/
CRadialMotionBlur::CRadialMotionBlur()
{
	#ifdef PLATFORM_PS3
	// Create static vertex buffers for sprite manipulation
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

	m_hUnscaledQuadData = RendererPlatform::CreateVertexStream( 4, sizeof( float ) * 4, 2, simpleQuadDesc );
	m_hUnscaledQuadData->Write( simpleQuad );

	for (int i=0; i<2; i++)
	{
		float fAdd = ADD*(i+1);
		float const simpleQuadUp[] = 
		{
			-(1.0f+fAdd),	1.0f+fAdd,		0.0f,	0.0f,		// 0
			1.0f+fAdd,		1.0f+fAdd,		1.0f,	0.0f,		// 1
			1.0f+fAdd,		-(1.0f+fAdd),	1.0f,	1.0f,		// 2
			-(1.0f+fAdd),	 -(1.0f+fAdd),	0.0f,	1.0f,		// 3
		};

		m_hScaleUpQuadData[i] = RendererPlatform::CreateVertexStream( 4, sizeof( float ) * 4, 2, simpleQuadDesc );
		m_hScaleUpQuadData[i]->Write( simpleQuadUp );
	}

	for (int i=0; i<2; i++)
	{
		float fAdd = ADD*(i+1);
		float const simpleQuadDown[] = 
		{
			-(1.0f-fAdd),  1.0f-fAdd,	0.0f,	0.0f,		// 0
			1.0f-fAdd,  1.0f-fAdd,		1.0f,	0.0f,		// 1
			1.0f-fAdd, -(1.0f-fAdd),	1.0f,	1.0f,		// 2
			-(1.0f-fAdd), -(1.0f-fAdd),	0.0f,	1.0f,		// 3
		};

		m_hScaleDownQuadData[i] = RendererPlatform::CreateVertexStream( 4, sizeof( float ) * 4, 2, simpleQuadDesc );
		m_hScaleDownQuadData[i]->Write( simpleQuadDown );
	}

	m_hDynamicQuadData = RendererPlatform::CreateVertexStream( 4, sizeof( float ) * 4, 2, simpleQuadDesc, Gc::kScratchBuffer );

	// Load the shaders
	m_hQuadVertexShader = DebugShaderCache::Get().LoadShader( "passthrough_pos_tex_vp.sho" );
	m_hQuadFragmentShader = DebugShaderCache::Get().LoadShader( "simpletexture_fp.sho" );
	m_hFilterDownsampleShader = DebugShaderCache::Get().LoadShader( "radialblurdownsample_fp.sho" );
	
	// Load the mask texture we use
	m_maskTexture = TextureManager::Get().LoadTexture_Neutral( "radialblurmask_colour_alpha.dds" );

	// Default mask size
	m_fMaskSize = 0.5f;

	// Disabled by default
	m_bEnabled = false;
	#endif
}


/**
	@param LDR720pBuffer Pointer to LDR 720p rendertarget.
	@param fMaskSize Size parameter for mask - this controls how much of the effect is visible.

	It ranges from -1.3 (not visible) to 0.5 (very visible). Actually you can use any value you like for this parameter.
*/
void CRadialMotionBlur::RenderInternal( RenderTarget::Ptr &LDR720pBuffer )
{
	#ifdef PLATFORM_PS3

	//
	// Shrink 720p buffer down to 180p
	//

	

	// allocate 180p render-target
	RenderTarget::Ptr pob180pTemp = SurfaceManager::Get().CreateRenderTarget( RENDER_TARGET_WIDTH, RENDER_TARGET_HEIGHT, GF_ARGB8 );

	// set framebuffer as a texture
	Renderer::Get().m_targetCache.SetColourTarget( pob180pTemp );
	Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_BILINEAR );
	Renderer::Get().SetTexture( 0, LDR720pBuffer->GetTexture() );

	GcKernel::Disable( Gc::kBlend );
	GcKernel::SetBlendEquation( Gc::kBlendEquationAdd );

	// Use a special filter designed to respect the nyquist limit, removing high frequencies from the source during the downsample
	float fXOffset = 0.25f/_R( 320.f );
	float fYOffset = 0.25f/_R( 180.f );
	CVector filter_offsets( fXOffset, fYOffset, -fXOffset, -fYOffset );
	m_hFilterDownsampleShader->SetPSConstantByName("filter_offsets", filter_offsets);

	Renderer::Get().SetVertexShader( m_hQuadVertexShader );
	Renderer::Get().SetPixelShader( m_hFilterDownsampleShader );

	Renderer::Get().m_Platform.SetStream( m_hUnscaledQuadData );
	Renderer::Get().m_Platform.DrawPrimitives( Gc::kQuads, 0, 4 );
	Renderer::Get().m_Platform.ClearStreams();
	
	// Gausian blur the 180p copy to help remove motion blur artefacts later
	m_blurHelper.RecursiveBlur( pob180pTemp, pob180pTemp->GetFormat(), CVector( 1.0f, 1.0f, 1.0f, 1.0f ), 0, 1 );

	Renderer::Get().SetVertexShader( m_hQuadVertexShader );
	Renderer::Get().SetPixelShader( m_hQuadFragmentShader );

	// Allocate these render targets
	RenderTarget::Ptr pob180pUp = SurfaceManager::Get().CreateRenderTarget( RENDER_TARGET_WIDTH, RENDER_TARGET_HEIGHT, GF_ARGB8 );
	RenderTarget::Ptr pob180pDown = SurfaceManager::Get().CreateRenderTarget( RENDER_TARGET_WIDTH, RENDER_TARGET_HEIGHT, GF_ARGB8 );

	//
	// Scale up blur into pob180pUp
	//

	// First blit is full alpha
	Renderer::Get().m_targetCache.SetColourTarget( pob180pUp );
	Renderer::Get().SetTexture( 0, pob180pTemp->GetTexture() );
	Renderer::Get().m_Platform.SetStream( m_hScaleUpQuadData[0] );
	Renderer::Get().m_Platform.DrawPrimitives( Gc::kQuads, 0, 4 );
	Renderer::Get().m_Platform.ClearStreams();

	// Second blit is half alpha
	GcKernel::Enable( Gc::kBlend );
	GcKernel::SetBlendColour(1.0f,1.0f,1.0f,0.5f);
	GcKernel::SetBlendFunc(	Gc::kBlendConstAlpha, Gc::kBlendOneMinusConstAlpha );
	Renderer::Get().m_Platform.SetStream( m_hScaleUpQuadData[1] );
	Renderer::Get().m_Platform.DrawPrimitives( Gc::kQuads, 0, 4 );
	Renderer::Get().m_Platform.ClearStreams();

	//
	// Scale down blur into pob180pDown
	//

	// First blit is full alpha
	GcKernel::Disable( Gc::kBlend );
	Renderer::Get().m_targetCache.SetColourTarget( pob180pDown );
	Renderer::Get().SetTexture( 0, pob180pTemp->GetTexture() );
	Renderer::Get().m_Platform.SetStream( m_hScaleDownQuadData[0] );
	Renderer::Get().m_Platform.DrawPrimitives( Gc::kQuads, 0, 4 );
	Renderer::Get().m_Platform.ClearStreams();

	// Second blit is half alpha
	GcKernel::Enable( Gc::kBlend );
	GcKernel::SetBlendFunc(	Gc::kBlendConstAlpha, Gc::kBlendOneMinusConstAlpha );
	Renderer::Get().m_Platform.SetStream( m_hScaleDownQuadData[1] );
	Renderer::Get().m_Platform.DrawPrimitives( Gc::kQuads, 0, 4 );
	Renderer::Get().m_Platform.ClearStreams();


	//
	// Combine these two with the original 180p buffer
	//
	Renderer::Get().SetTexture( 0, pob180pDown->GetTexture() );
	Renderer::Get().m_targetCache.SetColourTarget( pob180pTemp );
	Renderer::Get().m_Platform.SetStream( m_hUnscaledQuadData );
	Renderer::Get().m_Platform.DrawPrimitives( Gc::kQuads, 0, 4 );
	Renderer::Get().m_Platform.ClearStreams();

	Renderer::Get().SetTexture( 0, pob180pUp->GetTexture() );
	Renderer::Get().m_Platform.SetStream( m_hUnscaledQuadData );
	Renderer::Get().m_Platform.DrawPrimitives( Gc::kQuads, 0, 4 );
	Renderer::Get().m_Platform.ClearStreams();

	// draw the circular mask into pob180pTemp alpha only!
	Renderer::Get().SetTexture( 0, m_maskTexture );
	GcKernel::SetColourMask(false, false, false, true);
	GcKernel::Disable( Gc::kBlend );

	float const simpleQuad[] = 
	{
		-(1.0f-m_fMaskSize),  1.0f-m_fMaskSize,		0.0f,	0.0f,		// 0
		1.0f-m_fMaskSize,  1.0f-m_fMaskSize,		1.0f,	0.0f,		// 1
		1.0f-m_fMaskSize, -(1.0f-m_fMaskSize),		1.0f,	1.0f,		// 2
		-(1.0f-m_fMaskSize), -(1.0f-m_fMaskSize),	0.0f,	1.0f,		// 3
	};

	m_hDynamicQuadData->GetNewScratchMemory();
	m_hDynamicQuadData->Write( simpleQuad, 0, sizeof(simpleQuad) );

	Renderer::Get().m_Platform.SetStream( m_hDynamicQuadData );
	Renderer::Get().m_Platform.DrawPrimitives( Gc::kQuads, 0, 4 );
	Renderer::Get().m_Platform.ClearStreams();

	//
	// Draw the whole effect over the 720p back buffer using the mask defined above
	// 

	Renderer::Get().m_targetCache.SetColourTarget( LDR720pBuffer );

	// reset write mask and set blend mode to src alpha
	GcKernel::SetColourMask(true, true, true, false);
	GcKernel::Enable( Gc::kBlend );
	GcKernel::SetBlendFunc(	Gc::kBlendSrcAlpha, Gc::kBlendOneMinusSrcAlpha );
	Renderer::Get().SetTexture( 0, pob180pTemp->GetTexture() );
	Renderer::Get().m_Platform.SetStream( m_hUnscaledQuadData );
	Renderer::Get().m_Platform.DrawPrimitives( Gc::kQuads, 0, 4 );
	Renderer::Get().m_Platform.ClearStreams();

	// Clean up...
	SurfaceManager::Get().ReleaseRenderTarget(pob180pUp);
	SurfaceManager::Get().ReleaseRenderTarget(pob180pDown);
	SurfaceManager::Get().ReleaseRenderTarget(pob180pTemp);

	#endif
}
