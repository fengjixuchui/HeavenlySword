/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>

#include <hkmath/basetypes/hkGeometry.h>

#include <hkcollide/shape/mesh/hkMeshShape.h>
#include <hkcollide/shape/mesh/hkMeshMaterial.h>
#include <hkcollide/shape/triangle/hkTriangleShape.h>
#include <hkcollide/shape/simplemesh/hkSimpleMeshShape.h>
#include <hkcollide/util/hkTriangleUtil.h>
#include <hkbase/class/hkTypeInfo.h>

extern hkReal hkDefaultTriangleDegeneracyTolerance;

HK_REFLECTION_DEFINE_VIRTUAL(hkMeshShape);

hkMeshShape::hkMeshShape( hkReal radius,int numBitsForSubpartIndex )
:	hkShapeCollection( )
{
	m_scaling.setAll3( 1.0f );
	m_radius = radius;

	HK_ASSERT2(0x16aa7e0a, numBitsForSubpartIndex > 0 && numBitsForSubpartIndex < 32,\
		"cinfo.m_numBitsForSubpartIndex must be greater than zero and less than 32."\
		"See comment in construction info for details on how this parameter is used.");

	m_numBitsForSubpartIndex = numBitsForSubpartIndex;

}

hkMeshShape::hkMeshShape( hkFinishLoadedObjectFlag flag )
	:	hkShapeCollection(flag),
		m_subparts(flag)
{
	if( flag.m_finishing )
	{
		// 3.0 compatibility. m_materialIndexStridingType is loaded as binary zero
		// For 3.0 files material indices are always int8
		for( int i = 0; i < m_subparts.getSize(); ++i )
		{
			if( m_subparts[i].m_materialIndexStridingType == MATERIAL_INDICES_INVALID )
			{
				m_subparts[i].m_materialIndexStridingType = MATERIAL_INDICES_INT8;
			}
		}
	}
}

hkShapeType hkMeshShape::getType() const
{ 
	return HK_SHAPE_TRIANGLE_COLLECTION; 
}


hkShapeKey hkMeshShape::getFirstKey() const
{
	if ( m_subparts.getSize() == 0 )
	{
		return HK_INVALID_SHAPE_KEY;
	}

	hkShapeCollection::ShapeBuffer buffer;
	const hkShape* shape = getChildShape(0, buffer );
	const hkTriangleShape* tri = static_cast<const hkTriangleShape*>(shape);
	if ( hkTriangleUtil::isDegenerate( tri->getVertices()[0], tri->getVertices()[1], tri->getVertices()[2], hkDefaultTriangleDegeneracyTolerance ) == false )
	{
		return 0; 
	}
	return getNextKey( 0 );
}

// Get the next child shape key.
hkShapeKey hkMeshShape::getNextKey( hkShapeKey initialKey ) const
{
	hkShapeCollection::ShapeBuffer buffer;

	unsigned subPart = initialKey  >> ( 32 - m_numBitsForSubpartIndex );
	int triIndex = initialKey  & ( ~0U >> m_numBitsForSubpartIndex );

	while (1)
	{
		if ( ++triIndex >= m_subparts[subPart].m_numTriangles )
		{
			if ( ++subPart >= unsigned(m_subparts.getSize()) )
			{
				return HK_INVALID_SHAPE_KEY;
			}
			triIndex = 0;
		}
		hkShapeKey key = ( subPart << ( 32 - m_numBitsForSubpartIndex )) | triIndex;

		//
		//	check for valid triangle
		//

		const hkShape* shape = getChildShape(key, buffer );

		const hkTriangleShape* tri = static_cast<const hkTriangleShape*>(shape);
		if ( hkTriangleUtil::isDegenerate( tri->getVertices()[0], tri->getVertices()[1], tri->getVertices()[2], hkDefaultTriangleDegeneracyTolerance ) == false )
		{
			return key; 
		}
	}
}


const hkShape* hkMeshShape::getChildShape( hkShapeKey key, ShapeBuffer& buffer ) const
{
	// Extract triangle index and sub-part index
	const hkUint32 triangleIndex = key & ( ~0U >> m_numBitsForSubpartIndex );
	const hkUint32 subPartIndex = key >> ( 32 - m_numBitsForSubpartIndex );

	//int triIndex = initialKey  & ( ~0L >> m_numBitsForSubpartIndex );
	//unsigned subPart = initialKey  >> ( 32 - m_numBitsForSubpartIndex );

	// Grab a handle to the sub-part
	const Subpart& part = m_subparts[ subPartIndex ];

	HK_ASSERT2(0xad45bb32, part.m_indexBase, "Invalid mesh shape. First subpart has no elements/triangles.");

	// The three triangle vertices as float pointers to be filled in
	const float* vf0 = HK_NULL;
	const float* vf1 = HK_NULL;
	const float* vf2 = HK_NULL;

	// Extract the triangle indicies and vertices
	if ( part.m_stridingType == INDICES_INT16 )
	{
		const hkUint16* triangle = hkAddByteOffsetConst<hkUint16>( (const hkUint16*)part.m_indexBase, part.m_indexStriding * triangleIndex );
	
		// Grab the vertices
		vf0 = hkAddByteOffsetConst<float>( part.m_vertexBase, part.m_vertexStriding * triangle[0] );
		vf1 = hkAddByteOffsetConst<float>( part.m_vertexBase, part.m_vertexStriding * triangle[1] );
		vf2 = hkAddByteOffsetConst<float>( part.m_vertexBase, part.m_vertexStriding * triangle[2] );
	}
	else
	{
		const hkUint32* triangle = hkAddByteOffsetConst<hkUint32>( (const hkUint32*)part.m_indexBase, part.m_indexStriding * triangleIndex);
	
		// Grab the vertices 
		vf0 = hkAddByteOffsetConst<float>( part.m_vertexBase, part.m_vertexStriding * triangle[0] );
		vf1 = hkAddByteOffsetConst<float>( part.m_vertexBase, part.m_vertexStriding * triangle[1] );
		vf2 = hkAddByteOffsetConst<float>( part.m_vertexBase, part.m_vertexStriding * triangle[2] );
	}

	// generate hkVector4s out of our vertices
	hkVector4 vertex0; vertex0.set( m_scaling(0) * vf0[0], m_scaling(1) * vf0[1], m_scaling(2) * vf0[2], 0.0f );
	hkVector4 vertex1; vertex1.set( m_scaling(0) * vf1[0], m_scaling(1) * vf1[1], m_scaling(2) * vf1[2], 0.0f );
	hkVector4 vertex2; vertex2.set( m_scaling(0) * vf2[0], m_scaling(1) * vf2[1], m_scaling(2) * vf2[2], 0.0f );

	HK_ASSERT(0x73f97fa7,  sizeof( hkTriangleShape ) <= HK_SHAPE_BUFFER_SIZE );
	hkTriangleShape *triangleShape = new( buffer ) hkTriangleShape( m_radius );

	triangleShape->setVertex( 0, vertex0 );
	triangleShape->setVertex( 1, vertex1 );
	triangleShape->setVertex( 2, vertex2 );
	
	return triangleShape;
}

hkUint32 hkMeshShape::getCollisionFilterInfo( hkShapeKey key ) const
{
	const hkMeshMaterial* material = getMeshMaterial(key);

	if (material)
	{
		return material->m_filterInfo;
	}
	else
	{
		return 0;
	}
}



static void HK_CALL addToAabb(hkAabb& aabb, const hkTransform& localToWorld, const float* v,const hkVector4& scaling)
{
	hkVector4 vLocal; vLocal.set( scaling(0)*v[0], scaling(1)*v[1], scaling(2)*v[2], 0.0f );
	hkVector4 vWorld; vWorld.setTransformedPos( localToWorld, vLocal );

	aabb.m_min.setMin4( aabb.m_min, vWorld );
	aabb.m_max.setMax4( aabb.m_max, vWorld );
}



void hkMeshShape::getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out  ) const
{

	out.m_min.set(  HK_REAL_MAX,  HK_REAL_MAX,  HK_REAL_MAX );
	out.m_max.set( -HK_REAL_MAX, -HK_REAL_MAX, -HK_REAL_MAX );


	for (int s = 0; s < m_subparts.getSize(); s++)
	{
		const Subpart& part = m_subparts[s];

		// as getAabb is the first thing to be called upon addition of the shape
		// to a world, we check in debug iof the ptrs are ok
		HK_ASSERT2(0x6541f816, part.m_indexBase, "No indices provided in a subpart of a hkMeshShape." );
		HK_ASSERT2(0x6541f817, part.m_vertexBase, "No vertices provided in a subpart of a hkMeshShape." );

		for (int v = 0; v < part.m_numTriangles; v++ )
		{
			const float* vf0;
			const float* vf1;
			const float* vf2;
 
			if ( part.m_stridingType == INDICES_INT16)
			{
				const hkUint16* tri = hkAddByteOffsetConst<hkUint16>( (const hkUint16*)part.m_indexBase, part.m_indexStriding * v);
				vf0 = hkAddByteOffsetConst<float>(part.m_vertexBase, part.m_vertexStriding * tri[0] );
				vf1 = hkAddByteOffsetConst<float>(part.m_vertexBase, part.m_vertexStriding * tri[1] );
				vf2 = hkAddByteOffsetConst<float>(part.m_vertexBase, part.m_vertexStriding * tri[2] );
			}
			else
			{
				const hkUint32* tri = hkAddByteOffsetConst<hkUint32>( (const hkUint32*)part.m_indexBase, part.m_indexStriding * v);

				vf0 = hkAddByteOffsetConst<float>(part.m_vertexBase, part.m_vertexStriding * tri[0] );
				vf1 = hkAddByteOffsetConst<float>(part.m_vertexBase, part.m_vertexStriding * tri[1] );
				vf2 = hkAddByteOffsetConst<float>(part.m_vertexBase, part.m_vertexStriding * tri[2] );
			}
			addToAabb(out, localToWorld, vf0,m_scaling);
			addToAabb(out, localToWorld, vf1,m_scaling);
			addToAabb(out, localToWorld, vf2,m_scaling);
		}
	}

	hkVector4 tol4; tol4.setAll( tolerance + m_radius );
	out.m_min.sub4( tol4 );
	out.m_max.add4( tol4 );
	
}

void hkMeshShape::setScaling( const hkVector4& scaling ) 
{
	m_scaling = scaling;
}

void hkMeshShape::assertSubpartValidity( const Subpart& part )
{
	HK_ASSERT2(0x68fb31d4, m_subparts.getSize() < ((1 << m_numBitsForSubpartIndex) - 1 ), "You are adding too many subparts for the mesh shape. "\
		"You can change the number of bits usable for the subpart index by changing the m_numBitsForSubpartIndex in the mesh construction info.");

	HK_ASSERT2(0x6541f716,  part.m_vertexBase, "Subpart vertex base pointer is not set or null.");
	HK_ASSERT2(0x426c5d43,  part.m_vertexStriding >= 4, "Subpart vertex striding is not set or invalid (less than 4 bytes stride).");
	HK_ASSERT2(0x2223ecab,  part.m_numVertices > 0, "Subpart num vertices is not set or negative.");
	HK_ASSERT2(0x5a93ebb6,  part.m_indexBase, "Subpart index base pointer is not set or null.");
	HK_ASSERT2(0x12131a31,  ((part.m_stridingType == INDICES_INT16) || (part.m_stridingType == INDICES_INT32)), 
		"Subpart index type is not set or out of range (16 or 32 bit only).");
	HK_ASSERT2(0x492cb07c,  part.m_indexStriding >= 2, 
		"Subpart index striding pointer is not set or invalid (less than 2 bytes stride).");
	HK_ASSERT2(0x53c3cd4f,  part.m_numTriangles > 0, "Subpart num triangles is not set or negative.");
	HK_ASSERT2(0xad5aae43,  part.m_materialIndexBase == HK_NULL || part.m_materialIndexStridingType == MATERIAL_INDICES_INT8 || part.m_materialIndexStridingType == MATERIAL_INDICES_INT16, "Subpart materialIndexStridingType is not set or out of range (8 or 16 bit only).");

	HK_ASSERT2(0x7b8c4c78,	part.m_numTriangles-1 < (1<<(32-m_numBitsForSubpartIndex)),
		"There are only 32 bits available to index the sub-part and triangle in a "
		"hkMeshShape. This subpart has too many triangles, attempts to index a "
		"triangle could overflow the available bits. Try decreasing the number of "
		"bits reserved for the sub-part index.");
}

void hkMeshShape::addSubpart( const Subpart& part )
{
	HK_ON_DEBUG( assertSubpartValidity(part); )
	
	Subpart& p = m_subparts.expandOne();
	p = part;

	// disable materials
	if ( p.m_materialIndexBase == HK_NULL)
	{
		p.m_numMaterials = 1;
		p.m_materialBase = reinterpret_cast<const hkMeshMaterial*>( &hkVector4::getZero() );
		p.m_materialIndexBase = reinterpret_cast<const hkUint8*>( &hkVector4::getZero() );
	}
}


void hkMeshShape::calcStatistics( hkStatisticsCollector* collector ) const
{
	collector->beginObject("MeshShape", collector->MEMORY_SHARED, this);
	collector->addArray( "SubParts", collector->MEMORY_SHARED, this->m_subparts );
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
