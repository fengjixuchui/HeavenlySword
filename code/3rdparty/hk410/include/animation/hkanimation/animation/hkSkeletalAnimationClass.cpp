/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkanimation/animation/hkSkeletalAnimation.h'

#include <hkanimation/hkAnimation.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkanimation/animation/hkSkeletalAnimation.h>


// External pointer and enum types
extern const hkClass hkAnimatedReferenceFrameClass;
extern const hkClass hkAnnotationTrackClass;
extern const hkClassEnum* hkSkeletalAnimationTypeEnum;

//
// Enum hkSkeletalAnimation::Type
//
static const hkInternalClassEnumItem hkSkeletalAnimationTypeEnumItems[] =
{
	{0, "HK_UNKNOWN_ANIMATION"},
	{1, "HK_INTERLEAVED_ANIMATION"},
	{2, "HK_DELTA_COMPRESSED_ANIMATION"},
	{3, "HK_WAVELET_COMPRESSED_ANIMATION"},
};
static const hkInternalClassEnum hkSkeletalAnimationEnums[] = {
	{"Type", hkSkeletalAnimationTypeEnumItems, 4 }
};
extern const hkClassEnum* hkSkeletalAnimationTypeEnum = reinterpret_cast<const hkClassEnum*>(&hkSkeletalAnimationEnums[0]);

//
// Class hkSkeletalAnimation
//
const hkInternalClassMember hkSkeletalAnimation::Members[] =
{
	{ "type", HK_NULL, hkSkeletalAnimationTypeEnum, hkClassMember::TYPE_ENUM, hkClassMember::TYPE_VOID, 0, hkClassMember::ENUM_32, HK_OFFSET_OF(hkSkeletalAnimation,m_type) },
	{ "duration", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkSkeletalAnimation,m_duration) },
	{ "numberOfTracks", HK_NULL, HK_NULL, hkClassMember::TYPE_INT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkSkeletalAnimation,m_numberOfTracks) },
	{ "extractedMotion", &hkAnimatedReferenceFrameClass, HK_NULL, hkClassMember::TYPE_POINTER, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkSkeletalAnimation,m_extractedMotion) },
	{ "annotationTracks", &hkAnnotationTrackClass, HK_NULL, hkClassMember::TYPE_SIMPLEARRAY, hkClassMember::TYPE_POINTER, 0, 0, HK_OFFSET_OF(hkSkeletalAnimation,m_annotationTracks) }
};
extern const hkClass hkReferencedObjectClass;

extern const hkClass hkSkeletalAnimationClass;
const hkClass hkSkeletalAnimationClass(
	"hkSkeletalAnimation",
	&hkReferencedObjectClass, // parent
	sizeof(hkSkeletalAnimation),
	HK_NULL,
	0, // interfaces
	reinterpret_cast<const hkClassEnum*>(hkSkeletalAnimationEnums),
	1, // enums
	reinterpret_cast<const hkClassMember*>(hkSkeletalAnimation::Members),
	int(sizeof(hkSkeletalAnimation::Members)/sizeof(hkInternalClassMember)),
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
