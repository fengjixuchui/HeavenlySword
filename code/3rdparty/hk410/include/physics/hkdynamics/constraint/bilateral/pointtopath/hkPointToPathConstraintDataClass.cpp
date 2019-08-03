/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkdynamics/constraint/bilateral/pointtopath/hkPointToPathConstraintData.h'

#include <hkdynamics/hkDynamics.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkdynamics/constraint/bilateral/pointtopath/hkPointToPathConstraintData.h>


// External pointer and enum types
extern const hkClass hkBridgeAtomsClass;
extern const hkClass hkParametricCurveClass;
extern const hkClass hkSolverResultsClass;

//
// Enum hkPointToPathConstraintData::OrientationConstraintType
//
static const hkInternalClassEnumItem hkPointToPathConstraintDataOrientationConstraintTypeEnumItems[] =
{
	{0, "CONSTRAIN_ORIENTATION_INVALID"},
	{1, "CONSTRAIN_ORIENTATION_NONE"},
	{2, "CONSTRAIN_ORIENTATION_ALLOW_SPIN"},
	{3, "CONSTRAIN_ORIENTATION_TO_PATH"},
	{4, "CONSTRAIN_ORIENTATION_MAX_ID"},
};
static const hkInternalClassEnum hkPointToPathConstraintDataEnums[] = {
	{"OrientationConstraintType", hkPointToPathConstraintDataOrientationConstraintTypeEnumItems, 5 }
};
extern const hkClassEnum* hkPointToPathConstraintDataOrientationConstraintTypeEnum = reinterpret_cast<const hkClassEnum*>(&hkPointToPathConstraintDataEnums[0]);

//
// Class hkPointToPathConstraintData
//
const hkInternalClassMember hkPointToPathConstraintData::Members[] =
{
	{ "atoms", &hkBridgeAtomsClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkPointToPathConstraintData,m_atoms) },
	{ "path", &hkParametricCurveClass, HK_NULL, hkClassMember::TYPE_POINTER, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkPointToPathConstraintData,m_path) },
	{ "maxFrictionForce", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkPointToPathConstraintData,m_maxFrictionForce) },
	{ "angularConstrainedDOF", HK_NULL, hkPointToPathConstraintDataOrientationConstraintTypeEnum, hkClassMember::TYPE_ENUM, hkClassMember::TYPE_VOID, 0, hkClassMember::ENUM_8, HK_OFFSET_OF(hkPointToPathConstraintData,m_angularConstrainedDOF) },
	{ "transform_OS_KS", HK_NULL, HK_NULL, hkClassMember::TYPE_TRANSFORM, hkClassMember::TYPE_VOID, 2, 0, HK_OFFSET_OF(hkPointToPathConstraintData,m_transform_OS_KS) }
};
extern const hkClass hkConstraintDataClass;

extern const hkClass hkPointToPathConstraintDataClass;
const hkClass hkPointToPathConstraintDataClass(
	"hkPointToPathConstraintData",
	&hkConstraintDataClass, // parent
	sizeof(hkPointToPathConstraintData),
	HK_NULL,
	0, // interfaces
	reinterpret_cast<const hkClassEnum*>(hkPointToPathConstraintDataEnums),
	1, // enums
	reinterpret_cast<const hkClassMember*>(hkPointToPathConstraintData::Members),
	int(sizeof(hkPointToPathConstraintData::Members)/sizeof(hkInternalClassMember)),
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
