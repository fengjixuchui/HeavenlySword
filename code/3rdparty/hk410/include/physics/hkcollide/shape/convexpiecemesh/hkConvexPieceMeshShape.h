/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_CONVEXPIECE_MESH_SHAPE_H
#define HK_COLLIDE2_CONVEXPIECE_MESH_SHAPE_H

#include <hkinternal/collide/convexpiecemesh/hkConvexPieceStreamData.h>

#include <hkbase/htl/hkObjectArray.h>
#include <hkcollide/shape/collection/hkShapeCollection.h>
#include <hkbase/class/hkTypeInfo.h>

extern hkReal hkConvexShapeDefaultRadius;
extern const hkClass hkConvexPieceMeshShapeClass;

/// Using a hkConvexPieceMeshShape instead of a plain hkMeshShape for simulation level representations 
/// will result in a definite performance improvement - the degree of which will depend on the 
/// structure of the level and the type of interactions that occur with the level.
/// The best performance improvement will occur in levels that are highly 
/// tessellated and collisions between simple rigid bodies and the floor are the only collisions that
/// occur.
class hkConvexPieceMeshShape: public hkShapeCollection
{
	public:

		HK_DECLARE_REFLECTION();

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_CDINFO);

			/// Constructs a new hkConvexPieceMeshShape.
			///
			/// The inputMesh should be any hkShapeCollection that returns hkTriangleShape children.
			/// 
			/// The convexPieceStream must be created by the hkConvexPieceMeshBuilder::convexifyLandscape method.
		hkConvexPieceMeshShape( const hkShapeCollection* inputMesh, const hkConvexPieceStreamData* convexPieceStream, hkReal radius = hkConvexShapeDefaultRadius );

		~hkConvexPieceMeshShape();

		//
		// hkShape Collection interface 
		//

			/// hkShapeCollection interface implementation.
			/// Gets the first child shape key.
		virtual hkShapeKey getFirstKey() const;

			/// This function implements hkShapeCollection::getNextKey
			/// Gets the next child shape key.
		virtual hkShapeKey getNextKey( hkShapeKey oldKey ) const;


			///	hkShapeCollection interface implementation.
		const hkShape* getChildShape( hkShapeKey key, ShapeBuffer& buffer ) const;

			/// Tests if the vertex given by vertexId is set in the vertex bitstream
			/// of stream.
		const hkBool vertexIsSet( const hkUint32* stream, int vertexId ) const;

			/// hkShapeCollection interface implementation 
		virtual hkUint32 getCollisionFilterInfo( hkShapeKey key ) const;

		//
		// hkShape interface
		//

			/// Used to pre-calculate the aabb for this shape.
		void calcAabb();

			///	hkShape interface implementation.
			/// A precise but not very fast implementation of getting an AABB.
 		void getAabb( const hkTransform& localToWorld, hkReal tolerance, class hkAabb& out  ) const;

			/// The type of this class is a HK_SHAPE_COLLECTION
		virtual hkShapeType getType() const;

			/// Calculates memory statistics
		virtual void calcStatistics( hkStatisticsCollector* collector) const;

		//
		// Statistics retrieval interface
		//

			/// Contains statistical information about the structure of the convex piece mesh.
		struct Stats
		{
				/// The total number of triangles in the mesh.
			int m_numTriangles;

				/// The total number of convex pieces in the mesh.
			int m_numConvexPieces;

				/// The average number of triangles per convex piece.
				/// This should be used as the metric to determine how
				/// effective the builder algorithm is - the higher this value
				/// is the better.
			hkReal m_avgNumTriangles;

				/// The maximum number of triangles in any single convex piece.
			int m_maxTrianglesPerConvexPiece;

			Stats() : m_numTriangles(0), m_numConvexPieces(0), m_avgNumTriangles(0), m_maxTrianglesPerConvexPiece(0) {}
		};

			/// Fills statsOut to contains info about this mesh shape.
		void getStats( Stats& statsOut );

	public:
		
			/// The mesh, divided into convex pieces which are then stored in a 
			/// bit stream.
		const hkConvexPieceStreamData* m_convexPieceStream;

			/// The underlying meshShape
		const hkShapeCollection* m_displayMesh;

		hkConvexPieceMeshShape( hkFinishLoadedObjectFlag flag ) {}

	protected:
			/// The radius used in the construction of the convex pieces.
			/// This can only be set on contruction.
		hkReal m_radius;

};


#endif // HK_COLLIDE2_CONVEXPIECE_MESH_SHAPE_H

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
