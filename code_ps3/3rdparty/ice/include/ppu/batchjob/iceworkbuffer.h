/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_WORKBUFFER_H
#define ICE_WORKBUFFER_H

#include "icebase.h"
#include "icebatchjobbuffers.h"

#define BATCHJOB_VALIDATE_WORKBUFFER 0

namespace Ice
{
	namespace BatchJob
	{
		/*!
		 * This class manages allocations of SPU local store memory in the work buffer
		 * when constructing batch tasks, 
		 */
		class WorkBuffer
		{
		public:
			static const Location kLocationNull = 0;	// note that while WorkBuffer never contains location 0, 0 is a valid location (in the input buffer), so this is not NULL in general!
			static const U32F kAllocationIndexInvalid = (U32F)-1;

			struct Allocation {
				U32F		m_type;
				U32F		m_first;
				U32F		m_count;
				U32F		m_instance;
				U32F		m_offset;
				U32F		m_size;
			};

			/// Create a work buffer with the given total size and 4-bit (non-zero) tag
			WorkBuffer(U32F size, U32F tag);
			~WorkBuffer() {}

			/// Reset this work buffer into a pristine state
			void Reset() { FreeAllAllocations(); }

			/// Returns the 4-bit tag associated with this work buffer
			U32F GetTag() const { return m_tag; }

			/// Returns the number of allocations
			U32F GetNumAllocations() const { return m_numAllocationsLow + m_numAllocationsHigh; }
			/// Returns the number of allocations in the high region
			U32F GetNumLowAllocations() const { return m_numAllocationsLow; }
			/// Returns the number of allocations in the low region
			U32F GetNumHighAllocations() const { return m_numAllocationsHigh; }

			/// Returns the total size of this buffer
			U32F GetSize() const { return m_size; }
			/// Returns the total size remaining in the free region
			U32F GetFreeAreaSize() const { return m_offsetFreeEnd - m_offsetFreeStart; }
			/// Returns the location of the free region
			U32F GetFreeAreaOffset() const { return m_offsetFreeStart; }
			/// Returns the total size of all allocations
			U32F GetTotalAllocatedSize() const;

			/// Returns the i'th allocation from the start of the buffer, in either region
			Allocation* GetAllocation(U32F i)
			{ 
				 return (i < m_numAllocationsLow) ? m_pAllocationList[i] : 
						(i < m_numAllocationsLow + m_numAllocationsHigh) ? m_pAllocationList[kMaxAllocations-m_numAllocationsHigh + i-m_numAllocationsLow] : NULL;
			}
			Allocation const* GetAllocation(U32F i) const { return const_cast<WorkBuffer*>(this)->GetAllocation(i); }
			/// Returns the i'th low allocation from the start of the buffer
			Allocation* GetLowAllocation(U32F i) { return (i < m_numAllocationsLow) ? m_pAllocationList[i] : NULL; }
			Allocation const* GetLowAllocation(U32F i) const { return (i < m_numAllocationsLow) ? m_pAllocationList[i] : NULL; }
			/// Returns the i'th high allocation from the end of the buffer
			Allocation* GetHighAllocation(U32F i) { return (i < m_numAllocationsHigh) ? m_pAllocationList[kMaxAllocations-1-i] : NULL; }
			Allocation const* GetHighAllocation(U32F i) const { return (i < m_numAllocationsHigh) ? m_pAllocationList[kMaxAllocations-1-i] : NULL; }

			/// Returns the index of the allocation matching the given location, or kAllocationIndexInvalid
			U32F FindAllocationIndex(Location loc) const;

			/// Returns the allocation matching the given location, or NULL
			Allocation* FindAllocation(Location loc);
			Allocation const* FindAllocation(Location loc) const { return const_cast<WorkBuffer*>(this)->FindAllocation(loc); }

			/// Creates an allocation at the lower boundary of the free area
			Location AllocLow(U32F size, U32F type, U32F first = 0, U32F count = 1, U32F instance = 0);
			/// Creates an allocation at the upper boundary of the free region
			Location AllocHigh(U32F size, U32F type, U32F first = 0, U32F count = 1, U32F instance = 0);
			/// Creates an allocation at a specific free location in the low or free region.
			/// Returns the offset, or kAllocationNull if the location is not free or on other error.
			Location AllocLowAt(U32F offset, U32F size, U32F type, U32F first = 0, U32F count = 1, U32F instance = 0);
			/// Creates an allocation at a specific free location in the high or free region.
			/// Returns the offset, or kAllocationNull if the location is not free or on other error.
			Location AllocHighAt(U32F offset, U32F size, U32F type, U32F first = 0, U32F count = 1, U32F instance = 0);

			/// Free an allocation by location
			bool Free(Location loc);
			/// Free an allocation by pointer
			bool Free(Allocation *pAllocation);

			/// Merge all contiguous allocations that immediately follow this allocation which have the same type
			/// and sequential sequence numbers (i.e. next->m_first == alloc->m_first + alloc->m_count), and
			/// return the total size of the resulting block, or 0 on error.
			U32F MergeByType(Location loc);

			/// Returns the first location matching the given type and instance or kLocationNull
			Location FindLocationByType( U32F type, U32F instance = 0 ) const;

			/// On the PPU, LocationToPointer can be used to convert a location to a real pointer, given a buffer
			void* LocationToPointer(Location loc, void *pBuffer) const
			{
				if ((U32F)(loc & 0xF) != m_tag) {
					ICE_ASSERT((U32F)(loc & 0xF) == m_tag);
					return NULL;
				}
				return (U8*)pBuffer + (loc &~ 0xF);
			}
			/// On the PPU, PointerToLocation can be used to convert a pointer in a buffer into a location, given a buffer
			Location PointerToLocation(void *pLocation, void *pBuffer) const
			{
				U32F offset = (U8*)pLocation - (U8*)pBuffer;
				if ((offset & 0xF) || offset >= m_size) {
					ICE_ASSERT(!(offset & 0xF) && offset < m_size);
					return kLocationNull;
				}
				return offset | m_tag;
			}

		private:
			struct AllocationNode : public Allocation {
				struct AllocationNode *m_pNext;
			};
			static const U32F kMaxAllocations = 64;

			void InitAllocationPool();
			void FreeAllAllocations();
			Allocation *CreateAllocation(U32F offset, U32F size, U32F type, U32F first, U32F count, U32F instance);
			void FreeAllocation(Allocation *pAllocation);

			U32F FindAllocationListIndexByOffset(U32F offset) const;
			bool FreeByOffset(U32F offset);

#if BATCHJOB_VALIDATE_WORKBUFFER
			bool IsValid() const;
#endif

			AllocationNode	m_allocationPool[kMaxAllocations];
			AllocationNode	*m_pAllocationFreeList;

			Allocation*	m_pAllocationList[kMaxAllocations];
			U32F	m_size;
			U32F	m_tag;
			U32F	m_offsetFreeStart;
			U32F	m_offsetFreeEnd;
			U32F	m_numAllocationsLow;
			U32F	m_numAllocationsHigh;
		};
	}
}

#endif	//ICE_WORKBUFFER_H

