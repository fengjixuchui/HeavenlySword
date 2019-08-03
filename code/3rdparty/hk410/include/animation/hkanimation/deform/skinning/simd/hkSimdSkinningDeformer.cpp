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
#include <hkanimation/deform/skinning/simd/hkSimdSkinningDeformer.h>
#include <hkbase/monitor/hkMonitorStream.h>

#ifdef HK_PLATFORM_PS2
#define UNCACHED(a) ((void *)(int(a) | 0x20000000))
#define HK_PREFETCH(a) {asm __volatile__ ( "pref 0, 0(%0)" : : "r" (a) );}
#else
#define UNCACHED(a) a
#define HK_PREFETCH(a) /* Nothing */
#endif

 
hkSimdSkinningDeformer::hkSimdSkinningDeformer()
{
	// Zero the binding - leaving it unbound
	hkString::memSet( reinterpret_cast<void*>(&m_binding), 0, sizeof(hkSimdBinding) );
}

void HK_CALL hkSimdSkinningDeformer::deformAlignedInput( const hkTransform* m_worldCompositeMatrices, const hkSimdBinding& binding  )
{
	HK_TIMER_BEGIN("hkSimdSkinningDeformer::DeformAlignedInput", this);

	float* posOut = binding.m_oPosBase;
	float* normOut = binding.m_oNormBase;
	float* binormOut = binding.m_oBinormBase;
	float* tangentOut = binding.m_oTangentBase;
	const hkVector4* posIn = binding.m_iPosBase;
	const hkVector4* normIn = binding.m_iNormBase;
	const hkVector4* binormIn = binding.m_iBinormBase;
	const hkVector4* tangentIn = binding.m_iTangentBase;
	const hkUint8* weightIn = binding.m_iWeightBase;
	const hkUint8* indexIn = binding.m_iIndexBase;

	// Use the math libs vector4s. 
	// If it is using SIMD, then this function will be faster 
	// that using float macros. If the math lib is non simd
	// then this function should not copy the float vals, instead
	// it should use some simple float macros --> TODO
	hkVector4 resultP;
	hkVector4 resultN;	
	hkVector4 resultB;
	hkVector4 resultT;	

	hkVector4 tVec; 

	const hkSimdReal oneOver255 = 1.0f/ 255.0f;

	if (indexIn == HK_NULL) // Unindexed skinning
	{
		for(hkUint32 v=0; v < binding.m_numVerts; v++)
		{
			// prefetch
			resultP.setZero4();
			resultN.setZero4();
			resultB.setZero4();
			resultT.setZero4();

			for (hkUint8 w=0; w < binding.m_bonesPerVertex; w++)
			{
				if (weightIn[w] > 0) // worth checking for.
				{
					const hkSimdReal normalizedWeight = hkSimdReal(weightIn[w]) * oneOver255;
					const hkTransform& t = m_worldCompositeMatrices[w];
					if (binding.m_oPosBase)
					{
						tVec._setTransformedPos( t, *posIn);
						resultP.addMul4(normalizedWeight, tVec);
					}
					// Rotate by (tInverse)Transpose, but as they are rigid body transforms at the moment inv==transpose
					// Todo: when scale is added, we need to adjust this.
					const hkRotation& r = t.getRotation();
					if (binding.m_oNormBase) 
					{
						tVec._setRotatedDir( r, *normIn); 
						resultN.addMul4(normalizedWeight, tVec);
					}
					if (binding.m_oBinormBase) 
					{
						tVec._setRotatedDir( r, *binormIn); 
						resultB.addMul4(normalizedWeight, tVec);
					}
					if (binding.m_oTangentBase) 
					{
						tVec._setRotatedDir( r, *tangentIn); 
						resultT.addMul4(normalizedWeight, tVec);
					}
				}
			}

			posIn += binding.m_iPosVectorStride; 
			normIn += binding.m_iNormVectorStride;			
			binormIn += binding.m_iBinormVectorStride; 
			tangentIn += binding.m_iTangentVectorStride;			
			weightIn += binding.m_iWeightByteStride;
			
			if (binding.m_oPosBase)
			{
				posOut[0] = resultP(0);
				posOut[1] = resultP(1);
				posOut[2] = resultP(2);
				posOut +=  binding.m_oPosFloatStride; // stride in floats
			}
			if (binding.m_oNormBase)
			{
				// shouldn't need normalization if weights normalized and input normalized correctly
				// resultN.normalize3(); 
				normOut[0] = resultN(0);
				normOut[1] = resultN(1);
				normOut[2] = resultN(2);	
				normOut += binding.m_oNormFloatStride;
			}
			if (binding.m_oBinormBase)
			{
				// shouldn't need normalization if weights normalized and input normalized correctly
				// resultN.normalize3(); 
				binormOut[0] = resultB(0);
				binormOut[1] = resultB(1);
				binormOut[2] = resultB(2);	
				binormOut += binding.m_oBinormFloatStride;
			}
			if (binding.m_oTangentBase)
			{
				// shouldn't need normalization if weights normalized and input normalized correctly
				// resultN.normalize3(); 
				tangentOut[0] = resultT(0);
				tangentOut[1] = resultT(1);
				tangentOut[2] = resultT(2);	
				tangentOut += binding.m_oTangentFloatStride;
			}
		}
	}
	else // Palette Skinning
	{
		for(hkUint32 v=0; v < binding.m_numVerts; v++)
		{
			// prefetch
			resultP.setZero4();
			resultN.setZero4();
			resultB.setZero4();
			resultT.setZero4();

			for (hkUint8 w=0; w < binding.m_bonesPerVertex; w++)
			{
				if (weightIn[w] > 0) // worth checking for.
				{
					const hkSimdReal normalizedWeight = hkSimdReal(weightIn[w]) * oneOver255;
					const hkTransform& t = m_worldCompositeMatrices[ indexIn[w] ];
					if (binding.m_oPosBase)
					{
						tVec._setTransformedPos( t, *posIn); // inline simd
						resultP.addMul4(normalizedWeight, tVec);
					}
					const hkRotation& r = t.getRotation();
					if (binding.m_oNormBase) 
					{
						tVec._setRotatedDir( r, *normIn); 
						resultN.addMul4(normalizedWeight, tVec);
					}
					if (binding.m_oBinormBase) 
					{
						tVec._setRotatedDir( r, *binormIn); 
						resultB.addMul4(normalizedWeight, tVec);
					}
					if (binding.m_oTangentBase) 
					{
						tVec._setRotatedDir( r, *tangentIn); 
						resultT.addMul4(normalizedWeight, tVec);
					}
				}
			}

			posIn += binding.m_iPosVectorStride;
			normIn += binding.m_iNormVectorStride;
			binormIn += binding.m_iBinormVectorStride;
			tangentIn += binding.m_iTangentVectorStride;
			weightIn += binding.m_iWeightByteStride;
			indexIn += binding.m_iIndexByteStride;

			if (binding.m_oPosBase)
			{
				posOut[0] = resultP(0);
				posOut[1] = resultP(1);
				posOut[2] = resultP(2);
				posOut +=  binding.m_oPosFloatStride; // stride in floats
			}
			if (binding.m_oNormBase)
			{
				// shouldn't need normalization if weights normalized and input normalized correctly
				// resultN.normalize3(); 
				normOut[0] = resultN(0);
				normOut[1] = resultN(1);
				normOut[2] = resultN(2);	
				normOut += binding.m_oNormFloatStride;
			}
			if (binding.m_oBinormBase)
			{
				// shouldn't need normalization if weights normalized and input normalized correctly
				// resultN.normalize3(); 
				binormOut[0] = resultB(0);
				binormOut[1] = resultB(1);
				binormOut[2] = resultB(2);	
				binormOut += binding.m_oBinormFloatStride;
			}
			if (binding.m_oTangentBase)
			{
				// shouldn't need normalization if weights normalized and input normalized correctly
				// resultN.normalize3(); 
				tangentOut[0] = resultT(0);
				tangentOut[1] = resultT(1);
				tangentOut[2] = resultT(2);	
				tangentOut += binding.m_oTangentFloatStride;
			}
		}
	}
	HK_TIMER_END();		
}

void HK_CALL hkSimdSkinningDeformer::deformAligned( const hkTransform* m_worldCompositeMatrices, const hkSimdBinding& binding  )
{
	HK_TIMER_BEGIN_LIST("DeformAlign", "Bind");

	const int vectorOPosStride = binding.m_oPosFloatStride / 4;
	const int vectorONormStride = binding.m_oNormFloatStride / 4;
	const int vectorOBinormStride = binding.m_oBinormFloatStride / 4;
	const int vectorOTangentStride = binding.m_oTangentFloatStride / 4;

	hkVector4* posOut = (hkVector4*)UNCACHED(binding.m_oPosBase); // safe as aligned
	const hkVector4* posIn = binding.m_iPosBase;
	hkVector4* normOut = (hkVector4*)UNCACHED(binding.m_oNormBase);// safe as aligned
	const hkVector4* normIn = binding.m_iNormBase;
	hkVector4* binormOut = (hkVector4*)UNCACHED(binding.m_oBinormBase);// safe as aligned
	const hkVector4* binormIn = binding.m_iBinormBase;
	hkVector4* tangentOut = (hkVector4*)UNCACHED(binding.m_oTangentBase);// safe as aligned
	const hkVector4* tangentIn = binding.m_iTangentBase;

	const hkUint8* weightIn = binding.m_iWeightBase;
	const hkUint8* indexIn = binding.m_iIndexBase; 

	const hkSimdReal oneOver255 = 1.0f / 255.0f;

	if (indexIn == HK_NULL) // Unindexed skinning
	{	
		for(hkUint32 v=0; v < binding.m_numVerts; v++)
		{
			hkVector4 resultP;
			hkVector4 resultN;	
			hkVector4 resultB;
			hkVector4 resultT;	
			resultP.setZero4();
			resultN.setZero4();
			resultB.setZero4();
			resultT.setZero4();

			for (hkUint8 w=0; w < binding.m_bonesPerVertex; w++)
			{
				if (weightIn[w] > 0) // worth checking for?
				{
					const hkSimdReal normalizedWeight = hkSimdReal(weightIn[w]) * oneOver255;
					const hkTransform& t = m_worldCompositeMatrices[w];

					// Position
					if (binding.m_oPosBase != HK_NULL)
					{
						hkVector4 tVec;tVec.setTransformedPos( t, *posIn);
						resultP.addMul4(normalizedWeight, tVec);
					}

					// Normal
					const hkRotation& r = t.getRotation();
					if (binding.m_oNormBase != HK_NULL)
					{
						hkVector4 tVec;tVec._setRotatedDir( r, *normIn);
						resultN.addMul4(normalizedWeight, tVec);
					}
					// Binormal
					if (binding.m_oBinormBase != HK_NULL)
					{
						hkVector4 tVec;tVec._setRotatedDir( r, *binormIn);
						resultB.addMul4(normalizedWeight, tVec);
					}	
					// Tangent
					if (binding.m_oTangentBase != HK_NULL)
					{
						hkVector4 tVec;tVec._setRotatedDir( r, *tangentIn);
						resultT.addMul4(normalizedWeight, tVec);
					}
				}
			}

			*posOut = resultP;
			if (binding.m_oNormBase != HK_NULL)
				*normOut = resultN;
			if (binding.m_oBinormBase != HK_NULL)
				*binormOut = resultB;
			if (binding.m_oTangentBase != HK_NULL)
				*tangentOut = resultT;

			posOut += vectorOPosStride;
			normOut += vectorONormStride;
			binormOut += vectorOBinormStride;
			tangentOut += vectorOTangentStride;
			
			posIn += binding.m_iPosVectorStride;
			normIn += binding.m_iNormVectorStride;
			binormIn += binding.m_iBinormVectorStride;
			tangentIn += binding.m_iTangentVectorStride;
			weightIn += binding.m_iWeightByteStride;
		}
	}
	else // Palette Skinning
	{		
		for(hkUint32 v=0; v < binding.m_numVerts; v++)
		{
			//HK_TIMER_SPLIT_LIST("InitLoop");
			
			HK_PREFETCH(posIn + binding.m_iPosVectorStride);
			HK_PREFETCH(weightIn + binding.m_iWeightByteStride);
			HK_PREFETCH(indexIn + binding.m_iIndexByteStride);

			hkVector4 resultP;
			hkVector4 resultN;	
			hkVector4 resultB;
			hkVector4 resultT;	
			resultP.setZero4();
			resultN.setZero4();
			resultB.setZero4();
			resultT.setZero4();

			for (hkUint8 w=0; w < binding.m_bonesPerVertex; w++)
			{
				if (weightIn[w] > 0) // worth checking for?
				{
					//HK_TIMER_SPLIT_LIST("Fetch");
					const hkSimdReal normalizedWeight = hkSimdReal(weightIn[w]) * oneOver255;
					const hkTransform& t = m_worldCompositeMatrices[ indexIn[w] ];
					
					// Position
					//HK_TIMER_SPLIT_LIST("Pos");
					if (binding.m_oPosBase != HK_NULL)
					{
						hkVector4 tVec;tVec._setTransformedPos( t, *posIn);
						resultP.addMul4(normalizedWeight, tVec);					
					}

					//HK_TIMER_SPLIT_LIST("Rest");

					// Normal
					const hkRotation& r = t.getRotation();
					if (binding.m_oNormBase != HK_NULL)
					{
						hkVector4 tVec;tVec._setRotatedDir( r, *normIn);
						resultN.addMul4(normalizedWeight, tVec);
					}
					// Binormal
					if (binding.m_oBinormBase != HK_NULL)
					{
						hkVector4 tVec;tVec._setRotatedDir( r, *binormIn);
						resultB.addMul4(normalizedWeight, tVec);
					}
					// Tangent
					if (binding.m_oTangentBase != HK_NULL)
					{
						hkVector4 tVec;tVec._setRotatedDir( r, *tangentIn);
						resultT.addMul4(normalizedWeight, tVec);
					}
				}
			}

			*posOut = resultP;
			if (binding.m_oNormBase != HK_NULL)
				*normOut = resultN;
			if (binding.m_oBinormBase != HK_NULL)
				*binormOut = resultB;
			if (binding.m_oTangentBase != HK_NULL)
				*tangentOut = resultT;

			posOut += vectorOPosStride;
			normOut += vectorONormStride;
			binormOut += vectorOBinormStride;
			tangentOut += vectorOTangentStride;
			
			posIn += binding.m_iPosVectorStride;
			normIn += binding.m_iNormVectorStride;
			binormIn += binding.m_iBinormVectorStride;
			tangentIn += binding.m_iTangentVectorStride;
			weightIn += binding.m_iWeightByteStride;
			indexIn += binding.m_iIndexByteStride;
		}
	}

	HK_TIMER_END_LIST();
}

void HK_CALL hkSimdSkinningDeformer::deform( const hkTransform* m_worldCompositeMatrices, const hkSimdBinding& binding  )
{
	if (binding.m_outputAligned)
		hkSimdSkinningDeformer::deformAligned(m_worldCompositeMatrices, binding);
	else
		hkSimdSkinningDeformer::deformAlignedInput(m_worldCompositeMatrices, binding);
}

void hkSimdSkinningDeformer::deform( const hkTransform* m_worldCompositeMatrices )
{
	hkSimdSkinningDeformer::deform(m_worldCompositeMatrices, m_binding);
}

hkBool hkSimdSkinningDeformer::bind( const hkVertexDeformerInput& input, const hkxVertexBuffer* inputBuffer, hkxVertexBuffer* outputBuffer )
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

	// Check alignemnt is at least 16byte aligned
	hkBool unalignedInput = false;
	hkBool unalignedOutput = false;
	for ( int i=0; i< 4; i++)
	{
		hkUint32 property = (1<<i); //1,2,4,8 are the 4 vals to check
		if (props & property)
		{
			const hkUint64 aI = hkxVertexFormatUtil::getAlignment( property, *inputBuffer );
			const hkUint64 aO = hkxVertexFormatUtil::getAlignment( property, *outputBuffer );
			unalignedInput = unalignedInput || (aI < 0x10); // >16bytes
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
	
	// All OK so we can initialize the binding
	hkString::memSet( reinterpret_cast<void*>(&m_binding), 0, sizeof(hkSimdBinding) );

	m_binding.m_outputAligned = !unalignedOutput;

	m_binding.m_numVerts = numVerts;
	m_binding.m_bonesPerVertex = inputBuffer->m_format->m_numBonesPerVertex;
	m_binding.m_iWeightBase = reinterpret_cast<const hkUint8*>( (char*)(inputBuffer->m_vertexData) + inputBuffer->m_format->m_boneWeightOffset );

	// Initilaise indices if they exist
	// This switches matrix palettes on and off
	if (iProps & hkxVertexFormatUtil::HK_VERTEX_INDICES)
	{
		m_binding.m_iIndexBase = reinterpret_cast<const hkUint8*>( (char*)(inputBuffer->m_vertexData) + inputBuffer->m_format->m_boneIndexOffset );
	}

	// Adjust striding for vector4s
	m_binding.m_oPosFloatStride = (unsigned char)( outputBuffer->m_format->m_stride / sizeof(float) );
	m_binding.m_iPosVectorStride = (unsigned char)( inputBuffer->m_format->m_stride / sizeof(hkVector4) );

	m_binding.m_iWeightByteStride = m_binding.m_iIndexByteStride = inputBuffer->m_format->m_stride;
	m_binding.m_oTangentFloatStride = m_binding.m_oBinormFloatStride = m_binding.m_oNormFloatStride = m_binding.m_oPosFloatStride;
	m_binding.m_iTangentVectorStride = m_binding.m_iBinormVectorStride = m_binding.m_iNormVectorStride = m_binding.m_iPosVectorStride;
	

	if (input.m_deformPosition)
	{
		m_binding.m_oPosBase = reinterpret_cast<float*>( (char*)(outputBuffer->m_vertexData) + outputBuffer->m_format->m_positionOffset );
		m_binding.m_iPosBase = reinterpret_cast<const hkVector4*>( (char*)(inputBuffer->m_vertexData) + inputBuffer->m_format->m_positionOffset );
	}

	if (input.m_deformNormal)
	{
		m_binding.m_oNormBase = reinterpret_cast<float*>( (char*)(outputBuffer->m_vertexData) + outputBuffer->m_format->m_normalOffset );
		m_binding.m_iNormBase = reinterpret_cast<const hkVector4*>( (char*)(inputBuffer->m_vertexData) + inputBuffer->m_format->m_normalOffset );
	}

	if (input.m_deformTangent)
	{
		m_binding.m_oTangentBase = reinterpret_cast<float*>( (char*)(outputBuffer->m_vertexData) + outputBuffer->m_format->m_tangentOffset );
		m_binding.m_iTangentBase = reinterpret_cast<const hkVector4*>( (char*)(inputBuffer->m_vertexData) + inputBuffer->m_format->m_tangentOffset );
	}

	if (input.m_deformBinormal)
	{
		m_binding.m_oBinormBase = reinterpret_cast<float*>( (char*)(outputBuffer->m_vertexData) + outputBuffer->m_format->m_binormalOffset );
		m_binding.m_iBinormBase = reinterpret_cast<const hkVector4*>( (char*)(inputBuffer->m_vertexData) + inputBuffer->m_format->m_binormalOffset );
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
