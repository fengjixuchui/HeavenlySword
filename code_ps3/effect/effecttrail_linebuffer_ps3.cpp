//--------------------------------------------------
//!
//!	\file effecttrail_buffer.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "effect/effecttrail_line.h"
#include "effect/effecttrail_linebuffer.h"
#include "gfx/rendercontext.h"
#include "gfx/renderer.h"

//--------------------------------------------------
//!
//!	EffectTrailLineBuffer::ctor
//!
//--------------------------------------------------
void EffectTrailLineBuffer::PlatformConstruct()
{
	// find our fx handle
	m_pVertexShader = DebugShaderCache::Get().LoadShader( "effecttrail_line_vp.sho" );

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
}

//--------------------------------------------------
//!
//!	EffectTrailLineBuffer::PreRender
//! upload local parameters... these are specific to this edge effect
//!
//--------------------------------------------------
void EffectTrailLineBuffer::PreRender( const RenderStateBlock& rs )
{
	// this is a global effect constant
	CMatrix ImageTransform( CONSTRUCT_IDENTITY );
	if (m_pDef->m_rsDef.m_renderType == ERT_LOW_DYNAMIC_RANGE)
		ImageTransform = RenderingContext::Get()->m_postProcessingMatrix;
	
	m_pPixelShader->SetPSConstantByName( "g_ImageTransform", ImageTransform, 4 );

	// as is this
	CPoint eyePos = RenderingContext::Get()->GetEyePos();
	m_pVertexShader->SetVSConstantByName( "g_eyePos", eyePos );

	// these should be based on whether the effect is in local transform space or world space...
	CMatrix objectToWorld( CONSTRUCT_IDENTITY );
	CMatrix worldToObject( CONSTRUCT_IDENTITY );

	CMatrix worldViewProj = objectToWorld * RenderingContext::Get()->m_worldToScreen;
	m_pVertexShader->SetVSConstantByName( "m_worldViewProj", worldViewProj, 4 );

//	CVector fadetime( m_pDef->m_fFadeTime, 0.0f, 0.0f, 0.0f );
//	m_pVertexShader->SetVSConstantByName( "m_fadetime", fadetime );

	CVector RCPfadetime( 1.0f / m_pDef->m_fFadeTime, 0.0f, 0.0f, 0.0f );
	m_pVertexShader->SetVSConstantByName( "m_RCPfadetime", RCPfadetime );

	CVector lineWidth( m_pDef->m_fWidth, 0.0f, 0.0f, 0.0f );
	m_pVertexShader->SetVSConstantByName( "m_lineWidth", lineWidth );
}

//--------------------------------------------------
//!
//!	EffectTrailLineBuffer::Render
//!
//--------------------------------------------------
void EffectTrailLineBuffer::Render()
{
	if((m_bOnFirstLoop) && (m_iCurrPoint < 2))
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
		iMemToCopy = (m_iMaxPoints - m_iCurrPoint) * 2 * m_VB.GetVertexSize();
		uint8_t* pSrc = (uint8_t*)m_VB.GetVertex( m_iCurrPoint * 2 );
		m_VB.Write( pSrc, 0, iMemToCopy );
	}

	// copy from the start of the array to the current edge
	if (m_iCurrPoint > 0)
	{
		uint32_t size = m_iCurrPoint * 2 * m_VB.GetVertexSize();
		uint8_t* pSrc = (uint8_t*)m_VB.GetVertex(0);
		m_VB.Write( pSrc, iMemToCopy, size );
	}
	
	// finsh the submission
	m_VB.UnlockAndSubmit();

	// set index buffer and draw
	//-------------------------
	u_int iNumEdges = m_bOnFirstLoop ? (m_iCurrPoint - 1) : (m_iMaxPoints - 1);

	Renderer::Get().m_Platform.DrawPrimitives( Gc::kTriangleStrip, 0, iNumEdges * 2 );
	Renderer::Get().m_Platform.ClearStreams();
}

