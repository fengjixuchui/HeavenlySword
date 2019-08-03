/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_MULTI_SPHERE_SHAPE_H
#define HK_COLLIDE2_MULTI_SPHERE_SHAPE_H

#include <hkcollide/shape/collection/hkShapeCollection.h>
#include <hkcollide/shape/sphererep/hkSphereRepShape.h>

extern const hkClass hkMultiSphereShapeClass;

/// A compound shape made up of a number of spheres. This is useful as a fast approximation for complex surfaces,
/// as collision detection for spheres is very fast.
/// However, if two hkMultiSphereShape shapes collide, every sphere needs to be checked against every other sphere.
/// E.g. 10 spheres colliding with 10 spheres will result in 100 collision checks. Therefor higher order shapes like
/// hkCapsuleShape of hkConvexVerticesShape should be preferred.
class hkMultiSphereShape : public hkSphereRepShape
{
	public:
		HK_DECLARE_REFLECTION();

            /// 
		enum
		{ 
				/// The maximum number of spheres allowed in this hkMultiSphereShape.
			MAX_SPHERES = 8 
		};

			/// The w component of each vector3 is the sphere radius.
		hkMultiSphereShape(const hkVector4* spheres, int numSpheres);

			/// Get the shape's hkVector4 array that defines its spheres.
			/// Note: the .w component of the hkVector4 contains the radius of the sphere.
		inline const hkVector4* getSpheres() const;

			/// Get the number of spheres in the shape
		inline int getNumSpheres() const;


		//
		// hkSphereRepShape implementation
		//

			/// hkSphereRepShape implementation
		virtual void getCollisionSpheresInfo( hkCollisionSpheresInfo& infoOut ) const;

			/// hkSphereRepShape implementation
		virtual const hkSphere* getCollisionSpheres( hkSphere* sphereBuffer ) const;


		//
		// hkShape implementation
		//

			/// Gets this hkShape's type. For hkMultiSphereShapes, this is HK_SHAPE_MULTI_SPHERE.
		virtual hkShapeType getType() const;

		virtual void calcStatistics( hkStatisticsCollector* collector) const;

			// hkShape implementation
 		virtual void getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out ) const;

			/// output.m_extrainfo is set to the index of the sphere which is hit.
		virtual hkBool castRay( const hkShapeRayCastInput& input, hkShapeRayCastOutput& output ) const;
	
			// hkShape implementation
		virtual void castRayWithCollector( const hkShapeRayCastInput& input, const hkCdBody& cdBody, hkRayHitCollector& collector ) const;

	protected:

		int		m_numSpheres;
		hkVector4 m_spheres[8/*hkMultiSphereShape::MAX_SPHERES*/];

	public:

		hkMultiSphereShape( hkFinishLoadedObjectFlag flag ) : hkSphereRepShape(flag) {}

};

#include <hkcollide/shape/multisphere/hkMultiSphereShape.inl>

#endif // HK_COLLIDE2_MULTI_SPHERE_SHAPE_H

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
