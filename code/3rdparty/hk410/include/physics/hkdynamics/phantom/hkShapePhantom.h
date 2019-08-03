/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_SHAPE_PHANTOM_H
#define HK_DYNAMICS2_SHAPE_PHANTOM_H

#include <hkdynamics/phantom/hkPhantom.h>
#include <hkmath/basetypes/hkMotionState.h>

class hkCollisionEnvironment;
class hkCollisionDispatcher;
class hkCollidable;
class hkShape;
class hkCollisionAgent;
struct hkLinearCastInput;
struct hkCollisionInput;
class hkCdPointCollector;
class hkCdBodyPairCollector;

extern const hkClass hkShapePhantomClass;

/// This class represents a phantom with an arbitrary shape and transform for query purposes.<br>
/// Please read the hkPhantom documentation first.<br>
/// It can be used to:
///   - get the closest points between the shape and all other nearby objects
///   - linear cast the shape against all other objects
///   - get the penetrations between the shape and its neighbours.
/// - For each of the three functions, all the shapes in the world whose broadphase aabbs overlap
/// with the hkShapePhantom's broadphase aabb will be tested.
/// This phantom has no physical presence in the simulation.<br>
/// There are two implementations of this class:
///  - the hkSimpleShapePhantom  which does not do any narrowphase caching
///  - the hkCachingShapePhantom which caches the narrowphase by creating a
///    hkCollisionAgent for every overlapping pair.
/// <br>
/// - In order to decide which version to use, you should first read the hkGskBaseAgent documentation.
/// Some rules when to use hkCachingShapePhantom:
///   - Your shape is not a sphere and not a capsule (e.g. box, convexVertices...)
///   - CPU is more important than memory
///   - The position of your phantom moves with a reasonable velocity
///   - The length of the path in a linearcast is short
class hkShapePhantom : public hkPhantom 
{
	public:

		HK_DECLARE_REFLECTION();

			/// Constructor
		hkShapePhantom( const hkShape* shape, const hkTransform& transform );


			/// This function sets the position of the shape phantom and performs a linear cast from the new position of the shape
			/// phantom to the target position specified in hkLinearCastInput.
			/// - Note: this will update the broadphase aabb
			/// - Note: this will assume that the rotation is already set
			/// The reason that these two operations are performed together is for performance reasons 
			/// (so that only one call to update the broad phase aabb is needed)
			/// This version of setPositionAndLinearCast creates an aabb big enough to hold the entire cast of the shape, and
			/// calls the broad phase, then calls linearCast on all the overlapping objects the broadphase returns. 
			/// If you are performing a long linear cast you should NOT use this method. Use hkWorld::linearCast() instead.
			/// See comments for hkWorld::linearCast() for more details.
			/// The [optional] startPointCollector returns all the closest points
			/// at the start point of the linear cast. If you do not want this functionality, pass HK_NULL as the
			/// "startCollector".
		virtual void setPositionAndLinearCast( const hkVector4& position, const hkLinearCastInput& input, hkCdPointCollector& castCollector, hkCdPointCollector* startCollector ) = 0;

			/// You can call this in order to collect the closest points between the collidable and the objects
			/// at the phantoms current position. For each shape within collision tolerance of the shape in the phantom, the "collector"
			/// will receive a callback. See hkCdPointCollector for more details.
		virtual void getClosestPoints( hkCdPointCollector& collector ) = 0;
 
			/// This can be called in order to collect all penetrating shapes
			/// at the phantoms current position. For each shape which overlaps with the phantom's shape, the "collector" receives a calback.
			/// See hkCdBodyPairCollector for more details.
		virtual void getPenetrations( hkCdBodyPairCollector& collector ) = 0;

			/// Read access to the transform
		inline const hkTransform& getTransform() const;

			/// Sets the transform and updates the aabb
		void setTransform( const hkTransform& transform);

			/// A faster way to move the phantom, setting the translation only
		void setPosition( const hkVector4& position, hkReal extraTolerance = 0.0f );

			// Interface implementation
		hkWorldOperation::Result setShape( hkShape* shape );


	public:

		//
		// hkPhantom interface
		//
			/// Get the current aabb
		virtual void calcAabb( hkAabb& aabb );

	protected:

		virtual hkMotionState* getMotionState();
		class hkMotionState m_motionState;

	public:

		hkShapePhantom( class hkFinishLoadedObjectFlag flag ) : hkPhantom( flag ) {}

		//
		// INTERNAL USE ONLY
		//

		virtual void deallocateInternalArrays();
};

#include <hkdynamics/phantom/hkShapePhantom.inl>


#endif //HK_DYNAMICS2_SHAPE_PHANTOM_H


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
