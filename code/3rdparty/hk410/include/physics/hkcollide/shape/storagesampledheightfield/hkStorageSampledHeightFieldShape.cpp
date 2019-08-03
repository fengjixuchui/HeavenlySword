/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>
#include <hkcollide/shape/storagesampledheightfield/hkStorageSampledHeightFieldShape.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkStorageSampledHeightFieldShape);

hkStorageSampledHeightFieldShape::hkStorageSampledHeightFieldShape( const hkSampledHeightFieldBaseCinfo& info, hkArray<hkReal>& samples )
	:	hkSampledHeightFieldShape(info),
		m_triangleFlip(false)
{
	m_storage = samples;
}

hkStorageSampledHeightFieldShape::hkStorageSampledHeightFieldShape( const hkSampledHeightFieldShape* hf )
{
	m_xRes = hf->m_xRes;
	m_zRes = hf->m_zRes;
	m_triangleFlip = hf->getTriangleFlip();
	m_heightCenter = hf->m_heightCenter;
	m_intToFloatScale = hf->m_intToFloatScale;
	m_floatToIntScale = hf->m_floatToIntScale;
	m_floatToIntOffsetFloorCorrected = hf->m_floatToIntOffsetFloorCorrected;
	m_extents = hf->m_extents;

	m_storage.setSize( m_xRes * m_zRes );

	for(int z = 0; z < m_zRes; z++)
	{
		for(int x = 0; x < m_xRes; x++)
		{
			m_storage[z*m_xRes + x] = hf->getHeightAt( x, z );
		}
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
