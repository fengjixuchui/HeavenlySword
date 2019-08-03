/**
	\file fios_ps3_ppu.h

	PS3 PPU-specific functions for FIOS.

    Copyright (C) Sony Computer Entertainment America, Inc., 2005-2006.  All rights reserved.
*/

#ifndef _H_fios_ps3_ppu
#define _H_fios_ps3_ppu
#include "sceacommon/include/sceatargetmacros.h"
#if SCEA_TARGET_OS_PS3_PPU || SCEA_TARGET_OS_PS3_PU

#include "fios_media_ps3_ppu.h"

namespace fios {


/** \brief Loads the PS3 module dependencies.
	This function is automatically called by FIOSInit, but you can also
	manually invoke it if you wish to help get the modules for several of your
	dependencies loaded at the same time.
*/
extern FIOS_EXPORT void FIOSPS3LoadModuleDependencies();

/** \brief Explicit list of FIOS PS3 module dependencies.
	This is an array of PS3 sysmodule IDs, terminated by CELL_SYSMODULE_INVALID.
	These are the modules loaded by FIOSPS3LoadModuleDependencies(). */
extern FIOS_EXPORT uint16_t g_FIOSPS3ModuleDependencies[];



#ifndef FIOS_SIZET_MAX
#define FIOS_SIZET_MAX   (size_t)(0xFFFFFFFF)
#endif



}; /* namespace fios */

#endif /* SCEA_TARGET_OS_PS3_PPU || SCEA_TARGET_OS_PS3_PU */
#endif /* _H_fios_ps3_ppu */
