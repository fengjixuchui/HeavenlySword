//--------------------------------------------------
//!
//!	\file effecttrail_buffer.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "effecttrail_buffer.h"
#include "gfx/rendercontext.h"
#include "gfx/renderer.h"
#include "effect/effect_manager.h"
#include "core/visualdebugger.h"

//--------------------------------------------------
//!
//!	EffectTrailBuffer::ctor
//!
//--------------------------------------------------
EffectTrailBuffer::EffectTrailBuffer(	const EffectTrail_SimpleDef* pDef,
										const EffectTrail_EdgeDef* pEdge ) :
	m_pDef( pDef ),
	m_pEdge( pEdge ),
	m_iCurrEdge( 0 ),
	m_bOnFirstLoop( true )
{
	ntError( m_pDef );
	ntError( m_pEdge );

	// setup procedural vertex buffer
	m_VB.PushVertexElement( VD_STREAM_TYPE_FLOAT1, EE_BIRTH_TIME, "input.birthTime" );
	m_VB.PushVertexElement( VD_STREAM_TYPE_FLOAT3, EE_POSITON, "input.pos" );
	m_VB.PushVertexElement( VD_STREAM_TYPE_FLOAT1, EE_TEXTURE, "input.texcoord" );

	const CVector* pEdgeTemplate = m_pEdge->m_res.GetVectorArray();
	m_iVertsPerEdge = m_pEdge->m_res.GetNumPoints();
	m_iMaxEdges = m_pDef->GetMaxEdges();
	
	u_int iNumVerts = m_iMaxEdges * m_iVertsPerEdge;

	ntError_p( pEdgeTemplate, ("Invalid edge template") );
	ntError_p( m_iMaxEdges > 1, ("Invalid trail def") );
	ntError_p( m_iVertsPerEdge > 1, ("Invalid trail def, need at least 2 verts per trail edge") );
	ntError_p( iNumVerts < 0x10000, ("Trail is too large: reduce fade time, emmision rate or points in edge") );

	// allocate memory and clear it
	m_VB.BuildMe( iNumVerts );
	memset( m_VB.GetVertex(0), 0, m_VB.GetVertexSize() * m_VB.GetMaxVertices() );

	// init birth times
	for ( u_int i = 0; i < m_VB.GetMaxVertices(); i++ )
		*((float*)m_VB.GetVertexElement( i, EE_BIRTH_TIME )) = -m_pDef->m_fFadeTime;

	// finally set our static texture coordinate
	for ( u_int i = 0; i < m_iMaxEdges; i++ )
	{
		for ( u_int j = 0; j < m_iVertsPerEdge; j++ )
		{
			*((float*)m_VB.GetVertexElement( (i*m_iVertsPerEdge)+j, EE_TEXTURE )) = pEdgeTemplate[j].W();
		}
	}

	PlatformConstruct();

#ifndef _NO_DBGMEM_OR_RELEASE
	m_pDebugPoints = NT_NEW_CHUNK ( Mem::MC_MISC ) CPoint [ m_VB.GetMaxVertices() ];

	for ( u_int i = 0; i <  m_VB.GetMaxVertices(); i++ )
		m_pDebugPoints[i].Clear();
#endif
}

EffectTrailBuffer::~EffectTrailBuffer()
{
#ifndef _NO_DBGMEM_OR_RELEASE
	NT_DELETE_ARRAY_CHUNK( Mem::MC_MISC, m_pDebugPoints );
#endif
}

//--------------------------------------------------
//!
//!	EffectTrailBuffer::EmitEdge
//! Push the next edge into the buffer
//!
//--------------------------------------------------
void EffectTrailBuffer::EmitEdge( float fEmitTime, const CMatrix& frame )
{
	EdgeVertex newVert;
	newVert.birthTime = fEmitTime;

	const CVector* pEdgeTemplate = m_pEdge->m_res.GetVectorArray();
	u_int iStartIndex = (m_iCurrEdge * m_iVertsPerEdge);

	for (u_int i = 0; i < m_iVertsPerEdge; i++)
	{
		// need to copy these out so the W component doesnt screw the transform
		CPoint newPos = CPoint(	pEdgeTemplate[i].X(),
								pEdgeTemplate[i].Y(),
								pEdgeTemplate[i].Z() ) * frame;
		newVert.posX = newPos.X();
		newVert.posY = newPos.Y();
		newVert.posZ = newPos.Z();
		NT_MEMCPY( m_VB.GetVertex( iStartIndex + i ), &newVert, sizeof(EdgeVertex) );

#ifndef _NO_DBGMEM_OR_RELEASE
		m_pDebugPoints[ iStartIndex + i ] = newPos;
		m_debugFrame = frame;
#endif
	}

	m_iCurrEdge++;
	if (m_iCurrEdge >= m_iMaxEdges)
	{
		m_iCurrEdge = 0;
		m_bOnFirstLoop = false;
	}
}

//--------------------------------------------------
//!
//!	EffectTrailBuffer::Render
//!
//--------------------------------------------------
void EffectTrailBuffer::DebugRender()
{
#ifndef _NO_DBGMEM_OR_RELEASE
	if((m_bOnFirstLoop) && (m_iCurrEdge < 2))
		return;

	CPoint* pStartPoint = &m_pDebugPoints[ m_iCurrEdge * m_iVertsPerEdge ];

	CVector colour(0.0f, 0.0f, 0.0f, 1.0f);
	CVector gradient(CVector(1.0f, 1.0f, 1.0f, 0.0f) * (1.0f / m_iMaxEdges) );

	for ( u_int i = m_iCurrEdge; i < (m_iMaxEdges-1); i++ )
	{
		uint32_t dwCol = NTCOLOUR_FROM_FLOATS( colour.X(), colour.Y(), colour.Z(), colour.W() );

		for ( u_int j = 0; j < m_iVertsPerEdge; j++ )
		{
			if (!m_bOnFirstLoop)
				g_VisualDebug->RenderLine( *pStartPoint, *(pStartPoint+m_iVertsPerEdge), dwCol, 0 );
			pStartPoint++;
		}

		colour += gradient;
	}

	CPoint* pLastPoint = m_bOnFirstLoop ? m_pDebugPoints : pStartPoint;
	CPoint* pNextPoint = m_bOnFirstLoop ? (pLastPoint+m_iVertsPerEdge) : m_pDebugPoints;

	if(m_iCurrEdge > 0)
	{
		u_int iToDraw = m_bOnFirstLoop ? (m_iCurrEdge-2) : (m_iCurrEdge-1);
		for ( u_int i = 0; i < iToDraw; i++ )
		{
			uint32_t dwCol = NTCOLOUR_FROM_FLOATS( colour.X(), colour.Y(), colour.Z(), colour.W() );

			for ( u_int j = 0; j < m_iVertsPerEdge; j++ )
			{
				g_VisualDebug->RenderLine( *pLastPoint, *pNextPoint, dwCol, 0 );
				pLastPoint++;
				pNextPoint++;
			}
			
			colour += gradient;
			pLastPoint = (pNextPoint - m_iVertsPerEdge);
		}
	}

	const CVector* pEdgeTemplate = m_pEdge->m_res.GetVectorArray();
	CPoint start =  CPoint(	pEdgeTemplate[0].X(),
							pEdgeTemplate[0].Y(),
							pEdgeTemplate[0].Z() ) * m_debugFrame;

	for (u_int i = 1; i < m_iVertsPerEdge; i++)
	{
		CPoint next = CPoint(	pEdgeTemplate[i].X(),
								pEdgeTemplate[i].Y(),
								pEdgeTemplate[i].Z() ) * m_debugFrame;

		g_VisualDebug->RenderLine( start, next, 0xff0000ff, 0 );
		start = next;
	}

#endif
}
