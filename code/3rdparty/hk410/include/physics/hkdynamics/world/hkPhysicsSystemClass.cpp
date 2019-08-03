/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkdynamics/world/hkPhysicsSystem.h'

#include <hkdynamics/hkDynamics.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkdynamics/world/hkPhysicsSystem.h>


// External pointer and enum types
extern const hkClass hkActionClass;
extern const hkClass hkConstraintInstanceClass;
extern const hkClass hkPhantomClass;
extern const hkClass hkRigidBodyClass;

//
// Class hkPhysicsSystem
//
const hkInternalClassMember hkPhysicsSystem::Members[] =
{
	{ "rigidBodies", &hkRigidBodyClass, HK_NULL, hkClassMember::TYPE_ARRAY, hkClassMember::TYPE_POINTER, 0, 0, HK_OFFSET_OF(hkPhysicsSystem,m_rigidBodies) },
	{ "constraints", &hkConstraintInstanceClass, HK_NULL, hkClassMember::TYPE_ARRAY, hkClassMember::TYPE_POINTER, 0, 0, HK_OFFSET_OF(hkPhysicsSystem,m_constraints) },
	{ "actions", &hkActionClass, HK_NULL, hkClassMember::TYPE_ARRAY, hkClassMember::TYPE_POINTER, 0, 0, HK_OFFSET_OF(hkPhysicsSystem,m_actions) },
	{ "phantoms", &hkPhantomClass, HK_NULL, hkClassMember::TYPE_ARRAY, hkClassMember::TYPE_POINTER, 0, 0, HK_OFFSET_OF(hkPhysicsSystem,m_phantoms) },
	{ "name", HK_NULL, HK_NULL, hkClassMember::TYPE_CSTRING, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkPhysicsSystem,m_name) },
	{ "userData", HK_NULL, HK_NULL, hkClassMember::TYPE_ZERO, hkClassMember::TYPE_POINTER, 0, 0, HK_OFFSET_OF(hkPhysicsSystem,m_userData) },
	{ "active", HK_NULL, HK_NULL, hkClassMember::TYPE_BOOL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkPhysicsSystem,m_active) }
};
namespace
{
	struct hkPhysicsSystem_DefaultStruct
	{
		int s_defaultOffsets[7];
		typedef hkInt8 _hkBool;
		typedef hkReal _hkVector4[4];
		typedef hkReal _hkQuaternion[4];
		typedef hkReal _hkMatrix3[12];
		typedef hkReal _hkRotation[12];
		typedef hkReal _hkQsTransform[12];
		typedef hkReal _hkMatrix4[16];
		typedef hkReal _hkTransform[16];
		_hkBool m_active;
	};
	const hkPhysicsSystem_DefaultStruct hkPhysicsSystem_Default =
	{
		{-1,-1,-1,-1,-1,-1,HK_OFFSET_OF(hkPhysicsSystem_DefaultStruct,m_active)},
		true
	};
}
extern const hkClass hkReferencedObjectClass;

extern const hkClass hkPhysicsSystemClass;
const hkClass hkPhysicsSystemClass(
	"hkPhysicsSystem",
	&hkReferencedObjectClass, // parent
	sizeof(hkPhysicsSystem),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkPhysicsSystem::Members),
	int(sizeof(hkPhysicsSystem::Members)/sizeof(hkInternalClassMember)),
	&hkPhysicsSystem_Default
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
