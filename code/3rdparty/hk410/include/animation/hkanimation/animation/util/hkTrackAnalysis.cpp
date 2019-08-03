/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkanimation/hkAnimation.h>
#include <hkanimation/animation/util/hkTrackAnalysis.h>

#ifndef HK_PLATFORM_PS3SPU

#include <hkbase/memory/hkLocalArray.h>

#define _MASK_POS_BITS      ((hkUint16)3)
#define _MASK_ROT_BITS      ((hkUint16)12)
#define _MASK_SCALE_BITS    ((hkUint16)48)
#define _INVMASK_POS_BITS   (~(_MASK_POS_BITS))
#define _INVMASK_ROT_BITS   (~(_MASK_ROT_BITS))
#define _INVMASK_SCALE_BITS (~(_MASK_SCALE_BITS))

// As the asset may be stored to file after analyze
// and used in a different SIMD build type, the
// component mask may not be as was first seen, so 
// we use the more common connevtion of XYZW (PC FPU, PS2, etc) rather than WZYX (PC SIMD)
#if defined(HK_ARCH_IA32) && (HK_CONFIG_SIMD==HK_CONFIG_SIMD_ENABLED)
	#define USE_REVERSED_MASK 1
#endif

static hkUint8 _flipTable[16] = 
{
	// 0,  1,   2,  3,   4,   5,   6,   7,   8,   9,    A,   B,   C,   D,    E,  F,
 	0x0, 0x8, 0x4, 0xC, 0x2, 0xA, 0x6, 0xE, 0x1, 0x9, 0x5, 0xD, 0x3, 0xB, 0x7, 0xF
};

inline hkUint16 _flipXYZW(int m)
{
	// only lower 4 bits used, so m in the range 0 to 15
	return _flipTable[m];
}

void hkTrackAnalysis::analyze(const hkTrackAnalysis::AnalysisInput& input, hkUint16* staticMask, hkReal* staticDOFs )
{
	hkTrackAnalysis::PerBoneAnalysisInput perBoneInput;
	perBoneInput.m_boneData = input.m_boneData;
	perBoneInput.m_numberOfTracks = input.m_numberOfTracks;
	perBoneInput.m_numberOfPoses = input.m_numberOfPoses;

	int N = perBoneInput.m_numberOfTracks;

	perBoneInput.m_absolutePositionTolerance.setSize(N);
	perBoneInput.m_relativePositionTolerance.setSize(N);
	perBoneInput.m_rotationTolerance.setSize(N);
	perBoneInput.m_scaleTolerance.setSize(N);

	for (int i=0; i<N; i++)
	{
		perBoneInput.m_absolutePositionTolerance[i] = input.m_absolutePositionTolerance;
		perBoneInput.m_relativePositionTolerance[i] = input.m_relativePositionTolerance;
		perBoneInput.m_rotationTolerance[i] = input.m_rotationTolerance;
		perBoneInput.m_scaleTolerance[i] = input.m_scaleTolerance;
	}

	analyze(perBoneInput, staticMask, staticDOFs);
}


void hkTrackAnalysis::analyze(const hkTrackAnalysis::PerBoneAnalysisInput& input, hkUint16* staticMask, hkReal* staticDOFs )
{
	hkLocalArray<hkQsTransform> avgData(input.m_numberOfTracks);
	avgData.setSize(input.m_numberOfTracks);

	// Iterate through the bones and calculate the relative tolerance
	hkVector4 relPosTolerance;
	relPosTolerance.setAll(0.0f);
	{
		hkVector4 minPos; minPos.setAll3(HK_REAL_MAX);
		hkVector4 maxPos; maxPos.setAll3(-HK_REAL_MAX);

		for (hkUint32 t=0; t < input.m_numberOfTracks; t++)
		{
			for (hkUint32 p=0; p< input.m_numberOfPoses; p++)
			{
				hkVector4 pos = input.m_boneData[p * input.m_numberOfTracks + t].m_translation;
				minPos.setMin4( minPos, pos);
				maxPos.setMax4( maxPos, pos);
			}
		}
		relPosTolerance.setSub4( maxPos, minPos );
	}

	// Iterate through the tracks and fill in the bit array
	for (hkUint32 t=0; t < input.m_numberOfTracks; t++)
	{

		const hkReal absolutePositionTolerance = input.m_absolutePositionTolerance[t];
		const hkReal relativePositionTolerance = input.m_relativePositionTolerance[t];
		const hkReal rotationTolerance = input.m_rotationTolerance[t];
		const hkReal scaleTolerance = input.m_scaleTolerance[t];
		const hkSimdReal rptSquared = relativePositionTolerance * relativePositionTolerance;

		const bool ignoreRelativeTolerance = ( relativePositionTolerance > 0) &&
			( relPosTolerance.lengthSquared3() * rptSquared  < HK_REAL_EPSILON) ;

		// Calculate averages for this track
		avgData[t].setZero();

		{
			for (hkUint32 p=0; p< input.m_numberOfPoses; p++)
			{
				const hkQsTransform& bt = input.m_boneData[p * input.m_numberOfTracks + t];
				avgData[t].blendAddMul(bt);
			}

			avgData[t].blendNormalize (1.0f * input.m_numberOfPoses);
		}

		// Now compare the track to the average
		const hkVector4& meanPos = avgData[t].getTranslation();
		const hkVector4& meanQuat = avgData[t].getRotation().m_vec;
		const hkVector4& meanScale = avgData[t].getScale();

		{
			// Assume all are static
			hkUint16 trackMask = (HK_TRACK_STATIC << 4) | (HK_TRACK_STATIC << 2) | HK_TRACK_STATIC;

			// Check position
			if ((relativePositionTolerance > 0.0f) || (absolutePositionTolerance > 0.0f))
			{
				// Check this track against the average
				for (hkUint32 p=0; p < input.m_numberOfPoses; p++)
				{
					const hkQsTransform& bt = input.m_boneData[p * input.m_numberOfTracks + t];

					hkVector4 absDiff;
					absDiff.setSub4( meanPos, bt.m_translation );
					absDiff.setAbs4( absDiff );

					hkVector4 aComp; aComp.setAll3( absolutePositionTolerance );
					hkVector4 rComp; 
					rComp.setMul4(relativePositionTolerance, relPosTolerance);

					int aMask = absDiff.compareLessThanEqual4( aComp ) & HK_VECTOR3_COMPARE_MASK_XYZ;
					int rMask = absDiff.compareLessThanEqual4( rComp ) & HK_VECTOR3_COMPARE_MASK_XYZ;

					if ( absolutePositionTolerance > 0.0f)
					{
						if (aMask != HK_VECTOR3_COMPARE_MASK_XYZ)
						{
							trackMask &= _INVMASK_POS_BITS; // Flag position as dynamic

						#ifdef USE_REVERSED_MASK 
							trackMask  |= _flipXYZW(~aMask & HK_VECTOR3_COMPARE_MASK_XYZ) << 5; 
						#else
							trackMask  |= (~aMask & HK_VECTOR3_COMPARE_MASK_XYZ) << 5; // And not 6, since we don't use W (bit 0)
						#endif
						}
					}

					if ( !ignoreRelativeTolerance && (relativePositionTolerance > 0.0f) )
					{
						if (rMask != HK_VECTOR3_COMPARE_MASK_XYZ)
						{
							trackMask &= _INVMASK_POS_BITS; // Flag position as dynamic
						#ifdef USE_REVERSED_MASK 
							trackMask  |= _flipXYZW(~rMask & HK_VECTOR3_COMPARE_MASK_XYZ) << 5; 
						#else
							trackMask  |= (~rMask & HK_VECTOR3_COMPARE_MASK_XYZ) << 5; // And not 6, since we don't use W (bit 0)
						#endif
						}
					}
				}				
			}
			else
			{
				// tolerance is 0
				trackMask &=_INVMASK_POS_BITS; // Flag position as dynamic
				trackMask |= (0x7) << 6; // All 3 components 
			}

			// Check orientation
			if (rotationTolerance > 0.0f)
			{
				// Check this track against the average
				for (hkUint32 p=0; p < input.m_numberOfPoses; p++)
				{
					const hkQsTransform& bt = input.m_boneData[p * input.m_numberOfTracks + t];
					const hkVector4& thisQuat = bt.getRotation().m_vec;
										
					if (!thisQuat.equals4(meanQuat, rotationTolerance))
					{
						hkVector4 absDiff;
						absDiff.setSub4( meanQuat, bt.m_rotation.m_vec );
						absDiff.setAbs4( absDiff );

						hkVector4 aComp; aComp.setAll( rotationTolerance );

						int aMask = absDiff.compareLessThanEqual4( aComp ) & HK_VECTOR3_COMPARE_MASK_XYZW;

						if (aMask != HK_VECTOR3_COMPARE_MASK_XYZW)
						{
							trackMask &= _INVMASK_ROT_BITS; // Flag rotation as dynamic
						#ifdef USE_REVERSED_MASK 
							trackMask  |= _flipXYZW(~aMask & HK_VECTOR3_COMPARE_MASK_XYZW) << 9;
						#else
							trackMask  |= (~aMask & HK_VECTOR3_COMPARE_MASK_XYZW) << 9;
						#endif
						}
					}
				}
			}
			else
			{
				// tolerance is 0
				trackMask &= _INVMASK_ROT_BITS;	// Flag quaternion as dynamic
				trackMask |= (0xf) << 9; // All 4 components 
			}

			// Check scale
			if ((scaleTolerance > 0.0f) )
			{
				// Check this track against the average
				for (hkUint32 p=0; p < input.m_numberOfPoses; p++)
				{
					const hkQsTransform& bt = input.m_boneData[p * input.m_numberOfTracks + t];

					hkVector4 absDiff;
					absDiff.setSub4( meanScale, bt.getScale() );
					absDiff.setAbs4( absDiff );

					hkVector4 aComp; aComp.setAll3( scaleTolerance );

					int aMask = absDiff.compareLessThanEqual4( aComp ) & HK_VECTOR3_COMPARE_MASK_XYZ;

					if ( scaleTolerance > 0.0f)
					{
						if (aMask != HK_VECTOR3_COMPARE_MASK_XYZ)
						{
							trackMask &= _INVMASK_SCALE_BITS; // Flag scale as dynamic
						#ifdef USE_REVERSED_MASK 
							trackMask  |= _flipXYZW(~aMask & HK_VECTOR3_COMPARE_MASK_XYZ) << 12; 
						#else	
							trackMask  |= (~aMask & HK_VECTOR3_COMPARE_MASK_XYZ) << 12; // And not 13, since we don't use W (bit 0
						#endif
						}
					}

				}				
			}
			else
			{
				// tolerance is 0
				trackMask &= _INVMASK_SCALE_BITS; // Flag scale as dynamic
				trackMask |= (0x7) << 13; // All 3 components 
			}

			staticMask[t] = trackMask;
		}

		// Check if the track is the identity
		{
			// Check position
			if (((staticMask[t] & _MASK_POS_BITS) == HK_TRACK_STATIC) && 
				(((absolutePositionTolerance > 0.0f) && (meanPos.equals3( hkVector4::getZero(), absolutePositionTolerance*0.5f ))) ||
				 ((relativePositionTolerance > 0.0f) && (meanPos.equals3( hkVector4::getZero(), relPosTolerance( relPosTolerance.getMajorAxis()) * 0.5f * relativePositionTolerance )))))
			{
				staticMask[t]  = hkUint16((staticMask[t] & _INVMASK_POS_BITS) | HK_TRACK_CLEAR);
				avgData[t].m_translation.setZero4();
			}

			// Check orientation 
			if ( ((staticMask[t] & _MASK_ROT_BITS) == (HK_TRACK_STATIC << 2) ) && 
				(hkMath::fabs(meanQuat(3) - 1.0f) <= rotationTolerance*0.5f ) )
			{
				staticMask[t]  = hkUint16((staticMask[t] & _INVMASK_ROT_BITS) | (HK_TRACK_CLEAR << 2));
				avgData[t].m_rotation.setIdentity();
			}	

			hkVector4 one; one = hkQuadReal1111;
			// Check scale
			if ( ((staticMask[t] & _MASK_SCALE_BITS) == (HK_TRACK_STATIC << 4) )
				 && 
				 (meanScale.equals3(one, scaleTolerance * 0.5f)) )
			{
				staticMask[t]  = hkUint16((staticMask[t] & _INVMASK_SCALE_BITS) | (HK_TRACK_CLEAR << 4));
				avgData[t].m_scale.setAll(1.0f);
			}
		}
	}

	// Fill out the staticDOFs
	hkReal* sDof = staticDOFs;
	for (hkUint32 tr=0; tr < input.m_numberOfTracks; tr++)
	{
		const hkUint16 trackMask = staticMask[tr];
		const hkUint16 bitMask = hkUint16(trackMask >> 6);

		// Each track has potentially 10 degress of freedom
		// 3 pos, 4 rot, 3 scl.
		
		// Position
		if ( (trackMask & 0x3) != hkTrackAnalysis::HK_TRACK_CLEAR)
		{
			for (int d=0; d < 3; d++)
			{
				// Bit 0 = z, Bit 1 = y
				const int bm = bitMask & (1 << (2-d)); // always in standard XYZW form, so no need to reversee 
				if (bm == 0)
				{			
					*sDof++ = avgData[tr].m_translation(d);
				}
			}
		}

		// Rotation
		if ( ((trackMask >> 2) & 0x3) != hkTrackAnalysis::HK_TRACK_CLEAR)
		{
			for (int d=0; d < 4; d++)
			{
				// Bit 0 = w, Bit 1 = y
				const int bm = bitMask & (1 << ((3-d)+3)); // always in standard XYZW form, so no need to reversee 
				if (bm == 0)
				{					
					*sDof++ = avgData[tr].m_rotation(d);
				}
			}
		}

		// Scale
		if (((trackMask >> 4) & 0x3) != hkTrackAnalysis::HK_TRACK_CLEAR)
		{
			for (int d=0; d < 3; d++)
			{
				const int bm = bitMask & (1 << ((2-d)+7)); // always in standard XYZW form, so no need to reversee 
				if (bm == 0)
				{					
					*sDof++ = avgData[tr].m_scale(d);
				}
			}
		}
	}

	hkTrackAnalysis::AnalysisStats stats;
	calcStats( staticMask, input.m_numberOfTracks, stats);

	HK_REPORT_SECTION_BEGIN(0x432434a4, "Track Analysis");
	HK_REPORT("Total DOFs " << input.m_numberOfTracks * 10);
	HK_REPORT("Static position DOFs " << stats.m_staticPosDOF);
	HK_REPORT("Static rotation DOFs " << stats.m_staticRotDOF);
	HK_REPORT("Static scale DOFs " << stats.m_staticSclDOF);
	HK_REPORT("Clear position DOFs " << stats.m_clearPosDOF);
	HK_REPORT("Clear rotation DOFs " << stats.m_clearRotDOF);
	HK_REPORT("Clear scale DOFs " << stats.m_clearSclDOF);
	HK_REPORT("Dynamic position DOFs " << stats.m_dynamicPosDOF);
	HK_REPORT("Dynamic rotation DOFs " << stats.m_dynamicRotDOF);
	HK_REPORT("Dynamic scale DOFs " << stats.m_dynamicSclDOF);
	HK_REPORT("Redundancy " << stats.m_ratio);
	HK_REPORT_SECTION_END();

}


void hkTrackAnalysis::calcStats(const hkUint16* mask, hkUint32 numTracks, AnalysisStats& out)
{

	out.m_staticPosDOF = 0;
	out.m_staticRotDOF = 0;
	out.m_staticSclDOF = 0;
	out.m_clearPosDOF = 0;
	out.m_clearRotDOF = 0;
	out.m_clearSclDOF = 0;
	out.m_ratio = 0.0f;

	for (hkUint32 i=0; i < numTracks; i++)
	{
		if ( (mask[i] & 0x3) == hkTrackAnalysis::HK_TRACK_CLEAR )
			out.m_clearPosDOF+=3;
		if ( ((mask[i] >> 2) & 0x3) == hkTrackAnalysis::HK_TRACK_CLEAR )
			out.m_clearRotDOF+=4;
		if ( ((mask[i] >> 4) & 0x3) == hkTrackAnalysis::HK_TRACK_CLEAR )
			out.m_clearSclDOF+=3;
	}

	calcDynamicStats(mask, numTracks, out);

	out.m_staticPosDOF = (numTracks * 3) - out.m_dynamicPosDOF - out.m_clearPosDOF;
	out.m_staticRotDOF = (numTracks * 4) - out.m_dynamicRotDOF - out.m_clearRotDOF;
	out.m_staticSclDOF = (numTracks * 3) - out.m_dynamicSclDOF - out.m_clearSclDOF;

	out.m_ratio = hkReal( numTracks * 10) / (out.m_dynamicPosDOF + out.m_dynamicRotDOF + out.m_dynamicSclDOF);
}

void hkTrackAnalysis::channelSplit(const AnalysisSplit& input, hkReal* channelData)
{
	const hkUint32 numP = input.m_numberOfPoses;
	const hkUint32 numPR = hkMath::max2(numP, input.m_channelAlign);
	const hkUint32 numT = input.m_numberOfTracks;

	// Split data into consecutive channels
	hkUint32 dIdx = 0;
	for (hkUint32 t=0; t < numT; t++)
	{
		const hkUint16 trackMask = input.m_staticMask[t];
		const hkUint16 bitMask = hkUint16(trackMask >> 6);

		// Each track has potentially 10 degress of freedom
		// 3 pos, 4 rot, 3 scl.

		// Position
		if ( (trackMask  & 0x3) == hkTrackAnalysis::HK_TRACK_DYNAMIC)
		{
			for (int d=0; d < 3; d++)
			{
				// Bit 0 = z, Bit 1 = y
				if (bitMask & (1 << (2-d))) // always in standard XYZW form, so no need to reversee 
				{					
					for (hkUint32 p=0; p < numP; p++)
					{
						const hkUint32 rawIdx  = p * numT + t;
						channelData[dIdx * numPR + p] = input.m_data[rawIdx].m_translation(d);
					}

					// Pad out the last block by repeating the last element (so we don't affect quantisation)
					const hkUint32 lastIdx  = (numP-1) * numT + t;
					for (hkUint32 r=numP; r < numPR; r++)
					{
						channelData[dIdx * numPR + r] = input.m_data[lastIdx].m_translation(d);
					}
					dIdx++;
				}
			}
		}

		// Rotation
		if ( ((trackMask >> 2) & 0x3) == hkTrackAnalysis::HK_TRACK_DYNAMIC)
		{
			for (int d=0; d < 4; d++)
			{
				// Bit 0 = w, Bit 1 = y
				if (bitMask & (1 << ((3-d)+3))) // always in standard XYZW form, so no need to reversee 
				{					
					for (hkUint32 p=0; p < numP; p++)
					{
						const hkUint32 rawIdx  = p * numT + t;
						channelData[dIdx * numPR + p] = input.m_data[rawIdx].m_rotation.m_vec(d);
					}

					// Pad out the last block by repeating the last element (so we don't affect quantisation)
					const hkUint32 lastIdx  = (numP-1) * numT + t;
					for (hkUint32 r=numP; r < numPR; r++)
					{
						channelData[dIdx * numPR + r] = input.m_data[lastIdx].m_rotation.m_vec(d);
					}
					dIdx++;
				}
			}
		}

		// Scale
		if (((trackMask >> 4) & 0x3) == hkTrackAnalysis::HK_TRACK_DYNAMIC)
		{
			for (int d=0; d < 3; d++)
			{
				if (bitMask & (1 << ((2-d)+7))) // always in standard XYZW form, so no need to reversee 
				{					

					for (hkUint32 p=0; p < numP; p++)
					{
						const hkUint32 rawIdx  = p * numT + t;
						channelData[dIdx * numPR + p] = input.m_data[rawIdx].m_scale(d);
					}

					// Pad out the last block by repeating the last element (so we don't affect quantisation)
					const hkUint32 lastIdx  = (numP-1) * numT + t;
					for (hkUint32 r=numP; r < numPR; r++)
					{
						channelData[dIdx * numPR + r] = input.m_data[lastIdx].m_scale(d);
					}
					dIdx++;
				}
			}
		}
	}
}

#endif //!HK_PLATFORM_PS3SPU

void hkTrackAnalysis::calcDynamicStats(const hkUint16* mask, hkUint32 numTracks, AnalysisStats& out)
{
	out.m_dynamicPosDOF = 0;
	out.m_dynamicRotDOF = 0;
	out.m_dynamicSclDOF = 0;

	for (hkUint32 j=0; j < numTracks; j++)
	{
		hkUint32 detail = (mask[j] >> 6);

		// Pos
		out.m_dynamicPosDOF += (detail >> 0) & 0x1;
		out.m_dynamicPosDOF += (detail >> 1) & 0x1;
		out.m_dynamicPosDOF += (detail >> 2) & 0x1;

		// Rot
		out.m_dynamicRotDOF += (detail >> 3) & 0x1;
		out.m_dynamicRotDOF += (detail >> 4) & 0x1;
		out.m_dynamicRotDOF += (detail >> 5) & 0x1;
		out.m_dynamicRotDOF += (detail >> 6) & 0x1;

		// Scale
		out.m_dynamicSclDOF += (detail >> 7) & 0x1;
		out.m_dynamicSclDOF += (detail >> 8) & 0x1;
		out.m_dynamicSclDOF += (detail >> 9) & 0x1;
	}
}



void hkTrackAnalysis::recompose(const AnalysisCompose& input, const hkReal* dynamicDOFs, hkQsTransform* outputPose)
{

	const hkReal* staticDOFs = input.m_staticDOFs;

	for( hkUint32 i = 0; i < input.m_numberOfTracks; i++ )
	{
		const hkUint32 trackMask = input.m_staticMask[i];
		const hkUint32 bitMask = hkUint32(trackMask >> 6);

		hkQsTransform& bone = outputPose[i];

		// Position
		if ( (trackMask & 0x3) != hkTrackAnalysis::HK_TRACK_CLEAR)
		{
			bone.m_translation(0) = (bitMask & 4) ? *dynamicDOFs++ : *staticDOFs++;
			bone.m_translation(1) = (bitMask & 2) ? *dynamicDOFs++ : *staticDOFs++;
			bone.m_translation(2) = (bitMask & 1) ? *dynamicDOFs++ : *staticDOFs++;
		}
		else
		{
			bone.m_translation.setZero4();
		}


		// Rotation
		if ( ((trackMask >> 2) & 0x3) != hkTrackAnalysis::HK_TRACK_CLEAR )
		{
			bone.m_rotation.m_vec(0) = (bitMask & 64) ? *dynamicDOFs++ : *staticDOFs++;
			bone.m_rotation.m_vec(1) = (bitMask & 32) ? *dynamicDOFs++ : *staticDOFs++;
			bone.m_rotation.m_vec(2) = (bitMask & 16) ? *dynamicDOFs++ : *staticDOFs++;
			bone.m_rotation.m_vec(3) = (bitMask & 8) ? *dynamicDOFs++ : *staticDOFs++;
		}
		else
		{
			bone.m_rotation.setIdentity();
		}

		// Scale
		if ( ((trackMask >> 4) & 0x3) != hkTrackAnalysis::HK_TRACK_CLEAR )
		{
			bone.m_scale(0) = (bitMask & 512) ? *dynamicDOFs++ : *staticDOFs++;
			bone.m_scale(1) = (bitMask & 256) ? *dynamicDOFs++ : *staticDOFs++;
			bone.m_scale(2) = (bitMask & 128) ? *dynamicDOFs++ : *staticDOFs++;
		}
		else
		{
			bone.m_scale.setAll3(1.0f);
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
