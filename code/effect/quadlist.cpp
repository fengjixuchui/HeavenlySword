//--------------------------------------------------
//!
//!	\file quadlist.cpp
//!	Class that wraps all nessecary things for a dynamic
//! quad list.
//!
//--------------------------------------------------

#include "quadlist.h"
#include "effect_util.h"
#include "gfx/rendercontext.h"
#include "gfx/renderer.h"

#ifdef PLATFORM_PC
#include "gfx/dxerror_pc.h"
#endif

//--------------------------------------------------
//!
//!	QuadList::ReleaseResources()
//!
//--------------------------------------------------
void QuadList::ReleaseResources()
{
	m_bValid = false;
	m_VB.Reset();
	m_DB.Reset();

#ifdef PLATFORM_PC
	m_IB.SafeRelease();
#endif
}

//--------------------------------------------------
//!
//!	QuadListUnsorted::Initialise()
//!
//--------------------------------------------------
void QuadList::Initialise( u_int iNumQuads, bool bUsePointSprites, bool bUsingTriLists )
{
	ntError_p( !m_bValid, ("This quad list already initialised") );
	ntError_p( iNumQuads, ("This quad list is going to be empty") );

	UNUSED(bUsingTriLists);

	m_iNumQuads = iNumQuads;
	m_bUsingPointSprites = bUsePointSprites;
	m_iVertsPerQuad = m_bUsingPointSprites ? 1 : 4;

	if (m_bUsingPointSprites)
	{
		m_VB.BuildMe( m_iNumQuads );
	}
	else
	{
#ifdef PLATFORM_PC
		// we only support indexed tri lists / tri strips on PC
		m_bUsingTriLists = bUsingTriLists;

		// we can only draw a max of 65536 verts with a 16 bit index buffer...
		m_IB = EffectUtils::CreateQuadIndexBuffer( ntstd::Min( iNumQuads, 0x4000u ), m_iNumIndices, m_bUsingTriLists );
#endif
		m_VB.BuildMe( m_iNumQuads * m_iVertsPerQuad );
	}

	if (m_DB.GetVertexSize())
		m_DB.BuildMe( m_iNumQuads );

	m_bValid = true;
}

//--------------------------------------------------
//!
//!	QuadList::DrawQuads()
//!
//--------------------------------------------------
#ifdef PLATFORM_PC
void QuadList::DrawQuads( u_int iNumToRender )
{
	// we can only draw a max of 65536 verts with a 16 bit index buffer...
	u_int iBaseVert = 0;
	u_int iNumVertices = iNumToRender * m_iVertsPerQuad;

	while (iBaseVert < iNumVertices)
	{
		int iToDraw = ntstd::Min( iNumVertices - iBaseVert, 0x10000u );
		if (m_bUsingTriLists)
		{
			int iNumTris = (iToDraw >> 1);
			GetD3DDevice()->SetIndices( m_IB.Get() );
			dxerror( GetD3DDevice()->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, iBaseVert, 0,
																	iToDraw, 0, iNumTris ) );
		}
		else
		{
			int iNumIndices = ((iToDraw >> 2) * 6) - 2;
			GetD3DDevice()->SetIndices( m_IB.Get() );
			dxerror( GetD3DDevice()->DrawIndexedPrimitive( D3DPT_TRIANGLESTRIP, iBaseVert, 0,
																	iToDraw, 0, iNumIndices - 2 ) );
		}
		iBaseVert += iToDraw;
	}
}
#endif

//--------------------------------------------------
//!
//!	QuadList::Render()
//!
//--------------------------------------------------
void QuadList::Render( u_int iNumToRender )
{
	if ( iNumToRender == 0xffffffff )
		iNumToRender = m_iNumQuads;
	else
		iNumToRender = min( iNumToRender, m_iNumQuads );

	u_int iNumVerts = m_iVertsPerQuad * iNumToRender;

	if( (m_VB.CanSubmitToGPU( iNumVerts ) == false) || (iNumToRender == 0) )
		return;

	m_VB.SubmitToGPU( iNumVerts );

/*
	if( (m_VB.CanSubmitToGPU() == false) || (iNumToRender == 0) )
		return;

	m_VB.SubmitToGPU();
*/

#ifdef PLATFORM_PC
	if ( m_bUsingPointSprites )
	{
		dxerror( GetD3DDevice()->DrawPrimitive( D3DPT_POINTLIST, 0, iNumToRender ) );
	}
	else
	{
		DrawQuads( iNumToRender );
	}
#else
	if ( m_bUsingPointSprites )
	{
		Renderer::Get().m_Platform.DrawPrimitives( Gc::kPoints, 0, iNumToRender );
	}
	else
	{
		Renderer::Get().m_Platform.DrawPrimitives( Gc::kQuads, 0, iNumToRender * m_iVertsPerQuad );
	}
	Renderer::Get().m_Platform.ClearStreams();
#endif
}




//--------------------------------------------------
//!
//!	QuadListSorted::ReleaseResources()
//!
//--------------------------------------------------
void QuadListSorted::ReleaseResources()
{
	QuadList::ReleaseResources();
	NT_DELETE_ARRAY_CHUNK( Mem::MC_EFFECTS, m_pSortList );
}

//--------------------------------------------------
//!
//!	QuadListSorted::Initialise()
//!
//--------------------------------------------------
void QuadListSorted::Initialise( u_int iNumQuads, bool bUsePointSprites, bool )
{
	ntError_p( !m_bValid, ("This quad list already initialised") );
	ntError_p( iNumQuads, ("This quad list is going to be empty") );

	m_iNumQuads = iNumQuads;
	m_bUsingPointSprites = bUsePointSprites;
	m_iVertsPerQuad = m_bUsingPointSprites ? 1 : 4;

	if (m_bUsingPointSprites)
	{
		m_VB.BuildMe( m_iNumQuads );
	}
	else
	{
#ifdef PLATFORM_PC
		m_bUsingTriLists = true;
		
		// we can only draw a max of 65536 verts with a 16 bit index buffer...
		m_IB = EffectUtils::CreateQuadIndexBuffer( ntstd::Min( iNumQuads, 0x4000u ), m_iNumIndices, m_bUsingTriLists );
#endif
		m_VB.BuildMe( m_iNumQuads * m_iVertsPerQuad );
	}

	if (m_DB.GetVertexSize())
		m_DB.BuildMe( m_iNumQuads );

	m_bValid = true;
	m_pSortList = NT_NEW_CHUNK ( Mem::MC_EFFECTS ) SortNode[m_iNumQuads];
}

//--------------------------------------------------
//!
//!	QuadListSorted::Render()
//!
//--------------------------------------------------
void QuadListSorted::Render( u_int iNumToRender )
{
	if ( iNumToRender == 0xffffffff )
		iNumToRender = m_iNumQuads;
	else
		iNumToRender = min( iNumToRender, m_iNumQuads );

	u_int iNumVerts = iNumToRender * m_iVertsPerQuad;

	if( (m_VB.CanSubmitToGPU( iNumVerts ) == false) || (iNumToRender == 0) )
		return;

	CPoint camPos = RenderingContext::Get()->GetEyePos();

	// generate our sort list
	// nb, requires that our vertex buffer has quads that store their world
	// space postion in PV_ELEMENT_POS

	u_int iStep = m_bUsingPointSprites ? 1 : 4;

	for (u_int i = 0; i < iNumToRender; i++)
	{
		float* pPos = (float*) m_VB.GetVertexElement( i * iStep, PV_ELEMENT_POS );

		m_pSortList[i].m_fDistanceSq = (camPos - CPoint( pPos )).LengthSquared();
		m_pSortList[i].m_iParticleIndex = i;
	}

	ntstd::sort( &m_pSortList[0], &m_pSortList[iNumToRender], comparatorFurtherThan() );

	if ( m_bUsingPointSprites )
	{
		// retrieve a fresh discard buffer
		m_VB.LockForWrite( iNumVerts );

		// now copy sorted particles directly into this discard buffer
		uint32_t offset = 0;
		uint32_t size = m_VB.GetVertexSize();
		for (u_int i = 0; i < iNumToRender; i++)
		{
			m_VB.Write( (uint8_t*)m_VB.GetVertex( m_pSortList[i].m_iParticleIndex ), offset, size );
			offset += size;
		}

		// finsh the submission
		m_VB.UnlockAndSubmit();

#ifdef PLATFORM_PC
		dxerror( GetD3DDevice()->DrawPrimitive( D3DPT_POINTLIST, 0, iNumToRender ) );
#elif defined (PLATFORM_PS3)
		Renderer::Get().m_Platform.DrawPrimitives( Gc::kPoints, 0, iNumToRender );
#endif
	}
	else
	{
		// retrieve a fresh discard buffer
		m_VB.LockForWrite( iNumVerts );

		// now copy sorted particles directly into this discard buffer
		uint32_t offset = 0;
		uint32_t size = m_VB.GetVertexSize() * 4;
		for (u_int i = 0; i < iNumToRender; i++)
		{
			m_VB.Write( (uint8_t*)m_VB.GetVertex( m_pSortList[i].m_iParticleIndex * 4 ), offset, size );
			offset += size;
		}

		// finsh the submission
		m_VB.UnlockAndSubmit();

#ifdef PLATFORM_PC
		DrawQuads( iNumToRender );
#elif defined (PLATFORM_PS3)
		Renderer::Get().m_Platform.DrawPrimitives( Gc::kQuads, 0, iNumToRender * m_iVertsPerQuad );
#endif
	}

#ifdef PLATFORM_PS3
	Renderer::Get().m_Platform.ClearStreams();
#endif
}
