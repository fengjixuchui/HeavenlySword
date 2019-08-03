/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_NULL_CONTACT_MGR_H
#define HK_COLLIDE2_NULL_CONTACT_MGR_H

#include <hkcollide/agent/hkContactMgr.h>

class hkCollidable;
struct hkCollisionInput;
struct hkProcessCdPoint;


	/// An hkContactMgr that doesn't add any contact points.
class hkNullContactMgr: public hkContactMgr
{
	public:

			/// No contact points are added in this implementation, however HK_NULL is a valid contact point id (!= HK_INVALID_CONTACT_POINT )
		hkContactPointId addContactPoint( const hkCdBody& a, const hkCdBody& b, const hkProcessCollisionInput& env, hkProcessCollisionOutput& output, const hkGskCache* contactCache, hkContactPoint& cp)
		{ return 0; }

		void removeContactPoint( hkContactPointId cpId, hkCollisionConstraintOwner& constraintOwner )
		{}

		void processContact(const hkCollidable& a, const hkCollidable& b, const hkProcessCollisionInput& input, hkProcessCollisionData& collisionData )
		{}

		ToiAccept addToi( const hkCdBody& a, const hkCdBody& b, const hkProcessCollisionInput& input, hkProcessCollisionOutput& output, hkTime toi, hkContactPoint& cp, const hkGskCache* gskCache, hkReal& projectedVelocity, hkContactPointMaterial& material )
		{
			return TOI_REJECT;
		}

		void removeToi( const hkCollidable& a, const hkCollidable& b, class hkCollisionConstraintOwner& constraintOwner, hkContactPointMaterial& material )
		{}

		void processToi( struct hkToiEvent& event, hkReal rotateNormal, class hkArray<class hkEntity*>& outToBeActivated )
		{}


		hkResult reserveContactPoints( int numPoints )
		{
			return HK_SUCCESS;
		}

		void cleanup()
		{}

};

#endif // HK_COLLIDE2_NULL_CONTACT_MGR_H

/*
* Havok SDK - PUBLIC RELEASE, BUILD(#20060902)
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
