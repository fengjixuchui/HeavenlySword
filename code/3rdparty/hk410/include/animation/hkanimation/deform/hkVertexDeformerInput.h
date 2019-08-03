/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_VERTEX_DEFORMER_INPUT_H
#define HK_VERTEX_DEFORMER_INPUT_H

/// An input structure to specify which element of a vertex are deformed.
/// Note: undeformed elements are copied if they exist in the output buffer.
class hkVertexDeformerInput
{
	public:
	
		hkVertexDeformerInput() 
		{
			m_deformPosition = false;
			m_deformNormal = false;
			m_deformTangent = false;
			m_deformBinormal = false;
		}
			
		hkBool m_deformPosition;
		hkBool m_deformNormal;
		hkBool m_deformTangent;
		hkBool m_deformBinormal;
};


#endif // HK_VERTEX_DEFORMER_INPUT_H

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
