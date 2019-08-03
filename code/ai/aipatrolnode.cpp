//------------------------------------------------------------------------------------------
//!
//!	aipatrolnode.cpp
//!
//------------------------------------------------------------------------------------------

#include "ai/aipatrolnode.h"
#include "ai/aipatrolmanager.h"
#include "ai/aipatrolpath.h"
#include "ai/ainavgraphmanager.h"

#include "core/visualdebugger.h"


//------------------------------------------------------------------------------------------
//!
//!	PatrolNode::PatrolNode
//! Constructor
//!
//! Constructed with m_iPathNum( -1 ) to indicate no affiliation with a path.
//------------------------------------------------------------------------------------------

PatrolNode::PatrolNode() : m_iPathNum( -1 )
{
}

//------------------------------------------------------------------------------------------
//!
//!	PatrolNode::PostConstruct
//! 
//!
//------------------------------------------------------------------------------------------
void PatrolNode::PostConstruct()
{
	// add node to it's assigned path through the pathmanager
	if (m_iPathNum >= 0)
	{
		// get the path from the manager
		AIPatrolPath*	pPath = AIPatrolManager::Get().GetPath( m_iPathNum );

		// if the path doesn't exist, create it
		if (!pPath)
		{
			pPath = AIPatrolManager::Get().CreatePath( m_iPathNum );
		}
		ntAssert( pPath );

		// add the node to the path
		pPath->AddNode( this );
	}
}

//------------------------------------------------------------------------------------------
//!
//!	PatrolNode::SetPath
//! 
//!
//------------------------------------------------------------------------------------------
void PatrolNode::SetPath( const int iPathNum )
{
	m_iPathNum = iPathNum;
}

//------------------------------------------------------------------------------------------
//!
//!	PatrolNode::PaulsDebugRender
//! 
//!
//------------------------------------------------------------------------------------------

void PatrolNode::PaulsDebugRender()
{
#ifndef _GOLD_MASTER
	return;

	CQuat	obOrientation( CONSTRUCT_IDENTITY );
	float	fRadius = 0.1f;

	g_VisualDebug->RenderSphere( obOrientation, m_obPos,	fRadius, 0xff0f0fff );
	// if point out of graph, disable it, and set a debug message on it
	if (CAINavGraphManager::Get().InGraph( m_obPos ))
	{
		m_bEnabled = true;
	}
	else
	{
		m_bEnabled = false;
		m_strStatus = "OUT OF NAVGRAPH";
	}

	if (!m_bEnabled)
	{
		// print the status message, if there is one
		CPoint pt3D = m_obPos;
		pt3D.Y() += 0.5f;

		g_VisualDebug->Printf3D( pt3D, 0xffffffff, 0, m_strStatus.c_str() );
	}
#endif
}
