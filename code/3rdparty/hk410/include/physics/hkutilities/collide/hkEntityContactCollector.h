/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_CONTACT_COLLECTOR
#define HK_CONTACT_COLLECTOR

#include <hkmath/hkMath.h>
#include <hkdynamics/collide/hkCollisionListener.h>
#include <hkdynamics/entity/hkEntityListener.h>

class hkRigidBody;

	/// This class collects all contact points for a specified entity.\n
	/// For this class to work successfully, hkRigidBody::m_processContactCallbackSkip should be set to 0.
class hkEntityContactCollector : private hkCollisionListener, private hkEntityListener
{
	public:
		/*
		** public classes
		*/
		struct ContactPoint
		{
			hkContactPoint m_point;
			hkRigidBody* m_bodyA;
			hkRigidBody* m_bodyB;
		};

	public:

		virtual ~hkEntityContactCollector();
		
			/// Gets all contact points
		const hkArray<ContactPoint>& getContactPoints(){ return m_contactPoints; }

			/// Flips contact points (normal and bodies), so that bodyA is always refBody
		void flipContactPoints( hkRigidBody* refBody );

			/// Resets all contact points
		void reset();

			/// Adds the contact collector as a listener to an entity. This also stores a reference to the entity
			/// to allow it to remove itself as a listener on destruction.
		void addToEntity( hkEntity* entity );

			/// Removes the contact collector from the entity. 
		void removeFromEntity( hkEntity* entity );

	protected:
			// the hkCollisionListener interface implementation
		virtual void contactPointAddedCallback(	  hkContactPointAddedEvent& event);

			// the hkCollisionListener interface implementation
		virtual void contactPointRemovedCallback( hkContactPointRemovedEvent& event);

			// the hkCollisionListener interface implementation
		virtual void contactPointConfirmedCallback( hkContactPointConfirmedEvent& event);

			// the hkCollisionListener interface implementation
		virtual void contactProcessCallback( hkContactProcessEvent& event);

			// hkEntityListener interface implementation
		virtual void entityDeletedCallback( hkEntity* entity );



	protected:

		hkArray<ContactPoint> m_contactPoints;

		hkInplaceArray<hkEntity*, 1> m_entities;
};


#endif // HK_CONTACT_COLLECTOR

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
