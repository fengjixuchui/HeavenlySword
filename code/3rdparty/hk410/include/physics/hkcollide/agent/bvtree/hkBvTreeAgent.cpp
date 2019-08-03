/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>
#include <hkbase/memory/hkMemory.h>
#include <hkbase/debugutil/hkStatisticsCollector.h>

#include <hkmath/linear/hkSweptTransformUtil.h>
#include <hkcollide/agent/bvtree/hkBvTreeAgent.h>

#include <hkcollide/shape/collection/hkShapeCollection.h>
#include <hkcollide/shape/bvtree/hkBvTreeShape.h>
#include <hkcollide/shape/triangle/hkTriangleShape.h>

#include <hkcollide/agent/bvtree/hkBvTreeAgent.h>

#include <hkcollide/dispatch/hkAgentDispatchUtil.h>

#include <hkbase/htl/hkAlgorithm.h>


#include <hkcollide/agent/bvtree/hkBvTreeAgent.inl>

hkBvTreeAgent::hkBvTreeAgent( hkContactMgr* mgr )
:	hkCollisionAgent( mgr )
{
	m_cachedAabb.m_max.setAll3(HK_REAL_MAX);
	m_cachedAabb.m_min.setAll3(HK_REAL_MAX);
}

hkBool hkBvTreeAgent::m_useFastUpdate = false;
hkBool hkBvTreeAgent::m_useAabbCaching = true;

hkBool HK_CALL hkBvTreeAgent::getUseAabbCaching()
{
	return m_useAabbCaching;
}

void HK_CALL hkBvTreeAgent::setUseAabbCaching( hkBool useIt )
{
	m_useAabbCaching = useIt;
}

void HK_CALL hkBvTreeAgent::registerAgent(hkCollisionDispatcher* dispatcher)
{
	// register symmetric version
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc          = createBvTreeShapeAgent;
		af.m_getPenetrationsFunc = hkSymmetricAgent<hkBvTreeAgent>::staticGetPenetrations;
		af.m_getClosestPointFunc = hkSymmetricAgent<hkBvTreeAgent>::staticGetClosestPoints;
		af.m_linearCastFunc      = hkSymmetricAgent<hkBvTreeAgent>::staticLinearCast;
		af.m_isFlipped           = true;
		af.m_isPredictive		 = true;
		dispatcher->registerCollisionAgent(af, HK_SHAPE_BV_TREE, HK_SHAPE_ALL );
	}
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc          = createShapeBvAgent;
		af.m_getPenetrationsFunc = staticGetPenetrations;
		af.m_getClosestPointFunc = staticGetClosestPoints;
		af.m_linearCastFunc      = staticLinearCast;
		af.m_isFlipped           = false;
		af.m_isPredictive		 = true;
		dispatcher->registerCollisionAgent(af, HK_SHAPE_ALL, HK_SHAPE_BV_TREE );
	}
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc          = createBvBvAgent;
		af.m_getPenetrationsFunc = staticGetPenetrations;
		af.m_getClosestPointFunc = staticGetClosestPoints;
		af.m_linearCastFunc      = staticLinearCast;
		af.m_isFlipped           = false;
		af.m_isPredictive		 = true;
		dispatcher->registerCollisionAgent(af, HK_SHAPE_BV_TREE, HK_SHAPE_BV_TREE );
	}
}


hkCollisionAgent* HK_CALL hkBvTreeAgent::createBvTreeShapeAgent(	const hkCdBody& bodyA, 	const hkCdBody& bodyB,
																	const hkCollisionInput& input,	hkContactMgr* mgr )
{
	hkBvTreeAgent* agent = new hkSymmetricAgent<hkBvTreeAgent>( mgr );
	return agent;
}


hkCollisionAgent* HK_CALL hkBvTreeAgent::createShapeBvAgent(	const hkCdBody& bodyA, const hkCdBody& bodyB,
																	const hkCollisionInput& input,	hkContactMgr* mgr )
{
	hkBvTreeAgent* agent = new hkBvTreeAgent( mgr );
	return agent;
}



hkCollisionAgent* HK_CALL hkBvTreeAgent::createBvBvAgent(	const hkCdBody& bodyA, const hkCdBody& bodyB,
															const hkCollisionInput& input,	hkContactMgr* mgr )
{
	hkReal radiusA = bodyA.getMotionState()->m_objectRadius;
	hkReal radiusB = bodyB.getMotionState()->m_objectRadius;

		// we should call getAabb only on the smaller mopp tree, or
		// we risk to tall getAabb on a big landscape.
		// so if radiusA is smaller than radiusB it is allowed
		// to call bodyA->getAabb(). So we want to collide bodyA with mopp of bodyB
	if ( radiusA < radiusB)
	{
		hkBvTreeAgent* agent = new hkBvTreeAgent( mgr );
		return agent;
	}
	else
	{
		hkBvTreeAgent* agent = new hkSymmetricAgent<hkBvTreeAgent>( mgr );
		return agent;
	}
}



void hkBvTreeAgent::cleanup( hkCollisionConstraintOwner& info )
{
	hkArray<hkBvAgentEntryInfo>::iterator itr = m_collisionPartners.begin();
	hkArray<hkBvAgentEntryInfo>::iterator end = m_collisionPartners.end();

	while ( itr != end )
	{
		if (itr->m_collisionAgent != HK_NULL)
		{
			itr->m_collisionAgent->cleanup(info);
		}
		itr++;
	}

	delete this;
}

void hkBvTreeAgent::invalidateTim( hkCollisionInput& input )
{
	hkArray<hkBvAgentEntryInfo>::iterator itr = m_collisionPartners.begin();
	hkArray<hkBvAgentEntryInfo>::iterator end = m_collisionPartners.end();

	while ( itr != end )
	{
		if (itr->m_collisionAgent != HK_NULL)
		{
			itr->m_collisionAgent->invalidateTim(input);
		}
		itr++;
	}
}

void hkBvTreeAgent::warpTime( hkTime oldTime, hkTime newTime, hkCollisionInput& input )
{
	hkArray<hkBvAgentEntryInfo>::iterator itr = m_collisionPartners.begin();
	hkArray<hkBvAgentEntryInfo>::iterator end = m_collisionPartners.end();

	while ( itr != end )
	{
		if (itr->m_collisionAgent != HK_NULL)
		{
			itr->m_collisionAgent->warpTime( oldTime, newTime, input );
		}
		itr++;
	}
}

	// A helper class to use the hkAgentDispatchUtil
class hkAgentDispatchUtilHelper
{
	public:

		hkAgentDispatchUtilHelper( const hkCdBody& body )
			: m_bodyB( &body )
		{
		}

		const hkShapeCollection* m_collection;
		hkCdBody m_bodyB;
		
			// The following alignement command is required on the PS2 SN compiler.
			// Otherwise m_shapeBuffer is not 16-byte aligned, 
			// although the definition of hkShapeCollection::ShapeBuffer specifies it.
		HK_ALIGN16( hkShapeCollection::ShapeBuffer m_shapeBuffer );

		inline const hkCdBody* getBodyA( const hkCdBody& cIn, const hkCollisionInput& input, hkShapeKey key)
		{
			return &cIn;
		}

		inline const hkCdBody* getBodyB( const hkCdBody& cIn, const hkCollisionInput& input, hkShapeKey key )
		{
			m_bodyB.setShape( m_collection->getChildShape( key, m_shapeBuffer ), key);
			return &m_bodyB;
		}

		inline const hkShapeCollection* getShapeCollectionB( )
		{
			return m_collection;
		}
};

hkCollisionAgent* HK_CALL hkBvTreeAgent::defaultAgentCreate( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr )
{
	return input.m_dispatcher->getNewCollisionAgent( bodyA, bodyB, input, mgr );	
}

void hkBvTreeAgent::prepareCollisionPartners( const hkCdBody& bodyA,	const hkCdBody& bodyB,	const hkCollisionInput& input, hkCollisionConstraintOwner& constraintOwner )
{
	//
	// Calc the aabb
	//
	hkAabb aabb;
	{
		// compose transform
		hkTransform bTa;
		{
			const hkTransform& wTb = bodyB.getTransform();
			const hkTransform& wTa = bodyA.getTransform();
			bTa.setMulInverseMul( wTb, wTa );
		}


		//added an early out so if the aabb is the same, don't query the mopp and don't  sort, nor call the dispatch/agent
		{
			const hkReal checkEpsilon = input.m_tolerance * 0.5f;
			bodyA.getShape()->getAabb( bTa, input.m_tolerance + checkEpsilon, aabb );
			
			if ( m_useAabbCaching )
			{
				if (m_cachedAabb.contains( aabb ) )
				{
					return;
				}
				m_cachedAabb = aabb;
			}
		}
	}


	const hkBvTreeShape* bvB = static_cast<const hkBvTreeShape*>( bodyB.getShape() );

	//
	// query the BvTreeShape
	//
	hkInplaceArray<hkShapeKey,128> hitList;
	{
		bvB->queryAabb( aabb, hitList );
	}

	int newMemNeeded = ( hitList.getSize() - m_collisionPartners.getSize() ) * 128;

	if ( newMemNeeded > hkMemory::getInstance().getAvailableMemory() )
	{
		hkMemory::getInstance().m_memoryState = hkMemory::MEMORY_STATE_OUT_OF_MEMORY;
		return;
	}

	//
	//	update the m_collisionPartners
	//
	{
		const hkShapeCollection* shapeCollection = bvB->getShapeCollection();
		hkAgentDispatchUtilHelper helper(bodyB);
		helper.m_collection = shapeCollection;

		if(m_useFastUpdate)
		{
			hkAgentDispatchUtil<hkShapeKey, hkBvAgentEntryInfo, hkAgentDispatchUtilHelper>
				::fastUpdate( m_collisionPartners, hitList, bodyA, bodyB, input, m_contactMgr, constraintOwner, helper );
		}
		else
		{
			hkSort( hitList.begin(), hitList.getSize() );
			hkAgentDispatchUtil<hkShapeKey, hkBvAgentEntryInfo, hkAgentDispatchUtilHelper>
				::update( m_collisionPartners, hitList, bodyA, bodyB, input, m_contactMgr, constraintOwner, helper );
		}
	}
	// do a little checking
#if defined(HK_DEBUG)
	{
		for (int i = 0; i < hitList.getSize(); i++ )
		{
			const hkShapeKey& key = hitList[i];
			if ( ! ( key == m_collisionPartners[i].getKey() ) )
			{
				HK_ASSERT2(0x2e8d58bd,  0, "Internal consistency problem, probably a compiler error when havok libs where build" );
			}
		}
	}
#endif
	
}


void hkBvTreeAgent::prepareCollisionPartnersProcess( const hkCdBody& bodyA,	const hkCdBody& bodyB,	const hkProcessCollisionInput& input, hkCollisionConstraintOwner& constraintOwner )
{
	hkResult prepareSuccessfull;
	hkInplaceArray<hkShapeKey,128> hitList;
	{
		hkTransform bTa;
		{
			const hkTransform& wTb = bodyB.getTransform();
			const hkTransform& wTa = bodyA.getTransform();
			bTa.setMulInverseMul( wTb, wTa );
		}

		hkVector4 linearTimInfo;
		{
			const hkMotionState* msA = bodyA.getMotionState();
			const hkMotionState* msB = bodyB.getMotionState();
			hkSweptTransformUtil::calcTimInfo( *msA, *msB, input.m_stepInfo.m_deltaTime, linearTimInfo);
		}
		hkAabb* cachedAabb = (m_useAabbCaching)?&m_cachedAabb:HK_NULL;

		prepareSuccessfull = hkBvTreeAgent::calcAabbAndQueryTree( bodyA, bodyB, bTa, linearTimInfo, input, cachedAabb, hitList);
	}

	if ( prepareSuccessfull != HK_SUCCESS )
	{
		return;
	}

	//
	//	update the m_collisionPartners
	//
	{
		const hkBvTreeShape* bvB = static_cast<const hkBvTreeShape*>( bodyB.getShape() );
		const hkShapeCollection* shapeCollection = bvB->getShapeCollection();
		hkAgentDispatchUtilHelper helper(bodyB);
		helper.m_collection = shapeCollection;

		if(m_useFastUpdate)
		{
			hkAgentDispatchUtil<hkShapeKey, hkBvAgentEntryInfo, hkAgentDispatchUtilHelper>
				::fastUpdate( m_collisionPartners, hitList, bodyA, bodyB, input, m_contactMgr, constraintOwner, helper );
		}
		else
		{
			hkSort( hitList.begin(), hitList.getSize() );
			hkAgentDispatchUtil<hkShapeKey, hkBvAgentEntryInfo, hkAgentDispatchUtilHelper>
				::update( m_collisionPartners, hitList, bodyA, bodyB, input, m_contactMgr, constraintOwner, helper );
		}
	}
	// do a little checking
#if defined(HK_DEBUG)
	{
		for (int i = 0; i < hitList.getSize(); i++ )
		{
			const hkShapeKey& key = hitList[i];
			if ( ! ( key == m_collisionPartners[i].getKey() ) )
			{
				HK_ASSERT2(0x2e8d58bd,  0, "Internal consistency problem, probably a compiler error when havok libs where build" );
			}
		}
	}
#endif
	
}


void hkBvTreeAgent::calcAabbLinearCast(const hkCdBody& bodyA, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkAabb& aabbOut)
{
	// compose transform
	hkTransform bTa;
	{
		const hkTransform& wTb = bodyB.getTransform();
		const hkTransform& wTa = bodyA.getTransform();
		bTa.setMulInverseMul( wTb, wTa );
	}

	bodyA.getShape()->getAabb( bTa, input.m_tolerance, aabbOut );

	//
	//	expand the aabb
	//
	hkVector4 pathB; pathB.setRotatedInverseDir( bodyB.getTransform().getRotation(), input.m_path );
	hkVector4 zero; zero.setZero4();
	hkVector4 pathMin; pathMin.setMin4( zero, pathB );
	hkVector4 pathMax; pathMax.setMax4( zero, pathB );
	aabbOut.m_min.add4( pathMin );
	aabbOut.m_max.add4( pathMax );
}

void hkBvTreeAgent::staticCalcAabb(const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkAabb& aabbOut)
{
	// compose transform
	hkTransform bTa;
	{
		const hkTransform& wTb = bodyB.getTransform();
		const hkTransform& wTa = bodyA.getTransform();
		bTa.setMulInverseMul( wTb, wTa );
	}

	bodyA.getShape()->getAabb( bTa, input.m_tolerance, aabbOut );
}


/*
void hkBvTreeAgent::prepareCollisionPartnersLinearCast(const hkCdBody& bodyA, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkCollisionConstraintOwner& constraintOwner)
{
	//
	// Calc the aabb
	//
	hkAabb aabb;
	calcAabbLinearCast( bodyA, bodyB, input, aabb );
	m_cachedAabb = aabb;

	const hkBvTreeShape* bvB = static_cast<const hkBvTreeShape*>( bodyB.getShape() );

	//
	// query the BvTreeShape
	//
	hkInplaceArray<hkShapeKey,128> hitList;
	{
		bvB->queryAabb( aabb, hitList );
	}


	//
	//	update the m_collisionPartners
	//
	const hkShapeCollection* shapeCollection = bvB->getShapeCollection();
	{

		hkSort( hitList.begin(), hitList.getSize() );

		hkAgentDispatchUtilHelper helper(bodyB);
		helper.m_collection = shapeCollection;

		if(m_useFastUpdate)
		{
			hkAgentDispatchUtil<hkShapeKey, hkBvAgentEntryInfo, hkAgentDispatchUtilHelper>
				::fastUpdate( m_collisionPartners, hitList, bodyA, bodyB, input, m_contactMgr, constraintOwner, helper );
		}
		else
		{
			hkSort( hitList.begin(), hitList.getSize() );
			hkAgentDispatchUtil<hkShapeKey, hkBvAgentEntryInfo, hkAgentDispatchUtilHelper>
				::update( m_collisionPartners, hitList, bodyA, bodyB, input, m_contactMgr, constraintOwner, helper );
		}
	}
}
*/

void hkBvTreeAgent::updateShapeCollectionFilter( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCollisionConstraintOwner& constraintOwner )
{
	const hkBvTreeShape* bvB = static_cast<const hkBvTreeShape*>( bodyB.getShape() );
	const hkShapeCollection* shapeCollection = bvB->getShapeCollection();
	const hkShapeContainer* shapeContainer = shapeCollection->getContainer();

	hkShapeCollection::ShapeBuffer shapeBuffer;

	for ( int i = 0; i < m_collisionPartners.getSize(); ++i )
	{
		const hkShape* shape = shapeCollection->getChildShape( m_collisionPartners[i].getKey(), shapeBuffer );
		hkCdBody modifiedBodyB( &bodyB );
		modifiedBodyB.setShape( shape, m_collisionPartners[i].getKey() );

		if ( input.m_filter->isCollisionEnabled( input, bodyA, bodyB, *shapeContainer, m_collisionPartners[i].getKey() ) )
		{
			if ( m_collisionPartners[i].m_collisionAgent == hkNullAgent::getNullAgent() )
			{
				// Shape which was previously filtered is now not filtered. Create the agent
				m_collisionPartners[i].m_collisionAgent = input.m_dispatcher->getNewCollisionAgent( bodyA, modifiedBodyB, input, this->m_contactMgr );
			}
			else
			{
				// Shape was not previously filtered and is still not filtered
				m_collisionPartners[i].m_collisionAgent->updateShapeCollectionFilter( bodyA, modifiedBodyB, input, constraintOwner );
			}
		}
		else
		{
			// Shape is now filtered. If it was previously filtered do nothing. Check if it was not previously filtered.
			if ( m_collisionPartners[i].m_collisionAgent != hkNullAgent::getNullAgent() )
			{
				// Shape has just been filtered out. Delete the agent.
				m_collisionPartners[i].m_collisionAgent->cleanup( constraintOwner );
				m_collisionPartners[i].m_collisionAgent = hkNullAgent::getNullAgent();
			}
		}
	}
}



void hkBvTreeAgent::processCollision(	const hkCdBody& bodyA, const hkCdBody& bodyB,
										const hkProcessCollisionInput& input, 
										hkProcessCollisionOutput& result )
{
	HK_ASSERT2(0x352618d8,  m_contactMgr, HK_MISSING_CONTACT_MANAGER_ERROR_TEXT );

	HK_TIMER_BEGIN_LIST( "BvTree", "QueryTree" );
	prepareCollisionPartnersProcess( bodyA , bodyB , input, *result.m_constraintOwner );

	//
	// recursively process Collisions
	//
	hkShapeCollection::ShapeBuffer shapeBuffer;

	hkArray<hkBvAgentEntryInfo>::iterator itr = m_collisionPartners.begin();
	hkArray<hkBvAgentEntryInfo>::iterator end = m_collisionPartners.end();

	hkCdBody modifiedBodyB( &bodyB );

	HK_TIMER_SPLIT_LIST("NarrowPhase");
	const hkBvTreeShape* bvB = static_cast<const hkBvTreeShape*>(bodyB.getShape());
	const hkShapeCollection* shapeCollection = bvB->getShapeCollection();
	while ( itr != end )
	{
		const hkShape* shape = shapeCollection->getChildShape( itr->m_key, shapeBuffer );
		modifiedBodyB.setShape( shape, itr->m_key );

		if(0)
		{
			const hkTriangleShape* t = static_cast<const hkTriangleShape*>(shape);

			hkVector4 offset = bodyB.getTransform().getTranslation();
			hkVector4 a; a.setAdd4(t->getVertex(0), offset );
			hkVector4 b; b.setAdd4(t->getVertex(1), offset );
			hkVector4 c; c.setAdd4(t->getVertex(2), offset );
			
			hkVector4 center; center.setAdd4( a, b);
			center.add4( c);
			center.mul4( 1.0f/ 3.0f);


			//HK_DISPLAY_LINE( a, b, hkColor::YELLOW );
			//HK_DISPLAY_LINE( a, c, hkColor::YELLOW );
			//HK_DISPLAY_LINE( b, c, hkColor::YELLOW );
			//HK_DISPLAY_LINE( center, a, hkColor::YELLOW );
			//HK_DISPLAY_LINE( center, b, hkColor::YELLOW );
			//HK_DISPLAY_LINE( center, c, hkColor::YELLOW );
		}

		itr->m_collisionAgent->processCollision( bodyA, modifiedBodyB, input, result );
		itr++;
	}
	//HK_ON_DEBUG( hkprintf("Memory %i\n", m_collisionPartners.getSize() * ( 512 + sizeof(hkBvAgentEntryInfo) ) ) );
	HK_TIMER_END_LIST();
}



void hkBvTreeAgent::linearCast( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkCdPointCollector& collector, hkCdPointCollector* startCollector )
{
	hkBvTreeAgent::staticLinearCast( bodyA, bodyB, input, collector, startCollector );
/*

	HK_TIMER_BEGIN_LIST( "BvTree", "QueryTree" );

	prepareCollisionPartnersLinearCast( bodyA , bodyB , input);

	//
	// recursively collect linearCast results and Contact Points
	//

	HK_TIMER_SPLIT_LIST( "NarrowPhase" );

	{
		hkArray<hkBvAgentEntryInfo>::iterator itr = m_collisionPartners.begin();
		hkArray<hkBvAgentEntryInfo>::iterator end = m_collisionPartners.end();

		hkCdBody modifiedBodyB( &bodyB );
		const hkBvTreeShape* bvB = static_cast<const hkBvTreeShape*>(bodyB.getShape());
		hkShapeCollection::ShapeBuffer shapeBuffer;
		const hkShapeCollection* shapeCollection = bvB->getShapeCollection();

		while ( itr != end )
		{
			const hkShape* shape = shapeCollection->getChildShape( itr->m_key, shapeBuffer );
			modifiedBodyB.setShape( shape, itr->m_key );
			itr->m_collisionAgent->linearCast( bodyA, modifiedBodyB, input, collector, startCollector );
			itr++;
		}
	}

	HK_TIMER_END_LIST();
*/
}

void hkBvTreeAgent::staticLinearCast( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkCdPointCollector& collector, hkCdPointCollector* startCollector )
{
	HK_TIMER_BEGIN_LIST( "BvTree", "QueryTree" );

	//
	//	Get the aabb
	//
	hkAabb aabb;
	calcAabbLinearCast( bodyA, bodyB, input, aabb );

	const hkBvTreeShape* bvB = static_cast<const hkBvTreeShape*>( bodyB.getShape() );

	//
	// query the BvTreeShape
	//
	hkInplaceArray<hkShapeKey,128> hitList;
	{
		bvB->queryAabb( aabb, hitList );
	}

	//
	// recursively collect linearCast results and Contact Points
	//

	HK_TIMER_SPLIT_LIST( "NarrowPhase" );

	{
		hkShapeType typeA = bodyA.getShape()->getType();

		hkArray<hkShapeKey>::iterator itr = hitList.begin();
		hkArray<hkShapeKey>::iterator end = hitList.end();

		hkCdBody modifiedBodyB( &bodyB );

		hkShapeCollection::ShapeBuffer shapeBuffer;
		const hkShapeCollection* shapeCollection = bvB->getShapeCollection();
		const hkShapeContainer* shapeContainer = shapeCollection->getContainer();

		while ( itr != end )
		{
			if ( input.m_filter->isCollisionEnabled( input, bodyA, bodyB, *shapeContainer , *itr ) )
			{
			        const hkShape* shape = shapeCollection->getChildShape( *itr, shapeBuffer );
			        modifiedBodyB.setShape( shape, *itr );
			        hkShapeType typeB = shape->getType();
			        hkCollisionDispatcher::LinearCastFunc linCastFunc = input.m_dispatcher->getLinearCastFunc( typeA, typeB );
			        linCastFunc( bodyA, modifiedBodyB, input, collector, startCollector );
			}
			itr++;
		}
	}

	HK_TIMER_END_LIST();
}

void hkBvTreeAgent::getClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& collector  )
{
	hkBvTreeAgent::staticGetClosestPoints( bodyA, bodyB, input, collector );
/*
	HK_TIMER_BEGIN_LIST( "BvTree", "QueryTree" );

	prepareCollisionPartners( bodyA , bodyB , input);

	//
	// recursively call getClosestPoints
	//

	HK_TIMER_SPLIT_LIST( "NarrowPhase" );

	{
		hkArray<hkBvAgentEntryInfo>::iterator itr = m_collisionPartners.begin();
		hkArray<hkBvAgentEntryInfo>::iterator end = m_collisionPartners.end();

		hkCdBody modifiedBodyB( &bodyB );

		const hkBvTreeShape* bvB = static_cast<const hkBvTreeShape*>(bodyB.getShape());
		hkShapeCollection::ShapeBuffer shapeBuffer;
		const hkShapeCollection* shapeCollection = bvB->getShapeCollection();
		while ( itr != end )
		{
			const hkShape* shape = shapeCollection->getChildShape( itr->m_key, shapeBuffer );
			modifiedBodyB.setShape( shape, itr->m_key );
			itr->m_collisionAgent->getClosestPoints( bodyA, modifiedBodyB, input, collector );
			itr++;
		}
	}

	HK_TIMER_END_LIST();
*/

}

void hkBvTreeAgent::staticGetClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& collector  )
{
	HK_TIMER_BEGIN_LIST( "BvTree", "QueryTree" );

	//
	//	Get the aabb
	//
	hkAabb aabb;
	staticCalcAabb( bodyA, bodyB, input, aabb );

	const hkBvTreeShape* bvB = static_cast<const hkBvTreeShape*>( bodyB.getShape() );

	//
	// query the BvTreeShape
	//
	hkInplaceArray<hkShapeKey,128> hitList;
	{
		bvB->queryAabb( aabb, hitList );
	}

	//
	// recursively call getClosestPoints
	//
	HK_TIMER_SPLIT_LIST( "NarrowPhase" );
	{
		hkShapeType typeA = bodyA.getShape()->getType();

		hkArray<hkShapeKey>::iterator itr = hitList.begin();
		hkArray<hkShapeKey>::iterator end = hitList.end();

		hkCdBody modifiedBodyB( &bodyB );

		hkShapeCollection::ShapeBuffer shapeBuffer;
		const hkShapeCollection* shapeCollection = bvB->getShapeCollection();
		const hkShapeContainer* shapeContainer = shapeCollection->getContainer();

		while ( itr != end )
		{
			if ( input.m_filter->isCollisionEnabled( input, bodyA, bodyB, *shapeContainer , *itr ) )
			{
				const hkShape* shape = shapeCollection->getChildShape( *itr, shapeBuffer );
				modifiedBodyB.setShape( shape, *itr );
				hkShapeType typeB = shape->getType();
				hkCollisionDispatcher::GetClosestPointsFunc getClosestPoints = input.m_dispatcher->getGetClosestPointsFunc( typeA, typeB );
				getClosestPoints( bodyA, modifiedBodyB, input, collector );
			}
			itr++;
		}
	}

	HK_TIMER_END_LIST();
}


void hkBvTreeAgent::getPenetrations( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
{
	hkBvTreeAgent::staticGetPenetrations( bodyA, bodyB, input, collector);

/*
	HK_TIMER_BEGIN_LIST( "BvTree", "QueryTree" );

	prepareCollisionPartners( bodyA , bodyB , input);

	//
	// recursively check penetrations
	//
	HK_TIMER_SPLIT_LIST( "NarrowPhase" );

	{
		hkArray<hkBvAgentEntryInfo>::iterator itr = m_collisionPartners.begin();
		hkArray<hkBvAgentEntryInfo>::iterator end = m_collisionPartners.end();

		hkCdBody modifiedBodyB( &bodyB );
		const hkBvTreeShape* bvB = static_cast<const hkBvTreeShape*>(bodyB.getShape());
		const hkShapeCollection* shapeCollection = bvB->getShapeCollection();
		hkShapeCollection::ShapeBuffer shapeBuffer;

		while ( itr != end )
		{
			const hkShape* shape = shapeCollection->getChildShape( itr->m_key, shapeBuffer );
			modifiedBodyB.setShape( shape, itr->m_key );
			itr->m_collisionAgent->getPenetrations( bodyA, modifiedBodyB, input, collector );

			if ( collector.getEarlyOut() )
			{
				break;
			}
			itr++;
		}
	}

	HK_TIMER_END_LIST();*/

}

void hkBvTreeAgent::staticGetPenetrations( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
{
	HK_TIMER_BEGIN_LIST( "BvTree", "QueryTree" );

	//
	//	Get the aabb
	//
	hkAabb aabb;
	staticCalcAabb( bodyA, bodyB, input, aabb );

	const hkBvTreeShape* bvB = static_cast<const hkBvTreeShape*>( bodyB.getShape() );

	//
	// query the BvTreeShape
	//
	hkInplaceArray<hkShapeKey,128> hitList;
	{
		bvB->queryAabb( aabb, hitList );
	}
	HK_TIMER_SPLIT_LIST( "NarrowPhase" );

	//
	// recursively check penetrations
	//

	{
		hkShapeType typeA = bodyA.getShape()->getType();

		hkArray<hkShapeKey>::iterator itr = hitList.begin();
		hkArray<hkShapeKey>::iterator end = hitList.end();

		hkCdBody modifiedBodyB( &bodyB );

		hkShapeCollection::ShapeBuffer shapeBuffer;
		const hkShapeCollection* shapeCollection = bvB->getShapeCollection();
		const hkShapeContainer* shapeContainer = shapeCollection->getContainer();

		while ( itr != end )
		{
			if ( input.m_filter->isCollisionEnabled( input, bodyA, bodyB, *shapeContainer , *itr ) )
			{
			    const hkShape* shape = shapeCollection->getChildShape( *itr, shapeBuffer );
			    modifiedBodyB.setShape( shape, *itr );
			    hkShapeType typeB = shape->getType();
			    hkCollisionDispatcher::GetPenetrationsFunc getPenetrationsFunc = input.m_dispatcher->getGetPenetrationsFunc( typeA, typeB );

			    getPenetrationsFunc( bodyA, modifiedBodyB, input, collector );

			    if (collector.getEarlyOut() )
			    {
			 	    break;
				}
			}
			itr++;
		}
	}

	HK_TIMER_END_LIST();
}

void hkBvTreeAgent::calcStatistics( hkStatisticsCollector* collector ) const
{
	collector->beginObject( "BvTreeAgt", collector->MEMORY_RUNTIME, this );

	collector->addArray( "AgentPtrs", collector->MEMORY_RUNTIME, m_collisionPartners );

	hkArray<hkBvAgentEntryInfo>::const_iterator itr = m_collisionPartners.begin();
	hkArray<hkBvAgentEntryInfo>::const_iterator end = m_collisionPartners.end();

	while ( itr != end )
	{
		if (itr->m_collisionAgent != HK_NULL)
		{
			collector->addChildObject( "Agent", collector->MEMORY_RUNTIME, itr->m_collisionAgent );
		}
		itr++;
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
