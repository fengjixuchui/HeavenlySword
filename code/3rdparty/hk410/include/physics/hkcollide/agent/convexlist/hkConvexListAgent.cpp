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

#include <hkinternal/collide/gjk/hkGsk.h>

#include <hkcollide/shape/convex/hkConvexShape.h>
#include <hkcollide/shape/convexlist/hkConvexListShape.h>

#include <hkcollide/agent/shapecollection/hkShapeCollectionAgent.h>
#include <hkcollide/collector/pointcollector/hkClosestCdPointCollector.h>
#include <hkinternal/collide/agent3/machine/1n/hkAgent1nMachine.h>

#include <hkcollide/agent/hkCollisionAgentConfig.h>
#include <hkcollide/agent/hkCollisionQualityInfo.h>

#include <hkcollide/agent/convexlist/hkConvexListAgent.h>
#include <hkcollide/agent/shapecollection/hkShapeCollectionAgent.h>
#include <hkcollide/agent/list/hkListAgent.h>

#include <hkinternal/collide/gjk/gskmanifold/hkGskManifoldUtil.h>

#include <hkinternal/collide/agent3/predgskagent3/hkPredGskAgent3.h>
#include <hkinternal/collide/gjk/agent/hkGskAgentUtil.h>
#include <hkcollide/collector/bodypaircollector/hkFlagCdBodyPairCollector.h>
#include <hkcollide/collector/pointcollector/hkSimpleClosestContactCollector.h>
#include <hkcollide/agent/shapecollection/hkShapeCollectionAgent.h>

#include <hkcollide/filter/hkConvexListFilter.h>

#include <hkvisualize/hkDebugDisplay.h>

extern hkReal hkConvexShapeDefaultRadius;

//HK_COMPILE_TIME_ASSERT( sizeof( hkConvexListAgent ) == 12/*base*/ + 20/*tim*/ + 16/*cache*/ + 64/*manifold*/ );
//HK_COMPILE_TIME_ASSERT( sizeof( hkConvexListAgent::StreamData) < sizeof(hkGskManifold  ) );

hkConvexListAgent::hkConvexListAgent( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* mgr )
: hkPredGskfAgent( bodyA, bodyB, input, mgr )
{

	m_dispatcher = input.m_dispatcher;
	m_inGskMode = true;
	m_processFunctionCalled = false;

	const hkConvexShape* shapeA = static_cast<const hkConvexShape*>(bodyA.getShape());
	const hkConvexShape* shapeB = static_cast<const hkConvexShape*>(bodyB.getShape());
	hkTransform t; t.setMulInverseMul( bodyA.getTransform(), bodyB.getTransform() );
	m_cache.init( shapeA, shapeB, t );


	m_separatingNormal(3) = -1.f;
	m_timeOfSeparatingNormal = hkTime(-1.0f);
}



hkCollisionAgent* HK_CALL hkConvexListAgent::createConvexConvexListAgent(const 	hkCdBody& bodyA, const hkCdBody& bodyB, 
															 const  hkCollisionInput& input, hkContactMgr* mgr)
{
	hkCollisionAgent* agent;
	if ( mgr )
	{
		hkConvexListFilter::ConvexListCollisionType collisionType = input.m_convexListFilter->getConvexListCollisionType( bodyB, bodyA, input );
		switch( collisionType )
		{
		case hkConvexListFilter::TREAT_CONVEX_LIST_AS_NORMAL:
			{
				agent = new hkConvexListAgent( bodyA, bodyB, input, mgr );
				break;
			}
		case hkConvexListFilter::TREAT_CONVEX_LIST_AS_CONVEX:
			{
				agent = new hkPredGskfAgent( bodyA, bodyB, input, mgr );
				break;
			}
		case hkConvexListFilter::TREAT_CONVEX_LIST_AS_LIST:
			{
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
		agent = new hkShapeCollectionAgent(bodyA, bodyB, input, mgr);
	}
	return agent;
}

hkCollisionAgent* HK_CALL hkConvexListAgent::createConvexListConvexAgent( const hkCdBody& bodyA, const hkCdBody& bodyB, 
																	const  hkCollisionInput& input, hkContactMgr* mgr )
{
	hkCollisionAgent* agent;
	if ( mgr )
	{
		hkConvexListFilter::ConvexListCollisionType collisionType = input.m_convexListFilter->getConvexListCollisionType( bodyA, bodyB, input );
		switch( collisionType )
		{
		case hkConvexListFilter::TREAT_CONVEX_LIST_AS_NORMAL:
			{
				agent = new hkSymmetricAgent<hkConvexListAgent>(bodyA, bodyB, input, mgr);
				break;
			}
		case hkConvexListFilter::TREAT_CONVEX_LIST_AS_CONVEX:
			{
				agent = new hkPredGskfAgent( bodyA, bodyB, input, mgr );
				break;
			}
		case hkConvexListFilter::TREAT_CONVEX_LIST_AS_LIST:
			{
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
		agent = new hkSymmetricAgent<hkShapeCollectionAgent>(bodyA, bodyB, input, mgr);
	}
	return agent;
}


// Special dispatch function for convex list vs convex list
hkCollisionAgent* HK_CALL hkConvexListAgent::createConvexListConvexListAgent( const hkCdBody& bodyA, const hkCdBody& bodyB, 
																	const  hkCollisionInput& input, hkContactMgr* mgr )
{

	if ( mgr )
	{
		hkConvexListFilter::ConvexListCollisionType collisionTypeA = input.m_convexListFilter->getConvexListCollisionType( bodyA, bodyB, input );
		switch( collisionTypeA )
		{
		case hkConvexListFilter::TREAT_CONVEX_LIST_AS_NORMAL:
			{
				return new hkSymmetricAgent<hkConvexListAgent>(bodyA, bodyB, input, mgr);
			}
		case hkConvexListFilter::TREAT_CONVEX_LIST_AS_LIST:
			{
				return new hkShapeCollectionAgent(bodyA, bodyB, input, mgr); // DONE
			}
		case hkConvexListFilter::TREAT_CONVEX_LIST_AS_CONVEX:
			{
				hkConvexListFilter::ConvexListCollisionType collisionTypeB = input.m_convexListFilter->getConvexListCollisionType( bodyB, bodyA, input );

				switch( collisionTypeB )
				{
				case hkConvexListFilter::TREAT_CONVEX_LIST_AS_NORMAL:
					{
						return new hkConvexListAgent( bodyA, bodyB, input, mgr );
					}
				case hkConvexListFilter::TREAT_CONVEX_LIST_AS_CONVEX:
					{
						return new hkPredGskfAgent( bodyA, bodyB, input, mgr );
					}
				case hkConvexListFilter::TREAT_CONVEX_LIST_AS_LIST:
					{
						return new hkSymmetricAgent<hkShapeCollectionAgent>(bodyA, bodyB, input, mgr); // DONE
					}
				default:
					{
						HK_ASSERT2(0xeaf09646, 0, "Unknown ConvexListCollisionType returned");
					}
				}
			}
		default:
			{
				HK_ASSERT2(0xeaf09646, 0, "Unknown ConvexListCollisionType returned");
			}
		}
	}
	else
	{
		return new hkShapeCollectionAgent(bodyA, bodyB, input, mgr);
	}

	return HK_NULL;
}

void HK_CALL hkConvexListAgent::registerAgent(hkCollisionDispatcher* dispatcher)
{
	// register symmetric version
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc           = createConvexListConvexAgent;
		af.m_getPenetrationsFunc  = hkSymmetricAgent<hkConvexListAgent>::staticGetPenetrations;
		af.m_getClosestPointFunc  = hkSymmetricAgent<hkConvexListAgent>::staticGetClosestPoints;
		af.m_linearCastFunc       = hkSymmetricAgent<hkConvexListAgent>::staticLinearCast;
		af.m_isFlipped            = true;
		af.m_isPredictive		  = true;
	    dispatcher->registerCollisionAgent(af, HK_SHAPE_CONVEX_LIST, HK_SHAPE_CONVEX);
	}
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc           = createConvexConvexListAgent;
		af.m_getPenetrationsFunc  = hkConvexListAgent::staticGetPenetrations;
		af.m_getClosestPointFunc  = hkConvexListAgent::staticGetClosestPoints;
		af.m_linearCastFunc       = hkConvexListAgent::staticLinearCast;
		af.m_isFlipped            = false;
		af.m_isPredictive		  = true;
		dispatcher->registerCollisionAgent(af, HK_SHAPE_CONVEX, HK_SHAPE_CONVEX_LIST);
	}
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc           = createConvexListConvexListAgent;
		af.m_getPenetrationsFunc  = hkConvexListAgent::staticGetPenetrations;
		af.m_getClosestPointFunc  = hkConvexListAgent::staticGetClosestPoints;
		af.m_linearCastFunc       = hkConvexListAgent::staticLinearCast;
		af.m_isFlipped            = false;
		af.m_isPredictive		  = true;
		dispatcher->registerCollisionAgent(af, HK_SHAPE_CONVEX_LIST, HK_SHAPE_CONVEX_LIST );
	}
}

void hkConvexListAgent::invalidateTim( hkCollisionInput& input)
{
	if ( m_inGskMode )
	{
		hkPredGskfAgent::invalidateTim(input);
	}
	else
	{
		hkAgent1nMachine_InvalidateTim(getStream().m_agentTrack, input);
	}
}

void hkConvexListAgent::warpTime( hkTime oldTime, hkTime newTime, hkCollisionInput& input )
{
	if ( m_inGskMode )
	{
		hkPredGskfAgent::warpTime( oldTime, newTime, input );
	}
	else
	{
		hkAgent1nMachine_WarpTime(getStream().m_agentTrack, oldTime, newTime, input);
	}
}
void hkConvexListAgent::removePoint( hkContactPointId idToRemove )
{
	if ( m_inGskMode )
	{
		hkGskfAgent::removePoint( idToRemove );
	}
}

void hkConvexListAgent::commitPotential( hkContactPointId idToCommit )
{
	if ( m_inGskMode )
	{
		hkGskfAgent::commitPotential( idToCommit );
	}
}

void hkConvexListAgent::createZombie( hkContactPointId idTobecomeZombie )
{
	if ( m_inGskMode )
	{
		hkGskfAgent::createZombie( idTobecomeZombie );
	}
}


// hkCollisionAgent interface implementation.
void hkConvexListAgent::staticGetClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, class hkCdPointCollector& collector )
{
	HK_TIMER_BEGIN_LIST( "CvxList", "checkHull" );
	{
		hkFlagCdBodyPairCollector checker;
		hkGskBaseAgent::staticGetPenetrations( bodyA, bodyB, input, checker );
		if ( checker.hasHit() )
		{
			HK_TIMER_SPLIT_LIST("children");
			hkSymmetricAgent<hkShapeCollectionAgent>::staticGetClosestPoints( bodyA, bodyB, input, collector );
		}
		else
		{
			hkClosestCdPointCollector closestPoint;
			hkGskBaseAgent::staticGetClosestPoints( bodyA, bodyB, input, closestPoint );

			// if we have a hit, we need to check whether we are closer than our m_minDistanceToUseConvexHullForGetClosestPoints
			if ( closestPoint.hasHit() )
			{
				const hkConvexListShape* convexList = reinterpret_cast<const hkConvexListShape*>( bodyB.getShape() );
				if ( closestPoint.getHitContact().getDistance() > convexList->m_minDistanceToUseConvexHullForGetClosestPoints )
				{
					hkCdPoint hit( bodyA, bodyB );
					hit.m_contact = closestPoint.getHitContact();
					collector.addCdPoint( hit );
				}
				else
				{
					HK_TIMER_SPLIT_LIST("children");
					hkSymmetricAgent<hkShapeCollectionAgent>::staticGetClosestPoints( bodyA, bodyB, input, collector );
				}
			}
		}
	}
	HK_TIMER_END_LIST();
}

void hkConvexListAgent::getClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& collector ) 
{
	hkConvexListAgent::staticGetClosestPoints( bodyA, bodyB, input, collector );
}

void hkConvexListAgent::staticGetPenetrations( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
{
	HK_TIMER_BEGIN_LIST( "CvxList", "checkHull" );
	{
		hkFlagCdBodyPairCollector checker;
		hkGskBaseAgent::staticGetPenetrations( bodyA, bodyB, input, checker );
		if ( checker.hasHit() )
		{
			HK_TIMER_SPLIT_LIST("children");
			hkSymmetricAgent<hkShapeCollectionAgent>::staticGetPenetrations( bodyA, bodyB, input, collector );
		}
	}
	HK_TIMER_END_LIST();
}

void hkConvexListAgent::getPenetrations( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
{
	hkConvexListAgent::staticGetPenetrations( bodyA, bodyB, input, collector);
}


void hkConvexListAgent::staticLinearCast( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkCdPointCollector& collector, hkCdPointCollector* startCollector )
{
	HK_TIMER_BEGIN_LIST( "CvsListAgent", "checkHull" );
	{
		hkSimpleClosestContactCollector checker;
		hkGskBaseAgent::staticLinearCast( bodyA, bodyB, input, checker, &checker );
		if ( checker.hasHit() )
		{
			HK_TIMER_SPLIT_LIST("child");
			hkSymmetricAgent<hkShapeCollectionAgent>::staticLinearCast( bodyA, bodyB, input, collector, startCollector );
		}
	}
	HK_TIMER_END_LIST();
}

void hkConvexListAgent::linearCast( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkCdPointCollector& collector, hkCdPointCollector* startCollector )
{
	hkConvexListAgent::staticLinearCast( bodyA, bodyB, input, collector, startCollector );
}


void hkConvexListAgent::updateShapeCollectionFilter( const hkCdBody& bodyA, const hkCdBody& convexListShapeBodyB, const hkCollisionInput& input, hkCollisionConstraintOwner& constraintOwner )
{
	if ( !m_inGskMode )
	{
		hkAgent1nMachine_VisitorInput vin;
		vin.m_bodyA = &bodyA;
		vin.m_bvTreeBodyB = &convexListShapeBodyB;
		vin.m_input = &input;
		vin.m_contactMgr = m_contactMgr;
		vin.m_constraintOwner = &constraintOwner;
		vin.m_collectionShapeB = static_cast<const hkShapeCollection*>(convexListShapeBodyB.getShape());

		hkAgent1nMachine_UpdateShapeCollectionFilter( getStream().m_agentTrack, vin );
	}
}

void hkConvexListAgent::switchToStreamMode(hkCollisionConstraintOwner& constraintOwner)
{
	hkGskManifold_cleanup( m_manifold, m_contactMgr, constraintOwner );
	m_inGskMode = false;
	new ( &getStream().m_agentTrack ) hkAgent1nTrack();
	hkAgent1nMachine_Create( getStream().m_agentTrack );

	m_inStreamModeCounter = 25;
	getStream().m_inStreamModeTimDist = 0.0f;
}


void hkConvexListAgent::switchToGskMode(hkCollisionConstraintOwner& constraintOwner)
{
	hkAgent1nMachine_Destroy( getStream().m_agentTrack, m_dispatcher, m_contactMgr, constraintOwner );
	m_manifold.init();
	m_inGskMode = true;
}



void hkConvexListAgent::cleanup( hkCollisionConstraintOwner& constraintOwner )
{
	if ( m_inGskMode )
	{
		hkGskManifold_cleanup( m_manifold, m_contactMgr, constraintOwner );
	}
	else
	{
		hkAgent1nMachine_Destroy( getStream().m_agentTrack, m_dispatcher, m_contactMgr, constraintOwner );
	}
	delete this;
}


#if defined HK_COMPILER_MSVC
	// C4701: local variable 'output' may be used without having been initialized
#	pragma warning(disable: 4701)
#endif


struct hkProcessCollisionOutputBackup
{
	hkProcessCollisionOutputBackup( const hkProcessCollisionOutput& output )
	{
		m_firstPoint = output.m_firstFreeContactPoint;

		m_toi = output.m_toi;

		if ( output.m_potentialContacts )
		{
			m_weldingInformation = *output.m_potentialContacts;
		}
	}

	inline void rollbackOutput( const hkCdBody& bodyA,	const hkCdBody& bodyB, hkProcessCollisionOutput& output, hkContactMgr* mgr )
	{
		if ( output.m_toi.m_time != m_toi.m_time )
		{
			mgr->removeToi( *bodyA.getRootCollidable(), *bodyB.getRootCollidable(), *output.m_constraintOwner, output.m_toi.m_material );
			output.m_toi = m_toi;
		}

		output.m_firstFreeContactPoint = m_firstPoint;

		if ( output.m_potentialContacts )
		{
			*output.m_potentialContacts = m_weldingInformation;
		}
	}

	hkProcessCdPoint* m_firstPoint;
	hkProcessCollisionOutput::PotentialInfo m_weldingInformation;
	hkProcessCollisionOutput::ToiInfo m_toi;
};


class hkMapPointsToSubShapeContactMgr : public hkContactMgr
{
	public:

		hkMapPointsToSubShapeContactMgr( hkContactMgr* mgr )
		{
			m_contactMgr = mgr;
			m_invalidPointHit = false;
		}

		virtual hkContactPointId addContactPoint( const hkCdBody& a, const hkCdBody& b, const hkProcessCollisionInput& input, hkProcessCollisionOutput& output, const hkGskCache* contactCache, hkContactPoint& cp )
		{
			const hkCdBody* aMod = &a;
			const hkCdBody* bMod = &b;
			hkCdBody aTemp;
			hkCdBody bTemp;
			if (a.getShape()->getType() == HK_SHAPE_CONVEX_LIST )
			{
				int shapeKey = ( 0xff00 & contactCache->m_vertices[0] ) >> 8;

				for (int i = 1; i < contactCache->m_dimA; ++i)
				{
					int shapeKey2 = ( 0xff00 & contactCache->m_vertices[i] ) >> 8;
					if ( shapeKey2 != shapeKey )
					{
						m_invalidPointHit = true;
						return HK_INVALID_CONTACT_POINT;
					}
				}
				hkShapeCollection::ShapeBuffer buffer;
				const hkShape* childShape = static_cast<const hkConvexListShape*>(a.getShape())->getChildShape(shapeKey, buffer);
				new (&aTemp) hkCdBody(&a);
				aTemp.setShape( childShape, shapeKey );
				aMod = &aTemp;
			}

			if (b.getShape()->getType() == HK_SHAPE_CONVEX_LIST )
			{
				int shapeKey = (0xff00 & contactCache->m_vertices[contactCache->m_dimA]) >> 8;

				for (int i = contactCache->m_dimA + 1; i < contactCache->m_dimA + contactCache->m_dimB; ++i)
				{
					int shapeKey2 = (0xff00 & contactCache->m_vertices[i]) >> 8;
					if ( shapeKey2 != shapeKey )
					{
						m_invalidPointHit = true;
						return HK_INVALID_CONTACT_POINT;
					}
				}
				hkShapeCollection::ShapeBuffer buffer;
				const hkShape* childShape = static_cast<const hkConvexListShape*>(b.getShape())->getChildShape(shapeKey, buffer);
				new (&bTemp) hkCdBody(&b);
				bTemp.setShape( childShape, shapeKey );
				bMod = &bTemp;
			}
			return m_contactMgr->addContactPoint( *aMod, *bMod, input, output, contactCache, cp );
		}

		virtual hkResult reserveContactPoints( int numPoints )
		{
			return m_contactMgr->reserveContactPoints( numPoints );
		}

		virtual void removeContactPoint( hkContactPointId cpId, hkCollisionConstraintOwner& constraintOwner )
		{
			m_contactMgr->removeContactPoint( cpId, constraintOwner );
		}

		void processContact( const hkCollidable& a, const hkCollidable& b, const hkProcessCollisionInput& input, hkProcessCollisionData& collisionData )
		{
			m_contactMgr->processContact( a, b, input, collisionData );
		}

		virtual void cleanup()
		{
			m_contactMgr->cleanup();
		}

		virtual ToiAccept addToi( const hkCdBody& a, const hkCdBody& b, const hkProcessCollisionInput& input, hkProcessCollisionOutput& output, hkTime toi, hkContactPoint& cp, const hkGskCache* gskCache, hkReal& projectedVelocity, hkContactPointMaterial& materialOut )
		{
			const hkCdBody* aMod = &a;
			const hkCdBody* bMod = &b;
			hkCdBody aTemp;
			hkCdBody bTemp;
			if (a.getShape()->getType() == HK_SHAPE_CONVEX_LIST )
			{
				int shapeKey = ( 0xff00 & gskCache->m_vertices[0] ) >> 8;

				for (int i = 1; i < gskCache->m_dimA; ++i)
				{
					int shapeKey2 = ( 0xff00 & gskCache->m_vertices[i] ) >> 8;
					if ( shapeKey2 != shapeKey )
					{
						m_invalidPointHit = true;
						return TOI_REJECT;
					}
				}
				hkShapeCollection::ShapeBuffer buffer;
				const hkShape* childShape = static_cast<const hkConvexListShape*>(a.getShape())->getChildShape(shapeKey, buffer);
				new (&aTemp) hkCdBody(&a);
				aTemp.setShape( childShape, shapeKey );
				aMod = &aTemp;
			}

			if (b.getShape()->getType() == HK_SHAPE_CONVEX_LIST )
			{
				int shapeKey = ( 0xff00 & gskCache->m_vertices[gskCache->m_dimA] ) >> 8;

				for (int i = gskCache->m_dimA + 1; i < gskCache->m_dimA + gskCache->m_dimB; ++i)
				{
					int shapeKey2 = ( 0xff00 & gskCache->m_vertices[i] ) >> 8;
					if ( shapeKey2 != shapeKey )
					{
						m_invalidPointHit = true;
						return TOI_REJECT;
					}
				}
				hkShapeCollection::ShapeBuffer buffer;
				const hkShape* childShape = static_cast<const hkConvexListShape*>(b.getShape())->getChildShape(shapeKey, buffer);
				new (&bTemp) hkCdBody(&b);
				bTemp.setShape( childShape, shapeKey );
				bMod = &bTemp;
			}
			return m_contactMgr->addToi( *aMod, *bMod, input, output, toi, cp, gskCache, projectedVelocity, materialOut );
		}

		virtual void removeToi( const hkCollidable& a, const hkCollidable& b, class hkCollisionConstraintOwner& constraintOwner, hkContactPointMaterial& material )
		{
			m_contactMgr->removeToi( a, b, constraintOwner, material );
		}

		virtual void processToi( struct hkToiEvent& event, hkReal rotateNormal, class hkArray<class hkEntity*>& outToBeActivated )
		{
			m_contactMgr->processToi( event, rotateNormal, outToBeActivated );
		}

	public:

		hkContactMgr* m_contactMgr;
		hkBool m_invalidPointHit;

};

void hkConvexListAgent::processCollision(const hkCdBody& bodyA,						const hkCdBody& bodyB, 
										 const hkProcessCollisionInput& input,		hkProcessCollisionOutput& output)
{
 	HK_ASSERT2(0x57213df1,  m_contactMgr, HK_MISSING_CONTACT_MANAGER_ERROR_TEXT );

	HK_TIMER_BEGIN_LIST( "CvxLst", "Tim" );

	//
	//	Get the relative linear movement (xyz) and the worst case angular movment (w)
	//
	const hkConvexListShape* cls = reinterpret_cast<const hkConvexListShape*>( bodyB.getShape() );
	hkVector4 timInfo;
	hkSweptTransformUtil::calcTimInfo( *bodyA.getMotionState(), *bodyB.getMotionState(), input.m_stepInfo.m_deltaTime, timInfo);

	// some values to undo the output
	hkProcessCollisionOutputBackup outputBackup( output );

	if ( m_inGskMode )
	{
gskMode:
		if ( m_separatingNormal(3) > input.getTolerance() )
		{
			m_separatingNormal(3) -= timInfo.dot4xyz1( m_separatingNormal );
			if ( m_separatingNormal(3) > input.getTolerance() )
			{
				if ( m_manifold.m_numContactPoints)
				{
			  		hkGskManifold_cleanup( m_manifold, m_contactMgr, *output.m_constraintOwner );
				}
				goto END;
			}
		}
 		HK_TIMER_SPLIT_LIST( "Gsk" );


		// Wrap the contact manager in a version that will convert the points on the hull of the
		// convex list to points on the sub shapes
		hkMapPointsToSubShapeContactMgr mappingMgr( m_contactMgr );
		m_contactMgr = &mappingMgr;

		hkPredGskfAgent::processCollision( bodyA, bodyB, input, output );

		m_contactMgr = mappingMgr.m_contactMgr;


		if (mappingMgr.m_invalidPointHit)
		{
			// assert no added tois or contact points - this is not always the case currently - see below
			//HK_ASSERT()
switchToStreamModeLabel:

			// XXX - This line is necessary because when addContactPoint is called from line 167 in gskAgentUtil, the INVALID return
			// seems to be ignored and the point added anyway causing an assert in the process contact - some artifact of the welding code I think.
			outputBackup.rollbackOutput( bodyA, bodyB, output, m_contactMgr );
			switchToStreamMode( *output.m_constraintOwner );
			goto streamMode;
		}

		//
		// If we get a penetration (which is supported by 1 piece) normally we want to use the outer hull to push it out.
		// However if we start in the penetrating case, we want to use penetrations with the inner pieces
		//
		if ( m_manifold.m_numContactPoints)
		{
			if (!m_processFunctionCalled)
			{
				hkReal allowedPenetration = 2.0f * hkMath::min2(bodyA.getRootCollidable()->getAllowedPenetrationDepth(), bodyB.getRootCollidable()->getAllowedPenetrationDepth());
				if ( m_separatingNormal(3) < -allowedPenetration )
				{
					goto switchToStreamModeLabel;
				}
			}
		}
	}
	else
	{
streamMode:
		HK_TIMER_SPLIT_LIST( "Stream" );
		if ( m_inStreamModeCounter-- < 0)
		{
			m_inStreamModeCounter = 25;
			//if ( getStream().m_inStreamModeTimDist < 0.0f)
			{
				hkGsk::GetClosesetPointInput gskInput;
				hkTransform aTb;	aTb.setMulInverseMul( bodyA.getTransform(), bodyB.getTransform());
				{
					gskInput.m_shapeA = static_cast<const hkConvexShape*>(bodyA.getShape());
					gskInput.m_shapeB = static_cast<const hkConvexShape*>(bodyB.getShape());
					gskInput.m_aTb = &aTb;
					gskInput.m_transformA = &bodyA.getTransform();
					gskInput.m_collisionTolerance = input.getTolerance();
				}
				
				hkVector4 pointOnB;
				if( hkGsk::getClosestPoint( gskInput, m_cache, m_separatingNormal, pointOnB ) != HK_SUCCESS )
				{
					switchToGskMode( *output.m_constraintOwner );
					outputBackup.rollbackOutput( bodyA, bodyB, output, m_contactMgr );
					goto gskMode;
				}
				getStream().m_inStreamModeTimDist = -m_separatingNormal(3);
			}
		}
		getStream().m_inStreamModeTimDist -= timInfo.length3();
		//
		//	Set the input structure
		//
		hkAgent3ProcessInput in3;
		{
			in3.m_bodyA = &bodyA;
			in3.m_bodyB = &bodyB;
			in3.m_contactMgr = m_contactMgr;
			in3.m_input = &input;
			in3.m_linearTimInfo = timInfo;

			const hkMotionState* msA = bodyA.getMotionState();
			const hkMotionState* msB = bodyB.getMotionState();

			in3.m_aTb.setMulInverseMul(msA->getTransform(), msB->getTransform());
		}

		int size = cls->m_childShapes.getSize();
		hkLocalBuffer<hkShapeKey> hitList( size+1 );
		for ( int i = 0; i < size; i++ ){		hitList[i] = static_cast<hkUint32>(i);	}
		hitList[size] = HK_INVALID_SHAPE_KEY;

		hkAgent1nMachine_Process( getStream().m_agentTrack, in3, cls, hitList.begin(), size, output );
	}

END:;
	m_processFunctionCalled = true;
	HK_TIMER_END_LIST();
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
