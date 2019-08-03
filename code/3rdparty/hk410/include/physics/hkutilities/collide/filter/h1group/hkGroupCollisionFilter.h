/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_UTILITIES2_GROUP_COLLISION_FILTER_H
#define HK_UTILITIES2_GROUP_COLLISION_FILTER_H

#include <hkcollide/filter/hkCollisionFilter.h>

extern const hkClass hkGroupCollisionFilterClass;

/// The group collision filter. There are 32 possible collision groups, and
/// each entity can be assigned to any number of groups 
/// (one group is defined by one bit in a 32 bit integer).
/// You can then disable or enable collisions between these groups.
/// Objects that have not been assigned to a collision group have a collision group value of 0.
/// Note: this code is deprecated, please do not use it
/// Collision group 32 is reserved for use as a disable all collision filter, any bodies in that group
/// will not collide with anything
class hkGroupCollisionFilter : public hkCollisionFilter
{
	public:

		HK_DECLARE_REFLECTION();

			///
		hkGroupCollisionFilter();

			/// Enables collisions between the specified collision groups. If one of the parameters passed to this function is 0,
			/// the m_noGroupCollisionEnabled flag is set to true.
			/// This is a flag that indicates whether objects that are not assigned to any group can collide with objects that are group members.
			/// By default, it is set to true.
		void enableCollisionGroups(hkUint32 groupBitsA, hkUint32 groupBitsB);

			/// Disables collisions between the specified collision groups. If one of the parameters passed to this function is 0,
			/// the m_noGroupCollisionEnabled flag is set to false.
		void disableCollisionGroups(hkUint32 groupBitsA, hkUint32 groupBitsB);

			/// Returns true if the specified groups A and B have collisions enabled. 
			/// The special cases of (0,X) or (X,0) always return the m_noGroupCollisionEnabled value.
		hkBool isCollisionEnabled(hkUint32 groupBitsA, hkUint32 groupBitsB) const;


			/// hkCollisionFilter interface implementation which returns true if the objects are enabled to collide, based on their collision groups. 
			/// The implementation calls isCollisionEnabled( a->getOwner()->getCollisionFilterInfo() ... )
		virtual hkBool isCollisionEnabled(const hkCollidable& a, const hkCollidable& b ) const;

			// hkShapeCollectionFilter interface forwarding
		virtual	hkBool isCollisionEnabled( const hkCollisionInput& input, const hkCdBody& a, const hkCdBody& b, const hkShapeContainer& bContainer, hkShapeKey bKey  ) const;

			// hkRayShapeCollectionFilter interface forwarding
		virtual hkBool isCollisionEnabled( const hkShapeRayCastInput& aInput, const hkShape& shape, const hkShapeContainer& bContainer, hkShapeKey bKey ) const;

			// hkRayCollidableFilter interface forwarding
		virtual hkBool isCollisionEnabled( const hkWorldRayCastInput& a, const hkCollidable& collidableB ) const;

	public:

		hkBool		m_noGroupCollisionEnabled;
		hkUint32    m_collisionGroups[32];

	public:

		hkGroupCollisionFilter(hkFinishLoadedObjectFlag f) {}
};




#endif // HK_DYNAMICS2_GROUP_COLLISION_FILTER_H

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
