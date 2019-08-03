//--------------------------------------------------
//!
//!	\file hardwarecaps.h
//!	Encapsulate abstract capabilites of our hardware
//!
//--------------------------------------------------

#ifndef GFX_HARDWARECAPS_H
#define GFX_HARDWARECAPS_H

#if defined( PLATFORM_PC )
#	include "gfx/hardwarecaps_pc.h"
#elif defined( PLATFORM_PS3 )
#	include "gfx/hardwarecaps_ps3.h"
#endif

#endif // end GFX_HARDWARECAPS_H
