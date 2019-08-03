/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>
#include <hkcollide/shape/triangle/hkTriangleShape.h>
#include <hkcollide/util/hkAabbUtil.h>
#include <hkcollide/shape/hkShapeRayCastInput.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkTriangleShape);

#include <hkvisualize/hkDebugDisplay.h>

#if HK_POINTER_SIZE==4
HK_COMPILE_TIME_ASSERT( sizeof ( hkTriangleShape ) == 4 * 16 );
#endif


hkShapeType hkTriangleShape::getType() const
{
	return HK_SHAPE_TRIANGLE;
}


void hkTriangleShape::getSupportingVertex( const hkVector4& direction, hkCdVertex& supportingVertex ) const
{
	// binary equivilants
//!me do this when triangle uses new math lib	const hkRotation& v3 = reinterpret_cast< const hkRotation& >(m_triangle);
	const hkRotation* v3 = reinterpret_cast< const hkRotation* >(getVertices());

	hkVector4 supportDots;		supportDots._setRotatedInverseDir( *v3, direction );

	hkReal max;
	int vID;
	if( supportDots(2) > supportDots(1) )
	{
		vID = hkSizeOf(hkVector4)*2;
		max = supportDots(2);
	}
	else
	{
		vID = hkSizeOf(hkVector4);
		max = supportDots(1);
	}
	vID = ( supportDots(0) > max ) ? 0 : vID;

	static_cast<hkVector4&>(supportingVertex) = *hkAddByteOffsetConst<hkVector4>( getVertices(), vID );
	supportingVertex.setInt24W( vID );
}

void hkTriangleShape::convertVertexIdsToVertices( const hkVertexId* ids, int numIds, hkCdVertex* verticesOut) const
{
	for (int i = numIds-1; i>=0; i--)
	{
		static_cast<hkVector4&>(verticesOut[0]) =  *hkAddByteOffsetConst<hkVector4>( getVertices(), ids[0] );	// do a quick address calculation
		verticesOut[0].setInt24W( ids[0] );
		verticesOut++;
		ids++;
	}
}

void hkTriangleShape::getFirstVertex(hkVector4& v) const
{
	v = getVertex(0);
}

int hkTriangleShape::getNumVertices() const
{
	return 3; 
}

void hkTriangleShape::getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out  ) const
{
	hkVector4 tol4;
	tol4.setAll( tolerance + m_radius );

	hkVector4 v0; v0.setTransformedPos( localToWorld, getVertex(0) );
	hkVector4 v1; v1.setTransformedPos( localToWorld, getVertex(1) );
	hkVector4 v2; v2.setTransformedPos( localToWorld, getVertex(2) );

	out.m_min.setMin4( v0, v1 );
	out.m_max.setMax4( v0, v1 );
	out.m_min.setMin4( out.m_min, v2 );
	out.m_max.setMax4( out.m_max, v2 );

	out.m_min.sub4( tol4 );
	out.m_max.add4( tol4 );
}

void hkTriangleShape::getCollisionSpheresInfo( hkCollisionSpheresInfo& infoOut ) const
{
	infoOut.m_numSpheres = 3;
	infoOut.m_useBuffer = true;
}

const hkSphere* hkTriangleShape::getCollisionSpheres( hkSphere* sphereBuffer ) const
{
	hkSphere* s = sphereBuffer;

	{
		s->setPositionAndRadius( getVertex(0) );
		s->setRadius( m_radius );
		s++;
	}
	{
		s->setPositionAndRadius( getVertex(1) );
		s->setRadius( m_radius );
		s++;
	}
	{
		s->setPositionAndRadius( getVertex(2) );
		s->setRadius( m_radius );
		s++;
	}
	return sphereBuffer;
}

hkBool hkTriangleShape::castRay(const hkShapeRayCastInput& input, hkShapeRayCastOutput& results) const
{
	HK_TIMER_BEGIN("rcTriangle", HK_NULL);
	const hkVector4 &vert0=getVertex(0);
	const hkVector4 &vert1=getVertex(1);
	const hkVector4 &vert2=getVertex(2);

	hkVector4 v10; v10.setSub4( vert1, vert0 );
	hkVector4 v20; v20.setSub4( vert2, vert0 );

	hkVector4 triangleNormal; triangleNormal.setCross( v10, v20 );
	
	const hkReal dist = vert0.dot3(triangleNormal);
	hkReal dist_a = triangleNormal.dot3( input.m_from ) ;
	dist_a-= dist;
	hkReal dist_b = triangleNormal.dot3( input.m_to );
	dist_b -= dist;

	if ( dist_a * dist_b >= 0.0f)
	{
		// start point on the surface should give a hit
		if ( dist_a != 0.f || dist_b == 0.f )
		{
			HK_TIMER_END();
			return false; // same sign
		}
	}
	
	const hkReal proj_length = dist_a-dist_b;
	const hkReal distance = (dist_a)/(proj_length);
	// Now we have the intersection point on the plane, we'll see if it's inside the triangle
	// Add an epsilon as a tolerance for the raycast,
	// in case the ray hits exacly on the edge of the triangle.
	// It must be scaled for the triangle size.
	if(distance < results.m_hitFraction)
	{
		hkReal edge_tolerance =triangleNormal.lengthSquared3();		
		edge_tolerance *= -0.0001f;
		hkVector4 point; point.setInterpolate4( input.m_from, input.m_to, distance);
		{
			hkVector4 v0p; v0p.setSub4(vert0, point);
			hkVector4 v1p; v1p.setSub4(vert1, point);
			hkVector4 cp0; cp0.setCross( v0p, v1p );

			if ( (hkReal)(cp0.dot3(triangleNormal)) >=edge_tolerance) 
			{
				hkVector4 v2p; v2p.setSub4(vert2, point);
				hkVector4 cp1;
				cp1.setCross(v1p, v2p);
				if ( (hkReal)(cp1.dot3(triangleNormal)) >=edge_tolerance) 
				{
					hkVector4 cp2;
					cp2.setCross(v2p, v0p);
					if ( (hkReal)(cp2.dot3(triangleNormal)) >=edge_tolerance) 
					{
						results.m_hitFraction = distance;
						if ( dist_a > 0 )
						{
							results.m_normal = triangleNormal;
						}
						else
						{
							results.m_normal.setNeg4(triangleNormal);
						}
						results.m_normal.normalize3();
						results.setKey(HK_INVALID_SHAPE_KEY);
						HK_TIMER_END();
						return true;
					}
				}
			}
		}
	}
	HK_TIMER_END();
	return false;
}

void hkTriangleShape::calcStatistics( hkStatisticsCollector* collector ) const
{
	collector->beginObject("TriangleShp", collector->MEMORY_SHARED, this);
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
