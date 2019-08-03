/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkdynamics/motion/hkMotion.h'

#include <hkdynamics/hkDynamics.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkdynamics/motion/hkMotion.h>


// External pointer and enum types
extern const hkClass hkMotionStateClass;

//
// Enum hkMotion::MotionType
//
static const hkInternalClassEnumItem hkMotionMotionTypeEnumItems[] =
{
	{0, "MOTION_INVALID"},
	{1, "MOTION_DYNAMIC"},
	{2, "MOTION_SPHERE_INERTIA"},
	{3, "MOTION_STABILIZED_SPHERE_INERTIA"},
	{4, "MOTION_BOX_INERTIA"},
	{5, "MOTION_STABILIZED_BOX_INERTIA"},
	{6, "MOTION_KEYFRAMED"},
	{7, "MOTION_FIXED"},
	{8, "MOTION_THIN_BOX_INERTIA"},
	{9, "MOTION_MAX_ID"},
};
static const hkInternalClassEnum hkMotionEnums[] = {
	{"MotionType", hkMotionMotionTypeEnumItems, 10 }
};
extern const hkClassEnum* hkMotionMotionTypeEnum = reinterpret_cast<const hkClassEnum*>(&hkMotionEnums[0]);

//
// Class hkMotion
//
static const hkInternalClassMember hkMotionClass_Members[] =
{
	{ "type", HK_NULL, hkMotionMotionTypeEnum, hkClassMember::TYPE_ENUM, hkClassMember::TYPE_VOID, 0, hkClassMember::ENUM_8, HK_OFFSET_OF(hkMotion,m_type) },
	{ "deactivationIntegrateCounter", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT8, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkMotion,m_deactivationIntegrateCounter) },
	{ "deactivationNumInactiveFrames", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT8, hkClassMember::TYPE_VOID, 2, 0, HK_OFFSET_OF(hkMotion,m_deactivationNumInactiveFrames) },
	{ "motionState", &hkMotionStateClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkMotion,m_motionState) },
	{ "inertiaAndMassInv", HK_NULL, HK_NULL, hkClassMember::TYPE_VECTOR4, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkMotion,m_inertiaAndMassInv) },
	{ "linearVelocity", HK_NULL, HK_NULL, hkClassMember::TYPE_VECTOR4, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkMotion,m_linearVelocity) },
	{ "angularVelocity", HK_NULL, HK_NULL, hkClassMember::TYPE_VECTOR4, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkMotion,m_angularVelocity) },
	{ "deactivationRefPosition", HK_NULL, HK_NULL, hkClassMember::TYPE_VECTOR4, hkClassMember::TYPE_VOID, 2, 0, HK_OFFSET_OF(hkMotion,m_deactivationRefPosition) }
};
extern const hkClass hkReferencedObjectClass;

extern const hkClass hkMotionClass;
const hkClass hkMotionClass(
	"hkMotion",
	&hkReferencedObjectClass, // parent
	sizeof(hkMotion),
	HK_NULL,
	0, // interfaces
	reinterpret_cast<const hkClassEnum*>(hkMotionEnums),
	1, // enums
	reinterpret_cast<const hkClassMember*>(hkMotionClass_Members),
	int(sizeof(hkMotionClass_Members)/sizeof(hkInternalClassMember)),
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
