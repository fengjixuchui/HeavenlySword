/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkbase/hkBase.h>
#include <hkbase/memory/hkMemory.h>

HK_THREAD_LOCAL_IMPL( hkThreadMemory* ) hkThreadMemory::s_threadMemoryInstance;

hkThreadMemory::hkThreadMemory(hkMemory* memoryInstance, int maxNumElemsOnFreeList) 
: m_referenceCount(1)
{
	m_memory = memoryInstance;
	m_maxNumElemsOnFreeList = maxNumElemsOnFreeList;

	int i;
	for(i = MEMORY_MAX_ALL_ROW-1; i >= 0; --i )
	{
		m_free_list[i] = HK_NULL;
		m_num_free_blocks[i] = 0;
	}

	//XXX make shared readonly
	for(i = 0; i <= MEMORY_MAX_SIZE_SMALL_BLOCK; ++i )
	{
		int row = constSizeToRow(i);
		m_small_size_to_row_lut[ i ] = static_cast<char>(row);
		m_row_to_size_lut[row] = i;
	}

	for(i = 0; i < (MEMORY_MAX_SIZE_LARGE_BLOCK >> MEMORY_LARGE_BLOCK_RSHIFT_BITS); ++i)
	{
		int size = (i+1) << MEMORY_LARGE_BLOCK_RSHIFT_BITS;
		int row = constSizeToRow(size);
		m_large_size_to_row_lut[i] = row;
		m_row_to_size_lut[row] = size;
	}
}

void hkThreadMemory::releaseCachedMemory()
{
	void* ptrs[BATCH_SIZE];
	for(int rowIndex = MEMORY_MAX_ALL_ROW-1; rowIndex >= 0; --rowIndex )
	{
		MemoryElem* row = m_free_list[rowIndex];
		int rowBytes = m_row_to_size_lut[rowIndex];

		while( row )
		{
			int i;
			for( i = 0; i < BATCH_SIZE && row != HK_NULL; ++i )
			{
				ptrs[i] = row;
				row = row->m_next;
			}

			m_memory->deallocateChunkBatch(ptrs, i, rowBytes );
		}

		m_free_list[rowIndex] = HK_NULL;
		m_num_free_blocks[rowIndex] = 0;
	}
}

hkThreadMemory::~hkThreadMemory()
{
	releaseCachedMemory();
}

void hkThreadMemory::removeReference()
{
	--m_referenceCount;
	if(m_referenceCount==0)
	{
		delete this;
	}
}

void hkThreadMemory::addReference()
{
	++m_referenceCount;
}


#define MAGIC_MEMORY_WITH_SIZE 0x2345656

void* hkThreadMemory::onRowEmpty(int row)
{
	HK_COMPILE_TIME_ASSERT( BATCH_SIZE >= 1 );
	void* ptrs[BATCH_SIZE];
	m_memory->allocateChunkBatch(ptrs, BATCH_SIZE, m_row_to_size_lut[row] );
	for( int i = 1; i < BATCH_SIZE; ++i )
	{
		putOnFreeList(ptrs[i], row );
	}
	return ptrs[0];
}

void hkThreadMemory::onRowFull(int rowIndex)
{
	MemoryElem* row = m_free_list[rowIndex];
	int blockSize = m_row_to_size_lut[rowIndex];
	int numBlocks = m_num_free_blocks[rowIndex];

	int rowNeeded = m_maxNumElemsOnFreeList / 2;
	while( numBlocks > rowNeeded )
	{
		void* ptrs[BATCH_SIZE];
		int n = BATCH_SIZE  < numBlocks-rowNeeded ? BATCH_SIZE : numBlocks-rowNeeded;
		int i;
		for( i = 0; i < n; ++i )
		{
			ptrs[i] = row;
			row = row->m_next;
		}
		numBlocks -= n;
		m_memory->deallocateChunkBatch(ptrs, n, blockSize );
	}

	m_free_list[rowIndex] = row;
	m_num_free_blocks[rowIndex] = numBlocks;
}

//
// allocate and deallocate forward their calls to allocateChunk and deallocateChunk
// storing the extra info before the returned pointer
//

struct hkMemorySizeInfo
{
	hkInt32 m_magic;
	hkInt32 m_size;
	hkInt32 m_class;
	hkInt32 m_alignedOffset; // only used in aligned{Allocate,Deallocate}
};

#define MAGIC_MEMORY_WITH_SIZE 0x2345656

void* hkThreadMemory::allocate(int nbytes, HK_MEMORY_CLASS cl)
{
	hkMemorySizeInfo* x = static_cast<hkMemorySizeInfo*>(
		allocateChunk(nbytes + hkSizeOf(hkMemorySizeInfo), cl) );
	x->m_magic = MAGIC_MEMORY_WITH_SIZE;
	x->m_size = (int)nbytes;
	x->m_class = cl;
	return static_cast<void*>(x+1);
}

void  hkThreadMemory::deallocate(void* p)
{
	if (p) // as we allowed by convention to deallocate NULL
	{
		hkMemorySizeInfo* x = static_cast<hkMemorySizeInfo*>(p) - 1;
		HK_ASSERT2(0x3ecd6554,  x->m_magic == MAGIC_MEMORY_WITH_SIZE,
			"Deallocating freed memory or memory not allocated with hkPoolMemory::allocate.");
		x->m_magic = 0xdeadbeef;
		deallocateChunk( static_cast<void*>(x),	x->m_size + hkSizeOf( hkMemorySizeInfo ),
			static_cast<HK_MEMORY_CLASS>(x->m_class) );
	}
}

void* hkThreadMemory::alignedAllocate(int alignment, int nbytes, HK_MEMORY_CLASS cl)
{
	// allocate enough to hold the nbytes, the size info and the alignment window
	char* unaligned = reinterpret_cast<char*>(	allocateChunk(alignment + nbytes + hkSizeOf(hkMemorySizeInfo), cl) );

	// the aligned memory is the nearest aligned block, taking into account that the
	// sizeinfo which is placed just before the returned pointer.
	char* aligned = reinterpret_cast<char*>( HK_NEXT_MULTIPLE_OF( alignment, hkUlong(unaligned+hkSizeOf(hkMemorySizeInfo))) );

	// store the sizeinfo just before the returned pointer
	{
		hkMemorySizeInfo* x = reinterpret_cast<hkMemorySizeInfo*>(aligned) - 1;
		x->m_magic = MAGIC_MEMORY_WITH_SIZE;
		x->m_size = nbytes + alignment;
		x->m_class = cl;
		x->m_alignedOffset = (int)(aligned - unaligned);
	}
	return static_cast<void*>(aligned);
}

void hkThreadMemory::alignedDeallocate(void* p)
{
	if(p)
	{
		hkMemorySizeInfo* x = static_cast<hkMemorySizeInfo*>(p) - 1;
		HK_ASSERT2(0x5158bde7,  x->m_magic == MAGIC_MEMORY_WITH_SIZE,
			"Deallocating freed memory or memory not allocated with hkPoolMemory::alignedAllocate.");
		x->m_magic = 0xdeadbeef;

		char* unaligned = reinterpret_cast<char*>(p) - x->m_alignedOffset;

		deallocateChunk( static_cast<void*>(unaligned),
			x->m_size + hkSizeOf(hkMemorySizeInfo),
			static_cast<HK_MEMORY_CLASS>(x->m_class) );
	}
}

void* hkThreadMemory::allocateChunk(int nbytes, HK_MEMORY_CLASS cl)
{
	if ( nbytes <= MEMORY_MAX_SIZE_LARGE_BLOCK && m_maxNumElemsOnFreeList)
	{
		int row = getRow(nbytes);
		MemoryElem* n = m_free_list[row];
		if(n)
		{
			m_num_free_blocks[row] --;
			m_free_list[row] = n->m_next;
			return n;
		}
		return onRowEmpty( row );
	}
	else
	{
		return m_memory->allocateChunk( nbytes, cl );
	}
}

void  hkThreadMemory::deallocateChunk(void* p, int nbytes, HK_MEMORY_CLASS cl)
{
	if ( nbytes <= MEMORY_MAX_SIZE_LARGE_BLOCK && m_maxNumElemsOnFreeList )
	{
		int row = getRow(nbytes);
		putOnFreeList(p, row);
		if ( m_num_free_blocks[row] > m_maxNumElemsOnFreeList )
		{
			onRowFull(row);
		}
	}
	else
	{
		m_memory->deallocateChunk(p, nbytes, cl );
	}
}



void* hkThreadMemory::onStackOverflow(int nbytesin)
{
	HK_ASSERT(0x3fffb5df,  (nbytesin & 0xf) == 0);

	// Assert if the stack was empty
	HK_ASSERT2(0x2bebba62, ( m_stack.m_current != HK_NULL ),
						"The system needs to use a local stack for simulation.\n" \
						"You need to call hkThreadMemory::setStackArea() to set up a buffer "\
						"for this stack. Failure to do this results in lots of heap "\
						"allocations which can lead to large slowdowns." );

	// Warn if the stack just was not big enough

	HK_WARN(0x7dd65995, "The system has requested a heap allocation on stack overflow.\n"\
						"The requested size is " << nbytesin << "  and the allocated size is " << m_stackSize <<
						"\nThis can lead to large slowdowns. To fix this, you should\n" \
						"call hkThreadMemory::setStackArea() with a larger buffer.\n" \
						"Alternatively, you can override hkThreadMemory::onStackOverflow, and hkThreadMemory::onStackUnderflow\n"\
						"to hook into your own memory system.\n" \
						"See the hkbase user guide for more details.");


	const int smallestalloc = 4096;
	const int extrasize     = 1024;

	const int increasedSize = nbytesin + extrasize;
	const int size = (increasedSize > smallestalloc ) ? increasedSize : smallestalloc;
	
	//
	// NOTE: If you are overriding this function, copy and paste this implementation,
	// and simply replace the NEXT line with your own stack allocation function
	//
	char* newmem = static_cast<char*>(hkThreadMemory::getInstance().allocateChunk(size+hkSizeOf(Stack), HK_MEMORY_CLASS_ARRAY));

	// save old stack
	*reinterpret_cast<Stack*>(newmem) = m_stack;
	// update current
	m_stack.m_base = newmem + hkSizeOf(Stack);
	m_stack.m_current = m_stack.m_base + nbytesin;
	m_stack.m_end = m_stack.m_base + size;
	m_stack.m_prev = reinterpret_cast<Stack*>(newmem);
	return m_stack.m_base;
}

void hkThreadMemory::onStackUnderflow(void* ptr)
{
	char* chunkAddr = m_stack.m_base - hkSizeOf(Stack);
	int chunkSize = int(m_stack.m_end - m_stack.m_base) + hkSizeOf(Stack);
	HK_ASSERT(0x4398d3a1,  (hkUlong(chunkAddr) & 0xf) == 0);
	// restore previous stack
	m_stack = *reinterpret_cast<Stack*>(chunkAddr);
	//
	// NOTE: If you are overriding this function, copy and paste this implementation,
	// and simply replace the NEXT line with your own stack deallocation function.
	//
	deallocateChunk(chunkAddr, chunkSize, HK_MEMORY_CLASS_ARRAY);
}

void hkThreadMemory::setStackArea(void* buf, int nbytes)
{
	HK_ASSERT2(0x29cea8b0, (m_stack.m_prev == HK_NULL)	&& (m_stack.m_base == (char*)-1),		"Cannot call setStackArea when the stack is already in use.");

	hkUint32 alignOffset = hkUint32(hkUlong(buf)) & 15;
	m_stack.m_base = (char*)-1;
	m_stackSize = nbytes;
	if (alignOffset != 0)
	{
		m_stack.m_current = static_cast<char*>( buf ) + 16 - alignOffset;
		m_stack.m_end = m_stack.m_current + (nbytes - alignOffset);
	}
	else
	{
		m_stack.m_current = static_cast<char*>(buf);
		m_stack.m_end = m_stack.m_current + nbytes;
	}
}


void HK_CALL hkThreadMemory::replaceInstance(hkThreadMemory* m)
{
	if ( m )
	{
		m->addReference();
	}

	hkThreadMemory* curInstance = HK_THREAD_LOCAL_GET( s_threadMemoryInstance );
	if (curInstance)
	{
		curInstance->removeReference();
	}

	HK_THREAD_LOCAL_SET( s_threadMemoryInstance, m );
}

void HK_CALL hkThreadMemory::init()
{
	HK_THREAD_LOCAL_INIT_NULL( s_threadMemoryInstance );
}


void HK_CALL hkThreadMemory::quit()
{
	HK_THREAD_LOCAL_QUIT( s_threadMemoryInstance );
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
