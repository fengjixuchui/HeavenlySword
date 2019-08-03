//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Shared pointers for shared ownership of dynamically allocated objects and arrays.

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef FW_STD_SHARED_PTR_H
#define FW_STD_SHARED_PTR_H

//--------------------------------------------------------------------------------------------------
//	INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Fw/FwStd/FwStdAllocator.h>
#include <Fw/FwStd/FwStdHelpers.h>

//--------------------------------------------------------------------------------------------------
//	BEGIN NAMESPACE DEFINITION
//--------------------------------------------------------------------------------------------------

namespace FwStd
{
	
//--------------------------------------------------------------------------------------------------
//	CLASS DEFINITION
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@class
    
	@brief	Shared pointer base class template for shared ownership of dynamically allocated objects
	and arrays.

  	
	Shared pointers store a pointer to a dynamically allocated object. The object pointed to is guaranteed
	to be deleted when the last shared pointer sharing ownership is destroyed, in otherwords, when no more
	references to that object exist.
	
	The object referenced is typically allocated via FW_NEW, however, other memory allocation mechanisms
	can be used. In this case the appropriate deletion policy must be specified i.e. one that matches the
	allocation.
	
    @code
			
	// A simple guideline is to always initialise a named shared pointer with the result of an allocation.
		
	FwStd::SharedPtr<MyObject>	pMyObject = FW_NEW MyObject;

	// 	As a result, there is no need for any explicit deletion.

	@endcode

    This base class allows different types of shared pointers to be derived - each with their own deletion
	and reference count allocation policies.


	@param	T		Type of object pointed to.
    
	@param	D		Deletion policy. A function object that deletes the referenced object when the
		   			last shared pointer referencing it is destroyed.
          
	@param	A		Allocation policy for reference count storage. This must be implemented as an STL allocator.
					The default is @link Allocator FwStd::Allocator<int> @endlink .
	
	
	@note	Due to reference counting shared pointers that produce cyclic references will not delete the object
			being pointed to - use weak pointers to break these cycles.
	
	@note	This shared pointer implementation consumes twice as much memory than built-in pointers. This is
			because internally two builtin pointers are stored - one for the object and another for the
			shared reference count.
	
	@note	This shared pointer implementation does not introduce any further levels of indirection - dereferencing
			is the same as built-in pointers.

	@note	Shared pointers can be used with the STL containers and algorithms.

	@note	The deleter D must be compatible with the allocation method of the referenced object.
			For example, in the case of FW_NEW the deletion policy must call FW_DELETE.
			
	@note	No implicit conversion into a bool is defined. Doing so allows pointer arithmetic to 'silently'
			succeed with errorneous results. Instead we convert to an UnspecifiedBoolType to allow shared
			pointers to be used in boolean expressions. In addition, you can use the IsValid() and IsNull()
			member functions instead.
	
	@note	Please refer to http://www.boost.org/ for further details on shared pointers.
	
	
	@see	@link SharedPtr FwStd::SharedPtr<T> @endlink
	
	@see	@link SharedArrayPtr FwStd::SharedArrayPtr<T> @endlink

	@see	@link ScopedPtrBase FwStd::ScopedPtrBase<T> @endlink
**/
//--------------------------------------------------------------------------------------------------

template<typename T, typename D, typename A = FwStd::Allocator<int> >
class SharedPtrBase
{
public:                               
	
	// Typedefs
	
	/// Type of the instantiated class template - is used to simplify class definitions.
	
	typedef SharedPtrBase<T, D, A> ThisType;
   
	typedef T* (ThisType::*UnspecifiedBoolType)() const;
    
	
	// Construction & Destruction
	
	explicit inline SharedPtrBase(T* p = NULL);
	
	inline ~SharedPtrBase();
    
	inline SharedPtrBase(const ThisType& rOther);

	/**
		@brief	Allows construction from an implicitly cast-able type.
	 **/
		
	template< typename U, typename E >
	inline SharedPtrBase( const SharedPtrBase< U, E, A >& rOther ) 
	  : m_p( rOther.m_p ), 
		m_pRefCount( rOther.m_pRefCount )
	{
		if( m_pRefCount )
			++*m_pRefCount;
	}
	
	/**
		@brief	Allows construction from an statically cast-able type.
	 **/
		
	template< typename U, typename E >
	inline SharedPtrBase( const SharedPtrBase< U, E, A >& rOther, Detail::StaticCastTag ) 
	  : m_p( static_cast< T*>( rOther.m_p ) ), 
		m_pRefCount( rOther.m_pRefCount )
	{
		if( m_pRefCount )
			++*m_pRefCount;
	}
	
	/**
		@brief	Allows construction from an dynamically cast-able type.
	 **/
		
	template< typename U, typename E >
	inline SharedPtrBase( const SharedPtrBase< U, E, A >& rOther, Detail::DynamicCastTag ) 
	  : m_p( dynamic_cast< T*>( rOther.m_p ) ), 
		m_pRefCount( rOther.m_pRefCount )
	{
		if( m_pRefCount )
			++*m_pRefCount;
	}
	
														
														
	// Conversions
	
	inline operator UnspecifiedBoolType() const;
 
 	
	// Operators
	
	inline ThisType& operator=(const ThisType& rRhs);

	inline bool operator!() const;

	
	// Operations
	
	inline void Reset(T* pNew = NULL);
	
	inline void Swap(ThisType& rOther);


	// Access

	inline T* Get() const;
	
	inline int GetRefCount() const;
	

	// Inquiry

	inline bool IsUnique() const;
	
	inline bool IsValid() const;
	inline bool IsNull() const;


private:

	// Allow other smart pointers to read.

	template< typename U, typename E, typename B > friend class SharedPtrBase;

	// Typedefs
	
	/// Type of shared reference count pointer.
	
	typedef typename A::pointer		RefCountPtr;
	
	
	// Attributes
	
	T*		   		m_p;				///< Pointer to referenced object.
	
	RefCountPtr		m_pRefCount;	   	///< Pointer to shared reference count.
};

//--------------------------------------------------------------------------------------------------
//	INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief	Constructs a shared pointer that owns p or an empty shared pointer if no argument
			is specified.

	@param	p	Ptr to object being referenced.
**/
//--------------------------------------------------------------------------------------------------

template<typename T, typename D, typename A>
inline SharedPtrBase<T, D, A>::SharedPtrBase(T* p)
	:
	m_p(p)
{
	
	if (m_p)
	{
		A	allocator;
		
		m_pRefCount = allocator.allocate(1);
		
		FW_ASSERT(m_pRefCount);
		
		*m_pRefCount = 1;
	}
	else
	{
		m_pRefCount = NULL;
	}
}

//--------------------------------------------------------------------------------------------------
/**
	@brief	Destroys the object being referenced if this is the last shared pointer referencing it.
	
	If the shared pointer is empty this destructor does nothing.
**/
//--------------------------------------------------------------------------------------------------

template<typename T, typename D, typename A>
inline SharedPtrBase<T, D, A>::~SharedPtrBase()
{

	if (m_pRefCount && (--*m_pRefCount == 0))
	{
		D	deleter;
		A	allocator;
		
		deleter(m_p);
		allocator.deallocate(m_pRefCount, 1);
    }
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Copy constructs a shared pointer that shares ownership with the shared pointer rOther.
	
	@param		rOther		Shared pointer to be copied. If rOther is an empty pointer then the newly
							copy constructed pointer is also empty.
**/
//--------------------------------------------------------------------------------------------------

template<typename T, typename D, typename A>
inline SharedPtrBase<T, D, A>::SharedPtrBase(const ThisType& rOther)
	:
	m_p(rOther.m_p),
	m_pRefCount(rOther.m_pRefCount)
{
	
	if (m_pRefCount)
		++*m_pRefCount;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief	Conversion operator allowing shared pointers to be used safely in boolean expressions.
	
	
	@code
	
	FwStd::SharedPtr<MyObject>	pMyObject = FW_NEW MyObject;
	
	if (p)
		p->Update();
		
	@endcode
	
	
	@note	The target type UnspecifiedBoolType is actually a member function pointer - this avoids
			many of the implicit conversion pitfalls associated with converting to bool.
**/
//--------------------------------------------------------------------------------------------------
	
template<typename T, typename D, typename A>
inline SharedPtrBase<T, D, A>::operator typename SharedPtrBase<T, D, A>::UnspecifiedBoolType() const
{
	
	return (m_p == 0) ? 0 : &ThisType::Get;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief	Shared pointer assignment operator.
**/
//--------------------------------------------------------------------------------------------------

	
template<typename T, typename D, typename A>
inline typename SharedPtrBase<T, D, A>::ThisType& SharedPtrBase<T, D, A>::operator=(const ThisType& rRhs)
{
	
	ThisType pTemp(rRhs);
	Swap(pTemp);
	
	return *this;
}   

//--------------------------------------------------------------------------------------------------
/**
	@brief		Shared pointer 'not' operator.
	
	@return		True if no object is being referenced i.e. (p == NULL).
**/
//--------------------------------------------------------------------------------------------------

template<typename T, typename D, typename A>
inline bool SharedPtrBase<T, D, A>::operator!() const
{

	return (m_p == NULL);
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Exchanges the contents of two shared pointers.
	
	Is equivalent to
	
	Swap(a, b);

	@see			FwStd::Swap(SharedPtrBase<T, D, A>& rA, SharedPtrBase<T, D, A>& rB)
**/
//--------------------------------------------------------------------------------------------------

template<typename T, typename D, typename A>
inline void SharedPtrBase<T, D, A>::Swap(ThisType& rOther)
{

	T* pPrev = m_p;
	m_p = rOther.m_p;
	rOther.m_p = pPrev;
	
	RefCountPtr	pPrevRefCount = m_pRefCount;
	m_pRefCount = rOther.m_pRefCount;
	rOther.m_pRefCount = pPrevRefCount;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Resets the shared pointer. Any object currently being referenced is destroyed if no
				more references to that object exist.
				
				The shared pointer then references the allocated object pointed to by pNew, if any.

	@param		pNew	Ptr to the new object to be referenced or NULL.
**/
//--------------------------------------------------------------------------------------------------

template<typename T, typename D, typename A>
inline void SharedPtrBase<T, D, A>::Reset(T* pNew)
{
        	
	// Ensure self-resets are caught.
	FW_ASSERT((pNew == NULL) || (m_p != pNew));
    
	ThisType pTemp(pNew);
	Swap(pTemp);
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Returns a pointer to the object being referenced, if any.
		
	@return 	Pointer to object being referenced or NULL.
**/
//--------------------------------------------------------------------------------------------------
	
template<typename T, typename D, typename A>
inline T* SharedPtrBase<T, D, A>::Get() const
{

	return m_p;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Returns the number of shared pointers that share ownership of the referenced object.
	
	@return 	Total number of shared pointers that share ownership of the referenced object or 0
				if the shared pointer is empty.

	@note		The value returned includes the shared pointer *this.
**/
//--------------------------------------------------------------------------------------------------
	
template<typename T, typename D, typename A>
inline int SharedPtrBase<T, D, A>::GetRefCount() const
{

	return (m_pRefCount) ? *m_pRefCount : 0;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Returns true if shared pointer is unique i.e. has sole ownership of the referenced object.
	
	@return 	True if the shared pointer is unique i.e. (GetRefCount() == 1). Empty shared pointers
				always return false.
**/
//--------------------------------------------------------------------------------------------------

template<typename T, typename D, typename A>
inline bool SharedPtrBase<T, D, A>::IsUnique() const
{

	return (m_pRefCount && (*m_pRefCount == 1));
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Returns true if the shared pointer is referencing an object.
	
	@return 	True if an object is being referenced i.e. (p != NULL).
**/
//--------------------------------------------------------------------------------------------------

template<typename T, typename D, typename A>
inline bool SharedPtrBase<T, D, A>::IsValid() const
{

	return (m_p != NULL);
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Returns true if the shared pointer is NULL.
	
	@return 	True if no object is being referenced i.e. (p == NULL).
**/
//--------------------------------------------------------------------------------------------------

template<typename T, typename D, typename A>
inline bool SharedPtrBase<T, D, A>::IsNull() const
{

	return (m_p == NULL);
}

//--------------------------------------------------------------------------------------------------
//	INLINE FREE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief	   	Shared pointer equals comparison operator.

	@return	   	True if the shared pointers are equal i.e. (rLhs.Get() == rRhs.Get())
**/
//--------------------------------------------------------------------------------------------------

template<typename T0, typename D0, typename A0, typename T1, typename D1, typename A1>
inline bool operator==(const SharedPtrBase<T0, D0, A0>& rLhs, const SharedPtrBase<T1, D1, A1>& rRhs)
{

	return (rLhs.Get() == rRhs.Get());
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Shared pointer not equals comparison operator.

	@return			True if the shared pointers are not equal i.e. (rLhs.Get() != rRhs.Get())
**/
//--------------------------------------------------------------------------------------------------
	
template<typename T0, typename D0, typename A0, typename T1, typename D1, typename A1>
inline bool operator!=(const SharedPtrBase<T0, D0, A0>& rLhs, const SharedPtrBase<T1, D1, A1>& rRhs)
{

	return (rLhs.Get() != rRhs.Get());
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Shared pointer less than comparison operator.
	
	@return			True if (rLhs.Get() < rRhs.Get())
	
	@note			Allows shared pointers to be used with the STL algorithms.
	
	@note			Two shared pointers are equivalent if and only if they share ownership.
**/
//--------------------------------------------------------------------------------------------------
	
template<typename T0, typename D0, typename A0, typename T1, typename D1, typename A1>
inline bool operator<(const SharedPtrBase<T0, D0, A0>& rLhs, const SharedPtrBase<T1, D1, A1>& rRhs)
{

	return (rLhs.Get() < rRhs.Get());
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Exchanges the contents of two shared pointers.
	
	Is equivalent to
	
	a.Swap(b);

	@see			FwStd::SharedPtrBase::Swap()
**/
//--------------------------------------------------------------------------------------------------

template<typename T, typename D, typename A>
inline void Swap(SharedPtrBase<T, D, A>& rA, SharedPtrBase<T, D, A>& rB)
{
    
	rA.Swap(rB);
}

//--------------------------------------------------------------------------------------------------
//	CLASS DEFINITION
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief		Shared pointer class template for objects allocated with FW_NEW.

	
	The referenced object is guaranteed to be deleted when the last shared pointer sharing ownership is
	destroyed, in otherwords, when no more references to that object exist.
	
	@code
			
	// ...
	
	if (expr)
	{
		// Initialise the shared pointer upon object allocation.
		
		FwStd::SharedPtr<MyObject>	pMyObject = FW_NEW MyObject;

		// ...
		
		if (expr2)
		{
			FwStd::SharedPtr<MyObject>	pAnotherRef(pMyObject);
			
			// ...
			
			return;		// There is no need for an explicit call to FW_DELETE here or anywhere else.
		}
	}

	// ...
	
	@endcode
	
	
	@param		T	Type of object pointed to.


	@note		Pointer arithmetic is not defined since %FwStd::SharedPtr<T> only point to individual
				objects. The subscript operator [] is also not defined for the same reason. You can
				dereference the shared pointer using the * and -> operators.
		
	@note		Use @link SharedArrayPtr FwStd::SharedArrayPtr<T> @endlink for shared pointers to arrays.
	
	@see		@link SharedPtrBase FwStd::SharedPtrBase<T> @endlink
**/
//--------------------------------------------------------------------------------------------------

template<typename T, typename A = FwStd::Allocator<int> >
class SharedPtr : public SharedPtrBase<T, CheckedDeleteFuncObj<T>, A>
{
public:

	// Construction & Destruction
	
	/**
		@brief	Constructs a shared pointer that owns p or an empty shared pointer if no argument
				is specified.

		Invokes FwStd::CheckedDelete() upon object deletion.
		
		@param	p	Ptr to object being referenced.
	**/
	
	explicit inline SharedPtr(T* p = NULL) : SharedPtrBase<T, CheckedDeleteFuncObj<T>, A>(p) { }


	/**
		@brief	Allows construction from a type that is implicitly cast-able.	

		This constructor is intentionally not explicit.
	**/
	template< typename U >
	inline SharedPtr( const SharedPtr< U, A >& rOther ) : SharedPtrBase< T, CheckedDeleteFuncObj< T >, A >( rOther ) { }

	/**
		@brief	Allows construction from a type that is statically cast-able.	

		This constructor is intentionally not explicit.
	**/
	template< typename U >
	inline SharedPtr( const SharedPtr< U, A >& rOther, Detail::StaticCastTag tag ) : SharedPtrBase< T, CheckedDeleteFuncObj< T >, A >( rOther, tag ) { }

	/**
		@brief	Allows construction from a type that is dynamically cast-able.	

		This constructor is intentionally not explicit.
	**/
	template< typename U >
	inline SharedPtr( const SharedPtr< U, A >& rOther, Detail::DynamicCastTag tag ) : SharedPtrBase< T, CheckedDeleteFuncObj< T >, A >( rOther, tag ) { }


	// Operators

	inline T& operator*() const;
	inline T* operator->() const;
};

//--------------------------------------------------------------------------------------------------
//	INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief		Dereference operator - returns a reference to the object being pointed to.
	
	@return		Reference to the object being pointed to.
	
	@note		Dereferencing a NULL pointer is undefined and will assert on a DEBUG build.
**/
//--------------------------------------------------------------------------------------------------

template<typename T, typename A>
inline T& SharedPtr<T, A>::operator*() const
{

	FW_ASSERT((SharedPtr<T, A>::Get()));
	
	return *(SharedPtr<T, A>::Get());
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Dereference operator with member selection - allows a member to be referenced.
	
	@return		Pointer to object being pointed to.
	
	@note		Dereferencing a NULL pointer is undefined and will assert on a DEBUG build.
**/
//--------------------------------------------------------------------------------------------------

template<typename T, typename A>
inline T* SharedPtr<T, A>::operator->() const
{

	FW_ASSERT((SharedPtr<T, A>::Get()));
	
	return SharedPtr<T, A>::Get();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Allows a static_cast to a different pointer type within a shared pointer.
**/
//--------------------------------------------------------------------------------------------------

template< typename T, typename U, typename A >
inline SharedPtr< T, A > SharedStaticCast( const SharedPtr< U, A >& rOther )
{
	return SharedPtr< T, A >( rOther, Detail::StaticCastTag() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Allows a dynamic_cast to a different pointer type within a shared pointer.
**/
//--------------------------------------------------------------------------------------------------

template< typename T, typename U, typename A >
inline SharedPtr< T, A > SharedDynamicCast( const SharedPtr< U, A >& rOther )
{
	return SharedPtr< T, A >( rOther, Detail::DynamicCastTag() );
}

//--------------------------------------------------------------------------------------------------
//	CLASS DEFINITION
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief		Shared pointer class template for arrays allocated with operator FW_NEW.

	The referenced array is automatically deleted when the last shared pointer sharing ownership is
	destroyed, in otherwords, when no more references to that array exist.
	
	@code
			
	// ...
	
	if (expr)
	{
		// Initialise the shared pointer upon array allocation.
		
		FwStd::SharedArrayPtr<MyObject>	pMyObjects = FW_NEW MyObject[ 8 ];

		// ...
		
		if (expr2)
		{
			FwStd::SharedArrayPtr<MyObject>	pAnotherRef(pMyObject);
			
			// ...
			
			return;		// There is no need for an explicit FW_DELETE_ARRAY call here or anywhere else.
		}
	}

	// ...
	
	@endcode
	
	
	@param		T	Type of object pointed to.


	@note		Pointer arithmetic is not defined for %FwStd::SharedArrayPtr<T> - array navigation
				is limited to the subscript operator [].

	@note		The pointer dereference operators * and -> are not defined. Use the subscript operator []
				to access individual elements.

	@note		Use @link SharedPtr FwStd::SharedPtr<T> @endlink for shared pointers to individual objects.
					   
	@see		@link SharedPtrBase FwStd::SharedPtrBase<T> @endlink
**/
//--------------------------------------------------------------------------------------------------

template<typename T, typename A = FwStd::Allocator<int> >
class SharedArrayPtr : public SharedPtrBase<T, CheckedArrayDeleteFuncObj<T>, A>
{
public:

	// Construction & Destruction

	/**
		@brief	Constructs a shared array pointer that owns p or an empty shared pointer if no argument
				is specified.

		Invokes FwStd::CheckedArrayDelete() upon array deletion.
		
		@param	p	Ptr to array being referenced.
	**/
	
	explicit inline SharedArrayPtr(T* p = NULL) : SharedPtrBase<T, CheckedArrayDeleteFuncObj<T>, A>(p) { }

	
	// Operators
	
	inline T& operator[](int i) const;
};

//--------------------------------------------------------------------------------------------------
//	INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief		Subscript operator - returns a reference to the indexed element of the array being
				pointed to.
	
	@param		Array index.
	
	@return		Reference to the indexed element.
	
	@note		Dereferencing a NULL array pointer is undefined and will assert on a DEBUG build.
	
	@note		Negative indices are not allowed and will assert on a DEBUG build.
**/
//--------------------------------------------------------------------------------------------------

template<typename T, typename A>
inline T& SharedArrayPtr<T, A>::operator[](int i) const
{

	FW_ASSERT((SharedArrayPtr<T, A>::Get()) && (i >= 0));
	
	return (SharedArrayPtr<T, A>::Get())[i];
}

//--------------------------------------------------------------------------------------------------
//	END NAMESPACE DEFINITION
//--------------------------------------------------------------------------------------------------

} // FwStd

//--------------------------------------------------------------------------------------------------

#endif // FW_STD_SHARED_PTR_H
