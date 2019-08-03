/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_RAY_COLLIDABLE_FILTER_H
#define HK_COLLIDE2_RAY_COLLIDABLE_FILTER_H

struct hkWorldRayCastInput;

extern const hkClass hkRayCollidableFilterClass;

	/// This filter filters between ray casts and collidables.
	/// It is called when ray-casting through the broad phase, or ray casting against a number of hkCollidable objects.
	/// For ray against hkShapeCollection child shape the hkRayShapeCollectionFilter is used instead.
class hkRayCollidableFilter
{
	public:

		HK_DECLARE_REFLECTION();

		virtual ~hkRayCollidableFilter() { }

			/// Return true if the ray should hit the collidable, false if it should miss.
		virtual hkBool isCollisionEnabled( const hkWorldRayCastInput& a, const hkCollidable& collidableB ) const = 0;

};

#endif //HK_COLLIDE2_RAY_COLLIDABLE_FILTER_H

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
