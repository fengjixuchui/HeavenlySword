/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_UTILITIES2_DISABLE_ALL_COLLISION_FILTER_H
#define HK_UTILITIES2_DISABLE_ALL_COLLISION_FILTER_H

#include <hkcollide/filter/hkCollisionFilter.h>
#include <hkdynamics/entity/hkEntityListener.h>

extern const hkClass hkDisableEntityCollisionFilterClass;

	/// This class is subclassed to implement special code for disabling or enabling collisions between objects.
	/// Note: this code is deprecated, please do not use it
class hkDisableEntityCollisionFilter : public hkCollisionFilter, private hkEntityListener
{
	public:

		HK_DECLARE_REFLECTION();

			///
		hkDisableEntityCollisionFilter();

			///
		~hkDisableEntityCollisionFilter();

			///Adds entity if the pointer is valid and not already in list and returns true, else returns false.
		hkBool addEntityToFilter( hkEntity* new_entity);

			/// Removes the entity from the stored array if it exists and returns true, if the entity is not in the array it returns false.
		hkBool removeEntityFromFilter( hkEntity* new_entity);

			/// hkEntityListener implementation - removes any references to removed entities.
		virtual void entityRemovedCallback(hkEntity* entity);

			///
		virtual void entityDeletedCallback(hkEntity* entity);

			///
		int getNumStoredEntities() {return m_disabledEntities.getSize();}

			/// Returns true if the specified objects can collide, overrides from hkCollisionFilter
		virtual hkBool isCollisionEnabled(const hkCollidable& a, const hkCollidable& b ) const;

			// hkShapeCollectionFilter interface forwarding
		virtual	hkBool isCollisionEnabled( const hkCollisionInput& input, const hkCdBody& a, const hkCdBody& b, const hkShapeContainer& bCollection, hkShapeKey bKey  ) const;

			// hkRayShapeCollectionFilter interface forwarding
		virtual hkBool isCollisionEnabled( const hkShapeRayCastInput& aInput, const hkShape& shape, const hkShapeContainer& bContainer, hkShapeKey bKey ) const;

			// hkRayCollidableFilter interface forwarding
		virtual hkBool isCollisionEnabled( const hkWorldRayCastInput& a, const hkCollidable& collidableB ) const;

	public:

		hkArray<hkEntity*> m_disabledEntities;

	public:
	
		hkDisableEntityCollisionFilter(hkFinishLoadedObjectFlag f) {}
};


#endif // HK_EXPORT2_DISABLE_ALL_COLLISION_FILTER_H

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
