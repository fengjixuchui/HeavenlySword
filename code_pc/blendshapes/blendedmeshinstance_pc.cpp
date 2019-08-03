//--------------------------------------------------
//!
//!	\file blendshapes/blendedmeshinstance.cpp
//!
//--------------------------------------------------

#include "blendshapes/blendedmeshinstance.h"
#include "blendshapes/xpushapeblending.h"
#include "gfx/renderer.h"
#include "gfx/graphicsdevice.h"
#include "gfx/rendercontext.h"




VBHandle CreateMyDynamicVertexBuffer( const CMeshHeader* pMeshHeader )
{
	int bufferSize = pMeshHeader->m_iNumberOfVertices * pMeshHeader->m_iVertexStride;

	VBHandle pobBuffer = Renderer::Get().m_Platform.CreateDynamicVertexBuffer( bufferSize ) ;

	// fill with our pretty static data
	void* pDst;
	HRESULT hr = pobBuffer->Lock( 0, bufferSize, &pDst, D3DLOCK_DISCARD );
	UNUSED(hr);
	ntError_p( SUCCEEDED(hr), ( __FUNCTION__ ": Lock failed") );
	memcpy( pDst, pMeshHeader->m_pvVertexBufferData , bufferSize );

	pobBuffer->Unlock();

	// return the buffer
	return pobBuffer;
}



BlendedMeshInstance::BlendedMeshInstance( Transform const* pobTransform, CMeshHeader const* pobMeshHeader, bool bRenderOpaque, bool bShadowRecieve, bool bShadowCast )
	:	CMeshInstance( pobTransform, pobMeshHeader, bRenderOpaque, bShadowRecieve, bShadowCast, false ),
		m_pBlendShapes( 0 )
{
	SetIsShapeBlended( true );

	// replace static vertex buffer with a dynamic one. yay!
	m_hVertexBuffer[0] = CreateMyDynamicVertexBuffer( pobMeshHeader );

	// register with blend manager
	XPUShapeBlending::Get().Register( this );
}

BlendedMeshInstance::~BlendedMeshInstance( void )
{
	XPUShapeBlending::Get().Unregister( this );
}

MeshBSSetPtr_t BlendedMeshInstance::SetBlendShapes( MeshBSSetPtr_t pMeshBlendShapes )
{
	ntAssert( pMeshBlendShapes );
	/*ntAssert_p( pMeshBlendShapes->GetMeshName() == GetMeshHeader()->m_obNameHash, 
				("Incompatible BlendShape mesh header") )*/
	
	MeshBSSetPtr_t pPrevBlendShapes = m_pBlendShapes;
	m_pBlendShapes = pMeshBlendShapes;
	return pPrevBlendShapes;
}


MeshBSSetPtr_t BlendedMeshInstance::RemoveBlendShapes( void )
{
	MeshBSSetPtr_t pPrevBlendShapes = m_pBlendShapes;
	m_pBlendShapes = 0;
	return pPrevBlendShapes;
}


//-------------------------------------------------------
//						RENDERING
//-------------------------------------------------------


////! TODO: separate vertex declaration into different streams for position and only copy that
//void BlendedMeshInstance::RenderMesh() const
//{
//
//#ifdef PLATFORM_PC
//	//! set up streams
//	GetD3DDevice()->SetStreamSource( 
//		0, m_oVertexBuffer->GetVBHandle().Get(),
//		0, m_pobMeshHeader->m_iVertexStride 
//	);
//	GetD3DDevice()->SetIndices( m_hIndexBuffer.Get() );
//	
//	//! draw primitives
//	GetD3DDevice()->DrawIndexedPrimitive(
//		ConvertPRIMTYPEToD3DPRIM(m_meshType), 0, 
//		0, m_pobMeshHeader->m_iNumberOfVertices, 
//		0, m_iPolyCount
//	);
//
//	// what the hell happened with that?
//	//CRendererSettings::iRenderedPolyCount += m_iPolyCount;
//
//	//! clean up streams
//	GetD3DDevice()->SetIndices( 0 );
//	GetD3DDevice()->SetStreamSource( 0, 0, 0, 0 );
//#endif
//}




