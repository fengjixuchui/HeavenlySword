/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_PLANE_SHAPE_H
#define HK_COLLIDE2_PLANE_SHAPE_H

#include <hkcollide/shape/heightfield/hkHeightFieldShape.h>

extern const hkClass hkPlaneShapeClass;

	/// A hkPlaneShape containing a normal and distance from the origin.
	/// This plane shape is also bounded by a given aabb.
	/// The plane shape does not collide with it's edges.
	/// The user has to make sure, that the aabb is set correctly, else
	/// collision will be filtered away. This includes that the aabb is slightly bigger
	/// than the shape (use the tolerance as the thickness in the positive direction and
	/// some 'maximum penetration depth' in the other.
class hkPlaneShape : public hkHeightFieldShape
{
	public:

		HK_DECLARE_REFLECTION();

			/// Create a hkPlaneShape from a normal and a distance (from the origin to the hkPlaneShape, in the opposite direction of the normal).
			/// Also you need to specify an Aabb, which is used to restrict the extents of the shape
		hkPlaneShape(const hkVector4& plane, const hkAabb& aabb);

			/// Create a planeshape using a given direction, a center point and the halfExtents 
		hkPlaneShape( const hkVector4& direction, const hkVector4& center, const hkVector4& halfExtents );

			///	hkShape Interface implementation
 		void getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out  ) const;

			/// Gets this hkShape's type (HK_SHAPE_PLANE)
		virtual hkShapeType getType() const;

		virtual void calcStatistics( hkStatisticsCollector* collector) const;

			/// hkShape interface implementation
		virtual hkBool castRay(const hkShapeRayCastInput& input, hkShapeRayCastOutput& results) const;

			/// collector driven raycast implementation using the data driven 
		virtual void castRayWithCollector( const hkShapeRayCastInput& input, const hkCdBody& cdBody, hkRayHitCollector& collector ) const;

			/// cast a sphere
		virtual void castSphere( const hkSphereCastInput& input, const hkCdBody& cdBody, hkRayHitCollector& collector ) const;

			/// hkHeightFieldShape interface implementation.
		virtual void collideSpheres( const CollideSpheresInput& input, SphereCollisionOutput* outputArray) const;

			/// get the plane
		inline const hkVector4& getPlane() const { return m_plane; }

	protected:
		hkVector4 m_plane;
		hkVector4 m_aabbCenter;
		hkVector4 m_aabbHalfExtents;

	public:

		hkPlaneShape( hkFinishLoadedObjectFlag flag ) : hkHeightFieldShape(flag) {}

};


#endif // HK_COLLIDE2_PLANE_SHAPE_H


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
