/***************************************************************************************************
*
*	$Header:: /game/effects.cpp 4     13/08/03 10:39 Simonb                                        $
*
*
*
*	CHANGES
*
*	16/6/2003	SimonB	Created
*
***************************************************************************************************/

#include "anim/transform.h"
#include "effect/effect_sky.h"
#include "gfx/texture.h"
#include "gfx/renderstates.h"
#include "gfx/camera.h"
#include "gfx/renderer.h"
#include "gfx/graphicsdevice.h"
#include "gfx/shader.h"
#include "input/inputhardware.h"
#include "gfx/vertexdeclaration.h"
#include "gfx/rendercontext.h"
#include "gfx/depthhazeconsts.h"

/***************************************************************************************************
*
*	FUNCTION		SkyEffect::SkyEffect
*
*	DESCRIPTION		Creates a skylight effect with the given screenspace grid tessellation.
*
***************************************************************************************************/

SkyEffect::SkyEffect(int iGridWidth, int iGridHeight)
  : m_iGridWidth(iGridWidth), m_iGridHeight(iGridHeight)
{
	// allocate for verts and indices
	m_iNumVertices = (iGridWidth + 1)*(iGridHeight + 1);
	m_iNumIndices = (iGridWidth + 1)*iGridHeight*2 + 2*iGridHeight;
	m_pobVertices = Renderer::Get().m_Platform.CreateStaticVertexBuffer(m_iNumVertices*2*sizeof(float));
	m_pobIndices = Renderer::Get().m_Platform.CreateStaticIndexBuffer(m_iNumIndices*sizeof(u_short));

	// create a grid of vertices
	float* pfVerts;
	m_pobVertices->Lock(0, 0, reinterpret_cast<void**>(&pfVerts), 0);
	for(int iY = 0; iY <= iGridHeight; ++iY)
	{
		for(int iX = 0; iX <= iGridWidth; ++iX)
		{
			*pfVerts++ = _R(iX)/_R(iGridWidth);
			*pfVerts++ = _R(iY)/_R(iGridHeight);
		}
	}
	m_pobVertices->Unlock();

	// create the indices for a single strip over the grid
	u_short* pusIndices;
	m_pobIndices->Lock(0, 0, reinterpret_cast<void**>(&pusIndices), 0);
	for(int iY = 0; iY < iGridHeight; ++iY)
	{
		*pusIndices++ = static_cast<u_short>((iY + 0)*(iGridWidth + 1));
		for(int iX = 0; iX <= iGridWidth; ++iX)
		{
			*pusIndices++ = static_cast<u_short>((iY + 1)*(iGridWidth + 1) + iX);
			*pusIndices++ = static_cast<u_short>((iY + 0)*(iGridWidth + 1) + iX);
		}
		*pusIndices++ = static_cast<u_short>((iY + 1)*(iGridWidth + 1) + iGridWidth);
	}
	m_pobIndices->Unlock();

	m_pDebugVertexShader = NT_NEW DebugShader;
	m_pDebugVertexShader->SetFILEFunction( SHADERTYPE_VERTEX, "fxshaders/skylight_vs.hlsl", "skylight_vs", "vs_2_0" );
	m_pDebugPixelShader = NT_NEW DebugShader;
	m_pDebugPixelShader->SetFILEFunction( SHADERTYPE_PIXEL, "fxshaders/skylight_ps.hlsl", "skylight_ps", "ps_2_0" );

	// get the shaders
//	m_pobVertexShader = ShaderManager::Get().FindShader("skylight_vs");
//	m_pobPixelShader = ShaderManager::Get().FindShader("skylight_ps");
	m_pobVertexShader = m_pDebugVertexShader;
	m_pobPixelShader = m_pDebugPixelShader;

	ntAssert(m_pobVertexShader && m_pobPixelShader);

	// create the declaration
	D3DVERTEXELEMENT9 stDecl[] =
	{
		{ 0, 0,					D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0 }, 
		D3DDECL_END()
	};
	m_pobDecl = CVertexDeclarationManager::Get().GetDeclaration( stDecl );
}

/***************************************************************************************************
*
*	FUNCTION		SkyEffect::dtor
*
***************************************************************************************************/
SkyEffect::~SkyEffect()
{
	NT_DELETE( m_pobVertexShader );
	NT_DELETE( m_pobPixelShader );
}

/***************************************************************************************************
*
*	FUNCTION		SkyEffect::Render
*
*	DESCRIPTION		Renders the sky.
*
***************************************************************************************************/

void SkyEffect::Render()
{
	// bail out on wireframe
	if( CRendererSettings::bShowWireframe )
		return;

	// load the shaders
	Renderer::Get().SetVertexShader(m_pobVertexShader);
	Renderer::Get().SetPixelShader(m_pobPixelShader);
	Renderer::Get().m_Platform.SetVertexDeclaration( m_pobDecl );

	// compute the constants
	CVector aobTransform[] = 
	{
		CVector(2.0f, 0.0f, 0.0f, -1.0f), 
		CVector(0.0f, 2.0f, 0.0f, -1.0f), 
	};

	const float fY = ftanf( RenderingContext::Get()->m_pViewCamera->GetFOVAngle() / 2.0f );
	const float fX = fY*RenderingContext::Get()->m_fScreenAspectRatio;
	CDirection aobFrustum[] = 
	{
		CDirection( fX, -fY, 1.0f), 
		CDirection( fX,  fY, 1.0f), 
		CDirection(-fX, -fY, 1.0f), 
		CDirection(-fX,  fY, 1.0f), 
	};
	for(int iCorner = 0; iCorner < 4; ++iCorner)
		aobFrustum[iCorner] = aobFrustum[iCorner]*RenderingContext::Get()->m_pViewCamera->GetViewTransform()->GetWorldMatrix();

	CDirection aobScreenVectors[] = 
	{
		aobFrustum[0], 
		aobFrustum[2] - aobFrustum[0], 
		aobFrustum[1] - aobFrustum[0], 
	};

	Renderer::Get().SetVertexShaderConstant( 1, &aobTransform[0], 2 );
	Renderer::Get().SetVertexShaderConstant( 3, &aobScreenVectors[0], 3 );

	// identity world space
	Renderer::Get().SetVertexShaderConstant( 224, RenderingContext::Get()->m_worldToView );

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

	// direction of the sun in object space (world=object for sky)
	Renderer::Get().SetPixelShaderConstant( 0, RenderingContext::Get()->m_sunDirection );
	Renderer::Get().SetPixelShaderConstant( 1, obBeta1PlusBeta2 );
	Renderer::Get().SetPixelShaderConstant( 2, obBetaDash1 );
	Renderer::Get().SetPixelShaderConstant( 3, obBetaDash2 );
	Renderer::Get().SetPixelShaderConstant( 4, obRecipBeta1PlusBeta2);
	Renderer::Get().SetPixelShaderConstant( 5, obConstsA );
	const CVector obSunSplit = CDepthHazeSetting::GetSunColour();
	Renderer::Get().SetPixelShaderConstant( 6, obSunSplit );
	Renderer::Get().SetPixelShaderConstant( 7, obConstsG );
	Renderer::Get().SetPixelShaderConstant( 8, RenderingContext::Get()->m_worldToView );
	Renderer::Get().SetPixelShaderConstant( 12, RenderingContext::Get()->m_skyColour);
	// set the streams
	GetD3DDevice()->SetStreamSource(0, m_pobVertices.Get(), 0, 2*sizeof(float));
	GetD3DDevice()->SetIndices(m_pobIndices.Get());

	// render
	GetD3DDevice()->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, m_iNumVertices, 0, m_iNumIndices - 2);

	// clean up
	GetD3DDevice()->SetStreamSource(0, 0, 0, 0);
	GetD3DDevice()->SetIndices(0);
}
