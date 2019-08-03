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
#include <hkanimation/deform/skinning/fpu/hkFPUSkinningDeformer.h>
#include <hkbase/monitor/hkMonitorStream.h>

inline void _rotate( float* out, const float* in, const float* t)
{
	out[0] = in[0]*t[0] + in[1]*t[4] + in[2]*t[8];
	out[1] = in[0]*t[1] + in[1]*t[5] + in[2]*t[9];
	out[2] = in[0]*t[2] + in[1]*t[6] + in[2]*t[10];
}

inline void _transform( float* out, const float* in, const float* t )
{
	out[0] = in[0]*t[0] + in[1]*t[4] + in[2]*t[8]  + t[12];//in[3];
	out[1] = in[0]*t[1] + in[1]*t[5] + in[2]*t[9]  + t[13];//in[3];
	out[2] = in[0]*t[2] + in[1]*t[6] + in[2]*t[10] + t[14];//in[3];
}

inline void _addMul( float* inout, const float* in, float s )
{
	inout[0] += in[0] * s;
	inout[1] += in[1] * s;
	inout[2] += in[2] * s;
}

hkFPUSkinningDeformer::hkFPUSkinningDeformer()
{
	// Zero the binding - leaving it unbound
	hkString::memSet( reinterpret_cast<void*>(&m_binding), 0, sizeof(hkFloatBinding) );
}

void HK_CALL hkFPUSkinningDeformer::deform (  const hkTransform* m_worldCompositeMatrices, const hkFloatBinding& binding )
{
	HK_TIMER_BEGIN("hkFPUSkinningDeformer::Deform", this);

	const hkUint32 byteWeightStride = binding.m_iWeightStride * sizeof(float);
	const hkUint32 byteIndexStride = binding.m_iIndexStride * sizeof(float);
	float* posOut = binding.m_oPosBase;
	float* normOut = binding.m_oNormBase;
	float* binormOut = binding.m_oBinormBase;
	float* tangentOut = binding.m_oTangentBase;
	const float* posIn = binding.m_iPosBase;
	const float* normIn = binding.m_iNormBase;
	const float* binormIn = binding.m_iBinormBase;
	const float* tangentIn = binding.m_iTangentBase;
	const hkUint8* weightIn = binding.m_iWeightBase;
	const hkUint8* indexIn = binding.m_iIndexBase;
	float tVec[3];
	const float oneOver255 = 1.0f / 255;

	if (indexIn == HK_NULL) // Unindexed skinning
	{
		for( hkUint32 v=0; v < binding.m_numVerts; v++ )
		{
			if (binding.m_oPosBase)
			{ posOut[0] = 0; posOut[1] = 0; posOut[2] = 0; }

			if (binding.m_oNormBase)
			{ normOut[0] = 0; normOut[1] = 0; normOut[2] = 0; }

			if (binding.m_oBinormBase)
			{ binormOut[0] = 0; binormOut[1] = 0; binormOut[2] = 0; }

			if (binding.m_oTangentBase)
			{ tangentOut[0] = 0; tangentOut[1] = 0; tangentOut[2] = 0; }

			for (hkUint8 w=0; w < binding.m_bonesPerVertex; w++)
			{
				if (weightIn[w] > 0) // worth checking for.
				{
					const float normalizedWeight = weightIn[w] * oneOver255;
					const hkTransform& t = m_worldCompositeMatrices[ w ];
					if (binding.m_oPosBase)
					{
						_transform(tVec, posIn, (const float*)(&t));
						_addMul(posOut, tVec, normalizedWeight);
					}
					const float* r = (const float*)(&t.getRotation());
					if (binding.m_oNormBase) 
					{
						// Rotate by (tInverse)Transpose, but as they are rigid body transforms at the moment inv==transpose
						// Todo: when scale is added, we need to adjust this.
						_rotate( tVec, normIn, r); 
						_addMul(normOut, tVec, normalizedWeight);
					}
					if (binding.m_oBinormBase) 
					{
						// Rotate by (tInverse)Transpose, but as they are rigid body transforms at the moment inv==transpose
						// Todo: when scale is added, we need to adjust this.
						_rotate( tVec, binormIn, r); 
						_addMul(binormOut, tVec, normalizedWeight);
					}
					if (binding.m_oTangentBase) 
					{
						// Rotate by (tInverse)Transpose, but as they are rigid body transforms at the moment inv==transpose
						// Todo: when scale is added, we need to adjust this.
						_rotate( tVec, tangentIn, r); 
						_addMul(tangentOut, tVec, normalizedWeight);
					}
				}
			}

			posOut +=  binding.m_oPosStride; 
			posIn += binding.m_iPosStride;
			normOut += binding.m_oNormStride;
			normIn += binding.m_iNormStride;
			binormOut += binding.m_oBinormStride;
			binormIn += binding.m_iBinormStride;
			tangentOut += binding.m_oTangentStride;
			tangentIn += binding.m_iTangentStride;
			weightIn += byteWeightStride;
		}
	}
	else // Palette Skinning
	{
		for(hkUint32 v=0; v < binding.m_numVerts; v++)
		{
			if (binding.m_oPosBase)
			{ posOut[0] = 0; posOut[1] = 0; posOut[2] = 0; }
			
			if (binding.m_oNormBase)
			{ normOut[0] = 0; normOut[1] = 0; normOut[2] = 0; }

			if (binding.m_oBinormBase)
			{ binormOut[0] = 0; binormOut[1] = 0; binormOut[2] = 0; }

			if (binding.m_oTangentBase)
			{ tangentOut[0] = 0; tangentOut[1] = 0; tangentOut[2] = 0; }

			for (hkUint8 w=0; w < binding.m_bonesPerVertex; w++)
			{
				if (weightIn[w] > 0) // worth checking for.
				{
					const float normalizedWeight = weightIn[w] * oneOver255;
					const hkTransform& t = m_worldCompositeMatrices[ indexIn[w] ];
					if (binding.m_oPosBase)
					{
						_transform(tVec, posIn, (const float*)(&t));
						_addMul(posOut, tVec, normalizedWeight);
					}
					const float* r = (const float*)(&t.getRotation());
					if (binding.m_oNormBase) 
					{
						// Rotate by (tInverse)Transpose, but as they are rigid body transforms at the moment inv==transpose
						// Todo: when scale is added, we need to adjust this.
						_rotate( tVec, normIn, r); 
						_addMul(normOut, tVec, normalizedWeight);
					}
					if (binding.m_oBinormBase) 
					{
						// Rotate by (tInverse)Transpose, but as they are rigid body transforms at the moment inv==transpose
						// Todo: when scale is added, we need to adjust this.
						_rotate( tVec, binormIn, r); 
						_addMul(binormOut, tVec, normalizedWeight);
					}
					if (binding.m_oTangentBase) 
					{
						// Rotate by (tInverse)Transpose, but as they are rigid body transforms at the moment inv==transpose
						// Todo: when scale is added, we need to adjust this.
						_rotate( tVec, tangentIn, r); 
						_addMul(tangentOut, tVec, normalizedWeight);
					}
				}
			}

			posOut +=  binding.m_oPosStride; // stride in floats
			posIn += binding.m_iPosStride;
			normOut += binding.m_oNormStride;
			normIn += binding.m_iNormStride;
			binormOut += binding.m_oBinormStride;
			binormIn += binding.m_iBinormStride;
			tangentOut += binding.m_oTangentStride;
			tangentIn += binding.m_iTangentStride;
			weightIn += byteWeightStride;
			indexIn += byteIndexStride;
		}
	}

	HK_TIMER_END();		
}

void hkFPUSkinningDeformer::deform( const hkTransform* m_worldCompositeMatrices )
{
	hkFPUSkinningDeformer::deform(m_worldCompositeMatrices, m_binding);
}

hkBool hkFPUSkinningDeformer::bind( const hkVertexDeformerInput& input, const hkxVertexBuffer* inputBuffer, hkxVertexBuffer* outputBuffer )
{
	// Check buffer sizes match 
	const hkUint32 numVerts = outputBuffer->m_numVertexData;
	const hkUint32 iv = inputBuffer->m_numVertexData;
	if (iv != numVerts)
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
	hkUint32 iProps = hkxVertexFormatUtil::getVertexProperties(*inputBuffer->m_format);

	if ( ((iProps & props) != props) || ((oProps & props) != props) )
	{
		// Formats are not appropriate
		return false;
	}

	if ((iProps & hkxVertexFormatUtil::HK_VERTEX_WEIGHTS) == 0)
	{
		// No weights specified
		return false;
	}

	// All OK so we can initialize the binding
	hkString::memSet( reinterpret_cast<void*>(&m_binding), 0, sizeof(hkFloatBinding) );

	m_binding.m_numVerts = numVerts;
	m_binding.m_bonesPerVertex = inputBuffer->m_format->m_numBonesPerVertex;
	m_binding.m_iWeightBase = reinterpret_cast<const hkUint8*>( (char*)(inputBuffer->m_vertexData) + inputBuffer->m_format->m_boneWeightOffset );

	// Initilaise indices if they exist
	// This switches matrix palettes on and off
	if (iProps & hkxVertexFormatUtil::HK_VERTEX_INDICES)
	{
		m_binding.m_iIndexBase = reinterpret_cast<const hkUint8*>( (char*)(inputBuffer->m_vertexData) + inputBuffer->m_format->m_boneIndexOffset );
	}

	// Striding in floats
	// Striding in floats
	if ( ((outputBuffer->m_format->m_stride % sizeof(float)) != 0) ||
		((inputBuffer->m_format->m_stride % sizeof(float)) != 0) )
	{
		return false; // stride does not align to floats. Shouldn't really be able to happen.
	}

	m_binding.m_oPosStride = (unsigned char)(  outputBuffer->m_format->m_stride / sizeof(float) );
	m_binding.m_iPosStride = (unsigned char)( inputBuffer->m_format->m_stride / sizeof(float) );

	m_binding.m_iNormStride = m_binding.m_iBinormStride = m_binding.m_iTangentStride = m_binding.m_iPosStride;
	m_binding.m_oNormStride = m_binding.m_oBinormStride = m_binding.m_oTangentStride = m_binding.m_oPosStride;
	m_binding.m_iIndexStride = m_binding.m_iWeightStride = m_binding.m_iPosStride;

	if (input.m_deformPosition)
	{
		m_binding.m_oPosBase = reinterpret_cast<float*>( (char*)(outputBuffer->m_vertexData) + outputBuffer->m_format->m_positionOffset );
		m_binding.m_iPosBase = reinterpret_cast<float*>( (char*)(inputBuffer->m_vertexData) + inputBuffer->m_format->m_positionOffset );
	}
	

	if (input.m_deformNormal)
	{
		m_binding.m_oNormBase = reinterpret_cast<float*>( (char*)(outputBuffer->m_vertexData) + outputBuffer->m_format->m_normalOffset );
		m_binding.m_iNormBase = reinterpret_cast<float*>( (char*)(inputBuffer->m_vertexData) + inputBuffer->m_format->m_normalOffset );
	}


	if (input.m_deformTangent)
	{
		m_binding.m_oTangentBase = reinterpret_cast<float*>( (char*)(outputBuffer->m_vertexData) + outputBuffer->m_format->m_tangentOffset );
		m_binding.m_iTangentBase = reinterpret_cast<float*>( (char*)(inputBuffer->m_vertexData) + inputBuffer->m_format->m_tangentOffset );
	}

	if (input.m_deformBinormal)
	{
		m_binding.m_oBinormBase = reinterpret_cast<float*>( (char*)(outputBuffer->m_vertexData) + outputBuffer->m_format->m_binormalOffset );
		m_binding.m_iBinormBase = reinterpret_cast<float*>( (char*)(inputBuffer->m_vertexData) + inputBuffer->m_format->m_binormalOffset );
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
