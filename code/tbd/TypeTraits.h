/***************************************************************************************************
*
*	DESCRIPTION		Define a class to contain typetraits.
*
*	NOTES
*
***************************************************************************************************/

#ifndef TYPETRAITS_H_
#define TYPETRAITS_H_

//**************************************************************************************
//	Includes files.
//**************************************************************************************
#include "tbd/TypeSelect.h"

//**************************************************************************************
//	An empty type.
//**************************************************************************************
struct Empty {};

//**************************************************************************************
//	Result is true if T1 is the same type as T2, false otherwise.
//**************************************************************************************
template < typename T1, typename T2 >
struct SameType
{
	enum { result = false };
};
template < typename T >
struct SameType< T, T >
{
	enum { result = true };
};

//**************************************************************************************
//	Implementation detail for typetraits.
//**************************************************************************************
namespace Private
{
	template < typename T >
	struct IsPointer
	{
		enum { result = false };
	};
	template < typename T >
	struct IsPointer< T * >
	{
		enum { result = true };
	};
	template < typename T >
	struct IsPointer< const T * >
	{
		enum { result = true };
	};
	template < typename T >
	struct IsPointer< volatile T * >
	{
		enum { result = true };
	};

	template < typename T >
	struct IsReference
	{
		enum { result = false };
	};
	template < typename T >
	struct IsReference< T & >
	{
		enum { result = true };
	};
	template < typename T >
	struct IsReference< const T & >
	{
		enum { result = true };
	};
}

//**************************************************************************************
//	Various type traits for the type T.
//**************************************************************************************
template < typename T >
struct TypeTraits
{
	enum { IsFundamental =	SameType< T, char >::result ||
							SameType< T, unsigned char >::result ||
							SameType< T, short int >::result ||
							SameType< T, unsigned short int >::result ||
							SameType< T, int >::result ||
							SameType< T, unsigned int >::result ||
							SameType< T, long >::result ||
							SameType< T, unsigned long >::result ||
							SameType< T, float >::result ||
							SameType< T, double >::result ||
							SameType< T, long double >::result };

	enum { IsPointer = Private::IsPointer< T >::result };
	enum { IsReference = Private::IsReference< T >::result };

	typedef typename TypeSelect< IsFundamental || IsPointer || IsReference, T, T & >::ResultType ParameterType; 
};

#endif // TYPETRAITS_H_







