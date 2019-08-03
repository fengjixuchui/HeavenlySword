/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_ADDITIVE_ANIMATION_UTILITY_H
#define HK_ADDITIVE_ANIMATION_UTILITY_H

/// This utility creates an additive animation for an input animation and a base animation
/// The additive animation is created by 'subtracting' the base from the original to produce delta values.
/// There are two common forms supplied. The first subtracts a single pose from the original data.
/// The second subtracts an enitre animation.
/// A third less common form is also supplied which allows an animation to be subtracted from the reference 
/// pose of a character.
/// NOTE: When using these utilities you must still set the blend hint flag in the corresponding 
/// animation binding to hkAnimationBinding::ADDITIVE;
class hkAdditiveAnimationUtility
{
	public:

		struct Input
		{
				/// The number of tracks in the animation
			int m_numberOfTracks;					
				/// The number of poses (usually equates to number of frames)
			int m_numberOfPoses;
				/// The base data to remove. 
				/// This data should be either m_numberOfTracks or m_numberOfTracks * m_numberOfAnimationPoses elements in length depending on which create call is used 
			const hkQsTransform* m_baseData;
				/// The original data
			const hkQsTransform* m_originalData;
		};
				
			/// This creates an additive animation by subtracting the single pose stored in 'baseData' 
			/// from each of the poses passed in the the original data
			/// The result is stored in 'deltaOut'.
			/// This function is alias safe so you can reuse the same buffer for input and output if desired
		static void HK_CALL createAdditiveFromPose( const Input& input, hkQsTransform* deltaOut);

			/// This creates an additive animation by subtracting the animation passed in 'baseData'
			/// from each of the poses passed in the original data.
			/// The result is stored in 'deltaOut'.
			/// This function is alias safe so you can reuse the same buffer for input and output if desired
		static void HK_CALL createAdditiveFromAnimation( const Input& input, hkQsTransform* deltaOut);


		struct ReferencePoseInput
		{
				/// The number of tracks in the animation
			int m_numberOfTracks;					
				/// The number of poses (usually equates to number of frames)
			int m_numberOfPoses;
				/// The original data
			const hkQsTransform* m_originalData;

				/// The reference pose for the character
			hkQsTransform* m_referencePose;
			int m_numReferencePose;

				/// A mapping from track numbers to bone indices
			hkInt16* m_animationTrackToBoneIndices;
			hkInt32 m_numAnimationTrackToBoneIndices;
		};

			/// This creates an additive animation by subtracting the animation passed in 'baseData'
			/// from each of the poses passed in the original data.
			/// The result is stored in 'deltaOut'.
			/// This function is alias safe so you can reuse the same buffer for input and output if desired
		static void HK_CALL createAdditiveFromReferencePose( const ReferencePoseInput& input, hkQsTransform* deltaOut);

};


#endif // HK_ADDITIVE_ANIMATION_UTILITY_H

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
