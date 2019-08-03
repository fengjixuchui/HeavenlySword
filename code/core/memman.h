#if !defined(CORE_MEMMAN_H)
#define CORE_MEMMAN_H
//---------------------------------------
//!
//!	\file core\memman.h
//! The memory manager the only place to
//! uses OS level memory calls and handles the scope
//! segmented memory model I've decided on
//!
//! SUPER C:
//! This is a 'super' C implementation, it should be 
//! treated as C code using C++ style syntax and not 
//! true C++ code. In particular, ctors must not be 
//! required, nor dtors (they can exist for convience but
//! must not be required). No vtables and types must be PODs
//! \see http://kong:8080/confluence/display/CH/SuperC
//!
//---------------------------------------

#if !defined( CORE_MEM_H )
#include "core/mem.h"
#endif

//---------------------------------------
//!
//!	Contains the real meat of the memory system, 
//! this is a static due to Singleton calling delete itself...
//! manually call Init() ASAP after creation. 
//! and Kill() last of all
//!
//---------------------------------------
namespace Mem
{
	//! The stats of the base machine before the memory allocator kicked into gear
	struct BaseMemStats
	{
		uint32_t iInitialXddrFree;	//!< How much was main (XDDR) left after load (OS, exe, statics etc.)
		uint32_t iInitialGddrFree;	//!< How much graphics (Gddr) mem is initially availible
		uint32_t iInitialDebugFree;	//!< How much debug ram we have
		uint32_t iInitialUsed;		//!< How much is used by the SELF
	};

	// this structure is return in two places, one representing the global memory picture
	// the other the per frame picture, so 'reset' is either per frame or per game
	struct MemSubStats
	{
		uint32_t iNumAllocCalls;		//!< total number of allocs since reset
		uint32_t iNumFreeCalls;			//!< total number of frees since reset
		uint32_t iNumMemAlignCalls;		//!< total number of memaligns since reset

		//---------
		// NOTE
		// BYTE level data is only maintained while full memory tracking is on...
		//---------

		// note per frame this can be negative...
		int32_t iNumBytesInUse;					//!< current number of bytes alloc/memaligned ed and not yet freed since reset
		int32_t iNumBytesInUseHighWaterMark;	//!< the highest iNumBytesInUse has ever got since reset
		int32_t iNumBytesOverflowInUse;			//!< current number of bytes that have overflowed, so should be in this chunk but aren't recored here
		int32_t iNumBytesOverflowHighWaterMark;	//!< the highest number of bytes that have overflowed, so should be in this chunk but aren't recored here

		uint32_t iNumAllocedBytes;		//!< total bytes alloc'ed 
		uint32_t iNumFreedBytes;		//!< total bytes freed
		uint32_t iNumMemAlignedBytes;	//!< total bytes memaligned
		
		uint32_t iOriginalAllocationSize;	//!< total bytes allocated for chunk at memory system initialisation
	};

	struct MemStats
	{
		MemSubStats				sTotal;								//!< the totals for all pools
		MemSubStats				sChunks[Mem::MC_NUM_CHUNKS];		//!< per pool memory states
	};


	//! start her up
	void Init();
	//! turn her off
	void Kill();

	//! does the memory manager exists?
	bool ManagerExists();

	//! Begin the frame, use to maintain per-frame memory stats and sum the global 
	void FrameReset();

	//! return the memory allocation stats for the current frame
	const MemStats& GetFrameStats();

	//! return the memory allocation stats for the game as a whole this may be a frame late... (its summed during FrameReset)
	const MemStats& GetMemStats();

#if !defined( _HAVE_MEMORY_TRACKING )
	//! normal alloc alignment is only guareenteed to be natural 
	//! Heaps have a minimum of 16 byte alignments but others type do not
	uintptr_t Alloc( MEMORY_CHUNK chunk, uint32_t iSize );

	//! free a memory block
	void Free( MEMORY_CHUNK chunk, uintptr_t pMem );

	uintptr_t Realloc( MEMORY_CHUNK chunk, uintptr_t pMem, uint32_t iSize );

	//! alloc a block with the specified alignment
	uintptr_t MemAlign( MEMORY_CHUNK chunk, uint32_t iSize, uint32_t iAlign, bool assert_on_failure = true );
#else
	//! normal alloc alignment is only guareenteed to be natural 
	//! Heaps have a minimum of 16 byte alignments but others type do not
	uintptr_t Alloc( MEMORY_CHUNK chunk, uint32_t iSize, const char* pTag, const char* pSubTag, int iNum );

	//! free a memory block
	void Free( MEMORY_CHUNK chunk, uintptr_t pMem, const char* pTag, const char* pSubTag, int iNum );

	//! Beware! Can only shrink an existing memory block at the moment!
	uintptr_t Realloc( MEMORY_CHUNK chunk, uintptr_t pMem, uint32_t iSize, const char* pTag, const char* pSubTag, int iNum );

	//! alloc a block with the specified alignment
	uintptr_t MemAlign( MEMORY_CHUNK chunk, uint32_t iSize, uint32_t iAlign, const char* pTag, const char* pSubTag, int iNum, bool assert_on_failure = true );
	
	//! Dumps everything out to memory.log file. WARNING: takes ages.
	void DumpMemoryTrackerMap();

	//! Dumps just broad stats about how much allocation is done in my memory chunks
	void DumpSimpleChunkStats();

#endif

	void EnableMemoryTrackingLogging( bool bEnable );

	//! takes an checkpoint of the memory system.
	uintptr_t TakeMemoryCheckpoint();

	//! release the memory used by a memory checkpoint
	void FreeMemoryCheckpoint( uintptr_t checkpoint );

	//! compares the checkpoint passed in to the current memory system and dumps the differences.
	void DumpMemoryCheckpointDifference( uintptr_t checkpoint, 
										const char* pIdent = "NO IDENT", 
										const bool bLogLoseDeallocs  = false, 
										const bool bLogLoseAllocs = true );

	//! returns the amount of free space of the given chunk
	uint32_t GetFreeSpace( MEMORY_CHUNK chunk );

	//! Get the base memeory (memory offset 0x0) of the chunk (used for RSX stuff)
	uintptr_t GetBaseAddress( MEMORY_CHUNK chunk );

	//! return the current memory scope
	MEMORY_SCOPE GetCurrentMemoryScope();

	//! changes the current memory scope
	void SetCurrentMemoryScope( MEMORY_SCOPE scope );

	//! returns the initial before we used any memory stats
	const BaseMemStats& GetBaseMemStats();

	//! internal call to get the initial OS mem (called in Init)
	void InitOsMem( uintptr_t* ppXddr, uint32_t* pXddrSize,
					uintptr_t* ppGddr, uint32_t* pGddrSize,
					uintptr_t* ppDebug, uint32_t* pDebugSize );

	void FreeOsMem( uintptr_t pXddr, uintptr_t pGddr, uintptr_t pDebug );

#ifdef PLATFORM_PS3
	//! Memory container wrappers, this is just enough to get us up and running
	//! we make no attempt to share container memory with the main game
	static const uint32_t MAX_CONTAINER_SIZE = ( 8*Mb );
	
	uint32_t	MemoryContainer_Create( uint32_t size );
	void		MemoryContainer_Destroy( uint32_t id );
	void		MemoryContainer_GetSize( uint32_t id, uint32_t& total, uint32_t& avail );
#endif
};

#endif // end CORE_MEMMAN_H
