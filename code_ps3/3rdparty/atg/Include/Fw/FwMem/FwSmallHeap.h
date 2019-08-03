//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Small Allocations Heap

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef	FW_SMALL_HEAP_H
#define	FW_SMALL_HEAP_H	

#include <Fw/FwMem/FwPool.h>

//--------------------------------------------------------------------------------------------------
/**
	@class			FwSmallHeap

	@brief			A project-configurable heap for efficient storage of small objects. The heap is
					constructed out of a number of 'slots', configured by the application. Each slot
					is responsible for managing allocations within a certain range of byte sizes.

	@note			This system is used to manage allocations by API STL components, such as lists,
					maps and so on.. 
**/
//--------------------------------------------------------------------------------------------------

class	FwSmallHeap
{
public:
	//! Used to define initialisation parameters for a particular heap type
	struct	SmallHeapDefinition
	{
		int		m_size;							///< The type of small heap	
		int		m_capacity;						///< Number of items storable within the heap type
		bool	m_allowHeapOverflow;			///< 'true' if global heap is to be used when area used by type is full
	};

	//! Structure used to pass information about a particular slot.
	struct	QuerySlotInfo
	{
		int		m_minSize;						///< Minimum size that this slot manages
		int		m_maxSize;						///< Maximum size allocation that this slot manages
		int		m_capacity;						///< Number of allocations of size 'maxSize' that this slot can manage.
		int		m_numSlotAllocations;			///< Number of allocations actually made from slot storage.
		int		m_numOverflowAllocations;		///< Number of allocations made from heap, due to being out of room.
		int		m_slotAllocatedSize;			///< Number of *requested bytes* that were serviced by this slot.
		bool	m_allowHeapOverflow;			///< 'true' if this slot can use the heap when out of room, else 'false'.
	};
	
	//! Typedef for query callback.
	typedef void		( QueryCallback )( const QuerySlotInfo& queryInfo, void* pUserData );
	
	// Construction & Destruction
	static	void		Initialise( const SmallHeapDefinition* pDefinitionTable, bool allowOversizedAllocations = true );
	static	void		Shutdown( void );
	
	// Allocation & deallocation methods
	static	void*		Allocate( int byteSize );
	static	void		Deallocate( void* pItem, int byteSize );

	// Query functionality
#ifdef	ATG_DEBUG_MODE
	static	void		QueryHeap( QueryCallback* pCallback, void* pUserData );
#else
	static	void		QueryHeap( QueryCallback*, void* ) {};
#endif	// ATG_DEBUG_MODE

protected:
	static	inline bool	IsInitialised( void );
	static	inline bool	IsSizeManaged( int byteSize );
	static	inline bool	IsFromSmallHeap( void* pItem );

private:
	static	inline	int	AlignSize( int byteSize );

	//! Internal data associated with a slot
	struct	SlotInformation
	{
		char*	m_pStorage;					///< Pointer to storage for this slots items
		char**	m_pFreeItems;				///< Pointer to pointers to free items

		int		m_capacity;					///< How many items this slot can contain without attempting overflow
		int		m_slotSize;					///< The size of the managed items that this slot deals with

		int		m_numSlotAllocations;		///< The number of allocations made from the slot
		int		m_numOverflowAllocations;	///< The number of allocations that were allocated as overflow from the heap.
	
		int		m_slotAllocatedSize;		///< The number of *requested bytes* that were serviced by this slot. Not necessarily (m_slotSize * m_capacity)!

		bool	m_allowHeapOverflow;		///< 'true' if this slot will allocate from heap when out of room, else 'false'

	};
	
	static	const int			kSlotGranularity = 4;				///< Slot granularity (size difference between each slot)
	
	static	int					ms_numSlots;						///< Number of slots that we possibly deal with.
	static	int					ms_numActiveSlots;					///< The number of slots that we're actually using
	static	int					ms_numOversizedAllocations;			///< The number of allocations from heap due to being too large.
	static	char*				ms_pMemoryArea;						///< Pointer to a single allocation containing all our data
	static	int					ms_memoryAreaSize;					///< The size of the allocation above..
	
	static	bool				ms_allowOversizedAllocations;		///< 'true' if we allow oversized allocations (silently allocating from the heap)
	
	static	SlotInformation**	ms_pSlotPointers;					///< Pointer to array of pointers to SlotInformation structures (ms_numberOfSlots big)
	static	SlotInformation*	ms_pSlotInformation;				///< Pointer to an array of SlotInformation structures (m_numberOfActiveSlots big)
};

//--------------------------------------------------------------------------------------------------
/**
	@brief			Determines whether an size is manageable by the small heap.

	@result			'true' if we'be been initialised, else 'false'
**/
//--------------------------------------------------------------------------------------------------

inline	bool	FwSmallHeap::IsInitialised( void )
{
	return ( ms_pMemoryArea != 0 );
}


//--------------------------------------------------------------------------------------------------
/**
	@brief			Determines whether an size is manageable by the small heap.

	@param			byteSize	Size (in bytes)

	@result			'true' if we can manage this allocation, else 'false'

	@note			Items that are allocated that have a size that's too large to be managed by the
					small heap are allocated from the global heap. 
**/
//--------------------------------------------------------------------------------------------------

inline	bool	FwSmallHeap::IsSizeManaged( int byteSize )
{
	return ( ( byteSize / kSlotGranularity ) < ms_numSlots );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Determines whether the item was allcoated from small heap, or was overflow.

	@param			pItem		Pointer to an item.

	@result			'true' if it's from the small heap, 'false' if it was a heap allocation caused
					by slot overflow.
**/
//--------------------------------------------------------------------------------------------------

inline	bool	FwSmallHeap::IsFromSmallHeap( void* pItem )
{
	return ( pItem >= ms_pMemoryArea ) && ( pItem < ( ms_pMemoryArea + ms_memoryAreaSize ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Aligns a size based on granularity of slot.

	@param			pItem		Pointer to an item.

	@result			'true' if it's from the small heap, 'false' if it was a heap allocation caused
					by slot overflow.
**/
//--------------------------------------------------------------------------------------------------

inline	int		FwSmallHeap::AlignSize( int byteSize )
{
	return FwAlign( ( u32 )byteSize, kSlotGranularity );
}

#endif	// FW_SMALL_HEAP_H		
