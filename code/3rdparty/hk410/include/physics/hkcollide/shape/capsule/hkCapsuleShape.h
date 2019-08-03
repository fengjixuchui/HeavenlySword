/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_LINE_SEGMENT_SHAPE_H
#define HK_COLLIDE2_LINE_SEGMENT_SHAPE_H

#include <hkcollide/shape/convex/hkConvexShape.h>

extern const hkClass hkCapsuleShapeClass;

	/// A capsule defined by two points and a radius.
	/// The points are stored internally as hkSpheres, each point being the centre of one of the
	/// end spheres of the capsule.
class hkCapsuleShape : public hkConvexShape
{
	public:

		HK_DECLARE_REFLECTION();

			/// For raycasting, the part of the shape hit.
		enum RayHitType
		{
			HIT_CAP0,
			HIT_CAP1,
			HIT_BODY,
		};

			/// Creates a new hkCapsuleShape using the specified points and radius
		hkCapsuleShape( const hkVector4& vertexA,const hkVector4& vertexB, hkReal radius );

			/// Gets the pointer to the first vertex. This casts the corresponding hkSphere (m_vertexA) to a hkVector4*.
			/// You can then index by 0 or 1, to get the first or second vertex respectively.
		inline const hkVector4* getVertices() const;

			/// Gets a vertex given an index "i". "i" can be 0 or 1. This casts the corresponding hkSphere to a hkVector4.
		HK_FORCE_INLINE const hkVector4& getVertex(int i) const;

			/// Sets a vertex given an index "i". "i" can be 0 or 1.
		HK_FORCE_INLINE void setVertex(int i, const hkVector4& position );


		//
		// hkConvexShape interface
		//
		
			// hkConvexShape interface implementation.
		void getSupportingVertex( const hkVector4& direction, hkCdVertex& supportingVertex ) const;

			// hkConvexShape interface implementation.
		virtual void convertVertexIdsToVertices( const hkVertexId* ids, int numIds, hkCdVertex* verticesOut) const;

			/// hkConvexShape interface implementation.
			/// Returns the first vertex of this shape. This is only used for initialization of collision detection data.
		virtual void getFirstVertex(hkVector4& v) const;
			
			// hkConvexShape interface implementation.
		virtual int getNumVertices() const;


		//
		// hkSphereRepShape implementation
		//

			// hkSphereRepShape implementation
		virtual void getCollisionSpheresInfo( hkCollisionSpheresInfo& infoOut ) const;

			// hkSphereRepShape implementation
		virtual const hkSphere* getCollisionSpheres( hkSphere* sphereBuffer ) const;


		//
		// hkShape implementation
		//

			///	Depending on where the ray hits, output.m_extrainfo is set to a RayHitType.
		virtual hkBool castRay( const hkShapeRayCastInput& input, hkShapeRayCastOutput& output ) const;

			//	hkShape interface implementation. 
 		void getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out  ) const;

			/// Gets this hkShape's type. For hkCapsuleShapes, this is HK_SHAPE_CAPSULE
		virtual hkShapeType getType() const;

		virtual void calcStatistics( hkStatisticsCollector* collector) const;

	public:

		static void HK_CALL closestPointLineSeg( const hkVector4& A, const hkVector4& B, const hkVector4& B2, hkVector4& pt );
		static void HK_CALL closestInfLineSegInfLineSeg( const hkVector4& A, const hkVector4& dA, const hkVector4& B, const hkVector4& dB, hkReal& distSquared, hkReal& t, hkReal &u, hkVector4& p, hkVector4& q );

	protected:

		// The line's first point. 
		hkVector4  m_vertexA;

		// The line's second point. 
		hkVector4  m_vertexB;

	public:

		hkCapsuleShape( hkFinishLoadedObjectFlag flag ) : hkConvexShape(flag) {}

};

#include <hkcollide/shape/capsule/hkCapsuleShape.inl>

#endif // HK_COLLIDE2_LINE_SEGMENT_SHAPE_H

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
