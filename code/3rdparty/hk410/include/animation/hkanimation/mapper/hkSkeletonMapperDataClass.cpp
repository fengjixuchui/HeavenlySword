/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkanimation/mapper/hkSkeletonMapperData.h'

#include <hkanimation/hkAnimation.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkanimation/mapper/hkSkeletonMapperData.h>


// External pointer and enum types
extern const hkClass hkSkeletonClass;
extern const hkClass hkSkeletonMapperDataChainMappingClass;
extern const hkClass hkSkeletonMapperDataSimpleMappingClass;

//
// Class hkSkeletonMapperData::SimpleMapping
//
static const hkInternalClassMember hkSkeletonMapperData_SimpleMappingClass_Members[] =
{
	{ "boneA", HK_NULL, HK_NULL, hkClassMember::TYPE_INT16, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkSkeletonMapperData::SimpleMapping,m_boneA) },
	{ "boneB", HK_NULL, HK_NULL, hkClassMember::TYPE_INT16, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkSkeletonMapperData::SimpleMapping,m_boneB) },
	{ "aFromBTransform", HK_NULL, HK_NULL, hkClassMember::TYPE_QSTRANSFORM, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkSkeletonMapperData::SimpleMapping,m_aFromBTransform) }
};
const hkClass hkSkeletonMapperDataSimpleMappingClass(
	"hkSkeletonMapperDataSimpleMapping",
	HK_NULL, // parent
	sizeof(hkSkeletonMapperData::SimpleMapping),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkSkeletonMapperData_SimpleMappingClass_Members),
	int(sizeof(hkSkeletonMapperData_SimpleMappingClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkSkeletonMapperData::ChainMapping
//
static const hkInternalClassMember hkSkeletonMapperData_ChainMappingClass_Members[] =
{
	{ "startBoneA", HK_NULL, HK_NULL, hkClassMember::TYPE_INT16, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkSkeletonMapperData::ChainMapping,m_startBoneA) },
	{ "endBoneA", HK_NULL, HK_NULL, hkClassMember::TYPE_INT16, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkSkeletonMapperData::ChainMapping,m_endBoneA) },
	{ "startBoneB", HK_NULL, HK_NULL, hkClassMember::TYPE_INT16, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkSkeletonMapperData::ChainMapping,m_startBoneB) },
	{ "endBoneB", HK_NULL, HK_NULL, hkClassMember::TYPE_INT16, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkSkeletonMapperData::ChainMapping,m_endBoneB) },
	{ "startAFromBTransform", HK_NULL, HK_NULL, hkClassMember::TYPE_QSTRANSFORM, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkSkeletonMapperData::ChainMapping,m_startAFromBTransform) },
	{ "endAFromBTransform", HK_NULL, HK_NULL, hkClassMember::TYPE_QSTRANSFORM, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkSkeletonMapperData::ChainMapping,m_endAFromBTransform) }
};
const hkClass hkSkeletonMapperDataChainMappingClass(
	"hkSkeletonMapperDataChainMapping",
	HK_NULL, // parent
	sizeof(hkSkeletonMapperData::ChainMapping),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkSkeletonMapperData_ChainMappingClass_Members),
	int(sizeof(hkSkeletonMapperData_ChainMappingClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkSkeletonMapperData
//
static const hkInternalClassMember hkSkeletonMapperDataClass_Members[] =
{
	{ "skeletonA", &hkSkeletonClass, HK_NULL, hkClassMember::TYPE_POINTER, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkSkeletonMapperData,m_skeletonA) },
	{ "skeletonB", &hkSkeletonClass, HK_NULL, hkClassMember::TYPE_POINTER, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkSkeletonMapperData,m_skeletonB) },
	{ "simpleMappings", &hkSkeletonMapperDataSimpleMappingClass, HK_NULL, hkClassMember::TYPE_ARRAY, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkSkeletonMapperData,m_simpleMappings) },
	{ "chainMappings", &hkSkeletonMapperDataChainMappingClass, HK_NULL, hkClassMember::TYPE_ARRAY, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkSkeletonMapperData,m_chainMappings) },
	{ "unmappedBones", HK_NULL, HK_NULL, hkClassMember::TYPE_ARRAY, hkClassMember::TYPE_INT16, 0, 0, HK_OFFSET_OF(hkSkeletonMapperData,m_unmappedBones) },
	{ "keepUnmappedLocal", HK_NULL, HK_NULL, hkClassMember::TYPE_BOOL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkSkeletonMapperData,m_keepUnmappedLocal) }
};
extern const hkClass hkSkeletonMapperDataClass;
const hkClass hkSkeletonMapperDataClass(
	"hkSkeletonMapperData",
	HK_NULL, // parent
	sizeof(hkSkeletonMapperData),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkSkeletonMapperDataClass_Members),
	int(sizeof(hkSkeletonMapperDataClass_Members)/sizeof(hkInternalClassMember)),
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
