//! -------------------------------------------
//! AINavigAStar.h
//!
//! A* Class for the Navigation Graph 
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#include "ainavigastar.h"
#include "ainavigsystemman.h"
#include <limits.h>

#define ECHO_ON 0
 
//! -------------------------------------------------
//! Ctor, et al.
//! -------------------------------------------------
CNavigAStar::CNavigAStar() : m_pobStartAStarNode(0), m_pobGoalAStarNode(0), m_bIsInitialised(true)
{
	;
}

CNavigAStar::~CNavigAStar()
{
	// Free the dynamically allocated A* Nodes
	FreeData();
}

void CNavigAStar::FreeData ( void )
{
	// Free the dynamically allocated A* Nodes
	SNavigAStarNodeList::const_iterator obIt = this->m_OpenList.begin();
	SNavigAStarNodeList::const_iterator obEndIt = this->m_OpenList.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		NT_DELETE_CHUNK( Mem::MC_AI, (*obIt) );
	}
	SNavigAStarNodeList::const_iterator obItCl = this->m_ClosedList.begin();
	SNavigAStarNodeList::const_iterator obEndItCl = this->m_ClosedList.end();
	for( ; obItCl != obEndItCl; ++obItCl )
	{
		NT_DELETE_CHUNK( Mem::MC_AI, *obItCl );
	}
	// Empty lists;
	m_OpenList.clear();
	m_ClosedList.clear();
	// Clear Start-Goal Nodes
	m_pobStartAStarNode = NULL;

	if (m_pobGoalAStarNode)
	{
		NT_DELETE_CHUNK( Mem::MC_AI, m_pobGoalAStarNode);
		m_pobGoalAStarNode  = NULL;
	}

	m_bIsInitialised = true;
}

//! -------------------------------------------------
//! GetLowestCostNodeFromOpenList
//!
//!	 
//! -------------------------------------------------

SNavigAStarNode* CNavigAStar::GetLowestCostNodeFromOpenList( void ) const
{
	if (m_OpenList.empty())
	{
		ntPrintf("CNavigAStar::GetLowestCostNodeFromOpenList -> Empty Open List\n");
		return NULL;
	}

	// Iterate the Open List looking for the minimum cost

	float iMinCost = FLT_MAX-2;
	float iCost = FLT_MAX-2;
	SNavigAStarNode* pobMinScoretAStarNode = NULL;

	SNavigAStarNodeList::const_iterator obIt = this->m_OpenList.begin();
	SNavigAStarNodeList::const_iterator obEndIt = this->m_OpenList.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		iCost = (*obIt)->c;
		if ( iCost < iMinCost )
		{
			iMinCost = iCost;
			pobMinScoretAStarNode = (*obIt);
		}
	}
	return 	pobMinScoretAStarNode;
}

//! -------------------------------------------------
//! AddAStarNode
//!
//!	Adds an A* node to the selected list
//! -------------------------------------------------

void CNavigAStar::AddAStarNode ( SNavigAStarNode* pASNode, ENUM_ASTAR_LIST_TYPE eListType)
{
	switch (eListType)
	{
		case ASTAR_OPEN_LIST:
			m_OpenList.push_back(pASNode);
			if (ECHO_ON) { ntPrintf("ADD_OPEN_LIST: [%s]\n",ntStr::GetString(pASNode->pobNavigNode->GetName())); }
			return;
		case ASTAR_CLOSED_LIST:
			// Add the node to the closed list
			m_ClosedList.push_back(pASNode);	
			// And remove it from the open list
			SNavigAStarNodeList::iterator obIt = ntstd::find(m_OpenList.begin(), m_OpenList.end(), pASNode);
			m_OpenList.erase(obIt);	
			if (ECHO_ON) {ntPrintf("ADD_CLOSE_LIST: [%s]\n",ntStr::GetString(pASNode->pobNavigNode->GetName())); }
			if (ECHO_ON) {ntPrintf("DELETE_OPEN_LIST: [%s]\n",ntStr::GetString(pASNode->pobNavigNode->GetName())); }
			return;			
	}
	ntAssert_p((0), ("CNavigAStar::AddAStarNode -> Unknown List Type selected\n"));
}

////! --------------------------------------------------------------------
////! HasNode
////!
////!	Checks if the selected (open or closed) list has the selected node
////! --------------------------------------------------------------------
//
//bool CNavigAStar::HasNode ( SNavigAStarNode* pASNode, ENUM_ASTAR_LIST_TYPE eListType)
//{
//	switch (eListType)
//	{
//		case ASTAR_OPEN_LIST:
//			return (find( m_OpenList.begin(), m_OpenList.end(), pASNode) != m_OpenList.end());
//		case ASTAR_CLOSED_LIST:
//			return (find( m_ClosedList.begin(), m_ClosedList.end(), pASNode) != m_ClosedList.end());
//		default :
//			ntAssert_p((0), ("CNavigAStar::HasNode -> Unknown List Type selected\n"));
//			return false;
//	}
//}

//! --------------------------------------------------------------------
//! HasNode
//!
//!	Checks if the selected (open or closed) list has the selected node
//! --------------------------------------------------------------------

SNavigAStarNode* CNavigAStar::HasNode ( CAINavigNode* pNavigNode, ENUM_ASTAR_LIST_TYPE eListType)
{
	SNavigAStarNodeList*	pList = (( eListType == ASTAR_OPEN_LIST ) ? &m_OpenList : &m_ClosedList );
	
	SNavigAStarNodeList::iterator obIt		= pList->begin();
	SNavigAStarNodeList::iterator obItEnd	= pList->end();
	for ( ; obIt != obItEnd; ++obIt)
	{ 
		if ( (*obIt)->pobNavigNode == pNavigNode )
		{
			// Found
			return (*obIt);
		}
	}
	// Not found
	return NULL;
}

//! --------------------------------------------------------------------
//! MakePath ( CAINavigPath* pobNavigPath)
//!
//!	Generates an A* path and returnes it in stores it in pobNavigPath
//! This function assumes that pobNavigPath is empty
//! --------------------------------------------------------------------
bool CNavigAStar::MakePath ( CAINavigPath* pobNavigPath )
{
	if (!m_pobGoalAStarNode || !m_pobStartAStarNode)
	{
		ntAssert("CNavigAStar::MakePath -> A* Start and/or Goal node(s) are NULL\n");
		return false;
	}
	if (!m_bIsInitialised)
	{
		ntPrintf("CNavigAStar::MakePath -> Data is not INITIALISED\n");
		return false;
	}

	m_bIsInitialised = false;

	// A* Algorithm implementation
	// ---------------------------

	AddAStarNode(m_pobStartAStarNode,ASTAR_OPEN_LIST);

	while (!m_OpenList.empty())
	{

		SNavigAStarNode* pCurrentAStarNode = GetLowestCostNodeFromOpenList();
		if (ECHO_ON) { ntPrintf("OL_LowestCost : [%s]\n",ntStr::GetString(pCurrentAStarNode->pobNavigNode->GetName())); }

		if ( pCurrentAStarNode->IsEqual(m_pobGoalAStarNode) )
		{
			// Goal Node reached -> generate Path
			
			pobNavigPath->push_front(pCurrentAStarNode->pobNavigNode);
			while ( !pCurrentAStarNode->IsEqual(m_pobStartAStarNode) )
			{
				pCurrentAStarNode = pCurrentAStarNode->pParentNode;
				pobNavigPath->push_front(pCurrentAStarNode->pobNavigNode);
				pobNavigPath->PathReady();
			}
			pobNavigPath->PointToFirstNode();
			return true;
		} 
		else
		{
			// Goal not yet reached -> Add current node to the CLOSED LIST

			AddAStarNode(pCurrentAStarNode,ASTAR_CLOSED_LIST);
			
			// Get New (not in lists) Valid Neighbour NavigNodes and calculate their costs
			const AINavigArrowList* plistTgtArrows = CAINavigationSystemMan::Get().GetNavigGraphManager()->GetTgtArrows(pCurrentAStarNode->pobNavigNode);
			
			// -----------------------------------------------
			// Process each target arrow from the current node
			// -----------------------------------------------

			AINavigArrowList::const_iterator obIt	= plistTgtArrows->begin();
			AINavigArrowList::const_iterator obItEnd = plistTgtArrows->end();
			for ( ; obIt != obItEnd; ++obIt)
			{ 
				// Check that the arrow is enabled
				if ( !((*obIt)->IsEnabled()	) ||
					  ((*obIt)->IsBusy()	)
					)
				{
					continue;
				}

				// Get the Navigation Node pointed by the arrow

				CAINavigNode*		pobReacheableNavigNode	= (*obIt)->GetTgtNode();
				float				fInternodeDistance		= (*obIt)->GetDistance();
				
				// -----------------------------------------------
				// Is node in Open List?
				// -----------------------------------------------

				SNavigAStarNode*	pAStarInList			= HasNode( (pobReacheableNavigNode) ,ASTAR_OPEN_LIST );
				
				if ( pAStarInList )
				{
#ifndef _RELEASE
					ntPrintf("NODE [%s] already in OPEN LIST\n",ntStr::GetString(pobReacheableNavigNode->GetName()));
#endif
					// Calculate cost with the new node
					CDirection Line(m_pobGoalAStarNode->pobNavigNode->GetPos() - pobReacheableNavigNode->GetPos());
					float fNewS		= pCurrentAStarNode->s + fInternodeDistance;
					float fNewH		= Line.Length();
					float fNewCost	= fNewS + fNewH; // distance to node + heuristic
					if (pAStarInList->c > fNewCost)
					{
						// New path has lower cost. Update info.
						pAStarInList->pParentNode = pCurrentAStarNode;
						pAStarInList->s = fNewS;
						pAStarInList->h = fNewH;
						pAStarInList->c = fNewCost;
					}
					continue;
				}

				// -----------------------------------------------
				// Is node in Close List?
				// -----------------------------------------------

				pAStarInList = HasNode( (pobReacheableNavigNode) ,ASTAR_CLOSED_LIST );

				if ( pAStarInList )
				{
#ifndef _RELEASE
					ntPrintf("NODE [%s] already in CLOSE LIST\n",ntStr::GetString(pobReacheableNavigNode->GetName()));
#endif
					// Calculate cost with the new node
					CDirection Line(m_pobGoalAStarNode->pobNavigNode->GetPos() - pobReacheableNavigNode->GetPos());
					float fNewS		= pCurrentAStarNode->s + fInternodeDistance;
					float fNewH		= Line.Length();
					float fNewCost	= fNewS + fNewH; // distance to node + heuristic
					if (pAStarInList->c > fNewCost)
					{
						// New path has lower cost. Update info.
						pAStarInList->pParentNode = pCurrentAStarNode;
						pAStarInList->s = fNewS;
						pAStarInList->h = fNewH;
						pAStarInList->c = fNewCost;

						// UpdateParents
					}
					continue;
				}
				
				// -----------------------------------------------
				// New Node
				// -----------------------------------------------
				float c = 0 , s  = 0 , h = 0;
				s = pCurrentAStarNode->s + fInternodeDistance;
				h = pobReacheableNavigNode->GetDistanceToPoint(m_pobGoalAStarNode->pobNavigNode->GetPos(), DT_EUCLIDES );	
				c = s + h;
				SNavigAStarNode* pAStarNode = NT_NEW_CHUNK( Mem::MC_AI ) SNavigAStarNode ( pCurrentAStarNode,
																		pobReacheableNavigNode,
																		c,
																		s,
																		h);
				if (ECHO_ON) { ntPrintf("CREATING A* NODE for: [%s] from [%s] - s: %f, h: %f, c: %f\n",
												ntStr::GetString(pobReacheableNavigNode->GetName()),
												ntStr::GetString(pCurrentAStarNode->pobNavigNode->GetName()),
                                                s,h,c); 
				}
				AddAStarNode(pAStarNode,ASTAR_OPEN_LIST);
			}
		}
	} // WHILE_END
	return false;
}

//! --------------------------------------------------------------------
//! void CNavigAStar::GetAStarNodeContaining CAINavigNode* pNavigNode, ENUM_ASTAR_LIST_TYPE eListType) const
//!
//!
//! --------------------------------------------------------------------
SNavigAStarNode* CNavigAStar::GetAStarNodeContaining(CAINavigNode* pNavigNode, ENUM_ASTAR_LIST_TYPE eListType) const
{
	const SNavigAStarNodeList* pList = (( eListType == ASTAR_OPEN_LIST ) ? &m_OpenList : &m_ClosedList );
	SNavigAStarNodeList::const_iterator obIt	= pList->begin();
	SNavigAStarNodeList	::const_iterator obItEnd = pList->end();
	for ( ; obIt != obItEnd; ++obIt)
	{ 
		if ( (*obIt)->pobNavigNode == pNavigNode )
		{
			return (*obIt);
		}
	}
	return NULL;
}


//! --------------------------------------------------------------------
//! inline void SetStartEndNodes( CAINavigNode* pobStart, CAINavigNode* pobEnd )
//! --------------------------------------------------------------------
void CNavigAStar::SetStartEndNodes( CAINavigNode* pobStart, CAINavigNode* pobEnd )
{
	m_pobStartAStarNode = NT_NEW_CHUNK( Mem::MC_AI ) SNavigAStarNode ( NULL, pobStart, 0, 0, 0);
	m_pobGoalAStarNode  = NT_NEW_CHUNK( Mem::MC_AI ) SNavigAStarNode ( NULL, pobEnd, 0, 0, 0);
}

//! --------------------------------------------------------------------
//! bool CNavigAStar::MakePath ( CPoint* pobPs, CPoint* pobPe, CAINavigPath* pNavigPath )
//!
//! Generates a Path between 2 nodes (NOT NEEDED NOW)
//! --------------------------------------------------------------------
//bool CNavigAStar::MakePathWithConstraints ( CAINavigNode* pobNodeStart, CAINavigNode* pobNodeEnd, CAINavigPath* pobNavigPath )
//{
//	// Is there any path constraints? (i.e. intermediate nodes to pass through)
//	AINavigNodeList* pIN = pobNavigPath->GetIntermediateNodes();
//
//	if ( pIN == NULL ) return (MakePath (pobNodeStart, pobNodeEnd, pobNavigPath));
//
//	// Go Step By Step then...
//
//	CAINavigPath 
//	SNavigAStarNodeList::const_iterator obIt	= pList->begin();
//	SNavigAStarNodeList::const_iterator obItEnd = pList->end();
//	for ( ; obIt != obItEnd; ++obIt)
//	{ 
//		if ( (*obIt)->pobNavigNode == pNavigNode )
//		{
//			return (*obIt);
//		}
//	}
//	return NULL;
//
//
//}

//! --------------------------------------------------------------------
//! bool CNavigAStar::MakePath ( CPoint* pobPs, CPoint* pobPe, CAINavigPath* pNavigPath )
//!
//! Generates a Path between 2 nodes
//! --------------------------------------------------------------------
bool CNavigAStar::MakePath ( CAINavigNode* pobNodeStart, CAINavigNode* pobNodeEnd, CAINavigPath* pobNavigPath )
{
	// Ensure that the path container is empty
	if (!pobNavigPath->empty())
	{
		pobNavigPath->clear();
	}

	// Initialise node-validity info
	pobNavigPath->SetEndNodeValid(true);
	pobNavigPath->SetStartNodeValid(true);
	pobNavigPath->SetDifNavigGraphError(false);

	// Start checkings
	if (!pobNodeStart && !pobNodeEnd)
	{
		pobNavigPath->SetEndNodeValid(false);
		pobNavigPath->SetStartNodeValid(false);
		ntPrintf("CNavigAStar::MakePath -> NULL start AND end node passed\n");
		return false;
	}

	if (!pobNodeStart )
	{
		pobNavigPath->SetStartNodeValid(false);
		ntPrintf("CNavigAStar::MakePath -> NULL start node passed\n");
	}

	if ( !pobNodeEnd )
	{
		pobNavigPath->SetEndNodeValid(false);
		ntPrintf("CNavigAStar::MakePath -> NULL end node passed\n");
	}


	if ( pobNodeStart && pobNodeEnd && pobNodeStart->GetParentNavigGraph() != pobNodeEnd->GetParentNavigGraph() )
	{
		pobNavigPath->SetDifNavigGraphError(true);
		ntPrintf("CNavigAStar::MakePath -> START [%s:%s]- END [%s:%s] Nodes do not belong to the same NavigGraph!!\n",
			ntStr::GetString(pobNodeStart->GetParentNavigGraph()->GetName()),
			ntStr::GetString(pobNodeStart->GetName()),
			ntStr::GetString(pobNodeEnd->GetParentNavigGraph()->GetName()),
			ntStr::GetString(pobNodeEnd->GetName())			
			);
		return false;
	}

	if ( !pobNodeStart || !pobNodeEnd )
	{
		return false;
	}

	// Store node-validity info
	pobNavigPath->SetEndNodeValid(true);
	pobNavigPath->SetStartNodeValid(true);
	pobNavigPath->SetDifNavigGraphError(false);

	SetStartEndNodes(pobNodeStart, pobNodeEnd);

	// Generate path, free the dynamic space and lists used in the search, and return
	bool bPathFound = MakePath( pobNavigPath );
	FreeData();
	return (bPathFound);
}

//! --------------------------------------------------------------------
//! bool CNavigAStar::MakePath ( CPoint* pobPs, CAINavigNode* pobNodeEnd, CAINavigPath* pNavigPath )
//!
//!
//! --------------------------------------------------------------------
bool CNavigAStar::MakePath ( const CPoint& obPs, CAINavigNode* pobNodeEnd, CAINavigPath* pobNavigPath )
{
	float fDist = -1.0f;

	// Translate start point into node
	CAINavigNode* pobNodeStart = CAINavigationSystemMan::Get().GetNavigGraphManager()->GetClosestNode(obPs, &fDist);
	
	return ( MakePath(pobNodeStart, pobNodeEnd, pobNavigPath) );
}

//! --------------------------------------------------------------------
//! bool CNavigAStar::MakePath ( CPoint* pobPs, CPoint* pobPe, CAINavigPath* pNavigPath )
//!
//!
//! --------------------------------------------------------------------
bool CNavigAStar::MakePath ( const CPoint& obPs, const CPoint& obPe, CAINavigPath* pobNavigPath )
{
	float fDist = -1.0f;

	// Translate points into nodes
	CAINavigNode*		pobNodeStart	= CAINavigationSystemMan::Get().GetNavigGraphManager()->GetClosestNode(obPs, &fDist);
	CAINavigNode*		pobNodeEnd		= CAINavigationSystemMan::Get().GetNavigGraphManager()->GetClosestNode(obPe, &fDist);

	return ( MakePath(pobNodeStart, pobNodeEnd, pobNavigPath) );
}
