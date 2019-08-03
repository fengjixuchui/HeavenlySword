/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkvehicle/tyremarks/hkTyremarksInfo.h'

#include <hkvehicle/hkVehicle.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkvehicle/tyremarks/hkTyremarksInfo.h>


// External pointer and enum types
extern const hkClass hkTyremarkPointClass;
extern const hkClass hkTyremarksWheelClass;

//
// Class hkTyremarkPoint
//
static const hkInternalClassMember hkTyremarkPointClass_Members[] =
{
	{ "pointLeft", HK_NULL, HK_NULL, hkClassMember::TYPE_VECTOR4, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkTyremarkPoint,m_pointLeft) },
	{ "pointRight", HK_NULL, HK_NULL, hkClassMember::TYPE_VECTOR4, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkTyremarkPoint,m_pointRight) }
};
const hkClass hkTyremarkPointClass(
	"hkTyremarkPoint",
	HK_NULL, // parent
	sizeof(hkTyremarkPoint),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkTyremarkPointClass_Members),
	int(sizeof(hkTyremarkPointClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkTyremarksWheel
//
static const hkInternalClassMember hkTyremarksWheelClass_Members[] =
{
	{ "currentPosition", HK_NULL, HK_NULL, hkClassMember::TYPE_INT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkTyremarksWheel,m_currentPosition) },
	{ "numPoints", HK_NULL, HK_NULL, hkClassMember::TYPE_INT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkTyremarksWheel,m_numPoints) },
	{ "tyremarkPoints", &hkTyremarkPointClass, HK_NULL, hkClassMember::TYPE_ARRAY, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkTyremarksWheel,m_tyremarkPoints) }
};
extern const hkClass hkReferencedObjectClass;

const hkClass hkTyremarksWheelClass(
	"hkTyremarksWheel",
	&hkReferencedObjectClass, // parent
	sizeof(hkTyremarksWheel),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkTyremarksWheelClass_Members),
	int(sizeof(hkTyremarksWheelClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkTyremarksInfo
//
static const hkInternalClassMember hkTyremarksInfoClass_Members[] =
{
	{ "minTyremarkEnergy", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkTyremarksInfo,m_minTyremarkEnergy) },
	{ "maxTyremarkEnergy", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkTyremarksInfo,m_maxTyremarkEnergy) },
	{ "tyremarksWheel", &hkTyremarksWheelClass, HK_NULL, hkClassMember::TYPE_ARRAY, hkClassMember::TYPE_POINTER, 0, 0, HK_OFFSET_OF(hkTyremarksInfo,m_tyremarksWheel) }
};

extern const hkClass hkTyremarksInfoClass;
const hkClass hkTyremarksInfoClass(
	"hkTyremarksInfo",
	&hkReferencedObjectClass, // parent
	sizeof(hkTyremarksInfo),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkTyremarksInfoClass_Members),
	int(sizeof(hkTyremarksInfoClass_Members)/sizeof(hkInternalClassMember)),
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
