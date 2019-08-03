/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>

#include <hkmath/basetypes/hkGeometry.h>

#include <hkcollide/shape/mesh/hkMeshMaterial.h>
#include <hkcollide/shape/triangle/hkTriangleShape.h>
#include <hkcollide/shape/simplemesh/hkSimpleMeshShape.h>
#include <hkcollide/shape/convextranslate/hkConvexTranslateShape.h>
#include <hkcollide/shape/convextransform/hkConvexTransformShape.h>
#include <hkcollide/shape/extendedmeshshape/hkExtendedMeshShape.h>
#include <hkcollide/util/hkTriangleUtil.h>
#include <hkbase/class/hkTypeInfo.h>


extern hkReal hkDefaultTriangleDegeneracyTolerance;

HK_REFLECTION_DEFINE_VIRTUAL(hkExtendedMeshShape);

hkExtendedMeshShape::hkExtendedMeshShape( hkReal radius, int numBitsForSubpartIndex )
:	hkShapeCollection( )
{
	m_scaling.setAll3( 1.0f );
	m_radius = radius;

	HK_ASSERT2(0x16aa7e0a, numBitsForSubpartIndex > 0 && numBitsForSubpartIndex < 32,\
		"cinfo.m_numBitsForSubpartIndex must be greater than zero and less than 32."\
		"See comment in construction info for details on how this parameter is used.");

	m_numBitsForSubpartIndex = numBitsForSubpartIndex;

}

hkExtendedMeshShape::~hkExtendedMeshShape()
{
	for (int s =0; s < m_shapesSubparts.getSize(); s++)
	{
		ShapesSubpart& part = m_shapesSubparts[s];
		for (int i =0; i < part.m_numChildShapes; i++ ){ part.m_childShapes[i]->removeReference(); }
		hkDeallocateChunk( (void**)part.m_childShapes, part.m_numChildShapes, HK_MEMORY_CLASS_CDINFO );
	}
}

hkExtendedMeshShape::hkExtendedMeshShape( hkFinishLoadedObjectFlag flag )
	:	hkShapeCollection(flag),
		m_trianglesSubparts(flag),
		m_shapesSubparts(flag)
{
	if( flag.m_finishing )
	{
		// 3.0 compatibility. m_materialIndexStridingType is loaded as binary zero
		// For 3.0 files material indices are always int8
		{
			for( int i = 0; i < m_trianglesSubparts.getSize(); ++i )
			{
				if( m_trianglesSubparts[i].m_materialIndexStridingType == MATERIAL_INDICES_INVALID )
				{
					m_trianglesSubparts[i].m_materialIndexStridingType = MATERIAL_INDICES_INT8;
				}
			}
		}
		{
			for( int i = 0; i < m_shapesSubparts.getSize(); ++i )
			{
				if( m_shapesSubparts[i].m_materialIndexStridingType == MATERIAL_INDICES_INVALID )
				{
					m_shapesSubparts[i].m_materialIndexStridingType = MATERIAL_INDICES_INT8;
				}
			}
		}
	}
}

hkShapeType hkExtendedMeshShape::getType() const
{ 
	return HK_SHAPE_COLLECTION; 
}

hkShapeKey hkExtendedMeshShape::getFirstKey() const
{
	if ( ( getNumTrianglesSubparts() + getNumShapesSubparts() ) == 0 )
	{
		return HK_INVALID_SHAPE_KEY;
	}

	hkShapeCollection::ShapeBuffer buffer;
	hkShapeKey firstKey = (m_trianglesSubparts.getSize())? 0 : hkShapeKey(1 << 31);

	const hkShape* shape = getChildShape(firstKey, buffer );

	if ( shape->getType() != HK_SHAPE_TRIANGLE )
	{
		return firstKey; // first shape key is 0, but flagged as SHAPE
	}

	const hkTriangleShape* tri = static_cast<const hkTriangleShape*>(shape);
	if ( !hkTriangleUtil::isDegenerate( tri->getVertices()[0], tri->getVertices()[1], tri->getVertices()[2], hkDefaultTriangleDegeneracyTolerance ) )
	{
		return firstKey; 
	}

	return getNextKey( firstKey );
}

// Get the next child shape key.
hkShapeKey hkExtendedMeshShape::getNextKey( hkShapeKey initialKey ) const
{
	hkShapeCollection::ShapeBuffer buffer;

	unsigned int subpartIndex  = HK_MESH2SHAPE_EXTRACT_SUBPART_INDEX (initialKey);
	int          terminalIndex = HK_MESH2SHAPE_EXTRACT_TERMINAL_INDEX(initialKey);

	int subpartType = HK_MESH2SHAPE_IS_SUBPART_TYPE_SHAPES(initialKey);

	while (1)
	{
		if ( subpartType == 0  )
		{
			if ( ++terminalIndex >= m_trianglesSubparts[subpartIndex].m_numTriangleShapes )
			{
				terminalIndex = 0;
				if ( ++subpartIndex >= unsigned(m_trianglesSubparts.getSize()) )
				{
					if ( m_shapesSubparts.getSize() == 0 )
					{
						return HK_INVALID_SHAPE_KEY;
					}

					// continue with shape list
					subpartType = 1<<31;
					subpartIndex = 0;
					terminalIndex = -1;
					continue;
				}
			}
		}
		else // subpartType == 1<<31
		{
			if ( ++terminalIndex >= m_shapesSubparts[subpartIndex].m_numChildShapes )
			{
				if ( ++subpartIndex >= unsigned(m_shapesSubparts.getSize()) )
				{
					return HK_INVALID_SHAPE_KEY;
				}
				terminalIndex = 0;
			}
		}

		// calculate shape key for subpart
		hkShapeKey key = subpartType | ( subpartIndex << ( 32 - m_numBitsForSubpartIndex )) | terminalIndex;

		//
		//	check for valid triangle
		//

		const hkShape* shape = getChildShape(key, buffer );

		if ( shape->getType() != HK_SHAPE_TRIANGLE )
		{
			return key;
		}

		const hkTriangleShape* tri = static_cast<const hkTriangleShape*>(shape);
		if ( !hkTriangleUtil::isDegenerate( tri->getVertices()[0], tri->getVertices()[1], tri->getVertices()[2], hkDefaultTriangleDegeneracyTolerance ) )
		{
			return key; 
		}
	}
}

HK_COMPILE_TIME_ASSERT( sizeof(hkConvexTransformShape ) < hkShapeCollection::HK_SHAPE_BUFFER_SIZE);

const hkShape* hkExtendedMeshShape::getChildShape( hkShapeKey key, ShapeBuffer& buffer ) const
{
	// Extract triangle/child shape index and sub-part index
	const hkUint32 subpartIndex  = HK_MESH2SHAPE_EXTRACT_SUBPART_INDEX (key);
	const hkUint32 terminalIndex = HK_MESH2SHAPE_EXTRACT_TERMINAL_INDEX(key);

	// Grab a handle to the sub-part
	// We need the 'm_trianglesSubparts.getSize() > 0' check for the case when key == 0 (e.g. called from getFirstKey() ).
	if ( !HK_MESH2SHAPE_IS_SUBPART_TYPE_SHAPES(key) )
	{
		const TrianglesSubpart& part = m_trianglesSubparts[subpartIndex];

		HK_ASSERT2(0xad45bb32, part.m_indexBase, "Invalid mesh shape. First subpart has no elements/triangles.");

		// The three triangle vertices as float pointers to be filled in
		const float* vf0 = HK_NULL;
		const float* vf1 = HK_NULL;
		const float* vf2 = HK_NULL;

		// Extract the triangle indicies and vertices
		if ( part.m_stridingType == INDICES_INT16 )
		{
			const hkUint16* triangle = hkAddByteOffsetConst<hkUint16>( (const hkUint16*)part.m_indexBase, part.m_indexStriding * terminalIndex );
		
			// Grab the vertices
			vf0 = hkAddByteOffsetConst<float>( part.m_vertexBase, part.m_vertexStriding * triangle[0] );
			vf1 = hkAddByteOffsetConst<float>( part.m_vertexBase, part.m_vertexStriding * triangle[1] );
			vf2 = hkAddByteOffsetConst<float>( part.m_vertexBase, part.m_vertexStriding * triangle[2] );
		}
		else
		{
			const hkUint32* triangle = hkAddByteOffsetConst<hkUint32>( (const hkUint32*)part.m_indexBase, part.m_indexStriding * terminalIndex);
		
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
	else
	{
		const ShapesSubpart& part = m_shapesSubparts[subpartIndex];

		HK_ASSERT2( 0xf0323445, terminalIndex < (hkUint32)(part.m_numChildShapes), "Invalid shape key");
		const hkConvexShape* childShape = part.m_childShapes[terminalIndex];
		if ( !part.m_offsetSet )
		{
			return childShape;
		}
		else
		{
			if ( !part.m_rotationSet )
			{
				hkConvexTranslateShape* shape = new (buffer) hkConvexTranslateShape( childShape, part.m_transform.getTranslation() );
				return shape;
			}
			else
			{
				hkConvexTransformShape* shape = new (buffer) hkConvexTransformShape( childShape, part.m_transform );
				return shape;
			}
		}
	}
}

hkUint32 hkExtendedMeshShape::getCollisionFilterInfo( hkShapeKey key ) const
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



static inline void HK_CALL addToAabb(hkAabb& aabb, const hkTransform& localToWorld, const float* v,const hkVector4& scaling)
{
	hkVector4 vLocal; vLocal.set( scaling(0)*v[0], scaling(1)*v[1], scaling(2)*v[2], 0.0f );
	hkVector4 vWorld; vWorld.setTransformedPos( localToWorld, vLocal );

	aabb.m_min.setMin4( aabb.m_min, vWorld );
	aabb.m_max.setMax4( aabb.m_max, vWorld );
}



void hkExtendedMeshShape::getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out  ) const
{

	out.m_min.setAll(  HK_REAL_MAX );
	out.m_max.setNeg4( out.m_min );

	hkVector4 tol4; tol4.setAll( tolerance + m_radius );

	{
		for (int s = 0; s < m_trianglesSubparts.getSize(); s++)
		{
			const TrianglesSubpart& part = m_trianglesSubparts[s];

			hkAabb childAabb;
			childAabb.m_min.setAll(  HK_REAL_MAX );
			childAabb.m_max.setNeg4( out.m_min );

			// as getAabb is the first thing to be called upon addition of the shape
			// to a world, we check in debug iof the ptrs are ok
			HK_ASSERT2(0x6541f816, part.m_indexBase, "No indices provided in a subpart of a hkExtendedMeshShape." );
			HK_ASSERT2(0x6541f817, part.m_vertexBase, "No vertices provided in a subpart of a hkExtendedMeshShape." );
			for (int v = 0; v < part.m_numTriangleShapes; v++ )
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


				addToAabb(childAabb, localToWorld, vf0, m_scaling);
				addToAabb(childAabb, localToWorld, vf1, m_scaling);
				addToAabb(childAabb, localToWorld, vf2, m_scaling);
			}
			childAabb.m_min.sub4( tol4 );
			childAabb.m_max.add4( tol4 );
			out.m_min.setMin4( out.m_min, childAabb.m_min );
			out.m_max.setMax4( out.m_max, childAabb.m_max );
		}
	}

	{
		for (int s = 0; s < m_shapesSubparts.getSize(); s++)
		{
			const ShapesSubpart& part = m_shapesSubparts[s];
			for (int i =0 ; i < part.m_numChildShapes; i++)
			{
				hkAabb childAabb;
				part.m_childShapes[i]->getAabb( localToWorld, tolerance, childAabb );
				out.m_min.setMin4( out.m_min, childAabb.m_min );
				out.m_max.setMax4( out.m_max, childAabb.m_max );
			}
		}
	}

}

void hkExtendedMeshShape::setScaling( const hkVector4& scaling ) 
{
	m_scaling = scaling;
}

void hkExtendedMeshShape::assertTrianglesSubpartValidity( const TrianglesSubpart& part )
{
	HK_ASSERT2(0x68fb31d4, m_trianglesSubparts.getSize() < ((1 << (m_numBitsForSubpartIndex-1)) ), "You are adding too many triangle subparts for the mesh shape. "\
		"You can change the number of bits usable for the subpart index by changing the m_numBitsForSubpartIndex in the mesh construction info.");

	HK_ASSERT2(0x6541f716,  part.m_vertexBase, "Subpart vertex base pointer is not set or null.");
	HK_ASSERT2(0x426c5d43,  part.m_vertexStriding >= 4, "Subpart vertex striding is not set or invalid (less than 4 bytes stride).");
	HK_ASSERT2(0x2223ecab,  part.m_numVertices > 0, "Subpart num vertices is not set or negative.");
	HK_ASSERT2(0x5a93ebb6,  part.m_indexBase, "Subpart index base pointer is not set or null.");
	HK_ASSERT2(0x12131a31,  ((part.m_stridingType == INDICES_INT16) || (part.m_stridingType == INDICES_INT32)), 
		"Subpart index type is not set or out of range (16 or 32 bit only).");
	HK_ASSERT2(0x492cb07c,  part.m_indexStriding >= 2, 
		"Subpart index striding pointer is not set or invalid (less than 2 bytes stride).");
	HK_ASSERT2(0x53c3cd4f,  part.m_numTriangleShapes > 0, "Subpart num shapes is not set or negative.");
	HK_ASSERT2(0xad5aae43,  part.m_materialIndexBase == HK_NULL || part.m_materialIndexStridingType == MATERIAL_INDICES_INT8 || part.m_materialIndexStridingType == MATERIAL_INDICES_INT16, "Subpart materialIndexStridingType is not set or out of range (8 or 16 bit only).");

	HK_ASSERT2(0x7b8c4c78,	part.m_numTriangleShapes-1 < (1<<(32-m_numBitsForSubpartIndex)),
		"There are only 32 bits available to index the subpart and triangle in a "
		"hkExtendedMeshShape. This subpart has too many triangles. Attempts to index a "
		"triangle could overflow the available bits. Try decreasing the number of "
		"bits reserved for the subpart index.");
}

void hkExtendedMeshShape::assertShapesSubpartValidity( const ShapesSubpart& part )
{
	HK_ASSERT2(0x68fb32d4, m_shapesSubparts.getSize() < ((1 << (m_numBitsForSubpartIndex-1)) ), "You are adding too many shape subparts for the mesh shape. "\
		"You can change the number of bits usable for the subpart index by changing the m_numBitsForSubpartIndex in the mesh construction info.");

	HK_ASSERT2(0x51c3cd4f,  part.m_numChildShapes > 0, "Subpart num shapes is not set or negative.");

	HK_ASSERT2(0x7b834c78,	part.m_numChildShapes-1 < (1<<(32-m_numBitsForSubpartIndex)),
		"There are only 32 bits available to index the subpart and terminal shape in a "
		"hkExtendedMeshShape. This subpart has too many terminal shapes. Attempts to index a "
		"terminal shape could overflow the available bits. Try decreasing the number of "
		"bits reserved for the subpart index.");
}

void hkExtendedMeshShape::addTrianglesSubpart( const TrianglesSubpart& part )
{
	TrianglesSubpart& tsp = m_trianglesSubparts.expandOne();
	tsp = static_cast<const TrianglesSubpart&>(part);
	HK_ON_DEBUG( assertTrianglesSubpartValidity(tsp); )

	// disable materials
	if ( tsp.m_materialIndexBase == HK_NULL)
	{
		tsp.m_numMaterials = 1;
		tsp.m_materialBase      = reinterpret_cast<const hkMeshMaterial*>( &hkVector4::getZero() );
		tsp.m_materialIndexBase = reinterpret_cast<const hkUint8*>( &hkVector4::getZero() );
	}
}

void hkExtendedMeshShape::addShapesSubpart( const ShapesSubpart& part )
{
	ShapesSubpart& ssp = m_shapesSubparts.expandOne();
	const ShapesSubpart& in = static_cast<const ShapesSubpart&>(part);
	ssp = in;
	HK_ON_DEBUG( assertShapesSubpartValidity(ssp); )

	// copy shape array
	{
		ssp.m_childShapes = hkAllocateChunk<const hkConvexShape*const>(ssp.m_numChildShapes, HK_MEMORY_CLASS_CDINFO);
		hkString::memCpy4( (void*)(ssp.m_childShapes), in.m_childShapes, in.m_numChildShapes );
		for (int i=0; i < ssp.m_numChildShapes; i++) { ssp.m_childShapes[i]->addReference(); }
	}

	// disable materials
	if ( ssp.m_materialIndexBase == HK_NULL)
	{
		ssp.m_numMaterials = 1;
		ssp.m_materialBase      = reinterpret_cast<const hkMeshMaterial*>( &hkVector4::getZero() );
		ssp.m_materialIndexBase = reinterpret_cast<const hkUint8*>( &hkVector4::getZero() );
	}
}

hkExtendedMeshShape::ShapesSubpart::ShapesSubpart( const hkConvexShape*const* childShapes, int numChildShapes, const hkVector4& offset ) : Subpart( SUBPART_SHAPE )
{
	// copy shape array
	{
		m_childShapes = hkAllocateChunk<const hkConvexShape*const>(numChildShapes, HK_MEMORY_CLASS_CDINFO);
		m_numChildShapes = numChildShapes;

		hkString::memCpy4( (void*)(m_childShapes), childShapes, numChildShapes );
		for (int i=0; i < numChildShapes; i++) { m_childShapes[i]->addReference(); }
	}

	// init offset
	{
		m_transform.getRotation().setIdentity();
		m_rotationSet = false;
		m_transform.setTranslation( offset );

		if ( offset.equals3(hkVector4::getZero()))
		{
			m_offsetSet = false;
		}
		else
		{
			m_offsetSet = true;
		}
	}
}


hkExtendedMeshShape::ShapesSubpart::ShapesSubpart( const hkConvexShape*const* childShapes, int numChildShapes, const hkTransform& transform ): Subpart( SUBPART_SHAPE )
{
	// copy shape array
	{
		m_childShapes = hkAllocateChunk<const hkConvexShape*const>(numChildShapes, HK_MEMORY_CLASS_CDINFO);
		m_numChildShapes = numChildShapes;
		hkString::memCpy4( (void*)(m_childShapes), childShapes, numChildShapes );
		for (int i=0; i < numChildShapes; i++) { m_childShapes[i]->addReference(); }
	}
	m_transform = transform; 
	m_offsetSet = true;
	m_rotationSet = true;
}

hkExtendedMeshShape::ShapesSubpart::~ShapesSubpart()
{
	for (int i =0; i < m_numChildShapes; i++ ) { m_childShapes[i]->removeReference(); }
	hkDeallocateChunk( (void**)m_childShapes, m_numChildShapes, HK_MEMORY_CLASS_CDINFO );
}

void hkExtendedMeshShape::calcStatistics( hkStatisticsCollector* collector ) const
{
	collector->beginObject("MeshShape", collector->MEMORY_SHARED, this);
	collector->addArray( "TriangleSubParts", collector->MEMORY_SHARED, this->m_trianglesSubparts );
	collector->addArray( "ShapeSubParts", collector->MEMORY_SHARED, this->m_shapesSubparts );
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
