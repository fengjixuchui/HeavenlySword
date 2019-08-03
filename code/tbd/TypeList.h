/***************************************************************************************************
*
*	DESCRIPTION		Define a typelist generic component.
*
*	NOTES			This typelist implementation is similar to the Tiny Template Library's
*					implementation. The advantage of explicitly declaring all the types as
*					separate template parameters is that the compiler doesn't have to recurse
*					so deeply to get at what we're doing. The disadvantage is that we can only
*					have a fixed number of types.
*
***************************************************************************************************/

#ifndef TYPELIST_H_
#define TYPELIST_H_

//**************************************************************************************
//	Includes files.
//**************************************************************************************
#include "tbd/TypeTraits.h"

// Note requires two MACROS ALIGNTO and ALIGNOF to be defined with compile 
// specifc alignment functions

//**************************************************************************************
//	A template class to represent a list of types.
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
struct TypeList;

template
<
	typename T1,
	typename T2,
	typename T3,
	typename T4,
	typename T5,
	typename T6,
	typename T7,
	typename T8,
	typename T9,
	typename T10
>
struct TypeList
{
	typedef T1 Head;
	typedef TypeList< T2, T3, T4, T5, T6, T7, T8, T9, T10 > Tail;

	enum { Length = 1 + Tail::Length };
};

// Empty list specialisation.
template <>
struct TypeList< Empty, Empty, Empty, Empty, Empty, 
				 Empty, Empty, Empty, Empty, Empty >
{
	enum { Length = 0 };
};

namespace TL
{

//**************************************************************************************
//	Calculate the length of a list.
//**************************************************************************************
template < typename typelist >
struct Length
{
	static const int value = typelist::Length;
};

//**************************************************************************************
//	Get the type at the specified index.
//**************************************************************************************
template
<
	typename	typelist,
	int			type_idx,
	int			curr_idx = 0,
	bool		found = ( type_idx == curr_idx ),
	bool		invalid_idx  = ( Length< typelist >::value == 0 )
>
struct Get
{
	typedef typename Get< typename typelist::Tail, type_idx, curr_idx+1 >::type type; 
};

template
<
	typename	typelist,
	int			type_idx,
	int			curr_idx,
	bool		found
>
struct Get< typelist, type_idx, curr_idx, found, true >
{
	// Index if out of bounds.
};

template
<
	typename	typelist,
	int			type_idx,
	int			curr_idx,
	bool		invalid_idx
>
struct Get< typelist, type_idx, curr_idx, true, invalid_idx >
{
	typedef typename typelist::Head type;
};

//**************************************************************************************
//	Get the index of the specified type.
//**************************************************************************************
template
<
	typename	typelist,
	typename	type,
	int			curr_idx = 0,
	bool		found = ( SameType< typename typelist::Head, type >::result ),
	bool		invalid_type = ( Length< typelist >::value == 0 )
>
struct GetIdx
{
	static const int index = GetIdx< typename typelist::Tail, type, curr_idx+1 >::index;
};

template
<
	typename	typelist,
	typename	type,
	int			curr_idx,
	bool		found
>
struct GetIdx< typelist, type, curr_idx, found, true >
{
	static const int index = -1;
};

template
<
	typename	typelist,
	typename	type,
	int			curr_idx,
	bool		invalid_idx
>
struct GetIdx< typelist, type, curr_idx, true, invalid_idx >
{
	static const int index = curr_idx;
};

//**************************************************************************************
//	Return the type in the list with the largest size.
//**************************************************************************************
template
<
	typename	typelist,
	bool		invalid_idx = ( Length< typelist >::value == 0 )
>
struct GetLargestType
{
	static const int head_size = sizeof( typename typelist::Head );
	static const int tail_size = GetLargestType< typename typelist::Tail >::size;
	static const int size = head_size > tail_size ? head_size : tail_size;
};

template
<
	typename	typelist
>
struct GetLargestType< typelist, true >
{
	enum { size = 0 };
};

//**************************************************************************************
//	Return the largest alignment of the types... VC.Net specific!
//**************************************************************************************
template
<
	typename	typelist,
	bool		invalid_idx = ( Length< typelist >::value == 0 )
>
struct GetAlignment
{
	private:
		static const int head_align = ALIGNOF( typename typelist::Head );
		static const int tail_align = GetAlignment< typename typelist::Tail >::align;

	public:
		static const int align = head_align > tail_align ? head_align : tail_align;
};

template
<
	typename	typelist
>
struct GetAlignment< typelist, true >
{
	enum { align = 0 };
};

} // namespace TL

//**************************************************************************************
//	Alignment base class.
//**************************************************************************************
namespace Private
{
#define ALIGNED_POD( _size_ )																				\
			template<> struct AlignedPODBase::AlignedPodInternal< _size_ >												\
			{																											\
				ALIGNTO_PREFIX( _size_ ) struct type																	\
				{																										\
					char X[ _size_ ];																					\
				} ALIGNTO_POSTFIX( _size_ );																			\
				enum { alignment = ALIGNOF( type ) };																	\
			};
#define ALIGNED_POD_CHECK( _size_ )																						\
		static_assert_in_class( ( _size_ == sizeof( AlignedPodInternal< _size_ >::type ) ), SizeofNotEqualSize##_size_ );	\
		static_assert_in_class( ( _size_ == AlignedPodInternal< _size_ >::alignment ), SizeofNotEqualAlignof##_size_ )
			
	class AlignedPODBase
	{
		protected:
			//
			//	Only let derived classes access this stuff.
			//
			template< unsigned int AlignmentSize >
			struct AlignedPodInternal
			{
				static_assert_in_class( AlignmentSize == 0, BadAlignmentSize_OnlyUpTo128 );
			};
	};
	//
	//	Define one AlignedPod class specialisation for each alignment.
	//
	ALIGNED_POD( 1 );
	ALIGNED_POD( 2 );
	ALIGNED_POD( 4 );
	ALIGNED_POD( 8 );
	ALIGNED_POD( 16 );
	ALIGNED_POD( 32 );
	ALIGNED_POD( 64 );
	ALIGNED_POD( 128 );
	// can be up to 8192 - is it realistic?

	// class's job is purely to check compile time check the alignments
	class AlignedPODCheck : public AlignedPODBase
	{
		ALIGNED_POD_CHECK( 1 );
		ALIGNED_POD_CHECK( 2 );
		ALIGNED_POD_CHECK( 4 );
		ALIGNED_POD_CHECK( 8 );
		ALIGNED_POD_CHECK( 16 );
		ALIGNED_POD_CHECK( 32 );
		ALIGNED_POD_CHECK( 64 );
		ALIGNED_POD_CHECK( 128 );
	};

#undef ALIGNED_POD
#undef ALIGNED_POD_CHECK

}; // end namespace Private

template
<
	typename typelist
>
class AlignedPOD : private Private::AlignedPODBase
{
	private:
		enum { maxAlign = TL::GetAlignment< typelist >::align };

	public:
		//
		//	Typedef the AlignedPod corresponding to the alignment of
		//	the maximally aligned type in typelist.
		//
		typedef typename AlignedPodInternal< maxAlign >::type Result;
};

#endif // TYPELIST_H_

