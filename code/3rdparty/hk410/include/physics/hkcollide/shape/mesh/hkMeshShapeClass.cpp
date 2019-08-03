/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkcollide/shape/mesh/hkMeshShape.h'

#include <hkcollide/hkCollide.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkcollide/shape/mesh/hkMeshShape.h>


// External pointer and enum types
extern const hkClass hkMeshMaterialClass;
extern const hkClass hkMeshShapeSubpartClass;

//
// Enum hkMeshShape::IndexStridingType
//
static const hkInternalClassEnumItem hkMeshShapeIndexStridingTypeEnumItems[] =
{
	{0, "INDICES_INVALID"},
	{1, "INDICES_INT16"},
	{2, "INDICES_INT32"},
	{3, "INDICES_MAX_ID"},
};

//
// Enum hkMeshShape::MaterialIndexStridingType
//
static const hkInternalClassEnumItem hkMeshShapeMaterialIndexStridingTypeEnumItems[] =
{
	{0, "MATERIAL_INDICES_INVALID"},
	{1, "MATERIAL_INDICES_INT8"},
	{2, "MATERIAL_INDICES_INT16"},
	{3, "MATERIAL_INDICES_MAX_ID"},
};
static const hkInternalClassEnum hkMeshShapeEnums[] = {
	{"IndexStridingType", hkMeshShapeIndexStridingTypeEnumItems, 4 },
	{"MaterialIndexStridingType", hkMeshShapeMaterialIndexStridingTypeEnumItems, 4 }
};
extern const hkClassEnum* hkMeshShapeIndexStridingTypeEnum = reinterpret_cast<const hkClassEnum*>(&hkMeshShapeEnums[0]);
extern const hkClassEnum* hkMeshShapeMaterialIndexStridingTypeEnum = reinterpret_cast<const hkClassEnum*>(&hkMeshShapeEnums[1]);

//
// Class hkMeshShape::Subpart
//
static const hkInternalClassMember hkMeshShape_SubpartClass_Members[] =
{
	{ "vertexBase", HK_NULL, HK_NULL, hkClassMember::TYPE_ZERO, hkClassMember::TYPE_POINTER, 0, 0, HK_OFFSET_OF(hkMeshShape::Subpart,m_vertexBase) },
	{ "vertexStriding", HK_NULL, HK_NULL, hkClassMember::TYPE_INT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkMeshShape::Subpart,m_vertexStriding) },
	{ "numVertices", HK_NULL, HK_NULL, hkClassMember::TYPE_INT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkMeshShape::Subpart,m_numVertices) },
	{ "indexBase", HK_NULL, HK_NULL, hkClassMember::TYPE_ZERO, hkClassMember::TYPE_POINTER, 0, 0, HK_OFFSET_OF(hkMeshShape::Subpart,m_indexBase) },
	{ "stridingType", HK_NULL, hkMeshShapeIndexStridingTypeEnum, hkClassMember::TYPE_ENUM, hkClassMember::TYPE_VOID, 0, hkClassMember::ENUM_8, HK_OFFSET_OF(hkMeshShape::Subpart,m_stridingType) },
	{ "materialIndexStridingType", HK_NULL, hkMeshShapeIndexStridingTypeEnum, hkClassMember::TYPE_ENUM, hkClassMember::TYPE_VOID, 0, hkClassMember::ENUM_8, HK_OFFSET_OF(hkMeshShape::Subpart,m_materialIndexStridingType) },
	{ "indexStriding", HK_NULL, HK_NULL, hkClassMember::TYPE_INT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkMeshShape::Subpart,m_indexStriding) },
	{ "numTriangles", HK_NULL, HK_NULL, hkClassMember::TYPE_INT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkMeshShape::Subpart,m_numTriangles) },
	{ "materialIndexBase", HK_NULL, HK_NULL, hkClassMember::TYPE_ZERO, hkClassMember::TYPE_POINTER, 0, 0, HK_OFFSET_OF(hkMeshShape::Subpart,m_materialIndexBase) },
	{ "materialIndexStriding", HK_NULL, HK_NULL, hkClassMember::TYPE_INT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkMeshShape::Subpart,m_materialIndexStriding) },
	{ "materialBase", HK_NULL, HK_NULL, hkClassMember::TYPE_ZERO, hkClassMember::TYPE_POINTER, 0, 0, HK_OFFSET_OF(hkMeshShape::Subpart,m_materialBase) },
	{ "materialStriding", HK_NULL, HK_NULL, hkClassMember::TYPE_INT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkMeshShape::Subpart,m_materialStriding) },
	{ "numMaterials", HK_NULL, HK_NULL, hkClassMember::TYPE_INT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkMeshShape::Subpart,m_numMaterials) }
};
const hkClass hkMeshShapeSubpartClass(
	"hkMeshShapeSubpart",
	HK_NULL, // parent
	sizeof(hkMeshShape::Subpart),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkMeshShape_SubpartClass_Members),
	int(sizeof(hkMeshShape_SubpartClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkMeshShape
//
const hkInternalClassMember hkMeshShape::Members[] =
{
	{ "scaling", HK_NULL, HK_NULL, hkClassMember::TYPE_VECTOR4, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkMeshShape,m_scaling) },
	{ "numBitsForSubpartIndex", HK_NULL, HK_NULL, hkClassMember::TYPE_INT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkMeshShape,m_numBitsForSubpartIndex) },
	{ "subparts", &hkMeshShapeSubpartClass, HK_NULL, hkClassMember::TYPE_ARRAY, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkMeshShape,m_subparts) },
	{ "radius", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkMeshShape,m_radius) },
	{ "pad", HK_NULL, HK_NULL, hkClassMember::TYPE_INT32, hkClassMember::TYPE_VOID, 3, 0, HK_OFFSET_OF(hkMeshShape,m_pad) }
};
extern const hkClass hkShapeCollectionClass;

extern const hkClass hkMeshShapeClass;
const hkClass hkMeshShapeClass(
	"hkMeshShape",
	&hkShapeCollectionClass, // parent
	sizeof(hkMeshShape),
	HK_NULL,
	0, // interfaces
	reinterpret_cast<const hkClassEnum*>(hkMeshShapeEnums),
	2, // enums
	reinterpret_cast<const hkClassMember*>(hkMeshShape::Members),
	int(sizeof(hkMeshShape::Members)/sizeof(hkInternalClassMember)),
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
