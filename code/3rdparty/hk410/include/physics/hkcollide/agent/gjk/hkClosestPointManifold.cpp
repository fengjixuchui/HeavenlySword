/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkcollide/hkCollide.h>

#include <hkcollide/agent/gjk/hkClosestPointManifold.h>
#include <hkcollide/agent/hkCollisionInput.h>
#include <hkcollide/agent/hkContactMgr.h>
#include <hkinternal/collide/gjk/hkGsk.h>
#include <hkcollide/agent/hkCollisionAgentConfig.h>

HK_COMPILE_TIME_ASSERT( sizeof(hkProcessCdPoint) == 48);


//	try to find a point which can be safely deleted
//
//	steps:	which is most distant from all other points
// returns the point to be removed
// Note: the 5th point gets a slightly higher priority


	// (Collision config) This tolerance is used by the convex-convex manifold to
	// reject points which are too close together The default value is .05, i.e.
	// contactpoints less than 5 cm apart are considered to be equivalent. If you are
	// simulating an object with a side length of less than 5 cm you need to reduce
	// this tolerance.
static const hkReal WELD_TOLERANCE = 0.05f;

	// (Collision config) This tolerance is used by the convex-convex manifold to
	// remove points when the original points of contact from both objects have drifted
	// too far apart along the contact plane. The default value is 0.2f which means
	// that points that have slid apart by 20cm will be removed from the manifold. Note
	// that if the time step is large this tolerance may need to be increased in order
	// to avoid throwing out contact points that are necessary to maintain a proper
	// manifold.
static const hkReal DRIFT_TOLERANCE = 0.2f;


int hkClosestPointManifold::findRedundant5thPoint(const hkVector4** points)
{
	const int numPoints = 5;
	hkBool used[5];

	used[0] = false;
	used[1] = false;
	used[2] = false;
	used[3] = false;
	used[4] = false;


	// find ref point the furthest point from the start
	int idxA = 0;
	hkReal maxDistanceForFirstDiag = 0.0f;
	{
		{	// search a point most distant from the start point (5th point)
			const hkVector4 &ref = *points[numPoints-1];
			for (int i = 0; i < numPoints-1; i++)
			{
				const hkVector4 &acp = *points[i];
				hkVector4 diff; diff.setSub4( ref, acp );
				const hkReal dist = diff.lengthSquared3();
				if ( dist > maxDistanceForFirstDiag )
				{
					maxDistanceForFirstDiag = dist;
					idxA = i;
				}
			}
			used[idxA] = true;	// used it
		}
	}
	
	// find a point furthest from the furthest of the start, probably the start point
	int idxB = 4;
	{	// search a second point with the biggest distance, but favor start
		maxDistanceForFirstDiag *= 1.05f; // favor 4th point
		const hkVector4 &ref = *points[idxA];
		for (int i=0; i < numPoints; i++)
		{
			if ( used[i]  )
			{
				continue;
			}
			const hkVector4 &acp = *points[i];
			hkVector4 diff; diff.setSub4( ref, acp );
			const hkReal dist = diff.lengthSquared3();
			if ( dist > maxDistanceForFirstDiag )
			{
				maxDistanceForFirstDiag = dist;
				idxB = i;
			}
		}
		used[idxB] = true;
	}

	int idxC = 0;
	{	// find a point which is most distant from the diagonal between a and b
		hkVector4 diag; diag.setSub4( *points[idxA], *points[idxB] );
		hkReal maxDistance = 0.0f;
		const hkVector4 &pointA = *points[idxA];
		for (int i=0; i < numPoints; i++)
		{
			if ( used[i]  )
			{
				continue;
			}
			const hkVector4 &acp = *points[i];
			hkVector4 diff; diff.setSub4( acp, pointA );
			hkVector4 cross; cross.setCross( diff, diag );
			const hkReal dist = cross.dot3(cross);
			if ( dist > maxDistance )
			{
				maxDistance = dist;
				idxC = i;
			}
		}
		used[idxC] = true;
	}

	int idxD = 0;
	{	// search a forth point with the biggest distance to C
		hkReal maxDistance = 0.0f;
		const hkVector4 &ref = *points[idxC];
		for (int i=0; i < numPoints; i++)
		{
			if ( used[i]  )
			{
				continue;
			}
			const hkVector4 &acp = *points[i];
			hkVector4 diff; diff.setSub4( ref, acp );
			const hkReal dist = diff.dot3(diff);
			if ( dist > maxDistance )
			{
				maxDistance = dist;
				idxD = i;
			}
		}
		used[idxD] = true;
	}

	// find unused point
	{
		for (int i = 0; i < numPoints; i++)
		{
			if ( !used[i] )
			{
				return i;
			}
		}
	}
	return 0;
}

void hkClosestPointManifold::cleanup( hkAgentContactPoint* contactPoints, int& numPoints, hkContactMgr* mgr, hkCollisionConstraintOwner& constraintOwner )
{
	while ( numPoints )
	{
		numPoints--;
		mgr->removeContactPoint( hkContactPointId(contactPoints[ numPoints ].getContactPointId()), constraintOwner);
	}
}

void hkClosestPointManifold::addPoint(const  hkCdBody& ca, const hkCdBody& cb, const hkProcessCollisionInput &input, hkProcessCollisionOutput& output, 
									  const struct hkExtendedGskOut& cpInfo, hkReal createContactRangeMax, 
									  hkContactMgr* contactMgr, hkCollisionConstraintOwner& constraintOwner, hkAgentContactPoint* contactPoints, int& numPoints )
{
	const hkReal samePointTolerance1 = WELD_TOLERANCE;
	const hkReal samePointToleranceSquared = samePointTolerance1  * samePointTolerance1 ;

	hkReal shortestDist = samePointToleranceSquared;
	int nearestPoint = -1;
	// search the closest point
	int i;
	for( i = 0; i < numPoints; i++ )
	{
		hkAgentContactPoint &acp = contactPoints[i];

		hkVector4 diff;	diff.setSub4( acp.m_pointA, cpInfo.m_pointAinA );
		const hkReal distToManiPoint = diff.dot3(diff);
		if( distToManiPoint < shortestDist )
		{
			const hkReal normalTolerance = 0.1f * 0.1f;
			hkVector4 normalDiff; normalDiff.setSub4( acp.m_normal, cpInfo.m_normalInWorld );

			//
			// This line has to be in currently in order for the convex welder shape to work
			// However, it contributes to the problem with objects "hanging" if they rotate slowly around a 90 degree
			// edge in a triangle mesh (AB 9-6-03). 
			//
			if( (hkReal)(normalDiff.dot3(normalDiff)) < normalTolerance )
			{
				shortestDist = distToManiPoint;
				nearestPoint = i;
			}
		}
	}
	if ( nearestPoint >= 0 )
	{
		hkAgentContactPoint &acp = contactPoints[nearestPoint];
		hkReal id = acp.m_pointB(3);
		acp.m_pointA = cpInfo.m_pointAinA;
		acp.m_pointB = cpInfo.m_pointBinB;
		acp.m_normal = cpInfo.m_normalInWorld;
		acp.getDistance() = cpInfo.m_distance;
		acp.getPointAWeight() = cpInfo.m_pointAWeight;
		acp.m_pointB(3) = id;
		// this point is already in the manifold
		return;
	}

	//
	//	Check our create range distance
	//
	if ( cpInfo.m_distance > createContactRangeMax )
	{
		return;
	}

	// ok, we've got a new point
	
	{
		hkAgentContactPoint bufferPoint;
		hkAgentContactPoint &acp = (i<4) ? contactPoints[i] : bufferPoint;
		acp.m_pointA = cpInfo.m_pointAinA;
		acp.m_pointB = cpInfo.m_pointBinB;
		acp.m_normal = cpInfo.m_normalInWorld;
		acp.getPointAWeight() = cpInfo.m_pointAWeight;
		acp.getDistance() = cpInfo.m_distance;

		hkProcessCdPoint ccp;
		const hkTransform& wTa = ca.getTransform();
		const hkTransform& wTb = cb.getTransform();

		hkVector4 paw, pbw;
		paw.setTransformedPos( wTa, acp.m_pointA );
		pbw.setTransformedPos( wTb, acp.m_pointB );

		hkVector4 pos; pos.setInterpolate4( pbw, paw, cpInfo.m_pointAWeight );
		//pos.addMul4( 1.0f - cpInfo.m_pointAWeight, acp.m_normal );
		
		ccp.m_contact.getPosition() = pos;
		ccp.m_contact.setSeparatingNormal( acp.getSeparatingNormal()  );

		hkContactPointId newPointId = contactMgr->addContactPoint(	ca, cb, input, output, HK_NULL, ccp.m_contact );
		if ( newPointId != HK_INVALID_CONTACT_POINT )
		{
			acp.setContactPointId( newPointId );

			if ( numPoints == 4 )
			{
				const hkVector4* points[5];
				points[0] = &(contactPoints[0].m_pointA);
				points[1] = &(contactPoints[1].m_pointA);
				points[2] = &(contactPoints[2].m_pointA);
				points[3] = &(contactPoints[3].m_pointA);
				points[4] = &(acp.m_pointA);

				int indexToRemove = findRedundant5thPoint( points );
				if ( indexToRemove < 4)
				{
					contactMgr->removeContactPoint( hkContactPointId(contactPoints[indexToRemove].getContactPointId()), constraintOwner );
					contactPoints[indexToRemove] = acp;
				}
				else
				{
					contactMgr->removeContactPoint( newPointId, constraintOwner );
				}

			}
			else
			{
				numPoints++;
			}
		}
	}
}


void hkClosestPointManifold::getPoints(const hkCdBody& ca, const hkCdBody& cb, const hkProcessCollisionInput &input, hkReal dist, hkAgentContactPoint* contactPoints, int& numPoints, hkProcessCollisionOutput& contactPointsOut, hkContactMgr* contactMgr, hkCollisionConstraintOwner& constraintOwner  )
{
	const hkReal tolerance2d = DRIFT_TOLERANCE;
	const hkReal sqTolerance2d = tolerance2d * tolerance2d;

	int i = 0;

	hkProcessCdPoint* contactPoint = contactPointsOut.reserveContactPoints( numPoints );

	while( i < numPoints )
	{
		hkAgentContactPoint &manifoldPoint = contactPoints[i];

//		HK_ASSERT( manifoldPoint.getContactPointId() == manifoldPoint.m_contactPointId);
//		HK_ASSERT( manifoldPoint.getPointAWeight() >= 0.0f && manifoldPoint.getPointAWeight() <= 1.0f);
//		HK_ASSERT( manifoldPoint.getDistance() > -0.5f && manifoldPoint.getDistance() < 0.5f);
//		HK_ASSERT ( hkMath::fabs( hkReal(manifoldPoint.m_normal.length3())-1.0f) < 0.01f );

		const hkTransform& wTa = ca.getTransform();
		const hkTransform& wTb = cb.getTransform();


		hkVector4 pA; pA._setTransformedPos( wTa, manifoldPoint.m_pointA );
		hkVector4 pB; pB._setTransformedPos( wTb, manifoldPoint.m_pointB );


		hkVector4 vec;	vec.setSub4(pA, pB);
		const hkReal distance = vec.dot3(manifoldPoint.m_normal);
		
		hkVector4 projectedPoint;
		projectedPoint.setAddMul4( pB, manifoldPoint.m_normal, distance );

		hkVector4 vec2d;	vec2d.setSub4( pA, projectedPoint );

		const hkReal distance2d = vec2d.lengthSquared3();

		if(( distance < input.getTolerance()) && ( distance > dist ) && (distance2d < sqTolerance2d))
		{
			//
			// The point is valid. Fill in the details
			//
			hkVector4 pos; pos.setInterpolate4( pB, pA, manifoldPoint.getPointAWeight() );
		
			contactPoint->m_contact.getPosition() = pos;
			contactPoint->m_contact.setSeparatingNormal( manifoldPoint.m_normal, distance );
			contactPoint->m_contactPointId = hkContactPointId(manifoldPoint.getContactPointId());
			i++;
			contactPoint++;
			contactPointsOut.commitContactPoints(1);
		}
		else
		{
			contactMgr->removeContactPoint( hkContactPointId(manifoldPoint.getContactPointId()), constraintOwner );
			numPoints--;
			manifoldPoint = contactPoints[numPoints];
			contactPointsOut.abortContactPoints(1);
		}
	}
}

/*
* Havok SDK - CLIENT RELEASE, BUILD(#20060902)
*
* Confidential Information of Havok.  (C) Copyright 1999-2006 
* Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
* Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
* rights, and intellectual property rights in the Havok software remain in
* Havok and/or its suppliers.
*
* Use of this software for evaluation purposes is subject to and indicates 
* acceptance of the End User licence Agreement for this product. A copy of 
* the license is included with this software and is also available from salesteam@havok.com.
*
*/
