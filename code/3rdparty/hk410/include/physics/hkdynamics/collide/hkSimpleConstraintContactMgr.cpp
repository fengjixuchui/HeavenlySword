/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>

#include <hkbase/debugutil/hkStatisticsCollector.h>
#include <hkbase/thread/hkCriticalSection.h>

#include <hkcollide/agent/hkContactMgr.h>
#include <hkcollide/agent/hkProcessCdPoint.h>
#include <hkcollide/agent/hkProcessCollisionInput.h>
#include <hkcollide/agent/hkProcessCollisionOutput.h>
#include <hkcollide/dispatch/hkCollisionDispatcher.h>
#include <hkconstraintsolver/simpleConstraints/hkSimpleConstraintUtil.h>

#include <hkdynamics/entity/hkRigidBody.h>
#include <hkdynamics/entity/util/hkEntityCallbackUtil.h>

#include <hkdynamics/constraint/contact/hkSimpleContactConstraintData.h>
#include <hkdynamics/constraint/response/hkSimpleCollisionResponse.h>

#include <hkdynamics/collide/hkCollisionListener.h>
#include <hkdynamics/collide/hkSimpleConstraintContactMgr.h>

#include <hkdynamics/world/util/hkWorldCallbackUtil.h>
#include <hkdynamics/world/util/hkWorldOperationUtil.h>
#include <hkdynamics/world/hkWorld.h>
#include <hkdynamics/world/util/hkWorldConstraintUtil.h>

#include <hkinternal/dynamics/world/simulation/continuous/hkContinuousSimulation.h>


HK_COMPILE_TIME_ASSERT( sizeof(hkContactPoint) == 32 );
//HK_COMPILE_TIME_ASSERT( HK_OFFSET_OF( hkSimpleConstraintContactMgr, m_skinNextNprocessCallbacks ) == 12 );
HK_COMPILE_TIME_ASSERT( sizeof(hkSimpleConstraintContactMgr) <= 512 );

hkContactPointId hkContactPointConfirmedEvent::getContactPointId() const
{
	if ( isToi() )
	{
		return HK_INVALID_CONTACT_POINT;
	}

	const hkSimpleContactConstraintData* cc = this->m_contactData;
	int indexOfPoint   = int(this->m_contactPoint - cc->m_atom->getContactPoints());
	int contactPointId = cc->m_idMgrA.indexOf( indexOfPoint );
	return hkContactPointId(contactPointId);
}

const class hkDynamicsContactMgr* hkContactPointConfirmedEvent::getContactMgr() const
{
	if ( isToi() )
	{
		return HK_NULL;
	}

	int offset = HK_OFFSET_OF( hkSimpleConstraintContactMgr, m_contactConstraintData );
	const void* cmgr = hkAddByteOffsetConst( this->m_contactData, -offset );
	
	const hkSimpleConstraintContactMgr* contactMgr = reinterpret_cast<const hkSimpleConstraintContactMgr*>(cmgr);
	return contactMgr;
}


hkConstraintInstance* hkSimpleConstraintContactMgr::getConstraintInstance() 
{ 
	return &m_constraint; 
} 

hkSimpleConstraintContactMgr::hkSimpleConstraintContactMgr( hkWorld *sm, hkRigidBody *bodyA, hkRigidBody *bodyB )
	: m_contactConstraintData( &m_constraint ), //m_constraint(bodyA, bodyB, &m_contactConstraintData, hkConstraintInstance::PRIORITY_PSI, hkConstraintInstance::DO_NOT_ADD_REFERENCES__FOR_INTERNAL_USE_ONLY) // we don't want to add references to rigidBodies and constraintDatas
												m_constraint(hkConstraintInstance::PRIORITY_PSI)
{
	// Init constraint instance; don't add references to data and entities -- similarly those references have to be set to
	// HK_NULL before the constraintInstance destructor is called (to avoid removeReference() being called automatically.)
	m_constraint.m_data = &m_contactConstraintData;
	m_constraint.m_entities[0] = bodyA;
	m_constraint.m_entities[1] = bodyB;
	HK_ASSERT2(0xf0fe4356, bodyA, "EntityA not set.");
	
	// 

	m_world = sm;
	m_skipNextNprocessCallbacks = 0;
	m_reservedContactPoints = 0;

//  memsize 0 is a special case for objects that can't have their dtor called.
	m_contactConstraintData.m_memSizeAndFlags = 0;	// don't count this constraint in our calcStatistics
	m_constraint.m_memSizeAndFlags = 0;

	hkCollidableQualityType a = bodyA->getCollidable()->getQualityType();
	hkCollidableQualityType b = bodyB->getCollidable()->getQualityType();

	hkCollisionDispatcher* dispatcher = sm->getCollisionDispatcher();
	hkCollisionDispatcher::CollisionQualityIndex index = dispatcher->getCollisionQualityIndex( a, b );
	hkCollisionQualityInfo* qualityInfo = dispatcher->getCollisionQualityInfo( index );
	int constraintPriority = qualityInfo->m_constraintPriority;

	m_constraint.setPriority( hkConstraintInstance::ConstraintPriority(constraintPriority) );
}

hkSimpleConstraintContactMgr::~hkSimpleConstraintContactMgr()
{
	int size = m_contactConstraintData.m_atom->m_numContactPoints;
	if ( size )
	{
		HK_ASSERT2(0x739cc779,  0, "A contact mgr is deleted before all the contact points are removed.\n" \
					"Did you try to change the collision filter from a callback? You need to\n" \
					"do that outside the callback.");
		hkWorldOperationUtil::removeConstraintImmediately( m_world, &m_constraint );
	}

	// Simply set the entities and constraintData pointers to zero so that the ~hkConstraintInstance does not remove references.
	// (references are not thread safe )
	m_constraint.m_entities[0] = HK_NULL;
	m_constraint.m_entities[1] = HK_NULL;
	m_constraint.m_data = HK_NULL;
}


hkResult hkSimpleConstraintContactMgr::reserveContactPoints( int numPoints )
{
	int size = m_contactConstraintData.m_atom->m_numContactPoints;
	if ( size + numPoints + m_reservedContactPoints > HK_MAX_CONTACT_POINT-2 )
	{
		HK_WARN_ONCE( 0xf0e31234, "Maximum number of ContactPoints reached, dropping points Note: This message is fired only once and it means that one object has too many triangles in a specific area" );
		return HK_FAILURE;
	}
	m_reservedContactPoints = hkUchar(m_reservedContactPoints + numPoints);
	return HK_SUCCESS;
}

hkContactMgr::ToiAccept hkSimpleConstraintContactMgr::addToi( const hkCdBody& a, const hkCdBody& b, const hkProcessCollisionInput& input, hkProcessCollisionOutput& output, hkTime toi, hkContactPoint& cp, const hkGskCache* gskCache, hkReal& projectedVelocity, hkContactPointMaterial& material )
{
	hkEntity* ea = static_cast<hkEntity*>(a.getRootCollidable()->getOwner());
	hkEntity* eb = static_cast<hkEntity*>(b.getRootCollidable()->getOwner());

	const hkMaterial& materialA = ea->getMaterial();
	const hkMaterial& materialB = eb->getMaterial();

	material.setFriction(    hkMaterial::getCombinedFriction(    materialA.getFriction(), materialB.getFriction() ) ); 
	material.setRestitution( hkMaterial::getCombinedRestitution( materialA.getRestitution(), materialB.getRestitution() ) );

	hkToiPointAddedEvent event( *this, input, output, a,b, &cp, gskCache, &material, toi, projectedVelocity);
	{
		hkWorldCallbackUtil::fireContactPointAdded ( m_world, event );
		hkEntityCallbackUtil::fireContactPointAdded( ea, event );

		hkEntityCallbackUtil::fireContactPointAdded( eb, event );
	}
	projectedVelocity = event.m_projectedVelocity;
	return hkContactMgr::ToiAccept( event.m_status );
}

void hkSimpleConstraintContactMgr::removeToi( const hkCollidable& a, const hkCollidable& b, class hkCollisionConstraintOwner& constraintOwner, hkContactPointMaterial& material )
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


void hkSimpleConstraintContactMgr::processToi( struct hkToiEvent& event, hkReal rotateNormal, class hkArray<class hkEntity*>& outToBeActivated )
{
	hkWorld* world = event.m_entities[0]->getWorld();
	{
		hkContactPointConfirmedEvent confirmedEvent( hkContactPointAddedEvent::TYPE_TOI, *event.m_entities[0]->getCollidable(), *event.m_entities[1]->getCollidable(), HK_NULL, &event.m_contactPoint, &event.m_material, rotateNormal, event.m_seperatingVelocity );

		hkWorldCallbackUtil::fireContactPointConfirmed( world, confirmedEvent );
		hkEntityCallbackUtil::fireContactPointConfirmed( event.m_entities[0], confirmedEvent );
		hkEntityCallbackUtil::fireContactPointConfirmed( event.m_entities[1], confirmedEvent );
		rotateNormal = confirmedEvent.m_rotateNormal;
	}

	// Do simple response and mark bodies active/not active;

	hkRigidBody* body0 = static_cast<hkRigidBody*>(event.m_entities[0]);
	hkRigidBody* body1 = static_cast<hkRigidBody*>(event.m_entities[1]);

	if (!body0->isFixedOrKeyframed() || !body1->isFixedOrKeyframed())
	{
		    // Process the TOI-event contact using hkSimpleCollisionResponse
		    // ( doSimpleCollisionResponse does not require the bodies to backstepped now )
		hkLs_doSimpleCollisionResponse( world, event, rotateNormal, outToBeActivated );
	}
}

hkContactPointId hkSimpleConstraintContactMgr::addContactPoint(const 	hkCdBody& a, const hkCdBody& b, const hkProcessCollisionInput& input, hkProcessCollisionOutput& output, const hkGskCache* contactCache, hkContactPoint& cp)
{
	int size = m_contactConstraintData.m_atom->m_numContactPoints;

	if ( size + m_reservedContactPoints > HK_MAX_CONTACT_POINT-2 )
	{
		HK_WARN_ONCE(0x11fce585, "Maximum number of ContactPoints reached, dropping points. Note: This message is fired only once and it means that one object has too many triangles in a specific area");
		return HK_INVALID_CONTACT_POINT;
	}

	hkRigidBody* rba = static_cast<hkRigidBody*>(a.getRootCollidable()->getOwner());
	hkRigidBody* rbb = static_cast<hkRigidBody*>(b.getRootCollidable()->getOwner());

	if ( size == 0 )
	{
		output.m_constraintOwner->addConstraintToCriticalLockedIsland( &m_constraint );
	}

	hkContactPointId id;
	hkContactPointProperties* cpi;
	{
		hkContactPoint* dcp;
		id = m_contactConstraintData.allocateContactPoint( *output.m_constraintOwner, &dcp, &cpi );
		*dcp = cp;
	}

	// set the current penetration depth
	// get the current velocity

	hkReal projectedVel;
	{
		hkVector4 velA;		rba->getPointVelocity( cp.getPosition(), velA );
		hkVector4 velB;		rbb->getPointVelocity( cp.getPosition(), velB );

		hkVector4 deltaVel; deltaVel.setSub4( velA, velB );
		projectedVel = deltaVel.dot3( cp.getNormal() );
	}

	const hkMaterial& materialA = rba->getMaterial();
	const hkMaterial& materialB = rbb->getMaterial();

	cpi->setFriction(    hkMaterial::getCombinedFriction(    materialA.getFriction(), materialB.getFriction() ) ); 
	cpi->setRestitution( hkMaterial::getCombinedRestitution( materialA.getRestitution(), materialB.getRestitution() ) );
	cpi->m_impulseApplied = 0.0f;

		//
		// fire all events
		//
	{
		hkManifoldPointAddedEvent event( id, *this, input, output, a,b, &cp, contactCache, cpi, projectedVel);
		hkWorldCallbackUtil::fireContactPointAdded( m_world, event );
		hkEntityCallbackUtil::fireContactPointAdded( rba, event );
		hkEntityCallbackUtil::fireContactPointAdded( rbb, event );

		if ( event.m_status == HK_CONTACT_POINT_REJECT )
		{
			// Note: This will fire the removal event, so all listeners will be correctly informed of the state change.
			removeContactPoint( id, *output.m_constraintOwner );
			return HK_INVALID_CONTACT_POINT;
		}
		m_skipNextNprocessCallbacks = event.m_nextProcessCallbackDelay;
	}

	// check whether we need callbacks
	{
		if ( cpi->getRestitution() || m_world->m_collisionListeners.getSize() || rba->getCollisionListeners().getSize() || rbb->getCollisionListeners().getSize() )
		{
			// flag master for firing contact callbacks
			output.m_constraintOwner->addCallbackRequest( m_contactConstraintData.m_constraint, hkConstraintAtom::CALLBACK_REQUEST_CONTACT_POINT );
		}
		else
		{
			hkReal sumInvMass = rba->getMassInv() + rbb->getMassInv();
			hkReal mass = 1.0f / (sumInvMass + 1e-10f );
			cpi->m_impulseApplied = -0.2f * mass * projectedVel;

			cpi->m_internalSolverData = 0.0f;
			cpi->m_internalDataA      = cp.getDistance();
		}
	}

	return id;
}


void hkSimpleConstraintContactMgr::removeContactPoint( hkContactPointId cpId, hkCollisionConstraintOwner& constraintOwner )
{
		//
		// fire all events
		//
	hkEntity* entityA = m_constraint.getEntityA();
	hkEntity* entityB = m_constraint.getEntityB();
	{
		hkContactPointMaterial* mat = &m_contactConstraintData.getContactPointProperties( cpId );
		hkContactPointRemovedEvent event( cpId, *this,  constraintOwner, mat, entityA, entityB );
		
		hkWorldCallbackUtil::fireContactPointRemoved( m_world, event );
		hkEntityCallbackUtil::fireContactPointRemoved( entityA, event );
		hkEntityCallbackUtil::fireContactPointRemoved( entityB, event );
	}

	m_contactConstraintData.freeContactPoint( constraintOwner, cpId );

	int numRemainingContacts = m_contactConstraintData.m_atom->m_numContactPoints;

	if( numRemainingContacts == 0 )
	{
		constraintOwner.removeConstraintFromCriticalLockedIsland(&m_constraint);
	}
}


// hkDynamicsContactMgr interface implementation
hkContactPoint* hkSimpleConstraintContactMgr::getContactPoint( hkContactPointId id )
{
	hkContactPoint* dcp = &m_contactConstraintData.getContactPoint( id );
	return dcp;
}

hkContactPointProperties* hkSimpleConstraintContactMgr::getContactPointProperties( hkContactPointId id )
{
	return &m_contactConstraintData.getContactPointProperties( id );
}


void hkSimpleConstraintContactMgr::getAllContactPointIds( hkArray<hkContactPointId>& contactPointIds ) const
{
	m_contactConstraintData.m_idMgrA.getAllUsedIds(contactPointIds);
}



void hkSimpleConstraintContactMgr::toiCollisionResponseBeginCallback( const hkContactPoint& cp, struct hkSimpleConstraintInfoInitInput& inA, struct hkBodyVelocity& velA, hkSimpleConstraintInfoInitInput& inB, hkBodyVelocity& velB)
{
	m_contactConstraintData.collisionResponseBeginCallback( cp, inA, velA, inB, velB );
}



void hkSimpleConstraintContactMgr::toiCollisionResponseEndCallback( const hkContactPoint& cp, hkReal impulseApplied, struct hkSimpleConstraintInfoInitInput& inA, struct hkBodyVelocity& velA, hkSimpleConstraintInfoInitInput& inB, hkBodyVelocity& velB)
{
	m_contactConstraintData.collisionResponseEndCallback( cp, impulseApplied, inA, velA, inB, velB );
}


void hkSimpleConstraintContactMgr::processContact(	const hkCollidable& a,	const hkCollidable& b,
													const hkProcessCollisionInput& input,
													hkProcessCollisionData& collisionData )
{
	HK_ON_DEBUG(int size = collisionData.getNumContactPoints());

	HK_ASSERT2(0x25d95ef3, size < HK_MAX_CONTACT_POINT, "Too many contact points in a single collision pair. The system only handles 255 contact points or less between two objects.\
This is probably the result of creating bad collision geometries (i.e. meshes with many triangles in the same place) or having a too large collision tolerance. \
It can also result from not creating a hkBvTreeShape about your mesh shape.");

#ifdef HK_DEBUG
	// Check uniqueness of contact points
	if(1)
	{
		HK_ASSERT2(0x38847077,  collisionData.getNumContactPoints() <= 255, "Too many contact points between colliding entities (over 255) " );

		for (int i = 1; i < size; i++ )
		{
			for (int j = 0; j < i; j++ )
			{
				HK_ASSERT2(0x31ac481b,  collisionData.m_contactPoints[i].m_contactPointId !=  collisionData.m_contactPoints[j].m_contactPointId, "Agent reported a contact point twice" );
			}
		}

	}
#endif

	HK_ASSERT2(0x76cf302f,  size == m_contactConstraintData.m_atom->m_numContactPoints, "hkProcessCollisionOutput has a different number of contact points than the hkContactMgr" );

	//
	//	fire all events
	//
	{
		if ( m_skipNextNprocessCallbacks-- == 0) 
		{
			hkRigidBody* rba = static_cast<hkRigidBody*>(a.getOwner());
			hkRigidBody* rbb = static_cast<hkRigidBody*>(b.getOwner());

			m_skipNextNprocessCallbacks = hkMath::min2( rba->getProcessContactCallbackDelay(), rbb->getProcessContactCallbackDelay() );

			//
			// fire all events. Do not fire events every frame but use m_skipNextNprocessCallbacks to reduce the frequency
			//
			{
				hkContactProcessEvent event( *this, a,b, collisionData );

				
				hkProcessCdPoint* ccpEnd = collisionData.getEnd();
				hkContactPointProperties** cppp = &event.m_contactPointProperties[0];
				for( hkProcessCdPoint* ccp = collisionData.getFirstContactPoint(); ccp < ccpEnd; ccp++)
				{
					*(cppp++) = &m_contactConstraintData.getContactPointProperties( ccp->m_contactPointId );
				}
				
				hkWorldCallbackUtil::fireContactProcess( m_world, event );
				hkEntityCallbackUtil::fireContactProcess( rba, event );
				hkEntityCallbackUtil::fireContactProcess( rbb, event );
			}
		}
	}

	{
		hkProcessCdPoint* ccpEnd = collisionData.getEnd();
		for( hkProcessCdPoint* ccp = collisionData.getFirstContactPoint(); ccp < ccpEnd; ccp++)
		{
			hkContactPoint& cp = m_contactConstraintData.getContactPoint( ccp->m_contactPointId );
			ccp->m_contact.getPosition().storeUncached( &cp.getPosition() );
			ccp->m_contact.getSeparatingNormal().storeUncached( & cp.getSeparatingNormal() );
		}
	}
}



hkSimpleConstraintContactMgr::Factory::Factory(hkWorld *mgr)
{
	m_world = mgr;
}

hkContactMgr*	hkSimpleConstraintContactMgr::Factory::createContactMgr(const  hkCollidable& a,const  hkCollidable& b, const hkCollisionInput& input )
{
	hkRigidBody* bodyA = reinterpret_cast<hkRigidBody*>(a.getOwner() );
	hkRigidBody* bodyB = reinterpret_cast<hkRigidBody*>(b.getOwner() );

	hkSimpleConstraintContactMgr *mgr = new hkSimpleConstraintContactMgr( m_world, bodyA, bodyB );
	return mgr;
}



void hkSimpleConstraintContactMgr::calcStatistics( hkStatisticsCollector* collector ) const
{
	collector->beginObject(HK_NULL, collector->MEMORY_RUNTIME, this);

	collector->addChunk("ContactAtom", collector->MEMORY_RUNTIME, m_contactConstraintData.m_atom, m_contactConstraintData.m_atom->m_sizeOfAllAtoms);
	collector->addArray( "ContactIds", collector->MEMORY_RUNTIME, m_contactConstraintData.m_idMgrA.m_values );

	collector->endObject();
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
