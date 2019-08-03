/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */



hkSpuStack::hkSpuStack()
{
	m_next = HK_NULL;
	m_size = 0;
#if defined (HK_SIMULATE_SPU_DMA_ON_CPU)
	m_numAllocInfos = 0;
#endif
}

HK_FORCE_INLINE hkSpuStack::~hkSpuStack()
{
#if defined (HK_SIMULATE_SPU_DMA_ON_CPU)
	HK_ASSERT2(0x5f82931b, !m_numAllocInfos, "Not all allocations got freed");
#endif	
}

void hkSpuStack::initMemory(void* p, int size)
{
	m_next = p;
	m_size = size;

#if defined (HK_SIMULATE_SPU_DMA_ON_CPU)
	hkString::memSet(p, 0xcd, size);
	m_numBytesAllocatedHighMark = 0;
	m_maxStackSize = size;
#endif
}



void* hkSpuStack::allocateStack(int numBytes, const char* what)
{
	HK_ASSERT2(0xaf8365de, !(numBytes & 0x0f) , "Allocation-size should be a multiple of 16.");
	HK_ASSERT2(0xaf8365df, m_size >= hkUint32(numBytes), "Out of stack memory.");

	void* current = m_next;

	m_next = hkAddByteOffset(current, numBytes);
	HK_ON_DEBUG(m_size = m_size.val() - numBytes);

#if defined (HK_SIMULATE_SPU_DMA_ON_CPU)
	{
		char buffer[512];

		int currentlyAllocated = m_maxStackSize - m_size;
		hkString::snprintf(buffer,510, "HighMark=%i  by %s=%i", currentlyAllocated, what, numBytes );
		AllocInfo& allocInfo = m_allocInfos[m_numAllocInfos++];
		{
			allocInfo.m_p = current;
			allocInfo.m_size = numBytes;
			allocInfo.m_what = hkString::strDup(buffer);
		}
		if ( currentlyAllocated > m_numBytesAllocatedHighMark )
		{
			m_numBytesAllocatedHighMark = currentlyAllocated;
			HK_WARN_ALWAYS( 0xf0231232, "***** SpuStackHighMark reset ****");
			for (int i =0; i < m_numAllocInfos; i++)
			{
				HK_WARN_ALWAYS( 0xf0231233, m_allocInfos[i].m_what );
			}
		}
	}
#endif

	return current;
}



void hkSpuStack::deallocateStack(void* p)
{
#if defined (HK_SIMULATE_SPU_DMA_ON_CPU)
	{
		AllocInfo& allocInfo = m_allocInfos[--m_numAllocInfos];
		HK_ASSERT2(0xaf83d35f, allocInfo.m_p == p, "Deallocating invalid memory pointer." );
		hkDeallocate(allocInfo.m_what);
		hkString::memSet(allocInfo.m_p, 0xcd, allocInfo.m_size);
	}
#endif

	HK_ON_DEBUG(hkUint32 numBytes = hkUint32(hkGetByteOffset(p, m_next)));
	m_next = p;
	HK_ON_DEBUG(m_size = m_size.val() + numBytes);
}



void hkSpuStack::deallocateStack(int numBytes)
{
#if defined (HK_SIMULATE_SPU_DMA_ON_CPU)
	{
		AllocInfo& allocInfo = m_allocInfos[--m_numAllocInfos];
		HK_ASSERT2(0xaf83d35f, allocInfo.m_size == numBytes, "Deallocating invalid memory size." );
		hkString::memSet(allocInfo.m_p, 0xcd, allocInfo.m_size);
	}
#endif

	m_next = hkAddByteOffset(m_next.val(), -numBytes);
	HK_ON_DEBUG(m_size = m_size.val() + numBytes);
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
