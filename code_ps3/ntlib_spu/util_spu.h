//----------------------------------------------------------------------------------------
//! 
//! \filename ntlib_spu\util_spu.h
//! 
//!	Contains useful misc' definitions that would normally be in something like a pch.
//!
//! SUPER C:
//! This is a 'super' C implementation, it should be 
//! treated as C code using C++ style syntax and not 
//! true C++ code. In particular, ctors must not be 
//! required, nor dtors (they can exist for convience but
//! must not be required). No vtables and types must be PODs
//! \see http://kong:8080/confluence/display/CH/SuperC
//!
//----------------------------------------------------------------------------------------
#ifndef UTIL_SPU_H_
#define UTIL_SPU_H_

#ifndef __SPU__
#	error This file is for inclusion in SPU projects ONLY!
#endif // !__SPU__

#include "ntlib_spu/basetypes_spu.h"

// alignment macros
#define ALIGNTO_PREFIX( size )
#define ALIGNTO_POSTFIX( size )		__attribute__ (( aligned(size) ))

#define NOT_IMPLEMENTED

// Allocate some memory.void *Allocate( uint32_t length_in_bytes );

template<typename T> inline void ResolveOffset(T*& pobMember, const void* pvHead) 
{ 
	if(pobMember) 
		pobMember = reinterpret_cast<T*>(reinterpret_cast<intptr_t>(pvHead) + reinterpret_cast<intptr_t>(pobMember)); 
}

// Resolve offset for inline use - mainly for PPU/SPU shared code.
template < typename RetType, typename BaseType >
inline RetType *ResolveOffset( int32_t member_offset, const BaseType * const base )
{
	return member_offset == 0 ? NULL : reinterpret_cast< RetType * >( reinterpret_cast< intptr_t >( base ) + member_offset );
}

// Use to define an offset "pointer" from the base of a class. Really just a way to keep track of the actual type.
#define DEF_OFFSET( type ) int32_t

// Use to resolve an offset "pointer" to a pointer of known type.
#define OFFSET_PTR( type, member ) ResolveOffset< type >( member, this )

#define ROUND_POW2( num, pow2 )	( ( ( num ) + ( ( pow2 ) - 1 ) ) & ~( ( pow2 ) - 1 ) )

static const float EPSILON	=	1.0e-05f;

namespace ntstd
{
	template < typename T >
	inline T Min( const T a, const T b )
	{
		return a < b ? a : b;
	}

	template < typename T >
	inline T Max( const T a, const T b )
	{
		return a > b ? a : b;
	}

	template < typename T >
	inline T Clamp( const T v, const T lo, const T hi )
	{
		return Max( Min( v, hi ), lo );
	}
}

namespace Util
{
//! Is given number a power of 2?
template< typename T>
inline bool	IsPow2( const T iNum )
{
	return ( (iNum & ( iNum - 1 )) == 0 );
}

// Return the next highest power of two. See http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
unsigned inline int NextPow2(unsigned int iValue)
{
	iValue--;
	iValue |= iValue >> 1;
	iValue |= iValue >> 2;
	iValue |= iValue >> 4;
	iValue |= iValue >> 8;
	iValue |= iValue >> 16;
	iValue++;
	return iValue;
}

//! returns an aligned version (to alignment) of value
template< typename T>
inline T Align( const T value, const size_t alignment )
{
	return( (value + (alignment - 1)) & ~(alignment - 1) );
}

//! returns true if value is aligned to alignment
template< typename T>
inline bool IsAligned( const T value, const size_t alignment )
{
	return !(size_t(value) & (alignment - 1));
}

}

#define NT_MEMCPY(ptr,src,length)	memcpy(ptr,src,length)

#endif // !UTIL_SPU_H_

