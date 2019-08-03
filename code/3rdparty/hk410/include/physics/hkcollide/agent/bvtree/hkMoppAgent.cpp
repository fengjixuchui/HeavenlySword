/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>

#include <hkcollide/shape/mopp/hkMoppBvTreeShape.h>
#include <hkcollide/shape/collection/hkShapeCollection.h>

#include <hkcollide/agent/bvtree/hkMoppAgent.h>

#include <hkinternal/collide/mopp/machine/hkMoppAabbCastVirtualMachine.h>

#include <hkcollide/dispatch/hkAgentDispatchUtil.h>

#ifdef HK_MOPP_DEBUGGER_ENABLED
#	include <hkcollide/collector/pointcollector/hkClosestCdPointCollector.h>
#endif

#include <hkbase/htl/hkAlgorithm.h>


hkMoppAgent::hkMoppAgent( hkContactMgr* mgr )
:	hkBvTreeAgent( mgr )
{
}


void HK_CALL hkMoppAgent::registerAgent(hkCollisionDispatcher* dispatcher)
{
	// register symmetric version
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc          = createBvTreeShapeAgent;
		af.m_getPenetrationsFunc  = hkSymmetricAgent<hkMoppAgent>::staticGetPenetrations;
		af.m_getClosestPointFunc = hkSymmetricAgent<hkMoppAgent>::staticGetClosestPoints;
		af.m_linearCastFunc      = hkSymmetricAgent<hkMoppAgent>::staticLinearCast;
		af.m_isFlipped           = true;
		af.m_isPredictive		 = false;
		dispatcher->registerCollisionAgent(af, HK_SHAPE_MOPP, HK_SHAPE_ALL );
	}
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc          = createShapeBvAgent;
		af.m_getPenetrationsFunc  = staticGetPenetrations;
		af.m_getClosestPointFunc = staticGetClosestPoints;
		af.m_linearCastFunc      = staticLinearCast;
		af.m_isFlipped           = false;
		af.m_isPredictive		 = false;
		dispatcher->registerCollisionAgent(af, HK_SHAPE_ALL, HK_SHAPE_MOPP );
	}

	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc          = createBvBvAgent;
		af.m_getPenetrationsFunc  = staticGetPenetrations;
		af.m_getClosestPointFunc = staticGetClosestPoints;
		af.m_linearCastFunc      = staticLinearCast;
		af.m_isFlipped           = false;
		af.m_isPredictive		 = true;
		dispatcher->registerCollisionAgent(af, HK_SHAPE_MOPP, HK_SHAPE_MOPP );
	}
}


hkCollisionAgent* HK_CALL hkMoppAgent::createBvBvAgent(	const hkCdBody& bodyA, const hkCdBody& bodyB,
															const hkCollisionInput& input,	hkContactMgr* mgr )
{
	const hkMoppBvTreeShape* bvA = static_cast<const hkMoppBvTreeShape*>( bodyA.getShape() );
	const hkMoppBvTreeShape* bvB = static_cast<const hkMoppBvTreeShape*>( bodyB.getShape() );

	// This is where a dodgy mopp gets caught if it is added to a dispatcher on load from
	// the serialization.
	HK_ASSERT2( 0xec6c2e4d, bvA->getMoppCode() && bvB->getMoppCode(), "No Mopp Code in a MoppBvTreeShape.");

	int sizeA = bvA->getMoppCode()->m_data.getSize();
	int sizeB = bvB->getMoppCode()->m_data.getSize();

		// we should call getAabb only on the smaller mopp tree, or
		// we risk to tall getAabb on a big landscape.
		// so if radiusA is smaller than radiusB it is allowed
		// to call bodyA->getAabb(). So we want to collide bodyA with mopp of bodyB
	if ( sizeA < sizeB)
	{
		hkBvTreeAgent* agent = new hkMoppAgent( mgr );
		return agent;
	}
	else
	{
		hkBvTreeAgent* agent = new hkSymmetricAgent<hkMoppAgent>( mgr );
		return agent;
	}
}

void hkMoppAgent::staticLinearCast( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkCdPointCollector& collector, hkCdPointCollector* startCollector )
{

#ifdef HK_MOPP_DEBUGGER_ENABLED
	hkClosestCdPointCollector debugCollector;
	{
		hkAabb aabb;	calcAabbLinearCast( bodyA, bodyB, input, aabb );

		const hkMoppBvTreeShape* bvB = static_cast<const hkMoppBvTreeShape*>( bodyB.getShape() );

		//
		// query the BvTreeShape
		//
		hkInplaceArray<hkShapeKey,128> hitList;
		{
			bvB->queryAabb( aabb, hitList );
		}

		{
			hkShapeType typeA = bodyA.getShape()->getType();

			hkArray<hkShapeKey>::iterator itr = hitList.begin();
			hkArray<hkShapeKey>::iterator end = hitList.end();

			hkCdBody modifiedBodyB( &bodyB );

			hkShapeCollection::ShapeBuffer( shapeStorage, shapeBuffer );
			const hkShapeCollection* shapeCollection = bvB->getShapeCollection();

			while ( itr != end )
			{
				const hkShape* shape = shapeCollection->getChildShape( *itr, shapeBuffer );
				modifiedBodyB.setShape( shape, *itr );
				hkShapeType typeB = shape->getType();
				hkCollisionDispatcher::LinearCastFunc linCastFunc = input.m_dispatcher->getLinearCastFunc( typeA, typeB );

				linCastFunc( bodyA, modifiedBodyB, input, debugCollector, HK_NULL );
				itr++;
			}
		}
		hkMoppDebugger::getInstance().initDisabled();
		if ( debugCollector.hasHit() )
		{
			hkShapeKey hitKey = debugCollector.getHit().m_shapeKeyB;
			hkMoppDebugger::getInstance().initUsingCodeAndTri( bvB->getMoppCode(), hitKey );
		}

	}
#endif


	HK_TIMER_BEGIN( "Mopp", HK_NULL );

		// get the aabb
	hkAabb aabb;
	{
		hkTransform bTa;
		{
			const hkTransform& wTb = bodyB.getTransform();
			const hkTransform& wTa = bodyA.getTransform();
			bTa.setMulInverseMul( wTb, wTa );
		}
		bodyA.getShape()->getAabb( bTa, input.m_tolerance, aabb );
	}
	//
	//	expand the aabb
	//
	hkVector4 pathB; pathB.setRotatedInverseDir( bodyB.getTransform().getRotation(), input.m_path );

	hkMoppAabbCastVirtualMachine::hkAabbCastInput ai;
	ai.m_castBody = &bodyA;
	ai.m_moppBody = &bodyB;
	ai.m_from.setInterpolate4( aabb.m_min, aabb.m_max, 0.5f );
	ai.m_to.setAdd4( ai.m_from, pathB );
	ai.m_extents.setSub4( aabb.m_max, aabb.m_min );
	ai.m_extents.mul4( 0.5f );
	hkVector4 tol4; tol4.setAll3( input.getTolerance() );
	ai.m_extents.add4( tol4 );
	ai.m_collisionInput = &input;

	hkMoppAabbCastVirtualMachine machine;
	machine.aabbCast( ai, collector, startCollector );
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
