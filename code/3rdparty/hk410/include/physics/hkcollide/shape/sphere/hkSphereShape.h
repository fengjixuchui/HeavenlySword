/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_SPHERE_SHAPE_H
#define HK_COLLIDE2_SPHERE_SHAPE_H

#include <hkcollide/shape/convex/hkConvexShape.h>

extern const hkClass hkSphereShapeClass;

/// The hkSphereShape class is a utility class for storing information representing a basic
/// sphere shape. Note that the functions for getting and setting the radius are in the hkConvexShape base class.
/// Thus a sphere shape need not hold any data, it is simply an implicit point at the origin with a radius.
class hkSphereShape : public hkConvexShape
{
	public:

		HK_DECLARE_REFLECTION();

			/// Creates an arbitrary sphere with given radius.
		hkSphereShape(hkReal radius);


		//
		// hkConvexShape implementation
		//
		
			// hkConvexShape interface implentation.
		virtual void getSupportingVertex( const hkVector4& direction, hkCdVertex& supportingVertex ) const;

			// hkConvexShape interface implementation.
		virtual void convertVertexIdsToVertices( const hkVertexId* ids, int numIds, hkCdVertex* verticesOut) const;


		// hkConvexShape interface implentation.
		virtual void getFirstVertex(hkVector4& v) const;
			
			// hkConvexShape interface implentation.
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

			// hkShape interface implementation.
		void getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out ) const;

			/// Gets this hkShape's type. For hkSphereShapes, this is HK_SHAPE_SPHERE.
		virtual hkShapeType getType() const;

		virtual void calcStatistics( hkStatisticsCollector* collector) const;

			// hkShape interface implementation.
		virtual hkBool castRay(const hkShapeRayCastInput& input, hkShapeRayCastOutput& results) const;

	public:

		hkSphereShape( hkFinishLoadedObjectFlag flag ) : hkConvexShape(flag) {}

};

#endif // HK_COLLIDE2_SPHERE_SHAPE_H

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
