//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Base assertion macro definitions.

	@section BASE_ASSERTIONS_SECTION Base Assertions
	
	Base assertion macros are defined in all builds and provide a base upon which all assertions
	should be derived.
    
    Projects are free to derive from these to create their own project specific assertions; the
    inclusion and exclusion of which are defined by their own build rules.
	
	All API assertions are defined in terms of these base assertions also.
	
	@note	Neither API or project code should use these base assertion macros directly.
	
	
	@subsection BASE_RUN_TIME_ASSERTIONS_SECTION Base Run-time Assertions
    
	Base run-time assertions evaluate an expression at run-time - if this expression is false an
	assertion will be triggered and a run-time error message will be displayed. The base run-time
	assertions work in the same way as the ANSI %assert() macro.

	The exact behaviour of an triggered	assertion can be controlled by project code through the use
	of the static FwBaseAssertCtrl class. In this way, an application can decide how assertion 
	information is to be presented to the developer. Please see FwBaseAssertCtrl::DefaultHandler(), 
	in FwBaseAssert.cpp, for more information about replacment handler functionality.
	
	@note	Run-time assertions can only be used in function scope.
    
	
	@subsection BASE_STATIC_ASSERTIONS_SECTION Base Static Assertions
	
	Base static assertions evalulate a constant integral expression at compile time - if this expression
	is false an assertion will be triggered and a compile time error message will be displayed.
	
	In otherwords, a static assertion is a compile time equivalent of the ANSI %assert() macro. Differences
	include
	
		- 	When the condition is true the macro will not generate any code or data.
		
		-	Unlike normal run-time assertions, static asserts can also be used within namespace and class
			scope. This is in addition to traditional the function scope usage.
	
		- 	Static assertions are evaluated at template instantiation, as a result, they can be used to validate
			template parameters.	

	@note	For a detailed description of static assertions please refer to http://www.boost.org/
	

	@subsection ASSERTION_MESSAGES_SECTION Assertion Messages
	
	
	@subsubsection RUNTIME_ASSERTION_MESSAGES_SECTION Default Run-time Assertion Messages
	
	When using the default assertion handlers, all run-time assertion macros print out an assertion
	message in the following form:
	
	@code
	
		"Assertion failed: <expression>, file <filename>, line <line_number>"
	
	@endcode
	
	where
	
	@param <expression>		The assertion expression.
	@param <filename>		The name of the source file which triggered the assertion.
	@param <line_number>	The line number which triggered the assertion.
	
	
	In addition run-time assertion macros post-fixed with _MSG allow callers to add their own messages.
	All base assertions have _MSG variants.
	
	User-defined assertion messages are specified in the same form as %printf() arguments, that is,
	a format string followed by a variable argument list.
	
	@note Since ANSI macros do not support variable argument lists using ellipsis (...) all message
	arguments must be enclosed by parentheses.
	
    For example,
	
	@code
	
		// In this example, FW_ASSERT_MSG is derived from FW_BASE_ASSERT_MSG..
		FW_ASSERT_MSG(pResource->IsValid(), ("Resource %p is invalid!\n", pResource));
	
	@endcode
	
	As you can see in the example above the two message arguments (the format string and resource
	pointer 'pResource') have both been enclosed by parentheses.
	
	The easiest way to think about the assertion message format is as %printf() arguments -
	parentheses and all!
	
	
	@subsubsection STATIC_ASSERTION_MESSAGES_SECTION Static Assertion Messages
    
	Static assertion macros print an assertion message at compile time. Since this message is emitted
	by the compiler its exact form varies - depending on the compiler you're using.
	
	However, it should look something similar to the following
	
	@code
		
		"Illegal use of FW_STATIC_ASSERT_FAILED<false>"
	
	@endcode
	
	or specifically, in the case of Microsoft Visual Studio .NET 2003
	
	@code
		
		"error C2027: use of undefined type 'FW_STATIC_ASSERT_FAILED<B>' with [  B=false  ]	
	
	@endcode
	
	@note	Any compile time error containing a reference to %FW_STATIC_ASSERT_FAILED implies a static
			assertion failure.
			
	@note	Unlike run-time assertions user-defined messages cannot be added to static assertions.
	
	@note	(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef	FW_BASE_ASSERT_H
#define	FW_BASE_ASSERT_H

//--------------------------------------------------------------------------------------------------
//	MACRO DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
    @name   Helpers
**/
//--------------------------------------------------------------------------------------------------

//@{

//--------------------------------------------------------------------------------------------------
/**
	@brief  	Stops the code in a debugger friendly manner.

	@note 	    Used to implement the FW_BASE_ASSERT() macros.
**/
//--------------------------------------------------------------------------------------------------

#if defined(__PPC__)
#ifdef __SNC__
	#define	FW_BREAK() __builtin_stop()
#else
	#define	FW_BREAK() asm("trap")
#endif
#else
	#define	FW_BREAK() __debugbreak()
#endif

//@}

//--------------------------------------------------------------------------------------------------
/**
    @name   Base Run-time Assertions Support Class
**/
//--------------------------------------------------------------------------------------------------

//@{ 

class	FwBaseAssertCtrl
{
public:
	// Typedef used for our assertion handler
	typedef bool ( *AssertHandler )( const char* pCond, const char* pCondMsg, const char* pFile, int line );
	
	static	AssertHandler	SetHandler( AssertHandler pHandler );

	static	bool			InvokeHandler( const char* pCond, const char* pCondMsg, const char* pFile, int line );
	static	const char*		BuildConditionMsg( const char* pFormat, ... );

private:
	static	bool			DefaultHandler( const char* pCond, const char* pCondMsg, const char* pFile, int line );

	static	const int		kMaxConditionMsgSize = 512;					///< Size of condition message storage
	static	char			ms_conditionMsg[ kMaxConditionMsgSize ];	///< Static storage for condition message
	static	AssertHandler	ms_pHandler;								///< Pointer to current handler
};

///< Sets assertion handler.
inline	FwBaseAssertCtrl::AssertHandler	FwBaseAssertCtrl::SetHandler( AssertHandler pHandler )
{
	AssertHandler oldHandler = ms_pHandler;
	ms_pHandler = pHandler;
	return oldHandler;
}

///< Invokes assertion handler, passing on all parameters.
inline	bool	FwBaseAssertCtrl::InvokeHandler( const char* pCond, const char* pCondMsg, const char* pFile, int line )
{
	return ms_pHandler( pCond, pCondMsg, pFile, line );
}

//@}

//--------------------------------------------------------------------------------------------------
/**
    @name   Base Run-time Assertions
**/
//--------------------------------------------------------------------------------------------------

//@{

#define FW_BASE_ASSERT( condition )																	\
	do																								\
	{																								\
	    if ( FW_PREDICT( !( condition ), false ) )													\
		{																							\
			if ( FwBaseAssertCtrl::InvokeHandler( #condition, NULL, __FILE__, __LINE__  ) )			\
				FW_BREAK();																			\
		}																							\
	} while (false)

//--------------------------------------------------------------------------------------------------

#define FW_BASE_ASSERT_MSG( condition, condition_msg )										    	\
	do																								\
	{                                                                                               \
	    if ( FW_PREDICT( !( condition ), false ) )													\
		{																							\
			if ( FwBaseAssertCtrl::InvokeHandler( #condition, FwBaseAssertCtrl::BuildConditionMsg condition_msg, __FILE__, __LINE__ ) )	\
				FW_BREAK();																			\
		}                                                                                           \
	} while (false)

//@}

//--------------------------------------------------------------------------------------------------
/**
    @name   Base Static Assertions
**/
//--------------------------------------------------------------------------------------------------

//@{

// :NOTE: (vdiesi -- 11-5-4) Template for the implementation of static assertions.
//
// We only specialise for FW_STATIC_ASSERT_FAILED<true>. This causes the expression
//
//		sizeof(FW_STATIC_ASSERT_FAILED<false>)
//
// to be undefined upon assertion failure, in which case a compile time error message is displayed.

template <bool B> struct FW_STATIC_ASSERT_FAILED;

template<> struct FW_STATIC_ASSERT_FAILED<true> { };

//--------------------------------------------------------------------------------------------------

// :NOTE: (vdiesi -- 11-5-4) Static assertions often require some form of 'compiler magic'.
// As a result, their implementation tends to be very compiler specific. It's likely that
// this macro will need re-writing for other compilers.
//
// :NOTE: (vdiesi -- 11-5-4) Due to a bug in Microsoft Visual Studio .NET 2003 (vc7) we cannot use
// the generic Boost 1_31_0 implementation of static assert. From the Boost mailing list
//
//  "For MSVC7 use __COUNTER__ as __LINE__ macro broken with /ZI (see Q199057)"
//
// In addition, I've opted for the enum implementation rather than the typedef form.
//
// Trust me, resolving this problem was a complete and utter nightmare!


#if defined(_MSC_VER) && (_MSC_VER >= 1310)

#define FW_BASE_STATIC_ASSERT( condition )															\
		enum																						\
		{																							\
			FW_JOIN_MACRO_ARGS(FW_STATIC_ASSERT_ENUM_, __COUNTER__)									\
			= sizeof(FW_STATIC_ASSERT_FAILED< (bool)( condition ) >)								\
		}

#else

#define FW_BASE_STATIC_ASSERT( condition )															\
		enum																						\
		{																							\
			FW_JOIN_MACRO_ARGS(FW_STATIC_ASSERT_ENUM_, __LINE__)									\
			= sizeof(FW_STATIC_ASSERT_FAILED< (bool)( condition ) >)								\
		}
#endif

//@}

//--------------------------------------------------------------------------------------------------

#endif	//FW_BASE_ASSERT_H
