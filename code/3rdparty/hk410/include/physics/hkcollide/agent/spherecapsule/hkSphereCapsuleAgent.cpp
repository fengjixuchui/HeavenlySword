/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>

#include <hkmath/linear/hkVector4Util.h>

#include <hkcollide/shape/sphere/hkSphereShape.h>
#include <hkcollide/shape/capsule/hkCapsuleShape.h>
#include <hkcollide/agent/spherecapsule/hkSphereCapsuleAgent.h>
#include <hkinternal/collide/util/hkCollideTriangleUtil.h>


hkSphereCapsuleAgent::hkSphereCapsuleAgent( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr )
: hkIterativeLinearCastAgent( mgr )
{
	m_contactPointId = HK_INVALID_CONTACT_POINT;
}

void HK_CALL hkSphereCapsuleAgent::registerAgent(hkCollisionDispatcher* dispatcher)
{
	// register symmetric version
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc          = createCapsuleSphereAgent;
		af.m_getPenetrationsFunc  = hkSymmetricAgent<hkSphereCapsuleAgent>::staticGetPenetrations;
		af.m_getClosestPointFunc = hkSymmetricAgent<hkSphereCapsuleAgent>::staticGetClosestPoints;
		af.m_linearCastFunc      = hkSymmetricAgent<hkSphereCapsuleAgent>::staticLinearCast;
		af.m_isFlipped           = true;
		af.m_isPredictive		 = false;
	    dispatcher->registerCollisionAgent(af, HK_SHAPE_CAPSULE, HK_SHAPE_SPHERE);	
	}
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc          = createSphereCapsuleAgent;
		af.m_getPenetrationsFunc  = staticGetPenetrations;
		af.m_getClosestPointFunc = staticGetClosestPoints;
		af.m_linearCastFunc      = staticLinearCast;
		af.m_isFlipped           = false;
		af.m_isPredictive		 = false;
	    dispatcher->registerCollisionAgent(af, HK_SHAPE_SPHERE, HK_SHAPE_CAPSULE);	
	}
}


hkCollisionAgent* HK_CALL hkSphereCapsuleAgent::createCapsuleSphereAgent(const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr)
{
	hkSphereCapsuleAgent* agent = new hkSymmetricAgentLinearCast<hkSphereCapsuleAgent>(bodyA, bodyB, input, mgr);
	return agent;
}


hkCollisionAgent* HK_CALL hkSphereCapsuleAgent::createSphereCapsuleAgent(const  hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr)
{
    return new hkSphereCapsuleAgent( bodyA, bodyB, input, mgr );
}

void hkSphereCapsuleAgent::cleanup(hkCollisionConstraintOwner& constraintOwner)
{
	if(m_contactPointId != HK_INVALID_CONTACT_POINT)
	{
		m_contactMgr->removeContactPoint(m_contactPointId, constraintOwner );
	}
	delete this;
}

hkSphereCapsuleAgent::ClosestPointResult hkSphereCapsuleAgent::getClosestPointInl(const  hkCdBody& bodyA, const hkCdBody& bodyB,	const hkCollisionInput& input, hkContactPoint& cpoint )
{
    const hkSphereShape* sphereA = static_cast<const hkSphereShape*>(bodyA.getShape());
    const hkCapsuleShape* capsuleB = static_cast<const hkCapsuleShape*>(bodyB.getShape());

    const hkVector4& posA = bodyA.getTransform().getTranslation();

	hkVector4 capsB[2];
	hkVector4Util::transformPoints( bodyB.getTransform(), capsuleB->getVertices(), 2, &capsB[0]);

	hkCollideTriangleUtil::ClosestPointLineSegResult result;
	hkCollideTriangleUtil::closestPointLineSeg( posA, capsB[0], capsB[1], result );

	hkVector4 aMinusB; aMinusB.setSub4( posA, result.m_pointOnEdge );

	const hkReal radiusSum = sphereA->getRadius() + capsuleB->getRadius();
	const hkReal refDist = radiusSum + input.getTolerance();

	hkReal distSquared = aMinusB.lengthSquared3();
	if ( distSquared >= refDist * refDist )
	{
		return ST_CP_MISS;
	}

	hkReal dist;
	if ( distSquared > 0.0f )
	{
		dist = hkMath::sqrt( distSquared );
		cpoint.getSeparatingNormal() = aMinusB;
	}
	else
	{
		dist = 0.0f;
		hkVector4 edge; edge.setSub4( capsB[1], capsB[0] );
		hkVector4Util::calculatePerpendicularVector( edge, cpoint.getSeparatingNormal() );
	}
	cpoint.getSeparatingNormal().normalize3();
	cpoint.getPosition().setAddMul4( posA, cpoint.getNormal(), capsuleB->getRadius() - dist );
	cpoint.setDistance( dist - radiusSum );
	return ST_CP_HIT;
}

hkSphereCapsuleAgent::ClosestPointResult HK_CALL hkSphereCapsuleAgent::getClosestPoint(const  hkCdBody& bodyA, const hkCdBody& bodyB,	const hkCollisionInput& input, hkContactPoint& cpoint )
{
	return getClosestPointInl( bodyA, bodyB, input, cpoint );
}





void hkSphereCapsuleAgent::processCollision(const  hkCdBody& bodyA,  const hkCdBody& bodyB, const hkProcessCollisionInput& input, hkProcessCollisionOutput& result)
{
	HK_ASSERT2(0x1beb1a11,  m_contactMgr, HK_MISSING_CONTACT_MANAGER_ERROR_TEXT );

	HK_TIMER_BEGIN("SphereCapsule", HK_NULL);

	hkProcessCdPoint& point = *result.reserveContactPoints(1);

	if (getClosestPointInl( bodyA, bodyB, input, point.m_contact) != ST_CP_MISS)
	{
		if(m_contactPointId == HK_INVALID_CONTACT_POINT)
		{
			m_contactPointId = m_contactMgr->addContactPoint(bodyA, bodyB, input, result, HK_NULL, point.m_contact );
		}

		if ( m_contactPointId != HK_INVALID_CONTACT_POINT )
		{
			point.m_contactPointId = m_contactPointId;
			result.commitContactPoints(1);
		}
		else
		{
			result.abortContactPoints(1);
		}

	}
	else
	{
		result.abortContactPoints(1);
		if(m_contactPointId != HK_INVALID_CONTACT_POINT)
		{
			m_contactMgr->removeContactPoint(m_contactPointId, *result.m_constraintOwner );
			m_contactPointId = HK_INVALID_CONTACT_POINT;
		}
	}

	HK_TIMER_END();
}

void hkSphereCapsuleAgent::getClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& collector)
{
	HK_TIMER_BEGIN("SphereCapsule", HK_NULL);

	hkCdPoint event( bodyA, bodyB );

	if (getClosestPointInl( bodyA, bodyB, input, event.m_contact))
	{
		collector.addCdPoint( event );
	}

	HK_TIMER_END();
}

void hkSphereCapsuleAgent::staticGetClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& collector)
{
	HK_TIMER_BEGIN("SphereCapsule", HK_NULL);
	
	hkCdPoint event( bodyA, bodyB );

	if (getClosestPointInl( bodyA, bodyB, input, event.m_contact))
	{
		collector.addCdPoint( event );
	}

	HK_TIMER_END();
}
	
void hkSphereCapsuleAgent::staticGetPenetrations(const  hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
{
	HK_TIMER_BEGIN("SphereCapsule", HK_NULL);

    const hkSphereShape* sphereA = static_cast<const hkSphereShape*>(bodyA.getShape());
    const hkCapsuleShape* capsuleB = static_cast<const hkCapsuleShape*>(bodyB.getShape());

    const hkVector4& posA = bodyA.getTransform().getTranslation();

	hkVector4 capsB[2]; hkVector4Util::transformPoints( bodyB.getTransform(), capsuleB->getVertices(), 2, &capsB[0]);

	hkCollideTriangleUtil::ClosestPointLineSegResult result;
	hkCollideTriangleUtil::closestPointLineSeg( posA, capsB[0], capsB[1], result );

	hkVector4 aMinusB; aMinusB.setSub4( result.m_pointOnEdge, posA );

	const hkReal radiusSum = sphereA->getRadius() + capsuleB->getRadius();
	const hkReal refDist = radiusSum;

	hkReal distSquared = aMinusB.lengthSquared3();
	if ( distSquared < refDist * refDist )
	{
		collector.addCdBodyPair( bodyA, bodyB );
	}
    HK_TIMER_END();
}


void hkSphereCapsuleAgent::getPenetrations(const  hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
{
	staticGetPenetrations( bodyA, bodyB, input, collector );
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
