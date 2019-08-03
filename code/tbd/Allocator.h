/***************************************************************************************************
*
*	DESCRIPTION		Allocator classes for AlignedVector.h.
*
*	NOTES
*
***************************************************************************************************/

#ifndef ALLOCATOR_H_
#define ALLOCATOR_H_

//**************************************************************************************
//	Includes files.
//**************************************************************************************
#include "tbd/TypeTraits.h"
#include "tbd/TypeList.h"

//**************************************************************************************
//	Forward declarations.
//**************************************************************************************

//**************************************************************************************
//	
//**************************************************************************************
namespace Allocator
{
	static const int InfiniteItemCount = -1;
}

//**************************************************************************************
//	
//**************************************************************************************
template < typename type >
struct HeapAllocator
{
	static void *	allocate	( int size, HeapAllocator & )
	{
		return static_cast< void * >( NT_NEW char[ size ] );
	}

	static void		deallocate	( void *mem )
	{
		NT_DELETE_ARRAY( static_cast< char * >( mem ) );
	}

	static const bool	PreventAllocation	= false;
	static const int	MaxItemCount		= Allocator::InfiniteItemCount;
};

//**************************************************************************************
//	
//**************************************************************************************
template
<
	typename	type,
	int			MaxCount
>
struct StackAllocator
{
	static void *	allocate	( int size, StackAllocator &curr_allocator )
	{
		ntError_p( size < (int)( MaxCount*sizeof( type ) ), ("You must specify a correct MaxSize parameter.") );
		UNUSED( size );
		return static_cast< void * >( &( curr_allocator.m_Memory[ 0 ] ) );
	}

	static void		deallocate	( void * )
	{
		// Do nothing - allocation is on the stack.
	}

	static const bool	PreventAllocation	= true;				// Must be true as we can't reallocate memory.
	static const int	MaxItemCount		= MaxCount;

	private:
		// This is the memory we return for use.
		// If we need any higher alignment than 16 then change the line below.
		ALIGNTO_PREFIX( 16 ) char m_Memory[ MaxCount*sizeof( type ) ] ALIGNTO_POSTFIX( 16 );
};

//**************************************************************************************
//	
//**************************************************************************************
template
<
	typename	type,
	int			MaxCount,
	int			UniqueID			// This is needed so that created two statically allocated objects of the same type and size don't use the same memory.
>
struct StaticAllocator
{
	static void *	allocate	( int size, StaticAllocator & )
	{
		ntError_p( size < (int)( MaxCount*sizeof( type ) ), ("You must specify a correct MaxSize parameter.") );
		UNUSED( size );
		return static_cast< void * >( &( m_Memory[ 0 ] ) );
	}

	static void		deallocate	( void * )
	{
		// Do nothing - allocation is static.
	}

	static const bool	PreventAllocation	= true;
	static const int	MaxItemCount		= MaxCount;

	private:
		// This is the memory we return for use.
		// If we need any higher alignment than 16 then change the line below.
		ALIGNTO_PREFIX( 16 ) static char m_Memory[ MaxCount*sizeof( type ) ] ALIGNTO_POSTFIX( 16 );
};

template < typename type, int MaxCount, int UniqueID >
char StaticAllocator< type, MaxCount, UniqueID >::m_Memory[ MaxCount*sizeof( type ) ];

#endif	// !ALLOCATOR_H_

