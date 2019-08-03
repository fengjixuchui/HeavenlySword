/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_WORLD_RAY_CAST_INPUT
#define HK_WORLD_RAY_CAST_INPUT

#include <hkcollide/shape/hkShapeRayCastInput.h>

	/// This class is used as an input structure for hkWorld::castRay and hkAabbPhantom::castRay
struct hkWorldRayCastInput
{		
	/// The starting point of the ray in world space.
	hkVector4 m_from;

	/// The end point of the ray in world space.
	hkVector4 m_to;	

	/// Determines whether m_filterInfo is compared with filter info for hkShapeCollection child shapes
	/// in addition to it being tested against the hkCollidable's filter info (the filter set in
	/// hkRigidBodyCinfo::m_collisionFilterInfo). Default is false.
	hkBool m_enableShapeCollectionFilter;

	/// Ray's filter value. Default is zero.
	hkUint32 m_filterInfo;

	hkWorldRayCastInput(): m_enableShapeCollectionFilter(false), m_filterInfo(0) {}
};


#endif //HK_WORLD_RAY_CAST_INPUT

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
