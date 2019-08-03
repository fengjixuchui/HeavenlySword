/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkanimation/hkAnimationContainer.h'

#include <hkanimation/hkAnimation.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkanimation/hkAnimationContainer.h>


// External pointer and enum types
extern const hkClass hkAnimationBindingClass;
extern const hkClass hkBoneAttachmentClass;
extern const hkClass hkMeshBindingClass;
extern const hkClass hkSkeletalAnimationClass;
extern const hkClass hkSkeletonClass;

//
// Class hkAnimationContainer
//
static const hkInternalClassMember hkAnimationContainerClass_Members[] =
{
	{ "skeletons", &hkSkeletonClass, HK_NULL, hkClassMember::TYPE_SIMPLEARRAY, hkClassMember::TYPE_POINTER, 0, 0, HK_OFFSET_OF(hkAnimationContainer,m_skeletons) },
	{ "animations", &hkSkeletalAnimationClass, HK_NULL, hkClassMember::TYPE_SIMPLEARRAY, hkClassMember::TYPE_POINTER, 0, 0, HK_OFFSET_OF(hkAnimationContainer,m_animations) },
	{ "bindings", &hkAnimationBindingClass, HK_NULL, hkClassMember::TYPE_SIMPLEARRAY, hkClassMember::TYPE_POINTER, 0, 0, HK_OFFSET_OF(hkAnimationContainer,m_bindings) },
	{ "attachments", &hkBoneAttachmentClass, HK_NULL, hkClassMember::TYPE_SIMPLEARRAY, hkClassMember::TYPE_POINTER, 0, 0, HK_OFFSET_OF(hkAnimationContainer,m_attachments) },
	{ "skins", &hkMeshBindingClass, HK_NULL, hkClassMember::TYPE_SIMPLEARRAY, hkClassMember::TYPE_POINTER, 0, 0, HK_OFFSET_OF(hkAnimationContainer,m_skins) }
};
extern const hkClass hkAnimationContainerClass;
const hkClass hkAnimationContainerClass(
	"hkAnimationContainer",
	HK_NULL, // parent
	sizeof(hkAnimationContainer),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkAnimationContainerClass_Members),
	int(sizeof(hkAnimationContainerClass_Members)/sizeof(hkInternalClassMember)),
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
