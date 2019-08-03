/***************************************************************************************************
*
*	DESCRIPTION		An dynamically sized, doubling, array type that will cope with aligned types.
*
*	NOTES
*
***************************************************************************************************/

#ifndef ALIGNEDVECTOR_H_
#define ALIGNEDVECTOR_H_

//**************************************************************************************
//	Includes files.
//**************************************************************************************
#include "tbd/TypeTraits.h"
#include "tbd/TypeList.h"
#include "tbd/TypeSelect.h"
#include "tbd/Allocator.h"
#include "tbd/ArrayCopier.h"

//**************************************************************************************
//	Forward declarations.
//**************************************************************************************

//**************************************************************************************
//	
//**************************************************************************************
template
<
	typename	type,
	class		allocator	= HeapAllocator< type >,
	class		copierT		= PlacementNewCopier< type >
>
class AlignedVector : private allocator
{
	public:
		//
		//	Some typedefs.
		//
		typedef typename TypeTraits< const type >::ParameterType		parameter_type;

		// Work out which copier to use - if the standard copier was specified then see if we
		// can automatically switch to the faster one.
		static const bool	IsStandardCopier	=	SameType< PlacementNewCopier< type >, copierT >::result;

		static const bool	CanUseMemcpyToCopy	=	TypeTraits< type >::IsFundamental ||
													TypeTraits< type >::IsPointer;

		typedef	typename	TypeSelect< CanUseMemcpyToCopy,
										MemcpyCopier< type >,
										PlacementNewCopier< type > >::ResultType	AutoChosenCopier;
		typedef	typename	TypeSelect< IsStandardCopier,
										AutoChosenCopier,
										copierT >::ResultType				copier;

		static const bool	IsUsingMemcpy		=	SameType< MemcpyCopier< type >, copier >::result;

	public:
		//
		//	Iterators.
		//
		typedef	type *			iterator;
		typedef const type *	const_iterator;

		iterator		begin	()						{ return reinterpret_cast< type * >( m_Memory ); }
		iterator		end		()						{ return reinterpret_cast< type * >( m_Memory + m_NumItems*TypeSize ); }

	public:
		//
		//	Add/Remove elements.
		//
		void	push_back	( parameter_type item )
		{
			if ( m_NumItems == m_MaxNumItems )
				double_array();

			ntError( m_NumItems < m_MaxNumItems );
			NT_PLACEMENT_NEW( m_Memory + TypeSize * m_NumItems ) type( item );

			m_NumItems++;
		}

				// NOTE: Does not preserve order - moves the last element into the deleted one's space.
		void	quick_erase		( int i )
		{
			ntError_p( i < m_NumItems, ("Out of bounds.") );

			reinterpret_cast< type * >( m_Memory + i*TypeSize )->~type();
			NT_PLACEMENT_NEW( reinterpret_cast< type * >( m_Memory + i*TypeSize ) ) type( *reinterpret_cast< type * >( m_Memory + ( m_NumItems - 1 )*TypeSize ) );
			reinterpret_cast< type * >( m_Memory + ( m_NumItems - 1 )*TypeSize )->~type();

			m_NumItems--;
		}

				// NOTE: Does not preserve order - moves the last element into the deleted one's space.
		void	quick_erase		( iterator it )
		{
			ntError_p( it >= reinterpret_cast< type * >( m_Memory ), ("Out of bounds.") );
			ntError_p( it <= reinterpret_cast< type * >( m_Memory + ( m_NumItems - 1 )*TypeSize ), ("Out of bounds.") );

			it->~type();
			NT_PLACEMENT_NEW( reinterpret_cast< type * >( it ) ) type( *reinterpret_cast< type * >( m_Memory + ( m_NumItems - 1 )*TypeSize ) );
			reinterpret_cast< type * >( m_Memory + ( m_NumItems - 1 )*TypeSize )->~type();

			m_NumItems--;
		}

				// NOTE: STL compliant, order preserving, erase function.
		iterator	erase			( iterator it )
		{
			ntError_p( it >= reinterpret_cast< type * >( m_Memory ), ("Out of bounds.") );
			ntError_p( it <= reinterpret_cast< type * >( m_Memory + ( m_NumItems - 1 )*TypeSize ), ("Out of bounds.") );

			it->~type();

			for ( iterator shuf_it=it;(int32_t)shuf_it<(int32_t)m_Memory + (m_NumItems-1)*TypeSize; shuf_it++ )
			{
				NT_PLACEMENT_NEW( reinterpret_cast< type * >( shuf_it ) ) type( *( shuf_it + 1 ) );
				( shuf_it + 1 )->~type();
			}

			m_NumItems--;

			return it;
		}

		void	clear			()
		{
			m_NumItems = 0;

			for ( int32_t i=0;i<m_NumItems;i++ )
			{
				( reinterpret_cast< type * >( m_Memory ) + i )->~type();
			}
		}

	public:
		//
		//	Return the number of items currently in the array.
		//
		int32_t		size		()		const
		{
			return m_NumItems;
		}

		//
		//	Return the maximum number of items currently available to this array without having to resize.
		//
		int32_t		max_size	()		const
		{
			return m_MaxNumItems;
		}

	public:
		//
		//	Accessors.
		//
		type &			operator []	( int i )			{ ntError_p( i < m_NumItems, ("Out of bounds.") ); return *reinterpret_cast< type * >( m_Memory + TypeSize*i ); }
		const type &	operator []	( int i )	const	{ ntError_p( i < m_NumItems, ("Out of bounds.") ); return *reinterpret_cast< const type * >( m_Memory + TypeSize*i ); }

	public:
		AlignedVector		( int reserve_count = 8 )
		:	m_Memory		( NULL )
		,	m_MaxNumItems	( 0 )
		,	m_NumItems		( 0 )
		{
			reserve( reserve_count );
		}

		AlignedVector		( const AlignedVector &copy )
		:	m_Memory		( NULL )
		,	m_MaxNumItems	( 0 )
		,	m_NumItems		( 0 )
		{
			*this = copy;
		}

		AlignedVector		( const type *data, int num_objects )
		:	m_Memory		( NULL )
		,	m_MaxNumItems	( 0 )
		,	m_NumItems		( 0 )
		{
			reserve( num_objects );

			for ( int i=0;i<num_objects;i++ )
			{
				NT_PLACEMENT_NEW( m_Memory + i*TypeSize ) type( data[ i ] );
			}

			m_NumItems = num_objects;
		}

		~AlignedVector		()
		{
			destroy();

			m_MaxNumItems = 0;
			m_NumItems = 0;
		}

		AlignedVector &	operator = ( const AlignedVector &rhs )
		{
			if ( this == &rhs )
			{
				return *this;
			}

			destroy();
			m_NumItems = 0;
			m_MaxNumItems = 0;

			reserve( rhs.m_MaxNumItems );

			copier::copy(	reinterpret_cast< type * >( m_Memory ),
							reinterpret_cast< const type * >( rhs.m_MaxNumItems ),
							rhs.m_NumItems );

			m_NumItems = rhs.m_NumItems;

			return *this;
		}

	private:
		//
		//	Helper functions.
		//
		void	destroy		()
		{
			for ( int i=0;i<m_NumItems;i++ )
			{
				reinterpret_cast< type * >( m_Memory + i*TypeSize )->~type();
			}

			allocator::deallocate( m_Memory );
			m_Memory = NULL;
		}

		void	double_array()
		{
			// If we can't alter our allocation size then don't try to!
			if ( allocator::PreventAllocation )
			{
				// We can be here if this is a statically allocated array.
				ntError_p( false, ("You are attempting to add too many elements to a statically allocated vector.") );
				return;
			}

			int new_count = m_NumItems == 0 ? 2 : m_NumItems*2;

			char *temp_memory = static_cast< char * >( allocator::allocate( new_count * TypeSize, *this ) );
			copier::copy(	reinterpret_cast< type * >( temp_memory ),
							reinterpret_cast< const type * >( m_Memory ),
							m_NumItems );

			destroy();
			m_Memory = temp_memory;

			m_MaxNumItems = new_count;
		}

		void	reserve		( int count )
		{
			if ( m_Memory == NULL )
			{
				m_Memory = static_cast< char * >( allocator::allocate( count * TypeSize, *this ) );
				m_MaxNumItems = allocator::PreventAllocation ? allocator::MaxItemCount : count;
				m_NumItems = 0;
			}
			else
			{
				ntError_p( !allocator::PreventAllocation, ("You cannot resize a statically allocated vector.") );
				if ( m_MaxNumItems - m_NumItems < count )
				{
					char *temp_memory = static_cast< char * >( allocator::allocate( ( m_NumItems + count ) * TypeSize, *this ) );
					copier::copy(	reinterpret_cast< type *>( temp_memory ),
									reinterpret_cast< const type * >( m_Memory ),
									m_NumItems );

					destroy();
					m_Memory = temp_memory;
				}
			}
		}

	private:
		//
		//	Aggregated members.
		//
		static const int Alignment	= ALIGNOF( type );
		static const int TypeSize	= sizeof( type );

		union
		{
			char *m_Memory;
			typename AlignedPOD< TypeList< type > >::Result	m_DoNotUse_HereForAlignmentOnly;
		};

		int		m_MaxNumItems;
		int		m_NumItems;
};

//**************************************************************************************
//	
//**************************************************************************************
#include "tbd/AlignedVector.inl"

#endif	// !ALIGNEDVECTOR_H_

