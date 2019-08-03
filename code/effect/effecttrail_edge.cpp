//--------------------------------------------------
//!
//!	\file effecttrail_edge.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "effecttrail_edge.h"
#include "objectdatabase/dataobject.h"
#include "camera/camutils.h"
#include "effect_util.h"
#include "core/visualdebugger.h"

START_STD_INTERFACE( EffectTrail_EdgeNode )

	IPOINT(	EffectTrail_EdgeNode,	Pos )
	IFLOAT(	EffectTrail_EdgeNode,	Texture )

	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )
	DECLARE_DEBUGRENDER_FROMNET_CALLBACK( DebugRender )
END_STD_INTERFACE

START_STD_INTERFACE( EffectTrail_EdgeDef )

	PUBLISH_PTR_CONTAINER_AS(		m_obEdgeNodes, EdgeNodes )
	IVECTOR(	EffectTrail_EdgeDef, EdgeOrientYPR )

	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )
	DECLARE_DEBUGRENDER_FROMNET_CALLBACK( DebugRender )
END_STD_INTERFACE

void ForceLinkFunction12()
{
	ntPrintf("!ATTN! Calling ForceLinkFunction12() !ATTN!\n");
}

//--------------------------------------------------
//!
//!	EffectTrail_EdgeDef::ctor
//!
//--------------------------------------------------
EffectTrail_EdgeNode::EffectTrail_EdgeNode()
{
	m_obPos.Clear();
	m_fTexture = 0.0f;

	m_bHasChanged = false;
	m_pParent = 0;
}

void EffectTrail_EdgeNode::DebugRender()
{
	if (m_pParent)
	{
		// This will be unsafe if our parent has been deleted, 
		// say within welder, but theres not much i can do about that..
		m_pParent->DebugRender();
	}
}




//--------------------------------------------------
//!
//!	EffectTrail_EdgeDefResources::ctor
//!
//--------------------------------------------------
EffectTrail_EdgeDefResources::EffectTrail_EdgeDefResources() :
	m_pParent(0),
	m_pVectors(0),
	m_iNumPoints(0)
{
	EffectResourceMan::Get().RegisterResource( *this );
}

EffectTrail_EdgeDefResources::~EffectTrail_EdgeDefResources()
{
	if (m_pVectors)
		NT_DELETE_ARRAY_CHUNK( Mem::MC_EFFECTS, m_pVectors );
	EffectResourceMan::Get().ReleaseResource( *this );
}

//--------------------------------------------------
//!
//!	EffectTrail_EdgeDefResources::GenerateResources
//!
//--------------------------------------------------
void EffectTrail_EdgeDefResources::GenerateResources()
{
	ntError( m_pParent );

	if (m_pVectors)
		NT_DELETE_ARRAY_CHUNK( Mem::MC_EFFECTS, m_pVectors );

	// setup our edge array
	if ( m_pParent->m_obEdgeNodes.size() > 1 )
	{
		m_iNumPoints = m_pParent->m_obEdgeNodes.size();
		m_pVectors = NT_NEW_CHUNK ( Mem::MC_EFFECTS ) CVector [ m_iNumPoints ];
	
		int iIndex = 0;
		for (	ntstd::List<EffectTrail_EdgeNode*>::iterator it = m_pParent->m_obEdgeNodes.begin();
				it != m_pParent->m_obEdgeNodes.end(); ++it, iIndex++ )
		{
			m_pVectors[iIndex] = CVector( (*it)->m_obPos ) * m_pParent->GetEdgeOrient();
			m_pVectors[iIndex].W() = (*it)->m_fTexture;
		}
	}
	else // nowt there, so generate defaults
	{
		// generate edge vertices
		m_iNumPoints = 2;
		m_pVectors = NT_NEW_CHUNK ( Mem::MC_EFFECTS ) CVector[ m_iNumPoints ];

		// w component is the V texture coordinate
		m_pVectors[0] = CVector( CPoint( 0.0f, 0.0f, 0.0f ) );
		m_pVectors[1] = CVector( CPoint( 0.0f, 2.0f, 0.0f ) );

		m_pVectors[0].W() = 0.0f;
		m_pVectors[1].W() = 1.0f;
	}

	ResourcesOutOfDate(); // this flushes any erronious refresh detects
	m_bRequireRefresh = false;
}

//--------------------------------------------------
//!
//!	EffectTrail_EdgeDefResources::ResourcesOutOfDate
//! per frame callback to see if we need regenerating
//!
//--------------------------------------------------
bool EffectTrail_EdgeDefResources::ResourcesOutOfDate() const
{
	if (m_pParent->HasChanged())
		m_bRequireRefresh = true;

	return m_bRequireRefresh;
}





//--------------------------------------------------
//!
//!	EffectTrail_EdgeDef::ctor
//!
//--------------------------------------------------
EffectTrail_EdgeDef::EffectTrail_EdgeDef()
{
	m_obEdgeOrientYPR.Clear();
	m_debugRenderMat.SetIdentity();
	m_bHasChanged = false;
	m_res.m_pParent = this;
}

void EffectTrail_EdgeDef::PostConstruct()
{
	m_edgeOrient = CMatrix( CCamUtil::QuatFromYawPitchRoll(	m_obEdgeOrientYPR.X() * DEG_TO_RAD_VALUE,
															m_obEdgeOrientYPR.Y() * DEG_TO_RAD_VALUE,
															m_obEdgeOrientYPR.Z() * DEG_TO_RAD_VALUE ), CPoint(0.0f, 0.0f, 0.0f) );

	for (	ntstd::List<EffectTrail_EdgeNode*>::iterator it = m_obEdgeNodes.begin();
			it != m_obEdgeNodes.end(); ++it )
	{
		(*it)->SetParent(this);
	}
}

void EffectTrail_EdgeDef::DebugRender()
{
#ifndef _GOLD_MASTER
	// draw our edge list
	CPoint start( CONSTRUCT_CLEAR );
	CPoint end( CONSTRUCT_CLEAR );

	CMatrix local2world = (m_edgeOrient * m_debugRenderMat);

	for (	ntstd::List<EffectTrail_EdgeNode*>::iterator it = m_obEdgeNodes.begin();
			it != m_obEdgeNodes.end(); ++it )
	{
		ntstd::List<EffectTrail_EdgeNode*>::iterator next = it;
		++next;

		if (next != m_obEdgeNodes.end())
		{
			start = (*it)->m_obPos * local2world;
			end = (*next)->m_obPos * local2world;

			g_VisualDebug->RenderLine( start, end, 0xffff0000, 0 );
		}
	}

	// display our matrix too.
	EffectUtils::DebugRenderFrame( local2world, 1.0f );
#endif
}


