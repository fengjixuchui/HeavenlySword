//--------------------------------------------------
//!
//!	\file irradiance_ps3.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------


#ifndef GFX_IRRADIANCE_H
#include "gfx/irradiance_ps3.h"
#endif

#ifndef GFX_RENDERER_PS3_H_
#include "gfx/renderer.h"
#endif

#include "gfx/surfacemanager.h"

//--------------------------------------------------
//!
//!	Irradiance Manager
//! constructor
//!
//--------------------------------------------------
IrradianceManager::IrradianceManager()
{
	m_pIrradianceVertexShader = DebugShaderCache::Get().LoadShader( "irradiance_vp.sho" );
	m_pIrradiancePixelShader = DebugShaderCache::Get().LoadShader( "irradiance_fp.sho" );
	m_pDownsampleVertexShader = DebugShaderCache::Get().LoadShader( "irradiance_mipmaps_vp.sho" );
	m_pDownsamplePixelShader = DebugShaderCache::Get().LoadShader( "irradiance_mipmaps_fp.sho" );

	// create screen quad data
	GcStreamField	simpleQuadDescCubeMap[] = 
	{
		GcStreamField( FwHashedString( "IN.position" ), 0, Gc::kFloat, 2 ),
		GcStreamField( FwHashedString( "IN.sampling_direction" ), 8, Gc::kFloat, 3 ),

	};

	// initialize 6 streams
	for (unsigned int i = 0; i < 6; i++)
		m_hSimpleQuadData[i] = RendererPlatform::CreateVertexStream( 3, sizeof( float ) * 5, 2, simpleQuadDescCubeMap );

	static float const simpleQuadPosX[] = 
	{
		// POSITION			// SAMPLING DIRECTION
		-1.0f,	 1.0f,		1.0f,	1.0f,	1.0f,
		 3.0f,	 1.0f,		1.0f,	1.0f,	-3.0f,
		-1.0f,	-3.0f,  	1.0f,	-3.0f,	1.0f,
	};
	m_hSimpleQuadData[0]->Write( simpleQuadPosX );

	static float const simpleQuadNegX[] = 
	{
		// POSITION			// SAMPLING DIRECTION
		-1.0f,	 1.0f,		-1.0f,	1.0f,	-1.0f,
		 3.0f,	 1.0f,		-1.0f,	1.0f,	3.0f,
		-1.0f,	-3.0f,  	-1.0f,	-3.0f,	-1.0f,
	};
	m_hSimpleQuadData[1]->Write( simpleQuadNegX );

	static float const simpleQuadPosY[] = 
	{
		// POSITION			// SAMPLING DIRECTION
		-1.0f,	 1.0f,		-1.0f,	1.0f,	-1.0f,
		 3.0f,	 1.0f,		3.0f,	1.0f,	-1.0f,
		-1.0f,	-3.0f,  	-1.0f,	1.0f,	3.0f,
	};
	m_hSimpleQuadData[2]->Write( simpleQuadPosY );

	static float const simpleQuadNegY[] = 
	{
		// POSITION			// SAMPLING DIRECTION
		-1.0f,	 1.0f,		-1.0f,	-1.0f,	1.0f,
		 3.0f,	 1.0f,		3.0f,	-1.0f,	1.0f,
		-1.0f,	-3.0f,  	-1.0f,	-1.0f,	-3.0f,
	};
	m_hSimpleQuadData[3]->Write( simpleQuadNegY );

	static float const simpleQuadPosZ[] = 
	{
		// POSITION			// SAMPLING DIRECTION
		-1.0f,	 1.0f,		-1.0f,	1.0f,	1.0f,
		 3.0f,	 1.0f,		3.0f,	1.0f,	1.0f,
		-1.0f,	-3.0f,  	-1.0f,	-3.0f,	1.0f,
	};
	m_hSimpleQuadData[4]->Write( simpleQuadPosZ );

	static float const simpleQuadNegZ[] = 
	{
		// POSITION			// SAMPLING DIRECTION
		-1.0f,	 1.0f,		1.0f,	1.0f,	-1.0f,
		 3.0f,	 1.0f,		-3.0f,	1.0f,	-1.0f,
		-1.0f,	-3.0f,  	1.0f,	-3.0f,	-1.0f,
	};
	m_hSimpleQuadData[5]->Write( simpleQuadNegZ );

	// create screen quad data
	GcStreamField	simpleQuadDesc[] = 
	{
		GcStreamField( FwHashedString( "IN.position" ), 0, Gc::kFloat, 2 ),
	};

	m_hSimpleQuadData[6] = RendererPlatform::CreateVertexStream( 3, sizeof( float ) * 2, 1, simpleQuadDesc );

	static float const simpleQuad[] = 
	{
		// POSITION			
		-1.0f,	 1.0f,		
		 3.0f,	 1.0f,		
		-1.0f,	-3.0f,  	
	};
	m_hSimpleQuadData[6]->Write( simpleQuad );

}
//--------------------------------------------------
//!
//!	Irradiance Manager
//! generates a cube map that cache irradiance
//!
//--------------------------------------------------
void IrradianceManager::GenerateIrradianceCubeMap( const Texture::Ptr pCubeMap, const SHChannelMatrices& SHCoeffs )
{
	// check if lighting has changed since the last time
	SHChannelMatrices diffSH;
	float delta = 0;
	for ( unsigned int i = 0; i < 3; i++ )
	{
		for ( unsigned int j = 0; j < 4; j++ )
		{
			diffSH.m_aChannelMats[i].Row(j) = SHCoeffs.m_aChannelMats[i].Row(j) - m_cachedSHCoeffs.m_aChannelMats[i].Row(j);
			delta += diffSH.m_aChannelMats[i].Row(j).Dot( diffSH.m_aChannelMats[i].Row(j) );
		}
	}

static const float fSHEpsilon = 0.001f;

	// if lighting has changed we compute a new cube map
	if ( delta >= fSHEpsilon )
	{
		// cache SH coefficients
		memcpy( (void*)&m_cachedSHCoeffs.m_aChannelMats, (void*)&SHCoeffs, sizeof (m_cachedSHCoeffs.m_aChannelMats) );


		// compute how many mipmap levels we have to generate
		unsigned int  iMipLevels = (unsigned int)( logf( (float)pCubeMap->m_Platform.GetTexture()->GetWidth() ) / logf( 2.0f ) );

		// disable blending, z buffer writes and  back face culling
		Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );
		Renderer::Get().SetZBufferMode( GFX_ZMODE_DISABLED );
		Renderer::Get().SetCullMode( GFX_CULLMODE_NONE );
		Renderer::Get().SetAlphaTestMode(GFX_ALPHATEST_NONE);
	
		// set a vertex and a pixel shader
		m_pIrradianceVertexShader->SetVSConstantByName( "texture_offset", CVector( 0.25f / (float)(2 << iMipLevels  ), 0.0f, 0.0f, 0.0f ) );
		Renderer::Get().SetVertexShader( m_pIrradianceVertexShader );
		m_pIrradiancePixelShader->SetPSConstantByName( "fill_sh", (void*)&m_cachedSHCoeffs.m_aChannelMats, 12 );
		Renderer::Get().SetPixelShader( m_pIrradiancePixelShader );

		// render irradiance into each cube map face
		for ( unsigned int i = 0; i < 6; i++ )
		{
			// get (per cube map face) the first mip level
			GcTextureHandle pTexture = pCubeMap->m_Platform.GetTexture()->GetCubeMipLevel( (Gc::TexCubeFace)i, 0 );
			RenderTarget::Ptr pRendTarget = SurfaceManager::Get().CreateRenderTarget( RenderTarget::CreationStruct( GcRenderBuffer::Create( pTexture ) ) );
			// set render target
			Renderer::Get().m_targetCache.SetColourTarget( pRendTarget );
			// set vertex stream
			Renderer::Get().m_Platform.SetStream( m_hSimpleQuadData[i] );
			// kick rendering!
			Renderer::Get().m_Platform.DrawPrimitives( Gc::kTriangles, 0, 3 );
		}

		// generate mip maps ( we need mip maps if we don't want to be texture bandwidth limited )
		// reset streams
		Renderer::Get().m_Platform.ClearStreams();
		// set new shaders
		Renderer::Get().SetVertexShader( m_pDownsampleVertexShader );
		Renderer::Get().SetPixelShader( m_pDownsamplePixelShader );	
		// set vertex stream
		Renderer::Get().m_Platform.SetStream( m_hSimpleQuadData[6] );
		for ( unsigned int j = 0; j < 6; j++ )
		{
			for ( unsigned int i = 0; i < iMipLevels; i++ )
			{
				m_pDownsampleVertexShader->SetVSConstantByName( "texture_offset", CVector( 0.25f / (float)(2<<(iMipLevels - i) ), 0.0f, 0.0f, 0.0f ) );
				Texture::Ptr pTexture = SurfaceManager::Get().CreateTexture( Texture::CreationStruct( 
										GcRenderBuffer::Create( pCubeMap->m_Platform.GetTexture()->GetCubeMipLevel( (Gc::TexCubeFace)j, i ) ) ,true ) );
		
				Renderer::Get().SetTexture( 0, pTexture );
				Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );
				Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_BILINEAR);
			
				GcTextureHandle pTexturePlusOne = pCubeMap->m_Platform.GetTexture()->GetCubeMipLevel( (Gc::TexCubeFace)j, i + 1 );
				RenderTarget::Ptr pRendTarget = SurfaceManager::Get().CreateRenderTarget( RenderTarget::CreationStruct( GcRenderBuffer::Create( pTexturePlusOne ) ) );
				// set render target
				Renderer::Get().m_targetCache.SetColourTarget( pRendTarget );
				// kick rendering!
				Renderer::Get().m_Platform.DrawPrimitives( Gc::kTriangles, 0, 3 );
			}
		}

		// reset some stuff..
		Renderer::Get().m_Platform.ClearStreams();
		Renderer::Get().SetCullMode(GFX_CULLMODE_NORMAL);
		Renderer::Get().SetZBufferMode( GFX_ZMODE_NORMAL );
		Renderer::Get().SetTexture( 0, Texture::NONE );

	}
}
