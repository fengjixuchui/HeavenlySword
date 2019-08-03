/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkbase/monitor/hkMonitorStreamAnalyzer.h'

#include <hkbase/hkBase.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkbase/monitor/hkMonitorStreamAnalyzer.h>


// External pointer and enum types
extern const hkClass ColorTableColorPairClass;
extern const hkClass NodeNodeClass;
extern const hkClass ThreadDrawInputColorTableClass;
extern const hkClass hkMonitorStreamAnalyzerClass;
extern const hkClass hkMonitorStreamFrameInfoClass;
extern const hkClass hkMonitorStreamStringMapStringMapClass;

//
// Class hkMonitorStreamStringMap::StringMap
//
static const hkInternalClassMember hkMonitorStreamStringMap_StringMapClass_Members[] =
{
	{ "id", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT64, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkMonitorStreamStringMap::StringMap,m_id) },
	{ "string", HK_NULL, HK_NULL, hkClassMember::TYPE_CSTRING, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkMonitorStreamStringMap::StringMap,m_string) }
};
const hkClass hkMonitorStreamStringMapStringMapClass(
	"hkMonitorStreamStringMapStringMap",
	HK_NULL, // parent
	sizeof(hkMonitorStreamStringMap::StringMap),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkMonitorStreamStringMap_StringMapClass_Members),
	int(sizeof(hkMonitorStreamStringMap_StringMapClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkMonitorStreamStringMap
//
static const hkInternalClassMember hkMonitorStreamStringMapClass_Members[] =
{
	{ "map", &hkMonitorStreamStringMapStringMapClass, HK_NULL, hkClassMember::TYPE_ARRAY, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkMonitorStreamStringMap,m_map) }
};
extern const hkClass hkMonitorStreamStringMapClass;
const hkClass hkMonitorStreamStringMapClass(
	"hkMonitorStreamStringMap",
	HK_NULL, // parent
	sizeof(hkMonitorStreamStringMap),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkMonitorStreamStringMapClass_Members),
	int(sizeof(hkMonitorStreamStringMapClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Enum hkMonitorStreamFrameInfo::AbsoluteTimeCounter
//
static const hkInternalClassEnumItem hkMonitorStreamFrameInfoAbsoluteTimeCounterEnumItems[] =
{
	{0, "ABSOLUTE_TIME_TIMER_0"},
	{1, "ABSOLUTE_TIME_TIMER_1"},
	{0xffffffff, "ABSOLUTE_TIME_NOT_TIMED"},
};
static const hkInternalClassEnum hkMonitorStreamFrameInfoEnums[] = {
	{"AbsoluteTimeCounter", hkMonitorStreamFrameInfoAbsoluteTimeCounterEnumItems, 3 }
};
extern const hkClassEnum* hkMonitorStreamFrameInfoAbsoluteTimeCounterEnum = reinterpret_cast<const hkClassEnum*>(&hkMonitorStreamFrameInfoEnums[0]);

//
// Class hkMonitorStreamFrameInfo
//
static const hkInternalClassMember hkMonitorStreamFrameInfoClass_Members[] =
{
	{ "heading", HK_NULL, HK_NULL, hkClassMember::TYPE_CSTRING, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkMonitorStreamFrameInfo,m_heading) },
	{ "indexOfTimer0", HK_NULL, HK_NULL, hkClassMember::TYPE_INT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkMonitorStreamFrameInfo,m_indexOfTimer0) },
	{ "indexOfTimer1", HK_NULL, HK_NULL, hkClassMember::TYPE_INT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkMonitorStreamFrameInfo,m_indexOfTimer1) },
	{ "absoluteTimeCounter", HK_NULL, hkMonitorStreamFrameInfoAbsoluteTimeCounterEnum, hkClassMember::TYPE_ENUM, hkClassMember::TYPE_VOID, 0, hkClassMember::ENUM_32, HK_OFFSET_OF(hkMonitorStreamFrameInfo,m_absoluteTimeCounter) },
	{ "timerFactor0", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkMonitorStreamFrameInfo,m_timerFactor0) },
	{ "timerFactor1", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkMonitorStreamFrameInfo,m_timerFactor1) }
};
const hkClass hkMonitorStreamFrameInfoClass(
	"hkMonitorStreamFrameInfo",
	HK_NULL, // parent
	sizeof(hkMonitorStreamFrameInfo),
	HK_NULL,
	0, // interfaces
	reinterpret_cast<const hkClassEnum*>(hkMonitorStreamFrameInfoEnums),
	1, // enums
	reinterpret_cast<const hkClassMember*>(hkMonitorStreamFrameInfoClass_Members),
	int(sizeof(hkMonitorStreamFrameInfoClass_Members)/sizeof(hkInternalClassMember)),
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
