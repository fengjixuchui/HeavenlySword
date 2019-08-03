/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>

#include <hkbase/memory/hkLocalBuffer.h>

#include <hkmath/linear/hkSweptTransformUtil.h>

#include <hkcollide/shape/list/hkListShape.h>
#include <hkcollide/agent/list/hkListAgent.h>
#include <hkcollide/agent/shapecollection/hkShapeCollectionAgent.h>
#include <hkcollide/agent/hkShapeCollectionFilter.h>
#include <hkcollide/agent/null/hkNullAgent.h>

//#include <hkcollide/shape/hkShape.h>
//typedef hkUint32 hkShapeKey;
//#include <hkcollide/agent/shapecollection/hkShapeCollectionAgent.h>
//#include <hkcollide/collector/pointcollector/hkClosestCdPointCollector.h>
#include <hkinternal/collide/agent3/machine/1n/hkAgent1nMachine.h>


void HK_CALL hkListAgent::registerAgent(hkCollisionDispatcher* dispatcher)
{
	// symmetric
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc          = createListAAgent;

			// fallback to hkShapeCollection Agent implementations
		af.m_getPenetrationsFunc  = hkShapeCollectionAgent::staticGetPenetrations;	
		af.m_getClosestPointFunc = hkShapeCollectionAgent::staticGetClosestPoints;
		af.m_linearCastFunc      = hkShapeCollectionAgent::staticLinearCast;
		af.m_isFlipped           = true;
		af.m_isPredictive		 = true;

		dispatcher->registerCollisionAgent(af, HK_SHAPE_LIST, HK_SHAPE_ALL);
	}

	// direct
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc          = createListBAgent;

			// fallback to hkShapeCollection Agent implementations
		af.m_getPenetrationsFunc  = hkSymmetricAgent<hkShapeCollectionAgent>::staticGetPenetrations;
		af.m_getClosestPointFunc = hkSymmetricAgent<hkShapeCollectionAgent>::staticGetClosestPoints;
		af.m_linearCastFunc      = hkSymmetricAgent<hkShapeCollectionAgent>::staticLinearCast;
		af.m_isFlipped           = false;
		af.m_isPredictive		 = true;
		dispatcher->registerCollisionAgent(af, HK_SHAPE_ALL, HK_SHAPE_LIST);
	}
}


hkCollisionAgent* HK_CALL hkListAgent::createListAAgent(const hkCdBody& bodyA, const hkCdBody& bodyB, 
														const hkCollisionInput& input, hkContactMgr* contactMgr)
{
	hkListAgent* agent = new hkSymmetricAgent<hkListAgent>(bodyA, bodyB, input, contactMgr);
	return agent;
}

hkCollisionAgent* HK_CALL hkListAgent::createListBAgent(const hkCdBody& bodyA, const hkCdBody& bodyB, 
									const hkCollisionInput& input, hkContactMgr* contactMgr)
{
	hkListAgent* agent = new hkListAgent(bodyA, bodyB, input, contactMgr);
	return agent;
}



hkListAgent::hkListAgent(const  hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* contactMgr)
: hkCollisionAgent( contactMgr )
{
	m_dispatcher = input.m_dispatcher;

	// Initialize the agent stream
	new ( &m_agentTrack ) hkAgent1nTrack();
	hkAgent1nMachine_Create( m_agentTrack );
}


void hkListAgent::cleanup( hkCollisionConstraintOwner& constraintOwner )
{
	hkAgent1nMachine_Destroy( m_agentTrack, m_dispatcher, m_contactMgr, constraintOwner );
	delete this;
}

void hkListAgent::invalidateTim( hkCollisionInput& input )
{
	hkAgent1nMachine_InvalidateTim(m_agentTrack, input);
}

void hkListAgent::warpTime( hkTime oldTime, hkTime newTime, hkCollisionInput& input )
{
	hkAgent1nMachine_WarpTime(m_agentTrack, oldTime, newTime, input);
}


void hkListAgent::processCollision( const hkCdBody& bodyA, const hkCdBody& bodyB, 
									const hkProcessCollisionInput& input, hkProcessCollisionOutput& output)
{
	HK_ASSERT2(0x158d6356,  m_contactMgr, HK_MISSING_CONTACT_MANAGER_ERROR_TEXT );

	HK_INTERNAL_TIMER_BEGIN("list", this);

	const hkListShape* lShapeB = static_cast<const hkListShape*>(bodyB.getShape());
	
	//
	//	Set the input structure
	//
	hkAgent3ProcessInput in3;
	{
		in3.m_bodyA = &bodyA;
		in3.m_bodyB = &bodyB;
		in3.m_contactMgr = m_contactMgr;
		in3.m_input = &input;

		const hkMotionState* msA = bodyA.getMotionState();
		const hkMotionState* msB = bodyB.getMotionState();

		hkSweptTransformUtil::calcTimInfo( *msA, *msB, input.m_stepInfo.m_deltaTime, in3.m_linearTimInfo);

		in3.m_aTb.setMulInverseMul(msA->getTransform(), msB->getTransform());
	}

	int size = lShapeB->m_childInfo.getSize();
	hkLocalBuffer<hkShapeKey> hitList( size+1 );
	for ( int i = 0; i < size; i++ ){		hitList[i] = static_cast<hkUint32>(i);	}
	hitList[size] = HK_INVALID_SHAPE_KEY;

	hkAgent1nMachine_Process( m_agentTrack, in3, lShapeB, hitList.begin(), size, output );

	HK_INTERNAL_TIMER_END();
}

void hkListAgent::getClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& collector ) 
{
	hkListAgent::staticGetClosestPoints( bodyA, bodyB, input, collector );
}

		// hkCollisionAgent interface implementation.
void hkListAgent::staticGetClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& collector)
{
	HK_INTERNAL_TIMER_BEGIN("ListAgent", this);
	hkSymmetricAgent<hkShapeCollectionAgent>::staticGetClosestPoints( bodyA, bodyB, input, collector );
	HK_INTERNAL_TIMER_END();

}
	
void hkListAgent::linearCast( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkCdPointCollector& collector, hkCdPointCollector* startCollector )
{
	hkListAgent::staticLinearCast( bodyA, bodyB, input, collector, startCollector );
}

void hkListAgent::staticLinearCast( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkCdPointCollector& collector, hkCdPointCollector* startCollector )
{
	HK_INTERNAL_TIMER_BEGIN("ListAgent", this);
	hkSymmetricAgent<hkShapeCollectionAgent>::staticLinearCast( bodyA, bodyB, input, collector, startCollector );
	HK_INTERNAL_TIMER_END();
}

void hkListAgent::getPenetrations( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
{
	hkListAgent::staticGetPenetrations( bodyA, bodyB, input, collector);
}

void hkListAgent::staticGetPenetrations( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
{
	HK_INTERNAL_TIMER_BEGIN("ListAgent", this);
	hkSymmetricAgent<hkShapeCollectionAgent>::staticGetPenetrations( bodyA, bodyB, input, collector );
	HK_INTERNAL_TIMER_END();
}

void hkListAgent::updateShapeCollectionFilter( const hkCdBody& bodyA, const hkCdBody& listShapeBodyB, const hkCollisionInput& input, hkCollisionConstraintOwner& constraintOwner )
{
	hkAgent1nMachine_VisitorInput vin;
	vin.m_bodyA = &bodyA;
	vin.m_bvTreeBodyB = &listShapeBodyB;
	vin.m_input = &input;
	vin.m_contactMgr = m_contactMgr;
	vin.m_constraintOwner = &constraintOwner;
	vin.m_collectionShapeB = static_cast<const hkShapeCollection*>(listShapeBodyB.getShape());

	hkAgent1nMachine_UpdateShapeCollectionFilter( m_agentTrack, vin );
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
