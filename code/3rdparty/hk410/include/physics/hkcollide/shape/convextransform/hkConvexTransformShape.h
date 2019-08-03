/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_CONVEX_TRANSFORM_SHAPE_H
#define HK_COLLIDE2_CONVEX_TRANSFORM_SHAPE_H

#include <hkcollide/shape/convex/hkConvexShape.h>
#include <hkcollide/shape/hkShapeContainer.h>

extern const hkClass hkConvexTransformShapeClass;

	/// An hkConvexTransformShape contains an hkShape and an additional transform for that shape. 
	///	This is useful, for instance, if you
	/// want to position child shapes correctly when constructing a compound shape.
	/// The advantage of using hkConvexTransformShape over hkTransformShape is that
	/// it does not require additional agents to be created, as the hkConvexTransformShape is
	/// a convex shape and directly works with GSK.
	/// However, if you use the hkConvexTransformShape wrapping an hkBoxShape, no hkBoxBoxAgent will be
	/// created, but the hkGskfAgent.
class hkConvexTransformShape : public hkConvexShape
{
	public:

		HK_DECLARE_REFLECTION();

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_CDINFO);

			/// Constructs a new convex transform shape.
		hkConvexTransformShape(const hkConvexShape* childShape, const hkTransform& transform);
					
			/// Sets the current transform.
			/// Don't do this once the shape is added to a world
			/// as the agents may have cached data dependent on it.
		void setTransform( const hkTransform& transform );


		//
		// hkConvexShape implementation 
		//
			// hkConvexShape interface implementation.
		void getSupportingVertex( const hkVector4& dir, hkCdVertex& supportingVertexOut ) const;

			// hkConvexShape interface implementation.
		virtual void convertVertexIdsToVertices( const hkVertexId* ids, int numIds, hkCdVertex* verticesOut) const;

			// hkConvexShape interface implentation.
		virtual void getFirstVertex(hkVector4& v) const;

		//
		// hkSphereRepShape implementation
		//

			// hkSphereRepShape implementation
		virtual void getCollisionSpheresInfo( hkCollisionSpheresInfo& infoOut ) const;

			// hkSphereRepShape implementation
		virtual const hkSphere* getCollisionSpheres( hkSphere* sphereBuffer ) const;


			/// Get the child shape.
		inline const hkConvexShape* getChildShape() const;

			/// Gets the transform from the child shape's space to this transform shape's local space.
		inline const hkTransform& getTransform() const;


		//
		// hkShape Implementation
		//

			//	hkShape interface implementation.
		void getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out ) const;

			//	hkShape interface implementation.
		virtual hkReal getMaximumProjection( const hkVector4& direction ) const;

			/// Gets this hkShape's type. For hkConvexTransformShapes, this is HK_SHAPE_CONVEX_TRANSFORM.
		virtual hkShapeType getType() const;
	
			//	hkShape interface implementation.
		virtual hkBool castRay( const hkShapeRayCastInput& input, hkShapeRayCastOutput& results ) const;

			//	hkShape interface implementation.
		void castRayWithCollector( const hkShapeRayCastInput& input, const hkCdBody& parentCdBody, hkRayHitCollector& collector ) const;

		virtual void calcStatistics( hkStatisticsCollector* collector) const;

		virtual const hkShapeContainer* getContainer() const;
		
	protected:

		class hkSingleShapeContainer m_childShape;

		hkTransform m_transform;

	public:
		
		hkConvexTransformShape( class hkFinishLoadedObjectFlag flag ) : hkConvexShape(flag), m_childShape(flag) {}
};

#include <hkcollide/shape/convextransform/hkConvexTransformShape.inl>

#endif // HK_COLLIDE2_CONVEX_TRANSFORM_SHAPE_H

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
