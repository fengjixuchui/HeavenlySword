//--------------------------------------------------
//!
//!	\file effecttrail_buffer.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "effect/effecttrail_line.h"
#include "effect/effecttrail_linebuffer.h"
#include "effect/effect_resourceman.h"
#include "gfx/rendercontext.h"

//--------------------------------------------------
//!
//!	EffectTrailBuffer::ctor
//!
//--------------------------------------------------
void EffectTrailLineBuffer::PlatformConstruct()
{
	// find our fx handle
	m_ppFX = EffectResourceMan::Get().RetrieveFXHandle( "effecttrail_line" );
}

//--------------------------------------------------
//!
//!	EffectTrailLineBuffer::PreRender
//! upload local parameters... these are specific to this edge effect
//!
//--------------------------------------------------
void EffectTrailLineBuffer::PreRender( const RenderStateBlock& rs )
{
	UNUSED(rs);

	u_int iTechnique = TrailUtils::GetTechniqueID( m_pDef->m_eTexMode, rs );

	HRESULT hr;
	hr = (*m_ppFX)->SetTechnique( TrailUtils::GetTechniqueName(iTechnique) );
	ntAssert_p( hr == S_OK,("Technique %s not found in this FX file", TrailUtils::GetTechniqueName(iTechnique) ) );

	EffectUtils::SetGlobalFXParameters( m_ppFX->Get() );

	// these should be based on whether the effect is in local transform space or world space...
	CMatrix objectToWorld( CONSTRUCT_IDENTITY );
	CMatrix worldToObject( CONSTRUCT_IDENTITY );

	CMatrix worldViewProj = objectToWorld * RenderingContext::Get()->m_worldToScreen;
	FX_SET_VALUE_VALIDATE( (*m_ppFX), "m_worldViewProj", &worldViewProj, sizeof(CMatrix) );

	if (rs.m_renderType == ERT_HDR_DEPTH_HAZED)
	{
		CMatrix viewMat = objectToWorld * RenderingContext::Get()->m_worldToView;
		FX_SET_VALUE_VALIDATE( (*m_ppFX), "m_worldView", &viewMat, sizeof(CMatrix) );

		CPoint viewPos = RenderingContext::Get()->GetEyePos() * worldToObject;
		FX_SET_VALUE_VALIDATE( (*m_ppFX), "m_viewPosition_objectS", &viewPos, sizeof(float) * 3 );

		CVector temp( RenderingContext::Get()->m_sunDirection * worldToObject );
		FX_SET_VALUE_VALIDATE( (*m_ppFX), "m_sunDir_objectS", &temp, sizeof(float) * 3 );
	}

	FX_SET_VALUE_VALIDATE( (*m_ppFX), "m_fadetime", &m_pDef->m_fFadeTime, sizeof(float) );

	float fRCPfadetime = 1.0f / m_pDef->m_fFadeTime;
	FX_SET_VALUE_VALIDATE( (*m_ppFX), "m_RCPfadetime", &fRCPfadetime, sizeof(float) );

	FX_SET_VALUE_VALIDATE( (*m_ppFX), "m_lineWidth", &m_pDef->m_fWidth, sizeof(float) );
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

	u_int iNumPasses;
	(*m_ppFX)->Begin( &iNumPasses, 0 );

	ntError_p( iNumPasses == 1, ("Multipass not supported") );
	(*m_ppFX)->BeginPass(0);

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

	HRESULT hr;
	hr = GetD3DDevice()->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, iNumEdges * 2 );

	ntAssert( hr == S_OK );

	(*m_ppFX)->EndPass();
	(*m_ppFX)->End();
}
