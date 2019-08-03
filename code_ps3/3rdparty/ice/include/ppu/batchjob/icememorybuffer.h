/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_MEMORYBUFFER_H
#define ICE_MEMORYBUFFER_H

#define ICEMEMORYBUFFER_CLEAR_ALIGNMENT_SPACE 0

namespace Ice {
	namespace BatchJob {
		/*!
		 *	Helper class to encapsulate memory allocations from a contiguous block of memory.
		 */
		class MemoryBuffer
		{
			U8		 *m_pMemoryStart;
			U64		  m_maxSize;
			U64		  m_allocatedSize;
		
		public:
			MemoryBuffer(void* pBuffer, U64 bufferSize) { Initialize(pBuffer, bufferSize); }
		
			void Initialize(void *pBuffer, U64 bufferSize)
			{
				m_pMemoryStart = (U8*)pBuffer;
				m_maxSize = bufferSize;
				m_allocatedSize = 0;
			}
		
			//! Throw out the contents of this buffer and start allocating from the start again.
			void Reset() { m_allocatedSize = 0; }
		
			void *GetCurrentPos() { return m_pMemoryStart + m_allocatedSize; }
			void const *GetCurrentPos() const { return m_pMemoryStart + m_allocatedSize; }
		
			U64 GetMaxSize() const { return m_maxSize; }
			U64 GetAllocatedSize() const { return m_allocatedSize; }
			U64 GetRemainingSize() const { return m_maxSize - m_allocatedSize; }
		
			//! Allocate an unaligned block of memory.
			void *Allocate(U32F size)
			{
				ICE_ASSERT(m_allocatedSize + size <= m_maxSize);
				void *pAlloc = m_pMemoryStart + m_allocatedSize;
				m_allocatedSize += size;
				return pAlloc;
			}
			/*!
			 * Allocate a block of memory, aligned to an 'alignment' byte boundary.
			 * Note that this does not guarantee that the following block will also be aligned, as
			 * size may not be aligned.
			 * 'alignment' must be a power of 2.
			 */
			void *AllocateAligned(U32F size, U32F alignment = 16)
			{
				ICE_ASSERT(!(alignment & (alignment-1)));
		
				U32F addr = U32(m_pMemoryStart + m_allocatedSize);
				U32F align = (alignment - addr) & (alignment - 1);
		
				ICE_ASSERT(m_allocatedSize + align + size <= m_maxSize);
				void *pAlloc = m_pMemoryStart + m_allocatedSize + align;
#if ICEMEMORYBUFFER_CLEAR_ALIGNMENT_SPACE
				for (U8 *pAlign = m_pMemoryStart + m_allocatedSize; pAlign < pAlloc; ++pAlign)
					*pAlign = 0;
#endif
				m_allocatedSize += align + size;
				return pAlloc;
			}
		};
	}
}

#endif	//ICE_MEMORYBUFFER_H

