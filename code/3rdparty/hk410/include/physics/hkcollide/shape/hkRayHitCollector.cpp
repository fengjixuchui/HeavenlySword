/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>
#include <hkcollide/shape/hkRayHitCollector.h>

int hkRayHitCollector::shapeKeysFromCdBody( hkShapeKey* buf, int maxKeys, const hkCdBody& body )
{
	hkCdBody const* bodies[hkShapeRayCastOutput::MAX_HIERARCHY_DEPTH];
	int i = 0;
	for( const hkCdBody* b = &body; b->getParent() != HK_NULL; ++i )
	{
		bodies[i] = b;
		b = b->getParent();
	}
	int j = 0;
	for( ; i > 0 && j < maxKeys-1; ++j )
	{
		buf[j] = bodies[--i]->getShapeKey();
	}
	buf[j] = HK_INVALID_SHAPE_KEY;
	return j + 1;
}

/*
* Havok SDK - CLIENT RELEASE, BUILD(#20060902)
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
