/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkvehicle/friction/hkVehicleFriction.h'

#include <hkvehicle/hkVehicle.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkvehicle/friction/hkVehicleFriction.h>


// External pointer and enum types
extern const hkClass hkVehicleFrictionDescriptionAxisDescriptionClass;
extern const hkClass hkVehicleFrictionStatusAxisStatusClass;

//
// Class hkVehicleFrictionDescription::AxisDescription
//
static const hkInternalClassMember hkVehicleFrictionDescription_AxisDescriptionClass_Members[] =
{
	{ "frictionCircleYtab", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 16, 0, HK_OFFSET_OF(hkVehicleFrictionDescription::AxisDescription,m_frictionCircleYtab) },
	{ "xStep", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkVehicleFrictionDescription::AxisDescription,m_xStep) },
	{ "xStart", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkVehicleFrictionDescription::AxisDescription,m_xStart) },
	{ "wheelSurfaceInertia", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkVehicleFrictionDescription::AxisDescription,m_wheelSurfaceInertia) },
	{ "wheelSurfaceInertiaInv", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkVehicleFrictionDescription::AxisDescription,m_wheelSurfaceInertiaInv) },
	{ "wheelChassisMassRatio", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkVehicleFrictionDescription::AxisDescription,m_wheelChassisMassRatio) },
	{ "wheelRadius", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkVehicleFrictionDescription::AxisDescription,m_wheelRadius) },
	{ "wheelRadiusInv", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkVehicleFrictionDescription::AxisDescription,m_wheelRadiusInv) },
	{ "wheelDownForceFactor", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkVehicleFrictionDescription::AxisDescription,m_wheelDownForceFactor) },
	{ "wheelDownForceSumFactor", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkVehicleFrictionDescription::AxisDescription,m_wheelDownForceSumFactor) }
};
const hkClass hkVehicleFrictionDescriptionAxisDescriptionClass(
	"hkVehicleFrictionDescriptionAxisDescription",
	HK_NULL, // parent
	sizeof(hkVehicleFrictionDescription::AxisDescription),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkVehicleFrictionDescription_AxisDescriptionClass_Members),
	int(sizeof(hkVehicleFrictionDescription_AxisDescriptionClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkVehicleFrictionDescription
//
static const hkInternalClassMember hkVehicleFrictionDescriptionClass_Members[] =
{
	{ "wheelDistance", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkVehicleFrictionDescription,m_wheelDistance) },
	{ "chassisMassInv", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkVehicleFrictionDescription,m_chassisMassInv) },
	{ "axleDescr", &hkVehicleFrictionDescriptionAxisDescriptionClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 2, 0, HK_OFFSET_OF(hkVehicleFrictionDescription,m_axleDescr) }
};
extern const hkClass hkVehicleFrictionDescriptionClass;
const hkClass hkVehicleFrictionDescriptionClass(
	"hkVehicleFrictionDescription",
	HK_NULL, // parent
	sizeof(hkVehicleFrictionDescription),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkVehicleFrictionDescriptionClass_Members),
	int(sizeof(hkVehicleFrictionDescriptionClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkVehicleFrictionStatus::AxisStatus
//
static const hkInternalClassMember hkVehicleFrictionStatus_AxisStatusClass_Members[] =
{
	{ "forward_slip_velocity", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkVehicleFrictionStatus::AxisStatus,m_forward_slip_velocity) },
	{ "side_slip_velocity", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkVehicleFrictionStatus::AxisStatus,m_side_slip_velocity) },
	{ "skid_energy_density", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkVehicleFrictionStatus::AxisStatus,m_skid_energy_density) },
	{ "side_force", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkVehicleFrictionStatus::AxisStatus,m_side_force) },
	{ "delayed_forward_impulse", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkVehicleFrictionStatus::AxisStatus,m_delayed_forward_impulse) },
	{ "sideRhs", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkVehicleFrictionStatus::AxisStatus,m_sideRhs) },
	{ "forwardRhs", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkVehicleFrictionStatus::AxisStatus,m_forwardRhs) },
	{ "relativeSideForce", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkVehicleFrictionStatus::AxisStatus,m_relativeSideForce) },
	{ "relativeForwardForce", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkVehicleFrictionStatus::AxisStatus,m_relativeForwardForce) }
};
const hkClass hkVehicleFrictionStatusAxisStatusClass(
	"hkVehicleFrictionStatusAxisStatus",
	HK_NULL, // parent
	sizeof(hkVehicleFrictionStatus::AxisStatus),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkVehicleFrictionStatus_AxisStatusClass_Members),
	int(sizeof(hkVehicleFrictionStatus_AxisStatusClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkVehicleFrictionStatus
//
static const hkInternalClassMember hkVehicleFrictionStatusClass_Members[] =
{
	{ "axis", &hkVehicleFrictionStatusAxisStatusClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 2, 0, HK_OFFSET_OF(hkVehicleFrictionStatus,m_axis) }
};
extern const hkClass hkVehicleFrictionStatusClass;
const hkClass hkVehicleFrictionStatusClass(
	"hkVehicleFrictionStatus",
	HK_NULL, // parent
	sizeof(hkVehicleFrictionStatus),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkVehicleFrictionStatusClass_Members),
	int(sizeof(hkVehicleFrictionStatusClass_Members)/sizeof(hkInternalClassMember)),
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
