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
#include "core/frustum.h"
#include "gfx/renderable.h"
#include "camera/camutils.h"
#include "effect/effect_shims.h" // debugrender
#include "effect/effect_manager.h"
#include "core/gatso.h"
#include "gfx/hardwarecaps.h"
#include "gfx/surfacemanager.h"
#include "gfx/shader.h"
#include "gfx/graphicsdevice.h"
#include "exec/exec.h"

//--------------------------------------------------
//!
//!	Allocate textures required by shadow map
//!
//--------------------------------------------------
ShadowMapSystem::ShadowMapSystem( int iShadowMapSize )
	{
	NT_NEW_CHUNK(Mem::MC_GFX) CShadowSystemController;
		
	m_iShadowMapSize = iShadowMapSize;

		// get the shadow map textures
		if( HardwareCapabilities::Get().SupportsHardwareShadowMaps() )
		{
			// this is just a dummy to keep D3D happy
			m_pDummy = SurfaceManager::Get().CreateRenderTarget(
			RenderTarget::CreationStruct( m_iShadowMapSize, m_iShadowMapSize, D3DFMT_R5G6B5, false ) );
			
			m_pShadowMap = SurfaceManager::Get().CreateRenderTarget(
			RenderTarget::CreationStruct( m_iShadowMapSize, m_iShadowMapSize, D3DFMT_D24X8, false, false, true ) );
		}
		else
		{
			m_pShadowMap = SurfaceManager::Get().CreateRenderTarget(
			RenderTarget::CreationStruct( m_iShadowMapSize, m_iShadowMapSize, D3DFMT_R32F, false ) );
				
			m_pShadowMapDepths = SurfaceManager::Get().CreateSurface(
			Surface::CreationStruct( m_iShadowMapSize, m_iShadowMapSize, D3DFMT_D24X8 ) );
		}
}

//--------------------------------------------------
//!
//!	Deallocate textures required by shadow map
//!
//--------------------------------------------------
ShadowMapSystem::~ShadowMapSystem()
	{
		// (in this case, shadow map data is in the rendertarget)
		SurfaceManager::Get().ReleaseRenderTarget( m_pShadowMap );

		// just free the depth buffer for now (is NULL for hardware shadow map support)
		if( m_pShadowMapDepths )
			SurfaceManager::Get().ReleaseSurface( m_pShadowMapDepths );
		if( m_pDummy )
			SurfaceManager::Get().ReleaseRenderTarget( m_pDummy );

	CShadowSystemController::Kill();
	}

// based off Direct3D MatrixOrthoOffCenterLH
static void MatrixOrthoOffCenter( CMatrix& matrix, float l, float r, float b, float t, float zn, float zf )
{
	matrix.Row(0) = CVector( -(2.f / (r-l)), 0.f, 0.f, 0.f );
	matrix.Row(1) = CVector( 0.f, 2.f / (t-b), 0.f, 0.f );
	matrix.Row(2) = CVector( 0.f, 0.f, 1.f / ( zf - zn ), 0.f );
	matrix.Row(3) = CVector( -(l+r)/(l-r), (t+b)/(b-t), zn / (zn - zf), 1.f );
}
static void MatrixOrthoOffCenter( CMatrix& matrix, const CAABB& box )
{
	MatrixOrthoOffCenter(matrix,	box.Min().X(), box.Max().X(), 
									box.Min().Y(), box.Max().Y(),
									box.Min().Z(), box.Max().Z() );
}
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

// function adaptors
namespace
{
void BuildShadowMapProjectionAsyncFunc0( void* pParam, void* )
{
	ShadowMapSystem* pClass = (ShadowMapSystem*)pParam;
	pClass->BuildShadowMapProjectionByIndex( 0 );
}
void BuildShadowMapProjectionAsyncFunc1( void* pParam, void* )
{
	ShadowMapSystem* pClass = (ShadowMapSystem*)pParam;
	pClass->BuildShadowMapProjectionByIndex( 1 );
}
void BuildShadowMapProjectionAsyncFunc2( void* pParam, void* )
{
	ShadowMapSystem* pClass = (ShadowMapSystem*)pParam;
	pClass->BuildShadowMapProjectionByIndex( 2 );
}

void BuildShadowMapProjectionAsyncFunc3( void* pParam, void* )
{
	ShadowMapSystem* pClass = (ShadowMapSystem*)pParam;
	pClass->BuildShadowMapProjectionByIndex( 3 );
}

}
void ShadowMapSystem::BuildShadowMapProjectionByIndex( unsigned int index )
{
	ShadowMapSystem::BuildShadowMapProjection( index, RenderingContext::Get()->m_shadowFrustumAABB[index], RenderingContext::Get()->m_shadowCasterAABB[index]  );
}
void ShadowMapSystem::PrepareShadowMapForRendering()
{
	Exec::RunAsyncFunction( &BuildShadowMapProjectionAsyncFunc0, this, 0 );
	Exec::RunAsyncFunction( &BuildShadowMapProjectionAsyncFunc1, this, 0 );
	Exec::RunAsyncFunction( &BuildShadowMapProjectionAsyncFunc2, this, 0 );
	Exec::RunAsyncFunction( &BuildShadowMapProjectionAsyncFunc3, this, 0 );

//	BuildShadowMapProjection( 0, RenderingContext::Get()->m_shadowFrustumAABB[0], RenderingContext::Get()->m_shadowCasterAABB[0]  );
//	BuildShadowMapProjection( 1, RenderingContext::Get()->m_shadowFrustumAABB[1], RenderingContext::Get()->m_shadowCasterAABB[1]  );
//	BuildShadowMapProjection( 2, RenderingContext::Get()->m_shadowFrustumAABB[2], RenderingContext::Get()->m_shadowCasterAABB[2]  );
//	BuildShadowMapProjection( 3, RenderingContext::Get()->m_shadowFrustumAABB[3], RenderingContext::Get()->m_shadowCasterAABB[3]  );

	// set up the shadow map render targets and depth stencil
	if(  HardwareCapabilities::Get().SupportsHardwareShadowMaps() )
	{
		Renderer::Get().m_targetCache.SetColourAndDepthTargets( m_pDummy, m_pShadowMap->GetSurface() );
		Renderer::Get().Clear( D3DCLEAR_ZBUFFER , 0, 1.0f, 0 );
	}
	else
	{
		Renderer::Get().m_targetCache.SetColourAndDepthTargets( m_pShadowMap, m_pShadowMapDepths );
		Renderer::Get().Clear( D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00ff0000, 1.0f, 0 );
	}


	// render the shadow casters into the buffer (building up z)
	GetD3DDevice()->SetRenderState( D3DRS_SCISSORTESTENABLE, TRUE );

	if(  HardwareCapabilities::Get().SupportsHardwareShadowMaps() )
	{
		const float fDepthBias = 0.0000001f;
	    const float fBiasSlope = 2.0f;
//		const float fDepthBias = 0.0004f;
//	    const float fBiasSlope = 5.0f;

// reasonable reverse bias?
//		const float fDepthBias = -0.000004f;
//	    const float fBiasSlope = 0.5f;


		Renderer::Get().SetBlendMode( GFX_BLENDMODE_DISABLED );
		GetD3DDevice()->SetRenderState( D3DRS_DEPTHBIAS, *(uint32_t*)&fDepthBias);
		GetD3DDevice()->SetRenderState( D3DRS_SLOPESCALEDEPTHBIAS, *(uint32_t*)&fBiasSlope);

		Renderer::Get().SetCullMode( GFX_CULLMODE_NORMAL );
	} else
	{
		Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );
		Renderer::Get().SetCullMode( GFX_CULLMODE_REVERSED );
	}

	Exec::WaitUntilFunctionTasksComplete();
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
	obPostProjection[3] = CVector( 0.5f, 0.5f, 0.0f, 1.0f );

	RenderingContext::Get()->m_shadowMapProjection[0] = RenderingContext::Get()->m_shadowMapProjection[0] * obPostProjection;
	RenderingContext::Get()->m_shadowMapProjection[1] = RenderingContext::Get()->m_shadowMapProjection[1] * obPostProjection;
	RenderingContext::Get()->m_shadowMapProjection[2] = RenderingContext::Get()->m_shadowMapProjection[2] * obPostProjection;
	RenderingContext::Get()->m_shadowMapProjection[3] = RenderingContext::Get()->m_shadowMapProjection[3] * obPostProjection;

	// reset render states
	Renderer::Get().SetCullMode( GFX_CULLMODE_NORMAL );
	GetD3DDevice()->SetRenderState( D3DRS_SCISSORTESTENABLE, FALSE );
	GetD3DDevice()->SetRenderState( D3DRS_DEPTHBIAS, 0 );
	GetD3DDevice()->SetRenderState( D3DRS_SLOPESCALEDEPTHBIAS, 0 );

	// set on our rendering context
	RenderingContext::Get()->m_pShadowMap = m_pShadowMap->GetTexture();
}


//--------------------------------------------------
//!
//!	Build the shadow map projection
//!
//--------------------------------------------------
void ShadowMapSystem::BuildShadowMapProjection( unsigned int index, const CAABB& frustumAABB, const CAABB& casterAABB )
{
    const CDirection eyeLightDir = RenderingContext::Get()->m_shadowDirection * RenderingContext::Get()->m_worldToView;

	float t;
	CPoint hitPoint;
	CPoint lightPt;

	if( !frustumAABB.IsValid() || !casterAABB.IsValid() )
	{
		ScissorRegion stScissor = {0};

		RenderingContext::Get()->m_shadowMapProjection[index] = CMatrix(CONSTRUCT_IDENTITY );
		RenderingContext::Get()->m_shadowScissor[index] = stScissor;
		return;
	}



	if( casterAABB.IntersectRay( frustumAABB.GetCentre(), eyeLightDir, t, hitPoint ) )
	{
		lightPt = frustumAABB.GetCentre() + 2.f * t * eyeLightDir;
	} else
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

//	stScissor.left = 0; 
//	stScissor.top = 0;
//	stScissor.right = iHalfShadowMapSize - 1;
//	stScissor.bottom = iHalfShadowMapSize - 1;

	RenderingContext::Get()->m_shadowMapProjection[index] = ShadowProjection * obSubRectOffset;
	RenderingContext::Get()->m_shadowScissor[index] = stScissor;
}

void ShadowMapSystem::DebugRender()
{
	ScreenSprite* pSprite = NT_NEW_CHUNK(Mem::MC_GFX) ScreenSprite;

	pSprite->SetPosition( CPoint( 320, 200,  0.0f ) );

	pSprite->SetWidth( 600 );
	pSprite->SetHeight( 360 );
	pSprite->SetTexture( m_pShadowMap->GetTexture() );
	pSprite->SetColour(0xd0ffffff);

	ScreenSpriteShim* pShim = NT_NEW_CHUNK(Mem::MC_EFFECTS) ScreenSpriteShim( pSprite );
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
	m_pStencilToColourVS = ShaderManager::Get().FindShader( "stencil_to_colour_vs" );
	m_pStencilToColourPS = ShaderManager::Get().FindShader( "stencil_to_colour_ps" );
	ntAssert( m_pStencilToColourVS && m_pStencilToColourPS );

	D3DVERTEXELEMENT9 stDecl[] = 
	{
		{ 0, 0,					D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0 }, 
		D3DDECL_END()
	};
	m_pStencilToColourDecl = CVertexDeclarationManager::Get().GetDeclaration( stDecl );
}

//--------------------------------------------------
//!
//!	Allocate textures required by shadow system
//!
//--------------------------------------------------
void StencilShadowSystem::AllocateTextures(int iWidth,int iHeight,bool bCacheable)
{
	ntAssert( CShadowSystemController::Get().IsShadowMapActive() );

	// get a full-size LDR buffer, 
	// set up the viewport into these surfaces
	RenderTarget::CreationStruct createParams( iWidth, iHeight, D3DFMT_A8R8G8B8, false );
	createParams.bCacheable = bCacheable;

	m_pStencilResult = SurfaceManager::Get().CreateRenderTarget( createParams );
}

//--------------------------------------------------
//!
//!	Deallocate textures required by shadow system
//!
//--------------------------------------------------
void StencilShadowSystem::DeallocateTextures()
{
	ntAssert( CShadowSystemController::Get().IsShadowMapActive() );

	// put the rendertargets back in the cache
	SurfaceManager::Get().ReleaseRenderTarget( m_pStencilResult );
}

//--------------------------------------------------
//!
//!	convert stencil contents to RGB
//!
//--------------------------------------------------
void StencilShadowSystem::RenderStencilToColour()
{
	static const float afTri[] = 
	{
		-1.0f,  1.0f,
		 3.0f,  1.0f,
		-1.0f, -3.0f,
	};

	Renderer::Get().m_Platform.SetVertexDeclaration( m_pStencilToColourDecl );
	Renderer::Get().SetVertexShader( m_pStencilToColourVS );
	Renderer::Get().SetPixelShader( m_pStencilToColourPS );

	// render a coloured quad to the stencilled regions
	Renderer::Get().SetZBufferMode( GFX_ZMODE_DISABLED );

	GetD3DDevice()->SetRenderState( D3DRS_STENCILENABLE, TRUE );
	GetD3DDevice()->SetRenderState( D3DRS_STENCILFUNC, D3DCMP_NOTEQUAL );
	GetD3DDevice()->SetRenderState( D3DRS_STENCILREF, 0 );

	GetD3DDevice()->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 1, afTri, 2*sizeof( float ) );

	GetD3DDevice()->SetRenderState( D3DRS_STENCILENABLE, FALSE );
	GetD3DDevice()->SetRenderState( D3DRS_STENCILFUNC, D3DCMP_ALWAYS );

	Renderer::Get().SetZBufferMode( GFX_ZMODE_NORMAL );
}

//--------------------------------------------------
//!
//!	free render targets, blur stencil result. or not.
//!
//--------------------------------------------------
void StencilShadowSystem::FinaliseStencilMap()
{
	RenderingContext::Get()->m_pStencilTarget = m_pStencilResult->GetTexture();
}

//--------------------------------------------------
//!
//!	Debug rendering sprite..
//!
//--------------------------------------------------

void StencilShadowSystem::DebugRender()
{
	ScreenSprite* pSprite = NT_NEW_CHUNK(Mem::MC_GFX) ScreenSprite;

	pSprite->SetPosition( CPoint( 320, 540,  0.0f ) );

	pSprite->SetWidth( 600 );
	pSprite->SetHeight( 360 );
	pSprite->SetTexture( m_pStencilResult->GetTexture() );
	pSprite->SetColour( NTCOLOUR_ARGB(0xff,0xff,0xff,0xff) );

	ScreenSpriteShim* pShim = NT_NEW_CHUNK(Mem::MC_EFFECTS) ScreenSpriteShim( pSprite );
	pShim->m_bHDR = false;
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
	return CRendererSettings::bEnableShadows && HardwareCapabilities::Get().SupportsShaderModel3();
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
	for( unsigned int iRenderable = 0; iRenderable < RenderingContext::Get()->m_aShadowCastingRenderables.size(); ++iRenderable )
	{
		RenderingContext::Get()->m_aShadowCastingRenderables[iRenderable]->RenderShadowMap();
	}
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
