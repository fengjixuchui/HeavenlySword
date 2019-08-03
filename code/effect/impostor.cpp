//------------------------------------------------------------------------------------------
//!
//!	\file worldsprite.cpp
//! A world space quad sprite with a single texture and colour
//!
//------------------------------------------------------------------------------------------

#include "effect/impostor.h"
#include "gfx/renderer.h"
#include "gfx/texturemanager.h"
#include "gfx/shader.h"
#include "gfx/rendercontext.h"
#include "gfx/depthhazeconsts.h"
#include "gfx/renderable.h"
#include "gfx/textureatlas.h"
#include "anim/transform.h"
#include "game/randmanager.h"
#include "core/timer.h"

//------------------------------------------------------------------------------------------
//!
//!	PointImpostor::ctor
//!
//------------------------------------------------------------------------------------------
PointImpostor::PointImpostor( const ImpostorDef& def, int max ) :
	m_def(def)
{
	m_max = max;
	m_num = max;

	ntAssert_p( m_max > 0, ("Invalid size for impostor list") );

	// allocate storage for our sprites
	m_spriteData.GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT3,	PVE_POSITON,	"IN.position" );
	m_spriteData.GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT1,	PVE_SIZE,		"IN.size" );
	m_spriteData.GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT1,	PVE_RANDVALUE,	"IN.randseed" );

	m_spriteData.Initialise( m_max, true );

	PointVertex spriteTemplate;
	spriteTemplate.SetSize(m_def.m_fWidth);

	for (int i = 0; i < m_max; i++)
	{
		// unique seed per particle
		float fRandSeed = erandf( 1.0f );
		spriteTemplate.SetSeed(fRandSeed);

		void* pSprite = m_spriteData.GetGPUData().GetVertex( i );
		NT_MEMCPY( pSprite, &spriteTemplate, sizeof( PointVertex ) );
	}

	#ifdef PLATFORM_PC	

	ntError_p(0,("Point impostors not written for PC yet"));

	#elif defined(PLATFORM_PS3)

	if (m_def.m_type == ImpostorDef::I_RANDOM)
	{
		m_pVertexShader = DebugShaderCache::Get().LoadShader( "point_impostor_rand_vp.sho" );
		m_pDepthVS = DebugShaderCache::Get().LoadShader( "point_impostor_randdepth_vp.sho" );
		m_pPixelShader = DebugShaderCache::Get().LoadShader( "point_impostor_simple_fp.sho" );
		m_pDepthPS = DebugShaderCache::Get().LoadShader( "point_impostor_depth_fp.sho" );
	}
	else
	{
		m_pVertexShader = DebugShaderCache::Get().LoadShader( "point_impostor_anim_vp.sho" );
		m_pDepthVS = DebugShaderCache::Get().LoadShader( "point_impostor_animdepth_vp.sho" );
		m_pPixelShader = DebugShaderCache::Get().LoadShader( "point_impostor_anim_fp.sho" );
		m_pDepthPS = DebugShaderCache::Get().LoadShader( "point_impostor_animdepth_fp.sho" );
	}

	#endif

	m_pSurfaceAtlas = TextureAtlasManager::Get().GetAtlas( m_def.m_surfaceTex.c_str() );
	ntError_p( m_pSurfaceAtlas, ("Problem with texture atlas %s\n", m_def.m_surfaceTex.c_str() ));

	m_pNormalAtlas = TextureAtlasManager::Get().GetAtlas( m_def.m_normalTex.c_str() );
	ntError_p( m_pNormalAtlas, ("Problem with texture atlas %s\n", m_def.m_normalTex.c_str() ));

	// for the moment is much simpler if we assume we always have the same number
	// of frames and the same dimension in our surface and normal atlases

	ntError_p(	m_pSurfaceAtlas->GetNumEntries() == m_pNormalAtlas->GetNumEntries(), 
				("Atlas %s and %s have differing numbers of textures\n", m_def.m_surfaceTex.c_str(), m_def.m_normalTex.c_str() ) );

	ntError_p(	m_pSurfaceAtlas->GetEntryByIndex(0)->GetWidth() == m_pNormalAtlas->GetEntryByIndex(0)->GetWidth(), 
				("Atlas %s and %s have differing widths\n", m_def.m_surfaceTex.c_str(), m_def.m_normalTex.c_str() ) );
}

//------------------------------------------------------------------------------------------
//!
//!	PointImpostor::SetSize
//! methods to set the size(s) of a sprite / sprites
//!
//------------------------------------------------------------------------------------------
void PointImpostor::SetSize( float fSize, int iItem )
{
	ntAssert_p( iItem < m_max, ("PointImpostor index larger than current num of impostors") );
	((PointVertex*)m_spriteData.GetGPUData().GetVertex( iItem ))->SetSize(fSize);
}

//------------------------------------------------------------------------------------------
//!
//!	PointImpostor::SetPosition
//! we could use width and height here to get the actual corners of the sprite. BUT, doing
//! it in the vertex shader is effectivly free as the shader is piss simple, and means we
//! dont have to enforce a usage model on the order of data setting.
//!
//------------------------------------------------------------------------------------------
void PointImpostor::SetPosition( const CPoint& pos, int iItem )
{
	ntAssert_p( iItem < m_max, ("PointImpostor index larger than current num of impostors") );
	((PointVertex*)m_spriteData.GetGPUData().GetVertex( iItem ))->SetPos(pos.X(),pos.Y(),pos.Z());
}

//------------------------------------------------------------------------------------------
//!
//!	PointImpostor::SetSeed
//! Lets us use a packed array of sprites by maintaining our seeds ourselves
//!
//------------------------------------------------------------------------------------------
void PointImpostor::SetSeed( const float seed, int iItem )
{
	ntAssert_p( iItem < m_max, ("PointImpostor index larger than current num of impostors") );
	((PointVertex*)m_spriteData.GetGPUData().GetVertex( iItem ))->SetSeed(seed);
}

//------------------------------------------------------------------------------------------
//!
//!	PointImpostor::RenderDepth
//! Draw the depth of the impostors only
//!
//------------------------------------------------------------------------------------------
void PointImpostor::RenderDepthInternal( const CMatrix& worldViewProj )
{
	// ATTN! assumes a valid render context when we're called, not unreasonable!
	CMatrix camMat = RenderingContext::Get()->m_pViewCamera->GetViewTransform()->GetWorldMatrix();
	
	CPoint cameraZ( camMat.GetZAxis() );
	cameraZ.W() = -( camMat.GetTranslation().Dot( cameraZ ) );
	
	CVector vpScalars(	Renderer::Get().m_targetCache.GetWidthScalar(),
						Renderer::Get().m_targetCache.GetHeightScalar(),
						0.0f, 0.0f );

	CVector alphatest( m_def.m_fAlphaTest, 0.0f, 0.0f, 0.0f );

	#ifdef PLATFORM_PC

	UNUSED(worldViewProj);
	ntError_p(0,("Point impostors not written for PC yet"));

	#elif defined(PLATFORM_PS3)

	CVector NumTextures( _R(m_pSurfaceAtlas->GetNumEntries()), 0.0f, 0.0f, 0.0f );
	CVector TexWidth( m_pSurfaceAtlas->GetEntryByIndex(0)->GetWidth(), 0.0f, 0.0f, 0.0f );

	if (m_def.m_type == ImpostorDef::I_ANIMATED)
	{
		float fTime = CTimer::Get().GetGameTime() / m_def.m_fCycleTime;
		CVector fraction( (fTime - (int)fTime), 0.0f, 0.0f, 0.0f );

		m_pDepthVS->SetVSConstantByName( "currTimeN", fraction );
	}

	m_pDepthPS->SetPSConstantByName( "TA_numTex", NumTextures );
	m_pDepthPS->SetPSConstantByName( "TA_TexWidth", TexWidth );

	m_pDepthVS->SetVSConstantByName( "cameraZ", cameraZ );
	m_pDepthVS->SetVSConstantByName( "vpScalars", vpScalars );
	m_pDepthVS->SetVSConstantByName( "worldViewProj", worldViewProj );

	m_pDepthPS->SetPSConstantByName( "alphatest", alphatest );

	Renderer::Get().SetVertexShader( m_pDepthVS, true );
	Renderer::Get().SetPixelShader( m_pDepthPS, true );

	#endif

	Renderer::Get().SetPointSpriteEnable( true );

	// we assume that whoever calls our render has alread set / disabled the requried render states
	// so we just need to set texture states.
	Renderer::Get().SetTexture( 0, m_pSurfaceAtlas->GetAtlasTexture(), true );
	Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_WRAPALL );
	Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_BILINEAR );

	// write our dynamic buffer to the command buffer and render
	m_spriteData.Render( m_num );

	Renderer::Get().SetPointSpriteEnable( false );
}

//------------------------------------------------------------------------------------------
//!
//!	PointImpostor::RenderShadowMap
//! Render for shadow casting.
//!
//------------------------------------------------------------------------------------------
void PointImpostor::RenderShadowMap()
{
	// shadows not supported for PS impostors yet.
/*
	if(( m_FrameFlags & (	CRenderable::RFF_CAST_SHADOWMAP0 | CRenderable::RFF_CAST_SHADOWMAP1 | 
							CRenderable::RFF_CAST_SHADOWMAP2 | CRenderable::RFF_CAST_SHADOWMAP3) ) == false)
		return;

	Renderer::Get().SetCullMode( GFX_CULLMODE_NONE );

	if( m_FrameFlags & CRenderable::RFF_CAST_SHADOWMAP0 )
	{
		Renderer::Get().SetScissorRegion( &RenderingContext::Get()->m_shadowScissor[0] );
		RenderDepthInternal( RenderingContext::Get()->m_shadowMapProjection[0] );
	} 
	if( m_FrameFlags & CRenderable::RFF_CAST_SHADOWMAP1 )
	{
		Renderer::Get().SetScissorRegion( &RenderingContext::Get()->m_shadowScissor[1] );
		RenderDepthInternal( RenderingContext::Get()->m_shadowMapProjection[1] );
	} 
	if( m_FrameFlags & CRenderable::RFF_CAST_SHADOWMAP2 )
	{
		Renderer::Get().SetScissorRegion( &RenderingContext::Get()->m_shadowScissor[2] );
		RenderDepthInternal( RenderingContext::Get()->m_shadowMapProjection[2] );
	} 
	if( m_FrameFlags & CRenderable::RFF_CAST_SHADOWMAP3 )
	{
		Renderer::Get().SetScissorRegion( &RenderingContext::Get()->m_shadowScissor[3] );
		RenderDepthInternal( RenderingContext::Get()->m_shadowMapProjection[3] );
	}

	Renderer::Get().SetCullMode( GFX_CULLMODE_NORMAL );
*/
}

//------------------------------------------------------------------------------------------
//!
//!	PointImpostor::RenderMaterial
//! Draw the impostors in world space.
//!
//------------------------------------------------------------------------------------------
void PointImpostor::RenderDepth()
{
	RenderDepthInternal( RenderingContext::Get()->m_worldToScreen );
}

//------------------------------------------------------------------------------------------
//!
//!	PointImpostor::RenderMaterial
//! Draw the impostors in world space.
//!
//------------------------------------------------------------------------------------------
void PointImpostor::RenderMaterial()
{
	// ATTN! assumes a valid render context when we're called, not unreasonable!
	CMatrix camMat = RenderingContext::Get()->m_pViewCamera->GetViewTransform()->GetWorldMatrix();
	const CMatrix& worldViewProj = RenderingContext::Get()->m_worldToScreen;

	CPoint cameraZ( camMat.GetZAxis() );
	cameraZ.W() = -( camMat.GetTranslation().Dot( cameraZ ) );
	
	CVector vpScalars(	Renderer::Get().m_targetCache.GetWidthScalar(),
						Renderer::Get().m_targetCache.GetHeightScalar(),
						0.0f, 0.0f );

	#ifdef PLATFORM_PC

	UNUSED(worldViewProj);
	ntError_p(0,("Point impostors not written for PC yet"));

	#elif defined(PLATFORM_PS3)

	m_pVertexShader->SetVSConstantByName( "cameraZ", cameraZ );
	m_pVertexShader->SetVSConstantByName( "vpScalars", vpScalars );
	m_pVertexShader->SetVSConstantByName( "worldViewProj", worldViewProj );

	// required for depth haze
	m_pVertexShader->SetVSConstantByName( "worldView", RenderingContext::Get()->m_worldToView );

	CPoint viewPos = RenderingContext::Get()->GetEyePos();
	m_pVertexShader->SetVSConstantByName( "viewPosition_objectS", viewPos );
	m_pVertexShader->SetVSConstantByName( "sunDir_objectS", RenderingContext::Get()->m_sunDirection );

	m_pVertexShader->SetVSConstantByName( "DHConstsA", CDepthHazeSetting::GetAConsts() );
	m_pVertexShader->SetVSConstantByName( "DHConstsG", CDepthHazeSetting::GetGConsts() );
	m_pVertexShader->SetVSConstantByName( "DHB1plusB2", CDepthHazeSetting::GetBeta1PlusBeta2() );
	m_pVertexShader->SetVSConstantByName( "DHRecipB1plusB2", CDepthHazeSetting::GetOneOverBeta1PlusBeta2() );
	m_pVertexShader->SetVSConstantByName( "DHBdash1", CDepthHazeSetting::GetBetaDash1() );
	m_pVertexShader->SetVSConstantByName( "DHBdash2", CDepthHazeSetting::GetBetaDash2() );
	m_pVertexShader->SetVSConstantByName( "sunColour", CDepthHazeSetting::GetSunColour() );

	CVector NumTextures( _R(m_pSurfaceAtlas->GetNumEntries()), 0.0f, 0.0f, 0.0f );
	CVector TexWidth( m_pSurfaceAtlas->GetEntryByIndex(0)->GetWidth(), 0.0f, 0.0f, 0.0f );

	if (m_def.m_type == ImpostorDef::I_ANIMATED)
	{
		float fTime = CTimer::Get().GetGameTime() / m_def.m_fCycleTime;
		CVector fraction( (fTime - (int)fTime), 0.0f, 0.0f, 0.0f );

		m_pVertexShader->SetVSConstantByName( "currTimeN", fraction );
	}
	
	m_pPixelShader->SetPSConstantByName( "TA_numTex", NumTextures );
	m_pPixelShader->SetPSConstantByName( "TA_TexWidth", TexWidth );

	// required for per pixel lighting
	m_pPixelShader->SetPSConstantByName( "g_keyLightDir", RenderingContext::Get()->m_toKeyLight );
	m_pPixelShader->SetPSConstantByName( "g_keyDirColour", RenderingContext::Get()->m_keyColour );
	
	// Gaaaaaah! WTF???? i dont know why i need to flip the camera Z here.
	CMatrix viewToWorld = camMat;
	viewToWorld.SetZAxis( -viewToWorld.GetZAxis() );
	m_pPixelShader->SetPSConstantByName( "viewToWorld", viewToWorld );

	// SH coeffs	
	m_pPixelShader->SetPSConstantByName( "fillSHCoeffs", RenderingContext::Get()->m_SHMatrices.m_aChannelMats, 12 );

	// For shadow map lookup
	CVector posToUV( CONSTRUCT_CLEAR );
	posToUV.X() = 1.f / (Renderer::Get().m_targetCache.GetWidth() - 1);
	posToUV.Y() = 1.f / (Renderer::Get().m_targetCache.GetHeight() - 1);

	m_pPixelShader->SetPSConstantByName( "posToUV", posToUV );

	// discard over this much
	CVector alphatest( m_def.m_fAlphaTest, 0.0f, 0.0f, 0.0f );
	m_pPixelShader->SetPSConstantByName( "alphatest", alphatest );

	Renderer::Get().SetVertexShader( m_pVertexShader, true );
	Renderer::Get().SetPixelShader( m_pPixelShader, true );
	
	m_pSurfaceAtlas->GetAtlasTexture()->m_Platform.GetTexture()->SetGammaCorrect( Gc::kGammaCorrectSrgb );
//	m_surface->m_Platform.GetTexture()->SetLevelBias( -3 );

	#endif

	Renderer::Get().SetPointSpriteEnable( true );

	// we assume that whoever calls our render has alread set / disabled the requried render states
	// so we just need to set texture states.

	Renderer::Get().SetTexture( 0, m_pSurfaceAtlas->GetAtlasTexture(), true );
	Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_WRAPALL );
	Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_BILINEAR );

	Renderer::Get().SetTexture( 1, m_pNormalAtlas->GetAtlasTexture(), true );
	Renderer::Get().SetSamplerAddressMode( 1, TEXTUREADDRESS_WRAPALL );
	Renderer::Get().SetSamplerFilterMode( 1, TEXTUREFILTER_BILINEAR );

	Renderer::Get().SetTexture( 2, RenderingContext::Get()->m_pStencilTarget, true );
	Renderer::Get().SetSamplerAddressMode( 2, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerFilterMode( 2, TEXTUREFILTER_POINT );

	// write our dynamic buffer to the command buffer and render
	m_spriteData.Render( m_num );

	Renderer::Get().SetPointSpriteEnable( false );
}





//------------------------------------------------------------------------------------------
//!
//!	QuadImpostor::ctor
//!
//------------------------------------------------------------------------------------------
QuadImpostor::QuadImpostor( const ImpostorDef& def, int max ) :
	m_def(def)
{
	m_max = max;
	m_num = max;

	ntAssert_p( m_max > 0, ("Invalid size for impostor list") );

	// allocate storage for our sprites
	m_spriteData.GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT3,	QVE_POSITON,	"IN.position" );
	m_spriteData.GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT2,	QVE_WIDTHHEIGHT,"IN.widthheight" );
	m_spriteData.GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT2,	QVE_TEXCOORD,	"IN.texcoord" );
	m_spriteData.GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT1,	QVE_RANDVALUE,	"IN.randseed" );

	m_spriteData.Initialise( m_max, false );

	QuadVertex spriteTemplate[4];

#ifdef PLATFORM_PC
	spriteTemplate[0].SetTex(0.0f, 0.0f);
	spriteTemplate[1].SetTex(1.0f, 0.0f);
	spriteTemplate[2].SetTex(0.0f, 1.0f);
	spriteTemplate[3].SetTex(1.0f, 1.0f);
#else
	spriteTemplate[0].SetTex(0.0f, 0.0f);
	spriteTemplate[1].SetTex(1.0f, 0.0f);
	spriteTemplate[2].SetTex(1.0f, 1.0f);
	spriteTemplate[3].SetTex(0.0f, 1.0f);
#endif

	spriteTemplate[0].SetWH(m_def.m_fWidth, m_def.m_fHeight);
	spriteTemplate[1].SetWH(m_def.m_fWidth, m_def.m_fHeight);
	spriteTemplate[2].SetWH(m_def.m_fWidth, m_def.m_fHeight);
	spriteTemplate[3].SetWH(m_def.m_fWidth, m_def.m_fHeight);

	for (int i = 0; i < m_max; i++)
	{
		// unique seed per particle
		float fRandSeed = erandf( 1.0f );
		spriteTemplate[0].SetSeed(fRandSeed);
		spriteTemplate[1].SetSeed(fRandSeed);
		spriteTemplate[2].SetSeed(fRandSeed);
		spriteTemplate[3].SetSeed(fRandSeed);

		void* pSprite = m_spriteData.GetGPUData().GetVertex( i*4 );
		NT_MEMCPY( pSprite, spriteTemplate, sizeof( QuadVertex ) * 4 );
	}

	#ifdef PLATFORM_PC	

	m_pVertexShader = DebugShaderCache::Get().LoadShaderFILE( SHADERTYPE_VERTEX, "fxshaders/impostor_quad_vs.hlsl", "main", "vs_3_0" );
	m_pPixelShader = DebugShaderCache::Get().LoadShaderFILE( SHADERTYPE_PIXEL, "fxshaders/impostor_simple_ps.hlsl", "main", "ps_3_0" );
	m_pDepthVS = DebugShaderCache::Get().LoadShaderFILE( SHADERTYPE_VERTEX, "fxshaders/impostor_quaddepth_vs.hlsl", "main", "vs_3_0" );
	m_pDepthPS = DebugShaderCache::Get().LoadShaderFILE( SHADERTYPE_PIXEL, "fxshaders/impostor_depth_ps.hlsl", "main", "ps_3_0" );

	#elif defined(PLATFORM_PS3)

	if (m_def.m_type == ImpostorDef::I_RANDOM)
	{
		m_pVertexShader = DebugShaderCache::Get().LoadShader( "impostor_rand_vp.sho" );
		m_pDepthVS = DebugShaderCache::Get().LoadShader( "impostor_randdepth_vp.sho" );
		m_pPixelShader = DebugShaderCache::Get().LoadShader( "impostor_simple_fp.sho" );
		m_pDepthPS = DebugShaderCache::Get().LoadShader( "impostor_depth_fp.sho" );
	}
	else
	{
		m_pVertexShader = DebugShaderCache::Get().LoadShader( "impostor_anim_vp.sho" );
		m_pDepthVS = DebugShaderCache::Get().LoadShader( "impostor_animdepth_vp.sho" );
		m_pPixelShader = DebugShaderCache::Get().LoadShader( "impostor_anim_fp.sho" );
		m_pDepthPS = DebugShaderCache::Get().LoadShader( "impostor_animdepth_fp.sho" );
	}

	#endif

	m_pSurfaceAtlas = TextureAtlasManager::Get().GetAtlas( m_def.m_surfaceTex.c_str() );
	ntError_p( m_pSurfaceAtlas, ("Problem with texture atlas %s\n", m_def.m_surfaceTex.c_str() ));

	m_pNormalAtlas = TextureAtlasManager::Get().GetAtlas( m_def.m_normalTex.c_str() );
	ntError_p( m_pNormalAtlas, ("Problem with texture atlas %s\n", m_def.m_normalTex.c_str() ));

	// for the moment is much simpler if we assume we always have the same number
	// of frames and the same dimension in our surface and normal atlases

	ntError_p(	m_pSurfaceAtlas->GetNumEntries() == m_pNormalAtlas->GetNumEntries(), 
				("Atlas %s and %s have differing numbers of textures\n", m_def.m_surfaceTex.c_str(), m_def.m_normalTex.c_str() ) );

	ntError_p(	m_pSurfaceAtlas->GetEntryByIndex(0)->GetWidth() == m_pNormalAtlas->GetEntryByIndex(0)->GetWidth(), 
				("Atlas %s and %s have differing widths\n", m_def.m_surfaceTex.c_str(), m_def.m_normalTex.c_str() ) );
}

//------------------------------------------------------------------------------------------
//!
//!	QuadImpostor::SetSize
//! methods to set the size(s) of a sprite / sprites
//!
//------------------------------------------------------------------------------------------
void QuadImpostor::SetWidth( float fWidth, int iItem )
{
	ntAssert_p( iItem < m_max, ("QuadImpostor index larger than current num of impostors") );

	QuadVertex* pSprite = (QuadVertex*)m_spriteData.GetGPUData().GetVertex( iItem*4 );
	pSprite->SetW(fWidth);	pSprite++;
	pSprite->SetW(fWidth);	pSprite++;
	pSprite->SetW(fWidth);	pSprite++;
	pSprite->SetW(fWidth);
}

void QuadImpostor::SetHeight( float fHeight, int iItem )
{
	ntAssert_p( iItem < m_max, ("QuadImpostor index larger than current num of impostors") );

	QuadVertex* pSprite = (QuadVertex*)m_spriteData.GetGPUData().GetVertex( iItem*4 );
	pSprite->SetH(fHeight);	pSprite++;
	pSprite->SetH(fHeight);	pSprite++;
	pSprite->SetH(fHeight);	pSprite++;
	pSprite->SetH(fHeight);
}

//------------------------------------------------------------------------------------------
//!
//!	QuadImpostor::SetPosition
//! we could use width and height here to get the actual corners of the sprite. BUT, doing
//! it in the vertex shader is effectivly free as the shader is piss simple, and means we
//! dont have to enforce a usage model on the order of data setting.
//!
//------------------------------------------------------------------------------------------
void QuadImpostor::SetPosition( const CPoint& pos, int iItem )
{
	ntAssert_p( iItem < m_max, ("QuadImpostor index larger than current num of impostors") );

	QuadVertex* pSprite = (QuadVertex*)m_spriteData.GetGPUData().GetVertex( iItem*4 );
	pSprite->SetPos(pos.X(),pos.Y(),pos.Z());	pSprite++;
	pSprite->SetPos(pos.X(),pos.Y(),pos.Z());	pSprite++;
	pSprite->SetPos(pos.X(),pos.Y(),pos.Z());	pSprite++;
	pSprite->SetPos(pos.X(),pos.Y(),pos.Z());
}

//------------------------------------------------------------------------------------------
//!
//!	QuadImpostor::SetSeed
//! Lets us use a packed array of sprites by maintaining our seeds ourselves
//!
//------------------------------------------------------------------------------------------
void QuadImpostor::SetSeed( const float seed, int iItem )
{
	ntAssert_p( iItem < m_max, ("QuadImpostor index larger than current num of impostors") );

	QuadVertex* pSprite = (QuadVertex*)m_spriteData.GetGPUData().GetVertex( iItem*4 );
	pSprite->SetSeed(seed);	pSprite++;
	pSprite->SetSeed(seed);	pSprite++;
	pSprite->SetSeed(seed);	pSprite++;
	pSprite->SetSeed(seed);
}

//------------------------------------------------------------------------------------------
//!
//!	QuadImpostor::RenderDepth
//! Draw the depth of the impostors only
//!
//------------------------------------------------------------------------------------------
void QuadImpostor::RenderDepthInternal( const CMatrix& worldViewProj )
{
	// ATTN! assumes a valid render context when we're called, not unreasonable!
	CMatrix camMat = RenderingContext::Get()->m_pViewCamera->GetViewTransform()->GetWorldMatrix();

	CDirection cameraUnitAxisX = -RenderingContext::Get()->m_pViewCamera->GetViewTransform()->GetWorldMatrix().GetXAxis();
	CDirection cameraUnitAxisY = -RenderingContext::Get()->m_pViewCamera->GetViewTransform()->GetWorldMatrix().GetYAxis();
	CVector alphatest( m_def.m_fAlphaTest, 0.0f, 0.0f, 0.0f );

	#ifdef PLATFORM_PC

	Renderer::Get().SetVertexShader( m_pDepthVS );
	Renderer::Get().SetPixelShader( m_pDepthPS );

	Renderer::Get().SetVertexShaderConstant( 1, cameraUnitAxisX );
	Renderer::Get().SetVertexShaderConstant( 2, cameraUnitAxisY );
	Renderer::Get().SetVertexShaderConstant( 3, worldViewProj );
	
	Renderer::Get().SetPixelShaderConstant( 1, alphatest );

	#elif defined(PLATFORM_PS3)

	CVector NumTextures( _R(m_pSurfaceAtlas->GetNumEntries()), 0.0f, 0.0f, 0.0f );
	CVector TexWidth( m_pSurfaceAtlas->GetEntryByIndex(0)->GetWidth(), 0.0f, 0.0f, 0.0f );

	if (m_def.m_type == ImpostorDef::I_ANIMATED)
	{
		float fTime = CTimer::Get().GetGameTime() / m_def.m_fCycleTime;
		CVector fraction( (fTime - (int)fTime), 0.0f, 0.0f, 0.0f );

		m_pDepthVS->SetVSConstantByName( "currTimeN", fraction );

		m_pDepthPS->SetPSConstantByName( "TA_numTex", NumTextures );
		m_pDepthPS->SetPSConstantByName( "TA_TexWidth", TexWidth );
	}
	else
	{
		m_pDepthVS->SetVSConstantByName( "TA_numTex", NumTextures );
		m_pDepthVS->SetVSConstantByName( "TA_TexWidth", TexWidth );
	}

	m_pDepthVS->SetVSConstantByName( "cameraUnitAxisX", cameraUnitAxisX );
	m_pDepthVS->SetVSConstantByName( "cameraUnitAxisY", cameraUnitAxisY );
	m_pDepthVS->SetVSConstantByName( "worldViewProj", worldViewProj );

	m_pDepthPS->SetPSConstantByName( "alphatest", alphatest );

	Renderer::Get().SetVertexShader( m_pDepthVS, true );
	Renderer::Get().SetPixelShader( m_pDepthPS, true );

	#endif

	// we assume that whoever calls our render has alread set / disabled the requried render states
	// so we just need to set texture states.
	Renderer::Get().SetTexture( 0, m_pSurfaceAtlas->GetAtlasTexture(), true );
	Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_WRAPALL );
	Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_BILINEAR );

	// write our dynamic buffer to the command buffer and render
	m_spriteData.Render( m_num );
}

//------------------------------------------------------------------------------------------
//!
//!	QuadImpostor::RenderShadowMap
//! Render for shadow casting.
//!
//------------------------------------------------------------------------------------------
void QuadImpostor::RenderShadowMap()
{
	if(( m_FrameFlags & (	CRenderable::RFF_CAST_SHADOWMAP0 | CRenderable::RFF_CAST_SHADOWMAP1 | 
							CRenderable::RFF_CAST_SHADOWMAP2 | CRenderable::RFF_CAST_SHADOWMAP3) ) == false)
		return;

	Renderer::Get().SetCullMode( GFX_CULLMODE_NONE );

	if( m_FrameFlags & CRenderable::RFF_CAST_SHADOWMAP0 )
	{
		Renderer::Get().SetScissorRegion( &RenderingContext::Get()->m_shadowScissor[0] );
		RenderDepthInternal( RenderingContext::Get()->m_shadowMapProjection[0] );
	} 
	if( m_FrameFlags & CRenderable::RFF_CAST_SHADOWMAP1 )
	{
		Renderer::Get().SetScissorRegion( &RenderingContext::Get()->m_shadowScissor[1] );
		RenderDepthInternal( RenderingContext::Get()->m_shadowMapProjection[1] );
	} 
	if( m_FrameFlags & CRenderable::RFF_CAST_SHADOWMAP2 )
	{
		Renderer::Get().SetScissorRegion( &RenderingContext::Get()->m_shadowScissor[2] );
		RenderDepthInternal( RenderingContext::Get()->m_shadowMapProjection[2] );
	} 
	if( m_FrameFlags & CRenderable::RFF_CAST_SHADOWMAP3 )
	{
		Renderer::Get().SetScissorRegion( &RenderingContext::Get()->m_shadowScissor[3] );
		RenderDepthInternal( RenderingContext::Get()->m_shadowMapProjection[3] );
	}

	Renderer::Get().SetCullMode( GFX_CULLMODE_NORMAL );
}

//------------------------------------------------------------------------------------------
//!
//!	QuadImpostor::RenderMaterial
//! Draw the impostors in world space.
//!
//------------------------------------------------------------------------------------------
void QuadImpostor::RenderDepth()
{
	RenderDepthInternal( RenderingContext::Get()->m_worldToScreen );
}

//------------------------------------------------------------------------------------------
//!
//!	QuadImpostor::RenderMaterial
//! Draw the impostors in world space.
//!
//------------------------------------------------------------------------------------------
void QuadImpostor::RenderMaterial()
{
	// ATTN! assumes a valid render context when we're called, not unreasonable!
	CMatrix camMat = RenderingContext::Get()->m_pViewCamera->GetViewTransform()->GetWorldMatrix();
	const CMatrix& worldViewProj = RenderingContext::Get()->m_worldToScreen;

	CDirection cameraUnitAxisX = -RenderingContext::Get()->m_pViewCamera->GetViewTransform()->GetWorldMatrix().GetXAxis();
	CDirection cameraUnitAxisY = -RenderingContext::Get()->m_pViewCamera->GetViewTransform()->GetWorldMatrix().GetYAxis();

	#ifdef PLATFORM_PC

	Renderer::Get().SetVertexShader( m_pVertexShader );
	Renderer::Get().SetPixelShader( m_pPixelShader );

	Renderer::Get().SetVertexShaderConstant( 1, cameraUnitAxisX );
	Renderer::Get().SetVertexShaderConstant( 2, cameraUnitAxisY );
	Renderer::Get().SetVertexShaderConstant( 3, worldViewProj );

	// required for depth haze
	Renderer::Get().SetVertexShaderConstant( 7, RenderingContext::Get()->m_worldToView );

	CPoint viewPos = RenderingContext::Get()->GetEyePos();
	Renderer::Get().SetVertexShaderConstant( 11, viewPos );
	Renderer::Get().SetVertexShaderConstant( 12, RenderingContext::Get()->m_sunDirection );

	Renderer::Get().SetVertexShaderConstant( 13, CDepthHazeSetting::GetAConsts() );
	Renderer::Get().SetVertexShaderConstant( 14, CDepthHazeSetting::GetGConsts() );
	Renderer::Get().SetVertexShaderConstant( 15, CDepthHazeSetting::GetBeta1PlusBeta2() );
	Renderer::Get().SetVertexShaderConstant( 16, CDepthHazeSetting::GetOneOverBeta1PlusBeta2() );
	Renderer::Get().SetVertexShaderConstant( 17, CDepthHazeSetting::GetBetaDash1() );
	Renderer::Get().SetVertexShaderConstant( 18, CDepthHazeSetting::GetBetaDash2() );
	Renderer::Get().SetVertexShaderConstant( 19, CDepthHazeSetting::GetSunColour() );

	// for animation
	CVector NumTextures( _R(m_pSurfaceAtlas->GetNumEntries()), 0.0f, 0.0f, 0.0f );
	Renderer::Get().SetVertexShaderConstant( 20, NumTextures );

	CVector TexWidth( m_pSurfaceAtlas->GetEntryByIndex(0)->GetWidth(), 0.0f, 0.0f, 0.0f );
	Renderer::Get().SetVertexShaderConstant( 21, TexWidth );

	// required for per pixel lighting
	Renderer::Get().SetPixelShaderConstant( 0, RenderingContext::Get()->m_toKeyLight );
	Renderer::Get().SetPixelShaderConstant( 1, RenderingContext::Get()->m_keyColour );
	
	// Gaaaaaah! WTF???? i dont know why i need to flip the camera Z here.
	CMatrix viewToWorld = camMat;
	viewToWorld.SetZAxis( -viewToWorld.GetZAxis() );
	Renderer::Get().SetPixelShaderConstant( 2, viewToWorld );

	// SH coeffs	
	Renderer::Get().SetPixelShaderConstant( 6, RenderingContext::Get()->m_SHMatrices.m_aChannelMats, 12 );

	// For shadow map lookup
	CVector posToUV( CONSTRUCT_CLEAR );
	posToUV.X() = 1.f / (Renderer::Get().m_targetCache.GetWidth() - 1);
	posToUV.Y() = 1.f / (Renderer::Get().m_targetCache.GetHeight() - 1);

	Renderer::Get().SetPixelShaderConstant( 19, posToUV );

	#elif defined(PLATFORM_PS3)

	m_pVertexShader->SetVSConstantByName( "cameraUnitAxisX", cameraUnitAxisX );
	m_pVertexShader->SetVSConstantByName( "cameraUnitAxisY", cameraUnitAxisY );
	m_pVertexShader->SetVSConstantByName( "worldViewProj", worldViewProj );

	// required for depth haze
	m_pVertexShader->SetVSConstantByName( "worldView", RenderingContext::Get()->m_worldToView );

	CPoint viewPos = RenderingContext::Get()->GetEyePos();
	m_pVertexShader->SetVSConstantByName( "viewPosition_objectS", viewPos );
	m_pVertexShader->SetVSConstantByName( "sunDir_objectS", RenderingContext::Get()->m_sunDirection );

	m_pVertexShader->SetVSConstantByName( "DHConstsA", CDepthHazeSetting::GetAConsts() );
	m_pVertexShader->SetVSConstantByName( "DHConstsG", CDepthHazeSetting::GetGConsts() );
	m_pVertexShader->SetVSConstantByName( "DHB1plusB2", CDepthHazeSetting::GetBeta1PlusBeta2() );
	m_pVertexShader->SetVSConstantByName( "DHRecipB1plusB2", CDepthHazeSetting::GetOneOverBeta1PlusBeta2() );
	m_pVertexShader->SetVSConstantByName( "DHBdash1", CDepthHazeSetting::GetBetaDash1() );
	m_pVertexShader->SetVSConstantByName( "DHBdash2", CDepthHazeSetting::GetBetaDash2() );
	m_pVertexShader->SetVSConstantByName( "sunColour", CDepthHazeSetting::GetSunColour() );

	CVector NumTextures( _R(m_pSurfaceAtlas->GetNumEntries()), 0.0f, 0.0f, 0.0f );
	CVector TexWidth( m_pSurfaceAtlas->GetEntryByIndex(0)->GetWidth(), 0.0f, 0.0f, 0.0f );

	if (m_def.m_type == ImpostorDef::I_ANIMATED)
	{
		float fTime = CTimer::Get().GetGameTime() / m_def.m_fCycleTime;
		CVector fraction( (fTime - (int)fTime), 0.0f, 0.0f, 0.0f );

		m_pVertexShader->SetVSConstantByName( "currTimeN", fraction );

		m_pPixelShader->SetPSConstantByName( "TA_numTex", NumTextures );
		m_pPixelShader->SetPSConstantByName( "TA_TexWidth", TexWidth );
	}
	else
	{
		m_pVertexShader->SetVSConstantByName( "TA_numTex", NumTextures );
		m_pVertexShader->SetVSConstantByName( "TA_TexWidth", TexWidth );
	}

	// required for per pixel lighting
	m_pPixelShader->SetPSConstantByName( "g_keyLightDir", RenderingContext::Get()->m_toKeyLight );
	m_pPixelShader->SetPSConstantByName( "g_keyDirColour", RenderingContext::Get()->m_keyColour );
	
	// Gaaaaaah! WTF???? i dont know why i need to flip the camera Z here.
	CMatrix viewToWorld = camMat;
	viewToWorld.SetZAxis( -viewToWorld.GetZAxis() );
	m_pPixelShader->SetPSConstantByName( "viewToWorld", viewToWorld );

	// SH coeffs	
	m_pPixelShader->SetPSConstantByName( "fillSHCoeffs", RenderingContext::Get()->m_SHMatrices.m_aChannelMats, 12 );

	// For shadow map lookup
	CVector posToUV( CONSTRUCT_CLEAR );
	posToUV.X() = 1.f / (Renderer::Get().m_targetCache.GetWidth() - 1);
	posToUV.Y() = 1.f / (Renderer::Get().m_targetCache.GetHeight() - 1);

	m_pPixelShader->SetPSConstantByName( "posToUV", posToUV );

	// discard over this much
	CVector alphatest( m_def.m_fAlphaTest, 0.0f, 0.0f, 0.0f );
	m_pPixelShader->SetPSConstantByName( "alphatest", alphatest );

	Renderer::Get().SetVertexShader( m_pVertexShader, true );
	Renderer::Get().SetPixelShader( m_pPixelShader, true );
	
	m_pSurfaceAtlas->GetAtlasTexture()->m_Platform.GetTexture()->SetGammaCorrect( Gc::kGammaCorrectSrgb );
//	m_surface->m_Platform.GetTexture()->SetLevelBias( -3 );

	#endif

	// we assume that whoever calls our render has alread set / disabled the requried render states
	// so we just need to set texture states.

	Renderer::Get().SetTexture( 0, m_pSurfaceAtlas->GetAtlasTexture(), true );
	Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_WRAPALL );
	Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_BILINEAR );

	Renderer::Get().SetTexture( 1, m_pNormalAtlas->GetAtlasTexture(), true );
	Renderer::Get().SetSamplerAddressMode( 1, TEXTUREADDRESS_WRAPALL );
	Renderer::Get().SetSamplerFilterMode( 1, TEXTUREFILTER_BILINEAR );

	Renderer::Get().SetTexture( 2, RenderingContext::Get()->m_pStencilTarget, true );
	Renderer::Get().SetSamplerAddressMode( 2, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerFilterMode( 2, TEXTUREFILTER_POINT );

	// write our dynamic buffer to the command buffer and render
	m_spriteData.Render( m_num );
}

