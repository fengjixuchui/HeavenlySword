/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKANIMATION_ANIMATION_WAVELETCOMPRESSED_HKWAVELETCOMPRESSEDANIMATION_XML_H
#define HKANIMATION_ANIMATION_WAVELETCOMPRESSED_HKWAVELETCOMPRESSEDANIMATION_XML_H

#include <hkanimation/animation/hkSkeletalAnimation.h>
	
/// hkWaveletSkeletalAnimation meta information
extern const class hkClass hkWaveletSkeletalAnimationClass;

class hkInterleavedSkeletalAnimation;

/// The information needed to construct a quantized and delta compressed stream of Bone Transforms)
/// Supports block encoding for faster random access.
class hkWaveletSkeletalAnimation : public hkSkeletalAnimation
{
	public:

		HK_DECLARE_CLASS_ALLOCATOR( HK_MEMORY_CLASS_ANIM_COMPRESSED );
		HK_DECLARE_REFLECTION();

			// Construction parameters
		struct CompressionParams
		{
			hkUint16 m_quantizationBits;
			hkUint16 m_blockSize;
			hkReal	 m_scaleTolerance;
			hkUint16 m_preserve;
			hkReal   m_truncProp;

			// TrackAnalysis tolerances.
			hkReal m_absolutePositionTolerance;	// Set to 0 to use only relative tolerance
			hkReal m_relativePositionTolerance; // Set to 0 to use only abs tolerance
			hkReal m_rotationTolerance;

			CompressionParams();
		};
		
			/// This structure is used when specifying per track compression settings
		struct PerTrackCompressionParams
		{
		public:
			/// List of CompressionParams to enable per-bone compression settings
			/// On initialisation only a single element is allocated. 
			hkArray<struct hkWaveletSkeletalAnimation::CompressionParams> m_parameterPalette;

			/// An array of indices into the palette above 
			hkArray<int> m_trackIndexToPaletteIndex;
		};

			/// Constructor compresses data
		hkWaveletSkeletalAnimation(const hkInterleavedSkeletalAnimation& raw, const CompressionParams& params);

		    /// Constructor allowing different compression settings for each track in the animation
		hkWaveletSkeletalAnimation(const hkInterleavedSkeletalAnimation& raw, const PerTrackCompressionParams& params);

				
			/// Get the pose at a given time
			/// Note: If you are calling this method directly you may find some quantization error present in the rotations.
			/// The blending done in hkAnimatedSkeleton is not sensitive to rotation error so rather than renormalize here
			/// we defer it until blending has been completed. If you are using this method directly you may want to call 
			/// hkSkeletonUtils::normalizeRotations() on the results.
		virtual void samplePose(hkReal time, hkQsTransform* poseLocalSpaceOut, hkChunkCache* cache) const;
		
			/// Get a subset of the pose at a given time
		virtual void samplePartialPose(hkReal time, hkUint32 maxTrack, hkQsTransform* poseLocalSpaceOut, hkChunkCache* cache) const;

			/// Returns the number of original samples / frames of animation
		virtual int getNumOriginalFrames() const;

			/*
			* Block decompression
			*/

			/// Return the number of chunks of data required to sample a pose at time t
		virtual int getNumDataChunks(hkReal time) const;

			/// Return the chunks of data required to sample a pose at time t
		virtual void getDataChunks(hkReal time, DataChunk* dataChunks, int m_numDataChunks) const;

			/// Get a subset of the pose at a given time using data chunks
		static void HK_CALL samplePartialWithDataChunks(hkReal time, hkUint32 maxTrack, hkQsTransform* poseLocalSpaceOut, const DataChunk* dataChunks, int m_numDataChunks);

		void getBlockDataAndSize(int blockNum, DataChunk& dataChunkOut) const;
	public:
		
		int m_numberOfPoses;

		/// The number of poses in each encoded block
		int m_blockSize;

		/// 
		struct QuantizationFormat
		{
			HK_DECLARE_REFLECTION();

			QuantizationFormat( ) { }
			QuantizationFormat( hkFinishLoadedObjectFlag flag ) {} 

			/// 
			hkUint8 m_maxBitWidth;

			/// 
			hkUint8 m_preserved;

			/// Number of dynamic tracks that are quantized and stored
			hkUint32 m_numD;

			/// Index into the data buffer for the quantization offsets
			hkUint32 m_offsetIdx;

			/// Index into the data buffer for the quantization scales
			hkUint32 m_scaleIdx;

			// Index into the data buffer for the quantization bidwidths
			hkUint32 m_bitWidthIdx;
		};

		/// Quantization Description
		struct QuantizationFormat m_qFormat;

		/// Index into the data buffer for the Track Mask
		hkUint32 m_staticMaskIdx;

		/// Index into the data buffer for the  Static DOF Data
		hkUint32 m_staticDOFsIdx;

		/// Index into the data buffer for the block indices
		hkUint32 m_blockIndexIdx;

		/// Size of the block indices (stored as hkUint32)
		hkUint32 m_blockIndexSize;

		/// Index into the data buffer for the Quantization Data
		hkUint32 m_quantizedDataIdx;

		/// Size of the Quantization Data (stored as hkUint8)
		hkUint32 m_quantizedDataSize;

		/// The data buffer where compressed and static data is kept
		hkUint8* m_dataBuffer;
		int		 m_numDataBuffer;
	
	public:
	
		// Constructor for initialisation of vtable fixup
		HK_FORCE_INLINE hkWaveletSkeletalAnimation( hkFinishLoadedObjectFlag flag ) : hkSkeletalAnimation(flag),
			m_qFormat(flag) { if (flag.m_finishing) handleEndian(); }

		~hkWaveletSkeletalAnimation();

	private:
			/// Initialize the animation with construction info
		void initialize(const hkInterleavedSkeletalAnimation& raw, const PerTrackCompressionParams& params);

			/// Swap the endianness in the data buffer as appropriate
		void handleEndian();
};


#endif // HKANIMATION_ANIMATION_WAVELETCOMPRESSED_HKWAVELETCOMPRESSEDANIMATION_XML_H

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
