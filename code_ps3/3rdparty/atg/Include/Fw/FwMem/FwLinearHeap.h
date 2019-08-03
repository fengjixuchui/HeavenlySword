//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Simple Linear Heap

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef	FW_LINEAR_HEAP_H
#define	FW_LINEAR_HEAP_H

//--------------------------------------------------------------------------------------------------
/**
	@class			FwLinearHeap
	
	@brief			Simple heap that supports linear allocation from top or bottom of heap space.

	While use of our memory management system is encouraged, it's accepted that, from time to time,
	projects will require a simpler (but more rigid) allocation scheme. This particular system 
	performs linear allocation from both the low or high ends of the heap. No deallocation mechanism,
	other than the Reset/ResetLow/ResetHigh functions, is provided. 

	So, for an example 16Kb heap, when the FwLinearHeap object is created, we might have the
	following layout within the heap:

	@code

			---------------------------------------------------------------------
			|								16Kb								|
			---------------------------------------------------------------------
			^																	^
	   Current Low														   Current High

	@endcode

	If we then use AllocLow() to allocate 6Kb from the low end of the heap, then the heap state 
	changes to this:

	@code

			---------------------------------------------------------------------
			|  6Kb used low bytes	|				  10Kb Free					|
			---------------------------------------------------------------------
			^						^											^
		Returned				Current Low							    	Current High
	   Allocation 

	@endcode
			
	If we *then* use AllocHigh() to allocate 8 Kb from the high end of the heap, then the state
	changes to this:

	@code

			---------------------------------------------------------------------
			|  6Kb used low bytes	| 2Kb Free	|	  8Kb used high bytes		|
			---------------------------------------------------------------------
									^			^								
								   Low	  Returned Pointer
										  + Current High  

	@endcode

**/
//--------------------------------------------------------------------------------------------------

class	FwLinearHeap : public FwNonCopyable
{
public:
	// Constructors
	inline	FwLinearHeap( u32 heapSize );
	inline	FwLinearHeap( void* pHeapMemory, u32 heapSize );
	inline	FwLinearHeap();
	inline	~FwLinearHeap();

	// In-place construction
	inline	void	SetHeapParameters( u32 heapSize );
	inline	void	SetHeapParameters( void* pHeapMemory, u32 heapSize );

	// Manual advancing & alignment of low addresses
	inline	void*	GetLowAddress( void ) const;
	inline	void	AdvanceLowAddress( u32 byteSize );
    inline	void	AlignLowAddress( u32 alignment );

	// Core allocation methods
	inline	void*	AllocLow( u32 byteSize, u32 alignment = 4 );
	inline	void*	AllocHigh( u32 byteSize, u32 alignment = 4 );

	// Retrieval of heap statistics
	inline	u32		GetUsedBytesLow( void ) const;
	inline	u32		GetUsedBytesHigh( void ) const;
	inline	u32		GetUsedBytes( void ) const;
	inline	u32		GetFreeBytes( void ) const;

	// Reset methods
	inline	void	ResetLow( void );
	inline	void	ResetHigh( void );
	inline	void	Reset( void );

	// Accessors
	inline	u8*		GetHeapBase( void ) const;


private:

#ifdef	ATG_DEBUG_MODE
	static const u8	kClearValue = 0xcd;			///< Memory is filled with this on debug builds.
#endif // ATG_DEBUG_MODE
	
	u8*		m_pHeapBase;						///< Pointer to the base of the heap
	u8*		m_pHeapTop;							///< Pointer to the top of the heap

	u8*		m_pCurrentLow;						///< Pointer to working low heap pointer
	u8*		m_pCurrentHigh;						///< Pointer to working high heap pointer

	u32		m_heapSize;							///< Heap size (in bytes)
	bool	m_heapOwnsAllocation;				///< States whether the heap memory was allocated by this object

};

//--------------------------------------------------------------------------------------------------
/**
	@brief			Initialisation of empty FwLinearHeap object of user-specified size.

	@param			heapSize	Size of heap. Must be > 0, and must be a multiple of 4 bytes.

	@note			On DEBUG builds, memory is filled to kClearValue.
**/
//--------------------------------------------------------------------------------------------------

inline	void	FwLinearHeap::SetHeapParameters( u32 heapSize )
{
	// Validate that we haven't already been fully defined..
	FW_ASSERT( !m_pHeapBase );
	FW_ASSERT( heapSize > 0 );
	FW_ASSERT( FwIsAligned( heapSize, 4 ) );

	// Allocate some memory for the heap.. this heap owns its own memory allocation
	m_heapSize				= heapSize;
	m_pHeapBase				= FW_NEW u8[ heapSize ];
	m_pHeapTop				= m_pHeapBase + heapSize;
	m_heapOwnsAllocation	= true;

	// Set heap pointers up..
	m_pCurrentLow = m_pHeapBase;
	m_pCurrentHigh = m_pHeapTop;

#ifdef	ATG_DEBUG_MODE
	memset( m_pHeapBase, kClearValue, heapSize );
#endif // ATG_DEBUG_MODE
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Initialisation of empty FwLinearHeap object with user-specified size and memory

	@param			pHeapMemory		User allocated memory address for heap. Must be 16 byte aligned.

	@param			heapSize		Size of heap. Must be > 0, and must be a multiple of 4 bytes.

	@note			On DEBUG builds, memory is filled to kClearValue.
**/
//--------------------------------------------------------------------------------------------------

inline	void	FwLinearHeap::SetHeapParameters( void* pHeapMemory, u32 heapSize )
{
	// Validate that we haven't already been fully defined..
	FW_ASSERT( !m_pHeapBase );
	FW_ASSERT( pHeapMemory );
	FW_ASSERT( FwIsAligned( pHeapMemory, 16 ) );
	FW_ASSERT( heapSize > 0 );
	FW_ASSERT( FwIsAligned( heapSize, 4 ) );

	// We don't allocate memory for the heap here.. we're using external memory.
	m_heapSize				= heapSize;
	m_pHeapBase				= ( u8* )pHeapMemory;
	m_pHeapTop				= m_pHeapBase + heapSize;
	m_heapOwnsAllocation	= false;

	// Set heap pointers up..
	m_pCurrentLow = m_pHeapBase;
	m_pCurrentHigh = m_pHeapTop;

#ifdef	ATG_DEBUG_MODE
	memset( m_pHeapBase, kClearValue, heapSize );
#endif // ATG_DEBUG_MODE
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Construction of FwLinearHeap object of user-specified size.

	@param			heapSize	Size of heap. Must be > 0, and must be a multiple of 4 bytes.

	@note			On DEBUG builds, memory is filled to kClearValue.
**/
//--------------------------------------------------------------------------------------------------

inline	FwLinearHeap::FwLinearHeap( u32 heapSize )
	:
	m_pHeapBase(NULL),
	m_heapOwnsAllocation( false )
{
	SetHeapParameters( heapSize );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Construction of FwLinearHeap object with user-specified size and memory

	@param			pHeapMemory		User allocated memory address for heap. Must be 16 byte aligned.

	@param			heapSize		Size of heap. Must be > 0, and must be a multiple of 4 bytes.

	@note			On DEBUG builds, memory is filled to kClearValue.
**/
//--------------------------------------------------------------------------------------------------

inline	FwLinearHeap::FwLinearHeap( void* pHeapMemory, u32 heapSize )
	:
	m_pHeapBase(NULL),
	m_heapOwnsAllocation( false )
{
	SetHeapParameters( pHeapMemory, heapSize );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Default construction of FwLinearHeap.
**/
//--------------------------------------------------------------------------------------------------

inline	FwLinearHeap::FwLinearHeap()
	:
	m_pHeapBase(NULL),
	m_heapOwnsAllocation( false )
{
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Destruction of FwLinearHeap.

	@note			No checks as to whether memory is still referred to by other objects is
					performed. Memory is not released for heaps created with an external memory
					allocation.
**/
//--------------------------------------------------------------------------------------------------

inline	FwLinearHeap::~FwLinearHeap()
{
	if ( m_heapOwnsAllocation )
		FW_DELETE_ARRAY( m_pHeapBase );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the current low adddress for the heap.

	@return			Pointer holding current low address (towards the base of the heap)
**/
//--------------------------------------------------------------------------------------------------

inline	void*	FwLinearHeap::GetLowAddress( void ) const
{
	return m_pCurrentLow;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Advances the current low address for the heap by a number of bytes.

	@param			byteSize	Number of bytes to advance. Must be multiple of 4 bytes.
**/
//--------------------------------------------------------------------------------------------------

inline	void	FwLinearHeap::AdvanceLowAddress( u32 byteSize )
{
	FW_ASSERT( FwIsAligned( byteSize, 4 ) );
	FW_ASSERT( ( m_pCurrentLow + byteSize) <= m_pCurrentHigh );
	m_pCurrentLow += byteSize;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Align the current low address in the heap to the specified alignment.

	@param			alignment		Required alignment. Must be a power of two.

	@note			Empty space caused by the alignment process is not filled with zero.
**/
//--------------------------------------------------------------------------------------------------

inline	void	FwLinearHeap::AlignLowAddress( u32 alignment )
{
	FW_ASSERT( FwIsPow2( alignment ) );
    m_pCurrentLow = ( u8* )FwAlign( m_pCurrentLow, alignment);
	FW_ASSERT( m_pCurrentLow <= m_pCurrentHigh );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Allocates a number of bytes from the low end of the heap.

	@param			byteSize		Number of bytes to allocate. Must be multiple of 4 bytes.
	@param			alignment		Required alignment. Must be a power of two.

	@return			Pointer to allocated memory.
**/
//--------------------------------------------------------------------------------------------------

inline	void*	FwLinearHeap::AllocLow( u32 byteSize, u32 alignment )
{
	FW_ASSERT( FwIsPow2( alignment ) );
	FW_ASSERT( FwIsAligned( byteSize, 4 ) );

	m_pCurrentLow = (u8*) FwAlign( m_pCurrentLow, alignment );
	void*	pMemory = m_pCurrentLow; 
	FW_ASSERT( pMemory < m_pCurrentHigh );
	m_pCurrentLow += byteSize;
	FW_ASSERT( m_pCurrentLow <= m_pCurrentHigh );

	return pMemory;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Allocates a number of bytes from the high end of the heap.

	@param			byteSize		Number of bytes to allocate. Must be multiple of 4 bytes.
	@param			alignment		Required alignment. Must be a power of two.

	@return			Pointer to allocated memory.
**/
//--------------------------------------------------------------------------------------------------

inline	void*	FwLinearHeap::AllocHigh( u32 byteSize, u32 alignment )
{
	FW_ASSERT( FwIsPow2( alignment ) );
	FW_ASSERT( FwIsAligned( byteSize, 4 ) );

	m_pCurrentHigh -= byteSize;

	if ( !FwIsAligned( m_pCurrentHigh, alignment ) )
	{
		m_pCurrentHigh -= alignment;
		m_pCurrentHigh = ( u8* )FwAlign( m_pCurrentHigh, alignment );
	}

	FW_ASSERT( m_pCurrentHigh >= m_pCurrentLow );

	return m_pCurrentHigh;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the number of bytes used in the low end of the heap.

	@return			Number of bytes used.
**/
//--------------------------------------------------------------------------------------------------

inline	u32		FwLinearHeap::GetUsedBytesLow( void ) const
{
	return	( u32 )( m_pCurrentLow - m_pHeapBase );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the number of bytes used in the high end of the heap.

	@return			Number of bytes used.
**/
//--------------------------------------------------------------------------------------------------

inline	u32		FwLinearHeap::GetUsedBytesHigh( void ) const
{
	return	( u32 )( m_pHeapTop - m_pCurrentHigh );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the total number of bytes used in the heap.

	@return			Number of bytes used.
**/
//--------------------------------------------------------------------------------------------------

inline	u32		FwLinearHeap::GetUsedBytes( void ) const
{
	return ( u32 )( m_heapSize - ( m_pCurrentHigh - m_pCurrentLow ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the total number of bytes free in the heap.

	@return			Number of bytes used.
**/
//--------------------------------------------------------------------------------------------------

inline	u32		FwLinearHeap::GetFreeBytes( void ) const
{
	return ( u32 )( m_pCurrentHigh - m_pCurrentLow );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Resets the low end of the heap.

	@note			No checks as to whether memory in the low end of the heap is still being
					referred to by other objects.	

	@note			On DEBUG and DEVELOPMENT builds, memory is filled to kClearValue.
**/
//--------------------------------------------------------------------------------------------------

inline	void	FwLinearHeap::ResetLow( void )
{
#ifdef	ATG_DEBUG_MODE
	memset( m_pHeapBase, kClearValue, GetUsedBytesLow() );
#endif	// ATG_DEBUG_MODE

	m_pCurrentLow = m_pHeapBase;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Resets the high end of the heap.

	@note			No checks as to whether memory in the high end of the heap is still being
					referred to by other objects.	

	@note			On DEBUG and DEVELOPMENT builds, memory is filled to kClearValue.
**/
//--------------------------------------------------------------------------------------------------

inline	void	FwLinearHeap::ResetHigh( void )
{
#ifdef	ATG_DEBUG_MODE
	memset( m_pCurrentHigh, kClearValue, GetUsedBytesHigh() );
#endif // ATG_DEBUG_MODE

	m_pCurrentHigh = m_pHeapTop;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Resets the entire heap

	@note			No checks as to whether memory in the heap is still being referred to by other
					objects.	

	@note			On DEBUG and DEVELOPMENT builds, memory is filled to kClearValue.
**/
//--------------------------------------------------------------------------------------------------

inline	void	FwLinearHeap::Reset( void )
{
#ifdef	ATG_DEBUG_MODE
	memset( m_pHeapBase, kClearValue, m_heapSize );
#endif // ATG_DEBUG_MODE

	m_pCurrentLow = m_pHeapBase;
	m_pCurrentHigh = m_pHeapTop;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Get heap base.
	
	@return			Pointer to heap base.
**/
//--------------------------------------------------------------------------------------------------

inline	u8*		FwLinearHeap::GetHeapBase( void ) const
{
	return m_pHeapBase;
}

//--------------------------------------------------------------------------------------------------

#endif	// FW_LINEAR_HEAP_H
