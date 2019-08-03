/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_AABB_PHANTOM_H
#define HK_DYNAMICS2_AABB_PHANTOM_H

#include <hkdynamics/phantom/hkPhantom.h>
#include <hkmath/basetypes/hkAabb.h>

class hkSimpleStatisticsCollector;
struct hkWorldRayCastInput;
struct hkWorldRayCastOutput;
class hkRayHitCollector;

extern const hkClass hkAabbPhantomClass;

/// This phantom simply maintains a list of hkCollidable objects that overlap with the phantom's aabb
/// hkAabbPhantoms are very fast as they use only broadphase collision detection. They are recommended
///  - for any triggers / regions you need for game logic for which a detailed shape is not necessary (i.e you can
///    use an aabb). This phantom has no hkShape associated with it. If you call getShape() on the hkCollidable
///    stored in this phantom, it will return HK_NULL.<br>
///  - to speed up short-ray casts, which are repeated every frame at roughly the same position
///    (e.g the wheels of the car in the Havok vehicle SDK).
///    See hkAabbPhantom::castRay() for details.<br>
///    Note: This is highly not recommended if you have long or non frame coherent raycasts.
class hkAabbPhantom : public hkPhantom
{
	public:

		HK_DECLARE_REFLECTION();

			/// To add a phantom to the world, you use hkWorld::addEntity( myPhantom ).
		hkAabbPhantom( const hkAabb& aabb, hkUint32 collisionFilterInfo = 0 );

		~hkAabbPhantom();


			/// Gets the hkPhantom type. For this class the type is HK_PHANTOM_AABB
		virtual hkPhantomType getType() const;

			///Gets the list of collidables that are currently overlapping with the phantom.
		inline hkArray<hkCollidable*>& getOverlappingCollidables();

			/// Casts a ray specified by "input". For each hit found the "collector" object receives
			/// a callback. See the hkRayHitCollector for details.
			/// Note: You must make sure that the hkAabb for the phantom contains the ray being cast.
			/// call setAabb() with an aabb of the correct size in the correct place if this is not the case.
			/// The phantom castRay collects up all the collidables overlapping with the aabb of the ray, and
			/// calls castRay against these collidables. This is usually much fast than calling hkWorld::castRay().
			/// If you are calling a long ray cast, you should use
			/// hkWorld::castRay instead.
		void castRay( const hkWorldRayCastInput& input, hkRayHitCollector& collector ) const;

			/// Casts a ray specified by "input". This fills out the output structure with the closest
			/// ray hit.
			/// Note: You must make sure that the hkAabb for the phantom contains the ray being cast.
			/// call setAabb() with an aabb of the correct size in the correct place if this is not the case.
			/// The phantom castRay collects up all the collidables overlapping with the aabb of the ray, and
			/// calls castRay against these collidables. If you are calling a long ray cast, you should use
			/// hkWorld::castRay instead.
		void castRay( const hkWorldRayCastInput& input, hkWorldRayCastOutput& output ) const;


		//
		// hkPhantom interface
		//

			// hkPhantom interface implementation
		virtual void calcAabb( hkAabb& aabb );

			// hkPhantom interface implementation
		virtual void addOverlappingCollidable( hkCollidable* collidable );

			// hkPhantom interface implementation
		virtual hkBool isOverlappingCollidableAdded( hkCollidable* collidable );

			// hkPhantom interface implementation
		virtual void removeOverlappingCollidable( hkCollidable* collidable );
			
			/// hkPhantom clone interface
		virtual hkPhantom* clone() const; 

			/// Sets the aabb. 
			/// This will also update the the broadphase, which may trigger the callbacks addOverlappingCollidable and removeOverlappingCollidable
		void setAabb(const hkAabb& newAabb);

			/// Get the aabb of the phantom
		inline const hkAabb& getAabb( ) const;

		void calcStatistics( hkStatisticsCollector* collector ) const;

	protected:

		virtual hkMotionState* getMotionState(){ return HK_NULL; }

		class hkAabb m_aabb;	
		hkArray<hkCollidable*> m_overlappingCollidables; //+nosave

	public:
		hkAabbPhantom( class hkFinishLoadedObjectFlag flag ) : hkPhantom ( flag ){}

		//
		// INTERNAL USE ONLY
		//
	
		virtual void deallocateInternalArrays();
};

#include <hkdynamics/phantom/hkAabbPhantom.inl>

#endif	// HK_DYNAMICS2_AABB_PHANTOM_H

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
