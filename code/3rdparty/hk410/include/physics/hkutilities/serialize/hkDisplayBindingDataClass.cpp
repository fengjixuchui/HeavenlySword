/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkutilities/serialize/hkDisplayBindingData.h'

#include <hkbase/hkBase.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkutilities/serialize/hkDisplayBindingData.h>


// External pointer and enum types
extern const hkClass hkPhysicsSystemClass;
extern const hkClass hkPhysicsSystemDisplayBindingClass;
extern const hkClass hkRigidBodyClass;
extern const hkClass hkRigidBodyDisplayBindingClass;
extern const hkClass hkxMeshClass;

//
// Class hkRigidBodyDisplayBinding
//
static const hkInternalClassMember hkRigidBodyDisplayBindingClass_Members[] =
{
	{ "rigidBody", &hkRigidBodyClass, HK_NULL, hkClassMember::TYPE_POINTER, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkRigidBodyDisplayBinding,m_rigidBody) },
	{ "displayObject", &hkxMeshClass, HK_NULL, hkClassMember::TYPE_POINTER, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkRigidBodyDisplayBinding,m_displayObject) },
	{ "rigidBodyFromDisplayObjectTransform", HK_NULL, HK_NULL, hkClassMember::TYPE_MATRIX4, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkRigidBodyDisplayBinding,m_rigidBodyFromDisplayObjectTransform) }
};
const hkClass hkRigidBodyDisplayBindingClass(
	"hkRigidBodyDisplayBinding",
	HK_NULL, // parent
	sizeof(hkRigidBodyDisplayBinding),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkRigidBodyDisplayBindingClass_Members),
	int(sizeof(hkRigidBodyDisplayBindingClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkPhysicsSystemDisplayBinding
//
static const hkInternalClassMember hkPhysicsSystemDisplayBindingClass_Members[] =
{
	{ "bindings", &hkRigidBodyDisplayBindingClass, HK_NULL, hkClassMember::TYPE_ARRAY, hkClassMember::TYPE_POINTER, 0, 0, HK_OFFSET_OF(hkPhysicsSystemDisplayBinding,m_bindings) },
	{ "system", &hkPhysicsSystemClass, HK_NULL, hkClassMember::TYPE_POINTER, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkPhysicsSystemDisplayBinding,m_system) }
};
const hkClass hkPhysicsSystemDisplayBindingClass(
	"hkPhysicsSystemDisplayBinding",
	HK_NULL, // parent
	sizeof(hkPhysicsSystemDisplayBinding),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkPhysicsSystemDisplayBindingClass_Members),
	int(sizeof(hkPhysicsSystemDisplayBindingClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkDisplayBindingData
//
static const hkInternalClassMember hkDisplayBindingDataClass_Members[] =
{
	{ "rigidBodyBindings", &hkRigidBodyDisplayBindingClass, HK_NULL, hkClassMember::TYPE_ARRAY, hkClassMember::TYPE_POINTER, 0, 0, HK_OFFSET_OF(hkDisplayBindingData,m_rigidBodyBindings) },
	{ "physicsSystemBindings", &hkPhysicsSystemDisplayBindingClass, HK_NULL, hkClassMember::TYPE_ARRAY, hkClassMember::TYPE_POINTER, 0, 0, HK_OFFSET_OF(hkDisplayBindingData,m_physicsSystemBindings) }
};
extern const hkClass hkDisplayBindingDataClass;
const hkClass hkDisplayBindingDataClass(
	"hkDisplayBindingData",
	HK_NULL, // parent
	sizeof(hkDisplayBindingData),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkDisplayBindingDataClass_Members),
	int(sizeof(hkDisplayBindingDataClass_Members)/sizeof(hkInternalClassMember)),
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
