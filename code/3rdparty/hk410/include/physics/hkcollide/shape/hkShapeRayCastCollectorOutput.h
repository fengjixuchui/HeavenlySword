/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_SHAPE_RAY_COLLECTOR_OUTPUT_H
#define HK_SHAPE_RAY_COLLECTOR_OUTPUT_H

#include <hkcollide/shape/hkShape.h>

	/// The structure used for hkShape::castRayWithCollector results.
	/// Note: the structure can be used only for one raycast,
	/// If you want to reuse it, you have to call reset().
struct hkShapeRayCastCollectorOutput
{
		/// Constructor initialises the hitFraction to 1
	inline hkShapeRayCastCollectorOutput();

		/// Resets this structure if you want to reuse it for another raycast, by setting the hitFraction to 1
	inline void reset();

		/// Returns true, if the ray had hit
	inline hkBool hasHit() const;

		/// The normalized normal of the surface hit by the ray. This normal is in the space of the ray.
	hkVector4 m_normal;

		/// This value will be in the range of [0..1]. It determines where along the ray the hit occurred, where 1 is the end position
		/// of the ray and 0 is the start position of the ray. It is also used as an "early out" value and has to be initialized with 1.0f
	hkReal m_hitFraction;

		/// Optional shape-dependent extra information.
		/// e.g. Heightfields store the packed x and y coordinates of the triangle hit. Multisphere shapes store the index of the sphere.
		/// See the shapes castRay documentation to see which shapes support extraInfo.
	int m_extraInfo;
};

#include <hkcollide/shape/hkShapeRayCastCollectorOutput.inl>

#endif //HK_SHAPE_RAY_COLLECTOR_OUTPUT_H

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
