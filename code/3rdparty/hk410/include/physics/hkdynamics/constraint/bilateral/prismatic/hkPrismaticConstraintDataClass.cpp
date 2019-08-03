/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkdynamics/constraint/bilateral/prismatic/hkPrismaticConstraintData.h'

#include <hkdynamics/hkDynamics.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkdynamics/constraint/bilateral/prismatic/hkPrismaticConstraintData.h>


// External pointer and enum types
extern const hkClass hkAngConstraintAtomClass;
extern const hkClass hkLinConstraintAtomClass;
extern const hkClass hkLinFrictionConstraintAtomClass;
extern const hkClass hkLinLimitConstraintAtomClass;
extern const hkClass hkLinMotorConstraintAtomClass;
extern const hkClass hkPrismaticConstraintDataAtomsClass;
extern const hkClass hkSetLocalTransformsConstraintAtomClass;
extern const hkClass hkSolverResultsClass;

//
// Enum hkPrismaticConstraintData::Atoms::Axis
//
static const hkInternalClassEnumItem hkPrismaticConstraintDataAtomsAxisEnumItems[] =
{
	{0, "AXIS_SHAFT"},
	{1, "AXIS_PERP_TO_SHAFT"},
};
static const hkInternalClassEnum hkPrismaticConstraintDataAtomsEnums[] = {
	{"Axis", hkPrismaticConstraintDataAtomsAxisEnumItems, 2 }
};
extern const hkClassEnum* hkPrismaticConstraintDataAtomsAxisEnum = reinterpret_cast<const hkClassEnum*>(&hkPrismaticConstraintDataAtomsEnums[0]);

//
// Class hkPrismaticConstraintData::Atoms
//
static const hkInternalClassMember hkPrismaticConstraintData_AtomsClass_Members[] =
{
	{ "transforms", &hkSetLocalTransformsConstraintAtomClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkPrismaticConstraintData::Atoms,m_transforms) },
	{ "motor", &hkLinMotorConstraintAtomClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkPrismaticConstraintData::Atoms,m_motor) },
	{ "friction", &hkLinFrictionConstraintAtomClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkPrismaticConstraintData::Atoms,m_friction) },
	{ "ang", &hkAngConstraintAtomClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkPrismaticConstraintData::Atoms,m_ang) },
	{ "lin0", &hkLinConstraintAtomClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkPrismaticConstraintData::Atoms,m_lin0) },
	{ "lin1", &hkLinConstraintAtomClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkPrismaticConstraintData::Atoms,m_lin1) },
	{ "linLimit", &hkLinLimitConstraintAtomClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkPrismaticConstraintData::Atoms,m_linLimit) }
};
const hkClass hkPrismaticConstraintDataAtomsClass(
	"hkPrismaticConstraintDataAtoms",
	HK_NULL, // parent
	sizeof(hkPrismaticConstraintData::Atoms),
	HK_NULL,
	0, // interfaces
	reinterpret_cast<const hkClassEnum*>(hkPrismaticConstraintDataAtomsEnums),
	1, // enums
	reinterpret_cast<const hkClassMember*>(hkPrismaticConstraintData_AtomsClass_Members),
	int(sizeof(hkPrismaticConstraintData_AtomsClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkPrismaticConstraintData
//
static const hkInternalClassMember hkPrismaticConstraintDataClass_Members[] =
{
	{ "atoms", &hkPrismaticConstraintDataAtomsClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkPrismaticConstraintData,m_atoms) }
};
extern const hkClass hkConstraintDataClass;

extern const hkClass hkPrismaticConstraintDataClass;
const hkClass hkPrismaticConstraintDataClass(
	"hkPrismaticConstraintData",
	&hkConstraintDataClass, // parent
	sizeof(hkPrismaticConstraintData),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkPrismaticConstraintDataClass_Members),
	int(sizeof(hkPrismaticConstraintDataClass_Members)/sizeof(hkInternalClassMember)),
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
