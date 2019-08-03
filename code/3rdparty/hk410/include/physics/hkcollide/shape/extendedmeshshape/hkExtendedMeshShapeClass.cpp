/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkcollide/shape/extendedmeshshape/hkExtendedMeshShape.h'

#include <hkcollide/hkCollide.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkcollide/shape/extendedmeshshape/hkExtendedMeshShape.h>


// External pointer and enum types
extern const hkClass ShapesSubpartconstClass;
extern const hkClass hkExtendedMeshShapeShapesSubpartClass;
extern const hkClass hkExtendedMeshShapeTrianglesSubpartClass;
extern const hkClass hkMeshMaterialClass;

//
// Enum hkExtendedMeshShape::IndexStridingType
//
static const hkInternalClassEnumItem hkExtendedMeshShapeIndexStridingTypeEnumItems[] =
{
	{0, "INDICES_INVALID"},
	{1, "INDICES_INT16"},
	{2, "INDICES_INT32"},
	{3, "INDICES_MAX_ID"},
};

//
// Enum hkExtendedMeshShape::MaterialIndexStridingType
//
static const hkInternalClassEnumItem hkExtendedMeshShapeMaterialIndexStridingTypeEnumItems[] =
{
	{0, "MATERIAL_INDICES_INVALID"},
	{1, "MATERIAL_INDICES_INT8"},
	{2, "MATERIAL_INDICES_INT16"},
	{3, "MATERIAL_INDICES_MAX_ID"},
};

//
// Enum hkExtendedMeshShape::SubpartType
//
static const hkInternalClassEnumItem hkExtendedMeshShapeSubpartTypeEnumItems[] =
{
	{0, "SUBPART_TRIANGLES"},
	{1, "SUBPART_SHAPE"},
};
static const hkInternalClassEnum hkExtendedMeshShapeEnums[] = {
	{"IndexStridingType", hkExtendedMeshShapeIndexStridingTypeEnumItems, 4 },
	{"MaterialIndexStridingType", hkExtendedMeshShapeMaterialIndexStridingTypeEnumItems, 4 },
	{"SubpartType", hkExtendedMeshShapeSubpartTypeEnumItems, 2 }
};
extern const hkClassEnum* hkExtendedMeshShapeIndexStridingTypeEnum = reinterpret_cast<const hkClassEnum*>(&hkExtendedMeshShapeEnums[0]);
extern const hkClassEnum* hkExtendedMeshShapeMaterialIndexStridingTypeEnum = reinterpret_cast<const hkClassEnum*>(&hkExtendedMeshShapeEnums[1]);
extern const hkClassEnum* hkExtendedMeshShapeSubpartTypeEnum = reinterpret_cast<const hkClassEnum*>(&hkExtendedMeshShapeEnums[2]);

//
// Class hkExtendedMeshShape::Subpart
//
static const hkInternalClassMember hkExtendedMeshShape_SubpartClass_Members[] =
{
	{ "type", HK_NULL, hkExtendedMeshShapeSubpartTypeEnum, hkClassMember::TYPE_ENUM, hkClassMember::TYPE_VOID, 0, hkClassMember::ENUM_8, HK_OFFSET_OF(hkExtendedMeshShape::Subpart,m_type) },
	{ "materialIndexStridingType", HK_NULL, hkExtendedMeshShapeIndexStridingTypeEnum, hkClassMember::TYPE_ENUM, hkClassMember::TYPE_VOID, 0, hkClassMember::ENUM_8, HK_OFFSET_OF(hkExtendedMeshShape::Subpart,m_materialIndexStridingType) },
	{ "materialIndexBase", HK_NULL, HK_NULL, hkClassMember::TYPE_ZERO, hkClassMember::TYPE_POINTER, 0, 0, HK_OFFSET_OF(hkExtendedMeshShape::Subpart,m_materialIndexBase) },
	{ "materialIndexStriding", HK_NULL, HK_NULL, hkClassMember::TYPE_INT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkExtendedMeshShape::Subpart,m_materialIndexStriding) },
	{ "materialBase", HK_NULL, HK_NULL, hkClassMember::TYPE_ZERO, hkClassMember::TYPE_POINTER, 0, 0, HK_OFFSET_OF(hkExtendedMeshShape::Subpart,m_materialBase) },
	{ "materialStriding", HK_NULL, HK_NULL, hkClassMember::TYPE_INT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkExtendedMeshShape::Subpart,m_materialStriding) },
	{ "numMaterials", HK_NULL, HK_NULL, hkClassMember::TYPE_INT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkExtendedMeshShape::Subpart,m_numMaterials) }
};
extern const hkClass hkExtendedMeshShapeSubpartClass;
const hkClass hkExtendedMeshShapeSubpartClass(
	"hkExtendedMeshShapeSubpart",
	HK_NULL, // parent
	sizeof(hkExtendedMeshShape::Subpart),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkExtendedMeshShape_SubpartClass_Members),
	int(sizeof(hkExtendedMeshShape_SubpartClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkExtendedMeshShape::TrianglesSubpart
//
static const hkInternalClassMember hkExtendedMeshShape_TrianglesSubpartClass_Members[] =
{
	{ "numTriangleShapes", HK_NULL, HK_NULL, hkClassMember::TYPE_INT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkExtendedMeshShape::TrianglesSubpart,m_numTriangleShapes) },
	{ "vertexBase", HK_NULL, HK_NULL, hkClassMember::TYPE_ZERO, hkClassMember::TYPE_POINTER, 0, 0, HK_OFFSET_OF(hkExtendedMeshShape::TrianglesSubpart,m_vertexBase) },
	{ "vertexStriding", HK_NULL, HK_NULL, hkClassMember::TYPE_INT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkExtendedMeshShape::TrianglesSubpart,m_vertexStriding) },
	{ "numVertices", HK_NULL, HK_NULL, hkClassMember::TYPE_INT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkExtendedMeshShape::TrianglesSubpart,m_numVertices) },
	{ "indexBase", HK_NULL, HK_NULL, hkClassMember::TYPE_ZERO, hkClassMember::TYPE_POINTER, 0, 0, HK_OFFSET_OF(hkExtendedMeshShape::TrianglesSubpart,m_indexBase) },
	{ "stridingType", HK_NULL, hkExtendedMeshShapeIndexStridingTypeEnum, hkClassMember::TYPE_ENUM, hkClassMember::TYPE_VOID, 0, hkClassMember::ENUM_8, HK_OFFSET_OF(hkExtendedMeshShape::TrianglesSubpart,m_stridingType) },
	{ "indexStriding", HK_NULL, HK_NULL, hkClassMember::TYPE_INT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkExtendedMeshShape::TrianglesSubpart,m_indexStriding) }
};

const hkClass hkExtendedMeshShapeTrianglesSubpartClass(
	"hkExtendedMeshShapeTrianglesSubpart",
	&hkExtendedMeshShapeSubpartClass, // parent
	sizeof(hkExtendedMeshShape::TrianglesSubpart),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkExtendedMeshShape_TrianglesSubpartClass_Members),
	int(sizeof(hkExtendedMeshShape_TrianglesSubpartClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkExtendedMeshShape::ShapesSubpart
//
static const hkInternalClassMember hkExtendedMeshShape_ShapesSubpartClass_Members[] =
{
	{ "childShapes", &hkConvexShapeClass, HK_NULL, hkClassMember::TYPE_SIMPLEARRAY, hkClassMember::TYPE_POINTER, 0, 0, HK_OFFSET_OF(hkExtendedMeshShape::ShapesSubpart,m_childShapes) },
	{ "offsetSet", HK_NULL, HK_NULL, hkClassMember::TYPE_BOOL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkExtendedMeshShape::ShapesSubpart,m_offsetSet) },
	{ "rotationSet", HK_NULL, HK_NULL, hkClassMember::TYPE_BOOL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkExtendedMeshShape::ShapesSubpart,m_rotationSet) },
	{ "transform", HK_NULL, HK_NULL, hkClassMember::TYPE_TRANSFORM, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkExtendedMeshShape::ShapesSubpart,m_transform) }
};

const hkClass hkExtendedMeshShapeShapesSubpartClass(
	"hkExtendedMeshShapeShapesSubpart",
	&hkExtendedMeshShapeSubpartClass, // parent
	sizeof(hkExtendedMeshShape::ShapesSubpart),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkExtendedMeshShape_ShapesSubpartClass_Members),
	int(sizeof(hkExtendedMeshShape_ShapesSubpartClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkExtendedMeshShape
//
const hkInternalClassMember hkExtendedMeshShape::Members[] =
{
	{ "scaling", HK_NULL, HK_NULL, hkClassMember::TYPE_VECTOR4, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkExtendedMeshShape,m_scaling) },
	{ "numBitsForSubpartIndex", HK_NULL, HK_NULL, hkClassMember::TYPE_INT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkExtendedMeshShape,m_numBitsForSubpartIndex) },
	{ "trianglesSubparts", &hkExtendedMeshShapeTrianglesSubpartClass, HK_NULL, hkClassMember::TYPE_ARRAY, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkExtendedMeshShape,m_trianglesSubparts) },
	{ "shapesSubparts", &hkExtendedMeshShapeShapesSubpartClass, HK_NULL, hkClassMember::TYPE_ARRAY, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkExtendedMeshShape,m_shapesSubparts) },
	{ "radius", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkExtendedMeshShape,m_radius) },
	{ "pad", HK_NULL, HK_NULL, hkClassMember::TYPE_INT32, hkClassMember::TYPE_VOID, 3, 0, HK_OFFSET_OF(hkExtendedMeshShape,m_pad) }
};
extern const hkClass hkShapeCollectionClass;

extern const hkClass hkExtendedMeshShapeClass;
const hkClass hkExtendedMeshShapeClass(
	"hkExtendedMeshShape",
	&hkShapeCollectionClass, // parent
	sizeof(hkExtendedMeshShape),
	HK_NULL,
	0, // interfaces
	reinterpret_cast<const hkClassEnum*>(hkExtendedMeshShapeEnums),
	3, // enums
	reinterpret_cast<const hkClassMember*>(hkExtendedMeshShape::Members),
	int(sizeof(hkExtendedMeshShape::Members)/sizeof(hkInternalClassMember)),
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
