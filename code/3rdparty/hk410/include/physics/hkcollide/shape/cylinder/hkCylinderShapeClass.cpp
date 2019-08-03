/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkcollide/shape/cylinder/hkCylinderShape.h'

#include <hkcollide/hkCollide.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkcollide/shape/cylinder/hkCylinderShape.h>


//
// Enum hkCylinderShape::VertexIdEncoding
//
static const hkInternalClassEnumItem hkCylinderShapeVertexIdEncodingEnumItems[] =
{
	{7, "VERTEX_ID_ENCODING_IS_BASE_A_SHIFT"},
	{6, "VERTEX_ID_ENCODING_SIN_SIGN_SHIFT"},
	{5, "VERTEX_ID_ENCODING_COS_SIGN_SHIFT"},
	{4, "VERTEX_ID_ENCODING_IS_SIN_LESSER_SHIFT"},
	{0x0f, "VERTEX_ID_ENCODING_VALUE_MASK"},
};
static const hkInternalClassEnum hkCylinderShapeEnums[] = {
	{"VertexIdEncoding", hkCylinderShapeVertexIdEncodingEnumItems, 5 }
};
extern const hkClassEnum* hkCylinderShapeVertexIdEncodingEnum = reinterpret_cast<const hkClassEnum*>(&hkCylinderShapeEnums[0]);

//
// Class hkCylinderShape
//
const hkInternalClassMember hkCylinderShape::Members[] =
{
	{ "cylRadius", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkCylinderShape,m_cylRadius) },
	{ "cylBaseRadiusFactorForHeightFieldCollisions", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkCylinderShape,m_cylBaseRadiusFactorForHeightFieldCollisions) },
	{ "vertexA", HK_NULL, HK_NULL, hkClassMember::TYPE_VECTOR4, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkCylinderShape,m_vertexA) },
	{ "vertexB", HK_NULL, HK_NULL, hkClassMember::TYPE_VECTOR4, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkCylinderShape,m_vertexB) },
	{ "perpendicular1", HK_NULL, HK_NULL, hkClassMember::TYPE_VECTOR4, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkCylinderShape,m_perpendicular1) },
	{ "perpendicular2", HK_NULL, HK_NULL, hkClassMember::TYPE_VECTOR4, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkCylinderShape,m_perpendicular2) }
};
namespace
{
	struct hkCylinderShape_DefaultStruct
	{
		int s_defaultOffsets[6];
		typedef hkInt8 _hkBool;
		typedef hkReal _hkVector4[4];
		typedef hkReal _hkQuaternion[4];
		typedef hkReal _hkMatrix3[12];
		typedef hkReal _hkRotation[12];
		typedef hkReal _hkQsTransform[12];
		typedef hkReal _hkMatrix4[16];
		typedef hkReal _hkTransform[16];
		hkReal m_cylBaseRadiusFactorForHeightFieldCollisions;
	};
	const hkCylinderShape_DefaultStruct hkCylinderShape_Default =
	{
		{-1,HK_OFFSET_OF(hkCylinderShape_DefaultStruct,m_cylBaseRadiusFactorForHeightFieldCollisions),-1,-1,-1,-1},
		0.8f
	};
}
extern const hkClass hkConvexShapeClass;

extern const hkClass hkCylinderShapeClass;
const hkClass hkCylinderShapeClass(
	"hkCylinderShape",
	&hkConvexShapeClass, // parent
	sizeof(hkCylinderShape),
	HK_NULL,
	0, // interfaces
	reinterpret_cast<const hkClassEnum*>(hkCylinderShapeEnums),
	1, // enums
	reinterpret_cast<const hkClassMember*>(hkCylinderShape::Members),
	int(sizeof(hkCylinderShape::Members)/sizeof(hkInternalClassMember)),
	&hkCylinderShape_Default
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
