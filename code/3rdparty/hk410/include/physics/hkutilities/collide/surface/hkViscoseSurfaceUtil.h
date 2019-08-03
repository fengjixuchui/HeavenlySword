/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_UTILITIES2_VISCOSE_SURFACE_UTIL
#define HK_UTILITIES2_VISCOSE_SURFACE_UTIL


#include <hkmath/hkMath.h>
#include <hkdynamics/collide/hkCollisionListener.h>

#include <hkdynamics/entity/hkEntity.h>
#include <hkdynamics/entity/hkEntityListener.h>


class hkRigidBody;

	/// Makes a surface a little slippery (like thick oil or tar).
	/// It allows for removing objects which got stuck in a narrow gap.
	/// Typical applications include cars getting stuck between two buildings
	/// Note: you have to call makeSurfaceViscose before you add the entity to the world
	/// Note: this class only works if the default hkSimpleConstraintContactMgr is used
class hkViscoseSurfaceUtil: public hkReferencedObject, private hkCollisionListener, private hkEntityListener
{
	public:
		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_UTILITIES);

			/// Creates a handle to a surface velocity
		static inline void HK_CALL makeSurfaceViscose(hkRigidBody* entity);	
		
	protected:
		hkViscoseSurfaceUtil( hkRigidBody* entity );

			// The hkCollisionListener interface implementation
		virtual void contactPointAddedCallback(	  hkContactPointAddedEvent& event);

			// The hkCollisionListener interface implementation
		virtual void contactPointRemovedCallback( hkContactPointRemovedEvent& event){}

			// The hkCollisionListener interface implementation
		virtual void contactProcessCallback( hkContactProcessEvent& event){}

			// The hkCollisionListener interface implementation
		virtual void contactPointConfirmedCallback(hkContactPointConfirmedEvent& event){}

		virtual void entityDeletedCallback( hkEntity* entity );

		//
		// For internal use
		//	
	public:
		hkEntity*   m_entity;
};

void HK_CALL hkViscoseSurfaceUtil::makeSurfaceViscose(hkRigidBody* entity)
{
	new hkViscoseSurfaceUtil( entity );
}

#endif		// HK_UTILITIES2_VISCOSE_SURFACE_UTIL

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
