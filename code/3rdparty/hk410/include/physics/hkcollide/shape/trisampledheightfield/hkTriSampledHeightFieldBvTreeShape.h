/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_TRI_SAMPLED_HEIGHTFIELD_BVTREE_SHAPE
#define HK_TRI_SAMPLED_HEIGHTFIELD_BVTREE_SHAPE

#include <hkcollide/shape/bvtree/hkBvTreeShape.h>

extern const hkClass hkTriSampledHeightFieldBvTreeShapeClass;

class hkTriSampledHeightFieldCollection;

/// This shape must be used in conjunction with the hkTriSampledHeightFieldCollection. Please see the documentation for
/// the hkTriSampledHeightFieldCollection for details on what the classes do.
class hkTriSampledHeightFieldBvTreeShape : public hkBvTreeShape
{
	public:

		HK_DECLARE_REFLECTION();

			/// The constructor takes a pointer to hkTriSampledHeightFieldCollection.
			/// This class is designed to work together with the hkTriSampledHeightFieldCollection.
			/// The value "doAabbRejection" can be used to turn off the aabb rejection test.
			/// The queryAabb() method (which will be used by agents/user queries using 
			/// the hkBvTreeShape interface) uses its projection onto the heightfield grid
			/// space to determine what triangles to return.
			/// To ensure that the resulting triangles are at least close to the
			/// AABB in the direction of projection (that the AABB is not hovering completely
			/// above or below all points in that area of heightfield), this test checks that it
			/// at least overlaps the single AABB bounding all those triangles.
			/// By default it is true. Setting it to false disables the test,
			/// which may give a small speedup for flat heightfields.
		hkTriSampledHeightFieldBvTreeShape( const hkTriSampledHeightFieldCollection* c, hkBool doAabbRejection = true );
		
		//
		// hkBvTreeShape interface
		//

			/// Returns the hkShapeKey for all shapes in the hkShapeCollection that intersect with the sphereQuery.
			/// This implementation simply bounds the sphere by an AABB and uses the queryAabb() method. Thus it is not as
			/// accurate as it could be if using a more sophisticated algorithm, and can return triangles completely 
			/// outside the sphere.
		virtual void querySphere( const hkSphere &sphereQuery, hkArray< hkShapeKey >& hits ) const;

			/// Returns the hkShapeKey for all shapes in the hkShapeCollection that intersect with the obb (defined by obbTransform and obbExtent).
			/// This implementation simply bounds the OBB by an AABB and uses the queryAabb() method. Thus it is not as
			/// accurate as it could be if using a more sophisticated algorithm, and can return triangles completely outside the OBB.
		virtual void queryObb( const hkTransform& obbTransform, const hkVector4& obbExtent, hkReal tolerance, hkArray< hkShapeKey >& hits ) const;

			/// Returns the hkShapeKey for all shapes in the hkShapeCollection that intersect with the AABB.
		virtual void queryAabb( const hkAabb& aabb, hkArray<hkShapeKey>& hits ) const;

		//
		// hkShape interface
		//

		virtual void getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out ) const;

		virtual hkBool castRay( const hkShapeRayCastInput& input, hkShapeRayCastOutput& output ) const;

		virtual void castRayWithCollector( const hkShapeRayCastInput& input, const hkCdBody& cdBody, hkRayHitCollector& collector ) const;

	public:

		hkBool m_wantAabbRejectionTest;

	public:

		hkTriSampledHeightFieldBvTreeShape( hkFinishLoadedObjectFlag flag ) : hkBvTreeShape(flag) {}

	private:

		inline const hkTriSampledHeightFieldCollection* getChild() const;
};


#endif // HK_TRI_SAMPLED_HEIGHTFIELD_BVTREE_SHAPE

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
