/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkdynamics/constraint/hkConstraintInstance.h'

#include <hkdynamics/hkDynamics.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkdynamics/constraint/hkConstraintInstance.h>


// External pointer and enum types
extern const hkClass hkConstraintAtomClass;
extern const hkClass hkConstraintDataClass;
extern const hkClass hkConstraintInstanceClass;
extern const hkClass hkConstraintInternalClass;
extern const hkClass hkConstraintOwnerClass;
extern const hkClass hkConstraintRuntimeClass;
extern const hkClass hkEntityClass;
extern const hkClass hkModifierConstraintAtomClass;
extern const hkClassEnum* hkConstraintInstanceConstraintPriorityEnum;

//
// Enum hkConstraintInstance::ConstraintPriority
//
static const hkInternalClassEnumItem hkConstraintInstanceConstraintPriorityEnumItems[] =
{
	{0, "PRIORITY_INVALID"},
	{1, "PRIORITY_PSI"},
	{2, "PRIORITY_TOI"},
	{3, "PRIORITY_TOI_HIGHER"},
	{4, "PRIORITY_TOI_FORCED"},
};

//
// Enum hkConstraintInstance::InstanceType
//
static const hkInternalClassEnumItem hkConstraintInstanceInstanceTypeEnumItems[] =
{
	{0, "TYPE_NORMAL"},
	{1, "TYPE_CHAIN"},
};

//
// Enum hkConstraintInstance::AddReferences
//
static const hkInternalClassEnumItem hkConstraintInstanceAddReferencesEnumItems[] =
{
	{0, "DO_NOT_ADD_REFERENCES"},
	{1, "DO_ADD_REFERENCES"},
};
static const hkInternalClassEnum hkConstraintInstanceEnums[] = {
	{"ConstraintPriority", hkConstraintInstanceConstraintPriorityEnumItems, 5 },
	{"InstanceType", hkConstraintInstanceInstanceTypeEnumItems, 2 },
	{"AddReferences", hkConstraintInstanceAddReferencesEnumItems, 2 }
};
extern const hkClassEnum* hkConstraintInstanceConstraintPriorityEnum = reinterpret_cast<const hkClassEnum*>(&hkConstraintInstanceEnums[0]);
extern const hkClassEnum* hkConstraintInstanceInstanceTypeEnum = reinterpret_cast<const hkClassEnum*>(&hkConstraintInstanceEnums[1]);
extern const hkClassEnum* hkConstraintInstanceAddReferencesEnum = reinterpret_cast<const hkClassEnum*>(&hkConstraintInstanceEnums[2]);

//
// Class hkConstraintInstance
//
const hkInternalClassMember hkConstraintInstance::Members[] =
{
	{ "owner", HK_NULL, HK_NULL, hkClassMember::TYPE_ZERO, hkClassMember::TYPE_POINTER, 0, 0, HK_OFFSET_OF(hkConstraintInstance,m_owner) },
	{ "data", &hkConstraintDataClass, HK_NULL, hkClassMember::TYPE_POINTER, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkConstraintInstance,m_data) },
	{ "constraintModifiers", &hkModifierConstraintAtomClass, HK_NULL, hkClassMember::TYPE_POINTER, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkConstraintInstance,m_constraintModifiers) },
	{ "entities", &hkEntityClass, HK_NULL, hkClassMember::TYPE_POINTER, hkClassMember::TYPE_STRUCT, 2, 0, HK_OFFSET_OF(hkConstraintInstance,m_entities) },
	{ "priority", HK_NULL, hkConstraintInstanceConstraintPriorityEnum, hkClassMember::TYPE_ENUM, hkClassMember::TYPE_VOID, 0, hkClassMember::ENUM_8, HK_OFFSET_OF(hkConstraintInstance,m_priority) },
	{ "wantRuntime", HK_NULL, HK_NULL, hkClassMember::TYPE_BOOL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkConstraintInstance,m_wantRuntime) },
	{ "name", HK_NULL, HK_NULL, hkClassMember::TYPE_CSTRING, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkConstraintInstance,m_name) },
	{ "userData", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkConstraintInstance,m_userData) },
	{ "internal", HK_NULL, HK_NULL, hkClassMember::TYPE_ZERO, hkClassMember::TYPE_POINTER, 0, 0, HK_OFFSET_OF(hkConstraintInstance,m_internal) }
};
extern const hkClass hkReferencedObjectClass;

const hkClass hkConstraintInstanceClass(
	"hkConstraintInstance",
	&hkReferencedObjectClass, // parent
	sizeof(hkConstraintInstance),
	HK_NULL,
	0, // interfaces
	reinterpret_cast<const hkClassEnum*>(hkConstraintInstanceEnums),
	3, // enums
	reinterpret_cast<const hkClassMember*>(hkConstraintInstance::Members),
	int(sizeof(hkConstraintInstance::Members)/sizeof(hkInternalClassMember)),
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
