/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkanimation/hkAnimation.h>
#include <hkanimation/animation/util/hkAdditiveAnimationUtility.h>

void HK_CALL hkAdditiveAnimationUtility::createAdditiveFromPose( const Input& input, hkQsTransform* deltaOut)
{
	for (int p = (input.m_numberOfPoses - 1); p >= 0 ; p--)
	{
		for (int t=0; t < input.m_numberOfTracks; ++t)
		{
			const hkQsTransform& qIn = input.m_originalData[ p * input.m_numberOfTracks + t ];
			hkQsTransform& qOut = deltaOut[ p * input.m_numberOfTracks + t ];
			qOut.setMulMulInverse( qIn, input.m_baseData[t] );
		}
	}
}

void HK_CALL hkAdditiveAnimationUtility::createAdditiveFromAnimation( const Input& input, hkQsTransform* deltaOut)
{
	for (int p = (input.m_numberOfPoses - 1); p >= 0 ; p--)
	{
		for (int t=0; t < input.m_numberOfTracks; ++t)
		{
			const hkQsTransform& qIn = input.m_originalData[ p * input.m_numberOfTracks + t ];
			hkQsTransform& qOut = deltaOut[ p * input.m_numberOfTracks + t ];
			qOut.setMulMulInverse( qIn, input.m_baseData[p * input.m_numberOfTracks + t] );
		}
	}
}



void HK_CALL hkAdditiveAnimationUtility::createAdditiveFromReferencePose( const ReferencePoseInput& input, hkQsTransform* deltaOut)
{
	HK_ASSERT2(0x543e343e, input.m_numberOfTracks == input.m_numAnimationTrackToBoneIndices, "Mapping from tracks to bone indices does not match input");

	for (int p = (input.m_numberOfPoses - 1); p >= 0 ; p--)
	{
		for (int t=0; t < input.m_numberOfTracks; ++t)
		{
			const hkQsTransform& qIn = input.m_originalData[ p * input.m_numberOfTracks + t ];
			hkQsTransform& qOut = deltaOut[ p * input.m_numberOfTracks + t ];
			qOut.setMulMulInverse( qIn, input.m_referencePose[ input.m_animationTrackToBoneIndices[t] ] );
		}
	}
}

/*
* Havok SDK - CLIENT RELEASE, BUILD(#20060902)
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
