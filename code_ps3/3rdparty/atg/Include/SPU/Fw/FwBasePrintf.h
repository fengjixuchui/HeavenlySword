//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Replacement for C Standard Library 'printf', allowing additional filtering control.

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef	FW_BASE_PRINTF_H
#define	FW_BASE_PRINTF_H

// -------------------------------------------------------------------------------------------------

#include <spu_printf.h>

// -------------------------------------------------------------------------------------------------

#ifdef	__GNUC__
extern "C" void	FwBasePrintf( const char* pFormat, ... ) __attribute__( ( format( printf, 1, 2 ) ) );
extern "C" void	FwOldBasePrintf( const char* pFormat, ... ) __attribute__( ( format( printf, 1, 2 ) ) );
#endif	//__GNUC__

// -------------------------------------------------------------------------------------------------
// Framework-specific printf() replacement. 

#ifdef	ATG_PRINTF_ENABLED

	#ifdef KERNEL_SPU_THREADS
		#define FwPrintf	spu_printf
		#define FwOldPrintf	spu_printf
	#else
		#define	FwPrintf	FwBasePrintf
		#define	FwOldPrintf	FwOldBasePrintf
	#endif
#else
	extern "C" void	FwPrintf( const char* pFormat, ... ) __attribute__( ( format( printf, 1, 2 ) ) );
	inline	void	FwPrintf( const char*, ... )	{}
	extern "C" void	FwOldPrintf( const char* pFormat, ... ) __attribute__( ( format( printf, 1, 2 ) ) );
	inline	void	FwOldPrintf( const char*, ... )	{}
#endif	// ATG_PRINTF_ENABLED

#endif	// FW_BASE_PRINTF_H
