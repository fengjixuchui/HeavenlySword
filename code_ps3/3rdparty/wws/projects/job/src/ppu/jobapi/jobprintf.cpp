/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		JobPrintf is a common function name for calling printf between PPU and SPU.
				On PPU it just passes through to printf.
				On SPU this function should be used instead of spu_printf as this version is
				interrupt safe.
				Note that JobPrintf can be conditionally compiled out based on PRINTF_ENABLED
				whereas JobBasePrintf always exists.
**/
//--------------------------------------------------------------------------------------------------

#include <jobapi/jobprintf.h>
#include <stdio.h>

//--------------------------------------------------------------------------------------------------

//Implementing this here means we don't have to <have stdio.h> being forced
//out in our header file into a everyone's source files
void JobBasePrintf( const char* fmt, ... )
{
	va_list args;

	va_start( args, fmt );
	vprintf( fmt, args );
	va_end( args );
}

//--------------------------------------------------------------------------------------------------
