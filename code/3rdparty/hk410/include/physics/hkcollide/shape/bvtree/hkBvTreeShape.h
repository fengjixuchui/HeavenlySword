/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_BV_TREE_SHAPE_H
#define HK_COLLIDE2_BV_TREE_SHAPE_H

#include <hkcollide/shape/hkShape.h>
#include <hkcollide/shape/hkShapeContainer.h>
#include <hkcollide/shape/collection/hkShapeCollection.h>
#include <hkmath/basetypes/hkSphere.h>

class hkShapeCollection;

extern const hkClass hkBvTreeShapeClass;

/// An hkBvTreeShape adds bounding volume tree information to an hkShapeCollection, such as an hkMeshShape.
/// This is an abstract base class. See hkMoppBvTreeShape for an implementation.
///
/// <b>What does the bounding volume tree do?</b><br>
///
/// A bounding volume tree is useful in situations where you need to check for collisions between a moving object
/// and a large static geometry, such as a landscape. <br> \n
/// The shapes that make up the landscape are hierarchically grouped in
/// a binary bounding volume tree.
/// At every node in the tree there exists a bounding polytope, which encapsulates all 
/// of its children. The top-level bounding volume contains the entire landscape, while
/// the nodes on the leaf levels encapsulate one geometric primitive, normally a
/// triangle. The fit of this bounding volume can be perfect (as in some AABB trees), or can 
/// have an extra margin/tolerance built in (e.g. MOPP):\n\n\n
/// <center><img src="pix/twoTriangles.gif"></center>\n
///
/// Instead of checking whether the moving object is colliding with each of the triangles in the landscape in turn,
/// which would be extremely time-consuming, the bounding box of the moving object
/// is checked against the bounding volume tree - first, whether it is intersecting with the top-level bounding volume, then with
/// any of its child bounding volumes, and so on until the check reaches the leaf nodes. A list of any potentially colliding triangles
/// is then passed to the narrow phase collision detection. You can think of
/// the bounding volume tree as a filter to the narrow phase collision
/// detection system.<br>
class hkBvTreeShape: public hkShape
{
	public:
		
		HK_DECLARE_REFLECTION();

			/// Creates an hkBvTreeShape with the specified hkShapeCollection.
		inline hkBvTreeShape( const hkShapeCollection* collection );

			/// Returns the hkShapeKey for all shapes in the hkShapeCollection that intersect with the sphereQuery.
		virtual void querySphere( const hkSphere &sphereQuery, hkArray< hkShapeKey >& hits ) const = 0;

			/// Returns the hkShapeKey for all shapes in the hkShapeCollection that intersect with the obb (defined by obbTransform and obbExtent).
		virtual void queryObb( const hkTransform& obbTransform, const hkVector4& obbExtent, hkReal tolerance, hkArray< hkShapeKey >& hits ) const = 0;

			/// Returns the hkShapeKey for all shapes in the hkShapeCollection that intersect with the AABB
		virtual void queryAabb( const hkAabb& aabb, hkArray<hkShapeKey>& hits ) const = 0;

			/// Gets the hkShapeCollection.
		inline const hkShapeCollection* getShapeCollection() const;

			/// Gets this hkShape's type. For hkBvTreeShapes, this is HK_SHAPE_BV_TREE.
		virtual hkShapeType getType() const;

		virtual void calcStatistics( hkStatisticsCollector* collector) const;

		virtual const hkShapeContainer* getContainer() const;
		
	protected:

		class hkSingleShapeContainer m_child;

	public:

		hkBvTreeShape( hkFinishLoadedObjectFlag flag ) : hkShape(flag), m_child(flag) {}
};


#include <hkcollide/shape/bvtree/hkBvTreeShape.inl>

#endif // HK_COLLIDE2_BV_TREE_SHAPE_H

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
