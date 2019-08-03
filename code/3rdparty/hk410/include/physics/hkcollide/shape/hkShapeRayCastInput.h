/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_SHAPE_RAY_CAST_INPUT
#define HK_SHAPE_RAY_CAST_INPUT

#include <hkmath/hkMath.h>

class hkRayShapeCollectionFilter;

	/// This is the input structure you need to fill out in order to call any hkShape::castRay() function
struct hkShapeRayCastInput
{
	HK_DECLARE_REFLECTION();

		/// The constructor initialises the filter pointer to HK_NULL, which means that no shape collection filtering
		/// will be performed by default.
	inline hkShapeRayCastInput();


		/// The start position of the ray in local space
	hkVector4 m_from;

		/// The end position of the ray in local space
	hkVector4 m_to;

		/// Filter information associated with this ray.
		/// You only need to set this if you supply a filter (by setting the m_rayShapeCollectionFilter) which
		/// accesses this value. For example, the supplied filter, hkGroupFilter, uses this value, so if you use
		/// this the hkGroupFilter as the m_rayShapeCollectionFilter you must set this value.
	hkUint32 m_filterInfo;

		/// This is the filter which will be used to decide whether to hit or not hit.
		/// Note: this variable will normally be set automatically by the engine if you
		/// call hkWorld::castRay / hkAabbPhantom::castRay
	const hkRayShapeCollectionFilter*	m_rayShapeCollectionFilter;

};

#include <hkcollide/shape/hkShapeRayCastInput.inl>

#endif // HK_SHAPE_RAY_CAST_INPUT

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
