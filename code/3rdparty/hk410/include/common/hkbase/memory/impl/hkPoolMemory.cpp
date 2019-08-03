/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkbase/hkBase.h>
#include <hkbase/memory/impl/hkPoolMemory.h>
#include <hkbase/monitor/hkMonitorStream.h>

#ifdef HK_DEBUG
#	define MEMORY_SCRUB(PTR, WHAT, NBYTES) hkString::memSet(PTR, WHAT, NBYTES)
// if you want to use NAN: #	define MEMORY_SCRUB(PTR, WHAT, NBYTES) { for (int i =0; i < NBYTES/4; i++) ((int*)PTR)[i] = 0x7FC00000; }
#else
#	define MEMORY_SCRUB(PTR, WHAT, NBYTES) /* nothing */
#endif

HK_COMPILE_TIME_ASSERT( sizeof( hkPoolMemory::hkMemoryPage ) == hkThreadMemory::PAGE_ALIGN );

int hkPoolMemory::getAllocatedSize( int nbytes )
{
	if (nbytes <= hkThreadMemory::MEMORY_MAX_SIZE_LARGE_BLOCK)
	{
		int row = getRow(nbytes);
		int size = m_row_to_size_lut[ row ];
		return size;
	}
	return hkMemory::getAllocatedSize( nbytes );
}

HK_FORCE_INLINE void hkPoolMemory::_putOnFreeList(void* p, int row)
{
	HK_ASSERT(0x277337bc,  row < hkThreadMemory::MEMORY_MAX_ALL_ROW );
	m_blocks_in_use[row]--;

	MemoryElem* me = static_cast<MemoryElem *>(p);
	MEMORY_SCRUB(me, MEMORY_FILL_FREE_SPACE, m_row_to_size_lut[row]);
	me->m_next = m_free_list[row];
	m_free_list[row] = me;
}

HK_FORCE_INLINE void* hkPoolMemory::_allocate_and_chop_page( int srcrow )
{
	if( srcrow < hkThreadMemory::MEMORY_MAX_SMALL_ROW)
	{
		int row = srcrow;
		int size = m_row_to_size_lut[ row ];
		int allocated_size = size;

		// need to alloc a new page?
		if ( size + m_current_page_space > m_current_page_end)
		{
			int allocSize = hkSizeOf(hkMemoryPage) + hkThreadMemory::MEMORY_SYSTEM_PAGE_SIZE;
			HK_TIMER_BEGIN("PageSystemMalloc", HK_NULL );
			hkMemoryPage* page = static_cast<hkMemoryPage*>( hkSystemMalloc( allocSize, hkThreadMemory::PAGE_ALIGN) );
			MEMORY_SCRUB(page, MEMORY_FILL_FREE_SPACE, allocSize);
			HK_TIMER_END();

			page->m_next = m_allocated_memory_pages;
			m_allocated_memory_pages = page;
			m_current_page_start = reinterpret_cast<char*>(page+1);
			m_current_page_space = m_current_page_start;
			m_current_page_end = m_current_page_space + hkThreadMemory::MEMORY_SYSTEM_PAGE_SIZE;
			m_memoryStatistics.m_numPages++;
		}

		void* result = static_cast<void *>(m_current_page_space);
		//
		// Welcome to the innards of the havok memory manager.
		// If you get a null pointer dereference on the next line it
		// is most likely because you did not call hkBaseSystem::init()
		//
		m_current_page_space += size;

		//each block will be put on a free list, but this does not mean, that
		//the number of blocks in use decreases (which is an assumption in the _putOnFreeList() call)
		int stats_save = m_blocks_in_use[row];
		// allocate rest to get make sure the alignment is ok
		while ( allocated_size < 512 )
		{
			if( size + m_current_page_space < m_current_page_end)
			{
				_putOnFreeList( m_current_page_space, row);
				m_current_page_space += size;
			}
			else
			{
				break;
			}
			allocated_size += size;
		}
		m_blocks_in_use[row] = stats_save;
		return result;
	}
	else
	{
		int row = srcrow;
		int size = m_row_to_size_lut[ row ];

		int allocSize = hkSizeOf(hkMemoryPage) + hkThreadMemory::MEMORY_SYSTEM_PAGE_SIZE;
		HK_TIMER_BEGIN("SystemAlloc", HK_NULL );
		hkMemoryPage* page = static_cast<hkMemoryPage*>( hkSystemMalloc(allocSize, hkThreadMemory::PAGE_ALIGN) );
		MEMORY_SCRUB(page, MEMORY_FILL_FREE_SPACE, allocSize);
		HK_TIMER_END();
		if(m_allocated_memory_pages != HK_NULL)
		{
			page->m_next = m_allocated_memory_pages->m_next;
			m_allocated_memory_pages->m_next = page;
		}
		else
		{
			page->m_next = HK_NULL;
			m_allocated_memory_pages = page;
		}
		m_memoryStatistics.m_numPages++;

		//each block will be put on a free list, but this does not mean, that
		//the number of blocks in use decreases (which is an assumption in the _putOnFreeList() call)
		int stats_save = m_blocks_in_use[row];
		char* result = reinterpret_cast<char*>(page+1);
		char* cur = result+size;

		for( int used = size; used < hkThreadMemory::MEMORY_SYSTEM_PAGE_SIZE; used += size)
		{
			_putOnFreeList( cur, row);
			cur += size;
		}
		m_blocks_in_use[row] = stats_save;
		return result;
	}
}

hkPoolMemory::hkPoolMemory(): m_criticalSection(4000)
{
	m_current_page_start = HK_NULL;
	m_current_page_end   = HK_NULL;
	m_current_page_space = HK_NULL;
	m_allocated_memory_pages = HK_NULL;
	

	int i;
	for(i = hkThreadMemory::MEMORY_MAX_ALL_ROW-1; i >= 0; --i )
	{
		m_free_list[i] = HK_NULL;
		m_blocks_in_use[i] = 0;
	}

	for(i = 0; i <= hkThreadMemory::MEMORY_MAX_SIZE_SMALL_BLOCK; ++i )
	{
		int row = hkThreadMemory::constSizeToRow(i);
		m_small_size_to_row_lut[ i ] = static_cast<char>(row);
		m_row_to_size_lut[row] = i;
	}
	for(i = 0; i < (hkThreadMemory::MEMORY_MAX_SIZE_LARGE_BLOCK >> hkThreadMemory::MEMORY_LARGE_BLOCK_RSHIFT_BITS); ++i)
	{
		int size = (i+1) << hkThreadMemory::MEMORY_LARGE_BLOCK_RSHIFT_BITS;
		int row = hkThreadMemory::constSizeToRow(size);
		m_large_size_to_row_lut[i] = row;
		m_row_to_size_lut[row] = size;
	}

	//initialise OS memory stats
	m_memoryStatistics.m_numSysAllocs = 0;
	m_memoryStatistics.m_sysAllocsSize = 0;
	m_memoryStatistics.m_sysAllocsHighMark = 0;

	m_memoryStatistics.m_numPages = 0;
	m_memoryStatistics.m_sizeOfPage = hkThreadMemory::MEMORY_SYSTEM_PAGE_SIZE;
	m_memoryStatistics.m_pageOverhead = hkSizeOf(hkMemoryPage);
	m_memoryStatistics.m_pageMemoryUsed = 0;

	m_numValidBigBlocks = 0;
}

hkPoolMemory::~hkPoolMemory()
{
	freeRuntimeBlocks();

	HK_ON_DEBUG( int allocSize = hkSizeOf(hkMemoryPage) + hkThreadMemory::MEMORY_SYSTEM_PAGE_SIZE );
	while ( m_allocated_memory_pages != HK_NULL )
	{
		hkMemoryPage* p = m_allocated_memory_pages;
		m_allocated_memory_pages = m_allocated_memory_pages->m_next;
		MEMORY_SCRUB(p, MEMORY_FILL_FREE_SPACE, allocSize );
		hkSystemFree( static_cast<void *>(p) );
	}
}



//if statistics are disabled, these functions are empty
#if HK_CONFIG_MEMORY_STATS == HK_CONFIG_MEMORY_STATS_DISABLED
void hkPoolMemory::printStatistics(hkOstream* c)
{
	HK_ASSERT(0x675bd4a3, c!=HK_NULL);
	c->printf("Statistics are disabled, please enable them in hkbase/config/hkConfigMemoryStats.h\n");
}

HK_FORCE_INLINE void hkPoolMemory::_updateStatsAllocate(int nbytes, HK_MEMORY_CLASS cl)
{
}

HK_FORCE_INLINE void hkPoolMemory::_updateStatsDeallocate(int nbytes, HK_MEMORY_CLASS cl)
{
}

HK_FORCE_INLINE hkMemoryClassInfo* hkPoolMemory::_getInfo(int classID)
{
	return (hkMemoryClassInfo*)HK_NULL;
}

void hkPoolMemory::printMemoryClassTree(hkOstream* c,int id, int level)
{
}

#else

#include <hkbase/string/hkString.h>

//print out the information for a class
HK_FORCE_INLINE void HK_CALL printInfo(hkOstream* c,hkMemoryClassInfo* info, int level)
{
	if(	info->m_classStats.m_size_in_use |
		info->m_classStats.m_blocks_in_use |
		info->m_classStats.m_n_allocates |
		info->m_classStats.m_new_size_in_use |
		info->m_classStats.m_n_news |
		info->m_classStats.m_max_size_in_use)
	{
		const int indent = level * 2;
		char buf[30];

		for(int i = 0; i < level; i++)
		{
			hkString::sprintf(&buf[(i << 1)],"--");
		}

		hkString::sprintf(&buf[indent],&info->m_name[16]);
		c->printf( "%-20s, %10i, %10i, %10i, %10i, %10i, %10i\n",
			buf,
			info->m_classStats.m_size_in_use,
			info->m_classStats.m_blocks_in_use,
			info->m_classStats.m_n_allocates,
			info->m_classStats.m_new_size_in_use,
			info->m_classStats.m_n_news,
			info->m_classStats.m_max_size_in_use);
	}
}

hkMemoryClassInfo* hkPoolMemory::_getInfo(int classID)
{
	//Any zeroes in a level will stop parsing of sub-classes
	// i.e. 01000000 is BASE class (at level 0) 01023300 (is a subclass at level 2)
	// while 01003356 is BASE class also (at level 0)
	hkMemoryClassInfo* info;

	info = &(hkMemoryClassesTable[classID]);

	return info;
}

//recursively print the tree from this entry on
//NOTE: any nodes encountered which are not children of this node
//will not be printed
void hkPoolMemory::printMemoryClassTree(hkOstream* c,int id, int level)
{
	hkMemoryClassInfo* info = _getInfo(id);
	//print out the root info for this sub-tree
	printInfo(c,info,level);

	//print out the sub-trees of this one
	int i = id + 1;
	hkMemoryClassInfo* subInfo = &info[1];

	//search through the array for
	while(subInfo->m_name != HK_NULL)
	{
		if(subInfo->m_parent == id)
		{
			printMemoryClassTree(c,i,level + 1);
		}
		i++;
		subInfo = &subInfo[1];
	}
}

void hkPoolMemory::printStatistics(hkOstream* c)
{
	int system_allocs = 0;
	unsigned i;
	hkMemoryPage* page = m_allocated_memory_pages;

	while(page != HK_NULL)
	{
		system_allocs++;
		page = page->m_next;
	}

	HK_ASSERT (0x4c53d323, c!=HK_NULL);

	c->printf("\nMemory allocated by the Operating System for the Pool\n");
	c->printf(  "*****************************************************\n");

	c->printf("page size = %d\n", hkThreadMemory::MEMORY_SYSTEM_PAGE_SIZE + hkSizeOf(hkMemoryPage));
	c->printf("number of pages = %d\n", m_memoryStatistics.m_numPages );

	c->printf("\nMemory allocated by the Operating System outside the Pool\n");
	c->printf(  "*********************************************************\n");

	c->printf("num allocs, total size, high mark\n");
	c->printf("%10i,%11i,%10i\n",
		m_memoryStatistics.m_numSysAllocs,
		m_memoryStatistics.m_sysAllocsSize,
		m_memoryStatistics.m_sysAllocsHighMark);

	c->printf("\nDetails by type\n");
	c->printf("***************\n");
	c->printf("\n%-20s, %10s, %10s, %10s, %10s, %10s, %10s\n","type", "block bytes", "blocks", "allocs", "OS bytes", "news", "high mark (block bytes)");

	propogateStatsUp();

	printMemoryClassTree(c,0,0);

	undoPropogate();

	c->printf("\nCurrent Memory details by size\n");
	c->printf(  "******************************\n");

	int size_in_use = 0;

	int blocks[hkThreadMemory::MEMORY_MAX_ALL_ROW];
	c->printf( "block size, blocks in use, free blocks, size in use, size not in use\n");
	for (i = 1; i < hkThreadMemory::MEMORY_MAX_ALL_ROW; i++)
	{
		int free_blocks = 0;
		for ( MemoryElem* el = m_free_list[i];   el != HK_NULL;   el = el->m_next )
		{
			free_blocks++;
		}
		c->printf( "%10i,%14i,%12i,%12i,%12i\n",
			m_row_to_size_lut[i],
			m_blocks_in_use[i],
			free_blocks,
			m_blocks_in_use[i] * m_row_to_size_lut[i],
			free_blocks * m_row_to_size_lut[i]);
		size_in_use += m_row_to_size_lut[i] * m_blocks_in_use[i];
		blocks[i] = m_blocks_in_use[i] + free_blocks;
	}


	c->printf("\nPre-allocate source code\n");
	c->printf("************************\n\n");
	c->printf("\t{\n");
	c->printf("\t\tint sizes[] = { 8, 16, 32, 48, 64, 96, 128, 160, 192, 256, 320, 512, 1024, 2048, 4096, 8192, -1 };\n");
	c->printf("\t\tint blocks[] = { ");
	for (i = 1; i < hkThreadMemory::MEMORY_MAX_ALL_ROW; i++)
	{
		c->printf("%d, ", blocks[i] );
	}
	c->printf("-1 };\n");
	c->printf("\t\thkArray<void*> ptrs;\n");
	c->printf("\t\tint j = 0;\n");
	c->printf("\t\twhile (sizes[j] != -1 )\n");
	c->printf("\t\t{\n");
	c->printf("\t\t\tint i;\n");
	c->printf("\t\t\tfor (i = 0; i < blocks[j]; ++i )\n");
	c->printf("\t\t\t\tptrs.pushBack( hkMemory::getInstance().allocateChunk(sizes[j], HK_MEMORY_CLASS_PRE_ALLOCATED ) );\n");
	c->printf("\t\t\tfor (i = 0; i < blocks[j]; ++i )\n");
	c->printf("\t\t\t\thkMemory::getInstance().deallocateChunk(ptrs[i], sizes[j], HK_MEMORY_CLASS_PRE_ALLOCATED );\n");
	c->printf("\t\t\tptrs.clear();\n");
	c->printf("\t\t\tj++;\n");
	c->printf("\t\t}\n");
	c->printf("\t}\n");




	c->printf("\nCurrent Memory Usage Summary\n\n");
	c->printf(  "****************************\n");

	int size_free = (m_memoryStatistics.m_sizeOfPage * m_memoryStatistics.m_numPages) - size_in_use;

	hk_size_t totalOSMemory = m_memoryStatistics.getSizeAllocatedByMemoryManager();

	c->printf("Total memory in pool = %i\n",size_in_use + size_free + m_memoryStatistics.m_pageOverhead * m_memoryStatistics.m_numPages);
	c->printf("Unused memory in pool = %i (= %2.3f percent of total)\n",size_free,(((float)size_free) / (size_in_use + size_free)) * 100);
	c->printf("Memory System Overhead = %i\n",m_memoryStatistics.m_pageOverhead * m_memoryStatistics.m_numPages);
	c->printf("Total outside Pool   = %i\n\n",m_memoryStatistics.m_sysAllocsSize);
	c->printf("**********************\n");
	c->printf("Total memory (pool and system) = %i\n",totalOSMemory);
}

HK_FORCE_INLINE void hkPoolMemory::_updateStatsAllocate(int nbytes, HK_MEMORY_CLASS cl)
{
	hkMemoryClassInfo* info = _getInfo(cl);

	HK_ASSERT2(0x39efcaad, info != HK_NULL,"Memory class identifier represents unknown class");

	if (nbytes <= hkThreadMemory::MEMORY_MAX_SIZE_LARGE_BLOCK)
	{
		int row = getRow(nbytes);
		int size = m_row_to_size_lut[ row ];

		info->m_classStats.m_n_allocates++;
		info->m_classStats.m_size_in_use += size;
		info->m_classStats.m_blocks_in_use++;
		if( info->m_classStats.m_size_in_use > info->m_classStats.m_max_size_in_use)
		{
			info->m_classStats.m_max_size_in_use = info->m_classStats.m_size_in_use;
		}
	}
	else
	{
		info->m_classStats.m_n_news += 1;
		info->m_classStats.m_new_size_in_use += nbytes;
	}
}

HK_FORCE_INLINE void hkPoolMemory::_updateStatsDeallocate(int nbytes, HK_MEMORY_CLASS cl)
{
	hkMemoryClassInfo* info = _getInfo(cl);

	HK_ASSERT2(0x207584f7, info!=HK_NULL,"Memory class identifier represents unknown class");
	if (nbytes <= hkThreadMemory::MEMORY_MAX_SIZE_LARGE_BLOCK)
	{
		int row = getRow( nbytes );
		int size = m_row_to_size_lut[ row ];

		info->m_classStats.m_size_in_use -= size;
		info->m_classStats.m_blocks_in_use--;
	}
	else
	{
		info->m_classStats.m_new_size_in_use -= nbytes;
	}
}


void hkPoolMemory::propogateStatsUp()
{
	//walk the classes, from the bottom up
	for(int i = (HK_MEMORY_CLASS_MAX - 1); i > 0; i--)
	{
		hkMemoryClassInfo* info	= _getInfo(i);
		hkMemoryClassInfo* superInfo = _getInfo(info->m_parent);

		superInfo->m_classStats += info->m_classStats;
	}
}

void hkPoolMemory::undoPropogate()
{
	//walk the classes, from the bottom up
	for(int i = 1; i <= (HK_MEMORY_CLASS_MAX - 1); i++)
	{
		hkMemoryClassInfo* info	= _getInfo(i);
		hkMemoryClassInfo* superInfo = _getInfo(info->m_parent);

		superInfo->m_classStats -= info->m_classStats;
	}

}

#endif

#if defined(HK_PLATFORM_GC) && defined(__MWERKS__)
#pragma optimization_level 0
#endif

//
// allocateChunk and deallocateChunk
//
void* hkPoolMemory::allocateChunk(int nbytes, HK_MEMORY_CLASS cl)
{
	//HK_TIMER_BEGIN("allocateChunk", HK_NULL );

	m_criticalSection.enter();
	_updateStatsAllocate(nbytes,cl);
	void* ret;

	if ( nbytes <= hkThreadMemory::MEMORY_MAX_SIZE_LARGE_BLOCK)
	{
		int row = getRow(nbytes);
		m_blocks_in_use[row]++;
		m_memoryStatistics.m_pageMemoryUsed += m_row_to_size_lut[row];
		MemoryElem* n = m_free_list[row];
		if(n)
		{
			m_free_list[row] = n->m_next;
			ret = n;
		}
		else // we ran out of space for that row
		{
			ret = _allocate_and_chop_page( row );
		}
	}
	else
	{
		m_memoryStatistics.m_numSysAllocs++;
		m_memoryStatistics.m_sysAllocsSize += nbytes;
		if ( m_memoryStatistics.m_sysAllocsSize > m_memoryStatistics.m_sysAllocsHighMark )
		{
			m_memoryStatistics.m_sysAllocsHighMark = m_memoryStatistics.m_sysAllocsSize;
		}
		ret = hkSystemMalloc( nbytes, hkThreadMemory::PAGE_ALIGN );
		HK_ASSERT2(0x73e45643, ret, "hkSystemMalloc failed" );
	}
	MEMORY_SCRUB(ret, MEMORY_FILL_UNINITIALIZED, nbytes);
	m_criticalSection.leave();
	//HK_TIMER_END();
	return ret;
}

void hkPoolMemory::allocateChunkBatch( void** blocksOut, int nblocks, int nbytes )
{
	m_criticalSection.enter();
	int rowIndex = getRow(nbytes);
	MemoryElem* head = m_free_list[rowIndex];
	for( int i = 0; i < nblocks; ++i )
	{
		_updateStatsAllocate(m_row_to_size_lut[rowIndex], HK_MEMORY_CLASS_ROOT);//XXX
		if( head )
		{
			blocksOut[i] = head;
			head = head->m_next;
		}
		else // we ran out of space for that row
		{
			m_free_list[rowIndex] = HK_NULL;
			blocksOut[i] = static_cast<MemoryElem*>(_allocate_and_chop_page( rowIndex ));
			head = m_free_list[rowIndex];
		}
	}
	m_free_list[rowIndex] = head;
	m_blocks_in_use[rowIndex] += nblocks;
	m_memoryStatistics.m_pageMemoryUsed += nblocks * m_row_to_size_lut[rowIndex];
	m_criticalSection.leave();
}

#if defined(HK_PLATFORM_GC) && defined(__MWERKS__)
#pragma optimization_level reset
#endif

void  hkPoolMemory::deallocateChunk(void* p, int nbytes, HK_MEMORY_CLASS cl)
{
	if(p)
	{
		m_criticalSection.enter();
		_updateStatsDeallocate(nbytes,cl);

		if ( nbytes <= hkThreadMemory::MEMORY_MAX_SIZE_LARGE_BLOCK )
		{
			int row = getRow(nbytes);
			m_memoryStatistics.m_pageMemoryUsed -= m_row_to_size_lut[row];
			_putOnFreeList(p, row);
		}
		else
		{
			m_memoryStatistics.m_sysAllocsSize -= nbytes;
			MEMORY_SCRUB(p, MEMORY_FILL_FREE_SPACE, nbytes);
			hkSystemFree(p);
		}
		m_criticalSection.leave();
	}
}

void hkPoolMemory::deallocateChunkBatch(void** blocks, int nblocks, int nbytes )
{
	m_criticalSection.enter();
	int rowIndex = getRow(nbytes);
	int rowSize = m_row_to_size_lut[rowIndex];
	for( int i = 0; i < nblocks; ++i )
	{
		_updateStatsDeallocate(rowSize, HK_MEMORY_CLASS_ROOT);
		_putOnFreeList(blocks[i], rowIndex);
	}
	m_memoryStatistics.m_pageMemoryUsed -= nblocks*rowSize;
	m_criticalSection.leave();
}

void hkPoolMemory::putOnFreeList(void* p, int row)
{
	_putOnFreeList(p,row);
}
void hkPoolMemory::updateStatsAllocate(int nbytes, HK_MEMORY_CLASS cl)
{
	_updateStatsAllocate(nbytes,cl);
}
void hkPoolMemory::updateStatsDeallocate(int nbytes, HK_MEMORY_CLASS cl)
{
	_updateStatsDeallocate(nbytes,cl);
}
hkMemoryClassInfo* hkPoolMemory::getInfo(int classID)
{
	return _getInfo(classID);
}



void hkPoolMemory::preAllocateRuntimeBlock(int nbytes, HK_MEMORY_CLASS cl)
{
	m_criticalSection.enter();

	HK_ASSERT2(0xafee4265, nbytes >= hkThreadMemory::MEMORY_MAX_SIZE_LARGE_BLOCK, "Big block too small.");
	HK_ASSERT2(0xafee4264, m_numValidBigBlocks < BIG_BLOCKS_SLOTS, "Out of big memory block slots.");

	//
	// append new block in slot list
	//
	BigBlockData* newBigBlock = &m_bigBlockDataArray[m_numValidBigBlocks++];
	{
		newBigBlock->m_available			= true;
		newBigBlock->m_size					= nbytes;
		newBigBlock->m_memory				= allocateChunk(nbytes, cl);
		newBigBlock->m_class				= cl;
		newBigBlock->m_providedByOutside	= false;
	}

	m_criticalSection.leave();
}

void hkPoolMemory::freeRuntimeBlocks()
{
	m_criticalSection.enter();

	for ( int i=0; i<m_numValidBigBlocks; i++)
	{
		BigBlockData* bigBlock = &m_bigBlockDataArray[i];
		if ( bigBlock->m_providedByOutside == false )
		{
			deallocateChunk(bigBlock->m_memory, bigBlock->m_size, bigBlock->m_class);
		}
	}
	m_numValidBigBlocks = 0;

	m_criticalSection.leave();
}

void* hkPoolMemory::allocateRuntimeBlock(int nbytes, HK_MEMORY_CLASS cl)
{
	if ( nbytes < hkThreadMemory::MEMORY_MAX_SIZE_LARGE_BLOCK)
	{
		return hkThreadMemory::getInstance().allocateChunk( nbytes, cl );
	}

	hkCriticalSectionLock lock(&m_criticalSection);

	BigBlockData* smallestBigBlock = HK_NULL;

	//
	// try to find the smallest(!) fitting block that is available
	//
	for ( int i=0; i<m_numValidBigBlocks; i++)
	{
		BigBlockData* bigBlock = &m_bigBlockDataArray[i];

		if ( bigBlock->m_available == true && bigBlock->m_size >= nbytes )
		{
			if ( smallestBigBlock == HK_NULL || bigBlock->m_size < smallestBigBlock->m_size )
			{
				smallestBigBlock = bigBlock;
			}
		}
	}

	if ( smallestBigBlock != HK_NULL )
	{
		smallestBigBlock->m_available = false;
		return smallestBigBlock->m_memory;
	}

	//
	// no block available or no block large enough: allocate a new block
	//
	void* memory;
	{

		//
		// calculate next 'best fitting' size; 64 bytes is the estimated OS memory management overhead
		//
		int sizeToAllocate;
		{
			sizeToAllocate = 1;
			for (int i = nbytes+64; i >0; i = i>>1)
			{
				sizeToAllocate += sizeToAllocate;
			}
			sizeToAllocate -= 64;
		}

		if ( m_numValidBigBlocks < BIG_BLOCKS_SLOTS )
		{
			HK_WARN_ALWAYS(0xaf55adde, "No block of size " << nbytes << " currently available. Allocating new block from system memory.");
			BigBlockData* newBigBlock = &m_bigBlockDataArray[m_numValidBigBlocks++];
			{
				newBigBlock->m_available			= false;
				newBigBlock->m_size					= sizeToAllocate;
				newBigBlock->m_memory				= allocateChunk(sizeToAllocate, cl);
				newBigBlock->m_class				= cl;
				newBigBlock->m_providedByOutside	= false;
			}
			memory = newBigBlock->m_memory;
		}
		else
		{
			HK_WARN_ALWAYS(0xaf55adde, "No block of size " << nbytes << " currently available and out of big block memory slots. Allocating unmanaged system memory.");
			memory = allocateChunk(nbytes, cl);
		}
	}

	return memory;
}

void hkPoolMemory::deallocateRuntimeBlock(void* p, int nbytes, HK_MEMORY_CLASS cl)
{
	if ( nbytes < hkThreadMemory::MEMORY_MAX_SIZE_LARGE_BLOCK)
	{
		hkThreadMemory::getInstance().deallocateChunk( p, nbytes, cl );
		return;
	}

	hkCriticalSectionLock lock(&m_criticalSection);

	for ( int i=0; i<m_numValidBigBlocks; i++)
	{
		BigBlockData* bigBlock = &m_bigBlockDataArray[i];
		if ( bigBlock->m_memory == p )
		{
			bigBlock->m_available = true;
			return;
		}
	}

	HK_WARN_ALWAYS(0xaf55addf, "Deallocating unmanaged big block.");
	deallocateChunk(p, nbytes, cl);
}

void hkPoolMemory::provideRuntimeBlock(void* p, int nbytes, HK_MEMORY_CLASS cl)
{
	m_criticalSection.enter();

	HK_ASSERT2(0xaf55adda, m_numValidBigBlocks < BIG_BLOCKS_SLOTS, "Out of big memory block slots. Cannot accept provided memory.");

	BigBlockData* newBigBlock = &m_bigBlockDataArray[m_numValidBigBlocks++];
	{
		newBigBlock->m_available			= true;
		newBigBlock->m_size					= nbytes;
		newBigBlock->m_memory				= p;
		newBigBlock->m_class				= cl;
		newBigBlock->m_providedByOutside	= true;
	}

	m_criticalSection.leave();
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
