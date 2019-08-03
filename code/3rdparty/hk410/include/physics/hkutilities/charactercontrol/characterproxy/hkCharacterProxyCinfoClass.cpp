/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkutilities/charactercontrol/characterproxy/hkCharacterProxyCinfo.h'

#include <hkbase/hkBase.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkutilities/charactercontrol/characterproxy/hkCharacterProxyCinfo.h>


// External pointer and enum types
extern const hkClass hkShapePhantomClass;

//
// Class hkCharacterProxyCinfo
//
static const hkInternalClassMember hkCharacterProxyCinfoClass_Members[] =
{
	{ "position", HK_NULL, HK_NULL, hkClassMember::TYPE_VECTOR4, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkCharacterProxyCinfo,m_position) },
	{ "velocity", HK_NULL, HK_NULL, hkClassMember::TYPE_VECTOR4, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkCharacterProxyCinfo,m_velocity) },
	{ "dynamicFriction", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkCharacterProxyCinfo,m_dynamicFriction) },
	{ "staticFriction", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkCharacterProxyCinfo,m_staticFriction) },
	{ "keepContactTolerance", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkCharacterProxyCinfo,m_keepContactTolerance) },
	{ "up", HK_NULL, HK_NULL, hkClassMember::TYPE_VECTOR4, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkCharacterProxyCinfo,m_up) },
	{ "extraUpStaticFriction", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkCharacterProxyCinfo,m_extraUpStaticFriction) },
	{ "extraDownStaticFriction", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkCharacterProxyCinfo,m_extraDownStaticFriction) },
	{ "shapePhantom", &hkShapePhantomClass, HK_NULL, hkClassMember::TYPE_POINTER, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkCharacterProxyCinfo,m_shapePhantom) },
	{ "keepDistance", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkCharacterProxyCinfo,m_keepDistance) },
	{ "contactAngleSensitivity", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkCharacterProxyCinfo,m_contactAngleSensitivity) },
	{ "userPlanes", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkCharacterProxyCinfo,m_userPlanes) },
	{ "maxCharacterSpeedForSolver", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkCharacterProxyCinfo,m_maxCharacterSpeedForSolver) },
	{ "characterStrength", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkCharacterProxyCinfo,m_characterStrength) },
	{ "characterMass", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkCharacterProxyCinfo,m_characterMass) },
	{ "maxSlope", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkCharacterProxyCinfo,m_maxSlope) },
	{ "penetrationRecoverySpeed", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkCharacterProxyCinfo,m_penetrationRecoverySpeed) },
	{ "maxCastIterations", HK_NULL, HK_NULL, hkClassMember::TYPE_INT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkCharacterProxyCinfo,m_maxCastIterations) },
	{ "refreshManifoldInCheckSupport", HK_NULL, HK_NULL, hkClassMember::TYPE_BOOL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkCharacterProxyCinfo,m_refreshManifoldInCheckSupport) }
};
namespace
{
	struct hkCharacterProxyCinfo_DefaultStruct
	{
		int s_defaultOffsets[19];
		typedef hkInt8 _hkBool;
		typedef hkReal _hkVector4[4];
		typedef hkReal _hkQuaternion[4];
		typedef hkReal _hkMatrix3[12];
		typedef hkReal _hkRotation[12];
		typedef hkReal _hkQsTransform[12];
		typedef hkReal _hkMatrix4[16];
		typedef hkReal _hkTransform[16];
		hkReal m_contactAngleSensitivity;
		int m_maxCastIterations;
		bool m_refreshManifoldInCheckSupport;
	};
	const hkCharacterProxyCinfo_DefaultStruct hkCharacterProxyCinfo_Default =
	{
		{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,HK_OFFSET_OF(hkCharacterProxyCinfo_DefaultStruct,m_contactAngleSensitivity),-1,-1,-1,-1,-1,-1,HK_OFFSET_OF(hkCharacterProxyCinfo_DefaultStruct,m_maxCastIterations),HK_OFFSET_OF(hkCharacterProxyCinfo_DefaultStruct,m_refreshManifoldInCheckSupport)},
		10,10,false
	};
}
extern const hkClass hkCharacterProxyCinfoClass;
const hkClass hkCharacterProxyCinfoClass(
	"hkCharacterProxyCinfo",
	HK_NULL, // parent
	sizeof(hkCharacterProxyCinfo),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkCharacterProxyCinfoClass_Members),
	int(sizeof(hkCharacterProxyCinfoClass_Members)/sizeof(hkInternalClassMember)),
	&hkCharacterProxyCinfo_Default
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
