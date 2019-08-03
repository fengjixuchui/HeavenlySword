/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COMPRESSION_H
#define HK_COMPRESSION_H

#include <hkbase/hkBase.h>
#include <hkmath/hkMath.h>


extern "C"
{
		/*
		 * Block Encoding
		 */

	struct hkBlockDesc {
		hkUint32 m_bitWidth;	// The bit size of the original data
		hkUint32 m_runSymbol;	// The most commonly occuring symbol - this should be in little endian format
		hkUint32 m_preserved;	// The number of unencoded 4 byte chunks at the start of the stream
	};

		/// Calculate the number of bytes required to encode the given raw data using the description provided
	hkUint32 HK_CALL hkCalcBlockEncodedSize(const hkUint8* rawData, hkUint32 rawDataSizeBytes, hkBlockDesc& desc);

		/// Encode the given raw data using the description provided
	void HK_CALL hkBlockEncode( const hkUint8* rawData, hkUint32 rawDataSizeBytes, hkBlockDesc& desc, hkUint8* encodedData);

		/// Decode the given data using the description provided
	hkUint32 HK_CALL hkBlockDecode( const hkUint8* encodedData, hkBlockDesc& desc, hkUint8* rawData, hkUint32 rawDataSizeBytes);

		/*
		 *	Wavelet Compression
		 */

		/// This applies a wavelet transformation to a set of real numbers
		/// and produces a set of wavelet components in place. The components generally have a 
		/// much smaller range of values than the original data and can be represented
		/// with fewer bits than the original raw data signal.
		/// The number of raw data components (nValues) must be a power of 2.
	void HK_CALL hkWaveletTransform( hkReal* rawData, hkUint32 nValues );

		/// This applies the inverse wavelet transformation to a set of wavelet coefficients
		/// and reproduces the original raw signal in place. 
		/// The number of raw data components (nValues) must be a power of 2.
	void HK_CALL hkInverseWaveletTransform( hkReal* waveletData, hkUint32 nValues );

		/// This method iterates through the wavelet coefficients and truncates a proportion of them to 0
		/// This truncation allows both the quantizer and encoder to do a much better job.
		/// The proportion should be between 0 (lossless) and 1 (fully lossy).
	void HK_CALL hkWaveletCoefficientThreshold( hkReal* waveletData, hkUint32 nValues, hkReal proportion );

		/*
		 *	Delta Compression
		 */

		/// This applies a delta transformation to a set of real numbers
		/// and produces a set of delta coefficients in place. The components generally have a 
		/// much smaller range of values than the original data and can be represented
		/// with fewer bits than the original raw data signal.
	void HK_CALL hkDeltaTransform( hkReal* rawData, hkUint32 nValues );

		/// This applies the inverse delta transformation to a set of delta coefficients
		/// and reproduces the original raw signal in place. 
	void HK_CALL hkInverseDeltaTransform( hkReal* deltaData, hkUint32 nValues);

		/*
		 * Quantization
		 */

	struct hkQuantizeDesc {
		hkUint8  m_bitWidth;	// The number of bits for each quantized element
		hkUint8  m_preserved;	// The number of unquantized reals at the start of the stream
		hkReal   m_scale;		// The scale of the data = (max-min)
		hkReal   m_offset;		// The offset of the data (zero is preserved)
	};

		/// Calculates a quantisation description for the given data
		/// precision > 1/(2^bitWidth)
	void HK_CALL hkQuantizeCalcDesc( const hkReal* data, hkUint32 nValues, hkQuantizeDesc& desc, hkUint32 quantizedBits = 16, hkUint32 preserve = 0 );

		/// Calculates the buffer size required to quantize the number of given bytes with the given header
	hkUint32 HK_CALL hkCalcQuantizedSize(hkUint32 nValues, const hkQuantizeDesc& desc);

		/// Quantizes the real coefficients into the data buffer.
	void HK_CALL hkQuantizeReal( const hkReal* data, hkUint32 nValues, const hkQuantizeDesc& desc, hkUint8* quantized );

		/// Expands the data into the preallocated buffer
	void HK_CALL hkExpandReal( const hkUint8* quantized, const hkQuantizeDesc& desc, hkReal* data, hkUint32 nValues );

		/*
		 *	Quaternion 32 bit compression
		 */

		/// Compress the quaternion to a 32 bit number		
	void HK_CALL hkQuantizeQuaternion( const hkQuaternion& in, hkUint32& out );

		/// Decompress a 32 number back into a quaternion
	void HK_CALL hkExpandQuaternion( const hkUint32& in, hkQuaternion& out );
}


#endif // HK_COMPRESSION_H

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
