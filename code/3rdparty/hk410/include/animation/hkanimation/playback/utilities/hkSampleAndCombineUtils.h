/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_BLEND_UTILS_H
#define HK_BLEND_UTILS_H

#include <hkbase/hkBase.h>

class hkSampleAndCombineUtils
{
	
	public:

	struct BlendParameters
	{
		const hkQsTransform* m_animationTransformsIn;
		int m_numAnimationTransforms;
		int m_numBones;
		hkReal m_masterWeight;
		const hkUint8* m_perBoneWeights;
		const hkInt16* m_trackToBoneMapping;

		BlendParameters() : m_animationTransformsIn(HK_NULL),
							m_numAnimationTransforms(0),
							m_numBones(0),
							m_masterWeight(1.0f),
							m_perBoneWeights(HK_NULL),
							m_trackToBoneMapping(HK_NULL)
		{}
	};

	static void HK_CALL blendNormal(BlendParameters& paramsIn, hkQsTransform* accumulationPoseInOut, hkReal* perBoneWeights);
	
	static void HK_CALL blendAdditive(BlendParameters& paramsIn, hkQsTransform* accumulationPoseInOut);

	static hkUint32 HK_CALL getMaxTrack(const hkInt16* trackToBoneMapping, const hkUint8* perBoneWeights, hkUint32 numBones, hkUint32 maxTracks);
};


#endif // HK_BLEND_UTILS_H

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
