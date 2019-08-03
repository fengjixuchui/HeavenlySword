/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		This SPU module *must* be uploaded to the fixed address that was specified
				at compile time
**/
//--------------------------------------------------------------------------------------------------

#include <jobapi/jobapi.h>
#include <jobapi/jobspudma.h>
#include <jobapi/jobdefinition.h>
#include <jobapi/jobprintf.h>

#include "fixedaddrmodule.h"

//--------------------------------------------------------------------------------------------------

extern "C"
{
U32 g_foo WWSJOB_UNINITIALIZED();
//This exists solely for us to take the address of.
//Taking the address at C++ level will automatically give us the correct location for the variable
//at run-time.  If the code is PIC, this will have been compensated for.
//Doing "ila" at assembler level will give use the correct compile time value for the g_foo label.
//If the code is compiled to a fixed position, both the C and asm levels should give the same result.
//If the code is PIC, they will give different results.
//(Note that this doesn't actually confirm that the code has been uploaded to the correct address).
}

//--------------------------------------------------------------------------------------------------

extern "C" void JobMain( void );

//--------------------------------------------------------------------------------------------------

void JobMain( void )
{
	// enable this line if you want to single step every time this job starts
	//WWSJOB_BREAKPOINT();

	JobPrintf( "***Fixed Addr Module: Begin\n" );

	//Read out parameters before starting next job loading
	//FixedAddrModuleParams params;
	//WwsJob_JobApiCopyParams( &params, sizeof(params) );

	//Start next job loading
	WwsJob_JobApiLoadNextJob();

	//do all the actual work *after* we've started the next job loading

	JobPrintf( "***Fixed Addr Module: Run-time located address of g_foo = 0x%X\n", (U32)&g_foo );

	U32* pFoo;
	asm( "ila	%0, g_foo" : "=r" (pFoo) );

	JobPrintf( "***Fixed Addr Module: Compile time address of g_foo = 0x%X\n", (U32)pFoo );

	WWSJOB_ASSERT( pFoo == &g_foo );	//These should be the same if it's fixed address code

	JobPrintf( "***Fixed Addr Module: Done\n" );
}

//--------------------------------------------------------------------------------------------------
