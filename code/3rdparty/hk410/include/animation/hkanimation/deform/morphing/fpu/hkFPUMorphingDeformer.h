/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_FPU_MORPHING_DEFORMER_H
#define HK_FPU_MORPHING_DEFORMER_H

#include <hkanimation/deform/morphing/hkMorphingDeformer.h>

/// The derived class for a FPU based implementation of weighted vertex deformation.
/// Applies to both indexed and non indexed skinning.
/// By FPU it really means that it will not enforce any alignment on the input 
/// or output data, but if your math configuration uses SIMD, it will use the SIMD ops
/// where it can. If you have data that has its deformable members (pos, normals, etc)
/// properly aligned so that they can be cast to hkVector4, then use the SIMD version of this deformer
/// as it will be more streamlined.
class hkFPUMorphingDeformer : public hkMorphingDeformer
{
	public:
		
			/// Initializes to an unbound deformer
		hkFPUMorphingDeformer();

			/// Bind the buffers 
			/// The output buffer should be preallocated.
			/// Returns false if the deformer does not support the input or output buffer format.
			/// The input and output buffers must be appropriately aligned (16 byte)
		hkBool bind( const hkVertexDeformerInput& input, const hkxVertexBuffer* inputBuffer1, const hkxVertexBuffer* inputBuffer2,  hkxVertexBuffer* outputBuffer );

			/// Interpolate the input buffers into the output buffer.
			/// The deformer must first be bound and the output buffer locked before deforming.
		virtual void deform ( hkReal delta );

	private:

		struct hkFPUBinding
		{
			// Input buffer 1
			const float* m_i1PosBase;
			const float* m_i1NormBase;
			const float* m_i1BinormBase;
			const float* m_i1TangentBase;
			hkUint8 m_i1Stride;

			// Input Buffer 2
			const float* m_i2PosBase;
			const float* m_i2NormBase;
			const float* m_i2BinormBase;
			const float* m_i2TangentBase;
			hkUint8 m_i2Stride;

			// Output Buffer
			float* m_oPosBase;
			float* m_oNormBase;
			float* m_oBinormBase;
			float* m_oTangentBase;
			hkUint8 m_oStride;

			hkUint32 m_numVerts;
		};

		struct hkFPUBinding m_binding;
};

#endif // HK_FPU_MORPHING_DEFORMER_H

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
