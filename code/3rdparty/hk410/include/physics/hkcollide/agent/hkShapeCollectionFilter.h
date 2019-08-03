/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


#ifndef HK_COLLIDE2_SHAPE_COLLECTION_FILTER_H
#define HK_COLLIDE2_SHAPE_COLLECTION_FILTER_H

#include <hkcollide/shape/hkShape.h>

struct hkCollisionInput;

	/// A filter which allows for filtering shapes against subshapes
	/// It is called when a shape encounters a shape collection. For each sub shape of the shape collection that needs to be tested against the shape
	/// the filter is called.
class hkShapeCollectionFilter
{
	public:

		HK_DECLARE_REFLECTION();

			/// See whether body A collides with a subshape of shapeCollection B
			/// "input" is the collision input used for the initial collide query
			/// "bodyA" is the cd body for the single shape
			/// "bvTreeBodyB" is the cd body for the bvTree shape or shape collection (if there's no bvTree shape)
			/// "collectionShapeB" is the shape collection of "bvTreeBodyB"
			/// "keyB" is the key needed to access the sub shape of collectionB. If you return true from this function, the system goes on to
			/// perform a collision check between "a" the child shape of "bvTreeBodyB" given by "keyB".
		virtual hkBool isCollisionEnabled( const hkCollisionInput& input, const hkCdBody& bodyA, const hkCdBody& bvTreeBodyB, const hkShapeContainer& containerShapeB, hkShapeKey keyB  ) const = 0;

		/// Virtual destructor for derived objects
		virtual ~hkShapeCollectionFilter() { }
};


#endif //HK_COLLIDE2_SHAPE_COLLECTION_FILTER_H

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
