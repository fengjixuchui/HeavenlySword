/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_CONVEX_LIST_SHAPE_H
#define HK_COLLIDE2_CONVEX_LIST_SHAPE_H

#include <hkcollide/shape/list/hkListShape.h>
#include <hkcollide/shape/convex/hkConvexShape.h>

extern const hkClass hkConvexListShapeClass;

class hkListShapeCinfo;

/// A simple static list of hkShapes. You can use this shape class to create compound bodies.
/// A list shape can hold a mix of different shape types e.g. an ice cream cone could be made 
/// from a list shape containing a sphere for the ice cream and a convex vertices shape 
/// for the wafer cone.
/// The convex list shape is very good for smaller objects, which are made of a few convex pieces.
/// You will get a very good CPU for this convex list, as long as the hkConvexListShape is 
/// only colliding with its convex hull. A typical example would be a car made of several parts.
/// If your objects looks more like a space ship, you should consider using hkListShape wrapped in a
/// hkMoppShape.
/// Note: This implementation has some limitations:
///     - the number of child objects is restricted to 255
///     - the number of vertices of each child object is restricted to 255
///     - The radius of all child objects must be equal.  For this reason you cannot combine 
///		hkSphereShapes, hkCapsuleShapes and hkConvexVerticesShapes in hkConvexListShapes.
/// Please see the user guide for more details on the hkConvexListShape
class hkConvexListShape : public hkConvexShape, public hkShapeContainer
{
	public:

		HK_DECLARE_REFLECTION();

			/// Constructs a list shape with an array of pointers to shapes.
		hkConvexListShape( const hkConvexShape*const* shapeArray, int numShapes );

			/// The destructor removes references to child shapes.
		~hkConvexListShape();
	
			/// Set whether you want to use a cached version of the aabb for getAabb().
			/// The default is true.
			/// If this is set to true a cached aabb of the children's aabbs will be used,
			/// otherwise getAabb will query all children and combine their aabbs.
			/// If true, the aabb returned is bigger, but is much faster to evaluate.
		void setUseCachedAabb( bool useCachedAabb );

			/// Get whether you want to use a cached version of the aabb for getAabb().
			/// The default is true.
			/// If this is set to true a cached aabb of the children's aabbs will be used,
			/// otherwise getAabb will query all children and combine their aabbs.
			/// If true, the aabb returned is bigger, but is much faster to evaluate.
		bool getUseCachedAabb();


			/// Returns the ith child shape.
		inline const hkShape* getChildShape(int i) const { return m_childShapes[i]; }

		//
		// hkConvexShape interface
		//

			// Implemented method of hkConvexShape
		virtual void getSupportingVertex( const hkVector4& dir, hkCdVertex& supportingVertexOut ) const;

			// Implemented method of hkConvexShape
		virtual void convertVertexIdsToVertices( const hkVertexId* ids, int numIds, hkCdVertex* verticesOut) const;

			// Implemented method of hkConvexShape
		virtual void getFirstVertex(hkVector4& v) const;

		//
		// hkSphereRepShape interface
		//

			// Implemented method of hkSphereRepShape
		virtual void getCollisionSpheresInfo( hkCollisionSpheresInfo& infoOut ) const;

			// Implemented method of hkSphereRepShape
		virtual const hkSphere* getCollisionSpheres( hkSphere* sphereBuffer ) const;



		//
		// hkShapeContainer interface
		//

			// hkShapeContainer interface implementation.
		virtual int getNumChildShapes() const;

			/// Get the first child shape key.
		virtual hkShapeKey getFirstKey() const;

			/// Get the next child shape key.
		virtual hkShapeKey getNextKey( hkShapeKey oldKey ) const;

			/// hkShapeCollection interface implementation. Always returns 0, and warns, because you cannot filter hkConvexListShapes.
		virtual hkUint32 getCollisionFilterInfo( hkShapeKey key ) const;

			/// Note that a hkListShape does not use the char* buffer for its returned shape.
		virtual const hkShape* getChildShape(hkShapeKey key, ShapeBuffer& buffer ) const;
			
		
		//
		// hkShape interface
		//


			// Implemented method of hkShape
		virtual void castRayWithCollector( const hkShapeRayCastInput& input, const hkCdBody& cdBody, hkRayHitCollector& collector ) const;

			// Implemented method of hkShape
		virtual hkBool castRay( const hkShapeRayCastInput& input, hkShapeRayCastOutput& results ) const;

			// Implemented method of hkShape
		virtual void getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out ) const;

			/// Gets this hkShape's type. For hkListShapes, this is HK_SHAPE_CONVEX_LIST.
			/// Implemented method of hkShape
		virtual hkShapeType getType() const;

			// Implemented method of hkShape
		virtual const hkShapeContainer* getContainer() const;

			// Implemented method of hkReferencedObject
		virtual void calcStatistics( hkStatisticsCollector* collector) const;



	protected:

		void setShapesAndRadius( const hkConvexShape*const* shapeArray, int numShapes );

	public:

		hkConvexListShape( class hkFinishLoadedObjectFlag flag ) : hkConvexShape(flag), m_childShapes(flag) {}

	public:


			/// A distance which is used for the getClosestPoint() call. If the distance between
			/// your query object and the convex hull is bigger than
			/// the value of this member, the function only returns this distance.
			/// Otherwise it recursively checks its children
		hkReal	m_minDistanceToUseConvexHullForGetClosestPoints;

	protected:

		hkVector4	m_aabbHalfExtents;
		hkVector4	m_aabbCenter;
		bool m_useCachedAabb; //+default(false)

	public:

		hkArray<const hkConvexShape*> m_childShapes;
		

};


#endif // HK_COLLIDE2_CONVEX_LIST_SHAPE_H

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
