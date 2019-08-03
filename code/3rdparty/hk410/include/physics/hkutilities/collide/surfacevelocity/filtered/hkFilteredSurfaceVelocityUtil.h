/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_UTILITIES2_FILTERED_SURFACE_VELOCITY_UTIL
#define HK_UTILITIES2_FILTERED_SURFACE_VELOCITY_UTIL


#include <hkutilities/collide/surfacevelocity/hkSurfaceVelocityUtil.h>

class hkRigidBody;

	/// Adds a surface velocity to all enabled entities.
	/// Note: If you want to set a surface velocity once and forever and you do not keep a pointer to
	/// the instance, do not forget to call removeReference
class hkFilteredSurfaceVelocityUtil: public hkSurfaceVelocityUtil
{
	public:

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_UTILITIES);

			/// Creates a handle to a filtered surface velocity.
			///
			/// Will call setSurfaceVelocity() in hkResponseModifier, which gives the 
			/// surface a conveyor-belt like behavior. The propertyId is used as the property key
			/// to store a flag within an enabled entity. This value should be unique.
		hkFilteredSurfaceVelocityUtil(hkRigidBody* body, const hkVector4& surfaceVelocityWorld, int propertyId);

			/// Enables surface velocity for the supplied entity.
			///
			/// Also stores a flag as property in the entity using the unique propertyId supplied in this
			/// class'es constructor.
			/// To understand constraintOwner, please read the reference manual for hkResponseModifier
		void enableEntity(hkEntity *entity, class hkConstraintOwner* constraintOwner = HK_NULL );

			/// Disables surface velocity for the supplied entity. Also removes the flag property from entity.
			/// To understand constraintOwner, please read the reference manual for hkResponseModifier
		void disableEntity(hkEntity *entity, hkConstraintOwner* constraintOwner = HK_NULL);


	protected:

			// The hkCollisionListener interface implementation
		virtual void contactPointAddedCallback(hkContactPointAddedEvent& event);


	protected:

			/// Custom property id used when marking an entity as 'enabled'
		int m_propertyId;

};

#endif		// HK_UTILITIES2_FILTERED_SURFACE_VELOCITY_UTIL

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
