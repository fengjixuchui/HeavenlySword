//--------------------------------------------------
//!
//!	\file weaponchains.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "weaponchains.h"
#include "gfx/texturemanager.h"
#include "effect/effect_resourceman.h"
#include "gfx/rendercontext.h"
#include "gfx/shadowsystem.h"
#include "objectdatabase/dataobject.h"
#include "gfx/renderer.h"
#include "gfx/renderable.h"
#include "gfx/graphicsdevice.h"
#include "gfx/depthhazeconsts.h"

START_STD_INTERFACE( WeaponChainDef )

	PUBLISH_VAR_AS(	m_surfTexName, SurfaceTexture )
	PUBLISH_VAR_AS(	m_normTexName, NormalTexture )
	PUBLISH_VAR_AS(	m_fChainWidth, ChainWidth )
	PUBLISH_VAR_AS(	m_fTextureLength, TextureLength )
	PUBLISH_VAR_AS(	m_fSpecularPower, SpecularPower )
	PUBLISH_VAR_AS(	m_fAlphaRef, AlphaRef )
	PUBLISH_VAR_AS(	m_fDiffuseColour, DiffuseColour )
	PUBLISH_VAR_AS(	m_fSpecularColour, SpecularColour )
	
//	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct ) 
//	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )

END_STD_INTERFACE


//--------------------------------------------------
//!
//!	WeaponChainDef::WeaponChainDef
//!
//--------------------------------------------------
WeaponChainDef::WeaponChainDef() :
	m_surfTexName( "vs_chain_gk_01_cc.dds" ),
	m_normTexName( "vs_chain_gk_01_nm.dds" ),
	m_fChainWidth( 0.1f ),
	m_fTextureLength( 0.2f ),
	m_fSpecularPower( 2.0f ),
	m_fAlphaRef( 0.5f ),
	m_fDiffuseColour( 1.0f, 1.0f, 1.0f, 1.0f ),
	m_fSpecularColour( 1.0f, 1.0f, 1.0f, 1.0f )
{}

//--------------------------------------------------
//!
//!	WeaponChain::ctor
//!
//--------------------------------------------------
WeaponChain::WeaponChain( u_int iNumVerts, const WeaponChainDef* pDef ) :
	m_pDef(pDef),
	m_bOwnsDef(false)
{
	if (!m_pDef)
	{
		m_pDef = NT_NEW_CHUNK( Mem::MC_EFFECTS ) WeaponChainDef;
		m_bOwnsDef = true;
	}

	m_iNumVerts = iNumVerts;
	ntAssert( m_iNumVerts > 1 );

	m_bVBInvalid = true;
	// [scee_st] didn't use ARRAY before chunking
	m_pVerts = NT_NEW_ARRAY_CHUNK( Mem::MC_EFFECTS ) CPoint [m_iNumVerts];
	for (u_int i = 0; i < m_iNumVerts; i++)
		m_pVerts[i].Clear();

	m_pSurfaceTexture = TextureManager::Get().LoadTexture_Neutral( ntStr::GetString(m_pDef->m_surfTexName) );
	m_pNormalTexture = TextureManager::Get().LoadTexture_Neutral( ntStr::GetString(m_pDef->m_normTexName) );

	// setup procedural vertex buffer
	m_VB.PushVertexElement( VD_STREAM_TYPE_FLOAT3, VE_POSITON,		"input.position_objectS" );
	m_VB.PushVertexElement( VD_STREAM_TYPE_FLOAT3, VE_NORMAL,		"input.normal_objectS" );
	m_VB.PushVertexElement( VD_STREAM_TYPE_FLOAT3, VE_TANGENT,		"input.tangent_objectS" );
	m_VB.PushVertexElement( VD_STREAM_TYPE_FLOAT3, VE_BINORMAL,		"input.binormal_objectS" );
	m_VB.PushVertexElement( VD_STREAM_TYPE_FLOAT2, VE_NMAP_TCOORD,	"input.normalMapTexcoord" );
	m_VB.PushVertexElement( VD_STREAM_TYPE_FLOAT2, VE_SURF_TCOORD,	"input.diffuseTexcoord0" );

	// allocate memory and clear it
	m_VB.BuildMe( 2 * m_iNumVerts );
	ntAssert( m_VB.GetVertexSize() == sizeof( Vertex ) );
	memset( m_VB.GetVertex(0), 0, m_VB.GetVertexSize() * 2 * m_iNumVerts );

#ifdef PLATFORM_PC

	// find our fx handle
	m_ppFX = EffectResourceMan::Get().RetrieveFXHandle( "rangestancechain" );

#else

	m_pVertexShader_Colour = DebugShaderCache::Get().LoadShader( "weaponchains_vp.sho" );
	m_pVertexShader_Depth = DebugShaderCache::Get().LoadShader( "weaponchains_depth_vp.sho" );
	m_pVertexShader_ShadowCast = DebugShaderCache::Get().LoadShader( "weaponchains_shadow_vp.sho" );

	m_pPixelShader_Colour = DebugShaderCache::Get().LoadShader( "weaponchains_fp.sho" );
	m_pPixelShader_Depth = DebugShaderCache::Get().LoadShader( "weaponchains_depth_fp.sho" );

#endif
}

WeaponChain::~WeaponChain()
{
	NT_DELETE_ARRAY_CHUNK( Mem::MC_EFFECTS, m_pVerts );
	if (m_bOwnsDef)
	{
		NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_pDef );
	}
}

//--------------------------------------------------
//!
//!	WeaponChain::SetVertexPosition
//!
//--------------------------------------------------
void WeaponChain::SetVertexPosition( const CPoint& pos, u_int iIndex )
{
	ntAssert( iIndex < m_iNumVerts );
	m_pVerts[ iIndex ] = pos;
	m_bVBInvalid = true;
}

//--------------------------------------------------
//!
//!	WeaponChain::BuildBasisVecs
//!
//--------------------------------------------------
float WeaponChain::BuildBasisVecs(	const CPoint& startPos,
									const CPoint& endPos,
									const CPoint& eyePos,
									CDirection& normal,
									CDirection& binormal,
									CDirection& tangent )
{
	tangent = (endPos ^ startPos);
	float fTanLen = tangent.Length();
	tangent.Normalise();

	CDirection eyeRay = ((endPos + startPos)*0.5f) ^ eyePos;
	binormal = tangent.Cross( eyeRay );
	binormal.Normalise();

	normal = tangent.Cross( binormal );
	normal.Normalise();

	return fTanLen;
}								

//--------------------------------------------------
//!
//!	WeaponChain::BuildVertexBuffer
//!
//--------------------------------------------------
void WeaponChain::BuildVertexBuffer()
{
	if (!m_bVBInvalid)
		return;

	CPoint eyePos = RenderingContext::Get()->GetEyePos();
	CDirection prev_normal, prev_binormal, prev_tangent;

	float fTexCoord = 0.0f;

	for (u_int i = 0; i < (m_iNumVerts - 1); i++ )
	{
		// calc basis vecs for this link
		CDirection normal, binormal, tangent;
		float fLinkLength = BuildBasisVecs( m_pVerts[i], m_pVerts[i+1], eyePos,
							normal, binormal, tangent );

		// get averaged basis vecs
		CDirection ave_normal(normal);
		CDirection ave_binormal(binormal);
		CDirection ave_tangent(tangent);

		if (i > 0)
		{
			ave_normal = (ave_normal + prev_normal) * 0.5f;
			ave_binormal = (ave_binormal + prev_binormal) * 0.5f;
			ave_tangent = (ave_tangent + prev_tangent) * 0.5f;
		}

		// set actual VB data for this edge
		//-----------------------------------
		Vertex temp;
		temp.normal[0] = ave_normal.X(); temp.normal[1] = ave_normal.Y(); temp.normal[2] = ave_normal.Z();
		temp.binormal[0] = ave_binormal.X(); temp.binormal[1] = ave_binormal.Y(); temp.binormal[2] = ave_binormal.Z();
		temp.tangent[0] = ave_tangent.X(); temp.tangent[1] = ave_tangent.Y(); temp.tangent[2] = ave_tangent.Z();

		// top vertex
		CPoint newPos =  m_pVerts[i] + (ave_binormal * m_pDef->m_fChainWidth * 0.5f);
		temp.pos[0] = newPos.X(); temp.pos[1] = newPos.Y(); temp.pos[2] = newPos.Z();
		temp.nmaptex[0] = temp.surftex[0] = 0.0f;
		temp.nmaptex[1] = temp.surftex[1] = fTexCoord;
		m_VB.SetVertex( i * 2, &temp );

		// bottom vertex
		newPos = m_pVerts[i] - (ave_binormal * m_pDef->m_fChainWidth * 0.5f);
		temp.pos[0] = newPos.X(); temp.pos[1] = newPos.Y(); temp.pos[2] = newPos.Z();
		temp.nmaptex[0] = temp.surftex[0] = 1.0f;
		temp.nmaptex[1] = temp.surftex[1] = fTexCoord;
		m_VB.SetVertex( (i * 2)+1, &temp );

		// update our tex coord factor
		fTexCoord += fLinkLength / m_pDef->m_fTextureLength;

		// cache basis for next edge
		prev_normal = normal;
		prev_binormal = binormal;
		prev_tangent = tangent;
	}

	// fill in the final edge
	//------------------------------
	Vertex temp;
	temp.normal[0] = prev_normal.X(); temp.normal[1] = prev_normal.Y(); temp.normal[2] = prev_normal.Z();
	temp.binormal[0] = prev_binormal.X(); temp.binormal[1] = prev_binormal.Y(); temp.binormal[2] = prev_binormal.Z();
	temp.tangent[0] = prev_tangent.X(); temp.tangent[1] = prev_tangent.Y(); temp.tangent[2] = prev_tangent.Z();

	// top vertex
	CPoint newPos =  m_pVerts[m_iNumVerts-1] + (prev_binormal * m_pDef->m_fChainWidth * 0.5f);
	temp.pos[0] = newPos.X(); temp.pos[1] = newPos.Y(); temp.pos[2] = newPos.Z();
	temp.nmaptex[0] = temp.surftex[0] = 0.0f;
	temp.nmaptex[1] = temp.surftex[1] = fTexCoord;
	m_VB.SetVertex( (m_iNumVerts * 2)-2, &temp );

	// bottom vertex
	newPos = m_pVerts[m_iNumVerts-1] - (prev_binormal * m_pDef->m_fChainWidth * 0.5f);
	temp.pos[0] = newPos.X(); temp.pos[1] = newPos.Y(); temp.pos[2] = newPos.Z();
	temp.nmaptex[0] = temp.surftex[0] = 1.0f;
	temp.nmaptex[1] = temp.surftex[1] = fTexCoord;
	m_VB.SetVertex( (m_iNumVerts * 2)-1, &temp );

	m_bVBInvalid = false;
}

//--------------------------------------------------
//!
//!	WeaponChain::RenderMaterial
//! set alpha blending states, assuming were in the solid rendering slot
//!
//--------------------------------------------------
void WeaponChain::RenderMaterial()
{
#ifdef PLATFORM_PC
	Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );
	InternalRender( false, false, false );
	Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );
#else
	InternalRender( false, false, false );
#endif
}
//--------------------------------------------------
//!
//!	WeaponChain::RenderDepth
//!
//--------------------------------------------------

void WeaponChain::RenderDepth()
{
#ifdef PLATFORM_PC
	Renderer::Get().SetAlphaTestModeN( GFX_ALPHATEST_GREATER, m_pDef->m_fAlphaRef );
	InternalRender( true, false, false );
	Renderer::Get().SetAlphaTestMode( GFX_ALPHATEST_NONE );
#else
	InternalRender( true, false, false );
#endif
}

//--------------------------------------------------
//!
//!	WeaponChain::RenderShadowMap
//!
//--------------------------------------------------

void WeaponChain::RenderShadowMap()
{
#ifdef PLATFORM_PC
	Renderer::Get().SetAlphaTestModeN( GFX_ALPHATEST_GREATER, m_pDef->m_fAlphaRef );
	InternalRender( true, false, false );
	Renderer::Get().SetAlphaTestMode( GFX_ALPHATEST_NONE );
#else
	InternalRender( false, true, false );
#endif
}

//--------------------------------------------------
//!
//!	WeaponChain::RenderRecieveShadowMap
//!
//--------------------------------------------------
void WeaponChain::RenderRecieveShadowMap()
{
#ifdef PLATFORM_PC
	Renderer::Get().SetAlphaTestModeN( GFX_ALPHATEST_GREATER, m_pDef->m_fAlphaRef );
	InternalRender( false, false, true );
	Renderer::Get().SetAlphaTestMode( GFX_ALPHATEST_NONE );
#endif
	// we do not support this technique on PS3, as we rely on the fact we can render depth
	// and thus recieve shadows in the full screen recieve pass
}

//--------------------------------------------------
//!
//!	WeaponChain::InternalRender
//! Actually does the work
//!
//---
#ifdef PLATFORM_PC

void WeaponChain::InternalRender( bool bDepth, bool bShadowMap, bool bRecieveShadow )
{
	if (m_bVBInvalid)
		BuildVertexBuffer();

	D3DXHANDLE h;

	// upload info to GPU
	//----------------------------------------------
	m_VB.SubmitToGPU();


	if( bDepth )
	{
		// the render states is set to no pixel shader output
		(*m_ppFX)->SetTechnique( "depth_output" );
	} else if( bShadowMap )
	{
		if( m_FrameFlags & (CRenderable::RFF_CAST_SHADOWMAP0 | 
							CRenderable::RFF_CAST_SHADOWMAP1 | 
							CRenderable::RFF_CAST_SHADOWMAP2 |
							CRenderable::RFF_CAST_SHADOWMAP3) )
		{
			// currently the same as depth 
			// TODO doesn't read the diffuse texture for alpha test
			(*m_ppFX)->SetTechnique( "depth_output" );
		} else
		{
			return;
		}
	} else if( bRecieveShadow )
	{
		if( m_FrameFlags & (CRenderable::RFF_RECIEVES_SHADOWMAP0 | 
							CRenderable::RFF_RECIEVES_SHADOWMAP1 | 
							CRenderable::RFF_RECIEVES_SHADOWMAP2 |
							CRenderable::RFF_RECIEVES_SHADOWMAP3) )
		{
			(*m_ppFX)->SetTechnique( "recieve_shadowSMH" );
		} else
		{
			return;
		}
	} else
	{
		// set correct FX material techiniqe
		// --------------------------------------------
		if ( CShadowSystemController::Get().IsShadowMapActive() )
			(*m_ppFX)->SetTechnique( "getshadowterm" );
		else
			(*m_ppFX)->SetTechnique( "notshadowed" );

		// these setting are only needed for the proper material render
		FX_SET_VALUE_VALIDATE( (*m_ppFX),
								FXMaterial::GetFXPropertyTagString( PROPERTY_SPECULAR_POWER ),
								&m_pDef->m_fSpecularPower, sizeof(float) * 1 );

		FX_SET_VALUE_VALIDATE( (*m_ppFX),
								FXMaterial::GetFXPropertyTagString( PROPERTY_SPECULAR_COLOUR ),
								&m_pDef->m_fSpecularColour, sizeof(float) * 3 );
	
		FX_GET_HANDLE_FROM_NAME( (*m_ppFX), h, FXMaterial::GetFXPropertyTagString( TEXTURE_NORMAL_MAP ) );
		(*m_ppFX)->SetTexture( h, m_pNormalTexture->m_Platform.Get2DTexture() );
	}

	Renderer::Get().SetCullMode( GFX_CULLMODE_NONE );

	// upload FX paramters
	//----------------------------------------------
	FXMaterial::UploadGlobalParameters( (*m_ppFX) );
	FXMaterial::UploadObjectParameters( (*m_ppFX), CVecMath::GetIdentity(), CVecMath::GetIdentity() );

	FX_SET_VALUE_VALIDATE( (*m_ppFX),
							FXMaterial::GetFXPropertyTagString( PROPERTY_DIFFUSE_COLOUR0 ),
							&m_pDef->m_fDiffuseColour, sizeof(float) * 4 );


	FX_GET_HANDLE_FROM_NAME( (*m_ppFX), h, FXMaterial::GetFXPropertyTagString( TEXTURE_DIFFUSE0 ) );
	(*m_ppFX)->SetTexture( h, m_pSurfaceTexture->m_Platform.Get2DTexture() );


	// draw our primitives
	//-------------------------------------
	u_int iNumPasses;
	(*m_ppFX)->Begin( &iNumPasses, 0 );

	ntError_p( iNumPasses == 1, ("Multipass not supported yet") );
	(*m_ppFX)->BeginPass(0);

	m_VB.SubmitToGPU();	// submit verts to GPU

	if( bShadowMap )
	{
		HRESULT hr;
		if( m_FrameFlags & CRenderable::RFF_CAST_SHADOWMAP0 )
		{
			Renderer::Get().SetVertexShaderConstant( 1, RenderingContext::Get()->m_shadowMapProjection[0] );
			Renderer::Get().SetScissorRegion( &RenderingContext::Get()->m_shadowScissor[0] );
			hr = GetD3DDevice()->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 * (m_iNumVerts-1) );
			ntAssert( hr == S_OK );
		} 
		if( m_FrameFlags & CRenderable::RFF_CAST_SHADOWMAP1 )
		{
			Renderer::Get().SetVertexShaderConstant( 1, RenderingContext::Get()->m_shadowMapProjection[1] );
			Renderer::Get().SetScissorRegion( &RenderingContext::Get()->m_shadowScissor[1] );
			hr = GetD3DDevice()->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 * (m_iNumVerts-1) );
			ntAssert( hr == S_OK );
		} 
		if( m_FrameFlags & CRenderable::RFF_CAST_SHADOWMAP2 )
		{
			Renderer::Get().SetVertexShaderConstant( 1, RenderingContext::Get()->m_shadowMapProjection[2] );
			Renderer::Get().SetScissorRegion( &RenderingContext::Get()->m_shadowScissor[2] );
			hr = GetD3DDevice()->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 * (m_iNumVerts-1) );
			ntAssert( hr == S_OK );
		} 
		if( m_FrameFlags & CRenderable::RFF_CAST_SHADOWMAP3 )
		{
			Renderer::Get().SetVertexShaderConstant( 1, RenderingContext::Get()->m_shadowMapProjection[3] );
			Renderer::Get().SetScissorRegion( &RenderingContext::Get()->m_shadowScissor[3] );
			hr = GetD3DDevice()->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 * (m_iNumVerts-1) );
			ntAssert( hr == S_OK );
		} 
	}
	else
	{
		HRESULT hr;
		hr = GetD3DDevice()->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 * (m_iNumVerts-1) );

		ntAssert( hr == S_OK );
	}

	// cleanup
	//-------------------------------------
	(*m_ppFX)->EndPass();
	(*m_ppFX)->End();

	Renderer::Get().SetCullMode( GFX_CULLMODE_NORMAL );
}

#else

void WeaponChain::InternalRender( bool bDepth, bool bShadowMap, bool )
{
	//! NOTE! when using heresy, our caching is broken, hence the force
	// methods on set Pixel shader and set Vertex shader
	Shader* pVertexShader;
	Shader* pPixelShader;

	if( bDepth )
	{
		pVertexShader = m_pVertexShader_Depth;
		pPixelShader = m_pPixelShader_Depth;

		pVertexShader->SetVSConstantByName( "m_worldViewProj", RenderingContext::Get()->m_worldToScreen, 4 );

		CVector alphaRef( m_pDef->m_fAlphaRef, 0.0f, 0.0f, 0.0f );
		pPixelShader->SetPSConstantByName( "m_fAlphaRef", alphaRef );
	}
	else if( bShadowMap )
	{
		if( m_FrameFlags & (CRenderable::RFF_CAST_SHADOWMAP0 | 
							CRenderable::RFF_CAST_SHADOWMAP1 | 
							CRenderable::RFF_CAST_SHADOWMAP2 |
							CRenderable::RFF_CAST_SHADOWMAP3) )
		{
			pVertexShader = m_pVertexShader_ShadowCast;
			pPixelShader = m_pPixelShader_Depth;

			pVertexShader->SetVSConstantByName( "m_worldViewProj", RenderingContext::Get()->m_worldToScreen, 4 );

			CVector alphaRef( m_pDef->m_fAlphaRef, 0.0f, 0.0f, 0.0f );
			pPixelShader->SetPSConstantByName( "m_fAlphaRef", alphaRef );
		}
		else
			return;
	}
	else
	{
		pVertexShader = m_pVertexShader_Colour;
		pPixelShader = m_pPixelShader_Colour;

		// vertex shader params
		CMatrix mat( CONSTRUCT_IDENTITY );
		pVertexShader->SetVSConstantByName( "g_fillSHCoeffs", RenderingContext::Get()->m_SHMatrices.m_aChannelMats, 12 );
		pVertexShader->SetVSConstantByName( "m_world", mat, 4 );
		pVertexShader->SetVSConstantByName( "m_worldViewProj", RenderingContext::Get()->m_worldToScreen, 4 );

		pVertexShader->SetVSConstantByName( "m_worldView", RenderingContext::Get()->m_worldToView, 4 );
		pVertexShader->SetVSConstantByName( "m_viewPosition_objectS", RenderingContext::Get()->GetEyePos() );
		pVertexShader->SetVSConstantByName( "m_sunDir_objectS", RenderingContext::Get()->m_sunDirection );

		pVertexShader->SetVSConstantByName( "g_DHConstsA", CDepthHazeSetting::GetAConsts() );
		pVertexShader->SetVSConstantByName( "g_DHConstsG", CDepthHazeSetting::GetGConsts() );
		pVertexShader->SetVSConstantByName( "g_DHB1plusB2", CDepthHazeSetting::GetBeta1PlusBeta2() );
		pVertexShader->SetVSConstantByName( "g_DHRecipB1plusB2", CDepthHazeSetting::GetOneOverBeta1PlusBeta2() );
		pVertexShader->SetVSConstantByName( "g_DHBdash1", CDepthHazeSetting::GetBetaDash1() );
		pVertexShader->SetVSConstantByName( "g_DHBdash2", CDepthHazeSetting::GetBetaDash2() );
		pVertexShader->SetVSConstantByName( "g_sunColour", CDepthHazeSetting::GetSunColour() );

		pVertexShader->SetVSConstantByName( "m_keyLightDir_objectS", RenderingContext::Get()->m_toKeyLight );

		// pixel shader params
		CVector specular( m_pDef->m_fSpecularPower, 0.0f, 0.0f, 0.0f );
		pPixelShader->SetPSConstantByName( "m_specularPower", specular );
		pPixelShader->SetPSConstantByName( "m_specularColour", m_pDef->m_fSpecularColour );
		pPixelShader->SetPSConstantByName( "m_diffuseColour0", m_pDef->m_fDiffuseColour );

		pPixelShader->SetPSConstantByName( "g_keyDirColour", RenderingContext::Get()->m_keyColour );
		
		CVector scaleAndBias( 0.02f, -0.01f, 0.0f, 0.0f );
		pPixelShader->SetPSConstantByName( "g_parallaxScaleAndBias", scaleAndBias );

		Renderer::Get().SetTexture( 0, m_pNormalTexture, true );
		Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_WRAPALL );
		Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_BILINEAR );

		CVector posToUV( CONSTRUCT_CLEAR );
		posToUV.X() = 1.f / (Renderer::Get().m_targetCache.GetWidth() - 1);
		posToUV.Y() = 1.f / (Renderer::Get().m_targetCache.GetHeight() - 1);

		pPixelShader->SetPSConstantByName( "g_VPOStoUV", posToUV );

		Renderer::Get().SetTexture( 5, RenderingContext::Get()->m_pStencilTarget, true );
		Renderer::Get().SetSamplerAddressMode( 5, TEXTUREADDRESS_CLAMPALL );
		Renderer::Get().SetSamplerFilterMode( 5, TEXTUREFILTER_POINT );

		CVector alphaRef( m_pDef->m_fAlphaRef, 0.0f, 0.0f, 0.0f );
		pPixelShader->SetPSConstantByName( "m_fAlphaRef", alphaRef );
	}

	if (m_bVBInvalid)
		BuildVertexBuffer();

	// Set renderstates and other parameters
	//----------------------------------------------
	Renderer::Get().SetCullMode( GFX_CULLMODE_NONE );

	m_pSurfaceTexture->m_Platform.GetTexture()->SetLevelBias( -3 );
	m_pSurfaceTexture->m_Platform.GetTexture()->SetGammaCorrect( Gc::kGammaCorrectSrgb );

	Renderer::Get().SetTexture( 2, m_pSurfaceTexture, true );
	Renderer::Get().SetSamplerAddressMode( 2, TEXTUREADDRESS_WRAPALL );
	Renderer::Get().SetSamplerFilterMode( 2, TEXTUREFILTER_BILINEAR );

	// draw our primitives
	//-------------------------------------
	Renderer::Get().SetPixelShader( pPixelShader, true );

	int toDraw = m_iNumVerts*2;

	if( bShadowMap )
	{
		if( m_FrameFlags & CRenderable::RFF_CAST_SHADOWMAP0 )
		{
			pVertexShader->SetVSConstantByName( "m_shadowMapMat", RenderingContext::Get()->m_shadowMapProjection[0], 4 );
			Renderer::Get().SetVertexShader( pVertexShader, true );
			Renderer::Get().SetScissorRegion( &RenderingContext::Get()->m_shadowScissor[0] );

			if (m_VB.CanSubmitToGPU())
			{
				m_VB.SubmitToGPU();
				Renderer::Get().m_Platform.DrawPrimitives( Gc::kTriangleStrip, 0, toDraw );
			}
		} 
		if( m_FrameFlags & CRenderable::RFF_CAST_SHADOWMAP1 )
		{
			pVertexShader->SetVSConstantByName( "m_shadowMapMat1", RenderingContext::Get()->m_shadowMapProjection[1], 4 );
			Renderer::Get().SetVertexShader( pVertexShader, true );
			Renderer::Get().SetScissorRegion( &RenderingContext::Get()->m_shadowScissor[1] );
			
			if (m_VB.CanSubmitToGPU())
			{
				m_VB.SubmitToGPU();
				Renderer::Get().m_Platform.DrawPrimitives( Gc::kTriangleStrip, 0, toDraw );
			}
		} 
		if( m_FrameFlags & CRenderable::RFF_CAST_SHADOWMAP2 )
		{
			pVertexShader->SetVSConstantByName( "m_shadowMapMat2", RenderingContext::Get()->m_shadowMapProjection[2], 4 );
			Renderer::Get().SetVertexShader( pVertexShader, true );
			Renderer::Get().SetScissorRegion( &RenderingContext::Get()->m_shadowScissor[2] );
			
			if (m_VB.CanSubmitToGPU())
			{
				m_VB.SubmitToGPU();
				Renderer::Get().m_Platform.DrawPrimitives( Gc::kTriangleStrip, 0, toDraw );
			}
		} 
		if( m_FrameFlags & CRenderable::RFF_CAST_SHADOWMAP3 )
		{
			pVertexShader->SetVSConstantByName( "m_shadowMapMat3", RenderingContext::Get()->m_shadowMapProjection[3], 4 );
			Renderer::Get().SetVertexShader( pVertexShader, true );
			Renderer::Get().SetScissorRegion( &RenderingContext::Get()->m_shadowScissor[3] );
			
			if (m_VB.CanSubmitToGPU())
			{
				m_VB.SubmitToGPU();
				Renderer::Get().m_Platform.DrawPrimitives( Gc::kTriangleStrip, 0, toDraw );
			}
		} 
	}
	else
	{
		Renderer::Get().SetVertexShader( pVertexShader, true );
		
		if (m_VB.CanSubmitToGPU())
		{
			m_VB.SubmitToGPU();
			Renderer::Get().m_Platform.DrawPrimitives( Gc::kTriangleStrip, 0, toDraw );
		}
	}

	// cleanup
	//-------------------------------------
	Renderer::Get().m_Platform.ClearStreams();
	Renderer::Get().SetCullMode( GFX_CULLMODE_NORMAL );
}

#endif
