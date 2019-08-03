/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkbase/class/hkClassMember.h'

#include <hkbase/hkBase.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkbase/class/hkClassMember.h>


// External pointer and enum types
extern const hkClass hkClassClass;
extern const hkClass hkClassEnumClass;

//
// Enum hkClassMember::Type
//
static const hkInternalClassEnumItem hkClassMemberTypeEnumItems[] =
{
	{0, "TYPE_VOID"},
	{1, "TYPE_BOOL"},
	{2, "TYPE_CHAR"},
	{3, "TYPE_INT8"},
	{4, "TYPE_UINT8"},
	{5, "TYPE_INT16"},
	{6, "TYPE_UINT16"},
	{7, "TYPE_INT32"},
	{8, "TYPE_UINT32"},
	{9, "TYPE_INT64"},
	{10, "TYPE_UINT64"},
	{11, "TYPE_REAL"},
	{12, "TYPE_VECTOR4"},
	{13, "TYPE_QUATERNION"},
	{14, "TYPE_MATRIX3"},
	{15, "TYPE_ROTATION"},
	{16, "TYPE_QSTRANSFORM"},
	{17, "TYPE_MATRIX4"},
	{18, "TYPE_TRANSFORM"},
	{19, "TYPE_ZERO"},
	{20, "TYPE_POINTER"},
	{21, "TYPE_FUNCTIONPOINTER"},
	{22, "TYPE_ARRAY"},
	{23, "TYPE_INPLACEARRAY"},
	{24, "TYPE_ENUM"},
	{25, "TYPE_STRUCT"},
	{26, "TYPE_SIMPLEARRAY"},
	{27, "TYPE_HOMOGENEOUSARRAY"},
	{28, "TYPE_VARIANT"},
	{29, "TYPE_CSTRING"},
	{30, "TYPE_ULONG"},
	{31, "TYPE_MAX"},
};

//
// Enum hkClassMember::Flags
//
static const hkInternalClassEnumItem hkClassMemberFlagsEnumItems[] =
{
	{1, "POINTER_OPTIONAL"},
	{2, "POINTER_VOIDSTAR"},
	{8, "ENUM_8"},
	{16, "ENUM_16"},
	{32, "ENUM_32"},
	{64, "ARRAY_RAWDATA"},
};

//
// Enum hkClassMember::Range
//
static const hkInternalClassEnumItem hkClassMemberRangeEnumItems[] =
{
	{0, "INVALID"},
	{1, "DEFAULT"},
	{2, "ABS_MIN"},
	{4, "ABS_MAX"},
	{8, "SOFT_MIN"},
	{16, "SOFT_MAX"},
	{32, "RANGE_MAX"},
};
static const hkInternalClassEnum hkClassMemberEnums[] = {
	{"Type", hkClassMemberTypeEnumItems, 32 },
	{"Flags", hkClassMemberFlagsEnumItems, 6 },
	{"Range", hkClassMemberRangeEnumItems, 7 }
};
extern const hkClassEnum* hkClassMemberTypeEnum = reinterpret_cast<const hkClassEnum*>(&hkClassMemberEnums[0]);
extern const hkClassEnum* hkClassMemberFlagsEnum = reinterpret_cast<const hkClassEnum*>(&hkClassMemberEnums[1]);
extern const hkClassEnum* hkClassMemberRangeEnum = reinterpret_cast<const hkClassEnum*>(&hkClassMemberEnums[2]);

//
// Class hkClassMember
//
const hkInternalClassMember hkClassMember::Members[] =
{
	{ "name", HK_NULL, HK_NULL, hkClassMember::TYPE_CSTRING, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkClassMember,m_name) },
	{ "class", &hkClassClass, HK_NULL, hkClassMember::TYPE_POINTER, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkClassMember,m_class) },
	{ "enum", &hkClassEnumClass, HK_NULL, hkClassMember::TYPE_POINTER, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkClassMember,m_enum) },
	{ "type", HK_NULL, hkClassMemberTypeEnum, hkClassMember::TYPE_ENUM, hkClassMember::TYPE_VOID, 0, hkClassMember::ENUM_8, HK_OFFSET_OF(hkClassMember,m_type) },
	{ "subtype", HK_NULL, hkClassMemberTypeEnum, hkClassMember::TYPE_ENUM, hkClassMember::TYPE_VOID, 0, hkClassMember::ENUM_8, HK_OFFSET_OF(hkClassMember,m_subtype) },
	{ "cArraySize", HK_NULL, HK_NULL, hkClassMember::TYPE_INT16, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkClassMember,m_cArraySize) },
	{ "flags", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT16, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkClassMember,m_flags) },
	{ "offset", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT16, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkClassMember,m_offset) }
};
extern const hkClass hkClassMemberClass;
const hkClass hkClassMemberClass(
	"hkClassMember",
	HK_NULL, // parent
	sizeof(hkClassMember),
	HK_NULL,
	0, // interfaces
	reinterpret_cast<const hkClassEnum*>(hkClassMemberEnums),
	3, // enums
	reinterpret_cast<const hkClassMember*>(hkClassMember::Members),
	int(sizeof(hkClassMember::Members)/sizeof(hkInternalClassMember)),
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
