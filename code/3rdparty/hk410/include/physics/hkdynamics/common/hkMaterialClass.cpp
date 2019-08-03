/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkdynamics/common/hkMaterial.h'

#include <hkdynamics/hkDynamics.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkdynamics/common/hkMaterial.h>


//
// Enum hkMaterial::ResponseType
//
static const hkInternalClassEnumItem hkMaterialResponseTypeEnumItems[] =
{
	{0, "RESPONSE_INVALID"},
	{1, "RESPONSE_SIMPLE_CONTACT"},
	{2, "RESPONSE_REPORTING"},
	{3, "RESPONSE_NONE"},
	{4, "RESPONSE_MAX_ID"},
};
static const hkInternalClassEnum hkMaterialEnums[] = {
	{"ResponseType", hkMaterialResponseTypeEnumItems, 5 }
};
extern const hkClassEnum* hkMaterialResponseTypeEnum = reinterpret_cast<const hkClassEnum*>(&hkMaterialEnums[0]);

//
// Class hkMaterial
//
const hkInternalClassMember hkMaterial::Members[] =
{
	{ "responseType", HK_NULL, hkMaterialResponseTypeEnum, hkClassMember::TYPE_ENUM, hkClassMember::TYPE_VOID, 0, hkClassMember::ENUM_8, HK_OFFSET_OF(hkMaterial,m_responseType) },
	{ "friction", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkMaterial,m_friction) },
	{ "restitution", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkMaterial,m_restitution) }
};
extern const hkClass hkMaterialClass;
const hkClass hkMaterialClass(
	"hkMaterial",
	HK_NULL, // parent
	sizeof(hkMaterial),
	HK_NULL,
	0, // interfaces
	reinterpret_cast<const hkClassEnum*>(hkMaterialEnums),
	1, // enums
	reinterpret_cast<const hkClassMember*>(hkMaterial::Members),
	int(sizeof(hkMaterial::Members)/sizeof(hkInternalClassMember)),
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
