/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_SHAPE_COLLECTION_H
#define HK_COLLIDE2_SHAPE_COLLECTION_H

#include <hkcollide/shape/hkShape.h>
#include <hkcollide/shape/hkShapeContainer.h>

extern hkReal hkConvexShapeDefaultRadius;

extern const hkClass hkShapeCollectionClass;

	/// An interface to a collection of Shapes, each of which can be identified using a hkShapeKey.
class hkShapeCollection : public hkShape, public hkShapeContainer
{		
	public:

		//+abstract(1)

		HK_DECLARE_REFLECTION();

			/// Empty constructor
		hkShapeCollection();

			//
			// hkShape interface
			//

			/// Implements the castRay function. Note that for shape collections with many sub-shapes this
			/// function can be very slow. It is better to use a hkBvTreeShape::castRay instead
		virtual hkBool castRay( const hkShapeRayCastInput& input, hkShapeRayCastOutput& results ) const;
	
			/// Note: the default implementation call hkShape::castRay( ..., hkShapeRayCastOutput& results )
		virtual void castRayWithCollector( const hkShapeRayCastInput& input, const hkCdBody& cdBody, hkRayHitCollector& collector ) const;

			/// Gets the AABB for the hkShape given a local to world transform and an extra tolerance.
			/// This default implementation is rather slow and just iterates over all children
 		virtual void getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out ) const;

			/// Support for creating bounding volume hierarchies of shapes.
			/// This default implementation is rather slow and just iterates over all children
		virtual hkReal getMaximumProjection( const hkVector4& direction ) const;


			/// Gets this hkShape's type. Unless overridden by a derived class this returns HK_SHAPE_COLLECTION.
		virtual hkShapeType getType() const;

		virtual void calcStatistics( hkStatisticsCollector* collector) const;

		virtual const hkShapeContainer* getContainer() const;

		virtual bool disableWelding() const;

	public:

		hkShapeCollection( hkFinishLoadedObjectFlag flag ) : hkShape(flag) {}

			/// By default the landscape collision detection removes interior edge collisions.
			/// If you want to disable this feature, simple set this parameter to true.
		hkBool m_disableWelding;
};

#endif // HK_COLLIDE2_SHAPE_COLLECTION_H

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
