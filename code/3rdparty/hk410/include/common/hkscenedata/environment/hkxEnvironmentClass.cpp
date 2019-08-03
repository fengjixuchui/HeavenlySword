/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkscenedata/environment/hkxEnvironment.h'

#include <hkscenedata/hkSceneData.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkscenedata/environment/hkxEnvironment.h>


// External pointer and enum types
extern const hkClass hkxEnvironmentVariableClass;

//
// Class hkxEnvironment::Variable
//
static const hkInternalClassMember hkxEnvironment_VariableClass_Members[] =
{
	{ "name", HK_NULL, HK_NULL, hkClassMember::TYPE_CSTRING, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkxEnvironment::Variable,m_name) },
	{ "value", HK_NULL, HK_NULL, hkClassMember::TYPE_CSTRING, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkxEnvironment::Variable,m_value) }
};
const hkClass hkxEnvironmentVariableClass(
	"hkxEnvironmentVariable",
	HK_NULL, // parent
	sizeof(hkxEnvironment::Variable),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkxEnvironment_VariableClass_Members),
	int(sizeof(hkxEnvironment_VariableClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkxEnvironment
//
const hkInternalClassMember hkxEnvironment::Members[] =
{
	{ "variables", &hkxEnvironmentVariableClass, HK_NULL, hkClassMember::TYPE_ARRAY, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkxEnvironment,m_variables) }
};
extern const hkClass hkxEnvironmentClass;
const hkClass hkxEnvironmentClass(
	"hkxEnvironment",
	HK_NULL, // parent
	sizeof(hkxEnvironment),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkxEnvironment::Members),
	int(sizeof(hkxEnvironment::Members)/sizeof(hkInternalClassMember)),
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
