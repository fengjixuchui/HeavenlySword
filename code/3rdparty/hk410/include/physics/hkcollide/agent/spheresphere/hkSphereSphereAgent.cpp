/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>

#include <hkcollide/shape/sphere/hkSphereShape.h>

#include <hkcollide/agent/spheresphere/hkSphereSphereAgent.h>

hkSphereSphereAgent::hkSphereSphereAgent( hkContactMgr* contactMgr): hkIterativeLinearCastAgent( contactMgr )
{
	m_contactPointId = HK_INVALID_CONTACT_POINT;
}


void HK_CALL hkSphereSphereAgent::registerAgent(hkCollisionDispatcher* dispatcher)
{
	hkCollisionDispatcher::AgentFuncs af;
	af.m_createFunc          = createSphereSphereAgent;
	af.m_getPenetrationsFunc  = staticGetPenetrations;
	af.m_getClosestPointFunc = staticGetClosestPoints;
	af.m_linearCastFunc      = staticLinearCast;

    dispatcher->registerCollisionAgent(af, HK_SHAPE_SPHERE, HK_SHAPE_SPHERE);	
}


hkCollisionAgent* HK_CALL hkSphereSphereAgent::createSphereSphereAgent(const  hkCdBody& bodyA, const hkCdBody& bodyB, 
                                                                const hkCollisionInput& input, hkContactMgr* contactMgr)
{
    return new hkSphereSphereAgent( contactMgr );
}

void hkSphereSphereAgent::cleanup(hkCollisionConstraintOwner& constraintOwner)
{
	if(m_contactPointId != HK_INVALID_CONTACT_POINT)
	{
		m_contactMgr->removeContactPoint(m_contactPointId, constraintOwner );
		m_contactPointId = HK_INVALID_CONTACT_POINT;
	}

	delete this;
}

void hkSphereSphereAgent::processCollision( const hkCdBody& bodyA,  const hkCdBody& bodyB, 
                                            const hkProcessCollisionInput& input, hkProcessCollisionOutput& result)
{
	HK_ASSERT2(0x5103c8d8,  m_contactMgr, HK_MISSING_CONTACT_MANAGER_ERROR_TEXT );
	HK_TIMER_BEGIN("SphereSphere", HK_NULL);

	const hkSphereShape* sphereA = static_cast<const hkSphereShape*>(bodyA.getShape());
    const hkSphereShape* sphereB = static_cast<const hkSphereShape*>(bodyB.getShape());

    const hkVector4& posA = bodyA.getTransform().getTranslation();
    const hkVector4& posB = bodyB.getTransform().getTranslation();

    hkVector4 vec;    vec.setSub4( posA, posB );

    const hkReal distSquared = vec.dot3(vec);
    const hkReal radiusSum = sphereA->getRadius() + sphereB->getRadius() + input.getTolerance();

    if ( distSquared < radiusSum * radiusSum  )
    {
		hkProcessCdPoint& point = *result.reserveContactPoints(1);

		hkReal distance;
		if ( distSquared > 0 )
		{
			hkReal invDist = hkMath::sqrtInverse(distSquared);
			point.m_contact.getSeparatingNormal().setMul4( invDist, vec );
			distance = distSquared * invDist - (sphereA->getRadius() + sphereB->getRadius());
		}
		else
		{
			point.m_contact.getSeparatingNormal() = hkTransform::getIdentity().getColumn(0);
			distance = - (sphereA->getRadius() + sphereB->getRadius());
		}

		point.m_contact.getPosition().setAddMul4(  posB, point.m_contact.getNormal(), sphereB->getRadius() );
		point.m_contact.setDistance( distance );

		if(m_contactPointId == HK_INVALID_CONTACT_POINT)
		{
			m_contactPointId = m_contactMgr->addContactPoint(bodyA, bodyB, input, result, HK_NULL, point.m_contact );
		}
		if (m_contactPointId != HK_INVALID_CONTACT_POINT )
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
		if(m_contactPointId != HK_INVALID_CONTACT_POINT)
		{
			m_contactMgr->removeContactPoint(m_contactPointId, *result.m_constraintOwner );
			m_contactPointId = HK_INVALID_CONTACT_POINT;
		}
    }

	HK_TIMER_END();
}

hkBool hkSphereSphereAgent::getClosestPoint( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactPoint& contact )
{
	const hkSphereShape* sphereA = static_cast<const hkSphereShape*>(bodyA.getShape());
    const hkSphereShape* sphereB = static_cast<const hkSphereShape*>(bodyB.getShape());

    const hkVector4& posA = bodyA.getTransform().getTranslation();
    const hkVector4& posB = bodyB.getTransform().getTranslation();

    hkVector4 vec;    vec.setSub4( posA, posB );

    const hkReal distSquared = vec.dot3(vec);
    const hkReal radiusSum = sphereA->getRadius() + sphereB->getRadius() + input.getTolerance();

    if ( distSquared < radiusSum * radiusSum )
    {
		hkReal distance;

		if (distSquared > 0 )
		{
	        hkReal invDist = hkMath::sqrtInverse(distSquared);
			contact.getSeparatingNormal().setMul4( invDist, vec );
			distance = distSquared * invDist;
		}
		else
		{
			distance = 0.0f;
			contact.setNormal( hkTransform::getIdentity().getColumn(0) );
		}
		distance -= sphereA->getRadius() + sphereB->getRadius();
		contact.getPosition().setAddMul4(  posB, contact.getNormal(), sphereB->getRadius() );
		contact.setDistance( distance );
		return true;
    }
	return false;
}


void hkSphereSphereAgent::getClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& collector)
{
	HK_TIMER_BEGIN("SphereSphere", HK_NULL);

	hkCdPoint event( bodyA, bodyB );
	if (getClosestPoint( bodyA, bodyB, input, event.m_contact ))
	{
		collector.addCdPoint(event);
	}
	HK_TIMER_END();
}

void hkSphereSphereAgent::staticGetClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& collector)
{
	HK_TIMER_BEGIN("SphereSphere", HK_NULL);

	hkCdPoint event( bodyA, bodyB );
	if (getClosestPoint( bodyA, bodyB, input, event.m_contact ))
	{
		collector.addCdPoint(event);
	}
	HK_TIMER_END();
}


void hkSphereSphereAgent::getPenetrations( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
{
	staticGetPenetrations(bodyA, bodyB, input, collector);
}


void hkSphereSphereAgent::staticGetPenetrations( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
{
	HK_TIMER_BEGIN("SphereSphere", HK_NULL);

    const hkSphereShape* sphereA = static_cast<const hkSphereShape*>(bodyA.getShape());
    const hkSphereShape* sphereB = static_cast<const hkSphereShape*>(bodyB.getShape());

    const hkVector4& posA = bodyA.getTransform().getTranslation();
    const hkVector4& posB = bodyB.getTransform().getTranslation();

    hkVector4 vec;
	vec.setSub4( posB, posA );

    const hkReal distSquared = vec.dot3(vec);
    const hkReal radiusSum = sphereA->getRadius() + sphereB->getRadius();

	if( distSquared < radiusSum * radiusSum )
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
