/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkcollide/hkCollide.h>

#include <hkmath/linear/hkSweptTransformUtil.h>

#include <hkinternal/collide/gjk/hkGsk.h>

#include <hkcollide/shape/convex/hkConvexShape.h>

#include <hkcollide/agent/hkCollisionAgentConfig.h>

#include <hkcollide/agent/gjk/hkGskBaseAgent.h>

//HK_COMPILE_TIME_ASSERT( sizeof( hkGskBaseAgent ) == 24 /* 12=base + 12=cache */ );

void hkGskBaseAgent::processCollision(	const hkCdBody& bodyA,						const hkCdBody& bodyB, 
										const hkProcessCollisionInput& input,		hkProcessCollisionOutput& result)
{
	HK_ASSERT2(0xf0040345,  0, HK_MISSING_CONTACT_MANAGER_ERROR_TEXT );
}


hkGskBaseAgent::hkGskBaseAgent(	const hkCdBody& bodyA,	const hkCdBody& bodyB, hkContactMgr* mgr ): hkIterativeLinearCastAgent( mgr )
{
	const hkConvexShape* shapeA = static_cast<const hkConvexShape*>(bodyA.getShape());
	const hkConvexShape* shapeB = static_cast<const hkConvexShape*>(bodyB.getShape());
	hkTransform t; t.setMulInverseMul( bodyA.getTransform(), bodyB.getTransform() );
	if ( shapeB->getType() == HK_SHAPE_TRIANGLE )
	{
		m_cache.initTriangle( shapeA, reinterpret_cast<const hkTriangleShape*>(shapeB), t );
	}
	else
	{
		m_cache.init( shapeA, shapeB, t );
	}
	m_separatingNormal.setZero4();
	m_separatingNormal(3) = -1.f;
	m_timeOfSeparatingNormal = hkTime(-1.0f);
	hkReal maxPenA = bodyA.getRootCollidable()->m_allowedPenetrationDepth;
	hkReal maxPenB = bodyB.getRootCollidable()->m_allowedPenetrationDepth;
	m_allowedPenetration = hkMath::min2( maxPenA, maxPenB );

}

void hkGskBaseAgent::invalidateTim( hkCollisionInput& input)
{
	m_separatingNormal.setZero4();
	m_timeOfSeparatingNormal = hkTime(-1.0f);
}

void hkGskBaseAgent::cleanup( hkCollisionConstraintOwner& constraintOwner )
{
	delete this;
}

void hkGskBaseAgent::warpTime( hkTime oldTime, hkTime newTime, hkCollisionInput& input )
{
	if (m_timeOfSeparatingNormal == oldTime)
	{
		m_timeOfSeparatingNormal = newTime;
	}
	else
	{
		m_timeOfSeparatingNormal = hkTime(-1.0f);
		m_separatingNormal.setZero4();
	}
}



#if defined HK_COMPILER_MSVC
	// C4701: local variable 'output' may be used without having been initialized
#	pragma warning(disable: 4701)
#endif

hkBool hkGskBaseAgent::_getClosestPoint(	const hkCdBody& bodyA, const hkCdBody& bodyB, 
										const hkCollisionInput& input, struct hkExtendedGskOut& output )
{
	const hkConvexShape* shapeA = static_cast<const hkConvexShape*>(bodyA.getShape());
	const hkConvexShape* shapeB = static_cast<const hkConvexShape*>(bodyB.getShape());


	{
		// Get the relative transform for the two bodies for the collision detector
		hkTransform aTb;	aTb.setMulInverseMul( bodyA.getTransform(), bodyB.getTransform());

		// Call the collision detector
		{
			hkGsk gsk;
			hkVector4 separatingNormal;
			gsk.init( shapeA, shapeB, m_cache );
			gsk.getClosestFeature(shapeA, shapeB, aTb, separatingNormal);
			gsk.checkForChangesAndUpdateCache( m_cache );
			gsk.convertFeatureToClosestDistance( separatingNormal, output );
		}

		// convert contact normal to world space...
		output.m_normalInWorld._setRotatedDir( bodyA.getTransform().getRotation(), output.m_normalInA);

		const hkReal dist = output.m_distance - shapeA->getRadius() - shapeB->getRadius();
		output.m_distance = dist;

		if( output.m_distance < input.getTolerance() )
		{
			// adjust the contact points by the radius
			output.m_pointAinA.subMul4(shapeA->getRadius(), output.m_normalInA);
			hkVector4 pointBinA; pointBinA.setAddMul4( output.m_pointAinA, output.m_normalInA, -dist );
			output.m_pointBinB._setTransformedInversePos(aTb, pointBinA);
			return true;
		}
	}
	return false;
}

void hkGskBaseAgent::calcSeparatingNormal( const hkCdBody& bodyA, const hkCdBody& bodyB, hkReal earlyOutTolerance, hkGsk& gsk, hkVector4& separatingNormalOut )
{
	const hkConvexShape* shapeA = static_cast<const hkConvexShape*>(bodyA.getShape());
	const hkConvexShape* shapeB = static_cast<const hkConvexShape*>(bodyB.getShape());

	// Get the relative transform for the two bodies for the collision detector
	hkTransform aTb;	aTb.setMulInverseMul( bodyA.getTransform(), bodyB.getTransform());

		// Call the collision detector
	hkVector4 separatingNormal; gsk.getClosestFeature(shapeA, shapeB, aTb, separatingNormal);

	separatingNormalOut._setRotatedDir( bodyA.getTransform().getRotation(), separatingNormal);
	separatingNormalOut(3) = separatingNormal(3) - shapeA->getRadius() - shapeB->getRadius();
}

HK_FORCE_INLINE hkBool hkGskBaseAgent::staticGetClosestPoint(	const hkCdBody& bodyA,	const hkCdBody& bodyB,
														   const hkTransform& aTb, const hkCollisionInput& input,
														   hkGskCache& cache, struct hkExtendedGskOut& output)

{

	HK_INTERNAL_TIMER_SPLIT_LIST( "Gsk" );

	const hkConvexShape* shapeA = static_cast<const hkConvexShape*>(bodyA.getShape());
	const hkConvexShape* shapeB = static_cast<const hkConvexShape*>(bodyB.getShape());

	{
		// Call the collision detector
		hkVector4 separatingNormal;
		hkGsk gsk;
		gsk.init( shapeA, shapeB, cache );
		const hkGskStatus gskStatus = gsk.getClosestFeature(shapeA, shapeB, aTb, separatingNormal);

		if(gskStatus == HK_GSK_OK )
		{
			gsk.convertFeatureToClosestDistance( separatingNormal, output );

			// convert contact normal to world space...
			output.m_normalInWorld.setRotatedDir( bodyA.getTransform().getRotation(), output.m_normalInA);

			const hkReal dist = output.m_distance - shapeA->getRadius() - shapeB->getRadius();
			output.m_distance = dist;

			if(output.m_distance < input.getTolerance())
			{
				// adjust the contact points by the radius
				output.m_pointAinA.addMul4(-shapeA->getRadius(), output.m_normalInA);
				hkVector4 pointBinA; pointBinA.setAddMul4( output.m_pointAinA, output.m_normalInA, -dist );
				output.m_pointBinB._setTransformedInversePos(aTb, pointBinA);
				return true;
			} 

		}
	}
	return false;
}

void hkGskBaseAgent::staticGetClosestPoints( const hkCdBody& bodyA , const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& collector  )
{
	HK_TIMER_BEGIN( "Gsk", HK_NULL );

	hkExtendedGskOut output;

	hkTransform aTb; 
	hkGskCache cache;
	{
		const hkConvexShape* sA = static_cast<const hkConvexShape*>(bodyA.getShape());
		const hkConvexShape* sB = static_cast<const hkConvexShape*>(bodyB.getShape());
		aTb.setMulInverseMul( bodyA.getTransform(), bodyB.getTransform() );
		if ( sB->getType() == HK_SHAPE_TRIANGLE )
		{
			cache.initTriangle( sA, reinterpret_cast<const hkTriangleShape*>(sB), aTb );
		}
		else
		{
			cache.init( sA, sB, aTb );
		}
	}


	if( staticGetClosestPoint(bodyA, bodyB, aTb, input, cache, output) )
	{
		hkCdPoint event( bodyA, bodyB );

		event.m_contact.getPosition()._setTransformedPos( bodyB.getTransform(), output.m_pointBinB );
		event.m_contact.setSeparatingNormal( output.m_normalInWorld, output.m_distance );

		collector.addCdPoint( event );
	}

	HK_TIMER_END();
}



void hkGskBaseAgent::getPenetrations( const hkCdBody& bodyA,	const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
{
	HK_TIMER_BEGIN_LIST( "Gsk", "tim" );

	// we cannot use tims in here as we might not have a motion state
	//	if(0)
	//	{
	//		hkVector4 timInfo;
	//		hkSweptTransformUtil::calcTimInfo( *bodyA.getMotionState(), *bodyB.getMotionState(), input.m_stepInfo.m_deltaTime, timInfo);
	//		if ( m_separatingNormal(3) > input.getTolerance() )
	//		{
	//			m_separatingNormal(3) -= timInfo.dot4xyz1( m_separatingNormal );
	//			if ( m_separatingNormal(3) > input.getTolerance() )
	//			{
	//				goto END;
	//			}
	//		}
	//	}


	HK_TIMER_SPLIT_LIST( "SepNormal");
	{
		const hkConvexShape* shapeA = static_cast<const hkConvexShape*>(bodyA.getShape());
		const hkConvexShape* shapeB = static_cast<const hkConvexShape*>(bodyB.getShape());
		

		// Get the relative transform for the two bodies for the collision detector
		hkTransform aTb; aTb.setMulInverseMul( bodyA.getTransform(), bodyB.getTransform());

		// Call the collision detector
		hkGsk gsk;
		gsk.m_doNotHandlePenetration = true;

		hkVector4 separatingNormalInA;
		gsk.init( shapeA, shapeB, m_cache );
		hkGskStatus gjkStatus = gsk.getClosestFeature(shapeA, shapeB, aTb, separatingNormalInA);
		gsk.checkForChangesAndUpdateCache( m_cache );

		if(gjkStatus == HK_GSK_OK)
		{
			const hkReal dist = separatingNormalInA(3) - shapeA->getRadius() - shapeB->getRadius();
			m_separatingNormal._setRotatedDir( bodyA.getTransform().getRotation(), separatingNormalInA );
			m_separatingNormal(3) = dist;
			if ( dist < 0.0f )
			{
				collector.addCdBodyPair( bodyA, bodyB );
			}
		}
		else
		{
			m_separatingNormal.setZero4();
			collector.addCdBodyPair( bodyA, bodyB );
		}
	}
//END:;
	HK_TIMER_END_LIST();
}

void hkGskBaseAgent::staticGetPenetrations( const hkCdBody& bodyA,	const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector  )
{

	HK_TIMER_BEGIN( "Gsk", HK_NULL );

	const hkConvexShape* shapeA = static_cast<const hkConvexShape*>(bodyA.getShape());
	const hkConvexShape* shapeB = static_cast<const hkConvexShape*>(bodyB.getShape());
	

	// Get the relative transform for the two bodies for the collision detector
	hkTransform aTb;
	aTb.setMulInverseMul( bodyA.getTransform(), bodyB.getTransform());

	hkGskCache cache;
	{
		if ( shapeB->getType() == HK_SHAPE_TRIANGLE )
		{
			cache.initTriangle( shapeA, reinterpret_cast<const hkTriangleShape*>(shapeB), aTb );
		}
		else
		{
			cache.init( shapeA, shapeB, aTb );
		}
	}

	// Call the collision detector
	hkGsk gsk;
	gsk.m_doNotHandlePenetration = true;

	gsk.init(shapeA, shapeB, cache);
	hkVector4 separatingNormal;
	const hkGskStatus gjkStatus = gsk.getClosestFeature(shapeA, shapeB, aTb, separatingNormal);

	HK_TIMER_END();

	if( gjkStatus == HK_GSK_OK )
	{
		const hkReal dist = separatingNormal(3) - shapeA->getRadius() - shapeB->getRadius();
		if ( dist < 0.0f )
		{
			collector.addCdBodyPair( bodyA, bodyB );
		}
	}
	else
	{
		collector.addCdBodyPair( bodyA, bodyB );
	}
}


hkBool hkGskBaseAgent::getClosestPoint(	const hkCdBody& bodyA, const hkCdBody& bodyB, 
										const hkCollisionInput& input, struct hkExtendedGskOut& output )
{
	return _getClosestPoint( bodyA, bodyB, input, output );
}

void hkGskBaseAgent::getClosestPoints( const hkCdBody& bodyA, const hkCdBody& bodyB,
									   const hkCollisionInput& input, hkCdPointCollector& collector  )
{
	HK_TIMER_BEGIN( "Gsk", HK_NULL );

	hkGsk::GetClosesetPointInput gskInput;
	hkTransform aTb;	aTb.setMulInverseMul( bodyA.getTransform(), bodyB.getTransform());
	{
		gskInput.m_shapeA = static_cast<const hkConvexShape*>(bodyA.getShape());
		gskInput.m_shapeB = static_cast<const hkConvexShape*>(bodyB.getShape());
		gskInput.m_aTb = &aTb;
		gskInput.m_transformA = &bodyA.getTransform();
		gskInput.m_collisionTolerance = input.getTolerance();
	}
	
	hkVector4 separatingNormal;
	hkVector4 pointOnB;
	if( hkGsk::getClosestPoint( gskInput, m_cache, separatingNormal, pointOnB ) == HK_SUCCESS )
	{
		hkCdPoint event( bodyA, bodyB );

		event.m_contact.getPosition() = pointOnB;
		event.m_contact.setSeparatingNormal( separatingNormal );
		collector.addCdPoint( event );
	}
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
