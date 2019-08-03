/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_PHANTOM_H
#define HK_DYNAMICS2_PHANTOM_H

#include <hkdynamics/world/hkWorldObject.h>
#include <hkdynamics/phantom/hkPhantomOverlapListener.h>
#include <hkdynamics/phantom/hkPhantomType.h>

class hkAabb;
class hkCollidable;
class hkPhantomCinfo;
class hkPhantomListener;

extern const hkClass hkPhantomClass;

/// Helper function that returns a hkPhantom if the collidable's broad phase handle is of type hkWorldObject::BROAD_PHASE_PHANTOM
inline hkPhantom* HK_CALL hkGetPhantom(const hkCollidable* collidable);

/// A phantom is simply an aabb entry in the world's broad phase. It can be moved around,
/// and receives events when a new aabb overlap occurs with another phantom or entity,
/// or an existing overlap ends. These events can occur either when you move the phantom around, or another
/// object moves into or out of the phantom's aabb. <br>
/// One major reason for using phantoms is to exploit frame coherency the broadphase. Moving objects
/// around within the broadphase can be 5-100 times faster than querying the broadphase from scratch.<br>
/// Therefore phantoms should be is used for collision queries where you wish to make several queries for
/// an object that is relatively persistent in location:
/// <ul>
///     <li>use hkShapePhantom if your query involves a shape (e.g find all objects penetrating a sphere)
///     <li>use hkAabbPhantom if your query does not use a shape (e.g. ray casts or simple AABB checks)
///      You can also an hkAabbPhantom to create areas where events occur if the area is entered.
/// </ul>
/// <br>
/// Phantoms can raise a number of events:
/// <ul>
///     <li>phantom added/removed from the world ( received by hkPhantomListener)
///     <li>broadphase overlaps created/destroyed (received by hkPhantomOverlapListener)
///      Note: the hkPhantomOverlapListener can actually reject a new overlap, thereby acting as 
///      a user collision filter.
/// </ul>
/// <br>
/// Notes:
/// <ul>
///   <li>All broad phase objects in the havok world are actually hkCollidable objects.
///   <li>The addition and removal callback functions are addOverlappingCollidable and removeOverlappingCollidable (with the cast done for you).
///   <li>All hkPhantom objects use the hkWorld::getCollisionFilter as the filter for broad phase updates, and shape queries. The one notable
///       exception is that removeOverlappingCollidable() callbacks are fired independent of the current filter. See the UserGuide for more details.
/// </ul>
/// Attention: You only get good broadphase CPU, if the aabb is not jumping randomly around in your world. If it is,
/// the hkPhantom can only do a poor job of caching the broadphase and can result in a slower performance than calling the 
/// some functions in hkWorld.<br>
class hkPhantom : public hkWorldObject
{
	public:
	
		HK_DECLARE_REFLECTION();

		HK_DECLARE_CLASS_ALLOCATOR( HK_MEMORY_CLASS_PHANTOM );

		~hkPhantom();
			
			/// Gets the hkPhantom type.
		virtual hkPhantomType getType() const = 0;

			/// Get the current aabb
		virtual void calcAabb( hkAabb& aabb ) = 0;
		
			/// Called by the broadphase for every new overlapping collidable
		virtual void addOverlappingCollidable( hkCollidable* collidable ) = 0;

			/// Tests whether a collidable is known in the phantom
		virtual hkBool isOverlappingCollidableAdded( hkCollidable* collidable ) = 0;


			/// Called by the broadphase for every removed collidable - This may get called even if there was
			/// no corresponding addOverlappingCollidable() if the bodies are filtered not to collide.
			/// See the UserGuide for more details.
		virtual void removeOverlappingCollidable( hkCollidable* collidable ) = 0;

			/// Given a phantom (this), return a new phantom that shares all static data
			/// such as the shape, but clones any dynamic runtime data.
		virtual hkPhantom* clone() const = 0; 

		//
		// Listeners
		//

			/// Adds a phantom listener to the phantom
		void addPhantomListener( hkPhantomListener* el);

			/// Removes an phantom listener from the phantom
		void removePhantomListener( hkPhantomListener* el);

			/// Adds a phantom listener to the phantom
		void addPhantomOverlapListener( hkPhantomOverlapListener* el);

			/// Removes an phantom listener from the phantom
		void removePhantomOverlapListener( hkPhantomOverlapListener* el);

			/// Get const access to the array of phantom listeners
		inline const hkArray<hkPhantomListener*>& getPhantomListeners() const;

			/// Get const access to the array of phantom overlap listeners
		inline const hkArray<hkPhantomOverlapListener*>& getPhantomOverlapListeners() const;

		void calcStatistics( hkStatisticsCollector* collector ) const;

	public:

			// Called internally by hkWorld, this function updates all cached agents (if there are any)
			// to make sure they correspond correctly with the world's shape collection filter
		virtual void updateShapeCollectionFilter() {}

			// Fires phantom-added callbacks through all attached hkPhantomListeners
		void firePhantomAdded( );
			// Fires phantom-removed callbacks through all attached hkPhantomListeners
		void firePhantomRemoved( );
			// Fires phantom's shape set callbacks through all attached hkPhantomListeners
		void firePhantomShapeSet( );


	protected:

		inline hkPhantom( const hkShape* shape );

		void firePhantomDeleted( );

	protected:
			/// fires the callbacks and returns hkCollidableAccept
		inline hkCollidableAccept fireCollidableAdded( const hkCollidable* collidable );

			/// fires the callbacks and returns hkCollidableAccept
		inline void fireCollidableRemoved( const hkCollidable* collidable, hkBool collidableWasAdded );

	public:
			// used by hkWorldOperationUtil
		void updateBroadPhase( const hkAabb& aabb );
		
	protected:

		hkArray<hkPhantomOverlapListener*> m_overlapListeners; //+nosave
		hkArray<hkPhantomListener*>	       m_phantomListeners; //+nosave


	public:

		hkPhantom( class hkFinishLoadedObjectFlag flag ) : hkWorldObject( flag ) {}

		//
		// INTERNAL USE ONLY
		//

		// Called when a phantom is removed from the world.
		// In the case of an object being loaded from a packfile
		// This method ensures that any arrays that have grown dynamically 
		// After loading are correctly deallocated.
		virtual void deallocateInternalArrays();
};



#include <hkdynamics/phantom/hkPhantom.inl>


#endif	//HK_DYNAMICS2_PHANTOM_H

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
