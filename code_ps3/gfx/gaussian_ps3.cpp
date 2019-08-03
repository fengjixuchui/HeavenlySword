/***************************************************************************************************
*
*	DESCRIPTION
*
*	NOTES
*
***************************************************************************************************/

#include "gfx/gaussian.h"
#include "gfx/renderer.h"
#include "gfx/graphicsdevice.h"
#include "gfx/surfacemanager.h"

//--------------------------------------------------
//!
//!	NewGaussianBlur::NewGaussianBlur
//! blur helper class
//!
//--------------------------------------------------
NewGaussianBlur::NewGaussianBlur()
{	
	m_vertexShader = DebugShaderCache::Get().LoadShader( "resample_4tap_vp.sho" );
	m_pixelShader = DebugShaderCache::Get().LoadShader( "simpletexture4_lerp_fp.sho" );

	GcStreamField	simpleQuadDesc[] = 
	{
		GcStreamField( FwHashedString( "IN.position" ), 0, Gc::kFloat, 2 ),	// 2 * 4 bytes
		GcStreamField( FwHashedString( "IN.texcoord" ), 8, Gc::kFloat, 2 ),	// 2 * 4 bytes
	};

	float const simpleQuad[] = 
	{
		-1.0f,  1.0f,	0.0f,	0.0f,		// 0
		 1.0f,  1.0f,	1.0f,	0.0f,		// 1
		 1.0f, -1.0f,	1.0f,	1.0f,		// 2
		-1.0f, -1.0f,	0.0f,	1.0f,		// 3
	};

	m_hSimpleQuadData = RendererPlatform::CreateVertexStream( 4, sizeof( float ) * 4, 2, simpleQuadDesc );
	m_hSimpleQuadData->Write( simpleQuad );
}

//--------------------------------------------------
//!
//!	NewGaussianBlur::RecursiveBlur
//! recursive blur of a texture.
//!	the result ends up replacing pSource
//!
//--------------------------------------------------
void NewGaussianBlur::RecursiveBlur( RenderTarget::Ptr& pSource, GFXFORMAT fmt, const CVector& weight, int iPassStart, int iPassEnd )
{
	// set up renderstates
	Renderer::Get().SetCullMode( GFX_CULLMODE_NONE );

	// blur this pass
	Renderer::Get().SetVertexShader( m_vertexShader );

	const CVector aWeights[] = 
	{
		0.25f*weight, 
		0.25f*weight, 
		0.25f*weight, 
		0.25f*weight, 
	};

	m_pixelShader->SetPSConstantByName( "weights", aWeights, 4 );
	
	Renderer::Get().SetPixelShader( m_pixelShader );

	// get a temporary texture, the same as our src
	RenderTarget::Ptr pTemp = SurfaceManager::Get().CreateRenderTarget( 
					RenderTarget::CreationStruct( pSource->GetWidth(), pSource->GetHeight(), fmt ) );

	Renderer::Get().m_Platform.SetStream( m_hSimpleQuadData );

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

		m_vertexShader->SetVSConstantByName( "offsets", afOffsets, 4 );

		Renderer::Get().m_Platform.DrawPrimitives( Gc::kQuads, 0, 4 );

		for(int iStage = 0; iStage < 4; ++iStage)
			Renderer::Get().SetTexture( iStage, Texture::NONE );

		// swap the targets
		pTemp.Swap( pSource );
	}

	// restore default renderstates
	Renderer::Get().SetCullMode( GFX_CULLMODE_NORMAL );
	Renderer::Get().m_Platform.ClearStreams();
	SurfaceManager::Get().ReleaseRenderTarget( pTemp );
}
