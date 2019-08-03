/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_PHANTOM_SHAPE_H
#define HK_COLLIDE2_PHANTOM_SHAPE_H

#include <hkcollide/shape/hkShape.h>

extern const hkClass hkPhantomCallbackShapeClass;

class hkCollidable;
struct hkCollisionInput;

/// This shape allows you to create "phantom" shapes, which have no physical effect in a scene but which can trigger events
/// when other shapes come close. This shape is typically the child shape of a hkBvShape,
/// where the bounding volume of the hkBvShape is the boundary used for the phantom.
/// Phantom shapes can also be used without a hkBvShape. In this case the phantom area is defined by it's Aabb. However, in 
/// this you are better of using hkdynamics/phantom/hkAabbPhantom objects
class hkPhantomCallbackShape : public hkShape
{
	public:

		HK_DECLARE_REFLECTION();
		
	
			/// This callback is called when the phantom shape starts intersecting with another shape.
		virtual void phantomEnterEvent( const hkCollidable* phantomColl, const hkCollidable* otherColl,	const hkCollisionInput& env ) = 0;

			/// This callback is called when the phantom shape stops intersecting with another shape.
		virtual void phantomLeaveEvent( const hkCollidable* phantomColl, const hkCollidable* otherColl ) = 0;


		//
		// hkShape interface
		//

			/// hkShape interface implementation. Always returns false
		virtual hkBool castRay( const hkShapeRayCastInput& input, hkShapeRayCastOutput& results ) const;

			// hkShape interface implementation
		virtual void castRayWithCollector( const hkShapeRayCastInput& input, const hkCdBody& cdBody, hkRayHitCollector& collector ) const;

			/// hkShape interface implementation. Note that phantom shapes are most usually used in combination with an hkBvShape, 
			/// so getAabb() never gets called. However, if you are using the phantom shape directly with the broadphase, you need to
			/// implement this function. The default implementation returns an AABB with no volume.
 		virtual void getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out ) const;

			///Gets this hkShape's type. For hkPhantomCallbackShapes, this is HK_SHAPE_PHANTOM_CALLBACK.
		virtual hkShapeType getType() const;

		virtual void calcStatistics( hkStatisticsCollector* collector) const;
		
	public:

		hkPhantomCallbackShape(): hkShape( ){}

	public:

		hkPhantomCallbackShape( hkFinishLoadedObjectFlag flag ) : hkShape(flag) {}

};


#endif // HK_COLLIDE2_PHANTOM_SHAPE_H

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
