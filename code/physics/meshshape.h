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
#ifndef __MESHSHAPE_H__
#define __MESHSHAPE_H__

#pragma once

#include <hkcollide/shape/mesh/hkMeshShape.h>
#include "physicsmaterial.h"

namespace Physics
{
	class hsMeshShape : public hkMeshShape
	{
		typedef hkMeshShape base;
	public:
		hsMeshShape(hkReal radius = hkConvexShapeDefaultRadius, int numBitsForSubpartIndex = 12);
		~hsMeshShape();

		void addSubpart(const Subpart &part);
#ifndef _RELEASE
		CPoint * GetDebugRenderTriangleList(); 
#endif

	protected:
		const float* m_vertices;
		const hkInt16* m_indexes;
		const hkUint8* m_materialIndexes;
		const hsMeshMaterial* m_materialBase;
#ifndef _RELEASE
		CPoint*	m_debugRenderTriangleList;
#endif

	};
};

#endif //__MESHSHAPE_H__
