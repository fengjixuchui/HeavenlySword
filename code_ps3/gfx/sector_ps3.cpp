//--------------------------------------------------
//!
//!	\file sector_ps3.cpp
//!	PS3 portion of the sector renderer
//!
//--------------------------------------------------

#include "gfx/sector.h"
#include "gfx/levelofdetail.h"
#include "gfx/exposure.h"
#include "gfx/renderer.h"
#include "gfx/rendercontext.h"
#include "gfx/renderable.h"
#include "gfx/hardwarecaps.h"
#include "gfx/surfacemanager.h"
#include "gfx/graphicsdevice.h"
#include "gfx/pictureinpicture.h"
#include "gfx/batchrender_ppu_spu.h"
#include "gfx/renderer_ppu_spu.h"
#include "game/entitymanager.h"
#include "gfx/meshinstance.h"
#include "movies/movieplayer.h"
#include "game/renderablecomponent.h"

#include "core/gatso.h"
#include "core/visualdebugger.h"
#include "input/inputhardware.h"
//#include "game/command.h"
#include "game/keybinder.h"

#include "heresy/heresy_capi.h"

#ifdef _SPEEDTREE
	#include "speedtree/SpeedTreeManager_ps3.h"
#endif

#include "army/armymanager.h"		//! TODO Deano gfx needs army!

#define DUMP_FRAMEBUFFER( name, framebuffer )			\
	if (bDumpView)										\
	{													\
		static char fileName[32];						\
		sprintf( fileName, "%s_%d", name, iViewID );	\
		framebuffer->SaveToDisk( fileName );			\
	}

#ifdef _PROFILING
#define START_COUNTER()					{ tempCounter = Renderer::Get().m_renderCount;}
#define STOP_COUNTER( counter )			{ counter += (Renderer::Get().m_renderCount - tempCounter); }
#define SETPS_DISABLE_IF_TRUE( var )	{ if (var) Renderer::Get().m_Platform.SetProfilePSSuspension(true); }
#define SETPS_ENABLE_IF_TRUE( var )		{ if (var) Renderer::Get().m_Platform.SetProfilePSSuspension(false); }
#define START_GPU_REPORT( ReportID, ViewID ) { 	Renderer::Get().m_Platform.m_pGPUReport->StartReport( ReportID, iViewID );	}		
#define STOP_GPU_REPORT( ReportID, ViewID ) { 	Renderer::Get().m_Platform.m_pGPUReport->CloseReport( ReportID, iViewID );	}	
#else
#define START_COUNTER()
#define STOP_COUNTER( counter )
#define SETPS_DISABLE_IF_TRUE( var )
#define SETPS_ENABLE_IF_TRUE( var )
#define START_GPU_REPORT( ReportID, ViewID )	
#define STOP_GPU_REPORT( ReportID, ViewID ) 
#endif





// register key
void CSector::RegisterKey()
{
	// Register some commands
	CommandBaseInput<const int&>* pToggle = CommandManager::Get().CreateCommand("CSector", this, &CSector::CommandToggleFlags, "CSector Toggle flags");
#ifdef _PROFILING
	KeyBindManager::Get().RegisterKey("rendering", pToggle, "disable shadow receive PS", int(F_PROFILEDISABLE_SHADOWRECIEVEPS), KEYS_PRESSED, KEYC_F2);
	KeyBindManager::Get().RegisterKey("rendering", pToggle, "disable opaque PS", int(F_PROFILEDISABLE_OPAQUERENDERPS), KEYS_PRESSED, KEYC_F3);
#endif
	KeyBindManager::Get().RegisterKey("global", pToggle, "dump view", int(F_DUMPVIEW), KEYS_PRESSED, KEYC_INSERT);
}



COMMAND_RESULT CSector::CommandToggleFlags(const int& kind)
{
	switch(kind)
	{
	case F_PROFILEDISABLE_SHADOWRECIEVEPS:
		{
			m_debugMask.Toggle(F_PROFILEDISABLE_SHADOWRECIEVEPS);
			break;
		}
	case F_PROFILEDISABLE_OPAQUERENDERPS:
		{
			m_debugMask.Toggle(F_PROFILEDISABLE_OPAQUERENDERPS);
			break;
		}
	case F_DUMPVIEW:
		{
			m_debugMask.Set(F_DUMPVIEW,true);
			break;
		}
	default:
		{
			ntAssert_p(false, ("beurk !"));
			break;
		}
	}

	return CR_SUCCESS;
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
	viewport.ResetCounters();

	// need to pre-compute exposure and tone mapping constants
	// read back exposure computed by the GPU
	float exposure =  *((float*)viewport.m_exposureLastVal->m_Platform.GetRenderBuffer()->GetDataAddress());
	// save exposure and tone mapping pixel shader constants 
	RenderingContext::Get()->m_ExposureAndToneMapConsts = CVector( RenderingContext::Get()->m_exposureSettings.m_fKeyValueMapping / exposure,
														1.0f/(	RenderingContext::Get()->m_exposureSettings.m_fLuminanceBurnout*
														RenderingContext::Get()->m_exposureSettings.m_fLuminanceBurnout ), exposure, exposure );

	//g_vExposureReadBack = RenderingContext::Get()->m_ExposureAndToneMapConsts;


#ifdef _PROFILING
	RenderCounter	tempCounter;
	
	//CRendererSettings::bProfileDisable_ShadowRecievePS = m_debugMask[F_PROFILEDISABLE_SHADOWRECIEVEPS];
	//CRendererSettings::bProfileDisable_OpaqueRenderPS = m_debugMask[F_PROFILEDISABLE_OPAQUERENDERPS];

	//if ( CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_F2 ) )
	//	CRendererSettings::bProfileDisable_ShadowRecievePS = !CRendererSettings::bProfileDisable_ShadowRecievePS;
	//if ( CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_F3 ) )
	//	CRendererSettings::bProfileDisable_OpaqueRenderPS = !CRendererSettings::bProfileDisable_OpaqueRenderPS;

	if ( CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_KPAD_PERIOD) )
		CRendererSettings::bEnableBatchRenderer = !CRendererSettings::bEnableBatchRenderer;

#endif
	int		iViewID = viewport.GetDebugID();

	bool	bDumpView = m_debugMask[F_DUMPVIEW] || m_bDoDumpFrame;
	m_debugMask.Set(F_DUMPVIEW,false);

	m_bDoDumpFrame = m_bForceDump;
	m_bForceDump = false;

	//bool	bDumpView = false;
	//if	(
	//	(CInputHardware::Exists()) &&
	//	(CInputHardware::Get().GetKeyboard().IsKeyPressed( KEYC_INSERT ))
	//	)
	//	bDumpView = true;

#ifdef _SPEEDTREE
	// do per viewport update
	if (SpeedTreeManager::Exists()) SpeedTreeManager::Get().PerViewportUpdate();
#endif	// _SPEEDTREE

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

	//---------------------------------------------------------------------------------
	// check for fullscreen video, as should clear lists here if movie is active
	//---------------------------------------------------------------------------------
	if ( MoviePlayer::Get().HasMainViewportMovie() )
	{
		// clear some visible renderables vectors
		RenderingContext::Get()->m_aBatchedShadowCastingRenderables.resize(0);
		RenderingContext::Get()->m_aShadowCastingOpaqueRenderables.resize(0);
		RenderingContext::Get()->m_aBatchedVisibleOpaqueRenderables.resize(0);
		RenderingContext::Get()->m_aVisibleOpaqueRenderables.resize(0);
		RenderingContext::Get()->m_aVisibleAlphaRenderables.resize(0);
	}

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
	// Generate the shadow map cast by the key light
	//---------------------------------------------------------------------------------
	CGatso::Start( "RenderContext::GenerateShadowMap" );
	START_COUNTER();
	START_GPU_REPORT( GPUReport::GPRID_ShadowMapsPass, iViewID )
	
	if( CShadowSystemController::Get().IsShadowMapActive() )
	{
#ifdef _SPEEDTREE
		// do camera update (for billboard)
		if (SpeedTreeManager::Exists())
		{
			SpeedTreeManager::Get().UpdateForLight();
			SpeedTreeManager::Get().ResetRendering();
		}
#endif	// _SPEEDTREE

		GenerateShadowMap();
	}
	
	STOP_GPU_REPORT( GPUReport::GPRID_ShadowMapsPass, iViewID )
	STOP_COUNTER( viewport.m_SMapCount );
	CGatso::Stop( "RenderContext::GenerateShadowMap" );
	STOP_COUNTER( viewport.m_SMapCount );

#ifdef _SPEEDTREE
	// do camera update (for billboard)
	if (SpeedTreeManager::Exists())
	{
		SpeedTreeManager::Get().FinishRendering();
		SpeedTreeManager::Get().UpdateForCamera();
		SpeedTreeManager::Get().ResetRendering();
	}
#endif	// _SPEEDTREE

	//---------------------------------------------------------------------------------
	// Generate our float 32 zbuffer for fast shadow recieves and depth of field effects
	// and a genuine Z buffer in non-MSAA space for HDR and LDR alpha effects.
	//---------------------------------------------------------------------------------
	CGatso::Start( "RenderContext::Zprepass" );
	START_COUNTER();
	START_GPU_REPORT( GPUReport::GPRID_ZPrePass, iViewID );
	//---------------------------------------------------------------------------------
	// DONT MOVE THESE LINES, we need to clear the MSAAed render target BEFORE rendering 
	// the z pre pass to make sure the ZCULL buffer will be reused in the color pass
	//---------------------------------------------------------------------------------
	RenderTarget::Ptr MSAAColour = viewport.GetMSAAColour();
	RenderTarget::Ptr MSAADepth = viewport.GetMSAADepth();
	Renderer::Get().m_targetCache.SetColourAndDepthTargets( MSAAColour, MSAADepth );
	Renderer::Get().Clear( Gc::kColourBufferBit || Gc::kDepthBufferBit, 0, 1.0f, 0 );
	//---------------------------------------------------------------------------------

	RenderTarget::Ptr depthBufferR32F = viewport.GetFloatZBuffer();
	RenderTarget::Ptr nonMSAAdepth = viewport.GetZBuffer();

	Renderer::Get().m_targetCache.SetColourAndDepthTargets( depthBufferR32F, nonMSAAdepth );

	// NOTE! we no longer clear colour buffer here, slight optimisation
	Renderer::Get().FastFloatClear( Gc::kDepthBufferBit, 1.0f, 0 );

	if( !CRendererSettings::bEnableBackfaceCulling )
		Renderer::Get().SetCullMode( GFX_CULLMODE_NONE );

	Renderer::Get().SetZBufferMode( GFX_ZMODE_LESSEQUAL );

#ifdef USE_GPU_PERFORMANCE_MONITOR
	Heresy_PushBuffer* pMainPB = (Heresy_PushBuffer*)&GcKernel::GetContext();
	Heresy_TriggerPerfCounter( pMainPB );
#endif


	// this shader passes the depth through to the colour output as well

	// render opaque batched renderables
	BatchRenderer::Render( (void*)&RenderingContext::Get()->m_aBatchedVisibleOpaqueRenderables, kRendPassPreZ );
	// render opaque non batched renderables
	SpuRenderer::Render( (void*)&RenderingContext::Get()->m_aVisibleOpaqueRenderables, kRendPassPreZ );

#ifdef _SPEEDTREE
	if (SpeedTreeManager::Exists())
	{
		SpeedTreeManager::Get().FinishRendering();
		SpeedTreeManager::Get().ResetRendering();
	}
#endif	// _SPEEDTREE

#ifdef USE_GPU_PERFORMANCE_MONITOR
	Heresy_TriggerPerfCounter( pMainPB );
#endif

	Renderer::Get().SetZBufferMode( GFX_ZMODE_LESSEQUAL_READONLY );
	
	CGatso::Stop( "RenderContext::Zprepass" );
	STOP_GPU_REPORT( GPUReport::GPRID_ZPrePass, iViewID );
	STOP_COUNTER( viewport.m_ZPrepassCount );
	
	DUMP_FRAMEBUFFER( "A_nonMSAAdepth", nonMSAAdepth );

	//---------------------------------------------------------------------------------
	// Generate our full screen shadow texture in NON-MSAA space
	//---------------------------------------------------------------------------------
	CGatso::Start( "RenderContext::RecieveShadows" );
	START_COUNTER();
	START_GPU_REPORT( GPUReport::GPRID_OcclusionGatheringPass, iViewID );

	if( CShadowSystemController::Get().IsShadowMapActive() )
	{
		// we're going to be efficient and reuse our LDR buffer as the stencil result,
		// KNOWING that we're not going to require the shadow buffer in LDR space.
		// if this changes we require a whole new full size buffer.

		SETPS_DISABLE_IF_TRUE(CRendererSettings::bProfileDisable_ShadowRecievePS);

	#ifdef _SPEEDTREE
		if (SpeedTreeManager::Exists())
		{
			SpeedTreeManager::Get().FinishRendering();
			SpeedTreeManager::Get().ResetRendering();
		}
	#endif	// _SPEEDTREE

		// Prepare to receive shadow term
		RecieveShadows( fPlanePercents );
		RenderTarget::Ptr stencilResult = viewport.GetBackBuffer();

		if ( CRendererSettings::bUseFastShadowRecieve )
		{
			// gather occlusion term with a full screen pass
			Renderer::Get().m_targetCache.SetColourTarget( stencilResult );

			m_ShadowGatherer.GatherOcclusionTerm( depthBufferR32F->GetTexture() );
		}
		else
		{
			// gather occlusion term re-rendering the scene in camera space
			Renderer::Get().m_targetCache.SetColourAndDepthTargets( stencilResult, nonMSAAdepth );
			Renderer::Get().SetZBufferMode( GFX_ZMODE_LESSEQUAL_READONLY );
			Renderer::Get().Clear( Gc::kColourBufferBit, 0, 1.0f, 0 );
			
			CShadowSystemController::Get().RenderRecievers();
		}
		
		SETPS_ENABLE_IF_TRUE(CRendererSettings::bProfileDisable_ShadowRecievePS);

		RenderingContext::Get()->m_pStencilTarget = stencilResult->GetTexture();
		//m_stencilShadowSystem.DebugRender( stencilResult );
	}

	STOP_GPU_REPORT( GPUReport::GPRID_OcclusionGatheringPass, iViewID );
	STOP_COUNTER( viewport.m_RecieveShadowCount );
	CGatso::Stop( "RenderContext::RecieveShadows" );

	//---------------------------------------------------------------------------------
	// irradiance cache pass
	//---------------------------------------------------------------------------------
	CGatso::Start( "RenderContext::GenerateIrradianceCache" );
	START_COUNTER();
	START_GPU_REPORT( GPUReport::GPRID_IrradianceCachePass, iViewID );

	// generate (if needed) a new irradiance cube map	
	// retrieve irradiance cube map 
	RenderingContext::Get()->m_pIrradianceCache = viewport.GetIrradianceCache();
	m_IrradianceManager.GenerateIrradianceCubeMap( RenderingContext::Get()->m_pIrradianceCache, RenderingContext::Get()->m_SHMatrices );

/*	for ( unsigned int j = 0; j < 6; j++ )
	{
		for ( unsigned int i = 0; i < 7; i++ )
		{
			GcTextureHandle pTexturePlusOne = RenderingContext::Get()->m_pIrradianceCache->m_Platform.GetTexture()->GetCubeMipLevel( (Gc::TexCubeFace)j, i );
			RenderTarget::Ptr pRendTarget = SurfaceManager::Get().CreateRenderTarget( RenderTarget::CreationStruct( GcRenderBuffer::Create( pTexturePlusOne ) ) );

			char filename[32] = "B_CubeMap__";
			filename[9] = '1' + j;	
			filename[10] = '1' + i;	
			DUMP_FRAMEBUFFER( filename, pRendTarget );
		}
	} */

	STOP_GPU_REPORT( GPUReport::GPRID_IrradianceCachePass, iViewID );
	STOP_COUNTER( viewport.m_RenderOpaqueCount );
	CGatso::Stop( "RenderContext::GenerateIrradianceCache" );


	//---------------------------------------------------------------------------------
	// prepass Z for the opaque pass
	//---------------------------------------------------------------------------------
	Renderer::Get().m_targetCache.SetColourAndDepthTargets( MSAAColour, MSAADepth );

	if (CRendererSettings::bUseZPrepass)
	{
		CGatso::Start( "RenderContext::Zprepass" );
		START_COUNTER();

		if( !CRendererSettings::bEnableBackfaceCulling )
			Renderer::Get().SetCullMode( GFX_CULLMODE_NONE );

		// disable colour writes
		Renderer::Get().SetBlendMode( GFX_BLENDMODE_DISABLED );
		Renderer::Get().SetZBufferMode( GFX_ZMODE_LESSEQUAL );

	#ifdef _SPEEDTREE
		if (SpeedTreeManager::Exists())
		{
			SpeedTreeManager::Get().FinishRendering();
			SpeedTreeManager::Get().ResetRendering();
		}
	#endif	// _SPEEDTREE


		// render depth only
		// render opaque batched renderables
		BatchRenderer::Render( (void*)&RenderingContext::Get()->m_aBatchedVisibleOpaqueRenderables, kRendPassPreZ );
		// render opaque non batched renderables
		SpuRenderer::Render( (void*)&RenderingContext::Get()->m_aVisibleOpaqueRenderables, kRendPassPreZ );

		Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );
		Renderer::Get().SetZBufferMode( GFX_ZMODE_LESSEQUAL_READONLY );
		
		CGatso::Stop( "RenderContext::Zprepass" );
		STOP_COUNTER( viewport.m_ZPrepassCount );
	}

	//---------------------------------------------------------------------------------
	// Render opaques (this is in MSAA space if we have AA)
	//---------------------------------------------------------------------------------
	CGatso::Start( "RenderContext::RenderOpaque" );
	START_COUNTER();
	START_GPU_REPORT( GPUReport::GPRID_OpaquePass, iViewID );
	SETPS_DISABLE_IF_TRUE(CRendererSettings::bProfileDisable_OpaqueRenderPS);

	if (CRendererSettings::bShowWireframe)
		Renderer::Get().SetFillMode( GFX_FILL_WIREFRAME );

#ifdef _SPEEDTREE
	// do camera update (for billboard)
	if (SpeedTreeManager::Exists())
	{
		SpeedTreeManager::Get().FinishRendering();
		SpeedTreeManager::Get().ResetRendering();
	}
#endif	// _SPEEDTREE

	if( CRendererSettings::bShowWireframe )
	{
		Renderer::Get().Clear( Gc::kColourBufferBit, 0x0, 1.0f, 0 );
	}

	BatchRenderer::Render( (void*)&RenderingContext::Get()->m_aBatchedVisibleOpaqueRenderables, kRendPassColorOpaque );
	SpuRenderer::Render( (void*)&RenderingContext::Get()->m_aVisibleOpaqueRenderables, kRendPassColorOpaque );


#ifdef _SPEEDTREE
	// do camera update (for billboard)
	if (SpeedTreeManager::Exists())
	{
		SpeedTreeManager::Get().FinishRendering();
	}
#endif	// _SPEEDTREE

	if (CRendererSettings::bShowWireframe)
		Renderer::Get().SetFillMode( GFX_FILL_SOLID );

	SETPS_ENABLE_IF_TRUE(CRendererSettings::bProfileDisable_OpaqueRenderPS);

	if( !CRendererSettings::bEnableBackfaceCulling )
		Renderer::Get().SetCullMode( GFX_CULLMODE_NORMAL );

	Renderer::Get().SetZBufferMode( GFX_ZMODE_NORMAL );

	STOP_GPU_REPORT( GPUReport::GPRID_OpaquePass, iViewID );
	CGatso::Stop( "RenderContext::RenderOpaque" );
	STOP_COUNTER( viewport.m_RenderOpaqueCount );

	//---------------------------------------------------------------------------------
	// our sky box / atmospherics
	//---------------------------------------------------------------------------------
	

	// skybox is now officially dead, but we can use it for debug levels
	if (CRendererSettings::bEnableSky || CRendererSettings::bShowWireframe)
	{
		START_GPU_REPORT( GPUReport::GPRID_SkyPass, iViewID );
		RenderSky();
		STOP_GPU_REPORT( GPUReport::GPRID_SkyPass, iViewID );
	}

	DUMP_FRAMEBUFFER( "B_PostOpaque", MSAAColour );
	DUMP_FRAMEBUFFER( "B_MSAAZBuffer", MSAADepth );


	//---------------------------------------------------------------------------------
	// Now convert to a float16 render target so we can perform HDR alpha blending
	//---------------------------------------------------------------------------------
	START_GPU_REPORT( GPUReport::GPRID_NAO32ToHDR, iViewID );
	RenderTarget::Ptr HDRf16 = viewport.GetFloatColour();

	// as we're now in non-MSAA space, switch to that version of the zbuffer
	Renderer::Get().m_targetCache.SetColourAndDepthTargets( HDRf16, nonMSAAdepth );

	// now we need to convert from NAO32 space to float16 space
	m_NAO32converter.Convert( MSAAColour->GetTexture() );

	STOP_GPU_REPORT( GPUReport::GPRID_NAO32ToHDR, iViewID );
	DUMP_FRAMEBUFFER( "B_PostOpaque_float16", HDRf16 );


	//---------------------------------------------------------------------------------
	// render HDR alpha blended primitives
	//---------------------------------------------------------------------------------
	START_COUNTER();
	START_GPU_REPORT( GPUReport::GPRID_HDRAlphaPass, iViewID );
	RenderHDRAlpha(true);
	STOP_GPU_REPORT( GPUReport::GPRID_HDRAlphaPass, iViewID );
	STOP_COUNTER( viewport.m_RenderAlphaCount );

	DUMP_FRAMEBUFFER( "C_PostHDRAlpha", HDRf16 );

	//---------------------------------------------------------------------------------
	// sample the HDR back buffer for exposure
	//---------------------------------------------------------------------------------
	CGatso::Start( "RenderContext::SampleExposure" );
	START_GPU_REPORT( GPUReport::GPRID_ExposurePass, iViewID );
	
	if (CRendererSettings::bEnableExposure && (CRendererSettings::iDebugLayer == 0) )
		m_EXPController.SampleCurrBackBuffer( HDRf16 );

	STOP_GPU_REPORT( GPUReport::GPRID_ExposurePass, iViewID );
	CGatso::Stop( "RenderContext::SampleExposure" );

	//---------------------------------------------------------------------------------
	// Generate bloom and ghosting texture from HDR target
	//---------------------------------------------------------------------------------
	CGatso::Start( "RenderContext::LensArtefacts" );
	START_GPU_REPORT( GPUReport::GPRID_LensEffectPass, iViewID );
	
	RenderTarget::Ptr lensArtefacts = m_lensArtifacts.Generate( viewport.GetKeyLuminance(), HDRf16->GetTexture() );

	STOP_GPU_REPORT( GPUReport::GPRID_LensEffectPass, iViewID );
	CGatso::Stop( "RenderContext::LensArtefacts" );

/*
	//---------------------------------------------------------------------------------
	// render full screen post processing on the HDR target
	//---------------------------------------------------------------------------------
	CGatso::Start( "RenderContext::DepthOfField" );

	m_depthOfField.ApplyDepthOfField( HDRf16, depthBufferR32F, MSAAColour );

	CGatso::Stop( "RenderContext::DepthOfField" );
*/

	//---------------------------------------------------------------------------------
	// Dump HDR buffer if required
	//---------------------------------------------------------------------------------
	//DUMP_FRAMEBUFFER( "C2_PostDOFEffects", HDRf16 );

	//---------------------------------------------------------------------------------
	// now we move from HDR space to our linear back buffer.
	// If we're rendering into a multisampled or supersampled render target let's resolve it
	//---------------------------------------------------------------------------------
	CGatso::Start( "RenderContext::ConvertToLinear" );
	START_GPU_REPORT( GPUReport::GPRID_ToneMappingPass, iViewID );

	RenderTarget::Ptr nonMSAAcolour = viewport.GetBackBuffer();
	Renderer::Get().m_targetCache.SetColourAndDepthTargets( nonMSAAcolour, nonMSAAdepth );
	m_HDRResolver.ResolveHDRtoLinear( HDRf16, lensArtefacts->GetTexture(), viewport.GetKeyLuminance() );

	STOP_GPU_REPORT( GPUReport::GPRID_ToneMappingPass, iViewID );
	CGatso::Stop( "RenderContext::ConvertToLinear" );

	DUMP_FRAMEBUFFER( "D_PostResolve", nonMSAAcolour );

	//---------------------------------------------------------------------------------
	// render full screen post processing on the HDR target
	//---------------------------------------------------------------------------------
	CGatso::Start( "RenderContext::DepthOfField" );
	START_GPU_REPORT( GPUReport::GPRID_DOFPass, iViewID );

	m_depthOfField.ApplyDepthOfField( nonMSAAcolour, depthBufferR32F, MSAAColour );
	Renderer::Get().m_targetCache.SetColourAndDepthTargets( nonMSAAcolour, nonMSAAdepth );

	STOP_GPU_REPORT( GPUReport::GPRID_DOFPass, iViewID );
	CGatso::Stop( "RenderContext::DepthOfField" );

	//---------------------------------------------------------------------------------
	// Render LDR effects
	//---------------------------------------------------------------------------------
	START_COUNTER();
	START_GPU_REPORT( GPUReport::GPRID_LDRAlphaPass, iViewID );
	RenderLDRAlpha( true );
	STOP_GPU_REPORT( GPUReport::GPRID_LDRAlphaPass, iViewID );
	STOP_COUNTER( viewport.m_RenderAlphaCount );

	DUMP_FRAMEBUFFER( "E_PostLDREffects", nonMSAAcolour );
	
	// free all our other temp textures
	SurfaceManager::Get().ReleaseRenderTarget( lensArtefacts );
	
	// Debug display our depth of field effect here
	m_depthOfField.DebugDisplayDepthOfField();
 
	//---------------------------------------------------------------------------------
	// Radial motion blur
	//---------------------------------------------------------------------------------
	START_GPU_REPORT( GPUReport::GPRID_RadialBlurPass, iViewID );
	m_radialMotionBlur.Render(nonMSAAcolour);
	STOP_GPU_REPORT( GPUReport::GPRID_RadialBlurPass, iViewID );

	//---------------------------------------------------------------------------------
	// Render debug prims
	//---------------------------------------------------------------------------------
#ifndef _GOLD_MASTER
	CGatso::Start( "RenderContext::DebugDraw3D" );
	g_VisualDebug->Draw3D();
	CGatso::Stop( "RenderContext::DebugDraw3D" );
#endif

	DUMP_FRAMEBUFFER( "F_ViewFinal", nonMSAAcolour );
	
	viewport.FinaliseCounters();
}
