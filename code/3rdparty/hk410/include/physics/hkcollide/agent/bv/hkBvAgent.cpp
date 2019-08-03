/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>

#include <hkmath/basetypes/hkAabb.h>

#include <hkcollide/agent/bv/hkBvAgent.h>

#include <hkcollide/shape/collection/hkShapeCollection.h>
#include <hkcollide/shape/bv/hkBvShape.h>

#include <hkcollide/dispatch/contactmgr/hkContactMgrFactory.h>
#include <hkcollide/dispatch/hkAgentDispatchUtil.h>
#include <hkcollide/agent/hkLinearCastCollisionInput.h>

#include <hkcollide/collector/bodypaircollector/hkFlagCdBodyPairCollector.h>
#include <hkcollide/collector/pointcollector/hkSimpleClosestContactCollector.h>


hkBvAgent::hkBvAgent(const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr)
:hkCollisionAgent( mgr )
{

	const hkBvShape* bvShape = static_cast<const hkBvShape*>(bodyA.getShape());

	hkCdBody newA( &bodyA );
	newA.setShape( bvShape->getBoundingVolumeShape() );

	m_boundingVolumeAgent = input.m_dispatcher->getNewCollisionAgent( newA, bodyB, input, mgr );
	m_childAgent = HK_NULL;
}

void HK_CALL hkBvAgent::registerAgent(hkCollisionDispatcher* dispatcher)
{
	// register symmetric version
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc          = createShapeBvAgent;
		af.m_getPenetrationsFunc  = hkSymmetricAgent<hkBvAgent>::staticGetPenetrations;
		af.m_getClosestPointFunc = hkSymmetricAgent<hkBvAgent>::staticGetClosestPoints;
		af.m_linearCastFunc      = hkSymmetricAgent<hkBvAgent>::staticLinearCast;
		af.m_isFlipped           = true;
		af.m_isPredictive		 = true;	// there is no fallback
		dispatcher->registerCollisionAgent(af, HK_SHAPE_ALL, HK_SHAPE_BV );
	}
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc          = createBvShapeAgent;
		af.m_getPenetrationsFunc  = staticGetPenetrations;
		af.m_getClosestPointFunc = staticGetClosestPoints;
		af.m_linearCastFunc      = staticLinearCast;
		af.m_isFlipped           = false;
		af.m_isPredictive		 = true;	// there is no fallback
		dispatcher->registerCollisionAgent(af, HK_SHAPE_BV, HK_SHAPE_ALL );
	}
}



hkCollisionAgent* HK_CALL hkBvAgent::createBvShapeAgent(const hkCdBody& A, const hkCdBody& B, const hkCollisionInput& input, hkContactMgr* mgr)
{
	hkBvAgent* agent = new hkBvAgent(A, B, input, mgr);
	return agent;
}


hkCollisionAgent* HK_CALL hkBvAgent::createShapeBvAgent(const hkCdBody& A, const hkCdBody& B, const hkCollisionInput& input, hkContactMgr* mgr)
{
	hkBvAgent* agent = new hkSymmetricAgent<hkBvAgent>( A, B, input, mgr );
	return agent;
}


void hkBvAgent::cleanup(hkCollisionConstraintOwner& constraintOwner)
{
	m_boundingVolumeAgent->cleanup( constraintOwner );
	if ( m_childAgent )
	{
		m_childAgent->cleanup( constraintOwner);
		m_childAgent = HK_NULL;
	}

	delete this;
}

void hkBvAgent::invalidateTim( hkCollisionInput& input)
{
	m_boundingVolumeAgent->invalidateTim(input);
	if ( m_childAgent )
	{
		m_childAgent->invalidateTim(input);
	}
}

void hkBvAgent::warpTime( hkTime oldTime, hkTime newTime, hkCollisionInput& input )
{
	m_boundingVolumeAgent->warpTime( oldTime, newTime, input );
	if ( m_childAgent )
	{
		m_childAgent->warpTime( oldTime, newTime, input );
	}
}

void hkBvAgent::removePoint( hkContactPointId idToRemove )
{
	if ( m_childAgent )
	{
		m_childAgent->removePoint( idToRemove );
	}
}

void hkBvAgent::commitPotential( hkContactPointId newId )
{
	if ( m_childAgent )
	{
		m_childAgent->commitPotential( newId );
	}
}

void hkBvAgent::createZombie( hkContactPointId idTobecomeZombie )
{
	if ( m_childAgent )
	{
		m_childAgent->createZombie( idTobecomeZombie );
	}
}

void hkBvAgent::processCollision(const hkCdBody& bodyAin, const hkCdBody& bodyB, const hkProcessCollisionInput& input, hkProcessCollisionOutput& result)
{
	HK_ASSERT2(0x325d7b4b,  m_contactMgr, HK_MISSING_CONTACT_MANAGER_ERROR_TEXT );
	HK_TIMER_BEGIN_LIST( "hkBvAgent", "checkBvShape" );

	const hkBvShape* bvShape = static_cast<const hkBvShape*>(bodyAin.getShape());

	hkCdBody newA( &bodyAin);
	newA.setShape( bvShape->getBoundingVolumeShape() );


	hkFlagCdBodyPairCollector collector;
	m_boundingVolumeAgent->getPenetrations( newA, bodyB, input, collector );
	if ( collector.hasHit() )
	{
		HK_TIMER_SPLIT_LIST("child");
		newA.setShape( bvShape->getChildShape() );
		if ( ! m_childAgent )
		{
			m_childAgent = input.m_dispatcher->getNewCollisionAgent( newA, bodyB, input, m_contactMgr );
		}
		m_childAgent->processCollision( newA, bodyB, input, result );
	}
	else
	{
		if ( m_childAgent )
		{
			m_childAgent->cleanup( *result.m_constraintOwner );
			m_childAgent = HK_NULL;
		}
	}
	HK_TIMER_END_LIST();
}


void hkBvAgent::linearCast( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkCdPointCollector& pointDetails, hkCdPointCollector* startCollector)
{
	HK_TIMER_BEGIN_LIST( "hkBvAgent", "checkBvShape" );

	const hkBvShape* bvShape = static_cast<const hkBvShape*>(bodyA.getShape());
	hkCdBody newA( &bodyA);
	newA.setShape( bvShape->getBoundingVolumeShape() );

	hkSimpleClosestContactCollector checker;
	m_boundingVolumeAgent->linearCast( newA, bodyB, input, checker, &checker );
	if ( checker.hasHit() )
	{
		HK_TIMER_SPLIT_LIST("child");
		newA.setShape( bvShape->getChildShape() );
		if ( ! m_childAgent )
		{
			m_childAgent = input.m_dispatcher->getNewCollisionAgent( newA, bodyB, input, m_contactMgr );
		}
		m_childAgent->linearCast( newA, bodyB, input, pointDetails, startCollector );
	}
	else
	{
// we do not have a constraintOwner, so we cannot delete the child agent
// 		if ( m_childAgent )
// 		{
// 			m_childAgent->cleanup( ) ;
// 			m_childAgent = HK_NULL;
// 		}
	}
	HK_TIMER_END_LIST();
}

void hkBvAgent::staticLinearCast( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkCdPointCollector& pointDetails, hkCdPointCollector* startCollector)
{
	HK_TIMER_BEGIN_LIST( "hkBvAgent", "checkBvShape" );
	{

		const hkBvShape* bvShape = static_cast<const hkBvShape*>(bodyA.getShape());
		hkCdBody newA( &bodyA );
		newA.setShape( bvShape->getBoundingVolumeShape() );


		hkShapeType typeA = newA.getShape()->getType();
		hkShapeType typeB = bodyB.getShape()->getType();

		hkCollisionDispatcher::LinearCastFunc bvLinearCastFunc = input.m_dispatcher->getLinearCastFunc( typeA, typeB );

		hkSimpleClosestContactCollector checker;
		bvLinearCastFunc( newA, bodyB, input, checker, &checker );
		if ( checker.hasHit() )
		{
			HK_TIMER_SPLIT_LIST("child");
			newA.setShape( bvShape->getChildShape() );
			typeA = newA.getShape()->getType();

			hkCollisionDispatcher::LinearCastFunc childLinearCast = input.m_dispatcher->getLinearCastFunc( typeA, typeB );

			childLinearCast( newA, bodyB, input, pointDetails, startCollector );
		}
	}
	HK_TIMER_END_LIST();
}


// hkCollisionAgent interface implementation.
void hkBvAgent::getClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, class hkCdPointCollector& collector )
{
	HK_TIMER_BEGIN_LIST( "hkBvAgent", "checkBvShape" );
	{
		const hkBvShape* bvShape = static_cast<const hkBvShape*>(bodyA.getShape());

		hkCdBody newA( &bodyA );
		newA.setShape( bvShape->getBoundingVolumeShape() );
		hkFlagCdBodyPairCollector checker;
		m_boundingVolumeAgent->getPenetrations( newA, bodyB, input, checker );
		if ( checker.hasHit() )
		{
			HK_TIMER_SPLIT_LIST("child");
			newA.setShape( bvShape->getChildShape() );
			if ( ! m_childAgent )
			{
				m_childAgent = input.m_dispatcher->getNewCollisionAgent( newA, bodyB, input, m_contactMgr );
			}
			m_childAgent->getClosestPoints( newA, bodyB, input, collector );
		}
		else
		{
// we do not have a constraintOwner, so we cannot delete the child agent
// 			if ( m_childAgent )
// 			{
// 				m_childAgent->cleanup() ;
// 				m_childAgent = HK_NULL;
// 			}
		}
	}
	HK_TIMER_END_LIST();
}


// hkCollisionAgent interface implementation.
void hkBvAgent::staticGetClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, class hkCdPointCollector& collector )
{
	HK_TIMER_BEGIN_LIST( "hkBvAgent", "checkBvShape" );
	{
		const hkBvShape* bvShape = static_cast<const hkBvShape*>(bodyA.getShape());

		hkCdBody newA( &bodyA );
		newA.setShape( bvShape->getBoundingVolumeShape() );

		hkShapeType typeA = newA.getShape()->getType();
		hkShapeType typeB = bodyB.getShape()->getType();

		hkCollisionDispatcher::GetPenetrationsFunc boundingVolumeGetPenetrations = input.m_dispatcher->getGetPenetrationsFunc( typeA, typeB );

		hkFlagCdBodyPairCollector checker;
		boundingVolumeGetPenetrations( newA, bodyB, input, checker );

		if ( checker.hasHit() )
		{
			HK_TIMER_SPLIT_LIST("child");
			newA.setShape( bvShape->getChildShape() );
			typeA = newA.getShape()->getType();

			hkCollisionDispatcher::GetClosestPointsFunc childGetClosestPoint = input.m_dispatcher->getGetClosestPointsFunc( typeA, typeB );

			childGetClosestPoint( newA, bodyB, input, collector );

		}
	}
	HK_TIMER_END_LIST();
}




void hkBvAgent::getPenetrations( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
{
	HK_TIMER_BEGIN( "hkBvAgent", HK_NULL );

	const hkBvShape* bvShape = static_cast<const hkBvShape*>(bodyA.getShape());
	hkCdBody newA( &bodyA );
	newA.setShape( bvShape->getBoundingVolumeShape() );


	m_boundingVolumeAgent->getPenetrations(newA, bodyB, input, collector);

	HK_TIMER_END();
}

void hkBvAgent::staticGetPenetrations( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
{
	HK_TIMER_BEGIN( "hkBvAgent", HK_NULL );

	const hkBvShape* bvShape = static_cast<const hkBvShape*>(bodyA.getShape());
	hkCdBody newA( &bodyA );
	newA.setShape( bvShape->getBoundingVolumeShape() );

	hkShapeType typeA = newA.getShape()->getType();
	hkShapeType typeB = bodyB.getShape()->getType();

	hkCollisionDispatcher::GetPenetrationsFunc boundingVolumeGetPenetrations = input.m_dispatcher->getGetPenetrationsFunc( typeA, typeB );

	boundingVolumeGetPenetrations(newA, bodyB, input, collector);

	HK_TIMER_END();
}

void hkBvAgent::updateShapeCollectionFilter( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCollisionConstraintOwner& constraintOwner )
{
	const hkBvShape* bvShape = static_cast<const hkBvShape*>(bodyA.getShape());
	hkCdBody newA( &bodyA);
	newA.setShape( bvShape->getBoundingVolumeShape() );

	m_boundingVolumeAgent->updateShapeCollectionFilter( newA, bodyB, input, constraintOwner );

	if ( m_childAgent )
	{
		newA.setShape( bvShape->getChildShape() );
		m_childAgent->updateShapeCollectionFilter( newA, bodyB, input, constraintOwner );
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
