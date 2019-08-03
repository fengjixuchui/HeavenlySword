/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>

#include <hkcollide/shape/fastmesh/hkFastMeshShape.h>
#include <hkcollide/shape/triangle/hkTriangleShape.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkFastMeshShape);

const hkShape* hkFastMeshShape::getChildShape( hkShapeKey key, ShapeBuffer& buffer ) const
{
	HK_ASSERT2(0x6810be4e,  m_subparts.getSize() == 1, "hkFastMeshShape only works with one subpart" );

	// Extract triangle index and sub-part index
	const hkUint32 triangleIndex = key;

	// Grab a handle to the sub-part
	const Subpart& part = m_subparts[ 0 ];

	HK_ASSERT2(0x27c96ec3,  part.m_stridingType == INDICES_INT16, "hkFastMeshShape only works with INDICES_INT16");
	HK_ASSERT2(0x3e4da054,  (part.m_vertexStriding & 15) == 0, "hkFastMeshShape only works with vertex striding of multiple of 16");
	HK_ASSERT2(0x21ca33fc,  (hkUlong(part.m_vertexBase) & 15) == 0, "hkFastMeshShape only works with aligned vertices");

	// The three triangle vertices as float pointers to be filled in
	const hkUint16* triangle = hkAddByteOffsetConst<hkUint16>( (const hkUint16*)part.m_indexBase, part.m_indexStriding * triangleIndex );
	
	// Grab the vertices
	const hkVector4* base = reinterpret_cast<const hkVector4*>( part.m_vertexBase );
	const hkVector4* vf0 = hkAddByteOffsetConst<hkVector4>( base, part.m_vertexStriding * triangle[0] );
	const hkVector4* vf1 = hkAddByteOffsetConst<hkVector4>( base, part.m_vertexStriding * triangle[1] );
	const hkVector4* vf2 = hkAddByteOffsetConst<hkVector4>( base, part.m_vertexStriding * triangle[2] );
	
	HK_ASSERT(0x51b0bd8f,  sizeof( hkTriangleShape ) <= HK_SHAPE_BUFFER_SIZE );
	hkTriangleShape *triangleShape = new( buffer ) hkTriangleShape( m_radius );

	triangleShape->getVertex(0).setMul4( *vf0, m_scaling );
	triangleShape->getVertex(1).setMul4( *vf1, m_scaling );
	triangleShape->getVertex(2).setMul4( *vf2, m_scaling );

	return triangleShape;
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
