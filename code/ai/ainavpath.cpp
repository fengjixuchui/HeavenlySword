/***************************************************************************************************
*
*	DESCRIPTION		ainavpath.cpp - A path between 2 points, composed of edges between nodes in a
*					navgraph
*
*	NOTES
*
***************************************************************************************************/

#include "ai/ainavpath.h"
#include "ai/ainavgraphmanager.h"

#include "core/visualdebugger.h"
#include "game/entitymanager.h"

/***************************************************************************************************
*
*	FUNCTION		
*
*	DESCRIPTION		
*
***************************************************************************************************/

CAINavPath::~CAINavPath( void )
{
	CleanUp();
}

/***************************************************************************************************
*
*	FUNCTION		
*
*	DESCRIPTION		
*
***************************************************************************************************/
void
CAINavPath::CleanUp()
{

	NT_DELETE_CHUNK( Mem::MC_AI, m_pobEdgeItr );
	m_pobEdgeItr = 0;

	// we've come to the end of the edge list, so purge the list
	// to prevent another call starting the cycle again
	m_obEdgeList.clear();
}

/***************************************************************************************************
*
*	FUNCTION		
*
*	DESCRIPTION		
*
*	NOTES			XXX: this is a right old mess, mainly because:
*							1. it needs splitting into a GetFirst and GetNext function
*							2. the prototype stringpulling needs moving into a separate function
*
***************************************************************************************************/

int CAINavPath::GetNextEdge()
{
	// bail out if there's no edges
	if (m_obEdgeList.empty())
	{
		return -1;
	}

	// create an iterator if we haven't already
	if (!m_pobEdgeItr)
	{
        m_pobEdgeItr = NT_NEW_CHUNK( Mem::MC_AI ) EdgeList::iterator;
		(*m_pobEdgeItr) = m_obEdgeList.begin();
	}
	else
	{	
		// increment the iterator
		++(*m_pobEdgeItr);

		if ((*m_pobEdgeItr) != m_obEdgeList.end())
		{
		}
		else
		{
			CleanUp();
			return -1;
		}
	}
	return (**m_pobEdgeItr);
}

int CAINavPath::GetEdge( int iEdgeNum )
{
	EdgeList::iterator itr = m_obEdgeList.begin();

	for(int i = 0; itr != m_obEdgeList.end(), i < iEdgeNum; ++itr, ++i)
	{
	}

	if (itr == m_obEdgeList.end())
	{
		return -1;
	}

	return *itr;
}

void CAINavPath::DebugRender()
{
#ifndef _GOLD_MASTER
	CPoint	lineStart( m_obPathStart );

	EdgeList::iterator itr = m_obEdgeList.begin();
	for(; itr != m_obEdgeList.end(); ++itr)
	{
		CPoint	lineEnd( CAINavGraphManager::Get().GetConnectionPos( (*itr) ) );

		// render a line from the previous node pos to this
		CPoint	drawStart( lineStart.X(), lineStart.Y() + 0.5f, lineStart.Z() );
		CPoint	drawEnd( lineEnd.X(), lineEnd.Y() + 0.5f, lineEnd.Z() );
		g_VisualDebug->RenderLine( drawStart, drawEnd, 0xffffff00, DPF_NOCULLING );

		lineStart = lineEnd;
	}

	CPoint	drawStart( lineStart.X(), lineStart.Y() + 0.5f, lineStart.Z() );
	CPoint	drawEnd( m_obPathDest.X(), m_obPathDest.Y() + 0.5f, m_obPathDest.Z() );
	g_VisualDebug->RenderLine( drawStart, drawEnd, 0xffffff00, DPF_NOCULLING );
#endif
}


void CAINavPath::Simplify()
{
	int nodeHint = m_obStartNode;

	// test straight line path from start to destination
	if (CAINavGraphManager::Get().LineContainedByGraph( m_obPathStart, m_obPathDest, nodeHint ))
	{
		// delete the rest of the path nodes and return
		CleanUp();
		return;		
	}

	// set the start node of the path to be our "from" position for line testing
	CPoint from = m_obPathStart;
	CPoint testPos;

	for (EdgeList::iterator it = m_obEdgeList.begin();
		it != m_obEdgeList.end();
		)
	{
		// test line, "from" to the node after the current itr
		EdgeList::iterator testNode = it;
		++testNode;
		if (testNode == m_obEdgeList.end())
		{
			testPos = m_obPathDest;
		}
		else
		{
			testPos = CAINavGraphManager::Get().GetConnectionPos( (*testNode) );
		}

		// if there's a straight line between from and testnode, then itr isn't needed
		if (CAINavGraphManager::Get().LineContainedByGraph(
			from, testPos, nodeHint ))
		{
			// remove itr from the path
			m_obEdgeList.erase( it );
			it = testNode;
		}
		else
		{
			// make this node the "from" node
			from = CAINavGraphManager::Get().GetConnectionPos( (*it) );
			++it;
		}
	}

}






