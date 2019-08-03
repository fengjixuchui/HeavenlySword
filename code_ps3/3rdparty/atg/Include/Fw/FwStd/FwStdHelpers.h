//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Framework Std namespace helpers.

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef FW_STD_HELPERS_H
#define FW_STD_HELPERS_H

//--------------------------------------------------------------------------------------------------
//	NAMESPACE DEFINITION
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@namespace		FwStd
	
	@brief			Framework C++ Standard Library functionality and extensions.

	Includes
	
	- STL replacements e.g. wrappers for STL containers using FwStd::Allocator as the default allocator.
	
	- STL extensions e.g. smart pointer varients, name map etc.
	
	- Other std like helper classes and functions.

	@note			Many of the std extensions are based on the Boost C++ Libraries. Please refer to
					http://www.boost.org/ for further details.
**/
//--------------------------------------------------------------------------------------------------

namespace FwStd
{
	
	// Class Declarations
	
	template<typename T> class CheckedDeleteFuncObj;
	template<typename T> class CheckedArrayDeleteFuncObj;
	
	
	// Function Declarations

	template<typename T> inline void CheckedDelete(T* const p);
	template<typename T> inline void CheckedArrayDelete(T* const p);


	// Tag classes
	
	namespace Detail
	{
		struct StaticCastTag {};
		struct DynamicCastTag {};
	}
}

//--------------------------------------------------------------------------------------------------
//	INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief		Replacement delete mechanism which fails at compile-time if type T is incomplete.

	Allows errors caused by deleting objects with incomplete types to be trapped at compile-time.
	For example, when a class with a non-trivial destructor or class-specific delete operator has
	an incomplete type.

	The effect of %FwStd::CheckedDelete(p) is equivalent to
	
		FW_DELETE( p );
		
		
	@param			p	Ptr to object being deleted.

	@see			@link CheckedDeleteFuncObj FwStd::CheckedDeleteFuncObj<T> @endlink
**/
//--------------------------------------------------------------------------------------------------

template<typename T> inline void FwStd::CheckedDelete(T* const p)
{

	typedef char typeMustBeComplete[sizeof(T) ? 1 : -1];
	(void) sizeof(typeMustBeComplete);
	
	FW_DELETE( p );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Replacement array delete mechanism which fails at compile-time if type T is incomplete.

	Allows errors caused by deleting arrays with incomplete types to be trapped at compile-time.
	For example, when a class with a non-trivial destructor or class-specific delete operator has
	an incomplete type.

	The effect of %FwStd::CheckedArrayDelete(p) is equivalent to
	
		FW_DELETE_ARRAY( p );
		
		
	@param			p	Ptr to array being deleted.

	@see			@link CheckedArrayDeleteFuncObj FwStd::CheckedArrayDeleteFuncObj<T> @endlink
**/
//--------------------------------------------------------------------------------------------------

template<typename T> inline void FwStd::CheckedArrayDelete(T* const p)
{
	
	typedef char typeMustBeComplete[sizeof(T) ? 1 : -1];
	(void) sizeof(typeMustBeComplete);
    
    FW_DELETE_ARRAY( p );
}

//--------------------------------------------------------------------------------------------------
//	CLASS DEFINITION
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@class
	
	@brief			Checked delete function object.

	@see			FwStd::CheckedDelete()
**/
//--------------------------------------------------------------------------------------------------

template<typename T> class FwStd::CheckedDeleteFuncObj
{
public:

	/**
		@brief	Checked delete function call operator.

		Invokes FwStd::CheckedDelete()
	**/
	
	void operator()(T* const p) const { FwStd::CheckedDelete(p); }
};

//--------------------------------------------------------------------------------------------------
//	CLASS DEFINITION
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@class
	
	@brief			Checked array delete function object.

	@see			FwStd::CheckedArrayDelete()
**/
//--------------------------------------------------------------------------------------------------

template<typename T> class FwStd::CheckedArrayDeleteFuncObj
{
public:
	
	/**
		@brief	Checked array delete function call operator.

		Invokes FwStd::CheckedArrayDelete()
	**/
	
	void operator()(T* const p) const { FwStd::CheckedArrayDelete(p); }
};

//--------------------------------------------------------------------------------------------------

#endif // FW_STD_HELPERS_H
