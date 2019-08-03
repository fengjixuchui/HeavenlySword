/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkdynamics/world/hkWorldCinfo.h'

#include <hkdynamics/hkDynamics.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkdynamics/world/hkWorldCinfo.h>


// External pointer and enum types
extern const hkClass hkAabbClass;
extern const hkClass hkCollisionFilterClass;
extern const hkClass hkConvexListFilterClass;
extern const hkClass hkWorldMemoryWatchDogClass;

//
// Enum hkWorldCinfo::SolverType
//
static const hkInternalClassEnumItem hkWorldCinfoSolverTypeEnumItems[] =
{
	{0, "SOLVER_TYPE_INVALID"},
	{1, "SOLVER_TYPE_2ITERS_SOFT"},
	{2, "SOLVER_TYPE_2ITERS_MEDIUM"},
	{3, "SOLVER_TYPE_2ITERS_HARD"},
	{4, "SOLVER_TYPE_4ITERS_SOFT"},
	{5, "SOLVER_TYPE_4ITERS_MEDIUM"},
	{6, "SOLVER_TYPE_4ITERS_HARD"},
	{7, "SOLVER_TYPE_8ITERS_SOFT"},
	{8, "SOLVER_TYPE_8ITERS_MEDIUM"},
	{9, "SOLVER_TYPE_8ITERS_HARD"},
	{10, "SOLVER_TYPE_MAX_ID"},
};

//
// Enum hkWorldCinfo::SimulationType
//
static const hkInternalClassEnumItem hkWorldCinfoSimulationTypeEnumItems[] =
{
	{0, "SIMULATION_TYPE_INVALID"},
	{1, "SIMULATION_TYPE_DISCRETE"},
	{2, "SIMULATION_TYPE_CONTINUOUS"},
	{3, "SIMULATION_TYPE_MULTITHREADED"},
};

//
// Enum hkWorldCinfo::ContactPointGeneration
//
static const hkInternalClassEnumItem hkWorldCinfoContactPointGenerationEnumItems[] =
{
	{0, "CONTACT_POINT_ACCEPT_ALWAYS"},
	{1, "CONTACT_POINT_REJECT_DUBIOUS"},
	{2, "CONTACT_POINT_REJECT_MANY"},
};

//
// Enum hkWorldCinfo::BroadPhaseBorderBehaviour
//
static const hkInternalClassEnumItem hkWorldCinfoBroadPhaseBorderBehaviourEnumItems[] =
{
	{0, "BROADPHASE_BORDER_ASSERT"},
	{1, "BROADPHASE_BORDER_FIX_ENTITY"},
	{2, "BROADPHASE_BORDER_REMOVE_ENTITY"},
	{3, "BROADPHASE_BORDER_DO_NOTHING"},
};
static const hkInternalClassEnum hkWorldCinfoEnums[] = {
	{"SolverType", hkWorldCinfoSolverTypeEnumItems, 11 },
	{"SimulationType", hkWorldCinfoSimulationTypeEnumItems, 4 },
	{"ContactPointGeneration", hkWorldCinfoContactPointGenerationEnumItems, 3 },
	{"BroadPhaseBorderBehaviour", hkWorldCinfoBroadPhaseBorderBehaviourEnumItems, 4 }
};
extern const hkClassEnum* hkWorldCinfoSolverTypeEnum = reinterpret_cast<const hkClassEnum*>(&hkWorldCinfoEnums[0]);
extern const hkClassEnum* hkWorldCinfoSimulationTypeEnum = reinterpret_cast<const hkClassEnum*>(&hkWorldCinfoEnums[1]);
extern const hkClassEnum* hkWorldCinfoContactPointGenerationEnum = reinterpret_cast<const hkClassEnum*>(&hkWorldCinfoEnums[2]);
extern const hkClassEnum* hkWorldCinfoBroadPhaseBorderBehaviourEnum = reinterpret_cast<const hkClassEnum*>(&hkWorldCinfoEnums[3]);

//
// Class hkWorldCinfo
//
static const hkInternalClassMember hkWorldCinfoClass_Members[] =
{
	{ "gravity", HK_NULL, HK_NULL, hkClassMember::TYPE_VECTOR4, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWorldCinfo,m_gravity) },
	{ "broadPhaseQuerySize", HK_NULL, HK_NULL, hkClassMember::TYPE_INT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWorldCinfo,m_broadPhaseQuerySize) },
	{ "contactRestingVelocity", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWorldCinfo,m_contactRestingVelocity) },
	{ "broadPhaseBorderBehaviour", HK_NULL, hkWorldCinfoBroadPhaseBorderBehaviourEnum, hkClassMember::TYPE_ENUM, hkClassMember::TYPE_VOID, 0, hkClassMember::ENUM_8, HK_OFFSET_OF(hkWorldCinfo,m_broadPhaseBorderBehaviour) },
	{ "broadPhaseWorldAabb", &hkAabbClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWorldCinfo,m_broadPhaseWorldAabb) },
	{ "collisionTolerance", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWorldCinfo,m_collisionTolerance) },
	{ "collisionFilter", &hkCollisionFilterClass, HK_NULL, hkClassMember::TYPE_POINTER, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkWorldCinfo,m_collisionFilter) },
	{ "convexListFilter", &hkConvexListFilterClass, HK_NULL, hkClassMember::TYPE_POINTER, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkWorldCinfo,m_convexListFilter) },
	{ "expectedMaxLinearVelocity", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWorldCinfo,m_expectedMaxLinearVelocity) },
	{ "expectedMinPsiDeltaTime", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWorldCinfo,m_expectedMinPsiDeltaTime) },
	{ "memoryWatchDog", &hkWorldMemoryWatchDogClass, HK_NULL, hkClassMember::TYPE_POINTER, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkWorldCinfo,m_memoryWatchDog) },
	{ "broadPhaseNumMarkers", HK_NULL, HK_NULL, hkClassMember::TYPE_INT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWorldCinfo,m_broadPhaseNumMarkers) },
	{ "contactPointGeneration", HK_NULL, hkWorldCinfoContactPointGenerationEnum, hkClassMember::TYPE_ENUM, hkClassMember::TYPE_VOID, 0, hkClassMember::ENUM_8, HK_OFFSET_OF(hkWorldCinfo,m_contactPointGeneration) },
	{ "solverTau", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWorldCinfo,m_solverTau) },
	{ "solverDamp", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWorldCinfo,m_solverDamp) },
	{ "solverIterations", HK_NULL, HK_NULL, hkClassMember::TYPE_INT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWorldCinfo,m_solverIterations) },
	{ "solverMicrosteps", HK_NULL, HK_NULL, hkClassMember::TYPE_INT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWorldCinfo,m_solverMicrosteps) },
	{ "iterativeLinearCastEarlyOutDistance", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWorldCinfo,m_iterativeLinearCastEarlyOutDistance) },
	{ "iterativeLinearCastMaxIterations", HK_NULL, HK_NULL, hkClassMember::TYPE_INT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWorldCinfo,m_iterativeLinearCastMaxIterations) },
	{ "highFrequencyDeactivationPeriod", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWorldCinfo,m_highFrequencyDeactivationPeriod) },
	{ "lowFrequencyDeactivationPeriod", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWorldCinfo,m_lowFrequencyDeactivationPeriod) },
	{ "shouldActivateOnRigidBodyTransformChange", HK_NULL, HK_NULL, hkClassMember::TYPE_BOOL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWorldCinfo,m_shouldActivateOnRigidBodyTransformChange) },
	{ "wantOldStyleDeactivation", HK_NULL, HK_NULL, hkClassMember::TYPE_BOOL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWorldCinfo,m_wantOldStyleDeactivation) },
	{ "deactivationReferenceDistance", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWorldCinfo,m_deactivationReferenceDistance) },
	{ "toiCollisionResponseRotateNormal", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWorldCinfo,m_toiCollisionResponseRotateNormal) },
	{ "enableDeactivation", HK_NULL, HK_NULL, hkClassMember::TYPE_BOOL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWorldCinfo,m_enableDeactivation) },
	{ "simulationType", HK_NULL, hkWorldCinfoSimulationTypeEnum, hkClassMember::TYPE_ENUM, hkClassMember::TYPE_VOID, 0, hkClassMember::ENUM_8, HK_OFFSET_OF(hkWorldCinfo,m_simulationType) },
	{ "enableSimulationIslands", HK_NULL, HK_NULL, hkClassMember::TYPE_BOOL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWorldCinfo,m_enableSimulationIslands) },
	{ "minDesiredIslandSize", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWorldCinfo,m_minDesiredIslandSize) },
	{ "processActionsInSingleThread", HK_NULL, HK_NULL, hkClassMember::TYPE_BOOL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWorldCinfo,m_processActionsInSingleThread) },
	{ "frameMarkerPsiSnap", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWorldCinfo,m_frameMarkerPsiSnap) }
};
namespace
{
	struct hkWorldCinfo_DefaultStruct
	{
		int s_defaultOffsets[31];
		typedef hkInt8 _hkBool;
		typedef hkReal _hkVector4[4];
		typedef hkReal _hkQuaternion[4];
		typedef hkReal _hkMatrix3[12];
		typedef hkReal _hkRotation[12];
		typedef hkReal _hkQsTransform[12];
		typedef hkReal _hkMatrix4[16];
		typedef hkReal _hkTransform[16];
		_hkVector4 m_gravity;
		hkInt32 m_broadPhaseQuerySize;
		hkReal m_collisionTolerance;
		hkReal m_expectedMaxLinearVelocity;
		hkReal m_expectedMinPsiDeltaTime;
		hkReal m_solverDamp;
		hkInt32 m_solverIterations;
		hkInt32 m_solverMicrosteps;
		hkReal m_iterativeLinearCastEarlyOutDistance;
		hkInt32 m_iterativeLinearCastMaxIterations;
		hkReal m_highFrequencyDeactivationPeriod;
		hkReal m_lowFrequencyDeactivationPeriod;
		_hkBool m_shouldActivateOnRigidBodyTransformChange;
		hkReal m_deactivationReferenceDistance;
		hkReal m_toiCollisionResponseRotateNormal;
		_hkBool m_enableDeactivation;
		_hkBool m_enableSimulationIslands;
		hkUint32 m_minDesiredIslandSize;
		_hkBool m_processActionsInSingleThread;
		hkReal m_frameMarkerPsiSnap;
	};
	const hkWorldCinfo_DefaultStruct hkWorldCinfo_Default =
	{
		{HK_OFFSET_OF(hkWorldCinfo_DefaultStruct,m_gravity),HK_OFFSET_OF(hkWorldCinfo_DefaultStruct,m_broadPhaseQuerySize),-1,-1,-1,HK_OFFSET_OF(hkWorldCinfo_DefaultStruct,m_collisionTolerance),-1,-1,HK_OFFSET_OF(hkWorldCinfo_DefaultStruct,m_expectedMaxLinearVelocity),HK_OFFSET_OF(hkWorldCinfo_DefaultStruct,m_expectedMinPsiDeltaTime),-1,-1,-1,-1,HK_OFFSET_OF(hkWorldCinfo_DefaultStruct,m_solverDamp),HK_OFFSET_OF(hkWorldCinfo_DefaultStruct,m_solverIterations),HK_OFFSET_OF(hkWorldCinfo_DefaultStruct,m_solverMicrosteps),HK_OFFSET_OF(hkWorldCinfo_DefaultStruct,m_iterativeLinearCastEarlyOutDistance),HK_OFFSET_OF(hkWorldCinfo_DefaultStruct,m_iterativeLinearCastMaxIterations),HK_OFFSET_OF(hkWorldCinfo_DefaultStruct,m_highFrequencyDeactivationPeriod),HK_OFFSET_OF(hkWorldCinfo_DefaultStruct,m_lowFrequencyDeactivationPeriod),HK_OFFSET_OF(hkWorldCinfo_DefaultStruct,m_shouldActivateOnRigidBodyTransformChange),-1,HK_OFFSET_OF(hkWorldCinfo_DefaultStruct,m_deactivationReferenceDistance),HK_OFFSET_OF(hkWorldCinfo_DefaultStruct,m_toiCollisionResponseRotateNormal),HK_OFFSET_OF(hkWorldCinfo_DefaultStruct,m_enableDeactivation),-1,HK_OFFSET_OF(hkWorldCinfo_DefaultStruct,m_enableSimulationIslands),HK_OFFSET_OF(hkWorldCinfo_DefaultStruct,m_minDesiredIslandSize),HK_OFFSET_OF(hkWorldCinfo_DefaultStruct,m_processActionsInSingleThread),HK_OFFSET_OF(hkWorldCinfo_DefaultStruct,m_frameMarkerPsiSnap)},
		{0,-9.8f,0},1024,.1f,200,1.0f/30.0f,.6f,4,1,.01f,20,.2f,10,true,0.02f,0.2f,true,true,64,true,.0001f
	};
}
extern const hkClass hkReferencedObjectClass;

extern const hkClass hkWorldCinfoClass;
const hkClass hkWorldCinfoClass(
	"hkWorldCinfo",
	&hkReferencedObjectClass, // parent
	sizeof(hkWorldCinfo),
	HK_NULL,
	0, // interfaces
	reinterpret_cast<const hkClassEnum*>(hkWorldCinfoEnums),
	4, // enums
	reinterpret_cast<const hkClassMember*>(hkWorldCinfoClass_Members),
	int(sizeof(hkWorldCinfoClass_Members)/sizeof(hkInternalClassMember)),
	&hkWorldCinfo_Default
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
