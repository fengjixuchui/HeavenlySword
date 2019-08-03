//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file airepulsion.cpp
//!                                                                                         
//------------------------------------------------------------------------------------------

#include "Physics/config.h"
#include "Physics/system.h"
#include "Physics/compoundlg.h"

#include "ai/airepulsion.h"
#include "ai/aicoverpoint.h"

#include "core/visualdebugger.h"
#include "core/boundingvolumes.h"

#include "game/entitymanager.h"
#include "game/movementcontrollerinterface.h"
#include "game/query.h"
#include "game/renderablecomponent.h"

static const bool AVOID_RENDER = false;

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIObjectAvoidance::AddEntity
//!                                                                                         
//------------------------------------------------------------------------------------------

int AIObjectAvoidance::AddEntity( const CEntity* pobEntity, float radius )
{
	ntAssert( m_iNumEnts < NUM_REPELLERS );

	for (int i = 0; i < m_iNumEnts; ++i)
	{
		if (m_pobEntities[i] == pobEntity)
		{
			m_bDontAvoid[i] = false;
			return m_iNumEnts;
		}
	}

	m_pobEntities[m_iNumEnts] = pobEntity;
	m_fRadii[m_iNumEnts] = radius;
	return m_iNumEnts++;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIObjectAvoidance::RemoveEntity
//!                                                                                         
//------------------------------------------------------------------------------------------

void AIObjectAvoidance::RemoveEntity( const CEntity* pobEntity )
{
	for (int i = 0; i < m_iNumEnts; ++i)
	{
		if (m_pobEntities[i] == pobEntity)
		{
			m_bDontAvoid[i] = true;
			break;
		}
	}
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIObjectAvoidance::AddAvoidPoint
//!                                                                                         
//------------------------------------------------------------------------------------------

int AIObjectAvoidance::AddAvoidPoint( const AvoidPoint* point )
{
	ntAssert( m_iNumPoints < NUM_REPELLERS );

	m_pobPoints[m_iNumPoints] = point;
	return m_iNumPoints++;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIObjectAvoidance::GetNumRepellers
//!                                                                                         
//------------------------------------------------------------------------------------------

int AIObjectAvoidance::GetNumRepellers() const
{
	return m_iNumEnts;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIObjectAvoidance::GetPos
//!                                                                                         
//------------------------------------------------------------------------------------------

CPoint AIObjectAvoidance::GetPos( const int iNum ) const
{
	ntAssert( iNum < m_iNumEnts );
	return m_pobEntities[iNum]->GetPosition();
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIObjectAvoidance::IsObstructed
//!                                                                                         
//------------------------------------------------------------------------------------------

bool AIObjectAvoidance::IsObstructed( const CPoint& obPos, const float fRadius, CSphereBound* pResult ) const
{
	float fRadiusSqrd = fRadius * fRadius;

	for (int i = 0; i < m_iNumEnts + m_iNumPoints; ++i)
	{
		// Only contribute repulsion if the object is worth avoiding
		if ( i < m_iNumEnts && m_bDontAvoid[i])
		{
			continue;
		}

		// work out repulsion dir
		CPoint obRepulsionDir;
		float TestRadius		= 1.0f;
		float TestRadiusSqrd	= 1.0f;

		if (i < m_iNumEnts)
		{
			obRepulsionDir	= obPos - m_pobEntities[i]->GetPosition();
			TestRadius		= m_fRadii[i];
			TestRadiusSqrd	= TestRadius * TestRadius;
		}
		else
		{
			obRepulsionDir	= obPos - m_pobPoints[i-m_iNumEnts]->GetPos();
		}

		obRepulsionDir.Y() = 0.0f;

		// discard cases where entity is too close or too far away
		if ( obRepulsionDir.LengthSquared() < (fRadiusSqrd + TestRadiusSqrd) )
		{
			if( pResult )
			{
				*pResult = CSphereBound( m_pobEntities[i]->GetPosition(), TestRadius );
			}
			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIObjectAvoidance::IsObstructed
//!                                                                                         
//!	This version checks for obstruction between 2 points                                                                                         
//!                                                                                         
//! Note that the means of testing is a bit skanky at the moment.
//! TODO: convert this whole thing over to Havok phantoms and use capsule vs. OBB for this	
//!                                                                                         
//!  A little optimisation to the function.
//!                                                                                         
//------------------------------------------------------------------------------------------

bool AIObjectAvoidance::IsObstructed( const CPoint& from, const CPoint& to, const float fRadius, CSphereBound* pResult ) const
{
	float fRadiusSqrd = fRadius * fRadius;


	// Generate a vector from the from/to points
	CPoint TestLine = to - from;
	float LineLen = TestLine.Length();

	// Same start and end? Return a now obstruction test
	if( LineLen == 0 )
		return IsObstructed( from, fRadius );

	// Create a unit len
	CPoint UnitTest = TestLine / LineLen;

	// For each obsticle... 
	for (int i = 0; i < m_iNumEnts + m_iNumPoints; ++i)
	{
		// Only contribute repulsion if the object is worth avoiding
		if ( i < m_iNumEnts && m_bDontAvoid[i])
			continue;

		// work out repulsion dir
		CPoint ObsticlePoint;
		float TestRadius		= 1.0f;
		float TestRadiusSqrd	= 1.0f;


		if (i < m_iNumEnts)
		{
			ObsticlePoint = m_pobEntities[i]->GetPosition() - from;
			TestRadius = m_fRadii[i];
			TestRadiusSqrd = TestRadius * TestRadius;
		}
		else
		{
			ObsticlePoint = m_pobPoints[i-m_iNumEnts]->GetPos() - from;
		}

		float Scalar = UnitTest.Dot( ObsticlePoint );
		CPoint ClosestPoint = UnitTest * Scalar;
		float DistSqrd = (ObsticlePoint - ClosestPoint).LengthSquared();

		if( Scalar > -(fRadius + TestRadius) && 
			Scalar < (LineLen + fRadius + TestRadius) && 
			(DistSqrd < (fRadiusSqrd + TestRadiusSqrd))  )
		{
			if( pResult )
			{
				*pResult = CSphereBound( m_pobEntities[i]->GetPosition(), TestRadius );
			}
			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIObjectAvoidance::GetAngle
//!                                                                                         
//------------------------------------------------------------------------------------------

float AIObjectAvoidance::GetAngle( const CPoint& obVec1, const CPoint& obVec2 )
{
	// Remove any Y translation on the two input vectors
	CDirection obTempRotateFrom( obVec1.X(), 0.0f, obVec1.Z() );
	CDirection obTempRotateTo( obVec2.X(), 0.0f, obVec2.Z() );

	float fAngle = MovementControllerUtilities::RotationAboutY( obTempRotateFrom, obTempRotateTo );
	if (fabs( fAngle ) > PI)
	{
		if (fAngle < 0.0f)
		{
			fAngle += TWO_PI;
		}
		else
		{
			fAngle -= TWO_PI;
		}
	}

	return fAngle;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIObjectAvoidance::GetAngle
//!                                                                                         
//------------------------------------------------------------------------------------------

float AIObjectAvoidance::GetAngle( const CDirection& obVec1, const CPoint& obVec2 )
{
	// Remove any Y translation on the two input vectors
	CDirection obTempRotateTo( obVec2.X(), 0.0f, obVec2.Z() );

	float fAngle = MovementControllerUtilities::RotationAboutY( obVec1, obTempRotateTo );
	if (fabs( fAngle ) > PI)
	{
		if (fAngle < 0.0f)
		{
			fAngle += TWO_PI;
		}
		else
		{
			fAngle -= TWO_PI;
		}
	}

	return fAngle;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIObjectAvoidance::FirstFrameInit
//!                                                                                         
//------------------------------------------------------------------------------------------

void
AIObjectAvoidance::FirstFrameInit()
{
	// get rid of all our old entity references
	Clear();

	// Create an entity query for the types of objects we're after.
	// For now this is heavy interactable objects.
	CEntityQuery			obAvoidQuery;
	CEQCShouldAvoid	obClause;

	obAvoidQuery.AddClause( obClause );

	// run the query and add the entities it returns to our list of repellers 
	CEntityManager::Get().FindEntitiesByType( obAvoidQuery, CEntity::EntType_AllButStatic );

	int count   = 0;
	QueryResultsContainerType::iterator obEnd = obAvoidQuery.GetResults().end();
	for ( QueryResultsContainerType::iterator obIt = obAvoidQuery.GetResults().begin(); obIt != obEnd; ++obIt )
	{
		CEntity* pEnt = *obIt;
		float Radius = 1.0f;

		if( pEnt->GetRenderableComponent() )
		{
			CPoint Min = pEnt->GetRenderableComponent()->GetWorldSpaceAABB().Min();
			CPoint Max = pEnt->GetRenderableComponent()->GetWorldSpaceAABB().Max();

			Min.Y() = 0.0f;
			Max.Y() = 0.0f;
			
			Radius = (Max-Min).Length();
		}

		AddEntity( *obIt, Radius );
		count++;
	}

	ntPrintf( "entities added to repulsion manager %d\n", count );
}

/***************************************************************************************************
*
*	FUNCTION		
*
*	DESCRIPTION		
*
***************************************************************************************************/

CPoint AIObjectAvoidance::MakeForce( const CEntity* pobAI, const int /*iNum*/, const CPoint& obPos, const CPoint& obHeading, float /*fTimeStep*/ )
{
	const float fCharRadius( 0.5f );
	const float fObstacleRadius( 1.0f );
	const float MinDetectionBoxLength = 5.0f;
	const float MaxSpeed = 0.5f;
	//UNUSED(MinDetectionBoxLength);

	float fHeadingLen = obHeading.Length();

	// Check for a valid float number
	if( fHeadingLen < FLT_EPSILON || fHeadingLen > FLT_MAX )
		return CPoint( CONSTRUCT_CLEAR );

	//the detection box length is proportional to the agent's velocity
	float fDetBoxLength = MinDetectionBoxLength + ((fHeadingLen / MaxSpeed) * MinDetectionBoxLength);
	//float fDetBoxLength = 1.5f;

	//tag all obstacles within range of the box for processing
	//m_pVehicle->World()->TagObstaclesWithinViewRange(m_pVehicle, m_dDBoxLength);

	//this will keep track of the closest intersecting obstacle (CIB)
	int ClosestIntersectingObstacle = -1;
 
	//this will be used to track the distance to the CIB
	double DistToClosestIP = FLT_MAX;

	//this will record the transformed local coordinates of the CIB
	CPoint LocalPosOfClosestObstacle;

	bool behind = false;
	CDirection headingDir( obHeading / obHeading.Length() );
	CDirection up( 0.0f, 1.0f, 0.0f );
	CDirection forward( 0.0f, 0.0f, 1.0f );
	//CDirection forward( 1.0f, 0.0f, 0.0f );

	for (int i = 0; i < m_iNumEnts + m_iNumPoints; ++i)
	{
		// Only contribute repulsion if the object is worth avoiding
		if ( i < m_iNumEnts && (m_bDontAvoid[i] || m_pobEntities[i] == pobAI))
		{
			continue;
		}

		//calculate this obstacle's position in local space
		CMatrix obEntityLWInv = pobAI->GetMatrix().GetFullInverse();

		/*
		CMatrix obEntityLW( up, MovementControllerUtilities::RotationAboutY( forward, headingDir ) );
		obEntityLW.SetTranslation( pobAI->GetPosition() );
		CMatrix obEntityLWInv = obEntityLW.GetFullInverse();
		*/

		CPoint	pos;
		float	fRadius;
		if (i < m_iNumEnts)
		{
			pos = obPos - m_pobEntities[i]->GetPosition();
			fRadius = m_fRadii[i];
		}
		else
		{
			ntAssert( i-m_iNumEnts < m_iNumPoints );
			pos = obPos - m_pobPoints[i-m_iNumEnts]->GetPos();
			fRadius = 1.0f;
		}

		CPoint LocalPos = pos * obEntityLWInv;

		//if the local position has a negative x value then it must lay
		//behind the agent. (in which case it can be ignored)
		if ((LocalPos.Z() + fObstacleRadius) >= 0)
		{
			//if the distance from the x axis to the object's position is less
			//than its radius + half the width of the detection box then there
			//is a potential intersection.
			//double ExpandedRadius = (*curOb)->BRadius() + m_pVehicle->BRadius();
			double ExpandedRadius = fObstacleRadius + fCharRadius;

			if (fabs(LocalPos.X()) < ExpandedRadius)
			{
				//now to do a line/circle intersection test. The center of the 
				//circle is represented by (cX, cY). The intersection points are 
				//given by the formula x = cX +/-sqrt(r^2-cY^2) for y=0. 
				//We only need to look at the smallest positive value of x because
				//that will be the closest point of intersection.
				double cX = LocalPos.Z();
				double cY = LocalPos.X();
		          
				//we only need to calculate the sqrt part of the above equation once
				double SqrtPart = sqrt(ExpandedRadius*ExpandedRadius - cY*cY);

				double ip = cX - SqrtPart;

				if (ip <= 0.0)
				{
					ip = cX + SqrtPart;
				}

				//test to see if this is the closest so far. If it is keep a
				//record of the obstacle and its local coordinates
				if (ip < DistToClosestIP)
				{
					DistToClosestIP = ip;

					ClosestIntersectingObstacle = i;

					LocalPosOfClosestObstacle = LocalPos;
				}         
			}
		}
		else
		{
			//ntPrintf( "behind\n" );
			behind = true;
		}
	}

	//if we have found an intersecting obstacle, calculate a steering 
	//force away from it
	CPoint SteeringForce(CONSTRUCT_CLEAR);

	if (ClosestIntersectingObstacle != -1)
	{
		//the closer the agent is to an object, the stronger the 
		//steering force should be
		double multiplier = 1.0 + (fDetBoxLength - LocalPosOfClosestObstacle.Z()) /
							fDetBoxLength;

		const float avoidBoost = 1.5f;

		//calculate the lateral force
		SteeringForce.X() = float((fObstacleRadius - LocalPosOfClosestObstacle.X())  * multiplier) * avoidBoost;

		//apply a braking force proportional to the obstacles distance from
		//the vehicle. 
		//const double BrakingWeight = 0.2;
		const double BrakingWeight = 0.0;

		SteeringForce.Z() = float((fObstacleRadius - LocalPosOfClosestObstacle.Z()) * BrakingWeight);
		//ntPrintf( "avoiding\n" );
	}
	else if (!behind)
	{
		//ntPrintf( "no obstacle\n" );
	}

	//finally, convert the steering vector from local to world space
	CMatrix mat = pobAI->GetMatrix();
	mat.SetTranslation( CPoint(CONSTRUCT_CLEAR) );
	CPoint WorldSteeringForce = SteeringForce * mat;
	WorldSteeringForce *= 0.5f;

#ifndef _GOLD_MASTER
	if (AVOID_RENDER)
	{
		CPoint start = obPos;
		CPoint end = obPos + (WorldSteeringForce * 5.0f);
		start.Y() += 0.5f;
		end.Y() += 0.5f;
		g_VisualDebug->RenderLine( start, end, 0xff0000ff, DPF_NOCULLING );

		start = obPos;
		end = obPos + (obHeading * 10.0f);
		start.Y() += 0.5f;
		end.Y() += 0.5f;
		g_VisualDebug->RenderLine( start, end, 0xff00ff00, DPF_NOCULLING );
	}
#endif

	return WorldSteeringForce;
}


void AIObjectAvoidance::Update( float /*fTimeStep*/ )
{
	// all we need to do here is sweep for collapsed objects and mark them as "don't avoid"
	for (int i = 0; i < m_iNumEnts; ++i)
	{
		if (m_bDontAvoid[i])
		{
			continue;
		}

#ifndef _GOLD_MASTER
		if (AVOID_RENDER)
		{
			CQuat	obOrientation( CONSTRUCT_IDENTITY );
			g_VisualDebug->RenderSphere( obOrientation, m_pobEntities[i]->GetPosition(), m_fRadii[i], 0xff0fff0f );
		}
#endif

		if (m_pobEntities[i]->GetPhysicsSystem())
		{
			Physics::CompoundLG* pobState = (Physics::CompoundLG*) m_pobEntities[i]->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::COMPOUND_RIGID_LG );
			if (pobState && pobState->IsCollapsed())
			{
				m_bDontAvoid[i] = true;
			}
		}
	}

}













