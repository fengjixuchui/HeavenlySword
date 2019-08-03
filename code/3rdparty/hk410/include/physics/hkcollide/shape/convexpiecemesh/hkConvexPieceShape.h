/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_CONVEX_PIECE_SHAPE_H
#define HK_COLLIDE2_CONVEX_PIECE_SHAPE_H

#include <hkcollide/shape/convex/hkConvexShape.h>
#include <hkcollide/shape/mesh/hkMeshShape.h>

extern hkReal hkConvexShapeDefaultRadius;

/// Returned as a child shape by the hkSimulationMeshShape
class hkConvexPieceShape : public hkConvexShape, protected hkShapeContainer
{
	public:

		hkConvexPieceShape( hkReal radius = hkConvexShapeDefaultRadius );

		//
		// hkConvexShape implementation
		//

			/// hkConvexShape interface implementation. 
			///\param vertexID is used to speed up repeated call to this function. In order to work properly, vertexID myst be initialized
			/// to zero initially.
		virtual void getSupportingVertex( const hkVector4& direction, hkCdVertex& supportingVertex ) const;

			/// hkConvexShape interface implementation.
		virtual void convertVertexIdsToVertices( const hkVertexId* ids, int numIds, hkCdVertex* verticesOut) const;

			/// hkConvexShape interface implementation
		virtual void getFirstVertex(hkVector4& v) const;


		//
		// hkShape implementation
		//
		
			/// Used to pre-calculate the aabb for this shape.
		void calcAabb();

			// hkShape interface implementation.
		virtual void getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out  ) const;

			/// hkShape interface implementation.
		virtual hkBool castRay(const hkShapeRayCastInput& input,hkShapeRayCastOutput& results) const;

			// hkShape interface implementation.
		virtual void castRayWithCollector( const hkShapeRayCastInput& input, const hkCdBody& cdBody, hkRayHitCollector& collector ) const;

			/// Gets this hkShape's primary type. For hkConvexPieceShapes, this is HK_SHAPE_CONVEX_PIECE.
		virtual hkShapeType getType() const;


		//
		// hkSphereRepShape implementation
		//

			/// hkSphereRepShape implementation
		virtual void getCollisionSpheresInfo( hkCollisionSpheresInfo& infoOut ) const;

			/// hkSphereRepShape implementation
		virtual const hkSphere* getCollisionSpheres( hkSphere* sphereBuffer ) const;

		//
		// Shape Container
		//

			/// 
		virtual const hkShapeContainer* getContainer() const;

		virtual hkShapeKey getFirstKey() const;

		virtual hkShapeKey getNextKey( hkShapeKey oldKey ) const;

		virtual const hkShape* getChildShape( hkShapeKey key, ShapeBuffer& buffer ) const;

	public:

			/// Array of vertices on the perimeter of this convex piece
		hkVector4* m_vertices;

			/// The number of vertices in this convex piece.
		int m_numVertices;

			/// The underlying display mesh.
		const hkShapeCollection* m_displayMesh;

			/// ShapeKeys for the triangles in the underlying display mesh that form this convex piece.
		const hkShapeKey* m_displayShapeKeys;

			/// The number of triangles in this convex piece.
		int m_numDisplayShapeKeys;
};


#endif // HK_COLLIDE2_CONVEX_PIECE_SHAPE_H


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
