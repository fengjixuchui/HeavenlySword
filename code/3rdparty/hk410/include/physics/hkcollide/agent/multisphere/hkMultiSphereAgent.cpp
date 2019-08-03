/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>
#include <hkcollide/shape/sphere/hkSphereShape.h>
#include <hkcollide/agent/multisphere/hkMultiSphereAgent.h>
#include <hkcollide/shape/multisphere/hkMultiSphereShape.h>

void HK_CALL hkMultiSphereAgent::registerAgent(hkCollisionDispatcher* dispatcher)
{
	// register symmetric version
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc          = createListBAgent;
		af.m_getPenetrationsFunc  = hkSymmetricAgent<hkMultiSphereAgent>::staticGetPenetrations;
		af.m_getClosestPointFunc = hkSymmetricAgent<hkMultiSphereAgent>::staticGetClosestPoints;
		af.m_linearCastFunc      = hkSymmetricAgent<hkMultiSphereAgent>::staticLinearCast;
		af.m_isFlipped           = true;
		af.m_isPredictive		 = true;
		dispatcher->registerCollisionAgent(af, HK_SHAPE_ALL, HK_SHAPE_MULTI_SPHERE );
	}
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc          = createListAAgent;
		af.m_getPenetrationsFunc  = staticGetPenetrations;
		af.m_getClosestPointFunc = staticGetClosestPoints;
		af.m_linearCastFunc      = staticLinearCast;
		af.m_isFlipped           = false;
		af.m_isPredictive		 = true;
		dispatcher->registerCollisionAgent(af, HK_SHAPE_MULTI_SPHERE, HK_SHAPE_ALL );
	}
}


hkCollisionAgent* HK_CALL hkMultiSphereAgent::createListAAgent(const hkCdBody& bodyA, const hkCdBody& bodyB, 
									const hkCollisionInput& input, hkContactMgr* mgr)
{
	hkMultiSphereAgent* agent = new hkMultiSphereAgent(bodyA, bodyB, input, mgr);

	return agent;
}


hkCollisionAgent* HK_CALL hkMultiSphereAgent::createListBAgent(const hkCdBody& bodyA, const hkCdBody& bodyB, 
									const hkCollisionInput& input, hkContactMgr* mgr)
{
	hkMultiSphereAgent* agent = new hkSymmetricAgent<hkMultiSphereAgent>(bodyA, bodyB, input, mgr);
	
	return agent;
}


hkMultiSphereAgent::hkMultiSphereAgent(const  hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr)
: hkCollisionAgent( mgr )
{

	//
	// initialize all the new child agents
	//
	const hkMultiSphereShape* MultiSphere = static_cast<const hkMultiSphereShape*>(bodyA.getShape());

	int numChildren = MultiSphere->getNumSpheres();
	m_agents.reserve( numChildren );
	
	hkSphereShape sphereShape(0.0f);

	hkMotionState ms = *bodyA.getMotionState();
	hkCdBody newOperandA( &bodyA, &ms );
	
	const hkVector4* spheres = MultiSphere->getSpheres();
	for (int i = 0; i < numChildren; i++ )
	{
		hkVector4 offsetWs;
		offsetWs._setRotatedDir( ms.getTransform().getRotation(), spheres[0] );
		ms.getTransform().getTranslation().setAdd4( bodyA.getTransform().getTranslation(), offsetWs );
		ms.getSweptTransform().m_centerOfMass0.setAdd4( bodyA.getMotionState()->getSweptTransform().m_centerOfMass0, offsetWs );
		ms.getSweptTransform().m_centerOfMass1.setAdd4( bodyA.getMotionState()->getSweptTransform().m_centerOfMass1, offsetWs );

		sphereShape.setRadius( spheres[0](3) );
		newOperandA.setShape( &sphereShape, i );

		{
			KeyAgentPair& ap = *m_agents.expandByUnchecked(1);
			ap.m_agent = input.m_dispatcher->getNewCollisionAgent(newOperandA, bodyB, input, mgr);
			ap.m_key = i;
		}
		spheres++;
	}
	
}


void hkMultiSphereAgent::cleanup( hkCollisionConstraintOwner& info )
{
	for (int i = 0; i < m_agents.getSize(); ++i)
	{
		m_agents[i].m_agent->cleanup( info );
	}
	delete this;
}


void hkMultiSphereAgent::processCollision(const  hkCdBody& bodyA, const hkCdBody& bodyB, 
									const hkProcessCollisionInput& input, hkProcessCollisionOutput& result)
{
	HK_ASSERT2(0x2cadb41e,  m_contactMgr, HK_MISSING_CONTACT_MANAGER_ERROR_TEXT );

	HK_INTERNAL_TIMER_BEGIN( "MultiSphere", this );
	
	const hkMultiSphereShape* MultiSphere = static_cast<const hkMultiSphereShape*>(bodyA.getShape());
	
	//
	// call collision agents for shapeB against all shapeAs
	//
	
	hkMotionState ms = *bodyA.getMotionState();
	hkSphereShape sphereShape(0.0f);
	hkCdBody newOperandA( &bodyA, &ms );

	KeyAgentPair* agentPair = m_agents.begin();

	for ( int i = m_agents.getSize() -1; i>=0; i-- )
	{
		const hkVector4& sphere = MultiSphere->getSpheres()[ agentPair->m_key ];

		hkVector4 offsetWs;
		offsetWs._setRotatedDir( ms.getTransform().getRotation(), sphere );
		ms.getTransform().getTranslation().setAdd4( bodyA.getTransform().getTranslation(), offsetWs );
		ms.getSweptTransform().m_centerOfMass0.setAdd4( bodyA.getMotionState()->getSweptTransform().m_centerOfMass0, offsetWs );
		ms.getSweptTransform().m_centerOfMass1.setAdd4( bodyA.getMotionState()->getSweptTransform().m_centerOfMass1, offsetWs );

		sphereShape.setRadius( sphere(3) );
		newOperandA.setShape( &sphereShape, i );

		agentPair->m_agent->processCollision(newOperandA, bodyB, input, result);
		agentPair++;
	}


	HK_INTERNAL_TIMER_END();
}


		
void hkMultiSphereAgent::getClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& collector )
{
	HK_INTERNAL_TIMER_BEGIN( "MultiSphere", this );

	const hkMultiSphereShape* MultiSphere = static_cast<const hkMultiSphereShape*>(bodyA.getShape());
	
	//
	// call collision agents for shapeB against all shapeAs
	//
	
	hkMotionState ms = *bodyA.getMotionState();
	hkSphereShape sphereShape(0.0f);
	hkCdBody newOperandA( &bodyA, &ms );
	

	KeyAgentPair* agentPair = m_agents.begin();

	for ( int i = m_agents.getSize() -1; i>=0; i-- )
	{
		const hkVector4& sphere = MultiSphere->getSpheres()[ agentPair->m_key ];
		hkVector4 off;	off._setRotatedDir( ms.getTransform().getRotation(), sphere );
		ms.getTransform().getTranslation().setAdd4( bodyA.getTransform().getTranslation(), off );

		sphereShape.setRadius( sphere(3) );
		newOperandA.setShape( &sphereShape, i );
		agentPair->m_agent->getClosestPoints(newOperandA, bodyB, input, collector);
		agentPair++;
	}

	HK_INTERNAL_TIMER_END();
}


void hkMultiSphereAgent::staticGetClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& collector )
{
	HK_INTERNAL_TIMER_BEGIN( "MultiSphere", this );

	const hkMultiSphereShape* MultiSphere = static_cast<const hkMultiSphereShape*>(bodyA.getShape());
	
	//
	// call collision agents for shapeB against all shapeAs
	//

	hkMotionState ms = *bodyA.getMotionState();
	hkSphereShape sphereShape(0.0f);
	hkCdBody newOperandA( &bodyA, &ms );
	
	hkShapeType typeB = bodyB.getShape()->getType();

	for ( int key = 0; key < MultiSphere->getNumSpheres(); key++ )
	{
		const hkVector4& sphere = MultiSphere->getSpheres()[ key ];
		hkVector4 off;	off._setRotatedDir( ms.getTransform().getRotation(), sphere );
		ms.getTransform().getTranslation().setAdd4( bodyA.getTransform().getTranslation(), off );
		sphereShape.setRadius( sphere(3) );
		newOperandA.setShape( &sphereShape, key );

		hkShapeType typeA = sphereShape.getType();
		hkCollisionDispatcher::GetClosestPointsFunc getClosestPointFunc = input.m_dispatcher->getGetClosestPointsFunc( typeA, typeB );

		getClosestPointFunc(newOperandA, bodyB, input, collector);
	}

	HK_INTERNAL_TIMER_END();
}


void hkMultiSphereAgent::linearCast( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkCdPointCollector& collector, hkCdPointCollector* startCollector )
{
	HK_INTERNAL_TIMER_BEGIN( "MultiSphere", this );

	const hkMultiSphereShape* MultiSphere = static_cast<const hkMultiSphereShape*>(bodyA.getShape());
	
	//
	// call collision agents for shapeB against all shapeAs
	//
	
	hkMotionState ms = *bodyA.getMotionState();
	hkSphereShape sphereShape(0.0f);
	hkCdBody newOperandA( &bodyA, &ms );
	
	KeyAgentPair* agentPair = m_agents.begin();

	for ( int i = m_agents.getSize() -1; i>=0; i-- )
	{
		const hkVector4& sphere = MultiSphere->getSpheres()[ i ];
		hkVector4 off;	off._setRotatedDir( ms.getTransform().getRotation(), sphere );
		ms.getTransform().getTranslation().setAdd4( bodyA.getTransform().getTranslation(), off );

		sphereShape.setRadius( sphere(3) );
		newOperandA.setShape( &sphereShape, i );
		agentPair->m_agent->linearCast(newOperandA, bodyB, input, collector, startCollector );
	}

	HK_INTERNAL_TIMER_END();
}

void hkMultiSphereAgent::staticLinearCast( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkCdPointCollector& collector, hkCdPointCollector* startCollector )
{
	HK_INTERNAL_TIMER_BEGIN( "MultiSphere", this );

	const hkMultiSphereShape* MultiSphere = static_cast<const hkMultiSphereShape*>(bodyA.getShape());
	
	//
	// call collision agents for shapeB against all shapeAs
	//
	
	hkMotionState ms = *bodyA.getMotionState();
	hkSphereShape sphereShape(0.0f);
	hkCdBody newOperandA( &bodyA, &ms );
	
	hkShapeType typeB = bodyB.getShape()->getType();

	for ( int key = 0; key < MultiSphere->getNumSpheres(); key++ )
	{
		const hkVector4& sphere = MultiSphere->getSpheres()[ key ];
		hkVector4 off;	off._setRotatedDir( ms.getTransform().getRotation(), sphere );
		ms.getTransform().getTranslation().setAdd4( bodyA.getTransform().getTranslation(), off );
		sphereShape.setRadius( sphere(3) );
		newOperandA.setShape( &sphereShape, key );

		hkShapeType typeA = sphereShape.getType();
		hkCollisionDispatcher::LinearCastFunc linearCastFunc = input.m_dispatcher->getLinearCastFunc( typeA, typeB );
		linearCastFunc(newOperandA, bodyB, input, collector, startCollector );
	}

	HK_INTERNAL_TIMER_END();
}

void hkMultiSphereAgent::getPenetrations( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
{
	HK_INTERNAL_TIMER_BEGIN( "MultiSphere", this );

	const hkMultiSphereShape* MultiSphere = static_cast<const hkMultiSphereShape*>(bodyA.getShape());
	
	hkMotionState ms = *bodyA.getMotionState();
	hkSphereShape sphereShape(0.0f);
	hkCdBody newOperandA( &bodyA, &ms );
	

	KeyAgentPair* agentPair = m_agents.begin();

	for ( int i = m_agents.getSize() -1; i>=0; i-- )
	{
		const hkVector4& sphere = MultiSphere->getSpheres()[ agentPair->m_key ];
		hkVector4 off;	off._setRotatedDir( ms.getTransform().getRotation(), sphere );
		ms.getTransform().getTranslation().setAdd4( bodyA.getTransform().getTranslation(), off );

		sphereShape.setRadius( sphere(3) );
		newOperandA.setShape( &sphereShape, i );

		agentPair->m_agent->getPenetrations(newOperandA, bodyB, input, collector );
		if ( collector.getEarlyOut() )
		{
			break;
		}
		agentPair++;
	}

	HK_INTERNAL_TIMER_END();
}

void hkMultiSphereAgent::staticGetPenetrations( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
{
	HK_INTERNAL_TIMER_BEGIN( "MultiSphere" , this);

	const hkMultiSphereShape* MultiSphere = static_cast<const hkMultiSphereShape*>(bodyA.getShape());
	
	hkMotionState ms = *bodyA.getMotionState();
	hkSphereShape sphereShape(0.0f);
	hkCdBody newOperandA( &bodyA, &ms );
	
	hkShapeType typeB = bodyB.getShape()->getType();

	for ( int key = 0; key < MultiSphere->getNumSpheres(); key++ )
	{
		const hkVector4& sphere = MultiSphere->getSpheres()[ key ];
		hkVector4 off;	off._setRotatedDir( ms.getTransform().getRotation(), sphere );
		ms.getTransform().getTranslation().setAdd4( bodyA.getTransform().getTranslation(), off );
		sphereShape.setRadius( sphere(3) );
		newOperandA.setShape( &sphereShape, key );

		hkShapeType typeA = sphereShape.getType();
		hkCollisionDispatcher::GetPenetrationsFunc getPenetrationsFunc = input.m_dispatcher->getGetPenetrationsFunc( typeA, typeB );
		getPenetrationsFunc(newOperandA, bodyB, input, collector );
		if( collector.getEarlyOut() )
		{
			break;
		}
	}

	HK_INTERNAL_TIMER_END();
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
