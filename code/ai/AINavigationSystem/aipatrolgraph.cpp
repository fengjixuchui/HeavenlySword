//! -------------------------------------------
//! AIPatrolPath.cpp
//!
//! Navigation path for patroling 
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#include "aipatrolgraph.h"
#include "ainaviggraph.h"
#include "game/messagehandler.h"
#include "core/visualdebugger.h"
#include "ainavigpath.h"

//!--------------------------------------------
//!	Constructor , et al.
//!--------------------------------------------

CAIPatrolGraph::CAIPatrolGraph( CAINavigGraph* pNG ) : m_pNavigGraph(pNG),
													   m_bOpenGraph(false), m_uiNumOfPatrollers(0), m_uiGraphSize(0)
{	
	if (!pNG) return;

	m_uiGraphSize = pNG->GetNodesList()->size();
	if ( m_uiGraphSize < 2 ) { m_uiGraphSize = 0; return ; }
	m_vNavigNodePath.reserve(m_uiGraphSize);
	
	// Store the path in a vector
	AINavigNodeList::const_iterator obIt	= pNG->GetNodesList()->begin();
	for ( unsigned int i = 0; i < m_uiGraphSize; ++i )
	{
		m_vNavigNodePath.push_back(*obIt);
		obIt++;
	}
	// Records the type of patrol graph;
	m_bOpenGraph = pNG->IsOpenPatrolGraph();

	// Stores the name
	m_ksName = pNG->GetName();

	// Prepare Patrollers' Vector

	m_vPatrollers.reserve(MAX_PATROLLERS_PER_PATROL_PATH);	// !!! - (Dario) this must be parametrised

}

CAIPatrolGraph::~CAIPatrolGraph()
{
	// Release the Dyn. Allocated Memory

	for (unsigned int i = 0; i < m_uiNumOfPatrollers; i++)
	{
		NT_DELETE_CHUNK( Mem::MC_AI, m_vPatrollers[i]);
	}
}

//!--------------------------------------------
//!	AlreadyAdded
//!--------------------------------------------
int CAIPatrolGraph::GetPatrollerIndex ( CEntity* pEnt )
{
	if (!pEnt) return -1;

	for (unsigned int i = 0; i < m_uiNumOfPatrollers; i++)
	{
		if (m_vPatrollers[i]->pEntPatroller == pEnt) return i;
	}
	return -1;
}

//!--------------------------------------------
//!	GetNodeIndex
//!--------------------------------------------
int CAIPatrolGraph::GetNodeIndex ( CAINavigNode* pNode)
{
	for (unsigned int i = 0; i < m_uiGraphSize; i++)
	{
		if (m_vNavigNodePath[i] == pNode) return i;
	}
	return -1;
}

//!--------------------------------------------
//!	AddPatroller
//!--------------------------------------------
bool CAIPatrolGraph::AddPatroller ( CEntity* pEnt )
{
	if (!pEnt) return false;

	if (m_uiNumOfPatrollers > MAX_PATROLLERS_PER_PATROL_PATH) 
	{
		ntPrintf("AddPatroller: Too many patrollers [%d] (max = %d) for this patrol graph [%s]",	m_uiNumOfPatrollers, 
																									MAX_PATROLLERS_PER_PATROL_PATH,
																									ntStr::GetString(GetName())
																									);
		return false;
	}

	if (GetPatrollerIndex(pEnt) < 0)
	{
		// Find wich node is the closest to the AI's location
		float fDist = -1;
		CAINavigNode* pNN = m_pNavigGraph->GetClosestNode( pEnt->GetPosition(), &fDist );
		int iNodeIndex = GetNodeIndex(pNN);
		if ( (fDist > 200.0f) || (iNodeIndex<0) ) // !!! - (Dario) this must be parametrised
		{
			ntPrintf("AddPatroller: Patroller faaar,faaar away from home...\n");
			// Take first node;
			iNodeIndex = 0;
			if (!pNN) pNN = m_vNavigNodePath[0];
			fDist = -1;
			// May be do something else here?
		}

		SPatroller* pP = NT_NEW_CHUNK( Mem::MC_AI ) SPatroller(pEnt,iNodeIndex);
		if (!pP) return false;

		m_vPatrollers[m_uiNumOfPatrollers++] = pP;
		ntPrintf("Patroller [%s] added to patrol path [%s]. Starting Node : [%s]. Distance : %0.2f\n", ntStr::GetString(pEnt->GetName()),
																										ntStr::GetString(m_pNavigGraph->GetName()),
																										ntStr::GetString(pNN->GetName()),
																										fDist);
		return true;
		
	}
	return false;
}

//!--------------------------------------------
//!	RemovePatroller
//!--------------------------------------------
bool CAIPatrolGraph::RemovePatroller ( CEntity* pEnt )
{
	if (!pEnt) return false;
	
	int iIndex = -1;

	if ( (iIndex = GetPatrollerIndex(pEnt)) >= 0 )
	{
		m_uiNumOfPatrollers--;
		NT_DELETE_CHUNK( Mem::MC_AI, m_vPatrollers[iIndex]);
		return true;
	}
	return false;
}

//!--------------------------------------------
//!	GetCurrentNode
//!--------------------------------------------
CAINavigNode* CAIPatrolGraph::GetCurrentNode ( CEntity*	pEnt )
{
	if (!pEnt) return NULL;

	int iIndex = GetPatrollerIndex(pEnt);

	if (iIndex<0) return NULL;

	int iNodeIndex = m_vPatrollers[iIndex]->uiCurrentNodeIndex;

	return (m_vNavigNodePath[iNodeIndex]); 
}

//! -------------------------------------------
//! PointToNextNode
//! -------------------------------------------
void CAIPatrolGraph::PointToNextNode ( CEntity* pEnt )
{
	int iIndex = -1;

	if ( (iIndex = GetPatrollerIndex(pEnt)) < 0 ) return;

	// Check first if the current node has an Investigate Trigger

	GameEvent* pGE = (m_vNavigNodePath[m_vPatrollers[iIndex]->uiCurrentNodeIndex])->GetPatrolEvent();

	if (pGE) 
	{
		#ifndef _RELEASE
			ntPrintf("Firing Event: %s\n",ntStr::GetString(pGE->GetName()));
		#else
			ntPrintf("Firing Node Event\n");
		#endif
		pGE->FireEvent(pEnt); // Fire the Event
	}

	if (!m_bOpenGraph)
	{
		// Closed Path. Easy.
		if ( ++(m_vPatrollers[iIndex]->uiCurrentNodeIndex) == m_uiGraphSize ) m_vPatrollers[iIndex]->uiCurrentNodeIndex = 0;
		return;
	}
	else
	{
		// Open Path
		if (!m_vPatrollers[iIndex]->bGoingBackwards)
		{
			// Going from Begining to End
			if ( ++(m_vPatrollers[iIndex]->uiCurrentNodeIndex) == m_uiGraphSize ) 
			{ 
				m_vPatrollers[iIndex]->uiCurrentNodeIndex -= 2;
				m_vPatrollers[iIndex]->bGoingBackwards = true;
			}
			return;
		}
		else
		{
			// Going from End to Begining
			if ( --(m_vPatrollers[iIndex]->uiCurrentNodeIndex) == 0 ) { m_vPatrollers[iIndex]->bGoingBackwards = false; }
			return;
		}
	}
}

//! -------------------------------------------
//! MakePath (Under construction!)
//! -------------------------------------------
void CAIPatrolGraph::MakePath ( CEntity* pEntAI, CEntity* pEntReporter, CAINavigPath* pPath )
{
	int iIndexAI = -1;
	int iIndexReporter = -1;
	bool bAIHasBiggerIndex = false;

	if ( (iIndexAI = GetPatrollerIndex(pEntAI)) < 0 || (iIndexReporter = GetPatrollerIndex(pEntReporter)) < 0) return;

	unsigned int uiAICNI = m_vPatrollers[iIndexAI]->uiCurrentNodeIndex;
	unsigned int uiReporterCNI = m_vPatrollers[iIndexReporter]->uiCurrentNodeIndex;

	bAIHasBiggerIndex = ( uiAICNI > uiReporterCNI );

	pPath->clear();

	//if (m_bOpenGraph)
	{
		unsigned int i = uiAICNI;
		while ( i!=uiReporterCNI )
		{
			CAINavigNode* pNN = (m_vNavigNodePath[i]);
			pPath->push_back(pNN);
			i = (bAIHasBiggerIndex) ? i-1 : i+1;
			if ( i == m_uiGraphSize ) 
				i = 0;
		}
		pPath->push_back(m_vNavigNodePath[i]);
	}
	//else
	//{
	//	// Check in both directions
	//	CAINavigPath PathDec;

	//	float fDist		= 0;
	//	float fDistMin	= FLT_MAX - 2;
	//	unsigned int i	= uiAICNI;
	//	CAINavigNode* pPrevNN = m_vNavigNodePath[i];
	//	pPath->push_back(pPrevNN);

	//	// Increasing Index
	//	while ( i!=uiReporterCNI )
	//	{
	//		i++;
	//		if ( i == m_uiGraphSize ) 
	//			i = 0;
	//		CAINavigNode* pNN = (m_vNavigNodePath[i]);
	//		pPath->push_back(pNN);
	//		fDist += pNN->GetDistanceToSourceNode(pPrevNN);
	//		pPrevNN = m_vNavigNodePath[i];
	//	}
	//	fDistMin = fDist;
	//	
	//	// Decreasing Index

	//	i	= uiAICNI;
	//	fDist = 0.0f;
	//	PathDec.clear();
	//	PathDec.push_back(pPrevNN);

	//	while ( i!=uiReporterCNI )
	//	{
	//		i--;
	//		if ( i == m_uiGraphSize ) 
	//			i = 0;
	//		CAINavigNode* pNN = (m_vNavigNodePath[i]);
	//		PathDec.push_back(pNN);
	//		fDist += pNN->GetDistanceToSourceNode(pPrevNN);
	//		pPrevNN = m_vNavigNodePath[i];
	//	}

	//	if (fDist < fDistMin)
	//	{
	//		// Copy the shortest path (Decreasing Index)
	//		pPath->clear();
	//		pPath->push_back(pPrevNN);
	//		i	= uiAICNI;
	//		
	//		while ( i!=uiReporterCNI )
	//		{
	//			i--;
	//			if ( i == m_uiGraphSize ) 
	//				i = 0;
	//			CAINavigNode* pNN = (m_vNavigNodePath[i]);
	//			pPath->push_back(pNN);
	//		}
	//	}	
	//}
}

//! ------------------------------------------------------------------------------------------
//! DebugRender
//! ------------------------------------------------------------------------------------------
void CAIPatrolGraph::DebugRender (void)
{
#ifndef _GOLD_MASTER
	CQuat	obOrientation( CONSTRUCT_IDENTITY );
	static const float fRadius = 0.2f;
	static float fEventRadius = 0.0f;
	const static CPoint DBG_Y_OFFSET = CPoint(0,0.1f,0);

	// Go through each node

	if ( m_uiGraphSize > 1 )
	{
		CPoint pt3DFrom = m_vNavigNodePath[0]->GetPos();
		CPoint ptFirst = pt3DFrom;

		for ( unsigned int i = 0; i < m_uiGraphSize; ++i )
		{
			CPoint pt3D = m_vNavigNodePath[i]->GetPos();
			g_VisualDebug->RenderSphere( obOrientation, pt3D,	fRadius, 0xff0ffff0 );
			g_VisualDebug->Printf3D( pt3D, DC_YELLOW, 0, "Patrol Node (%d): %s",i, ntStr::GetString(m_vNavigNodePath[0]->GetName()) );

			// Draw arrows
			CPoint pt3DTo	= m_vNavigNodePath[i]->GetPos();
			g_VisualDebug->RenderLine(pt3DFrom , pt3DTo , DC_YELLOW);
			pt3DFrom = pt3DTo;

			// Draw Associated events
			GameEvent* pGE = m_vNavigNodePath[i]->GetPatrolEvent();
			if ( pGE )
			{
				fEventRadius +=0.1f;
				if ( fEventRadius>1.5f ) fEventRadius = 0.0f;

				CMatrix ArcMatrix(CONSTRUCT_IDENTITY);
				ArcMatrix.SetTranslation(pt3D+DBG_Y_OFFSET);
				g_VisualDebug->RenderArc(ArcMatrix, fEventRadius , TWO_PI,  DC_PURPLE);
				#ifndef _RELEASE
					g_VisualDebug->Printf3D(pt3D+10*DBG_Y_OFFSET,DC_WHITE,0,"Event : %s", ntStr::GetString(pGE->GetName()));
				#else
					g_VisualDebug->Printf3D(pt3D+10*DBG_Y_OFFSET,DC_WHITE,0,"Node-Event");
				#endif
			}
		}

		if (!m_bOpenGraph) g_VisualDebug->RenderLine(pt3DFrom , ptFirst , DC_YELLOW);
	}
#endif
}

