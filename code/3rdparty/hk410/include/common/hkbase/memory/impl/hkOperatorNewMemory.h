/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_OPERATOR_NEW_MEMORY
#define HK_OPERATOR_NEW_MEMORY

#include <hkbase/memory/hkMemory.h>

// given a 4 byte aligned pointer (as typically returned by new)
// round it up to align and store the offset just before the
// returned pointer.
static HK_FORCE_INLINE void* hkMemoryRoundUp(void* pvoid, int align=16)
{
	HK_ASSERT2(0x60cf3558,  (reinterpret_cast<hkUlong>(pvoid) & 0x3) == 0, "Pointer was not 4 byte aligned");
	hkUlong origptr = reinterpret_cast<hkUlong>(pvoid);
	hkUlong aligned = HK_NEXT_MULTIPLE_OF( align, origptr+1);
	reinterpret_cast<int*>(aligned)[-1] = int(aligned - origptr);
	return reinterpret_cast<void*>(aligned);
}

// given a pointer from hkMemoryRoundUp, recover the original pointer.
static HK_FORCE_INLINE void* hkMemoryRoundDown(void* p)
{
	int offset = reinterpret_cast<int*>(p)[-1];
	return static_cast<char*>(p) - offset;
}

/// Simply forward all calls to ::new and ::delete
/// Note: If you have overridden global new and delete to align on
/// 16 byte boundaries you can safely remove the hkMemoryRound{Up,Down}
/// from all but the aligne{Allocate,Deallocate} methods.
class hkOperatorNewMemory : public hkMemory
{
	public:

		virtual void allocateChunkBatch(void** blocksOut, int nblocks, int nbytes)
		{
			for( int i = 0; i < nblocks; ++i )
			{
				blocksOut[i] = allocateChunk(nbytes, HK_MEMORY_CLASS_ROOT);
			}
		}

		virtual void deallocateChunkBatch(void** blocksOut, int nblocks, int nbytes)
		{
			for( int i = 0; i < nblocks; ++i )
			{
				deallocateChunk(blocksOut[i], nbytes, HK_MEMORY_CLASS_ROOT);
			}
		}

		virtual void* allocateChunk(int nbytes, HK_MEMORY_CLASS cl)
		{
			return hkMemoryRoundUp(::new char[nbytes+16]);
		}

		virtual void deallocateChunk(void* p, int nbytes, HK_MEMORY_CLASS )
		{
			if(p)
			{
				::delete [] static_cast<char*>( hkMemoryRoundDown(p) );
			}
		}

		virtual void printStatistics(hkOstream* c)
		{
		}

		virtual void preAllocateRuntimeBlock(int nbytes, HK_MEMORY_CLASS cl)
		{
			HK_WARN_ALWAYS(0xaf55ad00, "Not implemented for hkOperatorNewMemory. Doing nothing.");
		}

		virtual void* allocateRuntimeBlock(int nbytes, HK_MEMORY_CLASS cl)
		{
			HK_WARN_ALWAYS(0xaf55ad01, "Not implemented for hkOperatorNewMemory. Forwarding to OS.");
			void* memory = allocateChunk(nbytes, cl);
			return memory;
		}

		virtual void deallocateRuntimeBlock(void* p, int nbytes, HK_MEMORY_CLASS cl)
		{
			HK_WARN_ALWAYS(0xaf55ad02, "Not implemented for hkOperatorNewMemory. Forwarding to OS.");
			deallocateChunk(p, nbytes, cl);
		}

		virtual void provideRuntimeBlock(void*, int nbytes, HK_MEMORY_CLASS cl)
		{
			HK_WARN_ALWAYS(0xaf55ad03, "Not implemented for hkOperatorNewMemory. Doing nothing.");
		}

		virtual void freeRuntimeBlocks()
		{
			HK_WARN_ALWAYS(0xaf55ad03, "Not implemented for hkOperatorNewMemory. Doing nothing.");
		}

};

#endif // HK_OPERATOR_NEW_MEMORY



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
