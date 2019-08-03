/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_UTILITIES2_SOFT_CONTACT_UTIL
#define HK_UTILITIES2_SOFT_CONTACT_UTIL


#include <hkmath/hkMath.h>
#include <hkdynamics/collide/hkCollisionListener.h>

#include <hkdynamics/entity/hkEntity.h>
#include <hkdynamics/entity/hkEntityListener.h>

class hkRigidBody;

	/// A listener class used to soften collisions.
	///
	/// The utility scales the forces applied between bodies during collision resolution. This results in softened collisions and contact.
	/// Note that this utility should not be used for pairs of bodies between which continuous collision detection and response is performed,
	/// as this might significantly impact simulations performance.
	/// This feature is currently beta - use it with care.
class hkSoftContactUtil: public hkReferencedObject, private hkCollisionListener, private hkEntityListener
{
	public:	
		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_UTILITIES);

			/// Adds the soft contact util as a listener to bodyA.
			///
			/// If a contact point is added between bodyA and bodyB, the forces applied between the bodies are changed.
			/// This may be used to achieve soft contacts.
			/// If bodyB is set to NULL, than bodyA will get soft contacts with all bodies
		hkSoftContactUtil( hkRigidBody* bodyA, hkRigidBody* optionalBodyB, hkReal forceScale, hkReal maxAccel );

		~hkSoftContactUtil();

	protected:

			// The hkCollisionListener interface implementation
		void contactPointAddedCallback(	hkContactPointAddedEvent& event );

			// The hkCollisionListener interface implementation
		void contactPointRemovedCallback( hkContactPointRemovedEvent& event ){}

			// The hkCollisionListener interface implementation
		void contactPointConfirmedCallback( hkContactPointConfirmedEvent& event){}

			// The hkCollisionListener interface implementation
		void contactProcessCallback( hkContactProcessEvent& event );

			// The hkCollisionListener interface implementation
		void entityDeletedCallback( hkEntity* entity );

		//
		//	Internal public section
		//
	protected:
		hkRigidBody* m_bodyA;
		hkRigidBody* m_bodyB;

	public:
		hkReal m_forceScale;
		hkReal m_maxAcceleration;
};

#endif		// HK_UTILITIES2_SOFT_CONTACT_UTIL

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
