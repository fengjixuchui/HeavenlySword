/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKANIMATION_MOTION_ANIMATED_REFERENCE_FRAME_UTILS_H
#define HKANIMATION_MOTION_ANIMATED_REFERENCE_FRAME_UTILS_H

/// Extracts motion data from an animation track
class hkAnimatedReferenceFrameUtils
{
public:

	/// This routine transforms animated data into the space of the animated reference frame.
	static void transformIntoAnimatedReferenceFrame( const class hkAnimatedReferenceFrame* motion, class hkQsTransform* transformsInOut, int m_numTransforms, int stride);
};

#endif // HKANIMATION_MOTION_ANIMATED_REFERENCE_FRAME_UTILS_H

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
