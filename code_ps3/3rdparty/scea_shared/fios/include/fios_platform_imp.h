/**
	\file fios_platform_imp.h

	Includes the appropriate implementation definitions for each platform.

    Copyright (C) Sony Computer Entertainment America, Inc., 2005-2006.  All rights reserved.
*/
#ifndef _H_fios_platform_imp
#define _H_fios_platform_imp

#include "sceacommon/include/sceatargetmacros.h"

#if SCEA_TARGET_OS_PSP
#include "psp/fios_platform_imp_psp.h"
#elif SCEA_TARGET_OS_PS3_PPU || SCEA_TARGET_OS_PS3_PU
#include "ps3/ppu/fios_platform_imp_ps3_ppu.h"
#elif SCEA_TARGET_OS_WIN32
#include "win32/fios_platform_imp_win32.h"
#elif SCEA_TARGET_OS_UNIX
#include "unix/fios_platform_imp_unix.h"
#else
#error "Unsupported target OS!"
#endif

#endif /* _H_fios_platform_imp */
