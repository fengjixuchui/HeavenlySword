/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkinternal/dynamics/constraint/bilateral/ragdoll/hkRagdollConstraintData.h'

#include <hkinternal/hkInternal.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkinternal/dynamics/constraint/bilateral/ragdoll/hkRagdollConstraintData.h>


// External pointer and enum types
extern const hkClass hkAngFrictionConstraintAtomClass;
extern const hkClass hkBallSocketConstraintAtomClass;
extern const hkClass hkConeLimitConstraintAtomClass;
extern const hkClass hkRagdollConstraintDataAtomsClass;
extern const hkClass hkRagdollMotorConstraintAtomClass;
extern const hkClass hkSetLocalTransformsConstraintAtomClass;
extern const hkClass hkSolverResultsClass;
extern const hkClass hkTwistLimitConstraintAtomClass;

//
// Enum hkRagdollConstraintData::MotorIndex
//
static const hkInternalClassEnumItem hkRagdollConstraintDataMotorIndexEnumItems[] =
{
	{0, "MOTOR_TWIST"},
	{1, "MOTOR_PLANE"},
	{2, "MOTOR_CONE"},
};
static const hkInternalClassEnum hkRagdollConstraintDataEnums[] = {
	{"MotorIndex", hkRagdollConstraintDataMotorIndexEnumItems, 3 }
};
extern const hkClassEnum* hkRagdollConstraintDataMotorIndexEnum = reinterpret_cast<const hkClassEnum*>(&hkRagdollConstraintDataEnums[0]);

//
// Enum hkRagdollConstraintData::Atoms::Axis
//
static const hkInternalClassEnumItem hkRagdollConstraintDataAtomsAxisEnumItems[] =
{
	{0, "AXIS_TWIST"},
	{1, "AXIS_PLANES"},
	{2, "AXIS_CROSS_PRODUCT"},
};
static const hkInternalClassEnum hkRagdollConstraintDataAtomsEnums[] = {
	{"Axis", hkRagdollConstraintDataAtomsAxisEnumItems, 3 }
};
extern const hkClassEnum* hkRagdollConstraintDataAtomsAxisEnum = reinterpret_cast<const hkClassEnum*>(&hkRagdollConstraintDataAtomsEnums[0]);

//
// Class hkRagdollConstraintData::Atoms
//
static const hkInternalClassMember hkRagdollConstraintData_AtomsClass_Members[] =
{
	{ "transforms", &hkSetLocalTransformsConstraintAtomClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkRagdollConstraintData::Atoms,m_transforms) },
	{ "ragdollMotors", &hkRagdollMotorConstraintAtomClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkRagdollConstraintData::Atoms,m_ragdollMotors) },
	{ "angFriction", &hkAngFrictionConstraintAtomClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkRagdollConstraintData::Atoms,m_angFriction) },
	{ "twistLimit", &hkTwistLimitConstraintAtomClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkRagdollConstraintData::Atoms,m_twistLimit) },
	{ "coneLimit", &hkConeLimitConstraintAtomClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkRagdollConstraintData::Atoms,m_coneLimit) },
	{ "planesLimit", &hkConeLimitConstraintAtomClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkRagdollConstraintData::Atoms,m_planesLimit) },
	{ "ballSocket", &hkBallSocketConstraintAtomClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkRagdollConstraintData::Atoms,m_ballSocket) }
};
const hkClass hkRagdollConstraintDataAtomsClass(
	"hkRagdollConstraintDataAtoms",
	HK_NULL, // parent
	sizeof(hkRagdollConstraintData::Atoms),
	HK_NULL,
	0, // interfaces
	reinterpret_cast<const hkClassEnum*>(hkRagdollConstraintDataAtomsEnums),
	1, // enums
	reinterpret_cast<const hkClassMember*>(hkRagdollConstraintData_AtomsClass_Members),
	int(sizeof(hkRagdollConstraintData_AtomsClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkRagdollConstraintData
//
static const hkInternalClassMember hkRagdollConstraintDataClass_Members[] =
{
	{ "atoms", &hkRagdollConstraintDataAtomsClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkRagdollConstraintData,m_atoms) }
};
extern const hkClass hkConstraintDataClass;

extern const hkClass hkRagdollConstraintDataClass;
const hkClass hkRagdollConstraintDataClass(
	"hkRagdollConstraintData",
	&hkConstraintDataClass, // parent
	sizeof(hkRagdollConstraintData),
	HK_NULL,
	0, // interfaces
	reinterpret_cast<const hkClassEnum*>(hkRagdollConstraintDataEnums),
	1, // enums
	reinterpret_cast<const hkClassMember*>(hkRagdollConstraintDataClass_Members),
	int(sizeof(hkRagdollConstraintDataClass_Members)/sizeof(hkInternalClassMember)),
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
