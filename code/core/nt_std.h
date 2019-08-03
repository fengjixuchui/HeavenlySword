//-----------------------------------------------------------------------------------------
//! 
//! \filename core\nt_std.h
//! A header to wrap our usage of the C++ ntstd. use ntstd::List instead of nstd::list for example
//! 
//-----------------------------------------------------------------------------------------

#if !defined CORE_NT_STD_H
#define CORE_NT_STD_H

#include "ntstdallocator.h"
#include <Fw/FwStd/FwStdAllocator.h>
#include <Fw/FwStd/FwStdList.h>
#include <Fw/FwStd/FwStdMap.h>

#include <iterator>
#include <set>

#ifdef PLATFORM_PC
#	include "core/ntstd_vector.h"
#else
#	include <vector>
#endif

#include <string>
#include <iterator>
#include <algorithm>
#include <fstream>
#include <numeric>

#include <ctype.h>

//-----------------------------------------------------------------------------------------
//! 
//!	ntstd that uses custom version approved for use in the game itself
//! 
//-----------------------------------------------------------------------------------------
namespace ntstd
{
	// algorithm, could be optimised?
	using std::advance;
	using std::sort;
	using std::for_each;
	using std::find;
	using std::distance;
	using std::transform;
	using std::iter_swap;
	using std::less;
	using std::make_pair;

	// added for alexhey
	using std::binary_function;
	using std::accumulate;
	using std::equal_range;

	// functional prob leave as is?
	using std::mem_fun;


	// List, Map and MultiMap from ATG's FW but we have our our default allocator so I've copied them...
	//--------------------------------------------------------------------------------------------------
	/**
		@class			List
	**/
	//--------------------------------------------------------------------------------------------------

	template< class T, int C = Mem::MC_MISC, class A = allocator< T, C > >
	class	List : public std::list< T, A >
	{
	public:
		// for GET_CONTAINER_MEMORY_CHUNK macro
		enum MemChunkTrait
		{
			memChunk_ = C
		};

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
		List()																	: std::list<T,A>()					{};
		explicit List( const allocator_type& Al )								: std::list<T,A>( Al )				{};
		explicit List( size_type Count )										: std::list<T,A>( Count )			{};
		List( size_type Count, const T& Val )									: std::list<T,A>( Count, Val )		{};
		List( size_type Count, const T& Val, const allocator_type& Al )			: std::list<T,A>( Count, Val, Al )	{};

		template< class Iter >
		List( Iter F, Iter L )													: std::list<T,A>( F, L )			{};
			
		template< class Iter >
		List( Iter F, Iter L, const allocator_type& Al )						: std::list<T,A>( F, L, Al )		{};
	};

	//--------------------------------------------------------------------------------------------------
	/**
		@class			FwMap
	**/
	//--------------------------------------------------------------------------------------------------
	template< class K, class T, class P = less< K >, int C = Mem::MC_MISC, class A = allocator< std::pair< const K, T >, C > >
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
	**/
	//--------------------------------------------------------------------------------------------------

	template< class K, class T, class P = less< K >, int C = Mem::MC_MISC, class A = allocator< std::pair< const K, T >, C > >
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

	//--------------------------------------------------------------------------------------------------
	/**
		@class			List
	**/
	//--------------------------------------------------------------------------------------------------

	template< class T, int C = Mem::MC_MISC, class A = allocator< T, C > >
	class	Vector : public std::vector< T, A >
	{
	public:
		// Standard types
		typedef typename std::vector< T, A >::allocator_type	allocator_type;
		typedef	typename std::vector< T, A >::value_type		value_type;
		typedef typename std::vector< T, A >::size_type		size_type;
		typedef typename std::vector< T, A >::difference_type	difference_type;
		typedef	typename std::vector< T, A >::pointer			pointer;
		typedef	typename std::vector< T, A >::const_pointer	const_pointer;
		typedef	typename std::vector< T, A >::reference		reference;
		typedef	typename std::vector< T, A >::const_reference	const_reference;

		// All our constructors have to call the base constructors on 'std::list'.. so, this is what all these are.
		Vector()																	: std::vector<T,A>()					{};
		explicit Vector( const allocator_type& Al )									: std::vector<T,A>( Al )				{};
		explicit Vector( size_type Count )											: std::vector<T,A>( Count )				{};
		Vector( size_type Count, const T& Val )										: std::vector<T,A>( Count, Val )		{};
		Vector( size_type Count, const T& Val, const allocator_type& Al )			: std::vector<T,A>( Count, Val, Al )	{};

		template< class Iter >
		Vector( Iter F, Iter L )													: std::vector<T,A>( F, L )				{};
			
		template< class Iter >
		Vector( Iter F, Iter L, const allocator_type& Al )							: std::vector<T,A>( F, L, Al )			{};
	};

	//--------------------------------------------------------------------------------------------------
	/**
		@class			Set
	**/
	//--------------------------------------------------------------------------------------------------

	template< class T, class P = less< T >, int C = Mem::MC_MISC, class A = allocator< T, C > >
	class	Set : public std::set< T, P, A >
	{
	public:
		// Standard types
		typedef typename std::set< T, P, A >::allocator_type	allocator_type;
		typedef	typename std::set< T, P, A >::value_type		value_type;
		typedef typename std::set< T, P, A >::size_type		size_type;
		typedef typename std::set< T, P, A >::difference_type	difference_type;
		typedef	typename std::set< T, P, A >::pointer			pointer;
		typedef	typename std::set< T, P, A >::const_pointer	const_pointer;
		typedef	typename std::set< T, P, A >::reference		reference;
		typedef	typename std::set< T, P, A >::const_reference	const_reference;

		// All our constructors have to call the base constructors on 'std::list'.. so, this is what all these are.
		Set()																	: std::set<T,P,A>()					{};
		explicit Set( const allocator_type& Al )									: std::set<T,P,A>( Al )				{};
		explicit Set( size_type Count )											: std::set<T,P,A>( Count )			{};
		Set( size_type Count, const T& Val )										: std::set<T,P,A>( Count, Val )		{};
		Set( size_type Count, const T& Val, const allocator_type& Al )			: std::set<T,P,A>( Count, Val, Al )	{};

		template< class Iter >
		Set( Iter F, Iter L )													: std::set<T,P,A>( F, L )			{};
			
		template< class Iter >
		Set( Iter F, Iter L, const allocator_type& Al )						: std::set<T,P,A>( F, L, Al )		{};
	};

	using std::iterator;
	using std::reverse_iterator;

	using std::pair;
	using std::inserter;

	// our string type to be 'improved' a lot
	typedef std::basic_string<char, std::char_traits<char>, allocator< char, Mem::MC_MISC > > String;
	// io to be replaced
	typedef std::basic_istream<char, std::char_traits<char> > Istream;
	typedef std::basic_ostream<char, std::char_traits<char> > Ostream;
	typedef std::basic_ifstream<char, std::char_traits<char> > Ifstream;
	typedef std::basic_ofstream<char, std::char_traits<char> > Ofstream;
	typedef std::basic_istringstream<char, std::char_traits<char>, allocator< char, Mem::MC_MISC > > Istringstream;
	typedef std::basic_ostringstream<char, std::char_traits<char>, allocator< char, Mem::MC_MISC > > Ostringstream;

	using std::endl;

	template<typename T> inline T Min(T const& a, T const& b)					{ return (a < b) ? a : b; }
	template<typename T> inline T Max(T const& a, T const& b)					{ return (a > b) ? a : b; }
	template<typename T> inline T Clamp( T const& v, T const& mn, T const& mx )	{ return Max( Min( v, mx ), mn ); }


	//! cross platform namespaced tolower function
	inline int Tolower( int chr )
	{
		return tolower( chr );
	}

	inline int ConvertSlash( int ch )
	{
		if ( ch == (int)'/' )
			return (int)'\\';

		return ch;
	}
};

#endif // end nt_std
