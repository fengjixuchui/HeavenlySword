/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>
#include <hkcollide/shape/bv/hkBvShape.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkBvShape);

void hkBvShape::getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out  ) const
{
	getBoundingVolumeShape()->getAabb( localToWorld, tolerance, out );
}

hkShapeType hkBvShape::getType() const
{
	return HK_SHAPE_BV;
}

hkBool hkBvShape::castRay( const hkShapeRayCastInput& input, hkShapeRayCastOutput& results ) const
{
	// Comment in this code if you wish to get a callback from a phantom callback shape only
	// if the ray hits the bv shape. This is commented out, because if the ray starts inside
	// the bv shape, it will not hit the bv shape, so no callback at all will be fired.

	//hkShapeRayCastOutput testOutput;
	//if ( getBoundingVolumeShape()->castRay( input, testOutput) )
	HK_TIMER_BEGIN("rcBvShape", HK_NULL);
	results.changeLevel(1);
	hkBool result = m_childShape->castRay( input, results );
	results.changeLevel(-1);
	if( result )
	{
		results.setKey(0);
	}
	HK_TIMER_END();
	return result;
}


void hkBvShape::castRayWithCollector(const hkShapeRayCastInput& input, const hkCdBody& cdBody, hkRayHitCollector& collector ) const
{
	// Comment in this code if you wish to get a callback from a phantom callback shape only
	// if the ray hits the bv shape. This is commented out, because if the ray starts inside
	// the bv shape, it will not hit the bv shape, so no callback at all will be fired.

	//hkShapeRayCastOutput testOutput;
	//if ( getBoundingVolumeShape()->castRay( input, testOutput) )
	HK_TIMER_BEGIN("rcBvShape", HK_NULL);
	{
		hkCdBody body( &cdBody );
		const hkShape* child = getChildShape();
		body.setShape( child, 0 );
		child->castRayWithCollector( input, body, collector );
	}
	HK_TIMER_END();
}

void hkBvShape::calcStatistics( hkStatisticsCollector* collector ) const
{
	collector->beginObject("BvShape", collector->MEMORY_SHARED, this);
	collector->addChildObject( "Child", collector->MEMORY_SHARED, getChildShape() );
	collector->endObject();
}

const hkShapeContainer* hkBvShape::getContainer() const
{
	return &m_childShape;
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
