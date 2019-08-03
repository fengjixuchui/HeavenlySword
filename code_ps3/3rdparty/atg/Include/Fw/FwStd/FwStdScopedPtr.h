//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Scoped pointers for sole ownership of dynamically allocated objects and arrays.

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef FW_STD_SCOPED_PTR_H
#define FW_STD_SCOPED_PTR_H

//--------------------------------------------------------------------------------------------------
//	INCLUDES
//--------------------------------------------------------------------------------------------------

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
    
	@brief	Scoped pointer base class template for sole ownership of dynamically allocated objects and arrays.

  	
	The scoped pointer class template stores a pointer to a dynamically allocated object. The object
	pointed to is guaranteed to be deallocated when the pointer goes out of scope.
	
    The object referenced is typically allocated via FW_NEW, however, other memory allocation mechanisms
	can be used. In this case the appropriate deletion policy must be specified i.e. one that matches the
	allocation.
	
	Scoped pointers implement a basic "resource acquisition is initialisation", without shared ownership
	or transfer of ownership. It's class name and enforcement of semantics signals its intent of maintaining
	sole ownership within the current scope.
	
	This base class allows different types of scoped pointers to be derived - each with their own deletion
	policy.


	@param	T		Type of object pointed to.
    
	@param	D		Deletion policy. A function object that deletes the referenced object when the
		   			pointer goes out of scope.
          
	
	@note	Scoped pointers do not consume more memory than the built-in pointer type - their size is
			identical.
	
	@note	Scoped pointers are very simple and most operations are every bit as fast as their built-in
			pointer counterparts i.e. when optimisation is enabled and member functions are inlined.
    
	@note	Scoped pointers cannot be used with the STL containers since they're non-copyable. Use shared
			pointers instead. Refer to @link SharedPtrBase FwStd::SharedPtrBase<T> @endlink for details.

	@note	The deleter D must be compatible with the allocation method of the referenced object.
			For example, in the case of operator FW_NEW the deletion policy must call FW_DELETE.
			
	@note	No implicit conversion into a bool is defined. Doing so allows pointer arithmetic to 'silently'
			succeed with erroneous results. Instead we convert to an UnspecifiedBoolType to allow scoped
			pointers to be used in boolean expressions. In addition, you can use the IsValid() and IsNull()
			member functions instead.
	
	@note	Please refer to http://www.boost.org/ for further details on scoped pointers.
	
	
	@see	@link ScopedPtr FwStd::ScopedPtr<T> @endlink
	
	@see	@link ScopedArrayPtr FwStd::ScopedArrayPtr<T> @endlink

	@see	@link SharedPtrBase FwStd::SharedPtrBase<T> @endlink
**/
//--------------------------------------------------------------------------------------------------

template<typename T, typename D> class ScopedPtrBase : public FwNonCopyable
{
public:                               
	
	// Typedef
	
	/// Type of the instantiated class template - is used to simplify class definitions.
	
	typedef ScopedPtrBase<T, D> ThisType;
	 
	
	typedef T* (ThisType::*UnspecifiedBoolType)() const;

	
	// Construction & Destruction
	
	explicit inline ScopedPtrBase(T* p = NULL);
	
    inline ~ScopedPtrBase();


	// Conversions
	
	inline operator UnspecifiedBoolType() const;

	
	// Operators
	
	inline bool operator!() const;
	
	
	// Operations

	inline void Reset(T* pNew = NULL);
	
	inline void Swap(ThisType& rOther);


	// Access
	
	inline T* Get() const;


	// Inquiry
	
	inline bool IsValid() const;
	inline bool IsNull() const;


private:

	// Attributes
	
	T*		m_p;			///< Pointer to referenced object.
};

//--------------------------------------------------------------------------------------------------
//	INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief	Constructs a scoped pointer that solely owns p or an empty scoped pointer if no argument
			is specified.

	@param	p	Ptr to object being referenced.
**/
//--------------------------------------------------------------------------------------------------

template<typename T, typename D>
inline ScopedPtrBase<T, D>::ScopedPtrBase(T* p)
	:
	m_p(p)
{

}

//--------------------------------------------------------------------------------------------------
/**
	@brief	Destroys the object being referenced, if any.
**/
//--------------------------------------------------------------------------------------------------

template<typename T, typename D>
inline ScopedPtrBase<T, D>::~ScopedPtrBase()
{
	
	D	deleter;
		
	deleter(m_p);
}

//--------------------------------------------------------------------------------------------------
/**
	@brief	Conversion operator allowing scoped pointers to be used safely in boolean expressions.
	
	
	@code
	
	FwStd::ScopedPtr<MyObject>	pMyObject( FW_NEW MyObject );
	
	if (p)
		p->Update();
		
	@endcode
	
	
	@note	The target type UnspecifiedBoolType is actually a member function pointer - this avoids
			many of the implicit conversion pitfalls associated with converting to bool.
**/
//--------------------------------------------------------------------------------------------------
	
template<typename T, typename D>
inline ScopedPtrBase<T, D>::operator typename ScopedPtrBase<T, D>::UnspecifiedBoolType() const
{
	
	return (m_p == 0) ? 0 : &ThisType::Get;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Scoped pointer 'not' operator.
	
	@return		True if no object is being referenced i.e. (p == NULL).
**/
//--------------------------------------------------------------------------------------------------

template<typename T, typename D>
inline bool ScopedPtrBase<T, D>::operator!() const
{

	return (m_p == NULL);
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Exchanges the contents of two scoped pointers.
	
	Is equivalent to
	
	Swap(a, b);

	@see			FwStd::Swap(ScopedPtrBase<T, D>& rA, ScopedPtrBase<T, D>& rB)
**/
//--------------------------------------------------------------------------------------------------

template<typename T, typename D>
inline void ScopedPtrBase<T, D>::Swap(ThisType& rOther)
{
	
	T* pPrev = m_p;
	m_p = rOther.m_p;
	rOther.m_p = pPrev;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Resets the scoped pointer by destroying the object currently being referenced, if any.
				It then references the allocated object pointed to by pNew, if any.

	@param		pNew	Ptr to the new object to be referenced. If no argument is specified the scoped
						pointer is simply cleared and any previously referenced object is destroyed.

	@note		The caller must ensure that pNew is not referenced by any other smart pointer - this
				prevents multiple deletion.
**/
//--------------------------------------------------------------------------------------------------

template<typename T, typename D>
inline void ScopedPtrBase<T, D>::Reset(T* pNew)
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

template<typename T, typename D>
inline T* ScopedPtrBase<T, D>::Get() const
{
	
	return m_p;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Returns true if the scoped pointer is referencing an object.
	
	@return 	True if an object is being referenced i.e. (p != NULL).
**/
//--------------------------------------------------------------------------------------------------

template<typename T, typename D>
inline bool ScopedPtrBase<T, D>::IsValid() const
{
	
	return (m_p != NULL);
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Returns true if the scoped pointer is NULL.
	
	@return 	True if no object is being referenced i.e. (p == NULL).
**/
//--------------------------------------------------------------------------------------------------

template<typename T, typename D>
inline bool ScopedPtrBase<T, D>::IsNull() const
{
	
	return (m_p == NULL);
}

//--------------------------------------------------------------------------------------------------
//	INLINE FREE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief			Exchanges the contents of two scoped pointers.
	
	Is equivalent to
	
	a.Swap(b);

	@see			FwStd::ScopedPtrBase<T, D>::Swap()
**/
//--------------------------------------------------------------------------------------------------

template<typename T, typename D>
inline void Swap(ScopedPtrBase<T, D>& rA, ScopedPtrBase<T, D>& rB)
{
    
	rA.Swap(rB);
}

//--------------------------------------------------------------------------------------------------
//	CLASS DEFINITION
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief		Scoped pointer class template for objects allocated with FW_NEW.

	The referenced object is automatically deleted when the scoped pointer goes out of scope - there
	is no need for an explicit call to FW_DELETE.
	
	@code
			
	// ...
	
	if (expr)
	{
		// Initialise the scoped pointer upon object allocation.
		
		FwStd::ScopedPtr<MyObject>	pMyObject( FW_NEW MyObject );

		// ...
		
		if (expr2)
		{
			// ...
			
			return;		// There is no need for an explicit FW_DELETE call here or anywhere else.
		}
	}

	// ...
	
	@endcode
	
	
	@param		T	Type of object pointed to.


	@note		Pointer arithmetic is not defined since %FwStd::ScopedPtr<T> only point to individual
				objects. The subscript operator [] is also not defined for the same reason. You can
				dereference the scoped pointer using the * and -> operators instead.
		
	@note		Use @link ScopedArrayPtr FwStd::ScopedArrayPtr<T> @endlink for scoped pointers to arrays.
	
	@see		@link ScopedPtrBase FwStd::ScopedPtrBase<T> @endlink
**/
//--------------------------------------------------------------------------------------------------

template<typename T> class ScopedPtr : public ScopedPtrBase<T, CheckedDeleteFuncObj<T> >
{
public:

	// Construction & Destruction
	
	/**
		@brief	Constructs a scoped pointer that solely owns p or an empty scoped pointer if no argument
				is specified.

		Invokes FwStd::CheckedDelete() upon object deletion.
		
		@param	p	Ptr to object being referenced.
	**/
	
	explicit inline ScopedPtr(T* p = NULL) : ScopedPtrBase<T, CheckedDeleteFuncObj<T> >(p) { }


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

template<typename T> inline T& ScopedPtr<T>::operator*() const
{

	FW_ASSERT(ScopedPtr<T>::Get());
	
	return *(ScopedPtr<T>::Get());
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Dereference operator with member selection - allows a member to be referenced.
	
	@return		Pointer to object being pointed to.
	
	@note		Dereferencing a NULL pointer is undefined and will assert on a DEBUG build.
**/
//--------------------------------------------------------------------------------------------------

template<typename T> inline T* ScopedPtr<T>::operator->() const
{

	FW_ASSERT(ScopedPtr<T>::Get());
	
	return ScopedPtr<T>::Get();
}

//--------------------------------------------------------------------------------------------------
//	CLASS DEFINITION
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief		Scoped pointer class template for arrays allocated with FW_NEW.

	The referenced array is automatically deleted when the scoped pointer goes out of scope - there
	is no need for an explicit FW_DELETE_ARRAY.
	
	@code
			
	// ...
	
	if (expr)
	{
		// Initialise the scoped pointer upon array allocation.
		
		FwStd::ScopedArrayPtr<MyObject>	pMyObjects( FW_NEW MyObject[ 8 ] );

		// ...
		
		if (expr2)
		{
			// ...
			
			return;		// There is no need for an explicit FW_DELETE_ARRAY call here or anywhere else.
		}
	}

	// ...
	
	@endcode
	
	
	@param		T	Type of object pointed to.


	@note		Pointer arithmetic is not defined for %FwStd::ScopedArrayPtr<T> - array navigation
				is limited to the subscript operator [].

	@note		The pointer dereference operators * and -> are not defined. Use the subscript operator []
				to access individual elements.

	@note		Use @link ScopedPtr FwStd::ScopedPtr<T> @endlink for scoped pointers to individual objects.
					   
	@see		@link ScopedPtrBase FwStd::ScopedPtrBase<T> @endlink
**/
//--------------------------------------------------------------------------------------------------

template<typename T> class ScopedArrayPtr : public ScopedPtrBase<T, CheckedArrayDeleteFuncObj<T> >
{
public:

	// Construction & Destruction

	/**
		@brief	Constructs a scoped array pointer that solely owns p or an empty scoped pointer if no argument
				is specified.

		Invokes FwStd::CheckedArrayDelete() upon array deletion.
		
		@param	p	Ptr to array being referenced.
	**/
	
	explicit inline ScopedArrayPtr(T* p = NULL) : ScopedPtrBase<T, CheckedArrayDeleteFuncObj<T> >(p) { }


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

template<typename T> inline T& ScopedArrayPtr<T>::operator[](int i) const
{
			   
	FW_ASSERT(ScopedArrayPtr<T>::Get() && (i >= 0));
	
	return (ScopedArrayPtr<T>::Get())[i];
}

//--------------------------------------------------------------------------------------------------
//	END NAMESPACE DEFINITION
//--------------------------------------------------------------------------------------------------

} // FwStd

//--------------------------------------------------------------------------------------------------

#endif // FW_STD_SCOPED_PTR_H
