//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Macros to help hide low level platform/compiler specific functionality.
	
	This file exists to abstract low level platform or compiler specific functionality. The macros
	present within it should rely only on the basic ATG SDK types, and the switches/macros defined
	as part of the default build process (including those in FwSwitches.h). Under no circumstances
	should this file make any reference to other ATG SDK functionality. This includes, but is not
	restricted to, assertions, printfs, or any of the contents of FwHelpers.h. 
	
	@note		(c) Copyright Sony Computer Entertainment 2006. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef	FW_PLATFORM_H
#define	FW_PLATFORM_H

//--------------------------------------------------------------------------------------------------
/**
	@brief			Branch Hinting 
	
	SPU code relies heavily on branch hinting in order to obtain reasonable performance. When this
	macro is used on X86, no branch hinting is performed - as it's not supported by that
	compiler. PPU compiler also seems to understand this hint.

	@param			cond		-	Condition to evaluate
	@param			expected	-	Expected result from evaluation
**/
//--------------------------------------------------------------------------------------------------

#if ( defined( __SPU__ ) || defined( __PPU__ ) )
#define FW_PREDICT( cond, expected )	__builtin_expect( ( cond ), ( expected ) )
#else
#define FW_PREDICT( cond, expected )	( cond ) 
#endif // ( defined( __SPU__ ) || defined( __PPU__ ) )

#endif	// FW_PLATFORM_H
