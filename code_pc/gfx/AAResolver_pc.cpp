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
#include "gfx/surfacemanager.h"
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
	DebugShader	m_vertexShader;
	DebugShader	m_bilinearFiltering_PS;
	DebugShader	m_gaussianFiltering_PS;
	CVertexDeclaration m_pDecl;
};

//--------------------------------------------------
//!
//!	AAResolver_impl
//! construct
//!
//--------------------------------------------------
AAResolver_impl::AAResolver_impl( void )
{
	m_vertexShader.SetFILEFunction( SHADERTYPE_VERTEX, "fxshaders/aaresolver_vp.hlsl", "main", "vs_2_0" );
	m_bilinearFiltering_PS.SetFILEFunction( SHADERTYPE_PIXEL, "fxshaders/aaresolver_bilinear_fp.hlsl", "main", "ps_2_0" );
	m_gaussianFiltering_PS.SetFILEFunction( SHADERTYPE_PIXEL, "fxshaders/aaresolver_gaussian_fp.hlsl", "main", "ps_2_0" );

	D3DVERTEXELEMENT9 simpleQuadDesc[] = 
	{
		{ 0, 0,					D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0 }, 
		D3DDECL_END()
	};
	m_pDecl = CVertexDeclarationManager::Get().GetDeclaration( simpleQuadDesc );


}

//--------------------------------------------------
//!
//!	AAResolver_impl::BilinearFiltering
//! Perform a very simple bilinear filtering to downsample a multisampled buffer
//!
//--------------------------------------------------


void AAResolver_impl::Resolve ( const RenderTarget::Ptr& pSourceBuffer, const AAResolveMode eAAResMode )
{

	Renderer::Get().m_Platform.SetVertexDeclaration( m_pDecl );
	Renderer::Get().SetVertexShader( &m_vertexShader );

	if (eAAResMode == AARES_GAUSSIAN_FILTER) Renderer::Get().SetPixelShader( &m_gaussianFiltering_PS );
	else Renderer::Get().SetPixelShader( &m_bilinearFiltering_PS );
	
	Renderer::Get().SetTexture( 0, pSourceBuffer->GetTexture() );

	Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_BILINEAR);

	Renderer::Get().SetZBufferMode( GFX_ZMODE_DISABLED );
	Renderer::Get().SetCullMode( GFX_CULLMODE_NONE );

/*
	static float const simpleQuad[] = 
	{
		-1.0f,	 1.0f,
		 3.0f,	 1.0f, 
		-1.0f,	-3.0f
	};*/

	static float const simpleQuad[] = 
	{
		-1.0f,	 1.0f,
		 1.0f,	 1.0f, 
		 1.0f,	-1.0f,
		 1.0f,	-1.0f,
		 -1.0f,	 -1.0f,
		-1.0f,	 1.0f,
	};


	GetD3DDevice()->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 2, simpleQuad, 2*sizeof( float ) );

	// cleanup
	//-------------------------------------------------------------
	Renderer::Get().SetTexture( 0, Texture::NONE );
	Renderer::Get().SetZBufferMode( GFX_ZMODE_NORMAL );
	Renderer::Get().SetCullMode( GFX_CULLMODE_NORMAL );

}

//--------------------------------------------------
//!
//!	AAResolver
//! Public interface to our GPU based resolver
//!
//--------------------------------------------------
AAResolver::AAResolver()
{
	m_pImpl = NT_NEW_CHUNK( Mem::MC_GFX ) AAResolver_impl;
}
AAResolver::~AAResolver()
{
	NT_DELETE_CHUNK(Mem::MC_GFX, m_pImpl);
}
void AAResolver::Resolve( const RenderTarget::Ptr& pSourceBuffer, const AAResolveMode eAAResMode )
{
	m_pImpl->Resolve( pSourceBuffer, eAAResMode );
}
