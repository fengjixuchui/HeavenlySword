/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkinternal/collide/mopp/code/hkMoppCode.h'

#include <hkinternal/hkInternal.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkinternal/collide/mopp/code/hkMoppCode.h>


// External pointer and enum types
extern const hkClass hkMoppCodeCodeInfoClass;

//
// Class hkMoppCode::CodeInfo
//
static const hkInternalClassMember hkMoppCode_CodeInfoClass_Members[] =
{
	{ "offset", HK_NULL, HK_NULL, hkClassMember::TYPE_VECTOR4, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkMoppCode::CodeInfo,m_offset) }
};
const hkClass hkMoppCodeCodeInfoClass(
	"hkMoppCodeCodeInfo",
	HK_NULL, // parent
	sizeof(hkMoppCode::CodeInfo),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkMoppCode_CodeInfoClass_Members),
	int(sizeof(hkMoppCode_CodeInfoClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkMoppCode
//
static const hkInternalClassMember hkMoppCodeClass_Members[] =
{
	{ "info", &hkMoppCodeCodeInfoClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkMoppCode,m_info) },
	{ "data", HK_NULL, HK_NULL, hkClassMember::TYPE_ARRAY, hkClassMember::TYPE_UINT8, 0, 0, HK_OFFSET_OF(hkMoppCode,m_data) }
};
extern const hkClass hkReferencedObjectClass;

extern const hkClass hkMoppCodeClass;
const hkClass hkMoppCodeClass(
	"hkMoppCode",
	&hkReferencedObjectClass, // parent
	sizeof(hkMoppCode),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkMoppCodeClass_Members),
	int(sizeof(hkMoppCodeClass_Members)/sizeof(hkInternalClassMember)),
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
