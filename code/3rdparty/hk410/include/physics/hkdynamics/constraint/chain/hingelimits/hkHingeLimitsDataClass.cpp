/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkdynamics/constraint/chain/hingelimits/hkHingeLimitsData.h'

#include <hkdynamics/hkDynamics.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkdynamics/constraint/chain/hingelimits/hkHingeLimitsData.h>


// External pointer and enum types
extern const hkClass hk2dAngConstraintAtomClass;
extern const hkClass hkAngLimitConstraintAtomClass;
extern const hkClass hkHingeLimitsDataAtomsClass;
extern const hkClass hkSetLocalRotationsConstraintAtomClass;
extern const hkClass hkSolverResultsClass;

//
// Enum hkHingeLimitsData::Atoms::Axis
//
static const hkInternalClassEnumItem hkHingeLimitsDataAtomsAxisEnumItems[] =
{
	{0, "AXIS_AXLE"},
	{1, "AXIS_PERP_TO_AXLE_1"},
	{2, "AXIS_PERP_TO_AXLE_2"},
};
static const hkInternalClassEnum hkHingeLimitsDataAtomsEnums[] = {
	{"Axis", hkHingeLimitsDataAtomsAxisEnumItems, 3 }
};
extern const hkClassEnum* hkHingeLimitsDataAtomsAxisEnum = reinterpret_cast<const hkClassEnum*>(&hkHingeLimitsDataAtomsEnums[0]);

//
// Class hkHingeLimitsData::Atoms
//
static const hkInternalClassMember hkHingeLimitsData_AtomsClass_Members[] =
{
	{ "rotations", &hkSetLocalRotationsConstraintAtomClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkHingeLimitsData::Atoms,m_rotations) },
	{ "angLimit", &hkAngLimitConstraintAtomClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkHingeLimitsData::Atoms,m_angLimit) },
	{ "2dAng", &hk2dAngConstraintAtomClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkHingeLimitsData::Atoms,m_2dAng) }
};
const hkClass hkHingeLimitsDataAtomsClass(
	"hkHingeLimitsDataAtoms",
	HK_NULL, // parent
	sizeof(hkHingeLimitsData::Atoms),
	HK_NULL,
	0, // interfaces
	reinterpret_cast<const hkClassEnum*>(hkHingeLimitsDataAtomsEnums),
	1, // enums
	reinterpret_cast<const hkClassMember*>(hkHingeLimitsData_AtomsClass_Members),
	int(sizeof(hkHingeLimitsData_AtomsClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkHingeLimitsData
//
static const hkInternalClassMember hkHingeLimitsDataClass_Members[] =
{
	{ "atoms", &hkHingeLimitsDataAtomsClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkHingeLimitsData,m_atoms) }
};
extern const hkClass hkConstraintDataClass;

extern const hkClass hkHingeLimitsDataClass;
const hkClass hkHingeLimitsDataClass(
	"hkHingeLimitsData",
	&hkConstraintDataClass, // parent
	sizeof(hkHingeLimitsData),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkHingeLimitsDataClass_Members),
	int(sizeof(hkHingeLimitsDataClass_Members)/sizeof(hkInternalClassMember)),
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
