/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>
#include <hkcollide/util/hkAabbUtil.h>

void HK_CALL hkAabbUtil::calcAabb( const float* vertexArray, int numVertices, int striding, hkAabb& aabbOut )
{
	aabbOut.m_min(0) = vertexArray[0];
	aabbOut.m_min(1) = vertexArray[1];
	aabbOut.m_min(2) = vertexArray[2];
	aabbOut.m_min(3) = 0.0f;

	aabbOut.m_max = aabbOut.m_min;

	float* v = const_cast<float*>(vertexArray);
	for (int i = 0; i < numVertices; i++)
	{
		aabbOut.m_max(0) = hkMath::max2( aabbOut.m_max(0), v[0] );
		aabbOut.m_max(1) = hkMath::max2( aabbOut.m_max(1), v[1] );
		aabbOut.m_max(2) = hkMath::max2( aabbOut.m_max(2), v[2] );

		aabbOut.m_min(0) = hkMath::min2( aabbOut.m_min(0), v[0] );
		aabbOut.m_min(1) = hkMath::min2( aabbOut.m_min(1), v[1] );
		aabbOut.m_min(2) = hkMath::min2( aabbOut.m_min(2), v[2] );

		v = hkAddByteOffset<float>( v, striding );
	}

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
