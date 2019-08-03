//--------------------------------------------------
//!
//!	\file pictureinpicture.cpp
//!	objects managing final back buffer compositing
//! of multiple viewports.
//!
//--------------------------------------------------

#include "army/armymanager.h"
#include "gfx/camera.h"
#include "gfx/pictureinpicture.h"
#include "gfx/shader.h"
#include "gfx/renderer.h"
#include "gfx/surfacemanager.h"
#include "gfx/sector.h"
#include "gfx/display.h"
#include "gfx/graphicsdevice.h"
#include "gfx/lightingtemplates.h"
#include "gfx/texturemanager.h"
#include "core/visualdebugger.h"
#include "core/profiling.h"
#include "core/gatso.h"
#include "input/inputhardware.h"
#include "game/capturesystem.h"
#include "gfx/batchrender_ppu_spu.h"
#include "effect/screensprite.h"

#include "gui/guimanager.h"

#include <cell/gcm.h>

//-----------------------------------------------------
//!
//! PIPView::PIPView
//! create a dynamic VB for rendering with
//!
//-----------------------------------------------------
PIPView::PIPView() :
	m_pCamera(0),
	m_bActive(false),
	m_bAllocated(false),
	m_iPriority(0),
	m_fWidth(1), m_fHeight(1),
	m_fX(0), m_fY(0),
	m_iFadeCol( NTCOLOUR_ARGB(0xff,0,0,0) ),
	m_fFadeFraction(0.0f)
{
	GcStreamField	simpleQuadDesc[] = 
	{
		GcStreamField( FwHashedString( "IN.position" ), 0, Gc::kFloat, 2 ),	// 2 * 4 bytes
		GcStreamField( FwHashedString( "IN.texcoord" ), 8, Gc::kFloat, 2 ),	// 2 * 4 bytes
	};

	m_hFinalQuad = RendererPlatform::CreateVertexStream(	4, sizeof( float ) * 4, 2,
															simpleQuadDesc, Gc::kScratchBuffer );
}


//-----------------------------------------------------
//!
//! PIPView::SetupView
//! Allocate VRAM and set as render targets
//!
//-----------------------------------------------------
void PIPView::SetupView(uint32_t iBBWidth, uint32_t iBBHeight)
{
	ntAssert_p( !m_bAllocated, ("PIPView already setup.") );
	m_bAllocated = true;

	// we have to make sure these dimensions fit within the original 
	// VRAM allocation
	float fWidth = clamp(m_fWidth,0.0f,1.0f);
	float fHeight = clamp(m_fHeight,0.0f,1.0f);

#ifdef _PROFILING
	if (CRendererSettings::bProfileLowFillrate)
	{
		fWidth *= 0.01f;
		fHeight *= 0.01f;
	}
	else if (CRendererSettings::bUseLowRezMode)
	{
		fWidth *= 0.5f;
		fHeight *= 0.5f;
	}
#endif

	uint32_t iWidth = (uint32_t)(fWidth * iBBWidth);
	uint32_t iHeight = (uint32_t)(fHeight * iBBHeight);

	// create buffers from parent manager's VRAM
	//----------------------------------------------------------------------

	// LDR back buffer
	m_pBackBuffer = Renderer::Get().m_pPIPManager->GetAndLockSharedRB(
		PIPManager::BACKBUFFER_LDR,
		iWidth, iHeight, GF_ARGB8 );
	
	// LDR depth buffer
	m_pZBuffer = Renderer::Get().m_pPIPManager->GetAndLockSharedRB(
		PIPManager::DEPTHBUFFER_LDR,
		iWidth, iHeight, GF_D24S8 );

	// float depth buffer
	m_pFloatZBuffer = Renderer::Get().m_pPIPManager->GetAndLockSharedRB(
		PIPManager::DEPTHBUFFER_FLOAT,
		iWidth, iHeight, GF_R32F );

	// float HDR buffer
	m_pFloatColour = Renderer::Get().m_pPIPManager->GetAndLockSharedRB(
		PIPManager::BACKBUFFER_FLOAT,
		iWidth, iHeight, GF_ABGR16F );

	// HDR back and depth buffers
	uint32_t iHDRWidth = (CRendererSettings::GetAAMode() == AAM_SUPERSAMPLE_4X) ? (iWidth<<1) : iWidth;
	uint32_t iHDRHeight = (CRendererSettings::GetAAMode() == AAM_SUPERSAMPLE_4X) ? (iHeight<<1) : iHeight;

	m_pHDRBackBuffer = Renderer::Get().m_pPIPManager->GetAndLockSharedRB(
		PIPManager::BACKBUFFER_HDR,
		iHDRWidth, iHDRHeight, GF_ARGB8,
		(CRendererSettings::GetAAMode() == AAM_MULTISAMPLE_4X) ? GAA_MULTISAMPLE_4X : GAA_MULTISAMPLE_NONE );

	m_pHDRZBuffer = Renderer::Get().m_pPIPManager->GetAndLockSharedRB(
		PIPManager::DEPTHBUFFER_HDR,
		iHDRWidth, iHDRHeight, GF_D24S8,
		(CRendererSettings::GetAAMode() == AAM_MULTISAMPLE_4X) ? GAA_MULTISAMPLE_4X : GAA_MULTISAMPLE_NONE );

	// did not used a for loop to cause if someone'd change the enumerated cube map faces order this code would break (Marco)
	uint32_t irrCacheSize = Renderer::Get().m_pPIPManager->GetIrradCacheSize();
	m_pIrradianceCache = Renderer::Get().m_pPIPManager->GetAndLockSharedTexture(
		PIPManager::IRRADCACHE_HDR,irrCacheSize, irrCacheSize, GF_ABGR16F );

	Renderer::Get().m_targetCache.SetColourAndDepthTargets( m_pBackBuffer, m_pZBuffer );
}

#ifdef _PROFILING

//-----------------------------------------------------
//!
//!	Reset our drawing counters
//!
//-----------------------------------------------------
void PIPView::ResetCounters()
{
	m_startCount = Renderer::Get().m_renderCount;
	m_SMapCount.Reset();
	m_ZPrepassCount.Reset();
	m_RecieveShadowCount.Reset();
	m_RenderOpaqueCount.Reset();
	m_RenderAlphaCount.Reset();
	m_MiscCount.Reset();
	m_TotalCount.Reset();
}

//-----------------------------------------------------
//!
//!	Finalise our drawing counters
//!
//-----------------------------------------------------
void PIPView::FinaliseCounters()
{
	RenderCounter accounted;
	accounted += m_SMapCount;
	accounted += m_ZPrepassCount;
	accounted += m_RecieveShadowCount;
	accounted += m_RenderOpaqueCount;
	accounted += m_RenderAlphaCount;

	m_TotalCount = Renderer::Get().m_renderCount - m_startCount;
	m_MiscCount = m_TotalCount - accounted;
}

//-----------------------------------------------------
//!
//!	Debug display our drawing parameters
//!
//-----------------------------------------------------
void PIPView::PrintCounters(float fStartX, float fStartY)
{
	uint32_t iNormalCol = NTCOLOUR_ARGB( 0xff, 0, 0xe0, 0xe0 );
	uint32_t iArtistCol = NTCOLOUR_ARGB( 0xff, 0, 0x40, 0xf0 );
	uint32_t iRed = NTCOLOUR_ARGB( 0xff, 0xff, 0x00, 0x00 );
	uint32_t iYellow = NTCOLOUR_ARGB( 0xff, 0xff, 0xff, 0x00 );

	float fGPUReport[ GPUReport::GPRID_UnknownPass ];

#ifdef _PROFILING
	if ( Renderer::Get().m_Platform.m_pGPUReport )
	{
		for (unsigned int i = 0; i < GPUReport::GPRID_UnknownPass; i++)
			fGPUReport[i] = Renderer::Get().m_Platform.m_pGPUReport->GetReportFrameRelativeTime( (GPUReport::GPUR_ID)i );
	}
#endif

	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, "Render ShadowMap: Draw:%d Verts:%d Polys:%d",
		m_SMapCount.m_iDraws, m_SMapCount.m_iVerts, m_SMapCount.m_iPolys); fStartY += 12.0f;
	
	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, "Z Prepass:        Draw:%d Verts:%d Polys:%d",
		m_ZPrepassCount.m_iDraws, m_ZPrepassCount.m_iVerts, m_ZPrepassCount.m_iPolys ); fStartY += 12.0f;

	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, "Recieve Shadows:  Draw:%d Verts:%d Polys:%d",
		m_RecieveShadowCount.m_iDraws, m_RecieveShadowCount.m_iVerts, m_RecieveShadowCount.m_iPolys ); fStartY += 12.0f;

	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, "Render Opaque:    Draw:%d Verts:%d Polys:%d",
		m_RenderOpaqueCount.m_iDraws, m_RenderOpaqueCount.m_iVerts, m_RenderOpaqueCount.m_iPolys); fStartY += 12.0f;

	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, "Render Alpha:     Draw:%d Verts:%d Polys:%d",
		m_RenderAlphaCount.m_iDraws, m_RenderAlphaCount.m_iVerts, m_RenderAlphaCount.m_iPolys ); fStartY += 12.0f;

	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, "Misc Draw Calls:  Draw:%d Verts:%d Polys:%d",
		m_MiscCount.m_iDraws, m_MiscCount.m_iVerts, m_MiscCount.m_iPolys ); fStartY += 12.0f;

	g_VisualDebug->Printf2D( fStartX, fStartY, iRed, 0, "Total Draw Calls: Draw:%d Verts:%d Polys:%d",
		m_TotalCount.m_iDraws, m_TotalCount.m_iVerts, m_TotalCount.m_iPolys ); fStartY += 12.0f;

	fStartY += 3*12.0f;
	g_VisualDebug->Printf2D( fStartX, fStartY, iYellow, 0, "GPU reports are relative to a 30 hz frame"); fStartY += 12.0f;

	g_VisualDebug->Printf2D( fStartX, fStartY, iArtistCol, 0, "Shadow Maps Pass:                        %.2f%%", 
							 fGPUReport[ GPUReport::GPRID_ShadowMapsPass ]  ); fStartY += 12.0f;
	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, "ZPre Pass:                               %.2f%%", 
							 fGPUReport[ GPUReport::GPRID_ZPrePass ]  ); fStartY += 12.0f;
	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, "Occlusion Gathering Pass:                %.2f%%", 
							 fGPUReport[ GPUReport::GPRID_OcclusionGatheringPass ]  ); fStartY += 12.0f;
	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, "Irradiance Cache Pass:                   %.2f%%", 
							 fGPUReport[ GPUReport::GPRID_IrradianceCachePass ]  ); fStartY += 12.0f;
	g_VisualDebug->Printf2D( fStartX, fStartY, iArtistCol, 0, "Opaque Renderables Pass:                 %.2f%%", 
							 fGPUReport[ GPUReport::GPRID_OpaquePass ]  ); fStartY += 12.0f;
	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, "Sky Pass:                                %.2f%%", 
							 fGPUReport[ GPUReport::GPRID_SkyPass ]  ); fStartY += 12.0f;
	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, "NAO32 To FP64 Pass:                      %.2f%%", 
							 fGPUReport[ GPUReport::GPRID_NAO32ToHDR ]  ); fStartY += 12.0f;
	g_VisualDebug->Printf2D( fStartX, fStartY, iArtistCol, 0, "HDR Alpha Pass:                          %.2f%%", 
							 fGPUReport[ GPUReport::GPRID_HDRAlphaPass ]  ); fStartY += 12.0f;
	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, "Exposure Pass:                           %.2f%%", 
							 fGPUReport[ GPUReport::GPRID_ExposurePass ]  ); fStartY += 12.0f;
	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, "Lens Effact Pass:                        %.2f%%", 
							 fGPUReport[ GPUReport::GPRID_LensEffectPass ]  ); fStartY += 12.0f;
	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, "ToneMap + Lens + ColorMat + GammaCorr:   %.2f%%", 
							 fGPUReport[ GPUReport::GPRID_ToneMappingPass ]  ); fStartY += 12.0f;
	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, "Depth Of Field Pass:                     %.2f%%", 
							 fGPUReport[ GPUReport::GPRID_DOFPass ]  ); fStartY += 12.0f;
	g_VisualDebug->Printf2D( fStartX, fStartY, iArtistCol, 0, "LDR Alpha Pass:                          %.2f%%", 
							 fGPUReport[ GPUReport::GPRID_LDRAlphaPass ]  ); fStartY += 12.0f;
	g_VisualDebug->Printf2D( fStartX, fStartY, iNormalCol, 0, "Radial Blur Pass:                        %.2f%%", 
							 fGPUReport[ GPUReport::GPRID_RadialBlurPass ]  ); fStartY += 12.0f;


}

#endif // _PROFILING

//-----------------------------------------------------
//!
//! PIPView::SetActive
//! Do any other persistant allocations or init we require
//!
//-----------------------------------------------------
void PIPView::SetActive( bool bActive )
{
	if (m_bActive == bActive)
		return;

	if (m_bActive)
	{
		// we're deactivating, free stuff
		m_bActive = false;
		//SurfaceManager::Get().ReleaseRenderTarget(m_exposureLastVal);
		//SurfaceManager::Get().ReleaseRenderTarget(m_exposureLastValTemp);
		SurfaceManager::Get().ReleaseTexture(m_exposureOveride);

		NT_FREE_CHUNK(Mem::MC_RSX_MAIN_USER, (uintptr_t)m_pExposureXDRTextures);
	}
	else
	{
		// we're activating for the first time, alloc stuff
		m_bActive = true;

		// allocate and clear some XDR memory to store our exposure textures
		m_pExposureXDRTextures = (void*)NT_MEMALIGN_CHUNK( Mem::MC_RSX_MAIN_USER, Gc::kTextureAlignment * 2, Gc::kTextureAlignment );
		memset( m_pExposureXDRTextures, 0x0, Gc::kTextureAlignment * 2 );
		
		RenderTarget::CreationStruct exposureStruct =
		RenderTarget::CreationStruct( m_pExposureXDRTextures, Gc::kTextureAlignment, 1, 1, 	GF_R32F, GAA_MULTISAMPLE_NONE, true );
		m_exposureLastVal = SurfaceManager::Get().CreateRenderTarget( exposureStruct );

		RenderTarget::CreationStruct exposureStructTemp = 
		RenderTarget::CreationStruct( (void*)((int)m_pExposureXDRTextures + (int)Gc::kTextureAlignment), Gc::kTextureAlignment, 1, 1, GF_R32F, GAA_MULTISAMPLE_NONE, true );
		m_exposureLastValTemp = SurfaceManager::Get().CreateRenderTarget( exposureStructTemp );

		m_exposureOveride = TextureManager::CreateProcedural2DTexture( 2, 2, GF_ARGB8, CVector(1.0f,1.0f,1.0f,1.0f) );
	}
}

//-----------------------------------------------------
//!
//! PIPView::FreeView
//! Free VRAM used by view
//!
//-----------------------------------------------------
void PIPView::FreeView()
{
	ntAssert_p( m_bAllocated, ("PIPView not setup.") );
	m_bAllocated = false;

	Renderer::Get().m_pPIPManager->ReleaseSharedRB( PIPManager::BACKBUFFER_LDR );
	Renderer::Get().m_pPIPManager->ReleaseSharedRB( PIPManager::DEPTHBUFFER_LDR );
	Renderer::Get().m_pPIPManager->ReleaseSharedRB( PIPManager::DEPTHBUFFER_FLOAT );
	Renderer::Get().m_pPIPManager->ReleaseSharedRB( PIPManager::BACKBUFFER_FLOAT );
	Renderer::Get().m_pPIPManager->ReleaseSharedRB( PIPManager::BACKBUFFER_HDR );
	Renderer::Get().m_pPIPManager->ReleaseSharedRB( PIPManager::DEPTHBUFFER_HDR );
	Renderer::Get().m_pPIPManager->ReleaseSharedTexture( PIPManager::IRRADCACHE_HDR );
}

//--------------------------------------------------
//!
//! PIPView::GetTexture
//! Retrieve our view render target as a texture
//!
//--------------------------------------------------
Texture::Ptr PIPView::GetTexture() const
{
	ntAssert_p( m_bAllocated, ("PIPView not setup.") );
	return m_pBackBuffer->GetTexture();
}




//-----------------------------------------------------
//!
//! PIPManager::ctor
//! Allocate shaders and VRAM
//!
//-----------------------------------------------------
PIPManager::PIPManager()
{
	for (uint32_t i = 0; i < MAX_VIEWS; i++)
		m_views[i].SetDebugID(i);

	m_bViewsRendering		= false;
	m_pCurrentView			= 0;
	m_iCompositeClearCol	= NTCOLOUR_ARGB(0xff,0x10,0x10,0x10);
	m_iFadeCol				= m_iCompositeClearCol;
	m_fFadeFraction			= 0.f;

	m_compositeVS = DebugShaderCache::Get().LoadShader( "passthrough_pos_tex_vp.sho" );
	m_compositePS = DebugShaderCache::Get().LoadShader( "simpletexcol_lerp_fp.sho" );

	m_iBBWidth = (uint32_t)DisplayManager::Get().GetInternalWidth();
	m_iBBHeight = (uint32_t)DisplayManager::Get().GetInternalHeight();

	uint32_t iAllocWidth = m_iBBWidth;
	uint32_t iAllocHeight = m_iBBHeight;

#ifdef _PROFILING
	if (CRendererSettings::bProfileLowFillrate)
	{
		iAllocWidth = (int)(_R(iAllocWidth)*0.01f);
		iAllocHeight = (int)(_R(iAllocHeight)*0.01f);
	}
#endif

	// create shared VRAM resources
	m_iAllocFlags = 0;

	// back buffer
	RenderTarget::CreationStruct createParamsBB( iAllocWidth, iAllocHeight, GF_ARGB8 );
	m_pBackBuffer = SurfaceManager::Get().CreateRenderTarget( createParamsBB );

	// Z buffer
	RenderTarget::CreationStruct createParamsZ( iAllocWidth, iAllocHeight, GF_D24S8 );
	m_pZBuffer = SurfaceManager::Get().CreateRenderTarget( createParamsZ );

	// floating point Z buffer
	RenderTarget::CreationStruct createParamsFZ( iAllocWidth, iAllocHeight, GF_R32F );
	m_pFloatZBuffer = SurfaceManager::Get().CreateRenderTarget( createParamsFZ );

	// floating point colour buffer
	RenderTarget::CreationStruct createParamsFHDR( iAllocWidth, iAllocHeight, GF_ABGR16F );
	m_pFloatColour = SurfaceManager::Get().CreateRenderTarget( createParamsFHDR );

	// HDR back buffer and Z buffer
	AAMode eAAMode = CRendererSettings::GetAAMode();
	switch (eAAMode)
	{
		case AAM_MULTISAMPLE_4X:
		{
			m_pHDRBackBuffer = SurfaceManager::Get().CreateRenderTarget( iAllocWidth, iAllocHeight, GF_ARGB8, true, GAA_MULTISAMPLE_4X );
			m_pHDRZBuffer = SurfaceManager::Get().CreateRenderTarget( iAllocWidth, iAllocHeight, GF_D24S8, false, GAA_MULTISAMPLE_4X );

			// WARNING: IF you change the allocation order of our main render targets you can break this optmization!!
			// Re-use RSX ZCull memory filled by the z pre pass in the color pass (Marco)
			GcRenderBufferHandle msBuf = m_pHDRZBuffer->m_Platform.GetRenderBuffer();
			// first argument ( 1 ) is the index for our z cull slot
			// fifth argument ( 0 ) is the address of the z cull memory that is assigned by ICE to the z pre pass zbuffer, we want to reuse it!!
			cellGcmSetZcull( 1, msBuf->GetDataOffset(), msBuf->GetWidth(), msBuf->GetHeight(), 0, CELL_GCM_ZCULL_Z24S8, CELL_GCM_SURFACE_SQUARE_ROTATED_4, CELL_GCM_ZCULL_LESS, CELL_GCM_ZCULL_LONES, CELL_GCM_LESS, 0x80, 0xFF );
		
			break;
		}
	
		case AAM_SUPERSAMPLE_4X:
		{
			m_pHDRBackBuffer = SurfaceManager::Get().CreateRenderTarget( iAllocWidth*2, iAllocHeight*2, GF_ARGB8);
			m_pHDRZBuffer = SurfaceManager::Get().CreateRenderTarget( iAllocWidth*2, iAllocHeight*2, GF_D24S8, false);
			break;
		}
		
		default:
		{
			m_pHDRBackBuffer = SurfaceManager::Get().CreateRenderTarget( iAllocWidth, iAllocHeight, GF_ARGB8);
			m_pHDRZBuffer = SurfaceManager::Get().CreateRenderTarget( iAllocWidth, iAllocHeight, GF_D24S8, false);
			break;
		}
	}

	// IRRADIANCE CACHE
	// to cache our irradiance lighting we need a floating point cube map
	TexturePlatform::CreationStruct irradianceStruct = Texture::CreationStruct( IRRAD_CACHE_SIZE, IRRAD_CACHE_SIZE, 
																				ConvertGFXFORMATToGCFORMAT(GF_ABGR16F), 
																				(uint32_t)( logf(IRRAD_CACHE_SIZE)/logf(2.0f)) + 1 );
	// make sure we ask for a cube map (default constructor initialises that struct as a 2D texture)
	irradianceStruct.eType = TT_CUBE_TEXTURE;
	m_pIrradianceCache = SurfaceManager::Get().CreateTexture( irradianceStruct );
	

}

//-----------------------------------------------------
//!
//! PIPManager::PIPManager
//! Free VRAM used by views
//!
//-----------------------------------------------------
PIPManager::~PIPManager()
{
	SurfaceManager::Get().ReleaseRenderTarget( m_pBackBuffer );
	SurfaceManager::Get().ReleaseRenderTarget( m_pZBuffer );
	SurfaceManager::Get().ReleaseRenderTarget( m_pFloatZBuffer );
	SurfaceManager::Get().ReleaseRenderTarget( m_pFloatColour );
	SurfaceManager::Get().ReleaseRenderTarget( m_pHDRBackBuffer );
	SurfaceManager::Get().ReleaseRenderTarget( m_pHDRZBuffer );
	SurfaceManager::Get().ReleaseTexture( m_pIrradianceCache );
}

//-----------------------------------------------------
//!
//! PIPManager::GetAndLockSharedRB
//! Access particular VRAM buffer
//!
//-----------------------------------------------------
RenderTarget::Ptr PIPManager::GetAndLockSharedRB( BUFFER_TYPE eType, int32_t width, uint32_t height, GFXFORMAT eformat, GFXAAMODE aamode )
{
	ntError_p( (m_iAllocFlags & (1<<eType)) == 0, ("This buffer is already locked") );
	m_iAllocFlags |= (1<<eType);
	
	void* pMem = 0;
	uint32_t iPitch = 0;
	GcTextureHandle textureMapHandle;

	switch (eType)
	{
	case BACKBUFFER_LDR:
		pMem = m_pBackBuffer->m_Platform.GetRenderBuffer()->GetDataAddress();
		iPitch = m_pBackBuffer->m_Platform.GetRenderBuffer()->GetPitch();
		break;

	case DEPTHBUFFER_LDR:
		pMem = m_pZBuffer->m_Platform.GetRenderBuffer()->GetDataAddress();
		iPitch = m_pZBuffer->m_Platform.GetRenderBuffer()->GetPitch();
		break;

	case DEPTHBUFFER_FLOAT:
		pMem = m_pFloatZBuffer->m_Platform.GetRenderBuffer()->GetDataAddress();
		iPitch = m_pFloatZBuffer->m_Platform.GetRenderBuffer()->GetPitch();
		break;

	case BACKBUFFER_FLOAT:
		pMem = m_pFloatColour->m_Platform.GetRenderBuffer()->GetDataAddress();
		iPitch = m_pFloatColour->m_Platform.GetRenderBuffer()->GetPitch();
		break;

	case BACKBUFFER_HDR:
		pMem = m_pHDRBackBuffer->m_Platform.GetRenderBuffer()->GetDataAddress();
		iPitch = m_pHDRBackBuffer->m_Platform.GetRenderBuffer()->GetPitch();
		break;

	case DEPTHBUFFER_HDR:
		pMem = m_pHDRZBuffer->m_Platform.GetRenderBuffer()->GetDataAddress();
		iPitch = m_pHDRZBuffer->m_Platform.GetRenderBuffer()->GetPitch();
		break;

	default:
		ntError_p( 0, ("Unrecognised buffer type") );
		break;
	}

	RenderTarget::CreationStruct createParams( pMem, iPitch, width, height, eformat, aamode );
	return SurfaceManager::Get().CreateRenderTarget( createParams );
}

//-----------------------------------------------------
//!
//! PIPManager::GetAndLockSharedTexture
//! Access particular VRAM buffer
//!
//-----------------------------------------------------
Texture::Ptr PIPManager::GetAndLockSharedTexture( BUFFER_TYPE eType, int32_t width, uint32_t height, GFXFORMAT eformat, GFXAAMODE aamode )
{
	ntError_p( (m_iAllocFlags & (1<<eType)) == 0, ("This buffer is already locked") );
	m_iAllocFlags |= (1<<eType);

	Texture::Ptr texturePointer;

	switch (eType)
	{
		case IRRADCACHE_HDR:
			texturePointer = m_pIrradianceCache;
			break;
		default:
			ntError_p( 0, ("Unrecognised texture type") );
			break;
	}

	return texturePointer;
}


//-----------------------------------------------------
//!
//! PIPManager::ReleaseSharedRB
//! Mark particular VRAM buffer as available for use
//!
//-----------------------------------------------------
void PIPManager::ReleaseSharedRB( BUFFER_TYPE eType )
{
	ntError_p( (m_iAllocFlags & (1<<eType)), ("This buffer is not locked") );
	m_iAllocFlags &= ~(1<<eType);
}

//-----------------------------------------------------
//!
//! PIPManager::ReleaseSharedTexture
//! Mark particular VRAM buffer as available for use
//!
//-----------------------------------------------------
void PIPManager::ReleaseSharedTexture( BUFFER_TYPE eType )
{
	ReleaseSharedRB( eType );
}

// put valid view in views
void PIPManager::GetValidViews(SortableList<PIPView>& views)
{
	// now sort our views in priority order
	for (uint32_t i = 0; i < MAX_VIEWS; i++)
	{
		if (m_views[i].GetActive())
		{
			ntAssert_p( m_views[i].GetCamera(), ("Active view with no camera") );
			views.m_list.push_back( &m_views[i] );
		}
	}
}

//-----------------------------------------------------
//!
//! PIPManager::RenderBasic
//! Sets up simple back buffers and clears them
//!
//-----------------------------------------------------
void	PIPManager::RenderBasic()
{
	Renderer::Get().m_targetCache.SetColourAndDepthTargets( m_pBackBuffer, m_pZBuffer );
	Renderer::Get().Clear( Gc::kAllBufferBits, 0, 1.0f, 0 );


}

//-----------------------------------------------------
//!
//! PIPManager::PresentBasic
//! Copy to hardware back buffer
//!
//-----------------------------------------------------
void	PIPManager::PresentBasic()
{
#ifndef _GOLD_MASTER
	// flush any cached debug primitives or text
	CGatso::Start( "PIPManager::DebugDraw2D" );

	g_VisualDebug->Draw2D();
	g_VisualDebug->Reset();

	CGatso::Stop( "PIPManager::DebugDraw2D" );
#endif

	Renderer::Get().m_Platform.FlushCaches();
	Renderer::Get().m_targetCache.SetColourTarget( Renderer::GetHardwareBackBuffer() );

	// need to clear hardwared back buffer if we're bordering
	if (DisplayManager::Get().AspectMatchesInternal() == false)
		Renderer::Get().Clear( Gc::kColourBufferBit, m_iCompositeClearCol, 1.0f, 0 );

	DisplayFullscreenTexture( m_pBackBuffer->GetTexture() );

	//---------------------------------------------------------------------------------
	// Dump final buffer if required
	//---------------------------------------------------------------------------------
	if	(
		(CInputHardware::Exists()) &&
		(CInputHardware::Get().GetKeyboard().IsKeyPressed( KEYC_INSERT ))
		)
		Renderer::Get().m_targetCache.GetPrimaryColourTarget()->SaveToDisk( "FinalBuffer" );

	// swap the surfaces
	Renderer::Get().Present();
}

//-----------------------------------------------------
//!
//! PIPManager::RenderLevel
//! Renders all the views currently required by a level
//!
//-----------------------------------------------------
void	PIPManager::RenderLevel()
{
	// right, first we're going clear our real back buffer.
	RenderTarget::Ptr pCompositingBuffer = Renderer::GetHardwareBackBuffer();

	// now we clear it and loop over our views, rendering them one by one
	Renderer::Get().m_targetCache.SetColourTarget( pCompositingBuffer );
	Renderer::Get().Clear( Gc::kColourBufferBit, m_iCompositeClearCol, 1.0f, 0 );

	// now sort our views in priority order
	SortableList<PIPView>	views;
	GetValidViews(views);
	//for (uint32_t i = 0; i < MAX_VIEWS; i++)
	//{
	//	if (m_views[i].GetActive())
	//	{
	//		ntAssert_p( m_views[i].GetCamera(), ("Active view with no camera") );
	//		views.m_list.push_back( &m_views[i] );
	//	}
	//}

	ntAssert_p( !views.m_list.empty(), ("Must have atleast one view here") );
	views.SortListAscending();

	// now render them
	m_bViewsRendering = true;
	for (	SortableList<PIPView>::listIt it = views.m_list.begin();
			it != views.m_list.end(); ++it )
	{
		m_pCurrentView = (*it);

		// allocate views render targets
		m_pCurrentView->SetupView( m_iBBWidth, m_iBBHeight );

		// render the sector
		CSector::Get().Render( m_pCurrentView->GetCamera() );

		// swap to compositing buffer and draw view
		Renderer::Get().m_targetCache.SetColourTarget( pCompositingBuffer );

		// set render states
		Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );
		Renderer::Get().SetCullMode( GFX_CULLMODE_NONE );

		// set PS consts
		float r,g,b,a;
		NTCOLOUR_EXTRACT_FLOATS( m_pCurrentView->GetFadeColour(), r,g,b,a )
		CVector	fadeCol( r,g,b,a );
		CVector	fadeFrac( m_pCurrentView->GetFadeFraction(),0.f,0.f,0.f );

		m_compositePS->SetPSConstantByName( "colour", fadeCol );
		m_compositePS->SetPSConstantByName( "lerp_frac", fadeFrac );

		// set the shaders
		Renderer::Get().SetVertexShader( m_compositeVS, true );
		Renderer::Get().SetPixelShader( m_compositePS, true );

		// set dimensions
		float fLeft, fTop, fWidth, fHeight;
		m_pCurrentView->GetViewPos( fLeft, fTop );
		m_pCurrentView->GetViewDim( fWidth, fHeight );

		Renderer::Get().SetTexture( 0, m_pCurrentView->GetTexture() );
		Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );
		Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_BILINEAR );

		ViewVertex	verts[4];
		CalcViewspaceVerts( fLeft, fTop, fLeft + fWidth, fTop + fHeight, verts );

		if (m_pCurrentView->GetVB()->QueryGetNewScratchMemory())
		{
			m_pCurrentView->GetVB()->GetNewScratchMemory();
			m_pCurrentView->GetVB()->Write( verts, 0, sizeof( ViewVertex ) * 4 );

			// draw
			Renderer::Get().m_Platform.SetStream( m_pCurrentView->GetVB() );
			Renderer::Get().m_Platform.DrawPrimitives( Gc::kQuads, 0, 4 );
			Renderer::Get().m_Platform.ClearStreams();
		}

		// free the view's render targets
		m_pCurrentView->FreeView();

		// cleanup
		Renderer::Get().SetTexture( 0, Texture::NONE );
		Renderer::Get().SetBlendMode( GFX_BLENDMODE_NORMAL );
		Renderer::Get().SetCullMode( GFX_CULLMODE_NORMAL );

		if ( CInputHardware::Get().GetContext() == INPUT_CONTEXT_RENDER_DEBUG  )
		{
			float fStartX = (fLeft + (fWidth*0.5f))*m_iBBWidth;
			float fStartY = (fTop + (fHeight*0.5f))*m_iBBHeight;
			m_pCurrentView->PrintCounters(fStartX,fStartY);
		}
	}
	m_bViewsRendering = false;
	m_pCurrentView = 0;

	// now if we're not on a display device that matches our internal dimensions
	// copy back to our full sized buffer for HUD, Debug and other misc rendering b4 our final present.
	if (DisplayManager::Get().DeviceMatchesInternal() == false)
	{
		Renderer::Get().m_targetCache.SetColourTarget( m_pBackBuffer );

		ScreenSprite sprite;
		sprite.SetTexture( pCompositingBuffer->GetTexture() );
		sprite.SetPosition( CPoint( DisplayManager::Get().GetInternalWidth() * 0.5f, DisplayManager::Get().GetInternalHeight() * 0.5f, 0.0f ) );
		sprite.SetWidth( DisplayManager::Get().GetInternalWidth() );
		sprite.SetHeight( DisplayManager::Get().GetInternalHeight() );
		sprite.Render();

		// now we intentionally leave this back buffer on, for the hud and debug rendering
	}
}

//-----------------------------------------------------
//!
//! PIPManager::CalcViewspaceVerts
//! Generates two D3D viewspace triangles
//!
//-----------------------------------------------------
void PIPManager::CalcViewspaceVerts(	float fLeft, float fTop, float fRight, float fBottom,
										ViewVertex* pVerts )
{
	pVerts[0] = ViewVertex( 2.0f*fLeft - 1.0f,	1.0f - 2.0f*fTop,		0.0f,	0.0f );
	pVerts[1] = ViewVertex( 2.0f*fRight - 1.0f,	1.0f - 2.0f*fTop,		1.0f,	0.0f );
	pVerts[2] = ViewVertex( 2.0f*fRight - 1.0f,	1.0f - 2.0f*fBottom,	1.0f,	1.0f );
	pVerts[3] = ViewVertex( 2.0f*fLeft - 1.0f,	1.0f - 2.0f*fBottom,	0.0f,	1.0f );
}

//-----------------------------------------------------
//!
//!	PIPManager::DisplayFullscreenTexture
//! Displays a texture that is assumed to have square pixels
//! (thats important!) in a manner that fits our output
//! display mode. Used for placing internal backbuffer
//! onto the Gc back buffer, or for loading screens
//!
//-----------------------------------------------------
void PIPManager::DisplayFullscreenTexture( Texture::Ptr pTexture )
{
	ScreenSprite sprite;
	sprite.SetTexture( pTexture );
	sprite.SetPosition( CPoint( DisplayManager::Get().GetDeviceWidth() * 0.5f, DisplayManager::Get().GetDeviceHeight() * 0.5f, 0.0f ) );

	float fTextureAspect = _R(pTexture->GetWidth()) / _R(pTexture->GetHeight());
	if ( fTextureAspect > DisplayManager::Get().GetDeviceAspect() )
	{
		// our display is 'square-er' than our texture (say a 16:9 texture on a 4:3 display)
		// this means we're bordering on top and bottom
		sprite.SetWidth( DisplayManager::Get().GetDeviceWidth() );
		sprite.SetHeight( (DisplayManager::Get().GetDeviceAspect() / fTextureAspect) * DisplayManager::Get().GetDeviceHeight() );
	}
	else
	{
		// our texture is 'square-er' than our display (say a 4:3 texture on a 16:9 display)
		// this means we're bordering on left and right
		sprite.SetWidth( (fTextureAspect / DisplayManager::Get().GetDeviceAspect()) * DisplayManager::Get().GetDeviceWidth() );
		sprite.SetHeight( DisplayManager::Get().GetDeviceHeight() );
	}
			
	sprite.Render();
}

//-----------------------------------------------------
//!
//!	PIPManager::PresentLevel
//! We have finished drawing, so swap buffers
//!
//-----------------------------------------------------
void PIPManager::PresentLevel()
{
	//---------------------------------------------------------------------------------
	// Dump entire frame push buffer if required
	//---------------------------------------------------------------------------------
	if	(
		(CInputHardware::Exists()) &&
		(CInputHardware::Get().GetKeyboard().IsKeyPressed( KEYC_KPAD_MULTIPLY ))
		)
	{
		Renderer::Get().m_Platform.DumpPushBuffer(  Renderer::Get().m_Platform.GetStartPushBufferAddr(), 
													Renderer::Get().m_Platform.GetCurrentPushBufferAddr(), 
													"/app_home/content_ps3/_hspb.bin" );
	}
#ifndef _GOLD_MASTER
	// flush any cached debug primitives or text
	CGatso::Start( "PIPManager::DebugDraw2D" );

	g_VisualDebug->Draw2D();
	g_VisualDebug->Reset();

	CGatso::Stop( "PIPManager::DebugDraw2D" );
#endif

	// Update capture manager 
	CaptureSystem::Get().Update(0.0f);

	// Okay, so were rendering our old back buffer back into the Gc back buffer
	if (DisplayManager::Get().DeviceMatchesInternal() == false)
	{
		Renderer::Get().m_Platform.FlushCaches();
		Renderer::Get().m_targetCache.SetColourTarget( Renderer::GetHardwareBackBuffer() );

		// need to clear back buffer if we're bordering
		if (DisplayManager::Get().AspectMatchesInternal() == false)
			Renderer::Get().Clear( Gc::kColourBufferBit, m_iCompositeClearCol, 1.0f, 0 );

		DisplayFullscreenTexture( m_pBackBuffer->GetTexture() );
	}

	//---------------------------------------------------------------------------------
	// Dump final buffer if required
	//---------------------------------------------------------------------------------
	if	(
		(CInputHardware::Exists()) &&
		(CInputHardware::Get().GetKeyboard().IsKeyPressed( KEYC_INSERT ))
		)
		Renderer::Get().m_targetCache.GetPrimaryColourTarget()->SaveToDisk( "FinalBuffer" );

	// make sure our batch rendering is finished.
	BatchRenderer::PrePresent();

	// swap the surfaces
	Renderer::Get().Present();
}
