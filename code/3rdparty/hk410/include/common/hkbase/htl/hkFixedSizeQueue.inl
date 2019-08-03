/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


template <typename T>
hkFixedSizeQueue<T>::hkFixedSizeQueue( T* data, int dataSize )
	:	m_data( data ),
		m_capacity( dataSize ),
		m_head(0),
		m_tail(0),
		m_elementsInUse(0)
{
}

template <typename T>
hkFixedSizeQueue<T>::hkFixedSizeQueue( )
	:	m_data( 0 ),
		m_capacity( 0 ),
		m_head(0),
		m_tail(0),
		m_elementsInUse(0)
{
}

template <typename T>
inline hkBool hkFixedSizeQueue<T>::isEmpty() const
{
	return m_elementsInUse == 0;
}


template <typename T>
inline int hkFixedSizeQueue<T>::getSize() const
{
	return m_elementsInUse;
}


template <typename T>
inline int hkFixedSizeQueue<T>::getCapacity() const
{
	return m_capacity;
}


template <typename T>
inline void hkFixedSizeQueue<T>::clear()
{
	m_head = 0;
	m_tail = 0;
	m_elementsInUse = 0;
}


template <typename T>
inline void hkFixedSizeQueue<T>::enqueue( const T& element )
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
}


template <typename T>
inline void hkFixedSizeQueue<T>::enqueueInFront( const T& element )
{	
	HK_ASSERT2(0x81fa7245, m_elementsInUse < m_capacity, "Trying to add an element to a fixed size queue which is full");

	if( m_head == 0 )
	{
		m_head = m_capacity;
	}

	void* ppuAddr = HK_INDEX_CPU_PTR(m_data, --m_head, T);
	hkSpuDmaManager::putToMainMemoryAndWaitForCompletion( ppuAddr, &element, sizeof(element), hkSpuDmaManager::WRITE_NEW );
	hkSpuDmaManager::performFinalChecks( ppuAddr, &element, sizeof(element));
	m_elementsInUse++;
}


template <typename T>
inline void hkFixedSizeQueue<T>::dequeue( T& data )
{
	HK_ASSERT(0xf032ed23, m_elementsInUse );
	{
		const void* ppuAddr = HK_INDEX_CPU_PTR(m_data, m_head, T);
		hkSpuDmaManager::getFromMainMemoryAndWaitForCompletion( &data, ppuAddr, sizeof(data), hkSpuDmaManager::READ_COPY );
		hkSpuDmaManager::performFinalChecks( ppuAddr, &data, sizeof(data));

		if( ++m_head == m_capacity )
		{
			m_head = 0;
		}
		m_elementsInUse--;
	}
}

template <typename T>
inline void hkFixedSizeQueue<T>::dequeue(  )
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
