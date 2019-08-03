/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_UTILITIES2_MASS_CHANGER_UTIL
#define HK_UTILITIES2_MASS_CHANGER_UTIL


#include <hkmath/hkMath.h>
#include <hkdynamics/collide/hkCollisionListener.h>

#include <hkdynamics/entity/hkEntity.h>
#include <hkdynamics/entity/hkEntityListener.h>

class hkRigidBody;

/// A listener class used to change the masses of objects during collision
class hkCollisionMassChangerUtil: public hkReferencedObject, private hkCollisionListener, private hkEntityListener
{
	public:	
		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_UTILITIES);

			/// Adds the mass changer util as a listener to bodyA
			/// if a contact point is added between bodyA and bodyB, the relative mass of the collision is changed.
			/// you specify this using the invMassA for the apparent inverse mass of body A in the collision, and
			/// invMassB for the apparant inverse mass of body B in the collision.
			/// By default invMassA is 0, and invMassB is 1, i.e. bodyA will appear to be immovable from body B's perspective
		hkCollisionMassChangerUtil( hkRigidBody* bodyA, hkRigidBody* bodyB, float inverseMassA = 0, float inverseMassB = 1 );

		~hkCollisionMassChangerUtil();

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


	protected:

		hkRigidBody* m_bodyA;
		hkRigidBody* m_bodyB;
		hkReal m_inverseMassA;
		hkReal m_inverseMassB;
};

#endif		// HK_UTILITIES2_MASS_CHANGER_UTIL

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
