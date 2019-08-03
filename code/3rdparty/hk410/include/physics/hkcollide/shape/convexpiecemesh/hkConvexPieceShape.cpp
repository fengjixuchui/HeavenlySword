/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>

#include <hkcollide/shape/convexpiecemesh/hkConvexPieceShape.h>
#include <hkcollide/shape/hkShapeRayCastInput.h>
#include <hkcollide/shape/hkShapeType.h>

#include <hkcollide/shape/hkRayShapeCollectionFilter.h>

hkConvexPieceShape::hkConvexPieceShape( hkReal radius )
: hkConvexShape( radius )
{
}


hkShapeType hkConvexPieceShape::getType() const
{
	return HK_SHAPE_CONVEX_PIECE;
}

void hkConvexPieceShape::getFirstVertex(hkVector4& v) const
{
	HK_ASSERT( 0x0, m_numVertices > 0 );
	v = m_vertices[0];
}

void hkConvexPieceShape::getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out ) const
{
	out.m_min.setAll3(  HK_REAL_MAX);
	out.m_max.setAll3( -HK_REAL_MAX );

	for ( int i = 0 ; i < m_numVertices ; i++ )
	{
		hkVector4 vWorld; vWorld._setTransformedPos( localToWorld, m_vertices[ i ] );
		out.m_min.setMin4( out.m_min, vWorld );
		out.m_max.setMax4( out.m_max, vWorld );
	}

	hkVector4 tol4; tol4.setAll3( tolerance + m_radius);
	out.m_min.sub4( tol4 );
	out.m_max.add4( tol4 );
}


void hkConvexPieceShape::getCollisionSpheresInfo( hkCollisionSpheresInfo& infoOut ) const
{
	infoOut.m_numSpheres = m_numVertices;
	infoOut.m_useBuffer = true;
}

const hkSphere* hkConvexPieceShape::getCollisionSpheres( hkSphere* sphereBuffer ) const
{
	hkSphere* s = sphereBuffer;

	for ( int i = 0 ; i < m_numVertices ; i++ )
	{
		s->setPositionAndRadius(m_vertices[ i ]);
		s->setRadius( m_radius );
		s++;
	}

	return sphereBuffer;
}


void hkConvexPieceShape::getSupportingVertex( const hkVector4& dir, hkCdVertex& supportingVertex ) const
{
	hkReal maxDot = - HK_REAL_MAX;
	int vertexId = 0;

	hkShapeCollection::ShapeBuffer triangleBuffer;
	for ( int i = 0; i < m_numDisplayShapeKeys; i++)
	{
		hkCdVertex support;

		const hkTriangleShape* triangle = static_cast< const hkTriangleShape*>( m_displayMesh->getChildShape(m_displayShapeKeys[i], triangleBuffer) );
		triangle->getSupportingVertex( dir, support );
		hkReal dot = support.dot3( dir );
		if ( dot > maxDot )
		{
			maxDot = dot;
			supportingVertex = support;
			vertexId = i*3 + (support.getId() / hkSizeOf(hkVector4));
		}
	}
	supportingVertex.setInt24W( vertexId );
}

void hkConvexPieceShape::convertVertexIdsToVertices( const hkVertexId* ids, int numIds, hkCdVertex* verticesOut) const
{
	hkShapeCollection::ShapeBuffer triangleBuffer;
	for (int i = numIds-1; i>=0; i--)
	{
		int vertexId = ids[0];
		hkVector4& v = verticesOut[0];

		HK_ASSERT( 0x0, vertexId < m_numDisplayShapeKeys*3 );

		const hkTriangleShape* triangle = static_cast< const hkTriangleShape*>( m_displayMesh->getChildShape(m_displayShapeKeys[vertexId/3], triangleBuffer) );
		v = triangle->getVertex( vertexId%3 );
		v.setInt24W( vertexId );
		verticesOut++;
		ids++;
	}
}

hkBool hkConvexPieceShape::castRay(const hkShapeRayCastInput& input,hkShapeRayCastOutput& results) const
{
	HK_TIMER_BEGIN("rcConvxPiece", HK_NULL);
	if( m_numDisplayShapeKeys == 0 )
	{
		HK_WARN(0x530ccd4f, "You are trying to raycast against a triangulated convex shape with no plane equations. Raycasting will always return no hit in this case.");
	}

	hkShapeCollection::ShapeBuffer buffer;
	int closestKey = HK_INVALID_SHAPE_KEY;
	results.changeLevel(1);

	for ( int i = 0 ; i < m_numDisplayShapeKeys ;i++ )
	{
		// only raycast against child Shapes that have collisions enabled.

		if ( input.m_rayShapeCollectionFilter )
		{
			if ( !input.m_rayShapeCollectionFilter->isCollisionEnabled( input, *m_displayMesh, *m_displayMesh, m_displayShapeKeys[i] ) )
			{
				// ignore this childShape
				continue;
			}
		}
		
		const hkShape* childShape = m_displayMesh->getChildShape( m_displayShapeKeys[i], buffer );

		// Return the closest hit
		if ( childShape->castRay( input, results ))
		{
			closestKey = i;
		}
	}
	results.changeLevel(-1);
	if( hkShapeKey(closestKey) != HK_INVALID_SHAPE_KEY )
	{
		results.setKey(closestKey);
	}
	HK_TIMER_END();
	return ( closestKey != -1 );
}

void hkConvexPieceShape::castRayWithCollector( const hkShapeRayCastInput& input, const hkCdBody& cdBody, hkRayHitCollector& collector ) const
{
	HK_TIMER_BEGIN("rcConvxPiece", HK_NULL);
	if( m_numDisplayShapeKeys == 0 )
	{
		HK_WARN(0x530ccd4f, "You are trying to raycast against a triangulated convex shape with no plane equations. Raycasting will always return no hit in this case.");
	}

	hkShapeCollection::ShapeBuffer buffer;

	for ( int i = 0 ; i < m_numDisplayShapeKeys ;i++ )
	{
		// only raycast against child Shapes that have collisions enabled.

		if ( input.m_rayShapeCollectionFilter )
		{
			if ( !input.m_rayShapeCollectionFilter->isCollisionEnabled( input, *m_displayMesh, *m_displayMesh, m_displayShapeKeys[i] ) )
			{
				// ignore this childShape
				continue;
			}
		}
		
		const hkShape* childShape = m_displayMesh->getChildShape( m_displayShapeKeys[i], buffer );
		hkCdBody body(&cdBody);
		body.setShape(childShape, i);
		childShape->castRayWithCollector( input, body, collector );
	}
	HK_TIMER_END();
}

const hkShapeContainer* hkConvexPieceShape::getContainer() const
{
	return this;
}

// Don't return our shape keys, but an index into our shape keys.

hkShapeKey hkConvexPieceShape::getFirstKey() const
{
	return 0;
}

hkShapeKey hkConvexPieceShape::getNextKey( hkShapeKey oldKey ) const
{
	int newKey = oldKey + 1;
	return (newKey < m_numDisplayShapeKeys)
		? newKey
		: HK_INVALID_SHAPE_KEY;
}

const hkShape* hkConvexPieceShape::getChildShape( hkShapeKey key, ShapeBuffer& buffer ) const
{
	HK_ASSERT( 0, hkUlong(key) < hkUlong(m_numDisplayShapeKeys) );
	return m_displayMesh->getChildShape( m_displayShapeKeys[key], buffer );
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
