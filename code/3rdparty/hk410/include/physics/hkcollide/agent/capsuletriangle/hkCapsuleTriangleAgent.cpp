/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>

#include <hkmath/linear/hkVector4Util.h>

#include <hkcollide/shape/capsule/hkCapsuleShape.h>
#include <hkcollide/shape/triangle/hkTriangleShape.h>
#include <hkcollide/agent/capsuletriangle/hkCapsuleTriangleAgent.h>
#include <hkinternal/collide/util/hkCollideCapsuleUtil.h>

hkCapsuleTriangleAgent::hkCapsuleTriangleAgent( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr )
: hkIterativeLinearCastAgent( mgr )
{
	m_contactPointId[0] = HK_INVALID_CONTACT_POINT;
	m_contactPointId[1] = HK_INVALID_CONTACT_POINT;
	m_contactPointId[2] = HK_INVALID_CONTACT_POINT;
    const hkTriangleShape* triB = static_cast<const hkTriangleShape*>(bodyB.getShape());
	hkCollideTriangleUtil::setupPointTriangleDistanceCache( triB->getVertices(), m_triangleCache );
}

void HK_CALL hkCapsuleTriangleAgent::registerAgent(hkCollisionDispatcher* dispatcher)
{
	// register symmetric version
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc          = createTriangleCapsuleAgent;
		af.m_getPenetrationsFunc  = hkSymmetricAgent<hkCapsuleTriangleAgent>::staticGetPenetrations;
		af.m_getClosestPointFunc  = hkSymmetricAgent<hkCapsuleTriangleAgent>::staticGetClosestPoints;
		af.m_linearCastFunc       = hkSymmetricAgent<hkCapsuleTriangleAgent>::staticLinearCast;
		af.m_isFlipped           = true;
		af.m_isPredictive		 = false;
	    dispatcher->registerCollisionAgent(af, HK_SHAPE_TRIANGLE, HK_SHAPE_CAPSULE);
	}
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc          = createCapsuleTriangleAgent;
		af.m_getPenetrationsFunc  = staticGetPenetrations;
		af.m_getClosestPointFunc = staticGetClosestPoints;
		af.m_linearCastFunc      = staticLinearCast;
		af.m_isFlipped           = false;
		af.m_isPredictive		 = false;
	    dispatcher->registerCollisionAgent(af, HK_SHAPE_CAPSULE, HK_SHAPE_TRIANGLE);
	}
}


hkCollisionAgent* HK_CALL hkCapsuleTriangleAgent::createTriangleCapsuleAgent(const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr)
{
	hkCapsuleTriangleAgent* agent = new hkSymmetricAgentLinearCast<hkCapsuleTriangleAgent>(bodyA, bodyB, input, mgr);
	return agent;
}


hkCollisionAgent* HK_CALL hkCapsuleTriangleAgent::createCapsuleTriangleAgent(const  hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr)
{
    return new hkCapsuleTriangleAgent( bodyA, bodyB, input, mgr );
}

// hkAgent interface implementation
void hkCapsuleTriangleAgent::cleanup(hkCollisionConstraintOwner& constraintOwner)
{
	for (int i = 0; i < 3; i++ )
	{
		if(m_contactPointId[i] != HK_INVALID_CONTACT_POINT)
		{
			m_contactMgr->removeContactPoint(m_contactPointId[i], constraintOwner );
		}
	}
	delete this;
}

// note the searchManifold parameter had to be made int due to an Internal Compiler Error in gcc 2.95.3 when using hkBool
void hkCapsuleTriangleAgent::getClosestPointsInl( const  hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCollideTriangleUtil::PointTriangleDistanceCache& cache, int searchManifold, hkContactPoint* points )
{
    const hkCapsuleShape* capsuleA = static_cast<const hkCapsuleShape*>(bodyA.getShape());
    const hkTriangleShape* triB = static_cast<const hkTriangleShape*>(bodyB.getShape());

	hkVector4 endPoints[2];

	hkVector4Util::transformPoints( bodyA.getTransform(), capsuleA->getVertices(), 2, endPoints );

	hkVector4 triVertices[3];
	hkVector4Util::transformPoints( bodyB.getTransform(), triB->getVertices(), 3, &triVertices[0]);

	hkCollideCapsuleUtilCapsVsTri( endPoints, capsuleA->getRadius(), triVertices, triB->getRadius(), cache, input.getTolerance(), searchManifold, points );

}

// note the searchManifold parameter had to be made int due to an Internal Compiler Error in gcc 2.95.3 when using hkBool
void hkCapsuleTriangleAgent::getClosestPointsPublic( const  hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCollideTriangleUtil::PointTriangleDistanceCache& cache, int searchManifold, hkContactPoint* points )
{
	getClosestPointsInl( bodyA, bodyB, input, cache, searchManifold, points );
}

hkCapsuleTriangleAgent::ClosestPointResult hkCapsuleTriangleAgent::getClosestPointInternal(const  hkCdBody& bodyA, const hkCdBody& bodyB,	const hkCollisionInput& input, hkCollideTriangleUtil::PointTriangleDistanceCache& cache, hkContactPoint& cpoint )
{
	hkContactPoint points[3];
	getClosestPointsInl( bodyA, bodyB, input, cache, false, points );

	hkReal dist0 = points[0].getDistance();
	hkReal dist1 = points[1].getDistance();
	if ( dist0 < dist1 )
	{
		if ( dist0 < input.getTolerance() )
		{
			cpoint = points[0];
			return ST_CP_HIT;
		}
	}
	else
	{
		if ( dist1 < input.getTolerance() )
		{
			cpoint = points[1];
			return ST_CP_HIT;
		}
	}
	return ST_CP_MISS;
}

hkCapsuleTriangleAgent::ClosestPointResult HK_CALL hkCapsuleTriangleAgent::getClosestPoint(const  hkCdBody& bodyA, const hkCdBody& bodyB,	const hkCollisionInput& input, hkCollideTriangleUtil::PointTriangleDistanceCache& cache, hkContactPoint& cpoint )
{
	return getClosestPointInternal( bodyA, bodyB, input, cache, cpoint );
}





void hkCapsuleTriangleAgent::processCollision(const  hkCdBody& bodyA,  const hkCdBody& bodyB, const hkProcessCollisionInput& input, hkProcessCollisionOutput& result)
{
	HK_ASSERT2(0x4d200eea,  m_contactMgr, HK_MISSING_CONTACT_MANAGER_ERROR_TEXT );

	HK_TIMER_BEGIN("CapsuleTri", HK_NULL);

	hkContactPoint points[3];
	getClosestPointsInl( bodyA, bodyB, input, m_triangleCache, true, points );
	{
		for (int p = 0; p < 3; p++ )
		{
			if ( points[p].getDistance() < input.getTolerance() )
			{
				if(m_contactPointId[p] == HK_INVALID_CONTACT_POINT)
				{
					m_contactPointId[p] = m_contactMgr->addContactPoint(bodyA, bodyB, input, result, HK_NULL, points[p] );
				}

				if ( m_contactPointId[p] != HK_INVALID_CONTACT_POINT )
				{
					hkProcessCdPoint& point = *result.reserveContactPoints(1);
					result.commitContactPoints(1);
					point.m_contact.getPosition()        = points[p].getPosition();
					point.m_contact.getSeparatingNormal() = points[p].getSeparatingNormal();
					point.m_contactPointId = m_contactPointId[p];
				}
			}
			else
			{
				if(m_contactPointId[p] != HK_INVALID_CONTACT_POINT)
				{
					m_contactMgr->removeContactPoint( m_contactPointId[p], *result.m_constraintOwner );
					m_contactPointId[p] = HK_INVALID_CONTACT_POINT;
				}
			}
		}
	}

	HK_TIMER_END();
}

void hkCapsuleTriangleAgent::getClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& collector)
{
	HK_TIMER_BEGIN("CapsTriangle", HK_NULL);

	hkCdPoint event( bodyA, bodyB );

	if (getClosestPointInternal( bodyA, bodyB, input, m_triangleCache, event.m_contact))
	{
		collector.addCdPoint( event );
	}

	HK_TIMER_END();
}

void hkCapsuleTriangleAgent::staticGetClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& collector)
{
	HK_TIMER_BEGIN("CapsTriangle", HK_NULL);

	hkCollideTriangleUtil::PointTriangleDistanceCache cache;
	{
		const hkTriangleShape* triB = static_cast<const hkTriangleShape*>(bodyB.getShape());
		hkCollideTriangleUtil::setupPointTriangleDistanceCache( triB->getVertices(), cache );
	}

	hkCdPoint event( bodyA, bodyB );

	if (getClosestPointInternal( bodyA, bodyB, input, cache, event.m_contact))
	{
		collector.addCdPoint( event );
	}

	HK_TIMER_END();
}


void hkCapsuleTriangleAgent::getPenetrations(const  hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
{
	HK_TIMER_BEGIN("CapsTriangle", HK_NULL);
 	hkContactPoint points[3];
	getClosestPointsInl( bodyA, bodyB, input, m_triangleCache, false, points );

	hkReal dist0 = points[0].getDistance();
	hkReal dist1 = points[1].getDistance();
	if ( dist0 < 0.f || dist1 < .0f )
	{
		collector.addCdBodyPair( bodyA, bodyB );
	}

    HK_TIMER_END();

}


void hkCapsuleTriangleAgent::staticGetPenetrations(const  hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
{
	HK_TIMER_BEGIN("CapsTriangle", HK_NULL);
	hkCollideTriangleUtil::PointTriangleDistanceCache cache;
	{
		const hkTriangleShape* triB = static_cast<const hkTriangleShape*>(bodyB.getShape());
		hkCollideTriangleUtil::setupPointTriangleDistanceCache( triB->getVertices(), cache );
	}
	hkContactPoint points[3];
	getClosestPointsInl( bodyA, bodyB, input, cache, false, points );

	hkReal dist0 = points[0].getDistance();
	hkReal dist1 = points[1].getDistance();
	if ( dist0 < 0.f || dist1 < 0.f )
	{
		collector.addCdBodyPair( bodyA, bodyB );
	}

	HK_TIMER_END();

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
