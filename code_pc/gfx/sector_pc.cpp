//--------------------------------------------------
//!
//!	\file sector_pc.cpp
//!	PC portion of the sector renderer
//!
//--------------------------------------------------

#include "gfx/sector.h"
#include "gfx/levelofdetail.h"
#include "gfx/renderer.h"
#include "gfx/hardwarecaps.h"
#include "gfx/surfacemanager.h"
#include "gfx/graphicsdevice.h"
#include "gfx/rendercontext.h"
#include "gfx/renderable.h"
#include "gfx/pictureinpicture.h"

#ifdef _SPEEDTREE
	#include "speedtree/SpeedTreeManager.h"
#endif

#include "core/visualdebugger.h"
#include "core/gatso.h"
#include "input/inputhardware.h"


#define DUMP_FRAMEBUFFER( name, framebuffer )			\
	if (bDumpView)										\
	{													\
		static char fileName[32];						\
		sprintf( fileName, "%s_%d", name, iViewID );	\
		framebuffer->SaveToDisk( fileName );			\
	}

//--------------------------------------------------
//!
//!	CSector::RenderContext
//!
//--------------------------------------------------
void CSector::RenderContext()
{
	// retrieve parent viewport
	PIPView& viewport = Renderer::Get().m_pPIPManager->GetCurrentView();

	bool	bDumpView = false;
	int		iViewID = viewport.GetDebugID();

	if	(
		(CInputHardware::Exists()) &&
		(CInputHardware::Get().GetKeyboard().IsKeyPressed( KEYC_INSERT )) || m_bDoDumpFrame
		)
	{
		bDumpView = true;
	}

	m_bDoDumpFrame = m_bForceDump;
	m_bForceDump = false;

	//---------------------------------------------------------------------------------
	// setup LOD system
	// (using last frames visibility flags)
	//---------------------------------------------------------------------------------
	CGatso::Start( "RenderContext::SetLODs" );
	
	if (CLODManager::Exists())
		CLODManager::Get().SetLODs( RenderingContext::Get()->m_pCullCamera );
	
	CGatso::Stop( "RenderContext::SetLODs" );

	//---------------------------------------------------------------------------------
	// do visibility calculation
	//---------------------------------------------------------------------------------
	CGatso::Start( "RenderContext::CalculateVisibility" );
	
	float fPlanePercents[5];
	CalculatePlanePercents( fPlanePercents );
	CalculateVisibility( fPlanePercents );
	
	CGatso::Stop( "RenderContext::CalculateVisibility" );

	if (CRendererSettings::bShowBoundingBoxes)
	{
		for( unsigned int iRenderable = 0; iRenderable < RenderingContext::Get()->m_aVisibleOpaqueRenderables.size(); ++iRenderable )
			RenderingContext::Get()->m_aVisibleOpaqueRenderables[iRenderable]->DisplayBounds();

		for( unsigned int iRenderable = 0; iRenderable < RenderingContext::Get()->m_aBatchedVisibleOpaqueRenderables.size(); ++iRenderable )
			RenderingContext::Get()->m_aBatchedVisibleOpaqueRenderables[iRenderable]->DisplayBounds();
		
		for( unsigned int iRenderable = 0; iRenderable < RenderingContext::Get()->m_aVisibleAlphaRenderables.size(); ++iRenderable )
			RenderingContext::Get()->m_aVisibleAlphaRenderables[iRenderable]->DisplayBounds();
	}

	//---------------------------------------------------------------------------------
	// Retrieve STD render targets that have been set for the rendering of this context
	//---------------------------------------------------------------------------------
	RenderTarget::Ptr	backBufferLDR = Renderer::Get().m_targetCache.GetPrimaryColourTarget();
	ZBuffer::Ptr		zBuffer = Renderer::Get().m_targetCache.GetDepthTarget();

	ntAssert_p( backBufferLDR, ("Must have had a valid backbuffer set before RenderContext()") );
	ntAssert_p( zBuffer, ("Must have had a valid zbuffer set before RenderContext()") );


	//---------------------------------------------------------------------------------
	// Generate the shadow map cast by the key light
	//---------------------------------------------------------------------------------
	CGatso::Start( "RenderContext::GenerateShadowMap" );
	
	if( CShadowSystemController::Get().IsShadowMapActive() )
	{
#ifdef _SPEEDTREE
		if (SpeedTreeManager::Exists())
		{
			SpeedTreeManager::Get().UpdateForLight();
		}
#endif	// _SPEEDTREE
		GenerateShadowMap();
	}
	
	CGatso::Stop( "RenderContext::GenerateShadowMap" );

#ifdef _SPEEDTREE
	if (SpeedTreeManager::Exists())
	{
		SpeedTreeManager::Get().UpdateForCamera();
	}
#endif	// _SPEEDTREE

	//---------------------------------------------------------------------------------
	// Retrieve a new HDR buffer 
	//---------------------------------------------------------------------------------
	GFXFORMAT HDRFormat = Renderer::Get().GetHDRFormat();
	
	bool bHDRAlphaBlend = false;
	if (HardwareCapabilities::Get().IsValidAlphaBlendRTFormat( HDRFormat ))
		bHDRAlphaBlend = true;

	RenderTarget::CreationStruct createParamsHDR(	backBufferLDR->GetWidth(),
													backBufferLDR->GetHeight(),
													ConvertGFXFORMATToD3DFORMAT(HDRFormat), false );
	
	createParamsHDR.bCacheable = backBufferLDR->m_Platform.IsCacheable();
	RenderTarget::Ptr backBufferHDR = SurfaceManager::Get().CreateRenderTarget( createParamsHDR );
	
	// clear the alpha to 127, which is neutral for depth of field.	
	Renderer::Get().m_targetCache.SetColourAndDepthTargets( backBufferHDR, zBuffer );
	Renderer::Get().Clear( D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0x80000000, 1.0f, 0 );

	//---------------------------------------------------------------------------------
	// prepass Z (needed for the multi-res shadowmapper and should help fillrate nicely as a bonus...)
	//---------------------------------------------------------------------------------
	CGatso::Start( "RenderContext::Zprepass" );

	Renderer::Get().SetBlendMode( GFX_BLENDMODE_DISABLED );
	Renderer::Get().SetZBufferMode( GFX_ZMODE_LESSEQUAL );
 
	if( !CRendererSettings::bEnableBackfaceCulling )
		Renderer::Get().SetCullMode( GFX_CULLMODE_NONE );

	// render all the opaque meshes
	for( unsigned int iRenderable = 0; iRenderable < RenderingContext::Get()->m_aVisibleOpaqueRenderables.size(); ++iRenderable )
		RenderingContext::Get()->m_aVisibleOpaqueRenderables[iRenderable]->RenderDepth();

	Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );
	Renderer::Get().SetZBufferMode( GFX_ZMODE_LESSEQUAL_READONLY );

	CGatso::Stop( "RenderContext::Zprepass" );

	//---------------------------------------------------------------------------------
	// Generate our full screen shadow texture
	//---------------------------------------------------------------------------------
	CGatso::Start( "RenderContext::RecieveShadows" );

	if( CShadowSystemController::Get().IsShadowMapActive() )
	{
		// set up the viewport (use existing z-buffer)
		m_stencilShadowSystem.AllocateTextures( backBufferHDR->GetWidth(),
												backBufferHDR->GetHeight(),
												backBufferHDR->m_Platform.IsCacheable() );
		
		RenderTarget::Ptr stencilResult = m_stencilShadowSystem.m_pStencilResult;
		
		Renderer::Get().m_targetCache.SetColourAndDepthTargets( stencilResult, zBuffer );
		Renderer::Get().Clear( D3DCLEAR_TARGET, 0, 1.0f, 0 );

		// recieve shadows into our screen aligned shadow buffer
		RecieveShadows( fPlanePercents );

		/* stencil shadows not used at this time
		// render the stencil shadows
		GetShadows().RenderStencilCasters();
		m_stencilShadowSystem.RenderStencilToColour();
		*/

		m_stencilShadowSystem.FinaliseStencilMap();

		#ifdef _DEBUG_SHADOW
			m_stencilShadowSystem.DebugRender();
		#endif // _DEBUG_SHADOW
	}

	CGatso::Stop( "RenderContext::RecieveShadows" );

	//---------------------------------------------------------------------------------
	// Render opaques into the HDR buffer
	//---------------------------------------------------------------------------------
	CGatso::Start( "RenderContext::RenderOpaque" );

	Renderer::Get().m_targetCache.SetColourAndDepthTargets( backBufferHDR, zBuffer );
	
	if (CRendererSettings::bShowWireframe)
		Renderer::Get().SetFillMode( GFX_FILL_WIREFRAME );
	
	for( unsigned int iRenderable = 0; iRenderable < RenderingContext::Get()->m_aVisibleOpaqueRenderables.size(); ++iRenderable )
		RenderingContext::Get()->m_aVisibleOpaqueRenderables[iRenderable]->RenderMaterial();

	if (CRendererSettings::bShowWireframe)
		Renderer::Get().SetFillMode( GFX_FILL_SOLID );

	if( !CRendererSettings::bEnableBackfaceCulling )
		Renderer::Get().SetCullMode( GFX_CULLMODE_NORMAL );

	Renderer::Get().SetZBufferMode( GFX_ZMODE_NORMAL );

	CGatso::Stop( "RenderContext::RenderOpaque" );

	// we're done with the full screen shadow texture
	if( CShadowSystemController::Get().IsShadowMapActive() )
		m_stencilShadowSystem.DeallocateTextures();

	DUMP_FRAMEBUFFER( "A_PostOpaque", backBufferHDR );

	//---------------------------------------------------------------------------------
	// our sky box / atmospherics
	//---------------------------------------------------------------------------------
	RenderSky();

	//---------------------------------------------------------------------------------
	// render HDR alpha blended primitives
	//---------------------------------------------------------------------------------
	RenderHDRAlpha(bHDRAlphaBlend);

	DUMP_FRAMEBUFFER( "B_PostHDRAlpha", backBufferHDR );

	//---------------------------------------------------------------------------------
	// sample the HDR back buffer for exposure
	//---------------------------------------------------------------------------------
	CGatso::Start( "RenderContext::SampleExposure" );
	
	if (CRendererSettings::bEnableExposure)
	{
		if (CRendererSettings::bUseGPUExposure)
			m_EXPControllerGPU.SampleCurrBackBuffer( backBufferHDR );
		else
			m_EXPControllerCPU.SampleCurrBackBuffer( backBufferHDR );
	}

	CGatso::Stop( "RenderContext::SampleExposure" );

	//---------------------------------------------------------------------------------
	// Generate bloom and ghosting texture from HDR target
	//---------------------------------------------------------------------------------
	CGatso::Start( "RenderContext::LensArtefacts" );
	
	RenderTarget::Ptr lensArtefacts;
	if (CRendererSettings::bUseGPUExposure)
		lensArtefacts = m_lensArtifacts.Generate( viewport.GetKeyLuminance(), backBufferHDR );
	else
		lensArtefacts = m_lensArtifacts.Generate( viewport.m_fExposureLastVal , backBufferHDR );

	CGatso::Stop( "RenderContext::LensArtefacts" );

	//---------------------------------------------------------------------------------
	// render full screen post processing on the HDR target
	//---------------------------------------------------------------------------------
	CGatso::Start( "RenderContext::DepthOfField" );

	m_depthOfField.ApplyDepthOfField( backBufferHDR );

	CGatso::Stop( "RenderContext::DepthOfField" );

	DUMP_FRAMEBUFFER( "C_PostDOFEffects", backBufferHDR );

	//---------------------------------------------------------------------------------
	// prepare for LDR back buffer generation
	//---------------------------------------------------------------------------------
	Renderer::Get().m_targetCache.SetColourTarget( backBufferLDR );

	// disable SRG write for the conversion from HDR to LDR
	GetD3DDevice()->SetRenderState( D3DRS_SRGBWRITEENABLE, FALSE );

	//---------------------------------------------------------------------------------
	// now we move from HDR space to our linear back buffer.
	//---------------------------------------------------------------------------------
	CGatso::Start( "RenderContext::ConvertToLinear" );

	if (CRendererSettings::bUseGPUExposure)
		m_HDRResolver.ResolveHDRtoLinear( backBufferHDR, lensArtefacts->GetTexture(), viewport.GetKeyLuminance() );
	else
		m_HDRResolver.ResolveHDRtoLinear( backBufferHDR, lensArtefacts->GetTexture(), viewport.m_fExposureLastVal );
	
	CGatso::Stop( "RenderContext::ConvertToLinear" );

	DUMP_FRAMEBUFFER( "D_PostResolve", backBufferLDR );

	//---------------------------------------------------------------------------------
	// Render LDR effects
	//---------------------------------------------------------------------------------
	Renderer::Get().m_targetCache.SetColourAndDepthTargets( backBufferLDR, zBuffer );
		
	RenderLDRAlpha( bHDRAlphaBlend );

	DUMP_FRAMEBUFFER( "E_PostLDREffects", backBufferLDR );

	// free all our other temp textures
	SurfaceManager::Get().ReleaseRenderTarget( backBufferHDR );
	SurfaceManager::Get().ReleaseRenderTarget( lensArtefacts );

	// Debug display our depth of field effect here
	m_depthOfField.DebugDisplayDepthOfField();

	//---------------------------------------------------------------------------------
	// Render debug prims
	//---------------------------------------------------------------------------------
	CGatso::Start( "RenderContext::DebugDraw3D" );
	g_VisualDebug->Draw3D();
	CGatso::Stop( "RenderContext::DebugDraw3D" );

	//---------------------------------------------------------------------------------
	// render levels graphs if required
	//---------------------------------------------------------------------------------
#ifndef _RELEASE

	// update the levels graph
	if( CRendererSettings::bShowLevelsGraph )
	{
		// update the levels graph, use 50 levels
		m_levelsGraph.Update( 50, backBufferLDR->GetSurface() );

		// render a stonking great big graph
		m_levelsGraph.RenderGraph( 
			CPoint( 
				backBufferLDR->GetWidth()/2.0f, 
				backBufferLDR->GetHeight()*2.0f/3.0f, 
				0.0f
			), 
			CPoint( 
				backBufferLDR->GetWidth()-10.0f, 
				backBufferLDR->GetHeight()-10.0f, 
				0.0f
			)
		);
	}

#endif

	DUMP_FRAMEBUFFER( "F_ViewFinal", backBufferLDR );
}


