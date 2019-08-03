/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>
#include <hkcollide/shape/sphere/hkSphereShape.h>
#include <hkcollide/shape/hkShapeRayCastInput.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkSphereShape);


hkSphereShape::hkSphereShape(hkReal radius)	
: hkConvexShape(radius)
{
}

hkShapeType hkSphereShape::getType() const
{
	return HK_SHAPE_SPHERE;
}

void hkSphereShape::getSupportingVertex( const hkVector4& direction, hkCdVertex& supportingVertex ) const
{
	supportingVertex.setZero4();
}

void hkSphereShape::convertVertexIdsToVertices( const hkVertexId* ids, int numIds, hkCdVertex* verticesOut) const
{
	for (int i = numIds-1; i>=0; i--)
	{
		verticesOut[0].setZero4();
		verticesOut++;
	}
}


void hkSphereShape::getFirstVertex(hkVector4& v) const
{
	v.setZero4();
}
		
int hkSphereShape::getNumVertices() const
{ 
	return 1; 
}

void hkSphereShape::getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out ) const
{
	hkVector4 tol4; tol4.setAll( tolerance + m_radius );

	out.m_min.setSub4( localToWorld.getTranslation(), tol4 );
	out.m_max.setAdd4( localToWorld.getTranslation(), tol4 );
}


void hkSphereShape::getCollisionSpheresInfo( hkCollisionSpheresInfo& infoOut ) const
{
	infoOut.m_numSpheres = 1;
	infoOut.m_useBuffer = true;
}

const hkSphere* hkSphereShape::getCollisionSpheres( hkSphere* sphereBuffer ) const
{
	hkSphere& s = *sphereBuffer;
	hkVector4 v; v.set(0,0,0,m_radius);
	s.setPositionAndRadius(v);
	return sphereBuffer;
}





hkBool hkSphereShape::castRay(const hkShapeRayCastInput& input,hkShapeRayCastOutput& results) const
{
	//
	//	This functions is a modified version of
	//  http://www.siggraph.org/education/materials/HyperGraph/raytrace/rtinter1.htm
	//  Modifications include changing the order of if statements to prevent
	//  any division which can produce a number greater than 1
	//

	HK_TIMER_BEGIN("rcSphere", HK_NULL);
	{
	    hkReal	radius2 = m_radius * m_radius;
    
	    // 
	    // solve quadratic function: ax*x + bx + c = 0
	    //
	    hkVector4 dir; dir.setSub4( input.m_to, input.m_from);
    
	    hkReal B = hkReal(dir.dot3( input.m_from ));
	    if ( B >= 0 )
	    {
		    // ray points away from sphere center
		    goto returnFalse;
	    }

		const hkReal A = dir.lengthSquared3();

		//
		//	Check for long rays (check for startpoints being 10 times outside the radius
		//
		hkReal offset;
		hkVector4 midPoint;
		if ( B * B > A * radius2 * 100.0f)
		{
			// no hit if length is smaller than the distance of the startpoint to the center
			if ( A < radius2 )
			{
				goto returnFalse;
			}
			offset = -B;
			midPoint.setInterpolate4(input.m_from, input.m_to, offset/A);
			B = 0.0f;
		}
		else
		{
			offset = 0.0f;
			midPoint = input.m_from;
		}

		const hkReal C = hkReal(midPoint.lengthSquared3()) - radius2;
		const hkReal det = B*B - A*C;

		if ( det <= 0 )
		{
			//
			//	Infinite ray does not hit
			//
			goto returnFalse;
		}

		const hkReal sqDet = hkMath::sqrt( det );

		const hkReal t2 = -B - sqDet;
		hkReal t = t2 + offset;

		if ( t >= (A * results.m_hitFraction))
		{
			//
			//	hits behind endpoint or is greater than previous hit fraction
			//
			goto returnFalse;
		}

		if ( t < 0 )
		{
			//
			// start point inside
			//
			goto returnFalse;
		}

		//  Note: we know that t > 0
		//  Also that A > t 
		//  So this division is safe and results in a point between 0 and 1

		t = t/A;

		results.m_hitFraction = t;
		results.m_normal.setInterpolate4( input.m_from, input.m_to, t );
		results.m_normal.mul4( 1.0f / m_radius );
		results.setKey(HK_INVALID_SHAPE_KEY);
		HK_TIMER_END();
		return true;
	}

returnFalse:
	HK_TIMER_END();
	return false;
}


void hkSphereShape::calcStatistics( hkStatisticsCollector* collector ) const
{
	collector->beginObject("SphereShape", collector->MEMORY_SHARED, this);
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
