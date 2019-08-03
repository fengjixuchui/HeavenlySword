/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkdynamics/entity/hkEntity.h'

#include <hkdynamics/hkDynamics.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkdynamics/entity/hkEntity.h>


// External pointer and enum types
extern const hkClass hkActionClass;
extern const hkClass hkCollisionListenerClass;
extern const hkClass hkConstraintInstanceClass;
extern const hkClass hkConstraintInternalClass;
extern const hkClass hkEntityActivationListenerClass;
extern const hkClass hkEntityDeactivatorClass;
extern const hkClass hkEntityListenerClass;
extern const hkClass hkMaterialClass;
extern const hkClass hkMaxSizeMotionClass;
extern const hkClass hkSimulationIslandClass;

//
// Class hkEntity
//
const hkInternalClassMember hkEntity::Members[] =
{
	{ "simulationIsland", HK_NULL, HK_NULL, hkClassMember::TYPE_ZERO, hkClassMember::TYPE_POINTER, 0, 0, HK_OFFSET_OF(hkEntity,m_simulationIsland) },
	{ "material", &hkMaterialClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkEntity,m_material) },
	{ "deactivator", &hkEntityDeactivatorClass, HK_NULL, hkClassMember::TYPE_POINTER, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkEntity,m_deactivator) },
	{ "constraintsMaster", HK_NULL, HK_NULL, hkClassMember::TYPE_ZERO, hkClassMember::TYPE_ARRAY, 0, 0, HK_OFFSET_OF(hkEntity,m_constraintsMaster) },
	{ "constraintsSlave", HK_NULL, HK_NULL, hkClassMember::TYPE_ZERO, hkClassMember::TYPE_ARRAY, 0, 0, HK_OFFSET_OF(hkEntity,m_constraintsSlave) },
	{ "constraintRuntime", HK_NULL, HK_NULL, hkClassMember::TYPE_ARRAY, hkClassMember::TYPE_UINT8, 0, 0, HK_OFFSET_OF(hkEntity,m_constraintRuntime) },
	{ "storageIndex", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT16, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkEntity,m_storageIndex) },
	{ "processContactCallbackDelay", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT16, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkEntity,m_processContactCallbackDelay) },
	{ "autoRemoveLevel", HK_NULL, HK_NULL, hkClassMember::TYPE_INT8, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkEntity,m_autoRemoveLevel) },
	{ "solverData", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkEntity,m_solverData) },
	{ "uid", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkEntity,m_uid) },
	{ "motion", &hkMaxSizeMotionClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkEntity,m_motion) },
	{ "collisionListeners", HK_NULL, HK_NULL, hkClassMember::TYPE_ZERO, hkClassMember::TYPE_ARRAY, 0, 0, HK_OFFSET_OF(hkEntity,m_collisionListeners) },
	{ "activationListeners", HK_NULL, HK_NULL, hkClassMember::TYPE_ZERO, hkClassMember::TYPE_ARRAY, 0, 0, HK_OFFSET_OF(hkEntity,m_activationListeners) },
	{ "entityListeners", HK_NULL, HK_NULL, hkClassMember::TYPE_ZERO, hkClassMember::TYPE_ARRAY, 0, 0, HK_OFFSET_OF(hkEntity,m_entityListeners) },
	{ "actions", HK_NULL, HK_NULL, hkClassMember::TYPE_ZERO, hkClassMember::TYPE_ARRAY, 0, 0, HK_OFFSET_OF(hkEntity,m_actions) }
};
namespace
{
	struct hkEntity_DefaultStruct
	{
		int s_defaultOffsets[16];
		typedef hkInt8 _hkBool;
		typedef hkReal _hkVector4[4];
		typedef hkReal _hkQuaternion[4];
		typedef hkReal _hkMatrix3[12];
		typedef hkReal _hkRotation[12];
		typedef hkReal _hkQsTransform[12];
		typedef hkReal _hkMatrix4[16];
		typedef hkReal _hkTransform[16];
		hkUint32 m_uid;
	};
	const hkEntity_DefaultStruct hkEntity_Default =
	{
		{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,HK_OFFSET_OF(hkEntity_DefaultStruct,m_uid),-1,-1,-1,-1,-1},
		0xffffffff
	};
}
extern const hkClass hkWorldObjectClass;

extern const hkClass hkEntityClass;
const hkClass hkEntityClass(
	"hkEntity",
	&hkWorldObjectClass, // parent
	sizeof(hkEntity),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkEntity::Members),
	int(sizeof(hkEntity::Members)/sizeof(hkInternalClassMember)),
	&hkEntity_Default
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
