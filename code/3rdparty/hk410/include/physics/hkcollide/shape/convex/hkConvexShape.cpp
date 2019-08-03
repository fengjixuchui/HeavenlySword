/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>
#include <hkcollide/shape/convex/hkConvexShape.h>
#include <hkcollide/shape/hkRayHitCollector.h>

#if HK_POINTER_SIZE==4
HK_COMPILE_TIME_ASSERT( sizeof(hkConvexShape) == 16 );
#endif

hkReal hkConvexShapeDefaultRadius = 0.05f;


hkReal hkConvexShape::getMaximumProjection( const hkVector4& direction ) const
{
	hkCdVertex supportingVertex;
	getSupportingVertex( direction, supportingVertex );
	const hkReal result = hkReal(supportingVertex.dot3 ( direction )) + m_radius;
	return result;
}


void hkConvexShape::castRayWithCollector( const hkShapeRayCastInput& inputLocal, const hkCdBody& cdBody, hkRayHitCollector& collector ) const
{
	HK_ASSERT2(0x7f1735a0,  cdBody.getShape() == this, "inconsistent cdBody, shapePointer is wrong" );
	hkShapeRayCastOutput results;
	results.m_hitFraction = collector.m_earlyOutHitFraction;

	if ( castRay( inputLocal, results ) )
	{
		HK_ASSERT2(0x6ad83e81, results.m_shapeKeys[0] == HK_INVALID_SHAPE_KEY, "Non leaf convex shape needs to override castRayWithCollector");
		results.m_normal._setMul3( cdBody.getTransform().getRotation(), results.m_normal );
		collector.addRayHit( cdBody, results );
	}
}



hkShapeType hkConvexShape::getType() const
{
	return HK_SHAPE_CONVEX; 
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
