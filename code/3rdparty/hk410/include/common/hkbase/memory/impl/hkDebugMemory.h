/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// The demoframework will use hkDebugMemory if the demos project properties
// is given the "-c" (mem Check) option in 'Debugging->Command Arguments'.
// hkDebugMemory will not operate in debug configuration as defined by Havok.
// Debug configuration is an optimized build with line numbers enabled.
// If there are memory leaks, their location and call stack 
// are printed to the debug window.

#ifndef HK_DEBUG_MEMORY_H
#define HK_DEBUG_MEMORY_H

#include <hkbase/memory/hkMemory.h>
#include <hkbase/memory/hkStackTracer.h>
#include <hkbase/thread/hkCriticalSection.h>

// use STL here so we avoid memory recursion.
#include <map>

/// Memory implementation to find common memory problems.
/// hkDebugMemory finds leaks, multiple frees and underrun/overruns.
/// hkDebugMemory always scrubs allocations in contrast to hkPoolMemory
/// which only scrubs when HK_DEBUG is defined.
/// See the userguide for more details.<p>
class hkDebugMemory : public hkMemory
{
	public:

		typedef void (*OutputStringFunc)(const char* s, void* arg);

		static void HK_CALL defaultOutputStringFunc(const char* s, void* a);


		virtual void allocateChunkBatch(void** blocksOut, int nblocks, int nbytes )
		{
			for( int i = 0; i < nblocks; ++i )
			{
				blocksOut[i] = allocateChunk(nbytes, HK_MEMORY_CLASS_ROOT);
			}
		}
		virtual void deallocateChunkBatch(void** blocks, int nblocks, int nbytes )
		{
			for( int i = 0; i < nblocks; ++i )
			{
				deallocateChunk(blocks[i], nbytes, HK_MEMORY_CLASS_ROOT);
			}
		}

		hkDebugMemory( OutputStringFunc outFunc = defaultOutputStringFunc, void* funcArg = HK_NULL);

		~hkDebugMemory();

		virtual void* allocateChunk(int nbytes, HK_MEMORY_CLASS cl);

		virtual void deallocateChunk(void*, int nbytes, HK_MEMORY_CLASS );

		virtual void preAllocateRuntimeBlock(int nbytes, HK_MEMORY_CLASS cl);

		virtual void* allocateRuntimeBlock(int nbytes, HK_MEMORY_CLASS cl);

		virtual void deallocateRuntimeBlock(void*, int nbytes, HK_MEMORY_CLASS cl);

		virtual void provideRuntimeBlock(void*, int nbytes, HK_MEMORY_CLASS cl);

		virtual void freeRuntimeBlocks();

		virtual void printStatistics(hkOstream* c);

		virtual hkBool isOk() const;

	protected:

		void printMemoryReport();

		virtual void* internalAllocate(int nbytes, HK_MEMORY_CLASS, int flags, int alignment=16);

		virtual void internalFree(void* p, int bytesToFree, int flags);

		void outputString(const char* s)
		{
			(*m_outputStringFunc)(s, m_outputStringArg);
		}

	public:

		enum { MAX_STACKTRACE = 10 };
		enum { MEMORY_PADDING = 16 };
		enum MemFlags
		{
			MEM_DEFAULT = 0,
			MEM_CHUNK = 1,
			MEM_ALIGNED = 2
		};

		struct PointerInfo
		{
			PointerInfo() : realMem(HK_NULL), numStackTrace(0), numBytes(-1), flags(MEM_DEFAULT) { }

			void* realMem;
			hkUlong stackTrace[MAX_STACKTRACE];
			int numStackTrace;
			int numBytes;
			int flags;
		};

	protected:

		std::map<void*, PointerInfo> m_activePointers;
		OutputStringFunc m_outputStringFunc;
		void* m_outputStringArg;

		hkStackTracer m_tracer;
		mutable hkCriticalSection m_section;
};


hkDebugMemory::hkDebugMemory( OutputStringFunc func, void* arg) :
	m_outputStringFunc(func),
	m_outputStringArg(arg),
	m_section(0)
{
}

hkDebugMemory::~hkDebugMemory()
{
	printMemoryReport();
}

void hkDebugMemory::printMemoryReport()
{
	hkCriticalSectionLock lock( &m_section );
	if( m_activePointers.size() != 0)
	{
		std::map<void*,PointerInfo>::iterator i = m_activePointers.begin();
		std::map<void*,PointerInfo>::iterator e = m_activePointers.end();

		outputString("**************************************************************\n");
		outputString("* BEGIN MEMORY LEAK REPORT                                   *\n");
		outputString("**************************************************************\n");

		for( ; i!=e; ++i)
		{
			const PointerInfo& pinfo = i->second;
			char buf[256];
			hkString::snprintf(buf, 256, "\n%i bytes leaked. Data at 0x%p. Stack trace follows:\n", pinfo.numBytes, i->first );
			outputString(buf);
			// skip first two frames - they are always allocateX/internalAllocate
			m_tracer.dumpStackTrace( pinfo.stackTrace + 2, pinfo.numStackTrace - 2, m_outputStringFunc, m_outputStringArg );
		}

		outputString("**************************************************************\n");
		outputString("* END MEMORY LEAK REPORT                                     *\n");
		outputString("**************************************************************\n");
	}
	else
	{
		outputString("**************************************************************\n");
		outputString("* NO HAVOK MEMORY LEAKS FOUND                                *\n");
		outputString("**************************************************************\n");
	}
}

void hkDebugMemory::printStatistics(hkOstream* c)
{
	hkCriticalSectionLock lock( &m_section );
	if(c)
	{
		c->printf("No statistics available\n");
	}
}

static void checkUnderOverrun(void* pfree, const hkDebugMemory::PointerInfo& info)
{
	hkUint8* check = static_cast<hkUint8*>(info.realMem);
	int prepadBytes = (int)hkUlong( static_cast<char*>(pfree) - static_cast<char*>(info.realMem) );

	int i;
	for( i = 0; i < prepadBytes; ++i )
	{
		HK_ASSERT(0x6af0c498, check[i] == hkMemory::MEMORY_FILL_GUARD_AREA);
	}

	check += prepadBytes + info.numBytes;

	for( i = 0; i < hkDebugMemory::MEMORY_PADDING; ++i )
	{
		HK_ASSERT(0x431a98ae, check[i] == hkMemory::MEMORY_FILL_GUARD_AREA);
	}
}

void* hkDebugMemory::internalAllocate(int nbytes, HK_MEMORY_CLASS, int flags, int alignment )
{
	if( nbytes )
	{
		hkCriticalSectionLock lock( &m_section );

		PointerInfo pointerinfo;
		pointerinfo.numStackTrace = m_tracer.getStackTrace(pointerinfo.stackTrace, MAX_STACKTRACE);
		pointerinfo.numBytes = nbytes;
		pointerinfo.flags = flags;

		HK_ASSERT( 0x1be63280, MEMORY_PADDING >= 4 );
		HK_ASSERT( 0x1be63281, (MEMORY_PADDING % sizeof(int)) == 0 );
		HK_ASSERT( 0x1be63282, (alignment % sizeof(int)) == 0 );

		// allocate a little more (MEMORY_PADDING) at each end
		hk_size_t postpad = MEMORY_PADDING;
		// if alignment greater than padding is requested we need to be careful
		hk_size_t prepad = (MEMORY_PADDING >= alignment) ? MEMORY_PADDING : alignment;
		pointerinfo.realMem = hkSystemMalloc( int(nbytes + prepad + postpad), alignment);
		char* realMem = static_cast<char*>(pointerinfo.realMem);

		// scrub memory
		hkString::memSet( realMem, MEMORY_FILL_GUARD_AREA, int(prepad) );
		hkString::memSet( realMem + prepad, MEMORY_FILL_UNINITIALIZED, nbytes );
		hkString::memSet( realMem + prepad + nbytes, MEMORY_FILL_GUARD_AREA, int(postpad) );

		void* memory = realMem + prepad;
		m_activePointers[memory] = pointerinfo;

		m_memoryStatistics.m_numSysAllocs += 1;
		m_memoryStatistics.m_sysAllocsSize += nbytes;
		if( m_memoryStatistics.m_sysAllocsSize > m_memoryStatistics.m_sysAllocsHighMark )
		{
			m_memoryStatistics.m_sysAllocsHighMark = m_memoryStatistics.m_sysAllocsSize;
		}

		return memory;
	}
	else
	{
		return HK_NULL;
	}
}

void hkDebugMemory::internalFree(void* pfree, int bytesToFree, int flags)
{
	hkCriticalSectionLock lock( &m_section );

	std::map<void*,PointerInfo>::iterator i = m_activePointers.find(pfree);
	HK_ASSERT2( 0x356976c1, i != m_activePointers.end(), "Freeing junk pointer");
	const PointerInfo& info = i->second;

	HK_ASSERT2( 0x5861b912, (info.flags & MEM_ALIGNED) == (flags & MEM_ALIGNED), "Mismatched aligned methods");
	HK_ASSERT2( 0x5861b913, (info.flags & MEM_CHUNK) == (flags & MEM_CHUNK), "Mismatched chunk methods");
	if(bytesToFree == -1)
	{
		HK_ASSERT2( 0x5861b911, (flags & MEM_CHUNK) == 0, "Calling deallocate on a chunk");
	}
	else
	{
		HK_ASSERT2( 0x743ce000, info.numBytes == bytesToFree, "Freeing chunk of wrong size");
	}

	checkUnderOverrun(pfree, info);
	{ // scrub area
		int prepadBytes = (int)hkUlong( static_cast<char*>(pfree) - static_cast<char*>(info.realMem) );
		hkString::memSet( info.realMem, MEMORY_FILL_FREE_SPACE, prepadBytes + info.numBytes + MEMORY_PADDING );
	}

	m_memoryStatistics.m_numSysAllocs -= 1;
	m_memoryStatistics.m_sysAllocsSize -= bytesToFree;

	hkSystemFree( info.realMem );
	m_activePointers.erase(i);
}

void* hkDebugMemory::allocateChunk(int nbytes, HK_MEMORY_CLASS c)
{
	return internalAllocate(nbytes, c, MEM_CHUNK);
}

void hkDebugMemory::deallocateChunk(void* p, int nbytes, HK_MEMORY_CLASS cl)
{
	if(p)
	{
		internalFree(p, nbytes, MEM_CHUNK);
	}
}

hkBool hkDebugMemory::isOk() const
{
	hkCriticalSectionLock lock( &m_section );
	std::map<void*,PointerInfo>::const_iterator i = m_activePointers.begin();
	std::map<void*,PointerInfo>::const_iterator e = m_activePointers.end();

	for( ; i!=e; ++i)
	{
		checkUnderOverrun( i->first, i->second );
	}
	return true;
}

#ifdef HK_PLATFORM_WIN32
#	include <hkbase/fwd/hkwindows.h>
void HK_CALL hkDebugMemory::defaultOutputStringFunc( const char* s, void* context)
{
	OutputDebugString(s);
}
#else
#	include <hkbase/fwd/hkcstdio.h>
void HK_CALL hkDebugMemory::defaultOutputStringFunc( const char* s, void* context)
{
	using namespace std;
	printf("%s",s);
}
#endif

void hkDebugMemory::preAllocateRuntimeBlock(int nbytes, HK_MEMORY_CLASS cl)
{
}

void* hkDebugMemory::allocateRuntimeBlock(int nbytes, HK_MEMORY_CLASS cl)
{
	void* memory = allocateChunk(nbytes, cl);
	return memory;
}

void hkDebugMemory::deallocateRuntimeBlock(void* p, int nbytes, HK_MEMORY_CLASS cl)
{
	deallocateChunk(p, nbytes, cl);
}

void hkDebugMemory::provideRuntimeBlock(void*, int nbytes, HK_MEMORY_CLASS cl)
{
}

void hkDebugMemory::freeRuntimeBlocks()
{
}

#endif // HK_DEBUG_MEMORY_H


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
