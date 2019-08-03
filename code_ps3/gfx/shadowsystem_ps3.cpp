/***************************************************************************************************
*
*	DESCRIPTION
*
*	NOTES
*
***************************************************************************************************/

#include "gfx/shadowsystem.h"
#include "gfx/renderer.h"
#include "gfx/rendercontext.h"
#include "gfx/surfacemanager.h"
#include "core/frustum.h"
#include "gfx/renderable.h"
#include "camera/camutils.h"
#include "effect/effect_shims.h" // debugrender
#include "effect/effect_manager.h"
#include "core/gatso.h"
#include "gfx/hardwarecaps.h"
#include "gfx/texturemanager.h"
#include "gfx/batchrender_ppu_spu.h"
#include "gfx/renderer_ppu_spu.h"

#ifdef _SPEEDTREE
	#include "speedtree/SpeedTreeManager_ps3.h"
#endif


//--------------------------------------------------
//!
//!	ShadowMapSystem::ctor
//! allocation done in ctor to ensure were in tiled mem
//!
//--------------------------------------------------
ShadowMapSystem::ShadowMapSystem( int iShadowMapSize )
{
	NT_NEW_CHUNK( Mem::MC_GFX ) CShadowSystemController;

	// since we use 4x multisampling to generate shadow maps we divide shadow map size by two (Marco)
	//iShadowMapSize = iShadowMapSize>>1;

	m_iShadowMapSize = iShadowMapSize;

#ifdef _PROFILING
	if (CRendererSettings::bProfileLowFillrate)
		m_iShadowMapSize = (int)(_R(m_iShadowMapSize)*0.01f);
	else if (CRendererSettings::bUseLowRezMode)
		m_iShadowMapSize = m_iShadowMapSize>>1;
#endif

//#define TEST_SWIZZLED_SMAP
#ifdef TEST_SWIZZLED_SMAP

	bool bUseSwizzled = true;
	uint32_t iPitchOrSwizzle = bUseSwizzled ? 0 : m_iShadowMapSize * 4;

	GcTextureHandle hTex = GcTexture::Create(	Gc::kTexture2D,
												1,
												m_iShadowMapSize,
												m_iShadowMapSize,
												1,
												Gc::kTexFormatD24X8,
												false,
												iPitchOrSwizzle,
												Gc::kStaticBuffer );
	m_pShadowMap = SurfaceManager::Get().CreateRenderTarget( RenderTarget::CreationStruct( GcRenderBuffer::Create( hTex->GetMipLevel(0) ) ) );
	
	GcTextureHandle hTex2 = GcTexture::Create(	Gc::kTexture2D,
												1,
												m_iShadowMapSize,
												m_iShadowMapSize,
												1,
												Gc::kTexFormatARGB8,
												false,
												iPitchOrSwizzle,
												Gc::kStaticBuffer );
	m_pAlias = SurfaceManager::Get().CreateRenderTarget( RenderTarget::CreationStruct( GcRenderBuffer::Create( hTex2->GetMipLevel(0) ) ) );

	if (bUseSwizzled)
	{
		ntError( m_pAlias->m_Platform.GetRenderBuffer()->IsSwizzled() );
		ntError( m_pShadowMap->m_Platform.GetRenderBuffer()->IsSwizzled() );
	}

#else
	// get the shadow map texture
	m_pShadowMap = SurfaceManager::Get().CreateRenderTarget( m_iShadowMapSize, m_iShadowMapSize, GF_D24S8, true, GAA_MULTISAMPLE_NONE );
#endif

#ifndef _DEBUG_SHADOW
	// fake up an alias to this render target
	RenderTarget::CreationStruct aliasParams(
		m_pShadowMap->m_Platform.GetRenderBuffer()->GetDataAddress(),
		m_pShadowMap->m_Platform.GetRenderBuffer()->GetPitch(),
		m_iShadowMapSize, m_iShadowMapSize, GF_ARGB8, GAA_MULTISAMPLE_NONE);

	m_pAlias = SurfaceManager::Get().CreateRenderTarget( aliasParams );
#else
	m_pAlias = SurfaceManager::Get().CreateRenderTarget( m_iShadowMapSize, m_iShadowMapSize, GF_R32F );
#endif
}

//--------------------------------------------------
//!
//!	ShadowMapSystem::dtor
//! Free memory
//!
//--------------------------------------------------
ShadowMapSystem::~ShadowMapSystem()
{
	SurfaceManager::Get().ReleaseRenderTarget( m_pShadowMap );
#ifdef TEST_SWIZZLED_SMAP
	SurfaceManager::Get().ReleaseRenderTarget( m_pAlias );
#endif

	CShadowSystemController::Kill();
}

//--------------------------------------------------
//!
//!	MatrixOrthoOffCenter
//! based off Direct3D MatrixOrthoOffCenterLH
//!
//--------------------------------------------------
static void MatrixOrthoOffCenter( CMatrix& matrix, float l, float r, float b, float t, float zn, float zf )
{
	matrix.Row(0) = CVector( -(2.f / (r-l)), 0.f, 0.f, 0.f );
	matrix.Row(1) = CVector( 0.f, 2.f / (t-b), 0.f, 0.f );
	matrix.Row(2) = CVector( 0.f, 0.f, 1.f / ( zf - zn ), 0.f );
	matrix.Row(3) = CVector( -(l+r)/(l-r), (t+b)/(b-t), zn / (zn - zf), 1.f );
}
/* REMOVED AS NOT REFERENCED
static void MatrixOrthoOffCenter( CMatrix& matrix, const CAABB& box )
{
	MatrixOrthoOffCenter(matrix,	box.Min().X(), box.Max().X(), 
									box.Min().Y(), box.Max().Y(),
									box.Min().Z(), box.Max().Z() );
}
*/
static void	MatrixLookAt( CMatrix& obMat, const CPoint& obEye, const CPoint& obAt, const CDirection& obUp )
{	
	CDirection	obZAxis( obAt ^ obEye );
	obZAxis.Normalise();
	CDirection	obXAxis = obUp.Cross( obZAxis );
	obXAxis.Normalise();
	CDirection	obYAxis = obZAxis.Cross( obXAxis );

	CPoint obTrans(	-obXAxis.Dot( CDirection(obEye) ),
					-obYAxis.Dot( CDirection(obEye) ),
					-obZAxis.Dot( CDirection(obEye) ) );

	obMat.Row(0) = CVector( obXAxis.X(), obYAxis.X(), obZAxis.X(), 0.f );
	obMat.Row(1) = CVector( obXAxis.Y(), obYAxis.Y(), obZAxis.Y(), 0.f );
	obMat.Row(2) = CVector( obXAxis.Z(), obYAxis.Z(), obZAxis.Z(), 0.f );
	obMat.Row(3) = CVector( obTrans.X(), obTrans.Y(), obTrans.Z(), 1.f );
}
//--------------------------------------------------
//!
//!	ShadowMapSystem::PrepareShadowMapForRendering
//! called before shadow casters renderered
//!
//--------------------------------------------------
void ShadowMapSystem::PrepareShadowMapForRendering()
{
	Renderer::Get().m_targetCache.SetColourAndDepthTargets( m_pAlias, m_pShadowMap );

#ifndef _DEBUG_SHADOW
	Renderer::Get().Clear( Gc::kDepthBufferBit, 0, 1.0f, 0 );
#else
	Renderer::Get().FastFloatClear(Gc::kAllBufferBits, 1.0f, 0);
#endif

	bool bFullShadowMap = false;
	if (CRendererSettings::iShadowQuality == 1)
		bFullShadowMap = true;

	BuildShadowMapProjection( 0, RenderingContext::Get()->m_shadowFrustumAABB[0], RenderingContext::Get()->m_shadowCasterAABB[0], bFullShadowMap  );
	BuildShadowMapProjection( 1, RenderingContext::Get()->m_shadowFrustumAABB[1], RenderingContext::Get()->m_shadowCasterAABB[1], bFullShadowMap  );
	BuildShadowMapProjection( 2, RenderingContext::Get()->m_shadowFrustumAABB[2], RenderingContext::Get()->m_shadowCasterAABB[2], bFullShadowMap  );
	//BuildShadowMapProjection( 3, RenderingContext::Get()->m_shadowFrustumAABB[3], RenderingContext::Get()->m_shadowCasterAABB[3], bFullShadowMap  );

	// render the shadow casters into the buffer (building up z)

	// Marcos
	float fDepthBias = RenderingContext::Get()->m_shadowMapDef.m_fShadowBias;
	float fBiasSlope = RenderingContext::Get()->m_shadowMapDef.m_fShadowSlope;

	GcKernel::SetPolygonOffset(fBiasSlope, fDepthBias);
	GcKernel::Enable(Gc::kPolygonOffsetFill);

	// important to disable colour writes, so we dont crap up the shadow map
#ifndef _DEBUG_SHADOW
	Renderer::Get().SetBlendMode( GFX_BLENDMODE_DISABLED );
#else
	Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );
#endif

	Renderer::Get().SetCullMode( GFX_CULLMODE_NORMAL );
}

//--------------------------------------------------
//!
//!	Finish off the shadow map
//!
//--------------------------------------------------
void ShadowMapSystem::FinaliseShadowMap()
{
	ntAssert( CShadowSystemController::Get().IsShadowMapActive() );

	// add post-projection to the shadow map 
	CMatrix obPostProjection( CONSTRUCT_IDENTITY );
	obPostProjection[0][0] = 0.5f;
	obPostProjection[1][1] = -0.5f;
	//obPostProjection[2][2] = 0.5f;
	obPostProjection[3] = CVector( 0.5f, 0.5f, 0.0f, 1.0f );

    for (unsigned int index = 0; index < 3; ++ index)
    {
	    RenderingContext::Get()->m_shadowMapProjection[index] = RenderingContext::Get()->m_shadowMapProjection[index] * obPostProjection;
        RenderingContext::Get()->m_viewToLightTransform[index] *= obPostProjection;
    }

	Renderer::Get().SetCullMode( GFX_CULLMODE_NORMAL );

	// reset depth bias	
	GcKernel::Disable(Gc::kPolygonOffsetFill);
	GcKernel::SetPolygonOffset(0,0);

	// assign it to the rendering context
	RenderingContext::Get()->m_pShadowMap = m_pShadowMap->GetTexture();

	// set the compare function here
	RenderingContext::Get()->m_pShadowMap->m_Platform.GetTexture()->SetCompareFunc( Gc::kGEqual );
}

//--------------------------------------------------
//!
//!	Build the shadow map projection
//!
//--------------------------------------------------
void ShadowMapSystem::BuildShadowMapProjection( unsigned int index, const CAABB& frustumAABB, const CAABB& casterAABB, bool bFullShadowMap )
{
    const CDirection eyeLightDir = RenderingContext::Get()->m_shadowDirection * RenderingContext::Get()->m_worldToView;


	CPoint hitPoint;
	CPoint lightPt;

	if( !frustumAABB.IsValid() || !casterAABB.IsValid() )
	{
		ScissorRegion stScissor = {0};

		RenderingContext::Get()->m_shadowMapProjection[index] = CMatrix(CONSTRUCT_IDENTITY );
		RenderingContext::Get()->m_shadowScissor[index] = stScissor;
		return;
	}

	/*
	float t;
	if( casterAABB.IntersectRay( frustumAABB.GetCentre(), eyeLightDir, t, hitPoint ) )
	{
		lightPt = frustumAABB.GetCentre() + 2.f * t * eyeLightDir;
	} else*/
	// The AABB intersection part was commented out cause it was causing some nasty projection (thus Z sign) flipping (Marco)
	{
		// no hit so just pick a far away point...
		lightPt = frustumAABB.GetCentre() + 2.f *eyeLightDir * 100000.f;
	}
  
    //  if the yAxis and the view direction are aligned, choose a different up vector, to avoid singularity
    //  artifacts
	CDirection axis = CVecMath::GetYAxis();
    if ( fabsf(eyeLightDir.Dot(axis))>0.99f )
		axis = CVecMath::GetZAxis();

	CMatrix lightView, lightProj;
	MatrixLookAt( lightView, frustumAABB.GetCentre(), lightPt, axis );
	CAABB lvFrustumAABB = frustumAABB;
	CAABB lvCasterAABB = casterAABB;
	lvFrustumAABB.Transform( lightView );
	lvCasterAABB.Transform( lightView );
	MatrixOrthoOffCenter( lightProj, lvFrustumAABB.Min().X(), lvFrustumAABB.Max().X(), 
									lvFrustumAABB.Min().Y(), lvFrustumAABB.Max().Y(),
									lvCasterAABB.Min().Z(), lvCasterAABB.Max().Z() );

	CMatrix ShadowProjection = (RenderingContext::Get()->m_worldToView * lightView) * lightProj;


	int iHalfShadowMapSize = m_iShadowMapSize / 2;

	ScissorRegion stScissor = {0};
	CMatrix obSubRectOffset( CONSTRUCT_IDENTITY );
	obSubRectOffset[0][0] = 0.5f;
	obSubRectOffset[1][1] = 0.5f;

	if (bFullShadowMap)
	{
		stScissor.left = 1; 
		stScissor.top =  1;
		stScissor.right = m_iShadowMapSize - 1;
		stScissor.bottom = m_iShadowMapSize - 1;
	}
	else
	{
		switch(index)
		{
		case 0:
			stScissor.left = 1; 
			stScissor.top = iHalfShadowMapSize + 1;
			stScissor.right = iHalfShadowMapSize - 1;
			stScissor.bottom = m_iShadowMapSize - 2;
			obSubRectOffset[3] = CVector( -0.5f, -0.5f, 0.0f, 1.0f );
			break;
		case 1:
			stScissor.left = iHalfShadowMapSize + 1; 
			stScissor.top = iHalfShadowMapSize + 1;
			stScissor.right = m_iShadowMapSize - 2;
			stScissor.bottom = m_iShadowMapSize - 2;
			obSubRectOffset[3] = CVector( 0.5f, -0.5f, 0.0f, 1.0f );
			break;
		case 2:
			stScissor.left = 1; 
			stScissor.top = 1;
			stScissor.right = iHalfShadowMapSize - 1;
			stScissor.bottom = iHalfShadowMapSize - 1;
			obSubRectOffset[3] = CVector( -0.5f, 0.5f, 0.0f, 1.0f );
			break;
		case 3:
			stScissor.left = iHalfShadowMapSize + 1; 
			stScissor.top = 1;
			stScissor.right = m_iShadowMapSize - 2;
			stScissor.bottom = iHalfShadowMapSize - 1;
			obSubRectOffset[3] = CVector( 0.5f, 0.5f, 0.0f, 1.0f );
			break;
		} 
	}

	RenderingContext::Get()->m_shadowMapProjection[index] = ShadowProjection  * obSubRectOffset;
	RenderingContext::Get()->m_shadowScissor[index] = stScissor;
    RenderingContext::Get()->m_viewToLightTransform[index] = (lightView * lightProj)  * obSubRectOffset;
}

void ShadowMapSystem::DebugRender()
{
	ScreenSprite* pSprite = NT_NEW_CHUNK( Mem::MC_GFX ) ScreenSprite;

	pSprite->SetPosition( CPoint( 320, 200,  0.0f ) );

	pSprite->SetWidth( 600 );
	pSprite->SetHeight( 360 );
	pSprite->SetTexture( m_pAlias->GetTexture() );
	pSprite->SetColour( NTCOLOUR_ARGB(0xff,0xff,0xff,0xff) );

	ScreenSpriteShim* pShim = NT_NEW_CHUNK( Mem::MC_EFFECTS ) ScreenSpriteShim( pSprite );
	pShim->m_bHDR = false;
	pShim->m_bAlpha = true;
	EffectManager::Get().AddEffect( pShim );
}

 


//--------------------------------------------------
//!
//!	constructor
//!
//--------------------------------------------------
StencilShadowSystem::StencilShadowSystem()
{
	// find the shaders
	m_stencilToColourVS = DebugShaderCache::Get().LoadShader( "passthrough_pos_vp.sho" );
	m_stencilToColourPS = DebugShaderCache::Get().LoadShader( "white_fp.sho" );

	GcStreamField	simpleQuadDesc[] = 
	{
		GcStreamField( FwHashedString( "IN.position" ), 0, Gc::kFloat, 2 ),	// 2 * 4 bytes
	};

	float const simpleQuad[] = 
	{
		-1.0f,  1.0f,// 0
		 1.0f,  1.0f,// 1
		 1.0f, -1.0f,// 2
		-1.0f, -1.0f,// 3
	};

	m_hSimpleQuadData = RendererPlatform::CreateVertexStream( 4, sizeof( float ) * 4, 2, simpleQuadDesc );
	m_hSimpleQuadData->Write( simpleQuad );
}

//--------------------------------------------------
//!
//!	convert stencil contents to RGB
//!
//--------------------------------------------------
void StencilShadowSystem::RenderStencilToColour()
{
	Renderer::Get().SetVertexShader( m_stencilToColourVS );
	Renderer::Get().SetPixelShader( m_stencilToColourPS );

	Renderer::Get().m_Platform.SetStream( m_hSimpleQuadData );

	// render a coloured quad to the stencilled regions
	Renderer::Get().SetZBufferMode( GFX_ZMODE_DISABLED );

	GcKernel::Enable(Gc::kStencilTestFront);
	GcKernel::Enable(Gc::kStencilTestBack);

	GcKernel::SetStencilTest(Gc::kNotEqual, 0, 0xff);

	Renderer::Get().m_Platform.DrawPrimitives( Gc::kQuads, 0, 4 );
	Renderer::Get().m_Platform.ClearStreams();

	GcKernel::Disable(Gc::kStencilTestFront);
	GcKernel::Disable(Gc::kStencilTestBack);

	Renderer::Get().SetZBufferMode( GFX_ZMODE_NORMAL );
}

//--------------------------------------------------
//!
//!	free render targets, blur stencil result. or not.
//!
//--------------------------------------------------
void StencilShadowSystem::DebugRender( RenderTarget::Ptr stencilResult )
{
	ScreenSprite* pSprite = NT_NEW_CHUNK( Mem::MC_GFX ) ScreenSprite;
	pSprite->SetPosition( CPoint( 400, 540,  0.0f ) );

	pSprite->SetWidth( 600 );
	pSprite->SetHeight( 360 );
	pSprite->SetTexture( stencilResult->GetTexture() );
	pSprite->SetColour( NTCOLOUR_ARGB(0xff,0xff,0xff,0xff) );

	ScreenSpriteShim* pShim = NT_NEW_CHUNK( Mem::MC_EFFECTS ) ScreenSpriteShim( pSprite );

	// HDR, as we're reusing the LDR buffer as the stencil result, so cant
	// render it then.
	pShim->m_bHDR = true;
	pShim->m_bAlpha = false;
	EffectManager::Get().AddEffect( pShim );
}




/***************************************************************************************************
*
*	FUNCTION		CShadowSystemController::IsShadowMapActive
*
*	DESCRIPTION
*
***************************************************************************************************/

bool CShadowSystemController::IsShadowMapActive() const
{
	return CRendererSettings::bEnableShadows;
}

/***************************************************************************************************
*
*	FUNCTION		CShadowSystemController::IsStencilActive
*
*	DESCRIPTION
*
***************************************************************************************************/

bool CShadowSystemController::IsStencilActive() const
{
	return IsShadowMapActive() && true;//m_bUseStencilling;
}

/***************************************************************************************************
*
*	FUNCTION		CShadowSystemController::GetShadowMapSize
*
*	DESCRIPTION
*
***************************************************************************************************/
void CShadowSystemController::RenderCasters() const
{
	BatchRenderer::Render( (void*)&RenderingContext::Get()->m_aBatchedShadowCastingRenderables, kRendPassShadowMap );
	SpuRenderer::Render( (void*)&RenderingContext::Get()->m_aShadowCastingOpaqueRenderables, kRendPassShadowMap );

#ifdef _SPEEDTREE
	// do camera update (for billboard)
	if (SpeedTreeManager::Exists())
	{
		SpeedTreeManager::Get().FinishRendering();
	}
#endif

	SpuRenderer::Render( (void*)&RenderingContext::Get()->m_aShadowCastingAlphaRenderables, kRendPassShadowMap );
}

void CShadowSystemController::RenderRecievers() const
{
	for( unsigned int iRenderable = 0; iRenderable < RenderingContext::Get()->m_aVisibleOpaqueRenderables.size(); ++iRenderable )
	{
		RenderingContext::Get()->m_aVisibleOpaqueRenderables[iRenderable]->RenderShadowOnly();
	}

	Renderer::Get().SetAlphaTestMode( GFX_ALPHATEST_GREATER, 127 );

	for( unsigned int iRenderable = 0; iRenderable <RenderingContext::Get()->m_aVisibleAlphaRenderables.size(); ++iRenderable )
	{
		RenderingContext::Get()->m_aVisibleAlphaRenderables[iRenderable]->RenderShadowOnly();
	}

	Renderer::Get().SetAlphaTestMode( GFX_ALPHATEST_NONE );
}


/***************************************************************************************************
*
*	FUNCTION		CShadowSystemController::SetStencilActive
*
*	DESCRIPTION
*
***************************************************************************************************/

void CShadowSystemController::SetStencilActive( bool bActive )
{
	m_bUseStencilling = bActive;
}
