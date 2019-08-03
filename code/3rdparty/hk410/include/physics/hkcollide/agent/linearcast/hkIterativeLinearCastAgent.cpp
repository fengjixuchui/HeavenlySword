/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>

#include <hkcollide/agent/linearcast/hkIterativeLinearCastAgent.h>
#include <hkcollide/collector/pointcollector/hkSimpleClosestContactCollector.h>
#include <hkcollide/agent/hkLinearCastCollisionInput.h>
#include <hkcollide/agent/hkCollisionAgentConfig.h>

void hkIterativeLinearCastAgent::linearCast( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkCdPointCollector& collector, hkCdPointCollector* startCollector )
{
	hkSimpleClosestContactCollector startPointCollector;

	hkCollisionInput in2 = input;
	in2.m_tolerance += input.m_cachedPathLength;

	getClosestPoints( bodyA, bodyB, in2, startPointCollector );

	if ( !startPointCollector.hasHit() )
	{
		return;
	}


	hkCdPoint event( bodyA, bodyB );

	const hkContactPoint& startPoint = startPointCollector.getHitContact();
	{
		event.m_contact.getPosition() =  startPoint.getPosition();
		event.m_contact.getSeparatingNormal() = startPoint.getSeparatingNormal();

		hkReal dist = startPoint.getDistance();
		if (dist < input.m_tolerance )
		{
			if ( startCollector )
			{
				startCollector->addCdPoint( event );
			}
		}
	}	

	const hkVector4& path = input.m_path;

	{
		const hkReal startDistance = startPoint.getDistance();
		const hkReal pathProjected = startPoint.getNormal().dot3( path );
		const hkReal endDistance   = startDistance + pathProjected;

		//
		//	Check whether we could move the full distance
		//
		{
			if ( endDistance > 0.0f )
			{
				// not hitting at all
				return;
			}

			HK_ASSERT2(0x35e26c9c,  input.m_maxExtraPenetration >= 0.f, "You have to set the  m_maxExtraPenetration to something bigger than 0");
			if ( pathProjected + input.m_maxExtraPenetration >= 0.f )
			{
				// we are not moving closer than m_maxExtraPenetration
				return;
			}
		}

		//
		// check for early outs
		//
		if ( startDistance <= input.m_config->m_iterativeLinearCastEarlyOutDistance )
		{
			if ( startDistance > 0.f )
			{
				const hkReal castFraction = startDistance / ( startDistance - endDistance);
				if ( castFraction > collector.getEarlyOutDistance() )
				{
					return;
				}
				event.m_contact.setDistance( castFraction );
			}
			else
			{
				// we are hitting imediately
				event.m_contact.setDistance( 0.0f );
			}

			// early out, because we are already very close
			collector.addCdPoint( event );
			return;
		}

		// now endDistance is negative and starDistance position, so this division is allowed
		event.m_contact.setDistance(  startDistance / ( startDistance - endDistance) );
	}


	//
	// now find precise collision point
	//
	{
		hkSimpleClosestContactCollector checkPointCollector;
		hkMotionState ms = *bodyA.getMotionState();
		hkCdBody bodyACopy( &bodyA, &ms);
		bodyACopy.setShape( bodyA.getShape() );

		for ( int i = input.m_config->m_iterativeLinearCastMaxIterations-1; i>=0; i-- )
		{

			//
			//	Move bodyA along the path and recheck the collision 
			//
			{
				checkPointCollector.reset();
				//
				// Move the object along the path
				//
				const hkVector4& oldPosition  =  bodyA.getMotionState()->getTransform().getTranslation();
				hkVector4 newPosition;	newPosition.setAddMul4( oldPosition, path, event.m_contact.getDistance() );
				ms.getTransform().setTranslation( newPosition );

				getClosestPoints( bodyACopy, bodyB , in2, checkPointCollector);

				
				HK_ASSERT2(0x3ea49f9b,  checkPointCollector.hasHit(), "The collision agent reports no hit when queried a second time (the second time is closer than the first time");
			}

			//
			// redo the checks
			//
			{
				const hkContactPoint& checkPoint = checkPointCollector.getHitContact();
				const hkVector4& normal = checkPoint.getNormal();

				hkReal pathProjected2 = normal.dot3( path );
				if ( pathProjected2 >= 0 )
				{
					return;	// normal points away
				}
				pathProjected2 = - pathProjected2;


				const hkReal startDistance2 = checkPoint.getDistance();

				//
				//	pre distance is the negative already traveled distance relative to the new normal
				//
				const hkReal preDistance = pathProjected2 * event.m_contact.getDistance();
				HK_ASSERT2(0x730fe223,  preDistance >= 0.0f, "Numerical accuracy problem in linearCast" );

				if ( startDistance2 + preDistance > pathProjected2 )
				{
					// endDistance + preDistance = realEndDistance;
					// if realEndDistance > 0, than endplane is not penetrated, so no hit
					return;
				}

				// now we know that pathProjected2 < 0
				// so the division is safe
				const hkReal castFraction = event.m_contact.getDistance() + (startDistance2 / pathProjected2);
				if ( castFraction > collector.getEarlyOutDistance() )
				{
					return;
				}

				event.m_contact.setPosition( checkPoint.getPosition() );
				event.m_contact.setSeparatingNormal( checkPoint.getNormal(), castFraction );
				
				if ( startDistance2 <= input.m_config->m_iterativeLinearCastEarlyOutDistance )
				{
					// early out, because we are already very close
					break;
				}
			}
		}
	}
	collector.addCdPoint( event );
}


void hkIterativeLinearCastAgent::staticLinearCast( const hkCdBody& bodyA, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkCdPointCollector& collector, hkCdPointCollector* startCollector )
{
	hkSimpleClosestContactCollector startPointCollector;

	hkCollisionInput in2 = input;
	in2.m_tolerance += input.m_cachedPathLength;


	hkShapeType typeA = bodyA.getShape()->getType();
	hkShapeType typeB = bodyB.getShape()->getType();
	hkCollisionDispatcher::GetClosestPointsFunc getClosestPoints = input.m_dispatcher->getGetClosestPointsFunc( typeA, typeB );

	getClosestPoints( bodyA, bodyB, in2, startPointCollector );

	if ( !startPointCollector.hasHit() )
	{
		return;
	}


	hkCdPoint event( bodyA, bodyB );

	const hkContactPoint& startPoint = startPointCollector.getHitContact();

	{
		event.m_contact.getSeparatingNormal() = startPoint.getSeparatingNormal();
		event.m_contact.getPosition() = startPoint.getPosition();
		const hkReal dist = startPoint.getDistance();

		if (dist < input.m_tolerance )
		{
			if ( startCollector )
			{
				startCollector->addCdPoint( event );
			}
		}
	}	

	const hkVector4& path = input.m_path;

	{
		const hkReal startDistance = startPoint.getDistance();
		const hkReal pathProjected = startPoint.getNormal().dot3( path );
		const hkReal endDistance   = startDistance + pathProjected;

		//
		//	Check whether we could move the full distance
		//
		{
			if ( endDistance > 0.0f )
			{
				// not hitting at all
				return;
			}

			HK_ASSERT2(0x74a9aa26,  input.m_maxExtraPenetration >= 0.f, "You have to set the  m_maxExtraPenetration to something bigger than 0");
			if ( pathProjected + input.m_maxExtraPenetration >= 0.f )
			{
				// we are not moving closer than m_maxExtraPenetration
				return;
			}
		}

		//
		// check for early outs
		//
		if ( startDistance <= input.m_config->m_iterativeLinearCastEarlyOutDistance )
		{
			if ( startDistance > 0.f )
			{
				const hkReal castFraction = startDistance / ( startDistance - endDistance);
				if ( castFraction > collector.getEarlyOutDistance() )
				{
					return;
				}
				event.m_contact.setDistance( castFraction );
			}
			else
			{
				// we are hitting imediately
				event.m_contact.setDistance( 0.0f );
			}

			// early out, because we are already very close
			collector.addCdPoint( event );
			return;
		}

		// now endDistance is negative and starDistance position, so this division is allowed
		event.m_contact.setDistance(  startDistance / ( startDistance - endDistance) );
	}


	//
	// now find precise collision point
	//
	{
		hkSimpleClosestContactCollector checkPointCollector;
		hkMotionState ms = *bodyA.getMotionState();
		hkCdBody bodyACopy( &bodyA, &ms);
		bodyACopy.setShape( bodyA.getShape() );

		for ( int i = input.m_config->m_iterativeLinearCastMaxIterations-1; i>=0; i-- )
		{

			//
			//	Move bodyA along the path and recheck the collision 
			//
			{
				checkPointCollector.reset();
				//
				// Move the object along the path
				//
				const hkVector4& oldPosition  =  bodyA.getMotionState()->getTransform().getTranslation();
				hkVector4 newPosition;	newPosition.setAddMul4( oldPosition, path, event.m_contact.getDistance() );
				ms.getTransform().setTranslation( newPosition );

				getClosestPoints( bodyACopy, bodyB , in2, checkPointCollector);

				if ( !checkPointCollector.hasHit() )
				{
					return;
				}
			}

			//
			// redo the checks
			//
			{
				const hkContactPoint& checkPoint = checkPointCollector.getHitContact();
				const hkVector4& normal = checkPoint.getNormal();

				hkReal pathProjected2 = normal.dot3( path );
				if ( pathProjected2 >= 0 )
				{
					return;	// normal points away
				}
				pathProjected2 = - pathProjected2;


				const hkReal startDistance2 = checkPoint.getDistance();

				//
				//	pre distance is the negative already traveled distance relative to the new normal
				//
				const hkReal preDistance = pathProjected2 * event.m_contact.getDistance();
				HK_ASSERT2(0x573be33d,  preDistance >= 0.0f, "Numerical accuracy problem in linearCast" );

				if ( startDistance2 + preDistance > pathProjected2 )
				{
					// endDistance + preDistance = realEndDistance;
					// if realEndDistance > 0, than endplane is not penetrated, so no hit
					return;
				}

				// now we know that pathProjected2 < 0
				// so the division is safe
				const hkReal castFraction = event.m_contact.getDistance() + (startDistance2 / pathProjected2);
				if ( castFraction > collector.getEarlyOutDistance() )
				{
					return;
				}

				event.m_contact.setPosition( checkPoint.getPosition() );
				event.m_contact.setSeparatingNormal( checkPoint.getNormal(), castFraction );

				if ( startDistance2 <= input.m_config->m_iterativeLinearCastEarlyOutDistance )
				{
					// early out, because we are already very close
					break;
				}
			}
		}
	}
	collector.addCdPoint( event );
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
