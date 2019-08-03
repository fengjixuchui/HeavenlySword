/***************************************************************************************************
*
*	DESCRIPTION
*
*	NOTES
*
***************************************************************************************************/

#include "effect/effect_sky_ps3.h"
#include "anim/transform.h"
#include "gfx/renderer.h"
#include "gfx/rendercontext.h"
#include "gfx/depthhazeconsts.h"

//--------------------------------------------------
//!
//!	SkyEffect::ctor
//! Creates a skylight effect with the given screenspace grid tessellation.
//!
//--------------------------------------------------
SkyEffect::SkyEffect( int iGridWidth, int iGridHeight ) :
	m_iGridWidth(iGridWidth),
	m_iGridHeight(iGridHeight)
{
	m_vertexShader = DebugShaderCache::Get().LoadShader( "skylight_vp.sho" );
	m_pixelShader = DebugShaderCache::Get().LoadShader( "skylight_fp.sho" );

	// allocate for verts and indices
	m_iNumVertices = (iGridWidth + 1)*(iGridHeight + 1);
	m_iNumIndices = (iGridWidth + 1)*iGridHeight*2 + 2*iGridHeight;
	
	GcStreamField desc( FwHashedString( "IN.position" ), 0, Gc::kFloat, 2 );
	u_int iVertSize = sizeof( float ) * 2;

	m_pVertices = RendererPlatform::CreateVertexStream( m_iNumVertices, iVertSize, 1, &desc );
	m_pIndices = RendererPlatform::CreateIndexStream( Gc::kIndex16, m_iNumIndices );

	vertex* pTempVerts = NT_NEW_ARRAY_CHUNK ( Mem::MC_EFFECTS ) vertex[m_iNumVertices];

	// create a grid of vertices
	u_int iWhere = 0;
	for(int iY = 0; iY <= iGridHeight; ++iY)
	{
		for(int iX = 0; iX <= iGridWidth; ++iX)
		{
			pTempVerts[iWhere].position[0] = _R(iX)/_R(iGridWidth);
			pTempVerts[iWhere++].position[1] =  _R(iY)/_R(iGridHeight);
		}
	}
	m_pVertices->Write( pTempVerts );
	NT_DELETE_ARRAY_CHUNK( Mem::MC_EFFECTS, pTempVerts );
	
	// create the indices for a single strip over the grid
	iWhere = 0;
	uint16_t* pTempIndices = NT_NEW_ARRAY_CHUNK ( Mem::MC_EFFECTS ) uint16_t[m_iNumIndices];

	for(int iY = 0; iY < iGridHeight; ++iY)
	{
		pTempIndices[iWhere++] = static_cast<uint16_t>( (iY + 0)*(iGridWidth + 1) );

		for(int iX = 0; iX <= iGridWidth; ++iX)
		{
			pTempIndices[iWhere++] = static_cast<uint16_t>((iY + 1)*(iGridWidth + 1) + iX);
			pTempIndices[iWhere++] = static_cast<uint16_t>((iY + 0)*(iGridWidth + 1) + iX);
		}

		pTempIndices[iWhere++] = static_cast<uint16_t>((iY + 1)*(iGridWidth + 1) + iGridWidth);
	}
	m_pIndices->Write( pTempIndices );
	NT_DELETE_ARRAY_CHUNK( Mem::MC_EFFECTS, pTempIndices );
}

//--------------------------------------------------
//!
//!	SkyEffect::Render
//! Renders the sky
//!
//--------------------------------------------------
void SkyEffect::Render()
{
	// bail out on wireframe
//	if( CRendererSettings::bShowWireframe )
//		return;

	// load the shaders
	Renderer::Get().SetVertexShader( m_vertexShader );

	// compute the constants
	CVector transform[] = 
	{
		CVector(2.0f, 0.0f, 0.0f, -1.0f), 
		CVector(0.0f, 2.0f, 0.0f, -1.0f), 
	};

	const float fY = ftanf( RenderingContext::Get()->m_pViewCamera->GetFOVAngle() / 2.0f );
	const float fX = fY*RenderingContext::Get()->m_fScreenAspectRatio;
	CDirection frustum[] = 
	{
		CDirection( fX, -fY, 1.0f), 
		CDirection( fX,  fY, 1.0f), 
		CDirection(-fX, -fY, 1.0f), 
		CDirection(-fX,  fY, 1.0f), 
	};

	for(int iCorner = 0; iCorner < 4; ++iCorner)
		frustum[iCorner] = frustum[iCorner]*RenderingContext::Get()->m_pViewCamera->GetViewTransform()->GetWorldMatrix();

	CDirection screenVectors[] = 
	{
		frustum[0], 
		frustum[2] - frustum[0], 
		frustum[1] - frustum[0], 
	};

	m_vertexShader->SetVSConstantByName( "transform",		transform,  2 );
	m_vertexShader->SetVSConstantByName( "screenvectors",	screenVectors, 3 );	

	// CONSTS_A = 1 < log2 *e, termMultiplier >
	const CVector obConstsA = CDepthHazeSetting::GetAConsts();

	// we intentionally use seperate ones here
	//----------------------------------------

	// g == Henley/Greenstein phase function eccentricity (Probability a photon has been scattered)
	const float g = RenderingContext::Get()->m_keyLight.m_fHenleyGreensteinEccentricity;

	// CONSTS_G = <(1-g)^2, SunPower, SunMultiplier> 
	const CVector obConstsG( (1-g)*(1-g), RenderingContext::Get()->m_keyLight.m_fSunPower, RenderingContext::Get()->m_keyLight.m_fSunMultiplier,0);

//	const CVector obConstsG = CDepthHazeSetting::GetGConsts();

	// beta_1 = Br = Rayleigh coefficient<RGB> beta_2 = Mie coefficient<RGB>
	const CVector obBeta1PlusBeta2		= CDepthHazeSetting::GetBeta1PlusBeta2();
	const CVector obRecipBeta1PlusBeta2 = CDepthHazeSetting::GetOneOverBeta1PlusBeta2();
	const CVector obBetaDash1 			= CDepthHazeSetting::GetBetaDash1();
	const CVector obBetaDash2 			= CDepthHazeSetting::GetBetaDash2();
	const CVector obSunSplit			= CDepthHazeSetting::GetSunColour();

	m_pixelShader->SetPSConstantByName( "sunDir", RenderingContext::Get()->m_sunDirection );
	m_pixelShader->SetPSConstantByName( "beta1PlusBeta2", obBeta1PlusBeta2 );
	m_pixelShader->SetPSConstantByName( "betaDash1", obBetaDash1 );
	m_pixelShader->SetPSConstantByName( "betaDash2", obBetaDash2 );
	m_pixelShader->SetPSConstantByName( "oneOverBeta1PlusBeta2", obRecipBeta1PlusBeta2);
	m_pixelShader->SetPSConstantByName( "aConsts", obConstsA );
	m_pixelShader->SetPSConstantByName( "sunColour", obSunSplit );
	m_pixelShader->SetPSConstantByName( "gConsts", obConstsG );
	m_pixelShader->SetPSConstantByName( "worldViewMatrix", RenderingContext::Get()->m_worldToView, 4 );
	m_pixelShader->SetPSConstantByName( "skyColour", RenderingContext::Get()->m_skyColour);

	Renderer::Get().SetPixelShader( m_pixelShader );

	// set the streams
	Renderer::Get().m_Platform.SetStream( m_pVertices );

	// render
	Renderer::Get().m_Platform.DrawIndexedPrimitives( Gc::kTriangleStrip, 0, m_iNumIndices, m_pIndices );
	Renderer::Get().m_Platform.ClearStreams();
}
