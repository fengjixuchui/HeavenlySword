//--------------------------------------------------
//!
//!	\file effecttrail_linebuffer.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "effecttrail_linebuffer.h"
#include "effecttrail_line.h"
#include "gfx/rendercontext.h"
#include "core/visualdebugger.h"

//--------------------------------------------------
//!
//!	EffectTrailLineBuffer::ctor
//!
//--------------------------------------------------
EffectTrailLineBuffer::EffectTrailLineBuffer( const EffectTrail_LineDef* pDef ) :
	m_pDef( pDef ),
	m_iCurrPoint( 0 ),
	m_iLastPoint( 0 ),
	m_bOnFirstLoop( true ),
	m_lastPoint( CONSTRUCT_CLEAR )
{
	ntError( m_pDef );

	m_iMaxPoints = m_pDef->GetMaxPoints();
	ntError_p( m_iMaxPoints > 1, ("Invalid trail def") );

	// setup procedural vertex buffer
	m_VB.PushVertexElement( VD_STREAM_TYPE_FLOAT1, LE_BIRTH_TIME, "input.birthTime" );
	m_VB.PushVertexElement( VD_STREAM_TYPE_FLOAT3, LE_POSITON, "input.pos" );
	m_VB.PushVertexElement( VD_STREAM_TYPE_FLOAT3, LE_TOLAST_VEC, "input.tolast" );
	m_VB.PushVertexElement( VD_STREAM_TYPE_FLOAT3, LE_TONEXT_VEC, "input.tonext" );
	m_VB.PushVertexElement( VD_STREAM_TYPE_FLOAT1, LE_TEXTURE, "input.texcoord" );

	// allocate memory and clear it
	m_VB.BuildMe( m_iMaxPoints * 2 );
	memset( m_VB.GetVertex(0), 0, m_VB.GetVertexSize() * m_VB.GetMaxVertices() );

	// init birth times
	for ( u_int i = 0; i < m_VB.GetMaxVertices(); i++ )
		*((float*)m_VB.GetVertexElement( i, LE_BIRTH_TIME )) = -m_pDef->m_fFadeTime;

	// finally set our static texture coordinate
	for ( u_int i = 0; i < m_iMaxPoints; i++ )
	{
		*((float*)m_VB.GetVertexElement( (i*2)+0, LE_TEXTURE )) = 0.0f;
		*((float*)m_VB.GetVertexElement( (i*2)+1, LE_TEXTURE )) = 1.0f;
	}

	PlatformConstruct();

#ifndef _RELEASE
	m_pDebugPoints = NT_NEW CPoint [ m_iMaxPoints ];

	for ( u_int i = 0; i <  m_iMaxPoints; i++ )
		m_pDebugPoints[i].Clear();
#endif
}

EffectTrailLineBuffer::~EffectTrailLineBuffer()
{
#ifndef _RELEASE
	NT_DELETE_ARRAY_CHUNK( Mem::MC_EFFECTS, m_pDebugPoints );
#endif
}

//--------------------------------------------------
//!
//!	EffectTrailLineBuffer::EmitPoint
//! Push the next edge into the buffer
//!
//--------------------------------------------------
void EffectTrailLineBuffer::EmitPoint( float fEmitTime, const CPoint& newpoint )
{
	// set the latest point up
	EdgeVertex newVert;
	newVert.birthTime = fEmitTime;

	newVert.posX = newpoint.X();
	newVert.posY = newpoint.Y();
	newVert.posZ = newpoint.Z();

	CDirection toLast = newpoint ^ m_lastPoint;
	toLast.Normalise();

	newVert.toLastX = newVert.toNextX = toLast.X();
	newVert.toLastY = newVert.toNextY = toLast.Y();
	newVert.toLastZ = newVert.toNextZ = toLast.Z();

	NT_MEMCPY( m_VB.GetVertex( (m_iCurrPoint*2) + 0 ), &newVert, sizeof(EdgeVertex) );
	NT_MEMCPY( m_VB.GetVertex( (m_iCurrPoint*2) + 1 ), &newVert, sizeof(EdgeVertex) );

	// set the previous point's 'to' vectors up
	if (!( m_bOnFirstLoop && (m_iCurrPoint == 0))) // make sure we're not the first point
	{
		m_lastVertex.toNextX = toLast.X();
		m_lastVertex.toNextY = toLast.Y();
		m_lastVertex.toNextZ = toLast.Z();

		// special case for line initialisation
		if (m_bOnFirstLoop && (m_iCurrPoint == 1))
		{
			m_lastVertex.toLastX = toLast.X();
			m_lastVertex.toLastY = toLast.Y();
			m_lastVertex.toLastZ = toLast.Z();
		}

		NT_MEMCPY( m_VB.GetVertex( (m_iLastPoint*2) + 0 ), &m_lastVertex, sizeof(EdgeVertex) );
		NT_MEMCPY( m_VB.GetVertex( (m_iLastPoint*2) + 1 ), &m_lastVertex, sizeof(EdgeVertex) );
	}
	
	// buffer this vertex info
	m_lastVertex = newVert;
	m_lastPoint = newpoint;

#ifndef _RELEASE
	m_pDebugPoints[m_iCurrPoint] = newpoint;
#endif

	m_iLastPoint = m_iCurrPoint;
	m_iCurrPoint++;
	if (m_iCurrPoint >= m_iMaxPoints)
	{
		m_iCurrPoint = 0;
		m_bOnFirstLoop = false;
	}
}

//--------------------------------------------------
//!
//!	EffectTrailLineBuffer::DebugRender
//!
//--------------------------------------------------
void EffectTrailLineBuffer::DebugRender()
{
#ifndef _RELEASE
	if((m_bOnFirstLoop) && (m_iCurrPoint < 2))
		return;

	CPoint* pStartPoint = &m_pDebugPoints[ m_iCurrPoint ];

	CVector colour(0.0f, 0.0f, 0.0f, 1.0f);
	CVector gradient(CVector(1.0f, 1.0f, 1.0f, 0.0f) * (1.0f / m_iMaxPoints) );

	for ( u_int i = m_iCurrPoint; i < (m_iMaxPoints-1); i++ )
	{
		uint32_t dwCol = NTCOLOUR_FROM_FLOATS( colour.X(), colour.Y(), colour.Z(), colour.W() );

		if (!m_bOnFirstLoop)
			g_VisualDebug->RenderLine( *pStartPoint, *(pStartPoint+1), dwCol, 0 );
		pStartPoint++;

		colour += gradient;
	}

	CPoint* pLastPoint = m_bOnFirstLoop ? m_pDebugPoints : pStartPoint;
	CPoint* pNextPoint = m_bOnFirstLoop ? (pLastPoint+1) : m_pDebugPoints;

	if(m_iCurrPoint > 0)
	{
		u_int iToDraw = m_bOnFirstLoop ? (m_iCurrPoint-2) : (m_iCurrPoint-1);
		for ( u_int i = 0; i < iToDraw; i++ )
		{
			uint32_t dwCol = NTCOLOUR_FROM_FLOATS( colour.X(), colour.Y(), colour.Z(), colour.W() );
			
			g_VisualDebug->RenderLine( *pLastPoint, *pNextPoint, dwCol, 0 );
			pNextPoint++;
			
			colour += gradient;
			pLastPoint = (pNextPoint - 1);
		}
	}

	g_VisualDebug->RenderSphere( CVecMath::GetQuatIdentity(), m_lastPoint, 0.1f, 0xffff0000, 0 );
	
#endif
}
