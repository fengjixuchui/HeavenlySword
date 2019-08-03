/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>
#include <hkcollide/shape/capsule/hkCapsuleShape.h>
#include <hkcollide/shape/triangle/hkTriangleShape.h>
#include <hkcollide/agent/capsulecapsule/hkCapsuleCapsuleAgent.h>
#include <hkmath/linear/hkVector4Util.h>
#include <hkinternal/collide/util/hkCollideCapsuleUtil.h>

#include <hkbase/htl/hkAlgorithm.h>

hkCapsuleCapsuleAgent::hkCapsuleCapsuleAgent( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr )
: hkIterativeLinearCastAgent( mgr )
{
	m_contactPointId[0] = HK_INVALID_CONTACT_POINT;
	m_contactPointId[1] = HK_INVALID_CONTACT_POINT;
	m_contactPointId[2] = HK_INVALID_CONTACT_POINT;
}

hkCollisionAgent* HK_CALL hkCapsuleCapsuleAgent::createCapsuleCapsuleAgent(const  hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr)
{
    return new hkCapsuleCapsuleAgent( bodyA, bodyB, input, mgr );
}


void HK_CALL hkCapsuleCapsuleAgent::registerAgent(hkCollisionDispatcher* dispatcher)
{
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc          = createCapsuleCapsuleAgent;
		af.m_getPenetrationsFunc = staticGetPenetrations;
		af.m_getClosestPointFunc = staticGetClosestPoints;
		af.m_linearCastFunc      = staticLinearCast;
		af.m_isFlipped           = false;
		af.m_isPredictive		 = false;
	    dispatcher->registerCollisionAgent(af, HK_SHAPE_CAPSULE, HK_SHAPE_CAPSULE);	
	}
}


// hkAgent interface implementation
void hkCapsuleCapsuleAgent::cleanup(hkCollisionConstraintOwner& constraintOwner)
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



static HK_FORCE_INLINE void HK_CALL getThreeClosestPointsInl( const  hkCdBody& bodyA, const hkCdBody& bodyB,	const hkCollisionInput& input, hkContactPoint* points )
{
	const hkReal maxD = input.getTolerance();
	points[0].setDistance( maxD );
	points[1].setDistance( maxD );
	points[2].setDistance( maxD );

    const hkCapsuleShape* capsuleA = static_cast<const hkCapsuleShape*>(bodyA.getShape());
    const hkCapsuleShape* capsuleB = static_cast<const hkCapsuleShape*>(bodyB.getShape());
	hkVector4 pA[2]; hkVector4Util::transformPoints( bodyA.getTransform(), capsuleA->getVertices(), 2, pA );
	hkVector4 pB[2]; hkVector4Util::transformPoints( bodyB.getTransform(), capsuleB->getVertices(), 2, pB );

	hkCollideCapsuleUtilManifoldCapsVsCaps( pA, capsuleA->getRadius(), pB, capsuleB->getRadius(), points );
}


static HK_FORCE_INLINE hkResult HK_CALL getClosestPointInternal(const  hkCdBody& bodyA, const hkCdBody& bodyB,	const hkCollisionInput& input, hkContactPoint& cpoint )
{
    const hkCapsuleShape* capsuleA = static_cast<const hkCapsuleShape*>(bodyA.getShape());
    const hkCapsuleShape* capsuleB = static_cast<const hkCapsuleShape*>(bodyB.getShape());

	hkVector4 pA[2]; hkVector4Util::transformPoints( bodyA.getTransform(), capsuleA->getVertices(), 2, pA );
	hkVector4 pB[2]; hkVector4Util::transformPoints( bodyB.getTransform(), capsuleB->getVertices(), 2, pB );

	return hkCollideCapsuleUtilClostestPointCapsVsCaps( pA, capsuleA->getRadius(), pB, capsuleB->getRadius(), input.getTolerance(), cpoint );
}





void hkCapsuleCapsuleAgent::processCollision(const  hkCdBody& bodyA,  const hkCdBody& bodyB, const hkProcessCollisionInput& input, hkProcessCollisionOutput& result)
{
	HK_ASSERT2(0x16962f6f,  m_contactMgr, HK_MISSING_CONTACT_MANAGER_ERROR_TEXT );

	HK_TIMER_BEGIN("CapsCaps", HK_NULL);

	hkContactPoint points[3];
	getThreeClosestPointsInl( bodyA, bodyB, input, points );
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
					point.m_contact.getPosition() = points[p].getPosition();
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

void hkCapsuleCapsuleAgent::getClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& collector)
{
	HK_TIMER_BEGIN("CapsCaps", HK_NULL);

	hkCdPoint event( bodyA, bodyB );

	if ( getClosestPointInternal( bodyA, bodyB, input, event.m_contact) == HK_SUCCESS)
	{
		collector.addCdPoint( event );
	}

	HK_TIMER_END();
}

void hkCapsuleCapsuleAgent::staticGetClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& collector)
{
	HK_TIMER_BEGIN("CapsCaps", HK_NULL);


	hkCdPoint event( bodyA, bodyB );

	if (getClosestPointInternal( bodyA, bodyB, input, event.m_contact) == HK_SUCCESS)
	{
		collector.addCdPoint( event );
	}

	HK_TIMER_END();
}
	
void hkCapsuleCapsuleAgent::staticGetPenetrations(const  hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
{
	HK_TIMER_BEGIN("CapsCaps", HK_NULL);

    const hkCapsuleShape* capsuleA = static_cast<const hkCapsuleShape*>(bodyA.getShape());
    const hkCapsuleShape* capsuleB = static_cast<const hkCapsuleShape*>(bodyB.getShape());

	hkVector4 pA[2]; hkVector4Util::transformPoints( bodyA.getTransform(), capsuleA->getVertices(), 2, pA );
	hkVector4 pB[2]; hkVector4Util::transformPoints( bodyB.getTransform(), capsuleB->getVertices(), 2, pB );

	hkVector4 dA; dA.setSub4( pA[1], pA[0] );
	hkVector4 dB; dB.setSub4( pB[1], pB[0] );

	hkCollideTriangleUtil::ClosestLineSegLineSegResult result;
	hkCollideTriangleUtil::closestLineSegLineSeg( pA[0], dA, pB[0], dB, result );

	hkReal radiusSum = capsuleA->getRadius() + capsuleB->getRadius();
	if ( result.m_distanceSquared < radiusSum * radiusSum )
	{
		collector.addCdBodyPair( bodyA, bodyB );
	}

    HK_TIMER_END();
}

void hkCapsuleCapsuleAgent::getPenetrations(const  hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
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
