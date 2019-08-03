/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_UTILITIES2_SURFACE_VELOCITY_UTIL
#define HK_UTILITIES2_SURFACE_VELOCITY_UTIL


#include <hkmath/hkMath.h>
#include <hkdynamics/collide/hkCollisionListener.h>

#include <hkdynamics/entity/hkEntityListener.h>

class hkRigidBody;

	/// Adds a surface velocity to an entity.
	/// Note: If you want to set a surface velocity once and forever and you do not keep a pointer to
	/// the instance, do not forget to call removeReference
class hkSurfaceVelocityUtil: public hkReferencedObject, private hkCollisionListener, private hkEntityListener
{
	public:

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_UTILITIES);

			/// Creates a handle to a (unfiltered) surface velocity.
			///
			/// Will call setSurfaceVelocity() in hkResponseModifier, which gives the 
			/// surface a conveyor-belt like behavior
			/// This is an entity listener, with 2 rigidbodies used to decide when to 
			/// change the mass and what to change it to. You may implement a world listener
			/// and a look-up table to change the response of any object in contact with any
			/// other object
		hkSurfaceVelocityUtil( hkRigidBody* body, const hkVector4& surfaceVelocityWorld);

			/// Sets the surface velocity for the rigid body in world space
		void setSurfaceVelocity( const hkVector4& velWorld );

			/// Get the surface velocity for the rigid body in world space
		inline const hkVector4& getSurfaceVelocity() const;

			/// Get a non-const reference to the surface velocity for the rigid body in world space
		inline hkVector4& getSurfaceVelocity();


	protected:

			// The hkCollisionListener interface implementation
		virtual void contactPointAddedCallback(	  hkContactPointAddedEvent& event);

			// The hkCollisionListener interface implementation
		virtual void contactPointConfirmedCallback( hkContactPointConfirmedEvent& event){}

			// The hkCollisionListener interface implementation
		virtual void contactPointRemovedCallback( hkContactPointRemovedEvent& event){}

			// The hkCollisionListener interface implementation
		virtual void contactProcessCallback( hkContactProcessEvent& event);

		virtual void entityDeletedCallback( hkEntity* entity );

	protected:

		hkRigidBody* m_rigidBody;
		hkVector4 m_surfaceVelocity;
};

#include <hkutilities/collide/surfacevelocity/hkSurfaceVelocityUtil.inl>

#endif		// HK_UTILITIES2_SURFACE_VELOCITY_UTIL

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
