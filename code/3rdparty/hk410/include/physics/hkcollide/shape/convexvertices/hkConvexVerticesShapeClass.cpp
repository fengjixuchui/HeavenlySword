/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkcollide/shape/convexvertices/hkConvexVerticesShape.h'

#include <hkcollide/hkCollide.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkcollide/shape/convexvertices/hkConvexVerticesShape.h>


// External pointer and enum types
extern const hkClass hkConvexVerticesShapeFourVectorsClass;

//
// Class hkConvexVerticesShape::FourVectors
//
static const hkInternalClassMember hkConvexVerticesShape_FourVectorsClass_Members[] =
{
	{ "x", HK_NULL, HK_NULL, hkClassMember::TYPE_VECTOR4, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkConvexVerticesShape::FourVectors,m_x) },
	{ "y", HK_NULL, HK_NULL, hkClassMember::TYPE_VECTOR4, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkConvexVerticesShape::FourVectors,m_y) },
	{ "z", HK_NULL, HK_NULL, hkClassMember::TYPE_VECTOR4, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkConvexVerticesShape::FourVectors,m_z) }
};
const hkClass hkConvexVerticesShapeFourVectorsClass(
	"hkConvexVerticesShapeFourVectors",
	HK_NULL, // parent
	sizeof(hkConvexVerticesShape::FourVectors),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkConvexVerticesShape_FourVectorsClass_Members),
	int(sizeof(hkConvexVerticesShape_FourVectorsClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkConvexVerticesShape
//
const hkInternalClassMember hkConvexVerticesShape::Members[] =
{
	{ "aabbHalfExtents", HK_NULL, HK_NULL, hkClassMember::TYPE_VECTOR4, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkConvexVerticesShape,m_aabbHalfExtents) },
	{ "aabbCenter", HK_NULL, HK_NULL, hkClassMember::TYPE_VECTOR4, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkConvexVerticesShape,m_aabbCenter) },
	{ "rotatedVertices", &hkConvexVerticesShapeFourVectorsClass, HK_NULL, hkClassMember::TYPE_ARRAY, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkConvexVerticesShape,m_rotatedVertices) },
	{ "numVertices", HK_NULL, HK_NULL, hkClassMember::TYPE_INT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkConvexVerticesShape,m_numVertices) },
	{ "planeEquations", HK_NULL, HK_NULL, hkClassMember::TYPE_ARRAY, hkClassMember::TYPE_VECTOR4, 0, 0, HK_OFFSET_OF(hkConvexVerticesShape,m_planeEquations) }
};
extern const hkClass hkConvexShapeClass;

extern const hkClass hkConvexVerticesShapeClass;
const hkClass hkConvexVerticesShapeClass(
	"hkConvexVerticesShape",
	&hkConvexShapeClass, // parent
	sizeof(hkConvexVerticesShape),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkConvexVerticesShape::Members),
	int(sizeof(hkConvexVerticesShape::Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
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
