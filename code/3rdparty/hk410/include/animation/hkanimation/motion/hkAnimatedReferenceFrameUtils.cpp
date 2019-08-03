/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkanimation/hkAnimation.h>
#include <hkanimation/motion/default/hkDefaultAnimatedReferenceFrame.h>
#include <hkanimation/motion/hkAnimatedReferenceFrameUtils.h>


void hkAnimatedReferenceFrameUtils::transformIntoAnimatedReferenceFrame( const hkAnimatedReferenceFrame* motion, hkQsTransform* transformsInOut, int m_numTransforms, int stride)
{
	const hkReal timeSlice = motion->getDuration() / (m_numTransforms  - 1);

	for (int t=0; t < m_numTransforms; t++)
	{
		hkQsTransform& animx = *reinterpret_cast<hkQsTransform*>( reinterpret_cast<char*>(transformsInOut) + stride * t );

		// Now, lets take a look at the motion we extracted
		const hkReal time = t * timeSlice;
		hkQsTransform motionx;
		motion->getReferenceFrame(time, motionx);

		// ORIGINAL_ROOT = MOTION * NEW_ROOT, so
		// NEW_ROOT = inv(MOTION) * ORIGINAL_ROOT
		hkQsTransform temp;
		temp.setMulInverseMul(motionx, animx);

		animx = temp;
	}
}



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
