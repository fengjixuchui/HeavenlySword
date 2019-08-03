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
				Note that calls to JobPrintf can be conditionally compiled out based on
				PRINTF_ENABLED, whereas JobBasePrintf always exists.
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_JOB_PPU_PRINTF_H
#define WWS_JOB_PPU_PRINTF_H

//--------------------------------------------------------------------------------------------------

#ifdef PRINTF_ENABLED
	#define JobPrintf				JobBasePrintf
#else
	#define JobPrintf( ... )		do { } while ( 0 )
#endif

//--------------------------------------------------------------------------------------------------

extern "C" void JobBasePrintf( const char* fmt, ... ) __attribute__( ( format( printf, 1, 2 ) ) );

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_PPU_PRINTF_H */
