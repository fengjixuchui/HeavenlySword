/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HKBASE_QUEUE_H
#define HKBASE_QUEUE_H

#if defined HK_PLATFORM_PS3SPU || defined HK_SIMULATE_SPU_DMA_ON_CPU
#	include <hkbase/thread/hkSpuDmaManager.h>
#endif
#if defined HK_SIMULATE_SPU_DMA_ON_CPU
#	include <hkbase/thread/util/hkSpuSimulator.h>
#endif


/// A FIFO circular queue
template <typename T>
class hkQueue
{
	public:

			/// Creates a zero length queue.
		hkQueue( int guaranteedExtraCapacity = 0 );

			/// Deallocates queue memory.
		~hkQueue();

			/// Creates an queue of capacity n. All elements are uninitialized.
		hkQueue( int capacity, int guaranteedExtraCapacity );

			/// Pushes a new element to the back of the queue and expands the storage if necessary.
		inline void enqueue( const T& element );

			/// Pushes a new element to the front of the queue and expands the storage if necessary.
		inline void enqueueInFront( const T& element );

			/// Fills in the data with the element at the front of the queue
		inline void dequeue( T& data );

			/// Remove one element
		inline void dequeue(  );

			/// Get read write access to the next element dequeue would return
		inline T* probeDequeue();

			/// Clears the queue
		inline void clear();

			/// Are there any elements left on the queue?
		inline hkBool isEmpty() const;

			/// How many elements are on the queue?
		inline int getSize() const;

			/// Returns the total capacity of the queue storage
		inline int getCapacity() const;

			/// Returns the extra capacity 
		inline int getGuaranteedExtraCapacity();
			
			/// Set the extra capacity
		inline void setGuaranteedExtraCapacity( int guaranteedExtraCapacity );

			/// Set the initial capacity for the queue.
			/// This must be called before any elements have been added to the queue.
		inline void setInitialCapacity( int capacity );

		// Allocates more internal queue storage
		inline void reserve(int n);

	private:
		inline void increaseCapacity();

			// frees the internal storage
		void releaseMemory();

			// queue memory management
		T* m_data;
		int m_capacity; 
		int m_guaranteedExtraCapacity;

			// queue FIFO management
		int m_head;
		int m_tail;
		int m_elementsInUse;
};

#include <hkbase/htl/hkQueue.inl>

#endif // HKBASE_QUEUE_H

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
