/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_CONVEX_VERTICES_SHAPE_H
#define HK_COLLIDE2_CONVEX_VERTICES_SHAPE_H

#include <hkcollide/shape/convex/hkConvexShape.h>
#include <hkmath/basetypes/hkStridedVertices.h>

extern const hkClass hkConvexVerticesShapeClass;

/// You can use this shape class to create a convex geometric object by specifying a set of vertices. 
/// Specify the vertices in the shape's local space. You must also provide the planes of the convex hull, which
/// can be computed using hkGeometryUtility::createConvexGeometry function (see the ConvexVerticesShapeApi for example).
class hkConvexVerticesShape : public hkConvexShape
{
	public:

		// 4 vectors stored transposed in the "columns" not the rows
		struct FourVectors
		{
			HK_DECLARE_REFLECTION();

			hkVector4 m_x;
			hkVector4 m_y;
			hkVector4 m_z;
		};

	public:

		HK_DECLARE_REFLECTION();

			/// Construct the shape from the given vertices and matching plane equations.
			/// These are plane equations of the convex hull and can be generated
			/// using the hkGeometryUtility::createConvexGeometry method. 
			/// This constructor makes an internal copy of the vertices.
			/// You should take care of not passing in unnecessary vertices, e.g. inner vertices or
			/// duplicated vertices. hkGeometryUtility::createConvexGeometry will also give
			/// you back a clean list of vertices to use. See our Havok demos
		hkConvexVerticesShape(hkStridedVertices vertsIn, const hkArray<hkVector4>& planeEquations, hkReal radius = hkConvexShapeDefaultRadius);

			/// Create from precomputed data.
			/// Note that numVertices is the actual number of vertices, not the
			/// number of FourVectors structures.
		hkConvexVerticesShape( FourVectors* rotatedVertices, int numVertices,
				hkVector4* planes, int numPlanes,
				const hkAabb& aabb, hkReal radius = hkConvexShapeDefaultRadius );

			/// The hkConvexVerticesShape stores the vertices in optimized form.
			/// This function will retrieve them into the vertices array.
			/// It copies the vertices so is able to be a const method.
		void getOriginalVertices( hkArray<hkVector4>& vertices ) const;

			/// Returns the plane equations passed into the constructor
		const hkArray<hkVector4>& getPlaneEquations() const;

		//
		// hkConvexShape implementation
		//

			/// hkConvexShape interface implementation. 
			/// \param vertexID  Used to speed up repeated call to this function. In order to work properly,
			///                  vertexID must be initialized to zero initially.
		virtual void getSupportingVertex( const hkVector4& direction, hkCdVertex& supportingVertex ) const;

			// hkConvexShape interface implementation.
		virtual void convertVertexIdsToVertices( const hkVertexId* ids, int numIds, hkCdVertex* verticesOut) const;

			// hkConvexShape interface implementation
		virtual void getFirstVertex(hkVector4& v) const;


		//
		// hkSphereRepShape implementation
		//

			/// hkSphereRepShape implementation, needed for colliding with hkHeightFieldShape
		virtual void getCollisionSpheresInfo( hkCollisionSpheresInfo& infoOut ) const;

			/// hkSphereRepShape implementation, needed for colliding with hkHeightFieldShape
		virtual const hkSphere* getCollisionSpheres( hkSphere* sphereBuffer ) const;


		//
		// hkShape implementation
		//
		
			// hkShape interface implementation.
		virtual void getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out  ) const;

			// hkShape interface implementation.
		virtual hkBool castRay(const hkShapeRayCastInput& input,hkShapeRayCastOutput& results) const;

			/// Gets this hkShape's type. For hkConvexVerticesShapes, this is HK_SHAPE_CONVEX_VERTICES.
		virtual hkShapeType getType() const;

		virtual void calcStatistics( hkStatisticsCollector* collector) const;

		void copyVertexData(const float* vertexIn, int byteStriding, int numVertices);

	protected:

		hkVector4	m_aabbHalfExtents;
		hkVector4	m_aabbCenter;

		// hkInplaceArray<FourVectors, 3> m_rotatedVertices;
		hkArray<struct FourVectors> m_rotatedVertices;
		hkInt32		m_numVertices;

		hkArray<hkVector4>	m_planeEquations;

	public:

		hkConvexVerticesShape( hkFinishLoadedObjectFlag flag ) : hkConvexShape(flag), m_rotatedVertices(flag), m_planeEquations(flag) {}
};


#endif // HK_COLLIDE2_CONVEX_VERTICES_SHAPE_H

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
