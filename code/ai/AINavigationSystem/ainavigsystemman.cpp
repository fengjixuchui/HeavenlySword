//! -------------------------------------------
//! ainavigsystemman.h
//!
//! AI Navigation System Manager
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//! Date	04/05/06 - Creation
//!--------------------------------------------


#include "ainavigsystemman.h"
#include "game/luaglobal.h" // To Expose methods to LUA 
#include "core/timer.h"
#include "core/gatso.h"

#include "core/visualdebugger.h"
#include "game/query.h"
#include "game/renderablecomponent.h"
#include "game/entitymanager.h"
#include "game/movement.h"
#include "game/shellconfig.h"
#include "game/aicomponent.h"

#include "physics/world.h"

#define IS_DEFAULT_FLAG(x) (x & NF_DEFAULT)
#define PRINT_PATH(x) if (g_ShellOptions->m_bPrintPathDebug) { ntPrintf(#x "\n"); pPath->PrintPath(); }
#define DEBUG_PRINT_NAVSYSMAN(condition_msg) if (g_ShellOptions->m_bDivingDebug) { Debug::Printf("NAVSYSMAN: "); Debug::Printf condition_msg; }
#define TEST_AI_VOLUMES (g_ShellOptions->m_bTestAIVols)


//------------------------------------------------------------------------------------------
// CAINavigationSystemMan Lua Interface
//------------------------------------------------------------------------------------------
LUA_EXPOSED_START(CAINavigationSystemMan)
	LUA_EXPOSED_METHOD(GenerateAIBotList,			GenerateAIBotList, "Generates a list of the AI Bots", "void", "void")
	LUA_EXPOSED_METHOD(GenerateDynObstacleList,		GenerateDynObstacleList, "Generates a list with the dyn obstacles", "void", "void")
	LUA_EXPOSED_METHOD(RemoveEntity,				RemoveEntity, "Removes an obstacle from the Dynamic Obstacles list", "entity", "entity to remove")
	LUA_EXPOSED_METHOD(SetDebugDiveCone,			SetDebugDiveCone, "", "float length, float half_raiuds", "length, half_raiuds")
	LUA_EXPOSED_METHOD(SetPostBoltShootingCoverTime,	SetPostArrowHitCoverTime, "", "float time", "time")
	LUA_EXPOSED_METHOD(AddPatroller,				AddPatroller,"Adds a Patroller to a Patrol Graph","entity Patroller, string Patrol Graph Name","Patroller")
	LUA_EXPOSED_METHOD(MovingVolley_SetVolleyShots, MovingVolley_SetVolleyShots, "","","")
	LUA_EXPOSED_METHOD(MovingVolley_StartVolley, MovingVolley_StartVolley, "","","")
	LUA_EXPOSED_METHOD(RemoveAIVolume, RemoveAIVolume, "","","")
	
//	LUA_EXPOSED_METHOD(RemoveLeader,RemoveLeader,"Removes a Leader","entity Leader","Leader to be removed")
LUA_EXPOSED_END(CAINavigationSystemMan)


//! ---------------------------------------------------------------------------------------------------------------------------------
//!													Constructor, Update and Management
//! ---------------------------------------------------------------------------------------------------------------------------------

//! ------------------------------------------------------------------------------------------
//! Constructor, et al
//! ------------------------------------------------------------------------------------------

CAINavigationSystemMan::CAINavigationSystemMan() :
		/*m_bCollisionWithAIVolumes(true),*/ m_bNewNavigationSystem(true),
		m_bRenderAIAvoidance(false), m_bFormationSlotChanging(false), m_bRenderWallAvoidance(false),
		m_bRenderTotalSteeringAction(false), m_bRenderNavigGraph(false), m_bRenderPatrolGraph(false),
		m_bRenderViewCones(false), m_bRenderKnowledge(false), m_bRenderAIWorldVolumes(false), m_bRenderVideoText(false),
		m_bMovingVolleyDebugRender(false),
		m_pAIList(NULL),
		m_fPostArrowHitRadiusSQR(49.0f), m_fPostArrowHitCoverTime(15.0f), m_fTimeWithoutArrowHits(0.0f)
{
	// Lua Registering
	ATTACH_LUA_INTERFACE(CAINavigationSystemMan);
	CLuaGlobal::Get().State().GetGlobals().Set("CAINavigationSystemMan", this);
}


void CAINavigationSystemMan::LevelUnload ( void )
{
	;
}

#ifndef _RELEASE
static void ShowsDemoText( void )
{
	static int i = 0;

	if (i++ < 32)
	{
		//g_VisualDebug->Printf2D(10.0f,10.0f,DC_WHITE,0,"WayPoint Nav./Leader Follwing/Obstacle(dyn,Static,AI) Avoidance");
		g_VisualDebug->Printf2D(10.0f,10.0f,DC_WHITE,0,"New AI Volumes Detection (Initial Version)");
	}
	else if (i++>64)
	{
		i=0;
	}
}
#endif

//! ------------------------------------------------------------------------------------------
//! Update
//! ------------------------------------------------------------------------------------------
void  CAINavigationSystemMan::Update(float fTimeChange)
{ 
	// Update Kai's hit timers
	Player* pPlayer = CEntityManager::Get().GetPlayer();
	if ( pPlayer && pPlayer->IsArcher())
	{
		if ( m_fTimeWithoutArrowHits < 120.0f )
			m_fTimeWithoutArrowHits+=fTimeChange;
	}
	else
		ResetBoltCoverTimer();

	// Update AI List
	//CEntityQuery m_obAIListQuery;
	m_obAIListQuery.ClearSelectedResults();

	CEntityManager::Get().FindEntitiesByType( m_obAIListQuery, CEntity::EntType_AI);
	m_pAIList = &m_obAIListQuery.GetResults();

	m_obAIDivingMan.Update(fTimeChange);
	
	// Debug Render
#ifndef _RELEASE
	if (m_bRenderNavigGraph)		m_obNavigGraphManager.DebugRenderNavigGraph(); 
	if (m_bRenderPatrolGraph)		m_obNavigGraphManager.DebugRenderPathGraph();
	if (m_bRenderAIWorldVolumes)	m_obAIWorldMan.DebugRender();
	if (m_bRenderVideoText)			ShowsDemoText();
	if (m_bMovingVolleyDebugRender) MovingVolleyDebugRender();
	if (g_ShellOptions->m_bDivingDebug && m_bRenderAIWorldVolumes) m_obAIDivingMan.DebugRender();

	if (TEST_AI_VOLUMES == true)
	{
		TestAIVolumes();
	}
#endif

}

//! ------------------------------------------------------------------------------------------
//! TriggerKaiHitAI
//! ------------------------------------------------------------------------------------------
void CAINavigationSystemMan::TriggerKaiHitAI( CEntity* pAI )
{
	if (!pAI || !pAI->IsAI())
		return;

	// Reset Cover-Timer counter
	ResetBoltCoverTimer();

	// Generate list of AIs
	CEntityQuery obAIBotList;
	CEntityManager::Get().FindEntitiesByType(obAIBotList, CEntity::EntType_AI);
	
	// Loop through the list of AIs
	QueryResultsContainerType::iterator obEnd	= obAIBotList.GetResults().end();
	QueryResultsContainerType::iterator obIt	= obAIBotList.GetResults().begin();
	for ( ; obIt != obEnd; ++obIt )
	{
		CEntity* pEntAI = (*obIt);
 
		// Don't count PAUSED, DEAD or MYSELF
		if (	pEntAI->ToCharacter()->IsPaused() ||
				pEntAI->ToCharacter()->IsDead() ||
				pAI == pEntAI
			)
				
		{
			continue;
		}
		
		// Only alert nearby AIs
		CDirection  Dist2NearbyAI = CDirection( pAI->GetPosition() - pEntAI->GetPosition());
		float fDist2NearbyAISQR = Dist2NearbyAI.LengthSquared();

		if ( fDist2NearbyAISQR > m_fPostArrowHitRadiusSQR )
			continue;
		
		// Inform nearby AIs of the hit
		Message obMessage(msg_ai_hit_by_kai_nearby);
		pEntAI->GetMessageHandler()->Receive( msg_ai_hit_by_kai_nearby );
	}
}

//------------------------------------------------------------------------------------------
// CalculateSteeringAction
//------------------------------------------------------------------------------------------
CDirection CAINavigationSystemMan::CalculateSteeringAction ( CAIMovement* pMov, float fTimeChange )
{
	CGatso::Start("CAINavigationSystemMan::CalculateSteeringAction");
	CDirection dirReturn = m_obSteeringLibrary.CalculateSteeringAction( pMov, fTimeChange );
	CGatso::Stop("CAINavigationSystemMan::CalculateSteeringAction");
	return dirReturn;
}

//------------------------------------------------------------------------------------------
// IsPosValidForFormation_OPF (One Per Frame)
//------------------------------------------------------------------------------------------
bool CAINavigationSystemMan::IsPosValidForFormation_OPF ( const CPoint& obOrigin, const CPoint& obTestPos, bool* bExecuted, float fThreshold  )
{
	static unsigned int uiLastCallerTime = 0;
	const unsigned int uiCurrentCallerTime = CTimer::Get().GetSystemTicks();

	*bExecuted = true;
		
	if ( uiLastCallerTime == uiCurrentCallerTime ) 
	{
		// Resource Locked
		uiLastCallerTime = uiCurrentCallerTime;
		*bExecuted = false;
		return true;
	}

	// Update Timer
	uiLastCallerTime = uiCurrentCallerTime;
	bool bValidY = fabsf(obOrigin.Y() - obTestPos.Y()) < 0.3f;

	return ( bValidY && !IsThereObstaclesAtPos(obTestPos,fThreshold) && HasLineOfSight(obOrigin,obTestPos,fThreshold));
}

//! ---------------------------------------------------------------------------------------------------------------------------------
//!													High Level Navigation Commands
//! ---------------------------------------------------------------------------------------------------------------------------------

//! -------------------------------------------
//! FollowPathTo (from/to Node or Entity)
//! -------------------------------------------
bool CAINavigationSystemMan::FollowPath ( AI* pEnt, bool* bExecuted, unsigned int flag )
{
	static unsigned int uiLastCallerTime = 0;
	const unsigned int uiCurrentCallerTime = CTimer::Get().GetSystemTicks();

	*bExecuted = true;

	if (!pEnt) 
		return false;
		
	if ( uiLastCallerTime == uiCurrentCallerTime ) 
	{
		// Resource Locked
		DEBUG_PRINT_NAVIGSYSTEMMAN(("FollowPath(AI) - TO NODE has delayed its execution for AI [%s]\n",ntStr::GetString(pEnt->GetName()) ));
		uiLastCallerTime = uiCurrentCallerTime;
		*bExecuted = false;
		return true;
	}

	// Update Timer
	uiLastCallerTime = uiCurrentCallerTime;

	CAIMovement* pMov = pEnt->GetAIComponent()->GetCAIMovement();
	
	// Get the AI's path container
	CAINavigPath* pPath	= pEnt->GetAIComponent()->GetCAIMovement()->GetPathContainer();
	
	// Check start and end node validity
	if (!pMov->IsDestinationNodeValid()) 
		return false;

	CAINavigNode* pEndNode = m_obNavigGraphManager.GetNodeWithName(pMov->GetDestinationNodeName());
	
	if (!pEndNode) 
		return false;

	// Pass the Intermediate Nodes to the Path Container (DECIDED NOT TO BE IMPLEMENTED)

	//	pPath->SetIntermediateNodes(m_obNavigGraphManager.GetIntermediateNodeList(pMov->GetNodesSetName()));

	// Make Path
	if (!pMov->IsStartNodeValid())
	{
		// Ai's location to Node Pathfinding
		m_obNavigGraphManager.MakePath(pEnt->GetPosition(), pEndNode, pPath );
	}
	else
	{
		// Node to Node Pathfinding
		CAINavigNode* pStartNode = m_obNavigGraphManager.GetNodeWithName(pMov->GetStartNodeName());
		if (!pStartNode) 
			return false;
		m_obNavigGraphManager.MakePath(pStartNode, pEndNode, pPath );
	}

	PRINT_PATH(NormalPath);

	// Found Path?

	pMov->SetFollowPathCompleted(pPath->empty());
	if (pPath->empty()) 
	{
		pMov->SetFollowPathCompleted(true);
		return false;
	}

	// Path optimisation

	if ( !(pPath->IsSingleNodePath()) )
	{
		pPath->TrimPath(pMov->GetTrimPath());
		PRINT_PATH(Trimed_Path:-------------);
		pPath->EnhanceFirstNode(pEnt->GetPosition());
		PRINT_PATH(Enhanced path:-------------);
	}

	// Activate the Walk/Run Movement Controller
	if( !pEnt->GetAIComponent()->ActivateController(CAIComponent::MC_WALKING) )
	{
		return false;
	}

	// Set-up the steering flags and max speed
	if ( IS_DEFAULT_FLAG(flag) ) 
		SetSteeringFlags(pEnt, NF_DEF_FOLLOW_PATH_COVER); 
	else 
		SetSteeringFlags(pEnt, flag);

	return true;
}



//! -------------------------------------------
//! FollowPathTo (to Node)
//! -------------------------------------------
bool CAINavigationSystemMan::FollowPathTo ( AI* pEnt, CAINavigNode* pobTargetNode, float fMaxSpeed, unsigned int flag )
{
	if (!pEnt) return false;

	CAINavigPath*	pPath	= pEnt->GetAIComponent()->GetCAIMovement()->GetPathContainer();

	// Set-up the steering flags and max speed
	if ( IS_DEFAULT_FLAG(flag) ) SetSteeringFlags(pEnt, NF_DEF_FOLLOW_PATH_COVER); else SetSteeringFlags(pEnt, flag);

	// Get Path
	m_obNavigGraphManager.MakePath(pEnt->GetPosition(), pobTargetNode, pPath );
	PRINT_PATH(NormalPath);
	
	// Prepare the Movement Controller
	bool bControllerActive = pEnt->GetAIComponent()->ActivateController(CAIComponent::MC_WALKING);

	CAIMovement* pMov = pEnt->GetAIComponent()->GetCAIMovement();

	// If there a path?
	if (pPath->empty() || !bControllerActive ) 
	{
		// Nop....
		pMov->SetFollowPathCompleted(true);
		return false;
	}
	else
	{
		pMov->SetFollowPathCompleted(false);
	}

	if ( !(pPath->IsSingleNodePath()))
	{
		// Optimise path
		pPath->TrimPath(pMov->GetTrimPath());
		PRINT_PATH(Trimed_Path:-------------);
		pPath->EnhanceFirstNode(pEnt->GetPosition());
		PRINT_PATH(Enhanced path:-------------);
		// Max Speed
		SetMaxSpeed(pEnt,fMaxSpeed);
	}
	return true;
}

//------------------------------------------------------------------------------------------
// FollowPathTo (to Point)
//------------------------------------------------------------------------------------------
bool CAINavigationSystemMan::FollowPathTo	( AI* pEnt, const CPoint& obTargetPoint, float fMaxSpeed, unsigned int flag )
{
	if (!pEnt) return false;

	CAINavigPath*	pPath	= pEnt->GetAIComponent()->GetCAIMovement()->GetPathContainer();

	// Set-up the steering flags
	if ( IS_DEFAULT_FLAG(flag) ) SetSteeringFlags(pEnt, NF_DEF_FOLLOW_PATH_COVER); else SetSteeringFlags(pEnt, flag);
	
	// Get Path
	m_obNavigGraphManager.MakePath(pEnt->GetPosition(), obTargetPoint, pPath );
	PRINT_PATH(NormalPath);

	// Prepare the Movement Controller
	CAIComponent* pAIC = pEnt->GetAIComponent();
	pAIC->SetAction(ACTION_WALK);
	pAIC->ActivateController(CAIComponent::MC_WALKING);
	if ( !(pPath->IsSingleNodePath()) )
	{
		// Optimise path
		//pPath->TrimPath();
		//PRINT_PATH(TrimedPath);
		pPath->EnhanceFirstNode(pEnt->GetPosition());
		// Max Speed
		SetMaxSpeed(pEnt,fMaxSpeed);
	}
	return (!(pPath->empty()));
}

//------------------------------------------------------------------------------------------
// FollowPathTo (to Entity)
//------------------------------------------------------------------------------------------
bool CAINavigationSystemMan::FollowPathTo	( AI* pEnt, const CEntity* pTargetEnt, bool* bExecuted, float fMaxSpeed, unsigned int flag )
{
	if (!pTargetEnt) return false;

	static unsigned int uiLastCallerTime = 0;
	const unsigned int uiCurrentCallerTime = CTimer::Get().GetSystemTicks();

	*bExecuted = true;
	
	if ( !pEnt ) return false;
	
	if ( uiLastCallerTime == uiCurrentCallerTime ) 
	{
		// Resource Locked
		DEBUG_PRINT_NAVIGSYSTEMMAN(("FollowPathTo(AI,Entity,...) has delayed its execution for AI [%s] - Tgt [%s]\n",ntStr::GetString(pEnt->GetName()),ntStr::GetString(pTargetEnt->GetName()) ));
		uiLastCallerTime = uiCurrentCallerTime;
		*bExecuted = false;
		return true;
	}
	
	// Update Timer
	uiLastCallerTime = uiCurrentCallerTime;

	return (FollowPathTo(pEnt, pTargetEnt->GetPosition(), fMaxSpeed, flag));
}

//------------------------------------------------------------------------------------------
// FollowPathTo (to Node Name)
//------------------------------------------------------------------------------------------
bool CAINavigationSystemMan::FollowPathTo ( AI* pEnt, const char* pcNode, float fMaxSpeed, unsigned int flag )
{
	if (!pcNode) return false;

	CAINavigNode* pNN = m_obNavigGraphManager.GetNodeWithName(CHashedString(pcNode));

	if (!pNN) return false;

	return (FollowPathTo(pEnt, pNN, fMaxSpeed, flag));
}

//------------------------------------------------------------------------------------------
// FollowPathToCoverPointInMinMaxRange
//------------------------------------------------------------------------------------------
bool CAINavigationSystemMan::FollowPathToCoverPointInMinMaxRange ( AI* pEnt, bool bForceExecution, bool* bExecuted, float fMaxSpeed, bool bStrafe, unsigned int flag )
{
	static unsigned int uiLastCallerTime = 0;
	const unsigned int uiCurrentCallerTime = CTimer::Get().GetSystemTicks();

	*bExecuted = true;
	
	if ( !pEnt ) return false;
	
	if ( !bForceExecution && uiLastCallerTime == uiCurrentCallerTime ) 
	{
		// Resource Locked
		DEBUG_PRINT_NAVIGSYSTEMMAN(("FollowPathToCoverPointInMinMaxRange() has delayed its execution for AI [%s]\n",ntStr::GetString(pEnt->GetName()) ));
		uiLastCallerTime = uiCurrentCallerTime;
		*bExecuted = false;
		return true;
	}
	
	// Update Timer
	uiLastCallerTime = uiCurrentCallerTime;

	const CEntity* pEnemy = pEnt->GetAIComponent()->GetCAIMovement()->GetEntityToAttack();
	// Find the closest valid cover point and book it
	CAINavigCoverPoint* pCP = m_obNavigGraphManager.GetClosestCoverPointInMinMaxRange(pEnt, pEnemy);
	CAINavigPath*	pPath	= pEnt->GetAIComponent()->GetCAIMovement()->GetPathContainer();
	pPath->clear();

	if (!pCP) return false;
	
	// Find a node that links to it
	CAINavigNode* pNN = m_obNavigGraphManager.GetANodeLinkedToCoverPoint(pCP);

	if (!pNN)
		return false;

	bool bPathFound = FollowPathTo(pEnt, pNN, fMaxSpeed, flag);

	if (bPathFound)
	{
		// We have a cover point and an path to it
		// ... so lets free its previous cover point
		
		CAIMovement* pMov = pEnt->GetAIComponent()->GetCAIMovement();

		pMov->SetClaimCoverPoint(NULL);
			
		//m_pMov->SetCoverPoint(NULL);
		pMov->SetGoingToCover(false);
		pMov->SetFollowPathCompleted(false);
        pMov->SetClaimCoverPoint(pCP);

		// Activate the proper controller
		if (bStrafe)
			pEnt->GetAIComponent()->ActivateController(CAIComponent::MC_STRAFING);
		else
			pEnt->GetAIComponent()->ActivateController(CAIComponent::MC_WALKING);

		pMov->ActivateFlag(CAINavigationSystemMan::NF_DEF_FOLLOW_PATH_DYN_COVER);
	}

	return (bPathFound);
}

////------------------------------------------------------------------------------------------
//// FollowLeader
////------------------------------------------------------------------------------------------
//void CAINavigationSystemMan::FollowLeader ( CEntity* pEnt, CEntity* pEntLeader, float fMaxSpeed, unsigned int flag )
//{
//	if ( (!pEntLeader) || (!pEnt) )	return; 
//
//	// Set-up the steering flags, leader and  max speed
//	if ( IS_DEFAULT_FLAG(flag) ) SetSteeringFlags(pEnt, NF_DEF_FOLLOW_LEADER); else SetSteeringFlags(pEnt, flag);
//	SetMaxSpeed	(pEnt,fMaxSpeed);
//	m_obLeaderMan.SetLeader(pEnt,pEntLeader);
//
//	CAIComponent* pAIC = pEnt->GetAIComponent();
//	pAIC->SetAction(ACTION_WALK);
//	pAIC->ActivateController(CAIComponent::MC_WALKING);
//}

//------------------------------------------------------------------------------------------
// FollowLeader
//------------------------------------------------------------------------------------------
//void CAINavigationSystemMan::FollowLeader ( CEntity* pEnt, CEntity* pEntLeader, float fMaxSpeed, unsigned int flag )
//{
//	if ( (!pEntLeader) || (!pEnt) )	return; 
//
//	static CEntity* pPlayer = CEntityManager::Get().GetPlayer();
//
//	// Set-up the steering flags, leader and  max speed
//	if ( pEntLeader == pPlayer ) 
//	{
//		if ( IS_DEFAULT_FLAG(flag) ) SetSteeringFlags(pEnt, NF_DEF_FOLLOW_ENTITY); else SetSteeringFlags(pEnt, flag);
//	}
//	else
//	{
//		if ( IS_DEFAULT_FLAG(flag) ) SetSteeringFlags(pEnt, NF_DEF_FOLLOW_ENTITY); else SetSteeringFlags(pEnt, flag);
//	}
//
//	SetMaxSpeed	(pEnt,fMaxSpeed);
//	//m_obLeaderMan.SetLeader(pEnt,pEntLeader);
//
//	CAIComponent* pAIC = pEnt->GetAIComponent();
//	pAIC->SetAction(ACTION_WALK);
//	pAIC->ActivateController(CAIComponent::MC_WALKING);
//}

//------------------------------------------------------------------------------------------
// FollowPatrolPath
// returns:		false (I cannot see my patrol path [PP]), true (otherwise)
// bSuccess:	false (I won't reach the PP), true (ok, I found an A* to the PP)
//------------------------------------------------------------------------------------------
bool CAINavigationSystemMan::FindPatrolPath(AI* pEnt, float fMaxSpeed, bool * bSuccess )
{
	*bSuccess = false;

	if (!pEnt) return false;
	
	CAINavigGraph* pNGPatroller = m_obNavigGraphManager.GetPatrollersGraph(pEnt);

	// Make sure I have a patroling path...
	if (!pNGPatroller) 
	{ 
		// Ok, no assigned patrol path was found, perhaps the nearest one could be given...
		ntAssert_p(0,("I am trying to patrol but have not been assigned to a patrolling path!!!")); 
		return false; 
	}

	// Can I see my patrol path?
	float fDist = -1.0f;
	CAINavigNode* pNNPatrolClosest = pNGPatroller->GetClosestNode(pEnt->GetPosition(), &fDist);

	if (!pNNPatrolClosest)
	{
		// I cannot see it... I will have to A* to it.
		CAINavigNode* pNN = m_obNavigGraphManager.GetClosestNode(pEnt->GetPosition(),&fDist);
		if (pNN)
		{
			// We are close to a node
			// Get Patrolling Path Locator
			*bSuccess = true;
			FollowPathTo(pEnt,pNGPatroller->GetPatrolLocator(),fMaxSpeed, NF_DEFAULT);
			return false;
		}
		else
		{
			// Big problem. I dont see my patrolling path nor any other node... 
			// ... I better stop quietly here...
			ntAssert_p(0,("I dont see my patrolling path nor any other node...!!!"));
			DisableMovement(pEnt);
			SetMaxSpeed	(pEnt,0);
			CAIComponent* pAIC = pEnt->GetAIComponent();
			pAIC->ActivateController(CAIComponent::MC_WALKING);
			return false;
		}
	}
	else
	{
		// Ok, the patrol path is in fron of me... go for it
		return true;
	}

}

//------------------------------------------------------------------------------------------
// FollowPathTo (to Point)
//------------------------------------------------------------------------------------------
bool CAINavigationSystemMan::FollowPathToReporter ( AI* pEnt, AI* pEntReporter, float fMaxSpeed, unsigned int flag  )
{
	if (!pEnt) return false;

	CAIMovement* pMov = pEnt->GetAIComponent()->GetCAIMovement();
	CAIPatrolGraph* pG = pMov->GetPatrolGraph();

	if (!pG) return false;

	CAINavigPath*	pPath	= pEnt->GetAIComponent()->GetCAIMovement()->GetPathContainer();

	// Set-up the steering flags
	if ( IS_DEFAULT_FLAG(flag) ) SetSteeringFlags(pEnt, NF_DEF_FOLLOW_PATH_COVER); else SetSteeringFlags(pEnt, flag);
	
	// Get Path
	pG->MakePath(pEnt, pEntReporter , pPath );
	PRINT_PATH(NormalPath);

	// Prepare the Movement Controller
	CAIComponent* pAIC = pEnt->GetAIComponent();
	pAIC->ActivateController(CAIComponent::MC_WALKING);
	if ( !(pPath->IsSingleNodePath()) )
	{
		pPath->EnhanceFirstNode(pEnt->GetPosition());
		// Max Speed
		SetMaxSpeed(pEnt,fMaxSpeed);
	}
	return (!(pPath->empty()));
}

//------------------------------------------------------------------------------------------
// FollowPatrolPath
//------------------------------------------------------------------------------------------
void CAINavigationSystemMan::FollowPatrolPath ( AI* pEnt, float fMaxSpeed, unsigned int flag )
{
	if (!pEnt) return;

	// Set-up the steering flags, leader and  max speed
	if ( IS_DEFAULT_FLAG(flag) ) SetSteeringFlags(pEnt, NF_DEF_PATROL_WALK); else SetSteeringFlags(pEnt, flag);
	SetMaxSpeed	(pEnt,fMaxSpeed);

	CAIComponent* pAIC = pEnt->GetAIComponent();
	pAIC->ActivateController(CAIComponent::MC_WALKING);

}

//------------------------------------------------------------------------------------------
// FollowEntity
//------------------------------------------------------------------------------------------
void CAINavigationSystemMan::FollowEntity ( AI* pEnt, unsigned int flag )
{
	if ( !pEnt )	return; 

	// Set-up the steering flags, max speed
	if ( IS_DEFAULT_FLAG(flag) ) SetSteeringFlags(pEnt, NF_DEF_FOLLOW_ENEMY); else SetSteeringFlags(pEnt, flag);

	CAIComponent* pAIC = pEnt->GetAIComponent();

	SetMaxSpeed	(pEnt,pAIC->GetCAIMovement()->GetChaseSpeed());

	pAIC->ActivateController(CAIComponent::MC_WALKING);
}

//------------------------------------------------------------------------------------------
// SteerToEntity
//------------------------------------------------------------------------------------------
void CAINavigationSystemMan::SteerToEntity ( AI* pEnt, unsigned int flag )
{
	ntAssert( pEnt );

	if ( !pEnt )	return; 

	// Set-up the steering flags, max speed
	if ( IS_DEFAULT_FLAG(flag) ) SetSteeringFlags(pEnt, NF_DEF_STEER_TO_ENTITY); else SetSteeringFlags(pEnt, flag);

	CAIComponent* pAIC = pEnt->GetAIComponent();

	SetMaxSpeed	(pEnt,pAIC->GetCAIMovement()->GetChaseSpeed());

	pAIC->ActivateController(CAIComponent::MC_WALKING);
}

//------------------------------------------------------------------------------------------
// SteerToEntity
//------------------------------------------------------------------------------------------
void CAINavigationSystemMan::ChaseMovingEntity ( AI* pEnt, unsigned int flag )
{
	if ( !pEnt )	return; 

	// Set-up the steering flags, max speed
	if ( IS_DEFAULT_FLAG(flag) ) SetSteeringFlags(pEnt, NF_DEF_CHASE_MOVING_ENTITY); else SetSteeringFlags(pEnt, flag);

	CAIComponent* pAIC = pEnt->GetAIComponent();

	SetMaxSpeed	(pEnt,pAIC->GetCAIMovement()->GetChaseSpeed());

	pAIC->ActivateController(CAIComponent::MC_WALKING);
}

//------------------------------------------------------------------------------------------
// SteerToLocatorNode
//------------------------------------------------------------------------------------------
bool CAINavigationSystemMan::SteerToLocatorNode ( AI* pEnt, unsigned int flag )
{
	if ( !pEnt ) 
		return false;

	CAIComponent* pAIC = pEnt->GetAIComponent();

	if (!pAIC->GetCAIMovement()->IsDestinationNodeValid()) 
		return false;

	// Set-up flags and controller
	if ( IS_DEFAULT_FLAG(flag) ) 
		SetSteeringFlags(pEnt, NF_DEF_STEER_TO_LOCATOR_NODE); 
	else 
		SetSteeringFlags(pEnt, flag);

	if( pAIC->ActivateController(CAIComponent::MC_WALKING) )
	{
		pAIC->GetCAIMovement()->SetMoveToSelectedPointCompleted(false);
		return true;
	}
	
	return false;
}

//------------------------------------------------------------------------------------------
// SteerToDestinationPoint
//------------------------------------------------------------------------------------------
bool CAINavigationSystemMan::SteerToDestinationPoint ( AI* pEnt, const CPoint& obPos, float fRadius, bool bStrafe, unsigned int flag )
{
	if ( !pEnt ) return false;
	if ( fRadius < 0.2f ) fRadius = 0.2f;

	CAIComponent* pAIC = pEnt->GetAIComponent();

	// Set-up flags, dest point and controller
	if ( IS_DEFAULT_FLAG(flag) ) SetSteeringFlags(pEnt, NF_DEF_STEER_TO_DEST_POINT); else SetSteeringFlags(pEnt, flag);

	pAIC->GetCAIMovement()->SetDestinationPoint( obPos, fRadius); // !!! - (Dario) Radius 0.5f to be parametrised
	pAIC->GetCAIMovement()->SetMoveToSelectedPointCompleted(true);

	bool bControllerActive = false;

	if (bStrafe)
		bControllerActive = pEnt->GetAIComponent()->ActivateController(CAIComponent::MC_STRAFING);
	else
		bControllerActive = pEnt->GetAIComponent()->ActivateController(CAIComponent::MC_WALKING);

	// If the controller activated, then mark the movement so that the destination point has yet to be reached.
	if( bControllerActive )
		pAIC->GetCAIMovement()->SetMoveToSelectedPointCompleted(false);

	return bControllerActive;
}

//------------------------------------------------------------------------------------------
// GoAroundVolume
//------------------------------------------------------------------------------------------
bool CAINavigationSystemMan::GoAroundVolume ( AI* pEnt, const CEntity* pEntToGoTo, bool* bExecuted, unsigned int flag )
{
	static unsigned int uiLastCallerTime = 0;
	const unsigned int uiCurrentCallerTime = CTimer::Get().GetSystemTicks();

	*bExecuted = true;
	
	if ( !pEnt ) return false;
	
	if ( uiLastCallerTime == uiCurrentCallerTime ) 
	{
		// Resource Locked
		uiLastCallerTime = uiCurrentCallerTime;
		*bExecuted = false;
		return true;
	}
	
	// Update Timer
	uiLastCallerTime = uiCurrentCallerTime;

	CAIComponent* pAIC	= pEnt->GetAIComponent();
	CAINavigPath* pPath	= pAIC->GetCAIMovement()->GetPathContainer();

	// ==============================
	// Check Volume Path
	// ==============================

	CAIWorldVolume* pWV = m_obAIWorldMan.GetClosestGoAroundVolumeOnTheWay(pEnt->GetPosition(), pEntToGoTo->GetPosition());

	if (pWV)
	{
		// There is a GoAroundVolume on the way
		if (!pWV->GetPathAroundVolume(pEnt->GetPosition(), pEntToGoTo->GetPosition(),pPath))
			return false;
	}

	// Set-up flags, dest point and controller
	if ( IS_DEFAULT_FLAG(flag) ) SetSteeringFlags(pEnt, NF_DEF_GO_AROUND_VOLUMES); else SetSteeringFlags(pEnt, flag);

	bool bControllerActive = false;
	bControllerActive = pEnt->GetAIComponent()->ActivateController(CAIComponent::MC_WALKING);

	// If the controller activated, then mark the movement so that the destination point has yet to be reached.
	if( bControllerActive )
		pAIC->GetCAIMovement()->SetGoAroundCompleted(false);

	return bControllerActive;
}


//------------------------------------------------------------------------------------------
// StrafeToCombatPoint
//------------------------------------------------------------------------------------------
bool CAINavigationSystemMan::StrafeToCombatPoint ( AI* pEnt, const CEntity* pEntTarget, float fMaxSpeed, const CPoint& obCombatPoint, unsigned int flag )
{
	if ( (!pEntTarget) || (!pEnt) )	
		return false; 

	// Set-up the steering flags, max speed
	if ( IS_DEFAULT_FLAG(flag) ) 
		SetSteeringFlags(pEnt, NF_DEF_STRAFE_ENEMY); 
	else 
		SetSteeringFlags(pEnt, flag);

	SetMaxSpeed	(pEnt,fMaxSpeed);


	// Set the facing diraction while moving
//	pMov->SetFacingAction(CDirection(pEntTarget->GetPosition() - pEnt->GetPosition()));
	
	// Set movement controller
	bool bControllerActive = pEnt->GetAIComponent()->ActivateController(CAIComponent::MC_STRAFING);

#ifndef _GOLD_MASTER
	if (CAINavigationSystemMan::Get().m_bRenderWallAvoidance && bControllerActive ) 
	{
		g_VisualDebug->RenderLine(pEnt->GetPosition(), obCombatPoint, DC_YELLOW);
	}
#endif

	// Set Combat point and facing action flag
	CAIMovement* pMov = pEnt->GetAIComponent()->GetCAIMovement();
	pMov->SetAttackPoint(obCombatPoint);
	pMov->SetMovingWhileFacingTgt(true);
	pMov->SetMoveToCombatPointCompleted( !bControllerActive );

	return bControllerActive;

}

//------------------------------------------------------------------------------------------
// StrafeToFormationPoint
//------------------------------------------------------------------------------------------
bool CAINavigationSystemMan::StrafeToFormationPoint ( AI* pEnt, const CEntity* pEntTarget, float fMaxSpeed, const CPoint& obCombatPoint, uint32_t& ruiCtrlID, unsigned int flag )
{
	if ( (!pEntTarget) || (!pEnt) )	
		return false; 

	// Mark any simple animations as complete. 
	pEnt->GetAIComponent()->CompleteSimpleAction();

	// Set-up the steering flags, max speed
	if ( IS_DEFAULT_FLAG(flag) ) 
		SetSteeringFlags(pEnt, NF_DEF_STRAFE_ENEMY); 
	else 
		SetSteeringFlags(pEnt, flag);


	SetMaxSpeed	(pEnt,fMaxSpeed);

	// Set Combat point and facing action flag
	CAIMovement* pMov = pEnt->GetAIComponent()->GetCAIMovement();
	pMov->SetAttackPoint(obCombatPoint);
	pMov->SetMovingWhileFacingTgt(true);
	pMov->SetMoveToCombatPointCompleted(false);
	pEnt->GetAIComponent()->SetPlayingFacingAction(false);


	// Set Clock or counterclockwise movement
	
	CDirection LineEnemy2Entity(pEnt->GetPosition() - pEntTarget->GetPosition());
	CDirection LinePoint2Entity(obCombatPoint - pEntTarget->GetPosition());
	CDirection PerpendicLineEnemy2Entity = m_obSteeringLibrary.GetPerpendicular(LineEnemy2Entity);

	if (PerpendicLineEnemy2Entity.Dot(LinePoint2Entity) > 0)
	{
		pMov->SetFormationMoveClockwise(true);
	}
	else
	{
		pMov->SetFormationMoveClockwise(false);
	}
	
	// 
	if( ruiCtrlID != pEnt->GetMovement()->GetNewControllerCount() )
	{
	// Set movement controller
		if( pEnt->GetAIComponent()->ActivateController(CAIComponent::MC_STRAFING, true) )
		{
			// Set the new controller id, if this changes again, then the active controller
			// function will need calling again.
			ruiCtrlID = pEnt->GetMovement()->GetNewControllerCount();
		}
		else
		{
			return false;
		}
	}
	else
	{
		return true;
	}

#ifndef _GOLD_MASTER
	if (CAINavigationSystemMan::Get().m_bRenderWallAvoidance) 
	{
		g_VisualDebug->RenderLine(pEnt->GetPosition()+CPoint(0,0.1f,0), obCombatPoint+CPoint(0,0.1f,0), DC_YELLOW);
	}
#endif

	return true;
}

//------------------------------------------------------------------------------------------
// GetDivingAction
//------------------------------------------------------------------------------------------
ENUM_DIVE_FULL_DIRECTION CAINavigationSystemMan::GetDivingAction ( CEntity* pEnt, bool *bExecuted )
{
	static unsigned int uiLastCallerTime = 0;
	const unsigned int uiCurrentCallerTime = CTimer::Get().GetSystemTicks();
	*bExecuted = true;
	
	bool bDiscarded = false;

	if (!pEnt || !pEnt->IsAI() || !m_obAIDivingMan.IsInTheDiversList( pEnt, &bDiscarded ) )
		return E_DONT_DIVE;

	if (bDiscarded)
	{
		return E_DONT_DIVE;
	}

	// ------------------------------------------------------------------
	// The Entity in in the divers list. Decide if he will dive or not
	// ------------------------------------------------------------------

    CAIMovement* pMov = ((AI*)pEnt)->GetAIComponent()->GetCAIMovement();

	if ( !pMov->IsInTimeForDiving() )
	{
		DEBUG_PRINT_NAVSYSMAN(( "AI: [%s] will NOT dive due to cooling period (%.3f<%0.3f)\n",ntStr::GetString(pEnt->GetName()), pMov->GetTimeSinceLastDive(), pMov->GetDivingFreeTime() ));
		return E_DONT_DIVE;
	}

	float fGameTimeScalar = CTimer::Get().GetGameTimeScalar();
	bool bInAfterTouch = ( fGameTimeScalar != 1);

	float fRand = erandf(1.0f);

	if (	(bInAfterTouch	&& fRand > pMov->GetDivingAftertouchProbability() ) ||
			(!bInAfterTouch	&& fRand > pMov->GetDivingProbability() )
		)
	{
		// Disregard current bolts
		DEBUG_PRINT_NAVSYSMAN(( "AI: [%s] will NOT dive %s due to low probability (%.3f<%.3f)\n",ntStr::GetString(pEnt->GetName()),bInAfterTouch ? "AFTERTOUCH" : "", bInAfterTouch ? pMov->GetDivingAftertouchProbability() : pMov->GetDivingProbability(), fRand ));
		m_obAIDivingMan.DisregardCurrentBolt( pEnt );
		return E_DONT_DIVE;
	}

	// ------------------------------------------------------------------
	// Lets make sure that only one AI dives on a single frame
	// ------------------------------------------------------------------
		
	if ( uiLastCallerTime == uiCurrentCallerTime ) 
	{
		// Resource Locked
		uiLastCallerTime = uiCurrentCallerTime;
		*bExecuted = false;
		return E_DONT_DIVE;
	}

	// Update Timer
	uiLastCallerTime = uiCurrentCallerTime;

	// ------------------------------------------------------------------
	// Check Panic Chances
	// ------------------------------------------------------------------
	fRand = erandf(1.0f);
	 
	if (	(bInAfterTouch && fRand < pMov->GetDivingAftertouchPanicProbability() ) ||
			(!bInAfterTouch && fRand < pMov->GetDivingPanicProbability() )
		)
	{
		// Panic!!!
		DEBUG_PRINT_NAVSYSMAN(( "AI: [%s] will PANIC %s due to probability (%.3f>%.3f)\n",ntStr::GetString(pEnt->GetName()),bInAfterTouch ? "AFTERTOUCH" : "", bInAfterTouch ? pMov->GetDivingAftertouchPanicProbability() : pMov->GetDivingPanicProbability(), fRand ));
		return E_DIVE_PANIC;
	}
	

	// ------------------------------------------------------------------------
	// Calculate the best dive dir. depending on the Bolt's trajectory
	// ------------------------------------------------------------------------
	unsigned int eDivDirDueToBoltDir = m_obAIDivingMan.GetDivingSideToAvoidBolt( pEnt );
	
	if ( eDivDirDueToBoltDir == E_DONT_DIVE )
		return E_DONT_DIVE;

	bool bLeft	= ( eDivDirDueToBoltDir & E_DIVE_SHORT_LEFT ) || ( eDivDirDueToBoltDir & E_DIVE_LONG_LEFT );
	bool bRight = ( eDivDirDueToBoltDir & E_DIVE_SHORT_RIGHT ) || ( eDivDirDueToBoltDir & E_DIVE_LONG_RIGHT );

	// ------------------------------------------------------------------------
	// Check the available space (WALLS) for diving
	// ------------------------------------------------------------------------
	pMov->ResetTimeSinceLastDive();
	DEBUG_PRINT_NAVSYSMAN(( "AI: [%s] will dive. Reseting Time Counter\n",ntStr::GetString(pEnt->GetName()) ));
	
	// Make sure the AI doesn't try to dive again for this bolt
	m_obAIDivingMan.DisregardCurrentBolt( pEnt );
	
	unsigned int uiValidDivingDirections = 0;

	if (bLeft && m_obAIWorldMan.TestDive_NoWallCollision(pEnt,E_DIVE_SHORT_LEFT))
		uiValidDivingDirections = E_DIVE_SHORT_LEFT;

	if (bRight && m_obAIWorldMan.TestDive_NoWallCollision(pEnt,E_DIVE_SHORT_RIGHT))
		uiValidDivingDirections += E_DIVE_SHORT_RIGHT;

	if (bLeft && m_obAIWorldMan.TestDive_NoWallCollision(pEnt,E_DIVE_LONG_LEFT))
		uiValidDivingDirections += E_DIVE_LONG_LEFT;

	if (bRight && m_obAIWorldMan.TestDive_NoWallCollision(pEnt,E_DIVE_LONG_RIGHT))
		uiValidDivingDirections += E_DIVE_LONG_RIGHT;

	// ------------------------------------------------------------------------
	// Check the available space (other AIs) for diving
	// ------------------------------------------------------------------------
	bool bShortLeft		= uiValidDivingDirections & E_DIVE_SHORT_LEFT;
	bool bShortRight	= uiValidDivingDirections & E_DIVE_SHORT_RIGHT;
	bool bLongLeft		= uiValidDivingDirections & E_DIVE_LONG_LEFT;
	bool bLongRight		= uiValidDivingDirections & E_DIVE_LONG_RIGHT;

	bShortLeft	= bShortLeft	? (TestDive_AICollision(pEnt,E_DIVE_SHORT_LEFT)		== NULL) : false;
	bShortRight = bShortRight	? (TestDive_AICollision(pEnt,E_DIVE_SHORT_RIGHT)	== NULL) : false;
	bLongLeft	= bLongLeft		? (TestDive_AICollision(pEnt,E_DIVE_LONG_LEFT)		== NULL) : false;
	bLongRight	= bLongRight	? (TestDive_AICollision(pEnt,E_DIVE_LONG_RIGHT)		== NULL) : false;


	// ------------------------------------------------------------------------
	// Based on the proximity data select a random dive movement
	// ------------------------------------------------------------------------ 
	float bRandChance = erandf(1.0f);

	//if (	(bInAfterTouch && bRandChance < pMov->GetDivingAftertouchPanicProbability() ) ||
	//		(!bInAfterTouch && bRandChance < pMov->GetDivingPanicProbability() )
	//	)
	//	return E_DIVE_PANIC;

	bool bTryLong	= (bRandChance < 0.5f);
	bool bTryShort	= (bRandChance > 0.5f && bRandChance < 0.9f );

	if (bTryLong && (bLongLeft || bLongRight) )
	{
		if (bLongLeft && bLongRight)
		{
			if (erandf(1.0f) < 0.5f)
				return E_DIVE_LONG_LEFT;
			else
				return E_DIVE_LONG_RIGHT;
		}
		else
		{
			if (bLongLeft)
				return E_DIVE_LONG_LEFT;
			
			return E_DIVE_LONG_RIGHT;
		}	
	}

	if ( bTryShort &&  (bShortLeft || bShortRight) )
	{
		if (bShortLeft && bShortRight)
		{
			if (erandf(1.0f) < 0.5f)
				return E_DIVE_SHORT_LEFT;
			else
				return E_DIVE_SHORT_RIGHT;
		}
		else
		{
			if (bShortLeft)
				return E_DIVE_SHORT_LEFT;
			
			return E_DIVE_SHORT_RIGHT;
		}	
	}

	return E_DIVE_CROUCH;
}

//------------------------------------------------------------------------------------------
// GetNavigationIntentions
//------------------------------------------------------------------------------------------
unsigned int CAINavigationSystemMan::GetNavigationIntentions( CEntity* pEnt )
{ 
	if (pEnt && pEnt->IsAI())
	{
		CAIMovement* pMov = pEnt->ToAI()->GetAIComponent()->GetCAIMovement();

		if (!pMov)
			return NAVINT_NONE;
		else
			return (pMov->GetIntention()->eNavIntentions);
	}
	return NAVINT_NONE;
}			 

//------------------------------------------------------------------------------------------
// SetDebugDiveCone
//------------------------------------------------------------------------------------------
void CAINavigationSystemMan::SetDebugDiveCone( float fLength, float fHalfRadius )
{
	float fL = fLength < 0.0f ? 0.0f : fLength;
	float fHR= fHalfRadius < 0.0f ? 0.0f : fHalfRadius;

	DEBUG_PRINT_NAVSYSMAN(( "SetDebugDiveCone: ConeLength [%.3f] - ConeHalfRadius [%.3f]", fL, fHR ));
	m_obAIDivingMan.SetDebugDiveCone (fL, fHR);
}

//

void CAINavigationSystemMan::TestAIVolumes(void)
{
	static const CPoint DBG_Y_OFFSET(0,0.5f,0);

	static CEntity* pPlayer = CEntityManager::Get().GetPlayer();

	if (pPlayer == 0)
		return;

	// Get the Heading and Side Dir
	CDirection BotHeading		= pPlayer->GetMatrix().GetZAxis();
	CDirection BotSideDir		= pPlayer->GetMatrix().GetXAxis();
	CDirection BotSideLeftDir	= BotHeading+BotSideDir;
	CDirection BotSideRightDir	= BotHeading-BotSideDir;

	static const float fDBLength =  2.0f;

	CPoint pt3DCentre		= pPlayer->GetPosition()+DBG_Y_OFFSET;
	CPoint pt3DStraight		= pt3DCentre+CPoint(BotHeading)*fDBLength;
	CPoint pt3DSideLeft		= pt3DCentre+CPoint(BotSideLeftDir) *(1.5f);
	CPoint pt3DSideRight	= pt3DCentre+CPoint(BotSideRightDir)*(1.5f);

	// Render Stright Detection Line
#ifndef _GOLD_MASTER
	g_VisualDebug->RenderLine(pt3DCentre, pt3DStraight, DC_WHITE);
#endif

	// Is there a wall Straight

	CDirection WallAvoidanceDir(CONSTRUCT_CLEAR);
	CDirection WallNormal(CONSTRUCT_CLEAR);
	CDirection WallNormalLeft(CONSTRUCT_CLEAR);
	CPoint		p2D(CONSTRUCT_CLEAR);

	bool bWallOnTheLeft		= false;
	bool bWallOnTheRight	= false;

	if ( m_obAIWorldMan.HasLineOfSight(pt3DCentre, pt3DStraight, 0.5f) )
	{
#ifndef _GOLD_MASTER
		g_VisualDebug->RenderLine(pt3DCentre, pt3DStraight, DC_CYAN);
#endif
		return;
	}

	if ( m_obAIWorldMan.IntersectsAIVolume(pt3DCentre, pt3DStraight, &p2D, &WallNormal ) )
	{
		// Turn in the AI's Side direction
		//WallAvoidanceDir = BotSideDir*WallNormal.Dot(BotSideDir); // !!! - Correction (Dario) it should be BotSideDir*(sign(WallNormal.Dot(BotSideDir))*WallNormal.Dot(BotHeading))
		float fClockWise = WallNormal.Dot(BotSideDir) > 0.f ? 1.0f : -1.0f;
		WallAvoidanceDir = BotSideDir*fClockWise;

#ifndef _GOLD_MASTER
		g_VisualDebug->RenderLine(pt3DCentre, pt3DStraight, DC_PURPLE);
		g_VisualDebug->RenderLine(pt3DCentre, CPoint(pt3DCentre+WallAvoidanceDir), DC_RED);
#endif

		return;
	}

	bWallOnTheLeft  = m_obAIWorldMan.IntersectsAIVolume(pt3DCentre, pt3DSideLeft,  &p2D, &WallNormalLeft );
	bWallOnTheRight = m_obAIWorldMan.IntersectsAIVolume(pt3DCentre, pt3DSideRight, &p2D, &WallNormalLeft );

	if ( !( bWallOnTheLeft || bWallOnTheRight ) )
	{
		// No side walls detected
		return;
	}
	if ( bWallOnTheLeft && bWallOnTheRight )
	{
		// Narrow path, don't move to the sides
		return;
	} 
	if ( bWallOnTheLeft )
	{
		// Turn Right
		WallAvoidanceDir = BotSideRightDir;
#ifndef _GOLD_MASTER
		g_VisualDebug->RenderLine(pt3DCentre, pt3DSideLeft, DC_PURPLE);
#endif
		return ;
	}
	if ( bWallOnTheRight )
	{
		// Turn Left	
		WallAvoidanceDir = BotSideLeftDir;
#ifndef _GOLD_MASTER
		g_VisualDebug->RenderLine(pt3DCentre, pt3DSideRight, DC_PURPLE);
#endif
		return;
	}
	return; // Not needed but required by the compiler

}

//------------------------------------------------------------------------------------------
// MovingVolleyDebugRender
//------------------------------------------------------------------------------------------
void CAINavigationSystemMan::MovingVolleyDebugRender(void)
{
#ifndef _GOLD_MASTER
	unsigned int sz = m_sMovingVolley.vectorVolleyAIs.size();
	float fX = 20.0f, fY = 50.0f;
	char state[32];
	strcpy(state,	(m_sMovingVolley.eVolleyStatus == VOLLEY_FREE)	? "VOLLEY_FREE" :
					(m_sMovingVolley.eVolleyStatus == VOLLEY_READY)	? "VOLLEY_READY" :
					(m_sMovingVolley.eVolleyStatus == VOLLEY_AIM)	? "VOLLEY_AIM" :
					(m_sMovingVolley.eVolleyStatus == VOLLEY_FIRE)	? "VOLLEY_FIRE" : "UNKNOWN!!!!"
			);
					
	g_VisualDebug->Printf2D(fX,fY, DC_GREEN, 0, "_____LIST OF MOVING VOLLEY AIs____"); fY += 20.0f;
	g_VisualDebug->Printf2D(fX,fY, DC_YELLOW, 0, "State [%s] - Participants: [%d]",state, sz ); fY += 20.0f;
	
	if (!m_sMovingVolley.pCommander)
	{
		g_VisualDebug->Printf2D(fX,fY, DC_RED, 0, "There is NO-COMMANDER!"); fY += 20.0f;
	}

	for (unsigned int i = 0; i < sz; i++)
	{
		CEntity* pAI = m_sMovingVolley.vectorVolleyAIs[i].pAI;
		E_MOVING_VOLLEY_STATUS eAIStatus = m_sMovingVolley.vectorVolleyAIs[i].eVolleyStatus;
		
		// Highlight Participants

		char stateAI[32];
			strcpy(stateAI,	(eAIStatus == VOLLEY_FREE)	? "VOLLEY_FREE" :
					(eAIStatus == VOLLEY_READY)	? "VOLLEY_READY" :
					(eAIStatus == VOLLEY_AIM)	? "VOLLEY_AIM" :
					(eAIStatus == VOLLEY_FIRE)	? "VOLLEY_FIRE" : "UNKNOWN!!!!"
			);

		CMatrix ArcMatrix(CONSTRUCT_IDENTITY);
		ArcMatrix.SetTranslation(pAI->GetPosition());
		
		if (m_sMovingVolley.pCommander && m_sMovingVolley.pCommander == pAI)
		{
			g_VisualDebug->Printf2D(fX,fY, DC_RED, 0, "Commander [%s] - %s", ntStr::GetString(pAI->GetName()), stateAI); fY += 20.0f;
			g_VisualDebug->Printf3D(pAI->GetPosition() + CPoint(0,1.7f,0) ,DC_RED,0,"COMMANDER[%s]", ntStr::GetString(pAI->GetName()));
			g_VisualDebug->RenderCube(ArcMatrix, DC_BLUE, 4096);
		}
		else
		{
			g_VisualDebug->Printf2D(fX,fY, DC_WHITE, 0, "Soldier [%s] - %s", ntStr::GetString(pAI->GetName()), stateAI); fY += 20.0f;
			g_VisualDebug->Printf3D(pAI->GetPosition() + CPoint(0,1.7f,0) ,DC_YELLOW,0,"SOLDIER[%s]", ntStr::GetString(pAI->GetName()));
			g_VisualDebug->RenderCube(ArcMatrix, DC_WHITE, 4096);
		}
	}
#endif
}

//------------------------------------------------------------------------------------------
// MovingVolley_AddAI
//------------------------------------------------------------------------------------------
void CAINavigationSystemMan::MovingVolley_AddAI ( CEntity* pEnt )
{
	if (!pEnt) 
		return;

	SMovingVolleyAIData obData(pEnt);
	m_sMovingVolley.vectorVolleyAIs.push_back(obData);

}

//------------------------------------------------------------------------------------------
// MovingVolley_RemoveAI
//------------------------------------------------------------------------------------------
void CAINavigationSystemMan::MovingVolley_RemoveAI ( CEntity* pEnt )
{
	if (!pEnt || m_sMovingVolley.vectorVolleyAIs.empty() ) 
		return;

	ntstd::Vector<SMovingVolleyAIData,Mem::MC_AI>::iterator obIt		= m_sMovingVolley.vectorVolleyAIs.begin();
	ntstd::Vector<SMovingVolleyAIData,Mem::MC_AI>::iterator obEndIt	= m_sMovingVolley.vectorVolleyAIs.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		CEntity* pAI = (*obIt).pAI;
		if (pAI == pEnt)
		{
			m_sMovingVolley.vectorVolleyAIs.erase(obIt);
			return;
		}
	}
}

//------------------------------------------------------------------------------------------
// MovingVolley_SetVolleyStatus
//------------------------------------------------------------------------------------------
void CAINavigationSystemMan::MovingVolley_SetVolleyStatus( CEntity* pEnt, E_MOVING_VOLLEY_STATUS e )
{
	if (!pEnt) 
		return;

	unsigned int sz = m_sMovingVolley.vectorVolleyAIs.size();
	for (unsigned int i = 0; i < sz; i++)
	{
		CEntity* pAI = m_sMovingVolley.vectorVolleyAIs[i].pAI;
		if (pAI == pEnt)
			m_sMovingVolley.vectorVolleyAIs[i].eVolleyStatus = e;
	}
}






