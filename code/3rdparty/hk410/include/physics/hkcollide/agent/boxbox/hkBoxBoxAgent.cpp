/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>

#include <hkcollide/shape/box/hkBoxShape.h>
#include <hkcollide/agent/boxbox/hkBoxBoxAgent.h>

#include <hkinternal/collide/boxbox/hkBoxBoxCollisionDetection.h>

#include <hkcollide/agent/boxbox/hkBoxBoxContactPoint.h>

#include <hkcollide/agent/gjk/hkGskfAgent.h>

hkBool hkBoxBoxAgent::m_attemptToFindAllEdges = false;


void HK_CALL hkBoxBoxAgent::registerAgent(hkCollisionDispatcher* dispatcher)
{
	hkCollisionDispatcher::AgentFuncs af;

	af.m_createFunc          = createBoxBoxAgent;
	af.m_getPenetrationsFunc  = staticGetPenetrations;
	af.m_getClosestPointFunc = staticGetClosestPoints;
	af.m_linearCastFunc      = staticLinearCast;
	af.m_isFlipped           = false;
	af.m_isPredictive		 = false;

	dispatcher->registerCollisionAgent(af, HK_SHAPE_BOX, HK_SHAPE_BOX);	
}


hkCollisionAgent* HK_CALL hkBoxBoxAgent::createBoxBoxAgent(const hkCdBody& bodyA, 	const hkCdBody& bodyB, 	const hkCollisionInput& input, hkContactMgr* contactMgr)
{
	const hkBoxShape* boxA = static_cast<const hkBoxShape*>(bodyA.getShape());
	const hkBoxShape* boxB = static_cast<const hkBoxShape*>(bodyB.getShape());

	const hkVector4& extA = boxA->getHalfExtents();
	const hkVector4& extB = boxB->getHalfExtents();

	// box box breaks down when the tolerance becomes larger than the minimum extent size
	// if this is the case create a convex-convex agent.
	if( ( input.m_tolerance >= extA(3) * 1.999f ) ||
		( input.m_tolerance >= extB(3) * 1.999f ) )
	{
		return hkGskfAgent::createGskfAgent( bodyA, bodyB, input, contactMgr ); 	
	}
	else
	{
		hkBoxBoxAgent* agent = new hkBoxBoxAgent(contactMgr);
		return agent;
	}

}

void hkBoxBoxAgent::cleanup(hkCollisionConstraintOwner& constraintOwner)
{
	for (int i = 0; i < m_manifold.getNumPoints(); i++)
	{
		m_contactMgr->removeContactPoint(m_manifold[i].m_contactPointId, constraintOwner );
	}
	delete this;
}


void hkBoxBoxAgent::processCollision(const hkCdBody& bodyA, const hkCdBody& bodyB, 
									const hkProcessCollisionInput& input, hkProcessCollisionOutput& result)
{
	HK_ASSERT2(0x3b792884,  m_contactMgr, HK_MISSING_CONTACT_MANAGER_ERROR_TEXT );
	HK_TIMER_BEGIN("BoxBox", this);
	const hkBoxShape* boxA = static_cast<const hkBoxShape*>(bodyA.getShape());
	const hkBoxShape* boxB = static_cast<const hkBoxShape*>(bodyB.getShape());

	const hkVector4& extA = boxA->getHalfExtents();
	const hkVector4& extB = boxB->getHalfExtents();

	hkVector4 rA; rA.setAll3( boxA->getRadius() );
	hkVector4 rA4; rA4.setAdd4( extA, rA );
	hkVector4 rB; rB.setAll3( boxB->getRadius() );
	hkVector4 rB4; rB4.setAdd4( extB, rB );

	hkTransform aTb; aTb.setMulInverseMul( bodyA.getTransform(), bodyB.getTransform() );
	hkBoxBoxCollisionDetection detector( bodyA, bodyB, &input, m_contactMgr, &result, aTb, bodyA.getTransform(), rA4, bodyB.getTransform(), rB4, input.getTolerance() );

	detector.calcManifold( m_manifold );

	HK_TIMER_END();
}

void hkBoxBoxAgent::staticGetClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, class hkCdPointCollector& collector  )
{
	HK_TIMER_BEGIN("BoxBox", this);
	const hkBoxShape* boxA = static_cast<const hkBoxShape*>(bodyA.getShape());
	const hkBoxShape* boxB = static_cast<const hkBoxShape*>(bodyB.getShape());

	hkVector4 rA; rA.setAll3( boxA->getRadius() );
	hkVector4 rA4; rA4.setAdd4( boxA->getHalfExtents(), rA );
	hkVector4 rB; rB.setAll3( boxB->getRadius() );
	hkVector4 rB4; rB4.setAdd4( boxB->getHalfExtents(), rB );

	hkTransform aTb; aTb.setMulInverseMul( bodyA.getTransform(), bodyB.getTransform() );
	hkBoxBoxCollisionDetection detector( bodyA, bodyB, HK_NULL, HK_NULL, HK_NULL,
										 aTb, bodyA.getTransform(), rA4,
										 bodyB.getTransform(), rB4, input.getTolerance() );
	
	hkCdPoint event( bodyA, bodyB );

	hkBool result = detector.calculateClosestPoint( event.m_contact );

	if (result)
	{ 
		collector.addCdPoint( event );
	}
	HK_TIMER_END();
}

void hkBoxBoxAgent::getClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, class hkCdPointCollector& collector  )
{
	staticGetClosestPoints( bodyA, bodyB, input, collector );
}


void hkBoxBoxAgent::staticGetPenetrations( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
{
	HK_TIMER_BEGIN("BoxBox", this);
	const hkBoxShape* boxA = static_cast<const hkBoxShape*>(bodyA.getShape());
	const hkBoxShape* boxB = static_cast<const hkBoxShape*>(bodyB.getShape());

	hkVector4 rA; rA.setAll3( boxA->getRadius() );
	hkVector4 rA4; rA4.setAdd4( boxA->getHalfExtents(), rA );
	hkVector4 rB; rB.setAll3( boxB->getRadius() );
	hkVector4 rB4; rB4.setAdd4( boxB->getHalfExtents(), rB );


	hkTransform aTb; aTb.setMulInverseMul( bodyA.getTransform(), bodyB.getTransform() );
	hkBoxBoxCollisionDetection detector( bodyA, bodyB, HK_NULL, HK_NULL, HK_NULL,
										 aTb, bodyA.getTransform(), rA4,
										 bodyB.getTransform(), rB4, input.getTolerance() );

	if (detector.getPenetrations())
	{
		collector.addCdBodyPair( bodyA, bodyB );
	}
	HK_TIMER_END();
}

void hkBoxBoxAgent::getPenetrations( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
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
