/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_CONVEX_SHAPE_H
#define HK_COLLIDE2_CONVEX_SHAPE_H

#include <hkcollide/shape/sphererep/hkSphereRepShape.h>

class hkCdVertex;
extern const hkClass hkConvexShapeClass;

extern hkReal hkConvexShapeDefaultRadius;
typedef hkUint16 hkVertexId;

#define HK_W_NOT_SET_TO_VERTEX_ID "The function getSupportingVertex did not set the .w component of the supporting vertex to the vertex id using set3asUin16"
	/// An interface shape class that allows its implementations to work with GJK. It also holds an extra radius to allow for shells around objects.
class hkConvexShape : public hkSphereRepShape
{
	public:		

		HK_DECLARE_REFLECTION();

			/// The GJK interface. It can use the vertexId, which is initialized with 0.
			/// but it has to set the vertexId and it has to set the .w component of supportingVertexOut to the
			/// vertex id using hkVector4::setInt24W
		virtual void getSupportingVertex( const hkVector4& direction, hkCdVertex& supportingVertexOut ) const = 0;

			/// convert an array of input vertex id as returned by getSupportingVertex into an array of vertices
			/// The .w component of the vertices out has to be set to the vertex id using hkVector4::setInt24W
		virtual void convertVertexIdsToVertices( const hkVertexId* ids, int numIds, hkCdVertex* vertixArrayOut) const = 0;
		
			/// Returns the first vertex of this shape. This is only used for initialization of collision detection data.
			/// it has to set the .w component of v to the vertex id using hkVector4::setInt24W
		virtual void getFirstVertex(hkVector4& v) const = 0;

			/// Gets the extra radius.
			/// The radius is used to create a thin "shell" around the object that is used as the shape's surface for collision detection purposes. 
			/// This can improve performance, as calculations for shapes that are actually interpenetrating are much more time-consuming than for interpenetrating shells.
		inline hkReal getRadius() const;

			/// Sets the extra radius
		inline void setRadius(hkReal radius);

			/// Return the number of distinct points that can be returned by getSupportingVertex() for optimization purposes.
			/// This defaults to -1. You do not need to implement this unless the shape has only 1, 2 or 3 vertices.
		inline virtual int getNumVertices() const;


		//
		// hkShape implementation
		//

			/// hkShape interface implementation. Note that this secondary type is inherited by the various
			/// hkConvexShape subclasses, such as hkBoxShape and hkSphereShape.
		virtual hkShapeType getType() const;

			/// This is an implementation of the hkShape::getMaximumProjection() function using getSupportingVertex.
		virtual hkReal getMaximumProjection( const hkVector4& direction ) const;

			/// This implementation of the callback driven castRay uses the data driven castRay function
			/// Implementation notes: For all convex shapes excepts hkSphere and hkCapsule the radius of the shape will be ignored.
		virtual void castRayWithCollector( const hkShapeRayCastInput& input, const hkCdBody& cdBody, hkRayHitCollector& collector ) const;


	protected:

			// The protected constructor, which sets the secondary type to be convex.
		inline hkConvexShape( hkReal radius );

		hkReal m_radius;

	public:

		hkConvexShape( hkFinishLoadedObjectFlag flag ) : hkSphereRepShape(flag) {}

};

#include <hkcollide/shape/convex/hkConvexShape.inl>

#endif // HK_COLLIDE2_CONVEX_SHAPE_H


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
