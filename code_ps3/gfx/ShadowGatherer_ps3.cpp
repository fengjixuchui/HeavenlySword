//--------------------------------------------------
//!
//!	\file ShadowGatherer_ps3.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "gfx/ShadowGatherer.h"
#include "gfx/shader.h"
#include "gfx/renderer.h"
#include "gfx/rendercontext.h"
#include "gfx/graphicsdevice.h"
#include "input/inputhardware.h"

//--------------------------------------------------
//!
//!	ShadowGatherer_impl
//! 
//--------------------------------------------------
class ShadowGatherer_impl
{
public:
	ShadowGatherer_impl( void );
	
	void GatherOcclusionTerm ( const Texture::Ptr& depthTexture );

private:
	DebugShader	*	m_vertexShader;
	DebugShader	*	m_pixelShader;
	VBHandle		m_hSimpleQuadData;
	
/*	DebugShader 	m_profileVertexShader;
	DebugShader 	m_profilePixelShader;
	VBHandle		m_hProfileVertexData;
	IBHandle		m_hProfileIndexData;  */

};

//--------------------------------------------------
//!
//!	ShadowGatherer_impl
//! construct
//!
//--------------------------------------------------

/*
#define PROFILE_VERTEX_COUNT 50001
static float profileVerts[PROFILE_VERTEX_COUNT*4];
static u_short dataIndexProfile[PROFILE_VERTEX_COUNT];

u_short shortmask = 0x0000;*/

ShadowGatherer_impl::ShadowGatherer_impl( void )
{
	m_vertexShader = DebugShaderCache::Get().LoadShader( "occlusiontermgathering_vp.sho" );
	m_pixelShader = DebugShaderCache::Get().LoadShader( "occlusiontermgathering_fp.sho" );

	// create screen quad data
	GcStreamField	simpleQuadDesc( FwHashedString( "IN.position" ), 0, Gc::kFloat, 2 ); 
	
	float ALIGNTO_POSTFIX(16) const simpleQuad[] = 
	{
		-1.0f,	 1.0f,
		 3.0f,	 1.0f, 
		-1.0f,	-3.0f
	}; 

	
	m_hSimpleQuadData = RendererPlatform::CreateVertexStream( 3, sizeof( float ) * 2, 1, &simpleQuadDesc );
	m_hSimpleQuadData->Write( simpleQuad );
   
	///////////////////////////////////////////////////////////////////////////
	// PROFILE STUFF, this code might be useful at some point in the future..
	/*
	m_profileVertexShader.SetCGFile( "profile_vp.sho" );
	m_profilePixelShader.SetCGFile( "profile_fp.sho" );
	


	// create screen quad data
	GcStreamField	profileDataDesc( FwHashedString( "IN.position" ), 0, Gc::kFloat, 3 ); 



	memset(profileVerts, 0, sizeof(profileVerts));

	for (int j=0; j<PROFILE_VERTEX_COUNT; j++)
	{
		profileVerts[(j*3)+0] = 0.1f + ((float)(j))/80000.0f + sinf(((float)(j)))*4.4f;
		profileVerts[(j*3)+1] = 0.2f + ((float)(j))/80000.0f - cosf(((float)(j))/20.0f + 12)*2.6f;
		profileVerts[(j*3)+2] = 0.3f + ((float)(j))/80000.0f - cosf(((float)(j))/200.0f + 3)*0.2f;
	} 


	m_hProfileVertexData = RendererPlatform::CreateVertexStream( PROFILE_VERTEX_COUNT, 12 , 1, &profileDataDesc );
	m_hProfileIndexData = RendererPlatform::CreateIndexStream(Gc::kIndex16, PROFILE_VERTEX_COUNT);

	

	u_short counter = 0;
	for (int i=0; i<PROFILE_VERTEX_COUNT; i++)
	{
		dataIndexProfile[i] = counter%0x03;
		counter++;
	}

	m_hProfileIndexData->Write(dataIndexProfile, 0, sizeof(u_short)*PROFILE_VERTEX_COUNT);


	m_hProfileVertexData->Write( profileVerts ); */

}

//--------------------------------------------------
//!
//!	ShadowGatherer_impl::GatherOcclusionTerm 
//! Draw a full screen quad: fetch a per pixel depth from a deph
//!
//--------------------------------------------------

void ShadowGatherer_impl::GatherOcclusionTerm ( const Texture::Ptr& depthTexture )
{
/*
	if (CInputHardware::Get().GetKeyboard().IsKeyPressed( KEYC_R ))
	{
		m_vertexShader.GenerateResources();
		m_pixelShader.GenerateResources();
	}

	Renderer::Get().SetVertexShader( &m_profileVertexShader );
	Renderer::Get().SetPixelShader( &m_profilePixelShader );

	Renderer::Get().m_Platform.SetStream( m_hProfileVertexData );

	Renderer::Get().SetBlendMode( GFX_BLENDMODE_DISABLED );
	Renderer::Get().SetZBufferMode( GFX_ZMODE_DISABLED );
	Renderer::Get().SetCullMode(GFX_CULLMODE_NONE);
	
	for (int i=0; i<2000; i++)
		Renderer::Get().m_Platform.DrawIndexedPrimitives( Gc::kTriangles, 0, PROFILE_VERTEX_COUNT, m_hProfileIndexData );

	Renderer::Get().m_Platform.ClearStreams();

	Renderer::Get().SetCullMode(GFX_CULLMODE_NORMAL);
	Renderer::Get().SetZBufferMode( GFX_ZMODE_NORMAL );

*/

	Renderer::Get().SetVertexShader( m_vertexShader );

	Renderer::Get().SetTexture( 0, depthTexture );
	Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_POINT);

	Renderer::Get().SetTexture( 1, RenderingContext::Get()->m_pShadowMap);
	Renderer::Get().SetSamplerAddressMode( 1, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerFilterMode( 1, TEXTUREFILTER_BILINEAR);


    m_pixelShader->SetPSConstantByName( "ScreenToLightSpaceTransform0", RenderingContext::Get() -> m_viewToLightTransform[0]);
    m_pixelShader->SetPSConstantByName( "ScreenToLightSpaceTransform1", RenderingContext::Get() -> m_viewToLightTransform[1]);
    m_pixelShader->SetPSConstantByName( "ScreenToLightSpaceTransform2", RenderingContext::Get() -> m_viewToLightTransform[2]);

	m_pixelShader->SetPSConstantByName( "shadowPlane0", CVector(RenderingContext::Get()->m_shadowPlanes[1].W(),RenderingContext::Get()->m_shadowPlanes[2].W(),RenderingContext::Get()->m_shadowPlanes[3].W(),0.0f ));


	m_pixelShader->SetPSConstantByName( "shadowRadii0", RenderingContext::Get()->m_shadowRadii[0]);
	m_pixelShader->SetPSConstantByName( "shadowRadii1", RenderingContext::Get()->m_shadowRadii[1]);
	m_pixelShader->SetPSConstantByName( "shadowRadii2", RenderingContext::Get()->m_shadowRadii[2]);

	m_pixelShader->SetPSConstantByName( "shadowDir", RenderingContext::Get() -> m_shadowDirection );

	CMatrix ViewToworld = RenderingContext::Get() -> m_worldToView.GetAffineInverse();
	m_pixelShader->SetPSConstantByName("ViewToWorld", ViewToworld);
	
	//CVector vCameraData = CVector(1.0f / RenderingContext::Get()->m_viewToScreen[0][0],
	//							  1.0f / RenderingContext::Get()->m_viewToScreen[1][1],
 //								  RenderingContext::Get()->m_viewToScreen[2][2],
	//							  RenderingContext::Get()->m_viewToScreen[3][2]);

	CVector vCameraData = CVector((double)(1.0 / (double)RenderingContext::Get()->m_viewToScreen[0][0]),
								  (double)(1.0 / (double)RenderingContext::Get()->m_viewToScreen[1][1]),
 								  RenderingContext::Get()->m_viewToScreen[2][2],
								  RenderingContext::Get()->m_viewToScreen[3][2]);


	m_pixelShader->SetPSConstantByName( "cameraData", vCameraData );

	Renderer::Get().SetPixelShader( m_pixelShader );

	Renderer::Get().m_Platform.SetStream( m_hSimpleQuadData );

	Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );
	Renderer::Get().SetZBufferMode( GFX_ZMODE_DISABLED );
	Renderer::Get().SetCullMode(GFX_CULLMODE_NONE);
	
	Renderer::Get().m_Platform.DrawPrimitives( Gc::kTriangles, 0, 3 );

	Renderer::Get().m_Platform.ClearStreams();

	Renderer::Get().SetCullMode(GFX_CULLMODE_NORMAL);
	Renderer::Get().SetZBufferMode( GFX_ZMODE_NORMAL );

	Renderer::Get().SetTexture( 0, Texture::NONE );
	Renderer::Get().SetTexture( 1, Texture::NONE ); 
}


//--------------------------------------------------
//!
//!	ShadowGatherer
//! Public interface to our GPU based shadow gatherer
//!
//--------------------------------------------------
ShadowGatherer::ShadowGatherer()
{
	m_pImpl = NT_NEW_CHUNK(Mem::MC_GFX)( ShadowGatherer_impl );
}
ShadowGatherer::~ShadowGatherer()
{
	NT_DELETE_CHUNK(Mem::MC_GFX, m_pImpl );
}
void ShadowGatherer::GatherOcclusionTerm ( const Texture::Ptr& depthTexture )
{
	m_pImpl->GatherOcclusionTerm ( depthTexture );
}
