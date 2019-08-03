//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Memory management (New/Delete Replacements)

	@note		(c) Copyright Sony Computer Entertainment 2005. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef FW_MEM_NEW_H
#define	FW_MEM_NEW_H

//--------------------------------------------------------------------------------------------------
/**
	@brief			Memory Tag

	We use this to differentiate our overloaded new operator (for single objects) from any other
	global new/delete operators. Use of an enum can cause problems relating to type conversion
	to/from int.
**/
//--------------------------------------------------------------------------------------------------

struct	FwMemTag
{
};

//--------------------------------------------------------------------------------------------------
/**
	@brief			Overloaded new/delete operators

	@note			These do *not* replace global new & delete. That's for you to do, if you want.
**/
//--------------------------------------------------------------------------------------------------

inline	void*	operator new( size_t size, const FwMemTag&, size_t alignment, const char* pTag, short lineNumber )
{
	return FwMem::CallAllocCallback( size, alignment, pTag, lineNumber );
}

inline	void*	operator new[]( size_t size, const FwMemTag&, size_t alignment, const char* pTag, short lineNumber )
{
	return FwMem::CallAllocCallback( size, alignment, pTag, lineNumber );
}

inline	void	operator delete( void* pMem, const FwMemTag&, size_t, const char*, short )
{
	FwMem::CallFreeCallback( pMem );
}

inline	void	operator delete[]( void* pMem, const FwMemTag&, size_t,const char*, short )
{
	FwMem::CallFreeCallback( pMem );
}

//--------------------------------------------------------------------------------------------------
/**
	Begin platform-specific evilness. Unfortunately Visual Studio will currently only let us 
	declare a template friend in an "enclosing" namespace. Since teams can use their own namespace
	this means we have to use the global namespace.

	GCC on the other hand *only* seems to work if the friend function is in its own namespace, 
	treating global friend declarations as if they were in the local namespace of the declaration.

	So until this gets fixed, we have to declare in a namespace on GCC, and in global scope on VS.
**/
//--------------------------------------------------------------------------------------------------

#ifdef _MSC_VER
#define FW_MEM_NEW_NS
#else
#define FW_MEM_NEW_NS FwMemNew
namespace FW_MEM_NEW_NS {
#endif

//--------------------------------------------------------------------------------------------------
/**
	@brief			Helpers to allow correct array destruction behaviour.

	FwInternalDeleteArray() uses processing that relies on the fact that "class A { T m_object }"
	and an object of type T both have the same memory footprint. As such we create objects of
	type T with an specialised 'operator new' to handle construction, and then use type A to delete
	them with a class-specific new, thus invoking the member m_object's destructor. 

	Single objects, due to the complexity in dealing with thinkg


	@param			pMem			Pointer to an object of type 'T', or NULL.
**/
//--------------------------------------------------------------------------------------------------

template <typename T> class FwAllocType
{
public:
	T	m_object;

	void	_DeleteArray()	{ delete[]	this; }

	static inline void	operator delete( void* pMem )		{ FwMem::CallFreeCallback( pMem ); }
	static inline void	operator delete[]( void* pMem )		{ FwMem::CallFreeCallback( pMem ); }	

private:
	// This is intentionally private, and unimplemented..
#ifndef __SNC__
	FwAllocType();
#endif
};

// We need to be able to identify the type of T without any constness
template< typename T >	struct	FwMemBaseType				{ typedef T value_type; };
template< typename T >	struct	FwMemBaseType< const T >	{ typedef T value_type; };

template <typename T> inline void	FwInternalDelete( T* pObject ) 
{ 
	// We need access to a non-const qualified version of 'T'..
	typedef	typename FwMemBaseType< T >::value_type	value_type;

	if ( pObject )
	{
		// Call the destructor, and then free the memory..
		const_cast< value_type* >( pObject )->~T();
		FwMem::CallFreeCallback( const_cast< value_type* >( pObject ) );
	}
}

template <typename T> inline void	FwInternalDeleteArray( T* pMem ) 
{ 
	( ( FwAllocType<T>* )( pMem ) )->_DeleteArray();
}

#ifndef _MSC_VER
} // namespace FW_MEM_NEW_NS
#endif 

//--------------------------------------------------------------------------------------------------
// Use this macro rather than 'friend' when you need the friend class to construct/destruct the object.

#define	FW_MEM_FRIEND()	template < typename >		friend class	FW_MEM_NEW_NS::FwAllocType;						\
						template < typename memT >	friend void		FW_MEM_NEW_NS::FwInternalDelete( memT* pObject );

//--------------------------------------------------------------------------------------------------
// Replacement new interface

#ifdef	_MSC_VER
#define	FW_FUNCTION_NAME	__FUNCSIG__
#else
#define	FW_FUNCTION_NAME	__PRETTY_FUNCTION__
#endif	//_MSC_VER

#ifdef	ATG_MEMORY_DEBUG_ENABLED
	#define	FW_NEW											new( FwMemTag(), 16, FW_FUNCTION_NAME, ( short )__LINE__ ) 
	#define	FW_TAGGED_NEW( tag, line )						new( FwMemTag(), 16, tag, ( short )line )

	#define	FW_ALIGNED_NEW( alignment )						new( FwMemTag(), ( alignment ), FW_FUNCTION_NAME, ( short )__LINE__ )
	#define	FW_TAGGED_ALIGNED_NEW( alignment, tag, line )	new( FwMemTag(), ( alignment ), tag, ( short )line )
#else
	#define	FW_NEW											new( FwMemTag(), 16, NULL, 0 ) 
	#define	FW_TAGGED_NEW( tag, line )						new( FwMemTag(), 16, NULL, 0 )

	#define	FW_ALIGNED_NEW( alignment )						new( FwMemTag(), ( alignment ), NULL, 0 )
	#define	FW_TAGGED_ALIGNED_NEW( alignment, tag, line )	new( FwMemTag(), ( alignment ), NULL, 0 )
#endif // ATG_MEMORY_DEBUG_ENABLED

//--------------------------------------------------------------------------------------------------
// Replacement delete interfaces

#define	FW_DELETE( pointer )				{ if(0) delete pointer; FW_MEM_NEW_NS::FwInternalDelete( pointer ); }
#define	FW_DELETE_ARRAY( pointer )			{ if(0) delete[] pointer; FW_MEM_NEW_NS::FwInternalDeleteArray( pointer ); }

//--------------------------------------------------------------------------------------------------
// This is here for completeness.. it makes it much clearer that it's a placement new being called.

#define	FW_PLACEMENT_NEW	new

#endif	// FW_MEM_NEW_H
