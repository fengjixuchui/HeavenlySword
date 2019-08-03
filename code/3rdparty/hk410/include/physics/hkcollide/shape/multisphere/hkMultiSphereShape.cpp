/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>
#include <hkcollide/shape/multisphere/hkMultiSphereShape.h>
#include <hkmath/linear/hkVector4Util.h>
#include <hkcollide/shape/transform/hkTransformShape.h>
#include <hkcollide/shape/sphere/hkSphereShape.h>
#include <hkcollide/shape/hkShapeRayCastInput.h>
#include <hkcollide/shape/hkRayHitCollector.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkMultiSphereShape);



hkMultiSphereShape::hkMultiSphereShape(const hkVector4* spheres, int numSpheres)
: hkSphereRepShape()
{
#if defined(HK_COMPILER_MSVC) && (HK_COMPILER_MSVC_VERSION >= 1300) // msvc6 bug
    HK_COMPILE_TIME_ASSERT(sizeof(m_spheres)/sizeof(hkVector4) == MAX_SPHERES);
#endif

	HK_ASSERT2(0x6d7430b4,  numSpheres <= hkMultiSphereShape::MAX_SPHERES, "the hkMultiSphereShape does not support so many spheres");
	for (int i = 0; i < numSpheres; i++ )
	{
		m_spheres[i] = spheres[i];
	}
	m_numSpheres = numSpheres; 
}


hkShapeType hkMultiSphereShape::getType() const
{
	return HK_SHAPE_MULTI_SPHERE;
}

void hkMultiSphereShape::getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out ) const
{
	hkVector4 worldSpheres[MAX_SPHERES];

	hkVector4Util::transformPoints( localToWorld, &m_spheres[0], getNumSpheres(), &worldSpheres[0] );
	hkVector4 absMin; absMin.setAll3(  HK_REAL_MAX );
	hkVector4 absMax; absMax.setAll3( -HK_REAL_MAX );
	
	for(int i = 0; i < m_numSpheres; ++i)
	{
		hkVector4 r;
		r.setBroadcast3clobberW(m_spheres[i], 3);
		hkVector4 min; min.setSub4(worldSpheres[i], r);
		hkVector4 max; max.setAdd4(worldSpheres[i], r);

		absMin.setMin4( absMin, min );
		absMax.setMax4( absMax, max );		
	}

	hkVector4 tol4; tol4.setAll3( tolerance );

	out.m_min.setSub4(absMin, tol4);
	out.m_max.setAdd4(absMax, tol4);
}

void hkMultiSphereShape::getCollisionSpheresInfo( hkCollisionSpheresInfo& infoOut ) const 
{
	infoOut.m_numSpheres = getNumSpheres();
	infoOut.m_useBuffer = false;
}

const hkSphere* hkMultiSphereShape::getCollisionSpheres( hkSphere* sphereBuffer ) const
{
	return reinterpret_cast<const hkSphere*>(&m_spheres[0]);
}

static int castRayInternal( const hkShapeRayCastInput& input, const hkVector4* m_spheres, int m_numSpheres, hkReal* distOut, int* indexOut )
{
	//
	//	This functions is a modified version of
	//  http://www.siggraph.org/education/materials/HyperGraph/raytrace/rtinter1.htm
	//  Modifications include changing the order of if statements to prevent
	//  any division which can produce a number greater than 1
	//

	HK_TIMER_BEGIN("rcMultiSpher", HK_NULL);

	// No hit found yet
	int numHits = 0;
	for ( int i = 0; i<m_numSpheres; i++ )
	{
		hkReal	radius2 = m_spheres[i](3) * m_spheres[i](3);

		// 
		// solve quadratic function: ax*x + bx + c = 0
		//

		hkVector4 localTo( input.m_to ); localTo.sub4( m_spheres[i] );
		hkVector4 localFrom( input.m_from ); localFrom.sub4( m_spheres[i] );
		hkVector4 dir; dir.setSub4( localTo, localFrom );

		const hkReal C = hkReal(localFrom.lengthSquared3()) - radius2;

		const hkReal B = 2.0f * hkReal(dir.dot3( localFrom ));
		if ( B >= 0 )
		{
			// ray points away from sphere center
			continue;
		}

		const hkReal A = dir.lengthSquared3();

		const hkReal det = B*B - 4*A*C;
		if ( det <= 0 )
		{
			//
			//	Infinite ray does not hit
			//
			continue;
		}

		const hkReal sqDet = hkMath::sqrt( det );

		hkReal t = -B - sqDet;
		t *= 0.5f;

		if ( t >= A )
		{
			//
			//	hits behind endpoint
			//
			continue;
		}

		if ( t < 0 )
		{
			//
			// start point inside
			//
			continue;
		}

		//  Note: we know that t > 0
		//  Also that A > t 
		//  So this division is safe and results in a point between 0 and 1

		t = t / A;

		// Check if this hit is closer than any previous

		distOut[numHits] = t;
		indexOut[numHits] = i;
		++numHits;
	}
	HK_TIMER_END();
	return numHits;
}

static int getBestHit( const hkVector4* m_spheres, int m_numSpheres,
					  hkReal* dist, int* sphereIndex, int nhit,
					  const hkShapeRayCastInput& input,
					  hkShapeRayCastOutput& results)
{
	hkReal bestDist = results.m_hitFraction;
	int bestIndexIndex = -1;
	for( int i = 0; i < nhit; ++i )
	{
		if( dist[i] < bestDist )
		{
			bestDist  = dist[i];
			bestIndexIndex = i;
		}
	}
	if( bestIndexIndex != -1 )
	{
		results.setKey(HK_INVALID_SHAPE_KEY);

		int i = sphereIndex[bestIndexIndex];
		results.m_hitFraction = bestDist;
		hkVector4 localTo; localTo.setSub4( input.m_to, m_spheres[i] );
		hkVector4 localFrom; localFrom.setSub4( input.m_from, m_spheres[i] );
		results.m_normal.setInterpolate4( localFrom, localTo, bestDist );
		results.m_normal.mul4( 1.0f / m_spheres[i](3) );
		results.m_extraInfo = i;

		return bestIndexIndex;
	}
	return -1;
}

hkBool hkMultiSphereShape::castRay( const hkShapeRayCastInput& input, hkShapeRayCastOutput& results ) const
{
	hkReal dist[MAX_SPHERES];
	int idx[MAX_SPHERES];
	int nhit = castRayInternal(input, m_spheres, m_numSpheres, dist, idx);
	return getBestHit(m_spheres, m_numSpheres,  dist, idx, nhit,  input, results) != -1;
}

void hkMultiSphereShape::castRayWithCollector( const hkShapeRayCastInput& inputLocal, const hkCdBody& cdBody, hkRayHitCollector& collector ) const
{
	HK_ASSERT2(0x4033ce56,  cdBody.getShape() == this, "inconsistent cdBody, shapePointer is wrong" );
	hkReal dist[MAX_SPHERES];
	int idx[MAX_SPHERES];
	int nhit = castRayInternal(inputLocal, m_spheres, m_numSpheres, dist, idx);
	
	hkShapeRayCastOutput results;
	while(nhit)
	{
		results.reset();
		int indexIndex = getBestHit(m_spheres, m_numSpheres,  dist, idx, nhit,  inputLocal, results);

		results.m_normal._setMul3( cdBody.getTransform().getRotation(), results.m_normal );
		collector.addRayHit( cdBody, results );

		--nhit;
		dist[indexIndex] = dist[nhit];
		idx[indexIndex] = idx[nhit];
	}
}


void hkMultiSphereShape::calcStatistics( hkStatisticsCollector* collector ) const
{
	collector->beginObject("MultiSphere", collector->MEMORY_SHARED, this);
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
