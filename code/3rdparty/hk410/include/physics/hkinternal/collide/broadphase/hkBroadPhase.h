/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_COLLIDE2_BROAD_PHASE_H
#define HK_COLLIDE2_BROAD_PHASE_H

#include <hkbase/hkBase.h>
#include <hkbase/thread/util/hkMultiThreadLock.h>

class hkBroadPhaseHandle;
class hkBroadPhaseHandlePair;
class hkAabb;
class hkPrimitiveCastCallback;
class hkBroadPhaseCastCollector;

//
typedef char hkBroadPhaseAabbCache;

/// The job of the broadphase is to quickly find pairs of AABBs that are intersecting, and thus to identify
/// pairs of objects that require narrowphase collision detection. Objects that can be
/// processed by the broadphase must have an hkBroadPhaseHandle, which is used as an id for each object by the broadphase. The broadphase actually keeps
/// an internal pointer to each hkBroadPhaseHandle, so do not forget this - for instance, do not copy handles.
///
/// The 16bit broadphase:
///   - Supports up to 2^14 objects (including markers)
///   - Has an internal resolution of 15 bits.
///
/// The 32bit broadphase:
///   - Supports up to 2^31 objects (including markers)
///   - Has an internal resolution of 31 bits.
///   - Is twice as slow, and uses twice as much memory as the 16bit version
///
/// Use the following macro to enable the 32bit broadphase. Put it *before* world construction. You will need to #include the hkBroadPhase.h header:
/// \code
/// #include <hkinternal/collide/broadphase/hkBroadPhase.h>
/// ...
/// HK_ENABLE_32BIT_BROADPHASE;
/// hkWorld* myWorld = new hkWorld(worldCinfo);
/// \endcode
class hkBroadPhase: public hkReferencedObject
{
	public:

		hkBroadPhase();
		~hkBroadPhase();

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_BROAD_PHASE);

			/// Adds an object to the broadphase. The new overlapping pairs are reported in pairsOut.
		virtual void addObject( hkBroadPhaseHandle* object, const hkAabb& aabb, hkArray<hkBroadPhaseHandlePair>& pairsOut  ) = 0;

			/// Adds a list of objects to the broadphase. The new overlapping pairs are reported in pairsOut.
			/// This is faster than repeatedly calling addObject, if you have a list of objects to add
		virtual void addObjectBatch( hkArray<hkBroadPhaseHandle*>& addObjectList, hkArray< hkAabb >& addAabbList, hkArray<hkBroadPhaseHandlePair>& newPairs ) = 0;

			/// Removes an object from the broadphase. The removed overlapping pairs are reported in pairsOut.
		virtual void removeObject( hkBroadPhaseHandle* object, hkArray<hkBroadPhaseHandlePair>& pairsOut ) = 0;

			/// Removes a list of objects from the broadphase. The removed overlapping pairs are reported in pairsOut.
			/// This is faster than repeatedly calling removeObject, if you have a list of objects to remove
		virtual void removeObjectBatch( hkArray<hkBroadPhaseHandle*>& removeObjectList, hkArray<hkBroadPhaseHandlePair>& delPairsOut ) = 0;

			/// Queries the total number of objects added to the broad phase.
		virtual int getNumObjects() = 0;

			/// Changes the position of an object and reports the changes. New overlaps are reported in addedPairs, removed pairs are reported in removedPairs.
			/// Note that a pair can be in both lists.
		virtual void updateAabbs( hkBroadPhaseHandle* objects[], const hkAabb* aabbs, int numObjects, hkArray<hkBroadPhaseHandlePair>& addedPairs, hkArray<hkBroadPhaseHandlePair>& removedPairs ) = 0;

			/// Optimizes internal memory layout. This can speed up the broadphase by up to 20%.
			/// This should be done after all static objects and before all dynamics objects
			/// are added to the scene.
		virtual void defragment() = 0;

			/// Gets all AABBs (for debug purposes).
		virtual void getAllAabbs( hkArray<hkAabb>& allAabbs ) = 0;

			/// Gets AABB from hkBroadPhaseHandle
		virtual void getAabb(const hkBroadPhaseHandle* object, hkAabb& aabb) const = 0;

			/// Finds all intersections between the input object and the rest.
		virtual void querySingleAabb( const hkAabb& aabb, hkArray<hkBroadPhaseHandlePair>& pairs_out) const = 0;

			/// Finds all intersection of an existing object and the rest
		virtual void reQuerySingleObject( const hkBroadPhaseHandle* object, hkArray<hkBroadPhaseHandlePair>& pairs_out) const = 0;

			/// If you want to shift the entire world including the broadphase, you need to call this
			/// function. It will simply correct all world space information by the shiftDistance.
			/// If your world is very large and both our graphics and physics coordinate spaces are single-precision,
			/// and if you stray far enough from the origin you'll get floating point errors.
			/// To prevent this you have to periodically shift our coordinate spaces when we detect we have moved too far.
			/// To shift the coordinate space, you have to reposition all objects in the world, which you can do by calling
			/// hkRigidBody::setposition(). However this is pretty CPU intensive and can have same bad side effects (disabling
		    /// tims for fixed objects).
		    ///
			/// A better way is to silently reposition all objects:
			///   - shift all world space information of all objects silently = avoid calling callbacks
			///   - updated the broadphase to reflect these new positions by calling hkBroadPhase::shiftAllObjects()
			///
			/// Notes:
			///     - This function does not work if markers are enabled
			///     - This function reports all objects which now touch the world extents (but did not touch it before)
			///     - Please use the hkutilities/collide/hkBroadPhaseBorder utility to prevent objects touching the world
			///       extents before.
			///     - Objects, which hit several borders are reported several times
			///     - Objects, which hit a border must be removed immediately to avoid internal broadphase inconsistency
			///     - The broadphase cannot shift to any floating point value. It has to round the shiftDistance up or down.
			///     - The actual shift distance is exported using effectiveShiftDistanceOut
			///
		virtual void shiftAllObjects( const hkVector4& shiftDistance, hkVector4& effectiveShiftDistanceOut, hkArray<hkBroadPhaseHandle*>& objectsEnteringBroadphaseBorder ) = 0;

			/// Shifts the broadphase by shiftDistance.
			/// Same notes as shiftAllObjects apply
		virtual void shiftBroadPhase( const hkVector4& shiftDistance, hkVector4& effectiveShiftDistanceOut, hkArray<hkBroadPhaseHandle*>& objectsEnteringBroadphaseBorder ) = 0;

			/// Input structure
		struct hkCastRayInput
		{
				/// the from position of multiple casts
			hkVector4		m_from;

				/// The number of casts you would like to make from the same m_from start position
			int       m_numCasts;

				/// Pointer to the first to position
			const hkVector4* m_toBase;

				/// the byte difference between two consecutive two positions, typically sizeof(hkVector4) = 16
			int		  m_toStriding;

				/// An optional cache. Note all rays should be within the aabb which was used to build the cache
			const hkBroadPhaseAabbCache* m_aabbCacheInfo;

			hkCastRayInput(): m_numCasts(1), m_toStriding( hkSizeOf(hkVector4)),m_aabbCacheInfo(HK_NULL){}
		};

			/// Queries the broadphase for overlapping AABBs along the ray 'from-to'.
			/// For each aabb the ray cast hits the collector passed in receives a callback.
		    /// Basically this cast allows for doing multiple casts starting from the same position.
		    /// For each input ray( from, to[x] ) the collector:
			///      at memory address: (collectorBase+x*collectorStriding) is called.<br>
			/// E.g.
			///  - by setting collectorStriding to 0, always the collecter collectorBase is called. You can identify
		    ///    the input ray by the castIndex in the hkBroadPhaseCastCollector::addBroadPhaseHandle.
		    ///  - Or if you have an array of MyCollector collectors[10] you set the collectorBase to &collectors[0] and
		    ///    the collectorStriding = hkSizeOf(MyCollector)
		virtual	void castRay( const hkCastRayInput& input, hkBroadPhaseCastCollector* collectorBase, int collectorStriding) const = 0;

			/// Get the size for the axis cache
		virtual int getAabbCacheSize() const = 0;

			/// Build the aabb cache. This is an optimization for broadphase raycasts.
			/// The idea is that the aabb cache is actually a reduced broadphase, just storing
			/// objects inside the aabb. Therefore using this cache can speed up rayCasts significantly<br>
			/// To work properly you have to:
			///  - Calc an aabb which encapsulates a set of raycasts. The smaller the aabb is the better.
		    ///    Sometimes it might make sense to sort the rays into groups and than build one cache
			///    for each group.
			///  - Allocate enough space to hold your cache (use getAabbCacheSize()). The cache size can be very
			///    big, it's actually 12 bytes per object in the broadphase. This is a worst case situation,
			///    however the cache size cannot be calculated easily in advance, so getAabbCacheSize always returns
			///    this worst case number. The best is to use hkAllocateStack to get the memory (check for overflows
			///    in your allocate stack memory, which could dramatically slow down the operations).
			///  - call calcAabbCache with your aabb and your cache memory.
			///  - Than you can call as many calls to castRay using these cache. As long as the rays are within the
			///    aabb everything works well.
			/// Note:
			///    The cache only represents a snapshot of the engine. As soon as something changes the broadphase
			///    (object moving, getting added, etc) the cache is invalid. If an invalid cache is used the engine
			///    can crash !!!!.
		virtual void calcAabbCache( const hkAabb& aabb, hkBroadPhaseAabbCache* aabbCacheOut) const = 0;

			/// Build the AABB cache from a list of collidables. This is useful for creating a cache when
		    /// you already have an AABB Phantom or a list of object that you know you want to raycast against.
		    /// This runs much faster that computing the cache from an AABB, because it doesn't have to search
		    /// through the broadphase.
		virtual void calcAabbCache( const hkArray<class hkCollidable*>& overlappingCollidables, hkBroadPhaseAabbCache* aabbCacheOut) const = 0;

			/// This structure is used as the input to hkBroadPhase::castAabb
		struct hkCastAabbInput
		{
				/// The start of the aabb cast (in world space)
			hkVector4 m_from;
				/// To end of the aabb cast (in world space)
			hkVector4 m_to;
				/// The half extents of the aabb
			hkVector4 m_halfExtents;

				/// An optional cache. Note: all casts should completly be within the aabb which was used to build the cache
			const hkBroadPhaseAabbCache* m_aabbCacheInfo;

			hkCastAabbInput(): m_aabbCacheInfo(HK_NULL){}
		};


			/// Queries the broadphase for overlapping AABBs along the aabb cast as specified in the input
			/// For each aabb the aabbcast hits the collector passed in receives a callback
		virtual void castAabb( const hkCastAabbInput& input, hkBroadPhaseCastCollector& collector ) const = 0;

			/// Turn on locking and multithreading checks
		void enableMultiThreading( int spinCountForCriticalSection);

		inline hkMultiThreadLock& getMultiThreadLock();
		inline const hkMultiThreadLock& getMultiThreadLock() const;

			/// Mark this class and all child classes for read only access for this thread
		HK_FORCE_INLINE void markForRead() const;

			/// Mark this class and all child classes for read write access for this thread
		HK_FORCE_INLINE void markForWrite();

			/// Undo markForRead
		HK_FORCE_INLINE void unmarkForRead() const;

			/// Undo markForWrite
		HK_FORCE_INLINE void unmarkForWrite();

			/// lock the broadphase
		HK_FORCE_INLINE void lock();

			/// unlock the broadphase
		HK_FORCE_INLINE void unlock();

		typedef hkBroadPhase* (HK_CALL *createBroadPhaseFunc)( const hkVector4& worldMin, const hkVector4& worldMax, int numMarkers );

		static createBroadPhaseFunc m_defaultCreationFunction;

	protected:

		void lockImplementation();
		void unlockImplementation();

	protected:

		mutable hkMultiThreadLock m_multiThreadLock;

		class hkCriticalSection *m_criticalSection;
};

hkBroadPhase* HK_CALL hk3AxisSweep16CreateBroadPhase( const hkVector4& worldMin, const hkVector4& worldMax, int numMarkers );
hkBroadPhase* HK_CALL hk3AxisSweep32CreateBroadPhase( const hkVector4& worldMin, const hkVector4& worldMax, int numMarkers );

// Use this macro to enable 32 bit broadphase. Put it *before* world construction. You will need to #include this header
// Example:
// #include <hkinternal/collide/broadphase/hkBroadPhase.h>
// ...
// HK_ENABLE_32BIT_BROADPHASE;
// hkWorld* myWorld = new hkWorld(worldCinfo);
#define HK_ENABLE_32BIT_BROADPHASE hkBroadPhase::m_defaultCreationFunction = hk3AxisSweep32CreateBroadPhase

#include <hkinternal/collide/broadphase/hkBroadPhase.inl>

#endif // HK_COLLIDE2_BROAD_PHASE_H


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
