/***************************************************************************************************
*
*	DESCRIPTION		The rendering pipeline for soft and stencil shadows.
*
*	NOTES			
*
***************************************************************************************************/

#ifndef GFX_SHADOWSYSTEM_H
#define GFX_SHADOWSYSTEM_H

#if defined( PLATFORM_PC )
#	include "gfx/shadowsystem_pc.h"
#elif defined( PLATFORM_PS3 )
#	include "gfx/shadowsystem_ps3.h"
#endif

#endif // ndef GFX_SHADOWSYSTEM_H
