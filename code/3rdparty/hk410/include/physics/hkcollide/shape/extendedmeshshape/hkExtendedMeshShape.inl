/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

int hkExtendedMeshShape::getNumTrianglesSubparts() const
{
	return m_trianglesSubparts.getSize();
}

int hkExtendedMeshShape::getNumShapesSubparts() const
{
	return m_shapesSubparts.getSize();
}

hkExtendedMeshShape::TrianglesSubpart& hkExtendedMeshShape::getTrianglesSubpartAt( int i )
{
	HK_ASSERT2(0x3031b232,  i < m_trianglesSubparts.getSize(), "You are trying to access a triangles-subpart which is not in the triangle subpart array");
	return m_trianglesSubparts[i];
}

const hkExtendedMeshShape::TrianglesSubpart& hkExtendedMeshShape::getTrianglesSubpartAt( int i ) const
{
	HK_ASSERT2(0x2bb0d984,  i < m_trianglesSubparts.getSize(), "You are trying to access a triangles-subpart which is not in the triangle subpart array");
	return m_trianglesSubparts[i];
}

hkExtendedMeshShape::ShapesSubpart& hkExtendedMeshShape::getShapesSubpartAt( int i )
{
	HK_ASSERT2(0x30312232,  i < m_shapesSubparts.getSize(), "You are trying to access a shapes-subpart which is not in the shape subpart array");
	return m_shapesSubparts[i];
}

const hkExtendedMeshShape::ShapesSubpart& hkExtendedMeshShape::getShapesSubpartAt( int i ) const
{
	HK_ASSERT2(0x2bb05984,  i < m_shapesSubparts.getSize(), "You are trying to access a shapes-subpart which is not in the shape subpart array");
	return m_shapesSubparts[i];
}

const hkExtendedMeshShape::Subpart& hkExtendedMeshShape::getSubPart( hkShapeKey key ) const
{
	hkUint32 subpartIndex = HK_MESH2SHAPE_EXTRACT_SUBPART_INDEX(key);

	if ( !HK_MESH2SHAPE_IS_SUBPART_TYPE_SHAPES(key) )
	{
		return m_trianglesSubparts[ subpartIndex ];
	}
	else
	{
		return m_shapesSubparts[ subpartIndex ];
	}
}

int hkExtendedMeshShape::getSubPartIndex( hkShapeKey key ) const
{
	const hkUint32 subpartIndex = HK_MESH2SHAPE_EXTRACT_SUBPART_INDEX(key);

	return subpartIndex;
}

int hkExtendedMeshShape::getTerminalIndexInSubPart( hkShapeKey key ) const
{
	int terminalIndex = HK_MESH2SHAPE_EXTRACT_TERMINAL_INDEX (key);
	return terminalIndex;
}

hkInt32 hkExtendedMeshShape::getNumBitsForSubpartIndex() const
{
	return m_numBitsForSubpartIndex;
}

hkExtendedMeshShape::SubpartType hkExtendedMeshShape::getSubpartType( hkShapeKey key ) const
{
	if ( !HK_MESH2SHAPE_IS_SUBPART_TYPE_SHAPES(key) )
	{
		return SUBPART_TRIANGLES;
	}
	else
	{
		return SUBPART_SHAPE;
	}
}

hkReal hkExtendedMeshShape::getRadius() const
{
	return m_radius;
}

void hkExtendedMeshShape::setRadius(hkReal r )
{
	m_radius = r;
}

inline const hkVector4&	hkExtendedMeshShape::getScaling() const
{
	return m_scaling;
}

inline const hkMeshMaterial* hkExtendedMeshShape::getMeshMaterial( hkShapeKey key ) const
{
	const hkUint32 subpartIndex  = HK_MESH2SHAPE_EXTRACT_SUBPART_INDEX (key);
	const hkUint32 terminalIndex = HK_MESH2SHAPE_EXTRACT_TERMINAL_INDEX(key);

	// Grab a handle to the sub-part
	const Subpart* part;
	if ( !HK_MESH2SHAPE_IS_SUBPART_TYPE_SHAPES(key) )
	{
		part = &m_trianglesSubparts[subpartIndex];
	}
	else
	{
		part = &m_shapesSubparts[subpartIndex];
	}

	if (part->m_materialIndexBase)
	{
		int materialId;
		HK_ASSERT2(0xad453fa2, part->m_materialIndexStridingType == MATERIAL_INDICES_INT8 || part->m_materialIndexStridingType == MATERIAL_INDICES_INT16, "Invalid hkExtendedMeshShape::SubPart::m_materialIndexStridingType.");
		if (part->m_materialIndexStridingType == MATERIAL_INDICES_INT8)
		{	
			materialId = *static_cast<const hkUint8 *>(hkAddByteOffsetConst( part->m_materialIndexBase, terminalIndex * part->m_materialIndexStriding ));
		}
		else
		{	
			materialId = *static_cast<const hkUint16*>(hkAddByteOffsetConst( part->m_materialIndexBase, terminalIndex * part->m_materialIndexStriding ));
		}
		HK_ASSERT2(0x26d359f1,  materialId < part->m_numMaterials, "Your mesh references a material which does not exist" );

		return hkAddByteOffsetConst<hkMeshMaterial>( part->m_materialBase, part->m_materialStriding * materialId );
	}
	else
	{
		return HK_NULL;
	}
}

inline hkExtendedMeshShape::Subpart::Subpart(SubpartType type)
{
	m_type = type;
	// materials (default is fine)
	//m_materialIndexStridingType = MATERIAL_INDICES_INVALID;
	m_materialIndexStridingType = MATERIAL_INDICES_INT8;
	m_materialIndexStriding = 0;
	m_materialStriding = 0;
	m_numMaterials = 1;
	m_materialBase = HK_NULL;
	m_materialIndexBase = HK_NULL;
}

inline hkExtendedMeshShape::TrianglesSubpart::TrianglesSubpart(): Subpart( SUBPART_TRIANGLES )
{
	// 'must set' values, defaults are error flags effectively for HK_ASSERTS in the cpp.
#ifdef HK_DEBUG
	m_vertexBase = HK_NULL;
	m_vertexStriding = -1;
	m_numVertices = -1;
	m_indexBase = HK_NULL;
	m_stridingType = INDICES_INVALID;
	m_indexStriding = -1;
	m_numTriangleShapes = -1;
#endif
}

/*
* Havok SDK - PUBLIC RELEASE, BUILD(#20060902)
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
