/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkmath/hkMath.h>

#include <hkbase/stream/hkOArchive.h>
#include <hkbase/stream/hkIArchive.h>
#include <hkbase/stream/hkOstream.h>
#include <hkbase/stream/hkIstream.h>

#include <hkcollide/shape/simplemesh/hkSimpleMeshShape.h>

#include <hkutilities/collide/hkTklStreamer.h>

hkSimpleMeshShape* HK_CALL hkTklStreamer::readStorageMeshFromTklStream(hkIstream &inputStream)
{
	hkSimpleMeshShape* newShape = new hkSimpleMeshShape();
	int numVertices = 0;
	inputStream >> numVertices;
	HK_ASSERT2(0x27f13f71,  numVertices > 2, "Less than three vertices, invalid tkl file" );
	newShape->m_vertices.setSize( numVertices );	

	for ( int v_it = 0; v_it < numVertices; v_it++ )
	{
		float x,y,z;
		inputStream >> x;
		inputStream >> y;
		inputStream >> z;
		newShape->m_vertices[v_it].set(x,y,z);
	}

	int numTriangles = 0;
	inputStream >> numTriangles;
	HK_ASSERT2(0x698c0975,  numTriangles > 0, "Less than 1 triangle, invalid tkl file" );
	newShape->m_triangles.setSize( numTriangles );

	for ( int tr_it = 0; tr_it < numTriangles; tr_it++ )
	{
		inputStream >> newShape->m_triangles[tr_it].m_a;
		inputStream >> newShape->m_triangles[tr_it].m_b;
		inputStream >> newShape->m_triangles[tr_it].m_c;
	}
	return newShape;
}

void HK_CALL hkTklStreamer::writeStorageMeshShapeToTklStream(hkSimpleMeshShape* shape, hkOstream &outputStream)
{
	const hkArray<hkVector4>& vertices = shape->m_vertices;
	const hkArray<hkSimpleMeshShape::Triangle>& triangles = shape->m_triangles;

	outputStream << vertices.getSize() << "\n";
	for ( int v_it = 0; v_it < vertices.getSize(); v_it++ )
	{
		outputStream << vertices[v_it](0) << " ";
		outputStream << vertices[v_it](1) << " ";
		outputStream << vertices[v_it](2);
		outputStream << "\n";
	}
	
	outputStream << triangles.getSize() << "\n";
	for ( int tri_it = 0; tri_it < triangles.getSize(); tri_it++ )
	{
		outputStream << triangles[tri_it].m_a << " ";
		outputStream << triangles[tri_it].m_b << " ";
		outputStream << triangles[tri_it].m_c;
		outputStream << "\n";
	}
}

hkSimpleMeshShape* HK_CALL hkTklStreamer::readStorageMeshFromBtklArchive(hkIArchive &inputArchive)
{
	hkSimpleMeshShape* newShape = new hkSimpleMeshShape();
	int numVertices = inputArchive.read32();
	HK_ASSERT2(0x64cbe9f5,  numVertices > 2, "Less than three vertices, invalid tkl file" );
	newShape->m_vertices.setSize( numVertices );
	
	for ( int v_it = 0; v_it < numVertices; v_it++ )
	{
		newShape->m_vertices[v_it].set(inputArchive.readFloat32(),
			inputArchive.readFloat32(), inputArchive.readFloat32() );
	}
	
	int numTriangles = inputArchive.read32();
	HK_ASSERT2(0x6e7127e5,  numTriangles > 0, "Less than 1 triangle, invalid tkl file" );
	newShape->m_triangles.setSize( numTriangles );
	
	for ( int tr_it = 0; tr_it < numTriangles; tr_it++ )
	{
		newShape->m_triangles[tr_it].m_a = inputArchive.read32();
		newShape->m_triangles[tr_it].m_b = inputArchive.read32();
		newShape->m_triangles[tr_it].m_c = inputArchive.read32();
	}
	return newShape;
}

void HK_CALL hkTklStreamer::writeStorageMeshShapeToBtklArchive(hkSimpleMeshShape* shape, hkOArchive &outputArchive)
{
	const hkArray<hkVector4>& vertices = shape->m_vertices;
	const hkArray<hkSimpleMeshShape::Triangle>& triangles = shape->m_triangles;
	
	outputArchive.write32( vertices.getSize() );
	for ( int v_it = 0; v_it < vertices.getSize(); v_it++ )
	{
		outputArchive.writeFloat32( vertices[v_it](0) );
		outputArchive.writeFloat32( vertices[v_it](1) );
		outputArchive.writeFloat32( vertices[v_it](2) );
	}
	
	outputArchive.write32( triangles.getSize() );
	for ( int tr_it = 0; tr_it < triangles.getSize(); tr_it++ )
	{
		outputArchive.write32( triangles[tr_it].m_a );
		outputArchive.write32( triangles[tr_it].m_b );
		outputArchive.write32( triangles[tr_it].m_c );
	}
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
