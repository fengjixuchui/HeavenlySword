//------------------------------------------------------------
//!
//! \file core/memorypool.h
//!	A memory pool system for pooled allocatation.
//!
//------------------------------------------------------------

#if !defined(CORE_MEMORYPOOL_H)
#define CORE_MEMORYPOOL_H

/*

//! policy to use the pointer to block allocator (overhead 4 bytes per element on 32 address systems)
struct MemoryPoolLargeItemPolicy{};
//! policy to use a bitmap block allocator (overhead 8 bytes per 64 elements (1 bit per element) )
struct MemoryPoolSmallItemPolicy{};

//! base memory pool, pick up policy to complete the type
template< typename SIZE_POLICY>
class MemoryPool
{
private:
	//! Allocates a pool with size elements each of granularity bytes.
	MemoryPool( size_t granularity, size_t size );
	
	//! Checks for emptiness before destructing.
	~MemoryPool();

	//! Allocates memory from the pool without construction.
	void* Allocate();

	//! Deallocates memory from the pool without destruction.
	void Deallocate( void* pBlock );

	//! Constructs an object from the pool.	
	template<typename T>
		T* Construct()
		{
			T* pBlock = reinterpret_cast<T*>( Allocate() );
			return NT_PLACEMENT_NEW( pBlock ) T;
		}

	//! Destructs an object back into the pool.
	template<typename T>
		void Destroy( T* pInstance )
		{
			pInstance->~T();
			Deallocate( pInstance );
		}
};

//--------------------------------------------------
//!
//!	A pool of largish memory objects to be used/reused.
//! Handles overflow gracefully by falling back to 
//! default allocator. Has an overhead of ptr size (4 bytes)
//! per item so isn't suited to small items (see SmallItemMemoryPool)
//!
//--------------------------------------------------
template<> class MemoryPool<MemoryPoolLargeItemPolicy>
{
public:
	//! Allocates a pool with size elements each of granularity bytes.
	MemoryPool( size_t granularity, size_t size, uintptr_t memAddr )   : 
		m_iGranularity( granularity ), 
		m_iSize( size ), 
		m_iUsed( 0 ), 
		m_iOverflow( 0 ),
		m_iMaxOverflow( 0 )
	{
		if( m_iSize > 0 )
		{
			m_aStorage = (char*)memAddr;
			m_aSlots = (void**) (m_aStorage+m_iSize*m_iGranularity);

			for( size_t i = 0; i < m_iSize; ++i )
				m_aSlots[i] = reinterpret_cast<void*>( m_aStorage + i*m_iGranularity );
		}
	}
	
	//! used to tell you have much ram a particular size memorypool requires
	static uint32_t CalculateMemorySizeRequired( size_t granularity, size_t size )
	{
		return (sizeof(char)*size*granularity) + (sizeof(void*) * size);
	}

	//! Checks for emptiness before destructing.
	~MemoryPool()
	{
		ntAssert_p( m_iUsed == 0, ("can't destroy a pool with outstanding allocations") );
		ntAssert_p( m_iOverflow == 0, ("can't destroy a pool with outstanding allocations" ) );
	}

	//! Gets the pool granularity.
	size_t GetGranularity() const { return m_iGranularity; }

	//! Gets the pool size in elements.
	size_t GetSize() const { return m_iSize; }

	//! Gets the number of elements allocated from pooled storage.
	size_t GetUsed() const { return m_iUsed; }

	//! Gets the number of elements allocated from non-pooled storage.
	size_t GetOverflow() const { return m_iOverflow; }

	//! Gets the maximum amount we overflowed by this session
	size_t GetMaxOverflow() const { return m_iMaxOverflow; }


	//! Allocates memory from the pool without construction.
	void* Allocate()
	{
		if( m_iUsed < m_iSize )
		{
			return m_aSlots[m_iUsed++];
		}
		else
		{
			// in the final game we should never overflow, but as we have in development
			// alloc off the debug heap and mark the maximum size we grew too so we can
			// bump things up
			++m_iOverflow;
			m_iMaxOverflow = ntstd::Max( m_iMaxOverflow, m_iOverflow );
			return reinterpret_cast<void*>( NT_NEW_ARRAY_CHUNK(Mem::MC_DEBUG) char[m_iGranularity] );
		}
	}

	//! Deallocates memory from the pool without destruction.
	void Deallocate( void* pBlock )
	{
		ntAssert_p( pBlock, ("null pointer argument") );

		if( IsFromPool( pBlock ) )
		{
			ntAssert_p( m_iUsed > 0, ("internal error") );
			m_aSlots[--m_iUsed] = pBlock;
		}
		else
		{
			ntAssert_p( m_iOverflow > 0, ("attempting to free overflow without any in use" ) );
			NT_DELETE_ARRAY_CHUNK( Mem::MC_DEBUG, reinterpret_cast<char*>( pBlock ) );
			--m_iOverflow;
		}
	}


	//! Constructs an object from the pool.	
	template<typename T>
		T* Construct()
		{
			ntAssert( sizeof( T ) <= m_iGranularity );
			T* pBlock = reinterpret_cast<T*>( Allocate() );
			return NT_PLACEMENT_NEW( pBlock ) T;
		}

	//! Destructs an object back into the pool.
	template<typename T>
		void Destroy( T* pInstance )
		{
			ntAssert( sizeof( T ) <= m_iGranularity );
			pInstance->~T();
			Deallocate( pInstance );
		}
	//! Returns true if the given instance is from pooled storage.
	bool IsFromPool( void const* pInstance ) const
	{
		char const* pBlock = reinterpret_cast<char const*>( pInstance );
		
		return( (m_aStorage <= pBlock) && 
				(pBlock < ( m_aStorage + m_iSize*m_iGranularity )) );
	}

private:

	size_t m_iGranularity;	//!< The size of each element in the pool in bytes.
	size_t m_iSize;			//!< The number of elements in pooled storage.
	size_t m_iUsed;			//!< The number of pooled allocations.
	size_t m_iOverflow;		//!< The number of non-pooled allocations.
	size_t m_iMaxOverflow;	//!< The maximum number of non-pooled allocations.

	char* m_aStorage;	//!< The pool storage.
	void** m_aSlots;		//!< The free list.
};


//--------------------------------------------------
//!
//!	A pool of small memory objects to be used/reused.
//! Overflow is 'safe' but extremely inefficient and shouldn't
//! be relied on production code, increase pool size if overflow
//! occurs.
//! Using a bit allocator tracker, so 8 bytes per 64 objects overhead
//! Uses a incremental tracking algo, that has a should have a good usual
//! speed but possible bad worse case.
//! Only allocates in chunks of 64 for not really useful for small groups
//! of thems.
//!
//--------------------------------------------------
template<> class MemoryPool<MemoryPoolSmallItemPolicy>
{
public:
	//! Allocates a pool with size elements each of granularity bytes.
	MemoryPool( size_t granularity, size_t size, uintptr_t memAddr )   : 
		m_iGranularity( granularity ), 
		m_iSize( size ), 
		m_iUsed( 0 ), 
		m_iOverflow( 0 ),
		m_iMaxOverflow( 0 )
	{
		if( m_iSize > 0 )
		{
			// we only allocate in groups of 64 elements
			m_iSize = (m_iSize+(64-1)) & ~(64-1);

			m_aStorage = (char*)memAddr;
			m_aSlots = (uint64_t*) (m_aStorage+m_iSize*m_iGranularity);

			for( size_t i = 0; i < m_iSize / sizeof(uint64_t); ++i )
				m_aSlots[i] = 0;

			m_iFirstFree = 0;
		}
	}
	//! used to tell you have much ram a particular size memorypool requires
	static uint32_t CalculateMemorySizeRequired( size_t granularity, size_t size )
	{
		return (sizeof(char)*size*granularity) + (sizeof(void*) * size);
	}

	//! Checks for emptiness before destructing.
	~MemoryPool()
	{
		ntAssert_p( m_iUsed == 0, ("can't destroy a pool with outstanding allocations") );
		ntAssert_p( m_iOverflow == 0, ("can't destroy a pool with outstanding allocations" ) );

	}


	//! Gets the pool granularity.
	size_t GetGranularity() const { return m_iGranularity; }

	//! Gets the pool size in elements.
	size_t GetSize() const { return m_iSize; }

	//! Gets the number of elements allocated from pooled storage.
	size_t GetUsed() const { return m_iUsed; }

	//! Gets the number of elements allocated from non-pooled storage.
	size_t GetOverflow() const { return m_iOverflow; }

	//! Gets the maximum amount we overflowed by this session
	size_t GetMaxOverflow() const { return m_iMaxOverflow; }

	//! Allocates memory from the pool without construction.
	void* Allocate()
	{
		if( m_iUsed < m_iSize )
		{
			return AllocFirstFree();
		}
		else
		{
			// in the final game we should never overflow, but as we have in development
			// alloc off the debug heap and mark the maximum size we grew too so we can
			// bump things up
			++m_iOverflow;
			m_iMaxOverflow = ntstd::Max( m_iMaxOverflow, m_iOverflow );
			return reinterpret_cast<void*>( NT_NEW_ARRAY_CHUNK(Mem::MC_DEBUG) char[m_iGranularity] );
		}
	}

	//! Deallocates memory from the pool without destruction.
	void Deallocate( void* pBlock )
	{
		ntAssert_p( pBlock, ("null pointer argument") );

		if( IsFromPool( pBlock ) )
		{
			ntAssert_p( m_iUsed > 0, ("internal error") );
			DeallocItem( reinterpret_cast<char*>( pBlock ) );
		}
		else
		{
			ntAssert_p( m_iOverflow > 0, ("attempting to free overflow without any in use" ) );
			NT_DELETE_ARRAY_CHUNK( Mem::MC_DEBUG, reinterpret_cast<char*>( pBlock ) );
			--m_iOverflow;
		}
	}

	//! Constructs an object from the pool.	
	template<typename T>
		T* Construct()
		{
			ntAssert( sizeof( T ) <= m_iGranularity );
			T* pBlock = reinterpret_cast<T*>( Allocate() );
			return NT_PLACEMENT_NEW( pBlock ) T;
		}

	//! Destructs an object back into the pool.
	template<typename T>
		void Destroy( T* pInstance )
		{
			ntAssert( sizeof( T ) <= m_iGranularity );
			pInstance->~T();
			Deallocate( pInstance );
		}
	//! Returns true if the given instance is from pooled storage.
	bool IsFromPool( void const* pInstance ) const
	{
		char const* pBlock = reinterpret_cast<char const*>( pInstance );
		
		return( (m_aStorage <= pBlock) && 
				(pBlock < ( m_aStorage + m_iSize*m_iGranularity )) );
	}

private:
 
// cocked up 082 system headers
#ifndef UINT64_MAX
#define UINT64_MAX	0xffffffffffffffffULL
#endif

#ifndef UINT64_C
#define UINT64_C(x) ((x) + (UINT64_MAX - UINT64_MAX))
#endif

	//! allocate the first free chunk
	char* AllocFirstFree()
	{
		uint64_t bitblock = m_aSlots[m_iFirstFree];
		int bit = FindInBlock( bitblock );
		ntAssert( bit != -1 );
	
		// mark the bit in use
		bitblock |= (UINT64_C(1) << bit);
		m_aSlots[m_iFirstFree] = bitblock;

		// compute the address this block is at
		char* addr = (m_iFirstFree * m_iGranularity * 64) + (bit * m_iGranularity) + m_aStorage;

		// 64 bit registers agogo. This will suck big time on Win32 but should still work
		while( (bitblock == UINT64_MAX ) && m_iFirstFree < m_iSize / sizeof(uint64_t) )
		{
			m_iFirstFree++;
			bitblock = m_aSlots[m_iFirstFree];
		}

		m_iUsed++;
		return addr;
	}

	//! giving a 64 bit bit block returns first free bit
	int FindInBlock( uint64_t bitblock )
	{
		// this is ripe for a bit of ASM or an intrinsic!
		for( unsigned int i = 0; i < 64;i++)
		{
			if( (bitblock & (UINT64_C(1) << i)) == 0 )
			{
				return (int) i;
			}
		}
		// didn't find a free bit
		return -1;
	}

	//! frees the item passed from the bitmap allocator
	void DeallocItem( char* ptr )
	{
		// convert pointer in a block and bit address
		uint32_t offset = ptr - m_aStorage;
		uint32_t bitblockaddr = offset / (m_iGranularity * 64);
		int bit = (offset / m_iGranularity) - (bitblockaddr * 64);
		ntAssert( bit >=0 && bit < 64 );

		// mark the bit free
		m_aSlots[bitblockaddr] &= ~(UINT64_C(1) << bit);

		// update first free if nessecary
		if( bitblockaddr < m_iFirstFree )
		{
			m_iFirstFree = bitblockaddr;
		}

		m_iUsed--;
	}

	size_t m_iGranularity;	//!< The size of each element in the pool in bytes.
	size_t m_iSize;			//!< The number of elements in pooled storage.
	size_t m_iUsed;			//!< The number of pooled allocations.
	size_t m_iOverflow;		//!< The number of non-pooled allocations.
	size_t m_iMaxOverflow;	//!< The maximum number of non-pooled allocations.

	char*		m_aStorage;		//!< The pool storage.
	uint64_t*	m_aSlots;		//!< The free list.
	uint32_t	m_iFirstFree;	//!< index of bitblock where the first free allocation is
};
*/

#endif // end CORE_MEMORY_POOL_H

