/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_MATH_AABB_H
#define HK_MATH_AABB_H

#include <hkmath/hkMath.h>

/// Axis aligned bounding box.
class hkAabb
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_CDINFO, hkAabb);
		HK_DECLARE_REFLECTION();

			/// An empty constructor, does not initialize anything
		hkAabb() { }

			///Creates a new AABB with the specified dimensions.
		HK_FORCE_INLINE hkAabb(const hkVector4& min, const hkVector4& max);

			/// Returns true if the given AABB overlaps with this one.
		HK_FORCE_INLINE hkBool overlaps( const hkAabb& testAabb ) const;

			/// Is this a valid aabb? I.e. no NaNs and min[i] <= max[i]
		hkBool isValid() const;

			/// Return true if 'other' is enclosed in this aabb.
			/// Boundaries are inclusive.
		HK_FORCE_INLINE hkBool contains(const hkAabb& other) const;

			/// Return true if 'other' is enclosed in this aabb.
			/// Boundaries are inclusive.
		HK_FORCE_INLINE hkBool containsPoint(const hkVector4& other) const;

	public:

			/// The minimum boundary of the aabb (i.e. the coordinates of the corner with the lowest numerical values).
		hkVector4 m_min;

			/// The maximum boundary of the aabb (i.e. the coordinates of the corner with the highest numerical values).
		hkVector4 m_max;
};

#include <hkmath/basetypes/hkAabb.inl>

#endif // HK_MATH_AABB_H

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
