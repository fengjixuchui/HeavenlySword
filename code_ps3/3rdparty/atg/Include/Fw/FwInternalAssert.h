//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Internal API Assertion macro definitions.

	@section API_ASSERTION_SECTION API Assertions
	
    These assertions are defined for internal use by the API i.e. the Framework and all other libraries
	and components. They are defined according to the API's build rules and are not intended to be used
	directly by project specific code. Projects should derive their own assertions from the base assertion
	macros.
	
	There are four types of API assertions
	
	@subsection API_RUN_TIME_ASSERTIONS_SECTION API Run-time Assertions

    Standard run-time assertions used by the API.
    
    @note   Defined only when ATG_ASSERTS_ENABLED is defined.

	
	@subsection API_STATIC_ASSERTIONS_SECTION API Static Assertions

    Static assertions used by the API.
    
    @note   Defined only when ATG_ASSERTS_ENABLED is defined.
    
	
	@subsection API_VERIFY_ASSERTIONS_SECTION API Verify Assertions
	
	Verify assertions used by the API - they are, assertions which always include their expression
	contents within the code, regardless of the API build type. In otherwords, %FW_VERIFY() acts just
	like %FW_ASSERT() when ATG_ASSERTS_ENABLED is defined, but when it is not defined the contents
	of the expression are still evaluated.
	
	For example,

	@code
	
	FW_VERIFY(Function(args));

	@endcode
	
	will ensure %Function() is always called. However, if %FW_ASSERT() were used, %Function()
	would not be called in a RELEASE build.

    %FW_VERIFY() can also be used as a "more robust" %FW_ASSERT(). Since the contents are always
	evaluated, %FW_VERIFY() acts more like a function than a macro (good C++ style).  So it is not
	possible for code that uses %FW_VERIFY() to mysteriously change its behaviour in a RELEASE build.
	In line with C++ style, this then relies on the optimizer to remove dead code, rather than
	using macros to remove it by hand ... but as always, a poor optimizer may not be as good as
	a macro at doing so.

    @note	Verify assertions are run-time assertions - implemented using the base run-time assertion
			macros.
	
	@note	Verify assertions will only be triggered when ATG_ASSERTS_ENABLED is defined, however,
			their expression code is always included. 
			
	@note	(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef	FW_INTERNAL_ASSERT_H
#define	FW_INTERNAL_ASSERT_H

//--------------------------------------------------------------------------------------------------
// We control ANSI assert()... so we make sure it's not already defined.

#undef	assert

//--------------------------------------------------------------------------------------------------
/**
    @name	API Run-time Assertions
**/
//--------------------------------------------------------------------------------------------------

// Define our public macro variants.. We need to control the definition of 'assert', as we're
// responsible for pulling in system headers in Fw.h.

//@{

#ifdef	ATG_ASSERTS_ENABLED

#define assert( condition )							FW_BASE_ASSERT( condition )
#define	FW_ASSERT( condition )						FW_BASE_ASSERT( condition )
#define FW_ASSERT_IF( condition1, condition2 )		FW_BASE_ASSERT( !(condition1) || (condition2) )
#define	FW_ASSERT_MSG( condition, condition_msg )	FW_BASE_ASSERT_MSG( condition, condition_msg )

#else

#define	assert( condition )							( ( void )0 )
#define	FW_ASSERT( condition )						( ( void )0 )
#define FW_ASSERT_IF( condition1, condition2 )		( ( void )0 )
#define	FW_ASSERT_MSG( condition, condition_msg )	( ( void )0 )

#endif	// ATG_ASSERTS_ENABLED

//@}

//--------------------------------------------------------------------------------------------------
/**
    @name	API Static Assertions
**/
//--------------------------------------------------------------------------------------------------

//@{

#ifdef	ATG_ASSERTS_ENABLED

#define	FW_STATIC_ASSERT( condition )				FW_BASE_STATIC_ASSERT( condition )

#else

#define	FW_STATIC_ASSERT( condition )

#endif	// ATG_ASSERTS_ENABLED

//@}

//--------------------------------------------------------------------------------------------------
/**
	@name	API Verify Assertions
**/
//--------------------------------------------------------------------------------------------------

//@{

#ifdef	ATG_ASSERTS_ENABLED

#define FW_VERIFY( condition )						FW_BASE_ASSERT( condition )
#define FW_VERIFY_IF( condition1, condition2 )		FW_BASE_ASSERT( !(condition1) || (condition2) )
#define FW_VERIFY_MSG( condition, condition_msg )	FW_BASE_ASSERT_MSG( condition, condition_msg )

#else

#define FW_VERIFY( condition )						( ( void )( condition ) )
#define FW_VERIFY_IF( condition1, condition2 )		( ( void )( !(condition1) || (condition2) ) )
#define FW_VERIFY_MSG( condition, condition_msg )	( ( void )( condition ) )

#endif	// ATG_ASSERTS_ENABLED

//@}

//--------------------------------------------------------------------------------------------------

#endif	//FW_INTERNAL_ASSERT_H
