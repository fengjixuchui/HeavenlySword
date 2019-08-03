//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Memory Debugging

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef	FW_MEM_DEBUG_H
#define	FW_MEM_DEBUG_H

#ifdef	ATG_MEMORY_DEBUG_ENABLED

//--------------------------------------------------------------------------------------------------
/**
	@class			FwMemDebug
	
	@brief			Primary interface for framework memory management system.

    The framework includes a replacement memory manager that will, in future, be tuned to the 
	requirements of the target platform. At the very least, allocations need to be 16-byte aligned,
	but there will be a need to handle fragmenting heaps in future versions of this system. 

	@warning		The internal structure of this subsystem is likely to undergo extensive change
					in future. Please don't rely on undocumented internal behaviour.
**/
//--------------------------------------------------------------------------------------------------

class	FwMemDebug
{
public:

	// This structure is what gets passed to the user when calling QueryAllocations().
	struct	AllocationInfo
	{
		void*		pMemory;
		int			blockSize;
		const char*	pTag;
		s16			lineNumber;
		s16			contextIndex;
	};

	typedef void		( QueryAllocationsCallback )( const AllocationInfo& allocInfo, void* pUserData );

	// System initialisation
	static	void		Initialise( void );

	// Dump functionality
	static	void		QueryAllocations( QueryAllocationsCallback* pCallback, void* pUserData );

	// Debug callbacks for alllocation 
	static	void*		DefaultAllocCallback( u32 size, u32 alignment, const char* pTag, short lineNumber );
	static	void		DefaultFreeCallback( void* pAddress );

	// Retrieval of summary statistics
	static	size_t		GetCurrentAllocationTotal( void )	{ return ms_currentAlloc; }
	static	size_t		GetCurrentAllocationCount( void )	{ return ms_currentAllocCount; }
	static	size_t		GetLargestAllocationTotal( void )	{ return ms_maxAlloc; }
	static	size_t		GetLargestAllocationCount( void )	{ return ms_maxAllocCount; }

private:
	// These bits are for debug only...
	static const unsigned char	kNoMansLandFill = 0xFD;	// fill no-man's land with this
	static const unsigned char	kAlignLandFill  = 0xED;	// fill no-man's land for aligned routines
	static const unsigned char	kDeadLandFill   = 0xDD;	// fill free objects with this
	static const unsigned char	kCleanLandFill  = 0xCD;	// fill new objects with this
	static const int			kNoMansLandSize	= 4;	// size of no-man's land area

	///! We pack a line number and a context index into 32-bits. This is a safe way of doing just that..
	union	MemLineAndContext
	{
		s32		lineAndContext;
		struct	
		{
			s16		lineNumber;
			s16		contextIndex;
		} Fields;
	};

	struct MemBlockHeader
	{												
		struct MemBlockHeader*	pBlockHeaderNext;				// This structure is 32 bytes on PC,
		struct MemBlockHeader*	pBlockHeaderPrev;				// and 48 on PS3. Please do not rely
		size_t					dataSize;						// on the structures being the same
		const char*				pStringTag;						// size

		unsigned int			numericTag;
		int						requestNumber;					
		int						futureExpansion;						
		unsigned char			gap[ kNoMansLandSize ];
		
		// followed by:
		//	unsigned char			data[ dataSize ]
		//	unsigned char			anotherGap[ kNoMansLandSize ]
	};

	// If MemBlockHeader isn't a multiple of 16 bytes, we want to know about it at compile time..
	FW_STATIC_ASSERT( ( sizeof( MemBlockHeader ) % 16 ) == 0 );

	// Helpers to get access to header/data from data/header pointers.	
	static	MemBlockHeader*	GetHeader( void* pDataAddress )
	{
		return ( ( MemBlockHeader* )pDataAddress ) - 1;
	}

	static	unsigned char*	GetData( void* pBlockAddress )
	{
		return ( ( unsigned char* )( ( MemBlockHeader* )pBlockAddress + 1 ) );
	}

	// Internal querying system
	struct	QueryParameters
	{
		QueryAllocationsCallback*	pCallback;
		void*						pUserData;
	};
	static	void	InternalQueryCallback( void* pMemoryBlock, void* pContext );

	// We manage our list of debug allocations using these pointers
	static	MemBlockHeader*	ms_pFirstBlock;
	static	MemBlockHeader*	ms_pLastBlock;

	// Memory statistics
	static	size_t			ms_totalAlloc;			// Grand total - sum of all allocations
	static	size_t			ms_currentAlloc;		// Total amount currently allocated
	static	size_t			ms_currentAllocCount;	// Number of allocations present
	static	size_t			ms_maxAlloc;			// Largest ever allocated at once
	static	size_t			ms_maxAllocCount;		// Largest ever number of allocations present

	// Request debugging
	static	int				ms_currentRequest;		// Current request number
	static  int				ms_breakOnAlloc;		// Break on allocation by request number
};

#endif		// ATG_MEMORY_DEBUG_ENABLED

#endif	// FW_MEM_DEBUG_H
