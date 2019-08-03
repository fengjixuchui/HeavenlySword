/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_TRACK_ANALYSIS_H
#define HK_TRACK_ANALYSIS_H

#include <hkanimation/animation/hkSkeletalAnimation.h>

class hkTrackAnalysis
{
	public:

			/// The input structure for analysing animation tracks
			/// Tolerances are conservative i.e. if both absolute and relative 
			/// tolerances are specified then the position must be within both
		struct AnalysisInput
		{
			const hkQsTransform* m_boneData;
			hkUint32 m_numberOfTracks;
			hkUint32 m_numberOfPoses;

			hkReal m_absolutePositionTolerance;	// Set to 0 to use only relative tolerance
			hkReal m_relativePositionTolerance; // Set to 0 to use only abs tolerance
			hkReal m_rotationTolerance;
			hkReal m_scaleTolerance;
		};

		struct PerBoneAnalysisInput
		{
			const hkQsTransform* m_boneData;
			hkUint32 m_numberOfTracks;
			hkUint32 m_numberOfPoses;

			hkArray<hkReal> m_absolutePositionTolerance;	// Set to 0 to use only relative tolerance
			hkArray<hkReal> m_relativePositionTolerance; // Set to 0 to use only abs tolerance
			hkArray<hkReal> m_rotationTolerance;
			hkArray<hkReal> m_scaleTolerance;
		};
		
		enum TrackType 
		{
			HK_TRACK_DYNAMIC = 0x00, // This track should use the dynamic value
			HK_TRACK_STATIC	 = 0x01, // This track should use the static value
			HK_TRACK_CLEAR   = 0x02	 // This track can be cleared / set to the identity
		};

			/// This method analyzes the input animation to find and 
			/// record static degrees of freedom in the animation.
			/// The method fills data into the arrays staticMask and staticData
			/// Both must be preallocated and at least m_numberOfTracks long.
			/// The hkUint16 in the static mask is laid out so :
			/// Bits 0..1 = Track Type for Position
			/// Bits 2..3 = Track Type for Rotation
			/// Bits 4..5 = Track Type for Scale
			/// Bits 6..8 = Z,Y,X flags for dynamic positions 
			/// Bits 9..12 = W,Z,Y,X flags for dynamic rotations
			/// Bits 13..15 = Z,Y,X flags for dynamic scales
		static void HK_CALL analyze( const AnalysisInput& input, hkUint16* staticMask, hkReal* staticDOFs );

		static void HK_CALL analyze( const PerBoneAnalysisInput& input, hkUint16* staticMask, hkReal* staticDOFs );

			/// An output structure used to gather statistics on an analysed track mask
		struct AnalysisStats
		{
			// Number of static tracks
			hkUint32 m_staticPosDOF;
			hkUint32 m_staticRotDOF;
			hkUint32 m_staticSclDOF;

			// Number of clear tracks
			hkUint32 m_clearPosDOF;
			hkUint32 m_clearRotDOF;
			hkUint32 m_clearSclDOF;

			// Number of dynamic degrees of freedom
			hkUint32 m_dynamicPosDOF;
			hkUint32 m_dynamicRotDOF;
			hkUint32 m_dynamicSclDOF;

			// Index of the first and last dynamic DOF of a target track
			hkUint32 m_dynamicDOFstart;
			hkUint32 m_dynamicDOFend;

			// Index of the first static DOF of a target track
			hkUint32 m_staticDOFstart;

			// Ratio of total tracks to dyanmic tracks
			hkReal	 m_ratio; 
		};

			/// This method computes the statistics in the AnalysisStats struct for a given static mask.
		static void HK_CALL calcStats(const hkUint16* staticMask, hkUint32 numTracks, AnalysisStats& out);

			/// This method computes information for the dynamic tracks
		static void HK_CALL calcDynamicStats(const hkUint16* staticMask, hkUint32 numTracks, AnalysisStats& out);
		
		struct AnalysisSplit
		{
			const hkQsTransform* m_data;
			hkUint32 m_numberOfTracks;
			hkUint32 m_numberOfPoses;
			hkUint32 m_channelAlign;
			const hkUint16* m_staticMask;
		};

		static void HK_CALL channelSplit(const AnalysisSplit& input, hkReal* channelData);

		struct AnalysisCompose
		{
			hkUint32 m_numberOfTracks;
			const hkUint16* m_staticMask;
			const hkReal* m_staticDOFs;
		};

		static void HK_CALL recompose(const AnalysisCompose& input, const hkReal* dynamicData, hkQsTransform* outputPose);
};


#endif // HK_TRACK_ANALYSIS_H

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
