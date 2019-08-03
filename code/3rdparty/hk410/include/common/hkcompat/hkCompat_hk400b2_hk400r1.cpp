/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkcompat/hkCompat.h>
#include <hkcompat/hkCompatUtil.h>
#include <hkbase/config/hkConfigVersion.h>
#include <hkserialize/version/hkVersionRegistry.h>
#include <hkserialize/version/hkVersionUtil.h>
#include <hkserialize/util/hkBuiltinTypeRegistry.h>

#include <hkcompat/hkHavokAllClasses.h>

static void MultithreadLock_400b2_400r1(
	hkVariant& oldObj,
	hkVariant& newObj,
	hkObjectUpdateTracker& )
{
	hkClassMemberAccessor newThreadId(newObj,		"threadId");
	hkClassMemberAccessor newLockBitStack(newObj,	"lockBitStack");
	hkClassMemberAccessor newLockCount(newObj,		"lockCount");

	if( newThreadId.isOk() && newLockBitStack.isOk() && newLockCount.isOk() )
	{
		newThreadId.asInt32() = (hkUint32)0xfffffff1; //UNLOCKED;
		newLockCount.asInt32() = 0;
		newLockBitStack.asInt16() = 0;
	}
}

static void Shape_400b2_400r1(
	hkVariant& oldObj,
	hkVariant& newObj,
	hkObjectUpdateTracker& )
{
	hkClassMemberAccessor newUserData(newObj, "userData");
	hkClassMemberAccessor oldUserData(oldObj, "userData");
	if( newUserData.isOk() && oldUserData.isOk() )
	{
		newUserData.asPointer() = (void*)(hkUlong)oldUserData.asInt32();
	}
}
 
static void BlenderGenerator_400b2_400r1(
	hkVariant& oldObj,
	hkVariant& newObj,
	hkObjectUpdateTracker& )
{
	hkClassMemberAccessor newAutoComputeSecondGeneratorWeight(newObj, "autoComputeSecondGeneratorWeight");
	hkClassMemberAccessor oldAutoComputeSecondGeneratorWeight(oldObj, "autoComputeSecondGeneratorWeight");

	newAutoComputeSecondGeneratorWeight.asBool() = ( oldAutoComputeSecondGeneratorWeight.asReal() != 0.0f );
}

static void BlendingTransition_400b2_400r1(
	hkVariant& oldObj,
	hkVariant& newObj,
	hkObjectUpdateTracker& )
{
	hkClassMemberAccessor newFlags(newObj, "flags");
	hkClassMemberAccessor oldFlags(oldObj, "flags");

	hkInt16 oldFlags16 = oldFlags.asInt16();
	hkInt16& newFlags16 = newFlags.asInt16();

	// flag 0x10 moved to 0x1; all other flags do not exist anymore
	if ( oldFlags16 & 0x10 )
	{
		newFlags16 = 1;
	}
	else
	{
		newFlags16 = 0;
	}
}

namespace hkCompat_hk400b2_hk400r1
{

#define REMOVED(TYPE) { 0,0, hkVersionUtil::VERSION_REMOVED, TYPE, HK_NULL }
#define BINARY_IDENTICAL(OLDSIG,NEWSIG,TYPE) { OLDSIG, NEWSIG, hkVersionUtil::VERSION_MANUAL, TYPE, HK_NULL }

static hkVersionUtil::ClassAction UpdateActions[] =
{
	// common
	{ 0x8bdd3e9a, 0x8bdd3e9a, hkVersionUtil::VERSION_VARIANT, "hkBoneAttachment", HK_NULL },
	{ 0xf598a34e, 0xf598a34e, hkVersionUtil::VERSION_VARIANT, "hkRootLevelContainer", HK_NULL },
	{ 0x853a899c, 0x853a899c, hkVersionUtil::VERSION_VARIANT, "hkRootLevelContainerNamedVariant", HK_NULL }, 
	{ 0x3d43489c, 0x3d43489c, hkVersionUtil::VERSION_VARIANT, "hkxMaterial", HK_NULL },
	{ 0x914da6c1, 0x914da6c1, hkVersionUtil::VERSION_VARIANT, "hkxAttribute", HK_NULL },
	{ 0x1667c01c, 0x1667c01c, hkVersionUtil::VERSION_VARIANT, "hkxAttributeGroup", HK_NULL }, 
	{ 0x0a62c79f, 0x0a62c79f, hkVersionUtil::VERSION_VARIANT, "hkxNode", HK_NULL }, 
	{ 0xe085ba9f, 0xe085ba9f, hkVersionUtil::VERSION_VARIANT, "hkxMaterialTextureStage", HK_NULL },

	// collide
	{ 0x6c787842, 0xb0463dc2, hkVersionUtil::VERSION_COPY, "hkCylinderShape", Shape_400b2_400r1 }, // Added m_cylBaseRadiusFactorForHeightFieldCollisions
	{ 0x46591ba5, 0x9ab27645, hkVersionUtil::VERSION_COPY, "hkShape", Shape_400b2_400r1 },

	//physics
	{ 0x56a4eeb6, 0x7107de4e, hkVersionUtil::VERSION_COPY, "hkWorldObject", HK_NULL },
	{ 0x7d7692dc, 0x7497262b, hkVersionUtil::VERSION_COPY, "hkMultiThreadLock", MultithreadLock_400b2_400r1 },

	// hkbehavior
	{ 0x36be79f3, 0x77cbe29a, hkVersionUtil::VERSION_COPY, "hkbAdditiveBinaryBlenderGenerator", HK_NULL },
	{ 0xa3595ae6, 0xcb1ea129, hkVersionUtil::VERSION_COPY, "hkbBinaryBlenderGenerator", HK_NULL },
	{ 0xaa2f93ea, 0x9259c780, hkVersionUtil::VERSION_COPY, "hkbBlenderGeneratorChild", HK_NULL },
	{ 0x50928eb7, 0x73d13d8b, hkVersionUtil::VERSION_COPY, "hkbBlenderGenerator", BlenderGenerator_400b2_400r1 },
	{ 0x6a48b9cc, 0x86405dd4, hkVersionUtil::VERSION_COPY, "hkbBlendingTransition", BlendingTransition_400b2_400r1 },
	{ 0x86947a2f, 0x6e04a880, hkVersionUtil::VERSION_COPY, "hkbCharacterData", HK_NULL },
	{ 0x1f7823c6, 0x37666936, hkVersionUtil::VERSION_COPY, "hkbClipGenerator", HK_NULL },
	{ 0x8363d8bf, 0xce515346, hkVersionUtil::VERSION_COPY, "hkbControlLookAtModifier", HK_NULL },
	{ 0x88ccffab, 0x62958e18, hkVersionUtil::VERSION_COPY, "hkbEvent", HK_NULL },
	{ 0xb486b469, 0x6d5dc665, hkVersionUtil::VERSION_COPY, "hkbFootIkModifier", HK_NULL },
	{ 0x12c3f7d2, 0x26f18ed8, hkVersionUtil::VERSION_COPY, "hkbFootPoseExtractionModifier", HK_NULL },
	{ 0x4008091f, 0x26036a03, hkVersionUtil::VERSION_COPY, "hkbGetUpModifier", HK_NULL },
	{ 0x51356225, 0xdcceb01d, hkVersionUtil::VERSION_COPY, "hkbReachModifier", HK_NULL },
	{ 0xf3766dd5, 0x0d68689c, hkVersionUtil::VERSION_COPY, "hkbHandIkModifier", HK_NULL },
	{ 0x4a4b2900, 0x3636ca41, hkVersionUtil::VERSION_COPY, "hkbLookAtModifier", HK_NULL },
	{ 0x4fa66239, 0xe39611c5, hkVersionUtil::VERSION_COPY, "hkbModifiedGenerator", HK_NULL },
	{ 0xe15733df, 0x9afe073a, hkVersionUtil::VERSION_COPY, "hkbPoseMatchingModifier", HK_NULL },
	{ 0x6bbd12e1, 0x2f8cf98d, hkVersionUtil::VERSION_COPY, "hkbPoweredRagdollControlsModifier", HK_NULL },
	{ 0x854b920e, 0x7a23640f, hkVersionUtil::VERSION_COPY, "hkbPoweredRagdollModifier", HK_NULL },
	{ 0x0ffad356, 0xe20769b8, hkVersionUtil::VERSION_COPY, "hkbRagdollDriverModifier", HK_NULL },
	{ 0xa95a25e0, 0xf7998da4, hkVersionUtil::VERSION_COPY, "hkbReferencePoseGenerator", HK_NULL },
	{ 0x7161df87, 0xcda57532, hkVersionUtil::VERSION_COPY, "hkbRigidBodyRagdollControlsModifier", HK_NULL },
	{ 0x0015833d, 0xa18be826, hkVersionUtil::VERSION_COPY, "hkbRigidBodyRagdollModifier", HK_NULL },
	{ 0xa435f17d, 0xac0fbec5, hkVersionUtil::VERSION_COPY, "hkbStateMachine", HK_NULL },
	{ 0xd2a94c18, 0xfd4a1d12, hkVersionUtil::VERSION_COPY, "hkbStateMachineStateInfo", HK_NULL },
	{ 0x604e8a4a, 0x61a06913, hkVersionUtil::VERSION_COPY, "hkbStateMachineTransitionInfo", HK_NULL },
	{ 0xd776a823, 0x7bd27e34, hkVersionUtil::VERSION_COPY, "hkbVariableSet", HK_NULL },
	{ 0x2f62e881, 0x3ea409cf, hkVersionUtil::VERSION_COPY, "hkbModifier", HK_NULL },
	{ 0x2f62e881, 0x3ea409cf, hkVersionUtil::VERSION_COPY, "hkbGenerator", HK_NULL },
	{ 0xbac80827, 0x605c0661, hkVersionUtil::VERSION_COPY, "hkbNode", HK_NULL },
	BINARY_IDENTICAL( 0x162dc9dd, 0x8916b3a7, "hkbClipTrigger" ),
	REMOVED( "hkbEventManager" ),
	REMOVED( "hkbEventManagerName" ),

	{ 0, 0, 0, HK_NULL, HK_NULL }
};

static hkVersionUtil::ClassRename renames[] =
{
	{ "hkbTransition", "hkbTransitionEffect" },
	{ "hkbBlendingTransition", "hkbBlendingTransitionEffect" },
	{ HK_NULL, HK_NULL }
};

extern const hkVersionUtil::UpdateDescription hkVersionUpdateDescription;
const hkVersionUtil::UpdateDescription hkVersionUpdateDescription =
{
	renames,
	UpdateActions,
	hkHavok400r1Classes::Classes
};

static hkResult HK_CALL update(
	hkArray<hkVariant>& objectsInOut,
	hkObjectUpdateTracker& tracker )
{
	static hkBool computedOffsets;
	if( computedOffsets == false )
	{
		hkVersionUtil::recomputeClassMemberOffsets( hkVersionUpdateDescription.classes );
		computedOffsets = true;
	}

	return hkVersionUtil::updateSingleVersion( objectsInOut, tracker, hkVersionUpdateDescription );
}

extern const hkVersionRegistry::Updater hkVersionUpdater;
const hkVersionRegistry::Updater hkVersionUpdater =
{
	hkHavok400b2Classes::VersionString,
	hkHavok400r1Classes::VersionString,
	update
};

} // namespace hkCompat_hk400b2_hk400r1

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
