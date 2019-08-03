/***************************************************************************************************
*
*	DESCRIPTION		ainavgraphmanager.cpp - manages the connected graph of navigation nodes that
*					provide the basis for pathfinding
*
*	NOTES
*
***************************************************************************************************/

#include "ai/aiastar.h"
#include "ai/ainavgraphmanager.h"
#include "ai/airepulsion.h"
#include "ai/ainavpath.h"
#include "ai/ainavnodes.h"
#include "ai/ainavedge.h"
#include "ai/aibehaviourmanager.h"
#include "ai/aidebugwall.h"
#include "ai/aisafezone.h"
#include "ai/aistatemachine.h"
#include "ai/ainavdata.h"
#include "ai/aidistance.h"

#include "core/osddisplay.h"
#include "core/visualdebugger.h"

#include "game/aicomponent.h"
#include "game/query.h"
#include "game/entitymanager.h"
#include "game/keywords.h"
#include "game/randmanager.h"

// for positioning of debug targets
#include "camera/camman.h"
#include "camera/camview.h"

#include "physics/world.h"

/***************************************************************************************************
*
*	FUNCTION		
*
*	DESCRIPTION		
*
***************************************************************************************************/

CAINavGraphManager::CAINavGraphManager( void ) : 
	m_bRender( false ),
	m_pobCurrentNode( NULL ),
	m_eCreateState( AINAVNODE_AABB_CREATE_STATE_START ),
	m_pobIntermediateData( NULL ),
	m_pobNavData( NULL ),
	m_obPathfindTestObstacle( 0 ),
	m_bDrawPathfindTestTarget( false ),
	m_uNumNodes( 0 ),
	m_iLastNodeAdded( -1 ),
	m_bFirstFrame( true ),
	m_pobTestPath( NULL ),
	m_safeZone( NULL )
{
	m_pobRepulsion = NT_NEW_CHUNK( Mem::MC_AI ) AIObjectAvoidance;
	ntAssert( m_pobRepulsion );

}

/***************************************************************************************************
*
*	FUNCTION		
*
*	DESCRIPTION		
*
***************************************************************************************************/

CAINavGraphManager::~CAINavGraphManager( void )
{
	// nav nodes are handled by the serialiser, so it frees them

	NT_DELETE_CHUNK( Mem::MC_AI, m_pobRepulsion );
	NT_DELETE_CHUNK( Mem::MC_AI,  m_pobTestPath );

	if (m_obPathfindTestObstacle)
	{
		m_obPathfindTestObstacle->RemoveFromWorld();
		NT_DELETE_CHUNK( Mem::MC_AI, m_obPathfindTestObstacle );
	}
	
	NT_DELETE_CHUNK( Mem::MC_AI, m_pobNavData );
	NT_DELETE_CHUNK( Mem::MC_AI, m_pobIntermediateData );
}

/***************************************************************************************************
*
*	FUNCTION		
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CAINavGraphManager::LevelUnload()
{
	m_bFirstFrame = true;
	m_obDebugWalls.clear();
	m_obNavNodes.clear();

	NT_DELETE_CHUNK( Mem::MC_AI, m_pobNavData );
	NT_DELETE_CHUNK( Mem::MC_AI, m_pobIntermediateData );
	m_pobNavData = NULL;
	m_pobIntermediateData = NULL;
}

/***************************************************************************************************
*
*	FUNCTION		
*
*	DESCRIPTION		
*
***************************************************************************************************/

void
CAINavGraphManager::AddRegion( CAINavNodeRegion* const pobNewNode )
{
	// give the node a unique id
	//pobNewNode->SetID( m_uNumNodes++ );

	// add it to the manager's node list
	m_obNavRegions.push_back( pobNewNode );

	if (!m_pobIntermediateData)
	{
		m_pobIntermediateData = NT_NEW_CHUNK( Mem::MC_AI ) AIIntermediateNavData;
	}
	m_pobIntermediateData->AddRegion( pobNewNode );
}

/***************************************************************************************************
*
*	FUNCTION		
*
*	DESCRIPTION		
*
***************************************************************************************************/

void
CAINavGraphManager::FirstFrameInit( void )
{
	// setup the repulsion manager
	m_pobRepulsion->FirstFrameInit();

	ntPrintf("AI PREPROCESS - START\n");
	if (m_pobIntermediateData)
	{
		m_pobIntermediateData->Preprocess();
		m_pobNavData = NT_NEW_CHUNK( Mem::MC_AI ) AINavData( m_pobIntermediateData );
		NT_DELETE_CHUNK( Mem::MC_AI, m_pobIntermediateData );
		m_pobIntermediateData = NULL;
	}
	ntPrintf("AI PREPROCESS - END\n");

	CHashedString walkName = "AI/Movement/Walk/walk";
	CHashedString followName = "AI/Movement/Follow/follow";
	CHashedString testBedName = "ai_TestBed/ai_TestBed";
	CHashedString groupTestName = "AI/Movement/Group/group";
	CHashedString testWalkwaysName = "AI/Walkways/walkways";
	CHashedString testTriName = "AI/Tritest/Walkways/walkways";
	CHashedString filterName = "AI/Filter/Filter";
	CHashedString basicBehavioursName = "AI/BasicBehaviours/BasicBehaviours";
	
	CHashedString startLevel( g_ShellOptions->m_dbgStartLevel );

	if (startLevel == walkName)
	{
		// setup random walk parameters
	}
	else if (startLevel == followName)
	{
		// setup follow parameters

		// get the first AI
		CEntityQuery			obAI1Query;
		CEQCIsSubStringInName	obNameClause1( "AI1" );
		CEQCIsEnemy				obEnemyClause;

		obAI1Query.AddClause( obNameClause1 );
		obAI1Query.AddClause( obEnemyClause );
		
		CEntityManager::Get().FindEntitiesByType( obAI1Query, CEntity::EntType_AI );
		
		QueryResultsContainerType::iterator obEnd = obAI1Query.GetResults().end();
		AI*	AI1 = NULL;
		for ( QueryResultsContainerType::iterator obIt = obAI1Query.GetResults().begin(); obIt != obEnd; ++obIt )
		{
			AI1 = (AI*)*obIt;
		}

		// replace his default behaviour with random walk

		// get the second AI
		CEntityQuery			obAI2Query;
		CEQCIsSubStringInName	obNameClause2( "AI2" );

		obAI2Query.AddClause( obNameClause2 );
		obAI2Query.AddClause( obEnemyClause );
		
		CEntityManager::Get().FindEntitiesByType( obAI2Query, CEntity::EntType_AI  );
		
		obEnd = obAI2Query.GetResults().end();
		AI*	AI2 = NULL;
		for ( QueryResultsContainerType::iterator obIt = obAI2Query.GetResults().begin(); obIt != obEnd; ++obIt )
		{
			AI2 = (AI*)*obIt;
		}

		ntAssert( AI1 != AI2 );
		ntAssert( AI1->GetAIComponent() );
		ntAssert( AI2->GetAIComponent() );

		// set his target to be the first AI
		AI2->GetAIComponent()->SetDirectTarget( AI1 );

		// replace his default behaviour with follow
		CAIStateMachine* pFollow = AI2->GetAIComponent()->GetBehaviourManager()->CreateBehaviour( CAIBehaviourManager::FOLLOWLEADER_DARIO, AI2->GetAIComponent(), AI2 );
		ntAssert( pFollow );
		AI2->GetAIComponent()->GetBehaviourManager()->Replace( *pFollow );

	}
	else if (startLevel == testBedName || startLevel == testWalkwaysName
			|| startLevel == testTriName || startLevel == basicBehavioursName)
	{
		// get the AI
		CEntityQuery			obAI1Query;
		CEQCIsSubStringInName	obNameClause( "Main_EnemyAI_1" );
		CEQCIsEnemy				obEnemyClause;

		obAI1Query.AddClause( obNameClause );
		obAI1Query.AddClause( obEnemyClause );
		
		CEntityManager::Get().FindEntitiesByType( obAI1Query, CEntity::EntType_AI  );
		
		QueryResultsContainerType::iterator obEnd = obAI1Query.GetResults().end();
		AI*	AI1 = NULL;
		for ( QueryResultsContainerType::iterator obIt = obAI1Query.GetResults().begin(); obIt != obEnd; ++obIt )
		{
			AI1 = (AI*)*obIt;
		}

		ntAssert( AI1->GetAIComponent() );

		// replace his default behaviour with walk to point
		//CAIStateMachine* pWalkToPoint = AI1->GetAIComponent()->GetBehaviourManager()->CreateBehaviour( CAIBehaviourManager::WALKTOPOINT, AI1->GetAIComponent(), AI1 );
		//ntAssert( pWalkToPoint );
		//AI1->GetAIComponent()->GetBehaviourManager()->Replace( *pWalkToPoint );

		// move the camera to the start room
		CPoint ptCamPos( AI1->GetPosition() );
		ptCamPos.X() += 15.0f;
		ptCamPos.Y() += 5.0f;
		CamMan::GetPrimaryView()->SetDebugCameraPlacement( ptCamPos, AI1->GetPosition() );

		// switch to free cam
		CamMan::GetPrimaryView()->SetDebugCameraMode(CamView::DM_FREE);
		CInputHardware::Get().SetPadContext( PAD_CONTEXT_DEBUG );

		// set the input context to AI
		INPUT_CONTEXT eContext = INPUT_CONTEXT_AI;
        CInputHardware::Get().SetContext( eContext );

	}
	else if (startLevel == groupTestName )
	{

		// get the AI
		CEntityQuery			obAI1Query;
		CEQCIsSubStringInName	obNameClause( "Main_EnemyAI_1" );
		CEQCIsEnemy				obEnemyClause;

		obAI1Query.AddClause( obNameClause );
		obAI1Query.AddClause( obEnemyClause );
		
		CEntityManager::Get().FindEntitiesByType( obAI1Query, CEntity::EntType_AI  );
		
		QueryResultsContainerType::iterator obEnd = obAI1Query.GetResults().end();
		AI*	AI1 = NULL;
		for ( QueryResultsContainerType::iterator obIt = obAI1Query.GetResults().begin(); obIt != obEnd; ++obIt )
		{
			AI1 = (AI*)*obIt;
		}

		ntAssert( AI1->GetAIComponent() );


		// move the camera to the start room
		CPoint ptCamPos( AI1->GetPosition() );
		ptCamPos.X() += 15.0f;
		ptCamPos.Y() += 5.0f;
		CamMan::GetPrimaryView()->SetDebugCameraPlacement( ptCamPos, AI1->GetPosition() );

		// switch to free cam
		CamMan::GetPrimaryView()->SetDebugCameraMode(CamView::DM_FREE);
		CInputHardware::Get().SetPadContext( PAD_CONTEXT_DEBUG );

		// set the input context to AI
		INPUT_CONTEXT eContext = INPUT_CONTEXT_AI;
        CInputHardware::Get().SetContext( eContext );
	}
	else if (startLevel == filterName)
	{
		m_pobNavData->AddKeywordToRegion( "filter", 3 );
	}	
}

/***************************************************************************************************
*
*	FUNCTION		
*
*	DESCRIPTION		
*
***************************************************************************************************/
int CAINavGraphManager::GetNumNodes() const
{
	return m_obNavNodes.size();
}

/***************************************************************************************************
*
*	FUNCTION		
*
*	DESCRIPTION		
*
***************************************************************************************************/

void
CAINavGraphManager::Update( float fTimeChange )
{
	// do some first frame initialisation
	if (m_bFirstFrame)
	{
		FirstFrameInit();
		m_bFirstFrame = false;
	}

	m_pobRepulsion->Update( fTimeChange );

	// Old code removed in keyboard usage cleanup
				}



/***************************************************************************************************
*
*	FUNCTION		
*
*	DESCRIPTION		
*
***************************************************************************************************/

void
CAINavGraphManager::DebugRender( void )
{
#ifndef _GOLD_MASTER
	// always render walls
	AIDebugWallList::iterator wallItr = m_obDebugWalls.begin();
	for(;wallItr!=m_obDebugWalls.end();++wallItr)
	{
		// render the wall as a debug AABB
		(*wallItr)->PaulsDebugRender();
	}

	// render the pathfinder test target points
	if (m_bDrawPathfindTestTarget)
	{
		CQuat	obOrientation( CONSTRUCT_IDENTITY );
		float	fRadius = 0.25f;
		uint32_t colour = m_bPathfindTestRun ? 0xffff0f0f : 0xff0fff0f;
		g_VisualDebug->RenderSphere( obOrientation, m_obPathfindTestTarget,	fRadius, colour );		
		fRadius = 0.5f;
		g_VisualDebug->RenderSphere( obOrientation, m_obPathfindTestObstaclePos,	fRadius, 0xffffffff );
	}

	if (!m_bRender)	{ return; }

	if (m_pobNavData)
	{
		m_pobNavData->DebugRender();
	}

	{
		AINavNodeRegionList::iterator itr = m_obNavRegions.begin();
		for(;itr!=m_obNavRegions.end();++itr)
		{
			// render the node as a debug AABB
			(*itr)->PaulsDebugRender();
		}
	}

	return;

	/*
	{
		AINavNodeAABBList::iterator itr = m_obNavNodes.begin();
		for(;itr!=m_obNavNodes.end();++itr)
		{
			// render the node as a debug AABB
			(*itr)->PaulsDebugRender();
		}
	}

	// render the patrol path
	CQuat	obOrientation( CONSTRUCT_IDENTITY );
	float	fRadius = 0.1f;
	CPoint	obCentrePoint( CONSTRUCT_CLEAR );

	ntstd::List<unsigned>::iterator pathItr = m_obPatrolPath.begin();
	for(;pathItr != m_obPatrolPath.end(); ++pathItr)
	{
		CAINavNodeAABB*	pobNode = GetNodeNum( (*pathItr) );
		pobNode->GetCentre( obCentrePoint );
		g_VisualDebug->RenderSphere( obOrientation, obCentrePoint,	fRadius, 0xff0f0fff );
	}
	*/
#endif
}

/***************************************************************************************************
*
*	FUNCTION		
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CAINavGraphManager::HasNavData() const
{
	return m_pobNavData && m_pobNavData->m_iNumTriangles > 0;
}

/***************************************************************************************************
*
*	FUNCTION		
*
*	DESCRIPTION		
*
***************************************************************************************************/
bool CAINavGraphManager::InGraph( const CPoint& obPos, const CKeywords* pobKeys, AINavHint* pHint ) const
{
	return m_pobNavData && m_pobNavData->GetContainingNode( obPos, pHint, pobKeys ) != -1;
}

/***************************************************************************************************
*
*	FUNCTION		CAINavGraphManager::InGraph
*
*	DESCRIPTION		Little experiment for AI where a start and end point are tested for navgraph
*					containment in the same navgraph tri or a adjacent tri
*
***************************************************************************************************/

INGRAPH_RETURN CAINavGraphManager::InGraph( const CPoint& obP1, const CPoint& obP2, CMatrix* pIntersectResult, const CKeywords* pobKeys ) const
{
	// Check that there is a navgraph to test against
	if ( !m_pobNavData )
	{
		return INGRAPH_ERROR;
	}

	int iTri1 = m_pobNavData->GetContainingNode( obP1, 0, pobKeys );
	int iTri2 = m_pobNavData->GetContainingNode( obP2, 0, pobKeys );

	// If the results are the same, then they;re either both in or out.
	if( iTri1 == iTri2 )
	{
		return iTri1 == -1 ? INGRAPH_BOTH_OUT : INGRAPH_SAME_TRI;
	}

	// Both valid but in different tri's
	if( iTri1 != -1 && iTri2 != -1 )
	{
		return INGRAPH_DIFFERNT_TRI;
	}

	// If required fill in the intersection matrix.
	if( pIntersectResult )
	{
		int iIn = iTri1 == -1 ? iTri2 : iTri1;
		//int iOut = iTri1 == -1 ? iTri1 : iTri2;

		if( !m_pobNavData->GetIntersectionEdge( iIn, obP1, obP2, pIntersectResult ) )
			return INGRAPH_ERROR;
	}

	// Return wether the start or end is out of the graph
	return iTri1 == -1 ? INGRAPH_START_OUT : INGRAPH_END_OUT;
}


static CPoint	test1( 1.0f, 0.0f, 0.0f );
static CPoint	test2( 0.0f, 0.0f, 1.0f );
static CPoint	test3( -1.0f, 0.0f, 0.0f );
static CPoint	test4( 0.0f, 0.0f, -1.0f );

/***************************************************************************************************
*
*	FUNCTION		
*
*	DESCRIPTION		
*
***************************************************************************************************/
bool CAINavGraphManager::InGraph( const CPoint& obPos, const float radius, const CKeywords* pobKeys, AINavCache* pCache ) const
{
	if( pCache )
	{
		return (	InGraph( obPos, pobKeys, pCache->m_aobHints + 1 ) && 
					InGraph( obPos + (test1 * radius), pobKeys, pCache->m_aobHints + 1 ) && 
					InGraph( obPos + (test2 * radius), pobKeys, pCache->m_aobHints + 2 ) &&
					InGraph( obPos + (test3 * radius), pobKeys, pCache->m_aobHints + 3 ) && 
					InGraph( obPos + (test4 * radius), pobKeys, pCache->m_aobHints + 4 ) );
	}

	return (	InGraph( obPos, pobKeys ) && 
				InGraph( obPos + (test1 * radius), pobKeys ) && 
				InGraph( obPos + (test2 * radius), pobKeys ) &&
				InGraph( obPos + (test3 * radius), pobKeys ) && 
				InGraph( obPos + (test4 * radius), pobKeys ) );
}


//--------------------------------------------------
//!
//!	CAINavGraphManager::GetRandomPosInNode
//!
//--------------------------------------------------

CKeywords* CAINavGraphManager::GetNodeKeywords( int nodeNum ) const
{
	return m_pobNavData->GetNodeKeywords( nodeNum );
}

//--------------------------------------------------
//!
//!	CAINavGraphManager::IsPosLegal
//!
//--------------------------------------------------

bool CAINavGraphManager::IsPosLegal( const CPoint& pos, CKeywords* keywords ) const
{	
	if( !m_pobNavData ) 
		return false;

	int node = m_pobNavData->GetContainingNode( pos );

	// return false if the point is out of the graph
	if (node == -1)
	{
		return false;
	}

	// return false if the supplied keywords are filtered by the containing node
	CKeywords* nodeKeywords = GetNodeKeywords( node );
	if (keywords && nodeKeywords)
	{
		return !(keywords->operator &( *nodeKeywords ));
	}

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CAINavGraphManager::InSafeZone( const CPoint& obPos, const float radius ) const 
{
	if (m_safeZone)
	{
		return m_safeZone->InZone( obPos, radius );
	}

	return InGraph( obPos );
};



inline CPoint GetPointFromBarycentric( const CPoint& a, const CPoint& b, const CPoint& c, float u, float v )
{
	float w = 1.0f - u - v;

	return CPoint(
		(a.X() * w) + (b.X() * u) + (c.X() * v),
		(a.Y() * w) + (b.Y() * u) + (c.Y() * v),
		(a.Z() * w) + (b.Z() * u) + (c.Z() * v)	);
}

//--------------------------------------------------
//!
//!	CAINavGraphManager::GetRandomPosInNode
//!
//--------------------------------------------------

CPoint CAINavGraphManager::GetRandomPosInNode( int nodeNum ) const
{
	// generate random barycentric coordinates
	float u = grandf( 1.0f );
	float v = grandf( 1.0f - u );

	// convert them to world coordinates
	return CPoint(	GetPointFromBarycentric(
						m_pobNavData->m_aVertices[m_pobNavData->m_aTriangles[nodeNum].a],
						m_pobNavData->m_aVertices[m_pobNavData->m_aTriangles[nodeNum].b],
						m_pobNavData->m_aVertices[m_pobNavData->m_aTriangles[nodeNum].c],
						u, v ) );
}

//--------------------------------------------------
//!
//!	CAINavGraphManager::GetRandomReachablePosInGraph
//!
//--------------------------------------------------

//CPoint CAINavGraphManager::GetRandomReachablePosInGraph( const CPoint& obPos, const CKeywords& keywords ) const
CPoint CAINavGraphManager::GetRandomReachablePosInGraph( const CPoint& obPos ) const
{
	// check that all connections are 2 way
	static bool printonce = true;
	if (printonce)
	{
		for (int i = 0; i < m_pobNavData->m_iNumTriangles; ++i)
		{
			//ntPrintf( "tri: %d   conns: %d\n", i, GetNumConnections( i ) );
			for (int j = 0; j < AINavData::AI_CONNECTIONS_PER_TRIANGLE; ++j)
			{
				int connectedTri = GetConnected( i, j );

				if (connectedTri != -1)
				{
					for (int k = 0; k < AINavData::AI_CONNECTIONS_PER_TRIANGLE; ++k)
					{
						int reverseConnect = GetConnected( connectedTri, k );
						if (reverseConnect == i)
						{
							break;
						}
						ntAssert( reverseConnect != -1 );
					}
				}
			}
		}
		printonce = false;
	}

	// generate number of steps randomly between 0 and 2 * num nodes
	int numSteps = static_cast<int>(grandf( m_pobNavData->m_iNumTriangles * 2 ));

	// get node containing obPos
	int current = m_pobNavData->GetContainingNode( obPos );
//	ntAssert( current != -1 );

	
	
	// XXX: This was happening at the start of the Walkways/Walkways level sometimes.
	//		Probably a bad fix, but had to return something :). ARV.
	// PMN: This fix will do for now. I'll change it when the designers or whoever
	//      have given me a concrete spec for what should happen in the game when 
	//		AIs find themselves outside of the navgraph

	if ( current == -1 )
	{
		return obPos;
	}

	//ntPrintf( "\n" );
	//ntPrintf( "---------------------\n" );
	//ntPrintf( "NEW RANDOM TARGET GEN\n" );
	//ntPrintf( "---------------------\n" );
	//ntPrintf( " - In node %d\n", current );
	//ntPrintf( " - Taking %d steps\n", numSteps );

	for (int i = 0; i < numSteps; ++i)
	{
		// count the number of valid connections on this node
		int numConnections = GetNumConnections( current );

		if (numConnections == 0)
		{
			break;
		}

        // get a random connection from the current node
		int connection = static_cast<int>(grandf( numConnections ));
		if (connection == numConnections)
		{
			--connection;
		}

		//ntPrintf( " - Chose connection %d / %d\n", connection, numConnections );

		// make the node on the other end of the connection the current node
		// as long as it's valid
		current = GetConnected( current, connection );
		ntAssert( current != -1 );
		//ntPrintf( " - Next node is %d\n", current );
	}

	//ntPrintf( " - Finished, target node is %d\n", current );

	// now we have a target node, pick a random position in it
	CPoint tmp = GetRandomPosInNode( current );
	bool ySet = false; 

	for (int i = 0; i < m_pobNavData->m_iNumRegions; i++)
	{
		if (m_pobNavData->m_aRegions[i].minTriIndex <= current && m_pobNavData->m_aRegions[i].maxTriIndex >= current)
		{
			tmp.Y() = m_pobNavData->m_aRegions[i].obWorldMax.Y();
			ySet = true; 
		}
	}
	ntAssert( ySet );
	return CPoint ( GetRandomPosInNode( current ) );
}

/***************************************************************************************************
*
*	FUNCTION		
*
*	DESCRIPTION		
*
***************************************************************************************************/

CAINavPath*
CAINavGraphManager::MakePath( const CPoint& obStart, const CPoint& obEnd, CKeywords* keywords = NULL )
{
	int	obStartNode = m_pobNavData->GetContainingNode( obStart );
	int obDestNode = m_pobNavData->GetContainingNode( obEnd );

	if (obStartNode != -1 && obDestNode != -1)
	{
		// create a pathfinder for this search
		CAStar	obPathfinder( keywords );
		bool success = obPathfinder.GeneratePath( obStartNode, obDestNode );

		if (!success)
		{
#ifdef USER_PaulN
			ntPrintf("Pathfinding warning: no valid path from %d -> %d\n", obStartNode, obDestNode);
#endif
			return NULL;
		}

		// build a list of waypoints from start to end through the nodes determined by the pathfinder
		// now the path is built, we grab the best node from the pathfinder and follow its parents back
		// to the start node

		int	from = -1;
		int	to = -1;
		CAIAStarNode*	best = obPathfinder.GetBestNode();

		CAINavPath* obNavPath = NT_NEW_CHUNK( Mem::MC_AI ) CAINavPath();

		while (best)
		{
            to = from;
			from = best->navNode;

			if (to != -1 && from != -1)
			{
				// find edge between to and from
				int obEdge = GetConnection( from, to );

				// add to FRONT of the list, as we're building the path in reverse order
				obNavPath->AddEdge( obEdge );
			}

			best = best->parent;
		}

		obNavPath->SetNodes( obStartNode, obDestNode );
		obNavPath->SetStart( obStart );
		obNavPath->SetDest( obEnd );
        
		return obNavPath;
	}
	else
	{
		//ntPrintf("Pathfinding Error: start and end must be in navgraph, s: %d e: %d\n", obStartNode, obDestNode);
	}
	return 0;
}

/***************************************************************************************************
*
*	FUNCTION		
*
*	DESCRIPTION		Checks line of sight from (floor based) positions
*					Adds an offset to y in a bodgy fashion (same as GetEyePos) so lines don't
*					intersect with small bumps on the floor
*
***************************************************************************************************/

bool
CAINavGraphManager::HasLineOfSight( const CPoint& obFrom, const CPoint& obTo, const CEntity* pobIgnoreEntity, const float fYOffset )
{
	CPoint	obOffsetFrom( obFrom.X(),	obFrom.Y() + fYOffset,	obFrom.Z() );
	CPoint	obOffsetTo	( obTo.X(),		obTo.Y() + fYOffset,	obTo.Z() );

	// do a raycast check
	Physics::TRACE_LINE_QUERY stQuery;

	Physics::RaycastCollisionFlag obFlag; obFlag.base = 0;
	// [Mus] - What settings for this cast ?
	obFlag.flags.i_am = Physics::COLLISION_ENVIRONMENT_BIT;
	obFlag.flags.i_collide_with = (	Physics::SMALL_INTERACTABLE_BIT				|
									Physics::LARGE_INTERACTABLE_BIT				);

	return !(Physics::CPhysicsWorld::Get().TraceLine( obOffsetFrom, obOffsetTo, pobIgnoreEntity, stQuery, obFlag ));
}



bool
CAINavGraphManager::HasLineOfSightWithWidth( const CPoint& obFrom, const CPoint& obTo, const CEntity* pobIgnoreEntity, const float fYOffset, const float fWidth )
{
/*
	CPoint	obOffsetFrom( obFrom.X(),	obFrom.Y() + fYOffset,	obFrom.Z() );
	CPoint	obOffsetTo	( obTo.X(),		obTo.Y() + fYOffset,	obTo.Z() );

	// do a raycast check
	TRACE_LINE_QUERY stQuery;

	Physics::RaycastCollisionFlag obFlag; obFlag.base = 0;
	// [Mus] - What settings for this cast ?
	obFlag.flags.i_am = Physics::COLLISION_ENVIRONMENT_BIT;
	obFlag.flags.i_collide_with = (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
									Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
									Physics::RAGDOLL_BIT						|
									Physics::SMALL_INTERACTABLE_BIT				|
									Physics::LARGE_INTERACTABLE_BIT				);
*/

	// take the vector from -> to and scale it to fWidth
	CPoint	obDiff( obTo - obFrom );
	obDiff /= obDiff.Length();
	obDiff *= fWidth;

	// rotate it 90 degrees and store the pos
	const float	fAngularModifier( HALF_PI );

	float fCos = cos( fAngularModifier );
	float fSin = sin( fAngularModifier );
	float fNewX = fCos * obDiff.X() + fSin * obDiff.Z();
	float fNewZ = fCos * obDiff.Z() - fSin * obDiff.X();

	obDiff.X() = fNewX;
	obDiff.Z() = fNewZ;

	CPoint obPos1( obFrom + obDiff );

	// rotate it 180 and store this pos
	fCos = cos( fAngularModifier * 2.0f );
	fSin = sin( fAngularModifier * 2.0f );
	fNewX = fCos * obDiff.X() + fSin * obDiff.Z();
	fNewZ = fCos * obDiff.Z() - fSin * obDiff.X();

	obDiff.X() = fNewX;
	obDiff.Z() = fNewZ;

	CPoint obPos2( obFrom + obDiff );

	// do the line of sight tests with the new generated positions
	return (		HasLineOfSight( obPos1, obTo, pobIgnoreEntity, fYOffset )
				&&	HasLineOfSight( obPos2, obTo, pobIgnoreEntity, fYOffset )	);

}

bool CAINavGraphManager::LineContainedByGraph( CPoint& from, CPoint& to, int& nodeHint )
{
	UNUSED(nodeHint);
	
	const float resolution = 0.5f;
	//const float resolution = 2.0f;
	CPoint diff = to - from;
	float divider = ceil( diff.Length() ) * (1.0f / resolution);
	CPoint increment = diff / divider;
	int count = static_cast<int>( divider ) - 1;

	// we can re-use our temp diff as the test point
	diff = from;
	const float radius = 0.7f;
	UNUSED(radius);

	for(int i = 0 ; i < count ; ++i )
	{
		diff += increment;
		if (!InGraph( diff, radius ))
		//if (!InGraph( diff ))
		{
			return false;
		}
	}

	return true;
}


void CAINavGraphManager::ClampPointToFloor( CPoint& obPos )
{
	// do a raycast check
	Physics::TRACE_LINE_QUERY stQuery;

	Physics::RaycastCollisionFlag obFlag; obFlag.base = 0;

	obFlag.flags.i_am = Physics::COLLISION_ENVIRONMENT_BIT;
	obFlag.flags.i_collide_with = (	Physics::SMALL_INTERACTABLE_BIT				|
									Physics::LARGE_INTERACTABLE_BIT				);

	Physics::CPhysicsWorld::Get().TraceLine( obPos, CPoint( obPos.X(), obPos.Y() - 100.0f, obPos.Z() ), NULL, stQuery, obFlag );
	obPos = stQuery.obIntersect;
}


//------------------------------------------------------------------------------------------
//!
//!	CAINavGraphManager::GetRandomPosInGraph
//! 
//!
//------------------------------------------------------------------------------------------

CPoint CAINavGraphManager::GetRandomPosInGraph() const
{
	return m_pobNavData->GetNodeCentre( grand() % m_pobNavData->m_iNumTriangles );
}

//------------------------------------------------------------------------------------------
//!
//!	CAINavGraphManager::GetNearestPosInGraph
//! 
//!
//------------------------------------------------------------------------------------------

CPoint CAINavGraphManager::GetNearestPosInGraph( const CPoint &pos ) const
{	
	float dist;
	float minDist = FLT_MAX;
	int closestNode = -1;
	CPoint closestPoint;
	for (int i = 0; i < m_pobNavData->m_iNumTriangles; ++i)
	{
		CPoint testPoint = AIDistance::ClosestPointOnTriangle(
			m_pobNavData->m_aVertices[ m_pobNavData->m_aTriangles[i].a ],
			m_pobNavData->m_aVertices[ m_pobNavData->m_aTriangles[i].b ],
			m_pobNavData->m_aVertices[ m_pobNavData->m_aTriangles[i].c ],
			pos,
			&dist );

		if (dist < minDist)
		{
			minDist = dist;
			closestNode = i;
			closestPoint = testPoint;
		}
	}
 
	ntAssert( closestNode != -1 );
	return closestPoint;
}

//------------------------------------------------------------------------------------------
//!
//!	CAINavGraphManager::GetNearestNode
//! 
//!
//------------------------------------------------------------------------------------------

int CAINavGraphManager::GetNearestNode( const CPoint &pos ) const
{
	float dist;
	float minDist = FLT_MAX;
	int closestNode = -1;
	for (int i = 0; i < m_pobNavData->m_iNumTriangles; ++i)
	{
		AIDistance::ClosestPointOnTriangle(
			m_pobNavData->m_aVertices[ m_pobNavData->m_aTriangles[i].a ],
			m_pobNavData->m_aVertices[ m_pobNavData->m_aTriangles[i].b ],
			m_pobNavData->m_aVertices[ m_pobNavData->m_aTriangles[i].c ],
			pos,
			&dist );

		if (dist < minDist)
		{
			minDist = dist;
			closestNode = i;
		}
	}
 
	ntAssert( closestNode != -1 );
	return closestNode;
}

//------------------------------------------------------------------------------------------
//!
//!	CAINavGraphManager::NodeContains
//! 
//!
//------------------------------------------------------------------------------------------

bool CAINavGraphManager::NodeContains( int nodeNum, const CPoint& obPos ) const
{
	return m_pobNavData->IsPointInTriangle( obPos, nodeNum );
}

//------------------------------------------------------------------------------------------
//!
//!	CAINavGraphManager::GetConnection
//! 
//!
//------------------------------------------------------------------------------------------

int CAINavGraphManager::GetConnection( int nodeA, int nodeB ) const
{
	for (int i = 0; i < AINavData::AI_CONNECTIONS_PER_TRIANGLE; i++)
	{
		if (m_pobNavData->m_aTriangles[nodeA].connections[i] != -1)
		{
			if (  (	m_pobNavData->m_aConnections[ m_pobNavData->m_aTriangles[nodeA].connections[i] ].tris[0] == nodeA
				&&	m_pobNavData->m_aConnections[ m_pobNavData->m_aTriangles[nodeA].connections[i] ].tris[1] == nodeB )
				||(	m_pobNavData->m_aConnections[ m_pobNavData->m_aTriangles[nodeA].connections[i] ].tris[0] == nodeB
				&&	m_pobNavData->m_aConnections[ m_pobNavData->m_aTriangles[nodeA].connections[i] ].tris[1] == nodeA )	)
			{
				return m_pobNavData->m_aTriangles[nodeA].connections[i];
			}
		}
	}
	return -1;
}

//------------------------------------------------------------------------------------------
//!
//!	CAINavGraphManager::GetConnectionPos
//! 
//!
//------------------------------------------------------------------------------------------

CPoint CAINavGraphManager::GetConnectionPos( int connectionNum ) const
{ 
	ntAssert( connectionNum > -1 && connectionNum < m_pobNavData->m_iNumConnections );
	return (
			m_pobNavData->m_aVertices[ m_pobNavData->m_aConnections[connectionNum].verts[0] ]
		+	m_pobNavData->m_aVertices[ m_pobNavData->m_aConnections[connectionNum].verts[1] ] ) * 0.5f;
}

//------------------------------------------------------------------------------------------
//!
//!	CAINavGraphManager::GetNumConnections
//! 
//!
//------------------------------------------------------------------------------------------

int CAINavGraphManager::GetNumConnections( int nodeNum ) const
{ 
	int count = 0;
	for (int i = 0; i < AINavData::AI_CONNECTIONS_PER_TRIANGLE; ++i)	// up to 3 connections on a node
	{
		if (m_pobNavData->m_aTriangles[nodeNum].connections[i] != -1)
		{
			++count;
		}
	}
	return count;
}

//------------------------------------------------------------------------------------------
//!
//!	CAINavGraphManager::GetConnected
//! 
//!
//------------------------------------------------------------------------------------------

int CAINavGraphManager::GetConnected( int nodeNum, int connectionNum ) const
{
	int i = m_pobNavData->m_aTriangles[nodeNum].connections[connectionNum];
	if (i == -1)
	{
		return -1;
	}

	if (m_pobNavData->m_aConnections[i].tris[0] == nodeNum)
	{
		return m_pobNavData->m_aConnections[i].tris[1];
	}
	else
	{
		ntAssert( m_pobNavData->m_aConnections[i].tris[1] == nodeNum );
        return m_pobNavData->m_aConnections[i].tris[0];
	}
}

//------------------------------------------------------------------------------------------
//!
//!	CAINavGraphManager::GetNodePos
//! 
//!
//------------------------------------------------------------------------------------------

CPoint CAINavGraphManager::GetNodePos( int nodeNum ) const
{
	return CPoint(
	(	m_pobNavData->m_aVertices[m_pobNavData->m_aTriangles[nodeNum].a]
	+	m_pobNavData->m_aVertices[m_pobNavData->m_aTriangles[nodeNum].b]
	+	m_pobNavData->m_aVertices[m_pobNavData->m_aTriangles[nodeNum].c]	) * 0.33333333333f );
}

