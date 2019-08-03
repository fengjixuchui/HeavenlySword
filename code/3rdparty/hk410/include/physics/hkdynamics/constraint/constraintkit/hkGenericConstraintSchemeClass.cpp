/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkdynamics/constraint/constraintkit/hkGenericConstraintScheme.h'

#include <hkdynamics/hkDynamics.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkdynamics/constraint/constraintkit/hkGenericConstraintScheme.h>


// External pointer and enum types
extern const hkClass hkConstraintInfoClass;
extern const hkClass hkConstraintModifierClass;
extern const hkClass hkConstraintMotorClass;
extern const hkClass hkGenericConstraintDataSchemeintClass;

//
// Class hkGenericConstraintDataScheme
//
static const hkInternalClassMember hkGenericConstraintDataSchemeClass_Members[] =
{
	{ "info", &hkConstraintInfoClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkGenericConstraintDataScheme,m_info) },
	{ "data", HK_NULL, HK_NULL, hkClassMember::TYPE_ARRAY, hkClassMember::TYPE_VECTOR4, 0, 0, HK_OFFSET_OF(hkGenericConstraintDataScheme,m_data) },
	{ "commands", HK_NULL, HK_NULL, hkClassMember::TYPE_ARRAY, hkClassMember::TYPE_INT32, 0, 0, HK_OFFSET_OF(hkGenericConstraintDataScheme,m_commands) },
	{ "modifiers", HK_NULL, HK_NULL, hkClassMember::TYPE_ZERO, hkClassMember::TYPE_ARRAY, 0, 0, HK_OFFSET_OF(hkGenericConstraintDataScheme,m_modifiers) },
	{ "motors", &hkConstraintMotorClass, HK_NULL, hkClassMember::TYPE_ARRAY, hkClassMember::TYPE_POINTER, 0, 0, HK_OFFSET_OF(hkGenericConstraintDataScheme,m_motors) }
};
extern const hkClass hkGenericConstraintDataSchemeClass;
const hkClass hkGenericConstraintDataSchemeClass(
	"hkGenericConstraintDataScheme",
	HK_NULL, // parent
	sizeof(hkGenericConstraintDataScheme),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkGenericConstraintDataSchemeClass_Members),
	int(sizeof(hkGenericConstraintDataSchemeClass_Members)/sizeof(hkInternalClassMember)),
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
