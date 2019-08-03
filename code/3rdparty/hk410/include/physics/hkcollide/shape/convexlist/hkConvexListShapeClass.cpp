/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkcollide/shape/convexlist/hkConvexListShape.h'

#include <hkcollide/hkCollide.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkcollide/shape/convexlist/hkConvexListShape.h>


// External pointer and enum types
extern const hkClass hkConvexShapeClass;

//
// Class hkConvexListShape
//
const hkInternalClassMember hkConvexListShape::Members[] =
{
	{ "minDistanceToUseConvexHullForGetClosestPoints", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkConvexListShape,m_minDistanceToUseConvexHullForGetClosestPoints) },
	{ "aabbHalfExtents", HK_NULL, HK_NULL, hkClassMember::TYPE_VECTOR4, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkConvexListShape,m_aabbHalfExtents) },
	{ "aabbCenter", HK_NULL, HK_NULL, hkClassMember::TYPE_VECTOR4, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkConvexListShape,m_aabbCenter) },
	{ "useCachedAabb", HK_NULL, HK_NULL, hkClassMember::TYPE_BOOL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkConvexListShape,m_useCachedAabb) },
	{ "childShapes", &hkConvexShapeClass, HK_NULL, hkClassMember::TYPE_ARRAY, hkClassMember::TYPE_POINTER, 0, 0, HK_OFFSET_OF(hkConvexListShape,m_childShapes) }
};
namespace
{
	struct hkConvexListShape_DefaultStruct
	{
		int s_defaultOffsets[5];
		typedef hkInt8 _hkBool;
		typedef hkReal _hkVector4[4];
		typedef hkReal _hkQuaternion[4];
		typedef hkReal _hkMatrix3[12];
		typedef hkReal _hkRotation[12];
		typedef hkReal _hkQsTransform[12];
		typedef hkReal _hkMatrix4[16];
		typedef hkReal _hkTransform[16];
		bool m_useCachedAabb;
	};
	const hkConvexListShape_DefaultStruct hkConvexListShape_Default =
	{
		{-1,-1,-1,HK_OFFSET_OF(hkConvexListShape_DefaultStruct,m_useCachedAabb),-1},
		false
	};
}

extern const hkClass hkConvexListShapeClass;
const hkClass hkConvexListShapeClass(
	"hkConvexListShape",
	&hkConvexShapeClass, // parent
	sizeof(hkConvexListShape),
	HK_NULL,
	1, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkConvexListShape::Members),
	int(sizeof(hkConvexListShape::Members)/sizeof(hkInternalClassMember)),
	&hkConvexListShape_Default
	);

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
