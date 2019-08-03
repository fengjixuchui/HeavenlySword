/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_NULL_COLLISION_FILTER_H
#define HK_COLLIDE2_NULL_COLLISION_FILTER_H

#include <hkcollide/filter/hkCollisionFilter.h>

extern const hkClass hkNullCollisionFilterClass;

	/// This class is disables no collisions
class hkNullCollisionFilter : public hkCollisionFilter
{
	public:

		HK_DECLARE_REFLECTION();

			/// Construct an hkNullCollisionFilter.
		hkNullCollisionFilter() { }

			///
		~hkNullCollisionFilter();

			/// Checks two collidables
		virtual hkBool isCollisionEnabled( const hkCollidable& a, const hkCollidable& b ) const { return true; }

			// hkShapeCollectionFilter interface forwarding
		virtual	hkBool isCollisionEnabled( const hkCollisionInput& input, const hkCdBody& a, const hkCdBody& b, const hkShapeContainer& bContainer, hkShapeKey bKey  ) const { return true; }

			// hkRayShapeCollectionFilter interface forwarding
		virtual hkBool isCollisionEnabled( const hkShapeRayCastInput& aInput, const hkShape& bShape, const hkShapeContainer& bContainer, hkShapeKey bKey ) const { return true; }

			// hkRayCollidableFilter interface forwarding
		virtual hkBool isCollisionEnabled( const hkWorldRayCastInput& a, const hkCollidable& collidableB ) const { return true; }

	public:

		hkNullCollisionFilter( class hkFinishLoadedObjectFlag flag ) {}
};

#endif // HK_COLLIDE2_COLLISION_FILTER_H

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
