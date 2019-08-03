/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_TRIANGLE_SHAPE_H
#define HK_COLLIDE2_TRIANGLE_SHAPE_H

#include <hkcollide/shape/convex/hkConvexShape.h>

extern const hkClass hkTriangleShapeClass;


/// A triangle shape with its details stored as an hkGeometry::Triangle.
/// This shape is typically created at runtime, for example from the hkMeshShape. You should use the hkMeshShape, or
/// a variant on it to store a permanent collection of triangles.
class hkTriangleShape : public hkConvexShape
{
	public:

		HK_DECLARE_REFLECTION();

			/// Default constructor - nothing gets initialized.
		HK_FORCE_INLINE hkTriangleShape(hkReal radius = hkConvexShapeDefaultRadius);
		
			/// Constructor that sets the points of the triangle.
		HK_FORCE_INLINE hkTriangleShape(const hkVector4& v0, const hkVector4& v1, const hkVector4& v2, hkReal radius = hkConvexShapeDefaultRadius );

			/// Get a pointer to the vertices of the triangle.
			/// Returns the hkGeometry::Triangle.
		HK_FORCE_INLINE const hkVector4* getVertices() const;

			/// Get a non const reference to a vertex.
			/// The parameter "i" must be 0, 1 or 2
		HK_FORCE_INLINE hkVector4& getVertex(int i);

			/// Get a const reference to a vertex.
			/// The parameter "i" must be 0, 1 or 2
		HK_FORCE_INLINE const hkVector4& getVertex(int i) const;

			/// Set a vertex
			/// The parameter "i" must be 0, 1 or 2
		HK_FORCE_INLINE void setVertex(int i, const hkVector4& vertex);


		//
		// hkConvexShape implementation
		//

			//	hkConvexShape interface implementation.
		virtual void getSupportingVertex( const hkVector4& direction, hkCdVertex& supportingVertex ) const;

			// hkConvexShape interface implementation.
		virtual void convertVertexIdsToVertices( const hkVertexId* ids, int numIds, hkCdVertex* verticesOut) const;

			//	hkConvexShape interface implementation.
		virtual void getFirstVertex(hkVector4& v) const;

			//	hkConvexShape interface implementation.
		virtual int getNumVertices() const;


		//
		// hkSphereRepShape implementation
		//

			/// hkSphereRepShape implementation
		virtual void getCollisionSpheresInfo( hkCollisionSpheresInfo& infoOut ) const;

			/// hkSphereRepShape implementation
		virtual const hkSphere* getCollisionSpheres( hkSphere* sphereBuffer ) const;


		//
		// hkShape implementation
		//

			// hkShape interface implementation
 		void getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out  ) const;

			// hkShape interface implementation		
		virtual hkBool castRay(const hkShapeRayCastInput& input,hkShapeRayCastOutput& results) const;

			/// Gets this hkShape's type. For hkTriangleShapes, this is HK_SHAPE_TRIANGLE.
		virtual hkShapeType getType() const;

		virtual void calcStatistics( hkStatisticsCollector* collector) const;

	protected:

		hkVector4 m_vertexA;
		hkVector4 m_vertexB;
		hkVector4 m_vertexC;

	public:

		hkTriangleShape( hkFinishLoadedObjectFlag flag ) : hkConvexShape( flag) {}

};

#include <hkcollide/shape/triangle/hkTriangleShape.inl>

#endif // HK_COLLIDE2_TRIANGLE_SHAPE_H


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
