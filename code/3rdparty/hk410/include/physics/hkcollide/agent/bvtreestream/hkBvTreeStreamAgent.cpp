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

#include <hkcollide/shape/bvtree/hkBvTreeShape.h>
#include <hkcollide/shape/triangle/hkTriangleShape.h>
#include <hkcollide/shape/collection/hkShapeCollection.h>

#include <hkcollide/agent/hkCollisionQualityInfo.h>

#include <hkcollide/agent/bvtree/hkBvTreeAgent.h>
#include <hkcollide/agent/bvtreestream/hkBvTreeStreamAgent.h>
#include <hkinternal/collide/agent3/machine/1n/hkAgent1nMachine.h>
#include <hkinternal/collide/agent3/machine/hkAgentMachineUtil.h>
#include <hkbase/htl/hkAlgorithm.h>

#include <hkcollide/filter/hkConvexListFilter.h>
#include <hkcollide/agent/shapecollection/hkShapeCollectionAgent.h>

#ifdef HK_DEBUG
//#	define HK_BV_TREE_DISPLAY_AABB
//#	define HK_DISPLAY_TRIANGLES
#endif

#if defined(HK_BV_TREE_DISPLAY_AABB) || defined( HK_DISPLAY_TRIANGLES )
#	include <hkvisualize/hkDebugDisplay.h>
#endif

#include <hkcollide/agent/bvtree/hkBvTreeAgent.inl>



hkBvTreeStreamAgent::hkBvTreeStreamAgent( const hkCdBody& bodyA, 	const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr )
:	hkCollisionAgent( mgr )
{
	m_dispatcher = input.m_dispatcher;
	m_cachedAabb.m_min.setZero4();
	m_cachedAabb.m_max.setZero4();
	hkAgent1nMachine_Create( m_agentTrack );
}

void HK_CALL hkBvTreeStreamAgent::registerAgent(hkCollisionDispatcher* dispatcher)
{
	// register symmetric version
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc           = createBvTreeShapeAgent;
		af.m_getPenetrationsFunc  = hkSymmetricAgent<hkBvTreeAgent>::staticGetPenetrations;
		af.m_getClosestPointFunc  = hkSymmetricAgent<hkBvTreeAgent>::staticGetClosestPoints;
		af.m_linearCastFunc       = hkSymmetricAgent<hkBvTreeAgent>::staticLinearCast;
		af.m_isFlipped            = true;
		af.m_isPredictive		  = true;
		dispatcher->registerCollisionAgent(af, HK_SHAPE_BV_TREE, HK_SHAPE_CONVEX );
	}
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc          =  createShapeBvAgent;
		af.m_getPenetrationsFunc  = hkBvTreeAgent::staticGetPenetrations;
		af.m_getClosestPointFunc =  hkBvTreeAgent::staticGetClosestPoints;
		af.m_linearCastFunc      =  hkBvTreeAgent::staticLinearCast;
		af.m_isFlipped           = false;
		af.m_isPredictive		  = true;
		dispatcher->registerCollisionAgent(af, HK_SHAPE_CONVEX, HK_SHAPE_BV_TREE );
	}
}

void HK_CALL hkBvTreeStreamAgent::registerConvexListAgent(hkCollisionDispatcher* dispatcher)
{
	// register symmetric version
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc           = dispatchBvTreeConvexList;
		af.m_getPenetrationsFunc  = hkSymmetricAgent<hkBvTreeAgent>::staticGetPenetrations;
		af.m_getClosestPointFunc  = hkSymmetricAgent<hkBvTreeAgent>::staticGetClosestPoints;
		af.m_linearCastFunc       = hkSymmetricAgent<hkBvTreeAgent>::staticLinearCast;
		af.m_isFlipped            = true;
		af.m_isPredictive		  = true;
		dispatcher->registerCollisionAgent(af, HK_SHAPE_BV_TREE, HK_SHAPE_CONVEX_LIST );
	}
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc          =  dispatchConvexListBvTree;
		af.m_getPenetrationsFunc  = hkBvTreeAgent::staticGetPenetrations;
		af.m_getClosestPointFunc =  hkBvTreeAgent::staticGetClosestPoints;
		af.m_linearCastFunc      =  hkBvTreeAgent::staticLinearCast;
		af.m_isFlipped           = false;
		af.m_isPredictive		  = true;
		dispatcher->registerCollisionAgent(af, HK_SHAPE_CONVEX_LIST, HK_SHAPE_BV_TREE );
	}
}

void HK_CALL hkBvTreeStreamAgent::registerMultiRayAgent(hkCollisionDispatcher* dispatcher)
{
	// register symmetric version
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc           = createBvTreeShapeAgent;
		af.m_getPenetrationsFunc  = hkSymmetricAgent<hkBvTreeAgent>::staticGetPenetrations;
		af.m_getClosestPointFunc  = hkSymmetricAgent<hkBvTreeAgent>::staticGetClosestPoints;
		af.m_linearCastFunc       = hkSymmetricAgent<hkBvTreeAgent>::staticLinearCast;
		af.m_isFlipped            = true;
		af.m_isPredictive		  = true;
		dispatcher->registerCollisionAgent(af, HK_SHAPE_BV_TREE, HK_SHAPE_MULTI_RAY );
	}
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc          =  createShapeBvAgent;
		af.m_getPenetrationsFunc  = hkBvTreeAgent::staticGetPenetrations;
		af.m_getClosestPointFunc =  hkBvTreeAgent::staticGetClosestPoints;
		af.m_linearCastFunc      =  hkBvTreeAgent::staticLinearCast;
		af.m_isFlipped           = false;
		af.m_isPredictive		  = true;
		dispatcher->registerCollisionAgent(af, HK_SHAPE_MULTI_RAY, HK_SHAPE_BV_TREE );
	}
}

hkCollisionAgent* HK_CALL hkBvTreeStreamAgent::dispatchBvTreeConvexList(	const hkCdBody& bodyA, 	const hkCdBody& bodyB,
																		const hkCollisionInput& input,	hkContactMgr* mgr )
{
	hkCollisionAgent* agent;
	if ( mgr )
	{
		hkConvexListFilter::ConvexListCollisionType collisionType = input.m_convexListFilter->getConvexListCollisionType( bodyB, bodyA, input );
		switch( collisionType )
		{
		case hkConvexListFilter::TREAT_CONVEX_LIST_AS_NORMAL:
		case hkConvexListFilter::TREAT_CONVEX_LIST_AS_CONVEX:			
			{
				// If we treat the object as a convex list (or convex), dispatch to the bvTree stream
				// in this case welding will work for triangles colliding with the outer hull of the object.
				agent = new hkSymmetricAgent<hkBvTreeStreamAgent>(bodyA, bodyB, input, mgr);
				break;
			}
		case hkConvexListFilter::TREAT_CONVEX_LIST_AS_LIST:
			{
				// If we treat the object as a list shape, dispatch to the shape collection agent
				// (the convex list shape is treated as the shape collection)
				// In this case welding fully works
				agent = new hkSymmetricAgent<hkShapeCollectionAgent>(bodyA, bodyB, input, mgr);
				break;
			}
		default:
			{
				agent = HK_NULL;
				HK_ASSERT2(0xeaf09646, 0, "Unknown ConvexListCollisionType returned");
			}
		}
	}
	else
	{
		agent = new hkSymmetricAgent<hkBvTreeStreamAgent>( bodyA, bodyB, input, mgr );
	}
	return agent;
}

hkCollisionAgent* HK_CALL hkBvTreeStreamAgent::dispatchConvexListBvTree(	const hkCdBody& bodyA, 	const hkCdBody& bodyB,
																			const hkCollisionInput& input,	hkContactMgr* mgr )
{
	hkCollisionAgent* agent;
	if ( mgr )
	{
		hkConvexListFilter::ConvexListCollisionType collisionType = input.m_convexListFilter->getConvexListCollisionType( bodyA, bodyB, input );
		switch( collisionType )
		{
		case hkConvexListFilter::TREAT_CONVEX_LIST_AS_NORMAL:
		case hkConvexListFilter::TREAT_CONVEX_LIST_AS_CONVEX:			
			{
				// If we treat the object as a convex list (or convex), dispatch to the bvTree stream
				// in this case welding will work for triangles colliding with the outer hull of the object.
				agent = new hkBvTreeStreamAgent(bodyA, bodyB, input, mgr);
				break;
			}
		case hkConvexListFilter::TREAT_CONVEX_LIST_AS_LIST:
			{
				// If we treat the object as a list shape, dispatch to the shape collection agent 
				// (the convex list shape is treated as the shape collection)
				// In this case welding fully works
				agent = new hkShapeCollectionAgent(bodyA, bodyB, input, mgr);
				break;
			}
		default:
			{
				agent = HK_NULL;
				HK_ASSERT2(0xeaf09646, 0, "Unknown ConvexListCollisionType returned");
			}
		}
	}
	else
	{
		agent = new hkSymmetricAgent<hkBvTreeStreamAgent>( bodyA, bodyB, input, mgr );
	}
	return agent;
}


hkCollisionAgent* HK_CALL hkBvTreeStreamAgent::createBvTreeShapeAgent(	const hkCdBody& bodyA, 	const hkCdBody& bodyB,
																	  const hkCollisionInput& input,	hkContactMgr* mgr )
{
	hkBvTreeStreamAgent* agent = new hkSymmetricAgent<hkBvTreeStreamAgent>( bodyA, bodyB, input, mgr );
	return agent;
}


hkCollisionAgent* HK_CALL hkBvTreeStreamAgent::createShapeBvAgent(	const hkCdBody& bodyA, const hkCdBody& bodyB,
																  const hkCollisionInput& input,	hkContactMgr* mgr )
{
	hkBvTreeStreamAgent* agent = new hkBvTreeStreamAgent( bodyA, bodyB, input, mgr );
	return agent;
}

void hkBvTreeStreamAgent::getPenetrations( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
{
	hkBvTreeAgent::staticGetPenetrations( bodyA, bodyB, input, collector);
}


void hkBvTreeStreamAgent::getClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& collector ) 
{
	hkBvTreeAgent::staticGetClosestPoints( bodyA, bodyB, input, collector );
}


void hkBvTreeStreamAgent::linearCast( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkCdPointCollector& collector, hkCdPointCollector* startCollector )
{
	hkBvTreeAgent::staticLinearCast( bodyA, bodyB, input, collector, startCollector );
}




void hkBvTreeStreamAgent::cleanup( hkCollisionConstraintOwner& info )
{
	hkAgent1nMachine_Destroy( m_agentTrack, m_dispatcher, m_contactMgr, info );
	delete this;
}

#ifdef HK_DISPLAY_TRIANGLES
static inline void HK_CALL hkBvTreeStreamAgent_displayTriangle( const hkTransform& transform, const hkShapeCollection* collection, hkShapeKey key )
{
	hkShapeCollection::ShapeBuffer shapeBuffer;

	const hkShape* shape = collection->getChildShape( key, shapeBuffer );
	if ( shape->getType() != HK_SHAPE_TRIANGLE)
	{
		return;
	}

	const hkTriangleShape* t = static_cast<const hkTriangleShape*>(shape);

	hkVector4 a; a.setTransformedPos(transform, t->getVertex(0));
	hkVector4 b; b.setTransformedPos(transform, t->getVertex(1));
	hkVector4 c; c.setTransformedPos(transform, t->getVertex(2));
	
	hkVector4 center; center.setAdd4( a, b);
	center.add4( c);
	center.mul4( 1.0f/ 3.0f);


	HK_DISPLAY_LINE( a, b, hkColor::YELLOW );
	HK_DISPLAY_LINE( a, c, hkColor::YELLOW );
	HK_DISPLAY_LINE( b, c, hkColor::YELLOW );
}
#endif

void hkBvTreeStreamAgent::processCollision(	const hkCdBody& bodyA, const hkCdBody& bodyB,
											const hkProcessCollisionInput& input, 	hkProcessCollisionOutput& output )
{
	HK_TIMER_BEGIN_LIST( "BvTree3", "QueryTree" );

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

	hkInplaceArray<hkShapeKey,128> hitList;
	hitList.pushBackUnchecked( HK_INVALID_SHAPE_KEY );

	hkResult prepareSuccessful;
	{
		hkTransform bTa;	bTa.setInverse( in3.m_aTb );
		prepareSuccessful = hkBvTreeAgent::calcAabbAndQueryTree( bodyA, bodyB, bTa, in3.m_linearTimInfo, input, &m_cachedAabb, hitList);
	}
#ifdef HK_DISPLAY_TRIANGLES
	{
		for ( int i =1; i < hitList.getSize();i++ )
		{
			const hkBvTreeShape* bvB = static_cast<const hkBvTreeShape*>(bodyB.getShape());
			const hkShapeCollection* shapeCollection = bvB->getShapeCollection();
			hkBvTreeStreamAgent_displayTriangle( bodyB.getTransform(), shapeCollection, hitList[i] );
		}
	}
#endif

	HK_TIMER_SPLIT_LIST("Narrow");
	if ( prepareSuccessful == HK_SUCCESS )
	{
		int newMemNeeded = ((hitList.getSize() / 4) + 1 - m_agentTrack.m_sectors.getSize()) * hkAgent1nSector::SECTOR_SIZE;

		if ( newMemNeeded > hkMemory::getInstance().getAvailableMemory() )
		{
			hkMemory::getInstance().m_memoryState = hkMemory::MEMORY_STATE_OUT_OF_MEMORY;
			return;
		}

		hkSort( hitList.begin(), hitList.getSize() );

		HK_ASSERT2 (0xf0432345, hitList[ hitList.getSize()-1 ] == HK_INVALID_SHAPE_KEY, 
								"Your result from queryAabb deleted the HK_INVALID_SHAPE_KEY entry" );
		const hkBvTreeShape* bvB = static_cast<const hkBvTreeShape*>(bodyB.getShape());
		const hkShapeCollection* shapeCollection = bvB->getShapeCollection();
		hkAgent1nMachine_Process( m_agentTrack, in3, shapeCollection, hitList.begin(), hitList.getSize(), output );
	}
	else
	{
		const hkBvTreeShape* bvB = static_cast<const hkBvTreeShape*>(bodyB.getShape());
		const hkShapeCollection* shapeCollection = bvB->getShapeCollection();
		hkAgent1nMachine_Process( m_agentTrack, in3, shapeCollection, HK_NULL, 0, output );
	}
	HK_TIMER_END_LIST();
}

void hkBvTreeStreamAgent::updateShapeCollectionFilter( const hkCdBody& bodyA, const hkCdBody& bvTreeBodyB, const hkCollisionInput& input, hkCollisionConstraintOwner& constraintOwner )
{
		// invalid cached
	m_cachedAabb.m_min.setZero4();
	m_cachedAabb.m_max.setZero4();

	const hkBvTreeShape* bvB = static_cast<const hkBvTreeShape*>(bvTreeBodyB.getShape());
	const hkShapeCollection* shapeCollection = bvB->getShapeCollection();

	hkAgent1nMachine_VisitorInput vin;
	vin.m_bodyA = &bodyA;
	vin.m_bvTreeBodyB = &bvTreeBodyB;
	vin.m_input = &input;
	vin.m_contactMgr = m_contactMgr;
	vin.m_constraintOwner = &constraintOwner;
	vin.m_collectionShapeB = shapeCollection;

	hkAgent1nMachine_UpdateShapeCollectionFilter( m_agentTrack, vin );
}

void hkBvTreeStreamAgent::invalidateTim(hkCollisionInput& input)
{
	hkAgent1nMachine_InvalidateTim(m_agentTrack, input);
}

void hkBvTreeStreamAgent::warpTime(hkTime oldTime, hkTime newTime, hkCollisionInput& input)
{
	hkAgent1nMachine_WarpTime(m_agentTrack, oldTime, newTime, input);
}


void hkBvTreeStreamAgent::calcStatistics( hkStatisticsCollector* collector ) const
{
	collector->beginObject( "BvTreeAgt3", collector->MEMORY_RUNTIME, this );

	collector->addArray( "SectorPtrs", collector->MEMORY_RUNTIME, m_agentTrack.m_sectors );
	hkAgentMachineUtil::calc1nStatistics(m_agentTrack, collector);
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
