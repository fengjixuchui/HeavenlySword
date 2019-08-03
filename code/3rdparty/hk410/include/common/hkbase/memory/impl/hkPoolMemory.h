/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_POOL_MEMORY
#define HK_POOL_MEMORY

#include <hkbase/memory/hkMemory.h>
#include <hkbase/memory/hkThreadMemory.h>
#include <hkbase/thread/hkCriticalSection.h>
#include <hkbase/config/hkConfigMemoryStats.h>

//This table is created iff the statistics are enabled
//it is not accessed otherwise
#include <hkbase/memoryclasses/hkMemoryClassesTable.h>
extern hkMemoryClassInfo hkMemoryClassesTable[];



/// The default havok memory manager.
/// This manager requests fixed size blocks from the system allocator
/// (hkSystemMalloc) and manages these blocks. Memory is not returned
/// until the destructor. Blocks larger than this block size (currently
/// 8192 bytes) are forwarded directly to the system allocator.
/// The pool can be preloaded based on previous runs. See the user guide.
/// This memory manager fills free and uninitialized memory when HK_DEBUG
/// is defined. See hkMemory::MemoryFill for the values.
class hkPoolMemory : public hkMemory
{
	public:

		struct MemoryElem
		{
			// NEVER NEW THIS
			MemoryElem *m_next;
		};

			// A header for each system allocation
		struct hkMemoryPage
		{
			// NEVER NEW THIS
			hkMemoryPage* m_next;
			hkInt32 m_pad[(hkThreadMemory::PAGE_ALIGN - hkSizeOf(hkMemoryPage*))/ hkSizeOf(hkInt32)];
		};
	public:

		hkPoolMemory();

		~hkPoolMemory();

		virtual void allocateChunkBatch(void** blocksOut, int nblocks, int nbytes );

		virtual void deallocateChunkBatch(void** blocks, int nblocks, int nbytes );

		virtual void* allocateChunk(int nbytes, HK_MEMORY_CLASS cl);

		virtual void deallocateChunk(void*, int nbytes, HK_MEMORY_CLASS cl);

		virtual void printStatistics(hkOstream* c);

		virtual int getAllocatedSize( int nbytes );

		virtual void preAllocateRuntimeBlock(int nbytes, HK_MEMORY_CLASS cl);

		virtual void* allocateRuntimeBlock(int nbytes, HK_MEMORY_CLASS cl);

		virtual void deallocateRuntimeBlock(void*, int nbytes, HK_MEMORY_CLASS cl);

		virtual void provideRuntimeBlock(void*, int nbytes, HK_MEMORY_CLASS cl);

		virtual void freeRuntimeBlocks();

	protected:

			// list of all pages allocated
		hkMemoryPage* m_allocated_memory_pages;

			// pointers to the start,end and current position in the current page
			// we make them char* instead of void* because ansi c++ does not permit
			// pointer arithmetic on void pointers
		char* m_current_page_start;
		char* m_current_page_end;
		char* m_current_page_space;

		hkCriticalSection m_criticalSection;

			// free list for blocks of each size
		MemoryElem* m_free_list[hkThreadMemory::MEMORY_MAX_ALL_ROW];

			// a lookup table of size of each block size
		int m_row_to_size_lut[hkThreadMemory::MEMORY_MAX_ALL_ROW];

			// a lookup table of sizes to small block size
		char m_small_size_to_row_lut[hkThreadMemory::MEMORY_MAX_SIZE_SMALL_BLOCK+1];

			// a lookup table of sizes to large block size
		int m_large_size_to_row_lut[ (hkThreadMemory::MEMORY_MAX_SIZE_LARGE_BLOCK >> hkThreadMemory::MEMORY_LARGE_BLOCK_RSHIFT_BITS) ];

			// statistics for blocks of each size
		int m_blocks_in_use[hkThreadMemory::MEMORY_MAX_ALL_ROW];

	protected:

		void putOnFreeList(void* p, int row);

			//update the stats when an allocate occurs (will do nothing if stats not enabled)
		void updateStatsAllocate(int nbytes, HK_MEMORY_CLASS cl);

			//update the stats when a deallocate occurs (will do nothing if stats not enabled)
		void updateStatsDeallocate(int nbytes, HK_MEMORY_CLASS cl);

			//fetch statistics info for a class (will do nothing if stats not enabled)
		hkMemoryClassInfo* getInfo(int classID);

			//when collating statistics, the stats should be propagated from subclasses to superclasses
		void propogateStatsUp();

			//when finished collating, we must undo these additions
		void undoPropogate();

			//print out the memory class hierarchy
		void printMemoryClassTree(hkOstream* c,int id, int level);

			// return the row of the free list for a given size
		HK_FORCE_INLINE int getRow(int nbytes)
		{
			return (nbytes <= hkThreadMemory::MEMORY_MAX_SIZE_SMALL_BLOCK)
						? int(m_small_size_to_row_lut[nbytes])
						: m_large_size_to_row_lut[ (nbytes-1) >> hkThreadMemory::MEMORY_LARGE_BLOCK_RSHIFT_BITS];
		}

	private:

		HK_FORCE_INLINE void* _allocate_and_chop_page( int row );
		HK_FORCE_INLINE void _putOnFreeList(void* p, int row);
		HK_FORCE_INLINE void _updateStatsAllocate(int nbytes, HK_MEMORY_CLASS cl);
		HK_FORCE_INLINE void _updateStatsDeallocate(int nbytes, HK_MEMORY_CLASS cl);
		HK_FORCE_INLINE hkMemoryClassInfo* _getInfo(int classID);
};


#endif // HK_POOL_MEMORY

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
