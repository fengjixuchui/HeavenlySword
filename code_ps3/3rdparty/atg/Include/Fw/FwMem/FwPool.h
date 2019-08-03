//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Memory Pools

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef	FW_POOL_H
#define	FW_POOL_H

//--------------------------------------------------------------------------------------------------
/**
	@class			FwPool
	
	@brief			A templated class designed to manage pools of objects.

	A pool is a templated class holding a fixed size array of data elements	which can be allocated
	from the pool for use, and deallocated when finished with. It is useful for code which has
	varying demands (but with a defined limit) for small amounts of data storage, and in these cases
	is more efficient than straightforward dynamic allocation, and reduces fragmentation.

	Use of pools is simple.. create a pool, allocate items from it, deallocate items when you're 
	done with them, and destroy the pool when you're finished.

	@code
		{
			// Create a pool with 32 objects in it..
			FwPool<MyTestClass>	myTestPool( 32 );
		
			// Allocate an item from it..
			MyTestClass* pMyClass = myTestPool.Allocate();

			// Use the item
			pMyClass->MyFunction();

			// Deallocate the item
			myTestPool.Deallocate( pMyClass );

			// At this point, myTestPool() is about to go out of scope, and will be destroyed. All
			// allocations from the pool should have been deallocated by now.
		}
	@endcode

	Pools can optionally resort to dynamic allocation when empty. An optional parameter on both the
	non-default constructor and the Resize() method controls this behaviour, as is shown in the
	following example:

	@code
		// Create a pool with 256 integers, that will not dynamically allocate additional pool items
		FwPool<int>	myFixedIntPool( 256, false );	

		// Create a pool with 128 integers, that *may* dynamically allocate additional pool items
		FwPool<int> myDynamicIntPool( 128, true );

		// Create an empty pool of integers, and then use the Resize() function to make the pool
		// contain 64 items, and allow dynamic allocation of additional pool items
		FwPool<int> myOtherIntPool;
		myOtherIntPool.Resize( 64, true );
	@endcode

**/
//--------------------------------------------------------------------------------------------------

template<typename T>
class FwPool
{
public:
	// Construction & Destruction
	inline	FwPool();
	inline	FwPool( int capacity, bool allowHeapOverflow = false );
	inline	~FwPool();

	// Allocation & deallocation methods
	inline	T*		Allocate(  void );
	inline	void	Deallocate( T* pItem );

	// Resize the pool
	inline	void	Resize( int capacity, bool allowHeapOverflow = false );

	// Returns the number of free items in the pool.
	inline	int		GetNumFreeItems( void ) const;

	// Returns the number of used items (including overflows)
	inline	int		GetNumUsedItems( void ) const;

	// Returns the capacity of the pool
	inline	int		GetCapacity( void ) const;

	// Returns whether the pool allows heap overflow
	inline	bool	AllowsHeapOverflow( void ) const;

private:
	// Internal helpers
	inline	int		GetPoolIndex( const T* pItem ) const;
	inline	T*		GetPoolItem( int item ) const;
	inline	bool	IsFromPool( const T* pobItem ) const;

	char*			m_pStorage;					///< Storage for the items themselves.
	T**				m_pFreeItems;				///< The pool free items list.

    int				m_capacity;					///< The capacity of the pool (in elements, not bytes).
	int				m_numPoolAllocations;		///< The number of active allocations from the pool.
	int				m_numOverflowAllocations;	///< The total number of active allocations from the heap (i.e. overflow).

	bool			m_allowHeapOverflow;		///< Determines whether use of heap on overflow is allowed
};

//--------------------------------------------------------------------------------------------------
/**
	@brief			Initialises the pool with no storage, and with heap overflow disabled.

	@note			This is here to allow for static pools to exist, that have their states
					initialised later via a call to Resize().
**/
//--------------------------------------------------------------------------------------------------

template <typename T>
inline	FwPool<T>::FwPool()
{
	m_capacity					= 0;
	m_numPoolAllocations		= 0;
	m_numOverflowAllocations	= 0;

	m_pStorage					= 0;
	m_pFreeItems				= 0;

	m_allowHeapOverflow			= false;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Initialises the pool with a set number of pooled items. 

	@param			capacity			Number of pool items.
	@param			allowHeapOverflow	'true' if we allow allocation of items on the heap when the
										pool becomes full, else 'false'.

	@note			It is perfectly safe to call this function with a capacity of 0. In this case
					the pool will perform all further allocations on the heap.
**/
//--------------------------------------------------------------------------------------------------

template <typename T>
inline	FwPool<T>::FwPool( int capacity, bool allowHeapOverflow ) 
{
	m_capacity					= 0;
	m_numPoolAllocations		= 0;
	m_numOverflowAllocations	= 0;

	m_pStorage					= 0;
	m_pFreeItems				= 0;

	Resize( capacity, allowHeapOverflow );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Destroys the pool. 

	This function is responsible for releasing all allocations owned by the pool. Note that it is 
	not the responsibily of the destructor to destroy all currently allocated pool items. However,
	on debug builds we do report information about outstanding allocations. 
**/
//--------------------------------------------------------------------------------------------------

template <typename T>
inline	FwPool<T>::~FwPool()
{
	// check we've not got any outstanding allocations
	FW_ASSERT_MSG( m_numOverflowAllocations == 0, ( "%s: Cannot destruct when there are %d outstanding overflow allocations", __FUNCTION__, m_numOverflowAllocations ) );
	FW_ASSERT_MSG( m_numPoolAllocations == 0, ( "%s: Cannot destruct when there are %d outstanding pool allocations", __FUNCTION__, m_numPoolAllocations ) );
	
	// Deallocate all memory associated with the pool
	FW_DELETE_ARRAY( m_pStorage );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Resizes the pool to the given capacity

	@param			capacity			Number of pool items.
	@param			allowHeapOverflow	'true' if we allow allocation of items on the heap when the
										pool becomes full, else 'false'.

	@note			It is perfectly safe to call this function with a capacity of 0. In this case
					the pool will perform all further allocations on the heap.
**/
//--------------------------------------------------------------------------------------------------

template <typename T>
inline	void FwPool<T>::Resize( int capacity, bool allowHeapOverflow )
{
	// Check we've not got any outstanding pool allocations
	FW_ASSERT_MSG( m_numPoolAllocations == 0, ( "%s: Cannot resize when there are %d outstanding pool allocations", __FUNCTION__, m_numPoolAllocations ) );

	// Release our current storage
	FW_DELETE_ARRAY( m_pStorage );
	m_pStorage		= NULL;
	m_pFreeItems	= NULL;

	// reset our counters
	m_capacity = capacity;
	m_numPoolAllocations = 0;

	if ( m_capacity > 0 )
	{
		// Make a single allocation that contains all our storage & management information.
		m_pStorage		= FW_NEW char[ FwAlign( sizeof( T ) * m_capacity, 16 ) + ( sizeof( T* ) * m_capacity ) ];
	
		// Compute our pointers to other areas within the single allocation
		m_pFreeItems	= ( T** )( m_pStorage + FwAlign( sizeof( T ) * m_capacity, 16 ) );

		// Reset the pool
		for (int item = 0; item < m_capacity; item++)
			m_pFreeItems[ item ] = GetPoolItem( item );
	}

	// Save our heap overflow status
	m_allowHeapOverflow = allowHeapOverflow;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Allocates an item from the pool.
	
	Allocates an item from the pool, or - if no pool space is available - from the heap. On debug
	builds a warning is output to the console when we start allocating from the heap.

	@note			Pool items have their default constructors called on allocation.

	@note			On builds without assertions, this function *will* perform dynamic allocation
					when the pool becomes empty. It's better to do this, than to crash in retail
					product.

	@return			Pointer to pool item.
**/
//--------------------------------------------------------------------------------------------------

template <typename T>
inline	T*	FwPool<T>::Allocate( void )
{
	// Allocate out of the pool if we can (we use placement new)
	if ( m_numPoolAllocations < m_capacity )
	{
		// Find a free slot
		T* pItem = m_pFreeItems[ m_numPoolAllocations ];
		m_numPoolAllocations++;

		// placement new on it
		return FW_PLACEMENT_NEW( pItem ) T;
	}

#ifdef	ATG_DEBUG_MODE
	// Post a warning if we've just started allocating from the heap
	if ( m_numOverflowAllocations == 0 )
		FwPrintf( "WARNING!! %s: pool of %d elements is full, started allocating from the heap\n", __FUNCTION__, m_capacity );
#endif	// ATG_DEBUG_MODE

	// Otherwise allocate off the heap (if possible)
	FW_ASSERT_MSG( m_allowHeapOverflow, ( "%s: pool of %d elements is full, and heap overflow is disallowed\n", __FUNCTION__, m_capacity ) );
	m_numOverflowAllocations++;
	return FW_NEW T;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Deallocates an existing allocation 

	@param			pItem		Pointer to an item.

	@note			Pool items have their default destructors called on deallocation.
**/
//--------------------------------------------------------------------------------------------------

template <typename T>
inline	void FwPool<T>::Deallocate( T* pItem )
{
	if ( IsFromPool( pItem ) )
	{
		// If allocated from the pool storage then put back into the pool
		FW_ASSERT_MSG( m_numPoolAllocations != 0, ( "%s: Too many pool deallocations, the pool is already full", __FUNCTION__ ) );
		m_numPoolAllocations--;
		m_pFreeItems[ m_numPoolAllocations ] = pItem;
	
		// Destruct the item
		pItem->~T();
	}
	else
	{
		if ( pItem )
		{
			// Otherwise delete the allocation
			FW_ASSERT( m_numOverflowAllocations > 0 );
			m_numOverflowAllocations--;
			FW_DELETE( pItem );
		}
	}
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return the number of free items within the pool.

	@return			An integer representing the number of free items within the pool.
**/
//--------------------------------------------------------------------------------------------------

template <typename T>
inline	int	FwPool<T>::GetNumFreeItems( void ) const
{
	return m_capacity - m_numPoolAllocations;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return the number of used items within the pool, including overflow allocations

	@return			An integer representing the number of used items.
**/
//--------------------------------------------------------------------------------------------------

template <typename T>
inline	int	FwPool<T>::GetNumUsedItems( void ) const
{
	return m_numPoolAllocations + m_numOverflowAllocations;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return the capacity of the pool.

	@return			An integer representing the capacity of the pool.
**/
//--------------------------------------------------------------------------------------------------

template <typename T>
inline	int	FwPool<T>::GetCapacity( void ) const
{
	return m_capacity;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return whether the pool allows heap overflow.

	@return			true if the pool allows heap overflow, else false
**/
//--------------------------------------------------------------------------------------------------

template <typename T>
inline	bool	FwPool<T>::AllowsHeapOverflow( void ) const
{
	return m_allowHeapOverflow;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Internal function used to determine whether a pool item was allocated from 
					the main storage block, or whether it was allocated from the heap.

	@param			pItem			Pointer to a pool item.

	@return			Pointer to allocated memory.
**/
//--------------------------------------------------------------------------------------------------

template <typename T>
inline	bool FwPool<T>::IsFromPool( const T* pItem ) const
{
	const char* pcItemStorage = ( const char* )( pItem );
	return ( pcItemStorage >= &m_pStorage[ 0 ] ) && ( pcItemStorage < &m_pStorage[ m_capacity * sizeof( T ) ] );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Given a pointer to a pool item, this returns the index within the storage block.

	@param			pItem			Pointer to a pool item that resides within pool storage. This
									cannot be an item that was allocated on the heap due to pool
									overflow.

	@return			Index of the pool item.

	@note			This function assumes a pool that is no more than 2^32 elements in size, due to
					the use of an integer return value.
	
**/
//--------------------------------------------------------------------------------------------------

template <typename T>
inline	int	FwPool<T>::GetPoolIndex( const T* pItem ) const
{
	const char* pItemStorage = ( const char* )( pItem );
	FW_ASSERT_MSG( ( ( pItemStorage >= &m_pStorage[ 0 ] ) && ( pItemStorage < &m_pStorage[ m_capacity * sizeof( T ) ] ) ), ( "%s: Internal error: object not from the pool", __FUNCTION__ ) );

	int index = ( int )( ( pItemStorage - &m_pStorage[0] ) / sizeof( T ) );        
	return index;	
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns a pointer to a pool item based on a supplied index

	@param			itemIndex		Index within pool storage

	@return			Pointer to the pool object at index 'itemIndex' within the storage block.
**/
//--------------------------------------------------------------------------------------------------

template <typename T>
inline	T*	FwPool<T>::GetPoolItem( int itemIndex ) const
{
	FW_ASSERT( ( itemIndex >= 0 ) && ( itemIndex < m_capacity ) );
	return ( T* )( &m_pStorage[ itemIndex * sizeof( T ) ] );
}

#endif	// FW_POOL_H
