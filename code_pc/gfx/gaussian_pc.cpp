/***************************************************************************************************
*
*	DESCRIPTION
*
*	NOTES
*
***************************************************************************************************/

#include "gfx/gaussian.h"
#include "gfx/shader.h"
#include "gfx/renderer.h"
#include "gfx/graphicsdevice.h"
#include "gfx/surfacemanager.h"

CGaussianBlurHelper::CGaussianBlurHelper()
{
	m_pobVertexShader = ShaderManager::Get().FindShader("effects_resample_4tap_vs");
	m_pobPixelShader = ShaderManager::Get().FindShader("effects_resample_4tap_ps");
	D3DVERTEXELEMENT9 stDecl[] = 
	{
		{ 0, 0,					D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0 }, 
		{ 0, 2*sizeof(float),	D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	0 }, 
		D3DDECL_END()
	};
	m_pobDeclaration = CVertexDeclarationManager::Get().GetDeclaration(&stDecl[0]);
}

void CGaussianBlurHelper::SeparableBlur( RenderTarget::Ptr const& pobSource, 
										 RenderTarget::Ptr const& pobTemp, 
										 CVector const& obWeight ) const
{
	// set up renderstates
	Renderer::Get().SetCullMode( GFX_CULLMODE_NONE );

	static const float afVertices[] = 
	{
		-1.0f,  1.0f,		0.0f, 0.0f, 
		3.0f,  1.0f,		2.0f, 0.0f, 
		-1.0f, -3.0f,		0.0f, 2.0f, 
	};

	// blur in two passes
	Renderer::Get().m_targetCache.SetColourTarget( pobTemp );

	Renderer::Get().SetVertexShader( m_pobVertexShader );
	Renderer::Get().m_Platform.SetVertexDeclaration( m_pobDeclaration );
	Renderer::Get().SetPixelShader( m_pobPixelShader );

	for(int iStage = 0; iStage < 4; ++iStage)
	{
		Renderer::Get().SetTexture( iStage, pobSource->GetTexture() );
		Renderer::Get().SetSamplerAddressMode( iStage, TEXTUREADDRESS_CLAMPALL );
		Renderer::Get().SetSamplerFilterMode( iStage, TEXTUREFILTER_BILINEAR );
	}

	static const float fOffset1 = 1.1f, fOffset2 = 4.75;

	const float fHorizOffset = 0.5f/_R(pobSource->GetWidth());
	const float afHorizOffsets[] = 
	{
		-fOffset2*fHorizOffset, 0.0f, 0.0f, 0.0f, 
		-fOffset1*fHorizOffset, 0.0f, 0.0f, 0.0f, 
		fOffset1*fHorizOffset, 0.0f, 0.0f, 0.0f, 
		fOffset2*fHorizOffset, 0.0f, 0.0f, 0.0f, 
	};

	float const fInner = 1.0f/3.0f, fOuter = 1.0f/6.0f;
	CVector const aobWeights[] = 
	{
		fOuter*obWeight, 
		fInner*obWeight, 
		fInner*obWeight, 
		fOuter*obWeight
	};

	Renderer::Get().SetVertexShaderConstant(1, &afHorizOffsets[0], 4);
	Renderer::Get().SetPixelShaderConstant(0, &aobWeights[0], 4);

	GetD3DDevice()->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, &afVertices[0], 4*sizeof(float));

	// second pass
	Renderer::Get().m_targetCache.SetColourTarget( pobSource );

	for(int iStage = 0; iStage < 4; ++iStage)
		Renderer::Get().SetTexture( iStage, pobTemp->GetTexture() );

	const float fVertOffset = 0.5f/_R( pobSource->GetHeight() );
	const float afVertOffsets[] = 
	{
		0.0f, -fOffset2*fVertOffset, 0.0f, 0.0f, 
		0.0f, -fOffset1*fVertOffset, 0.0f, 0.0f, 
		0.0f,  fOffset1*fVertOffset, 0.0f, 0.0f, 
		0.0f,  fOffset2*fVertOffset, 0.0f, 0.0f, 
	};

	Renderer::Get().SetVertexShaderConstant(1, &afVertOffsets[0], 4);

	GetD3DDevice()->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, &afVertices[0], 4*sizeof(float));

	// restore default renderstates
	for(int iStage = 0; iStage < 4; ++iStage)
		Renderer::Get().SetTexture( iStage, Texture::NONE );

	Renderer::Get().SetCullMode( GFX_CULLMODE_NORMAL );
}

void CGaussianBlurHelper::RecursiveBlur( RenderTarget::Ptr& pobSource, RenderTarget::Ptr& pobTemp, CVector const& obWeight, int iPassStart, int iPassEnd ) const
{
	// set up renderstates
	Renderer::Get().SetCullMode( GFX_CULLMODE_NONE );

	static const float afVertices[] = 
	{
		-1.0f,  1.0f,		0.0f, 0.0f, 
		3.0f,  1.0f,		2.0f, 0.0f, 
		-1.0f, -3.0f,		0.0f, 2.0f, 
	};

	CVector const aobWeights[] = 
	{
		0.25f*obWeight, 
		0.25f*obWeight, 
		0.25f*obWeight, 
		0.25f*obWeight, 
	};

	// blur this pass
	Renderer::Get().SetVertexShader( m_pobVertexShader );
	Renderer::Get().m_Platform.SetVertexDeclaration( m_pobDeclaration );
	Renderer::Get().SetPixelShader( m_pobPixelShader );

	for( int iPass = iPassStart; iPass < iPassEnd; ++iPass )
	{
		// set the target
		Renderer::Get().m_targetCache.SetColourTarget( pobTemp );

		for(int iStage = 0; iStage < 4; ++iStage)
		{
			Renderer::Get().SetTexture( iStage, pobSource->GetTexture() );
			Renderer::Get().SetSamplerAddressMode( iStage, TEXTUREADDRESS_CLAMPALL );
			Renderer::Get().SetSamplerFilterMode( iStage, TEXTUREFILTER_BILINEAR );
		}

		float fOffset;
		if( iPass > 0 )
			fOffset = powf( 1.5f, _R( iPass - 1 ) );
		else
			fOffset = 0.5f;

		const float fHorizOffset = fOffset/_R( pobSource->GetWidth() );
		const float fVertOffset = fOffset/_R( pobSource->GetHeight() );
		const float afOffsets[] = 
		{
			-fHorizOffset, -fVertOffset, 0.0f, 0.0f, 
			-fHorizOffset,  fVertOffset, 0.0f, 0.0f, 
			fHorizOffset, -fVertOffset, 0.0f, 0.0f, 
			fHorizOffset,  fVertOffset, 0.0f, 0.0f, 
		};

		Renderer::Get().SetVertexShaderConstant( 1, afOffsets, 4 );
		Renderer::Get().SetPixelShaderConstant( 0, aobWeights, 4 );

		GetD3DDevice()->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, &afVertices[0], 4*sizeof(float));

		for(int iStage = 0; iStage < 4; ++iStage)
			Renderer::Get().SetTexture( iStage, Texture::NONE );

		// swap the targets
		pobTemp.Swap( pobSource );
	}

	// restore default renderstates
	Renderer::Get().SetCullMode( GFX_CULLMODE_NORMAL );
}





//--------------------------------------------------
//!
//!	NewGaussianBlur::NewGaussianBlur
//! PC/XENON friendly blur helper class
//!
//--------------------------------------------------
NewGaussianBlur::NewGaussianBlur()
{
	m_pVertexShader = ShaderManager::Get().FindShader("effects_resample_4tap_vs");
	m_pPixelShader = ShaderManager::Get().FindShader("effects_resample_4tap_ps");
	D3DVERTEXELEMENT9 stDecl[] = 
	{
		{ 0, 0,					D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0 }, 
		{ 0, 2*sizeof(float),	D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	0 }, 
		D3DDECL_END()
	};
	m_pDecl = CVertexDeclarationManager::Get().GetDeclaration(&stDecl[0]);
}

//--------------------------------------------------
//!
//!	NewGaussianBlur::RecursiveBlur
//! recursive blur of a texture.
//!	the result ends up replacing pSource
//!
//--------------------------------------------------
void NewGaussianBlur::RecursiveBlur( RenderTarget::Ptr& pSource, D3DFORMAT fmt, const CVector& weight, int iPassStart, int iPassEnd ) const
{
	// set up renderstates
	Renderer::Get().SetCullMode( GFX_CULLMODE_NONE );

	static const float afVertices[] = 
	{
		-1.0f,  1.0f,		0.0f, 0.0f, 
		3.0f,  1.0f,		2.0f, 0.0f, 
		-1.0f, -3.0f,		0.0f, 2.0f, 
	};

	const CVector aWeights[] = 
	{
		0.25f*weight, 
		0.25f*weight, 
		0.25f*weight, 
		0.25f*weight, 
	};

	// blur this pass
	Renderer::Get().SetVertexShader( m_pVertexShader );
	Renderer::Get().m_Platform.SetVertexDeclaration( m_pDecl );
	Renderer::Get().SetPixelShader( m_pPixelShader );

	// get a temporary texture, the same as our src
	RenderTarget::CreationStruct createParams( pSource->GetWidth(), pSource->GetHeight(), fmt, false );
	createParams.bCacheable = pSource->m_Platform.IsCacheable();

	RenderTarget::Ptr pTemp = SurfaceManager::Get().CreateRenderTarget( createParams );

	for( int iPass = iPassStart; iPass < iPassEnd; ++iPass )
	{
		Renderer::Get().m_targetCache.SetColourTarget( pTemp );

		for(int iStage = 0; iStage < 4; ++iStage)
		{
			Renderer::Get().SetTexture( iStage, pSource->GetTexture() );
			Renderer::Get().SetSamplerAddressMode( iStage, TEXTUREADDRESS_CLAMPALL );
			Renderer::Get().SetSamplerFilterMode( iStage, TEXTUREFILTER_BILINEAR );
		}

		float fOffset;
		if( iPass > 0 )
			fOffset = powf( 1.5f, _R( iPass - 1 ) );
		else
			fOffset = 0.5f;

		const float fHorizOffset = fOffset/_R( pSource->GetWidth() );
		const float fVertOffset = fOffset/_R( pSource->GetHeight() );
		const float afOffsets[] = 
		{
			-fHorizOffset, -fVertOffset, 0.0f, 0.0f, 
			-fHorizOffset,  fVertOffset, 0.0f, 0.0f, 
			fHorizOffset, -fVertOffset, 0.0f, 0.0f, 
			fHorizOffset,  fVertOffset, 0.0f, 0.0f, 
		};

		Renderer::Get().SetVertexShaderConstant( 1, afOffsets, 4 );
		Renderer::Get().SetPixelShaderConstant( 0, aWeights, 4 );

		GetD3DDevice()->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, &afVertices[0], 4*sizeof(float));

		for(int iStage = 0; iStage < 4; ++iStage)
			Renderer::Get().SetTexture( iStage, Texture::NONE );

		// swap the targets
		pTemp.Swap( pSource );
	}

	// restore default renderstates
	Renderer::Get().SetCullMode( GFX_CULLMODE_NORMAL );

	SurfaceManager::Get().ReleaseRenderTarget( pTemp );
}
