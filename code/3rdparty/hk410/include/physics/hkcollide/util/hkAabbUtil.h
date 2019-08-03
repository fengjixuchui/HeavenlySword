/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


#ifndef HK_COLLIDE2_AABB_UTIL_H
#define HK_COLLIDE2_AABB_UTIL_H

class hkAabb;
class hkTransform;
class hkVector4;


/// A utility class for creating AABBs that contain various geometric objects.
class hkAabbUtil
{
	public:

			/// Calculates an AABB from an array of vertices.
		static void HK_CALL calcAabb( const float* vertexArray, int numVertices, int striding, hkAabb& aabbOut );

			/// Calculates an AABB from an OBB specified by a transform, a center, and an halfExtents vector and an extra radius
		static inline void HK_CALL calcAabb( const hkTransform& BvToWorld, const hkVector4& halfExtents, const hkVector4& center, float extraRadius, hkAabb& aabbOut);

			/// Calculates an AABB from an OBB specified by a transform, and an halfExtents vector.
		static inline void HK_CALL calcAabb( const hkTransform& BvToWorld, const hkVector4& halfExtents, float extraRadius, hkAabb& aabbOut);

};

#include <hkcollide/util/hkAabbUtil.inl>

#endif // HK_COLLIDE2_AABB_UTIL_H

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
