/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_REPORT_CONTACT_MGR_H
#define HK_DYNAMICS2_REPORT_CONTACT_MGR_H

#include <hkcollide/dispatch/contactmgr/hkContactMgrFactory.h>
#include <hkdynamics/collide/hkDynamicsContactMgr.h>

class hkWorld;
class hkCollidable;
struct hkCollisionInput;

/// Warning: there must be a maximum of ONE instance of this class pointing to a contact point.
/// No other persistent pointers to the contact point are allowed.
/// If the address of this class changes, hkContactMgr::moveContactPoint() must be called
class hkReportContactMgr: public hkDynamicsContactMgr
{
	public:

		hkReportContactMgr( hkWorld* world, hkRigidBody *bodyA, hkRigidBody *bodyB );

		~hkReportContactMgr();

			/// hkContactMgr interface implementation.
		virtual hkContactPointId addContactPoint(const 	hkCdBody& a, const hkCdBody& b, const hkProcessCollisionInput& input, hkProcessCollisionOutput& output, const hkGskCache* contactCache, hkContactPoint& cp );

			/// hkContactMgr interface implementation.
		virtual hkResult reserveContactPoints( int numPoints ){ return HK_SUCCESS; }

			/// hkContactMgr interface implementation.
		virtual void removeContactPoint( hkContactPointId cpId, hkCollisionConstraintOwner& info );

			/// hkContactMgr interface implementation.
		virtual void processContact(const hkCollidable& a, const hkCollidable& b, const hkProcessCollisionInput& input, hkProcessCollisionData& collisionData );

			/// hkContactMgr interface implementation.
		virtual ToiAccept addToi( const hkCdBody& a, const hkCdBody& b, const hkProcessCollisionInput& input, hkProcessCollisionOutput& output, hkTime toi, hkContactPoint& cp, const hkGskCache* gskCache, hkReal& projectedVelocity, hkContactPointMaterial& material );

			/// hkContactMgr interface implementation.
		virtual void removeToi( const hkCollidable& a, const hkCollidable& b, class hkCollisionConstraintOwner& constraintOwner, hkContactPointMaterial& material );

			/// hkContactMgr interface implementation.
		virtual void processToi( struct hkToiEvent& event, hkReal rotateNormal, class hkArray<class hkEntity*>& outToBeActivated );

		virtual void cleanup(){ delete this; }

		virtual Type getType() { return TYPE_REPORT_CONTACT_MGR; }


	public:

			/// Class that creates instances of hkReportContactMgr.
		class Factory: public hkContactMgrFactory
		{
			public:
				Factory(hkWorld* mgr);

				hkContactMgr*	createContactMgr( const hkCollidable& a, const hkCollidable& b, const hkCollisionInput& input );

			protected:
				hkWorld* m_world;
		};


	protected:

		hkRigidBody*				m_bodyA;
		hkRigidBody*				m_bodyB;
		hkInt16						m_skipNextNprocessCallbacks;
};

#endif // HK_DYNAMICS2_REPORT_CONTACT_MGR_H

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
