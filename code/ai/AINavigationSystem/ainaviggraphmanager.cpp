//! --------------------------------------------------------
//! AINavigGraphManager.cpp
//!
//! New AIs Navigation Grpah Manager
//!
//! It handles AIs navigation system for the whole level 
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//! Date	04/05/06 - Creation
//!--------------------------------------------

#include "ainaviggraphmanager.h"
#include "game/aicomponent.h"
#include "game/entitymanager.h"

#include "aipatrolgraph.h"

// For Debuging
#include "core/visualdebugger.h"
#include "ainavigsystemman.h"

//! ------------------------------------------------------------------------------------------
//! CAINavigGraphManager::CAINavigGraphManager()
//!
//! Constructor
//! ------------------------------------------------------------------------------------------

CAINavigGraphManager::CAINavigGraphManager() :
		m_iNumberOfRegionsInLevel(0),
		m_iNumberOfPatrolGraphsInLevel(0)
{
	;
}

CAINavigGraphManager::~CAINavigGraphManager()
{
	// Free Dynamic Data ( All the CAIPatrolGraph(s) )
	AIPatrolGraphList::const_iterator obIt		= m_listPatrolGraphs.begin();
	AIPatrolGraphList::const_iterator obEndIt	= m_listPatrolGraphs.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		NT_DELETE_CHUNK( Mem::MC_AI, (*obIt) );
	}
}

//! ------------------------------------------------------------------------------------------------
//! IsLUADisabled
//! ------------------------------------------------------------------------------------------------
bool CAINavigGraphManager::IsNavigGraphActiveLUA ( CHashedString hsNGName )
{
	CAINavigGraph* pNG = GetNavigGraph(hsNGName);
	return (pNG ? pNG->IsNavigGraphActiveLUA() : false);
}

//! ------------------------------------------------------------------------------------------------
//! SetLUADisabled
//! ------------------------------------------------------------------------------------------------
void CAINavigGraphManager::SetNavigGraphActiveLUA ( CHashedString hsNGName, bool bOn )
{
	CAINavigGraph* pNG = GetNavigGraph(hsNGName);
	if (pNG)
		pNG->SetNavigGraphActiveLUA(bOn);
}

//! ------------------------------------------------------------------------------------------------
//! GetNavigGraph
//! ------------------------------------------------------------------------------------------------
CAINavigGraph* CAINavigGraphManager::GetNavigGraph ( CHashedString hsNGName )
{
	AINavigGraphList::const_iterator obIt		= m_listNavigGraphs.begin();
	AINavigGraphList::const_iterator obEndIt	= m_listNavigGraphs.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		CAINavigGraph* pNG = (*obIt);
		if ( pNG->GetName() == hsNGName )
		{
			return pNG;
		}
	}
	return NULL;
}

//! ------------------------------------------------------------------------------------------------
//! void CAINavigGraphManager::Add( CAINavigGraph* pNG)
//!
//! 
//! ------------------------------------------------------------------------------------------------

void CAINavigGraphManager::Add ( CAINavigGraph* pNG)
{
	if (pNG) 
	{ 
		if (pNG->IsPatrolGraph())
		{
			m_iNumberOfPatrolGraphsInLevel++;
	
			// Generate a CAIPatrolGrahp from the CAINavigGraph and add it to the list
			CAIPatrolGraph* pPG = NT_NEW_CHUNK( Mem::MC_AI ) CAIPatrolGraph(pNG);
			m_listPatrolGraphs.push_back(pPG);
		}
		else if (pNG->IsQueueGraph())
		{
			m_listQueueGraphs.push_back(pNG);
		}
		else
		{
			m_iNumberOfRegionsInLevel++; 
			m_listNavigGraphs.push_back(pNG);
		}
	}
	else
	{
		ntAssert_p(0, ("CAINavigGraphManager::Add -> NULL Nav Region passed\n"));
	}
}

//! ------------------------------------------------------------------------------------------------
//! CAINavigNode* CAINavigGraphManager::GetNavigGraphArea( const char* psAreaName ) 
//!
//! Returns the area that matches the name: psAreaName, or NULL if not found.
//! ------------------------------------------------------------------------------------------------
//CAINavigGraph* CAINavigGraphManager::GetNavigGraphArea( const char* psAreaName )
//{
//	if (!psAreaName) 
//	{
//		ntPrintf("CAINavigGraphManager::GetNavigGraphArea -> NULL parameter received (psAreaName)\n");
//		return NULL;
//	}
//	// Go through the list of areas and get the one with the psAreaName
//	CHashedString ksAreaName(psAreaName);
//
//	AINavigGraphList::const_iterator obItArea		= m_listNavigGraphs.begin();
//	AINavigGraphList::const_iterator obEndItArea	= m_listNavigGraphs.end();
//	for( ; obItArea != obEndItArea; ++obItArea )
//	{
//		if ( (*obItArea)->GetName() == ksAreaName )
//		{
//			return (*obItArea);
//		}
//	}
//	// Area Not found
//	ntPrintf("CAINavigGraphManager::GetNavigGraphArea -> Area ($s) not found. Number of areas: [%d]\n",psAreaName, this->m_listNavigGraphs.size());
//	return NULL;
//}

//! ------------------------------------------------------------------------------------------------
//! CAINavigNode* CAINavigGraphManager::GetPatrolGraph( const char* psName ) 
//!
//! Returns the patrol graph that matches the name: psName, or NULL if not found.
//! ------------------------------------------------------------------------------------------------
CAIPatrolGraph*	CAINavigGraphManager::GetPatrolGraph ( const char* psName )
{
	if (!psName) return NULL;

	CHashedString ksPatrolGraphName(psName);

	AIPatrolGraphList::const_iterator obIt		= m_listPatrolGraphs.begin();
	AIPatrolGraphList::const_iterator obEndIt	= m_listPatrolGraphs.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		if ( (*obIt)->GetName() == ksPatrolGraphName )
		{
			return (*obIt);
		}
	}
	// Area Not found
	ntPrintf("CAINavigGraphManager::GetPatrolGraph -> Patrol Path (%s) not found. Number of Patrol Paths: [%d]\n", ntStr::GetString(ksPatrolGraphName), m_listPatrolGraphs.size());
	return NULL;
}

//! ------------------------------------------------------------------------------------------------
//! AddPatroller
//!
//! Adds an Entity to a Patrol Path
//! ------------------------------------------------------------------------------------------------
void CAINavigGraphManager::AddPatroller( AI* pEnt , const char* psName )
{
	if (!pEnt || !psName) return;

	CAIPatrolGraph* pPG = GetPatrolGraph(psName);
	if (!pPG) return;

	pPG->AddPatroller(pEnt);

	pEnt->GetAIComponent()->GetCAIMovement()->SetPatrolGraph(pPG);
}

//! ------------------------------------------------------------------------------------------------
//! RemovePatroller
//!
//! Removes an Entity from a Patrol Path
//! ------------------------------------------------------------------------------------------------
void CAINavigGraphManager::RemovePatroller ( CEntity* pEnt, const char* psName )
{
	if (!pEnt || !psName) return;

	CAIPatrolGraph* pPG = GetPatrolGraph(psName);
	if (!pPG) return;

	pPG->RemovePatroller(pEnt);
}

//! ------------------------------------------------------------------------------------------------
//! CAINavigNode* CAINavigGraphManager::GetNode( CEntity* pEnt ) 
//!
//! Returns the node that matches pEnt, or NULL if not found.
//! ------------------------------------------------------------------------------------------------
//CAINavigNode* CAINavigGraphManager::GetNode( CEntity* pEnt, CAINavigGraph* pNG)
//{
//	AINavigGraphList::iterator obIt		= m_listNavigGraphs.begin();
//	AINavigGraphList::iterator obItEnd	= m_listNavigGraphs.end();
//	for ( ; obIt != obItEnd; ++obIt)
//	{
//		CAINavigNode* pNode = (*obIt)->GetNode(pEnt);
//		if (pNode) 
//		{
//			pNG = (*obIt);
//			return pNode;
//		}
//	}
//	// Not found
//	return NULL;
//}

//! ------------------------------------------------------------------------------------------------
//! CAINavigNode* CAINavigGraphManager::GetDoor( CEntity* pEnt ) 
//!
//! Returns the door that matches pEnt, or NULL if not found.
//! ------------------------------------------------------------------------------------------------
CAINavigArrow* CAINavigGraphManager::GetDoor( CEntity* pEnt )
{
	AINavigGraphList::iterator obIt		= m_listNavigGraphs.begin();
	AINavigGraphList::iterator obItEnd	= m_listNavigGraphs.end();
	for ( ; obIt != obItEnd; ++obIt)
	{
		CAINavigArrow* pArrow = (*obIt)->GetDoor(pEnt);
		if (pArrow) return pArrow;
	}
	// Not found
	return NULL;
}

//! ------------------------------------------------------------------------------------------------
//! CAINavigNode* CAINavigGraphManager::GetLadder( CEntity* pEnt ) 
//!
//! Returns the ladder that matches pEnt, or NULL if not found.
//! ------------------------------------------------------------------------------------------------
CAINavigArrow* CAINavigGraphManager::GetLadder( CEntity* pEnt )
{
	AINavigGraphList::iterator obIt		= m_listNavigGraphs.begin();
	AINavigGraphList::iterator obItEnd	= m_listNavigGraphs.end();
	for ( ; obIt != obItEnd; ++obIt)
	{
		CAINavigArrow* pArrow = (*obIt)->GetLadder(pEnt);
		if (pArrow) return pArrow;
	}
	// Not found
	return NULL;
}

//! ------------------------------------------------------------------------------------------------
//! CAINavigNode* CAINavigGraphManager::IsDoorOpen( CEntity* pEnt ) 
//! ------------------------------------------------------------------------------------------------
bool CAINavigGraphManager::IsDoorOpen( CEntity* pEnt )
{
	return (this->GetDoor(pEnt)->IsEnabled());
}

//! ------------------------------------------------------------------------------------------------
//! CAINavigNode* CAINavigGraphManager::IsLadderAvailable( CEntity* pEnt ) 
//! ------------------------------------------------------------------------------------------------
bool CAINavigGraphManager::IsLadderAvailable( CEntity* pEnt )
{
	return ( (this->GetDoor(pEnt)->IsEnabled()) && !(this->GetDoor(pEnt)->IsBusy()) );
}

//! ------------------------------------------------------------------------------------------
//! void CAINavigNode::GetClosestNode( CPoint pobPoint ) !!! - CAINavigGraph should be a parameter and the search constrained to only the selected one!!!!
//!
//! Returns the closest node to the given coordinates.
//! ------------------------------------------------------------------------------------------
CAINavigNode* CAINavigGraphManager::GetClosestNode( const CPoint& obPoint, float* fDist  )  
{
	*fDist = FLT_MAX;
	CAINavigNode* pClosestNode = NULL;

	AINavigGraphList::iterator obIt		= m_listNavigGraphs.begin();
	AINavigGraphList::iterator obItEnd	= m_listNavigGraphs.end();
	for ( ; obIt != obItEnd; ++obIt)
	{
		CAINavigGraph* pNG = (*obIt);

		if (pNG->IsLocatorGraph() || pNG->IsQueueGraph() || pNG->IsPatrolGraph() || !pNG->IsNavigGraphActiveLUA())
			continue;

		float fDistanceMnh = FLT_MAX;
		CAINavigNode* pNode = (*obIt)->GetClosestNode(obPoint, &fDistanceMnh);
		if (fDistanceMnh < *fDist ) 
		{
			*fDist			= fDistanceMnh;
			pClosestNode	= pNode;
		}
	}
	return (pClosestNode);
}

//! ------------------------------------------------------------------------------------------
//! bool CAINavigGraphManager::DeleteNode( CEntity* pEnt ) 
//!
//! Deletes a node (useful if a ladder is broken or a door is permanently locked
//! ------------------------------------------------------------------------------------------
//bool CAINavigGraphManager::DeleteNode( CEntity* pEnt )
//{
//	CAINavigGraph*	pNG		= NULL;
//	this->GetNodePointer(pEnt, pNG);
//	
//	return (pNG->DeleteNode(pEnt));
//}

//! ------------------------------------------------------------------------------------------
//! ntstd::List<CAINavigNode*>*	CAINavigGraphManager::GetTargetNodes ( CEntity* pEnt, CAINavigGraph* pNG) 
//!
//! 
//! ------------------------------------------------------------------------------------------
const AINavigArrowList*	CAINavigGraphManager::GetTgtArrows ( CAINavigNode* pobNode ) const
{
	if(!pobNode) { return NULL; }
	return (pobNode->GetTgtArrows());
}

//------------------------------------------------------------------------------------------
//!	GetPatrollersGraph
//------------------------------------------------------------------------------------------
CAINavigGraph* CAINavigGraphManager::GetPatrollersGraph	( CEntity* pEnt)
{
	if (!pEnt) return NULL;

	AIPatrolGraphList::iterator obIt	= m_listPatrolGraphs.begin();
	AIPatrolGraphList::iterator obItEnd	= m_listPatrolGraphs.end();
	for ( ; obIt != obItEnd; ++obIt)
	{
		CAINavigGraph* pNG = (*obIt)->GetPatrollersGraph(pEnt);
		if (pNG) return pNG;
	}
	// Not found
	return NULL;
}


//------------------------------------------------------------------------------------------
//!	DebugRenderNavigGraph
//------------------------------------------------------------------------------------------

void CAINavigGraphManager::DebugRenderNavigGraph()
{
#ifndef _GOLD_MASTER
	g_VisualDebug->Printf2D(10,10,DC_PURPLE,0,"Total Navigation Regions in Level: [%d] (Dario)", m_iNumberOfRegionsInLevel);

	AINavigGraphList::iterator obIt		= m_listNavigGraphs.begin();
	AINavigGraphList::iterator obItEnd	= m_listNavigGraphs.end();
	for ( ; obIt != obItEnd; ++obIt)
	{
		if (!(*obIt)->IsInActiveSector())
			continue;
		(*obIt)->DebugRender();

	}

	obIt		= m_listQueueGraphs.begin();
	obItEnd		= m_listQueueGraphs.end();
	for ( ; obIt != obItEnd; ++obIt)
	{
		if (!(*obIt)->IsInActiveSector())
			continue;
		(*obIt)->DebugRender();

	}
#endif	
}

//------------------------------------------------------------------------------------------
//!	DebugRenderPathGraph
//------------------------------------------------------------------------------------------

void CAINavigGraphManager::DebugRenderPathGraph()
{
	if (!m_listPatrolGraphs.empty())
	{
		(*(m_listPatrolGraphs.begin()))->DebugRender();
	}
}

//------------------------------------------------------------------------------------------
//!	MakePath (s)
//------------------------------------------------------------------------------------------
void CAINavigGraphManager::MakePath ( const CPoint& obPs, const CPoint& obPe, CAINavigPath* pobNavigPath )
{
	m_obAStar.MakePath(obPs,obPe,pobNavigPath);
}

void CAINavigGraphManager::MakePath ( const CPoint& obPs, CAINavigNode* pNe, CAINavigPath* pobNavigPath )
{
	m_obAStar.MakePath(obPs,pNe,pobNavigPath);
}

void CAINavigGraphManager::MakePath ( CAINavigNode* obNs, CAINavigNode* pNe, CAINavigPath* pobNavigPath )
{
	m_obAStar.MakePath(obNs,pNe,pobNavigPath);
}

void CAINavigGraphManager::MakePath ( const CPoint& obPs, const CHashedString & obNodeKName, CAINavigPath* pobNavigPath )
{
	CAINavigNode* pNN = GetNodeWithName(obNodeKName);

	if (pNN) m_obAStar.MakePath(obPs,pNN,pobNavigPath);
}

//! ------------------------------------------------------------------------------------------
//! GetNodeWithName
//! ------------------------------------------------------------------------------------------
CAINavigNode* CAINavigGraphManager::GetNodeWithName(const CHashedString & obNodeKName)
{
	if (m_listNavigGraphs.empty()) return NULL;

	CAINavigNode* pNN = NULL;

	AINavigGraphList::iterator obIt		= m_listNavigGraphs.begin();
	AINavigGraphList::iterator obItEnd	= m_listNavigGraphs.end();
	for ( ; obIt != obItEnd; ++obIt)
	{
		if (!(*obIt)->IsInActiveSector())
			continue;
		pNN = (*obIt)->GetNodeWithName(obNodeKName);
		if ( pNN ) return (pNN);
	}
	return NULL;
}


//! ------------------------------------------------------------------------------------------
//! GetNodePosWithName
//! ------------------------------------------------------------------------------------------
CPoint CAINavigGraphManager::GetNodePosWithName(const CHashedString & obNodeKName, bool * bFound, float * fRadius)
{
	CAINavigNode* pNode = GetNodeWithName(obNodeKName);
	
	if (!pNode) 
	{
		*bFound = false;
		*fRadius = 0;
		return CPoint(CONSTRUCT_CLEAR);
	}
	*bFound = true;
	*fRadius = pNode->GetRadiusSQR();
	return (pNode->GetPos());
}

//! ------------------------------------------------------------------------------------------
//! GetANodeLinkedToCoverPoint
//! ------------------------------------------------------------------------------------------
CAINavigNode* CAINavigGraphManager::GetANodeLinkedToCoverPoint	( CAINavigCoverPoint* pCP, CAINavigGraph* pNG ) const
{
	if ( !pCP )	return NULL;

	if ( pNG ) return (pNG->GetANodeLinkedToCoverPoint(pCP));

	AINavigGraphList::const_iterator obIt		= m_listNavigGraphs.begin();
	AINavigGraphList::const_iterator obEndIt	= m_listNavigGraphs.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		CAINavigGraph* pNavigGraph = (*obIt);
		if (!(*obIt)->IsInActiveSector())
			continue;
		CAINavigNode* pN = pNavigGraph->GetANodeLinkedToCoverPoint(pCP);
		if (pN)	return pN;

	}

	return NULL;
}
//! ------------------------------------------------------------------------------------------
//! GetClosestCoverPointInMinMaxRange
//! ------------------------------------------------------------------------------------------
CAINavigCoverPoint* CAINavigGraphManager::GetClosestCoverPointInMinMaxRange	( AI* pAI, const CEntity* pEnemy, CAINavigGraph* pNG )
{
	if ( !pAI || !pEnemy )	return NULL;

	if ( pNG ) return (pNG->GetClosestCoverPointInMinMaxRange(pAI, pEnemy));

	AINavigGraphList::const_iterator obIt		= m_listNavigGraphs.begin();
	AINavigGraphList::const_iterator obEndIt	= m_listNavigGraphs.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		CAINavigGraph* pNavigGraph = (*obIt);
		if (!(*obIt)->IsInActiveSector())
			continue;
		CAINavigCoverPoint* pCP = pNavigGraph->GetClosestCoverPointInMinMaxRange(pAI, pEnemy);
		if (pCP) return pCP;
	}

	return NULL;
}

//! ------------------------------------------------------------------------------------------
//! GetFirstVisibleNode
//! ------------------------------------------------------------------------------------------
CAINavigNode* CAINavigGraphManager::GetFirstVisibleNode( const CPoint &obPoint, CAINavigGraph* pNG ) 
{
	if ( pNG ) return (pNG->GetFirstVisibleNode(obPoint));
	
	AINavigGraphList::const_iterator obIt		= m_listNavigGraphs.begin();
	AINavigGraphList::const_iterator obEndIt	= m_listNavigGraphs.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		CAINavigGraph* pNavigGraph = (*obIt);
		if (!(*obIt)->IsInActiveSector())
			continue;
		CAINavigNode* pNN = pNavigGraph->GetFirstVisibleNode(obPoint);
		if (pNN) return pNN;
	}

	return NULL;
}

//! ------------------------------------------------------------------------------------------
//! GetAbsoluteClosestNode
//! ------------------------------------------------------------------------------------------
CAINavigNode* CAINavigGraphManager::GetAbsoluteClosestNode( const CPoint &obPoint, CAINavigGraph* pNG ) 
{
	float fMinDist = FLT_MAX - 2.0f;

	// If a CAINavigGraph is given, use it
	if ( pNG ) 
		return (pNG->GetAbsoluteClosestNode(obPoint,&fMinDist));
	
	// Otherwise use look through the whole set of naviggraphs
	CAINavigNode* pAbsClosestNode = NULL;

	AINavigGraphList::const_iterator obIt		= m_listNavigGraphs.begin();
	AINavigGraphList::const_iterator obEndIt	= m_listNavigGraphs.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		if (!(*obIt)->IsInActiveSector())
			continue;
		float fDist = FLT_MAX - 2.0f;
		CAINavigGraph* pNavigGraph = (*obIt);
		CAINavigNode* pNN = pNavigGraph->GetAbsoluteClosestNode(obPoint, &fDist);
		if ( pNN && (fDist < fMinDist) )
		{
			fMinDist = fDist;
			pAbsClosestNode = pNN;
		}
	}

	return pAbsClosestNode;
}

//! ------------------------------------------------------------------------------------------
//! GetIntermediateNodeList
//! ------------------------------------------------------------------------------------------
//AINavigNodeList* CAINavigGraphManager::GetIntermediateNodeList ( CHashedString hsName, CAINavigGraph* pNG )
//{
//	if ( pNG ) return (pNG->GetIntermediateNodeList(hsName));
//
//	AINavigGraphList::const_iterator obIt		= m_listNavigGraphs.begin();
//	AINavigGraphList::const_iterator obEndIt	= m_listNavigGraphs.end();
//	for( ; obIt != obEndIt; ++obIt )
//	{
//		CAINavigGraph* pNavigGraph = (*obIt);
//
//		AINavigNodeList* pNNS = pNavigGraph->GetIntermediateNodeList(hsName);
//		if (pNNS) return pNNS;
//	}
//
//	return NULL;
//}



