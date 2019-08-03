/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>

#include <hkcollide/agent/hkContactMgr.h>

#include <hkdynamics/entity/hkRigidBody.h>
#include <hkdynamics/entity/util/hkEntityCallbackUtil.h>

#include <hkdynamics/collide/hkReportContactMgr.h>
#include <hkdynamics/collide/hkCollisionListener.h>
#include <hkdynamics/world/util/hkWorldCallbackUtil.h>

#include <hkinternal/dynamics/world/simulation/continuous/hkContinuousSimulation.h>



hkReportContactMgr::hkReportContactMgr( hkWorld *sm, hkRigidBody *bodyA, hkRigidBody *bodyB )
{
	m_skipNextNprocessCallbacks = hkMath::min2( bodyA->getProcessContactCallbackDelay(), bodyB->getProcessContactCallbackDelay() );
	m_world = sm;
	m_bodyA = bodyA;
	m_bodyB = bodyB;
}

hkReportContactMgr::~hkReportContactMgr()
{
}


hkContactPointId hkReportContactMgr::addContactPoint(const 	hkCdBody& a, const hkCdBody& b, const hkProcessCollisionInput& input, hkProcessCollisionOutput& output, const hkGskCache* contactCache, hkContactPoint& pcp )
{
	hkContactPointId contactPointIdOut = 0; 

	hkRigidBody* rba = m_bodyA;
	hkRigidBody* rbb = m_bodyB;

	hkReal projectedVel;
	{
		hkVector4 velA;		rba->getPointVelocity( pcp.getPosition(), velA );
		hkVector4 velB;		rbb->getPointVelocity( pcp.getPosition(), velB );

		hkVector4 deltaVel; deltaVel.setSub4( velB, velA );
		projectedVel = deltaVel.dot3( pcp.getNormal() );
	}

		//
		// fire all events
		//
	hkManifoldPointAddedEvent event( contactPointIdOut, *this, input, output, a,b, &pcp, contactCache, HK_NULL, projectedVel);
	{
		hkWorldCallbackUtil::fireContactPointAdded( m_world, event );
		hkEntityCallbackUtil::fireContactPointAdded( rba, event );
		hkEntityCallbackUtil::fireContactPointAdded( rbb, event );
	}

	if ( event.m_status == HK_CONTACT_POINT_ACCEPT )
	{
		m_skipNextNprocessCallbacks = event.m_nextProcessCallbackDelay;
		return contactPointIdOut;
	}
	return HK_INVALID_CONTACT_POINT;
}

HK_COMPILE_TIME_ASSERT( hkContactMgr::TOI_ACCEPT == hkContactMgr::ToiAccept(HK_CONTACT_POINT_ACCEPT) );
HK_COMPILE_TIME_ASSERT( hkContactMgr::TOI_REJECT == hkContactMgr::ToiAccept(HK_CONTACT_POINT_REJECT) );

hkReportContactMgr::ToiAccept hkReportContactMgr::addToi( const hkCdBody& a, const hkCdBody& b, const hkProcessCollisionInput& input, hkProcessCollisionOutput& output, hkTime toi, 
														 hkContactPoint& cp, const hkGskCache* gskCache, hkReal& projectedVelocity, hkContactPointMaterial& material )
{
	material.reset();
	hkToiPointAddedEvent event( *this, input, output, a,b, &cp, gskCache, &material, toi, projectedVelocity);
	{
		hkWorldCallbackUtil ::fireContactPointAdded( m_world, event );
		hkEntityCallbackUtil::fireContactPointAdded( m_bodyA, event );
		hkEntityCallbackUtil::fireContactPointAdded( m_bodyB, event );
	}
	projectedVelocity = event.m_projectedVelocity;

	return hkContactMgr::ToiAccept(event.m_status);
}

void hkReportContactMgr::removeToi( const hkCollidable& a, const hkCollidable& b, class hkCollisionConstraintOwner& constraintOwner, hkContactPointMaterial& material )
{
	hkRigidBody* bodyA = hkGetRigidBody(&a);
	hkRigidBody* bodyB = hkGetRigidBody(&b);
	hkWorld* world = bodyA->getWorld();

	// Fire toi-point removed
	hkContactPointRemovedEvent removedEvent( HK_INVALID_CONTACT_POINT, *this, constraintOwner, &material, bodyA, bodyB );
	hkWorldCallbackUtil::fireContactPointRemoved( world, removedEvent );
	hkEntityCallbackUtil::fireContactPointRemoved( bodyA, removedEvent );
	hkEntityCallbackUtil::fireContactPointRemoved( bodyB, removedEvent );
}

void hkReportContactMgr::processToi( struct hkToiEvent& event, hkReal rotateNormal, class hkArray<class hkEntity*>& outToBeActivated )
{
	hkContactPointConfirmedEvent confirmedEvent( hkContactPointAddedEvent::TYPE_TOI, *event.m_entities[0]->getCollidable(), *event.m_entities[1]->getCollidable(), HK_NULL, &event.m_contactPoint, &event.m_material, rotateNormal, event.m_seperatingVelocity );

	hkWorld* world = event.m_entities[0]->getWorld();
	hkWorldCallbackUtil::fireContactPointConfirmed( world, confirmedEvent );
	hkEntityCallbackUtil::fireContactPointConfirmed( event.m_entities[0], confirmedEvent );
	hkEntityCallbackUtil::fireContactPointConfirmed( event.m_entities[1], confirmedEvent );
	rotateNormal = confirmedEvent.m_rotateNormal;
}


void hkReportContactMgr::removeContactPoint( hkContactPointId cpId, hkCollisionConstraintOwner& info )
{
		//
		// fire all events
		//
	hkRigidBody* rba = m_bodyA;
	hkRigidBody* rbb = m_bodyB;

	{
		hkContactPointRemovedEvent event( cpId, *this, info, HK_NULL, rba, rbb);
		
		hkWorldCallbackUtil::fireContactPointRemoved( m_world, event );
		hkEntityCallbackUtil::fireContactPointRemoved( rba, event );
		hkEntityCallbackUtil::fireContactPointRemoved( rbb, event );
	}

}

void hkReportContactMgr::processContact(const hkCollidable& a, const hkCollidable& b, const hkProcessCollisionInput& input, hkProcessCollisionData& collisionData )
{
	//
	//	fire all events
	//
	{
		if ( m_skipNextNprocessCallbacks-- != 0) 
		{
			return;
		}

		hkRigidBody* rba = static_cast<hkRigidBody*>(a.getOwner());
		hkRigidBody* rbb = static_cast<hkRigidBody*>(b.getOwner());

		m_skipNextNprocessCallbacks = hkMath::min2( rba->getProcessContactCallbackDelay(), rbb->getProcessContactCallbackDelay() );

		//
		// fire all events using frequency information
		//
		{
			hkContactProcessEvent event( *this, a,b, collisionData );
			for ( int i = collisionData.getNumContactPoints()-1; i>=0; i-- )
			{
				event.m_contactPointProperties[i] = HK_NULL;
			}

			hkWorldCallbackUtil::fireContactProcess( m_world, event );
			hkEntityCallbackUtil::fireContactProcess( rba, event );
			hkEntityCallbackUtil::fireContactProcess( rbb, event );
		}
	}
}


hkReportContactMgr::Factory::Factory(hkWorld *mgr)
{
	m_world = mgr;
}

hkContactMgr*	hkReportContactMgr::Factory::createContactMgr( const hkCollidable& a, const hkCollidable& b, const hkCollisionInput& env )
{
	hkRigidBody* bodyA = reinterpret_cast<hkRigidBody*>(a.getOwner() );
	hkRigidBody* bodyB = reinterpret_cast<hkRigidBody*>(b.getOwner() );

	hkReportContactMgr *mgr = new hkReportContactMgr( m_world, bodyA, bodyB);
	return mgr;
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
