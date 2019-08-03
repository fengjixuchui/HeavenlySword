//---------------------------------------
//!
//!	\file core\memman.cpp
//! The memory manager the only place to
//! uses OS level memory calls
//!
//---------------------------------------
#include "core/doubleenderframeallocator.h"
#include "core/heap.h"
#include "core/gfxmem.h"
#include "core/memman.h"

// temp hack
#if defined( PLATFORM_PS3 )
#include <Gc/GcInitParams.h>
#include "core/cellfsfile_ps3.h"

//#define MEMMAN_SAVE_STACK_TRACE
#define MEMMAN_STACK_TRACE_CHUNK (Mem::MC_PROCEDURAL)

#endif

// remove this from dev builds if it gets too slow.
#ifdef _HAVE_MEMORY_TRACKING
#define FILL_MEMORY
#endif

#define BYTES_TO_KB( n )	((n + 1023)/1024)
#define BYTES_TO_MB( m )	((BYTES_TO_KB(m) + 1023)/1024)

// This allows us to resize heaps in the debugger if we are having problems shuffling heap sizes
// although it has problems because the mem system is now initialised in static constructors :(
#define MEM_SIZE_DECLARE	 static const 

//---------------------------------------
// Memory Pool sizes. 
//---------------------------------------
#ifdef PLATFORM_PS3
MEM_SIZE_DECLARE unsigned int s_iAudioMemorySize			= Mem::AUDIO_CHUNK_SIZE;	// Audio memory for PS3 build
#else
MEM_SIZE_DECLARE unsigned int s_iAudioMemorySize			= 34 * Mem::Mb;		// Audio memory for PC build
#endif

#if defined( PLATFORM_PC )
#ifdef MEMMAN_SAVE_STACK_TRACE
MEM_SIZE_DECLARE unsigned int s_iDebugMemTracker			= 75 * Mem::Mb;		// enough for lots of allocation track structures
#else
MEM_SIZE_DECLARE unsigned int s_iDebugMemTracker			= 50 * Mem::Mb;		// enough for lots of allocation track structures
#endif
MEM_SIZE_DECLARE unsigned int s_iDebugMemMaxOverflow		= 400 * Mem::Mb;	// how much to allow us to overflow by if we allow any overflow
#else
MEM_SIZE_DECLARE unsigned int s_iDebugMemTracker			= 60 * Mem::Mb;		// enough for lots allocation track structures
MEM_SIZE_DECLARE unsigned int s_iDebugMemMaxOverflow		= 200 * Mem::Mb;	// how much to allow us to overflow by if we allow any overflow
#endif
MEM_SIZE_DECLARE uint32_t s_iDebugBufferSpace				= 512 * Mem::Kb;	// How much debug space to leave regardless of how much overflow is asked for

//---------------------------------------
// Defines
//---------------------------------------
#if defined(_HAVE_MEMORY_TRACKING)
//! TRACK_MEMORY_LEVEL
//! 0 = Dump leaked and double deletes when asked
//! 1 = Log non debug mallocs and free
//! 2 = Log every malloc and free!
#define TRACK_MEMORY_LEVEL 1

//#define BREAK_ON_ALLOC_ADDRESS 0x1D724970

//#define BREAK_ON_FREE_ADDRESS 0x1D724970

#endif

#ifdef MEMMAN_SAVE_STACK_TRACE
namespace
{
	const int maxStackDepth = 32;
	const int ownCallsToSkip = 3;

	//typedef ntstd::Vector<void*, Mem::MC_DEBUG_MEM_TRACKER> StackTraceContainer;
	//typedef void* StackTraceContainer[maxStackDepth];
	struct StackTraceContainer
	{
		StackTraceContainer()
			: refCount_(1)
			, size_(0)
		{
			memset(data_, 0, sizeof(void*) * maxStackDepth);
		}

		void*& operator[] (int i)  
		{
			if (i >= size_)
			{
				size_ = i + 1;
			}
			return data_[i];
		}

		void* const& operator[] (int i) const
		{
			return data_[i];
		}

		size_t size() const
		{
			return size_;
		}


		void* data_[maxStackDepth];
		int refCount_;
		int size_;
	};

	inline void StackTraceAddElement(StackTraceContainer* cont, void* elem, int index)
	{
		//cont -> push_back(elem);
		(*cont)[index] = elem;
	}

	inline size_t StackTraceGetSize(StackTraceContainer const* cont)
	{
		return cont -> size();
	}

	StackTraceContainer* DoStackTrace()
	{
		//StackTraceContainer* stack = NT_NEW_CHUNK(Mem::MC_DEBUG_MEM_TRACKER) StackTraceContainer(maxStackDepth, (void*)0);
		StackTraceContainer* stack = NT_NEW_CHUNK(Mem::MC_DEBUG_MEM_TRACKER) StackTraceContainer;

		volatile uint64_t stackHead;
		asm volatile( ""
			"std %%r1, %0\n"
			: "=m"(stackHead)
			);

		uint64_t* backChain = (uint64_t*)stackHead;

		// skip records of calls to StackTrace, MemLog etc.
		int skipCount = 0;
		do
		{
			backChain = (uint64_t*)(*backChain);
		} while (*backChain && ++skipCount < ownCallsToSkip);

		// save the call history
		int slot = 0;
		while (*backChain && slot < maxStackDepth)
		{
			void* returnAddress = (void*)backChain[2];
			StackTraceAddElement(stack, returnAddress, slot++);
			backChain = (uint64_t*)(*backChain);
		} 

		return stack;
	}

	void DumpStackTrace(StackTraceContainer const* stack)
	{
		if (!stack)	return;

		ntPrintf("---Stack Trace---\n");
		//for (StackTraceContainer::const_iterator iter = stack.begin(); iter != stack.end(); ++ iter)
		for (int i = 0; i < (int)StackTraceGetSize(stack) && (*stack)[i]; ++i)
		{
			ntPrintf("0x%x  ", (*stack)[i]);
		}
		ntPrintf("\n-----------------\n");
	}

}
#endif

// the memory system
namespace Mem
{

#ifndef _GOLD_MASTER
const char* GetChunkName( MEMORY_CHUNK chunk, bool bErrorUnknown = true )
{
	UNUSED( bErrorUnknown );
	ntError_p( (chunk < MC_NUM_CHUNKS) && bErrorUnknown, ("Unrecognised memory chunk %d", chunk) );

	switch (chunk)
	{
		case MC_RSX_MAIN_INTERNAL:	return	"MC_RSX_MAIN_INTERNAL";
		case MC_RSX_MAIN_USER:		return 	"MC_RSX_MAIN_USER";	 

		case MC_GFX: 				return 	"MC_GFX";
		case MC_LOADER: 			return 	"MC_LOADER";
		case MC_ANIMATION: 			return 	"MC_ANIMATION";
		case MC_HAVOK: 				return 	"MC_HAVOK";
		case MC_AUDIO: 				return 	"MC_AUDIO";

#ifndef _COLLAPSE_SMALL_CHUNKS
		case MC_ODB: 				return 	"MC_ODB";

		case MC_ARMY: 				return 	"MC_ARMY";
		case MC_LUA: 				return 	"MC_LUA";
		case MC_ENTITY: 			return 	"MC_ENTITY";
		case MC_MISC: 				return 	"MC_MISC";

		case MC_AI: 				return 	"MC_AI";
		case MC_CAMERA: 			return 	"MC_CAMERA";
		case MC_EFFECTS: 			return 	"MC_EFFECTS";
		case MC_PROCEDURAL: 		return 	"MC_PROCEDURAL";
#endif

		case MC_OVERFLOW:			return	"MC_OVERFLOW";

		#ifdef _HAVE_DEBUG_MEMORY
		case MC_DEBUG:				return "MC_DEBUG";
		#endif
			
		#ifdef _HAVE_MEMORY_TRACKING
		case MC_DEBUG_MEM_TRACKER:	return "MC_DEBUG_MEM_TRACKER";
		#endif

		default:
			ntError_p( bErrorUnknown, ("Unrecognised memory chunk %d", chunk) );
			return "MC_UNKNOWN";
	}
};
#endif // _GOLD_MASTER

//---------------------------------------
// Private classes
//---------------------------------------

class NullAllocator
{
public:
	void Initialise( uintptr_t , uint32_t ){};

	uintptr_t Alloc( uint32_t ){ return 0; }
	uintptr_t Shrink( uintptr_t, uint32_t ) { return 0; }
	void Free( uintptr_t ){ }
	uintptr_t MemAlign( uint32_t , uint32_t){ return 0; }
	bool IsOurs( uintptr_t ){ return false; }

	uint32_t GetCurrentFreeSpace() { return 0; }
	uintptr_t GetBaseAddress() { return 0; } 

};

#if defined(_HAVE_MEMORY_TRACKING)
struct AllocTracker
{
	AllocTracker() 
#ifdef MEMMAN_SAVE_STACK_TRACE
		: m_stackTrace(NULL)
#endif
	{}
	AllocTracker(	const char* pTag, 
					const char* pSubTag, int iNum, 
					uintptr_t pPtr, int iSize, MEMORY_CHUNK chunk, MEMORY_CHUNK oldchunk ) :
		m_pTag(pTag),
		m_pSubTag(pSubTag),
		m_iNum(iNum),
		m_pPtr(pPtr),
		m_iSize(iSize),
		m_eChunk( (uint8_t)chunk ),
		m_eOldChunk( (uint8_t)oldchunk )
#ifdef MEMMAN_SAVE_STACK_TRACE
		, m_stackTrace( MEMMAN_STACK_TRACE_CHUNK == chunk || MEMMAN_STACK_TRACE_CHUNK == oldchunk ? DoStackTrace() : NULL ) 
#endif

	{
	}

#ifdef MEMMAN_SAVE_STACK_TRACE
	AllocTracker(AllocTracker const& lhs)
		:
		m_pTag(lhs.m_pTag),
		m_pSubTag(lhs.m_pSubTag),
		m_iNum(lhs.m_iNum),
		m_pPtr(lhs.m_pPtr),
		m_iSize(lhs.m_iSize),
		m_eChunk( lhs.m_eChunk ),
		m_eOldChunk( lhs.m_eOldChunk ),
		m_stackTrace( lhs.m_stackTrace )		
	{
		if (m_stackTrace) m_stackTrace -> refCount_++;
	}

	AllocTracker& operator= (AllocTracker const& lhs)
	{
		if (&lhs != this)
		{
			DestroyStackTrace();
		}

		m_pTag		= lhs.m_pTag;
		m_pSubTag	= lhs.m_pSubTag;
		m_iNum		= lhs.m_iNum;
		m_pPtr		= lhs.m_pPtr;
		m_iSize		= lhs.m_iSize;
		m_eChunk	= lhs.m_eChunk;
		m_eOldChunk	= lhs.m_eOldChunk;
		m_stackTrace = lhs.m_stackTrace;
		if (m_stackTrace) m_stackTrace -> refCount_++;

		return *this;
	}

	~AllocTracker()
	{
		DestroyStackTrace();
	}

	void DestroyStackTrace()
	{
		if (m_stackTrace && 0 == -- m_stackTrace -> refCount_)
		{
			NT_DELETE_CHUNK(MC_DEBUG_MEM_TRACKER, m_stackTrace);
			m_stackTrace = NULL;
		}
	}
#endif

	const char*		m_pTag;			//!< primary tag (file usually)
    const char*		m_pSubTag;		//!< sub tag (function usually)
	int				m_iNum;			//!< tag num (line no usually)
	uintptr_t		m_pPtr;			//!< address of this allocation (same a Key)
	int				m_iSize;		//!< size of allocation
	uint8_t			m_eChunk;		//<! which chunk this memory is allocated from
	uint8_t			m_eOldChunk;		//<! which chunk this memory was requested from (for overflow tracking)
#ifdef MEMMAN_SAVE_STACK_TRACE
	StackTraceContainer* m_stackTrace;
#endif

	bool operator== ( const AllocTracker& track )
	{
		return( (m_pTag == track.m_pTag) &&
				(m_pSubTag == track.m_pSubTag) &&
				(m_iNum == track.m_iNum) &&
				(m_iSize == track.m_iSize) &&
				(m_pPtr == track.m_pPtr) &&
				(m_eChunk == track.m_eChunk) );
	}
};
typedef ntstd::Map< const uintptr_t,AllocTracker, ntstd::less<const uintptr_t>, Mem::MC_DEBUG_MEM_TRACKER > AllocTrackerMap;

enum LOG_TYPE 
{
	LT_ALLOC,
	LT_MEMALIGN,
	LT_FREE,
	LT_SHRINK
};
void LogTag( const LOG_TYPE type, MEMORY_CHUNK chunk, MEMORY_CHUNK oldchunk, const char* pTag, const char* pSubTag, const int iNum, uintptr_t pPtr, const int iSize, const bool bNoAllocs = false );
#endif

//---------------------------------------
//!
//!
//---------------------------------------
template< class GlobalPolicy, class LevelPolicy, class AreaPolicy >
class MemoryChunk
{
public:
	void Initialise(	uintptr_t pGlobalAddr, 
						uint32_t iGlobalSize,
						uintptr_t pLevelAddr, 
						uint32_t iLevelSize,
						uintptr_t pAreaAddr,
						uint32_t iAreaSize )
	{
		 m_GlobalAllocator.Initialise( pGlobalAddr, iGlobalSize );
		 m_LevelAllocator.Initialise( pLevelAddr, iLevelSize );
		 m_AreaAllocator.Initialise( pAreaAddr, iAreaSize );
#ifdef _HAVE_MEMORY_TRACKING
		 m_pGlobalAddr 	= pGlobalAddr;
		 m_iGlobalSize 	= iGlobalSize;
		 m_iLevelSize	= iLevelSize;
		 m_iAreaSize	= iAreaSize;
#endif	
	}

	//! Alloc
	uintptr_t Alloc( MEMORY_SCOPE scope, uint32_t iSize )
	{
		switch( scope )
		{
		case MS_GLOBAL:
			return m_GlobalAllocator.Alloc( iSize );
		case MS_LEVEL:
			return m_LevelAllocator.Alloc( iSize );
		case MS_AREA:
			return m_AreaAllocator.Alloc( iSize );
		default:
			return 0;
		}
	}
	//! MemAlign
	uintptr_t MemAlign( MEMORY_SCOPE scope, uint32_t iSize, uint32_t iAlign )
	{
		switch( scope )
		{
		case MS_GLOBAL:
			return m_GlobalAllocator.MemAlign( iSize, iAlign );
		case MS_LEVEL:
			return m_LevelAllocator.MemAlign( iSize, iAlign );
		case MS_AREA:
			return m_AreaAllocator.MemAlign( iSize, iAlign );
		default:
			return 0;
		}
	}

	//! Alloc
	uintptr_t Realloc( MEMORY_SCOPE scope, uintptr_t pAlloc, uint32_t iSize )
	{
		switch( scope )
		{
		case MS_GLOBAL:
			return m_GlobalAllocator.Shrink( pAlloc, iSize );
		case MS_LEVEL:
			return m_LevelAllocator.Shrink( pAlloc, iSize );
		case MS_AREA:
			return m_AreaAllocator.Shrink( pAlloc, iSize );
		default:
			return 0;
		}
	}


	//! Free is completely ignored.
	void Free( MEMORY_SCOPE scope, uintptr_t pAlloc )
	{
		switch( scope )
		{
		case MS_GLOBAL:
			ntAssert( m_GlobalAllocator.IsOurs( pAlloc ) );
			return m_GlobalAllocator.Free( pAlloc );
		case MS_LEVEL:
			ntAssert( m_LevelAllocator.IsOurs( pAlloc ) );
			return m_LevelAllocator.Free( pAlloc );
		case MS_AREA:
			ntAssert( m_AreaAllocator.IsOurs( pAlloc ) );
			return m_AreaAllocator.Free( pAlloc );
		}
	}

	//! tells us if the pointer belongs to this heap
	bool IsOurs( MEMORY_SCOPE scope, uintptr_t pAlloc )
	{
		switch( scope )
		{
		case MS_GLOBAL:
			return m_GlobalAllocator.IsOurs( pAlloc );
		case MS_LEVEL:
			return m_LevelAllocator.IsOurs( pAlloc );
		case MS_AREA:
			return m_AreaAllocator.IsOurs( pAlloc );
		default:
			return false;
		}
	}

	uint32_t GetCurrentFreeSpace(MEMORY_SCOPE scope)
	{
		switch( scope )
		{
		case MS_GLOBAL:
			return m_GlobalAllocator.GetCurrentFreeSpace();
		case MS_LEVEL:
			return m_LevelAllocator.GetCurrentFreeSpace();
		case MS_AREA:
			return m_AreaAllocator.GetCurrentFreeSpace();
		default:
			return false;
		}
	}

	uintptr_t GetBaseAddress(MEMORY_SCOPE scope)
	{
		switch( scope )
		{
		case MS_GLOBAL:
			return m_GlobalAllocator.GetBaseAddress();
		case MS_LEVEL:
			return m_LevelAllocator.GetBaseAddress();
		case MS_AREA:
			return m_AreaAllocator.GetBaseAddress();
		default:
			return 0;
		}
	}

#ifdef _HAVE_MEMORY_TRACKING
	uint32_t		GetTotalAllocated( void ) const
	{
		return m_iGlobalSize + m_iLevelSize + m_iAreaSize;
	}
#endif	
			
	GlobalPolicy	m_GlobalAllocator;
	LevelPolicy		m_LevelAllocator;
	AreaPolicy		m_AreaAllocator;
	
#ifdef _HAVE_MEMORY_TRACKING
	// This set is used for debugging memory layout e.g. for full dumps
	uintptr_t		m_pGlobalAddr;
	uint32_t		m_iGlobalSize;
	uint32_t		m_iLevelSize;
	uint32_t		m_iAreaSize;
#endif	
};

//! we allocate a 64K page for our own usage, this is static managed by this structure
struct MemManPage
{
	BaseMemStats				m_BaseMemStats;
	DoubleEnderFrameAllocator	m_XddrMan;
	DoubleEnderFrameAllocator	m_GddrMan;

#ifdef _HAVE_DEBUG_MEMORY
	DoubleEnderFrameAllocator	m_DebugMemMan;
#endif

	MEMORY_SCOPE		m_MemScope;

	MemoryChunk<DoubleEnderFrameAllocator, NullAllocator, NullAllocator > m_RsxMainAllocator;

	MemoryChunk<Heap, NullAllocator, NullAllocator > m_RsxMainUserAllocator;
	MemoryChunk<Heap, NullAllocator, NullAllocator > m_GfxAllocator;
	MemoryChunk<Heap, NullAllocator, NullAllocator > m_LoaderAllocator;
	MemoryChunk<Heap, NullAllocator, NullAllocator > m_AnimationAllocator;
	MemoryChunk<Heap, NullAllocator, NullAllocator > m_HavokAllocator;
	MemoryChunk<Heap, NullAllocator, NullAllocator > m_AudioAllocator;

#ifndef _COLLAPSE_SMALL_CHUNKS
	MemoryChunk<Heap, NullAllocator, NullAllocator > m_ODBAllocator;

	MemoryChunk<Heap, NullAllocator, NullAllocator > m_ArmyAllocator;
	MemoryChunk<Heap, NullAllocator, NullAllocator > m_LuaAllocator;
	MemoryChunk<Heap, NullAllocator, NullAllocator > m_EntityAllocator;
	MemoryChunk<Heap, NullAllocator, NullAllocator > m_MiscAllocator;

	MemoryChunk<Heap, NullAllocator, NullAllocator > m_AIAllocator;
	MemoryChunk<Heap, NullAllocator, NullAllocator > m_CameraAllocator;
	MemoryChunk<Heap, NullAllocator, NullAllocator > m_EffectsAllocator;
	MemoryChunk<Heap, NullAllocator, NullAllocator > m_ProceduralAllocator;
#endif

	MemoryChunk<Heap, NullAllocator, NullAllocator > m_OverflowAllocator;

#ifdef _HAVE_DEBUG_MEMORY
	MemoryChunk<Heap, NullAllocator, NullAllocator > m_DebugAllocator;
#endif

#ifdef _HAVE_MEMORY_TRACKING
	MemoryChunk<Heap, NullAllocator, NullAllocator > m_DebugMemTracker;
#endif

	// set to false by default, any allocator you want to allow overflow set its boolean to true
	bool				m_bAllowOverflow[ MC_NUM_CHUNKS ];

#if defined(_HAVE_MEMORY_TRACKING)
	AllocTrackerMap*	m_pMemTrackerMap;
	CriticalSection		m_TrackerCritSec;
	bool				m_bEnableMemoryTrackingLog;
	bool				m_MemTrackerInitOK;

	uintptr_t			m_initialMemCheckpoint;
#endif

	MemStats			m_TotalStats;
	MemStats			m_FrameStats;

	// some functions to allow clearer mem queries
	int32_t TotalInUse() const { return m_TotalStats.sTotal.iNumBytesInUse + m_FrameStats.sTotal.iNumBytesInUse; }

	int32_t UsedInChunk( MEMORY_CHUNK chunk )		const { return m_TotalStats.sChunks[chunk].iNumBytesInUse + m_FrameStats.sChunks[chunk].iNumBytesInUse; }
	int32_t OverflowInChunk( MEMORY_CHUNK chunk )	const { return m_TotalStats.sChunks[chunk].iNumBytesOverflowInUse + m_FrameStats.sChunks[chunk].iNumBytesOverflowInUse; }
	int32_t TotalInChunk( MEMORY_CHUNK chunk )		const { return UsedInChunk(chunk) + OverflowInChunk(chunk); }
	int32_t MaxInChunk( MEMORY_CHUNK chunk )		const 
	{
		int32_t frameMax = m_FrameStats.sChunks[chunk].iNumBytesInUseHighWaterMark +
							m_FrameStats.sChunks[chunk].iNumBytesOverflowHighWaterMark;
		int32_t totalMax = m_TotalStats.sChunks[chunk].iNumBytesInUseHighWaterMark +
							m_TotalStats.sChunks[chunk].iNumBytesOverflowHighWaterMark;
		return totalMax + frameMax;
	}
};

//---------------------------------------
// Static data
//---------------------------------------
static MemManPage* s_pMemManPage = 0;

//#define DUMP_TO_EXCEL
const char EXCEL_SEPARATOR	= '&';

#ifdef _HAVE_MEMORY_TRACKING

#ifndef DUMP_TO_EXCEL

// Macro to encapsulate outputting some rough stats. Helpful when you are working out which
// chunks you might be able to shuffle around...
template < class Allocator >
uint32_t DUMP_SIMPLE_ALLOCATOR_STATS( MEMORY_CHUNK chunk, const Allocator &allocator )
{
	uint32_t used = s_pMemManPage->TotalInChunk(chunk);
	uint32_t max = s_pMemManPage->MaxInChunk(chunk);
	uint32_t total_allocated = allocator.GetTotalAllocated();
	float percent_allocated = (_R(used) * 100.0f) / _R(total_allocated);

	ntPrintf( "%20s: %10dK %10dK %10dK %10.2f\n",
				GetChunkName( chunk ),
				BYTES_TO_KB( used ),
				BYTES_TO_KB( max ),
				BYTES_TO_KB( total_allocated ),
				percent_allocated );

	// return used without overflow, as will count the overflow chunk explicitly
	return s_pMemManPage->UsedInChunk(chunk);
}
				
#else

// Macro to encapsulate outputting some rough stats. Helpful when you are working out which
// chunks you might be able to shuffle around...
template < class Allocator >
uint32_t DUMP_SIMPLE_ALLOCATOR_STATS( chunk, const Allocator &allocator )
{
	uint64_t iUsed = s_pMemManPage->UsedInChunk(chunk);
	uint64_t iOverflow = s_pMemManPage->OverflowInChunk(chunk);

	uint64_t iUsedMax( s_pMemManPage->m_TotalStats.sChunks[chunk].iNumBytesInUseHighWaterMark );
	uint64_t iOverflowMax( s_pMemManPage->m_TotalStats.sChunks[chunk].iNumBytesOverflowHighWaterMark );

	uint64_t iLeftOver(allocator.GetTotalAllocated() - iUsedMax);
	ntPrintf( "%s %c %d %c %d %c %d %c %d %c %d\n",
				GetChunkName( chunk ),
				EXCEL_SEPARATOR,
				BYTES_TO_KB( iUsed ),
				EXCEL_SEPARATOR,
				BYTES_TO_KB( iUsedMax ),
				EXCEL_SEPARATOR,
				BYTES_TO_KB( iOverflow ),
				EXCEL_SEPARATOR,
				BYTES_TO_KB( iOverflowMax ),
				EXCEL_SEPARATOR,
				BYTES_TO_KB( iLeftOver ),
				 );

	// return used without overflow, as will count the overflow chunk explicitly
	return iUsed;
}

#endif

void DumpSimpleChunkStats()
{
	ntPrintf( "\n\nCHUNK USAGE STATISTICS\n\n" );

#ifdef DUMP_TO_EXCEL
	ntPrintf("Chunk %c Used %c UsedMax %c Overflow %c OverflowMax %c\n", EXCEL_SEPARATOR, EXCEL_SEPARATOR, EXCEL_SEPARATOR, EXCEL_SEPARATOR, EXCEL_SEPARATOR);
#endif

	// Note that these stats can be misleading as they don't account for fragmentation or overhead
	// (in particular the ODB chunk is very wasteful)

	int iTotalUsed = 0;

	//		   12345678901234567890: 1234567890: 1234567890: 1234567890: 1234567890
	ntPrintf( "               Chunk:    Current:        Max:      Total:          %%\n" );

	DUMP_SIMPLE_ALLOCATOR_STATS( Mem::MC_RSX_MAIN_INTERNAL , s_pMemManPage->m_RsxMainAllocator 	 	);
	DUMP_SIMPLE_ALLOCATOR_STATS( Mem::MC_RSX_MAIN_USER     , s_pMemManPage->m_RsxMainUserAllocator	); 
	
	DUMP_SIMPLE_ALLOCATOR_STATS( Mem::MC_GFX			   , s_pMemManPage->m_GfxAllocator			);
	DUMP_SIMPLE_ALLOCATOR_STATS( Mem::MC_LOADER            , s_pMemManPage->m_LoaderAllocator 	 	);
	DUMP_SIMPLE_ALLOCATOR_STATS( Mem::MC_ANIMATION         , s_pMemManPage->m_AnimationAllocator	);
	DUMP_SIMPLE_ALLOCATOR_STATS( Mem::MC_HAVOK			   , s_pMemManPage->m_HavokAllocator		);
	DUMP_SIMPLE_ALLOCATOR_STATS( Mem::MC_AUDIO             , s_pMemManPage->m_AudioAllocator		);

	// these ones we count the whole pool
	iTotalUsed += s_pMemManPage->m_RsxMainAllocator.GetTotalAllocated();

	iTotalUsed += s_pMemManPage->m_GfxAllocator.GetTotalAllocated();
	iTotalUsed += s_pMemManPage->m_LoaderAllocator.GetTotalAllocated();
	iTotalUsed += s_pMemManPage->m_AnimationAllocator.GetTotalAllocated();
	iTotalUsed += s_pMemManPage->m_HavokAllocator.GetTotalAllocated();
	iTotalUsed += s_pMemManPage->m_AudioAllocator.GetTotalAllocated();

	int iTotalAlloced = iTotalUsed;

	// these ones we add up how much we used, as they should collapse into the overflow chunk
#ifndef _COLLAPSE_SMALL_CHUNKS
	iTotalUsed += DUMP_SIMPLE_ALLOCATOR_STATS( Mem::MC_ODB               , s_pMemManPage->m_ODBAllocator 	 	 	);
	
	iTotalUsed += DUMP_SIMPLE_ALLOCATOR_STATS( Mem::MC_ARMY              , s_pMemManPage->m_ArmyAllocator 			);
	iTotalUsed += DUMP_SIMPLE_ALLOCATOR_STATS( Mem::MC_LUA               , s_pMemManPage->m_LuaAllocator 	 	 	);
	iTotalUsed += DUMP_SIMPLE_ALLOCATOR_STATS( Mem::MC_ENTITY            , s_pMemManPage->m_EntityAllocator 	 	);
	iTotalUsed += DUMP_SIMPLE_ALLOCATOR_STATS( Mem::MC_MISC              , s_pMemManPage->m_MiscAllocator 			);
	
	iTotalUsed += DUMP_SIMPLE_ALLOCATOR_STATS( Mem::MC_AI                , s_pMemManPage->m_AIAllocator 	 	 	);
	iTotalUsed += DUMP_SIMPLE_ALLOCATOR_STATS( Mem::MC_CAMERA            , s_pMemManPage->m_CameraAllocator 	 	);
	iTotalUsed += DUMP_SIMPLE_ALLOCATOR_STATS( Mem::MC_EFFECTS			 , s_pMemManPage->m_EffectsAllocator	 	);
	iTotalUsed += DUMP_SIMPLE_ALLOCATOR_STATS( Mem::MC_PROCEDURAL        , s_pMemManPage->m_ProceduralAllocator  	);
	
	iTotalAlloced += s_pMemManPage->m_ODBAllocator.GetTotalAllocated();

	iTotalAlloced += s_pMemManPage->m_ArmyAllocator.GetTotalAllocated();
	iTotalAlloced += s_pMemManPage->m_LuaAllocator.GetTotalAllocated();
	iTotalAlloced += s_pMemManPage->m_EntityAllocator.GetTotalAllocated();
	iTotalAlloced += s_pMemManPage->m_MiscAllocator.GetTotalAllocated();
	
	iTotalAlloced += s_pMemManPage->m_AIAllocator.GetTotalAllocated();
	iTotalAlloced += s_pMemManPage->m_CameraAllocator.GetTotalAllocated();
	iTotalAlloced += s_pMemManPage->m_EffectsAllocator.GetTotalAllocated();
	iTotalAlloced += s_pMemManPage->m_ProceduralAllocator.GetTotalAllocated();
#endif

	// any overflow from any of the above chunks will show up here
	iTotalUsed += DUMP_SIMPLE_ALLOCATOR_STATS( Mem::MC_OVERFLOW			 , s_pMemManPage->m_OverflowAllocator		);
	iTotalAlloced += s_pMemManPage->m_OverflowAllocator.GetTotalAllocated();
	float percent_allocated = (_R(iTotalUsed) * 100.0f) / _R(iTotalAlloced);

	ntPrintf( "\n" );
	
	//		   12345678901234567890: 1234567890: 1234567890: 1234567890: 1234567890
	ntPrintf( "                      %10dMb             %9dMb %9.2f\n",
				BYTES_TO_MB( iTotalUsed ),
				BYTES_TO_MB( iTotalAlloced ),
				percent_allocated );
}
			
#endif

namespace
{
	static uintptr_t GcXddrPtr = 0;
}

//---------------------------------------
//!
//! Get this party started. This allocates a 'special'
//! 64K page that is used to track its own stuff 
//! its also calculates how much RAM the exe has taken
//! Any remaining RAM is stuck as Debug RAM
//! Note: the calculation adds a 'fudge' factor in
//! non-release for the main XDDR alloc cos its likely the 
//! code segment and statics will be bigger in Debug than Release
//!
//---------------------------------------
#define DUMP_ALLOCS

#ifdef DUMP_ALLOCS
#define RECORD_ALLOC( size, string )													\
	{																					\
		iAllocated += size; iXddrSize -= size;													\
		char buffer[MAX_PATH];																\
		sprintf( buffer, "## Memman: %20s: %5dMb, iAllocated: %5dMb, iXDDRSize: %5dMb\n",			\
			string, BYTES_TO_MB(size), BYTES_TO_MB(iAllocated), BYTES_TO_MB(iXddrSize) );		\
		Debug::AlwaysOutputString( buffer );													\
	}
#endif

void Init()
{
	// cos currently the OS calls alloc early one...
	if(s_pMemManPage )
		return;

	static_assert( sizeof(MemManPage) < 0xFFFF, MemManPage_TooBig );

	uintptr_t pXddrMem;
	uint32_t iXddrSize;
	uintptr_t pGddrMem;
	uint32_t iGddrSize;
	uintptr_t pDebugMem;
	uint32_t iDebugSize;

	InitOsMem(	&pXddrMem, &iXddrSize, 
				&pGddrMem, &iGddrSize, 
				&pDebugMem, &iDebugSize );

#ifdef DUMP_ALLOCS
	uint32_t iAllocated = 0;
	char buffer[MAX_PATH];
	sprintf( buffer, "## Memman: InitialXDDR: %dMb Used By Self: %dMb\n", iXddrSize / Mem::Mb, Mem::GetBaseMemStats().iInitialUsed / Mem::Mb );
	Debug::AlwaysOutputString( buffer );
#endif

	// memman page now starts right at beginning of the non RSX xddr ram
	// this is due to RSX requiring megabyte alignment!
	s_pMemManPage = (MemManPage*) (pXddrMem + Mem::RSX_ADDRESSABLE_SIZE);
	memset(s_pMemManPage, 0, sizeof(MemManPage) );

	// now set up the high level frame allocators that manage the big
	// chunk allocs
	s_pMemManPage->m_XddrMan.Initialise( pXddrMem, iXddrSize );

#ifdef _HAVE_DEBUG_MEMORY
	s_pMemManPage->m_DebugMemMan.Initialise( pDebugMem, iDebugSize );
#endif

	s_pMemManPage->m_MemScope = MS_GLOBAL;

	//-=-=-=-=-=-=-=-=-=-
	// we need to allocate ourself and rsx main from our xddr frame allocator (bizarre I know)
	s_pMemManPage->m_XddrMan.Alloc( Mem::RSX_ADDRESSABLE_SIZE );
	RECORD_ALLOC( Mem::RSX_ADDRESSABLE_SIZE, "RSX alloc" );

	// allocate the mem man page itself.
	s_pMemManPage->m_XddrMan.Alloc( 64 * Kb );
	RECORD_ALLOC( 64 * Kb, "Mem man page" )

#ifdef _HAVE_DEBUG_MEMORY
	//=-=-=-=-=-=-=-=-=-=-=-= DEBUG RAM -=-=-=-=-=-=-=-=-=-=-=-=-
	// allocate the main chunk of debug ram
	s_pMemManPage->m_DebugAllocator.Initialise(	s_pMemManPage->m_DebugMemMan.Alloc(iDebugSize),
												iDebugSize,
												0,0,
												0,0 );
	s_pMemManPage->m_TotalStats.sChunks[ Mem::MC_DEBUG ].iOriginalAllocationSize = iDebugSize;
#endif

#ifdef _HAVE_MEMORY_TRACKING
	iDebugSize -= s_iDebugMemTracker;
	// sub allocate mem tracker memory
	s_pMemManPage->m_DebugMemTracker.Initialise(	s_pMemManPage->m_DebugAllocator.Alloc(MS_GLOBAL, s_iDebugMemTracker),
													s_iDebugMemTracker,
													0,0,
													0,0 );
	s_pMemManPage->m_TotalStats.sChunks[ Mem::MC_DEBUG_MEM_TRACKER ].iOriginalAllocationSize = s_iDebugMemTracker;

	FrameReset();
	s_pMemManPage->m_pMemTrackerMap = (AllocTrackerMap*) s_pMemManPage->m_DebugMemTracker.Alloc( MS_GLOBAL, sizeof(AllocTrackerMap) );
	NT_PLACEMENT_NEW(s_pMemManPage->m_pMemTrackerMap) AllocTrackerMap();
	s_pMemManPage->m_bEnableMemoryTrackingLog = false;
	s_pMemManPage->m_MemTrackerInitOK = true;
	s_pMemManPage->m_TrackerCritSec.Initialise();
	s_pMemManPage->m_initialMemCheckpoint = TakeMemoryCheckpoint();
#endif

	// MC_RSX_MAIN_INTERNAL
	//------------------------------------------------------------------------------------
	s_pMemManPage->m_RsxMainAllocator.Initialise(	pXddrMem,
													Mem::RSX_ADDRESSABLE_SIZE,
													0, 0,
													0, 0 );
	s_pMemManPage->m_TotalStats.sChunks[ Mem::MC_RSX_MAIN_INTERNAL ].iOriginalAllocationSize = Mem::RSX_ADDRESSABLE_SIZE;

	#if defined( PLATFORM_PS3 )
	static_assert( Mem::GcMem::ICE_RAM_SIZE == (uint32_t)GcInitParams::kDriverHostMemSize, IceDriverHostMemSize_Has_Changed );
	#endif

	// the Gc/Ice allocator has 0x100 alignment implicit so we match it here at the moment
	GcXddrPtr = NT_MEMALIGN_CHUNK(MC_RSX_MAIN_INTERNAL, Mem::GcMem::GC_XDDR_RAM, 0x100);

	// MC_RSX_MAIN_USER
	//------------------------------------------------------------------------------------
	s_pMemManPage->m_RsxMainUserAllocator.Initialise(	NT_MEMALIGN_CHUNK(MC_RSX_MAIN_INTERNAL, Mem::RSX_MAIN_USER_SIZE, 0x100),
														Mem::RSX_MAIN_USER_SIZE,
														0, 0,
														0, 0 );
	s_pMemManPage->m_TotalStats.sChunks[ Mem::MC_RSX_MAIN_USER ].iOriginalAllocationSize = Mem::RSX_MAIN_USER_SIZE;

	// MC_GFX
	//------------------------------------------------------------------------------------
	s_pMemManPage->m_GfxAllocator.Initialise(		s_pMemManPage->m_XddrMan.Alloc( Mem::GFX_CHUNK_SIZE ),
													Mem::GFX_CHUNK_SIZE,
													0, 0,
													0, 0 );
	s_pMemManPage->m_TotalStats.sChunks[ Mem::MC_GFX ].iOriginalAllocationSize = Mem::GFX_CHUNK_SIZE;
	s_pMemManPage->m_bAllowOverflow[ Mem::MC_GFX ] = true;
	RECORD_ALLOC( Mem::GFX_CHUNK_SIZE, "Mem::MC_GFX" )

	// MC_LOADER
	//------------------------------------------------------------------------------------
	s_pMemManPage->m_LoaderAllocator.Initialise(	s_pMemManPage->m_XddrMan.Alloc( Mem::LOADER_CHUNK_SIZE ),
													Mem::LOADER_CHUNK_SIZE,
													0, 0,
													0, 0 );
	s_pMemManPage->m_TotalStats.sChunks[ Mem::MC_LOADER ].iOriginalAllocationSize = Mem::LOADER_CHUNK_SIZE;
	RECORD_ALLOC( Mem::LOADER_CHUNK_SIZE, "Mem::MC_LOADER" )

	// MC_ANIMATION
	//------------------------------------------------------------------------------------
	s_pMemManPage->m_AnimationAllocator.Initialise(	s_pMemManPage->m_XddrMan.Alloc( Mem::ANIM_CHUNK_SIZE ),
													Mem::ANIM_CHUNK_SIZE,
													0, 0,
													0, 0 );
	s_pMemManPage->m_TotalStats.sChunks[ Mem::MC_ANIMATION ].iOriginalAllocationSize = Mem::ANIM_CHUNK_SIZE;
	s_pMemManPage->m_bAllowOverflow[ Mem::MC_ANIMATION ] = true;
	RECORD_ALLOC( Mem::ANIM_CHUNK_SIZE, "Mem::MC_ANIMATION" )

	// MC_AUDIO
	//------------------------------------------------------------------------------------
	s_pMemManPage->m_AudioAllocator.Initialise(		s_pMemManPage->m_XddrMan.Alloc(s_iAudioMemorySize),
													s_iAudioMemorySize,
													0, 0,
													0, 0 );
	s_pMemManPage->m_TotalStats.sChunks[ Mem::MC_AUDIO ].iOriginalAllocationSize = s_iAudioMemorySize;
	RECORD_ALLOC( s_iAudioMemorySize, "Mem::MC_AUDIO" )

#ifndef _COLLAPSE_SMALL_CHUNKS

	// MC_ODB
	//------------------------------------------------------------------------------------
	s_pMemManPage->m_ODBAllocator.Initialise(		s_pMemManPage->m_XddrMan.Alloc( Mem::OBD_CHUNK_SIZE ),
													Mem::OBD_CHUNK_SIZE,
													0, 0,
													0, 0 );
	s_pMemManPage->m_TotalStats.sChunks[ Mem::MC_ODB ].iOriginalAllocationSize = Mem::OBD_CHUNK_SIZE;
	s_pMemManPage->m_bAllowOverflow[ Mem::MC_ODB ] = true;
	RECORD_ALLOC( Mem::OBD_CHUNK_SIZE, "Mem::MC_ODB" )

	// MC_ARMY
	//------------------------------------------------------------------------------------
	s_pMemManPage->m_ArmyAllocator.Initialise(		s_pMemManPage->m_XddrMan.Alloc( Mem::ARMY_CHUNK_SIZE ),
													Mem::ARMY_CHUNK_SIZE,
													0, 0,
													0, 0 );
	s_pMemManPage->m_TotalStats.sChunks[ Mem::MC_ARMY ].iOriginalAllocationSize = Mem::ARMY_CHUNK_SIZE;
	s_pMemManPage->m_bAllowOverflow[ Mem::MC_ARMY ] = true;
	RECORD_ALLOC( Mem::ARMY_CHUNK_SIZE, "Mem::MC_ARMY" )

	// MC_LUA
	//------------------------------------------------------------------------------------
	s_pMemManPage->m_LuaAllocator.Initialise(		s_pMemManPage->m_XddrMan.Alloc( Mem::LUA_CHUNK_SIZE ),
													Mem::LUA_CHUNK_SIZE,
													0, 0,
													0, 0 );
	s_pMemManPage->m_TotalStats.sChunks[ Mem::MC_LUA ].iOriginalAllocationSize = Mem::LUA_CHUNK_SIZE;
	s_pMemManPage->m_bAllowOverflow[ Mem::MC_LUA ] = true;
	RECORD_ALLOC( Mem::LUA_CHUNK_SIZE, "Mem::MC_LUA" )

	// MC_ENTITY
	//------------------------------------------------------------------------------------
	s_pMemManPage->m_EntityAllocator.Initialise(	s_pMemManPage->m_XddrMan.Alloc( Mem::ENTITY_CHUNK_SIZE ),
													Mem::ENTITY_CHUNK_SIZE,
													0, 0,
													0, 0 );
	s_pMemManPage->m_TotalStats.sChunks[ Mem::MC_ENTITY ].iOriginalAllocationSize = Mem::ENTITY_CHUNK_SIZE;
	s_pMemManPage->m_bAllowOverflow[ Mem::MC_ENTITY ] = true;
	RECORD_ALLOC( Mem::ENTITY_CHUNK_SIZE, "Mem::MC_ENTITY" )

	// MC_MISC
	//------------------------------------------------------------------------------------
	s_pMemManPage->m_MiscAllocator.Initialise(		s_pMemManPage->m_XddrMan.Alloc( Mem::MISC_CHUNK_SIZE ),
													Mem::MISC_CHUNK_SIZE,
													0, 0,
													0, 0 );
	s_pMemManPage->m_TotalStats.sChunks[ Mem::MC_MISC ].iOriginalAllocationSize = Mem::MISC_CHUNK_SIZE;
	s_pMemManPage->m_bAllowOverflow[ Mem::MC_MISC ] = true;
	RECORD_ALLOC( Mem::MISC_CHUNK_SIZE, "Mem::MC_MISC" )

	// MC_AI
	//------------------------------------------------------------------------------------
	s_pMemManPage->m_AIAllocator.Initialise(		s_pMemManPage->m_XddrMan.Alloc( Mem::AI_CHUNK_SIZE ),
													Mem::AI_CHUNK_SIZE,
													0, 0,
													0, 0 );
	s_pMemManPage->m_TotalStats.sChunks[ Mem::MC_AI ].iOriginalAllocationSize = Mem::AI_CHUNK_SIZE;
	s_pMemManPage->m_bAllowOverflow[ Mem::MC_AI ] = true;
	RECORD_ALLOC( Mem::AI_CHUNK_SIZE, "Mem::MC_AI" )

	// MC_CAMERA
	//------------------------------------------------------------------------------------
	s_pMemManPage->m_CameraAllocator.Initialise(	s_pMemManPage->m_XddrMan.Alloc( Mem::CAMERA_CHUNK_SIZE ),
													Mem::CAMERA_CHUNK_SIZE,
													0, 0,
													0, 0 );
	s_pMemManPage->m_TotalStats.sChunks[ Mem::MC_CAMERA ].iOriginalAllocationSize = Mem::CAMERA_CHUNK_SIZE;
	s_pMemManPage->m_bAllowOverflow[ Mem::MC_CAMERA ] = true;
	RECORD_ALLOC( Mem::CAMERA_CHUNK_SIZE, "Mem::MC_CAMERA" )
	
	// MC_EFFECTS
	//------------------------------------------------------------------------------------
	s_pMemManPage->m_EffectsAllocator.Initialise(	s_pMemManPage->m_XddrMan.Alloc( Mem::EFFECT_CHUNK_SIZE ),
													Mem::EFFECT_CHUNK_SIZE,
													0, 0,
													0, 0 );
	s_pMemManPage->m_TotalStats.sChunks[ Mem::MC_EFFECTS ].iOriginalAllocationSize = Mem::EFFECT_CHUNK_SIZE;
	s_pMemManPage->m_bAllowOverflow[ Mem::MC_EFFECTS ] = true;
	RECORD_ALLOC( Mem::EFFECT_CHUNK_SIZE, "Mem::MC_EFFECTS" )

	// MC_PROCEDURAL
	//------------------------------------------------------------------------------------
	s_pMemManPage->m_ProceduralAllocator.Initialise(s_pMemManPage->m_XddrMan.Alloc( Mem::PROC_CHUNK_SIZE ),
													Mem::PROC_CHUNK_SIZE,
													0, 0,
													0, 0 );
	s_pMemManPage->m_TotalStats.sChunks[ Mem::MC_PROCEDURAL ].iOriginalAllocationSize = Mem::PROC_CHUNK_SIZE;
	s_pMemManPage->m_bAllowOverflow[ Mem::MC_PROCEDURAL ] = true;
	RECORD_ALLOC( Mem::PROC_CHUNK_SIZE, "Mem::MC_PROCEDURAL" )

#endif // _COLLAPSE_SMALL_CHUNKS

	// MC_OVERFLOW
	//------------------------------------------------------------------------------------
	ntPrintf( "Current free XDDR space after fixed-size pools: %dK\n", BYTES_TO_KB( s_pMemManPage->m_XddrMan.GetCurrentFreeSpace() ) );

#if defined (_HAVE_DEBUG_OVERFLOW) && defined (PLATFORM_PS3)

	// we have oodles of debug memory, use that for the overflow pool

	uint32_t overflowSize;
	if( (s_iDebugMemMaxOverflow + s_iDebugBufferSpace) > iDebugSize )
		overflowSize = iDebugSize - s_iDebugBufferSpace;
	else
		overflowSize = s_iDebugMemMaxOverflow;

	s_pMemManPage->m_OverflowAllocator.Initialise(	s_pMemManPage->m_DebugAllocator.Alloc(MS_GLOBAL, overflowSize),
													overflowSize,
													0,0,
													0,0 );
	s_pMemManPage->m_TotalStats.sChunks[ Mem::MC_OVERFLOW ].iOriginalAllocationSize = overflowSize;

	// we need to max out the rest of our main XDDR to move havok to the end of our valid MMU pages
	// see below comment

	ntError_p( s_pMemManPage->m_XddrMan.GetCurrentFreeSpace() > Mem::HAVOK_CHUNK_SIZE, ( "can't get space for final memory chunk -- memory is over-allocated" ) );
	uint32_t iWasteSize = s_pMemManPage->m_XddrMan.GetCurrentFreeSpace() - Mem::HAVOK_CHUNK_SIZE;

	s_pMemManPage->m_XddrMan.Alloc( iWasteSize );
	RECORD_ALLOC( iWasteSize, "Wastage" )

#else

	// our overflow pool is just what is left in XDDR after all the others have taken a bite

	ntError_p( s_pMemManPage->m_XddrMan.GetCurrentFreeSpace() > Mem::HAVOK_CHUNK_SIZE, ( "can't get space for final memory chunk -- memory is over-allocated" ) );
	uint32_t overflowSize = s_pMemManPage->m_XddrMan.GetCurrentFreeSpace() - Mem::HAVOK_CHUNK_SIZE;

	s_pMemManPage->m_OverflowAllocator.Initialise(	s_pMemManPage->m_XddrMan.Alloc(overflowSize),
													overflowSize,
													0, 0,
													0, 0 );
	s_pMemManPage->m_TotalStats.sChunks[ Mem::MC_OVERFLOW ].iOriginalAllocationSize = overflowSize;
	RECORD_ALLOC( overflowSize, "Mem::MC_OVERFLOW" )

#endif // _HAVE_DEBUG_OVERFLOW

	// MC_HAVOK
	//------------------------------------------------------------------------------------

	// Havok's memory is constrained to here
	s_pMemManPage->m_HavokAllocator.Initialise(		s_pMemManPage->m_XddrMan.Alloc( Mem::HAVOK_CHUNK_SIZE ),
													Mem::HAVOK_CHUNK_SIZE,
													0, 0,
													0, 0 );
	s_pMemManPage->m_TotalStats.sChunks[ Mem::MC_HAVOK ].iOriginalAllocationSize = Mem::HAVOK_CHUNK_SIZE;
	s_pMemManPage->m_bAllowOverflow[ Mem::MC_HAVOK ] = true;
	RECORD_ALLOC( Mem::HAVOK_CHUNK_SIZE, "Mem::MC_HAVOK" )

	// Show some initial stats to demonstrate how big our pools are
#ifdef _HAVE_MEMORY_TRACKING
#ifdef PLATFORM_PS3
	DumpSimpleChunkStats();
#endif
#endif

}

//---------------------------------------
//!
//! Kill 
//!
//---------------------------------------
void Kill()
{
	if(s_pMemManPage == 0)
		return;

	// release our sub allocation
	NT_FREE_CHUNK( MC_RSX_MAIN_INTERNAL, GetBaseAddress(MC_RSX_MAIN_USER) );
	NT_FREE_CHUNK( MC_RSX_MAIN_INTERNAL, GcXddrPtr );

#ifdef _HAVE_MEMORY_TRACKING
	DumpMemoryCheckpointDifference( s_pMemManPage->m_initialMemCheckpoint, "Final Shutdown Mem Checkpoint" );
	FreeMemoryCheckpoint( s_pMemManPage->m_initialMemCheckpoint );

	s_pMemManPage->m_DebugMemTracker.Free( MS_GLOBAL, (uintptr_t) s_pMemManPage->m_pMemTrackerMap );
	s_pMemManPage->m_TrackerCritSec.Kill();
#endif

	uintptr_t debug_mem_base_addr = 0;

#ifdef _HAVE_DEBUG_MEMORY
	debug_mem_base_addr = s_pMemManPage->m_DebugMemMan.GetBaseAddress();
#endif

	FreeOsMem(	s_pMemManPage->m_XddrMan.GetBaseAddress(), 
				0, // GDDR not managed yet 
				debug_mem_base_addr );

	s_pMemManPage = 0;
}

//---------------------------------------
//!
//! Does the memory already exist?
//!
//---------------------------------------
bool ManagerExists()
{
	return (s_pMemManPage != 0);
}

void SumFrameResetResults( const MemSubStats& frame, MemSubStats& out )
{
	out.iNumAllocCalls += frame.iNumAllocCalls;
	out.iNumFreeCalls += frame.iNumFreeCalls;
	out.iNumMemAlignCalls += frame.iNumMemAlignCalls;

#ifdef _HAVE_MEMORY_TRACKING
	out.iNumAllocedBytes += frame.iNumAllocedBytes;
	out.iNumFreedBytes += frame.iNumFreedBytes;
	out.iNumMemAlignedBytes += frame.iNumMemAlignedBytes;

	out.iNumBytesInUseHighWaterMark = ntstd::Max( out.iNumBytesInUseHighWaterMark, frame.iNumBytesInUseHighWaterMark + out.iNumBytesInUse );
	out.iNumBytesOverflowHighWaterMark = ntstd::Max( out.iNumBytesOverflowHighWaterMark, frame.iNumBytesOverflowHighWaterMark + out.iNumBytesOverflowInUse );
	
	out.iNumBytesInUse += frame.iNumBytesInUse;
	out.iNumBytesOverflowInUse += frame.iNumBytesOverflowInUse;
#endif

}

void FrameReset()
{
	ntAssert( ManagerExists() );

#ifdef _HAVE_MEMORY_TRACKING
	SumFrameResetResults( s_pMemManPage->m_FrameStats.sTotal, s_pMemManPage->m_TotalStats.sTotal );

	SumFrameResetResults( s_pMemManPage->m_FrameStats.sChunks[MC_RSX_MAIN_INTERNAL], s_pMemManPage->m_TotalStats.sChunks[MC_RSX_MAIN_INTERNAL] );
	SumFrameResetResults( s_pMemManPage->m_FrameStats.sChunks[MC_RSX_MAIN_USER], s_pMemManPage->m_TotalStats.sChunks[MC_RSX_MAIN_USER] );

	SumFrameResetResults( s_pMemManPage->m_FrameStats.sChunks[MC_GFX], s_pMemManPage->m_TotalStats.sChunks[MC_GFX] );
	SumFrameResetResults( s_pMemManPage->m_FrameStats.sChunks[MC_LOADER], s_pMemManPage->m_TotalStats.sChunks[MC_LOADER] );
	SumFrameResetResults( s_pMemManPage->m_FrameStats.sChunks[MC_ANIMATION], s_pMemManPage->m_TotalStats.sChunks[MC_ANIMATION] );
	SumFrameResetResults( s_pMemManPage->m_FrameStats.sChunks[MC_HAVOK], s_pMemManPage->m_TotalStats.sChunks[MC_HAVOK] );
	SumFrameResetResults( s_pMemManPage->m_FrameStats.sChunks[MC_AUDIO], s_pMemManPage->m_TotalStats.sChunks[MC_AUDIO] );

#ifndef _COLLAPSE_SMALL_CHUNKS
	SumFrameResetResults( s_pMemManPage->m_FrameStats.sChunks[MC_ODB], s_pMemManPage->m_TotalStats.sChunks[MC_ODB] );

	SumFrameResetResults( s_pMemManPage->m_FrameStats.sChunks[MC_ARMY], s_pMemManPage->m_TotalStats.sChunks[MC_ARMY] );
	SumFrameResetResults( s_pMemManPage->m_FrameStats.sChunks[MC_LUA], s_pMemManPage->m_TotalStats.sChunks[MC_LUA] );
	SumFrameResetResults( s_pMemManPage->m_FrameStats.sChunks[MC_ENTITY], s_pMemManPage->m_TotalStats.sChunks[MC_ENTITY] );
	SumFrameResetResults( s_pMemManPage->m_FrameStats.sChunks[MC_MISC], s_pMemManPage->m_TotalStats.sChunks[MC_MISC] );

	SumFrameResetResults( s_pMemManPage->m_FrameStats.sChunks[MC_AI], s_pMemManPage->m_TotalStats.sChunks[MC_AI] );
	SumFrameResetResults( s_pMemManPage->m_FrameStats.sChunks[MC_CAMERA], s_pMemManPage->m_TotalStats.sChunks[MC_CAMERA] );
	SumFrameResetResults( s_pMemManPage->m_FrameStats.sChunks[MC_EFFECTS], s_pMemManPage->m_TotalStats.sChunks[MC_EFFECTS] );
	SumFrameResetResults( s_pMemManPage->m_FrameStats.sChunks[MC_PROCEDURAL], s_pMemManPage->m_TotalStats.sChunks[MC_PROCEDURAL] );
#endif // _COLLAPSE_SMALL_CHUNKS

	SumFrameResetResults( s_pMemManPage->m_FrameStats.sChunks[MC_OVERFLOW], s_pMemManPage->m_TotalStats.sChunks[MC_OVERFLOW] );

	SumFrameResetResults( s_pMemManPage->m_FrameStats.sChunks[MC_DEBUG], s_pMemManPage->m_TotalStats.sChunks[MC_DEBUG] );
	SumFrameResetResults( s_pMemManPage->m_FrameStats.sChunks[MC_DEBUG_MEM_TRACKER], s_pMemManPage->m_TotalStats.sChunks[MC_DEBUG_MEM_TRACKER] );

	memset( &s_pMemManPage->m_FrameStats, 0, sizeof(MemStats) );
#endif
}

const MemStats& GetFrameStats()
{
	ntAssert( ManagerExists() );

	return s_pMemManPage->m_FrameStats;
}

const MemStats& GetMemStats()
{
	ntAssert( ManagerExists() );

	return s_pMemManPage->m_TotalStats;
}

#ifdef _HAVE_MEMORY_TRACKING

#define DUMP_CHUNK_START_AND_END( file, buffer, chunk, allocator )		\
	{ 																	\
		int len = snprintf(buffer, bufSize, "%d %s %d %d\n", chunk,	GetChunkName( chunk ), allocator.m_pGlobalAddr, allocator.m_iGlobalSize ); \
		file.Write(buffer, len < 0 ? bufSize : len);					\
	}

void DumpMemoryTrackerMap()
{
	DumpSimpleChunkStats();
	
	const unsigned int bufSize = 1024;
	char buffer[bufSize];

	#ifdef PLATFORM_PS3
	Util::GetFullGameDataFilePath("memory.log", buffer);
	CellFsFile dumpFile(buffer, File::FT_WRITE | File::FT_TEXT);
	#else
	Util::GetFiosFilePath("memory.log", buffer);
	File dumpFile(buffer, File::FT_WRITE | File::FT_TEXT);
	#endif

	ntPrintf("-------------Begin Memory Tracker Map---------------------\n");
	
	DUMP_CHUNK_START_AND_END( dumpFile, buffer, Mem::MC_RSX_MAIN_INTERNAL , s_pMemManPage->m_RsxMainAllocator 		);  
	DUMP_CHUNK_START_AND_END( dumpFile, buffer, Mem::MC_RSX_MAIN_USER     , s_pMemManPage->m_RsxMainUserAllocator   );  

	DUMP_CHUNK_START_AND_END( dumpFile, buffer, Mem::MC_GFX               , s_pMemManPage->m_GfxAllocator 	 		);  
	DUMP_CHUNK_START_AND_END( dumpFile, buffer, Mem::MC_LOADER            , s_pMemManPage->m_LoaderAllocator 		);  
	DUMP_CHUNK_START_AND_END( dumpFile, buffer, Mem::MC_ANIMATION         , s_pMemManPage->m_AnimationAllocator 	);  
	DUMP_CHUNK_START_AND_END( dumpFile, buffer, Mem::MC_HAVOK             , s_pMemManPage->m_HavokAllocator 		);  
	DUMP_CHUNK_START_AND_END( dumpFile, buffer, Mem::MC_AUDIO             , s_pMemManPage->m_AudioAllocator         );

#ifndef _COLLAPSE_SMALL_CHUNKS
	DUMP_CHUNK_START_AND_END( dumpFile, buffer, Mem::MC_ODB               , s_pMemManPage->m_ODBAllocator 	 		);  

	DUMP_CHUNK_START_AND_END( dumpFile, buffer, Mem::MC_ARMY              , s_pMemManPage->m_ArmyAllocator 	 		);  
	DUMP_CHUNK_START_AND_END( dumpFile, buffer, Mem::MC_LUA               , s_pMemManPage->m_LuaAllocator 	 		);  
	DUMP_CHUNK_START_AND_END( dumpFile, buffer, Mem::MC_ENTITY            , s_pMemManPage->m_EntityAllocator 		);  
	DUMP_CHUNK_START_AND_END( dumpFile, buffer, Mem::MC_MISC              , s_pMemManPage->m_MiscAllocator 	 		);  

	DUMP_CHUNK_START_AND_END( dumpFile, buffer, Mem::MC_AI                , s_pMemManPage->m_AIAllocator 	 		);  
	DUMP_CHUNK_START_AND_END( dumpFile, buffer, Mem::MC_CAMERA            , s_pMemManPage->m_CameraAllocator 		);  
	DUMP_CHUNK_START_AND_END( dumpFile, buffer, Mem::MC_EFFECTS           , s_pMemManPage->m_EffectsAllocator 		);  
	DUMP_CHUNK_START_AND_END( dumpFile, buffer, Mem::MC_PROCEDURAL        , s_pMemManPage->m_ProceduralAllocator 	);  
#endif

	DUMP_CHUNK_START_AND_END( dumpFile, buffer, Mem::MC_OVERFLOW		  , s_pMemManPage->m_OverflowAllocator 		);  

	// Give overall spans for the chunks (helps memory layout displays enormously)
#ifdef _HAVE_DEBUG_MEMORY
	DUMP_CHUNK_START_AND_END( dumpFile, buffer, Mem::MC_DEBUG             , s_pMemManPage->m_DebugAllocator 		);
#endif

#ifdef _HAVE_MEMORY_TRACKING
	DUMP_CHUNK_START_AND_END( dumpFile, buffer, Mem::MC_DEBUG_MEM_TRACKER , s_pMemManPage->m_DebugMemTracker 		);
#endif

	dumpFile.Write( "\n\n", 2 );

	AllocTrackerMap* pMap = s_pMemManPage->m_pMemTrackerMap;

	for (AllocTrackerMap::iterator iter = pMap -> begin(); iter != pMap -> end(); ++ iter)
	{
		if (iter -> second.m_eOldChunk == Mem::MC_GFX || iter -> second.m_eChunk == Mem::MC_GFX)
		{
		//ntPrintf("%i, %s, %s, %i, %x, %i\n", iter -> first, iter -> second.m_pTag, iter -> second.m_pSubTag, iter -> second.m_iNum, iter -> second.m_pPtr, iter -> second.m_iSize);
		int len = snprintf(buffer, bufSize, "%d, %s, %s, %d, %x, %d %d\n", iter -> first, iter -> second.m_pTag, iter -> second.m_pSubTag, iter -> second.m_iNum, iter -> second.m_pPtr, iter -> second.m_iSize, iter->second.m_eChunk);
		dumpFile.Write(buffer, len < 0 ? bufSize : len);
	}
	}

	ntPrintf("--------------End Memory Tracker Map ----------------------\n");

	dumpFile.Flush();
}

#endif // _HAVE_MEMORY_TRACKING

//--------------------------------------------------
//!
//!	Alloc
//!
//--------------------------------------------------
#ifndef _HAVE_MEMORY_TRACKING
uintptr_t Alloc( MEMORY_CHUNK chunk, uint32_t iSize )
#else
uintptr_t Alloc( MEMORY_CHUNK chunk, uint32_t iSize, const char* pTag, const char* pSubTag, int iNum )
#endif
{
	// Alexey : this is just a temporary hack to make global strings working
	if (!ManagerExists())
	{
		Init();
		//snPause();
	}

	ntAssert( ManagerExists() );
	ntError_p( chunk != MC_UNKNOWN, ("Allocations MUST specify a chunk\n") );

	MEMORY_CHUNK oldchunk = chunk;
	uintptr_t pAddr;

	switch( chunk )
	{
	case MC_RSX_MAIN_INTERNAL:
		pAddr = s_pMemManPage->m_RsxMainAllocator.Alloc( s_pMemManPage->m_MemScope, iSize ); break;

	case MC_RSX_MAIN_USER:
		pAddr = s_pMemManPage->m_RsxMainUserAllocator.Alloc( s_pMemManPage->m_MemScope, iSize ); break;

	case MC_GFX:
		pAddr = s_pMemManPage->m_GfxAllocator.Alloc( s_pMemManPage->m_MemScope, iSize ); 	break;

	case MC_LOADER:
		pAddr = s_pMemManPage->m_LoaderAllocator.Alloc( s_pMemManPage->m_MemScope, iSize );  break;

	case MC_ANIMATION:
		pAddr = s_pMemManPage->m_AnimationAllocator.Alloc( s_pMemManPage->m_MemScope, iSize );  break;

	case MC_HAVOK:
		pAddr = s_pMemManPage->m_HavokAllocator.Alloc( s_pMemManPage->m_MemScope, iSize ); 	break;

	case MC_AUDIO:
		pAddr = s_pMemManPage->m_AudioAllocator.Alloc( s_pMemManPage->m_MemScope, iSize ); break;

#ifndef _COLLAPSE_SMALL_CHUNKS
	case MC_ODB:
		pAddr = s_pMemManPage->m_ODBAllocator.Alloc( s_pMemManPage->m_MemScope, iSize );  break;

	case MC_ARMY:
		pAddr = s_pMemManPage->m_ArmyAllocator.Alloc( s_pMemManPage->m_MemScope, iSize );  break;

	case MC_LUA:
		pAddr = s_pMemManPage->m_LuaAllocator.Alloc( s_pMemManPage->m_MemScope, iSize );  break;

	case MC_ENTITY:
		pAddr = s_pMemManPage->m_EntityAllocator.Alloc( s_pMemManPage->m_MemScope, iSize );  break;

	case MC_MISC:
		pAddr = s_pMemManPage->m_MiscAllocator.Alloc( s_pMemManPage->m_MemScope, iSize );	break;

	case MC_AI:
		pAddr = s_pMemManPage->m_AIAllocator.Alloc( s_pMemManPage->m_MemScope, iSize );  break;

	case MC_CAMERA:
		pAddr = s_pMemManPage->m_CameraAllocator.Alloc( s_pMemManPage->m_MemScope, iSize );  break;

	case MC_EFFECTS:
		pAddr = s_pMemManPage->m_EffectsAllocator.Alloc( s_pMemManPage->m_MemScope, iSize );  break;

	case MC_PROCEDURAL:
		pAddr = s_pMemManPage->m_ProceduralAllocator.Alloc( s_pMemManPage->m_MemScope, iSize );  break;

#else // _COLLAPSE_SMALL_CHUNKS
	// allow all of the above to fall through to overflow
	case MC_ODB:
	case MC_ARMY:
	case MC_LUA:
	case MC_ENTITY:
	case MC_MISC:
	case MC_AI:
	case MC_CAMERA:
	case MC_EFFECTS:
	case MC_PROCEDURAL:
		chunk = MC_OVERFLOW;
#endif // _COLLAPSE_SMALL_CHUNKS

	case MC_OVERFLOW:
		pAddr = s_pMemManPage->m_OverflowAllocator.Alloc( s_pMemManPage->m_MemScope, iSize ); 	break;

#ifdef _HAVE_DEBUG_MEMORY
	case MC_DEBUG:
		pAddr = s_pMemManPage->m_DebugAllocator.Alloc( s_pMemManPage->m_MemScope, iSize );	break;
#endif

#ifdef _HAVE_MEMORY_TRACKING
	case MC_DEBUG_MEM_TRACKER:
		pAddr = s_pMemManPage->m_DebugMemTracker.Alloc( s_pMemManPage->m_MemScope, iSize ); break;
#endif

	default:
		ntAssert(0);
		pAddr = 0;
	};

	if( pAddr == 0 && s_pMemManPage->m_bAllowOverflow[ chunk ] )
	{
		pAddr = s_pMemManPage->m_OverflowAllocator.Alloc( s_pMemManPage->m_MemScope, iSize );
		chunk = MC_OVERFLOW;
	}

#if BREAK_ON_ALLOC_ADDRESS
	if( (uintptr_t)pAddr == BREAK_ON_ALLOC_ADDRESS )
	{
		DebugBreakNow();
	}
#endif

#if defined(_HAVE_MEMORY_TRACKING)
	if (0 == pAddr)
	{
		// dump total stats
		ntPrintf( "-------- Out of Memory -----------\n" );
		ntPrintf( " Total Bytes Current Allocated %d\n", s_pMemManPage->TotalInUse() );
		ntPrintf( " Total Bytes for Chunk %s, Current Allocated %d\n", GetChunkName(chunk), s_pMemManPage->TotalInChunk(chunk) );

		DumpSimpleChunkStats();

		//DumpMemoryTrackerMap();

		ntPrintf( "-------- Out of Memory -----------\n" );
	}

	ntError_p( pAddr, ("Out of memory trying to allocate 0x%x from chunk %s", iSize, GetChunkName(chunk) ) );
	LogTag( LT_ALLOC, chunk, oldchunk, pTag, pSubTag, iNum, pAddr, iSize, (chunk == MC_DEBUG_MEM_TRACKER)? true : false );
#else
	ntError_p( pAddr, ("Out of memory trying to allocate 0x%x from chunk %s", iSize, GetChunkName(chunk) ) );
	UNUSED( oldchunk );
#endif

	return pAddr;
}

//--------------------------------------------------
//!
//!	ConvertAddressToChunk
//!
//--------------------------------------------------
MEMORY_CHUNK ConvertAddressToChunk( uintptr_t pMem )
{
	// slow oh so slow the algorithm is blindly simple...
	// ask each allocator if the memory is owned by it,
	// if it is return that chunk name

	if( s_pMemManPage->m_OverflowAllocator.IsOurs( s_pMemManPage->m_MemScope, pMem ) == true )
		return MC_OVERFLOW;

#ifdef _HAVE_MEMORY_TRACKING
	if( s_pMemManPage->m_DebugMemTracker.IsOurs( s_pMemManPage->m_MemScope, pMem ) == true )
		return MC_DEBUG_MEM_TRACKER;
#endif

#ifdef _HAVE_DEBUG_MEMORY
	if( s_pMemManPage->m_DebugAllocator.IsOurs( s_pMemManPage->m_MemScope, pMem ) == true )
		return MC_DEBUG;
#endif

	if( s_pMemManPage->m_GfxAllocator.IsOurs( s_pMemManPage->m_MemScope, pMem ) == true )
		return MC_GFX;

	if( s_pMemManPage->m_AnimationAllocator.IsOurs( s_pMemManPage->m_MemScope, pMem ) == true )
		return MC_ANIMATION;

	if( s_pMemManPage->m_HavokAllocator.IsOurs( s_pMemManPage->m_MemScope, pMem ) == true )
		return MC_HAVOK;

	if( s_pMemManPage->m_RsxMainAllocator.IsOurs( s_pMemManPage->m_MemScope, pMem ) == true )
	{
		if( s_pMemManPage->m_RsxMainUserAllocator.IsOurs( s_pMemManPage->m_MemScope, pMem ) == true )
			return MC_RSX_MAIN_USER;
		return MC_RSX_MAIN_INTERNAL;
	}

	if( s_pMemManPage->m_LoaderAllocator.IsOurs( s_pMemManPage->m_MemScope, pMem ) == true )
		return MC_LOADER;

	if( s_pMemManPage->m_AudioAllocator.IsOurs( s_pMemManPage->m_MemScope, pMem ) == true )
		return MC_AUDIO;

#ifndef _COLLAPSE_SMALL_CHUNKS
	if( s_pMemManPage->m_MiscAllocator.IsOurs( s_pMemManPage->m_MemScope, pMem ) == true )
		return MC_MISC;

	if( s_pMemManPage->m_ODBAllocator.IsOurs( s_pMemManPage->m_MemScope, pMem ) == true )
		return MC_ODB;

	if( s_pMemManPage->m_ArmyAllocator.IsOurs( s_pMemManPage->m_MemScope, pMem ) == true )
		return MC_ARMY;

	if( s_pMemManPage->m_LuaAllocator.IsOurs( s_pMemManPage->m_MemScope, pMem ) == true )
		return MC_LUA;

	if( s_pMemManPage->m_EntityAllocator.IsOurs( s_pMemManPage->m_MemScope, pMem ) == true )
		return MC_ENTITY;

	if( s_pMemManPage->m_AIAllocator.IsOurs( s_pMemManPage->m_MemScope, pMem ) == true )
		return MC_AI;

	if( s_pMemManPage->m_CameraAllocator.IsOurs( s_pMemManPage->m_MemScope, pMem ) == true )
		return MC_CAMERA;

	if( s_pMemManPage->m_EffectsAllocator.IsOurs( s_pMemManPage->m_MemScope, pMem ) == true )
		return MC_EFFECTS;

	if( s_pMemManPage->m_ProceduralAllocator.IsOurs( s_pMemManPage->m_MemScope, pMem ) == true )
		return MC_PROCEDURAL;

#endif // _COLLAPSE_SMALL_CHUNKS

	return MC_UNKNOWN;
}

//--------------------------------------------------
//!
//!	Realloc
//!
//--------------------------------------------------
#if !defined( _HAVE_MEMORY_TRACKING )
uintptr_t Realloc( MEMORY_CHUNK chunk, uintptr_t pMem, uint32_t iSize )
#else
uintptr_t Realloc( MEMORY_CHUNK chunk, uintptr_t pMem, uint32_t iSize, const char* pTag, const char* pSubTag, int iNum )
#endif
{
	ntAssert( ManagerExists() );

	if( chunk == MC_UNKNOWN )
	{
		chunk = ConvertAddressToChunk( pMem );
		ntError_p( chunk != MC_UNKNOWN, ("Unknown Address (not from our memory system!) passed to Free. Address: %p\n", pMem ) );
	}

	MEMORY_CHUNK oldchunk = chunk;
	
	// check for overflow
	if( s_pMemManPage->m_bAllowOverflow[ chunk ] )
	{
		if( s_pMemManPage->m_OverflowAllocator.IsOurs( s_pMemManPage->m_MemScope, pMem ) )
			chunk = MC_OVERFLOW;
	}

#ifdef _HAVE_MEMORY_TRACKING
	// log stuff so that double delete can find things before free is called
	LogTag( LT_SHRINK, chunk, oldchunk, pTag, pSubTag, iNum, pMem, iSize, false );
#else
	UNUSED( oldchunk );
#endif

	uintptr_t pAddr = 0;

	switch( chunk )
	{
	case MC_RSX_MAIN_INTERNAL:
		pAddr = s_pMemManPage->m_RsxMainAllocator.Realloc( s_pMemManPage->m_MemScope, pMem, iSize ); break;

	case MC_RSX_MAIN_USER:
		pAddr = s_pMemManPage->m_RsxMainUserAllocator.Realloc( s_pMemManPage->m_MemScope, pMem, iSize ); break;

	case MC_GFX:
		pAddr = s_pMemManPage->m_GfxAllocator.Realloc( s_pMemManPage->m_MemScope, pMem, iSize ); break;

	case MC_LOADER:
		pAddr = s_pMemManPage->m_LoaderAllocator.Realloc( s_pMemManPage->m_MemScope, pMem, iSize ); break;

	case MC_ANIMATION:
		pAddr = s_pMemManPage->m_AnimationAllocator.Realloc( s_pMemManPage->m_MemScope, pMem, iSize ); break;

	case MC_HAVOK:
		pAddr = s_pMemManPage->m_HavokAllocator.Realloc( s_pMemManPage->m_MemScope, pMem, iSize ); break;

	case MC_AUDIO:
		pAddr = s_pMemManPage->m_AudioAllocator.Realloc( s_pMemManPage->m_MemScope, pMem, iSize ); break;

#ifndef _COLLAPSE_SMALL_CHUNKS
	case MC_ODB:
		pAddr = s_pMemManPage->m_ODBAllocator.Realloc( s_pMemManPage->m_MemScope, pMem, iSize ); break;

	case MC_ARMY:
		pAddr = s_pMemManPage->m_ArmyAllocator.Realloc( s_pMemManPage->m_MemScope, pMem, iSize ); break;

	case MC_LUA:
		pAddr = s_pMemManPage->m_LuaAllocator.Realloc( s_pMemManPage->m_MemScope, pMem, iSize ); break;

	case MC_ENTITY:
		pAddr = s_pMemManPage->m_EntityAllocator.Realloc( s_pMemManPage->m_MemScope, pMem, iSize ); break;

	case MC_MISC:
		pAddr = s_pMemManPage->m_MiscAllocator.Realloc( s_pMemManPage->m_MemScope, pMem, iSize ); break;

	case MC_AI:
		pAddr = s_pMemManPage->m_AIAllocator.Realloc( s_pMemManPage->m_MemScope, pMem, iSize ); break;

	case MC_CAMERA:
		pAddr = s_pMemManPage->m_CameraAllocator.Realloc( s_pMemManPage->m_MemScope, pMem, iSize ); break;

	case MC_EFFECTS:
		pAddr = s_pMemManPage->m_EffectsAllocator.Realloc( s_pMemManPage->m_MemScope, pMem, iSize ); break;

	case MC_PROCEDURAL:
		pAddr = s_pMemManPage->m_ProceduralAllocator.Realloc( s_pMemManPage->m_MemScope, pMem, iSize ); break;

#else // _COLLAPSE_SMALL_CHUNKS
	// allow all of the above to fall through to overflow
	case MC_ODB:
	case MC_ARMY:
	case MC_LUA:
	case MC_ENTITY:
	case MC_MISC:
	case MC_AI:
	case MC_CAMERA:
	case MC_EFFECTS:
	case MC_PROCEDURAL:
		chunk = MC_OVERFLOW;
#endif // _COLLAPSE_SMALL_CHUNKS

	case MC_OVERFLOW:
		pAddr = s_pMemManPage->m_OverflowAllocator.Realloc( s_pMemManPage->m_MemScope, pMem, iSize ); break;

#ifdef _HAVE_DEBUG_MEMORY
	case MC_DEBUG:
		pAddr = s_pMemManPage->m_DebugAllocator.Realloc( s_pMemManPage->m_MemScope, pMem, iSize ); break;
#endif

#ifdef _HAVE_MEMORY_TRACKING
	case MC_DEBUG_MEM_TRACKER:
		pAddr = s_pMemManPage->m_DebugMemTracker.Realloc( s_pMemManPage->m_MemScope, pMem, iSize ); break;
#endif

	default:
		ntError_p(0,("Reallocing from unhandled chunk %s", GetChunkName(chunk,false) ));
	}

	return pAddr;
}

//--------------------------------------------------
//!
//! Free
//!
//--------------------------------------------------
#if !defined( _HAVE_MEMORY_TRACKING )
void Free( MEMORY_CHUNK chunk, uintptr_t pMem )
#else
void Free( MEMORY_CHUNK chunk, uintptr_t pMem, const char* pTag, const char* pSubTag, int iNum )
#endif
{
	ntAssert( ManagerExists() );

#if BREAK_ON_FREE_ADDRESS
	if(pMem == BREAK_ON_FREE_ADDRESS)
	{
		DebugBreakNow();
	}
#endif

	// if we don't know our chunk (god I hate C++ sometimes...) 
	if( chunk == MC_UNKNOWN )
	{
		chunk = ConvertAddressToChunk( pMem );
		ntError_p( chunk != MC_UNKNOWN, ("Unknown Address (not from our memory system!) passed to Free. Address: %p\n", pMem ) );
	}

	//ntError_p(chunk == ConvertAddressToChunk( pMem ), ("Memory at %p belongs to chunk %s while being deallocated from chunk %s\n", pMem, GetChunkName(ConvertAddressToChunk( pMem )), GetChunkName(chunk)));

	MEMORY_CHUNK oldchunk = chunk;

	// check for an overflowed allocation
	if( s_pMemManPage->m_bAllowOverflow[ chunk ] )
	{
		if( s_pMemManPage->m_OverflowAllocator.IsOurs( s_pMemManPage->m_MemScope, pMem ) )
			chunk = MC_OVERFLOW;
	}

#if defined(_HAVE_MEMORY_TRACKING)
	// log stuff so that double delete can find things before free is called
	LogTag( LT_FREE, chunk, oldchunk, pTag, pSubTag, iNum, pMem, 0, false );
#else
	UNUSED( oldchunk );
#endif		

	switch( chunk )
	{
	case MC_RSX_MAIN_INTERNAL:
		s_pMemManPage->m_RsxMainAllocator.Free( s_pMemManPage->m_MemScope, pMem ); break;

	case MC_RSX_MAIN_USER:
		s_pMemManPage->m_RsxMainUserAllocator.Free( s_pMemManPage->m_MemScope, pMem ); break;

	case MC_GFX:
		s_pMemManPage->m_GfxAllocator.Free( s_pMemManPage->m_MemScope, pMem ); break;

	case MC_LOADER:
		s_pMemManPage->m_LoaderAllocator.Free( s_pMemManPage->m_MemScope, pMem ); break;

	case MC_ANIMATION:
		s_pMemManPage->m_AnimationAllocator.Free( s_pMemManPage->m_MemScope, pMem ); break;

	case MC_HAVOK:
		s_pMemManPage->m_HavokAllocator.Free( s_pMemManPage->m_MemScope, pMem ); break;

	case MC_AUDIO:
		s_pMemManPage->m_AudioAllocator.Free( s_pMemManPage->m_MemScope, pMem ); break;

#ifndef _COLLAPSE_SMALL_CHUNKS

	case MC_ODB:
		s_pMemManPage->m_ODBAllocator.Free( s_pMemManPage->m_MemScope, pMem ); break;

	case MC_ARMY:
		s_pMemManPage->m_ArmyAllocator.Free( s_pMemManPage->m_MemScope, pMem ); break;

	case MC_LUA:
		s_pMemManPage->m_LuaAllocator.Free( s_pMemManPage->m_MemScope, pMem ); break;

	case MC_ENTITY:
		s_pMemManPage->m_EntityAllocator.Free( s_pMemManPage->m_MemScope, pMem ); break;

	case MC_MISC:
		s_pMemManPage->m_MiscAllocator.Free( s_pMemManPage->m_MemScope, pMem ); break;

	case MC_AI:
		s_pMemManPage->m_AIAllocator.Free( s_pMemManPage->m_MemScope, pMem ); break;

	case MC_CAMERA:
		s_pMemManPage->m_CameraAllocator.Free( s_pMemManPage->m_MemScope, pMem ); break;

	case MC_EFFECTS:
		s_pMemManPage->m_EffectsAllocator.Free( s_pMemManPage->m_MemScope, pMem ); break;

	case MC_PROCEDURAL:
		s_pMemManPage->m_ProceduralAllocator.Free( s_pMemManPage->m_MemScope, pMem ); break;

#else // _COLLAPSE_SMALL_CHUNKS
	// allow all of the above to fall through to overflow
	case MC_ODB:
	case MC_ARMY:
	case MC_LUA:
	case MC_ENTITY:
	case MC_MISC:
	case MC_AI:
	case MC_CAMERA:
	case MC_EFFECTS:
	case MC_PROCEDURAL:
		chunk = MC_OVERFLOW;
#endif // _COLLAPSE_SMALL_CHUNKS

	case MC_OVERFLOW:
		s_pMemManPage->m_OverflowAllocator.Free( s_pMemManPage->m_MemScope, pMem ); break;

#ifdef _HAVE_DEBUG_MEMORY
	case MC_DEBUG:
		s_pMemManPage->m_DebugAllocator.Free( s_pMemManPage->m_MemScope, pMem ); break;
#endif

#ifdef _HAVE_MEMORY_TRACKING
	case MC_DEBUG_MEM_TRACKER:
		s_pMemManPage->m_DebugMemTracker.Free( s_pMemManPage->m_MemScope, pMem ); break;
#endif

	default:
		ntAssert(0);
	}
}

//--------------------------------------------------
//!
//! MemAlign
//!
//--------------------------------------------------
#if !defined( _HAVE_MEMORY_TRACKING )
uintptr_t MemAlign( MEMORY_CHUNK chunk, uint32_t iSize, uint32_t iAlign, bool assert_on_failure )
#else
uintptr_t MemAlign( MEMORY_CHUNK chunk, uint32_t iSize, uint32_t iAlign, const char* pTag, const char* pSubTag, int iNum, bool assert_on_failure )
#endif
{
	UNUSED( assert_on_failure );
	ntAssert( ManagerExists() );

	ntError_p( chunk != MC_UNKNOWN, ("Allocations MUST specify a chunk\n") );
	MEMORY_CHUNK oldchunk = chunk;

	uintptr_t pAddr;
	switch( chunk )
	{
	case MC_RSX_MAIN_INTERNAL:
		pAddr = s_pMemManPage->m_RsxMainAllocator.MemAlign( s_pMemManPage->m_MemScope, iSize, iAlign ); break;

	case MC_RSX_MAIN_USER:
		pAddr = s_pMemManPage->m_RsxMainUserAllocator.MemAlign( s_pMemManPage->m_MemScope, iSize, iAlign ); break;

	case MC_GFX:
		pAddr = s_pMemManPage->m_GfxAllocator.MemAlign( s_pMemManPage->m_MemScope, iSize, iAlign ); break;

	case MC_LOADER:
		pAddr = s_pMemManPage->m_LoaderAllocator.MemAlign( s_pMemManPage->m_MemScope, iSize, iAlign ); break;

	case MC_ANIMATION:
		pAddr = s_pMemManPage->m_AnimationAllocator.MemAlign( s_pMemManPage->m_MemScope, iSize, iAlign ); break;

	case MC_HAVOK:
		pAddr = s_pMemManPage->m_HavokAllocator.MemAlign( s_pMemManPage->m_MemScope, iSize, iAlign ); break;

	case MC_AUDIO:
		pAddr = s_pMemManPage->m_AudioAllocator.MemAlign( s_pMemManPage->m_MemScope, iSize, iAlign ); break;

#ifndef _COLLAPSE_SMALL_CHUNKS

	case MC_ODB:
		pAddr = s_pMemManPage->m_ODBAllocator.MemAlign( s_pMemManPage->m_MemScope, iSize, iAlign ); break;

	case MC_ARMY:
		pAddr = s_pMemManPage->m_ArmyAllocator.MemAlign( s_pMemManPage->m_MemScope, iSize, iAlign ); break;

	case MC_LUA:
		pAddr = s_pMemManPage->m_LuaAllocator.MemAlign( s_pMemManPage->m_MemScope, iSize, iAlign ); break;

	case MC_ENTITY:
		pAddr = s_pMemManPage->m_EntityAllocator.MemAlign( s_pMemManPage->m_MemScope, iSize, iAlign ); break;

	case MC_MISC:
		pAddr = s_pMemManPage->m_MiscAllocator.MemAlign( s_pMemManPage->m_MemScope, iSize, iAlign ); break;

	case MC_AI:
		pAddr = s_pMemManPage->m_AIAllocator.MemAlign( s_pMemManPage->m_MemScope, iSize, iAlign ); break;

	case MC_CAMERA:
		pAddr = s_pMemManPage->m_CameraAllocator.MemAlign( s_pMemManPage->m_MemScope, iSize, iAlign ); break;

	case MC_EFFECTS:
		pAddr = s_pMemManPage->m_EffectsAllocator.MemAlign( s_pMemManPage->m_MemScope, iSize, iAlign ); break;

	case MC_PROCEDURAL:
		pAddr = s_pMemManPage->m_ProceduralAllocator.MemAlign( s_pMemManPage->m_MemScope, iSize, iAlign ); break;

#else // _COLLAPSE_SMALL_CHUNKS
	// allow all of the above to fall through to overflow
	case MC_ODB:
	case MC_ARMY:
	case MC_LUA:
	case MC_ENTITY:
	case MC_MISC:
	case MC_AI:
	case MC_CAMERA:
	case MC_EFFECTS:
	case MC_PROCEDURAL:
		chunk = MC_OVERFLOW;
#endif // _COLLAPSE_SMALL_CHUNKS

	case MC_OVERFLOW:
		pAddr = s_pMemManPage->m_OverflowAllocator.MemAlign( s_pMemManPage->m_MemScope, iSize, iAlign ); break;

#ifdef _HAVE_DEBUG_MEMORY
	case MC_DEBUG:
		pAddr = s_pMemManPage->m_DebugAllocator.MemAlign( s_pMemManPage->m_MemScope, iSize, iAlign ); break;
#endif

#ifdef _HAVE_MEMORY_TRACKING
	case MC_DEBUG_MEM_TRACKER:
		pAddr = s_pMemManPage->m_DebugMemTracker.MemAlign( s_pMemManPage->m_MemScope, iSize, iAlign ); break;
#endif

	default:
		ntAssert(0);
		pAddr = 0;
	}

	if( pAddr == 0 && s_pMemManPage->m_bAllowOverflow[ chunk ] )
	{
		pAddr = s_pMemManPage->m_OverflowAllocator.MemAlign( s_pMemManPage->m_MemScope, iSize, iAlign );
		chunk = MC_OVERFLOW;
	}

#if BREAK_ON_ALLOC_ADDRESS
	if( (uintptr_t)pAddr == BREAK_ON_ALLOC_ADDRESS )
	{
		DebugBreakNow();
	}
#endif

	ntError_p( ( pAddr & ( iAlign - 1 ) ) == 0, ("Memory manager should be returning aligned memory here, but isn't.") );

#if defined(_HAVE_MEMORY_TRACKING)
	if (0 == pAddr && assert_on_failure )
	{
		// dump total stats
		ntPrintf( "-------- Out of Memory -----------\n" );
		ntPrintf( " Total Bytes Current Allocated %d\n", s_pMemManPage->TotalInUse() );
		ntPrintf( " Total Bytes for Chunk %s, Current Allocated %d\n", GetChunkName(chunk), s_pMemManPage->TotalInChunk(chunk) );

		DumpSimpleChunkStats();

		//DumpMemoryTrackerMap();

		ntPrintf( "-------- Out of Memory -----------\n" );
	}

	ntError_p( pAddr != NULL || !assert_on_failure, ("Out of memory trying to allocate 0x%x from chunk %s", iSize, GetChunkName(chunk) ) );

	if ( pAddr != NULL || assert_on_failure )
	{
		LogTag( LT_ALLOC, chunk, oldchunk, pTag, pSubTag, iNum, pAddr, iSize, (chunk == MC_DEBUG_MEM_TRACKER)? true : false );
	}
#else
	ntError_p( pAddr != NULL || !assert_on_failure, ("Out of memory trying to allocate 0x%x from chunk %s", iSize, GetChunkName(chunk) ) );
	UNUSED( oldchunk );
#endif

	return pAddr;
}

//--------------------------------------------------
//!
//! GetFreeSpace
//!
//--------------------------------------------------
uint32_t GetFreeSpace( MEMORY_CHUNK chunk )
{
	switch( chunk )
	{
	case MC_RSX_MAIN_INTERNAL:
		return s_pMemManPage->m_RsxMainAllocator.GetCurrentFreeSpace(s_pMemManPage->m_MemScope); break;

	case MC_RSX_MAIN_USER:
		return s_pMemManPage->m_RsxMainUserAllocator.GetCurrentFreeSpace(s_pMemManPage->m_MemScope); break;

	case MC_GFX:
		return s_pMemManPage->m_GfxAllocator.GetCurrentFreeSpace(s_pMemManPage->m_MemScope); break;

	case MC_LOADER:
		return s_pMemManPage->m_LoaderAllocator.GetCurrentFreeSpace(s_pMemManPage->m_MemScope); break;

	case MC_ANIMATION:
		return s_pMemManPage->m_AnimationAllocator.GetCurrentFreeSpace(s_pMemManPage->m_MemScope); break;

	case MC_HAVOK:
		return s_pMemManPage->m_HavokAllocator.GetCurrentFreeSpace(s_pMemManPage->m_MemScope); break;

	case MC_AUDIO:
		return s_pMemManPage->m_AudioAllocator.GetCurrentFreeSpace(s_pMemManPage->m_MemScope); break;

#ifndef _COLLAPSE_SMALL_CHUNKS

	case MC_ODB:
		return s_pMemManPage->m_ODBAllocator.GetCurrentFreeSpace(s_pMemManPage->m_MemScope); break;

	case MC_ARMY:
		return s_pMemManPage->m_ArmyAllocator.GetCurrentFreeSpace(s_pMemManPage->m_MemScope); break;

	case MC_LUA:
		return s_pMemManPage->m_LuaAllocator.GetCurrentFreeSpace(s_pMemManPage->m_MemScope); break;

	case MC_ENTITY:
		return s_pMemManPage->m_EntityAllocator.GetCurrentFreeSpace(s_pMemManPage->m_MemScope); break;

	case MC_MISC:
		return s_pMemManPage->m_MiscAllocator.GetCurrentFreeSpace(s_pMemManPage->m_MemScope); break;

	case MC_AI:
		return s_pMemManPage->m_AIAllocator.GetCurrentFreeSpace(s_pMemManPage->m_MemScope); break;
	
	case MC_CAMERA:
		return s_pMemManPage->m_CameraAllocator.GetCurrentFreeSpace(s_pMemManPage->m_MemScope); break;

	case MC_EFFECTS:
		return s_pMemManPage->m_EffectsAllocator.GetCurrentFreeSpace(s_pMemManPage->m_MemScope); break;

	case MC_PROCEDURAL:
		return s_pMemManPage->m_ProceduralAllocator.GetCurrentFreeSpace(s_pMemManPage->m_MemScope); break;

#else // _COLLAPSE_SMALL_CHUNKS
	// allow all of the above to fall through to overflow
	case MC_ODB:
	case MC_ARMY:
	case MC_LUA:
	case MC_ENTITY:
	case MC_MISC:
	case MC_AI:
	case MC_CAMERA:
	case MC_EFFECTS:
	case MC_PROCEDURAL:
		chunk = MC_OVERFLOW;
#endif // _COLLAPSE_SMALL_CHUNKS

	case MC_OVERFLOW:
		return s_pMemManPage->m_OverflowAllocator.GetCurrentFreeSpace(s_pMemManPage->m_MemScope); break;

#ifdef _HAVE_DEBUG_MEMORY
	case MC_DEBUG:
		return s_pMemManPage->m_DebugAllocator.GetCurrentFreeSpace(s_pMemManPage->m_MemScope); break;
#endif

#ifdef _HAVE_MEMORY_TRACKING
	case MC_DEBUG_MEM_TRACKER:
		return s_pMemManPage->m_DebugMemTracker.GetCurrentFreeSpace(s_pMemManPage->m_MemScope); break;
#endif

	default:
		ntAssert(0);
		return 0;
	}
	
	// [scee_st] GCC bitches about use of this function when inlined if this isn't here --
	// looks like no-one has actually used it up to now!
	return 0;
}

//--------------------------------------------------
//!
//! GetFreeSpace
//!
//--------------------------------------------------
uint32_t GetBaseAddress( MEMORY_CHUNK chunk )
{
	switch( chunk )
	{
	case MC_RSX_MAIN_INTERNAL:
		return s_pMemManPage->m_RsxMainAllocator.GetBaseAddress(s_pMemManPage->m_MemScope); break;

	case MC_RSX_MAIN_USER:
		return s_pMemManPage->m_RsxMainUserAllocator.GetBaseAddress(s_pMemManPage->m_MemScope); break;

	case MC_GFX:
		return s_pMemManPage->m_GfxAllocator.GetBaseAddress( s_pMemManPage->m_MemScope); break;

	case MC_LOADER:
		return s_pMemManPage->m_LoaderAllocator.GetBaseAddress(s_pMemManPage->m_MemScope); break;

	case MC_ANIMATION:
		return s_pMemManPage->m_AnimationAllocator.GetBaseAddress(s_pMemManPage->m_MemScope); break;

	case MC_HAVOK:
		return s_pMemManPage->m_HavokAllocator.GetBaseAddress( s_pMemManPage->m_MemScope); break;

	case MC_AUDIO:
		return s_pMemManPage->m_AudioAllocator.GetBaseAddress(s_pMemManPage->m_MemScope); break;

#ifndef _COLLAPSE_SMALL_CHUNKS

	case MC_ODB:
		return s_pMemManPage->m_ODBAllocator.GetBaseAddress(s_pMemManPage->m_MemScope); break;

	case MC_ARMY:
		return s_pMemManPage->m_ArmyAllocator.GetBaseAddress(s_pMemManPage->m_MemScope); break;

	case MC_LUA:
		return s_pMemManPage->m_LuaAllocator.GetBaseAddress(s_pMemManPage->m_MemScope); break;

	case MC_ENTITY:
		return s_pMemManPage->m_EntityAllocator.GetBaseAddress(s_pMemManPage->m_MemScope); break;

	case MC_MISC:
		return s_pMemManPage->m_MiscAllocator.GetBaseAddress(s_pMemManPage->m_MemScope); break;

	case MC_AI:
		return s_pMemManPage->m_AIAllocator.GetBaseAddress(s_pMemManPage->m_MemScope); break;

	case MC_CAMERA:
		return s_pMemManPage->m_CameraAllocator.GetBaseAddress(s_pMemManPage->m_MemScope); break;

	case MC_EFFECTS:
		return s_pMemManPage->m_EffectsAllocator.GetBaseAddress(s_pMemManPage->m_MemScope); break;

	case MC_PROCEDURAL:
		return s_pMemManPage->m_ProceduralAllocator.GetBaseAddress(s_pMemManPage->m_MemScope); break;

#else // _COLLAPSE_SMALL_CHUNKS
	// allow all of the above to fall through to overflow
	case MC_ODB:
	case MC_ARMY:
	case MC_LUA:
	case MC_ENTITY:
	case MC_MISC:
	case MC_AI:
	case MC_CAMERA:
	case MC_EFFECTS:
	case MC_PROCEDURAL:
		chunk = MC_OVERFLOW;
#endif // _COLLAPSE_SMALL_CHUNKS

	case MC_OVERFLOW:
		return s_pMemManPage->m_OverflowAllocator.GetBaseAddress(s_pMemManPage->m_MemScope); break;

#ifdef _HAVE_DEBUG_MEMORY
	case MC_DEBUG:
		return s_pMemManPage->m_DebugAllocator.GetBaseAddress(s_pMemManPage->m_MemScope); break;
#endif

#ifdef _HAVE_MEMORY_TRACKING
	case MC_DEBUG_MEM_TRACKER:
		return s_pMemManPage->m_DebugMemTracker.GetBaseAddress(s_pMemManPage->m_MemScope); break;
#endif

	default:
		ntAssert(0);
		return 0;
	}
	return 0;
}

//--------------------------------------------------
//!
//! Turn of or on the logging of the tags (so you can find what you looking for)
//!
//--------------------------------------------------
void EnableMemoryTrackingLogging( bool bEnable )
{
#ifdef _HAVE_MEMORY_TRACKING
	s_pMemManPage->m_bEnableMemoryTrackingLog = bEnable;
#else
	UNUSED(bEnable);
#endif
}

#ifdef _HAVE_MEMORY_TRACKING
//--------------------------------------------------
//!
//!	Takes the memory tags and either dumps them to a file or stores them for compares.
//! 
//! the app.
//!	\param wType Allocating or Deallocating?
//! \param pTag main tag usually C file that made the call
//! \param pSubTag usually the function that made the call
//! \param iNum usually line in file that made the call
//! \param pPtr address allocated or freed
//! \param iSize size paramter if the call used one
//! \param bNoAllocs wether LogTag is allowed to make DebugPoolAllocs to store
//!			its tracking data. DebugPoolsAllocs themselve set this to true to stop
//!			reentracy
//!
//--------------------------------------------------
void LogTag(	const LOG_TYPE eType, 
				const MEMORY_CHUNK chunk, const MEMORY_CHUNK oldchunk, const char* pTag, const char* pSubTag, const int iNum, uintptr_t pPtr, const int iSize, 
				const bool bNoAllocs )
{
	if( bNoAllocs == true || s_pMemManPage->m_MemTrackerInitOK == false)
		return;

	ScopedCritical sc( s_pMemManPage->m_TrackerCritSec );

	// either insert a new tracker node or retrieve one... benign if we have no tracking info...
	if( eType == LT_ALLOC || eType == LT_MEMALIGN )
	{
		// if we have no logging tag, its not one of ours so we have nothing to work with... 
		// this makes me nervous as our memory stats will also be cocked up...
		if ( pTag == 0 )
			pTag = "Unknown allocation";

		ntAssert_p( pPtr != 0, ("Memory Allocation failed") );

#ifdef FILL_MEMORY
		// fill in our newly allcated memory to a nasty value
		memset( (uint8_t*) pPtr, 0xA9, iSize );
#endif

		if( eType == LT_ALLOC )
		{
			s_pMemManPage->m_FrameStats.sTotal.iNumAllocCalls++;
			s_pMemManPage->m_FrameStats.sTotal.iNumAllocedBytes += iSize;
			s_pMemManPage->m_FrameStats.sChunks[chunk].iNumAllocCalls++;
			s_pMemManPage->m_FrameStats.sChunks[chunk].iNumAllocedBytes += iSize;
		}
		else
		{
			s_pMemManPage->m_FrameStats.sTotal.iNumMemAlignCalls++;
			s_pMemManPage->m_FrameStats.sTotal.iNumMemAlignedBytes += iSize;
			s_pMemManPage->m_FrameStats.sChunks[chunk].iNumMemAlignCalls++;
			s_pMemManPage->m_FrameStats.sChunks[chunk].iNumMemAlignedBytes += iSize;
		}

		s_pMemManPage->m_FrameStats.sTotal.iNumBytesInUse += iSize;
		s_pMemManPage->m_FrameStats.sTotal.iNumBytesInUseHighWaterMark = 
			ntstd::Max( s_pMemManPage->m_FrameStats.sTotal.iNumBytesInUseHighWaterMark, s_pMemManPage->m_FrameStats.sTotal.iNumBytesInUse );

		s_pMemManPage->m_FrameStats.sChunks[chunk].iNumBytesInUse += iSize;
		s_pMemManPage->m_FrameStats.sChunks[chunk].iNumBytesInUseHighWaterMark = 
			ntstd::Max( s_pMemManPage->m_FrameStats.sChunks[chunk].iNumBytesInUseHighWaterMark, s_pMemManPage->m_FrameStats.sChunks[chunk].iNumBytesInUse );
		
		if( chunk != oldchunk )
		{
			s_pMemManPage->m_FrameStats.sChunks[oldchunk].iNumBytesOverflowInUse += iSize;
			s_pMemManPage->m_FrameStats.sChunks[oldchunk].iNumBytesOverflowHighWaterMark = 
				ntstd::Max( s_pMemManPage->m_FrameStats.sChunks[oldchunk].iNumBytesOverflowHighWaterMark, s_pMemManPage->m_FrameStats.sChunks[oldchunk].iNumBytesOverflowInUse );
		}

		// allocate a list node in debug memory
		(*s_pMemManPage->m_pMemTrackerMap)[ pPtr ] = AllocTracker( pTag, pSubTag, iNum, pPtr, iSize, chunk, oldchunk ); 

#if TRACK_MEMORY_LEVEL > 0
#if TRACK_MEMORY_LEVEL == 1
		if( !s_pMemManPage->m_bEnableMemoryTrackingLog )
			return;
#endif // end TRACK_MEMORY_LEVEL 1 (conditional logging of memory allocs)
		ntPrintf( "%s(%d) : TrackMemory Alloc in %s, memory block address = 0x%0X, size = %i bytes, pooled = %d\n", pTag, iNum, pSubTag, pPtr, iSize, ((iSize & 0x80000000)!=0) );
#endif // end TRACK_MEMORY_LEVEL 1 or 2 (log all allocs or conditionall log allocs)
	}
	else
	{
		// delete 0 allowed not tracking or logging to do
		if( pPtr == 0 )
			return;

		// scan debug list for alloc, free it if everything is O.K.
		// else dump a message to the log
		AllocTrackerMap::iterator itATM = s_pMemManPage->m_pMemTrackerMap->find( pPtr );

		if( itATM != s_pMemManPage->m_pMemTrackerMap->end() )
		{
			uint32_t iSizeRet = itATM->second.m_iSize;

			MEMORY_CHUNK chunkRet = (MEMORY_CHUNK) itATM->second.m_eChunk;
			MEMORY_CHUNK oldchunkRet = (MEMORY_CHUNK) itATM->second.m_eOldChunk;

			if ( eType == LT_FREE )
			{
#ifdef FILL_MEMORY
				// set freed memory to 0xD1
				memset( (uint8_t*) itATM->second.m_pPtr, 0xD1, iSizeRet );
#endif

				// now update stats
				s_pMemManPage->m_FrameStats.sTotal.iNumFreeCalls++;
				s_pMemManPage->m_FrameStats.sTotal.iNumFreedBytes += iSizeRet;
				s_pMemManPage->m_FrameStats.sTotal.iNumBytesInUse -= iSizeRet;
				s_pMemManPage->m_FrameStats.sChunks[chunkRet].iNumFreeCalls++;
				s_pMemManPage->m_FrameStats.sChunks[chunkRet].iNumFreedBytes += iSizeRet;
				s_pMemManPage->m_FrameStats.sChunks[chunkRet].iNumBytesInUse -= iSizeRet;
				if( chunkRet != oldchunkRet )
				{
					s_pMemManPage->m_FrameStats.sChunks[oldchunkRet].iNumBytesOverflowInUse -= iSizeRet;
				}
				// we found one so delete it
				s_pMemManPage->m_pMemTrackerMap->erase( itATM );
			}
			else	// SHRINK!!!
			{
				itATM->second.m_iSize = iSize;
				uint32_t	bytesToFree = iSizeRet - iSize;

				s_pMemManPage->m_FrameStats.sTotal.iNumFreedBytes += bytesToFree;
				s_pMemManPage->m_FrameStats.sTotal.iNumBytesInUse -= bytesToFree;
				s_pMemManPage->m_FrameStats.sChunks[chunkRet].iNumFreedBytes += bytesToFree;
				s_pMemManPage->m_FrameStats.sChunks[chunkRet].iNumBytesInUse -= bytesToFree;
				if( chunkRet != oldchunkRet )
				{
					s_pMemManPage->m_FrameStats.sChunks[oldchunkRet].iNumBytesOverflowInUse -= bytesToFree;
				}
			}
		}
		else
		{
			ntError_p( chunk == MC_DEBUG_MEM_TRACKER, ("Deleting item 0x%x from chunk (%s), that does not have mem tag, has been deleted already.", pPtr, GetChunkName(chunk) ) );
		}
	}
}
#endif

//--------------------------------------------------
//!
//! Takes a Memory checkpoint in _HAVE_MEMORY_TRACKING, used
//! for memory leak detection among other things
//!	\return an opaque handle to a copy of the mapped tracking data
//!
//--------------------------------------------------
uintptr_t TakeMemoryCheckpoint()
{
#if defined(_HAVE_MEMORY_TRACKING)
	AllocTrackerMap* pCheckpoint = (AllocTrackerMap*) s_pMemManPage->m_DebugAllocator.Alloc( MS_GLOBAL, sizeof(AllocTrackerMap) );
	NT_PLACEMENT_NEW(pCheckpoint) AllocTrackerMap();
	(*pCheckpoint) = (*s_pMemManPage->m_pMemTrackerMap);
	return (uintptr_t) pCheckpoint;
#else
	return 0;
#endif
}

void FreeMemoryCheckpoint( uintptr_t checkpoint )
{
#if defined(_HAVE_MEMORY_TRACKING)
	s_pMemManPage->m_DebugAllocator.Free( MS_GLOBAL, checkpoint );
#else
	UNUSED(checkpoint);
#endif
}


//--------------------------------------------------
//!
//! Compares the passed in memory checkpoint to the 
//! current memory state and dumps any differences 
//! between the two.
//! Default paramaters give a memory leak check marked 
//! with an identifier of NO IDENT
//!
//! \param a Memory checkpoint from TakeMemoryCheckpoint
//! \param pIdent bit of text to search for in log file
//! \param bLogLoseDeallocs Log frees that have OCCURED since TakeMemoryCheckpoint
//! \param bLogLoseAllocs Log allocations that have before TakeMemoryCheckpoint that haven't been freed (Memory leaks)
//!
//--------------------------------------------------
void DumpMemoryCheckpointDifference( uintptr_t checkpoint, 
													const char* pIdent, 
													const bool bLogLoseDeallocs, 
													const bool bLogLoseAllocs )
{
#if defined(_HAVE_MEMORY_TRACKING)
	ntAssert_p( checkpoint != 0, ("Invalid checkpoint") );
	ntAssert_p( (bLogLoseDeallocs == true) || (bLogLoseAllocs == true), ("Stupid parameters! that would be NOP") );
	AllocTrackerMap* pCheckpoint = (AllocTrackerMap*) checkpoint;
	AllocTrackerMap* pGlobalClone = (AllocTrackerMap*) TakeMemoryCheckpoint();

	ntPrintf("***********************************************************************************************\n");
	ntPrintf("DumpMemoryCheckpointDifference Ident : %s\n", pIdent );

	if( bLogLoseDeallocs )
	{
		ntPrintf("----------------------------------------------------------\n");
		ntPrintf("Allocations freed since TakeMemoryCheckpoint was called\n");
		ntPrintf("----------------------------------------------------------\n");
	}

	// look up every allocation in checkpoint and remove it from
	// the cloned globals list (log it as an extra deallocation if its not)
	AllocTrackerMap::const_iterator itATM = pCheckpoint->begin();
	while( itATM != pCheckpoint->end() )
	{
		AllocTrackerMap::iterator itGC = pGlobalClone->find( (*itATM).second.m_pPtr );
		if( itGC == pGlobalClone->end() )
		{
			// log that checkpoint has an allocation the global list hasn't
			// so something that was there has when TakeMemoryCheckpoint was 
			// called has been freed before DumpMemoryCheckpointDifference
			// was called
			if( bLogLoseDeallocs )
			{
				ntPrintf( "%s(%d) : ", (*itATM).second.m_pTag, (*itATM).second.m_iNum );
				ntPrintf( "%s, memory block address = 0x%0X\n", (*itATM).second.m_pSubTag, (*itATM).second.m_pPtr );
#ifdef MEMMAN_SAVE_STACK_TRACE
				DumpStackTrace( itATM -> second.m_stackTrace );
#endif
			}

		} else
		{
			pGlobalClone->erase( itGC );
		}
		++itATM;
	}

	if( bLogLoseAllocs )
	{
		ntPrintf( "------------------------------------------------------------------------------------------------\n");
		ntPrintf( "Allocations that have occured since TakeMemoryCheckpoint that haven't been free'd (memory leaks)\n");
		ntPrintf( "------------------------------------------------------------------------------------------------\n");
		// now walk the cloned global list and dump any thing left
		// as allocations
		AllocTrackerMap::const_iterator itGC = pGlobalClone->begin();
		while( itGC != pGlobalClone->end() )
		{
			// every item here was allocated after TakeMemoryCheckpoint but
			// wasn't free by the time DumpMemoryCheckpointDifference.
			// classically a memory leak
			ntPrintf( "%s(%d) : ", (*itGC).second.m_pTag, (*itGC).second.m_iNum );
			ntPrintf( "%s, memory block address = 0x%0X, size = %i bytes\n", (*itGC).second.m_pSubTag, (*itGC).second.m_pPtr, (*itGC).second.m_iSize );
#ifdef MEMMAN_SAVE_STACK_TRACE
			DumpStackTrace( itGC -> second.m_stackTrace );
#endif

			++itGC;
		}
	}

	// free up the temporary work checkpoint
	FreeMemoryCheckpoint( (uintptr_t)pGlobalClone );

	ntPrintf("***********************************************************************************************\n");
#else
	UNUSED(checkpoint);
	UNUSED(pIdent);
	UNUSED(bLogLoseDeallocs);
	UNUSED(bLogLoseAllocs);

	return;
#endif
}

//#endif

} // end namespace Mem
