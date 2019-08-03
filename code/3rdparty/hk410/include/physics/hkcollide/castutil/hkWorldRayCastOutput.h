/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_WORLD_RAY_CAST_OUTPUT
#define HK_WORLD_RAY_CAST_OUTPUT

#include <hkcollide/shape/hkShapeRayCastOutput.h>

class hkCollidable;

	/// A structure holding the raycast information of an hkWorld::castRay() or hkAabbPhantom::castRay()
struct hkWorldRayCastOutput: public hkShapeRayCastOutput
{
		/// Constructor.
	inline hkWorldRayCastOutput();

		/// Resets this structure if you want to reuse it for another raycast, by setting the hitFraction to 1
	inline void reset();

		/// Returns true if the raycast has hit an object
	inline hkBool hasHit() const;

		/// The root collidable. Use getOwner() to get to the hkEntity or hkPhantom
	const hkCollidable* m_rootCollidable;

		/// Comparison operator required for sorting
	inline hkBool operator<( const hkWorldRayCastOutput& b ) const;
};

#include <hkcollide/castutil/hkWorldRayCastOutput.inl>


#endif //HK_WORLD_RAY_CAST_OUTPUT

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
