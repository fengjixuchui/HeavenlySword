//--------------------------------------------------
//!
//!	\file sector.cpp
//!	platform agnostic portions of the sector renderer
//!
//--------------------------------------------------

#include "gfx/sector.h"
#include "gfx/renderer.h"
#include "gfx/renderable.h"
#include "gfx/levellighting.h"
#include "gfx/rendercontext.h"
#include "gfx/pictureinpicture.h"

#include "core/gatso.h"
#include "core/frustum.h"
#include "core/visualdebugger.h"

#include "anim/transform.h"

#include "effect/effect_manager.h"
#include "effect/effect_sky.h"

#ifdef PLATFORM_PS3
#include "gfx/updateskin_ppu_spu.h"
#include "gfx/batchrender_ppu_spu.h"
#include "gfx/renderer_ppu_spu.h"
#endif

//--------------------------------------------------
//!
//!	ctor
//!
//--------------------------------------------------
CSector::CSector() :
	m_shadowMapSystem(2048),
	m_pSkyBox(0),
	m_pCloudsEffect(0),
	m_bForceDump(false),
	m_bDoDumpFrame(false)
{
	// do this after classes inited, so shadow system gets tiled memory
	Renderer::Get().m_pPIPManager->GetView(0).SetActive(true);

#ifdef PLATFORM_PS3
	m_debugMask.Reset();
#endif // PLATFORM_PS3
}

//--------------------------------------------------
//!
//!	dtor
//!
//--------------------------------------------------
CSector::~CSector()
{
	if (m_pSkyBox)
		NT_DELETE_CHUNK( Mem::MC_GFX, m_pSkyBox );
}

//--------------------------------------------------
//!
//!	CreateEffects
//!
//--------------------------------------------------
void CSector::CreateEffects()
{
	if (m_pSkyBox)
		NT_DELETE_CHUNK( Mem::MC_GFX, m_pSkyBox );

	m_pSkyBox = NT_NEW_CHUNK( Mem::MC_GFX ) SkyEffect( 20, 20 );

	CStaticLensConfig::LevelReset();
}

//--------------------------------------------------
//!
//!	Render
//!
//--------------------------------------------------
void CSector::Render( const CCamera* pCamera )
{
	// create the context
	ntAssert( RenderingContext::GetIndex() == -1 );
	RenderingContext::PushContext();


	RenderingContext::Get()->m_pViewCamera = pCamera;
	RenderingContext::Get()->m_fScreenAspectRatio = Renderer::Get().m_targetCache.GetAspectRatio();
	RenderingContext::Get()->m_pViewCamera->GetProjection( RenderingContext::Get()->m_fScreenAspectRatio, RenderingContext::Get()->m_viewToScreen );

	RenderingContext::Get()->m_worldToView = pCamera->GetViewTransform()->GetWorldMatrix().GetAffineInverse();
	RenderingContext::Get()->m_worldToScreen = RenderingContext::Get()->m_worldToView * RenderingContext::Get()->m_viewToScreen;
	RenderingContext::Get()->m_pCullCamera = pCamera;

	// render the sector
	CGatso::Start( "CSector::RenderContext" );
	RenderContext();
	CGatso::Stop( "CSector::RenderContext" );

	// release the context
	RenderingContext::PopContext();
}

//--------------------------------------------------
//!
//!	Establish shadowing settings
//!
//--------------------------------------------------
void CSector::CalculatePlanePercents( float* fPlanePercents )
{
	fPlanePercents[0] = RenderingContext::Get()->m_shadowMapDef.m_fShadowPlane0;
	fPlanePercents[1] = RenderingContext::Get()->m_shadowMapDef.m_fShadowPlane1;
	fPlanePercents[2] = RenderingContext::Get()->m_shadowMapDef.m_fShadowPlane2;
	fPlanePercents[3] = RenderingContext::Get()->m_shadowMapDef.m_fShadowPlane3;
	fPlanePercents[4] = RenderingContext::Get()->m_shadowMapDef.m_fShadowPlane4;

	if( CRendererSettings::iShadowQuality < 4 )
	{
		for( int i=0; i < 4-CRendererSettings::iShadowQuality;i++)
		{
			fPlanePercents[4-i-1] = fPlanePercents[4-i];
		}
	}
}

//--------------------------------------------------
//!
//!	calculate visible renderables
//!
//--------------------------------------------------
void CSector::CalculateVisibility( const float fPlanePercents[5] )
{
	// retrieve the current lighting context
	LevelLighting::Get().RetrieveContextLighting( RenderingContext::Get()->GetEyePos() );

	// collide the renderables with the frustum to build the visible list
	if( CShadowSystemController::Get().IsShadowMapActive() )
	{
		m_obRenderableSpace.SetVisibleFrustum( fPlanePercents );
	}
	else
	{
		m_obRenderableSpace.SetVisibleFrustum();
	}

// invoke an SPU module that update all skinning data required to render all the visible skinned meshes in the current frame
#ifdef PLATFORM_PS3
//--------------------------------------------------
//!
//! DANGER! ACHTUNG! PERICOLO!
//! In the following function I'm preparing data for a SPU Job:
//! this SPU Code will update our bones matrices but it WILL NOT update 
//!	the CHierarchy Flags with a ClearFlagBits( HIERF_MAY_REQUIRE_SKIN_MATRIX_UPDATE )
//! cause it would be damn costly on the SPU (atomic ops..)
//! Flags are updated with the PPU before these matrices are updated for real!!!
//! A sync with this SPU job happens after the update of visible and opaque lists
//! Pleae be careful, and DO NOT modify bone matrices data between these 2 calls 
//! BTW I can't even imagine why one would want to do something like that ;) (Marco)
//--------------------------------------------------
	SpuUpdateSkin::UpdateSkin();
#endif

	GetRenderables().UpdateVisibleOpaqueAndAlphaLists();

	CFrustum frustum( RenderingContext::Get()->m_pCullCamera, RenderingContext::Get()->m_fScreenAspectRatio );
	EffectManager::Get().PreRender( &frustum );

#ifdef PLATFORM_PS3
	SpuUpdateSkin::Sync();
#endif
}

//--------------------------------------------------
//!
//!	Generate shadow map for key directional light
//!
//--------------------------------------------------


void CSector::GenerateShadowMap()
{
	if( CShadowSystemController::Get().IsShadowMapActive() )
	{
		// build the shadow map projection
		m_shadowMapSystem.PrepareShadowMapForRendering();
		CShadowSystemController::Get().RenderCasters();
		m_shadowMapSystem.FinaliseShadowMap();
		#ifdef _DEBUG_SHADOW
			m_shadowMapSystem.DebugRender();
		#endif // _DEBUG_SHADOW
	}
}

//--------------------------------------------------
//!
//!	Generate screen aligned shadow buffer
//!
//--------------------------------------------------
void CSector::RecieveShadows( const float /*fPlanePercents*/[5] )
{
	if( CShadowSystemController::Get().IsShadowMapActive() )
	{
		// render the shadow-map shadows into the buffer (building up z)
		for(int i=0;i < 4;i++)
		{
			CVector unitX4(1,0,0,0);
			CVector unitY4(0,1,0,0);
			CVector unitZ4(0,0,1,0);

			unitX4 = unitX4 * RenderingContext::Get()->m_shadowMapProjection[i];
			unitY4 = unitY4 * RenderingContext::Get()->m_shadowMapProjection[i];
			unitZ4 = unitZ4 * RenderingContext::Get()->m_shadowMapProjection[i];
			CPoint unitX3 = CPoint(unitX4);
			CPoint unitY3 = CPoint(unitY4);
			CPoint unitZ3 = CPoint(unitZ4);
			float d0 = unitX3.X() * unitX3.X() + unitX3.Y() * unitX3.Y();
			float d1 = unitY3.X() * unitY3.X() + unitY3.Y() * unitY3.Y();
			float d2 = unitZ3.X() * unitZ3.X() + unitZ3.Y() * unitZ3.Y();

			CVector regist;
			if( d0 < d1 )
			{
				if( d1 < d2 )
				{
					// d0 is smallest so disgard unitX3
					regist.X() = unitY3.X();
					regist.Y() = unitY3.Y();
					regist.Z() = unitZ3.X();
					regist.W() = unitZ3.Y();
				} else
				{
					// d2 is smallest so disgard unitZ3
					if( d2 < d0 )
					{
						regist.X() = unitX3.X();
						regist.Y() = unitX3.Y();
						regist.Z() = unitY3.X();
						regist.W() = unitY3.Y();
					} else
					{
						// d0 is smallest so disgard unitX3
						regist.X() = unitY3.X();
						regist.Y() = unitY3.Y();
						regist.Z() = unitZ3.X();
						regist.W() = unitZ3.Y();
					}
				}
			} else 
			{
				if( d1 < d2 )
				{
					// d1 is smallest so disgard unitY3
					regist.X() = unitX3.X();
					regist.Y() = unitX3.Y();
					regist.Z() = unitZ3.X();
					regist.W() = unitZ3.Y();
				} else
				{
					// D3 is smallest so disgard unitZ3
					regist.X() = unitX3.X();
					regist.Y() = unitX3.Y();
					regist.Z() = unitY3.X();
					regist.W() = unitY3.Y();
				}
			}

			if( i >= 1 )
			{
				regist *= 1.5f;
			}

			RenderingContext::Get()->m_shadowRadii[i] = regist;
		}

#ifdef PLATFORM_PS3
		CMatrix oDepthRangeFix( CONSTRUCT_IDENTITY );
		oDepthRangeFix[2][2] = 0.5f;
		oDepthRangeFix[3][2] = 0.5f;

        for (unsigned int index = 0; index < 4; ++ index)
        {
		    RenderingContext::Get()->m_shadowMapProjection[index] *= oDepthRangeFix;
            RenderingContext::Get()->m_viewToLightTransform[index] *= oDepthRangeFix;
        }

#else
		CShadowSystemController::Get().RenderRecievers();
#endif
	}
}

//--------------------------------------------------
//!
//!	CSector::RenderSky
//!
//--------------------------------------------------
void CSector::RenderSky()
{
	CGatso::Start( "RenderContext::RenderSky" );
	if ((CRendererSettings::bEnableSky || CRendererSettings::bShowWireframe) && (m_pSkyBox))
	{
		Renderer::Get().SetZBufferMode( GFX_ZMODE_LESSEQUAL_READONLY );
		m_pSkyBox->Render();		
		Renderer::Get().SetZBufferMode( GFX_ZMODE_NORMAL );
	}
	CGatso::Stop( "RenderContext::RenderSky" );
}


//--------------------------------------------------
//!
//!	CSector::RenderHDRAlpha
//!
//--------------------------------------------------
void CSector::RenderHDRAlpha( bool bHDRAlphaSupport )
{
	CGatso::Start( "RenderContext::RenderAlphaHDR" );

	if (bHDRAlphaSupport)
	{
		// render all the HDR alpha meshes: we only support lerp for renderables
		// and alpha renderables never write Z
		Renderer::Get().SetZBufferMode( GFX_ZMODE_LESSEQUAL_READONLY );

		if (CRendererSettings::bShowWireframe)
			Renderer::Get().SetFillMode( GFX_FILL_WIREFRAME );

		if(CRendererSettings::bUseAlphaBlending)
			Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );
		else
			Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );

#ifdef PLATFORM_PS3
		SpuRenderer::Render( (void*)&RenderingContext::Get()->m_aVisibleAlphaRenderables, kRendPassColorTransparent );
#else
		for( unsigned int iRenderable = 0; iRenderable <RenderingContext::Get()->m_aVisibleAlphaRenderables.size(); ++iRenderable )
			RenderingContext::Get()->m_aVisibleAlphaRenderables[iRenderable]->RenderMaterial();
#endif

		Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );
		Renderer::Get().SetZBufferMode( GFX_ZMODE_NORMAL );

		if (CRendererSettings::bShowWireframe)
			Renderer::Get().SetFillMode( GFX_FILL_SOLID );

		EffectManager::Get().RenderHDR();
	}

	CGatso::Stop( "RenderContext::RenderAlphaHDR" );
}

//--------------------------------------------------
//!
//!	CSector::RenderLDRAlpha
//!
//--------------------------------------------------
void CSector::RenderLDRAlpha( bool bHDRAlphaSupport )
{
	CGatso::Start( "RenderContext::RenderLDRAlpha" );

	if (CRendererSettings::bShowWireframe)
		Renderer::Get().SetFillMode( GFX_FILL_WIREFRAME );

	if (!bHDRAlphaSupport)
	{
		// render all the HDR alpha meshes as LDR
		Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );
		Renderer::Get().SetZBufferMode( GFX_ZMODE_LESSEQUAL_READONLY );

		for( unsigned int iRenderable = 0; iRenderable <RenderingContext::Get()->m_aVisibleAlphaRenderables.size(); ++iRenderable )
			RenderingContext::Get()->m_aVisibleAlphaRenderables[iRenderable]->RenderMaterial();

		Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );
		Renderer::Get().SetZBufferMode( GFX_ZMODE_LESSEQUAL );

		EffectManager::Get().RenderHDR();
		EffectManager::Get().RenderLDR();
	}
	else
	{
		EffectManager::Get().RenderLDR();
	}
	
	if (CRendererSettings::bShowWireframe)
		Renderer::Get().SetFillMode( GFX_FILL_SOLID );

	CGatso::Stop( "RenderContext::RenderLDRAlpha" );
}


