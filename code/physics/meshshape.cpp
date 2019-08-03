//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/meshshape.h
//!	
//!	MeshShape inherited from Havok hkMeshShape. It will deallocate memory used for vertex and index buffers.
//! hkMeshShape does not deallocate it. 
//!
//!	Author: Peter Feher
//! Created: 2006.07.18
//!
//---------------------------------------------------------------------------------------------------------
#include "Physics/config.h"

#include "meshshape.h"

namespace Physics
{	
	
	hsMeshShape::hsMeshShape(hkReal radius, int numBitsForSubpartIndex) :
	hkMeshShape(radius, numBitsForSubpartIndex),
	m_vertices(0),
	m_indexes(0),
	m_materialIndexes(0),
	m_materialBase(0)
#ifndef _RELEASE
	, m_debugRenderTriangleList(0)
#endif
	{};

	hsMeshShape::~hsMeshShape()
	{
		NT_DELETE_CHUNK( MC_PHYSICS, m_vertices);
		NT_DELETE_CHUNK( MC_PHYSICS, m_indexes);
		NT_DELETE_CHUNK( MC_PHYSICS, m_materialIndexes);
		NT_DELETE_CHUNK( MC_PHYSICS, m_materialBase);
#ifndef _RELEASE
		NT_DELETE_CHUNK( MC_PHYSICS, m_debugRenderTriangleList);
#endif
	};

	void hsMeshShape::addSubpart(const Subpart &part)
	{
		// hsMeshShape supports only one subpart
		m_vertices = (const float *)part.m_vertexBase;
		m_indexes = (const hkInt16 *)part.m_indexBase;
		m_materialIndexes = (const hkUint8 *)part.m_materialIndexBase;
		m_materialBase = (const hsMeshMaterial *)part.m_materialBase;

		base::addSubpart(part);
	}

#ifndef _RELEASE
	CPoint * hsMeshShape::GetDebugRenderTriangleList()
	{
		if (!m_debugRenderTriangleList)
		{
			const Subpart& subpart = getSubPart(0);
			m_debugRenderTriangleList = NT_NEW_CHUNK( MC_PHYSICS )  CPoint[subpart.m_numTriangles * 3];
			for(int i = 0; i < subpart.m_numTriangles; i++)
			{
				int indx = m_indexes[3 * i];
				m_debugRenderTriangleList[3 * i] = CPoint( m_vertices[3 * indx], m_vertices[3 * indx + 1], m_vertices[3 * indx + 2]);

				indx = m_indexes[3 * i + 2];
				m_debugRenderTriangleList[3 * i + 1] = CPoint( m_vertices[3 * indx], m_vertices[3 * indx + 1], m_vertices[3 * indx + 2]);

				indx = m_indexes[3 * i + 1];
				m_debugRenderTriangleList[3 * i + 2] = CPoint( m_vertices[3 * indx], m_vertices[3 * indx + 1], m_vertices[3 * indx + 2]);

			}			
		}

		return m_debugRenderTriangleList;
	}
#endif
}
