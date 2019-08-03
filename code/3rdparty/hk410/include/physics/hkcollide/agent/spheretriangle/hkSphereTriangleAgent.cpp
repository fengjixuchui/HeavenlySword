/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>

#include <hkcollide/shape/sphere/hkSphereShape.h>
#include <hkcollide/shape/triangle/hkTriangleShape.h>
#include <hkcollide/agent/spheretriangle/hkSphereTriangleAgent.h>
#include <hkmath/linear/hkVector4Util.h>


hkSphereTriangleAgent::hkSphereTriangleAgent( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr )
: hkIterativeLinearCastAgent( mgr )
{
	m_contactPointId = HK_INVALID_CONTACT_POINT;
    const hkTriangleShape* triB = static_cast<const hkTriangleShape*>(bodyB.getShape());
	hkCollideTriangleUtil::setupClosestPointTriangleCache( triB->getVertices(), m_closestPointTriangleCache );
}

void HK_CALL hkSphereTriangleAgent::registerAgent(hkCollisionDispatcher* dispatcher)
{
	// register symmetric version
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc          = createTriangleSphereAgent;
		af.m_getPenetrationsFunc  = hkSymmetricAgent<hkSphereTriangleAgent>::staticGetPenetrations;
		af.m_getClosestPointFunc = hkSymmetricAgent<hkSphereTriangleAgent>::staticGetClosestPoints;
		af.m_linearCastFunc      = hkSymmetricAgent<hkSphereTriangleAgent>::staticLinearCast;
		af.m_isFlipped           = true;
		af.m_isPredictive		 = false;
	    dispatcher->registerCollisionAgent(af, HK_SHAPE_TRIANGLE, HK_SHAPE_SPHERE);
	}
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc          = createSphereTriangleAgent;
		af.m_getPenetrationsFunc  = staticGetPenetrations;
		af.m_getClosestPointFunc = staticGetClosestPoints;
		af.m_linearCastFunc      = staticLinearCast;
		af.m_isFlipped           = false;
		af.m_isPredictive		 = false;
	    dispatcher->registerCollisionAgent(af, HK_SHAPE_SPHERE, HK_SHAPE_TRIANGLE);
	}
}


hkCollisionAgent* HK_CALL hkSphereTriangleAgent::createTriangleSphereAgent(const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr)
{
	hkSphereTriangleAgent* agent = new hkSymmetricAgentLinearCast<hkSphereTriangleAgent>(bodyA, bodyB, input, mgr);
	return agent;
}


hkCollisionAgent* HK_CALL hkSphereTriangleAgent::createSphereTriangleAgent(const  hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr)
{
    return new hkSphereTriangleAgent( bodyA, bodyB, input, mgr );
}

void hkSphereTriangleAgent::cleanup(hkCollisionConstraintOwner& constraintOwner)
{
	if(m_contactPointId != HK_INVALID_CONTACT_POINT)
	{
		m_contactMgr->removeContactPoint(m_contactPointId, constraintOwner );
	}
	delete this;
}


hkSphereTriangleAgent::ClosestPointResult hkSphereTriangleAgent::getClosestPointInl(const  hkCdBody& bodyA, const hkCdBody& bodyB,	const hkCollisionInput& input, hkCollideTriangleUtil::ClosestPointTriangleCache& cache, hkContactPoint& cpoint )
{
    const hkSphereShape* sphereA = static_cast<const hkSphereShape*>(bodyA.getShape());
    const hkTriangleShape* triB = static_cast<const hkTriangleShape*>(bodyB.getShape());

    const hkVector4& posA = bodyA.getTransform().getTranslation();

	hkVector4 triVertices[3];
	hkVector4Util::transformPoints( bodyB.getTransform(), triB->getVertices(), 3, &triVertices[0]);

	hkCollideTriangleUtil::ClosestPointTriangleResult cptr;
	hkCollideTriangleUtil::ClosestPointTriangleStatus res = hkCollideTriangleUtil::closestPointTriangle( posA, &triVertices[0], cache, cptr );

	const hkReal radiusSum = sphereA->getRadius() + triB->getRadius();

	if ( cptr.distance < radiusSum + input.getTolerance() )
	{
		cpoint.getPosition().setAddMul4(  posA, cptr.hitDirection, triB->getRadius() -cptr.distance );
		cpoint.setSeparatingNormal( cptr.hitDirection, cptr.distance - radiusSum );

		if ( res == hkCollideTriangleUtil::HIT_TRIANGLE_FACE )
		{
			return ST_CP_FACE;
		}
		else
		{
			return ST_CP_EDGE;
		}

	}
	return ST_CP_MISS;
}

hkSphereTriangleAgent::ClosestPointResult HK_CALL hkSphereTriangleAgent::getClosestPoint(const  hkCdBody& bodyA, const hkCdBody& bodyB,	const hkCollisionInput& input, hkCollideTriangleUtil::ClosestPointTriangleCache& cache, hkContactPoint& cpoint )
{
	return getClosestPointInl( bodyA, bodyB, input, cache, cpoint );
}

void hkSphereTriangleAgent::processCollision(const  hkCdBody& bodyA,  const hkCdBody& bodyB, const hkProcessCollisionInput& input, hkProcessCollisionOutput& result)
{
	HK_ASSERT2(0x611dfe18,  m_contactMgr, HK_MISSING_CONTACT_MANAGER_ERROR_TEXT );

	HK_TIMER_BEGIN("SphereTri", HK_NULL);

	hkProcessCdPoint& point = *result.reserveContactPoints(1);

	if (getClosestPointInl( bodyA, bodyB, input, m_closestPointTriangleCache, point.m_contact) != ST_CP_MISS)
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

void hkSphereTriangleAgent::getClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& collector)
{
	HK_TIMER_BEGIN("SphereTri", HK_NULL);

	hkCdPoint event( bodyA, bodyB );

	if (getClosestPointInl( bodyA, bodyB, input, m_closestPointTriangleCache, event.m_contact))
	{
		collector.addCdPoint( event );
	}

	HK_TIMER_END();
}

void hkSphereTriangleAgent::staticGetClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& collector)
{
	HK_TIMER_BEGIN("SphereTri", HK_NULL);

	hkCollideTriangleUtil::ClosestPointTriangleCache cache;
	{
		const hkTriangleShape* triB = static_cast<const hkTriangleShape*>(bodyB.getShape());
		hkCollideTriangleUtil::setupClosestPointTriangleCache( triB->getVertices(), cache );
	}

	hkCdPoint event( bodyA, bodyB );

	if (getClosestPointInl( bodyA, bodyB, input, cache, event.m_contact))
	{
		collector.addCdPoint( event );
	}

	HK_TIMER_END();
}


void hkSphereTriangleAgent::getPenetrations(const  hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
{
	HK_TIMER_BEGIN("SphereTri", HK_NULL);

    const hkSphereShape* sphereA = static_cast<const hkSphereShape*>(bodyA.getShape());
    const hkTriangleShape* triB = static_cast<const hkTriangleShape*>(bodyB.getShape());

    const hkVector4& posA = bodyA.getTransform().getTranslation();
	hkVector4 posAinB; posAinB._setTransformedInversePos( bodyB.getTransform(), posA );


	hkCollideTriangleUtil::ClosestPointTriangleResult cptr;
	hkCollideTriangleUtil::closestPointTriangle( posAinB, &triB->getVertex(0), m_closestPointTriangleCache, cptr );

    const hkReal radiusSum = sphereA->getRadius() + triB->getRadius();
	if( cptr.distance < radiusSum)
	{
		collector.addCdBodyPair( bodyA, bodyB );
	}

    HK_TIMER_END();

}


void hkSphereTriangleAgent::staticGetPenetrations(const  hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
{
	HK_TIMER_BEGIN("SphereTri", HK_NULL);

    const hkSphereShape* sphereA = static_cast<const hkSphereShape*>(bodyA.getShape());
    const hkTriangleShape* triB = static_cast<const hkTriangleShape*>(bodyB.getShape());

	hkCollideTriangleUtil::ClosestPointTriangleCache cache;
	hkCollideTriangleUtil::setupClosestPointTriangleCache( triB->getVertices(), cache );

    const hkVector4& posA = bodyA.getTransform().getTranslation();
	hkVector4 posAinB; posAinB._setTransformedInversePos( bodyB.getTransform(), posA );


	hkCollideTriangleUtil::ClosestPointTriangleResult cptr;
	hkCollideTriangleUtil::closestPointTriangle( posAinB, &triB->getVertex(0), cache, cptr );

    const hkReal radiusSum = sphereA->getRadius() + triB->getRadius();
	if( cptr.distance < radiusSum)
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
