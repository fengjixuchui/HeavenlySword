/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkdynamics/constraint/chain/stiffspring/hkStiffSpringChainData.h'

#include <hkdynamics/hkDynamics.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkdynamics/constraint/chain/stiffspring/hkStiffSpringChainData.h>


// External pointer and enum types
extern const hkClass hkBridgeAtomsClass;
extern const hkClass hkStiffSpringChainDataConstraintInfoClass;

//
// Class hkStiffSpringChainData::ConstraintInfo
//
static const hkInternalClassMember hkStiffSpringChainData_ConstraintInfoClass_Members[] =
{
	{ "pivotInA", HK_NULL, HK_NULL, hkClassMember::TYPE_VECTOR4, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkStiffSpringChainData::ConstraintInfo,m_pivotInA) },
	{ "pivotInB", HK_NULL, HK_NULL, hkClassMember::TYPE_VECTOR4, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkStiffSpringChainData::ConstraintInfo,m_pivotInB) },
	{ "springLength", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkStiffSpringChainData::ConstraintInfo,m_springLength) }
};
const hkClass hkStiffSpringChainDataConstraintInfoClass(
	"hkStiffSpringChainDataConstraintInfo",
	HK_NULL, // parent
	sizeof(hkStiffSpringChainData::ConstraintInfo),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkStiffSpringChainData_ConstraintInfoClass_Members),
	int(sizeof(hkStiffSpringChainData_ConstraintInfoClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkStiffSpringChainData
//
static const hkInternalClassMember hkStiffSpringChainDataClass_Members[] =
{
	{ "atoms", &hkBridgeAtomsClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkStiffSpringChainData,m_atoms) },
	{ "infos", &hkStiffSpringChainDataConstraintInfoClass, HK_NULL, hkClassMember::TYPE_ARRAY, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkStiffSpringChainData,m_infos) },
	{ "tau", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkStiffSpringChainData,m_tau) },
	{ "damping", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkStiffSpringChainData,m_damping) },
	{ "cfm", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkStiffSpringChainData,m_cfm) }
};
extern const hkClass hkConstraintChainDataClass;

extern const hkClass hkStiffSpringChainDataClass;
const hkClass hkStiffSpringChainDataClass(
	"hkStiffSpringChainData",
	&hkConstraintChainDataClass, // parent
	sizeof(hkStiffSpringChainData),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkStiffSpringChainDataClass_Members),
	int(sizeof(hkStiffSpringChainDataClass_Members)/sizeof(hkInternalClassMember)),
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
