//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Intrusive pointers to allow objects with embedded reference counts.

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef FW_STD_INTRUSIVE_PTR_H
#define FW_STD_INTRUSIVE_PTR_H

#include <Fw/FwStd/FwStdHelpers.h>

namespace FwStd
{

//--------------------------------------------------------------------------------------------------
/**
	@class		IntrusivePtr
    
	@brief		An implementation of a flexible intrusive pointer class, based on that found in the 
				Boost C++ library.

	The IntrusivePtr class template stores a pointer to an object with an embedded reference count.
	Every new IntrusivePtr instance increments the reference count by using an unqualified call to
	the function IntrusivePtrAddRef(), passing it the pointer as an argument. Similarly, when an
	IntrusivePtr object is destroyed, it calls IntrusivePtrRelease(). This function is responsible
	for destroying the object when its reference count drops to zero. Finally, when querying the
	current reference counter, the function IntrusivePtrGetRefCount() is called. 

	The standard use rule is to derive the class that contains the reference count from the supplied
	'FwStd::IntrusivePtrCountedBase'. 
	
	When objects are not derived from 'IntrusivePtrCountedBase' object, the user is expected to 
	provide suitable definitions of these two functions. On compilers that support argument-dependent
	lookup, IntrusivePtrAddRef, IntrusivePtrRelease and IntrusivePtrGetRefCount should be defined in
	the namespace that corresponds to their parameter; otherwise, the definitions need to go in the
	namespace 'FwStd'. 

	Use of these specialised functions can allow a more efficient layout of the reference counted
	class, due to the way that the placement & storage type of the reference count is down to the
	application. Please note that the reference count on these specialised objects should be
	initialised to 1 on construction. Also note that regardless of how your reference count is
	stored (char, short etc), the specialisation of IntrusivePtrGetRefCount() *must* return an the
	reference count as an int. 

	The class template is parameterized on T, the type of the object pointed to. IntrusivePtr<T> can
	be implicitly converted to IntrusivePtr<U> whenever T* can be implicitly converted to U*.

	More often than not, it can make sense to encapsulate the intrusive pointer using a typedef to 
	reduce the possibility of people using raw pointers.

	@code
			// All end user code uses MyObjectPtr..
			typedef	FwStd::IntrusivePtr<MyObject>	MyObjectPtr;
	@endcode


	The main reasons to use IntrusivePtr are:

		* Some existing frameworks or OSes provide objects with embedded reference counts;
		* The memory footprint of IntrusivePtr is the same as the corresponding raw pointer;
		* IntrusivePtr<T> can be constructed from an arbitrary raw pointer of type T*.


	Examples
	--------	

	@code
		// IntrusivePtr via derivation of IntrusivePtrCountedBase
		class	MyObject	:	public IntrusivePtrCountedBase<MyObject>
		{
		public:
			MyObject()	{ m_myData = 1234; }
			~MyObject()	{ }

		private:
			int		m_myData;
		};
	@endcode

	@code
		// IntrusivePtr usage via specialisation of IntrusivePtrAddRef/Release/GetRefCount
		class	MyObject
		{
		public:
			MyObject()	{ m_myData = 1234; m_refCount = 1; }
			~MyObject()	{ }

		private:
			int		m_myData;
			int		m_refCount;
		};

		// The specialisations of IntrusivePrtAddRef/Release/GetRefCount don't have to be within
		// FwStd.. they can be global. But it can prevent unnecessary cluttering of the global namespace.
		namespace FwStd
		{
			inline void	IntrusivePtrAddRef( MyObject* p )
			{
				++p->m_refCount;
			}
		
			inline void	IntrusivePtrRelease( MyObject* p )
			{
				if ( --p->m_refCount == 0 )
					FW_DELETE( p );
			}

			inline u32 IntrusivePtrGetRefCount( MyObject* p )
			{
				return (u32)( p->m_refCount );
			}
		};
	@endcode

	@param	T		Type of object pointed to.
    
	@note	This pointer implementation does not introduce any further levels of indirection -
			dereferencing is the same as built-in pointers.

	@note	No implicit conversion into a bool is defined. Doing so allows pointer arithmetic to
			'silently' succeed with errorneous results. Instead we convert to an UnspecifiedBoolType
			to allow intrusive pointers to be used in boolean expressions. 
	
	@note	Please refer to http://www.boost.org/ for further details on intrusive pointers.

**/
//--------------------------------------------------------------------------------------------------

template<typename T> class IntrusivePtr
{
private:
	typedef IntrusivePtr	ThisType;
	typedef T*				(ThisType::*UnspecifiedBoolType)() const;


public:
	// Construction & Destruction
	explicit IntrusivePtr( T* p = NULL );
	~IntrusivePtr();

	// Assignment
	IntrusivePtr( const IntrusivePtr& rhs );
	IntrusivePtr&	operator = ( const IntrusivePtr& rhs );
	
	/**
		@brief	Allows construction from a type that is implicitly cast-able.	

		This constructor is intentionally not explicit.
	**/
	template< typename U >
	inline IntrusivePtr( const IntrusivePtr< U >& rOther ) : m_p( rOther.m_p )
	{
		if( m_p )
			IntrusivePtrAddRef( m_p );
	}

	/**
		@brief	Allows construction from a type that is statically cast-able.	
	**/
	template< typename U >
	inline IntrusivePtr( const IntrusivePtr< U >& rOther, Detail::StaticCastTag ) 
	  : m_p( static_cast< T* >( rOther.m_p ) )
	{
		if( m_p )
			IntrusivePtrAddRef( m_p );
	}

	/**
		@brief	Allows construction from a type that is dynamically cast-able.	
	**/
	template< typename U >
	inline IntrusivePtr( const IntrusivePtr< U >& rOther, Detail::DynamicCastTag ) 
	  : m_p( dynamic_cast< T* >( rOther.m_p ) )
	{
		if( m_p )
			IntrusivePtrAddRef( m_p );
	}

	// Miscellaneous
	void		Reset( T* rhs = NULL );
	T*			Get( void ) const;
	void		Swap( IntrusivePtr& rhs );
	int			GetRefCount( void ) const;

	// Operators
	T*			operator ->	() const;
	operator	UnspecifiedBoolType() const;

	// State query
	bool		IsValid( void ) const;
	bool		IsNull( void ) const;

private:
	template< typename U > friend class IntrusivePtr;

    T*	m_p;
};

//--------------------------------------------------------------------------------------------------
/**
	@brief		Allows a static_cast to a different pointer type within a shared pointer.
**/
//--------------------------------------------------------------------------------------------------

template< typename T, typename U >
IntrusivePtr< T > IntrusiveStaticCast( const IntrusivePtr< U >& rOther )
{
	return IntrusivePtr< T >( rOther, Detail::StaticCastTag() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Allows a dynamic_cast to a different pointer type within a shared pointer.
**/
//--------------------------------------------------------------------------------------------------

template< typename T, typename U >
IntrusivePtr< T > IntrusiveDynamicCast( const IntrusivePtr< U >& rOther )
{
	return IntrusivePtr< T >( rOther, Detail::DynamicCastTag() );
}

//--------------------------------------------------------------------------------------------------
/**
	@class	IntrusivePtrCountedBase
    
	@brief	This is a base class that applications can derive from to form objects that are usable
			by the IntrusivePtr class.

	If you want a different reference count type (eg, a short), or want precise control over the 
	positioning of the reference count for alignment purposes, you should consider specialising
	IntrusivePtrAddRef/IntrusivePtrRelease/IntrusivePtrGetRefCount functions as described in the
	IntrusivePtr documentation.

	@see	@link IntrusivePtr FwStd::IntrusivePtr<T> @endlink
**/
//--------------------------------------------------------------------------------------------------

template <typename T>
class	IntrusivePtrCountedBase
{
protected:
	// Only derived classes can construct these objects.
	IntrusivePtrCountedBase() : m_refCount( 1 ) {}

	// Reference count manipulator that constructs a handle from a member function.
	IntrusivePtr< T > IntrusivePtrFromThis()
	{
		T* pDerived = static_cast< T* >( this );
		++m_refCount;
		return IntrusivePtr< T >( pDerived );
	}
	
private:
	// Our friend AddRef/Release functions need access to the reference count.
	// Please note that these friends are *not* member functions. 
	friend	void	IntrusivePtrAddRef( IntrusivePtrCountedBase<T>* p )
	{
		++p->m_refCount;
	}

	friend	void	IntrusivePtrRelease( IntrusivePtrCountedBase<T>* p )
	{
		FW_ASSERT( p->m_refCount != 0 );
		if ( --p->m_refCount == 0 )
		{
			T* pDerived = static_cast< T* >( p );
			FwStd::CheckedDelete( pDerived );
		}
	}

	friend	int		IntrusivePtrGetRefCount( IntrusivePtrCountedBase<T>* p )
	{
		return ( int )p->m_refCount;
	}

	u32	m_refCount;
};

//--------------------------------------------------------------------------------------------------
/**
	@brief	Intrusive pointer constructor from raw pointer.
**/
//--------------------------------------------------------------------------------------------------

template<typename T>
inline	IntrusivePtr<T>::IntrusivePtr( T* p ): m_p( p )
{
}

//--------------------------------------------------------------------------------------------------
/**
	@brief	Intrusive pointer copy constructor.
**/
//--------------------------------------------------------------------------------------------------

template<typename T>
inline	IntrusivePtr<T>::IntrusivePtr( const IntrusivePtr<T>& rhs): m_p( rhs.m_p )
{
	if ( m_p != NULL )
		IntrusivePtrAddRef( m_p );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief	Intrusive pointer destructor, releases reference count of any referenced object.
**/
//--------------------------------------------------------------------------------------------------

template<typename T>
inline	IntrusivePtr<T>::~IntrusivePtr()
{
	if ( m_p != NULL )
		IntrusivePtrRelease( m_p );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief	Intrusive pointer assignment operator.
**/
//--------------------------------------------------------------------------------------------------

template<typename T>
inline	IntrusivePtr<T>& IntrusivePtr<T>::operator = ( const IntrusivePtr<T>& rhs )
{
	IntrusivePtr<T> temp( rhs );
	Swap( temp );
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Resets the intrusive pointer. Any object currently being referenced is destroyed if no
				more references to that object exist.
				
				The intrusive pointer then references the allocated object pointed to by pNew, if any.

	@param		pNew	Ptr to the new object to be referenced or NULL.
**/
//--------------------------------------------------------------------------------------------------

template<typename T>
inline	void IntrusivePtr<T>::Reset( T* pNew )
{
	IntrusivePtr<T> temp( pNew );
	Swap( temp );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Returns a pointer to the object being referenced, if any.
		
	@return 	Pointer to object being referenced or NULL.
**/
//--------------------------------------------------------------------------------------------------

template<typename T>
inline	T*	IntrusivePtr<T>::Get() const
{
	return m_p;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Exchanges the contents of two intrusive pointers.
**/
//--------------------------------------------------------------------------------------------------

template<typename T>
inline	void	IntrusivePtr<T>::Swap( IntrusivePtr<T>& rhs )
{
	T* tmp = m_p;
	m_p = rhs.m_p;
	rhs.m_p = tmp;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Returns the reference count of the current object
		
	@return 	Reference count. 
**/
//--------------------------------------------------------------------------------------------------

template<typename T>
inline	int	IntrusivePtr<T>::GetRefCount( void ) const
{
	return IntrusivePtrGetRefCount( m_p );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Dereference operator with member selection - allows a member to be referenced.
	
	@return		Pointer to object being referenced

	@note		Dereferencing a NULL pointer is undefined and will assert on a DEBUG build.
**/
//--------------------------------------------------------------------------------------------------

template<typename T>
inline	T*	IntrusivePtr<T>::operator -> () const
{
	FW_ASSERT( m_p != NULL );
	return m_p;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief	Conversion operator allowing intrusive pointers to be used safely in boolean expressions.
	
	@code
	
	FwStd::IntrusivePtr<MyObject>	pMyObject( FW_NEW MyObject );
	
	if (p)
		p->Update();
		
	@endcode
	
	
	@note	The target type UnspecifiedBoolType is actually a member function pointer - this avoids
			many of the implicit conversion pitfalls associated with converting to bool.
**/
//--------------------------------------------------------------------------------------------------

template<typename T>
inline	IntrusivePtr<T>::operator typename IntrusivePtr<T>::UnspecifiedBoolType() const
{
	return m_p == NULL ? NULL: &ThisType::Get;
}


//--------------------------------------------------------------------------------------------------
/**
	@brief		Returns true if the intrusive pointer is referencing an object.
	
	@return 	True if an object is being referenced i.e. (p != NULL).
**/
//--------------------------------------------------------------------------------------------------

template<typename T>
inline bool IntrusivePtr<T>::IsValid( void ) const
{
	return ( m_p != NULL );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Returns true if the intrusive pointer is NULL.
	
	@return 	True if no object is being referenced i.e. (p == NULL).
**/
//--------------------------------------------------------------------------------------------------

template<typename T>
inline bool IntrusivePtr<T>::IsNull( void ) const
{
	return ( m_p == NULL );
}

// Non-member functions

//--------------------------------------------------------------------------------------------------
/**
	@brief			Intrusive pointer equality comparison operator (Intrusive == Intrusive)

	@return			True if the pointers are equal
**/
//--------------------------------------------------------------------------------------------------

template<typename T, typename U>
inline bool operator == ( const IntrusivePtr<T>& lhs, const IntrusivePtr<U>& rhs )
{
	return lhs.Get() == rhs.Get();
}

//--------------------------------------------------------------------------------------------------
/*
	@brief			Intrusive pointer inequality comparison operator (Intrusive != Intrusive)

	@return			True if the pointers are not equal
**/
//--------------------------------------------------------------------------------------------------

template<typename T, typename U>
inline bool operator != ( const IntrusivePtr<T>& lhs, const IntrusivePtr<U>& rhs )
{
	return lhs.Get() != rhs.Get();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Intrusive pointer equality comparison operator (Intrusive == Raw Pointer)

	@return			True if the pointers are equal
**/
//--------------------------------------------------------------------------------------------------

template<typename T>
inline bool operator == ( const IntrusivePtr<T>& lhs, T* rhs )
{
	return lhs.Get() == rhs;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Intrusive pointer inequality comparison operator (Intrusive != Raw Pointer)

	@return			True if the pointers are not equal
**/
//--------------------------------------------------------------------------------------------------

template<typename T>
inline bool operator != ( const IntrusivePtr<T>& lhs, T* rhs )
{
	return lhs.Get() != rhs;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Intrusive pointer equality comparison operator (Raw Pointer == Intrusive)

	@return			True if the pointers are equal
**/
//--------------------------------------------------------------------------------------------------

template<typename T>
inline bool operator == ( T* lhs, const IntrusivePtr<T>& rhs )
{
	return lhs == lhs.Get();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Intrusive pointer inequality comparison operator (Raw Pointer != Intrusive)

	@return			True if the pointers are not equal
**/
//--------------------------------------------------------------------------------------------------

template<typename T>
inline bool operator != ( T* lhs, const IntrusivePtr<T>& rhs )
{
	return lhs != rhs.Get();
}

} // namespace FwStd

#endif	// FW_STD_INTRUSIVE_PTR_H
