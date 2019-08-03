//! -------------------------------------------
//! AINavigPath.cpp
//!
//! Navigation path returned by CAINavigStar
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#include "ainavigpath.h"
#include "core/visualdebugger.h"
#include "ainavigsystemman.h"

//! -------------------------------------------
//! DESTRUCTOR
//! -------------------------------------------
CAINavigPath::~CAINavigPath()
{
	this->clear();
}

//! -------------------------------------------
//! clear
//! -------------------------------------------
void CAINavigPath::clear( void )
{
	m_bFirstFollowPathFrame = true;

	if (m_listNavigNodePath.empty())
	{
		SetDeallocateManually(false);	
		return;
	}
	if (m_bContainsDynamicallyAllocatedNodes)
	{
		AINavigNodeList::iterator obIt		= m_listNavigNodePath.begin();
		AINavigNodeList::iterator obItEnd	= m_listNavigNodePath.end();
		for ( ; obIt != obItEnd; ++obIt)
		{
			CAINavigNode* pNode = (*obIt);
			if (pNode->HasBeenDynamicallyAllocated())
			{
				NT_DELETE_CHUNK(Mem::MC_AI, pNode );
			}
		}
	}
	m_listNavigNodePath.clear();
	SetDeallocateManually(false);
}

//! -------------------------------------------
//! GetGoalNode
//! -------------------------------------------
void CAINavigPath::PathReady( void )
{ 
	if (!m_listNavigNodePath.empty()) 
	{	
		m_obStartNode=*(m_listNavigNodePath.begin()); 
		m_obLastNode=*(--(m_listNavigNodePath.end())); 
	} 
}

//! -------------------------------------------
//! GetGoalNode
//! -------------------------------------------
CAINavigNode* CAINavigPath::GetGoalNode( void )
{ 
	if (!m_listNavigNodePath.empty())
	{
		return (*(--(m_listNavigNodePath.end()))); 
	}
	return NULL;
}

//! -------------------------------------------
//! PointToNextNode
//! -------------------------------------------
bool CAINavigPath::PointToNextNode ( void )
{
	if (m_listNavigNodePath.empty())
	{
		ntPrintf("CAINavigPath::GetNextNode -> Path List is empty!!\n");
		return false;
	}
	if (m_bPathFinished)
	{
		ntPrintf("CAINavigPath::GetNextNode -> Path is finished\n");
		return false;
	}
	// Otherwise, uptade the iterator with the next node and check it is valid
	if (++m_obIt==m_listNavigNodePath.end())
	{
		m_bPathFinished = true;
		return false;
	}
	// Valid one
	return true;
}

//! -------------------------------------------
//! PointToPrevNode
//! -------------------------------------------
bool CAINavigPath::PointToPrevNode	( void )
{
	if (m_listNavigNodePath.empty())
	{
		ntPrintf("CAINavigPath::GetPreviousNode -> Path List is empty!!\n");
		return (false);
	}
	if ( ( m_obIt == m_listNavigNodePath.begin() ) ||
		 ( m_bPathFinished) )
	{
		return (false);
	}
	--m_obIt;
	return true;
}

//! -------------------------------------------
//! PeekPreviousNode
//! -------------------------------------------
CAINavigNode* CAINavigPath::PeekPreviousNode	( void )
{
	if (m_listNavigNodePath.empty())
	{
		ntPrintf("CAINavigPath::GetPreviousNode -> Path List is empty!!\n");
		return (NULL);
	}
	if (  (m_obIt==m_listNavigNodePath.begin()) ||
		 (m_bPathFinished) )
		
	{
		return (NULL);
	} 

	// Copy the iterator
	AINavigNodeList::iterator m_obItPrev(m_obIt);

	return (*(--m_obItPrev));

}

//! -------------------------------------------
//! GetCurrentNode
//! -------------------------------------------
CAINavigNode* CAINavigPath::GetCurrentNode( void )
{
	if (m_listNavigNodePath.empty())
	{
		// ntPrintf("CAINavigPath::GetCurrentNode -> Path List is empty!!\n");
		return (NULL);
	}
	if (m_bPathFinished)
	{
		return (NULL); //(GetGoalNode());
	}
	// Otherwise, get the current one
	return (*m_obIt);
}

//! -------------------------------------------
//! PeekNextNode
//! -------------------------------------------
CAINavigNode* CAINavigPath::PeekNextNode ( void )
{
	if (m_listNavigNodePath.empty())
	{
		ntPrintf("CAINavigPath::PeekNextNode -> Path List is empty!!\n");
		return false;
	}
	if (m_bPathFinished)
	{
		ntPrintf("CAINavigPath::PeekNextNode -> Path is finished\n");
		return false;
	}
	// Copy the iterator
	AINavigNodeList::const_iterator m_obItPrev(m_obIt);
	if ( (++m_obItPrev)== m_listNavigNodePath.end() )
	{
		return (NULL);
	}
	// Every thing fine
	return (*m_obItPrev);
}

//! -------------------------------------------
//! GetCurrentNode
//! -------------------------------------------
void CAINavigPath::DeleteCurrentNode( void )
{
	if (m_listNavigNodePath.empty())
	{
		ntPrintf("CAINavigPath::DeleteCurrentNode -> Path List is empty!!\n");
	}
	if (m_obIt != m_listNavigNodePath.end())
	{
		m_listNavigNodePath.erase(m_obIt);
	}
}

//! -------------------------------------------
//! GetCurrentNode
//! -------------------------------------------
void CAINavigPath::DeletePreviousNode( void )
{
	if (m_listNavigNodePath.empty())
	{
		ntPrintf("CAINavigPath::DeleteCurrentNode -> Path List is empty!!\n");
	}
	// Copy the iterator
	AINavigNodeList::iterator m_obItPrev(m_obIt);
	if ( (--m_obItPrev) != m_listNavigNodePath.begin() )
	{
		m_listNavigNodePath.erase(m_obItPrev);
	}
}

//! -------------------------------------------
//! TrimPath
//! -------------------------------------------
unsigned int CAINavigPath::TrimPath ( bool bOn )
{
	if ( !bOn || m_listNavigNodePath.size() < 3 )
	{
		return 0;
	}
	
	unsigned int num = 0;

	// Start with the first node
	PointToFirstNode();
	CPoint pt3DNodeSrc = GetCurrentNode()->GetPos();
	PointToNextNode();
	CPoint pt3DNodeMiddle = GetCurrentNode()->GetPos();
	
	while (PointToNextNode())
	{
		CAINavigNode* node = GetCurrentNode();
		CPoint pt3DNodeEnd = node->GetPos();

        if (CAINavigationSystemMan::Get().HasLineOfSightExcludingGoAroundVolumes(pt3DNodeSrc,pt3DNodeEnd))
		{
			// Almost parallel (remove previous node)
			DeletePreviousNode();
			num++;
		}
		else
		{
			// Not aligned (keep the previous node)
			pt3DNodeSrc = pt3DNodeMiddle;
		}
		pt3DNodeMiddle = pt3DNodeEnd;
	}

	PointToFirstNode();
	return num;
}

//! -------------------------------------------
//! EnhanceFirstNode
//! -------------------------------------------
void CAINavigPath::EnhanceFirstNode ( const CPoint& obEntityPos ) 
{
	if ( size() > 1)
	{
		PointToFirstNode();
		if (CAINavigationSystemMan::Get().HasLineOfSightExcludingGoAroundVolumes(obEntityPos,PeekNextNode()->GetPos()))
		{
			ntPrintf("EnhanceFirstNode Took the NEXT NODE!!!\n");
			PointToNextNode();
		}
	}
}

//! -------------------------------------------
//! GetAvailableCoverPoint
//! -------------------------------------------
CAINavigCoverPoint* CAINavigPath::GetAvailableCoverPoint ( CAIMovement * pMov)
{
	if (!m_listNavigNodePath.empty() && !m_bPathFinished)
	{
		CAINavigNode* pNode = (*m_obIt);
		return (pNode->GetAvailableCoverPoint(pMov));
	}
	return NULL;
}

//! -------------------------------------------
//! PrintPath
//! -------------------------------------------

void CAINavigPath::PrintPath()
{

	ntPrintf("\n");

	if (m_listNavigNodePath.empty())
	{
		ntPrintf("CAINavigPath::PrintPath - EMPTY PATH\n");
		return;
	}
	// Print list
	int idx = 0;
	UNUSED( idx );
	float fDist = 0.f, fTotDist = 0.f;
	AINavigNodeList::iterator obIt		= m_listNavigNodePath.begin();
	AINavigNodeList::iterator obItEnd	= m_listNavigNodePath.end();
	CAINavigNode* pPrevNode = (*obIt);
	for ( ; obIt != obItEnd; ++obIt)
	{
		fDist = (*obIt)->GetDistanceToSourceNode(pPrevNode);
		fTotDist+=fDist;
		ntPrintf("Step [%d] = Node: %s - Dist: %.3f\n",idx++,(*obIt)->GetName().GetDebugString(), fDist );
		pPrevNode = (*obIt);
	}
	ntPrintf("Total Distance = %.3f\n",fTotDist);
}

//! -------------------------------------------
//! RenderPath
//! -------------------------------------------

void CAINavigPath::RenderPath()
{
#ifndef _GOLD_MASTER

	CQuat	obOrientation( CONSTRUCT_IDENTITY );
	static float fRadius = 0.2f;
	#define DBG_Y_OFFSET (CPoint(0.0f,0.5f,0.0f))

	if (m_listNavigNodePath.empty())
	{
		return;
	}
	// Render Path
	CPoint pt3DSource(CONSTRUCT_CLEAR);
	CPoint pt3DDest(CONSTRUCT_CLEAR);
	bool bFirstNode=true;

	// Go through each node
	AINavigNodeList::const_iterator obIt	= m_listNavigNodePath.begin();
	AINavigNodeList::const_iterator obEndIt = m_listNavigNodePath.end();

#ifndef _GOLD_MASTER
	g_VisualDebug->Printf3D( (*obIt)->GetPos()+DBG_Y_OFFSET, DC_RED, 0, "Node:%s",ntStr::GetString((*obIt)->GetName()));
	g_VisualDebug->Printf3D( GetGoalNode()->GetPos()+DBG_Y_OFFSET, DC_RED, 0, "Node:%s",ntStr::GetString(GetGoalNode()->GetName()));
#endif

	for( ; obIt != obEndIt; ++obIt )
	{
		CAINavigNode* pNN = (*obIt);
		pt3DDest = pNN->GetPos();
		// Draw Nodes
		g_VisualDebug->RenderSphere( obOrientation, pt3DDest+DBG_Y_OFFSET,	fRadius, DC_RED );
		// Draw Cover Points
		const AINavigArrowList* pNAL = pNN->GetCoverPointArrows();
		AINavigArrowList::const_iterator obItArr		= pNAL->begin();
		AINavigArrowList::const_iterator obEndItArr	= pNAL->end();
		for( ; obItArr != obEndItArr; ++obItArr )
		{
			CAINavigArrow* pArrow = (*obItArr);
			CAINavigCoverPoint* pCP = pArrow->GetCoverPoint();
			CPoint obPosCP = pCP->GetPos();
			g_VisualDebug->RenderLine( pt3DDest , obPosCP ,DC_GREEN);
			g_VisualDebug->RenderSphere( obOrientation, obPosCP, fRadius, DC_GREEN );
		}
		if (!bFirstNode)
		{	// Draw arrows
			g_VisualDebug->RenderLine( pt3DSource+DBG_Y_OFFSET , pt3DDest+DBG_Y_OFFSET ,DC_RED);
		}
		else bFirstNode = false;
		pt3DSource = pt3DDest;
	}
#endif
}
