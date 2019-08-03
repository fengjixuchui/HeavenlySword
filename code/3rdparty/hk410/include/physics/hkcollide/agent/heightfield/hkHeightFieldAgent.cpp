/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>
#include <hkmath/linear/hkVector4Util.h>
#include <hkcollide/shape/hkRayShapeCollectionFilter.h>
#include <hkcollide/shape/hkShapeRayCastInput.h>
#include <hkcollide/shape/hkRayHitCollector.h>
#include <hkcollide/shape/sphererep/hkSphereRepShape.h>
#include <hkcollide/shape/heightfield/hkHeightFieldShape.h>
#include <hkcollide/agent/heightfield/hkHeightFieldAgent.h>


hkHeightFieldAgent::hkHeightFieldAgent(const hkCdBody& A, const hkCdBody& B, const hkCollisionInput& input, hkContactMgr* mgr)
: hkCollisionAgent( mgr )
{
	// find the maximum number of collision spheres and initialize the m_contactPointId array
	if ( mgr )
	{
		const hkSphereRepShape* csShape = static_cast<const hkSphereRepShape*>(A.getShape());

		hkSphereRepShape::hkCollisionSpheresInfo info;
		csShape->getCollisionSpheresInfo( info );
		m_contactPointId.setSize( info.m_numSpheres, HK_INVALID_CONTACT_POINT );
	}
}


void HK_CALL hkHeightFieldAgent::registerAgent(hkCollisionDispatcher* dispatcher)
{
	// register symmetric version
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc          = createHeightFieldAAgent;
		af.m_getPenetrationsFunc  = hkSymmetricAgent<hkHeightFieldAgent>::staticGetPenetrations;
		af.m_getClosestPointFunc = hkSymmetricAgent<hkHeightFieldAgent>::staticGetClosestPoints;
		af.m_linearCastFunc      = hkSymmetricAgent<hkHeightFieldAgent>::staticLinearCast;
		af.m_isFlipped           = true;
		af.m_isPredictive		 = true;	// the heightfield agent is non really predictive, but we do not have an alternative
	    dispatcher->registerCollisionAgent(af, HK_SHAPE_HEIGHT_FIELD, HK_SHAPE_SPHERE_REP);	
	}
	{
		hkCollisionDispatcher::AgentFuncs af;
		af.m_createFunc          = createHeightFieldBAgent;
		af.m_getPenetrationsFunc  = staticGetPenetrations;
		af.m_getClosestPointFunc = staticGetClosestPoints;
		af.m_linearCastFunc      = staticLinearCast;
		af.m_isFlipped           = false;
		af.m_isPredictive		 = true;
	    dispatcher->registerCollisionAgent(af, HK_SHAPE_SPHERE_REP, HK_SHAPE_HEIGHT_FIELD);	
	}
}


hkCollisionAgent* hkHeightFieldAgent::createHeightFieldBAgent(const hkCdBody& bodyA, const hkCdBody& bodyB, 
															  const hkCollisionInput& input, hkContactMgr* mgr)
{
	hkHeightFieldAgent* agent = new hkHeightFieldAgent(bodyA, bodyB, input, mgr);
	return agent;
}


hkCollisionAgent* hkHeightFieldAgent::createHeightFieldAAgent(const hkCdBody& bodyA, const hkCdBody& bodyB, 
															  const hkCollisionInput& input, hkContactMgr* mgr)
{
	hkHeightFieldAgent* agent = new hkSymmetricAgent<hkHeightFieldAgent>(bodyA, bodyB, input, mgr);
	return agent;
}


void hkHeightFieldAgent::cleanup( hkCollisionConstraintOwner& constraintOwner )
{
	if ( m_contactMgr )
	{
		// Remove any unneeded contact points
		for (int i=0; i< m_contactPointId.getSize(); i++)
		{
			if (m_contactPointId[i] != HK_INVALID_CONTACT_POINT)
			{
				// Remove from contact manager
				m_contactMgr->removeContactPoint(m_contactPointId[i], constraintOwner );
			}
		}
	}
	delete this;
}

void hkHeightFieldAgent::processCollision(	const hkCdBody& csBody, const hkCdBody& hfBody, const hkProcessCollisionInput& input, hkProcessCollisionOutput& processOutput)
{
	HK_ASSERT2(0x7371164d,  m_contactMgr, HK_MISSING_CONTACT_MANAGER_ERROR_TEXT );

	HK_TIME_CODE_BLOCK( "HeightField", HK_NULL );

	const hkSphereRepShape* csShape = static_cast<const hkSphereRepShape*>(csBody.getShape());
	const hkHeightFieldShape* hfShape      = static_cast<const hkHeightFieldShape*>(hfBody.getShape());

	// Get the relative transform for the two bodies for the collision detector
	hkTransform bTa; bTa.setMulInverseMul( hfBody.getTransform(), csBody.getTransform() );

	const int numSpheres = m_contactPointId.getSize();

	hkSphere* sphereBuffer = hkAllocateStack<hkSphere>(numSpheres);

	hkContactPointId* cpId = m_contactPointId.begin();

	  //
	  //	Get the collision spheres in CollisionSphereSpace and transform them into heightfield space
	  //
	{
		const hkSphere* spheres = csShape->getCollisionSpheres( &sphereBuffer[0] );
		hkVector4Util::transformSpheres( bTa, &spheres[0].getPosition(), numSpheres, const_cast<hkVector4*>(&sphereBuffer->getPosition()) );
	}

		//
		// collide
		//
	hkHeightFieldShape::SphereCollisionOutput* out2 = hkAllocateStack<hkHeightFieldShape::SphereCollisionOutput>(numSpheres);
	{
		hkHeightFieldShape::CollideSpheresInput in2;
		in2.m_spheres = &sphereBuffer[0];
		in2.m_numSpheres = numSpheres;
		in2.m_tolerance = input.getTolerance();

		hfShape->collideSpheres( in2, &out2[0] );
	}

	//
	//	examine results
	//
	{
		hkHeightFieldShape::SphereCollisionOutput* outP = &out2[0];
		hkSphere*   sphereP = &sphereBuffer[0];

		hkReal tolerance = input.getTolerance();
		for (int i = numSpheres-1; i>=0; i--)
		{
			if ( outP[0](3) > tolerance )
			{
				if ( cpId[0] != HK_INVALID_CONTACT_POINT)
				{
					// Remove from contact manager
					m_contactMgr->removeContactPoint(cpId[0], *processOutput.m_constraintOwner );

					// Mark this point as INVALID
					cpId[0] = HK_INVALID_CONTACT_POINT;
				}
			}
			else
			{
				// Add point to mainfold
				hkProcessCdPoint& point = *processOutput.reserveContactPoints(1);

				hkVector4 position; position.setAddMul4( sphereP->getPosition(), outP[0], -sphereP->getRadius() );

				point.m_contact.getPosition()._setTransformedPos( hfBody.getTransform(), position );
				point.m_contact.getSeparatingNormal()._setRotatedDir( hfBody.getTransform().getRotation(), outP[0] );
				point.m_contact.setDistance( outP[0](3) );

							// If this point does not already exist
				if(*cpId == HK_INVALID_CONTACT_POINT)
				{
					// Add it to the contact manager
					*cpId = m_contactMgr->addContactPoint(csBody, hfBody, input, processOutput, HK_NULL, point.m_contact );
					if(*cpId == HK_INVALID_CONTACT_POINT)
					{
						processOutput.abortContactPoints(1);
					}
					else
					{
						processOutput.commitContactPoints(1);
					}
				}
				else
				{
					processOutput.commitContactPoints(1);
				}
				// Update ID
				point.m_contactPointId = *cpId;
			}
			sphereP++;
			outP++;
			cpId++;
		}
	}
	hkDeallocateStack<hkHeightFieldShape::SphereCollisionOutput>(out2);
	hkDeallocateStack<hkSphere>(sphereBuffer);
}


			// hkCollisionAgent interface implementation.
void hkHeightFieldAgent::getPenetrations(const hkCdBody& csBody, const hkCdBody& hfBody, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
{
	staticGetPenetrations( csBody, hfBody, input, collector );
}

			// hkCollisionAgent interface implementation.
void HK_CALL hkHeightFieldAgent::staticGetPenetrations(const hkCdBody& csBody, const hkCdBody& hfBody, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
{
	HK_TIMER_BEGIN_LIST( "HeightField", "GetSpheres" );

	const hkSphereRepShape* csShape = static_cast<const hkSphereRepShape*>(csBody.getShape());
	const hkHeightFieldShape* hfShape      = static_cast<const hkHeightFieldShape*>(hfBody.getShape());

	// Get the relative transform for the two bodies for the collision detector
	hkTransform bTa; bTa.setMulInverseMul( hfBody.getTransform(), csBody.getTransform() );

	hkSphereRepShape::hkCollisionSpheresInfo info;
	csShape->getCollisionSpheresInfo( info );

	const int numSpheres = info.m_numSpheres;
	hkSphere* sphereBuffer = hkAllocateStack<hkSphere>(numSpheres);


	  //
	  //	Get the collision spheres in CollisionSphereSpace and transform them into heightfield space
	  //
	{
		HK_INTERNAL_TIMER_SPLIT_LIST("getSpheres");
		const hkSphere* spheres = csShape->getCollisionSpheres( &sphereBuffer[0] );

		HK_INTERNAL_TIMER_SPLIT_LIST("transform");
		hkVector4Util::transformSpheres( bTa, &spheres[0].getPosition(), numSpheres, reinterpret_cast<hkVector4*>(&sphereBuffer[0]) );
	}

		//
		// collide
		//
	HK_TIMER_SPLIT_LIST("Collide");
	hkHeightFieldShape::SphereCollisionOutput* out2 = hkAllocateStack<hkHeightFieldShape::SphereCollisionOutput>(numSpheres);
	{
		hkHeightFieldShape::CollideSpheresInput in2;
		in2.m_spheres = &sphereBuffer[0];
		in2.m_numSpheres = numSpheres;
		in2.m_tolerance = input.getTolerance();

		hfShape->collideSpheres( in2, &out2[0] );
	}

	//
	//	examine results
	//
	HK_TIMER_SPLIT_LIST("Examine");
	{
		hkHeightFieldShape::SphereCollisionOutput* outP = &out2[0];
		for (int i = numSpheres-1; i>=0; i--)
		{
			if ( outP[0](3) < 0.0f )
			{
				collector.addCdBodyPair( csBody, hfBody );
				break;
			}
			outP++;
		}
	}
	hkDeallocateStack<hkHeightFieldShape::SphereCollisionOutput>(out2);
	hkDeallocateStack<hkSphere>(sphereBuffer);
	HK_TIMER_END_LIST();
}

			// hkCollisionAgent interface implementation.
void hkHeightFieldAgent::getClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& collector)
{
	staticGetClosestPoints( bodyA, bodyB, input, collector );
}
			
			// hkCollisionAgent interface implementation.
void HK_CALL hkHeightFieldAgent::staticGetClosestPoints( const hkCdBody& csBody, const hkCdBody& hfBody, const hkCollisionInput& input, class hkCdPointCollector& collector  )
{
	HK_TIMER_BEGIN_LIST( "HeightField", "bTA" );

	const hkSphereRepShape* csShape = static_cast<const hkSphereRepShape*>(csBody.getShape());
	const hkHeightFieldShape* hfShape      = static_cast<const hkHeightFieldShape*>(hfBody.getShape());

	// Get the relative transform for the two bodies for the collision detector
	hkTransform bTa; bTa.setMulInverseMul( hfBody.getTransform(), csBody.getTransform() );

	hkSphereRepShape::hkCollisionSpheresInfo info;
	csShape->getCollisionSpheresInfo( info );

	const int numSpheres = info.m_numSpheres;
	hkSphere* sphereBuffer = hkAllocateStack<hkSphere>(numSpheres);


	  //
	  //	Get the collision spheres in CollisionSphereSpace and transform them into heightfield space
	  //
	{
		HK_TIMER_SPLIT_LIST("getSpheres");
		const hkSphere* spheres = csShape->getCollisionSpheres( &sphereBuffer[0] );

		HK_TIMER_SPLIT_LIST("transform");
		hkVector4Util::transformSpheres( bTa, &spheres[0].getPosition(), numSpheres, reinterpret_cast<hkVector4*>(&sphereBuffer[0]) );
	}

		//
		// collide
		//
	HK_TIMER_SPLIT_LIST("collide");
	hkHeightFieldShape::SphereCollisionOutput* out2 = hkAllocateStack<hkHeightFieldShape::SphereCollisionOutput>(numSpheres);
	{
		hkHeightFieldShape::CollideSpheresInput in2;
		in2.m_spheres = &sphereBuffer[0];
		in2.m_numSpheres = numSpheres;
		in2.m_tolerance = input.getTolerance();

		hfShape->collideSpheres( in2, &out2[0] );
	}

		//
		//	examine results
		//
	HK_TIMER_SPLIT_LIST("examine");
	{
		hkHeightFieldShape::SphereCollisionOutput* outP = &out2[0];
		hkSphere*   sphereP = &sphereBuffer[0];

		hkReal tolerance = input.getTolerance();
		for (int i = numSpheres-1; i>=0; i--)
		{
			if ( outP[0](3) <= tolerance )
			{
				hkCdPoint point( csBody, hfBody );
				hkVector4 position; position.setAddMul4( sphereP->getPosition(), outP[0], -sphereP->getRadius() - outP[0](3));

				point.m_contact.getPosition()._setTransformedPos( hfBody.getTransform(), position );
				point.m_contact.getSeparatingNormal()._setRotatedDir( hfBody.getTransform().getRotation(), outP[0] );
				point.m_contact.setDistance( outP[0](3) );
				collector.addCdPoint( point );
			}
			sphereP++;
			outP++;
		}
	}
	hkDeallocateStack<hkHeightFieldShape::SphereCollisionOutput>(out2);
	hkDeallocateStack<hkSphere>(sphereBuffer);
	HK_TIMER_END_LIST();

}

class hkHeightFieldRayForwardingCollector : public hkRayHitCollector
{
	public:
		virtual void addRayHit( const hkCdBody& cdBody, const hkShapeRayCastCollectorOutput& hitInfo )
		{
			hkCdPoint point( m_csBody, cdBody );
			point.m_contact.getPosition().setAddMul4( m_currentFrom, m_path, hitInfo.m_hitFraction );

			point.m_contact.getSeparatingNormal()._setRotatedDir( cdBody.getTransform().getRotation(), hitInfo.m_normal );
			point.m_contact.getPosition().addMul4( -m_currentRadius, point.m_contact.getNormal());

			point.m_contact.setDistance( hitInfo.m_hitFraction );
			m_collector.addCdPoint( point );
			m_earlyOutHitFraction = hkMath::min2( m_collector.getEarlyOutDistance(), m_earlyOutHitFraction);
		}

		hkHeightFieldRayForwardingCollector( const hkCdBody& csBody, const hkVector4& path, hkCdPointCollector& collector )
			: m_path(path), m_csBody( csBody), m_collector( collector )
		{

		}

		hkVector4 m_currentFrom;
		hkReal    m_currentRadius;
		hkVector4 m_path;

		const hkCdBody&  m_csBody;
		hkCdPointCollector& m_collector;

};


void hkHeightFieldAgent::staticLinearCast( const hkCdBody& csBody, const hkCdBody& hfBody, const hkLinearCastCollisionInput& input, hkCdPointCollector& collector, hkCdPointCollector* startCollector )
{
	HK_TIMER_BEGIN_LIST( "HeightField", "ClosestPoints" );
	if ( startCollector )
	{
		staticGetClosestPoints( csBody, hfBody, input, *startCollector );
	}

	HK_TIMER_SPLIT_LIST("GetSpheres");

	const hkSphereRepShape* csShape			= static_cast<const hkSphereRepShape*>(csBody.getShape());
	const hkHeightFieldShape* hfShape       = static_cast<const hkHeightFieldShape*>(hfBody.getShape());

	// Get the relative transform for the two bodies for the collision detector
	hkTransform bTa; bTa.setMulInverseMul( hfBody.getTransform(), csBody.getTransform() );

	hkSphereRepShape::hkCollisionSpheresInfo info;
	csShape->getCollisionSpheresInfo( info );

	const int numSpheres = info.m_numSpheres;
	hkSphere* sphereBuffer = hkAllocateStack<hkSphere>(numSpheres);

	hkVector4 pathLocal; pathLocal._setRotatedInverseDir( hfBody.getTransform().getRotation(), input.m_path );

	  //
	  //	Get the collision spheres in CollisionSphereSpace and transform them into heightfield space
	  //
	const hkSphere* spheres = csShape->getCollisionSpheres( &sphereBuffer[0] );

		// Cast each sphere
	{
		HK_TIMER_SPLIT_LIST("CastSpheres");
		hkHeightFieldShape::hkSphereCastInput ray;
		ray.m_maxExtraPenetration = input.m_maxExtraPenetration;

		hkHeightFieldRayForwardingCollector rayCollector( csBody, input.m_path, collector );

		for (int i = 0; i < numSpheres; i++ )
		{
			ray.m_from._setTransformedPos( bTa, spheres->getPosition() );
			ray.m_radius = spheres->getRadius();
			ray.m_to.setAdd4( ray.m_from, pathLocal );

			// The adapter needs these values to work out the actual position
			rayCollector.m_currentFrom._setTransformedPos( csBody.getTransform(), spheres->getPosition());
			rayCollector.m_currentRadius = spheres->getRadius();

			hfShape->castSphere( ray, hfBody, rayCollector );
			spheres++;
		}
	}

	hkDeallocateStack<hkSphere>(sphereBuffer);
	HK_TIMER_END_LIST();
}

void hkHeightFieldAgent::linearCast( const hkCdBody& csBody, const hkCdBody& hfBody, const hkLinearCastCollisionInput& input, hkCdPointCollector& collector, hkCdPointCollector* startCollector )
{
	staticLinearCast( csBody, hfBody, input, collector, startCollector );
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
