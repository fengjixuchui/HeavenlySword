/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HKBASE_HKMEMORY_H
#define HKBASE_HKMEMORY_H

#include <hkbase/memoryclasses/hkMemoryClassDefinitions.h>

class hkOstream;
class hkThreadMemory;

#include <hkbase/config/hkConfigMemory.h>

	/// A struct holding memory allocation information
struct hkMemoryStatistics
{
		/// The total number of memory allocations bypassing the pool memory and going 
		/// directly to the system memory. If you use the hkPoolMemory, then very big
		/// allocations (greater then 8K) can't be handled by the pool and have to go 
		/// to the system memory
	int m_numSysAllocs;

		/// The total number of bytes allocated by the system memory
	int m_sysAllocsSize;

		/// The current maximum of the total number of bytes allocated by the system memory
	int m_sysAllocsHighMark;

		/// The number of pages allocated by the pool memory
	int	m_numPages;
		/// Size of one page without overhead
	int	m_sizeOfPage;
		/// Size of the overhead
	int m_pageOverhead;
		/// Bytes allocated within the pool memory
	int m_pageMemoryUsed;

		/// Returns the size which is allocated by the memory manager.
	inline int getSizeAllocatedByMemoryManager() const
	{
		return (m_sizeOfPage + m_pageOverhead) * m_numPages + m_sysAllocsSize;
	}

		/// Returns the size which is allocated within the memory manager.
		/// Note: If you allocate a block of size 3, it will be padded to 4 bytes
		/// and 4 will be added to m_pageMemoryUsed
	inline int getMemoryUsed() const
	{
		return m_pageMemoryUsed + m_sysAllocsSize;
	}
};

	/// All memory allocations in Havok are handled through an instance of this class.
	/// Note that all allocations greater than 8 bytes should be 16 byte aligned.
	/// Allocations of 8 bytes or less should be aligned to 8 bytes.
	/// We distinguish between allocations where the size is explicitly
	/// stored or not stored. See the hkBase user guide for more details.
class hkMemory
{
	public:

		HK_DECLARE_SYSTEM_ALLOCATOR();

		friend class hkThreadMemory;

			/// Initialize the reference count to 1.
		hkMemory();

			/// Cleanup.
		virtual ~hkMemory();

			/// Allocate nblocks of nbytes. This is equivalent to nblocks calls to
			/// allocateChunk but is more lock friendly.
		virtual void allocateChunkBatch(void** blocksOut, int nblocks, int nbytes) = 0;

			/// Deallocate nblocks of nbytes. This is equivalent to nblocks calls to
			/// deallocateChunk but is more lock friendly.
		virtual void deallocateChunkBatch(void** blocksOut, int nblocks, int nbytes) = 0;

			/// Allocates a fixed size piece of memory, aligned to 16 bytes.
			/// Allocations of 8 bytes or less are aligned to 8 bytes.
			/// Note that the size is not stored and must be supplied when deallocating.
		virtual void* allocateChunk( int nbytes, HK_MEMORY_CLASS cl) = 0;

			/// Deallocates a piece of memory that has been allocated by allocateChunk
		virtual void deallocateChunk( void*, int nbytes, HK_MEMORY_CLASS cl ) = 0;

			/// Allocates a block of size nbytes of fast runtime memory
			/// If a matching block is available this function will return it.
			/// If no matching block could be found this function will allocate a new block and issue a warning,
			/// add it to its managed array of blocks and return a pointer to it.
			///
			/// General info on fast runtime blocks:
			/// Regular stack memory is not working properly in a multi-threaded environment. To cater to the need for
			/// fast memory allocations during simulation, the concept of dedicated runtime memory blocks (working
			/// as a stack memory replacement) has been devised.
			/// Runtime block functions provide access to very fast memory allocation by internally
			/// storing an array of preallocated memory blocks that are re-used on the fly as soon
			/// as they have been declared available again by explicitly deallocating them. New
			/// allocations are only performed if no available memory block of matching size could
			/// be found in the internal array of managed block.
			/// Internally these runtime blocks are only used inside the simulation (i.e. during a call to
			/// stepDeltaTime() or stepProcessMt()) and can therefore be used freely outside by the user.
		virtual void* allocateRuntimeBlock(int nbytes, HK_MEMORY_CLASS cl) = 0;

			/// Deallocates a block of fast runtime memory that has been allocated by allocateRuntimeBlock()
			/// For more information on runtime blocks see allocateRuntimeBlock()
		virtual void deallocateRuntimeBlock(void*, int nbytes, HK_MEMORY_CLASS cl) = 0;

			/// Preallocates a fast runtime memory block of size nbytes
			/// For more information on runtime blocks see allocateRuntimeBlock()
		virtual void preAllocateRuntimeBlock(int nbytes, HK_MEMORY_CLASS cl) = 0;

			/// This allows the user to provide his own block(s) of memory for fast runtime memory
			/// Note that memory blocks that have been provided by the user will not be deallocated by a call to
			/// freeRuntimeBlocks(). The user has to take care of deallocation himself.
			/// For more information on runtime blocks see allocateRuntimeBlock()
		virtual void provideRuntimeBlock(void*, int nbytes, HK_MEMORY_CLASS cl) = 0;

			/// Deallocate all fast runtime memory blocks that are still allocated and that have NOT been provided
			/// by the user (by using provideRuntimeBlock())
			/// For more information on runtime blocks see allocateRuntimeBlock()
		virtual void freeRuntimeBlocks() = 0;



			/// Prints some statistics to the specified console.
		virtual void printStatistics(hkOstream* c) = 0;

			/// Find the memory rounded up by the memory allocator for a requested memory allocation
		virtual int getAllocatedSize( int size );

			/// Get a simple synopsis of memory usage.
		inline const hkMemoryStatistics& getMemoryStatistics() const
		{
			return m_memoryStatistics;
		}

			/// Get a simple synopsis of memory usage.
		HK_FORCE_INLINE int getAvailableMemory( );

			/// Some impls (the DebugMemory for instance) can do checks on the current
			/// allocated pool. If the impl does not support the check it will just return true.
		virtual hkBool isOk() const;

			/// Status of the memory manager.
		enum MemoryState
		{
			MEMORY_STATE_OK,
			MEMORY_STATE_OUT_OF_MEMORY
		};

		MemoryState m_memoryState;
		int m_criticalMemoryLimit;

		static inline hkMemory& HK_CALL getInstance();

		static void HK_CALL replaceInstance(hkMemory* m);

		HK_FORCE_INLINE void addReference();

		HK_FORCE_INLINE void removeReference();

			/// Constant values used in debug modes.
		enum MemoryFill
		{
				/// For guard bytes before and after allocations.
			MEMORY_FILL_GUARD_AREA = 0xFD,
				/// Unused, free memory is filled with this value.
			MEMORY_FILL_FREE_SPACE = 0xDD,
				/// Newly allocated memory.
			MEMORY_FILL_UNINITIALIZED = 0xCD
		};

	protected:

		static hkMemory* s_instance;
		int m_referenceCount;

			//statistics for total amount of memory used by havok
		hkMemoryStatistics m_memoryStatistics;

		struct BigBlockData
		{
			hkBool m_available;
			int m_size;
			void* m_memory;
			HK_MEMORY_CLASS m_class;
			hkBool m_providedByOutside;
		};

		enum { BIG_BLOCKS_SLOTS=128 };
		int m_numValidBigBlocks;
		BigBlockData m_bigBlockDataArray[BIG_BLOCKS_SLOTS];
};

hkMemory& HK_CALL hkMemory::getInstance()
{
	return *s_instance;
}

void hkMemory::removeReference()
{
	--m_referenceCount;
	if(m_referenceCount==0)
	{
		delete this;
	}
}

void hkMemory::addReference()
{
	++m_referenceCount;
}

HK_FORCE_INLINE int hkMemory::getAvailableMemory()
{
	hkMemory& mem = hkMemory::getInstance();
	int memoryUsed = mem.getMemoryStatistics().getMemoryUsed();
	if ( mem.m_criticalMemoryLimit <= memoryUsed)
	{
		return 0;
	}
	return mem.m_criticalMemoryLimit - memoryUsed;
}

#endif // HKBASE_HKMEMORY_H

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
