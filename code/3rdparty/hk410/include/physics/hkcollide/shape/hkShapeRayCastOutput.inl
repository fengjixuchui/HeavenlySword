/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

void hkShapeRayCastOutput::_reset()
{
	m_shapeKeyIndex = 0;
	m_shapeKeys[0] = HK_INVALID_SHAPE_KEY;
}

void hkShapeRayCastOutput::reset()
{
	hkShapeRayCastCollectorOutput::reset();
	_reset();
}


hkShapeRayCastOutput::hkShapeRayCastOutput()
{
	_reset();
}

void hkShapeRayCastOutput::changeLevel(int delta)
{
	HK_ASSERT(0x7ee30341, unsigned(m_shapeKeyIndex+delta) < MAX_HIERARCHY_DEPTH );
	m_shapeKeyIndex += delta;
}

inline void hkShapeRayCastOutput::setKey(hkShapeKey key)
{
	m_shapeKeys[m_shapeKeyIndex] = key;
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
