/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


#include <hkcollide/hkCollide.h>
#include <hkcollide/shape/multiray/hkMultiRayShape.h>
#include <hkmath/linear/hkVector4Util.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkMultiRayShape);


hkMultiRayShape::hkMultiRayShape(const Ray* Rays, int nRays, hkReal rayPenetrationDistance)
{
    m_rayPenetrationDistance = rayPenetrationDistance;
	Ray* ray = m_rays.expandBy( nRays );
	for (int i = nRays-1; i>=0; i-- )
	{
		*ray = *Rays;
		hkVector4 diff; diff.setSub4(ray->m_end , ray->m_start);
		ray->m_start(3) = diff.length3();
		
		// Extend it by the tolerance
		diff.normalize3();
		diff.mul4(m_rayPenetrationDistance);
		ray->m_end.add4(diff);

		ray++;
		Rays++;
	}
}

void hkMultiRayShape::castRayWithCollector( const hkShapeRayCastInput& input, const hkCdBody& cdBody, hkRayHitCollector& collector ) const
{
}

hkBool hkMultiRayShape::castRay( const hkShapeRayCastInput& input, hkShapeRayCastOutput& results ) const 
{
	return false;
}


void hkMultiRayShape::getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out ) const
{
	hkInplaceArrayAligned16<Ray,16> worldRays;
	worldRays.setSize( m_rays.getSize() );

	hkVector4Util::transformPoints( localToWorld, &m_rays[0].m_start, m_rays.getSize()*2, &worldRays[0].m_start );

	hkVector4 absMin; absMin.setAll3(  HK_REAL_MAX );
	hkVector4 absMax; absMax.setAll3( -HK_REAL_MAX );
	
	const Ray* ray =  &worldRays[0];
	for(int i = 0; i < worldRays.getSize(); ++i)
	{
		absMin.setMin4( absMin, ray->m_end );
		absMin.setMin4( absMin, ray->m_start );
		absMax.setMax4( absMax, ray->m_end );
		absMax.setMax4( absMax, ray->m_start );
		ray++;
	}
	out.m_min = absMin;
	out.m_max = absMax;

//	DISPLAY_POINT(absMin, 0xffffffff);
//	DISPLAY_POINT(absMax, 0xffffffff);
}


void hkMultiRayShape::calcStatistics( hkStatisticsCollector* collector ) const
{
	collector->beginObject("MultiRay", collector->MEMORY_SHARED, this);
	collector->addArray( "Rays", collector->MEMORY_SHARED, this->m_rays );
	collector->endObject();
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
