/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkanimation/mapper/hkSkeletonMapper.h'

#include <hkanimation/hkAnimation.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkanimation/mapper/hkSkeletonMapper.h>


// External pointer and enum types
extern const hkClass hkSkeletonMapperDataClass;

//
// Enum hkSkeletonMapper::ConstraintSource
//
static const hkInternalClassEnumItem hkSkeletonMapperConstraintSourceEnumItems[] =
{
	{0, "NO_CONSTRAINTS"},
	{1, "REFERENCE_POSE"},
	{2, "CURRENT_POSE"},
};
static const hkInternalClassEnum hkSkeletonMapperEnums[] = {
	{"ConstraintSource", hkSkeletonMapperConstraintSourceEnumItems, 3 }
};
extern const hkClassEnum* hkSkeletonMapperConstraintSourceEnum = reinterpret_cast<const hkClassEnum*>(&hkSkeletonMapperEnums[0]);

//
// Class hkSkeletonMapper
//
static const hkInternalClassMember hkSkeletonMapperClass_Members[] =
{
	{ "mapping", &hkSkeletonMapperDataClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkSkeletonMapper,m_mapping) }
};
extern const hkClass hkReferencedObjectClass;

extern const hkClass hkSkeletonMapperClass;
const hkClass hkSkeletonMapperClass(
	"hkSkeletonMapper",
	&hkReferencedObjectClass, // parent
	sizeof(hkSkeletonMapper),
	HK_NULL,
	0, // interfaces
	reinterpret_cast<const hkClassEnum*>(hkSkeletonMapperEnums),
	1, // enums
	reinterpret_cast<const hkClassMember*>(hkSkeletonMapperClass_Members),
	int(sizeof(hkSkeletonMapperClass_Members)/sizeof(hkInternalClassMember)),
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
