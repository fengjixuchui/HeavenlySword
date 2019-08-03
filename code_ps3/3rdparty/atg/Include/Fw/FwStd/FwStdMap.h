//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Wrapped version of STL's 'std::map' and 'std::multimap' containers that use our custom allocator

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef	FW_STD_MAP_H
#define	FW_STD_MAP_H

#include	<map>
#include	<Fw/FwStd/FwStdAllocator.h>

namespace   FwStd
{

//--------------------------------------------------------------------------------------------------
/**
	@class			FwMap
	
	@brief			A wrapped version of 'std::map' with a custom STL allocator that uses FwSmallHeap
					for memory allocations.		
**/
//--------------------------------------------------------------------------------------------------

template< class K, class T, class P= std::less< K >, class A = FwStd::Allocator< std::pair< const K, T > > >
class	Map : public std::map< K, T, P, A >
{
public:
	// Standard types
	typedef typename std::map< K, T, P, A >::allocator_type		allocator_type;
	typedef typename std::map< K, T, P, A >::value_type			value_type;
	typedef typename std::map< K, T, P, A >::size_type			size_type;
	typedef typename std::map< K, T, P, A >::key_compare		key_compare;

	// All our constructors have to call the base constructors on 'std::map'.. so, this is what all these are.
	Map() :														std::map<K,T,P,A>()					{}
	explicit Map(const key_compare& Pred) :						std::map<K,T,P,A>( Pred )			{}
	Map(const key_compare& Pred, const allocator_type& Al) :	std::map<K,T,P,A>( Pred, Al )		{}

	template<class Iter>
		Map(Iter F, Iter L) :									std::map<K,T,P,A>( F, L )			{}

	template<class Iter>
		Map(Iter F, Iter L,
		const key_compare& Pred) :								std::map<K,T,P,A>( F, L, Pred )		{}

	template<class Iter>
		Map(Iter F, Iter L,
		const key_compare& Pred, const allocator_type& Al) :	std::map<K,T,P,A>( F, L, Pred, Al )	{}
};


//--------------------------------------------------------------------------------------------------
/**
	@class			FwMultiMap
	
	@brief			A wrapped version of 'std::multimap' with a custom STL allocator that uses FwSmallHeap
					for memory allocations.		
**/
//--------------------------------------------------------------------------------------------------

template< class K, class T, class P= std::less< K >, class A = FwStd::Allocator< std::pair< const K, T > > >
class	MultiMap : public std::multimap< K, T, P, A >
{
public:
	// Standard types
	typedef typename std::multimap< K, T, P, A >::allocator_type	allocator_type;
	typedef typename std::multimap< K, T, P, A >::value_type		value_type;
	typedef typename std::multimap< K, T, P, A >::size_type			size_type;
	typedef typename std::multimap< K, T, P, A >::key_compare		key_compare;

	// All our constructors have to call the base constructors on 'std::map'.. so, this is what all these are.
	MultiMap() :													std::multimap<K,T,P,A>()					{}
	explicit MultiMap(const key_compare& Pred) :					std::multimap<K,T,P,A>( Pred )				{}
	MultiMap(const key_compare& Pred, const allocator_type& Al) :	std::multimap<K,T,P,A>( Pred, Al )			{}

	template<class Iter>
		MultiMap(Iter F, Iter L) :									std::multimap<K,T,P,A>( F, L )				{}

	template<class Iter>
		MultiMap(Iter F, Iter L,
		const key_compare& Pred) :									std::multimap<K,T,P,A>( F, L, Pred )		{}

	template<class Iter>
		MultiMap(Iter F, Iter L,
		const key_compare& Pred, const allocator_type& Al) :		std::multimap<K,T,P,A>( F, L, Pred, Al )	{}
};

}

#endif	// FW_STD_MAP_H
