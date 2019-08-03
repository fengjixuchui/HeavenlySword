//--------------------------------------------------
//!
//!	\file NAO32converter_ps3.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "gfx/NAO32converter.h"
#include "gfx/shader.h"
#include "gfx/renderer.h"
#include "gfx/rendercontext.h"
#include "gfx/graphicsdevice.h"

//--------------------------------------------------
//!
//!	NAO32converter_impl
//! 
//--------------------------------------------------
class NAO32converter_impl
{
public:
	NAO32converter_impl( void );
	
	void Convert ( const Texture::Ptr& toBeConverted );

private:
	DebugShader	*	m_vertexShader;
	DebugShader	*	m_pixelShader;
	VBHandle		m_hSimpleQuadData;
};

//--------------------------------------------------
//!
//!	NAO32converter_impl
//! construct
//!
//--------------------------------------------------
NAO32converter_impl::NAO32converter_impl( void )
{
	m_vertexShader = DebugShaderCache::Get().LoadShader( "fullscreen_vp.sho" );
	m_pixelShader = DebugShaderCache::Get().LoadShader( "logYUV_to_RGB_fp.sho" );

	// create screen quad data
	GcStreamField	simpleQuadDesc( FwHashedString( "IN.position" ), 0, Gc::kFloat, 2 ); 
	
	static float const simpleQuad[] = 
	{
		-1.0f,	 1.0f,
		 3.0f,	 1.0f, 
		-1.0f,	-3.0f
	};

	m_hSimpleQuadData = RendererPlatform::CreateVertexStream( 3, sizeof( float ) * 2, 1, &simpleQuadDesc );
	m_hSimpleQuadData->Write( simpleQuad );
}

//--------------------------------------------------
//!
//!	NAO32converter_impl::Convert 
//! Draw a full screen quad: call cg function 
//! _LogYUV_to_RGB on each pixel
//!
//--------------------------------------------------
void NAO32converter_impl::Convert ( const Texture::Ptr& depthTexture )
{
	Renderer::Get().SetVertexShader( m_vertexShader );
	Renderer::Get().SetPixelShader( m_pixelShader );

	// NB we use bilinear here, as we're converting from 
	// MSAA to non-MSAA space aswell in the current usage
	Renderer::Get().SetTexture( 0, depthTexture );
	Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_BILINEAR);

	Renderer::Get().m_Platform.SetStream( m_hSimpleQuadData );

	Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );
	Renderer::Get().SetZBufferMode( GFX_ZMODE_DISABLED );
	Renderer::Get().SetCullMode( GFX_CULLMODE_NONE );
	
	Renderer::Get().m_Platform.DrawPrimitives( Gc::kTriangles, 0, 3 );
	Renderer::Get().m_Platform.ClearStreams();

	Renderer::Get().SetCullMode(GFX_CULLMODE_NORMAL);
	Renderer::Get().SetZBufferMode( GFX_ZMODE_NORMAL );

	Renderer::Get().SetTexture( 0, Texture::NONE );
}


//--------------------------------------------------
//!
//!	NAO32converter
//! Public interface to our colour space converter
//!
//--------------------------------------------------
NAO32converter::NAO32converter()
{
	m_pImpl = NT_NEW_CHUNK(Mem::MC_GFX)( NAO32converter_impl );
}
NAO32converter::~NAO32converter()
{
	NT_DELETE_CHUNK(Mem::MC_GFX, m_pImpl );
}
void NAO32converter::Convert ( const Texture::Ptr& toBeConverted )
{
	m_pImpl->Convert ( toBeConverted );
}
