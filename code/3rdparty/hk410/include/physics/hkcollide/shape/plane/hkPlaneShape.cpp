/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


#include <hkcollide/hkCollide.h>

#include <hkcollide/shape/plane/hkPlaneShape.h>
#include <hkcollide/util/hkAabbUtil.h>
#include <hkcollide/shape/hkShapeRayCastInput.h>
#include <hkcollide/shape/hkRayHitCollector.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkPlaneShape);


hkPlaneShape::hkPlaneShape(const hkVector4& plane, const hkAabb& aabb)
{
	m_plane = plane;
	m_aabbHalfExtents.setSub4( aabb.m_max, aabb.m_min );
	m_aabbHalfExtents.mul4( 0.5f );
	m_aabbCenter.setInterpolate4( aabb.m_max, aabb.m_min, 0.5f );

}

hkPlaneShape::hkPlaneShape( const hkVector4& direction, const hkVector4& center, const hkVector4& halfExtents )
{
	m_plane = direction;
	m_plane(3) = - hkReal ( direction.dot3( center ) );
	m_aabbCenter = center;
	m_aabbHalfExtents = halfExtents;
}

void hkPlaneShape::getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out ) const
{
	hkAabbUtil::calcAabb( localToWorld, m_aabbHalfExtents, m_aabbCenter, tolerance,  out );
}

hkShapeType hkPlaneShape::getType() const
{
	return HK_SHAPE_PLANE;
}


hkBool hkPlaneShape::castRay(const hkShapeRayCastInput& input, hkShapeRayCastOutput& results) const
{
	HK_TIMER_BEGIN("rcPlane", HK_NULL);
	{

	    hkReal f = m_plane.dot4xyz1( input.m_from );
	    if ( f < 0 )
	    {
		    goto returnFalse;
	    }

		hkReal t = m_plane.dot4xyz1( input.m_to );
		if ( t >=0 )
		{
			goto returnFalse;
		}

		hkReal hitFraction = f / ( f - t );

		//
		//	Check, if it is inside the aabb
		//
		hkVector4 hitPoint; hitPoint.setInterpolate4( input.m_from, input.m_to, hitFraction );
		hitPoint.sub4( m_aabbCenter );
		hitPoint.setAbs4( hitPoint );
		int mask = hitPoint.compareLessThanEqual4( m_aabbHalfExtents ) & HK_VECTOR3_COMPARE_MASK_XYZ;

		if ( mask != HK_VECTOR3_COMPARE_MASK_XYZ )
		{
			goto returnFalse;
		}

		results.m_hitFraction = hitFraction;
		results.m_normal = m_plane;
		results.setKey( HK_INVALID_SHAPE_KEY );

		HK_TIMER_END();
		return true;
	}

returnFalse:
	HK_TIMER_END();
	return false;
}

void hkPlaneShape::castRayWithCollector( const hkShapeRayCastInput& inputLocal, const hkCdBody& cdBody, hkRayHitCollector& collector ) const
{
	HK_ASSERT2(0x7f1d0d08,  cdBody.getShape() == this, "inconsistent cdBody, shapePointer is wrong" );
	hkShapeRayCastOutput results;
	results.m_hitFraction = collector.m_earlyOutHitFraction;

	if ( castRay( inputLocal, results ) )
	{
		results.m_normal.setMul3( cdBody.getTransform().getRotation(), results.m_normal );
		collector.addRayHit( cdBody, results );
	}
}

void hkPlaneShape::castSphere( const hkSphereCastInput& inputLocal, const hkCdBody& cdBody, hkRayHitCollector& collector ) const
{
	hkReal radius = inputLocal.m_radius;
	hkReal f = m_plane.dot4xyz1( inputLocal.m_from );
	f-= radius;

	hkReal t = m_plane.dot4xyz1( inputLocal.m_to );
	t-= radius;

	if ( t >= 0.0f )
	{
		return;
	}

	if ( f - t  < inputLocal.m_maxExtraPenetration )
	{
		return;
	}

	hkReal hitFraction = ( f <= 0.0f ) ? 0.0f : ( f / (f - t) );

	//
	//	Check, if it is inside the aabb
	//
	hkVector4 hitPoint; hitPoint.setInterpolate4( inputLocal.m_from, inputLocal.m_to, hitFraction );
	hitPoint.sub4( m_aabbCenter );
	hitPoint.setAbs4( hitPoint );
	int mask = hitPoint.compareLessThanEqual4( m_aabbHalfExtents ) & HK_VECTOR3_COMPARE_MASK_XYZ;

	if ( mask != HK_VECTOR3_COMPARE_MASK_XYZ )
	{
		return;
	}

	hkShapeRayCastOutput output;
	output.m_hitFraction = hitFraction;
	output.m_normal = m_plane;
	output.setKey( HK_INVALID_SHAPE_KEY );
	collector.addRayHit( cdBody, output );

	return;
}


void hkPlaneShape::collideSpheres( const CollideSpheresInput& input, SphereCollisionOutput* outputArray) const
{
    hkVector4* o = outputArray;
    hkSphere* s = input.m_spheres;

    for (int i = input.m_numSpheres-1; i>=0 ; i-- )
    {
        o[0] = m_plane;
        hkReal d = m_plane.dot4xyz1( s->getPosition() );
        d -= s->getRadius();
        o[0](3) = d;

        o++;
        s++;
    } 
}


void hkPlaneShape::calcStatistics( hkStatisticsCollector* collector ) const
{
	collector->beginObject("PlaneShape", collector->MEMORY_SHARED, this);
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
