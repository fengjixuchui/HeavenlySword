/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkanimation/hkAnimation.h>
#include <hkscenedata/mesh/hkxVertexBuffer.h>
#include <hkscenedata/mesh/hkxVertexFormatUtil.h>
#include <hkscenedata/mesh/hkxVertexFormat.h>
#include <hkanimation/deform/morphing/simd/hkSimdMorphingDeformer.h>
#include <hkbase/monitor/hkMonitorStream.h>

hkSimdMorphingDeformer::hkSimdMorphingDeformer()
{
	// Zero the binding - leaving it unbound
	hkString::memSet( reinterpret_cast<void*>(&m_binding), 0, sizeof(hkSimdBinding) );
}

#define _COPY_VECTOR4_TO_FLOATS( F, V) { F[0] = V(0); F[1] = V(1); F[2] = V(2); }

void hkSimdMorphingDeformer::deformAlignedInput( hkReal delta )
{
	HK_TIMER_BEGIN("hkSimdMorphingDeformer::DeformAlignedInput", this);

	// We assume that prefetch means we don't have to place all the properties in the same inner loop.
	hkVector4 tVec;

	// Position
	if (m_binding.m_oPosBase != HK_NULL)
	{
		float* posOut = m_binding.m_oPosBase;
		const hkVector4* posIn1 = m_binding.m_i1PosBase;
		const hkVector4* posIn2 = m_binding.m_i2PosBase;

		for(hkUint32 v=0; v < m_binding.m_numVerts; v++)
		{
			// prefetch
			tVec.setInterpolate4( *posIn1, *posIn2, delta);
			_COPY_VECTOR4_TO_FLOATS(posOut, tVec);//SLOW
			posOut += m_binding.m_oStride ;
			posIn1 += m_binding.m_i1Stride;
			posIn2 += m_binding.m_i2Stride;
		}
	}

	// Normal
	if (m_binding.m_oNormBase != HK_NULL)
	{
		float* normOut = m_binding.m_oNormBase;
		const hkVector4* normIn1 = m_binding.m_i1NormBase;
		const hkVector4* normIn2 = m_binding.m_i2NormBase;

		for(hkUint32 v=0; v < m_binding.m_numVerts; v++)
		{
			// prefetch
			tVec.setInterpolate4( *normIn1, *normIn2, delta);
			tVec.normalize3();
			_COPY_VECTOR4_TO_FLOATS(normOut, tVec);//SLOW

			normOut += m_binding.m_oStride ;
			normIn1 += m_binding.m_i1Stride;
			normIn2 += m_binding.m_i2Stride;

		}
	}

	// Tangent
	if (m_binding.m_oTangentBase != HK_NULL)
	{
		float* tangentOut = m_binding.m_oTangentBase;
		const hkVector4* tangentIn1 = m_binding.m_i1TangentBase;
		const hkVector4* tangentIn2 = m_binding.m_i2TangentBase;

		for(hkUint32 v=0; v < m_binding.m_numVerts; v++)
		{
			// prefetch
			tVec.setInterpolate4( *tangentIn1, *tangentIn2, delta);
			tVec.normalize3();
			_COPY_VECTOR4_TO_FLOATS(tangentOut, tVec);//SLOW

			tangentOut += m_binding.m_oStride;
			tangentIn1 += m_binding.m_i1Stride;
			tangentIn2 += m_binding.m_i2Stride;
		}
	}

	// Binormal
	if (m_binding.m_oBinormBase != HK_NULL)
	{
		float* binormalOut = m_binding.m_oBinormBase;
		const hkVector4* binormalIn1 = m_binding.m_i1BinormBase;
		const hkVector4* binormalIn2 = m_binding.m_i2BinormBase;

		for(hkUint32 v=0; v < m_binding.m_numVerts; v++)
		{
			// prefetch
			tVec.setInterpolate4( *binormalIn1, *binormalIn2, delta);
			tVec.normalize3();
			_COPY_VECTOR4_TO_FLOATS(binormalOut, tVec); //SLOW

			binormalOut += m_binding.m_oStride ;
			binormalIn1 += m_binding.m_i1Stride;
			binormalIn2 += m_binding.m_i2Stride;
		}
	}

	HK_TIMER_END();
}


void hkSimdMorphingDeformer::deformAligned( hkReal delta )
{
	// We assume that prefetch means we don't have to place all the properties in the same inner loop.
	HK_TIMER_BEGIN("hkSimdMorphingDeformer::DeformAligned", this);

	const hkUint32 vStrideO = m_binding.m_oStride / 4; // in vector4s
	// Position
	if (m_binding.m_oPosBase != HK_NULL)
	{
		hkVector4* posOut = (hkVector4*)m_binding.m_oPosBase; // safe as aligned
		const hkVector4* posIn1 = m_binding.m_i1PosBase;
		const hkVector4* posIn2 = m_binding.m_i2PosBase;

		for(hkUint32 v=0; v < m_binding.m_numVerts; v++)
		{
			// prefetch
			posOut->setInterpolate4( *posIn1, *posIn2, delta);
			posOut += vStrideO;
			posIn1 += m_binding.m_i1Stride;
			posIn2 += m_binding.m_i2Stride;
		}
	}

	// Normal
	if (m_binding.m_oNormBase != HK_NULL)
	{
		hkVector4* normOut = (hkVector4*)m_binding.m_oNormBase;// safe as aligned
		const hkVector4* normIn1 = m_binding.m_i1NormBase;
		const hkVector4* normIn2 = m_binding.m_i2NormBase;

		for(hkUint32 v=0; v < m_binding.m_numVerts; v++)
		{
			// prefetch
			normOut->setInterpolate4( *normIn1, *normIn2, delta);
			normOut->normalize3();

			normOut += vStrideO;
			normIn1 += m_binding.m_i1Stride;
			normIn2 += m_binding.m_i2Stride;
			
		}
	}

	// Tangent
	if (m_binding.m_oTangentBase != HK_NULL)
	{
		hkVector4* tangentOut = (hkVector4*)m_binding.m_oTangentBase;// safe as aligned
		const hkVector4* tangentIn1 = m_binding.m_i1TangentBase;
		const hkVector4* tangentIn2 = m_binding.m_i2TangentBase;

		for(hkUint32 v=0; v < m_binding.m_numVerts; v++)
		{
			// prefetch
			tangentOut->setInterpolate4( *tangentIn1, *tangentIn2, delta);
			tangentOut->normalize3();

			tangentOut += vStrideO;
			tangentIn1 += m_binding.m_i1Stride;
			tangentIn2 += m_binding.m_i2Stride;
		}
	}

	// Binormal
	if (m_binding.m_oBinormBase != HK_NULL)
	{
		hkVector4* binormalOut = (hkVector4*)m_binding.m_oBinormBase;// safe as aligned
		const hkVector4* binormalIn1 = m_binding.m_i1BinormBase;
		const hkVector4* binormalIn2 = m_binding.m_i2BinormBase;

		for(hkUint32 v=0; v < m_binding.m_numVerts; v++)
		{
			// prefetch
			binormalOut->setInterpolate4( *binormalIn1, *binormalIn2, delta);
			binormalOut->normalize3();

			binormalOut += vStrideO;
			binormalIn1 += m_binding.m_i1Stride;
			binormalIn2 += m_binding.m_i2Stride;
		}
	}

	HK_TIMER_END();
}

void hkSimdMorphingDeformer::deform( hkReal delta )
{
	if (m_outputAligned)
		deformAligned(delta);
	else
		deformAlignedInput(delta);
}

hkBool hkSimdMorphingDeformer::bind( const hkVertexDeformerInput& input, const hkxVertexBuffer* inputBuffer1, const hkxVertexBuffer* inputBuffer2,  hkxVertexBuffer* outputBuffer )
{
	// Check buffer sizes match 
	const hkUint32 numVerts = outputBuffer->m_numVertexData;
	hkUint32 i1v = inputBuffer1->m_numVertexData;
	hkUint32 i2v = inputBuffer2->m_numVertexData;
	if ((i1v != numVerts) || (i1v != i2v))
	{
		// Buffer sizes are incorrect
		return false;
	}

	// Check input buffer and output buffer formats match
	hkUint32 props = 0;
	props |= input.m_deformPosition ? hkxVertexFormatUtil::HK_VERTEX_POSITION : hkxVertexFormatUtil::HK_VERTEX_NONE;
	props |= input.m_deformNormal   ? hkxVertexFormatUtil::HK_VERTEX_NORMAL   : hkxVertexFormatUtil::HK_VERTEX_NONE;
	props |= input.m_deformTangent  ? hkxVertexFormatUtil::HK_VERTEX_TANGENT  : hkxVertexFormatUtil::HK_VERTEX_NONE;
	props |= input.m_deformBinormal   ? hkxVertexFormatUtil::HK_VERTEX_BINORM   : hkxVertexFormatUtil::HK_VERTEX_NONE;

	hkUint32 oProps = hkxVertexFormatUtil::getVertexProperties(*outputBuffer->m_format);
	hkUint32 i1Props = hkxVertexFormatUtil::getVertexProperties(*inputBuffer1->m_format);
	hkUint32 i2Props = hkxVertexFormatUtil::getVertexProperties(*inputBuffer2->m_format);

	if ( ((i1Props & props) != props) || ((i2Props & props) != props) || ((oProps & props) != props) )
	{
		// Formats are not appropriate
		return false;
	}

	// Check alignemnt is at least 16byte aligned
	// Check alignemnt is at least 16byte aligned
	hkBool unalignedInput = false;
	hkBool unalignedOutput = false;
	for ( int i=0; i< 4; i++)
	{
		hkUint32 property = (1<<i); //1,2,4,8 are the 4 vals to check
		if (props & property)
		{
			const hkUint64 aI1 = hkxVertexFormatUtil::getAlignment( property, *inputBuffer1 );
			const hkUint64 aI2 = hkxVertexFormatUtil::getAlignment( property, *inputBuffer2 );
			const hkUint64 aO = hkxVertexFormatUtil::getAlignment( property, *outputBuffer );
			unalignedInput = unalignedInput || (aI1 < 0x10) || (aI2 < 0x10); // >16bytes
			unalignedOutput  = unalignedOutput || (aO < 0x10); // >16bytes
		}
	}

	// requires aligned input
	if (unalignedInput)
	{
		// position is not aligned
		return false;
	}

#if (HK_CONFIG_SIMD == HK_CONFIG_SIMD_ENABLED)
	if (unalignedOutput)
	{
		HK_WARN_ONCE(0x38b31d98, "You are using a SIMD deformer with unaligned output properties. This is slower that using aligned output.");
	}
#else
	HK_WARN_ONCE(0x38b31d99, "You are using a SIMD deformer but SIMD is not enabled. This will be slower than just using the FPU deformer, but will still work.");
#endif

	m_outputAligned = !unalignedOutput;

	// All OK so we can initialize the binding
	hkString::memSet( reinterpret_cast<void*>(&m_binding), 0, sizeof(hkSimdBinding) );

	m_binding.m_numVerts = numVerts;

	// Adjust striding for vector4s
	m_binding.m_oStride  = (unsigned char)( outputBuffer->m_format->m_stride / sizeof(float) );
	m_binding.m_i1Stride = (unsigned char)( inputBuffer1->m_format->m_stride / sizeof(hkVector4) );
	m_binding.m_i2Stride = (unsigned char)( inputBuffer2->m_format->m_stride / sizeof(hkVector4) );

	if (input.m_deformPosition)
	{
		m_binding.m_oPosBase  = reinterpret_cast<      float*>( (char*)(outputBuffer->m_vertexData) + outputBuffer->m_format->m_positionOffset );
		m_binding.m_i1PosBase = reinterpret_cast<const hkVector4*>( (char*)(inputBuffer1->m_vertexData) + inputBuffer1->m_format->m_positionOffset );
		m_binding.m_i2PosBase = reinterpret_cast<const hkVector4*>( (char*)(inputBuffer2->m_vertexData) + inputBuffer2->m_format->m_positionOffset );
	}

	if (input.m_deformNormal)
	{
		m_binding.m_oNormBase  = reinterpret_cast<      float*>( (char*)(outputBuffer->m_vertexData) + outputBuffer->m_format->m_normalOffset );
		m_binding.m_i1NormBase = reinterpret_cast<const hkVector4*>( (char*)(inputBuffer1->m_vertexData) + inputBuffer1->m_format->m_normalOffset );
		m_binding.m_i2NormBase = reinterpret_cast<const hkVector4*>( (char*)(inputBuffer2->m_vertexData) + inputBuffer2->m_format->m_normalOffset );
	}

	if (input.m_deformTangent)
	{
		m_binding.m_oTangentBase  = reinterpret_cast<      float*>( (char*)(outputBuffer->m_vertexData) + outputBuffer->m_format->m_tangentOffset );
		m_binding.m_i1TangentBase = reinterpret_cast<const hkVector4*>( (char*)(inputBuffer1->m_vertexData) + inputBuffer1->m_format->m_tangentOffset );
		m_binding.m_i2TangentBase = reinterpret_cast<const hkVector4*>( (char*)(inputBuffer2->m_vertexData) + inputBuffer2->m_format->m_tangentOffset );
	}

	if (input.m_deformBinormal)
	{
		m_binding.m_oNormBase  = reinterpret_cast<      float*>( (char*)(outputBuffer->m_vertexData) + outputBuffer->m_format->m_binormalOffset );
		m_binding.m_i1NormBase = reinterpret_cast<const hkVector4*>( (char*)(inputBuffer1->m_vertexData) + inputBuffer1->m_format->m_binormalOffset );
		m_binding.m_i2NormBase = reinterpret_cast<const hkVector4*>( (char*)(inputBuffer2->m_vertexData) + inputBuffer2->m_format->m_binormalOffset );
	}

	return true;
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
