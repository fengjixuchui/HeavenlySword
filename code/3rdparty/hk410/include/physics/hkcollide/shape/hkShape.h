/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_SHAPE_H
#define HK_COLLIDE2_SHAPE_H

#include <hkmath/hkMath.h>
#include <hkmath/basetypes/hkContactPoint.h>
#include <hkcollide/shape/hkShapeType.h>

class hkAabb;
struct hkShapeRayCastInput;
struct hkShapeRayCastOutput;
class hkRayHitCollector;
class hkCdBody;
class hkShapeContainer;

typedef hkUint32 hkShapeKey;
#define HK_INVALID_SHAPE_KEY 0xffffffff

extern const hkClass hkShapeClass;

/// The base class for narrow phase collision detection objects.
/// All narrow phase collision detection is performed between pairs of hkShape objects by creating appropriate hkCollisionAgent objects.
/// An hkShape can be a simple shape such as a box or sphere, a shape with additional transform information,
/// or a compound shape made up of simpler hkShapes. hkShape instances can be shared within or even between 
/// rigid bodies. See the hkShape subclasses for more details.
class hkShape : public hkReferencedObject
{
	public:

		HK_DECLARE_REFLECTION();
		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_CDINFO);

			/// Sets the user data to 0
		HK_FORCE_INLINE hkShape();

			/// Gets the hkShape type. This is used by the collision dispatcher to dispatch between pairs of shapes
		virtual hkShapeType getType() const = 0;

			/// Gets the AABB for the hkShape given a local to world transform and an extra tolerance.
		virtual void getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out ) const = 0;

			/// Support for creating bounding volume hierarchies of shapes.
			/// This function returns the maximal extent of a shape along a given direction. 
			/// It is not the same as hkConvexShape::getSupportingVertex, because the extent does not have to be exact, it just has to at least
			/// contain the shape. It is for the purposes of creating bounding volumes around the shape ( mid-phase ) rather than exact collision
			/// detection (narrow-phase).
			/// The default implementation of this function uses the aabb of the shape. For custom shapes, you can get a better fit.
		virtual hkReal getMaximumProjection( const hkVector4& direction ) const;

			/// Get the user data for the shape (initialized to 0)
		inline hkUlong getUserData() const;

			/// Set the user data of the shape: This is a real user data and not used by the engine otherwise.
			/// If you are interested in triangle indices, you can retrieve this information from the hkCdBody
			/// during most callbacks.
		inline void setUserData( hkUlong data );
		
			/// Finds the closest intersection between the shape and a ray defined in the shape's local space, starting at fromLocal, ending at toLocal.
			/// This is data driven, and places the results in hkShapeRayCastOutput
			/// Implementation notes: For all convex shapes except hkSphere and hkCapsule the radius of the shape will be ignored.
		virtual hkBool castRay( const hkShapeRayCastInput& input, hkShapeRayCastOutput& output ) const = 0;
	
			/// Finds the closest intersection between the shape and a ray defined in the shape's local space, starting at fromLocal, ending at toLocal.
			/// This is a callback driven raycast. For each hit found, the hkRayHitCollector receives a callback with the hit info.
			/// Implementation notes: For all convex shapes except hkSphere and hkCapsule the radius of the shape will be ignored.
		virtual void castRayWithCollector( const hkShapeRayCastInput& input, const hkCdBody& cdBody, hkRayHitCollector& collector ) const = 0;

			/// Query if the shape supports the container interface.
			/// Returns a pointer to the interface if the shape does has one or more child shapes.
			/// Otherwise returns null.
		virtual const hkShapeContainer* getContainer() const { return HK_NULL; }

	public:

		hkUlong m_userData;

	public:

		hkShape( class hkFinishLoadedObjectFlag flag ) {}
};

#include <hkcollide/shape/hkShape.inl>

#endif // HK_COLLIDE2_SHAPE_H

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
