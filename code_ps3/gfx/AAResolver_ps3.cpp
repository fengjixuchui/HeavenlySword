//--------------------------------------------------
//!
//!	\file AARresolver_ps3.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "gfx/AAResolver.h"
#include "gfx/shader.h"
#include "gfx/renderer.h"
#include "gfx/rendercontext.h"
#include "gfx/graphicsdevice.h"
#include "input/inputhardware.h"

//--------------------------------------------------
//!
//!	AAResolver_impl
//! 
//--------------------------------------------------
class AAResolver_impl
{
public:
	AAResolver_impl( void );
	
	void Resolve( const RenderTarget::Ptr& pSourceBuffer, const AAResolveMode eAAResMode );

private:
	DebugShader	*	m_vertexShader;
	DebugShader	*	m_bilinearFiltering_PS;
	DebugShader	*	m_gaussianFiltering_PS;
	VBHandle		m_hSimpleQuadData;
};

//--------------------------------------------------
//!
//!	AAResolver_impl
//! construct
//!
//--------------------------------------------------
AAResolver_impl::AAResolver_impl( void )
{
	m_vertexShader = DebugShaderCache::Get().LoadShader( "aaresolver_vp.sho" );
	m_bilinearFiltering_PS = DebugShaderCache::Get().LoadShader( "aaresolver_bilinear_fp.sho" );
	m_gaussianFiltering_PS = DebugShaderCache::Get().LoadShader(  "aaresolver_gaussian_fp.sho" );

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
//!	AAResolver_impl::BilinearFiltering
//! Perform a very simple bilinear filtering to downsample a multisampled buffer
//!
//--------------------------------------------------

void AAResolver_impl::Resolve ( const RenderTarget::Ptr& pSourceBuffer, const AAResolveMode eAAResMode )
{
	Renderer::Get().SetVertexShader( m_vertexShader );

	Renderer::Get().m_Platform.SetStream( m_hSimpleQuadData );

	Renderer::Get().SetTexture( 0, pSourceBuffer->GetTexture() );
	Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_BILINEAR);

	if (eAAResMode == AARES_GAUSSIAN_FILTER) Renderer::Get().SetPixelShader( m_gaussianFiltering_PS );
	else Renderer::Get().SetPixelShader( m_bilinearFiltering_PS );

	Renderer::Get().SetZBufferMode( GFX_ZMODE_DISABLED );

	Renderer::Get().m_Platform.DrawPrimitives( Gc::kTriangles, 0, 3 );
	Renderer::Get().m_Platform.ClearStreams();

	// cleanup
	//-------------------------------------------------------------
	Renderer::Get().SetTexture( 0, Texture::NONE );

	Renderer::Get().SetZBufferMode( GFX_ZMODE_NORMAL );
}

//--------------------------------------------------
//!
//!	AAResolver
//! Public interface to our GPU based resolver
//!
//--------------------------------------------------
AAResolver::AAResolver()
{
	m_pImpl = NT_NEW_CHUNK(Mem::MC_GFX) ( AAResolver_impl );
}
AAResolver::~AAResolver()
{
	NT_DELETE_CHUNK(Mem::MC_GFX, m_pImpl );
}
void AAResolver::Resolve( const RenderTarget::Ptr& pSourceBuffer, const AAResolveMode eAAResMode )
{
	m_pImpl->Resolve( pSourceBuffer, eAAResMode );
}


