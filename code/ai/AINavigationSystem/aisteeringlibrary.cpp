//! -------------------------------------------
//! aisteeringlibrary.cpp
//!
//! AI Steering Library
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//! Date	04/05/06 - Creation
//!--------------------------------------------

#include "aisteeringlibrary.h"
#include "ainavigsystemman.h"
#include "game/aicomponent.h"
#include "aipatrolgraph.h"
#include "editable/enums_ai.h"
#include "game/entityinfo.h"
// For Entity Querying
#include "game/entitymanager.h"
#include "game/query.h"
#include "game/renderablecomponent.h"
#include "game/movementcontrollerinterface.h"
#include "game/attacks.h"
// For Debugging
#include "core/visualdebugger.h"

// Macros for Readability

#define IS_ACTIVE(x)	( x & eNavigFlag )
#define STEER_WIGHT(x)	(*(pfSteeringWeights + x))
#define NORMALISE(x)	if (x.LengthSquared() > 0.99f) x.Normalise();
#define COLLISION_Y_OFFSET (0.5f)
#define DBG_Y_OFFSET CPoint(0,COLLISION_Y_OFFSET,0)

//! ------------------------------------------------------------------------------------------
//! Constructor, et al.
//! ------------------------------------------------------------------------------------------

CAISteeringLibrary::~CAISteeringLibrary()
{
	FreeData();
}


//! ---------------------------------------------------------------------------------------------------------------------------------
//!														Management and Initialization
//! ---------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// FreeData
//------------------------------------------------------------------------------------------
void CAISteeringLibrary::FreeData ( void )
{
	// Free the dynamically allocated SRE Elements

	for (unsigned int i = 0; i < m_vSRE.size(); i++)
	{
			NT_DELETE_CHUNK( Mem::MC_AI, m_vSRE[i]);
	}
	for (unsigned int i = 0; i < m_vBRE.size(); i++)
	{
			NT_DELETE_CHUNK( Mem::MC_AI, m_vBRE[i]);
	}
	
	// Empty lists;
	m_vSRE.clear();
	m_vBRE.clear();
	m_vAIBots.clear();
}

//------------------------------------------------------------------------------------------
// GenerateAIBotList
//------------------------------------------------------------------------------------------
void CAISteeringLibrary::GenerateAIBotList()
{
	// Empty AI Bots Tables
	m_vAIBots.clear();

	// Get all the AI characters (AI Bots)
	CEntityQuery obAIBotList;
	CEntityManager::Get().FindEntitiesByType(obAIBotList, CEntity::EntType_AI);

	QueryResultsContainerType::iterator obEnd	= obAIBotList.GetResults().end();
	QueryResultsContainerType::iterator obIt	= obAIBotList.GetResults().begin();
	for ( ; obIt != obEnd; ++obIt )
	{
		AddAIBot( *obIt );
	}

	ntPrintf("\n --- AI Bots ---\n");

	ntPrintf( "AI Bots added to repulsion manager: %d\n", m_vAIBots.size() );
	ntPrintf("\n");
	for (unsigned int i = 0; i < m_vAIBots.size(); i++)
	{
		ntPrintf("%d - AI Bot: [%s]\n",i,m_vAIBots[i]->GetName().c_str());
	}
}

//------------------------------------------------------------------------------------------
// AddAIBot
//------------------------------------------------------------------------------------------
void CAISteeringLibrary::AddAIBot	( CEntity* pEnt )
{
	if (!pEnt) { ntAssert_p(0,("AddAIBot -> NULL Entity passed\n")); return; }

	m_vAIBots.push_back(pEnt);
}

//------------------------------------------------------------------------------------------
// GenerateRepulsionTables
//------------------------------------------------------------------------------------------
void CAISteeringLibrary::GenerateDynObstacleList()
{
	// Empty Repulsion Tables
	FreeData();

	// Create an entity query for the types of objects we're after.
	// For now this is heavy interactable objects.
	CEntityQuery	obAvoidQuery;
	CEQCShouldAvoid	obClause;

	obAvoidQuery.AddClause( obClause );

	// run the query and add the entities it returns to our list of repellers 
	CEntityManager::Get().FindEntitiesByType( obAvoidQuery, CEntity::EntType_AllButStatic );

	//int count   = 0;
	QueryResultsContainerType::iterator obEnd = obAvoidQuery.GetResults().end();
	QueryResultsContainerType::iterator obIt = obAvoidQuery.GetResults().begin();
	for ( ; obIt != obEnd; ++obIt )
	{
		CEntity* pEnt = *obIt;
		float fRadius = 1.0f;

		LuaAttributeTable* pobAtt	= pEnt->GetAttributeTable();
		NinjaLua::LuaObject obObj	= pobAtt->GetAttribute("SharedAttributes");
		fRadius = obObj["AIAvoidRadius"].GetFloat();

		AddSphericEntity( *obIt, fRadius );
	}

	ntPrintf("\n --- OBSTACLES ---\n");

	ntPrintf( "Entities [Spheric] added to repulsion manager %d\n", m_vSRE.size() );
	ntPrintf( "Entities [Boxed]   added to repulsion manager %d\n", m_vBRE.size() );
	ntPrintf("\n");
	for (unsigned int i = 0; i < m_vSRE.size(); i++)
	{
		ntPrintf("Spheric Obstacle [%d] : %s - R: %.2f - X:%.2f-Y:%.2f-Z:%.2f\n",i,
																				   m_vSRE[i]->pEnt->GetName().c_str(),
																				   m_vSRE[i]->fRadius,
																				   m_vSRE[i]->pEnt->GetPosition().X(),
																				   m_vSRE[i]->pEnt->GetPosition().Y(),
																				   m_vSRE[i]->pEnt->GetPosition().Z());
	}
}

//------------------------------------------------------------------------------------------
// AddSphericEntity
//------------------------------------------------------------------------------------------
void CAISteeringLibrary::AddSphericEntity	( CEntity* pEnt, float fRadius)
{
	if (!pEnt) { ntAssert_p(0,("AddSphericEntity -> NULL Entity passed\n")); return; }

	SSphericRepulsiveEnts* SRE = NT_NEW_CHUNK( Mem::MC_AI ) SSphericRepulsiveEnts(pEnt, fRadius);
	m_vSRE.push_back(SRE);
}

//------------------------------------------------------------------------------------------
// RemoveEntity
//------------------------------------------------------------------------------------------
void CAISteeringLibrary::RemoveEntity ( CEntity* pEnt )
{
	SSphericRepulsiveEntsVector::iterator obItSRE = m_vSRE.begin();
	for (unsigned int i = 0; i < m_vSRE.size(); i++)
	{
		if ( m_vSRE[i]->pEnt == pEnt )
		{
			m_vSRE.erase(obItSRE);
			return;
		}
		++obItSRE;
	}
	SBoxedRepulsiveEntsVector::iterator obItBRE = m_vBRE.begin();
	for (unsigned int i = 0; i < m_vBRE.size(); i++)
	{
		if ( m_vBRE[i]->pEnt == pEnt )
		{
			m_vBRE.erase(obItBRE);
			return;
		}
		++obItBRE;
	}

	ntAssert_p(0,("RemoveEntity -> Entity not found in the repulsion list\n"));
}

//! ---------------------------------------------------------------------------------------------------------------------------------
//!														Tool Box
//! ---------------------------------------------------------------------------------------------------------------------------------
CDirection CAISteeringLibrary::Rotate(const CDirection& obDir, float fAngle_Rad)
{
	float fSin,fCos;
	CMaths::SinCos( fAngle_Rad, fSin, fCos );
	CDirection Rotation = CDirection (obDir.X()*fCos - obDir.Z()*fSin, obDir.Y(), obDir.X()*fSin + obDir.Z()*fCos);
	return Rotation;
}

//! ---------------------------------------------------------------------------------------------------------------------------------
//!														Steering Behaviours
//! ---------------------------------------------------------------------------------------------------------------------------------

//! -------------------------------------------
//! ArriveSolo
//! -------------------------------------------
CDirection CAISteeringLibrary::Arrive ( CAIMovement* pMov, const CPoint& ArrivePos )
{
	CDirection DesiredVelocity (CONSTRUCT_CLEAR);
	
	CDirection Line ( ArrivePos - pMov->GetPosition() );

	if ( Line.LengthSquared() > pMov->GetFollowEntityRadiusSQR() )
	{
		Line.Normalise();
		DesiredVelocity =  Line * pMov->GetMaxSpeed();
	}

	return ( DesiredVelocity );
}

//! -------------------------------------------
//! ArriveAtPoint
//! -------------------------------------------
CDirection CAISteeringLibrary::ArriveAtPoint ( CAIMovement* pMov )
{
	CDirection DesiredVelocity (CONSTRUCT_CLEAR);
	
	CDirection Line ( pMov->GetDestinationPos() - pMov->GetPosition() );

	if ( Line.LengthSquared() > pMov->GetDestinationRadiusSQR() )
	{
		Line.Normalise();
		DesiredVelocity =  Line * pMov->GetMaxSpeed();
		pMov->GetSMovCompleted()->bMoveToSelectedPointCompleted = false;
	}
	else
	{
		pMov->GetSMovCompleted()->bMoveToSelectedPointCompleted = true;
	}
	return ( DesiredVelocity );
}

//! -------------------------------------------
//! SeekSolo
//! -------------------------------------------
CDirection CAISteeringLibrary::Seek ( const CPoint& obMyPos, const CPoint& TargetPos, const float fMaxSpeed )
{
	CDirection DesiredVelocity =  CDirection( TargetPos - obMyPos ) * fMaxSpeed;

	return ( DesiredVelocity );
}

//! -------------------------------------------
//! MovoToPoint
//! -------------------------------------------
CDirection CAISteeringLibrary::StrafeToCombatPoint ( CAIMovement* pMov )
{
	CDirection DesiredVelocity (CONSTRUCT_CLEAR);
	
	// Vector to the attack point
	CDirection Line = pMov->GetAttackPoint() ^ pMov->GetPosition();

	if ( Line.LengthSquared() > pMov->GetAttackPointRadiusSQR() )
	{
		CDirection Line2Enemy ( pMov->GetPosition() - pMov->GetEntityToAttack()->GetPosition());
		CDirection LinePoint2Enemy ( pMov->GetAttackPoint() - pMov->GetEntityToAttack()->GetPosition());

		// Find the angle between the two vectors. 
		float fAngle = MovementControllerUtilities::RotationAboutY( Line2Enemy, LinePoint2Enemy );
		static const float CONSTANT_DIV = 10.0f;

		// Create a mini angle about which to rotate the enemy about
		fAngle /= CONSTANT_DIV;

		// Rotate the Line to enemy about the entity to attack
		DesiredVelocity = Line2Enemy * CMatrix( CDirection(0.0f, 1.0f, 0.0f), fAngle );

		// Create a unit vector with no magnitude. 
		DesiredVelocity.Normalise();

		// Lerp the radius from the enemies current to the target point. 
		float fLen1 = Line2Enemy.Length();
		float fLen2 = LinePoint2Enemy.Length();
		float fLen	= ((fLen2 - fLen1) / CONSTANT_DIV) + fLen1;

		// Set the radius in the direction vector
		DesiredVelocity *= fLen;

		// Define the vector in the correct space, mapping it from the target entity to a velocity vector of the enemy
		DesiredVelocity		-= Line2Enemy;

		// Normalise the directional vector with the required velocity
		DesiredVelocity.Normalise();
		DesiredVelocity		*= pMov->GetMaxSpeed();

		// Calculate facing direction
		pMov->SetMovingWhileFacingTgt(true);
		pMov->SetFacingAction(-Line2Enemy);
		pMov->SetMoveToCombatPointCompleted(false);
	}
	else
	{
		pMov->SetMoveToCombatPointCompleted(true);
		pMov->SetMovingWhileFacingTgt(false);
		pMov->DeactivateFlag(NF_TO_COMBAT_POINT);
	}
	return ( DesiredVelocity );
}

//! -------------------------------------------
//! StrafeToFormationPoint
//! -------------------------------------------
CDirection CAISteeringLibrary::StrafeToFormationPoint ( CAIMovement* pMov )
{
	CDirection DesiredVelocity (CONSTRUCT_CLEAR);
	
	CDirection Line2Point ( pMov->GetAttackPoint() - pMov->GetPosition() );
	CDirection Line2Enemy ( pMov->GetPosition() - pMov->GetEntityToAttack()->GetPosition());
	CDirection LinePoint2Enemy ( pMov->GetAttackPoint() - pMov->GetEntityToAttack()->GetPosition());

	float fDistanceSquaredToPoint = Line2Point.LengthSquared();

	if (fDistanceSquaredToPoint > pMov->GetAttackPointRadiusSQR())
	{
		// Find the angle between the two vectors. 
		float fAngle = MovementControllerUtilities::RotationAboutY( Line2Enemy, LinePoint2Enemy );
		static const float CONSTANT_DIV = 10.0f;

		// Create a mini angle about which to rotate the enemy about
		fAngle /= CONSTANT_DIV;

		// Rotate the Line to enemy about the entity to attack
		DesiredVelocity = Line2Enemy * CMatrix( CDirection(0.0f, 1.0f, 0.0f), fAngle );
		
		// Create a unit vector with no magnitude. 
		DesiredVelocity.Normalise();

		// Lerp the radius from the enemies current to the target point. 
		float fLen1 = Line2Enemy.Length();
		float fLen2 = LinePoint2Enemy.Length();
		float fLen = ((fLen2 - fLen1) / CONSTANT_DIV) + fLen1;

		// Set the radius in the direction vector
		DesiredVelocity *= fLen;

		// Define the vector in the correct space, mapping it from the target entity to a velocity vector of the enemy
		DesiredVelocity -= Line2Enemy;
			
		// Normalise the directional vector with the required velocity
		DesiredVelocity.Normalise();

		float fSpeed = pMov->GetMaxSpeed();

		// Slow down as approaching slot position.
#define SLOWDOWN_DISTANCE		2.0f

		if (fDistanceSquaredToPoint < (SLOWDOWN_DISTANCE * SLOWDOWN_DISTANCE))
		{
			fSpeed *= CMaths::SmoothStep(sqrt(fDistanceSquaredToPoint) / SLOWDOWN_DISTANCE);			
		}

		DesiredVelocity *= clamp( fSpeed, 0.3f, pMov->GetMaxSpeed() );

		// Calculate facing direction
		pMov->SetMovingWhileFacingTgt(true);
		pMov->SetFacingAction(-Line2Enemy);
		pMov->SetMoveToCombatPointCompleted(false);
	}
	else
	{
		pMov->SetMoveToCombatPointCompleted(true);
		pMov->SetMovingWhileFacingTgt(false);
		pMov->DeactivateFlag(NF_TO_COMBAT_POINT);
	}


	return DesiredVelocity;
}

//! -------------------------------------------
//! ChaseTarget
//! -------------------------------------------
CDirection CAISteeringLibrary::ChaseTarget (  CAIMovement* pMov )
{
	// Move to the last known player position

	CDirection DesiredVelocity (CONSTRUCT_CLEAR);
	
	CDirection Line ( pMov->GetCAIComp()->GetAIVision()->GetLastKnowEnemyPos() - pMov->GetPosition() );

	if ( Line.LengthSquared() > pMov->GetAttackRangeSQR() )
	{
		Line.Normalise();
		DesiredVelocity =  Line * pMov->GetMaxSpeed();
		pMov->GetSMovCompleted()->bChaseTargetCompleted = false;
	}
	else
	{
		pMov->GetSMovCompleted()->bChaseTargetCompleted = true;
	}

	return ( DesiredVelocity );
}

//! -------------------------------------------
//! ChaseTarget
//! -------------------------------------------
CDirection CAISteeringLibrary::ChaseMovingEntity (  CAIMovement* pMov )
{
	// Move to the last known player position

	CDirection DesiredVelocity (CONSTRUCT_CLEAR);
	
	CDirection Line ( pMov->GetEntityToAttack()->GetPosition() - pMov->GetPosition() );

	if ( Line.LengthSquared() > pMov->GetFollowEntityRadiusSQR() )
	{
		Line.Normalise();
		DesiredVelocity =  Line * pMov->GetMaxSpeed();
		pMov->GetSMovCompleted()->bChaseTargetCompleted = false;
	}
	else
	{
		pMov->GetSMovCompleted()->bChaseTargetCompleted = true;
	}

	return ( DesiredVelocity );
}

////! -------------------------------------------
////! FollowLeader
////! -------------------------------------------
//CDirection CAISteeringLibrary::FollowLeader (  CAIMovement* pMov )
//{
//	// Move to the last known player position
//	CDirection DesiredVelocity (CONSTRUCT_CLEAR);
//	
//	CDirection Line ( pMov->GetLeader()->GetPosition() - pMov->GetPosition() );
//
//	bool bLeaderHasArrived = pMov->GetLeader()->GetAIComponent()->GetCAIMovement()->IsFollowLeaderCompleted();
//
//	// Is it over?
//	if ( bLeaderHasArrived && pMov->IsFollowLeaderCompleted() ) return (DesiredVelocity);
//	
//	// Keep on following
//	if ( Line.LengthSquared() > pMov->GetFollowLeaderRadiusSQR() )
//	{
//		Line.Normalise();
//		DesiredVelocity =  Line * pMov->GetMaxSpeed();
//		pMov->GetSMovCompleted()->bFollowLeaderCompleted = false;
//	}
//	else
//	{
//		pMov->GetSMovCompleted()->bFollowLeaderCompleted = bLeaderHasArrived? true : false;
//	}
//
//	return ( DesiredVelocity );
//}

//! -------------------------------------------
//! FollowPlayer
//! -------------------------------------------
CDirection CAISteeringLibrary::FollowEntity (  CAIMovement* pMov )
{
	// Move to the last known player position
	CDirection FleeVelocity (CONSTRUCT_CLEAR);
	CDirection DesiredVelocity (CONSTRUCT_CLEAR);
	CDirection FlockingVelocity (CONSTRUCT_CLEAR);
	CDirection AIAvoidanceVelocity (CONSTRUCT_CLEAR);
	CDirection WallAvoidanceVelocity (CONSTRUCT_CLEAR);
	
	bool bFleeing = false;
	bool bNonZeroWall = false;

	CPoint LeaderEntityPos = pMov->GetEntityToFollow()->GetPosition(); 
	LeaderEntityPos.Y() = pMov->GetPosition().Y();

	CDirection Line ( LeaderEntityPos - pMov->GetPosition() );
	float fLineDistSQR = Line.LengthSquared();

	pMov->SetMovingWhileFacingTgt(false);

#ifndef _GOLD_MASTER
	if (CAINavigationSystemMan::Get().m_bRenderAIAvoidance)
		{
			// Render Follow Entity Radius
			CMatrix ArcMatrix = pMov->GetEntityToFollow()->GetMatrix();
			CPoint ArcTr = ArcMatrix.GetTranslation();
			ArcMatrix.SetTranslation(ArcTr+DBG_Y_OFFSET);
			g_VisualDebug->RenderArc(ArcMatrix, sqrt(pMov->GetFollowEntityRadiusSQR()), TWO_PI, DC_WHITE);
		}
#endif

	
	//if ( (fLineDistSQR > ( pMov->GetFollowEntityRadiusSQR() + 9.0f)) || 
	//	((fLineDistSQR > pMov->GetFollowEntityRadiusSQR() ) && !pMov->IsFollowEntityCompleted() )
	//	)
	if ( fLineDistSQR >  pMov->GetFollowEntityRadiusSQR() )		
	{
		// ===================================================
		// ======== OUT OF THE FOLLOW ENTITY RADIUS ==========
		// ===================================================

		float fDelta = sqrt(fLineDistSQR-pMov->GetFollowEntityRadiusSQR())/2;

		// Calculate Follow Entity
		Line.Normalise();
		DesiredVelocity = pMov->GetSteeringWeight(NWI_FLOCK) * Line; // * pMov->GetMaxSpeed();
		pMov->SetFollowEntityCompleted(false);

		// Calculate Wall Avoidance

		WallAvoidanceVelocity = pMov->GetSteeringWeight(NWI_S_OBSTACLE)*CalculateWallAvoidance(pMov,&bNonZeroWall);
	//	if (IsAIAvoidanceTriggered()) return (WallAvoidanceVelocity);

		// Calculate FLocking

	//	if (fDelta>1.0f)
		FlockingVelocity = pMov->GetSteeringWeight(NWI_FLOCK)*CalculateFlocking(pMov);



		// Calculate Total Action

		DesiredVelocity +=(FlockingVelocity+WallAvoidanceVelocity);

		if (DesiredVelocity.LengthSquared()>0.99) DesiredVelocity.Normalise();

		
		if (fDelta<0.99f) DesiredVelocity*=fDelta;


		pMov->GetCAIComp()->ActivateController(CAIComponent::MC_WALKING);
	}
	else
	{
		// AI has reached the entitie's follow radius
		pMov->SetFollowEntityCompleted(true);

		// Calculate AI Avoidance

		AIAvoidanceVelocity = pMov->GetSteeringWeight(NWI_AI_OBSTACLE)*CalculateAIAvoidance(pMov);
        
		// Calculate Flee Entity
		if ( fLineDistSQR < pMov->GetFleeDistSQR() )
		{
			FleeVelocity = pMov->GetSteeringWeight(NWI_FLEE)* CDirection( -Line ) * pMov->GetMaxSpeed();
			bFleeing = true;
		}	

		DesiredVelocity = AIAvoidanceVelocity + FleeVelocity;

		if (IsAIAvoidanceTriggered() || bFleeing)
		{
			if (DesiredVelocity.LengthSquared()>0.99) DesiredVelocity.Normalise();
			
			pMov->SetActionStyle(AS_NORMAL);
			pMov->SetFacingAction(Line);
			pMov->SetMovingWhileFacingTgt(true);

			pMov->GetCAIComp()->ActivateController(CAIComponent::MC_STRAFING);
		}
	}

	return ( DesiredVelocity );
}

//! -------------------------------------------
//! FleeSolo
//! -------------------------------------------
CDirection CAISteeringLibrary::Flee ( CAIMovement* pMov, const CPoint& obFleePoint  )
{
	CDirection Line ( pMov->GetPosition() - obFleePoint );

	if ( Line.LengthSquared() < pMov->GetFleeDistSQR() )
	{
		CDirection DesiredVelocity =  CDirection( pMov->GetPosition() - obFleePoint ) * pMov->GetMaxSpeed();
		return ( DesiredVelocity );
	}
	return CDirection(CONSTRUCT_CLEAR);
}
//! -------------------------------------------
//! FollowPath
//! -------------------------------------------
CDirection CAISteeringLibrary::FollowPath ( CAIMovement* pMov )
{
	CDirection SteeringDir(CONSTRUCT_CLEAR);
	CAINavigPath* pPath = pMov->GetPathContainer();

	if (!pPath->GetCurrentNode())
	{
		// Path is either finished or empty
		pMov->SetFollowPathCompleted(true);
		pMov->DeactivateMotion(); // pMov->DeActivateFlag(NF_FOLLOW_PATH);
		return SteeringDir;
	}

	CDirection Distance2Target(pPath->GetCurrentNode()->GetPos() - pMov->GetPosition() );

	// Check if the node requires to reduce the wall detection radius
	//if ( pPath->GetCurrentNode()->HasDisableWallDetFlag() && !pPath->IsStartNode())
	//{
	//	// Is the AI within range?
	//	if ( ( Distance2Target.LengthSquared() < pPath->GetCurrentNode()->GetNoWallDetectRadiusSQR() ) )
	//	{
	//		// Have the steering flags already set?
	//		if ( !pMov->HasStoredSteeringFlags() )
	//      {
	//			//pMov->StoreSteeringFlags();
	//			pMov->DeactivateFlag(NF_S_OBSTACLE);
	//		}
	//	}
	//}

	if ( Distance2Target.LengthSquared() < pPath->GetCurrentNode()->GetRadiusSQR() )
	{
		// Target the next node
		pPath->PointToNextNode();

		if (!pPath->GetCurrentNode())
		{
			// We reached the last node
			pMov->SetFollowPathCompleted(true);
			pMov->DeactivateMotion();// pMov->DeActivateFlag(NF_FOLLOW_PATH);
			return SteeringDir;
		}
	}


	SteeringDir = Seek(pMov->GetPosition(), pPath->GetCurrentNode()->GetPos(), pMov->GetMaxSpeed());
	if (SteeringDir.LengthSquared() > 1.0f) SteeringDir.Normalise();

	return ( SteeringDir );
}

//! -------------------------------------------
//! FollowPatrolPath
//! -------------------------------------------
CDirection CAISteeringLibrary::FollowPatrolPath ( CAIMovement* pMov )
{
	CDirection SteeringDir(CONSTRUCT_CLEAR);

	CAIPatrolGraph* pPG = pMov->GetPatrolGraph();
	
	if (!pPG) 
	{
		// This AI doesn't have a proper Patrol Path
		pMov->DeactivateMotion();
		return SteeringDir;
	}

	CEntity*		pAI = pMov->GetParent();
	CAINavigNode*	pNN = pMov->GetPatrolGraph()->GetCurrentNode(pAI);

	CDirection Distance2Target( pNN->GetPos() - pAI->GetPosition() );

	if ( Distance2Target.LengthSquared() < pNN->GetRadiusSQR() )
	{
		// Target the next node
		pPG->PointToNextNode(pAI);
		pNN = pMov->GetPatrolGraph()->GetCurrentNode(pAI);
	}

	SteeringDir = Seek(pAI->GetPosition(), pNN->GetPos(), pMov->GetMaxSpeed());
	if (SteeringDir.LengthSquared() > 1.0f) SteeringDir.Normalise();

	return ( SteeringDir );
}


//! -------------------------------------------
//! FollowPathWithCover
//! -------------------------------------------
CDirection CAISteeringLibrary::FollowPathWithCover ( CAIMovement* pMov, bool* bActive )
{
	CDirection SteeringDir(CONSTRUCT_CLEAR);
	*bActive = false;
	
	// Early exit if moving to cover
	if (pMov->IsMovingToCover() || pMov->IsUsingObjectInPath()) 
		return SteeringDir;

	// Otherwise walk the path
	CAINavigPath* pPath = pMov->GetPathContainer();

	if (!pPath->GetCurrentNode())
	{
		// Path is either finished or empty
		pMov->SetFollowPathCompleted(true);
		pMov->DeactivateMotion(); // pMov->DeActivateFlag(NF_FOLLOW_PATH);
		return SteeringDir;
	}

	// Cache Currentt Node's Pos
	CPoint obCurrentNodePos = pPath->GetCurrentNode()->GetPos();

	//  ================================================
	//  ============ FIRST FRAME CHECKING  =============
	//  ================================================ 
	
	if (pPath->IsFirstFollowPathFrame())
	{
		pPath->SetFirstFollowPathFrame(false);
		CDirection LineNode2AI( pMov->GetPosition() - obCurrentNodePos );
		LineNode2AI.Normalise();
		float fRandAngle = grandf( TWO_PI );

		CAINavigNode* pNextNode = pPath->PeekNextNode();
		if (pNextNode)
		{
			CDirection LineNode2NextNode( pNextNode->GetPos() - obCurrentNodePos );
			CDirection LeftSide = GetPerpendicular(LineNode2NextNode);
			float fDot = LeftSide.Dot(LineNode2AI);

			fRandAngle = fDot < 0.0f ? -grandf( HALF_PI ) : grandf( HALF_PI );
		}

		float fNodeWidthPercentage = (pNextNode && pNextNode->HasWidthPercentage()) ? pNextNode->GetWidthPercentage() : pMov->GetFollowPathRadiusPercentage();
		CDirection pointDir = Rotate(LineNode2AI,fRandAngle);
		CPoint obPointWithinNode = CPoint(pointDir*pPath->GetCurrentNode()->GetRadius()*fNodeWidthPercentage + obCurrentNodePos);
		pPath->SetPointWithinCurrentNode(obPointWithinNode);

		return SteeringDir;
	}

	//  ================================================
	//  =========  CHECK DISTANCE TO NODE  =============
	//  ================================================
	
	CDirection Distance2Target(obCurrentNodePos - pMov->GetPosition() );

	if ( Distance2Target.LengthSquared() < pPath->GetCurrentNode()->GetRadiusSQR() )
	{
		//  ================================================
		//  ============ HAS USE OBJECT INFO ? =============
		//  ================================================

		// --- Try First Arrow-Based UseObject
		CAINavigNode* pNextNode = pPath->PeekNextNode();
		CAINavigArrow* pArrow	= pPath->GetCurrentNode()->GetTargetArrow(pNextNode);
		if (pArrow)
		{
			CEntity* pObjectToUse	= pArrow->GetEntityToUse();
				
			if (pObjectToUse)
			{
				pMov->SetEntityToUseInPath(pObjectToUse);
				pMov->SetUsingObjectInPath(true);
				pPath->PointToNextNode();
				return ( SteeringDir );
			}
		}

		// --- Try Next Node-Based UseObject
		CEntity* pObjectToUse = pPath->GetCurrentNode()->GetEntityToUse();
		if (pObjectToUse)
		{
			pMov->SetEntityToUseInPath(pObjectToUse);
			pMov->SetUsingObjectInPath(true);
			pPath->PointToNextNode();
			return ( SteeringDir );
		}
		
		//  ================================================
		//  ================ COVER POINTS ? ================
		//  ================================================

		CAINavigCoverPoint* pBookedCoverPoint	= pMov->GetClaimedCoverPoint();
		bool bNodeLinksWithBookedCoverPoint		= pBookedCoverPoint && pPath->GetCurrentNode()->IsLinkedToCoverPoint(pBookedCoverPoint);

		CAINavigCoverPoint* pCoverPoint = NULL;
		
		if ( !pBookedCoverPoint && CAINavigationSystemMan::Get().IsValidCoverTime() ) 
		{
			pCoverPoint = pPath->GetAvailableCoverPoint(pMov);
		}
		else if (	pBookedCoverPoint && 
					bNodeLinksWithBookedCoverPoint && 
					pBookedCoverPoint->IsCoverInEnemysDirection(pMov->GetEntityToAttack()->GetPosition())
				)
		{
			pCoverPoint = pBookedCoverPoint;
		}

					
		//CAINavigCoverPoint* pCoverPoint = ( pBookedCoverPoint && bNodeLinksWithBookedCoverPoint ) ? 
		//									pBookedCoverPoint : pPath->GetAvailableCoverPoint(pMov);
			
		if ( pCoverPoint )
		{
			// Set-Up flags
			
			//pCoverPoint->SetClaimer(NULL);
			pCoverPoint->SetAvailabe(false);
			
			// Start Going to cover
				
			pMov->SetGoingToCover(true);
			pMov->SetCoverPoint(pCoverPoint);
				
			// Disable the path following movement will be done in the behaviour
			SteeringDir = CDirection(pCoverPoint->GetPos() - pMov->GetPosition());
			SteeringDir.Normalise();
			SteeringDir *=pMov->GetMaxSpeed();

			return ( SteeringDir );
		}

		//  ================================================
		//  ============ TARGET THE NEXT NODE ==============
		//  ================================================
		
		if (pNextNode)
		{
			CDirection LineNode2AI( pMov->GetPosition() - pNextNode->GetPos() );
			LineNode2AI.Normalise();
			CDirection LineNode2NextNode( pNextNode->GetPos() - obCurrentNodePos );
			CDirection LeftSide = GetPerpendicular(LineNode2NextNode);
			float fDot = LeftSide.Dot(LineNode2AI);

			CPoint obPointWithinNode(CONSTRUCT_CLEAR);
			bool bFound = false;

			// Find a valid point (try 10 times)
			for (int i = 0; i < 10; i++)
			{
				float fRandAngle = fDot < 0.0f ? -grandf( HALF_PI ) : grandf( HALF_PI );
			
				float fNodeWidthPercentage = pNextNode->HasWidthPercentage() ? pNextNode->GetWidthPercentage() : pMov->GetFollowPathRadiusPercentage();
				CDirection pointDir = Rotate(LineNode2AI,fRandAngle);
				obPointWithinNode = CPoint(pointDir*pNextNode->GetRadius()*fNodeWidthPercentage + pNextNode->GetPos());
				if ( CAINavigationSystemMan::Get().HasLineOfSightExcludingGoAroundVolumes(pMov->GetPosition(), obPointWithinNode) )
				{
					bFound = true;
					break;
				}
			}
			if ( !bFound )
				obPointWithinNode = pNextNode->GetPos();

			pPath->SetPointWithinCurrentNode(obPointWithinNode);

			// Set the next node as target
			pPath->PointToNextNode();
		}
		else
		{
			// We reached the last node
			pMov->SetFollowPathCompleted(true);
			pMov->DeactivateMotion();// pMov->DeActivateFlag(NF_FOLLOW_PATH);
			return SteeringDir;
		}
	}
	//  ================================================
	//  =========  SET UP THE MOVING DIRECTION =========
	//  ================================================
	
	SteeringDir = Seek(pMov->GetPosition(), pPath->GetPointWithinCurrentNode(), pMov->GetMaxSpeed());
//	if (SteeringDir.LengthSquared() > 1.0f) SteeringDir.Normalise();
	SteeringDir.Normalise();
	*bActive = true;
	return ( SteeringDir );
}

//! -------------------------------------------
//! GoAroundVolume
//! -------------------------------------------
CDirection CAISteeringLibrary::GoAroundVolume ( CAIMovement* pMov )
{
	CDirection SteeringDir(CONSTRUCT_CLEAR);
	CAINavigPath* pPath = pMov->GetPathContainer();
	if (!pPath) 
		return SteeringDir;

	if (!pPath->GetCurrentNode())
	{
		// Path is either finished or empty
		pMov->SetGoAroundCompleted(true);
		pMov->DeactivateMotion(); // pMov->DeActivateFlag(NF_FOLLOW_PATH);
		pPath->clear();
		return SteeringDir;
	}

	// Since the Node is automatically generated, correct Y() coordinate
	CPoint obCurrentNodePos = pPath->GetCurrentNode()->GetPos();
	obCurrentNodePos.Y() = pMov->GetPosition().Y();

	CDirection Distance2Target(obCurrentNodePos - pMov->GetPosition() );

	if ( Distance2Target.LengthSquared() < pPath->GetCurrentNode()->GetRadiusSQR() )
	{
		// Target the next node
		pPath->PointToNextNode();

		if (!pPath->GetCurrentNode())
		{
			// We reached the last node
			pMov->SetGoAroundCompleted(true);
			pMov->DeactivateMotion();// pMov->DeActivateFlag(NF_FOLLOW_PATH);
			pPath->clear();
			return SteeringDir;
		}

		obCurrentNodePos = pPath->GetCurrentNode()->GetPos();
		obCurrentNodePos.Y() = pMov->GetPosition().Y();
	}

	SteeringDir = Seek(pMov->GetPosition(), obCurrentNodePos, pMov->GetMaxSpeed());
	if (SteeringDir.LengthSquared() > 1.0f) SteeringDir.Normalise();

	return ( SteeringDir );
	
}

//! ---------------------------------------------------------------------------------------------------------------------------------
//!														Obstacle Avoidance Routines
//! ---------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// CalculateWallAvoidance
//------------------------------------------------------------------------------------------
CDirection CAISteeringLibrary::CalculateWallAvoidance (CAIMovement* pMov, bool* bActive )
{
	CDirection	WallAvoidanceDir (CONSTRUCT_CLEAR);
	CPoint		IPoint(CONSTRUCT_CLEAR);
	*bActive = false;

	SetAIAvoidanceTriggered(false);

	if (!pMov)
	{
		return WallAvoidanceDir;
	}
	
	CDirection WallNormal		(CONSTRUCT_CLEAR);
	CDirection WallNormalLeft	(CONSTRUCT_CLEAR);
	CDirection WallNormalRight	(CONSTRUCT_CLEAR);

	// Get the Heading and Side Dir
	CDirection BotHeading		= pMov->GetParent()->GetMatrix().GetZAxis();
	CDirection BotSideDir		= pMov->GetParent()->GetMatrix().GetXAxis();
	CDirection BotSideLeftDir	= BotHeading+BotSideDir;
	CDirection BotSideRightDir	= BotHeading-BotSideDir;

	CPoint pt3DCentre			= pMov->GetParent()->GetPosition()+DBG_Y_OFFSET;


	// Calculate the Heading Detection Length

	float fDBLength = pMov->GetMinWallDetRadius();
	float fSideDetLength = fDBLength*0.7f;
	

	// ===========================================================================================
	// =================  BACK DETECTION LINES (FORMATION)  ======================================
	// ===========================================================================================		

	if (pMov->IsInFormationMovement() || pMov->IsInDynamicCoverMovement())
	{
		//float temp = pMov->GetMinWallDetRadius()/2;

		//fDBLength = temp + 
		//			( pMov->GetSpeedMagnitude() / pMov->GetMaxSpeed() ) * temp;

		fSideDetLength = pMov->IsInDynamicCoverMovement() ? fDBLength*0.4f : fSideDetLength; //pMov->GetSideDetLength();

		// Check the back of the AI

		CPoint pt3DBackStraight		= pt3DCentre-CPoint(BotHeading)*fDBLength;
		CPoint pt3DBackSideRight	= pt3DCentre-CPoint(BotSideLeftDir) *(fSideDetLength);
		CPoint pt3DBackSideLeft		= pt3DCentre-CPoint(BotSideRightDir)*(fSideDetLength);
		
		// Render Detection Lines
#ifndef _GOLD_MASTER
		if (CAINavigationSystemMan::Get().m_bRenderWallAvoidance) 
				{ 
					g_VisualDebug->RenderLine(pt3DCentre, pt3DBackStraight, DC_WHITE);
					g_VisualDebug->RenderLine(pt3DCentre, pt3DBackSideLeft, DC_WHITE);
					g_VisualDebug->RenderLine(pt3DCentre, pt3DBackSideRight, DC_WHITE);
				}
#endif		

		bool bWallOnTheBackLeft		= false;
		bool bWallOnTheBackRight	= false;
		
		bWallOnTheBackLeft	= CAINavigationSystemMan::Get().IntersectsAIVolume(pt3DCentre, pt3DBackSideLeft,  &IPoint, &WallNormalLeft );
		bWallOnTheBackRight	= CAINavigationSystemMan::Get().IntersectsAIVolume(pt3DCentre, pt3DBackSideRight, &IPoint, &WallNormalLeft );

		if ( bWallOnTheBackLeft && !bWallOnTheBackRight )
		{
			// Move straight left
			WallAvoidanceDir = BotSideLeftDir;
#ifndef _GOLD_MASTER
					if (CAINavigationSystemMan::Get().m_bRenderWallAvoidance) { g_VisualDebug->RenderLine(pt3DCentre, pt3DBackSideLeft, DC_PURPLE); }
#endif
		} 
		else if ( !bWallOnTheBackLeft && bWallOnTheBackRight )
		{
			// Move straight right
			*bActive = true;
			WallAvoidanceDir = BotSideRightDir;
#ifndef _GOLD_MASTER
					if (CAINavigationSystemMan::Get().m_bRenderWallAvoidance) { g_VisualDebug->RenderLine(pt3DCentre, pt3DBackSideRight, DC_PURPLE); }
#endif
		}

		// Is there a wall Straight Back
		if (CAINavigationSystemMan::Get().IntersectsAIVolume(pt3DCentre, pt3DBackStraight, &IPoint, &WallNormal) )
		{
			// Move away from the wall
			*bActive = true;
			WallAvoidanceDir += BotHeading;
#ifndef _GOLD_MASTER
					if (CAINavigationSystemMan::Get().m_bRenderWallAvoidance) 
					{ 
						g_VisualDebug->RenderLine(pt3DCentre, pt3DBackStraight, DC_PURPLE);
						g_VisualDebug->RenderLine(pt3DCentre, CPoint(pt3DCentre+WallAvoidanceDir), DC_RED);
					}
#endif
		}
	//	return WallAvoidanceDir;
	}

	// ===========================================================================================
	// ==================    FRONT DETECTION LINES    ============================================
	// ===========================================================================================

	if (pMov->IsInDynamicCoverMovement())
	{
		fSideDetLength = fDBLength*0.5f; //pMov->GetSideDetLength()*0.6f;
	}
	else
	{
		/*fDBLength = pMov->GetMinWallDetRadius() + 
					( pMov->GetSpeedMagnitude() / pMov->GetMaxSpeed() ) *
					  pMov->GetMinWallDetRadius();
		fSideDetLength = pMov->GetSideDetLength();*/
		fSideDetLength = fSideDetLength; //pMov->GetSideDetLength()*0.6f;
	}

	CPoint pt3DStraight		= pt3DCentre+CPoint(BotHeading)*fDBLength;
	CPoint pt3DSideLeft		= pt3DCentre+CPoint(BotSideLeftDir) *(fSideDetLength);
	CPoint pt3DSideRight	= pt3DCentre+CPoint(BotSideRightDir)*(fSideDetLength);

	// Render Stright Detection Line
#ifndef _GOLD_MASTER
				if (CAINavigationSystemMan::Get().m_bRenderWallAvoidance) 
				{ 
					g_VisualDebug->RenderLine(pt3DCentre, pt3DStraight, DC_WHITE);
					g_VisualDebug->RenderLine(pt3DCentre, pt3DSideLeft, DC_WHITE);
					g_VisualDebug->RenderLine(pt3DCentre, pt3DSideRight, DC_WHITE);
				}
#endif

	// Is there a wall Straight

	bool bWallOppositeAI	= false;
	bool bWallOnTheLeft		= false;
	bool bWallOnTheRight	= false;

	bWallOppositeAI  = CAINavigationSystemMan::Get().IntersectsAIVolume(pt3DCentre, pt3DStraight, &IPoint, &WallNormal);

	if ( bWallOppositeAI )
	{
		// Turn in the AI's Side direction
		//WallAvoidanceDir = BotSideDir*WallNormal.Dot(BotSideDir); // !!! - Correction (Dario) it should be BotSideDir*(sign(WallNormal.Dot(BotSideDir))*WallNormal.Dot(BotHeading))
	//	float fClockWise = WallNormal.Dot(BotSideDir) > 0.f ? 1.0f : -1.0f;
	//	WallAvoidanceDir = BotSideDir*fClockWise;
		*bActive = true;
		float fSense		= WallNormal.Dot(BotSideDir) > 0.f ? -1.0f : 1.0f;
		WallAvoidanceDir	= GetPerpendicular(WallNormal)*fSense;

#ifndef _GOLD_MASTER
				if (CAINavigationSystemMan::Get().m_bRenderWallAvoidance) 
				{ 
					g_VisualDebug->RenderLine(pt3DCentre, pt3DStraight, DC_PURPLE);
					g_VisualDebug->RenderLine(pt3DCentre, CPoint(pt3DCentre+WallAvoidanceDir), DC_RED);
				}
#endif
		return WallAvoidanceDir;
	}

	bWallOnTheLeft  = CAINavigationSystemMan::Get().IntersectsAIVolume(pt3DCentre, pt3DSideLeft,  &IPoint, &WallNormalLeft );
	bWallOnTheRight = CAINavigationSystemMan::Get().IntersectsAIVolume(pt3DCentre, pt3DSideRight, &IPoint, &WallNormalRight );

	if ( !( bWallOnTheLeft || bWallOnTheRight ) )
	{
		// No side walls detected
		return WallAvoidanceDir;
	}
	if ( bWallOnTheLeft && bWallOnTheRight )
	{
		// Narrow path, don't move to the sides
		return WallAvoidanceDir;
	} 

	// ===========================================================================================
	// =====================  FOLLOW WALLS (FOLLOW ENTITY)  ======================================
	// ===========================================================================================		

	//if (!pMov->IsFollowEntityCompleted())
	//{
	//	if ( bWallOnTheLeft )
	//	{
	//		// Follow it
	//		WallAvoidanceDir = GetPerpendicular(WallNormalLeft);
	//		SetAIAvoidanceTriggered(true);
	//			if (CAINavigationSystemMan::Get().m_bRenderWallAvoidance) { g_VisualDebug->RenderLine(pt3DCentre, pt3DSideLeft, DC_BLUE); }
	//		return WallAvoidanceDir;
	//	}
	//	if ( bWallOnTheRight )
	//	{
	//		// Follow it
	//		WallAvoidanceDir = -GetPerpendicular(WallNormalRight);
	//		SetAIAvoidanceTriggered(true);
	//			if (CAINavigationSystemMan::Get().m_bRenderWallAvoidance) { g_VisualDebug->RenderLine(pt3DCentre, pt3DSideRight, DC_BLUE); }
	//		return WallAvoidanceDir;
	//	}
	//}

	
	// ===========================================================================================
	// ======================    NORMAL FREE STEERING CASE  ======================================
	// ===========================================================================================	

	if ( bWallOnTheLeft )
	{
		// Turn Right
		//WallAvoidanceDir = BotSideRightDir;
		*bActive = true;
		WallAvoidanceDir = GetPerpendicular(WallNormalLeft);
#ifndef _GOLD_MASTER
				if (CAINavigationSystemMan::Get().m_bRenderWallAvoidance) { g_VisualDebug->RenderLine(pt3DCentre, pt3DSideLeft, DC_PURPLE); }
#endif
		return WallAvoidanceDir;
	}
	if ( bWallOnTheRight )
	{
		// Turn Left	
		//WallAvoidanceDir = BotSideLeftDir;
		*bActive = true;
		WallAvoidanceDir = -GetPerpendicular(WallNormalRight);
#ifndef _GOLD_MASTER
			if (CAINavigationSystemMan::Get().m_bRenderWallAvoidance) { g_VisualDebug->RenderLine(pt3DCentre, pt3DSideRight, DC_PURPLE); }
#endif
		return WallAvoidanceDir;
	}
	return WallAvoidanceDir; // Not needed but required by the compiler
}

//------------------------------------------------------------------------------------------
// DoesProveIntersect
//------------------------------------------------------------------------------------------
bool DoesProveIntersect(const CPoint& obProvePos, const CPoint& obCentre, const float radius) //, float& dist)
{
	CDirection Line(obProvePos - obCentre);

	if (Line.Length() < radius) 
		return true; 
	else 
		return false;
}

//------------------------------------------------------------------------------------------
// CalculateDynObstacleAvoidance
//------------------------------------------------------------------------------------------
CDirection CAISteeringLibrary::CalculateDynObstAvoidance (CAIMovement* pMov )
{
	CDirection ObstacleAvoidanceDir (CONSTRUCT_CLEAR);

	if (!pMov)
	{
		return ObstacleAvoidanceDir;
	}

	// Get the Heading and Side Dir
	CDirection BotHeading		= pMov->GetParent()->GetMatrix().GetZAxis();
	CDirection BotSideDir		= pMov->GetParent()->GetMatrix().GetXAxis();
	CDirection BotSideLeftDir	= BotHeading+BotSideDir;
	CDirection BotSideRightDir	= BotHeading-BotSideDir;

	// Calculate the Heading Detection Length

	float fDBLength = pMov->GetMinDynObstDetRadius() + 
					( pMov->GetSpeedMagnitude() / pMov->GetMaxSpeed() ) *
					  pMov->GetMinDynObstDetRadius();

	CPoint pt3DCentre		= pMov->GetParent()->GetPosition()+DBG_Y_OFFSET;
	CPoint pt3DStraight		= pt3DCentre+CPoint(BotHeading)*fDBLength;
	CPoint pt3DSideLeft		= pt3DCentre+CPoint(BotSideLeftDir) * (pMov->GetSideDetLength());
	CPoint pt3DSideRight	= pt3DCentre+CPoint(BotSideRightDir)* (pMov->GetSideDetLength());

	// Is there an Obstacle Straight ahead

	bool bObstacleStraight		= false;
	bool bObstacleOnTheLeft		= false;
	bool bObstacleOnTheRight	= false;
	float bClockwise			= 0.0f;
	
	// Loop through the list of Objects
	for (unsigned int i = 0 ; i< m_vSRE.size(); i++)
	{
		
		// Calculate the Distance-to-Object Direction
		CDirection  Line2Obstacle = CDirection(m_vSRE[i]->pEnt->GetPosition() - pMov->GetParent()->GetPosition());
		const float fDistToLocalZ = Line2Obstacle.Dot(pMov->GetParent()->GetMatrix().GetXAxis());

		// Check the Proves

		bObstacleStraight	= DoesProveIntersect(pt3DStraight ,m_vSRE[i]->pEnt->GetPosition(), m_vSRE[i]->fRadius);
		
		if ( bObstacleStraight )
		{
			// Turn in the AI's Side direction
			bClockwise = (fDistToLocalZ < 0.f) ?  1.f : -1.f;
			ObstacleAvoidanceDir = bClockwise*BotSideDir;
#ifndef _GOLD_MASTER
					if (CAINavigationSystemMan::Get().m_bRenderWallAvoidance) 
					{ 
						g_VisualDebug->RenderLine(pt3DCentre, pt3DStraight, DC_CYAN); 
						g_VisualDebug->RenderLine(pt3DCentre, CPoint(pt3DCentre+ObstacleAvoidanceDir), DC_WHITE); 

						// Highlight the object to avoid
						CPoint pt3DObstacle = m_vSRE[i]->pEnt->GetPosition();
						CMatrix ArcMatrix(CONSTRUCT_IDENTITY);
						ArcMatrix.SetTranslation(pt3DObstacle+DBG_Y_OFFSET);
						g_VisualDebug->RenderArc(ArcMatrix, m_vSRE[i]->fRadius , TWO_PI,  DC_YELLOW);
						g_VisualDebug->Printf3D(pt3DObstacle+DBG_Y_OFFSET,DC_WHITE,0,"OBST : %s",m_vSRE[i]->pEnt->GetName().c_str());
					}
#endif
			return ObstacleAvoidanceDir;
		}

		bObstacleOnTheLeft	= DoesProveIntersect(pt3DSideLeft ,m_vSRE[i]->pEnt->GetPosition(), m_vSRE[i]->fRadius);
		bObstacleOnTheRight	= DoesProveIntersect(pt3DSideRight,m_vSRE[i]->pEnt->GetPosition(), m_vSRE[i]->fRadius);
		
		if ( !bObstacleOnTheRight && !bObstacleOnTheLeft && !bObstacleStraight)
		{
			continue; // Proves didn't find an obstacle
		}
		if ( !( bObstacleOnTheLeft || bObstacleOnTheRight ) )
		{
			// No side walls detected
			return ObstacleAvoidanceDir;
		}
		if ( bObstacleOnTheLeft && bObstacleOnTheRight )
		{
			// Narrow path, don't move to the sides
			return ObstacleAvoidanceDir;
		} 

		// Debug Render The object to avoid
#ifndef _GOLD_MASTER
			if (CAINavigationSystemMan::Get().m_bRenderWallAvoidance) 
			{ 
				// Highlight the object to avoid
				CPoint pt3DObstacle = m_vSRE[i]->pEnt->GetPosition();
				CMatrix ArcMatrix(CONSTRUCT_IDENTITY);
				ArcMatrix.SetTranslation(pt3DObstacle+DBG_Y_OFFSET);
				g_VisualDebug->RenderArc(ArcMatrix, m_vSRE[i]->fRadius , TWO_PI,  DC_YELLOW);
				g_VisualDebug->Printf3D(pt3DObstacle+DBG_Y_OFFSET,DC_WHITE,0,"OBST : %s",m_vSRE[i]->pEnt->GetName().c_str());
			}
#endif

		if ( bObstacleOnTheLeft )
		{
			// Turn Right
			ObstacleAvoidanceDir = BotSideRightDir;
#ifndef _GOLD_MASTER
					if (CAINavigationSystemMan::Get().m_bRenderWallAvoidance) { g_VisualDebug->RenderLine(pt3DCentre, pt3DSideLeft, DC_CYAN); }
#endif
			return ObstacleAvoidanceDir;
		}
		if ( bObstacleOnTheRight )
		{
			// Turn Left	
			ObstacleAvoidanceDir = BotSideLeftDir;
#ifndef _GOLD_MASTER
				if (CAINavigationSystemMan::Get().m_bRenderWallAvoidance) { g_VisualDebug->RenderLine(pt3DCentre, pt3DSideRight, DC_CYAN); }
#endif
			return ObstacleAvoidanceDir;
		}
	}
	return (ObstacleAvoidanceDir);
}

//------------------------------------------------------------------------------------------
// CalculateAIAvoidance
//------------------------------------------------------------------------------------------
CDirection	CAISteeringLibrary::CalculateAIAvoidance(CAIMovement* pMov )
{
	CDirection AIAvoidanceDir (CONSTRUCT_CLEAR);
	SetAIAvoidanceTriggered(false);

	if (!pMov)
	{
		return AIAvoidanceDir;
	}

	// Get the Heading and Side Dir
	CDirection BotHeading		= pMov->GetParent()->GetMatrix().GetZAxis();
	CDirection BotSideDir		= pMov->GetParent()->GetMatrix().GetXAxis();
//	CDirection BotSideLeftDir	= BotHeading+BotSideDir;
//	CDirection BotSideRightDir	= BotHeading-BotSideDir;

	// Calculate the Heading Detection Length

	float fDBLength = pMov->GetAIAvoidanceRadius();

	CPoint pt3DCentre		= pMov->GetParent()->GetPosition()+DBG_Y_OFFSET;
	CPoint pt3DStraight		= pt3DCentre+CPoint(BotHeading)*fDBLength;
	
	unsigned int uiNeighboursCount = 0;

	// Loop through the list of AIs

	m_pLocalAIList = CAINavigationSystemMan::Get().GetAIList();

	if (!m_pLocalAIList)
		return AIAvoidanceDir;

	QueryResultsContainerType::iterator obIt	= m_pLocalAIList->begin();
	QueryResultsContainerType::iterator obEnd	= m_pLocalAIList->end();

	for ( ; obIt != obEnd; ++obIt )
	{
		CEntity* pEntBot = (*obIt);
 
		// Don't count PAUSED, DEAD or MYSELF
		if (	pEntBot->ToCharacter()->IsPaused() ||
				pEntBot->ToCharacter()->IsDead() ||
				pMov->GetParent() == pEntBot
			)
				
		{
			continue;
		}

		// Calculate the Distance-to-AIBot and Direction
		CDirection  Dist2AIBot = CDirection( pMov->GetPosition() - pEntBot->GetPosition());
		float fDotDist2AIBotHeading = Dist2AIBot.Dot(BotHeading);

		// Don't count neighbours behind us (if AI not in formation).
		if ( fDotDist2AIBotHeading > 0.0f && !pMov->IsInFormationMovement())
			continue;

		float fDist2AIBot = Dist2AIBot.Length();

		if ( fDist2AIBot < 0.001f )
		{
			// Overlapping
			fDist2AIBot = 0.5f;
			Dist2AIBot = BotSideDir;
			AIAvoidanceDir += Dist2AIBot / fDist2AIBot;
			ntPrintf("Overlapping AIs: [%s] - [%s]\n",ntStr::GetString(pMov->GetParent()->GetName()),ntStr::GetString(pEntBot->GetName()));
		}
		else
		{
			float fAvoidanceDistance = fDBLength;

			if (!pEntBot->GetAttackComponent()->AI_Access_IsInCSStandard())
				fAvoidanceDistance = 2.0f;

			// Only avoid entities that are close enough
			if ( (fDist2AIBot - fAvoidanceDistance) > fDBLength )	// !!! - This should be parameterized
			{
				continue;
			}

			// Obstacle is in front of AIBot, but can be seen?
			if (!CAINavigationSystemMan::Get().HasLineOfSight( pMov->GetPosition(), pEntBot->GetPosition() ) )
			{
				// The AI is in another room
				continue;
			}
			CDirection LineAvoid = Dist2AIBot;
			NORMALISE(LineAvoid);
			AIAvoidanceDir += LineAvoid;
		}

		// Separation
		

		// ========================================================
		// ================ FORMATION ? ===========================
		// ========================================================

		if (pMov->IsInFormationMovement())
		{
			// The NPC is attacking. We need special avoidance to get out of its way - it will not avoid us.
			// Therefore try and avoid behind it.
			if (!pEntBot->GetAttackComponent()->AI_Access_IsInCSStandard())
			{
				CMatrix ThisMatrix = pMov->GetParent()->GetMatrix();
				CPoint ThisPoint = ThisMatrix.GetTranslation();

				uint32_t uColour;

				CDirection AttackerDirection = ((AI*)pEntBot)->GetAIComponent()->GetMovementDirection();

				// Reverse direction - perpendicular is the same direction as entity that we are avoiding is heading in.
				if (AttackerDirection.Dot(GetPerpendicular(Dist2AIBot)))
				{
					// Move to the right of the blocking entity.
					AIAvoidanceDir += CDirection(-Dist2AIBot.Z(),Dist2AIBot.Y(),Dist2AIBot.X());

					uColour = DC_BLUE;
				}
				else
				{
					// Alternatively move to the left if stepping under the feet of the blocking entity.
					AIAvoidanceDir += GetPerpendicular(Dist2AIBot);

					uColour = DC_GREEN;
				}

#ifndef _GOLD_MASTER
				if (CAINavigationSystemMan::Get().m_bRenderAIAvoidance)
				{
					g_VisualDebug->RenderLine(pt3DCentre, pt3DCentre+CPoint(AIAvoidanceDir),DC_CYAN); 

					CMatrix OtherMatrix = pEntBot->GetMatrix();
					CPoint OtherPoint = OtherMatrix.GetTranslation();
					OtherMatrix.SetTranslation(OtherPoint+DBG_Y_OFFSET);
					g_VisualDebug->RenderArc(OtherMatrix, 0.25f, TWO_PI, uColour);

					g_VisualDebug->RenderLine(ThisPoint+DBG_Y_OFFSET, OtherPoint+DBG_Y_OFFSET, uColour);
				}
#endif
			}
			else
			{
				AIAvoidanceDir += GetPerpendicular(Dist2AIBot);
			}
		}

		// ========================================================
		// ================ END FORMATION =========================
		// ========================================================

		uiNeighboursCount++;

#ifndef _GOLD_MASTER
		if (CAINavigationSystemMan::Get().m_bRenderAIAvoidance)
		{
			// Render Line to obstacle & highlight it
			g_VisualDebug->RenderLine(pt3DCentre,pt3DCentre+CPoint(Dist2AIBot), DC_RED); // AI------>AI
		}
#endif
	}

	// Calculate the final Velocity
	if (uiNeighboursCount > 0)
	{
		NORMALISE(AIAvoidanceDir);
		SetAIAvoidanceTriggered(true);

		// Render FlockVelocity and Detection Line
#ifndef _GOLD_MASTER
		if (CAINavigationSystemMan::Get().m_bRenderAIAvoidance) 
		{ 
			g_VisualDebug->RenderLine(pt3DCentre, pt3DStraight, DC_BLUE);
			g_VisualDebug->RenderLine(pt3DCentre,pt3DCentre+CPoint(AIAvoidanceDir),DC_CYAN); 

			CMatrix ArcMatrix = pMov->GetParent()->GetMatrix();
			CPoint ArcTr = ArcMatrix.GetTranslation();
			ArcMatrix.SetTranslation(ArcTr+DBG_Y_OFFSET);
			g_VisualDebug->RenderArc(ArcMatrix, fDBLength, TWO_PI, DC_BLUE);
		}
#endif
	}

	return AIAvoidanceDir;
}

//------------------------------------------------------------------------------------------
// CalculateFlocking
//------------------------------------------------------------------------------------------
CDirection	CAISteeringLibrary::CalculateFlocking(CAIMovement* pMov )
{
	CDirection AIAvoidanceDir (CONSTRUCT_CLEAR);

	if (!pMov)
	{
		return AIAvoidanceDir;
	}
	// Get the Heading and Side Dir
	CDirection BotHeading		= pMov->GetParent()->GetMatrix().GetZAxis();
	CDirection BotSideDir		= pMov->GetParent()->GetMatrix().GetXAxis();

	// Calculate the Heading Detection Length

	float fDBLength = pMov->GetAIFlockingRadius() + 
					( pMov->GetSpeedMagnitude() / pMov->GetMaxSpeed() ) *
					  pMov->GetAIFlockingRadius();

	CPoint pt3DCentre		= pMov->GetParent()->GetPosition()+DBG_Y_OFFSET;
	CPoint pt3DStraight		= pt3DCentre+CPoint(BotHeading)*fDBLength;
	
	CDirection AverageHeading		(CONSTRUCT_CLEAR);
	CDirection SeparationVelocity	(CONSTRUCT_CLEAR);
	CDirection CohesionVelocity		(CONSTRUCT_CLEAR);
	CPoint CentreOfMass (CONSTRUCT_CLEAR);

	unsigned int uiNeighboursCount = 0;

	// Loop through the list of AI Bots
	for (unsigned int i = 0 ; i< m_vAIBots.size(); i++)
	{
		// Don't count myself
		if ( pMov->GetParent() == m_vAIBots[i] )
		{
			continue;
		}
		// Calculate the Distance-to-AIBot and Direction
		CDirection  Dist2AIBot = CDirection( pMov->GetPosition() - m_vAIBots[i]->GetPosition());
		float fDist2AIBotSQR = Dist2AIBot.LengthSquared();	

		if ( fDist2AIBotSQR < 0.001f )
		{
			// Overlaping
			fDist2AIBotSQR = 1.0f;
			AIAvoidanceDir = BotSideDir;
		}
		else
		{
			// Only flock with entities that are close enough
			if ( (fDist2AIBotSQR - 0.5f) > fDBLength )	// !!! - This should be parameterised
			{
				continue;
			}
			
			// Obstacle is in front of AIBot, but can be seen?
			if (!CAINavigationSystemMan::Get().HasLineOfSight( pMov->GetPosition(), m_vAIBots[i]->GetPosition() ) )
			{
				// The AI is in another room
				continue;
			}

			// Do not take into acount entities that are behind a 30 degrees cone
			CDirection Line2AI = -Dist2AIBot;
			Line2AI.Normalise();
			if ( BotHeading.Dot(Line2AI) < -0.5 )
			{
				continue;
			}
		}

		// Separation inv. propoertional to the (dist)^2
		SeparationVelocity += 2*(Dist2AIBot / fDist2AIBotSQR);
		
		// Alignemet

		AverageHeading += m_vAIBots[i]->GetMatrix().GetZAxis();

		// Cohesion
	
		CentreOfMass +=  m_vAIBots[i]->GetPosition();

		uiNeighboursCount++;

#ifndef _GOLD_MASTER
				if (CAINavigationSystemMan::Get().m_bRenderAIAvoidance)
				{
					// Render Line to obstacle & highlight it
					g_VisualDebug->RenderLine(pt3DCentre,pt3DCentre+CPoint(Dist2AIBot),DC_RED); // AI------>AI
					CMatrix ArcMatrix = pMov->GetParent()->GetMatrix();
					CPoint ArcTr = ArcMatrix.GetTranslation();
					ArcMatrix.SetTranslation(ArcTr+DBG_Y_OFFSET);
					g_VisualDebug->RenderArc(ArcMatrix, fDBLength, TWO_PI, DC_GREEN);
				}
#endif
	}

	// Calculate the final Velocity

	if (uiNeighboursCount > 0)
	{
		AverageHeading	/= (float)uiNeighboursCount;	
		CentreOfMass	/= (float)uiNeighboursCount;
		NORMALISE(AverageHeading);
		CohesionVelocity = CDirection(CentreOfMass - pMov->GetParent()->GetPosition());
		NORMALISE(CohesionVelocity);

		AIAvoidanceDir = SeparationVelocity + AverageHeading + CDirection(CentreOfMass - pMov->GetParent()->GetPosition());
		NORMALISE(AIAvoidanceDir);

		// Render FlockVelocity and Detection Line
#ifndef _GOLD_MASTER
				if (CAINavigationSystemMan::Get().m_bRenderAIAvoidance) 
				{ 
					g_VisualDebug->RenderLine(pt3DCentre, pt3DStraight, DC_BLUE);
					g_VisualDebug->RenderLine(pt3DCentre,pt3DCentre+CPoint(AIAvoidanceDir),DC_CYAN); 
					g_VisualDebug->RenderPoint(CentreOfMass+DBG_Y_OFFSET,10.0f,DC_RED);
					g_VisualDebug->RenderLine(CentreOfMass+DBG_Y_OFFSET, pMov->GetParent()->GetPosition()+DBG_Y_OFFSET, DC_BLUE);
				}
#endif
	}
	
	return AIAvoidanceDir;
}

//! -------------------------------------------
//! IsThereObstaclesAtPos
//! -------------------------------------------
bool CAISteeringLibrary::IsThereObstaclesAtPos ( const CPoint& obPos, float fRadius )
{
	// Loop through the list of Dynamic Obstacles
	for (unsigned int i = 0 ; i< m_vSRE.size(); i++)
	{
		CDirection obDistObstacle2Pos(m_vSRE[i]->pEnt->GetPosition() - obPos);
		float fExtendedRadius = fRadius + m_vSRE[i]->fRadius;
		fExtendedRadius *= fExtendedRadius;

		if ( obDistObstacle2Pos.LengthSquared() < fExtendedRadius)
		{
			return true;
		}
	}
	return false;
}

//! ---------------------------------------------------------------------------------------------------------------------------------
//!														Calculate Steering Action
//! ---------------------------------------------------------------------------------------------------------------------------------

//! -------------------------------------------
//! CalculateSteeringAction
//! -------------------------------------------
CDirection CAISteeringLibrary::CalculateSteeringAction ( CAIMovement* pMov, float fTimeChange )
{
	CDirection Clear_Dir = CDirection(CONSTRUCT_CLEAR);
	CDirection TotalSteeringAction = CDirection(CONSTRUCT_CLEAR);
	UNUSED(fTimeChange);

	bool bNonZeroFPC	= false;
	bool bNonZeroFPCD	= false;
	bool bNonZeroWall	= false;

	float*					pfSteeringWeights	= pMov->GetSteeringWeights();
	ENUM_NAVIGATION_FLAGS	eNavigFlag			= (ENUM_NAVIGATION_FLAGS)pMov->GetSteeringFlags();
	
	// ============================
	// Basic Steering Actions
	// ============================

	if IS_ACTIVE(NF_D_OBSTACLE)			TotalSteeringAction += STEER_WIGHT(NWI_D_OBSTACLE)			*CalculateDynObstAvoidance	(pMov);
//	if IS_ACTIVE(NF_FOLLOW_PATH)		TotalSteeringAction += STEER_WIGHT(NWI_FOLLOW_PATH)			*FollowPath					(pMov);
	if IS_ACTIVE(NF_ARRIVE_AT_POINT)	TotalSteeringAction += STEER_WIGHT(NWI_ARRIVE)				*ArriveAtPoint				(pMov);
	if IS_ACTIVE(NF_CHASE_ENEMY) 		TotalSteeringAction += STEER_WIGHT(NWI_CHASE_ENEMY)			*ChaseTarget				(pMov);
	if IS_ACTIVE(NF_FLEE)				TotalSteeringAction += STEER_WIGHT(NWI_FLEE)				*Flee						(pMov, pMov->GetPlayer()->GetPosition());
	if IS_ACTIVE(NF_FOLLOW_ENTITY)		TotalSteeringAction += STEER_WIGHT(NWI_FOLLOW_ENTITY)		*FollowEntity				(pMov);
	if IS_ACTIVE(NF_FOLLOW_PATROL_PATH)	TotalSteeringAction += STEER_WIGHT(NWI_FOLLOW_PATROL_PATH )	*FollowPatrolPath			(pMov);
//	if IS_ACTIVE(NF_FLOCK)				TotalSteeringAction += STEER_WIGHT(NWI_FLOCK)				*CalculateFlocking			(pMov);
	if IS_ACTIVE(NF_TO_COMBAT_POINT)	TotalSteeringAction += STEER_WIGHT(NWI_TO_COMBAT_POINT)		*StrafeToFormationPoint		(pMov);
	if IS_ACTIVE(NF_FOLLOW_PATH_DYNCOVER)	TotalSteeringAction += STEER_WIGHT(NWI_TO_COMBAT_POINT)	*FollowPathWithCover		(pMov, &bNonZeroFPCD);
	if IS_ACTIVE(NF_CHASE_MOVING_ENTITY) TotalSteeringAction += STEER_WIGHT(NWI_TO_COMBAT_POINT)	*ChaseMovingEntity			(pMov);
	if IS_ACTIVE(NF_GO_AROUND_VOLUMES)	TotalSteeringAction += STEER_WIGHT(NWI_GO_AROUND_VOLUMES)	*GoAroundVolume				(pMov);

	// ============================
	// Follow Path Cover
	// ============================
	if (IS_ACTIVE(NF_FOLLOW_PATH_COVER) || IS_ACTIVE(NF_FOLLOW_PATH_DYNCOVER))
	{
		TotalSteeringAction += STEER_WIGHT(NWI_TO_COMBAT_POINT)*FollowPathWithCover(pMov, &bNonZeroFPC);
		//if (pMov->IsPointCloserThanMinRange(pMov->GetPosition()))
		//{
		//	TotalSteeringAction.Normalise();
		//	TotalSteeringAction *= pMov->GetWalkSpeed();
		////	pMov->GetCAIComp()->ActivateController(CAIComponent::MC_STRAFING);
		//}
	}

	// ============================
	// Static Obstacle Avoidance
	// ============================
	if IS_ACTIVE(NF_S_OBSTACLE)
	{
		if (pMov->IsInFormationMovement())
		{
			// When in formation, do as usual
			// ------------------------------
			TotalSteeringAction += STEER_WIGHT(NWI_S_OBSTACLE)*CalculateWallAvoidance(pMov, &bNonZeroWall);
		}
		else
		{
			// Try something new
			// ------------------------------
			bool bSupported = IS_ACTIVE(NF_ARRIVE_AT_POINT) || IS_ACTIVE(NF_TO_COMBAT_POINT) || IS_ACTIVE(NF_FOLLOW_PATH_COVER) || IS_ACTIVE(NF_FOLLOW_PATH_DYNCOVER);
			CPoint obTgtPoint = IS_ACTIVE(NF_ARRIVE_AT_POINT)	? pMov->GetDestinationPos() : 
								IS_ACTIVE(NF_TO_COMBAT_POINT)	? pMov->GetAttackPoint() : 
								( ( IS_ACTIVE(NF_FOLLOW_PATH_COVER) || IS_ACTIVE(NF_FOLLOW_PATH_DYNCOVER)) && bNonZeroFPC && pMov->GetPathContainer()->GetCurrentNode() ) ? pMov->GetPathContainer()->GetPointWithinCurrentNode() : 
								CPoint (1000.0f,0.0f,1000.0f);
			if (bSupported)
			{
				bool bHasLOS = CAINavigationSystemMan::Get().HasLineOfSightExcludingGoAroundVolumes(pMov->GetPosition(),obTgtPoint);
				if (!bHasLOS)
				{
					CDirection WallDir = CalculateWallAvoidance(pMov,&bNonZeroWall);
					if (bNonZeroWall)
					{
						TotalSteeringAction = WallDir;
						return TotalSteeringAction;
					}
				}
			}
			else
			{
				TotalSteeringAction += STEER_WIGHT(NWI_S_OBSTACLE)*CalculateWallAvoidance(pMov,&bNonZeroWall);
			}
		}
	}

	// ============================
	// AI Collision Avoidance
	// ============================
	if IS_ACTIVE(NF_AI_OBSTACLE)
	{
		if (pMov->IsInFormationMovement())
		{
			// When in formation, do as usual
			// ------------------------------
			TotalSteeringAction +=STEER_WIGHT(NWI_AI_OBSTACLE)*CalculateAIAvoidance(pMov);
		}
		else
		{
			// Try something new
			// ------------------------------
			CDirection AIVoidanceAction		= STEER_WIGHT(NWI_AI_OBSTACLE)*CalculateAIAvoidance(pMov);
			float fModuleAIVoidanceAction	= AIVoidanceAction.Length();
			
			if ( fModuleAIVoidanceAction > 0.05f && !pMov->IsSlowingDown() )
			{
				// There is AI Avoidance Action
				// ----------------------------
				CDirection FinalAction = TotalSteeringAction;
				FinalAction.Normalise();
				float fModuleTotalSteeringAction	= TotalSteeringAction.Length();
						
				if (fModuleTotalSteeringAction > AIMOV_SLOWWALK_SPEED)
				{
					// Apply only a braking action
					float fEffectiveSpeed = min(fModuleTotalSteeringAction, pMov->GetSpeedMagnitude());
					float fSpeedFactor =	(fEffectiveSpeed > AIMOV_FASTWALK_SPEED) ? AIMOV_FASTWALK_SPEED :
											(fEffectiveSpeed > AIMOV_SLOWWALK_SPEED) ? AIMOV_SLOWWALK_SPEED :
											0.0f;
					pMov->SetBreakSpeed(fSpeedFactor);
					pMov->ResetRemainingSlowDownTime();		
				}
				TotalSteeringAction = FinalAction*pMov->GetBreakSpeed();			
			}
		}
	}

	// ============================
	// Normalise and Return
	// ============================

	if ( TotalSteeringAction.LengthSquared()>0.99f )
	{
		TotalSteeringAction.Normalise();
	}
#ifndef _GOLD_MASTER
	if (CAINavigationSystemMan::Get().m_bRenderTotalSteeringAction) 
	{
		if (pMov->IsLeader()) g_VisualDebug->Printf3D(pMov->GetParent()->GetPosition(),DC_YELLOW,0,"LEADER");
		if IS_ACTIVE(NF_FOLLOW_PATROL_PATH) g_VisualDebug->Printf3D(pMov->GetParent()->GetPosition(),DC_YELLOW,0,"Patrolling");
		g_VisualDebug->RenderLine(pMov->GetPosition()+DBG_Y_OFFSET, CPoint(pMov->GetPosition()+DBG_Y_OFFSET+TotalSteeringAction), DC_GREEN);
	}
#endif
	return ( TotalSteeringAction );
}
