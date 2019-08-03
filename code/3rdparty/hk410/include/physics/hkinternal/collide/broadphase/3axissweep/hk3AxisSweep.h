/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE_3_AXIS_SWEEP_H
#define HK_COLLIDE_3_AXIS_SWEEP_H

#include <hkinternal/collide/broadphase/hkBroadPhase.h>
#include <hkbase/memory/hkLocalBuffer.h>



// define to build in runtime checks
#if 0 && defined(HK_DEBUG)
#	define CHECK_CONSISTENCY() checkConsistency()
#else
#	define CHECK_CONSISTENCY()
#endif


/// Implementation of an efficient broadphase algorithm.  Best used with a scene where the number
/// of entities added and removed during runtime is not excessive.  Works on a limited world size.
class hk3AxisSweep : public hkBroadPhase
{
	public:
#ifndef HK_BROADPHASE_32BIT
	typedef hkUint16 BpInt;
#else
	typedef hkUint32 BpInt;
#endif
	public:

			/// Anything within the min and max bounds will participate in broadphase culling
		hk3AxisSweep( const hkVector4& worldMin, const hkVector4& worldMax, int numMarkers);
		~hk3AxisSweep();

		void checkConsistency();

		//
		//	virtual functions implemented
		//

		virtual void addObject(    hkBroadPhaseHandle* object, const hkAabb& aabb, hkArray<hkBroadPhaseHandlePair>& newPairsOut  );

		virtual void addObjectBatch( hkArray<hkBroadPhaseHandle*>& addObjectList, hkArray< hkAabb >& addAabbList, hkArray<hkBroadPhaseHandlePair>& newPairs );

		virtual void removeObject( hkBroadPhaseHandle* object, hkArray<hkBroadPhaseHandlePair>& delPairsOut );

		virtual void removeObjectBatch( hkArray<hkBroadPhaseHandle*>& removeObjectList, hkArray<hkBroadPhaseHandlePair>& delPairsOut );

			/// This function returns the number of objects excluding the first objects
			/// which refers to the world's extents.
		virtual int getNumObjects();

		virtual void updateAabbs(  hkBroadPhaseHandle* objects[], const hkAabb* aabbs, int numObjects, hkArray<hkBroadPhaseHandlePair>& addedPairsOut, hkArray<hkBroadPhaseHandlePair>& removedPairsOut );

		virtual void defragment();

		virtual void querySingleAabb( const hkAabb& aabb, hkArray<hkBroadPhaseHandlePair>& pairs_out) const;

		virtual void reQuerySingleObject( const hkBroadPhaseHandle* object, hkArray<hkBroadPhaseHandlePair>& pairs_out) const;

			/// Gets all aabbs of all objects, good for debugging purposes.
			/// Can be used to display the broadphase contents, e.g.
			/// \code
			/// reinterpret_cast<hk3AxisSweep*>(broadPhase)->getAllAabbs( allAabbs );
			/// displayAabbs(allAabbs);
			/// \endcode
		virtual void getAllAabbs( hkArray<hkAabb>& allAabbs );

		virtual void getAabb(const hkBroadPhaseHandle* object, hkAabb& aabb) const;

			/// Fast Raycast through the broadphase using 3dda.
		virtual	void castRay( const hkCastRayInput& input, hkBroadPhaseCastCollector* collectorBase, int collectorStriding) const;

		virtual void castAabb( const hkCastAabbInput& input, hkBroadPhaseCastCollector& collector )    const;

		virtual int getAabbCacheSize() const;

		virtual void calcAabbCache( const hkAabb& aabb, hkBroadPhaseAabbCache* AabbCacheOut) const;

		virtual void calcAabbCache( const hkArray<hkCollidable*>& overlappingCollidables, hkBroadPhaseAabbCache* AabbCacheOut) const;
		class hkBpNode;
	protected:
		void calcAabbCacheInternal( const hkArray<const hkBpNode*>& overlaps, hkBroadPhaseAabbCache* AabbCacheOut) const;
	public:


		void calcStatistics( hkStatisticsCollector* collector) const;

		virtual void shiftAllObjects( const hkVector4& shiftDistance, hkVector4& effectiveShiftDistanceOut, hkArray<hkBroadPhaseHandle*>& objectsEnteringBroadphaseBorder );

		virtual void shiftBroadPhase( const hkVector4& shiftDistance, hkVector4& effectiveShiftDistanceOut, hkArray<hkBroadPhaseHandle*>& objectsEnteringBroadphaseBorder );

			/// Get the Aabb used for creation of this broadphase.
		const hkAabb& getOriginalAabb() const;

		struct hkBpAabb
		{
			HK_ALIGN16( hkUint32 m_min[4] );
			hkUint32 m_max[4];
		};

	public:

		class hkBpMarker
		{
			public:
				inline void* HK_CALL operator new(hk_size_t, void* p) { return p; }
				inline void HK_CALL operator delete(void*, void*) { }
				inline void HK_CALL operator delete(void*) { }
				BpInt m_nodeIndex;
				BpInt m_value;
				hkArray<BpInt> m_overlappingObjects;
		};

		class hkBpNode
		{
			public:

				static const int s_memberOffsets[];

				BpInt min_y;
				BpInt min_z;
				BpInt max_y;
				BpInt max_z;
				BpInt min_x;
				BpInt max_x;
				hkBroadPhaseHandle* m_handle;

				HK_FORCE_INLINE BpInt& getMin(int index)
				{
					if ( index == 0 ) return min_x;
					if ( index == 1 ) return min_y;
					return min_z;
				}

				HK_FORCE_INLINE BpInt& getMax(int index)
				{
					if ( index == 0 ) return max_x;
					if ( index == 1 ) return max_y;
					return max_z;
				}


				HK_FORCE_INLINE void setElem( int axis, int minmax, int value )
				{
#ifndef HK_BROADPHASE_32BIT
					HK_ASSERT(0x5887d49e,  value < 0x10000);
#endif
					BpInt* p = static_cast<BpInt*>(hkAddByteOffset( &min_y, s_memberOffsets[axis * 2 + minmax]));
					p[0] = BpInt(value);
				}

				HK_FORCE_INLINE int isMarker() const
				{
					return int(hkUlong(m_handle))& 1;
				}

				HK_FORCE_INLINE hkBpMarker& getMarker( hkBpMarker* markers) const
				{
					// Is 64-bit safe
					return *hkAddByteOffset( markers, int(hkUlong(m_handle)) & ~1 );
				}


				// check whether the objects are disjoint
				HK_FORCE_INLINE hkInt32 xyDisjoint( const hkBpNode& other ) const;
				HK_FORCE_INLINE hkInt32 xzDisjoint( const hkBpNode& other ) const;
				HK_FORCE_INLINE hkUint32 yzDisjoint( const hkBpNode& other ) const;
		};

	public:
		class hkBpEndPoint
		{
			public:
				int isMaxPoint() const { return m_value & 1; }
				static int HK_CALL isMaxPoint( int value )  { return value & 1; }
				hkBool operator <(const hkBpEndPoint &b) const 	{	return m_value < b.m_value;	}

			public:
				BpInt m_value;
				BpInt m_nodeIndex;
		};

		struct ValueIntPair
		{
			BpInt m_value;
			BpInt m_oldIndex;
			hkBool operator <(const ValueIntPair &b) const
			{
				return m_value < b.m_value;
			}
		};

#ifndef HK_BROADPHASE_32BIT
		// Note: AABB_MAX_VALUE is the integeger max value, AABB_MAX_FVALUE is the one we use for floating point clipping
	enum { AABB_MIN_VALUE = 0, AABB_MAX_VALUE = 0xfffc, AABB_MAX_FVALUE = 0xfffc, HK_BP_NUM_VALUE_BITS = 16 };
#else
		// we need to decrease the fvalue significantly to make sure that our result value is smaller than the AABB_MAX_VALUE
	enum { AABB_MIN_VALUE = 0, AABB_MAX_VALUE = 0x7ffffffc, AABB_MAX_FVALUE = 0x7ffff000, HK_BP_NUM_VALUE_BITS = 31 };
#endif

	static HK_ALIGN16( hkUint32 OneMask[4] );
	static HK_ALIGN16( float MaxVal[4] );

	public:
		class hkBpAxis
		{
			public:

				hkBpAxis(){}
				hkBpAxis( hkBpEndPoint *ep, int initialArraySize) : m_endPoints( ep, 0, initialArraySize ) { }
				inline void* HK_CALL operator new(hk_size_t, void* p) { return p; }
				inline void HK_CALL operator delete(void*, void*) { }
				inline void HK_CALL operator delete(void*) { }

			public:
				hkArray<hkBpEndPoint> m_endPoints;

				void mergeBatch( hkBpNode *nodes, int newIdx, int newNum, int axis, hkBpEndPoint* scratch );
				void removeBatch( hkBpNode *nodes, int axis, const hkFixedArray< int > &nodeRelocations );
				void insert( hkBpNode* nodes, int nodeIndex, BpInt min_position, BpInt maxPosition, BpInt& minInsertPositionOut, BpInt&  maxInsertPositionOut );
				void remove( int minPosition, int maxPosition);

				///	Finds a point P using the following rules.
				/// \code
				///	P[-1].m_value < value
				///	P[0].m_value >= value
				/// \endcode
				const hkBpEndPoint* find( const hkBpEndPoint* start, const hkBpEndPoint* end, BpInt value) const;
		};

	public:

		static void HK_FAST_CALL beginOverlapCheckMarker(   hkBpMarker* markers, hkBpNode& a, int nodeIndexA, hkBpNode& b, hkArray<hkBroadPhaseHandlePair>& m_newPairsOut);
		static void HK_FAST_CALL endOverlapCheckMarker  (   hkBpMarker* markers, hkBpNode& a, int nodeIndexA, hkBpNode& b, hkArray<hkBroadPhaseHandlePair>& m_deletedPairsOut);

	public:
		static void HK_FAST_CALL beginOverlap( hkBpNode& a, hkBpNode& b, hkArray<hkBroadPhaseHandlePair>& m_newPairsOut);
		static void HK_FAST_CALL endOverlap(   hkBpNode& a, hkBpNode& b, hkArray<hkBroadPhaseHandlePair>& m_deletedPairsOut);

		HK_FORCE_INLINE void convertAabbToInt( const hkAabb& aabb, hkBpAabb& aabbOut ) const;
		HK_FORCE_INLINE void convertVectorToInt( const hkVector4& vec, hkUint32* intsOut) const;

		HK_FORCE_INLINE int getNumMarkers() const;

	public:
		enum hkBpMarkerUse { HK_BP_NO_MARKER, HK_BP_USE_MARKER };
		//HK_FORCE_INLINE	void updateAxis( int axisIndex, hkBpNode* nodes, hkBpNode& node, hkUint32 nodeIndex, hkUint32 new_min, hkUint32 new_max, hkBpMarkerUse marker, hkArray<hkBroadPhaseHandlePair>& m_newPairsOut, hkArray<hkBroadPhaseHandlePair>& m_deletedPairsOut);
		HK_FORCE_INLINE void updateAabb( hkBroadPhaseHandle* object, const hkBpAabb& aabb, hkArray<hkBroadPhaseHandlePair>& m_newPairsOut, hkArray<hkBroadPhaseHandlePair>& m_deletedPairsOut);
	protected:
		void updateNodesAfterInsert( hkBpNode *nodes, int numNodes, hkBpNode& newNode );
		void updateNodesAfterDelete( hkBpNode *nodes, int numNodes, hkBpNode& oldNode );


		HK_FORCE_INLINE void querySingleAabbAddObject( hkBroadPhaseHandle* object, int newNodeIndex, const hkUint32 *bitfield, hkBpNode& refNode, hkArray<hkBroadPhaseHandlePair>& pairs_out) const;
		HK_FORCE_INLINE void querySingleAabbRemoveObject( hkBroadPhaseHandle* object, int newNodeIndex, const hkUint32 *bitfield, hkBpNode& refNode, hkArray<hkBroadPhaseHandlePair>& pairs_out) const;
		                void setBitsBasedOnXInterval( int numNodes, int x_value, const hkBpNode& queryNode, BpInt queryNodeIndex, hkUint32* bitField) const;

		enum hkBpQuerySingleType { HK_BP_REPORT_HANDLES, HK_BP_REPORT_NODES };
		HK_FORCE_INLINE void querySingleAabbSub( hkBroadPhaseHandle* object, const hkUint32 *bitfield, hkBpNode& refNode, hkBpQuerySingleType type, hkArray<hkBroadPhaseHandlePair>* pairs_out, hkArray<const hkBpNode*>* nodesOut) const;
		HK_FORCE_INLINE void _querySingleAabb( const hkAabb& aabb, hkBpQuerySingleType type, hkArray<hkBroadPhaseHandlePair>* pairs_out, hkArray<const hkBpNode*>* nodesOut) const;
		HK_FORCE_INLINE void addNodePair( const hkBpNode* n0, const hkBpNode* n1, hkArray<hkBroadPhaseHandlePair>& pairsOut, hkBool addMode) const;
			// expects these nodes endpoints to be sorted into the axes already
			// for every node we want to query, a bit with the node index in the bitFieldOfQueryNodes has to be set. (see staticFlipBit)
		void queryBatchAabbSub( hkUint32* bitFieldOfQueryNodes, hkArray<hkBroadPhaseHandlePair>& pairsOut, hkBool addMode ) const;

		void	getAabbFromNode(const hkBpNode& node, hkAabb & aabb) const;


		//
		//	Internal public section
		//
	public:
			// broadphase object variables
		hkAabb m_aabb;
		hkVector4 m_offsetLow;
		hkVector4 m_offsetHigh;
		hkVector4 m_scale;

		hkArray<hkBpNode> m_nodes;
		hkBpAxis m_axis[3];

		int m_numMarkers;
		int m_ld2NumMarkers;
		hkBpMarker *m_markers;

		// We currently do not know the rounding mode of our float to int conversion
		// if you use convertAabbToInt( x + m_intToFloatFloorCorrection) you will be sure that
		// the result will be identical to int( floor( x ) )
		hkReal    m_intToFloatFloorCorrection;
};


#endif // HK_COLLIDE_3_AXIS_SWEEP_H

/*
* Havok SDK - CLIENT RELEASE, BUILD(#20060902)
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
