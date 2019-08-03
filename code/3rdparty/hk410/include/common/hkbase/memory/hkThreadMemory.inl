/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

int hkThreadMemory::getRow(int nbytes) const
{
	return (nbytes <= MEMORY_MAX_SIZE_SMALL_BLOCK)
				? int(m_small_size_to_row_lut[nbytes])
				: m_large_size_to_row_lut[ (nbytes-1) >> MEMORY_LARGE_BLOCK_RSHIFT_BITS];
}

int hkThreadMemory::rowToSize( int row ) const
{
	return m_row_to_size_lut[row];
}


hkThreadMemory::Stack& HK_CALL hkThreadMemory::getStack()
{
	return m_stack;
}


hkThreadMemory* HK_CALL hkThreadMemory::getInstancePtr()
{	
	return HK_THREAD_LOCAL_GET(s_threadMemoryInstance);
}

hkThreadMemory& HK_CALL hkThreadMemory::getInstance()
{	
	hkThreadMemory* instance = getInstancePtr();
	HK_ASSERT( 0xf032de34, instance != HK_NULL && "No hkThreadMemory. Did you call hkBaseSystem::initThread() for your new thread?" );
	return *instance;	
}

int hkThreadMemory::constSizeToRow( int size )
{
	HK_ASSERT(0x2a4ab5b6, size <= hkThreadMemory::MEMORY_MAX_SIZE_LARGE_BLOCK );
	HK_ASSERT(0x5dee00b2, hkThreadMemory::MEMORY_MAX_ALL_ROW == 17);

	if (size <= 8 )	return 1;
	else if (size <= 16 )	return 2;
	else if (size <= 32 )	return 3;
	else if (size <= 48 )	return 4;
	else if (size <= 64 )	return 5;
	else if (size <= 96 )	return 6;
	else if (size <= 128 )	return 7;
	else if (size <= 160 )	return 8;
	else if (size <= 192 )	return 9;
	else if (size <= 256 )	return 10;
	else if (size <= 320 )	return 11;
	else if (size <= 512 )	return 12; // end small blocks
	else if (size <= 1024)	return 13;
	else if (size <= 2048)	return 14;
	else if (size <= 4096)	return 15;
	else if (size <= 8192)	return 16; // end large blocks.
	else
	{
		HK_BREAKPOINT();
		return -1;
	}
}

void hkThreadMemory::putOnFreeList(void* p, int row)
{
	HK_ASSERT(0x277337bc,  row < MEMORY_MAX_ALL_ROW );
	m_num_free_blocks[row]++;

	MemoryElem* me = static_cast<MemoryElem *>(p);
	me->m_next = m_free_list[row];
	m_free_list[row] = me;
}

void* hkThreadMemory::allocateChunkConstSize(int nbytes, HK_MEMORY_CLASS cl)
{
	HK_ASSERT2( 0xf0432dad, nbytes <= MEMORY_MAX_SIZE_LARGE_BLOCK, "The nbytes is limited, use allocateChunk() instead" );

	int row = constSizeToRow(nbytes);
	MemoryElem* n = m_free_list[row];
	if(n)
	{
		m_num_free_blocks[row] -= 1;
		m_free_list[row] = n->m_next;
		return n;
	}
	return onRowEmpty(row);
}

void hkThreadMemory::deallocateChunkConstSize(void* p, int nbytes, HK_MEMORY_CLASS cl)
{
	HK_ASSERT2( 0xf0432dad, nbytes <= MEMORY_MAX_SIZE_LARGE_BLOCK, "The nbytes is limited, use deallocateChunk() instead" );

	int row = constSizeToRow(nbytes);
	putOnFreeList(p, row);

	if ( m_num_free_blocks[row] > m_maxNumElemsOnFreeList )
	{
		onRowFull(row);
	}
}


void* hkThreadMemory::allocateStack(int nbytesin)
{
		// like HK_NEXT_MULTIPLE_OF, but always increases memory
	int actualBytes = (nbytesin+16) & (~((16)-1));
	char* p = m_stack.m_current;
	char* end = p + actualBytes;
	if( end <= m_stack.m_end )
	{
		m_stack.m_current = end;
		return p;
	}
	else
	{
		return onStackOverflow(actualBytes);
	}
}

void hkThreadMemory::deallocateStack(void* p)
{
	m_stack.m_current = static_cast<char*>(p);
	if( p == m_stack.m_base  )
	{
		onStackUnderflow(p);
	}
}

template <typename TYPE>
HK_FORCE_INLINE TYPE* HK_CALL hkAllocateStack(int n)
{
	hkThreadMemory& mem = hkThreadMemory::getInstance();
	return static_cast<TYPE*>( mem.allocateStack(n*hkSizeOf(TYPE)) );
}

template <typename TYPE>
HK_FORCE_INLINE void HK_CALL hkDeallocateStack(TYPE* ptr)
{
	hkThreadMemory::getInstance().deallocateStack( static_cast<void*>(ptr) );
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
