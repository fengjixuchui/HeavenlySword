/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkdynamics/constraint/hkConstraintData.h'

#include <hkdynamics/hkDynamics.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkdynamics/constraint/hkConstraintData.h>


// External pointer and enum types
extern const hkClass hkConstraintAtomClass;

//
// Enum hkConstraintData::ConstraintType
//
static const hkInternalClassEnumItem hkConstraintDataConstraintTypeEnumItems[] =
{
	{0, "CONSTRAINT_TYPE_BALLANDSOCKET"},
	{1, "CONSTRAINT_TYPE_HINGE"},
	{2, "CONSTRAINT_TYPE_LIMITEDHINGE"},
	{3, "CONSTRAINT_TYPE_POINTTOPATH"},
	{6, "CONSTRAINT_TYPE_PRISMATIC"},
	{7, "CONSTRAINT_TYPE_RAGDOLL"},
	{8, "CONSTRAINT_TYPE_STIFFSPRING"},
	{9, "CONSTRAINT_TYPE_WHEEL"},
	{10, "CONSTRAINT_TYPE_GENERIC"},
	{11, "CONSTRAINT_TYPE_CONTACT"},
	{12, "CONSTRAINT_TYPE_BREAKABLE"},
	{13, "CONSTRAINT_TYPE_MALLEABLE"},
	{14, "CONSTRAINT_TYPE_POINTTOPLANE"},
	{15, "CONSTRAINT_TYPE_PULLEY"},
	{18, "CONSTRAINT_TYPE_HINGE_LIMITS"},
	{19, "CONSTRAINT_TYPE_RAGDOLL_LIMITS"},
	{100, "BEGIN_CONSTRAINT_CHAIN_TYPES"},
	{100, "CONSTRAINT_TYPE_STIFF_SPRING_CHAIN"},
	{101, "CONSTRAINT_TYPE_BALL_SOCKET_CHAIN"},
	{102, "CONSTRAINT_TYPE_POWERED_CHAIN"},
};
static const hkInternalClassEnum hkConstraintDataEnums[] = {
	{"ConstraintType", hkConstraintDataConstraintTypeEnumItems, 20 }
};
extern const hkClassEnum* hkConstraintDataConstraintTypeEnum = reinterpret_cast<const hkClassEnum*>(&hkConstraintDataEnums[0]);

//
// Class hkConstraintData
//
static const hkInternalClassMember hkConstraintDataClass_Members[] =
{
	{ "userData", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkConstraintData,m_userData) }
};
extern const hkClass hkReferencedObjectClass;

extern const hkClass hkConstraintDataClass;
const hkClass hkConstraintDataClass(
	"hkConstraintData",
	&hkReferencedObjectClass, // parent
	sizeof(hkConstraintData),
	HK_NULL,
	0, // interfaces
	reinterpret_cast<const hkClassEnum*>(hkConstraintDataEnums),
	1, // enums
	reinterpret_cast<const hkClassMember*>(hkConstraintDataClass_Members),
	int(sizeof(hkConstraintDataClass_Members)/sizeof(hkInternalClassMember)),
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
