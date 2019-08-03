/***************************************************************************************************
*
*	$Header:: /game/meshinstance.cpp 6     24/07/03 11:53 Simonb                                   $
*
*
*
*	CHANGES
*
*	9/5/2003	SimonB	Created
*
***************************************************************************************************/

#include "anim/transform.h"
#include "gfx/meshinstance.h"
#include "gfx/renderer.h"
#include "gfx/graphicsdevice.h"
#include "gfx/rendercontext.h"

void CMeshInstance::RenderShadowMap()
{
	m_pMaterial->PreRenderDepth( m_pobTransform, true );

	// set up streams
	GetD3DDevice()->SetStreamSource( 
		0, m_hVertexBuffer[0].Get(), 
		0, m_pobMeshHeader->m_iVertexStride 
	);
	GetD3DDevice()->SetIndices( m_hIndexBuffer.Get() );

	if( m_FrameFlags & RFF_CAST_SHADOWMAP0 )
	{
		Renderer::Get().SetVertexShaderConstant( 1, m_pobTransform->GetWorldMatrixFast() * RenderingContext::Get()->m_shadowMapProjection[0] );
		Renderer::Get().SetScissorRegion( &RenderingContext::Get()->m_shadowScissor[0] );
		// draw primitives
		GetD3DDevice()->DrawIndexedPrimitive(
			ConvertPRIMTYPEToD3DPRIM(m_meshType), 0, 
			0, m_pobMeshHeader->m_iNumberOfVertices, 
			0, m_iPolyCount
		);
	}
	if( m_FrameFlags & RFF_CAST_SHADOWMAP1 )
	{
		Renderer::Get().SetVertexShaderConstant( 1, m_pobTransform->GetWorldMatrixFast() * RenderingContext::Get()->m_shadowMapProjection[1] );
		Renderer::Get().SetScissorRegion( &RenderingContext::Get()->m_shadowScissor[1] );
		// draw primitives
		GetD3DDevice()->DrawIndexedPrimitive(
			ConvertPRIMTYPEToD3DPRIM(m_meshType), 0, 
			0, m_pobMeshHeader->m_iNumberOfVertices, 
			0, m_iPolyCount
		);
	}
	if( m_FrameFlags & RFF_CAST_SHADOWMAP2 )
	{
		Renderer::Get().SetVertexShaderConstant( 1, m_pobTransform->GetWorldMatrixFast() * RenderingContext::Get()->m_shadowMapProjection[2] );
		Renderer::Get().SetScissorRegion( &RenderingContext::Get()->m_shadowScissor[2] );
		// draw primitives
		GetD3DDevice()->DrawIndexedPrimitive(
			ConvertPRIMTYPEToD3DPRIM(m_meshType), 0, 
			0, m_pobMeshHeader->m_iNumberOfVertices, 
			0, m_iPolyCount
		);
	}
	if( m_FrameFlags & RFF_CAST_SHADOWMAP3 )
	{
		Renderer::Get().SetVertexShaderConstant( 1, m_pobTransform->GetWorldMatrixFast() * RenderingContext::Get()->m_shadowMapProjection[3] );
		Renderer::Get().SetScissorRegion( &RenderingContext::Get()->m_shadowScissor[3] );
		// draw primitives
		GetD3DDevice()->DrawIndexedPrimitive(
			ConvertPRIMTYPEToD3DPRIM(m_meshType), 0, 
			0, m_pobMeshHeader->m_iNumberOfVertices, 
			0, m_iPolyCount
		);
	}


	// clean up streams
	GetD3DDevice()->SetIndices( 0 );
	GetD3DDevice()->SetStreamSource( 0, 0, 0, 0 );

	m_pMaterial->PostRenderDepth( true );
}

void CMeshInstance::RenderMesh() const
{
	// set up streams
	GetD3DDevice()->SetStreamSource( 
		0, m_hVertexBuffer[0].Get(), 
		0, m_pobMeshHeader->m_iVertexStride 
	);
	GetD3DDevice()->SetIndices( m_hIndexBuffer.Get() );
	
	// draw primitives
	GetD3DDevice()->DrawIndexedPrimitive(
		ConvertPRIMTYPEToD3DPRIM(m_meshType), 0, 
		0, m_pobMeshHeader->m_iNumberOfVertices, 
		0, m_iPolyCount
	);

	// clean up streams
	GetD3DDevice()->SetIndices( 0 );
	GetD3DDevice()->SetStreamSource( 0, 0, 0, 0 );
}


void CMeshInstance::BuildShadowMapPB(Heresy_PushBuffer**)
{
}
void CMeshInstance::BuildDepthPB(Heresy_PushBuffer**)
{
}
void CMeshInstance::BuildRenderPB(Heresy_PushBuffer**)
{
}
void CMeshInstance::RenderPB( restrict Heresy_PushBuffer* )
{
}
void CMeshInstance::DestroyPushBuffer(Heresy_PushBuffer *)
{
}

void CMeshInstance::DeletePushBuffers()
{
}

void CMeshInstance::ReleaseAreaResources( void )
{
	DeletePushBuffers();
}