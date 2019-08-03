/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_BOX_SHAPE_H
#define HK_COLLIDE2_BOX_SHAPE_H

#include <hkcollide/shape/convex/hkConvexShape.h>

extern const hkClass hkBoxShapeClass;

/// A simple box shape centered around the origin.
class hkBoxShape : public hkConvexShape
{
	public:	

		HK_DECLARE_REFLECTION();
		
			/// Creates a box with the given half extents ( An (X by Y by Z) box has the half-extents (X/2, Y/2, Z/2) ).
		hkBoxShape( const hkVector4& halfExtents, hkReal radius = hkConvexShapeDefaultRadius );

			/// Gets the half extents ( An (X by Y by Z) box has the half-extent (X/2, Y/2, Z/2) ).
		inline const hkVector4& getHalfExtents() const;

			/// Sets the half extents. Note that changing the half extents will not wake up sleeping objects.
		void setHalfExtents(const hkVector4& halfExtents);

		
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

			
		//
		// hkShape implementation
		//


			// hkShape interface implementation.
		void getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out  ) const;

			//	hkShape interface implementation.
		virtual hkBool castRay(const hkShapeRayCastInput& input, hkShapeRayCastOutput& results) const;
		
			/// Gets this hkShape's type. For hkBoxShapes, this is HK_SHAPE_BOX.
		virtual hkShapeType getType() const ;
			
		virtual void calcStatistics( hkStatisticsCollector* collector) const;


	protected:

		hkVector4 m_halfExtents;

	public:

		hkBoxShape( hkFinishLoadedObjectFlag flag ) : hkConvexShape(flag) {}

};


#include <hkcollide/shape/box/hkBoxShape.inl>

#endif // HK_COLLIDE2_BOX_SHAPE_H

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
