/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkdynamics/constraint/bilateral/hinge/hkHingeConstraintData.h'

#include <hkdynamics/hkDynamics.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkdynamics/constraint/bilateral/hinge/hkHingeConstraintData.h>


// External pointer and enum types
extern const hkClass hk2dAngConstraintAtomClass;
extern const hkClass hkBallSocketConstraintAtomClass;
extern const hkClass hkHingeConstraintDataAtomsClass;
extern const hkClass hkSetLocalTransformsConstraintAtomClass;
extern const hkClass hkSolverResultsClass;

//
// Enum hkHingeConstraintData::Atoms::Axis
//
static const hkInternalClassEnumItem hkHingeConstraintDataAtomsAxisEnumItems[] =
{
	{0, "AXIS_AXLE"},
};
static const hkInternalClassEnum hkHingeConstraintDataAtomsEnums[] = {
	{"Axis", hkHingeConstraintDataAtomsAxisEnumItems, 1 }
};
extern const hkClassEnum* hkHingeConstraintDataAtomsAxisEnum = reinterpret_cast<const hkClassEnum*>(&hkHingeConstraintDataAtomsEnums[0]);

//
// Class hkHingeConstraintData::Atoms
//
static const hkInternalClassMember hkHingeConstraintData_AtomsClass_Members[] =
{
	{ "transforms", &hkSetLocalTransformsConstraintAtomClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkHingeConstraintData::Atoms,m_transforms) },
	{ "2dAng", &hk2dAngConstraintAtomClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkHingeConstraintData::Atoms,m_2dAng) },
	{ "ballSocket", &hkBallSocketConstraintAtomClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkHingeConstraintData::Atoms,m_ballSocket) }
};
const hkClass hkHingeConstraintDataAtomsClass(
	"hkHingeConstraintDataAtoms",
	HK_NULL, // parent
	sizeof(hkHingeConstraintData::Atoms),
	HK_NULL,
	0, // interfaces
	reinterpret_cast<const hkClassEnum*>(hkHingeConstraintDataAtomsEnums),
	1, // enums
	reinterpret_cast<const hkClassMember*>(hkHingeConstraintData_AtomsClass_Members),
	int(sizeof(hkHingeConstraintData_AtomsClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkHingeConstraintData
//
static const hkInternalClassMember hkHingeConstraintDataClass_Members[] =
{
	{ "atoms", &hkHingeConstraintDataAtomsClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkHingeConstraintData,m_atoms) }
};
extern const hkClass hkConstraintDataClass;

extern const hkClass hkHingeConstraintDataClass;
const hkClass hkHingeConstraintDataClass(
	"hkHingeConstraintData",
	&hkConstraintDataClass, // parent
	sizeof(hkHingeConstraintData),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkHingeConstraintDataClass_Members),
	int(sizeof(hkHingeConstraintDataClass_Members)/sizeof(hkInternalClassMember)),
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
