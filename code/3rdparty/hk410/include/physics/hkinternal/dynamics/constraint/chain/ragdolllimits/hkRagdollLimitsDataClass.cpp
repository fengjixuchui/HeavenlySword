/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkinternal/dynamics/constraint/chain/ragdolllimits/hkRagdollLimitsData.h'

#include <hkinternal/hkInternal.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkinternal/dynamics/constraint/chain/ragdolllimits/hkRagdollLimitsData.h>


// External pointer and enum types
extern const hkClass hkConeLimitConstraintAtomClass;
extern const hkClass hkRagdollLimitsDataAtomsClass;
extern const hkClass hkSetLocalRotationsConstraintAtomClass;
extern const hkClass hkSolverResultsClass;
extern const hkClass hkTwistLimitConstraintAtomClass;

//
// Enum hkRagdollLimitsData::Atoms::Axis
//
static const hkInternalClassEnumItem hkRagdollLimitsDataAtomsAxisEnumItems[] =
{
	{0, "AXIS_TWIST"},
	{1, "AXIS_PLANES"},
	{2, "AXIS_CROSS_PRODUCT"},
};
static const hkInternalClassEnum hkRagdollLimitsDataAtomsEnums[] = {
	{"Axis", hkRagdollLimitsDataAtomsAxisEnumItems, 3 }
};
extern const hkClassEnum* hkRagdollLimitsDataAtomsAxisEnum = reinterpret_cast<const hkClassEnum*>(&hkRagdollLimitsDataAtomsEnums[0]);

//
// Class hkRagdollLimitsData::Atoms
//
static const hkInternalClassMember hkRagdollLimitsData_AtomsClass_Members[] =
{
	{ "rotations", &hkSetLocalRotationsConstraintAtomClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkRagdollLimitsData::Atoms,m_rotations) },
	{ "twistLimit", &hkTwistLimitConstraintAtomClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkRagdollLimitsData::Atoms,m_twistLimit) },
	{ "coneLimit", &hkConeLimitConstraintAtomClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkRagdollLimitsData::Atoms,m_coneLimit) },
	{ "planesLimit", &hkConeLimitConstraintAtomClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkRagdollLimitsData::Atoms,m_planesLimit) }
};
const hkClass hkRagdollLimitsDataAtomsClass(
	"hkRagdollLimitsDataAtoms",
	HK_NULL, // parent
	sizeof(hkRagdollLimitsData::Atoms),
	HK_NULL,
	0, // interfaces
	reinterpret_cast<const hkClassEnum*>(hkRagdollLimitsDataAtomsEnums),
	1, // enums
	reinterpret_cast<const hkClassMember*>(hkRagdollLimitsData_AtomsClass_Members),
	int(sizeof(hkRagdollLimitsData_AtomsClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkRagdollLimitsData
//
static const hkInternalClassMember hkRagdollLimitsDataClass_Members[] =
{
	{ "atoms", &hkRagdollLimitsDataAtomsClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkRagdollLimitsData,m_atoms) }
};
extern const hkClass hkConstraintDataClass;

extern const hkClass hkRagdollLimitsDataClass;
const hkClass hkRagdollLimitsDataClass(
	"hkRagdollLimitsData",
	&hkConstraintDataClass, // parent
	sizeof(hkRagdollLimitsData),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkRagdollLimitsDataClass_Members),
	int(sizeof(hkRagdollLimitsDataClass_Members)/sizeof(hkInternalClassMember)),
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
