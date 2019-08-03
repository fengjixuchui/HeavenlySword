//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Wrapped version of STL's 'std::list' container that uses our custom allocator

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef	FW_STD_LIST_H
#define	FW_STD_LIST_H

#include	<list>
#include	<Fw/FwStd/FwStdAllocator.h>

namespace	FwStd
{

//--------------------------------------------------------------------------------------------------
/**
	@class			List
	
	@brief			A wrapped version of 'std::list' with a custom STL allocator that uses FwSmallHeap
					for memory allocations.		
**/
//--------------------------------------------------------------------------------------------------

template< class T, class A = FwStd::Allocator< T > >
class	List : public std::list< T, A >
{
public:
	// Standard types
	typedef typename std::list< T, A >::allocator_type	allocator_type;
	typedef	typename std::list< T, A >::value_type		value_type;
	typedef typename std::list< T, A >::size_type		size_type;
	typedef typename std::list< T, A >::difference_type	difference_type;
	typedef	typename std::list< T, A >::pointer			pointer;
	typedef	typename std::list< T, A >::const_pointer	const_pointer;
	typedef	typename std::list< T, A >::reference		reference;
	typedef	typename std::list< T, A >::const_reference	const_reference;

	// All our constructors have to call the base constructors on 'std::list'.. so, this is what all these are.
	List()																	: std::list<T,A>()					{}
	explicit List( const allocator_type& Al )								: std::list<T,A>( Al )				{}
	explicit List( size_type Count )										: std::list<T,A>( Count )			{}
	List( size_type Count, const T& Val )									: std::list<T,A>( Count, Val )		{}
	List( size_type Count, const T& Val, const allocator_type& Al )			: std::list<T,A>( Count, Val, Al )	{}

	template< class Iter >
	List( Iter F, Iter L )													: std::list<T,A>( F, L )			{}
		
	template< class Iter >
	List( Iter F, Iter L, const allocator_type& Al )						: std::list<T,A>( F, L, Al )		{}
};

}

#endif	// FW_STD_LIST_H
