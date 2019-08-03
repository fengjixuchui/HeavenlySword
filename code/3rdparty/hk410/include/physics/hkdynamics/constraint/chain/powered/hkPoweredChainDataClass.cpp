/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkdynamics/constraint/chain/powered/hkPoweredChainData.h'

#include <hkdynamics/hkDynamics.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkdynamics/constraint/chain/powered/hkPoweredChainData.h>


// External pointer and enum types
extern const hkClass hkBridgeAtomsClass;
extern const hkClass hkConstraintMotorClass;
extern const hkClass hkPoweredChainDataConstraintInfoClass;

//
// Class hkPoweredChainData::ConstraintInfo
//
static const hkInternalClassMember hkPoweredChainData_ConstraintInfoClass_Members[] =
{
	{ "pivotInA", HK_NULL, HK_NULL, hkClassMember::TYPE_VECTOR4, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkPoweredChainData::ConstraintInfo,m_pivotInA) },
	{ "pivotInB", HK_NULL, HK_NULL, hkClassMember::TYPE_VECTOR4, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkPoweredChainData::ConstraintInfo,m_pivotInB) },
	{ "aTc", HK_NULL, HK_NULL, hkClassMember::TYPE_QUATERNION, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkPoweredChainData::ConstraintInfo,m_aTc) },
	{ "bTc", HK_NULL, HK_NULL, hkClassMember::TYPE_QUATERNION, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkPoweredChainData::ConstraintInfo,m_bTc) },
	{ "motors", &hkConstraintMotorClass, HK_NULL, hkClassMember::TYPE_POINTER, hkClassMember::TYPE_STRUCT, 3, 0, HK_OFFSET_OF(hkPoweredChainData::ConstraintInfo,m_motors) },
	{ "switchBodies", HK_NULL, HK_NULL, hkClassMember::TYPE_BOOL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkPoweredChainData::ConstraintInfo,m_switchBodies) }
};
const hkClass hkPoweredChainDataConstraintInfoClass(
	"hkPoweredChainDataConstraintInfo",
	HK_NULL, // parent
	sizeof(hkPoweredChainData::ConstraintInfo),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkPoweredChainData_ConstraintInfoClass_Members),
	int(sizeof(hkPoweredChainData_ConstraintInfoClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkPoweredChainData
//
static const hkInternalClassMember hkPoweredChainDataClass_Members[] =
{
	{ "atoms", &hkBridgeAtomsClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkPoweredChainData,m_atoms) },
	{ "infos", &hkPoweredChainDataConstraintInfoClass, HK_NULL, hkClassMember::TYPE_ARRAY, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkPoweredChainData,m_infos) },
	{ "tau", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkPoweredChainData,m_tau) },
	{ "damping", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkPoweredChainData,m_damping) },
	{ "cfmLinAdd", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkPoweredChainData,m_cfmLinAdd) },
	{ "cfmLinMul", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkPoweredChainData,m_cfmLinMul) },
	{ "cfmAngAdd", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkPoweredChainData,m_cfmAngAdd) },
	{ "cfmAngMul", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkPoweredChainData,m_cfmAngMul) },
	{ "maxErrorDistance", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkPoweredChainData,m_maxErrorDistance) }
};
namespace
{
	struct hkPoweredChainData_DefaultStruct
	{
		int s_defaultOffsets[9];
		typedef hkInt8 _hkBool;
		typedef hkReal _hkVector4[4];
		typedef hkReal _hkQuaternion[4];
		typedef hkReal _hkMatrix3[12];
		typedef hkReal _hkRotation[12];
		typedef hkReal _hkQsTransform[12];
		typedef hkReal _hkMatrix4[16];
		typedef hkReal _hkTransform[16];
		hkReal m_cfmLinAdd;
		hkReal m_cfmLinMul;
		hkReal m_cfmAngAdd;
		hkReal m_cfmAngMul;
	};
	const hkPoweredChainData_DefaultStruct hkPoweredChainData_Default =
	{
		{-1,-1,-1,-1,HK_OFFSET_OF(hkPoweredChainData_DefaultStruct,m_cfmLinAdd),HK_OFFSET_OF(hkPoweredChainData_DefaultStruct,m_cfmLinMul),HK_OFFSET_OF(hkPoweredChainData_DefaultStruct,m_cfmAngAdd),HK_OFFSET_OF(hkPoweredChainData_DefaultStruct,m_cfmAngMul),-1},
		0.1f*1.19209290e-07f,1.0f,0.1f*1.19209290e-07F,1.0f
	};
}
extern const hkClass hkConstraintChainDataClass;

extern const hkClass hkPoweredChainDataClass;
const hkClass hkPoweredChainDataClass(
	"hkPoweredChainData",
	&hkConstraintChainDataClass, // parent
	sizeof(hkPoweredChainData),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkPoweredChainDataClass_Members),
	int(sizeof(hkPoweredChainDataClass_Members)/sizeof(hkInternalClassMember)),
	&hkPoweredChainData_Default
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
