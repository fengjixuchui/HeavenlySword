/***************************************************************************************************
*
*	DESCRIPTION
*
*	NOTES
*
***************************************************************************************************/

#ifndef GFX_SECTOR_H
#define GFX_SECTOR_H

#if defined( PLATFORM_PC )
#	include "gfx/sector_pc.h"
#elif defined( PLATFORM_PS3 )
#	include "gfx/sector_ps3.h"
#endif

#endif // ndef _SECTOR_H
