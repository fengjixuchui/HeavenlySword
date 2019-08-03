/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


template <typename T>
hkQueue<T>::hkQueue(int guaranteedExtraCapacity)
	:	m_data( HK_NULL ),
		m_capacity( guaranteedExtraCapacity ),
		m_guaranteedExtraCapacity( guaranteedExtraCapacity ),
		m_head(0),
		m_tail(0),
		m_elementsInUse(0)
{
	if (guaranteedExtraCapacity > 0)
	{
		m_data = hkAllocateChunk<T>( guaranteedExtraCapacity, HK_MEMORY_CLASS_ARRAY );
	}
}


template <typename T>
hkQueue<T>::hkQueue( int n, int guaranteedExtraCapacity )
	:	m_data(hkAllocateChunk<T>( n + guaranteedExtraCapacity, HK_MEMORY_CLASS_ARRAY )),
		m_capacity(n + guaranteedExtraCapacity),
		m_guaranteedExtraCapacity(guaranteedExtraCapacity),
		m_head(0),
		m_tail(0),
		m_elementsInUse(0)
{
}

template <typename T>
inline void hkQueue<T>::setInitialCapacity( int capacity )
{
	HK_ASSERT2(0xefa09786, m_head == 0 && m_tail == 0, "setInititalCapacity must be called before any elements have been added to the queue.");

	int newCapacity = capacity > m_guaranteedExtraCapacity ? capacity : m_guaranteedExtraCapacity;

	if (m_data != HK_NULL)
	{
		hkDeallocateChunk<T>( m_data, m_capacity, HK_MEMORY_CLASS_ARRAY );
	}
	m_data = hkAllocateChunk<T>( newCapacity, HK_MEMORY_CLASS_ARRAY );
	m_capacity = newCapacity;
}



template <typename T>
hkQueue<T>::~hkQueue()
{
	releaseMemory();
}



template <typename T>
inline hkBool hkQueue<T>::isEmpty() const
{
	return m_elementsInUse == 0;
}


template <typename T>
inline int hkQueue<T>::getSize() const
{
	return m_elementsInUse;
}


template <typename T>
inline int hkQueue<T>::getCapacity() const
{
	return m_capacity;
}


template <typename T>
inline void hkQueue<T>::clear()
{
	m_head = 0;
	m_tail = 0;
	m_elementsInUse = 0;
}


template <typename T>
inline void hkQueue<T>::releaseMemory()
{
	if( m_capacity )
	{
		hkDeallocateChunk<T>( m_data, m_capacity, HK_MEMORY_CLASS_ARRAY );
	}
}


template <typename T>
inline void hkQueue<T>::reserve( int n )
{
	if( m_capacity < n)
	{
		// allocate a new buffer and copy existing data over
		if ( m_capacity * 2 >= n)
		{
			n = m_capacity * 2;
		}

		T* p = hkAllocateChunk<T>( n , HK_MEMORY_CLASS_ARRAY );

		if( m_data != HK_NULL )
		{
			if ( m_elementsInUse)
			{
				if ( m_tail < m_head )
				{
					// split
					int numHead = m_capacity - m_head;
					int numTail = m_tail;
					hkString::memCpy( p, m_data+m_head, ( sizeof(T) * numHead ) );
					hkString::memCpy( p + numHead, m_data, ( sizeof(T) * numTail ) );
				}
				else
				{
					// one block
					hkString::memCpy( p, m_data+m_head, ( sizeof(T) * m_elementsInUse ) );
				}
			}
			m_head = 0;
			m_tail = m_elementsInUse;
		}

		// release the old buffer
		releaseMemory();

		// assign to new buffer and set capacity
		m_data = (T*)p;
		m_capacity = n;
	}
}

template <typename T>
inline void hkQueue<T>::increaseCapacity( )
{
	if( m_capacity == 0 )
	{
		// if there is no current capacity default to 8 elements
		int initialSize = m_guaranteedExtraCapacity < 8 ? 8 : m_guaranteedExtraCapacity;
		reserve( initialSize );
	}

	else
	{
		reserve( m_capacity * 2 );
	}
}

// Places a new element to the back of the queue and expand storage if necessary.
template <typename T>
inline void hkQueue<T>::enqueue( const T& element )
{
#if defined (HK_PLATFORM_PS3SPU) || defined (HK_SIMULATE_SPU_DMA_ON_CPU)
	if ( HK_PLATFORM_IS_SPU )
	{
		HK_ASSERT2(0x81fa7245, m_elementsInUse < m_capacity, "Trying to add an element to a fixed size queue which is full");
		if( m_tail == m_capacity )
		{
			m_tail = 0;
		}
		void* ppuAddr = HK_INDEX_CPU_PTR(m_data, m_tail, T);
		hkSpuDmaManager::putToMainMemoryAndWaitForCompletion( ppuAddr, &element, sizeof(element), hkSpuDmaManager::WRITE_NEW );
		hkSpuDmaManager::performFinalChecks( ppuAddr, &element, sizeof(element) );
		m_tail++;

		m_elementsInUse++;
		return;
	}
	else
#endif
	{
	    if( ( m_elementsInUse + m_guaranteedExtraCapacity ) >= m_capacity )
	    {
		    increaseCapacity();
	    }
	    else if( m_tail == m_capacity )
	    {
		    m_tail = 0;
	    }
    
	    m_data[m_tail++] = element;
	    m_elementsInUse++;
    
	    //if (m_tail >= m_head) HK_ASSERT(0, m_tail - m_head == m_elementsInUse);
	    //else HK_ASSERT(0, m_capacity - m_head + m_tail == m_elementsInUse);
	}
}

// Places a new element to the back of the queue and expand storage if necessary.
template <typename T>
inline void hkQueue<T>::enqueueInFront( const T& element )
{
#if defined (HK_PLATFORM_PS3SPU) || defined (HK_SIMULATE_SPU_DMA_ON_CPU)
	if ( HK_PLATFORM_IS_SPU )
	{
		HK_ASSERT2(0x81fa7245, m_elementsInUse < m_capacity, "Trying to add an element to a fixed size queue which is full");

		if( m_head == 0 )
		{
			m_head = m_capacity;
		}

		void* ppuAddr = HK_INDEX_CPU_PTR(m_data, --m_head, T);
		hkSpuDmaManager::putToMainMemoryAndWaitForCompletion( ppuAddr, &element, sizeof(element), hkSpuDmaManager::WRITE_NEW);
		hkSpuDmaManager::performFinalChecks( ppuAddr, &element, sizeof(element));
		m_elementsInUse++;
		return;
	}
	else
#endif
	{
	    if( ( m_elementsInUse + m_guaranteedExtraCapacity ) >= m_capacity )
	    {
		    increaseCapacity();
	    }
    
	    if( m_head == 0 )
	    {
		    m_head = m_capacity;
	    }
    
	    m_data[--m_head] = element;
	    m_elementsInUse++;
	}
}


template <typename T>
inline void hkQueue<T>::dequeue( T& data )
{
#if defined (HK_PLATFORM_PS3SPU) || defined (HK_SIMULATE_SPU_DMA_ON_CPU)
	if ( HK_PLATFORM_IS_SPU )
	{
		HK_ASSERT(0xf032ed23, m_elementsInUse );

		const void* ppuAddr = HK_INDEX_CPU_PTR(m_data, m_head, T);
		hkSpuDmaManager::getFromMainMemoryAndWaitForCompletion( &data, ppuAddr, sizeof(data), hkSpuDmaManager::READ_COPY );
		hkSpuDmaManager::performFinalChecks( ppuAddr, &data, sizeof(data));

		if( ++m_head == m_capacity )
		{
			m_head = 0;
		}
		m_elementsInUse--;
		return;
	}
	else
#endif
	{
	    HK_ASSERT(0xf032ed23, m_elementsInUse );
	    {
		    data = m_data[m_head];
    
		    if( ++m_head == m_capacity )
		    {
			    m_head = 0;
		    }
		    m_elementsInUse--;
	    }
	}
}

template <typename T>
inline void hkQueue<T>::dequeue(  )
{
	HK_ASSERT(0xf032ed23, m_elementsInUse );
	{
		if( ++m_head == m_capacity )
		{
			m_head = 0;
		}
		m_elementsInUse--;
	}
}

template <typename T>
inline T* hkQueue<T>::probeDequeue(  )
{
	HK_ASSERT(0xf032ed23, m_elementsInUse );
	return &m_data[m_head];
}

template <typename T>
inline int hkQueue<T>::getGuaranteedExtraCapacity()
{
	return m_guaranteedExtraCapacity;
}
			
template <typename T>
inline void hkQueue<T>::setGuaranteedExtraCapacity( int guaranteedExtraCapacity )
{
	m_guaranteedExtraCapacity = guaranteedExtraCapacity;
	if( ( m_elementsInUse + m_guaranteedExtraCapacity ) >= m_capacity )
	{
		increaseCapacity();
	}
}



/*
* Havok SDK - PUBLIC RELEASE, BUILD(#20061017)
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
