/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		The bootstrapper is required for loading the job manager if it is over 16K
				This is not needed for the downcoded version
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_JOB_BOOT_STRAP_MAIN_H
#define WWS_JOB_BOOT_STRAP_MAIN_H

//--------------------------------------------------------------------------------------------------

#include <wwsbase/types.h>

//--------------------------------------------------------------------------------------------------

extern "C" void BootStrapMain( uintptr_t kernel_context, U64 work_ea ) __attribute__((noreturn));

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_BOOT_STRAP_MAIN_H */
