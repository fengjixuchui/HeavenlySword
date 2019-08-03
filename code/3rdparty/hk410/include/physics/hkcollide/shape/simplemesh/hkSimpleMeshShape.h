/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_SIMPLE_MESH_SHAPE_H
#define HK_SIMPLE_MESH_SHAPE_H

#include <hkcollide/shape/mesh/hkMeshShape.h>

extern const hkClass hkSimpleMeshShapeClass;

/// This shape is a very simple container for triangle soups and can't handle triangle strips.
/// It does not allow sharing of triangle data with the renderer through referencing.
/// Use hkMeshShape or your own implementation of a hkShapeCollection to share triangle data with the renderer.
class hkSimpleMeshShape : public hkShapeCollection
{
	public:

		HK_DECLARE_REFLECTION();

			/// Default constructor.
			/// The data for this shape is public, so simply fill in the
			/// member data after construction.
		hkSimpleMeshShape( hkReal radius = hkConvexShapeDefaultRadius );


		//
		// hkShapeCollection interface
		//

			/// Get the first child shape key.
		virtual hkShapeKey getFirstKey() const;

			/// Get the next child shape key.
		virtual hkShapeKey getNextKey( hkShapeKey oldKey ) const;

			// hkShapeCollection interface implementation.
		const hkShape* getChildShape( hkShapeKey key, ShapeBuffer& buffer ) const;


			/// Gets the extra radius for every triangle.
		inline hkReal getRadius() const;

			/// Sets the extra radius for every triangle.
		inline void setRadius(hkReal r );


		//
		// hkShape interface
		//


			// hkShape interface implementation.
 		virtual void getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out ) const;

			/// The type of this class is a HK_TRIANGLE_COLLECTION. This means that all shapes it returns
			/// are hkTriangleShapes.
		virtual hkShapeType getType() const;

		virtual void calcStatistics( hkStatisticsCollector* collector) const;
	public:

		struct Triangle
		{
			HK_DECLARE_REFLECTION();

			int m_a;
			int m_b;
			int m_c;
		};

			/// Array of vertices that the triangles can index into.
		hkArray<hkVector4> m_vertices;

			/// Array of triangles.  The triangles are triples of ints that are indices into the m_vertices array.
		hkArray<struct Triangle> m_triangles;

			/// Material indices. If you are not using material information, leave this array as 0 size.
		hkArray<hkUint8> m_materialIndices;

			/// The radius of the storage mesh shape. It is initialized to .05
		hkReal m_radius;



	public:

		hkSimpleMeshShape( hkFinishLoadedObjectFlag flag ) : hkShapeCollection(flag), m_vertices(flag), m_triangles(flag), m_materialIndices(flag) {}

};


#include <hkcollide/shape/simplemesh/hkSimpleMeshShape.inl>

#endif //HK_SIMPLE_MESH_SHAPE_H

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
