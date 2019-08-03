/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkanimation/animation/waveletcompressed/hkWaveletSkeletalAnimation.h'

#include <hkanimation/hkAnimation.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkanimation/animation/waveletcompressed/hkWaveletSkeletalAnimation.h>


// External pointer and enum types
extern const hkClass PerTrackCompressionParamsintClass;
extern const hkClass hkWaveletSkeletalAnimationCompressionParamsClass;
extern const hkClass hkWaveletSkeletalAnimationQuantizationFormatClass;

//
// Class hkWaveletSkeletalAnimation::QuantizationFormat
//
static const hkInternalClassMember hkWaveletSkeletalAnimation_QuantizationFormatClass_Members[] =
{
	{ "maxBitWidth", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT8, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWaveletSkeletalAnimation::QuantizationFormat,m_maxBitWidth) },
	{ "preserved", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT8, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWaveletSkeletalAnimation::QuantizationFormat,m_preserved) },
	{ "numD", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWaveletSkeletalAnimation::QuantizationFormat,m_numD) },
	{ "offsetIdx", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWaveletSkeletalAnimation::QuantizationFormat,m_offsetIdx) },
	{ "scaleIdx", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWaveletSkeletalAnimation::QuantizationFormat,m_scaleIdx) },
	{ "bitWidthIdx", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWaveletSkeletalAnimation::QuantizationFormat,m_bitWidthIdx) }
};
const hkClass hkWaveletSkeletalAnimationQuantizationFormatClass(
	"hkWaveletSkeletalAnimationQuantizationFormat",
	HK_NULL, // parent
	sizeof(hkWaveletSkeletalAnimation::QuantizationFormat),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkWaveletSkeletalAnimation_QuantizationFormatClass_Members),
	int(sizeof(hkWaveletSkeletalAnimation_QuantizationFormatClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkWaveletSkeletalAnimation
//
static const hkInternalClassMember hkWaveletSkeletalAnimationClass_Members[] =
{
	{ "numberOfPoses", HK_NULL, HK_NULL, hkClassMember::TYPE_INT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWaveletSkeletalAnimation,m_numberOfPoses) },
	{ "blockSize", HK_NULL, HK_NULL, hkClassMember::TYPE_INT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWaveletSkeletalAnimation,m_blockSize) },
	{ "qFormat", &hkWaveletSkeletalAnimationQuantizationFormatClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWaveletSkeletalAnimation,m_qFormat) },
	{ "staticMaskIdx", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWaveletSkeletalAnimation,m_staticMaskIdx) },
	{ "staticDOFsIdx", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWaveletSkeletalAnimation,m_staticDOFsIdx) },
	{ "blockIndexIdx", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWaveletSkeletalAnimation,m_blockIndexIdx) },
	{ "blockIndexSize", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWaveletSkeletalAnimation,m_blockIndexSize) },
	{ "quantizedDataIdx", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWaveletSkeletalAnimation,m_quantizedDataIdx) },
	{ "quantizedDataSize", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkWaveletSkeletalAnimation,m_quantizedDataSize) },
	{ "dataBuffer", HK_NULL, HK_NULL, hkClassMember::TYPE_SIMPLEARRAY, hkClassMember::TYPE_UINT8, 0, 0, HK_OFFSET_OF(hkWaveletSkeletalAnimation,m_dataBuffer) }
};
extern const hkClass hkSkeletalAnimationClass;

extern const hkClass hkWaveletSkeletalAnimationClass;
const hkClass hkWaveletSkeletalAnimationClass(
	"hkWaveletSkeletalAnimation",
	&hkSkeletalAnimationClass, // parent
	sizeof(hkWaveletSkeletalAnimation),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkWaveletSkeletalAnimationClass_Members),
	int(sizeof(hkWaveletSkeletalAnimationClass_Members)/sizeof(hkInternalClassMember)),
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
