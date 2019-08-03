/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


#include <hkcollide/hkCollide.h>
#include <hkcollide/shape/simplemesh/hkSimpleMeshShape.h>
#include <hkcollide/shape/triangle/hkTriangleShape.h>
#include <hkcollide/util/hkTriangleUtil.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkSimpleMeshShape);


hkSimpleMeshShape::hkSimpleMeshShape( hkReal radius )
: m_radius ( radius )
{
}


void hkSimpleMeshShape::getAabb(const hkTransform& localToWorld, hkReal tolerance, hkAabb& out) const
{
	// warning: not all vertices might be used, so it is not enough to go through vertexarray !
	// potential optimization (same for hkMeshShape): speedup by lazy evaluation and storing the cached version, having a modified flag

	out.m_min.set(  HK_REAL_MAX,  HK_REAL_MAX,  HK_REAL_MAX );
	out.m_max.set( -HK_REAL_MAX, -HK_REAL_MAX, -HK_REAL_MAX );

	for (int i=0;i<m_vertices.getSize();i++)
	{
		const hkVector4& vLocal = m_vertices[i];
		hkVector4 vWorld; vWorld.setTransformedPos( localToWorld, vLocal );

		out.m_min.setMin4( out.m_min, vWorld );
		out.m_max.setMax4( out.m_max, vWorld );
	}

	hkVector4 tol4; tol4.setAll( tolerance + m_radius);
	out.m_min.sub4( tol4 );
	out.m_max.add4( tol4 );
}


const hkShape* hkSimpleMeshShape::getChildShape( hkShapeKey key, ShapeBuffer& buffer) const
{

	int index = key;
	HK_ASSERT2(0x593f7a2f,  index >= 0 && index < m_triangles.getSize(), "hkShapeKey invalid");

	hkTriangleShape *ts = new( buffer ) hkTriangleShape( m_radius );

	if ( 1 ||  index & 1 )
	{
		ts->setVertex( 0, m_vertices[m_triangles[index].m_a] );
		ts->setVertex( 1, m_vertices[m_triangles[index].m_b] );
		ts->setVertex( 2, m_vertices[m_triangles[index].m_c] );
	}
	else
	{
		ts->setVertex( 2, m_vertices[m_triangles[index].m_a] );
		ts->setVertex( 1, m_vertices[m_triangles[index].m_b] );
		ts->setVertex( 0, m_vertices[m_triangles[index].m_c] );
	}
	return ts;
}


hkShapeKey hkSimpleMeshShape::getFirstKey() const
{
	return 0;
}

hkShapeKey hkSimpleMeshShape::getNextKey( hkShapeKey oldKey ) const
{
	for( int key = int(oldKey)+1; key < m_triangles.getSize(); ++key )
	{
		if ( hkTriangleUtil::isDegenerate( 
			m_vertices[m_triangles[key].m_a],
			m_vertices[m_triangles[key].m_b],
			m_vertices[m_triangles[key].m_c],
			hkDefaultTriangleDegeneracyTolerance ) == false )
		{
			return hkShapeKey(key); 
		}
	}
	return HK_INVALID_SHAPE_KEY;	
}



hkShapeType hkSimpleMeshShape::getType() const
{
	return HK_SHAPE_TRIANGLE_COLLECTION;
}


void hkSimpleMeshShape::calcStatistics( hkStatisticsCollector* collector ) const
{
	collector->beginObject("StorageMesh", collector->MEMORY_SHARED, this);
	collector->addArray( "Vertices", collector->MEMORY_SHARED, this->m_vertices );
	collector->addArray( "Triangles", collector->MEMORY_SHARED, this->m_triangles );
	collector->addArray( "MaterialIds", collector->MEMORY_SHARED, this->m_materialIndices );
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
