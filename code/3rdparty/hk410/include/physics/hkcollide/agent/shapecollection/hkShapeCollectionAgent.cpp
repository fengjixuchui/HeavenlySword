/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>

#include <hkcollide/shape/hkShapeContainer.h>

#include <hkcollide/agent/shapecollection/hkShapeCollectionAgent.h>
#include <hkcollide/agent/hkShapeCollectionFilter.h>

void HK_CALL hkShapeCollectionAgent::registerAgent(hkCollisionDispatcher* dispatcher)
{
	// register symmetric version
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc          = createListBAgent;
		af.m_getPenetrationsFunc  = hkSymmetricAgent<hkShapeCollectionAgent>::staticGetPenetrations;
		af.m_getClosestPointFunc = hkSymmetricAgent<hkShapeCollectionAgent>::staticGetClosestPoints;
		af.m_linearCastFunc      = hkSymmetricAgent<hkShapeCollectionAgent>::staticLinearCast;
		af.m_isFlipped           = true;
		af.m_isPredictive		 = true;
		dispatcher->registerCollisionAgent(af, HK_SHAPE_ALL, HK_SHAPE_COLLECTION );
	}
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc          = createListAAgent;
		af.m_getPenetrationsFunc  = staticGetPenetrations;
		af.m_getClosestPointFunc = staticGetClosestPoints;
		af.m_linearCastFunc      = staticLinearCast;
		af.m_isFlipped           = false;
		af.m_isPredictive		 = true;
		dispatcher->registerCollisionAgent(af, HK_SHAPE_COLLECTION, HK_SHAPE_ALL );
	}
}


hkCollisionAgent* HK_CALL hkShapeCollectionAgent::createListAAgent(const hkCdBody& bodyA, const hkCdBody& bodyB, 
									const hkCollisionInput& input, hkContactMgr* mgr)
{
	hkShapeCollectionAgent* agent = new hkShapeCollectionAgent(bodyA, bodyB, input, mgr);

	return agent;
}


hkCollisionAgent* HK_CALL hkShapeCollectionAgent::createListBAgent(const hkCdBody& bodyA, const hkCdBody& bodyB, 
									const hkCollisionInput& input, hkContactMgr* mgr)
{
	hkShapeCollectionAgent* agent = new hkSymmetricAgent<hkShapeCollectionAgent>(bodyA, bodyB, input, mgr);
	
	return agent;
}


hkShapeCollectionAgent::hkShapeCollectionAgent(const  hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr)
: hkCollisionAgent( mgr )
{
	hkCdBody newOperandA( &bodyA );

	//
	// initialize all the new child agents
	//
	const hkShapeContainer* shapeContainer = bodyA.getShape()->getContainer();
	HK_ASSERT2(0x75845342, shapeContainer != HK_NULL, "Shape collection agent called where bodyA is not a shape container");

	int numChildren = shapeContainer->getNumChildShapes();
	m_agents.reserve( numChildren );
	
	hkShapeContainer::ShapeBuffer shapeBuffer;

	hkShapeKey key = shapeContainer->getFirstKey();
	for (int i = 0; i < numChildren; key = shapeContainer->getNextKey(key), ++i)
	{
		newOperandA.setShape( shapeContainer->getChildShape( key, shapeBuffer), key );
		if ( input.m_filter->isCollisionEnabled( input, bodyB, bodyA, *shapeContainer , key ) )
		{
			KeyAgentPair& ap = *m_agents.expandByUnchecked(1);
			ap.m_agent = input.m_dispatcher->getNewCollisionAgent(newOperandA, bodyB, input, mgr);
			ap.m_key = key;
		}
	}
}


void hkShapeCollectionAgent::cleanup( hkCollisionConstraintOwner& info)
{
	for (int i = 0; i < m_agents.getSize(); ++i)
	{
		m_agents[i].m_agent->cleanup( info );
	}
	delete this;
}

void hkShapeCollectionAgent::invalidateTim( hkCollisionInput& input )
{
	for (int i = 0; i < m_agents.getSize(); i++)
	{
		m_agents[i].m_agent->invalidateTim(input);
	}
}

void hkShapeCollectionAgent::warpTime( hkTime oldTime, hkTime newTime, hkCollisionInput& input )
{
	for (int i = 0; i < m_agents.getSize(); i++)
	{
		m_agents[i].m_agent->warpTime(oldTime, newTime, input);
	}
}

void hkShapeCollectionAgent::processCollision(const  hkCdBody& bodyA, const hkCdBody& bodyB, 
									const hkProcessCollisionInput& input, hkProcessCollisionOutput& result)
{
	HK_ASSERT2(0x1a969a32,  m_contactMgr, HK_MISSING_CONTACT_MANAGER_ERROR_TEXT );
	HK_ON_DEBUG( if (bodyA.getShape()->getType() != HK_SHAPE_CONVEX_LIST) HK_WARN_ONCE(0x5607bb49,  "hkShapeCollection used without an hkBvTreeShape, possible huge performance loss"););

	HK_TIMER_BEGIN( "ShapeCollection", HK_NULL );
	
	const hkShapeContainer* shapeContainer = bodyA.getShape()->getContainer();
	HK_ASSERT2( 0xefee9806, shapeContainer != HK_NULL, "hkShapeCollectionAgent called on a shape which is not a shape container");

	//
	// call collision agents for shapeB against all shapeAs
	//
	
	hkCdBody newOperandA( &bodyA );

	KeyAgentPair* agentPair = m_agents.begin();
	hkShapeContainer::ShapeBuffer shapeBuffer;

	for ( int i = m_agents.getSize() -1; i>=0; i-- )
	{
		newOperandA.setShape( shapeContainer->getChildShape( agentPair->m_key, shapeBuffer), agentPair->m_key);
		agentPair->m_agent->processCollision(newOperandA, bodyB, input, result);
		agentPair++;
	}


	HK_TIMER_END();
}


		
void hkShapeCollectionAgent::getClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& collector )
{
	HK_TIMER_BEGIN( "ShapeCollection", HK_NULL );

	const hkShapeContainer* shapeContainer = bodyA.getShape()->getContainer();
	HK_ASSERT2( 0xefee9806, shapeContainer != HK_NULL, "hkShapeCollectionAgent called on a shape which is not a shape container");

	//
	// call collision agents for shapeB against all shapeAs
	//
	
	hkCdBody newOperandA( &bodyA );
	KeyAgentPair* agentPair = m_agents.begin();
	hkShapeContainer::ShapeBuffer shapeBuffer;

	for ( int i = m_agents.getSize() -1; i>=0; i-- )
	{
		newOperandA.setShape( shapeContainer->getChildShape( agentPair->m_key, shapeBuffer), agentPair->m_key);
		agentPair->m_agent->getClosestPoints(newOperandA, bodyB, input, collector);
		agentPair++;
	}

	HK_TIMER_END();
}


void hkShapeCollectionAgent::staticGetClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& collector )
{
	HK_TIMER_BEGIN( "ShapeCollection", HK_NULL );

	const hkShapeContainer* shapeContainer = bodyA.getShape()->getContainer();
	HK_ASSERT2( 0xefee9806, shapeContainer != HK_NULL, "hkShapeCollectionAgent called on a shape which is not a shape container");

	//
	// call collision agents for shapeB against all shapeAs
	//
	
	hkCdBody newOperandA( &bodyA );

	hkShapeType typeB = bodyB.getShape()->getType();

	hkShapeCollection::ShapeBuffer shapeBuffer;

	for ( hkShapeKey key = shapeContainer->getFirstKey(); key != HK_INVALID_SHAPE_KEY; key = shapeContainer->getNextKey(key) )
	{
		if ( input.m_filter->isCollisionEnabled( input, bodyB, bodyA, *shapeContainer , key) )
		{
			newOperandA.setShape( shapeContainer->getChildShape( key, shapeBuffer), key);
			hkShapeType typeA = newOperandA.getShape()->getType();
			hkCollisionDispatcher::GetClosestPointsFunc getClosestPointFunc = input.m_dispatcher->getGetClosestPointsFunc( typeA, typeB );

			getClosestPointFunc(newOperandA, bodyB, input, collector);
		}
	}

	HK_TIMER_END();
}


void hkShapeCollectionAgent::linearCast( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkCdPointCollector& collector, hkCdPointCollector* startCollector )
{
	HK_TIMER_BEGIN( "ShapeCollection", HK_NULL );

	const hkShapeContainer* shapeContainer = bodyA.getShape()->getContainer();	
	HK_ASSERT2( 0xefee9806, shapeContainer != HK_NULL, "hkShapeCollectionAgent called on a shape which is not a shape container");
	//
	// call collision agents for shapeB against all shapeAs
	//
	
	hkCdBody newOperandA( &bodyA );

	KeyAgentPair* agentPair = m_agents.begin();
	hkShapeCollection::ShapeBuffer shapeBuffer;

	for ( int i = m_agents.getSize() -1; i>=0; i-- )
	{
		newOperandA.setShape( shapeContainer->getChildShape( agentPair->m_key, shapeBuffer), agentPair->m_key);
		agentPair->m_agent->linearCast(newOperandA, bodyB, input, collector, startCollector );
		agentPair++;
	}

	HK_TIMER_END();
}

void hkShapeCollectionAgent::staticLinearCast( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkCdPointCollector& collector, hkCdPointCollector* startCollector )
{
	HK_TIMER_BEGIN( "ShapeCollection", HK_NULL );

	const hkShapeContainer* shapeContainer = bodyA.getShape()->getContainer();
	HK_ASSERT2( 0xefee9806, shapeContainer != HK_NULL, "hkShapeCollectionAgent called on a shape which is not a shape container");
	//
	// call collision agents for shapeB against all shapeAs
	//
	
	hkCdBody newOperandA( &bodyA );

	hkShapeType typeB = bodyB.getShape()->getType();

	hkShapeCollection::ShapeBuffer shapeBuffer;

	for ( hkShapeKey key = shapeContainer->getFirstKey(); key != HK_INVALID_SHAPE_KEY; key = shapeContainer->getNextKey(key) )
	{
		if ( input.m_filter->isCollisionEnabled( input, bodyB, bodyA, *shapeContainer, key) )
		{
			newOperandA.setShape( shapeContainer->getChildShape( key, shapeBuffer), key);
			hkShapeType typeA = newOperandA.getShape()->getType();
			hkCollisionDispatcher::LinearCastFunc linearCastFunc = input.m_dispatcher->getLinearCastFunc( typeA, typeB );
			linearCastFunc(newOperandA, bodyB, input, collector, startCollector );
		}
	}

	HK_TIMER_END();
}

void hkShapeCollectionAgent::getPenetrations( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
{
	HK_TIMER_BEGIN( "ShapeCollection", HK_NULL );

	const hkShapeContainer* shapeContainer = bodyA.getShape()->getContainer();
	HK_ASSERT2( 0xefee9806, shapeContainer != HK_NULL, "hkShapeCollectionAgent called on a shape which is not a shape container");
	
	hkCdBody newOperandA( &bodyA );

	KeyAgentPair* agentPair = m_agents.begin();
	
	hkShapeCollection::ShapeBuffer shapeBuffer;

	for ( int i = m_agents.getSize() -1; i>=0; i-- )
	{
		newOperandA.setShape( shapeContainer->getChildShape( agentPair->m_key, shapeBuffer), agentPair->m_key);
		HK_ASSERT2(0x24bd34bf, newOperandA.getShape() != HK_NULL , "No shape exists for corresponding agent");		
		agentPair->m_agent->getPenetrations(newOperandA, bodyB, input, collector );
		if ( collector.getEarlyOut() )
		{
			break;
		}
		agentPair++;
	}

	HK_TIMER_END();
}

void hkShapeCollectionAgent::staticGetPenetrations( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
{
	HK_TIMER_BEGIN( "ShapeCollection", HK_NULL );

	const hkShapeContainer* shapeContainer = bodyA.getShape()->getContainer();
	HK_ASSERT2( 0xefee9806, shapeContainer != HK_NULL, "hkShapeCollectionAgent called on a shape which is not a shape container");

	hkCdBody newOperandA( &bodyA );

	hkShapeType typeB = bodyB.getShape()->getType();

	hkShapeCollection::ShapeBuffer shapeBuffer;

	for ( hkShapeKey key = shapeContainer->getFirstKey(); key != HK_INVALID_SHAPE_KEY; key = shapeContainer->getNextKey(key) )
	{
		if ( input.m_filter->isCollisionEnabled( input, bodyB, bodyA, *shapeContainer , key) )
		{
			newOperandA.setShape( shapeContainer->getChildShape( key, shapeBuffer), key);
			hkShapeType typeA = newOperandA.getShape()->getType();
			hkCollisionDispatcher::GetPenetrationsFunc getPenetrationsFunc = input.m_dispatcher->getGetPenetrationsFunc( typeA, typeB );
			getPenetrationsFunc(newOperandA, bodyB, input, collector );
			if( collector.getEarlyOut() )
			{
				break;
			}
		}
	}

	HK_TIMER_END();
}

inline int hkShapeCollectionAgent::getAgentIndex( hkShapeKey key )
{
	for ( int index = 0; index < m_agents.getSize(); index++ )
	{
		if ( m_agents[index].m_key == key )
		{
			return index;
		}
	}
	return -1;
}


void hkShapeCollectionAgent::updateShapeCollectionFilter( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCollisionConstraintOwner& constraintOwner )
{
	hkCdBody newOperandA( &bodyA );

	//
	// initialize all the new child agents
	//
	const hkShapeContainer* shapeContainer = bodyA.getShape()->getContainer();
	HK_ASSERT2( 0xefee9806, shapeContainer != HK_NULL, "hkShapeCollectionAgent called on a shape which is not a shape container");


	int numChildren = shapeContainer->getNumChildShapes();
	
	hkShapeCollection::ShapeBuffer shapeBuffer;

	hkShapeKey key = shapeContainer->getFirstKey();
	for (int i = 0; i < numChildren; key = shapeContainer->getNextKey(key), ++i)
	{
		int index = getAgentIndex( key );
		if ( input.m_filter->isCollisionEnabled( input, bodyB, bodyA, *shapeContainer , key ) )
		{
			newOperandA.setShape( shapeContainer->getChildShape( key, shapeBuffer), key );

			if ( index == -1 )
			{
				// A new agent needs to be created and added to the list
				KeyAgentPair& newPair = m_agents.expandOne();
				newPair.m_key = key;
				newPair.m_agent = input.m_dispatcher->getNewCollisionAgent(newOperandA, bodyB, input, m_contactMgr);
			}
			else
			{
				m_agents[index].m_agent->updateShapeCollectionFilter( newOperandA, bodyB, input, constraintOwner );
			}
		}
		else
		{
			if ( index != -1 )
			{
				m_agents[index].m_agent->cleanup(constraintOwner);
				m_agents.removeAt(index);
			}
		}

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
