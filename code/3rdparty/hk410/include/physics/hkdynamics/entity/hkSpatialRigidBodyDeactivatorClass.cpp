/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkdynamics/entity/hkSpatialRigidBodyDeactivator.h'

#include <hkdynamics/hkDynamics.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkdynamics/entity/hkSpatialRigidBodyDeactivator.h>


// External pointer and enum types
extern const hkClass hkSpatialRigidBodyDeactivatorSampleClass;

//
// Class hkSpatialRigidBodyDeactivator::Sample
//
static const hkInternalClassMember hkSpatialRigidBodyDeactivator_SampleClass_Members[] =
{
	{ "refPosition", HK_NULL, HK_NULL, hkClassMember::TYPE_VECTOR4, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkSpatialRigidBodyDeactivator::Sample,m_refPosition) },
	{ "refRotation", HK_NULL, HK_NULL, hkClassMember::TYPE_QUATERNION, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkSpatialRigidBodyDeactivator::Sample,m_refRotation) }
};
const hkClass hkSpatialRigidBodyDeactivatorSampleClass(
	"hkSpatialRigidBodyDeactivatorSample",
	HK_NULL, // parent
	sizeof(hkSpatialRigidBodyDeactivator::Sample),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkSpatialRigidBodyDeactivator_SampleClass_Members),
	int(sizeof(hkSpatialRigidBodyDeactivator_SampleClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkSpatialRigidBodyDeactivator
//
const hkInternalClassMember hkSpatialRigidBodyDeactivator::Members[] =
{
	{ "highFrequencySample", &hkSpatialRigidBodyDeactivatorSampleClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkSpatialRigidBodyDeactivator,m_highFrequencySample) },
	{ "lowFrequencySample", &hkSpatialRigidBodyDeactivatorSampleClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkSpatialRigidBodyDeactivator,m_lowFrequencySample) },
	{ "radiusSqrd", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkSpatialRigidBodyDeactivator,m_radiusSqrd) },
	{ "minHighFrequencyTranslation", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkSpatialRigidBodyDeactivator,m_minHighFrequencyTranslation) },
	{ "minHighFrequencyRotation", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkSpatialRigidBodyDeactivator,m_minHighFrequencyRotation) },
	{ "minLowFrequencyTranslation", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkSpatialRigidBodyDeactivator,m_minLowFrequencyTranslation) },
	{ "minLowFrequencyRotation", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkSpatialRigidBodyDeactivator,m_minLowFrequencyRotation) }
};
extern const hkClass hkRigidBodyDeactivatorClass;

extern const hkClass hkSpatialRigidBodyDeactivatorClass;
const hkClass hkSpatialRigidBodyDeactivatorClass(
	"hkSpatialRigidBodyDeactivator",
	&hkRigidBodyDeactivatorClass, // parent
	sizeof(hkSpatialRigidBodyDeactivator),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkSpatialRigidBodyDeactivator::Members),
	int(sizeof(hkSpatialRigidBodyDeactivator::Members)/sizeof(hkInternalClassMember)),
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
