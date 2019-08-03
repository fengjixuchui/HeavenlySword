/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_SIMPLE_CONSTRAINT_CONTACT_MGR_H
#define HK_DYNAMICS2_SIMPLE_CONSTRAINT_CONTACT_MGR_H

#include <hkcollide/dispatch/contactmgr/hkContactMgrFactory.h>
#include <hkdynamics/collide/hkDynamicsContactMgr.h>
#include <hkdynamics/constraint/contact/hkSimpleContactConstraintData.h>

class hkWorld;

	/// This class acts as a bridge between hkCollide and hkDynamics constraint system.
	/// It collects all contact point information from the collision detector through the hkContactMgr interface
	/// and copies this data to an internal hkContactConstraint
class hkSimpleConstraintContactMgr: public hkDynamicsContactMgr
{
	public:

		hkSimpleConstraintContactMgr( hkWorld* world, hkRigidBody *bodyA, hkRigidBody *bodyB );
		~hkSimpleConstraintContactMgr();

			/// hkDynamicsContactMgr interface implementation.
		virtual hkContactPoint* getContactPoint( hkContactPointId id );

			/// hkDynamicsContactMgr interface implementation.
		virtual hkContactPointProperties* getContactPointProperties( hkContactPointId id );


			/// hkContactMgr interface implementation.
		virtual hkContactPointId addContactPoint(const 	hkCdBody& a, const hkCdBody& b, const hkProcessCollisionInput& input, hkProcessCollisionOutput& output, const hkGskCache* contactCache, hkContactPoint& cp );

			/// hkContactMgr interface implementation.
		virtual hkResult reserveContactPoints( int numPoints );
	
			/// hkContactMgr interface implementation.
		virtual void removeContactPoint( hkContactPointId cpId, hkCollisionConstraintOwner& constraintOwner );

			/// hkContactMgr interface implementation.
		virtual void processContact(const hkCollidable& a, const hkCollidable& b, const hkProcessCollisionInput& input, hkProcessCollisionData& collisionData );

			/// hkContactMgr interface implementation.
		virtual ToiAccept addToi( const hkCdBody& a, const hkCdBody& b, const hkProcessCollisionInput& input, hkProcessCollisionOutput& output, hkTime toi, hkContactPoint& cp, const hkGskCache* gskCache, hkReal& projectedVelocity, hkContactPointMaterial& material );

			/// hkContactMgr interface implementation.
		virtual void removeToi( const hkCollidable& a, const hkCollidable& b, class hkCollisionConstraintOwner& constraintOwner, hkContactPointMaterial& material );

			/// hkContactMgr interface implementation; apply custom toi-collision handling before localized solving
		virtual void processToi( struct hkToiEvent& event, hkReal rotateNormal, class hkArray<class hkEntity*>& outToBeActivated );

		virtual void cleanup(){ delete this; }

		virtual Type getType() { return TYPE_SIMPLE_CONSTRAINT_CONTACT_MGR; }


			/// Get the ids of all contact points in this collision.
		virtual void getAllContactPointIds( hkArray<hkContactPointId>& contactPointIds ) const;

			/// Get the constraint instance
		virtual hkConstraintInstance* getConstraintInstance();

		virtual void toiCollisionResponseBeginCallback( const hkContactPoint& cp, struct hkSimpleConstraintInfoInitInput& inA, struct hkBodyVelocity& velA, hkSimpleConstraintInfoInitInput& inB, hkBodyVelocity& velB);

		virtual void toiCollisionResponseEndCallback( const hkContactPoint& cp, hkReal impulseApplied, struct hkSimpleConstraintInfoInitInput& inA, struct hkBodyVelocity& velA, hkSimpleConstraintInfoInitInput& inB, hkBodyVelocity& velB);

	protected:
			/// produce some memory statistics
		void calcStatistics( hkStatisticsCollector* collector ) const;

	private:
			hkSimpleConstraintContactMgr(){}


	public:

			/// Class that creates instances of hkSimpleConstraintContactMgr.
		class Factory: public hkContactMgrFactory
		{
			public:
				Factory(hkWorld* mgr);

				hkContactMgr*	createContactMgr( const hkCollidable& a, const hkCollidable& b, const hkCollisionInput& input );

			protected:
				hkWorld* m_world;
		};

		//
		//	For internal use only
		//
	public:

		hkUint16					m_reservedContactPoints;
		hkUint16					m_skipNextNprocessCallbacks;

		hkSimpleContactConstraintData	m_contactConstraintData;
		hkConstraintInstance			m_constraint;

};


#endif // HK_DYNAMICS2_SIMPLE_CONSTRAINT_CONTACT_MGR_H

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
