/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>
#include <hkcollide/shape/hkShape.h>
#include <hkcollide/shape/collection/hkShapeCollection.h>

void hkCollidable::checkPerformance()
{
	if(m_shape)
	{
		if( m_shape->getType() == HK_SHAPE_TRANSFORM)
		{
			HK_WARN(0x2ff8c16f, "Collidable at address " << this << " has a transform shape as the root shape.\n" \
					"This can cause a significant performance loss. To avoid getting this message\n" \
					"compose the transform into the collidable and remove the transform shape.\n" \
					"Please see the 'hkTransformShape' documentation in the User Guide for more information.\n");
		}

		if( m_shape->getType() == HK_SHAPE_COLLECTION
			&& static_cast<const hkShapeCollection*>(m_shape)->getNumChildShapes() > 10 )
		{
			HK_WARN(0x578cef50, "Collidable at address " << this << " has a shape collecion without a bvshape.\n" \
					"This can cause performance loss. To avoid getting this message\n" \
					"add a hkBvShape above this shape.");
		}
	}
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
