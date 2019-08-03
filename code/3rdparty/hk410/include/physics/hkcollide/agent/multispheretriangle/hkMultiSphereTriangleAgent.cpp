/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>
#include <hkcollide/shape/multisphere/hkMultiSphereShape.h>
#include <hkcollide/shape/triangle/hkTriangleShape.h>
#include <hkcollide/agent/multispheretriangle/hkMultiSphereTriangleAgent.h>
#include <hkmath/linear/hkVector4Util.h>



//#define USE_ONE_SIDED_TRIANGLES
void HK_CALL hkMultiSphereTriangleAgent::registerAgent(hkCollisionDispatcher* dispatcher)
{
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc          = createTriangleMultiSphereAgent;
		af.m_getPenetrationsFunc  = hkSymmetricAgent<hkMultiSphereTriangleAgent>::staticGetPenetrations;
		af.m_getClosestPointFunc = hkSymmetricAgent<hkMultiSphereTriangleAgent>::staticGetClosestPoints;
		af.m_linearCastFunc      = hkSymmetricAgent<hkMultiSphereTriangleAgent>::staticLinearCast;
		af.m_isFlipped           = true;
		af.m_isPredictive		 = false;
	    dispatcher->registerCollisionAgent( af, HK_SHAPE_TRIANGLE, HK_SHAPE_MULTI_SPHERE);	
	}
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc          = createMultiSphereTriangleAgent;
		af.m_getPenetrationsFunc  = staticGetPenetrations;
		af.m_getClosestPointFunc = staticGetClosestPoints;
		af.m_linearCastFunc      = staticLinearCast;
		af.m_isFlipped           = false;
		af.m_isPredictive		 = false;
		dispatcher->registerCollisionAgent(af, HK_SHAPE_MULTI_SPHERE, HK_SHAPE_TRIANGLE);	
	}    
}

hkMultiSphereTriangleAgent::hkMultiSphereTriangleAgent( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* contactMgr )
: hkIterativeLinearCastAgent( contactMgr )
{
	for (int i = 0; i < hkMultiSphereShape::MAX_SPHERES; i++)
	{
		m_contactPointId[i] = HK_INVALID_CONTACT_POINT;
	}

    const hkTriangleShape* triB = static_cast<const hkTriangleShape*>(bodyB.getShape());

	const hkVector4* vertices = triB->getVertices();
	hkCollideTriangleUtil::setupClosestPointTriangleCache( &vertices[0], m_closestPointTriangleCache );
}

hkCollisionAgent* HK_CALL hkMultiSphereTriangleAgent::createTriangleMultiSphereAgent(const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* contactMgr)
{
	return new hkSymmetricAgentLinearCast<hkMultiSphereTriangleAgent>(bodyA, bodyB, input, contactMgr);
}


hkCollisionAgent* HK_CALL hkMultiSphereTriangleAgent::createMultiSphereTriangleAgent( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkContactMgr* contactMgr)
{
    return new hkMultiSphereTriangleAgent( bodyA, bodyB, input, contactMgr );
}






void hkMultiSphereTriangleAgent::cleanup(hkCollisionConstraintOwner& constraintOwner)
{
	for(int i = 0; i < hkMultiSphereShape::MAX_SPHERES; ++i)
	{
		if(m_contactPointId[i] != HK_INVALID_CONTACT_POINT)
		{
			m_contactMgr->removeContactPoint(m_contactPointId[i], constraintOwner );
		}
	}

	delete this;
}

void hkMultiSphereTriangleAgent::processCollision( const hkCdBody& bodyA,  const hkCdBody& bodyB, const hkProcessCollisionInput& input, hkProcessCollisionOutput& result)
{
	HK_ASSERT2(0x70e70d18,  m_contactMgr, HK_MISSING_CONTACT_MANAGER_ERROR_TEXT );

	HK_TIMER_BEGIN("MultiSphereTri", HK_NULL);

    const hkMultiSphereShape* msA = static_cast<const hkMultiSphereShape*>(bodyA.getShape());
    const hkTriangleShape* triB = static_cast<const hkTriangleShape*>(bodyB.getShape());

	const hkVector4* vertices = triB->getVertices();

    hkVector4 triVertices[3];
	hkVector4Util::transformPoints( bodyB.getTransform(), vertices, 3, triVertices );

	const hkVector4* localSpheres = msA->getSpheres();
	hkVector4 worldSpheres[hkMultiSphereShape::MAX_SPHERES];

	const int nsphere = msA->getNumSpheres();

	hkVector4Util::transformPoints( bodyA.getTransform(), &localSpheres[0], nsphere, &worldSpheres[0]  );

#ifdef USE_ONE_SIDED_TRIANGLES
	hkVector4 edge0; edge0.setSub4(triVertices[1], triVertices[0]);
	hkVector4 edge1; edge1.setSub4(triVertices[2], triVertices[1]);
	hkVector4 trinormal; trinormal.setCross( edge0, edge1 );
	trinormal.mul4( m_invTriangleNormalLength );
#endif

	const hkVector4* curSphere = &worldSpheres[0];
	const hkVector4* localSphere = &localSpheres[0];


	for(int j = nsphere-1; j>=0 ; curSphere++,localSphere++,j--)
	{

#ifdef USE_ONE_SIDED_TRIANGLES
		{
			hkVector4 vec; vec.setSub4( triVertices[0], curSphere );
			hkReal dist = trinormal.dot3( vec );
			if( dist > 0 )
			{
				goto removeContactPoint;
			}
			if ( dist < -radiusSum )
			{
				goto removeContactPoint;
			}
		}
#endif
		const hkReal sphereRadius = (*localSphere)(3);
		const hkReal radiusSum = sphereRadius + triB->getRadius();
    
		{
			hkCollideTriangleUtil::ClosestPointTriangleResult cptr;
			hkCollideTriangleUtil::closestPointTriangle( *curSphere, &triVertices[0], m_closestPointTriangleCache, cptr );

			if ( cptr.distance < radiusSum + input.getTolerance() )
			{
				hkProcessCdPoint& point = *result.reserveContactPoints(1);

				point.m_contact.getPosition().setAddMul4(  *curSphere, cptr.hitDirection, triB->getRadius() - cptr.distance );
				point.m_contact.setSeparatingNormal( cptr.hitDirection );
				point.m_contact.setDistance( cptr.distance - radiusSum );

				if(m_contactPointId[j] == HK_INVALID_CONTACT_POINT)
				{
					m_contactPointId[j] = m_contactMgr->addContactPoint(bodyA, bodyB, input, result, HK_NULL, point.m_contact );
				}

				if ( m_contactPointId[j] != HK_INVALID_CONTACT_POINT )
				{
					result.commitContactPoints(1);
					point.m_contactPointId = m_contactPointId[j];
				}
				else
				{
					result.abortContactPoints(1);
				}

				continue;
			}
		}
		{
#ifdef USE_ONE_SIDED_TRIANGLES
	removeContactPoint:
#endif
			if(m_contactPointId[j] != HK_INVALID_CONTACT_POINT)
			{
				m_contactMgr->removeContactPoint(m_contactPointId[j], *result.m_constraintOwner );
				m_contactPointId[j] = HK_INVALID_CONTACT_POINT;
			}
		}
	}

	HK_TIMER_END();
}

// hkCollisionAgent interface implementation.
void hkMultiSphereTriangleAgent::getClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& collector)
{
	HK_TIMER_BEGIN("MultiSphereTriangle", HK_NULL);

    const hkMultiSphereShape* msA = static_cast<const hkMultiSphereShape*>(bodyA.getShape());
    const hkTriangleShape* triB = static_cast<const hkTriangleShape*>(bodyB.getShape());

	const hkVector4* vertices = triB->getVertices();

    hkVector4 triVertices[3];
	hkVector4Util::transformPoints( bodyB.getTransform(), vertices, 3, triVertices );

	const hkVector4* localSpheres = msA->getSpheres();
	hkVector4 worldSpheres[hkMultiSphereShape::MAX_SPHERES];
	const int nsphere = msA->getNumSpheres();

	hkVector4Util::transformPoints( bodyA.getTransform(), &localSpheres[0], nsphere, &worldSpheres[0]  );

	//HK_ASSERT(0x6294297a, localSpheres.getSize() == m_contactPointId.getSize() );

	hkCdPoint event( bodyA, bodyB );

	for(int i = 0; i < nsphere; ++i)
	{
		const hkVector4& curSphere = worldSpheres[i];

		const hkReal sphereRadius = localSpheres[i](3);
		const hkReal radiusSum = sphereRadius + triB->getRadius();

		{
			hkCollideTriangleUtil::ClosestPointTriangleResult cptr;
			hkCollideTriangleUtil::closestPointTriangle( curSphere, &triVertices[0], m_closestPointTriangleCache, cptr );

			if ( cptr.distance < radiusSum + input.getTolerance() )
			{

				event.m_contact.getPosition().setAddMul4(  curSphere, cptr.hitDirection, -cptr.distance + triB->getRadius());
				event.m_contact.setSeparatingNormal( cptr.hitDirection, cptr.distance - radiusSum );

				collector.addCdPoint( event );
			}
		}
	}

	HK_TIMER_END();
}

// hkCollisionAgent interface implementation.
void hkMultiSphereTriangleAgent::staticGetClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& collector)
{
	HK_TIMER_BEGIN("MultiSphereTriangle", HK_NULL);

    const hkMultiSphereShape* msA = static_cast<const hkMultiSphereShape*>(bodyA.getShape());
    const hkTriangleShape* triB = static_cast<const hkTriangleShape*>(bodyB.getShape());

	const hkVector4* vertices = triB->getVertices();

    hkVector4 triVertices[3];
	hkVector4Util::transformPoints( bodyB.getTransform(), vertices, 3, triVertices );

	const hkVector4* localSpheres = msA->getSpheres();
	hkVector4 worldSpheres[hkMultiSphereShape::MAX_SPHERES];
	const int nsphere = msA->getNumSpheres();

	hkVector4Util::transformPoints( bodyA.getTransform(), &localSpheres[0], nsphere, &worldSpheres[0]  );

	hkCdPoint event( bodyA, bodyB );

	hkCollideTriangleUtil::ClosestPointTriangleCache closestPointTriangleCache;
	{
		const hkVector4* verticesB = triB->getVertices();
		hkCollideTriangleUtil::setupClosestPointTriangleCache( &verticesB[0], closestPointTriangleCache );
	}

	for(int i = 0; i < nsphere; ++i)
	{
		const hkVector4& curSphere = worldSpheres[i];

		const hkReal sphereRadius = localSpheres[i](3);
		const hkReal radiusSum = sphereRadius + triB->getRadius();

		{
			hkCollideTriangleUtil::ClosestPointTriangleResult cptr;
			hkCollideTriangleUtil::closestPointTriangle( curSphere, &triVertices[0], closestPointTriangleCache, cptr );

			if ( cptr.distance < radiusSum + input.getTolerance() )
			{

				event.m_contact.getPosition().setAddMul4(  curSphere, cptr.hitDirection, triB->getRadius() -cptr.distance );
				event.m_contact.setSeparatingNormal(  cptr.hitDirection, cptr.distance - radiusSum );
				collector.addCdPoint( event );
			}
		}
	}

	HK_TIMER_END();
}
	

void hkMultiSphereTriangleAgent::getPenetrations(const  hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
{

    const hkMultiSphereShape* msA = static_cast<const hkMultiSphereShape*>(bodyA.getShape());
    const hkTriangleShape* triB = static_cast<const hkTriangleShape*>(bodyB.getShape());

	const hkVector4* vertices = triB->getVertices();

    hkVector4 triVertices[3];
	hkVector4Util::transformPoints( bodyB.getTransform(), vertices, 3, triVertices );

	const hkVector4* localSpheres = msA->getSpheres();
	hkVector4 worldSpheres[hkMultiSphereShape::MAX_SPHERES];
	const int nsphere = msA->getNumSpheres();

	hkVector4Util::transformPoints( bodyA.getTransform(), &localSpheres[0], nsphere, &worldSpheres[0]  );

	hkCdPoint event( bodyA, bodyB );

	for(int i = 0; i < nsphere; ++i)
	{
		const hkVector4& curSphere = worldSpheres[i];

		const hkReal sphereRadius = localSpheres[i](3);
		const hkReal radiusSum = sphereRadius + triB->getRadius();

		{
			hkCollideTriangleUtil::ClosestPointTriangleResult cptr;
			hkCollideTriangleUtil::closestPointTriangle( curSphere, &triVertices[0], m_closestPointTriangleCache, cptr );

			if ( cptr.distance < radiusSum )
			{
				collector.addCdBodyPair( bodyA, bodyB );
				break;
			}
		}
	}
}

void hkMultiSphereTriangleAgent::staticGetPenetrations(const  hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
{

    const hkMultiSphereShape* msA = static_cast<const hkMultiSphereShape*>(bodyA.getShape());
    const hkTriangleShape* triB = static_cast<const hkTriangleShape*>(bodyB.getShape());

	const hkVector4* vertices = triB->getVertices();

    hkVector4 triVertices[3];
	hkVector4Util::transformPoints( bodyB.getTransform(), vertices, 3, triVertices );

	const hkVector4* localSpheres = msA->getSpheres();
	hkVector4 worldSpheres[hkMultiSphereShape::MAX_SPHERES];
	const int nsphere = msA->getNumSpheres();

	hkVector4Util::transformPoints( bodyA.getTransform(), &localSpheres[0], nsphere, &worldSpheres[0]  );

	hkCollideTriangleUtil::ClosestPointTriangleCache closestPointTriangleCache;
	{
		const hkVector4* verticesB = triB->getVertices();
		hkCollideTriangleUtil::setupClosestPointTriangleCache( &verticesB[0], closestPointTriangleCache );
	}

	hkCdPoint event( bodyA, bodyB );

	for(int i = 0; i < nsphere; ++i)
	{
		const hkVector4& curSphere = worldSpheres[i];

		const hkReal sphereRadius = localSpheres[i](3);
		const hkReal radiusSum = sphereRadius + triB->getRadius();

		{
			hkCollideTriangleUtil::ClosestPointTriangleResult cptr;
			hkCollideTriangleUtil::closestPointTriangle( curSphere, &triVertices[0], closestPointTriangleCache, cptr );

			if ( cptr.distance < radiusSum )
			{
				collector.addCdBodyPair( bodyA, bodyB );
				break;
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
