/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>

#include <hkcollide/agent/phantom/hkPhantomAgent.h>
#include <hkcollide/shape/phantomcallback/hkPhantomCallbackShape.h>


void HK_CALL hkPhantomAgent::registerAgent(hkCollisionDispatcher* dispatcher)
{
	// symmetric version = normal version
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc           = createPhantomAgent;
		af.m_getPenetrationsFunc  = staticGetPenetrations;
		af.m_getClosestPointFunc = staticGetClosestPoints;
		af.m_linearCastFunc      = staticLinearCast;
		af.m_isPredictive        = true;	// its really not predictive but we have no other fallback

		dispatcher->registerCollisionAgent(af, HK_SHAPE_PHANTOM_CALLBACK, HK_SHAPE_ALL);
		dispatcher->registerCollisionAgent(af, HK_SHAPE_ALL, HK_SHAPE_PHANTOM_CALLBACK);
	}
}

hkPhantomAgent::hkPhantomAgent(const hkCdBody& bodyA, const hkCdBody& bodyB, hkContactMgr* contactMgr)
: hkCollisionAgent( contactMgr )
{
	m_collidableA = bodyA.getRootCollidable();
	m_collidableB = bodyB.getRootCollidable();

	m_bodyTypeA = bodyA.getShape()->getType();
	m_bodyTypeB = bodyB.getShape()->getType();
}

hkCollisionAgent* HK_CALL hkPhantomAgent::createPhantomAgent(const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* contactMgr)
{
	hkPhantomAgent* pa = new hkPhantomAgent( bodyA, bodyB, contactMgr );
	if ( pa->m_bodyTypeA == HK_SHAPE_PHANTOM_CALLBACK )
	{
		const hkPhantomCallbackShape* constPhantomShape = static_cast<const hkPhantomCallbackShape*>( bodyA.getShape() );
		hkPhantomCallbackShape* phantomShape = const_cast<hkPhantomCallbackShape*>( constPhantomShape );
		phantomShape->phantomEnterEvent( bodyA.getRootCollidable(), bodyB.getRootCollidable(), input );
		pa->m_shapeA = phantomShape;
	}

	if ( pa->m_bodyTypeB == HK_SHAPE_PHANTOM_CALLBACK )
	{
		const hkPhantomCallbackShape* constPhantomShape = static_cast<const hkPhantomCallbackShape*>( bodyB.getShape() );
		hkPhantomCallbackShape* phantomShape = const_cast<hkPhantomCallbackShape*>( constPhantomShape );
		phantomShape->phantomEnterEvent( bodyB.getRootCollidable(), bodyA.getRootCollidable(), input );
		pa->m_shapeB = phantomShape;
	}

	return pa;
}

hkCollisionAgent* HK_CALL hkPhantomAgent::createNoPhantomAgent(const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr)
{

	if ( bodyA.getShape()->getType() == HK_SHAPE_PHANTOM_CALLBACK )
	{
		const hkPhantomCallbackShape* constPhantomShape = static_cast<const hkPhantomCallbackShape*>( bodyA.getShape() );
		hkPhantomCallbackShape* phantomShape = const_cast<hkPhantomCallbackShape*>( constPhantomShape );
		phantomShape->phantomEnterEvent( bodyA.getRootCollidable(), bodyB.getRootCollidable(), input );
	}

	if ( bodyB.getShape()->getType() == HK_SHAPE_PHANTOM_CALLBACK )
	{
		const hkPhantomCallbackShape* constPhantomShape = static_cast<const hkPhantomCallbackShape*>( bodyB.getShape() );
		hkPhantomCallbackShape* phantomShape = const_cast<hkPhantomCallbackShape*>( constPhantomShape );
		phantomShape->phantomEnterEvent( bodyB.getRootCollidable(), bodyA.getRootCollidable(), input );
	}

	return HK_NULL;	
}




void hkPhantomAgent::cleanup( hkCollisionConstraintOwner& constraintOwner )
{
	if ( m_bodyTypeA == HK_SHAPE_PHANTOM_CALLBACK )
	{
		m_shapeA->phantomLeaveEvent( m_collidableA, m_collidableB );
	}

	if ( m_bodyTypeB == HK_SHAPE_PHANTOM_CALLBACK )
	{
		m_shapeB->phantomLeaveEvent( m_collidableB, m_collidableA );
	}
	delete this;
}






void hkPhantomAgent::processCollision(const hkCdBody& bodyAin, const hkCdBody& bodyB, const hkProcessCollisionInput& input, hkProcessCollisionOutput& result)
{
}

void hkPhantomAgent::getPenetrations( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
{
	collector.addCdBodyPair( bodyA, bodyB );
}

void hkPhantomAgent::staticGetPenetrations( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
{
	collector.addCdBodyPair( bodyA, bodyB );
}

void hkPhantomAgent::getClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& pointDetails)
{
}

void hkPhantomAgent::staticGetClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& pointDetails)
{
}

void hkPhantomAgent::linearCast( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkCdPointCollector& collector, hkCdPointCollector* startCollector )
{
}

void hkPhantomAgent::staticLinearCast( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkCdPointCollector& collector, hkCdPointCollector* startCollector )
{
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
