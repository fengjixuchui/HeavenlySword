/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_SHAPE_RAY_CAST_RESULTS
#define HK_SHAPE_RAY_CAST_RESULTS

#include <hkcollide/shape/hkShapeRayCastCollectorOutput.h>

	/// The structure used for hkShape::castRay results.
	/// Note: the structure can be used only for one raycast,
	/// If you want to reuse it, you have to call reset()
struct hkShapeRayCastOutput : public hkShapeRayCastCollectorOutput
{
		/// Create an initialized output.
	inline hkShapeRayCastOutput();

		/// Resets this structure if you want to reuse it for another raycast, by setting the hitFraction to 1
	inline void reset();

		/// Maximum depth of key hierarchy.
	enum { MAX_HIERARCHY_DEPTH=8 };

		/// The shapekeys of all the intermediate shapes down to the leaf shape which has been hit.
		/// The list ends with HK_INVALID_SHAPE_KEY. See the "Ray cast hierarchy" section of the user guide.
	hkShapeKey m_shapeKeys[MAX_HIERARCHY_DEPTH];

		// Internal. Used for shapes containing child shapes.
	inline void changeLevel(int delta);

		// Internal. Used with changeLevel().
	inline void setKey(hkShapeKey key);

private:

	int m_shapeKeyIndex;

	inline void _reset();
};

#include <hkcollide/shape/hkShapeRayCastOutput.inl>


#endif //HK_SHAPE_RAY_CAST_RESULTS

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
