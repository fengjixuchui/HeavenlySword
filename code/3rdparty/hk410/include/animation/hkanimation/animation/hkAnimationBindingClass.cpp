/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkanimation/animation/hkAnimationBinding.h'

#include <hkanimation/hkAnimation.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkanimation/animation/hkAnimationBinding.h>


// External pointer and enum types
extern const hkClass hkSkeletalAnimationClass;

//
// Enum hkAnimationBinding::BlendHint
//
static const hkInternalClassEnumItem hkAnimationBindingBlendHintEnumItems[] =
{
	{0, "NORMAL"},
	{1, "ADDITIVE"},
};
static const hkInternalClassEnum hkAnimationBindingEnums[] = {
	{"BlendHint", hkAnimationBindingBlendHintEnumItems, 2 }
};
extern const hkClassEnum* hkAnimationBindingBlendHintEnum = reinterpret_cast<const hkClassEnum*>(&hkAnimationBindingEnums[0]);

//
// Class hkAnimationBinding
//
static const hkInternalClassMember hkAnimationBindingClass_Members[] =
{
	{ "animation", &hkSkeletalAnimationClass, HK_NULL, hkClassMember::TYPE_POINTER, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkAnimationBinding,m_animation) },
	{ "animationTrackToBoneIndices", HK_NULL, HK_NULL, hkClassMember::TYPE_SIMPLEARRAY, hkClassMember::TYPE_INT16, 0, 0, HK_OFFSET_OF(hkAnimationBinding,m_animationTrackToBoneIndices) },
	{ "blendHint", HK_NULL, hkAnimationBindingBlendHintEnum, hkClassMember::TYPE_ENUM, hkClassMember::TYPE_VOID, 0, hkClassMember::ENUM_8, HK_OFFSET_OF(hkAnimationBinding,m_blendHint) }
};
namespace
{
	struct hkAnimationBinding_DefaultStruct
	{
		int s_defaultOffsets[3];
		typedef hkInt8 _hkBool;
		typedef hkReal _hkVector4[4];
		typedef hkReal _hkQuaternion[4];
		typedef hkReal _hkMatrix3[12];
		typedef hkReal _hkRotation[12];
		typedef hkReal _hkQsTransform[12];
		typedef hkReal _hkMatrix4[16];
		typedef hkReal _hkTransform[16];
		hkInt8 /* enum BlendHint */ m_blendHint;
	};
	const hkAnimationBinding_DefaultStruct hkAnimationBinding_Default =
	{
		{-1,-1,HK_OFFSET_OF(hkAnimationBinding_DefaultStruct,m_blendHint)},
		0
	};
}
extern const hkClass hkAnimationBindingClass;
const hkClass hkAnimationBindingClass(
	"hkAnimationBinding",
	HK_NULL, // parent
	sizeof(hkAnimationBinding),
	HK_NULL,
	0, // interfaces
	reinterpret_cast<const hkClassEnum*>(hkAnimationBindingEnums),
	1, // enums
	reinterpret_cast<const hkClassMember*>(hkAnimationBindingClass_Members),
	int(sizeof(hkAnimationBindingClass_Members)/sizeof(hkInternalClassMember)),
	&hkAnimationBinding_Default
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
