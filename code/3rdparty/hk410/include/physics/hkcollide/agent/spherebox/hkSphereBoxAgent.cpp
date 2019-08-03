/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>

#include <hkcollide/shape/sphere/hkSphereShape.h>
#include <hkcollide/shape/box/hkBoxShape.h>
#include <hkcollide/agent/spherebox/hkSphereBoxAgent.h>
#include <hkmath/linear/hkVector4Util.h>

void HK_CALL hkSphereBoxAgent::registerAgent(hkCollisionDispatcher* dispatcher)
{
	// register symmetric version
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc          = createBoxSphereAgent;
		af.m_getPenetrationsFunc  = hkSymmetricAgent<hkSphereBoxAgent>::staticGetPenetrations;
		af.m_getClosestPointFunc = hkSymmetricAgent<hkSphereBoxAgent>::staticGetClosestPoints;
		af.m_linearCastFunc      = hkSymmetricAgent<hkSphereBoxAgent>::staticLinearCast;
		af.m_isFlipped           = true;
		af.m_isPredictive		 = false;
	    dispatcher->registerCollisionAgent(af, HK_SHAPE_BOX, HK_SHAPE_SPHERE);	
	}
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc          = createSphereBoxAgent;
		af.m_getPenetrationsFunc  = staticGetPenetrations;
		af.m_getClosestPointFunc = staticGetClosestPoints;
		af.m_linearCastFunc      = staticLinearCast;
		af.m_isFlipped           = false;
		af.m_isPredictive		 = false;
	    dispatcher->registerCollisionAgent(af, HK_SHAPE_SPHERE, HK_SHAPE_BOX);	
	}

}



hkCollisionAgent* HK_CALL hkSphereBoxAgent::createBoxSphereAgent(const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr)
{
	hkSphereBoxAgent* agent = new hkSymmetricAgentLinearCast<hkSphereBoxAgent>(mgr);
	return agent;
}


hkCollisionAgent* HK_CALL hkSphereBoxAgent::createSphereBoxAgent(const  hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr)
{
    return new hkSphereBoxAgent( mgr );
}


void hkSphereBoxAgent::cleanup( hkCollisionConstraintOwner& constraintOwner )
{
	if(m_contactPointId != HK_INVALID_CONTACT_POINT)
	{
		m_contactMgr->removeContactPoint(m_contactPointId, constraintOwner );
	}

	delete this;
}



hkBool  hkSphereBoxAgent::getClosestPoint(const  hkCdBody& bodyA, const hkCdBody& bodyB,  const hkCollisionInput& input, hkContactPoint& contactOut)
{

    const hkVector4& posA = bodyA.getTransform().getTranslation();
	hkVector4 posLocalB; posLocalB.setTransformedInversePos( bodyB.getTransform(), posA );

    const hkBoxShape* boxB = static_cast<const hkBoxShape*>(bodyB.getShape());
    const hkSphereShape* sphereA = static_cast<const hkSphereShape*>(bodyA.getShape());

	hkVector4 absPosB; absPosB.setAbs4( posLocalB );

	hkVector4 clippedAbsPos; clippedAbsPos.setMin4( absPosB, boxB->getHalfExtents() );
	hkVector4 delta; delta.setSub4( clippedAbsPos, absPosB );	

	int flags = delta.compareLessThanZero4();
	hkReal distance;
	if ( flags & HK_VECTOR3_COMPARE_MASK_XYZ )
	{
		//
		// Now we are outside
		//
		distance = delta.normalizeWithLength3();
		distance -= sphereA->getRadius() + boxB->getRadius();
		if ( distance > input.getTolerance())
		{
			return false;
		}
		hkVector4Util::mulSigns4(delta, posLocalB);
		delta.setNeg4( delta );
		contactOut.getSeparatingNormal()._setRotatedDir( bodyB.getTransform().getRotation(), delta );

	}
	else
	{
		//
		// Completely inside, search the smallest pentration
		//
		hkReal d0 = absPosB(0) - boxB->getHalfExtents()(0);
		hkReal d1 = absPosB(1) - boxB->getHalfExtents()(1);
		hkReal d2 = absPosB(2) - boxB->getHalfExtents()(2);

		int i;
		if ( d0 > d1 )
		{
			if ( d0 > d2 )
			{
				i = 0;
				contactOut.setNormal ( bodyB.getTransform().getColumn(0) );
				distance = d0;
			}
			else
			{
				i = 2;
				contactOut.setNormal ( bodyB.getTransform().getColumn(2) );
				distance = d2;
			}
		}
		else
		{
			if ( d1 > d2 )
			{
				i = 1;
				contactOut.setNormal ( bodyB.getTransform().getColumn(1) );
				distance = d1;
			}
			else
			{
				i = 2;
				contactOut.setNormal ( bodyB.getTransform().getColumn(2) );
				distance = d2;
			}
		}
		if ( posLocalB(i) < 0)
		{
			contactOut.getSeparatingNormal().setNeg4( contactOut.getSeparatingNormal() );
		}
		distance = distance  - boxB->getRadius() - sphereA->getRadius();
	}
	contactOut.getPosition().setAddMul4( posA, contactOut.getNormal(), - distance - sphereA->getRadius() );
	contactOut.setDistance( distance );
	return true;
}



void hkSphereBoxAgent::processCollision(const  hkCdBody& bodyA,  const hkCdBody& bodyB, const hkProcessCollisionInput& input, hkProcessCollisionOutput& result)
{
	HK_ASSERT2(0x4944b7cf,  m_contactMgr, HK_MISSING_CONTACT_MANAGER_ERROR_TEXT );

	HK_TIMER_BEGIN("SphereBox", HK_NULL);

	hkProcessCdPoint& point = *result.reserveContactPoints(1);

	if (getClosestPoint( bodyA, bodyB, input, point.m_contact))
	{
		if(m_contactPointId == HK_INVALID_CONTACT_POINT)
		{
			m_contactPointId = m_contactMgr->addContactPoint(bodyA, bodyB, input, result, HK_NULL, point.m_contact );
		}

		if (m_contactPointId != HK_INVALID_CONTACT_POINT)
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

void hkSphereBoxAgent::getClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& collector)
{
	HK_TIMER_BEGIN("SphereBox", HK_NULL);

	hkCdPoint event( bodyA, bodyB );
	
	if (getClosestPoint( bodyA, bodyB, input, event.m_contact))
	{
		collector.addCdPoint( event );
	}

	HK_TIMER_END();
}

void hkSphereBoxAgent::staticGetClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& collector)
{
	HK_TIMER_BEGIN("SphereBox", HK_NULL);

	hkCdPoint event( bodyA, bodyB );
	
	if (getClosestPoint( bodyA, bodyB, input, event.m_contact))
	{
		collector.addCdPoint( event );
	}

	HK_TIMER_END();
}
	
void hkSphereBoxAgent::staticGetPenetrations(const  hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
{
	HK_TIMER_BEGIN("SphereBox", HK_NULL);

    const hkSphereShape* sphereA = static_cast<const hkSphereShape*>(bodyA.getShape());
    const hkBoxShape* boxB = static_cast<const hkBoxShape*>(bodyB.getShape());

    const hkVector4& posA = bodyA.getTransform().getTranslation();

	hkVector4 posLocalB; posLocalB.setTransformedInversePos( bodyB.getTransform(), posA );
	hkVector4 absPosB; absPosB.setAbs4( posLocalB );
	absPosB.setMin4( absPosB, boxB->getHalfExtents() );

	hkVector4Util::mulSigns4(absPosB, posLocalB);
	hkVector4 posB; posB._setTransformedPos( bodyB.getTransform(), absPosB );

    hkVector4 vec;    vec.setSub4( posB, posA );

    const hkReal distSquared = vec.dot3(vec);
    const hkReal radiusSum = sphereA->getRadius() + boxB->getRadius();
	if (distSquared < radiusSum * radiusSum)
	{
		collector.addCdBodyPair( bodyA, bodyB );
	}
 
    HK_TIMER_END();

}

void hkSphereBoxAgent::getPenetrations(const  hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
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
