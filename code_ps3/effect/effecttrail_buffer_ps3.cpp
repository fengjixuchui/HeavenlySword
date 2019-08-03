//--------------------------------------------------
//!
//!	\file effecttrail_buffer.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "effect/effecttrail_buffer.h"
#include "gfx/rendercontext.h"
#include "gfx/renderer.h"
#include "effect/effect_manager.h"
#include "core/visualdebugger.h"

//--------------------------------------------------
//!
//!	EffectTrailBuffer::ctor
//!
//--------------------------------------------------
void EffectTrailBuffer::PlatformConstruct()
{
	// find our fx handle
	m_pVertexShader = DebugShaderCache::Get().LoadShader( "effecttrail_simple_vp.sho" );

	switch( m_pDef->m_eTexMode )
	{
	case TTM_UNTEXTURED:
		m_pPixelShader = DebugShaderCache::Get().LoadShader( "effecttrail_simple_notex_fp.sho" );
		break;

	case TTM_SIMPLE_TEXTURED:
		m_pPixelShader = DebugShaderCache::Get().LoadShader( "effecttrail_simple_tex_fp.sho" );
		break;

	case TTM_ANIM_TEXTURED:
		m_pPixelShader = DebugShaderCache::Get().LoadShader( "effecttrail_simple_animtex_fp.sho" );
		break;
	}

	ntAssert_p( m_pVertexShader, ("Failed to find vertex shader") );
	ntAssert_p( m_pPixelShader, ("Failed to find pixel shader") );

	// setup a static index buffer
	if (!EffectManager::AllocsDisabled())
	{
		int iNumQuads = (m_iVertsPerEdge - 1) * (m_iMaxEdges - 1);
		int iNumIndices = iNumQuads * 6;

		m_IB = RendererPlatform::CreateIndexStream( Gc::kIndex16, iNumIndices );

		int iCount = 0;
		for (u_int i = 0; i < (m_iMaxEdges-1); i++)
		{
			u_short	twoTris[6];
			for (u_int j = 0; j < (m_iVertsPerEdge-1) ; j++)
			{
				twoTris[0] = (u_short)((i*m_iVertsPerEdge)+0+j);					// 0
				twoTris[1] = (u_short)((i*m_iVertsPerEdge)+1+j);					// 1
				twoTris[2] = (u_short)((i*m_iVertsPerEdge)+m_iVertsPerEdge+j);		// 2

				twoTris[3] = (u_short)((i*m_iVertsPerEdge)+1+j);					// 1
				twoTris[4] = (u_short)((i*m_iVertsPerEdge)+m_iVertsPerEdge+j);		// 2
				twoTris[5] = (u_short)((i*m_iVertsPerEdge)+m_iVertsPerEdge+1+j);	// 3

				m_IB->Write( twoTris, iCount * sizeof(u_short), 6 * sizeof(u_short) );
				iCount += 6;
			}
		}

		ntAssert( iCount == iNumIndices );
	}
}

//--------------------------------------------------
//!
//!	EffectTrailBuffer::PreRender
//! upload local parameters... these are specific to this edge effect
//!
//--------------------------------------------------
void EffectTrailBuffer::PreRender( const RenderStateBlock& rs )
{
	// this is a global effect constant
	CMatrix ImageTransform( CONSTRUCT_IDENTITY );
	if (m_pDef->m_rsDef.m_renderType == ERT_LOW_DYNAMIC_RANGE)
		ImageTransform = RenderingContext::Get()->m_postProcessingMatrix;
	
	m_pPixelShader->SetPSConstantByName( "g_ImageTransform", ImageTransform, 4 );

	// these should be based on whether the effect is in local transform space or world space...
	CMatrix objectToWorld( CONSTRUCT_IDENTITY );
	CMatrix worldToObject( CONSTRUCT_IDENTITY );

	CMatrix worldViewProj = objectToWorld * RenderingContext::Get()->m_worldToScreen;
	m_pVertexShader->SetVSConstantByName( "m_worldViewProj", worldViewProj, 4 );

	CVector RCPfadetime( 1.0f / m_pDef->m_fFadeTime, 0.0f, 0.0f, 0.0f );
	m_pVertexShader->SetVSConstantByName( "m_RCPfadetime", RCPfadetime );
}

//--------------------------------------------------
//!
//!	EffectTrailBuffer::Render
//!
//--------------------------------------------------
void EffectTrailBuffer::Render()
{
	if((m_bOnFirstLoop) && (m_iCurrEdge < 2))
		return;

	if (m_VB.CanSubmitToGPU() == false)
		return;

	Renderer::Get().SetVertexShader( m_pVertexShader );
	Renderer::Get().SetPixelShader( m_pPixelShader );

	// submit verts to GPU
	//-------------------------
	m_VB.LockForWrite();

	// copy from current edge to the end of the array
	u_int iMemToCopy = 0;
	
	if (!m_bOnFirstLoop)
	{
		iMemToCopy = (m_iMaxEdges - m_iCurrEdge) * m_iVertsPerEdge * m_VB.GetVertexSize();
		uint8_t* pSrc = (uint8_t*)m_VB.GetVertex( m_iCurrEdge * m_iVertsPerEdge );
		m_VB.Write( pSrc, 0, iMemToCopy );
	}

	// copy from the start of the array to the current edge
	if (m_iCurrEdge > 0)
	{
		uint32_t size = m_iCurrEdge * m_iVertsPerEdge * m_VB.GetVertexSize();
		uint8_t* pSrc = (uint8_t*)m_VB.GetVertex(0);
		m_VB.Write( pSrc, iMemToCopy, size );
	}
	
	// finsh the submission
	m_VB.UnlockAndSubmit();

	// set index buffer and draw
	//-------------------------
	u_int iNumEdges = m_bOnFirstLoop ? (m_iCurrEdge - 1) : (m_iMaxEdges - 1);
	u_int iNumQuads = (m_iVertsPerEdge - 1) * iNumEdges;
	u_int iNumIndices = iNumQuads * 6;

	Renderer::Get().m_Platform.DrawIndexedPrimitives( Gc::kTriangles, 0, iNumIndices, m_IB );
	Renderer::Get().m_Platform.ClearStreams();
}

