/**
	\file fios.h

	Public API for the File I/O Scheduler.

    Copyright (C) Sony Computer Entertainment America, Inc., 2005-2006.  All rights reserved.
*/
/**
	\mainpage
 
	\author Drew Thaler, SCEA Tools and Technology (contractor). drew@multisolar.com
	\version FIOS 1.0rc5
	
	See the FIOS documentation on SHIP for an overview.
	 http://ship.scea.com/sf/docman/do/listDocuments/projects.file_io/docman.root
*/

#ifndef _H_fios
#define _H_fios
#pragma once

/*  Common SCEA headers.  */
#include "sceacommon/include/sceatargetmacros.h"
#include "sceacommon/include/sceabasetypes.h"

/*  System headers.  */
#include <stddef.h> /* size_t, ptrdiff_t */

/*
	Configuration. This is the only place that we explicitly don't use a relative
	include, so that you may override it easily by putting your own copy of this file
	higher in the include search path.
*/
#include "fios_configuration.h"

/*  Dependencies.  */
#if FIOS_MEMORYAPI_LIBMEM
# include "libmem/include/libmem.h"
#endif

#if FIOS_MEMORYAPI_SCEAMEM
# include "fios/include/fios_sceamem.h"
#endif

/*  FIOS support classes.  */
#include "fios/include/fios_common.h"
#include "fios/include/fios_types.h"
#include "fios/include/fios_platform.h"
#include "fios/include/fios_collections.h"

/*  FIOS primary classes.  */
#include "fios/include/fios_media.h"
#include "fios/include/fios_scheduler.h"

/*  FIOS media filter layers.  */
#include "fios/include/fios_catalogcache.h"

/*  Platform-specific stuff.  */
#if SCEA_TARGET_OS_PSP
# include "fios/include/psp/fios_psp.h"
#endif

#if SCEA_TARGET_OS_PS3_PPU || SCEA_TARGET_OS_PS3_PU
# include "fios/include/ps3/ppu/fios_ps3_ppu.h"
#endif

#if SCEA_TARGET_OS_WIN32
# include "fios/include/win32/fios_win32.h"
#endif

#if SCEA_TARGET_OS_UNIX
# include "fios/include/unix/fios_unix.h"
#endif


/**	Everything in the FIOS API is in the fios namespace. */
namespace fios {

/**
	\addtogroup Initialization Initialization
	@{
*/

/** \internal
	@{
*/
#define kFIOS_VERSION_STAGE_d      0
#define kFIOS_VERSION_STAGE_a      1
#define kFIOS_VERSION_STAGE_b      2
#define kFIOS_VERSION_STAGE_rc     3
#define kFIOS_VERSION_STAGE_f      4

#define FIOS_VERSION_MAJOR            ((FIOS_VERSION >> 24) & 0x00FF)
#define FIOS_VERSION_MINOR            ((FIOS_VERSION >> 20) & 0x000F)
#define FIOS_VERSION_BUGFIX           ((FIOS_VERSION >> 16) & 0x000F)
#define FIOS_VERSION_STAGE            ((FIOS_VERSION >> 12) & 0x000F)
#define FIOS_VERSION_STAGENUM         ((FIOS_VERSION >>  0) & 0x0FFF)

# define FIOS_VERSION_FROM_COMPONENTS(major, minor, bugfix, stage, stagenum) \
	(major < 1) ? ((((U32)(minor) & 0x0F) << 4) | (((U32)(bugfix) & 0x0F) << 0)): \
	(                                                        \
	(((U32)(major) & 0xFF) << 24)                          | \
	(((U32)(minor) & 0x0F) << 20)                          | \
	(((U32)(bugfix) & 0x0F) << 16)                         | \
	(((U32)(kFIOS_VERSION_STAGE_##stage) & 0x0F) << 12)    | \
	(((U32)(stagenum) & 0x0FFF) << 0)                        \
	)

# define FIOS_VERSION_STRING_FROM_COMPONENTS(major, minor, bugfix, stage, stagenum) \
	((bugfix == 0 && kFIOS_VERSION_STAGE_##stage == kFIOS_VERSION_STAGE_f) ? "" #major "." #minor : \
	 (bugfix == 0 && kFIOS_VERSION_STAGE_##stage != kFIOS_VERSION_STAGE_f) ? "" #major "." #minor #stage #stagenum : \
	 (bugfix > 0 && kFIOS_VERSION_STAGE_##stage == kFIOS_VERSION_STAGE_f)  ? "" #major "." #minor "." #bugfix : \
	                                                                         "" #major "." #minor "." #bugfix #stage #stagenum)

/** @} */

/**
	\def	FIOS_VERSION
	\brief  The FIOS header version as BCD.
	This indicates the version of the FIOS API header you are using. Newer versions
	always compare numerically greater than older versions.
    
	A new version scheme was started at FIOS 1.0.0rc1. The version scheme used for
	0.91 and earlier releases was different: 0.91 was 0x0091, etc. The older
	scheme still obeys numeric ordering relative to the new scheme: 0x0091 is less than 0x01004001.
*/
#define FIOS_VERSION	FIOS_VERSION_FROM_COMPONENTS(1,1,0,b,3)

/**
	\def    FIOS_VERSION_STRING
	\brief  The FIOS header version as a string.
	This indicates the version of the FIOS API header you are using, in a human-readable
	format suitable for printing. Please note: this value is not a C-string constant.
	Instead, it's an expression that evaluates to a C-string constant.
*/
#define FIOS_VERSION_STRING    FIOS_VERSION_STRING_FROM_COMPONENTS(1,1,0,b,3)

/*
	Global functions.
*/

/**
	\brief  Initializes FIOS
	\param[in]  pAllocator   The global allocator used for allocations that are not tied to a scheduler, such as media objects. Schedulers can be set to use this allocator, or a separate allocator.
*/
extern FIOS_EXPORT
void FIOSInit(FIOS_ALLOCATOR *pAllocator);

/**
	\brief Indicates whether FIOS has been initialized.
	\return True if FIOS has been initialized.
*/
extern FIOS_EXPORT
bool FIOSHasBeenInited();

/**
	\brief Returns the global FIOS memory allocator.
	\return The global allocator, or NULL if FIOS has not been initialized.
*/
extern FIOS_EXPORT
FIOS_ALLOCATOR * FIOSGetAllocator();

/*@}*/



}; /* namespace fios */

#endif /* _H_fios */

