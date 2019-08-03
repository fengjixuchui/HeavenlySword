/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_BV_SHAPE_H
#define HK_COLLIDE2_BV_SHAPE_H

#include <hkcollide/shape/hkShape.h>
#include <hkcollide/shape/hkShapeContainer.h>

extern const hkClass hkBvShapeClass;

/// The idea of a hkBvShape is to surround a complex shape or shape collection by
/// a simple bounding volume shape. As long as the bounding volume shape does not collide
/// we know that the complex shape can't collide.
/// So, a hkBvShape needs two hkShape members, one of which is the child shape, the other
/// of which describes the bounding volume used for the child shape. <br>
/// For example, you could use an hkBvShape to
/// create a phantom with a spherical bounding volume. In that case, the child shape would be a hkPhantomCallbackShape
/// and the bounding volume shape would be a hkSphereShape.
class hkBvShape : public hkShape
{
	public:	

		HK_DECLARE_REFLECTION();

			/// Constructs the shape. The first parameter is the bounding volume shape, the second is the child shape.
		inline hkBvShape( const hkShape* boundingVolumeShape, const hkShape* childShape );
		
		
			/// Destructor, removes references to child shapes.
		inline ~hkBvShape();


			/// Gets the bounding volume shape. 
		inline const hkShape* getBoundingVolumeShape() const;

			/// Gets the child shape.
		inline const hkShape* getChildShape() const;

		//
		// hkShape interface
		//

			/// A ray cast. The current implementation directly forwards to the child shape and does not check the bounding volume shape
		virtual hkBool castRay( const hkShapeRayCastInput& input, hkShapeRayCastOutput& results ) const;

			/// hkRayCollector driven raycast implementation using the data driven 
			/// A ray cast. The current implementation directly forwards to the child shape and does not check the bounding volume shape
		virtual void castRayWithCollector( const hkShapeRayCastInput& input, const hkCdBody& cdBody, hkRayHitCollector& collector ) const;

			//	hkShape interface implementation.
 		virtual void getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out  ) const;

			/// Gets this hkShape's type. For hkBvShapes, this is HK_SHAPE_BV.
		virtual hkShapeType getType() const;
	
		virtual void calcStatistics( hkStatisticsCollector* collector) const;

		virtual const hkShapeContainer* getContainer() const;

	public:

		const hkShape*		m_boundingVolumeShape;
		class hkSingleShapeContainer m_childShape;

	public:

		hkBvShape( hkFinishLoadedObjectFlag flag ) : hkShape(flag), m_childShape(flag) {}

};

#include <hkcollide/shape/bv/hkBvShape.inl>


#endif // HK_COLLIDE2_BV_SHAPE_H

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
