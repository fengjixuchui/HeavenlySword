/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


template <typename T>
HK_FORCE_INLINE T* hkSmallArray<T>::getData()
{
	HK_ASSERT2(0xf0100504,  m_info, "You tried to get data from an empty array" );
	return m_info->getData();
}
template <typename T>
HK_FORCE_INLINE const T* hkSmallArray<T>::getData() const 
{
	HK_ASSERT2(0xf0100504,  m_info, "You tried to get data from an empty array" );
	return m_info->getData();
}


template <typename T>
HK_FORCE_INLINE T& hkSmallArray<T>::operator[] (int i)
{
	T* data = getData();
	HK_ASSERT(0x394e9c6c,  i >= 0 && i < m_info->m_size);
	return data[i];
}

template <typename T>
HK_FORCE_INLINE const T& hkSmallArray<T>::operator[] (int i) const
{
	const T* data = getData();
	HK_ASSERT(0x264718f3,  i >= 0 && i < m_info->m_size);
	return data[i];
}


template <typename T>
HK_FORCE_INLINE int hkSmallArray<T>::getSize() const
{
	if ( !m_info )
	{
		return 0;
	}
	return m_info->m_size;
}

template <typename T>
HK_FORCE_INLINE int hkSmallArray<T>::getCapacity() const
{
	getData();
	return m_info->m_capacity;
}

template <typename T>
HK_FORCE_INLINE hkBool hkSmallArray<T>::isEmpty() const
{
	return (m_info == HK_NULL) || (m_info->m_size == 0);
}



template <typename T>
HK_FORCE_INLINE hkSmallArray<T>::hkSmallArray()
	:	m_info(HK_NULL)
{
}



template <typename T>
HK_FORCE_INLINE void hkSmallArray<T>::releaseMemory()
{
	if ( m_info )
	{
		hkThreadMemory::getInstance().deallocateChunk( (char*)(m_info), sizeof(T) * getCapacity() + sizeof(Info), HK_MEMORY_CLASS_ARRAY);
	}
}


template <typename T>
HK_FORCE_INLINE hkSmallArray<T>::~hkSmallArray()
{
	releaseMemory();
}

template <typename T>
HK_FORCE_INLINE void hkSmallArray<T>::clear()
{
	getData();
	m_info->m_size = 0;
}


template <typename T>
HK_FORCE_INLINE void hkSmallArray<T>::clearAndDeallocate()
{
	releaseMemory();
	m_info = HK_NULL;
}

template <typename T>
HK_FORCE_INLINE void hkSmallArray<T>::optimizeCapacity( int numFreeElemsLeft )
{
	if ( this->m_data )
	{
		int size = m_info->m_size + numFreeElemsLeft;
		if ( size*2 <= getCapacity() )
		{
			hkSmallArrayUtil::_reduce( this, hkSizeOf(T), HK_NULL, 0 );
		}
	}
}


template <typename T>
HK_FORCE_INLINE void hkSmallArray<T>::removeAt(int i)
{
	getData();
	HK_ASSERT(0x63bab20a,  i >= 0 && i < m_info->m_size);
	--m_info->m_size;
	getData()[i] = getData()[m_info->m_size];
}

template <typename T>
HK_FORCE_INLINE void hkSmallArray<T>::removeAtAndCopy(int index)
{
	HK_ASSERT(0x453a6437,  index >= 0 && index < m_info->m_size);
	--m_info->m_size;
	for(int i = index; i < m_info->m_size; ++i)
	{
		getData()[i] = getData()[i+1];
	}
}

template <typename T>
HK_FORCE_INLINE int hkSmallArray<T>::indexOf(const T& t) const
{
	if (m_info)
	{
		for(int i = 0; i < getSize(); ++i)
		{
			if( getData()[i] == t )
			{
				return i;
			}
		}
	}
	return -1;
}

template <typename T>
HK_FORCE_INLINE void hkSmallArray<T>::popBack()
{
	HK_ASSERT(0x5b57310e, this->m_size > 0);
	--m_info->m_size;
}


template <typename T>
HK_FORCE_INLINE void hkSmallArray<T>::reserve(int n)
{
	if( !m_info || getCapacity() < n)
	{
		int cap2 = 2 * getCapacity() + 1;
		int newSize = (n < cap2) ? cap2 : n;
		hkSmallArrayUtil::_reserve(this, newSize, sizeof(T));
	}
}

template <typename T>
HK_FORCE_INLINE void hkSmallArray<T>::pushBack(const T& t)
{
	if(!m_info || m_info->m_size == getCapacity())
	{
		hkSmallArrayUtil::_reserveMore( this, sizeof(T) );
	}
	getData()[m_info->m_size++] = t;
}

template <typename T>
HK_FORCE_INLINE void hkSmallArray<T>::pushBackUnchecked(const T& t)
{
	HK_ASSERT(0x3a2b4abb,  m_info->m_size < getCapacity());
	getData()[m_info->m_size++] = t;
}

template <typename T>
HK_FORCE_INLINE void hkSmallArray<T>::setSize(int n)
{
	reserve(n);
	m_info->m_size = hkUint16(n);
}

template <typename T>
HK_FORCE_INLINE void hkSmallArray<T>::setSizeUnchecked(int n)
{
	HK_ASSERT(0x39192e68,  n <= getCapacity());
	m_info->m_size = n;
}

template <typename T>
HK_FORCE_INLINE T* hkSmallArray<T>::expandBy( int n )
{
	int oldsize = getSize();
	setSize( oldsize + n );
	return getData()+oldsize;
}

template <typename T>
HK_FORCE_INLINE T& hkSmallArray<T>::expandOne( )
{
	if( !this->m_data || m_info->m_size == getCapacity())
	{
		hkSmallArrayUtil::_reserveMore( this, sizeof(T) );
	}
	return getData()[m_info->m_size++];
}


template <typename T>
HK_FORCE_INLINE T* hkSmallArray<T>::expandAt(int index, int numtoinsert)
{
	HK_ASSERT(0x2723cc08,  index >= 0 && index <= this->m_size );

	int newsize = numtoinsert;
	int numtomove = this->m_size - index;
	if (this->m_data){ newsize += m_info->m_size; numtomove += m_info->m_size; }
	if( newsize > getCapacity())
	{
		// note double copy from [i:end] not a problem in practice
		reserve(newsize);
	}
	copyBackwards(getData() + index + numtoinsert, getData() + index, numtomove);
	m_info->m_size = newsize;
	return getData() + index;
}


template <typename T>
typename hkSmallArray<T>::iterator hkSmallArray<T>::begin() 
{
	return getData();
}


template <typename T>
typename hkSmallArray<T>::iterator hkSmallArray<T>::end()
{
	return getData() + m_info->m_size;
}


template <typename T>
typename hkSmallArray<T>::const_iterator hkSmallArray<T>::begin() const
{
	return getData();
}


template <typename T>
typename hkSmallArray<T>::const_iterator hkSmallArray<T>::end() const
{
	return getData() + m_info->m_size;
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
