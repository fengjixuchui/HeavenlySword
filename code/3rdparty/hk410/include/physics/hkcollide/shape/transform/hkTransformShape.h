/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_TRANSFORM_SHAPE_H
#define HK_COLLIDE2_TRANSFORM_SHAPE_H

#include <hkcollide/shape/hkShape.h>
#include <hkcollide/shape/hkShapeContainer.h>

extern const hkClass hkTransformShapeClass;


	/// An hkTransformShape contains an hkShape and an additional transform for that shape. 
	///	This is useful, for instance, if you
	/// want to position child shapes correctly when constructing a compound shape.
class hkTransformShape : public hkShape
{
	public:

		HK_DECLARE_REFLECTION();

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_CDINFO);

			/// Constructs a new transform shape.
			/// This adds a reference to the child shape.
		hkTransformShape( const hkShape* childShape, const hkTransform& transform );
					
			/// Get the child shape.
		inline const hkShape* getChildShape() const;
		
			/// Gets the transform from the child shape's space to this transform shape's local space.
		inline const hkTransform& getTransform() const;

			/// Gets the rotation part of the transform as a quaternion
		inline const hkQuaternion& getRotation() const;

			/// Sets the current transform.
			/// Don't do this once the shape is added to a world
			/// as the agents may have cached data dependant on it.
		void setTransform(const hkTransform& transform);


		//
		// hkShape Implementation
		//

			//	hkShape interface implementation.
		void getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out ) const;

			/// Support for MOPP. hkShape interface implementation.
		virtual hkReal getMaximumProjection( const hkVector4& direction ) const;

			/// Gets this hkShape's type. For hkTransformShapes, this is HK_SHAPE_TRANSFORM.
		virtual hkShapeType getType() const;

		virtual void calcStatistics( hkStatisticsCollector* collector) const;

			//	hkShape interface implementation.
		virtual hkBool castRay( const hkShapeRayCastInput& input, hkShapeRayCastOutput& results ) const;

			//	hkShape interface implementation.
		virtual void castRayWithCollector( const hkShapeRayCastInput& input, const hkCdBody& cdBody, hkRayHitCollector& collector ) const;

			// Inherited.
		virtual const hkShapeContainer* getContainer() const;
		
	protected:

		class hkSingleShapeContainer m_childShape;
		hkQuaternion m_rotation;
		hkTransform m_transform;

	public:

		hkTransformShape( hkFinishLoadedObjectFlag flag ) : hkShape(flag), m_childShape(flag) {}

};

#include <hkcollide/shape/transform/hkTransformShape.inl>

#endif // HK_COLLIDE2_TRANSFORM_SHAPE_H

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
