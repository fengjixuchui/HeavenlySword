//! -------------------------------------------
//! AINavigGraph.cpp
//!
//! New AIs Navigation Grpah
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#include "ainaviggraph.h"
#include "ainavigsystemman.h"
#include "ai/aiuseobjectqueueman.h"
#include "objectdatabase/dataobject.h"
#include "game/messagehandler.h" // For DebugRender
#include "core/visualdebugger.h" // For DebugRender
#include "game/entitymanager.h" // For DebugRender
#include <stdlib.h>

START_CHUNKED_INTERFACE(CAINavigNodesSets, Mem::MC_AI)
	PUBLISH_PTR_CONTAINER_AS(m_listIntermediateNodes	, Nodes)
	PUBLISH_PTR_AS			(m_pobNavigGraph			, NavigRegion)
END_STD_INTERFACE

START_CHUNKED_INTERFACE(CAINavigGraph, Mem::MC_AI)
	PUBLISH_PTR_CONTAINER_AS	(m_listNodes				,			Nodes)
//	PUBLISH_PTR_CONTAINER_AS	(m_listDoorArrows			,			Doors)
//	PUBLISH_PTR_CONTAINER_AS	(m_listLadderArrows			,			Ladders)
//	PUBLISH_PTR_CONTAINER_AS	(m_listNavigGraphAreaLinks	,			NavGraphRegionLinks)
	PUBLISH_PTR_CONTAINER_AS	(m_listNodesSets			,			NodesSet)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_bIsPatrolGraph			, false,	IsPatrolGraph)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_bIsOpenPatrolGraph		, false,	IsOpenPGraph)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_bIsQueueGraph			, false,	IsQueueGraph)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_bIsLocatorGraph			, false,	IsLocatorGraph)
	PUBLISH_PTR_AS				(m_pPatrolGraphLocator		,			Locator)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_iRegionIndex				, -1,		Index)
	PUBLISH_PTR_AS				(m_pObjectToUse				,			UseObject)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_iMappedAreaInfo			,	0,		SectorBits )

	DECLARE_POSTCONSTRUCT_CALLBACK(PostConstruct)
	DECLARE_POSTPOSTCONSTRUCT_CALLBACK(PostPostConstruct)
END_STD_INTERFACE

//! ------------------------------------------------------------------------------------------
//! CAINavigNode::CAINavigNode()
//!
//! Constructor
//! ------------------------------------------------------------------------------------------

CAINavigGraph::CAINavigGraph() : m_ksName("EMPTY"), m_iRegionIndex(-1), m_bIsPatrolGraph(false), m_bIsOpenPatrolGraph(false), 
								 m_pObjectToUse(NULL), m_bIsQueueGraph(false), m_bIsLocatorGraph(false), m_bLUAEnabled(true), m_iMappedAreaInfo(0)
{
	;
}

//! ------------------------------------------------------------------------------------------
//! CAINavigGraph::PostPostConstruct()
//! ------------------------------------------------------------------------------------------
void CAINavigGraph::PostPostConstruct (void)
{
	if (m_bIsQueueGraph)
	{
		CAIQueueManager::Get().RegisterQueueGraph(m_pObjectToUse, &m_listNodes);
	}
}
//! ------------------------------------------------------------------------------------------
//! CAINavigGraph::PostConstruct()
//! ------------------------------------------------------------------------------------------
void CAINavigGraph::PostConstruct (void)
{
	// Get the Region Name and Add the Navigation Graph to the CAINavigGraphMan list
	DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer(this);
	if(pDO)
	{
		m_ksName = CHashedString((pDO->GetName()).GetString());
	}
	CAINavigationSystemMan::Get().GetNavigGraphManager()->Add( this );

	// Generate the list of Cover Points
	m_listCoverPoints.clear();
	
	AINavigNodeList::const_iterator obIt	= m_listNodes.begin();
	AINavigNodeList::const_iterator obEndIt = m_listNodes.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		CAINavigNode* pNode = (*obIt);
		const AINavigArrowList* pArrow = pNode->GetCoverPointArrows();

		AINavigArrowList::const_iterator obArrowIt		= pArrow->begin();
		AINavigArrowList::const_iterator obArrowEndIt	= pArrow->end();
		for( ; obArrowIt != obArrowEndIt; ++obArrowIt )
		{
			CAINavigArrow* pArrow = *obArrowIt;
			CAINavigCoverPoint* pCoverPoint = pArrow->GetCoverPoint();
			if (pCoverPoint) 
				m_listCoverPoints.push_back(pCoverPoint);
		}
	}

	// Check SectorBits

	if (HasInvalidSectorBits())
	{
		ntPrintf("(ERROR!) Loading CAINavigGraph: %s with BAD SectoBits (0)\n",ntStr::GetString(m_ksName) );
//		user_warn_msg( ("(ERROR!) Loading CAINavigGraph: %s with BAD SectoBits (0)\n",ntStr::GetString(m_ksName) ) );
	}
}

//! ------------------------------------------------------------------------------------------
//! void CAINavigNode::GetNode( CEntity* pobEntity ) 
//!
//! Returns the node that matches the name: pobEntity, or NULL if not found.
//! ------------------------------------------------------------------------------------------
CAINavigNode* CAINavigGraph::GetNode( CEntity* pobEntity )  
{
	if (!pobEntity)
	{
		ntPrintf("CAINavigGraph::GetNode -> NULL parameter received\n");
		return NULL;
	}

	// Go through the list of nodes and get the one with the pobEntity

	AINavigNodeList::const_iterator obIt	= m_listNodes.begin();
	AINavigNodeList::const_iterator obEndIt = m_listNodes.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		if ( (*obIt)->GetEntity() == pobEntity )
		{
			return (*obIt);
		}
	}
	// Not found
	return NULL;
}

//! ------------------------------------------------------------------------------------------
//! void CAINavigNode::GetDoor( CEntity* pobEntity ) 
//!
//! Returns the door (arrow) that matches pobEntity, or NULL if not found.
//! ------------------------------------------------------------------------------------------
CAINavigArrow* CAINavigGraph::GetDoor( CEntity* pobEntity )  
{
	if (!pobEntity)
	{
		ntPrintf("CAINavigGraph::GetDoor -> NULL parameter received\n");
		return NULL;
	}

	// Go through the list of doors and get the one with the pobEntity

	AINavigArrowList::const_iterator obIt		= m_listDoorArrows.begin();
	AINavigArrowList::const_iterator obEndIt	= m_listDoorArrows.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		if ( (*obIt)->GetEntity() == pobEntity )
		{
			return (*obIt);
		}
	}
	// Not found
	return NULL;
}

//! ------------------------------------------------------------------------------------------
//! void CAINavigNode::GetLadder( CEntity* pobEntity ) 
//!
//! Returns the door (arrow) that matches pobEntity, or NULL if not found.
//! ------------------------------------------------------------------------------------------
CAINavigArrow* CAINavigGraph::GetLadder( CEntity* pobEntity )  
{
	if (!pobEntity)
	{
		ntPrintf("CAINavigGraph::GetLadder -> NULL parameter received\n");
		return NULL;
	}

	// Go through the list of doors and get the one with the pobEntity

	AINavigArrowList::const_iterator obIt		= m_listLadderArrows.begin();
	AINavigArrowList::const_iterator obEndIt	= m_listLadderArrows.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		if ( (*obIt)->GetEntity() == pobEntity )
		{
			return (*obIt);
		}
	}
	// Not found
	return NULL;
}

//! ------------------------------------------------------------------------------------------
//! void CAINavigNode::GetClosestNode( const CPoint &pobPoint, float* fDist )
//!
//! Returns the closest node to the given coordinates.
//! ------------------------------------------------------------------------------------------
CAINavigNode* CAINavigGraph::GetClosestNode( const CPoint &obPoint, float* fDist )  
{
	*fDist = FLT_MAX;
	
	float fDistanceMnh = FLT_MAX;
	CAINavigNode* pobClosestNode = NULL;

	AINavigNodeList::const_iterator obIt	= m_listNodes.begin();
	AINavigNodeList::const_iterator obEndIt	= m_listNodes.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		// Check if there is a wall in between
		if (!CAINavigationSystemMan::Get().HasLineOfSight( obPoint+CPoint(0,0.5f,0), (*obIt)->GetPos()+CPoint(0,0.5f,0)) ) // !!! - Dario !!!! [Param] 0.5f ... 
		{
			continue;
		}
		// !!! - Using Manhattan distance
		fDistanceMnh = (*obIt)->GetDistanceToPoint( obPoint, DT_MANHATTAN);

		if ( fDistanceMnh < *fDist )
		{
			*fDist = fDistanceMnh;
			pobClosestNode = (*obIt);
		}
	}

	return pobClosestNode;
}


//! ------------------------------------------------------------------------------------------
//! AINAVIGNODE_TYPE CAINavigGraph::GetNodeType ( CEntity* pobEntity) const 
//!
//! 
//! ------------------------------------------------------------------------------------------
AINAVIGNODE_TYPE CAINavigGraph::GetNodeType ( CEntity* pobEntity) 
{
	CAINavigNode* pobNode = GetNode(pobEntity);
	if (!pobNode) {	return NDT_INVALID; }
	return (pobNode->GetType());	
}


//! ------------------------------------------------------------------------------------------
//! bool CAINavigGraph::IsDoorOpen ( CEntity* ) const
//!
//! 
//! ------------------------------------------------------------------------------------------
bool CAINavigGraph::IsDoorOpen( CEntity* pobEntity)
{
	CAINavigArrow* pobDoor = this->GetDoor(pobEntity);
	if (!pobDoor) { return false; }

	AINAVIGARROW_TYPE eNT = pobDoor->GetType();

	return ( ( eNT == NAT_DOOR ) && ( pobDoor->IsEnabled() ) );
}

//! ------------------------------------------------------------------------------------------
//! bool CAINavigGraph::IsLadderAvailable ( CEntity* ) const
//!
//! 
//! ------------------------------------------------------------------------------------------
bool CAINavigGraph::IsLadderAvailable( CEntity* pobEntity)
{
	CAINavigArrow* pobLadder = this->GetLadder(pobEntity);
	if (!pobLadder) { return false; }

	AINAVIGARROW_TYPE eNT = pobLadder->GetType();

	return ( ( eNT == NAT_LADDER ) && ( pobLadder->IsEnabled() ) &&
			 ( !pobLadder->IsBusy() ) 
		   );
}


//! ------------------------------------------------------------------------------------------
//! bool CAINavigGraph::SetEnableDoor ( CEntity* pobEntity, bool b) ) 
//!
//! Enables/Disables a node (DOOR or LADDER)
//! ------------------------------------------------------------------------------------------
bool CAINavigGraph::SetEnableDoor ( CEntity* pobEntity, bool b) 
{
	CAINavigArrow* pobDoor = this->GetDoor(pobEntity);
	if (!pobDoor) { return false; }
	pobDoor->SetEnable(b);
	return (true);
}

//! ------------------------------------------------------------------------------------------
//! bool CAINavigGraph::SetEnableLadder ( CEntity* pobEntity, bool b) ) 
//!
//! Enables/Disables a node (DOOR or LADDER)
//! ------------------------------------------------------------------------------------------
bool CAINavigGraph::SetEnableLadder ( CEntity* pobEntity, bool b) 
{
	CAINavigArrow* pobLadder = this->GetLadder(pobEntity);
	if (!pobLadder) { return false; }
	pobLadder->SetEnable(b);
	return (true);
}

//! ------------------------------------------------------------------------------------------
//! bool CAINavigGraph::DeleteNode( CEntity* pobEntity) 
//!
//! Deletes a node (useful if a ladder is broken or a door is permanently locked
//! ------------------------------------------------------------------------------------------
bool CAINavigGraph::DeleteNode( CEntity* pobEntity) 
{
	CAINavigNode* pobNode = this->GetNode(pobEntity);
	if (!pobNode) { return false; }

	pobNode->DetachSourceLinks();			// Break all the source links
	this->m_listNodes.remove(pobNode);		// Remove link from main node-list
	return (true);
}

//! ------------------------------------------------------------------------------------------
//! const AINavigArrowList* CAINavigGraph::GetTgtArrows( CAINavigNode* pobNode) const 
//!
//! 
//! ------------------------------------------------------------------------------------------
const AINavigArrowList* CAINavigGraph::GetTgtArrows( CAINavigNode* pobNode) const 
{ 
	if(!pobNode) { return NULL; }

	return (pobNode->GetTgtArrows());
}

//! ------------------------------------------------------------------------------------------
//! GetNodeWithName
//! ------------------------------------------------------------------------------------------
CAINavigNode* CAINavigGraph::GetNodeWithName(const CHashedString & obNodeKName)
{
	if (m_listNodes.empty()) return NULL;

	AINavigNodeList::const_iterator obIt	= m_listNodes.begin();
	AINavigNodeList::const_iterator obEndIt	= m_listNodes.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		if ( (*obIt)->HasName(obNodeKName) )
		{
			return (*obIt);
		}
	}
	return NULL;
}

//! ------------------------------------------------------------------------------------------
//! GetANodeLinkedToCoverPoint
//! ------------------------------------------------------------------------------------------
CAINavigNode* CAINavigGraph::GetANodeLinkedToCoverPoint	( CAINavigCoverPoint* pCP ) const
{
	if ( !pCP )	return NULL;

	AINavigNodeList::const_iterator obIt	= m_listNodes.begin();
	AINavigNodeList::const_iterator obEndIt	= m_listNodes.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		CAINavigNode* pNode = (*obIt);
		if ( pNode->IsLinkedToCoverPoint(pCP) )
		{
			return pNode;
		}
	}
	return NULL;
}
//! ------------------------------------------------------------------------------------------
//! GetClosestCoverPointInMinMaxRange
//! ------------------------------------------------------------------------------------------
CAINavigCoverPoint* CAINavigGraph::GetClosestCoverPointInMinMaxRange	( AI* pAI, const CEntity* pEnemy )
{
	if ( !pAI || !pEnemy)
		return NULL;

	float fDist = 0.0f;
	float fDistSQR = 0.0f;
	float fMinDistSQR = FLT_MAX-2;
	CAINavigCoverPoint* pClosestAvailableCP = NULL;
	
	CPoint obAIPos = pAI->GetPosition();
	CAINavigNode* pNodeAI = GetClosestNode(obAIPos, &fDist);
	if (!pNodeAI) return NULL;

	float fMinRange = pAI->GetAIComponent()->GetCAIMovement()->GetRangeMinDistSQR();
	float fMaxRange = pAI->GetAIComponent()->GetCAIMovement()->GetRangeMaxDistSQR();


	fDist = 0.0f;
	AINavigCoverPointList::const_iterator obIt		= m_listCoverPoints.begin();
	AINavigCoverPointList::const_iterator obEndIt	= m_listCoverPoints.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		CAINavigCoverPoint* pCP = *obIt;
		CDirection Line2Enemy( pCP->GetPos() - pEnemy->GetPosition() );
		float fDist2EnemySQR = Line2Enemy.LengthSquared();

		if ( fDist2EnemySQR > fMinRange && fDist2EnemySQR < fMaxRange )
		{
			// This is a Cover Point that is located within the MIN-MAX region
			
			if ( !pCP->IsDestroyed() && pCP->IsAvailabe() && 
				(pCP->GetClaimer() == NULL || pCP->GetClaimer() == pAI) &&
				 pCP->IsCoverInEnemysDirection(pEnemy->GetPosition()) &&
				!CAINavigationSystemMan::Get().HasLineOfSight(pCP->GetPos(), pEnemy->GetPosition()))
			{
				// The CP is available
				// Find the closest one to the AI
				CDirection Line2AI( pCP->GetPos() - obAIPos );
				fDistSQR = Line2AI.LengthSquared();

				if (fMinDistSQR > fDistSQR)
				{
					// New Min Dist
					fMinDistSQR = fDistSQR;
					pClosestAvailableCP = pCP;
				}
			}
		}
		else
		{
			continue;
		}
	}
	return pClosestAvailableCP;	
}

//! ------------------------------------------------------------------------------------------
//! void CAINavigNode::GetFirstVisibleNode( const CPoint &pobPoint )
//!
//! Returns the first node visible from the given point.
//! ------------------------------------------------------------------------------------------
CAINavigNode* CAINavigGraph::GetFirstVisibleNode( const CPoint &obPoint )  
{
	CAINavigNode* pobVisibleNode = NULL;

	AINavigNodeList::const_iterator obIt	= m_listNodes.begin();
	AINavigNodeList::const_iterator obEndIt	= m_listNodes.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		// Check if it is visible
		if (CAINavigationSystemMan::Get().HasLineOfSight( obPoint+CPoint(0,0.5f,0), (*obIt)->GetPos()+CPoint(0,0.5f,0)) ) // !!! - Dario !!!! [Param] 0.5f ... 
		{
			pobVisibleNode = *obIt;
			return pobVisibleNode;
		}
	}
	return NULL;
}

//! ------------------------------------------------------------------------------------------
//! void CAINavigNode::GetAbsoluteClosestNode( const CPoint &pobPoint )
//!
//! Returns the closest node to the given coordinates regardless of LOS.
//! ------------------------------------------------------------------------------------------
CAINavigNode* CAINavigGraph::GetAbsoluteClosestNode( const CPoint &obPoint, float* fMinDist )  
{
	*fMinDist = FLT_MAX - 2.0f;
	
	float fDistanceMnh = FLT_MAX;
	CAINavigNode* pobClosestNode = NULL;

	AINavigNodeList::const_iterator obIt	= m_listNodes.begin();
	AINavigNodeList::const_iterator obEndIt	= m_listNodes.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		// !!! - Using Manhattan distance
		fDistanceMnh = (*obIt)->GetDistanceToPoint( obPoint, DT_MANHATTAN);

		if ( fDistanceMnh < *fMinDist )
		{
			*fMinDist = fDistanceMnh;
			pobClosestNode = (*obIt);
		}
	}

	return pobClosestNode;
}

//! ------------------------------------------------------------------------------------------
//! GetNodesSet
//! ------------------------------------------------------------------------------------------
//AINavigNodeList* CAINavigGraph::GetIntermediateNodeList ( CHashedString hsName )
//{
//	AIAINavigNodesSetsList::const_iterator obIt		= m_listNodesSets.begin();
//	AIAINavigNodesSetsList::const_iterator obEndIt	= m_listNodesSets.end();
//	for( ; obIt != obEndIt; ++obIt )
//	{
//		CAINavigNodesSets* pNNS = *obIt;
//		if (pNNS && pNNS->GetName() == hsName)
//		{
//			return (pNNS->GetIntermediateNodesList());
//		}
//	}
//	return NULL;
//}

//! ------------------------------------------------------------------------------------------
//! DebugRenderQueueGraph
//! ------------------------------------------------------------------------------------------
void CAINavigGraph::DebugRenderQueueGraph (void)
{
#ifndef _GOLD_MASTER
	CQuat	obOrientation( CONSTRUCT_IDENTITY );
	const static float fRadius = 0.2f;
//	static float fEventRadius = 0.0f;
	static float fUseObjectRadius = 0.0f;
	const static CPoint DBG_Y_OFFSET = CPoint(0,0.1f,0);
	unsigned int uiColor = 0xff0ffff0;

	// Go through each node
	AINavigNodeList::const_iterator obIt	= m_listNodes.begin();
	AINavigNodeList::const_iterator obEndIt	= m_listNodes.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		CAINavigNode* pNode = (*obIt);
		// Draw the Node
		CPoint pt3D = pNode->GetPos();
			// Location
		g_VisualDebug->RenderSphere( obOrientation, pt3D,	fRadius, uiColor  );
			// Radius
		g_VisualDebug->RenderSphere( obOrientation, pt3D,	pNode->GetRadius(), uiColor, 4096 );
			// Name
		g_VisualDebug->Printf3D( pt3D, 0xffffffff, 0, "QNode: %s",ntStr::GetString(pNode->GetName()));
			// Queuing AI
		CAIQueueOjectFolder* pQF = CAIQueueManager::Get().GetFolder(m_pObjectToUse);
		if (pQF)
		{
			AI* pAI = pQF->GetAIQueueingInNode(pNode);
			g_VisualDebug->RenderLine( pt3D , pt3D+CPoint(0,2.0f,0) , pAI ? DC_RED : DC_GREEN);
			g_VisualDebug->RenderPoint( pt3D+CPoint(0,1.0f,0) , 5, DC_BLUE);
			g_VisualDebug->Printf3D( pt3D+CPoint(0.1f,2.0f,0), pAI ? DC_RED : DC_GREEN , 0, "AI: %s [%d]", 
									pAI ? ntStr::GetString(pAI->GetName()) : "EMPTY",
									pAI ? pAI->GetAIComponent()->GetCAIMovement()->GetQueueIndex() : -99
									);
		}
		else
		{
			g_VisualDebug->RenderLine( pt3D , pt3D+CPoint(0,1.0f,0) , DC_BLACK);
			g_VisualDebug->RenderPoint( pt3D+CPoint(0,1.0f,0) , 5, DC_BLUE);
			g_VisualDebug->Printf3D( pt3D+CPoint(0.1f,1.0f,0), DC_RED, 0, "DOES NOT HAVE A FOLDER!!!");
		}

		// Draw arrows
		AINavigArrowList::const_iterator obItArrow		= pNode->GetTgtArrows()->begin();
		AINavigArrowList::const_iterator obEndItArrow	= pNode->GetTgtArrows()->end();

		if (pNode->GetTgtArrows()->empty())
		{
			if (!m_pObjectToUse)
			{
				g_VisualDebug->Printf3D( pt3D, DC_RED, 0, "THIS QUEUEGRAPH DOES NOT HAVE AN ASSIGNED OBJECT TO USED" );
			}
			else
			{
				g_VisualDebug->RenderLine( pt3D , m_pObjectToUse->GetPosition() , DC_BLUE);
			}
			continue;
		}

		for( ; obItArrow != obEndItArrow; ++obItArrow )
		{
			CPoint pt3DFrom = (*obItArrow)->GetSrcNode()->GetPos();
			CPoint pt3DTo = (*obItArrow)->GetTgtNode()->GetPos();
			g_VisualDebug->RenderLine( pt3DFrom , pt3DTo ,DC_YELLOW);
		}
	}

	// HighLight Object to use
	if (m_pObjectToUse)
	{
		fUseObjectRadius +=0.1f;
		if ( fUseObjectRadius>1.5f ) fUseObjectRadius = 0.0f;
		CMatrix ArcMatrix(CONSTRUCT_IDENTITY);
		ArcMatrix.SetTranslation(m_pObjectToUse->GetPosition()+DBG_Y_OFFSET);
		g_VisualDebug->RenderArc(ArcMatrix, fUseObjectRadius , TWO_PI,  DC_PURPLE);
	}
#endif
}

//! ------------------------------------------------------------------------------------------
//! DebugRender
//! ------------------------------------------------------------------------------------------
void CAINavigGraph::DebugRender (void)
{
#ifndef _GOLD_MASTER
	CQuat	obOrientation( CONSTRUCT_IDENTITY );
	const static float fRadius = 0.2f;
	static float fEventRadius = 0.0f;
	static float fUseObjectRadius = 0.0f;
	const static CPoint DBG_Y_OFFSET = CPoint(0,0.1f,0);
	unsigned int uiColor = DC_WHITE;

	if (!this->IsInActiveSector())
		return;

	if (this->IsQueueGraph())
	{
		DebugRenderQueueGraph();
		return;
	}

	// Go through each node
	AINavigNodeList::const_iterator obIt	= m_listNodes.begin();
	AINavigNodeList::const_iterator obEndIt	= m_listNodes.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		CAINavigNode* pNode = (*obIt);
		// Draw the Node
		CPoint pt3D = pNode->GetPos();
		GameEvent* pGE = pNode->GetPatrolEvent();
			// Location
		g_VisualDebug->RenderSphere( obOrientation, pt3D,	fRadius, uiColor  );
			// Radius
		CMatrix ArcMatrixRadius(CONSTRUCT_IDENTITY);
		ArcMatrixRadius.SetTranslation(pt3D+DBG_Y_OFFSET);
		g_VisualDebug->RenderArc(ArcMatrixRadius, sqrt(pNode->GetRadiusSQR()) , TWO_PI,  uiColor);
		//g_VisualDebug->RenderSphere( obOrientation, pt3D,	sqrt(pNode->GetRadiusSQR()), uiColor, 4096 );

			// Wall disable Radius
		if (pNode->HasDisableWallDetFlag())
			g_VisualDebug->RenderSphere( obOrientation, pt3D,	sqrt(pNode->GetNoWallDetectRadiusSQR()), DC_YELLOW, 4096 );
			// Name
		#ifndef PLATFORM_PS3
			g_VisualDebug->Printf3D( pt3D, 0xffffffff, 0, "%s",ntStr::GetString(pNode->GetName()) );
		#endif

		// Draw arrows
		//AINavigArrowList*	plistTgtArrows = (*obIt)->GetTgtArrows();
		AINavigArrowList::const_iterator obItArrow		= pNode->GetTgtArrows()->begin();
		AINavigArrowList::const_iterator obEndItArrow	= pNode->GetTgtArrows()->end();
		for( ; obItArrow != obEndItArrow; ++obItArrow )
		{
			CAINavigArrow* pArrow = (*obItArrow);
			CAINavigNode* pSrcNode = pArrow->GetSrcNode();
			CAINavigNode* pTgtNode = pArrow->GetTgtNode();
			CPoint pt3DFrom = pSrcNode->GetPos();
			CPoint pt3DTo = pTgtNode->GetPos();
			CEntity* pUseEntity = pArrow->GetEntityToUse();
			g_VisualDebug->RenderLine( pt3DFrom , pt3DTo , DC_WHITE );

			CDirection dirArrow(pt3DTo-pt3DFrom);
			CDirection dirArrowPerpend = CAINavigationSystemMan::Get().GetPerpendicular(dirArrow);
			dirArrowPerpend.Normalise();
			CPoint ptLeftSrc = CPoint(pt3DFrom+ DBG_Y_OFFSET + dirArrowPerpend*pSrcNode->GetRadius());
			CPoint ptRightSrc = CPoint(pt3DFrom+ DBG_Y_OFFSET- dirArrowPerpend*pSrcNode->GetRadius());
			CPoint ptLeftTgt = CPoint(pt3DTo+ DBG_Y_OFFSET + dirArrowPerpend*pTgtNode->GetRadius());
			CPoint ptRightTgt = CPoint(pt3DTo+ DBG_Y_OFFSET- dirArrowPerpend*pTgtNode->GetRadius());
			g_VisualDebug->RenderLine( ptLeftSrc , ptLeftTgt , DC_YELLOW );
			g_VisualDebug->RenderLine( ptRightSrc , ptRightTgt , DC_YELLOW );


			if (pUseEntity)
			{
				CDirection dir(pt3DTo-pt3DFrom);
				float fDist = dir.Length()*0.3f;
				dir.Normalise();
				CPoint pt3DMiddle = pt3DFrom + CPoint(fDist*dir);
				g_VisualDebug->RenderLine( pt3DFrom , pt3DMiddle , DC_RED );
				g_VisualDebug->RenderLine( pt3DMiddle , pt3DMiddle+CPoint(0,-0.3f,0) , DC_RED );
				g_VisualDebug->RenderLine( pt3DMiddle , pt3DMiddle+CPoint(0,0.3f,0) , DC_RED );
				g_VisualDebug->RenderPoint( pt3DMiddle , 10 , DC_BLUE );
				g_VisualDebug->Printf3D( pt3DMiddle, DC_BLUE, 0, "Use:%s",ntStr::GetString(pUseEntity->GetName()) );

			}
			
		}

		// Draw Associated events

		if ( pGE )
		{
			fEventRadius +=0.1f;
			if ( fEventRadius>1.5f ) fEventRadius = 0.0f;

			CMatrix ArcMatrix(CONSTRUCT_IDENTITY);
			ArcMatrix.SetTranslation(pt3D+DBG_Y_OFFSET);
			g_VisualDebug->RenderArc(ArcMatrix, fEventRadius , TWO_PI,  DC_PURPLE);
			#ifndef _RELEASE
				g_VisualDebug->Printf3D(pt3D,DC_WHITE,0,"Event : %s", ntStr::GetString(pGE->GetName()));
			#else
				g_VisualDebug->Printf3D(pt3D,DC_WHITE,0,"Node-Event");
			#endif

		}

		// Draw Use Object Info
		//CEntity* pObjectToUse = pNode->GetEntityToUse();
		unsigned int uiUseObjColor = DC_YELLOW;

		CEntity* pObjectToUseInNode = pNode->GetEntityToUse();

		bool bNodeHasArrowsWithUseObject = false;
		ntstd::List<CEntity*> useObjectList;
		pNode->GetUseEntities(&useObjectList);
		
		if (!useObjectList.empty())
		{
			bNodeHasArrowsWithUseObject = true;
			if ( pObjectToUseInNode )
			{
				uiUseObjColor = DC_BLACK;
				g_VisualDebug->Printf3D(pt3D,DC_RED,0,"%s - Use Object Error (%s)",
									ntStr::GetString(pNode->GetName()),
									ntStr::GetString(pObjectToUseInNode->GetName()));
			}

			fUseObjectRadius +=0.1f;
			if ( fUseObjectRadius>1.5f ) fUseObjectRadius = 0.0f;
			ntstd::List<CEntity*>::const_iterator obIt		= useObjectList.begin();
			ntstd::List<CEntity*>::const_iterator obEndIt	= useObjectList.end();
			for( ; obIt != obEndIt; ++obIt )
			{
				CEntity* pObjectToUse = (*obIt);
				CMatrix ArcMatrix(CONSTRUCT_IDENTITY);
				ArcMatrix.SetTranslation(pObjectToUse->GetPosition()+DBG_Y_OFFSET);
				g_VisualDebug->RenderArc(ArcMatrix, fUseObjectRadius , TWO_PI,  DC_PURPLE);
				g_VisualDebug->RenderLine( pt3D , pObjectToUse->GetPosition()+DBG_Y_OFFSET , DC_PURPLE);
				g_VisualDebug->RenderSphere( obOrientation, pt3D, fUseObjectRadius, uiUseObjColor, 4096 );
			}
		}
		
		if ( pObjectToUseInNode && !bNodeHasArrowsWithUseObject) 
		{
			CMatrix ArcMatrix(CONSTRUCT_IDENTITY);
			ArcMatrix.SetTranslation(pObjectToUseInNode->GetPosition()+DBG_Y_OFFSET);
			g_VisualDebug->RenderArc(ArcMatrix, fUseObjectRadius , TWO_PI,  DC_PURPLE);
			g_VisualDebug->RenderLine( pt3D , pObjectToUseInNode->GetPosition()+DBG_Y_OFFSET , DC_PURPLE);
			g_VisualDebug->RenderSphere( obOrientation, pt3D, fUseObjectRadius, DC_BLUE, 4096 );
		}

		// Draw Cover Points
		
		const AINavigArrowList* plistCPArrows = pNode->GetCoverPointArrows();
		
		AINavigArrowList::const_iterator obItCPArrow		= plistCPArrows->begin();
		AINavigArrowList::const_iterator obEndItCPArrow		= plistCPArrows->end();
		for( ; obItCPArrow != obEndItCPArrow; ++obItCPArrow )
		{
			CAINavigArrow* pCPArrow = (*obItCPArrow);
			CAINavigCoverPoint* pCoverPoint = pCPArrow->GetCoverPoint();

			if (pCoverPoint)
			{
				unsigned int uiCoverPointRender = 0;
				CEntity* pEnemy = CEntityManager::Get().GetPlayer();

				if (	!pCoverPoint->IsDestroyed() && pCoverPoint->IsAvailabe() &&
						pEnemy && pCoverPoint->IsCoverInEnemysDirection(pEnemy->GetPosition()) &&
						!CAINavigationSystemMan::Get().HasLineOfSight(pCoverPoint->GetPos(), pEnemy->GetPosition()))
				{
					uiCoverPointRender = DPF_WIREFRAME;
				}

				unsigned int uiCoverColor = pCoverPoint->IsDestroyed()	? DC_BLACK :
											pCoverPoint->IsAvailabe()	? DC_GREEN : DC_RED;


				pt3D = pCoverPoint->GetPos();
				// Location
				g_VisualDebug->RenderSphere( obOrientation, pt3D,	fRadius, uiCoverColor, uiCoverPointRender );
				// Radius
				CMatrix ArcMatrixRadius(CONSTRUCT_IDENTITY);
				ArcMatrixRadius.SetTranslation(pt3D+DBG_Y_OFFSET);
				g_VisualDebug->RenderArc(ArcMatrixRadius, sqrt(pCoverPoint->GetRadiusSQR()) , TWO_PI,  uiCoverColor);
				//g_VisualDebug->RenderSphere( obOrientation, pt3D,	sqrt(pCoverPoint->GetRadiusSQR()), uiCoverColor, 4096 );
				// Name
				if (!pCoverPoint->GetClaimer())
					g_VisualDebug->Printf3D( pt3D, 0xffff00ff, 0, "%s - [%.2f]",ntStr::GetString(pCoverPoint->GetName()), pCoverPoint->GetChanceOfUse());
				else
					g_VisualDebug->Printf3D( pt3D, DC_YELLOW, 0, "Booked by : %s",(pCoverPoint->GetClaimer()->GetName()).c_str());
				// Draw arrows
				g_VisualDebug->RenderLine( pNode->GetPos() , pt3D ,uiCoverColor );
				// Valid Direction
				if (pCoverPoint->IsValidDirUsed())
				{
					g_VisualDebug->RenderLine( pt3D , CPoint(pt3D + pCoverPoint->GetValidDir()) , DC_BLUE );
					g_VisualDebug->RenderPoint( CPoint(pt3D + pCoverPoint->GetValidDir()) , 5, DC_BLUE );
				}
				// Is Jump Over?
				if (pCoverPoint->IsJumpOver())
				{
					g_VisualDebug->RenderLine( pt3D , CPoint(pt3D + CPoint(0.0f,1.5f,0.0f)) , DC_RED );
					g_VisualDebug->RenderPoint( CPoint(pt3D + CPoint(0.0f,1.5f,0.0f)) , 5, DC_YELLOW );
				}

			}
		}

	}
#endif
}
