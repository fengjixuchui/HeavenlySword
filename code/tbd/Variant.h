/***************************************************************************************************
*
*	DESCRIPTION		A variant data type.
*
*	NOTES
*
***************************************************************************************************/

#ifndef VARIANT_H_
#define VARIANT_H_

// The warning these pragmas suppress is a compiler bug - see VTableImpl::Destroy for details.
#ifdef _MSC_VER
#	pragma warning( push )
#	pragma warning( disable : 4189 )	// local variable is initialised but not referenced.
#endif

//**************************************************************************************
//	Includes files.
//**************************************************************************************
#include "tbd/TypeList.h"

//**************************************************************************************
//	A variant data type.
//**************************************************************************************
template
<
	typename T1  = Empty,
	typename T2  = Empty,
	typename T3  = Empty,
	typename T4  = Empty,
	typename T5  = Empty,
	typename T6  = Empty,
	typename T7  = Empty,
	typename T8  = Empty,
	typename T9  = Empty,
	typename T10 = Empty
>
class Variant
{
	public:
		//
		//	Typedef a typelist from our passed types.
		//
		typedef TypeList< T1, T2, T3, T4, T5, T6, T7, T8, T9, T10 > Typelist;

	public:
		//
		//	Grab data.
		//
		template < typename T >
		const T &Get() const
		{
			static_assert( (GetIdx< Typelist, T >::index != -1), T_is_not_a_member_of_our_typelist );
			ntError_p( (m_TypeIndex == GetIdx< Typelist, T >::index), ("Types incorrect.") );	// Check T is the type we were assigned at runtime.
			return *reinterpret_cast< const T * >( &( m_Memory[ 0 ] ) );
		}

	public:
		//
		//	Ctors, dtor, assignment operator.
		//
		template < typename T >
		Variant			( const T &d )
		:	m_VTable	( VTableImpl< T >::GetVTablePtr() )	// Grab a specialised vtable implementation for this type.
		,	m_TypeIndex	( GetIdx< Typelist, T >::index )	// Store the type index for run-time checking.
		{
			NT_PLACEMENT_NEW ( m_Memory ) T( d );						// Placement new the type into our memory.
		}

		~Variant()
		{
			// Destroy using the vtable.
			( m_VTable->Destroy )( *this );
		}

		Variant( const Variant &copy )
		{
			*this = copy;
		}

		Variant &operator = ( const Variant &rhs )
		{
			// Clone via the vtable to get specialised behaviour.
			( rhs.m_VTable->Clone )( rhs, *this );
			return *this;
		}

	private:
		//
		//	Declare our vtable structure.
		//
		struct VTable
		{
			void	( *Destroy )	( const Variant & );
			void	( *Clone )		( const Variant &, Variant & );
		};

		//
		//	Template vtable to specialise for type T.
		//
		template < typename T >
		struct VTableImpl
		{
			private:
				// Destroy var explicity using its dtor.
				static void Destroy( const Variant &var )
				{
					// The pragmas surrounding this class turn off the above warning, this is because the VC7.1
					// compiler thinks that 'data' isn't referenced. Very strange indeed...
					// If I amalgamate the two lines below and remove the temporary then it thinks
					// 'var' is an unused formal parameter... stranger and strangerer...
					const T &data( *reinterpret_cast< const T * >( &( var.m_Memory[ 0 ] ) ) );
					data.~T();
				}

				// Clone src into dest by using placement new and the appropriate ctor.
				static void Clone( const Variant &src, Variant &dest )
				{
					NT_PLACEMENT_NEW ( &( dest.m_Memory[ 0 ] ) ) T( *reinterpret_cast< const T * >( &( src.m_Memory[ 0 ] ) ) );
					dest.m_VTable = src.m_VTable;
					dest.m_TypeIndex = src.m_TypeIndex;
				}

			public:
				static const VTable *GetVTablePtr()
				{
					static const VTable vtable =
					{
						&Destroy,
						&Clone,
					};

					return &vtable;
				}
		};

		//
		//	Aggregated members.
		//
		union	// This nameless union uses the AlignedPOD to align our placement-new memory
				// on the largest alignment of any type in our typelist.
		{
			typename AlignedPOD< Typelist >::Result	m_DoNotUse_HereForAlignmentOnly;

			unsigned char		m_Memory[ GetLargestType< Typelist >::size ];	// Memory to placement new into.
		};
		const VTable *			m_VTable;			// Our vtable pointer.
		int						m_TypeIndex;		// The index in our typelist of the currently stored type.
};

#ifdef _MSC_VER
#	pragma warning( pop )
#endif

#endif // VARIANT_H_
