//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Custom STL-compliant allocator that utilises FwSmallHeap functionality

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef FW_STD_ALLOCATOR_H
#define FW_STD_ALLOCATOR_H

#include <Fw/FwMem/FwSmallHeap.h>

namespace	FwStd
{

//--------------------------------------------------------------------------------------------------
/**
	@class			Allocator
	
	@brief			A custom STL allocator that uses FwSmallHeap for allocations.		
	
	One of the major drawbacks of STL is that, by default, it can make a large number of small
	allocations - especially when dealing with lists, sets, and maps. These small allocations 
	fragment memory, and can waste large amounts of system memory due to the management information
	that goes with a particular allocation. For some STL objects it's possible to replace the
	allocator with one more suited for use on a console. 

	So, this particular implementation of an allocator utilises the FwSmallHeap memory allocation
	system to ensure that small allocations are efficiently managed via the use of pools. 

	Here are some examples as to how this allocator can be used..

	@code
		// All list node allocations will use FwSmallHeap pools
		std::list< int, FwStd::Allocator< int > > myListOfInts;
	@endcode

	@code
		// You can typedef to make things clearer if you want to...
		typedef	FwStd::Allocator< int >				MyIntAllocator;	
		typedef	std::list< int, MyIntAllocator >	MyIntList;

		// Now use our typedef'd list of ints..
		MyIntList	myListOfInts;
	@endcode

	@code
		// Map allocations can use FwSmallHeap too..
		std::map< int, int, std::less< int >, FwStd::Allocator< int > > mySampleMap
	@endcode
**/
//--------------------------------------------------------------------------------------------------

// Base class for generic allocators for 'T'
template< typename T >
struct	_Allocator_base
{	
	typedef	T	value_type;
};

// Base class for generic allocators for 'const T'
template< typename T >
struct	_Allocator_base< const T >
{
	typedef	T	value_type;
};

template< typename T >
class Allocator : public _Allocator_base< T >
{
public:
	typedef	_Allocator_base< T >			_MyBase;			//!< Which allocator base we used (internal use only)
	typedef	typename _MyBase::value_type	value_type;			//!< This will be a non-const version of T
	typedef size_t							size_type;			//!< A type that can represent the size of the largest object in the allocation model.
	typedef ptrdiff_t						difference_type;	//!< A type that can represent the difference between any two pointers in the allocation model.
	typedef value_type*						pointer;			//!< Pointer to T;
	typedef const value_type*				const_pointer;		//!< Pointer to const T.
	typedef value_type&						reference;			//!< Reference to T.
	typedef const value_type&				const_reference;	//!< Reference to const T.

	//! Default constructor
	Allocator()
	{
	}

	//! Copy constructor
	Allocator( const Allocator< T >& )
	{
	}

	//! Creates an allocator to the argument's pool
	template< typename U >
	Allocator( const Allocator< U >& )
	{
	}

	//! Destructor
	~Allocator()
	{
	}

	//! A structure to construct an allocator for a different type
	template< typename U >
	struct rebind
	{
		typedef Allocator< U > other;
	};

	//! Returns the address of the given reference
	pointer address( reference rElement ) const
	{   
		return &rElement;
	}

	//! Returns the address of the given reference
	const_pointer address( const_reference rElement ) const
	{
		return &rElement;
	}

	//! Constructs an element of T at the given pointer
	void construct( pointer pElement, const T& rVal )
	{
		FW_PLACEMENT_NEW( ( void* )pElement ) T( rVal );
	}

	//! Destroys an element of T at the given pointer
	void destroy( pointer pElement )
	{
		#ifdef	_MSC_VER
		pElement;			// This is due to a bug in VS.NET 2003.. it believes p is unused.		
		#endif	//_MSC_VER
		( pElement )->~T(); // in-place destruction
	}

	//! The largest value that can meaningfully passed to allocate.
	size_t max_size() const
	{
		size_t count = (size_t)( -1 ) / sizeof ( T );
		return( 0 < count ? count : 1 );
	}

	//! Memory is allocated for 'count' objects of type 'T' but objects are not constructed.
	pointer allocate( size_type count )
	{
		return allocate( count, NULL );
	}

	//! Memory is allocated for 'count' objects of type 'T' but objects are not constructed.
	pointer allocate( size_type count, const void* )
	{
		return ( pointer )FwSmallHeap::Allocate( count * sizeof( T ) );
	}

	//! Deallocates memory allocated by allocate. All objects must have been previously destroyed,
	//! and 'count' must match the value passed to 'allocate' to obtain the memory.
	void deallocate( pointer pElement, size_type count )
	{
		FwSmallHeap::Deallocate( pElement, count * sizeof( T ) );
	}

private:
	//! Disallow assignment..
	Allocator< T >& operator = ( const Allocator< T >& );
};

//!	Allocator standard template operators
template< typename T, typename U >
inline bool operator == ( const Allocator< T >&, const Allocator< U >& )
{
	return true;
}

template< typename T, typename U >
inline bool operator !=( const Allocator< T >&, const Allocator< U >& )
{
	return false;
}


//--------------------------------------------------------------------------------------------------
/**
	@class			Allocator<void>
	
	@brief			Specialisation of FwStd::Allocator for 'void' type.. see the main documentation
					for Allocator for more information.	
**/
//--------------------------------------------------------------------------------------------------

template<>
class Allocator< void >
{
public:
	typedef void		value_type;			//!< Identical to void.
	typedef void*		pointer;			//!< Pointer to void.
	typedef const void*	const_pointer;		//!< Pointer to const void.

	// ! Default constructor
	Allocator()
	{
	}

	//! Creates an allocator to the argument's pool
	template< typename U >
	Allocator( const Allocator< U >& )
	{
	}

	//! A struct to construct an allocator for a different type.
	template< typename U >
	struct rebind
	{
		typedef Allocator< U > other;
	};
};

}	// FwStd

#endif	// FW_STD_ALLOCATOR_H
