/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkdynamics/constraint/bilateral/wheel/hkWheelConstraintData.h'

#include <hkdynamics/hkDynamics.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkdynamics/constraint/bilateral/wheel/hkWheelConstraintData.h>


// External pointer and enum types
extern const hkClass hk2dAngConstraintAtomClass;
extern const hkClass hkLinConstraintAtomClass;
extern const hkClass hkLinLimitConstraintAtomClass;
extern const hkClass hkLinSoftConstraintAtomClass;
extern const hkClass hkSetLocalRotationsConstraintAtomClass;
extern const hkClass hkSetLocalTransformsConstraintAtomClass;
extern const hkClass hkSolverResultsClass;
extern const hkClass hkWheelConstraintDataAtomsClass;

//
// Enum hkWheelConstraintData::Atoms::Axis
//
static const hkInternalClassEnumItem hkWheelConstraintDataAtomsAxisEnumItems[] =
{
	{0, "AXIS_SUSPENSION"},
	{1, "AXIS_PERP_SUSPENSION"},
	{0, "AXIS_AXLE"},
	{1, "AXIS_STEERING"},
};
static const hkInternalClassEnum hkWheelConstraintDataAtomsEnums[] = {
	{"Axis", hkWheelConstraintDataAtomsAxisEnumItems, 4 }
};
extern const hkClassEnum* hkWheelConstraintDataAtomsAxisEnum = reinterpret_cast<const hkClassEnum*>(&hkWheelConstraintDataAtomsEnums[0]);

//
// Class hkWheelConstraintData::Atoms
//
static const hkInternalClassMember hkWheelConstraintData_AtomsClass_Members[] =
{
	{ "suspensionBase", &hkSetLocalTransformsConstraintAtomClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWheelConstraintData::Atoms,m_suspensionBase) },
	{ "lin0Limit", &hkLinLimitConstraintAtomClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWheelConstraintData::Atoms,m_lin0Limit) },
	{ "lin0Soft", &hkLinSoftConstraintAtomClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWheelConstraintData::Atoms,m_lin0Soft) },
	{ "lin1", &hkLinConstraintAtomClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWheelConstraintData::Atoms,m_lin1) },
	{ "lin2", &hkLinConstraintAtomClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWheelConstraintData::Atoms,m_lin2) },
	{ "steeringBase", &hkSetLocalRotationsConstraintAtomClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWheelConstraintData::Atoms,m_steeringBase) },
	{ "2dAng", &hk2dAngConstraintAtomClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWheelConstraintData::Atoms,m_2dAng) }
};
const hkClass hkWheelConstraintDataAtomsClass(
	"hkWheelConstraintDataAtoms",
	HK_NULL, // parent
	sizeof(hkWheelConstraintData::Atoms),
	HK_NULL,
	0, // interfaces
	reinterpret_cast<const hkClassEnum*>(hkWheelConstraintDataAtomsEnums),
	1, // enums
	reinterpret_cast<const hkClassMember*>(hkWheelConstraintData_AtomsClass_Members),
	int(sizeof(hkWheelConstraintData_AtomsClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkWheelConstraintData
//
static const hkInternalClassMember hkWheelConstraintDataClass_Members[] =
{
	{ "atoms", &hkWheelConstraintDataAtomsClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWheelConstraintData,m_atoms) },
	{ "initialAxleInB", HK_NULL, HK_NULL, hkClassMember::TYPE_VECTOR4, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWheelConstraintData,m_initialAxleInB) },
	{ "initialSteeringAxisInB", HK_NULL, HK_NULL, hkClassMember::TYPE_VECTOR4, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWheelConstraintData,m_initialSteeringAxisInB) }
};
extern const hkClass hkConstraintDataClass;

extern const hkClass hkWheelConstraintDataClass;
const hkClass hkWheelConstraintDataClass(
	"hkWheelConstraintData",
	&hkConstraintDataClass, // parent
	sizeof(hkWheelConstraintData),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkWheelConstraintDataClass_Members),
	int(sizeof(hkWheelConstraintDataClass_Members)/sizeof(hkInternalClassMember)),
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
