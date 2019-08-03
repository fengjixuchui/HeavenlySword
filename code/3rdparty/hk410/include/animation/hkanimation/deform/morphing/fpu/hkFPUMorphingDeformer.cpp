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
#include <hkanimation/deform/morphing/fpu/hkFPUMorphingDeformer.h>
#include <hkbase/monitor/hkMonitorStream.h>

hkFPUMorphingDeformer::hkFPUMorphingDeformer()
{
	// Zero the binding - leaving it unbound
	hkString::memSet( reinterpret_cast<void*>(&m_binding), 0, sizeof(hkFPUBinding) );
}

inline float _interpolate1( float a, float b, float t, float oneminust)
{
	return (oneminust*a) + (t*b);
}

inline void _normalize3( float* a )
{
	const float s = a[0]*a[0] + a[1]*a[1] + a[2]*a[2];
	a[0] /= s;
	a[1] /= s;
	a[2] /= s;
}

void hkFPUMorphingDeformer::deform ( hkReal delta )
{
	// We assume that prefetch means we don't have to place all the properties in the same inner loop.

	HK_TIMER_BEGIN("hkFPUMorphingDeformer::Deform", this);

	const float oneminusdelta = 1.0f - delta;

	// Position
	if (m_binding.m_oPosBase != HK_NULL)
	{
		float* posOut = m_binding.m_oPosBase;
		const float* posIn1 = m_binding.m_i1PosBase;
		const float* posIn2 = m_binding.m_i2PosBase;

		for(hkUint32 v=0; v < m_binding.m_numVerts; v++)
		{
			posOut[0] = _interpolate1(posIn1[0], posIn2[0], delta, oneminusdelta);
			posOut[1] = _interpolate1(posIn1[1], posIn2[1], delta, oneminusdelta);
			posOut[2] = _interpolate1(posIn1[2], posIn2[2], delta, oneminusdelta);
			
			// add float stride
			posOut += m_binding.m_oStride;
			posIn1 += m_binding.m_i1Stride;
			posIn2 += m_binding.m_i2Stride;
		}
	}

	// Normal
	if (m_binding.m_oNormBase != HK_NULL)
	{
		float* normOut = m_binding.m_oNormBase;
		const float* normIn1 = m_binding.m_i1NormBase;
		const float* normIn2 = m_binding.m_i2NormBase;

		for(hkUint32 v=0; v < m_binding.m_numVerts; v++)
		{
			normOut[0] = _interpolate1(normIn1[0], normIn2[0], delta, oneminusdelta);
			normOut[1] = _interpolate1(normIn1[1], normIn2[1], delta, oneminusdelta);
			normOut[2] = _interpolate1(normIn1[2], normIn2[2], delta, oneminusdelta);
			_normalize3(normOut);
			
			// add float stride
			normOut += m_binding.m_oStride;
			normIn1 += m_binding.m_i1Stride;
			normIn2 += m_binding.m_i2Stride;
		}
	}

	// Tangent
	if (m_binding.m_oTangentBase != HK_NULL)
	{
		float* tangentOut = m_binding.m_oTangentBase;
		const float* tangentIn1 = m_binding.m_i1TangentBase;
		const float* tangentIn2 = m_binding.m_i2TangentBase;

		for(hkUint32 v=0; v < m_binding.m_numVerts; v++)
		{
			// prefetch
			tangentOut[0] = _interpolate1(tangentIn1[0], tangentIn2[0], delta, oneminusdelta);
			tangentOut[1] = _interpolate1(tangentIn1[1], tangentIn2[1], delta, oneminusdelta);
			tangentOut[2] = _interpolate1(tangentIn1[2], tangentIn2[2], delta, oneminusdelta);
			_normalize3(tangentOut);
			
			// add float stride
			tangentOut += m_binding.m_oStride;
			tangentIn1 += m_binding.m_i1Stride;
			tangentIn2 += m_binding.m_i2Stride;
		}
	}

	// Binormal
	if (m_binding.m_oBinormBase != HK_NULL)
	{
		float* binormalOut = m_binding.m_oBinormBase;
		const float* binormalIn1 = m_binding.m_i1BinormBase;
		const float* binormalIn2 = m_binding.m_i2BinormBase;

		for(hkUint32 v=0; v < m_binding.m_numVerts; v++)
		{
			// prefetch
			binormalOut[0] = _interpolate1(binormalIn1[0], binormalIn2[0], delta, oneminusdelta);
			binormalOut[1] = _interpolate1(binormalIn1[1], binormalIn2[1], delta, oneminusdelta);
			binormalOut[2] = _interpolate1(binormalIn1[2], binormalIn2[2], delta, oneminusdelta);
			_normalize3(binormalOut);

			// add float strides
			binormalOut += m_binding.m_oStride;
			binormalIn1 += m_binding.m_i1Stride;
			binormalIn2 += m_binding.m_i2Stride;
		}
	}

	HK_TIMER_END();
}


hkBool hkFPUMorphingDeformer::bind( const hkVertexDeformerInput& input, const hkxVertexBuffer* inputBuffer1, const hkxVertexBuffer* inputBuffer2,  hkxVertexBuffer* outputBuffer )
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

	// All OK so we can initialize the binding
	hkString::memSet( reinterpret_cast<void*>(&m_binding), 0, sizeof(hkFPUBinding) );

	m_binding.m_numVerts = numVerts;

	// Striding in floats
	if ( ((outputBuffer->m_format->m_stride % sizeof(float)) != 0) ||
		 ((inputBuffer1->m_format->m_stride % sizeof(float)) != 0) ||
		 ((inputBuffer2->m_format->m_stride % sizeof(float)) != 0) )
	{
		return false; // stride does not align to floats. Shouldn't really be able to happen.
	}

	m_binding.m_oStride  = (unsigned char)( outputBuffer->m_format->m_stride / sizeof(float) );
	m_binding.m_i1Stride = (unsigned char)( inputBuffer1->m_format->m_stride / sizeof(float) );
	m_binding.m_i2Stride = (unsigned char)( inputBuffer2->m_format->m_stride / sizeof(float) );

	if (input.m_deformPosition)
	{
		m_binding.m_oPosBase  = reinterpret_cast<      float*>( (char*)(outputBuffer->m_vertexData) + outputBuffer->m_format->m_positionOffset );
		m_binding.m_i1PosBase = reinterpret_cast<const float*>( (char*)(inputBuffer1->m_vertexData) + inputBuffer1->m_format->m_positionOffset );
		m_binding.m_i2PosBase = reinterpret_cast<const float*>( (char*)(inputBuffer2->m_vertexData) + inputBuffer2->m_format->m_positionOffset );
	}

	if (input.m_deformNormal)
	{
		m_binding.m_oNormBase  = reinterpret_cast<      float*>( (char*)(outputBuffer->m_vertexData) + outputBuffer->m_format->m_normalOffset );
		m_binding.m_i1NormBase = reinterpret_cast<const float*>( (char*)(inputBuffer1->m_vertexData) + inputBuffer1->m_format->m_normalOffset );
		m_binding.m_i2NormBase = reinterpret_cast<const float*>( (char*)(inputBuffer2->m_vertexData) + inputBuffer2->m_format->m_normalOffset );
	}

	if (input.m_deformTangent)
	{
		m_binding.m_oTangentBase  = reinterpret_cast<      float*>( (char*)(outputBuffer->m_vertexData) + outputBuffer->m_format->m_tangentOffset );
		m_binding.m_i1TangentBase = reinterpret_cast<const float*>( (char*)(inputBuffer1->m_vertexData) + inputBuffer1->m_format->m_tangentOffset );
		m_binding.m_i2TangentBase = reinterpret_cast<const float*>( (char*)(inputBuffer2->m_vertexData) + inputBuffer2->m_format->m_tangentOffset );
	}

	if (input.m_deformBinormal)
	{
		m_binding.m_oNormBase  = reinterpret_cast<      float*>( (char*)(outputBuffer->m_vertexData) + outputBuffer->m_format->m_binormalOffset );
		m_binding.m_i1NormBase = reinterpret_cast<const float*>( (char*)(inputBuffer1->m_vertexData) + inputBuffer1->m_format->m_binormalOffset );
		m_binding.m_i2NormBase = reinterpret_cast<const float*>( (char*)(inputBuffer2->m_vertexData) + inputBuffer2->m_format->m_binormalOffset );
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
