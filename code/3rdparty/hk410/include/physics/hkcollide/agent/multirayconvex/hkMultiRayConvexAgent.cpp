/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>
#include <hkcollide/shape/multiray/hkMultiRayShape.h>
#include <hkcollide/agent/multirayconvex/hkMultiRayConvexAgent.h>
#include <hkmath/linear/hkVector4Util.h>
#include <hkcollide/agent/hkCdPointCollector.h>
#include <hkcollide/shape/hkShapeRayCastInput.h>


void HK_CALL hkMultiRayConvexAgent::registerAgent(hkCollisionDispatcher* dispatcher)
{
	// register symmetric version
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc          = createConvexMultiRayAgent;
		af.m_getPenetrationsFunc  = hkSymmetricAgent<hkMultiRayConvexAgent>::staticGetPenetrations;
		af.m_getClosestPointFunc = hkSymmetricAgent<hkMultiRayConvexAgent>::staticGetClosestPoints;
		af.m_linearCastFunc      = hkSymmetricAgent<hkMultiRayConvexAgent>::staticLinearCast;
		af.m_isFlipped           = true;
			// the agent is not really predictive, however there is no fallback available
		af.m_isPredictive		 = true;
		dispatcher->registerCollisionAgent( af, HK_SHAPE_CONVEX, HK_SHAPE_MULTI_RAY );	
	}
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc          = createMultiRayConvexAgent;
		af.m_getPenetrationsFunc  = staticGetPenetrations;
		af.m_getClosestPointFunc = staticGetClosestPoints;
		af.m_linearCastFunc      = staticLinearCast;
		af.m_isFlipped           = false;
			// the agent is not really predictive, however there is no fallback available
		af.m_isPredictive		 = true;
	    dispatcher->registerCollisionAgent( af, HK_SHAPE_MULTI_RAY, HK_SHAPE_CONVEX );	
	}
}


hkMultiRayConvexAgent::hkMultiRayConvexAgent( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr )
: hkIterativeLinearCastAgent(mgr)
{
	const hkMultiRayShape* msA = static_cast<const hkMultiRayShape*>(bodyA.getShape());
	int nRay = msA->getRays().getSize();

	m_contactInfo.setSize(nRay);
	for (int i = 0; i < nRay;i++) 
	{
		m_contactInfo[i].m_contactPointId = HK_INVALID_CONTACT_POINT;
	}
}



hkCollisionAgent* HK_CALL hkMultiRayConvexAgent::createConvexMultiRayAgent(const hkCdBody& bodyA,const  hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr)
{
	return new hkSymmetricAgent<hkMultiRayConvexAgent>(bodyA, bodyB, input, mgr);
}


hkCollisionAgent* HK_CALL hkMultiRayConvexAgent::createMultiRayConvexAgent(const  hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr)
{
    return new hkMultiRayConvexAgent( bodyA, bodyB, input, mgr );
}



void hkMultiRayConvexAgent::cleanup( hkCollisionConstraintOwner& constraintOwner )
{
	int nRay = m_contactInfo.getSize();
	for(int i = 0; i < nRay; ++i)
	{
		if(m_contactInfo[i].m_contactPointId != HK_INVALID_CONTACT_POINT)
		{
			m_contactMgr->removeContactPoint(m_contactInfo[i].m_contactPointId, constraintOwner );
		}
	}

	delete this;
}


void hkMultiRayConvexAgent::getClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, class hkCdPointCollector& collector  )
{
	HK_TIMER_BEGIN("multiRay-cvx", HK_NULL);

    const hkMultiRayShape* msA = static_cast<const hkMultiRayShape*>(bodyA.getShape());
    const hkConvexShape*  convexB = static_cast<const hkConvexShape*>(bodyB.getShape());

	hkTransform bTa;	bTa.setMulInverseMul( bodyB.getTransform(), bodyA.getTransform());

	const hkArray<hkMultiRayShape::Ray>& RaysA = msA->getRays();

	int nRays = RaysA.getSize();

	const hkMultiRayShape::Ray* rayA = RaysA.begin();

	hkCdPoint event( bodyA, bodyB );

	hkShapeRayCastInput rayInput;
	for (int i = 0; i < nRays; i++)
	{
		hkShapeRayCastOutput rayResults;

		rayInput.m_from._setTransformedPos( bTa, rayA->m_start );
		rayInput.m_to._setTransformedPos( bTa, rayA->m_end );
		
		hkBool rayHit = convexB->castRay( rayInput, rayResults);
		if ( rayHit )
		{
			const hkVector4& normal = rayResults.m_normal;

			const hkReal dist = rayResults.m_hitFraction;
			hkVector4 hitpoint;	hitpoint.setInterpolate4(rayInput.m_from,rayInput.m_to, dist );	

			event.m_contact.getPosition()._setTransformedPos( bodyB.getTransform(), hitpoint );
			event.m_contact.getSeparatingNormal()._setRotatedDir( bodyB.getTransform().getRotation(), normal);
			event.m_contact.setDistance( (dist-1.0f) * rayA->m_start(3) + msA->getRayPenetrationDistance() );// + input.m_tolerance;

			collector.addCdPoint(event);
		}
		rayA++;
	}

	HK_TIMER_END();
}

void hkMultiRayConvexAgent::staticGetClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, class hkCdPointCollector& collector  )
{
	HK_TIMER_BEGIN("multiRay-cvx", HK_NULL);

    const hkMultiRayShape* msA = static_cast<const hkMultiRayShape*>(bodyA.getShape());
    const hkConvexShape*  convexB = static_cast<const hkConvexShape*>(bodyB.getShape());

	hkTransform bTa;	bTa.setMulInverseMul( bodyB.getTransform(), bodyA.getTransform());

	const hkArray<hkMultiRayShape::Ray>& RaysA = msA->getRays();

	int nRays = RaysA.getSize();

	const hkMultiRayShape::Ray* rayA = RaysA.begin();

	hkCdPoint event( bodyA, bodyB );

	hkShapeRayCastInput rayInput;
	for (int i = 0; i < nRays; i++)
	{
		hkShapeRayCastOutput rayResults;

		rayInput.m_from._setTransformedPos( bTa, rayA->m_start );
		rayInput.m_to._setTransformedPos( bTa, rayA->m_end );
		
		hkBool rayHit = convexB->castRay( rayInput, rayResults);
		if ( rayHit )
		{
			const hkVector4& normal = rayResults.m_normal;

			const hkReal dist = rayResults.m_hitFraction;
			hkVector4 hitpoint;	hitpoint.setInterpolate4(rayInput.m_from,rayInput.m_to, dist );	

			event.m_contact.getPosition()._setTransformedPos( bodyB.getTransform(), hitpoint );
			event.m_contact.getSeparatingNormal()._setRotatedDir( bodyB.getTransform().getRotation(), normal);
			event.m_contact.setDistance( (dist-1.0f) * rayA->m_start(3) + msA->getRayPenetrationDistance() );// + input.m_tolerance;

			collector.addCdPoint(event);
		}
		rayA++;
	}

	HK_TIMER_END();
}
	
void hkMultiRayConvexAgent::processCollision( const hkCdBody& bodyA,  const hkCdBody& bodyB, const hkProcessCollisionInput& input, hkProcessCollisionOutput& result)
{
	HK_ASSERT2(0x4e7eca78,  m_contactMgr, HK_MISSING_CONTACT_MANAGER_ERROR_TEXT );

	HK_TIMER_BEGIN("multiRay-cvx", HK_NULL);
    
	const hkMultiRayShape* msA = static_cast<const hkMultiRayShape*>(bodyA.getShape());
    const hkConvexShape*  convexB = static_cast<const hkConvexShape*>(bodyB.getShape());

	hkTransform bTa;	bTa.setMulInverseMul( bodyB.getTransform(), bodyA.getTransform() );

	const hkArray<hkMultiRayShape::Ray>& RaysA = msA->getRays();

	int nRays = RaysA.getSize();

	hkShapeRayCastInput rayInput;

	const hkMultiRayShape::Ray* rayA = RaysA.begin();
	for (int i = 0; i < nRays; i++)
	{
		rayInput.m_from._setTransformedPos( bTa, rayA->m_start );
		rayInput.m_to._setTransformedPos( bTa, rayA->m_end );

		hkShapeRayCastOutput rayResults;
		hkBool rayHit = convexB->castRay( rayInput, rayResults);
		if ( !rayHit )
		{
			if(m_contactInfo[i].m_contactPointId != HK_INVALID_CONTACT_POINT)
			{
				m_contactMgr->removeContactPoint(m_contactInfo[i].m_contactPointId, *result.m_constraintOwner );
				m_contactInfo[i].m_contactPointId = HK_INVALID_CONTACT_POINT;
			}
		}
		else
		{
			hkProcessCdPoint& point = *result.reserveContactPoints(1);
			const hkVector4& normal = rayResults.m_normal;

			hkVector4 hitpoint;	hitpoint.setInterpolate4(rayInput.m_from,rayInput.m_to, rayResults.m_hitFraction);
			point.m_contact.getPosition().setTransformedPos( bodyB.getTransform(), hitpoint );

			point.m_contact.getSeparatingNormal().setRotatedDir( bodyB.getTransform().getRotation(), normal);
			point.m_contact.setDistance( (rayResults.m_hitFraction - 1.0f) * rayA->m_start(3) + msA->getRayPenetrationDistance());// + input.m_tolerance;

			if( m_contactInfo[i].m_contactPointId == HK_INVALID_CONTACT_POINT)
			{
				m_contactInfo[i].m_contactPointId = m_contactMgr->addContactPoint(bodyA, bodyB, input, result, HK_NULL, point.m_contact );
			}

			if ( m_contactInfo[i].m_contactPointId != HK_INVALID_CONTACT_POINT )
			{
				result.commitContactPoints(1);
				point.m_contactPointId = m_contactInfo[i].m_contactPointId;
			}
			else
			{
				result.abortContactPoints(1);
			}			
		}
		rayA++;
	}

	HK_TIMER_END();
}



void hkMultiRayConvexAgent::getPenetrations(const  hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
{
	staticGetPenetrations(bodyA, bodyB, input, collector);
}

void hkMultiRayConvexAgent::staticGetPenetrations(const  hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
{
	HK_TIMER_BEGIN("multiRay-cnvx-getPen", HK_NULL);

	const hkMultiRayShape*	rayShape	= static_cast<const hkMultiRayShape*>(bodyA.getShape());
	const hkConvexShape*	convexShape = static_cast<const hkConvexShape*>(bodyB.getShape());

	hkTransform convexTRay;
	convexTRay.setMulInverseMul( bodyB.getTransform(), bodyA.getTransform() );

	int rayCount = rayShape->getRays().getSize();

	const hkMultiRayShape::Ray* ray = rayShape->getRays().begin();

	hkShapeRayCastInput		rayInput;
	hkShapeRayCastOutput	rayOuput;

	// Cast each ray in the multi-ray shape against the convex
	// shape. Each time a hit found, add it to the collector.
	// When the collectors early out flag is set, stop.
	while (rayCount-- && !collector.getEarlyOut())
	{		
		rayInput.m_from._setTransformedPos( convexTRay, ray->m_start );
		rayInput.m_to._setTransformedPos(   convexTRay, ray->m_end );

		if ( convexShape->castRay( rayInput, rayOuput) ) // If hit
		{
			collector.addCdBodyPair(bodyA, bodyB); // Collect the hit
		}

		ray++; // Get the next ray
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
