/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HKBASE_FIXED_SIZE_QUEUE_H
#define HKBASE_FIXED_SIZE_QUEUE_H

#include <hkbase/thread/hkSpuUtils.h>

/// A FIFO circular queue that cannot be resized
template <typename T>
class hkFixedSizeQueue
{
	public:

			/// Creates a zero length queue, with a given storage area
			/// Data must point to a data block which is numElements * sizeof(T) big.
		hkFixedSizeQueue( T* data, int numElements );

			/// Initializes all variables to 0.
		hkFixedSizeQueue();

			/// Pushes a new element to the back of the queue - there must be spare storage for this
		inline void enqueue( const T& element );

			/// Pushes a new element to the front of the queue - there must be spare storage for this
		inline void enqueueInFront( const T& element );

			/// Fills in the data with the element at the front of the queue
		inline void dequeue( T& data );

			/// Remove one element
		inline void dequeue(  );

			/// Clears the queue
		inline void clear();

			/// Are there any elements left on the queue?
		inline hkBool isEmpty() const;

			/// How many elements are on the queue?
		inline int getSize() const;

			/// Returns the total capacity of the queue storage
		inline int getCapacity() const;


	public:

			// queue memory management
		HK_CPU_PTR(T*) m_data;
		int m_capacity; 
		int m_guaranteedCapacity;

			// queue FIFO management
		int m_head;
		int m_tail;
		int m_elementsInUse;
};

#include <hkbase/htl/hkFixedSizeQueue.inl>

#endif // HKBASE_FIXED_SIZE_QUEUE_H

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
